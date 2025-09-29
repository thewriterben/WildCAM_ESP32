/**
 * @file mobile_api.h
 * @brief Mobile-optimized API endpoints for ESP32 Wildlife Camera
 * 
 * Provides bandwidth-optimized and touch-friendly API endpoints
 * specifically designed for mobile device interaction.
 */

#ifndef MOBILE_API_H
#define MOBILE_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

/**
 * Mobile API endpoint configuration
 */
struct MobileAPIConfig {
    bool enableBandwidthOptimization = true;
    uint32_t maxImageSize = 1024 * 1024; // 1MB max for mobile
    uint32_t thumbnailSize = 150; // 150x150 thumbnails
    uint32_t previewSize = 640; // 640x480 preview
    uint32_t updateInterval = 3000; // 3 seconds for mobile updates
    bool enableBurstMode = true;
    uint8_t maxBurstCount = 10;
    uint32_t burstInterval = 500; // 500ms between burst shots
    
    MobileAPIConfig() {}
};

/**
 * Mobile-specific system status with reduced data
 */
struct MobileSystemStatus {
    // Essential status only
    int batteryPercentage = 0;
    bool isCharging = false;
    bool cameraReady = false;
    bool wifiConnected = false;
    int wifiSignal = 0;
    uint32_t freeStorage = 0;
    uint32_t imageCount = 0;
    bool monitoringActive = false;
    
    // Last detection info
    String lastSpecies = "";
    float lastConfidence = 0.0;
    uint32_t lastDetectionTime = 0;
    
    // Alert flags
    bool lowBattery = false;
    bool lowStorage = false;
    bool errorState = false;
    
    MobileSystemStatus() {}
};

/**
 * Mobile capture request parameters
 */
struct MobileCaptureRequest {
    enum CaptureMode {
        SINGLE = 0,
        BURST = 1,
        TIMER = 2,
        PREVIEW = 3
    };
    
    CaptureMode mode = SINGLE;
    uint8_t burstCount = 5;
    uint32_t timerDelay = 5000; // 5 seconds
    uint16_t quality = 85; // JPEG quality 0-100
    bool saveToSD = true;
    bool generateThumbnail = true;
    
    MobileCaptureRequest() {}
};

/**
 * Mobile camera settings
 */
struct MobileCameraSettings {
    int brightness = 0; // -2 to +2
    int contrast = 0;   // -2 to +2
    int saturation = 0; // -2 to +2
    bool autoExposure = true;
    int exposureValue = 0; // -2 to +2 if manual
    bool enableFlash = false;
    
    // Mobile-specific settings
    bool lowPowerMode = false;
    bool adaptiveQuality = true;
    uint16_t mobileQuality = 70; // Reduced quality for mobile
    
    MobileCameraSettings() {}
};

/**
 * Mobile notification settings
 */
struct MobileNotificationSettings {
    bool enableWildlifeAlerts = true;
    bool enableBatteryAlerts = true;
    bool enableStorageAlerts = true;
    bool enableSystemAlerts = true;
    
    // Alert thresholds
    int batteryThreshold = 20; // Alert when battery < 20%
    int storageThreshold = 10; // Alert when storage < 10%
    
    // Notification timing
    uint32_t quietHoursStart = 22 * 3600; // 10 PM
    uint32_t quietHoursEnd = 6 * 3600;    // 6 AM
    bool respectQuietHours = true;
    
    MobileNotificationSettings() {}
};

/**
 * Mobile API class for handling mobile-specific endpoints
 */
class MobileAPI {
private:
    MobileAPIConfig config_;
    MobileCameraSettings cameraSettings_;
    MobileNotificationSettings notificationSettings_;
    
    // State tracking
    bool burstModeActive_ = false;
    uint8_t burstCount_ = 0;
    uint32_t lastBurstTime_ = 0;
    
    // Performance tracking
    uint32_t lastStatusUpdate_ = 0;
    uint32_t bandwidthUsed_ = 0;
    uint32_t requestCount_ = 0;
    
public:
    MobileAPI();
    ~MobileAPI();
    
    // Configuration
    void setConfig(const MobileAPIConfig& config);
    MobileAPIConfig getConfig() const;
    
    // Mobile-optimized endpoints
    void handleMobileStatus(AsyncWebServerRequest* request);
    void handleMobileCapture(AsyncWebServerRequest* request);
    void handleMobilePreview(AsyncWebServerRequest* request);
    void handleMobileSettings(AsyncWebServerRequest* request);
    void handleMobileNotifications(AsyncWebServerRequest* request);
    void handleMobileImageList(AsyncWebServerRequest* request);
    void handleMobileThumbnail(AsyncWebServerRequest* request);
    void handleMobileBurstMode(AsyncWebServerRequest* request);
    
    // Camera control
    bool capturePhoto(const MobileCaptureRequest& request);
    bool startBurstMode(uint8_t count, uint32_t interval);
    void stopBurstMode();
    bool setCameraSettings(const MobileCameraSettings& settings);
    MobileCameraSettings getCameraSettings() const;
    
    // Status and monitoring
    MobileSystemStatus getMobileStatus();
    String generateMobileStatusJSON();
    String generateMobileImageListJSON(int page = 0, int pageSize = 20);
    String generateMobileSettingsJSON();
    
    // Notification management
    void setNotificationSettings(const MobileNotificationSettings& settings);
    MobileNotificationSettings getNotificationSettings() const;
    void sendMobileNotification(const String& type, const String& message, const String& data = "");
    
    // Bandwidth optimization
    void enableBandwidthOptimization(bool enable);
    uint32_t getBandwidthUsage() const;
    void resetBandwidthCounter();
    
    // Image processing
    bool generateMobileThumbnail(const String& imagePath, String& thumbnailPath);
    bool generateMobilePreview(const String& imagePath, String& previewPath);
    bool optimizeImageForMobile(const String& imagePath, String& optimizedPath);
    
    // Utility functions
    bool isMobileRequest(AsyncWebServerRequest* request);
    String getMobileUserAgent(AsyncWebServerRequest* request);
    bool isLowBandwidthMode() const;
    void updateBandwidthUsage(uint32_t bytes);
    
    // Integration with existing systems
    void updateFromPowerManager();
    void updateFromStorageManager();
    void updateFromWildlifeDetector();
    
private:
    // Internal helpers
    void processBurstCapture();
    bool shouldCompressResponse(AsyncWebServerRequest* request);
    String compressJSON(const String& json);
    void logMobileRequest(AsyncWebServerRequest* request, const String& endpoint);
    bool validateCaptureRequest(const MobileCaptureRequest& request);
    bool validateCameraSettings(const MobileCameraSettings& settings);
    
    // Quality adaptation
    uint16_t getAdaptiveQuality() const;
    uint16_t getAdaptiveImageSize() const;
    bool shouldReduceQuality() const;
    
    // Background tasks
    void startBackgroundTasks();
    void stopBackgroundTasks();
    void processPendingNotifications();
};

// Global mobile API instance
extern MobileAPI* g_mobileAPI;

// Helper macros for mobile API
#define MOBILE_API_LOG(format, ...) ESP_LOGI("MobileAPI", format, ##__VA_ARGS__)
#define MOBILE_API_ERROR(format, ...) ESP_LOGE("MobileAPI", format, ##__VA_ARGS__)

// Mobile response helpers
#define MOBILE_SUCCESS(msg) "{\"success\":true,\"message\":\"" msg "\"}"
#define MOBILE_ERROR(msg) "{\"success\":false,\"error\":\"" msg "\"}"
#define MOBILE_RESPONSE(success, msg) ((success) ? MOBILE_SUCCESS(msg) : MOBILE_ERROR(msg))

#endif // MOBILE_API_H