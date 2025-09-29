/**
 * @file model_manager.cpp
 * @brief Model Manager implementation for OTA model updates and versioning
 */

#include "model_manager.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_log.h>
#include <mbedtls/sha256.h>

static const char* TAG = "ModelManager";

// Default model paths
const char* MODEL_PATHS[] = {
    "/models/species_classifier_v1.0.0.tflite",
    "/models/behavior_analyzer_v1.0.0.tflite", 
    "/models/motion_detector_v1.0.0.tflite",
    "/models/object_detector_v1.0.0.tflite"
};

ModelManager::ModelManager() 
    : modelCount_(0)
    , otaStatus_(OTA_IDLE)
    , updateProgress_(0)
    , initialized_(false) {
    
    memset(models_, 0, sizeof(models_));
}

ModelManager::~ModelManager() {
    // Cleanup any loaded models
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].loaded) {
            unloadModel(models_[i].type);
        }
    }
}

bool ModelManager::initialize() {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing Model Manager");

    // Initialize SPIFFS for model storage
    if (!SPIFFS.begin(false)) {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS");
        return false;
    }

    // Initialize default model configurations
    initializeDefaultModels();

    // Try to load existing manifest
    if (SPIFFS.exists("/models/model_manifest.json")) {
        loadManifest("/models/model_manifest.json");
    }

    initialized_ = true;
    ESP_LOGI(TAG, "Model Manager initialized with %d models", modelCount_);

    return true;
}

void ModelManager::initializeDefaultModels() {
    // Species Classifier
    models_[0].type = MODEL_SPECIES_CLASSIFIER;
    strcpy(models_[0].name, "Species Classifier");
    strcpy(models_[0].version, "1.0.0");
    strcpy(models_[0].filename, "species_classifier_v1.0.0.tflite");
    models_[0].modelSize = 524288;  // 512KB
    models_[0].arenaSize = 524288;
    models_[0].accuracy = 0.92f;
    models_[0].inferenceTimeMs = 1200;
    
    // Behavior Analyzer
    models_[1].type = MODEL_BEHAVIOR_ANALYZER;
    strcpy(models_[1].name, "Behavior Analyzer");
    strcpy(models_[1].version, "1.0.0");
    strcpy(models_[1].filename, "behavior_analyzer_v1.0.0.tflite");
    models_[1].modelSize = 307200;  // 300KB
    models_[1].arenaSize = 384000;
    models_[1].accuracy = 0.87f;
    models_[1].inferenceTimeMs = 800;
    
    // Motion Detector
    models_[2].type = MODEL_MOTION_DETECTOR;
    strcpy(models_[2].name, "Motion Detector");
    strcpy(models_[2].version, "1.0.0");
    strcpy(models_[2].filename, "motion_detector_v1.0.0.tflite");
    models_[2].modelSize = 102400;  // 100KB
    models_[2].arenaSize = 128000;
    models_[2].accuracy = 0.95f;
    models_[2].inferenceTimeMs = 200;
    
    // Object Detector
    models_[3].type = MODEL_OBJECT_DETECTOR;
    strcpy(models_[3].name, "Object Detector");
    strcpy(models_[3].version, "1.0.0");
    strcpy(models_[3].filename, "object_detector_v1.0.0.tflite");
    models_[3].modelSize = 1048576;  // 1MB
    models_[3].arenaSize = 1048576;
    models_[3].accuracy = 0.90f;
    models_[3].inferenceTimeMs = 1500;
    
    modelCount_ = 4;
}

bool ModelManager::loadManifest(const char* manifestPath) {
    ESP_LOGI(TAG, "Loading manifest from: %s", manifestPath);
    
    File manifestFile = SPIFFS.open(manifestPath, "r");
    if (!manifestFile) {
        ESP_LOGE(TAG, "Failed to open manifest file");
        return false;
    }

    String jsonData = manifestFile.readString();
    manifestFile.close();

    return parseManifest(jsonData.c_str());
}

bool ModelManager::parseManifest(const char* jsonData) {
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        ESP_LOGE(TAG, "Failed to parse manifest JSON: %s", error.c_str());
        return false;
    }

    JsonArray modelsArray = doc["models"];
    modelCount_ = 0;

    for (JsonObject model : modelsArray) {
        if (modelCount_ >= MAX_MODELS) break;

        ModelInfo* info = &models_[modelCount_];
        
        // Parse model type
        String modelName = model["name"];
        if (modelName == "species_classifier") {
            info->type = MODEL_SPECIES_CLASSIFIER;
        } else if (modelName == "behavior_analyzer") {
            info->type = MODEL_BEHAVIOR_ANALYZER;
        } else if (modelName == "motion_detector") {
            info->type = MODEL_MOTION_DETECTOR;
        } else if (modelName == "object_detector") {
            info->type = MODEL_OBJECT_DETECTOR;
        } else {
            continue; // Skip unknown model types
        }

        // Parse model information
        strcpy(info->name, model["description"]);
        strcpy(info->version, model["version"]);
        strcpy(info->filename, model["filename"]);
        info->modelSize = model["model_size_bytes"];
        info->arenaSize = model["arena_size_bytes"];
        info->accuracy = model["accuracy"];
        info->inferenceTimeMs = model["inference_time_ms"];

        // Parse checksum if available
        if (model.containsKey("checksum")) {
            String checksumStr = model["checksum"];
            if (checksumStr.startsWith("sha256:")) {
                // Convert hex string to bytes (simplified implementation)
                // In production, use proper hex to bytes conversion
                memset(info->checksum, 0, MODEL_CHECKSUM_LEN);
            }
        }

        modelCount_++;
    }

    ESP_LOGI(TAG, "Loaded %d models from manifest", modelCount_);
    return true;
}

bool ModelManager::loadModel(ModelType type) {
    ModelInfo* info = nullptr;
    
    // Find model info
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].type == type) {
            info = &models_[i];
            break;
        }
    }

    if (!info) {
        ESP_LOGE(TAG, "Model type %d not found", type);
        return false;
    }

    if (info->loaded) {
        ESP_LOGW(TAG, "Model %s already loaded", info->name);
        return true;
    }

    ESP_LOGI(TAG, "Loading model: %s", info->name);

    // Check if model file exists
    String filepath = String("/models/") + info->filename;
    if (!SPIFFS.exists(filepath.c_str())) {
        ESP_LOGE(TAG, "Model file not found: %s", filepath.c_str());
        return false;
    }

    // Validate model if checksum available
    if (!validateModel(type)) {
        ESP_LOGE(TAG, "Model validation failed: %s", info->name);
        return false;
    }

    info->loaded = true;
    ESP_LOGI(TAG, "Model loaded successfully: %s", info->name);
    return true;
}

void ModelManager::unloadModel(ModelType type) {
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].type == type && models_[i].loaded) {
            models_[i].loaded = false;
            ESP_LOGI(TAG, "Model unloaded: %s", models_[i].name);
            break;
        }
    }
}

const ModelInfo* ModelManager::getModelInfo(ModelType type) const {
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].type == type) {
            return &models_[i];
        }
    }
    return nullptr;
}

bool ModelManager::validateModel(ModelType type) {
    const ModelInfo* info = getModelInfo(type);
    if (!info) {
        return false;
    }

    String filepath = String("/models/") + info->filename;
    if (!SPIFFS.exists(filepath.c_str())) {
        return false;
    }

    // For now, just check file size
    File modelFile = SPIFFS.open(filepath.c_str(), "r");
    if (!modelFile) {
        return false;
    }

    size_t fileSize = modelFile.size();
    modelFile.close();

    bool valid = (fileSize == info->modelSize);
    ESP_LOGI(TAG, "Model validation %s: %s (size: %zu/%zu)", 
             valid ? "passed" : "failed", info->name, fileSize, info->modelSize);
    
    return valid;
}

bool ModelManager::checkForUpdates(const char* serverUrl) {
    if (!WiFi.isConnected()) {
        ESP_LOGE(TAG, "WiFi not connected");
        return false;
    }

    otaStatus_ = OTA_CHECKING;
    ESP_LOGI(TAG, "Checking for updates from: %s", serverUrl);

    HTTPClient http;
    http.begin(String(serverUrl) + "/api/models/latest");
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        ESP_LOGI(TAG, "Update check response received");
        
        // Parse response to check for updates
        // Simplified implementation - in production, compare versions
        otaStatus_ = OTA_IDLE;
        http.end();
        return true;
    } else {
        ESP_LOGE(TAG, "Update check failed: %d", httpCode);
        otaStatus_ = OTA_ERROR;
        http.end();
        return false;
    }
}

bool ModelManager::updateModel(ModelType type, const char* updateUrl) {
    const ModelInfo* info = getModelInfo(type);
    if (!info) {
        ESP_LOGE(TAG, "Model not found for update");
        return false;
    }

    otaStatus_ = OTA_DOWNLOADING;
    updateProgress_ = 0;

    ESP_LOGI(TAG, "Starting update for model: %s", info->name);

    String tempPath = String("/tmp/") + info->filename;
    
    // Download new model
    if (!downloadFile(updateUrl, tempPath.c_str(), nullptr)) {
        ESP_LOGE(TAG, "Failed to download model update");
        otaStatus_ = OTA_ERROR;
        return false;
    }

    otaStatus_ = OTA_VALIDATING;
    
    // Validate downloaded model (simplified validation)
    File tempFile = SPIFFS.open(tempPath.c_str(), "r");
    if (!tempFile || tempFile.size() == 0) {
        ESP_LOGE(TAG, "Downloaded model is invalid");
        otaStatus_ = OTA_ERROR;
        return false;
    }
    tempFile.close();

    otaStatus_ = OTA_INSTALLING;

    // Replace old model with new one
    String modelPath = String("/models/") + info->filename;
    SPIFFS.remove(modelPath.c_str());
    SPIFFS.rename(tempPath.c_str(), modelPath.c_str());

    otaStatus_ = OTA_SUCCESS;
    ESP_LOGI(TAG, "Model update completed successfully: %s", info->name);
    
    return true;
}

bool ModelManager::downloadFile(const char* url, const char* filepath, 
                               void (*progressCallback)(uint8_t progress)) {
    HTTPClient http;
    http.begin(url);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        ESP_LOGE(TAG, "HTTP GET failed: %d", httpCode);
        http.end();
        return false;
    }

    File file = SPIFFS.open(filepath, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to create file: %s", filepath);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t totalSize = http.getSize();
    size_t written = 0;

    uint8_t buffer[1024];
    while (http.connected() && written < totalSize) {
        size_t available = stream->available();
        if (available) {
            size_t bytesToRead = min(available, sizeof(buffer));
            size_t bytesRead = stream->readBytes(buffer, bytesToRead);
            file.write(buffer, bytesRead);
            written += bytesRead;

            if (progressCallback && totalSize > 0) {
                uint8_t progress = (written * 100) / totalSize;
                progressCallback(progress);
                updateProgress_ = progress;
            }
        }
        vTaskDelay(1);
    }

    file.close();
    http.end();

    ESP_LOGI(TAG, "Downloaded %zu bytes to %s", written, filepath);
    return written == totalSize;
}

size_t ModelManager::getTotalMemoryUsage() const {
    size_t total = 0;
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].loaded) {
            total += models_[i].modelSize;
        }
    }
    return total;
}

uint8_t ModelManager::getLoadedModelCount() const {
    uint8_t count = 0;
    for (int i = 0; i < modelCount_; i++) {
        if (models_[i].loaded) {
            count++;
        }
    }
    return count;
}

uint8_t ModelManager::listModels(ModelInfo* models) const {
    if (!models) return 0;
    
    memcpy(models, models_, modelCount_ * sizeof(ModelInfo));
    return modelCount_;
}

const char* ModelManager::getModelTypeString(ModelType type) const {
    switch (type) {
        case MODEL_SPECIES_CLASSIFIER: return "Species Classifier";
        case MODEL_BEHAVIOR_ANALYZER: return "Behavior Analyzer";
        case MODEL_MOTION_DETECTOR: return "Motion Detector";
        case MODEL_OBJECT_DETECTOR: return "Object Detector";
        default: return "Unknown";
    }
}