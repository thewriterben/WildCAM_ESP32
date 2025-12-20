/**
 * @file CloudManager.cpp
 * @brief Cloud Integration Manager Implementation for WildCAM ESP32
 * 
 * Implements automatic image upload to backend server when WiFi is connected.
 * 
 * @version 1.0.0
 * @date 2024
 */

#include "CloudManager.h"
#include "config.h"
#include <SD_MMC.h>

#if LOGGING_ENABLED
#include "Logger.h"
#endif

// HTTP timeout settings
static const int HTTP_CONNECT_TIMEOUT_MS = 5000;
static const int HTTP_TIMEOUT_MS = 30000;

CloudManager::CloudManager()
    : _initialized(false)
    , _uploadEnabled(false)
    , _totalUploads(0)
    , _successfulUploads(0)
    , _failedUploads(0)
    , _lastUploadTime(0) {
}

CloudManager::~CloudManager() {
    clearQueue();
}

bool CloudManager::init(const String& serverUrl, const String& deviceId, const String& apiKey) {
    if (serverUrl.length() == 0 || deviceId.length() == 0) {
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Invalid server URL or device ID");
        #endif
        Serial.println("CloudManager: Invalid server URL or device ID");
        return false;
    }
    
    _serverUrl = serverUrl;
    _deviceId = deviceId;
    _apiKey = apiKey;
    
    // Remove trailing slash from server URL if present
    if (_serverUrl.endsWith("/")) {
        _serverUrl = _serverUrl.substring(0, _serverUrl.length() - 1);
    }
    
    _initialized = true;
    _uploadEnabled = true;
    
    #if LOGGING_ENABLED
    LOG_INFO("CloudManager initialized - Server: %s, Device: %s", 
             _serverUrl.c_str(), _deviceId.c_str());
    #endif
    Serial.printf("CloudManager initialized - Server: %s, Device: %s\n", 
                  _serverUrl.c_str(), _deviceId.c_str());
    
    return true;
}

bool CloudManager::registerDevice(const String& name, const String& locationName,
                                   float latitude, float longitude) {
    if (!_initialized || !isConnected()) {
        #if LOGGING_ENABLED
        LOG_WARN("CloudManager: Cannot register - not initialized or no WiFi");
        #endif
        return false;
    }
    
    HTTPClient http;
    String url = _serverUrl + "/api/cameras/register";
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "application/json");
    
    if (_apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _apiKey);
    }
    
    // Build registration JSON
    StaticJsonDocument<512> doc;
    doc["device_id"] = _deviceId;
    doc["name"] = name;
    doc["location_name"] = locationName;
    doc["firmware_version"] = FIRMWARE_VERSION;
    
    if (latitude != 0.0 || longitude != 0.0) {
        doc["latitude"] = latitude;
        doc["longitude"] = longitude;
    }
    
    String payload;
    serializeJson(doc, payload);
    
    #if LOGGING_ENABLED
    LOG_DEBUG("CloudManager: Registering device at %s", url.c_str());
    #endif
    
    int httpCode = http.POST(payload);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        #if LOGGING_ENABLED
        LOG_INFO("CloudManager: Device registered successfully");
        #endif
        Serial.println("CloudManager: Device registered successfully");
        http.end();
        return true;
    } else {
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Device registration failed, HTTP code: %d", httpCode);
        #endif
        Serial.printf("CloudManager: Device registration failed, HTTP code: %d\n", httpCode);
        http.end();
        return false;
    }
}

CloudUploadStatus CloudManager::uploadImage(camera_fb_t* fb, 
                                             const String& species,
                                             float confidence,
                                             const String& additionalMetadata) {
    if (!_initialized) {
        return UPLOAD_FAILED_NETWORK;
    }
    
    if (!_uploadEnabled) {
        return UPLOAD_QUEUED;
    }
    
    if (!isConnected()) {
        #if LOGGING_ENABLED
        LOG_WARN("CloudManager: No WiFi connection for upload");
        #endif
        return UPLOAD_FAILED_NETWORK;
    }
    
    if (fb == nullptr || fb->buf == nullptr || fb->len == 0) {
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Invalid frame buffer");
        #endif
        return UPLOAD_FAILED_FILE;
    }
    
    _totalUploads++;
    
    // Build metadata JSON
    StaticJsonDocument<512> metadataDoc;
    metadataDoc["device_id"] = _deviceId;
    metadataDoc["timestamp"] = millis();
    
    if (species.length() > 0) {
        metadataDoc["species"] = species;
        metadataDoc["confidence"] = confidence;
    }
    
    // Parse and merge additional metadata if provided
    if (additionalMetadata.length() > 0) {
        StaticJsonDocument<256> additionalDoc;
        DeserializationError error = deserializeJson(additionalDoc, additionalMetadata);
        if (!error) {
            for (JsonPair kv : additionalDoc.as<JsonObject>()) {
                metadataDoc[kv.key()] = kv.value();
            }
        }
    }
    
    String metadata;
    serializeJson(metadataDoc, metadata);
    
    int httpCode = sendImagePost(_serverUrl + "/api/detections", fb->buf, fb->len, metadata);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        _successfulUploads++;
        _lastUploadTime = millis();
        #if LOGGING_ENABLED
        LOG_INFO("CloudManager: Image uploaded successfully (%d bytes)", fb->len);
        #endif
        Serial.printf("CloudManager: Image uploaded successfully (%d bytes)\n", fb->len);
        return UPLOAD_SUCCESS;
    } else if (httpCode < 0) {
        _failedUploads++;
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Upload failed - network error: %d", httpCode);
        #endif
        return UPLOAD_FAILED_NETWORK;
    } else {
        _failedUploads++;
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Upload failed - server error: %d", httpCode);
        #endif
        return UPLOAD_FAILED_SERVER;
    }
}

CloudUploadStatus CloudManager::uploadImageFromSD(const String& filePath, const String& metadata) {
    if (!_initialized || !_uploadEnabled) {
        return UPLOAD_FAILED_NETWORK;
    }
    
    if (!isConnected()) {
        // Queue for later if not connected
        queueUpload(filePath, metadata);
        return UPLOAD_QUEUED;
    }
    
    // Read file from SD card
    File file = SD_MMC.open(filePath, FILE_READ);
    if (!file) {
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Failed to open file: %s", filePath.c_str());
        #endif
        return UPLOAD_FAILED_FILE;
    }
    
    size_t fileSize = file.size();
    if (fileSize == 0 || fileSize > CLOUD_MAX_FILE_SIZE) {
        file.close();
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Invalid file size: %d (max: %d)", fileSize, CLOUD_MAX_FILE_SIZE);
        #endif
        return UPLOAD_FAILED_FILE;
    }
    
    // Allocate buffer and read file
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (buffer == nullptr) {
        file.close();
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Failed to allocate buffer for file");
        #endif
        return UPLOAD_FAILED_FILE;
    }
    
    size_t bytesRead = file.read(buffer, fileSize);
    file.close();
    
    if (bytesRead != fileSize) {
        free(buffer);
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Failed to read file completely");
        #endif
        return UPLOAD_FAILED_FILE;
    }
    
    _totalUploads++;
    
    // Build metadata if not provided
    String finalMetadata = metadata;
    if (finalMetadata.length() == 0) {
        StaticJsonDocument<256> doc;
        doc["device_id"] = _deviceId;
        doc["timestamp"] = millis();
        doc["file_path"] = filePath;
        serializeJson(doc, finalMetadata);
    }
    
    int httpCode = sendImagePost(_serverUrl + "/api/detections", buffer, fileSize, finalMetadata);
    free(buffer);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        _successfulUploads++;
        _lastUploadTime = millis();
        #if LOGGING_ENABLED
        LOG_INFO("CloudManager: File uploaded successfully: %s", filePath.c_str());
        #endif
        return UPLOAD_SUCCESS;
    } else if (httpCode < 0) {
        _failedUploads++;
        return UPLOAD_FAILED_NETWORK;
    } else {
        _failedUploads++;
        return UPLOAD_FAILED_SERVER;
    }
}

int CloudManager::sendImagePost(const String& url, const uint8_t* imageData, size_t imageSize, const String& metadata) {
    HTTPClient http;
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.setConnectTimeout(HTTP_CONNECT_TIMEOUT_MS);
    
    if (_apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _apiKey);
    }
    
    // Create multipart form data with unique boundary using device ID and timestamp
    // This ensures uniqueness even if random() produces same value
    String boundary = "----WildCAM" + _deviceId.substring(0, 8) + String(millis()) + String(random(1000, 9999));
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    
    // Build multipart body
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"metadata\"\r\n\r\n";
    bodyStart += metadata + "\r\n";
    bodyStart += "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";
    
    String bodyEnd = "\r\n--" + boundary + "--\r\n";
    
    size_t totalSize = bodyStart.length() + imageSize + bodyEnd.length();
    
    // Allocate buffer for complete body
    uint8_t* body = (uint8_t*)malloc(totalSize);
    if (body == nullptr) {
        #if LOGGING_ENABLED
        LOG_ERROR("CloudManager: Failed to allocate body buffer");
        #endif
        http.end();
        return -1;
    }
    
    // Assemble body
    size_t offset = 0;
    memcpy(body + offset, bodyStart.c_str(), bodyStart.length());
    offset += bodyStart.length();
    memcpy(body + offset, imageData, imageSize);
    offset += imageSize;
    memcpy(body + offset, bodyEnd.c_str(), bodyEnd.length());
    
    // Send request
    int httpCode = http.POST(body, totalSize);
    
    free(body);
    http.end();
    
    return httpCode;
}

bool CloudManager::queueUpload(const String& filePath, const String& metadata) {
    if (_uploadQueue.size() >= MAX_QUEUE_SIZE) {
        #if LOGGING_ENABLED
        LOG_WARN("CloudManager: Upload queue full, dropping oldest item");
        #endif
        _uploadQueue.erase(_uploadQueue.begin());
    }
    
    UploadQueueItem item;
    item.filePath = filePath;
    item.metadata = metadata;
    item.retryCount = 0;
    item.timestamp = millis();
    
    _uploadQueue.push_back(item);
    
    #if LOGGING_ENABLED
    LOG_DEBUG("CloudManager: Queued upload for %s (queue size: %d)", 
              filePath.c_str(), _uploadQueue.size());
    #endif
    
    return true;
}

bool CloudManager::reportStatus(float batteryVoltage, int batteryPercent,
                                 float temperature, float humidity) {
    if (!_initialized || !isConnected()) {
        return false;
    }
    
    HTTPClient http;
    String url = _serverUrl + "/api/cameras/" + _deviceId + "/status";
    
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "application/json");
    
    if (_apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _apiKey);
    }
    
    StaticJsonDocument<256> doc;
    doc["battery_level"] = batteryPercent;
    doc["battery_voltage"] = batteryVoltage;
    
    if (temperature != 0.0) {
        doc["temperature"] = temperature;
    }
    if (humidity != 0.0) {
        doc["humidity"] = humidity;
    }
    
    String payload;
    serializeJson(doc, payload);
    
    int httpCode = http.POST(payload);
    http.end();
    
    if (httpCode == HTTP_CODE_OK) {
        #if LOGGING_ENABLED
        LOG_DEBUG("CloudManager: Status reported successfully");
        #endif
        return true;
    } else {
        #if LOGGING_ENABLED
        LOG_WARN("CloudManager: Status report failed, HTTP code: %d", httpCode);
        #endif
        return false;
    }
}

int CloudManager::process() {
    if (!_initialized || !_uploadEnabled || !isConnected()) {
        return 0;
    }
    
    return processQueue();
}

int CloudManager::processQueue() {
    int processed = 0;
    std::vector<size_t> itemsToRemove;
    
    for (size_t i = 0; i < _uploadQueue.size() && processed < 3; i++) {
        UploadQueueItem& item = _uploadQueue[i];
        
        CloudUploadStatus status = uploadImageFromSD(item.filePath, item.metadata);
        
        if (status == UPLOAD_SUCCESS) {
            itemsToRemove.push_back(i);
            processed++;
        } else if (status == UPLOAD_FAILED_FILE) {
            // File doesn't exist or is invalid, remove from queue
            itemsToRemove.push_back(i);
        } else {
            // Network or server error, increment retry count
            item.retryCount++;
            if (item.retryCount >= MAX_RETRY_COUNT) {
                #if LOGGING_ENABLED
                LOG_WARN("CloudManager: Max retries reached for %s, removing from queue", 
                         item.filePath.c_str());
                #endif
                itemsToRemove.push_back(i);
            }
        }
    }
    
    // Remove processed/failed items (in reverse order to maintain indices)
    for (int i = itemsToRemove.size() - 1; i >= 0; i--) {
        _uploadQueue.erase(_uploadQueue.begin() + itemsToRemove[i]);
    }
    
    return processed;
}

void CloudManager::clearQueue() {
    _uploadQueue.clear();
}

String CloudManager::getStatisticsJson() const {
    StaticJsonDocument<256> doc;
    doc["total_uploads"] = _totalUploads;
    doc["successful_uploads"] = _successfulUploads;
    doc["failed_uploads"] = _failedUploads;
    doc["queue_size"] = _uploadQueue.size();
    doc["last_upload_time"] = _lastUploadTime;
    doc["upload_enabled"] = _uploadEnabled;
    doc["connected"] = isConnected();
    
    String result;
    serializeJson(doc, result);
    return result;
}
