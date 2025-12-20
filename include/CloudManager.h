#ifndef CLOUD_MANAGER_H
#define CLOUD_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_camera.h>
#include <vector>

/**
 * @file CloudManager.h
 * @brief Cloud Integration Manager for WildCAM ESP32
 * 
 * Provides automatic image upload to backend server when WiFi is connected.
 * Supports:
 * - Automatic image upload with metadata
 * - Device registration and status reporting
 * - Queue-based upload with retry logic
 * - Bandwidth-aware upload scheduling
 * 
 * @version 1.0.0
 * @date 2024
 */

/**
 * @brief Upload queue item structure
 */
struct UploadQueueItem {
    String filePath;           ///< Path to the image file on SD card
    String metadata;           ///< JSON metadata string
    uint8_t retryCount;        ///< Number of upload attempts
    unsigned long timestamp;   ///< When the item was queued
};

/**
 * @brief Cloud upload status
 */
enum CloudUploadStatus {
    UPLOAD_SUCCESS = 0,
    UPLOAD_FAILED_NETWORK,
    UPLOAD_FAILED_SERVER,
    UPLOAD_FAILED_FILE,
    UPLOAD_FAILED_TIMEOUT,
    UPLOAD_QUEUED
};

/**
 * @brief CloudManager class for handling cloud connectivity
 * 
 * This class manages all cloud-related operations including:
 * - Device registration with the backend
 * - Image upload with metadata
 * - Status reporting
 * - Queue management for offline uploads
 */
class CloudManager {
private:
    String _serverUrl;              ///< Backend server URL
    String _deviceId;               ///< Unique device identifier
    String _apiKey;                 ///< API key for authentication
    bool _initialized;              ///< Initialization status
    bool _uploadEnabled;            ///< Whether uploads are enabled
    
    // Upload queue for offline operation
    std::vector<UploadQueueItem> _uploadQueue;
    static const size_t MAX_QUEUE_SIZE = 50;
    static const uint8_t MAX_RETRY_COUNT = 3;
    static const unsigned long UPLOAD_TIMEOUT_MS = 30000;
    
    // Statistics
    unsigned long _totalUploads;
    unsigned long _successfulUploads;
    unsigned long _failedUploads;
    unsigned long _lastUploadTime;
    
    /**
     * @brief Send HTTP POST request with image data
     * @param url Target URL
     * @param imageData Image buffer
     * @param imageSize Size of image data
     * @param metadata JSON metadata string
     * @return HTTP response code
     */
    int sendImagePost(const String& url, const uint8_t* imageData, size_t imageSize, const String& metadata);
    
    /**
     * @brief Process items in the upload queue
     * @return Number of items successfully processed
     */
    int processQueue();

public:
    /**
     * @brief Construct a new CloudManager object
     */
    CloudManager();
    
    /**
     * @brief Destroy the CloudManager object
     */
    ~CloudManager();
    
    /**
     * @brief Initialize the cloud manager
     * @param serverUrl Backend server URL (e.g., "http://192.168.1.100:5000")
     * @param deviceId Unique device identifier
     * @param apiKey Optional API key for authentication
     * @return true if initialization successful
     */
    bool init(const String& serverUrl, const String& deviceId, const String& apiKey = "");
    
    /**
     * @brief Check if cloud manager is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }
    
    /**
     * @brief Check if WiFi is connected
     * @return true if WiFi is connected
     */
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    
    /**
     * @brief Enable or disable cloud uploads
     * @param enabled true to enable uploads
     */
    void setUploadEnabled(bool enabled) { _uploadEnabled = enabled; }
    
    /**
     * @brief Check if uploads are enabled
     * @return true if uploads are enabled
     */
    bool isUploadEnabled() const { return _uploadEnabled; }
    
    /**
     * @brief Register device with the backend server
     * @param name Device name
     * @param location Location name
     * @param latitude GPS latitude (optional)
     * @param longitude GPS longitude (optional)
     * @return true if registration successful
     */
    bool registerDevice(const String& name, const String& location, 
                        float latitude = 0.0, float longitude = 0.0);
    
    /**
     * @brief Upload camera frame buffer to cloud
     * @param fb Camera frame buffer
     * @param species Detected species (optional)
     * @param confidence Detection confidence (optional)
     * @param additionalMetadata Additional JSON metadata (optional)
     * @return CloudUploadStatus indicating result
     */
    CloudUploadStatus uploadImage(camera_fb_t* fb, 
                                   const String& species = "",
                                   float confidence = 0.0,
                                   const String& additionalMetadata = "");
    
    /**
     * @brief Upload image from SD card to cloud
     * @param filePath Path to image file on SD card
     * @param metadata JSON metadata string
     * @return CloudUploadStatus indicating result
     */
    CloudUploadStatus uploadImageFromSD(const String& filePath, const String& metadata = "");
    
    /**
     * @brief Queue an image for later upload
     * @param filePath Path to image file on SD card
     * @param metadata JSON metadata string
     * @return true if queued successfully
     */
    bool queueUpload(const String& filePath, const String& metadata = "");
    
    /**
     * @brief Report device status to backend
     * @param batteryVoltage Current battery voltage
     * @param batteryPercent Battery percentage
     * @param temperature Device temperature (optional)
     * @param humidity Humidity reading (optional)
     * @return true if status reported successfully
     */
    bool reportStatus(float batteryVoltage, int batteryPercent,
                      float temperature = 0.0, float humidity = 0.0);
    
    /**
     * @brief Process pending uploads in the queue
     * Call this regularly to upload queued images
     * @return Number of items processed
     */
    int process();
    
    /**
     * @brief Get the number of items in upload queue
     * @return Queue size
     */
    size_t getQueueSize() const { return _uploadQueue.size(); }
    
    /**
     * @brief Get total upload count
     * @return Total number of upload attempts
     */
    unsigned long getTotalUploads() const { return _totalUploads; }
    
    /**
     * @brief Get successful upload count
     * @return Number of successful uploads
     */
    unsigned long getSuccessfulUploads() const { return _successfulUploads; }
    
    /**
     * @brief Get failed upload count
     * @return Number of failed uploads
     */
    unsigned long getFailedUploads() const { return _failedUploads; }
    
    /**
     * @brief Get last upload timestamp
     * @return Milliseconds since last successful upload
     */
    unsigned long getLastUploadTime() const { return _lastUploadTime; }
    
    /**
     * @brief Clear the upload queue
     */
    void clearQueue();
    
    /**
     * @brief Get upload statistics as JSON
     * @return JSON string with statistics
     */
    String getStatisticsJson() const;
};

#endif // CLOUD_MANAGER_H
