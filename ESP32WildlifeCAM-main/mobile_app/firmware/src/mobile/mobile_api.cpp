/**
 * @file mobile_api.cpp
 * @brief Mobile API implementation for ESP32 Wildlife Camera
 * 
 * Implementation of mobile-optimized API endpoints with bandwidth
 * optimization and touch-friendly controls.
 */

#include "mobile_api.h"
#include <esp_log.h>
#include <WiFi.h>

static const char* TAG = "MobileAPI";

// Global instance
MobileAPI* g_mobileAPI = nullptr;

MobileAPI::MobileAPI() {
    // Initialize with default configuration
    startBackgroundTasks();
    MOBILE_API_LOG("Mobile API initialized");
}

MobileAPI::~MobileAPI() {
    stopBackgroundTasks();
}

void MobileAPI::setConfig(const MobileAPIConfig& config) {
    config_ = config;
    MOBILE_API_LOG("Mobile API configuration updated");
}

MobileAPIConfig MobileAPI::getConfig() const {
    return config_;
}

// Mobile-optimized status endpoint
void MobileAPI::handleMobileStatus(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/status");
    
    MobileSystemStatus status = getMobileStatus();
    String response = generateMobileStatusJSON();
    
    updateBandwidthUsage(response.length());
    
    // Add mobile-specific headers
    AsyncWebServerResponse* resp = request->beginResponse(200, "application/json", response);
    resp->addHeader("Cache-Control", "no-cache, max-age=3");
    resp->addHeader("X-Mobile-Optimized", "true");
    
    if (shouldCompressResponse(request)) {
        resp->addHeader("Content-Encoding", "gzip");
    }
    
    request->send(resp);
}

// Mobile capture endpoint with burst mode support
void MobileAPI::handleMobileCapture(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/capture");
    
    MobileCaptureRequest captureReq;
    
    // Parse capture parameters
    if (request->hasParam("mode")) {
        String mode = request->getParam("mode")->value();
        if (mode == "burst") {
            captureReq.mode = MobileCaptureRequest::BURST;
        } else if (mode == "timer") {
            captureReq.mode = MobileCaptureRequest::TIMER;
        } else if (mode == "preview") {
            captureReq.mode = MobileCaptureRequest::PREVIEW;
        } else {
            captureReq.mode = MobileCaptureRequest::SINGLE;
        }
    }
    
    if (request->hasParam("burst_count")) {
        captureReq.burstCount = request->getParam("burst_count")->value().toInt();
    }
    
    if (request->hasParam("timer_delay")) {
        captureReq.timerDelay = request->getParam("timer_delay")->value().toInt() * 1000;
    }
    
    if (request->hasParam("quality")) {
        captureReq.quality = request->getParam("quality")->value().toInt();
    }
    
    // Apply mobile optimizations
    if (isLowBandwidthMode()) {
        captureReq.quality = min(captureReq.quality, (uint16_t)config_.mobileQuality);
    }
    
    // Validate request
    if (!validateCaptureRequest(captureReq)) {
        request->send(400, "application/json", MOBILE_ERROR("Invalid capture parameters"));
        return;
    }
    
    // Execute capture
    bool success = capturePhoto(captureReq);
    String response = MOBILE_RESPONSE(success, success ? "Capture initiated" : "Capture failed");
    
    updateBandwidthUsage(response.length());
    request->send(success ? 200 : 500, "application/json", response);
}

// Mobile preview endpoint for live camera view
void MobileAPI::handleMobilePreview(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/preview");
    
    // Get preview quality parameter
    uint16_t quality = config_.previewSize;
    if (request->hasParam("quality")) {
        String qualityStr = request->getParam("quality")->value();
        if (qualityStr == "low") quality = 320;
        else if (qualityStr == "medium") quality = 640;
        else if (qualityStr == "high") quality = 1024;
    }
    
    // Apply bandwidth optimization
    if (isLowBandwidthMode()) {
        quality = min(quality, (uint16_t)480);
    }
    
    // For now, return a placeholder response
    // In a full implementation, this would capture and return a preview image
    DynamicJsonDocument doc(256);
    doc["success"] = true;
    doc["preview_url"] = "/api/mobile/preview.jpg";
    doc["quality"] = quality;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    
    updateBandwidthUsage(response.length());
    request->send(200, "application/json", response);
}

// Mobile settings endpoint
void MobileAPI::handleMobileSettings(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/settings");
    
    if (request->method() == HTTP_GET) {
        // Return current settings
        String response = generateMobileSettingsJSON();
        updateBandwidthUsage(response.length());
        request->send(200, "application/json", response);
    } else if (request->method() == HTTP_POST) {
        // Update settings (placeholder implementation)
        // In a full implementation, this would parse POST data and update settings
        request->send(200, "application/json", MOBILE_SUCCESS("Settings updated"));
    }
}

// Mobile notifications endpoint
void MobileAPI::handleMobileNotifications(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/notifications");
    
    if (request->method() == HTTP_GET) {
        // Return notification settings
        DynamicJsonDocument doc(512);
        doc["wildlife_alerts"] = notificationSettings_.enableWildlifeAlerts;
        doc["battery_alerts"] = notificationSettings_.enableBatteryAlerts;
        doc["storage_alerts"] = notificationSettings_.enableStorageAlerts;
        doc["system_alerts"] = notificationSettings_.enableSystemAlerts;
        doc["battery_threshold"] = notificationSettings_.batteryThreshold;
        doc["storage_threshold"] = notificationSettings_.storageThreshold;
        doc["quiet_hours_start"] = notificationSettings_.quietHoursStart;
        doc["quiet_hours_end"] = notificationSettings_.quietHoursEnd;
        doc["respect_quiet_hours"] = notificationSettings_.respectQuietHours;
        
        String response;
        serializeJson(doc, response);
        updateBandwidthUsage(response.length());
        request->send(200, "application/json", response);
    } else if (request->method() == HTTP_POST) {
        // Update notification settings (placeholder)
        request->send(200, "application/json", MOBILE_SUCCESS("Notification settings updated"));
    }
}

// Mobile image list with pagination
void MobileAPI::handleMobileImageList(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/images");
    
    int page = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int pageSize = request->hasParam("size") ? request->getParam("size")->value().toInt() : 20;
    
    // Limit page size for mobile
    pageSize = min(pageSize, 50);
    
    String response = generateMobileImageListJSON(page, pageSize);
    updateBandwidthUsage(response.length());
    
    AsyncWebServerResponse* resp = request->beginResponse(200, "application/json", response);
    resp->addHeader("X-Mobile-Pagination", "true");
    request->send(resp);
}

// Mobile thumbnail endpoint
void MobileAPI::handleMobileThumbnail(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/thumbnail");
    
    if (!request->hasParam("image")) {
        request->send(400, "application/json", MOBILE_ERROR("Missing image parameter"));
        return;
    }
    
    String imagePath = request->getParam("image")->value();
    String thumbnailPath;
    
    if (generateMobileThumbnail(imagePath, thumbnailPath)) {
        // In a full implementation, this would serve the thumbnail file
        request->send(200, "application/json", MOBILE_SUCCESS("Thumbnail generated"));
    } else {
        request->send(500, "application/json", MOBILE_ERROR("Failed to generate thumbnail"));
    }
}

// Burst mode control
void MobileAPI::handleMobileBurstMode(AsyncWebServerRequest* request) {
    logMobileRequest(request, "/api/mobile/burst");
    
    if (request->hasParam("action")) {
        String action = request->getParam("action")->value();
        
        if (action == "start") {
            uint8_t count = request->hasParam("count") ? 
                request->getParam("count")->value().toInt() : 5;
            uint32_t interval = request->hasParam("interval") ? 
                request->getParam("interval")->value().toInt() : 1000;
            
            bool success = startBurstMode(count, interval);
            String response = MOBILE_RESPONSE(success, success ? "Burst mode started" : "Failed to start burst mode");
            request->send(success ? 200 : 500, "application/json", response);
        } else if (action == "stop") {
            stopBurstMode();
            request->send(200, "application/json", MOBILE_SUCCESS("Burst mode stopped"));
        } else {
            request->send(400, "application/json", MOBILE_ERROR("Invalid burst action"));
        }
    } else {
        // Return burst mode status
        DynamicJsonDocument doc(256);
        doc["active"] = burstModeActive_;
        doc["count"] = burstCount_;
        doc["last_capture"] = lastBurstTime_;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    }
}

// Capture photo implementation
bool MobileAPI::capturePhoto(const MobileCaptureRequest& request) {
    MOBILE_API_LOG("Capturing photo - mode: %d, quality: %d", request.mode, request.quality);
    
    // Apply adaptive quality if enabled
    uint16_t actualQuality = cameraSettings_.adaptiveQuality ? 
        getAdaptiveQuality() : request.quality;
    
    switch (request.mode) {
        case MobileCaptureRequest::SINGLE:
            // Single photo capture
            return true; // Placeholder - would implement actual capture
            
        case MobileCaptureRequest::BURST:
            return startBurstMode(request.burstCount, config_.burstInterval);
            
        case MobileCaptureRequest::TIMER:
            // Timer capture - would implement delay then capture
            return true;
            
        case MobileCaptureRequest::PREVIEW:
            // Preview capture - lower quality, faster capture
            return true;
            
        default:
            return false;
    }
}

// Burst mode implementation
bool MobileAPI::startBurstMode(uint8_t count, uint32_t interval) {
    if (burstModeActive_) {
        MOBILE_API_LOG("Burst mode already active");
        return false;
    }
    
    if (count > config_.maxBurstCount) {
        count = config_.maxBurstCount;
    }
    
    burstModeActive_ = true;
    burstCount_ = count;
    lastBurstTime_ = millis();
    
    MOBILE_API_LOG("Started burst mode: %d shots, %dms interval", count, interval);
    return true;
}

void MobileAPI::stopBurstMode() {
    if (burstModeActive_) {
        burstModeActive_ = false;
        burstCount_ = 0;
        MOBILE_API_LOG("Stopped burst mode");
    }
}

// Mobile status generation
MobileSystemStatus MobileAPI::getMobileStatus() {
    MobileSystemStatus status;
    
    // Update from integrated systems
    updateFromPowerManager();
    updateFromStorageManager();
    updateFromWildlifeDetector();
    
    // Basic system status
    status.cameraReady = true; // Would check actual camera status
    status.wifiConnected = WiFi.isConnected();
    status.wifiSignal = WiFi.RSSI();
    status.monitoringActive = true; // Would check monitoring state
    
    // Alert conditions
    status.lowBattery = status.batteryPercentage < notificationSettings_.batteryThreshold;
    status.lowStorage = status.freeStorage < (notificationSettings_.storageThreshold * 1024 * 1024);
    status.errorState = false; // Would check for system errors
    
    return status;
}

String MobileAPI::generateMobileStatusJSON() {
    MobileSystemStatus status = getMobileStatus();
    
    DynamicJsonDocument doc(768);
    doc["battery"] = status.batteryPercentage;
    doc["charging"] = status.isCharging;
    doc["camera_ready"] = status.cameraReady;
    doc["wifi_connected"] = status.wifiConnected;
    doc["wifi_signal"] = status.wifiSignal;
    doc["free_storage"] = status.freeStorage;
    doc["image_count"] = status.imageCount;
    doc["monitoring"] = status.monitoringActive;
    
    // Last detection
    if (status.lastSpecies.length() > 0) {
        doc["last_species"] = status.lastSpecies;
        doc["last_confidence"] = status.lastConfidence;
        doc["last_detection"] = status.lastDetectionTime;
    }
    
    // Alerts
    JsonObject alerts = doc.createNestedObject("alerts");
    alerts["low_battery"] = status.lowBattery;
    alerts["low_storage"] = status.lowStorage;
    alerts["error"] = status.errorState;
    
    // System info
    doc["timestamp"] = millis();
    doc["uptime"] = millis() / 1000;
    doc["free_heap"] = ESP.getFreeHeap();
    
    String response;
    serializeJson(doc, response);
    return response;
}

String MobileAPI::generateMobileImageListJSON(int page, int pageSize) {
    DynamicJsonDocument doc(2048);
    
    // Pagination info
    doc["page"] = page;
    doc["page_size"] = pageSize;
    doc["total_images"] = 0; // Would get actual count
    doc["total_pages"] = 0;
    
    // Images array (placeholder)
    JsonArray images = doc.createNestedArray("images");
    
    // Would populate with actual image data
    for (int i = 0; i < min(pageSize, 5); i++) {
        JsonObject img = images.createNestedObject();
        img["id"] = page * pageSize + i;
        img["filename"] = "IMG_" + String(img["id"].as<int>()) + ".jpg";
        img["thumbnail"] = "/api/mobile/thumbnail?image=" + img["filename"].as<String>();
        img["size"] = 1024 * (100 + i * 50);
        img["timestamp"] = millis() - (i * 3600000); // Mock timestamps
        img["species"] = i % 2 == 0 ? "deer" : "unknown";
        img["confidence"] = 0.7 + (i * 0.05);
    }
    
    String response;
    serializeJson(doc, response);
    return response;
}

String MobileAPI::generateMobileSettingsJSON() {
    DynamicJsonDocument doc(1024);
    
    // Camera settings
    JsonObject camera = doc.createNestedObject("camera");
    camera["brightness"] = cameraSettings_.brightness;
    camera["contrast"] = cameraSettings_.contrast;
    camera["saturation"] = cameraSettings_.saturation;
    camera["auto_exposure"] = cameraSettings_.autoExposure;
    camera["flash_enabled"] = cameraSettings_.enableFlash;
    camera["low_power_mode"] = cameraSettings_.lowPowerMode;
    camera["adaptive_quality"] = cameraSettings_.adaptiveQuality;
    camera["mobile_quality"] = cameraSettings_.mobileQuality;
    
    // API configuration
    JsonObject api = doc.createNestedObject("api");
    api["bandwidth_optimization"] = config_.enableBandwidthOptimization;
    api["max_image_size"] = config_.maxImageSize;
    api["thumbnail_size"] = config_.thumbnailSize;
    api["preview_size"] = config_.previewSize;
    api["update_interval"] = config_.updateInterval;
    api["burst_mode"] = config_.enableBurstMode;
    api["max_burst_count"] = config_.maxBurstCount;
    
    String response;
    serializeJson(doc, response);
    return response;
}

// Bandwidth optimization
bool MobileAPI::isLowBandwidthMode() const {
    return config_.enableBandwidthOptimization && 
           (WiFi.RSSI() < -70 || bandwidthUsed_ > 10 * 1024 * 1024); // 10MB threshold
}

void MobileAPI::updateBandwidthUsage(uint32_t bytes) {
    bandwidthUsed_ += bytes;
    requestCount_++;
}

uint32_t MobileAPI::getBandwidthUsage() const {
    return bandwidthUsed_;
}

void MobileAPI::resetBandwidthCounter() {
    bandwidthUsed_ = 0;
    requestCount_ = 0;
}

// Quality adaptation
uint16_t MobileAPI::getAdaptiveQuality() const {
    if (isLowBandwidthMode()) {
        return 60; // Lower quality for poor connections
    } else if (WiFi.RSSI() > -50) {
        return 85; // High quality for good connections
    } else {
        return 75; // Medium quality
    }
}

// Utility functions
bool MobileAPI::isMobileRequest(AsyncWebServerRequest* request) {
    String userAgent = getMobileUserAgent(request);
    return userAgent.indexOf("Mobile") >= 0 || 
           userAgent.indexOf("Android") >= 0 ||
           userAgent.indexOf("iPhone") >= 0 ||
           userAgent.indexOf("iPad") >= 0;
}

String MobileAPI::getMobileUserAgent(AsyncWebServerRequest* request) {
    if (request->hasHeader("User-Agent")) {
        return request->getHeader("User-Agent")->value();
    }
    return "";
}

bool MobileAPI::shouldCompressResponse(AsyncWebServerRequest* request) {
    return request->hasHeader("Accept-Encoding") &&
           request->getHeader("Accept-Encoding")->value().indexOf("gzip") >= 0;
}

void MobileAPI::logMobileRequest(AsyncWebServerRequest* request, const String& endpoint) {
    String userAgent = getMobileUserAgent(request);
    String clientIP = request->client()->remoteIP().toString();
    
    MOBILE_API_LOG("Mobile request: %s from %s [%s]", 
                   endpoint.c_str(), clientIP.c_str(), 
                   userAgent.substring(0, 50).c_str());
}

bool MobileAPI::validateCaptureRequest(const MobileCaptureRequest& request) {
    if (request.burstCount > config_.maxBurstCount) return false;
    if (request.quality > 100) return false;
    if (request.timerDelay > 300000) return false; // Max 5 minutes
    return true;
}

// Placeholder integration methods
void MobileAPI::updateFromPowerManager() {
    // Would integrate with actual PowerManager
    // Example: status.batteryPercentage = powerManager->getBatteryPercentage();
}

void MobileAPI::updateFromStorageManager() {
    // Would integrate with actual StorageManager
    // Example: status.freeStorage = storageManager->getFreeSpace();
}

void MobileAPI::updateFromWildlifeDetector() {
    // Would integrate with actual WildlifeDetector
    // Example: status.lastSpecies = detector->getLastDetectedSpecies();
}

// Background task management
void MobileAPI::startBackgroundTasks() {
    // Would start background tasks for burst mode, notifications, etc.
    MOBILE_API_LOG("Background tasks started");
}

void MobileAPI::stopBackgroundTasks() {
    // Would stop background tasks
    MOBILE_API_LOG("Background tasks stopped");
}

// Thumbnail generation placeholder
bool MobileAPI::generateMobileThumbnail(const String& imagePath, String& thumbnailPath) {
    // Would implement actual thumbnail generation
    thumbnailPath = imagePath + "_thumb.jpg";
    return true;
}

bool MobileAPI::generateMobilePreview(const String& imagePath, String& previewPath) {
    // Would implement actual preview generation
    previewPath = imagePath + "_preview.jpg";
    return true;
}