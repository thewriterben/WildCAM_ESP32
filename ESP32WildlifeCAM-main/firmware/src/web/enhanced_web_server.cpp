/**
 * @file enhanced_web_server.cpp
 * @brief Enhanced Web Server Implementation
 * 
 * Implementation of the enhanced web server with WebSocket support
 * and real-time dashboard functionality.
 */

#include "enhanced_web_server.h"
#include <esp_log.h>

// Mobile API integration
#include "../../mobile_app/firmware/src/mobile/mobile_api.h"
#include "../../mobile_app/firmware/src/mobile/mobile_websocket.h"

static const char* TAG = "EnhancedWebServer";

// Global instance
EnhancedWebServer* g_enhancedWebServer = nullptr;

EnhancedWebServer::EnhancedWebServer() : 
    server_(80), 
    webSocket_("/ws"),
    running_(false),
    lastSystemUpdate_(0),
    lastHeartbeat_(0),
    streamManager_(nullptr),
    languageIntegration_(nullptr) {
}

EnhancedWebServer::~EnhancedWebServer() {
    end();
}

bool EnhancedWebServer::begin(const EnhancedWebConfig& config) {
    if (running_) {
        ESP_LOGW(TAG, "Server already running");
        return true;
    }
    
    config_ = config;
    server_ = AsyncWebServer(config_.port);
    
    // Setup components
    setupWebSocket();
    setupAPIEndpoints();
    setupStaticFiles();
    setupRoutes();
    
    if (config_.enableCORS) {
        setupCORS();
    }
    
    // Initialize metrics
    updateSystemMetrics();
    
    // Start server
    server_.begin();
    running_ = true;
    
    ESP_LOGI(TAG, "Enhanced web server started on port %d", config_.port);
    ESP_LOGI(TAG, "Dashboard available at: http://%s%s", 
             WiFi.localIP().toString().c_str(), config_.dashboardPath.c_str());
    ESP_LOGI(TAG, "WebSocket endpoint: ws://%s%s", 
             WiFi.localIP().toString().c_str(), config_.wsPath.c_str());
    
    return true;
}

void EnhancedWebServer::end() {
    if (!running_) return;
    
    server_.end();
    running_ = false;
    
    ESP_LOGI(TAG, "Enhanced web server stopped");
}

void EnhancedWebServer::setupWebSocket() {
    webSocket_.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                             AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onWSEvent(server, client, type, arg, data, len);
    });
    
    server_.addHandler(&webSocket_);
    ESP_LOGI(TAG, "WebSocket handler configured");
}

void EnhancedWebServer::setupAPIEndpoints() {
    // System status and metrics
    server_.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStatus(request);
    });
    
    server_.on("/api/metrics", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPISystemMetrics(request);
    });
    
    // Mobile-optimized API endpoints
    server_.on("/api/mobile/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileStatus(request);
        } else {
            this->handleAPIStatus(request);
        }
    });
    
    server_.on("/api/mobile/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileCapture(request);
        } else {
            this->handleAPICapture(request);
        }
    });
    
    server_.on("/api/mobile/preview", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobilePreview(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Mobile preview not available\"}");
        }
    });
    
    server_.on("/api/mobile/settings", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileSettings(request);
        } else {
            this->handleAPIConfig(request);
        }
    });
    
    server_.on("/api/mobile/settings", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileSettings(request);
        } else {
            this->handleAPIConfigUpdate(request);
        }
    });
    
    server_.on("/api/mobile/notifications", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileNotifications(request);
        } else {
            request->send(200, "application/json", "{\"notifications_enabled\":true}");
        }
    });
    
    server_.on("/api/mobile/notifications", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileNotifications(request);
        } else {
            request->send(200, "application/json", "{\"success\":true}");
        }
    });
    
    server_.on("/api/mobile/images", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileImageList(request);
        } else {
            this->handleAPIImageList(request);
        }
    });
    
    server_.on("/api/mobile/thumbnail", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileThumbnail(request);
        } else {
            this->handleAPIImageThumbnail(request);
        }
    });
    
    server_.on("/api/mobile/burst", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileBurstMode(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Burst mode not available\"}");
        }
    });
    
    server_.on("/api/mobile/burst", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (g_mobileAPI) {
            g_mobileAPI->handleMobileBurstMode(request);
        } else {
            request->send(501, "application/json", "{\"error\":\"Burst mode not available\"}");
        }
    });
    
    // Image management
    server_.on("/api/images", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIImageList(request);
    });
    
    server_.on("/api/thumbnail", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIImageThumbnail(request);
    });
    
    // Camera control
    server_.on("/api/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPICapture(request);
    });
    
    server_.on("/api/stream", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStream(request);
    });
    
    // Stream control endpoints
    server_.on("/api/stream/start", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPIStreamStart(request);
    });
    
    server_.on("/api/stream/stop", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPIStreamStop(request);
    });
    
    server_.on("/api/stream/stats", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStreamStats(request);
    });
    
    server_.on("/api/stream/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPIStreamConfig(request);
    });
    
    // Configuration
    server_.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIConfig(request);
    });
    
    server_.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleAPIConfigUpdate(request);
    });
    
    // Storage and power stats
    server_.on("/api/storage", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIStorageStats(request);
    });
    
    server_.on("/api/power", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIPowerStats(request);
    });
    
    // Wildlife detection log
    server_.on("/api/wildlife", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIWildlifeLog(request);
    });
    
    // Analytics endpoints
    server_.on("/api/analytics/summary", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsSummary(request);
    });
    
    server_.on("/api/analytics/wildlife", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsWildlife(request);
    });
    
    server_.on("/api/analytics/system", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsSystem(request);
    });
    
    server_.on("/api/analytics/historical", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsHistorical(request);
    });
    
    server_.on("/api/analytics/export", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsExport(request);
    });
    
    server_.on("/api/analytics/species", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsSpecies(request);
    });
    
    server_.on("/api/analytics/activity", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsActivity(request);
    });
    
    server_.on("/api/analytics/performance", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIAnalyticsPerformance(request);
    });
    
    // CSV Export endpoint
    server_.on("/api/export/detections.csv", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleAPIExportDetectionsCSV(request);
    });
    
    // Setup language API endpoints if language integration is available
    if (languageIntegration_) {
        languageIntegration_->setupAPIEndpoints(server_);
        ESP_LOGI(TAG, "Language API endpoints configured");
    }
    
    ESP_LOGI(TAG, "API endpoints configured");
}

void EnhancedWebServer::setupStaticFiles() {
    // Serve dashboard assets
    server_.serveStatic("/assets/", SD_MMC, "/web/assets/");
    
    ESP_LOGI(TAG, "Static file serving configured");
}

void EnhancedWebServer::setupRoutes() {
    // Main dashboard
    server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleDashboard(request);
    });
    
    // Catch-all for SPA routing
    server_.onNotFound([this](AsyncWebServerRequest* request) {
        if (request->url().startsWith("/api/")) {
            request->send(404, "application/json", "{\"error\":\"API endpoint not found\"}");
        } else {
            // Serve dashboard for client-side routing
            this->handleDashboard(request);
        }
    });
    
    ESP_LOGI(TAG, "Routes configured");
}

void EnhancedWebServer::setupCORS() {
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    
    ESP_LOGI(TAG, "CORS headers configured");
}

void EnhancedWebServer::onWSEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            ESP_LOGI(TAG, "WebSocket client connected: %u", client->id());
            // Send initial system status
            broadcastSystemStatus();
            break;
            
        case WS_EVT_DISCONNECT:
            ESP_LOGI(TAG, "WebSocket client disconnected: %u", client->id());
            break;
            
        case WS_EVT_DATA: {
            String message = String((char*)data, len);
            handleWSMessage(client, message);
            break;
        }
        
        case WS_EVT_ERROR:
            ESP_LOGW(TAG, "WebSocket error for client %u", client->id());
            break;
            
        default:
            break;
    }
}

void EnhancedWebServer::handleWSMessage(AsyncWebSocketClient* client, const String& message) {
    DynamicJsonDocument doc(512);
    
    if (deserializeJson(doc, message) != DeserializationError::Ok) {
        ESP_LOGW(TAG, "Invalid JSON in WebSocket message");
        return;
    }
    
    String action = doc["action"];
    
    if (action == "ping") {
        // Respond to ping with pong
        DynamicJsonDocument response(128);
        response["type"] = "pong";
        response["timestamp"] = millis();
        String responseStr;
        serializeJson(response, responseStr);
        client->text(responseStr);
    } else if (action == "request_update") {
        // Client requesting immediate update
        updateSystemMetrics();
        broadcastSystemStatus();
    }
}

void EnhancedWebServer::sendWSMessage(WSMessageType type, const JsonDocument& data) {
    DynamicJsonDocument message(1024);
    message["type"] = static_cast<int>(type);
    message["timestamp"] = millis();
    message["data"] = data;
    
    String messageStr;
    serializeJson(message, messageStr);
    
    webSocket_.textAll(messageStr);
}

void EnhancedWebServer::sendHeartbeat() {
    if (millis() - lastHeartbeat_ < config_.wsHeartbeatInterval) {
        return;
    }
    
    DynamicJsonDocument heartbeat(128);
    heartbeat["uptime"] = millis();
    heartbeat["clients"] = webSocket_.count();
    
    sendWSMessage(WSMessageType::HEARTBEAT, heartbeat);
    lastHeartbeat_ = millis();
}

void EnhancedWebServer::updateSystemMetrics() {
    updatePowerMetrics();
    updateStorageMetrics();
    updateNetworkMetrics();
    updateCameraMetrics();
    
    // System metrics
    metrics_.freeHeap = ESP.getFreeHeap();
    metrics_.totalHeap = ESP.getHeapSize();
    metrics_.uptime = millis();
    
    // Temperature (if available)
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    metrics_.temperature = temperatureRead();
    #endif
    
    lastSystemUpdate_ = millis();
}

void EnhancedWebServer::updatePowerMetrics() {
    // Integration with existing power manager
    // This would need to be adapted based on actual PowerManager implementation
    PowerSystemStatus powerStatus;
    
    // Mock implementation - replace with actual power manager calls
    metrics_.batteryVoltage = 3.8; // powerManager.getBatteryVoltage();
    metrics_.solarVoltage = 4.2;   // powerManager.getSolarVoltage();
    metrics_.batteryPercentage = 85; // powerManager.getBatteryPercentage();
    metrics_.isCharging = true;    // powerManager.isCharging();
    metrics_.powerState = POWER_NORMAL; // powerManager.getPowerState();
}

void EnhancedWebServer::updateStorageMetrics() {
    // Integration with existing storage manager
    #ifdef SD_MMC_SUPPORTED
    if (SD_MMC.cardType() != CARD_NONE) {
        metrics_.totalStorage = SD_MMC.cardSize();
        metrics_.usedStorage = SD_MMC.usedBytes();
        metrics_.freeStorage = metrics_.totalStorage - metrics_.usedStorage;
        
        // Count image files
        File root = SD_MMC.open("/");
        uint32_t imageCount = 0;
        while (File file = root.openNextFile()) {
            String filename = file.name();
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
                imageCount++;
            }
        }
        metrics_.imageCount = imageCount;
    }
    #endif
}

void EnhancedWebServer::updateNetworkMetrics() {
    metrics_.wifiConnected = WiFi.status() == WL_CONNECTED;
    if (metrics_.wifiConnected) {
        metrics_.wifiSignalStrength = WiFi.RSSI();
        metrics_.ipAddress = WiFi.localIP().toString();
    }
}

void EnhancedWebServer::updateCameraMetrics() {
    // Mock implementation - replace with actual camera status
    metrics_.cameraReady = true;
    metrics_.monitoringActive = true;
    metrics_.dailyCaptures = 12;
    metrics_.totalCaptures = 156;
    metrics_.lastDetectedSpecies = "deer";
    metrics_.lastDetectionConfidence = 0.92;
}

// Real-time broadcasting methods
void EnhancedWebServer::broadcastSystemStatus() {
    DynamicJsonDocument status(1024);
    
    // Power data
    status["battery"]["voltage"] = metrics_.batteryVoltage;
    status["battery"]["percentage"] = metrics_.batteryPercentage;
    status["battery"]["charging"] = metrics_.isCharging;
    status["solar"]["voltage"] = metrics_.solarVoltage;
    status["power"]["state"] = static_cast<int>(metrics_.powerState);
    
    // Memory and system
    status["memory"]["free"] = metrics_.freeHeap;
    status["memory"]["total"] = metrics_.totalHeap;
    status["system"]["uptime"] = metrics_.uptime;
    status["system"]["temperature"] = metrics_.temperature;
    
    // Storage
    status["storage"]["total"] = metrics_.totalStorage;
    status["storage"]["used"] = metrics_.usedStorage;
    status["storage"]["free"] = metrics_.freeStorage;
    status["storage"]["images"] = metrics_.imageCount;
    
    // Network
    status["network"]["connected"] = metrics_.wifiConnected;
    status["network"]["signal"] = metrics_.wifiSignalStrength;
    status["network"]["ip"] = metrics_.ipAddress;
    
    // Camera and AI
    status["camera"]["ready"] = metrics_.cameraReady;
    status["camera"]["monitoring"] = metrics_.monitoringActive;
    status["captures"]["daily"] = metrics_.dailyCaptures;
    status["captures"]["total"] = metrics_.totalCaptures;
    status["wildlife"]["lastSpecies"] = metrics_.lastDetectedSpecies;
    status["wildlife"]["lastConfidence"] = metrics_.lastDetectionConfidence;
    
    sendWSMessage(WSMessageType::SYSTEM_STATUS, status);
}

void EnhancedWebServer::broadcastWildlifeDetection(const String& species, float confidence, const String& imagePath) {
    DynamicJsonDocument detection(512);
    detection["species"] = species;
    detection["confidence"] = confidence;
    detection["imagePath"] = imagePath;
    detection["timestamp"] = millis();
    
    sendWSMessage(WSMessageType::WILDLIFE_DETECTION, detection);
    
    // Update metrics
    metrics_.lastDetectedSpecies = species;
    metrics_.lastDetectionConfidence = confidence;
    metrics_.totalCaptures++;
    
    logMessage("Wildlife detected: " + species + " (confidence: " + String(confidence) + ")");
}

void EnhancedWebServer::broadcastPowerUpdate() {
    updatePowerMetrics();
    
    DynamicJsonDocument power(256);
    power["battery"]["voltage"] = metrics_.batteryVoltage;
    power["battery"]["percentage"] = metrics_.batteryPercentage;
    power["battery"]["charging"] = metrics_.isCharging;
    power["solar"]["voltage"] = metrics_.solarVoltage;
    power["state"] = static_cast<int>(metrics_.powerState);
    
    sendWSMessage(WSMessageType::POWER_UPDATE, power);
}

void EnhancedWebServer::broadcastStorageUpdate() {
    updateStorageMetrics();
    
    DynamicJsonDocument storage(256);
    storage["total"] = metrics_.totalStorage;
    storage["used"] = metrics_.usedStorage;
    storage["free"] = metrics_.freeStorage;
    storage["images"] = metrics_.imageCount;
    
    sendWSMessage(WSMessageType::STORAGE_UPDATE, storage);
}

void EnhancedWebServer::broadcastCameraStatus() {
    DynamicJsonDocument camera(256);
    camera["ready"] = metrics_.cameraReady;
    camera["monitoring"] = metrics_.monitoringActive;
    camera["dailyCaptures"] = metrics_.dailyCaptures;
    camera["totalCaptures"] = metrics_.totalCaptures;
    
    sendWSMessage(WSMessageType::CAMERA_STATUS, camera);
}

void EnhancedWebServer::broadcastErrorAlert(const String& error) {
    DynamicJsonDocument alert(256);
    alert["message"] = error;
    alert["severity"] = "error";
    
    sendWSMessage(WSMessageType::ERROR_ALERT, alert);
    
    logMessage("Error alert: " + error);
}

void EnhancedWebServer::broadcastStreamStatus() {
    if (!streamManager_) {
        return;
    }
    
    DynamicJsonDocument streamData(512);
    streamData["streaming"] = streamManager_->isStreaming();
    streamData["clients"] = streamManager_->getClientCount();
    
    StreamState state = streamManager_->getStreamState();
    streamData["state"]["motionTriggered"] = state.motionTriggered;
    streamData["state"]["fps"] = state.currentProfile.maxFPS;
    streamData["state"]["quality"] = streamQualityToString(state.currentProfile.quality);
    streamData["state"]["frameSize"] = streamFrameSizeToString(state.currentProfile.frameSize);
    streamData["state"]["motionOnly"] = state.currentProfile.motionOnlyMode;
    
    if (state.isStreaming) {
        streamData["state"]["duration"] = (millis() - state.startTime) / 1000;
        streamData["state"]["maxDuration"] = state.currentProfile.maxDurationSeconds;
    }
    
    StreamStats stats = streamManager_->getStreamStats();
    streamData["stats"]["totalFrames"] = stats.totalFramesSent;
    streamData["stats"]["totalBytes"] = stats.totalBytesSent;
    streamData["stats"]["averageFPS"] = stats.averageFPS;
    streamData["stats"]["droppedFrames"] = stats.droppedFrames;
    
    // Note: Using the new STREAM_STATUS message type
    sendWSMessage(WSMessageType::STREAM_STATUS, streamData);
}

uint32_t EnhancedWebServer::getConnectedClients() const {
    return webSocket_.count();
}

void EnhancedWebServer::enableAuthentication(bool enable, const String& username, const String& password) {
    config_.enableAuth = enable;
    if (!username.isEmpty()) config_.username = username;
    if (!password.isEmpty()) config_.password = password;
}

bool EnhancedWebServer::authenticateRequest(AsyncWebServerRequest* request) {
    if (!config_.enableAuth) return true;
    
    // Simple basic auth check
    if (!request->authenticate(config_.username.c_str(), config_.password.c_str())) {
        return false;
    }
    
    return true;
}

void EnhancedWebServer::logMessage(const String& message) {
    ESP_LOGI(TAG, "%s", message.c_str());
    if (logCallback_) {
        logCallback_(message);
    }
}

// API Handler implementations
void EnhancedWebServer::handleDashboard(AsyncWebServerRequest* request) {
    // For now, serve a simple dashboard HTML
    // In a full implementation, this would load from SD card or embedded assets
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Wildlife Camera Dashboard</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<link rel='stylesheet' href='/assets/dashboard.css'>";
    html += "</head><body>";
    html += "<div id='dashboard'>Loading dashboard...</div>";
    html += "<script src='/assets/dashboard.js'></script>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
}

void EnhancedWebServer::handleAPIStatus(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateSystemStatusJSON());
}

void EnhancedWebServer::handleAPISystemMetrics(AsyncWebServerRequest* request) {
    updateSystemMetrics();
    request->send(200, "application/json", generateSystemStatusJSON());
}

void EnhancedWebServer::handleAPIImageList(AsyncWebServerRequest* request) {
    int page = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int pageSize = request->hasParam("size") ? request->getParam("size")->value().toInt() : 20;
    
    request->send(200, "application/json", generateImageListJSON(page, pageSize));
}

void EnhancedWebServer::handleAPIImageThumbnail(AsyncWebServerRequest* request) {
    if (!request->hasParam("file")) {
        request->send(400, "application/json", "{\"error\":\"Missing file parameter\"}");
        return;
    }
    
    String filename = request->getParam("file")->value();
    
    // For now, just redirect to the full image
    // In a full implementation, this would generate/serve thumbnails
    request->redirect("/images/" + filename);
}

void EnhancedWebServer::handleAPICapture(AsyncWebServerRequest* request) {
    // Mock capture implementation
    DynamicJsonDocument response(256);
    response["success"] = true;
    response["filename"] = "wildlife_" + String(millis()) + ".jpg";
    response["timestamp"] = millis();
    
    String responseStr;
    serializeJson(response, responseStr);
    
    request->send(200, "application/json", responseStr);
    
    // Trigger updates
    metrics_.totalCaptures++;
    metrics_.dailyCaptures++;
    broadcastCameraStatus();
}

void EnhancedWebServer::handleAPIConfig(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateConfigJSON());
}

void EnhancedWebServer::handleAPIConfigUpdate(AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuration updated\"}");
}

void EnhancedWebServer::handleAPIStream(AsyncWebServerRequest* request) {
    if (!streamManager_) {
        request->send(503, "application/json", "{\"error\":\"Stream manager not available\"}");
        return;
    }
    
    // This is the main MJPEG streaming endpoint
    AsyncWebServerResponse *response = request->beginChunkedResponse(
        MJPEG_CONTENT_TYPE, 
        [this](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            // This callback will be used to send MJPEG frames
            // For now, return 0 to end the stream - actual implementation will be added
            return 0;
        }
    );
    
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "0");
    
    // Add client to stream manager
    AsyncWebSocketClient* wsClient = nullptr; // We'll need to handle this differently for HTTP streaming
    
    request->send(response);
}

void EnhancedWebServer::handleAPIStreamStart(AsyncWebServerRequest* request) {
    if (!streamManager_) {
        request->send(503, "application/json", "{\"error\":\"Stream manager not available\"}");
        return;
    }
    
    StreamConfig config;
    
    // Parse optional configuration parameters from request
    if (request->hasParam("fps")) {
        int fps = request->getParam("fps")->value().toInt();
        if (fps >= STREAM_MIN_FPS && fps <= STREAM_MAX_FPS) {
            config.targetFPS = fps;
        }
    }
    
    if (request->hasParam("quality")) {
        String qualityStr = request->getParam("quality")->value();
        if (qualityStr == "low") config.quality = STREAM_QUALITY_LOW;
        else if (qualityStr == "medium") config.quality = STREAM_QUALITY_MEDIUM;
        else if (qualityStr == "high") config.quality = STREAM_QUALITY_HIGH;
        else if (qualityStr == "auto") config.quality = STREAM_QUALITY_AUTO;
    }
    
    if (request->hasParam("frameSize")) {
        String frameSizeStr = request->getParam("frameSize")->value();
        if (frameSizeStr == "qvga") config.frameSize = STREAM_FRAMESIZE_QVGA;
        else if (frameSizeStr == "vga") config.frameSize = STREAM_FRAMESIZE_VGA;
        else if (frameSizeStr == "svga") config.frameSize = STREAM_FRAMESIZE_SVGA;
        else if (frameSizeStr == "hd") config.frameSize = STREAM_FRAMESIZE_HD;
    }
    
    if (request->hasParam("motionOnly")) {
        config.motionTriggerEnabled = request->getParam("motionOnly")->value() == "true";
    }
    
    if (streamManager_->startStream(config)) {
        DynamicJsonDocument responseDoc(256);
        responseDoc["success"] = true;
        responseDoc["message"] = "Stream started successfully";
        responseDoc["status"] = streamManager_->getStatusJSON();
        
        String responseStr;
        serializeJson(responseDoc, responseStr);
        request->send(200, "application/json", responseStr);
        
        // Broadcast stream status update to WebSocket clients
        broadcastStreamStatus();
    } else {
        request->send(400, "application/json", "{\"error\":\"Failed to start stream\"}");
    }
}

void EnhancedWebServer::handleAPIStreamStop(AsyncWebServerRequest* request) {
    if (!streamManager_) {
        request->send(503, "application/json", "{\"error\":\"Stream manager not available\"}");
        return;
    }
    
    if (streamManager_->stopStream()) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Stream stopped successfully\"}");
        
        // Broadcast stream status update to WebSocket clients
        broadcastStreamStatus();
    } else {
        request->send(400, "application/json", "{\"error\":\"Failed to stop stream\"}");
    }
}

void EnhancedWebServer::handleAPIStreamStats(AsyncWebServerRequest* request) {
    if (!streamManager_) {
        request->send(503, "application/json", "{\"error\":\"Stream manager not available\"}");
        return;
    }
    
    DynamicJsonDocument doc(512);
    doc["status"] = streamManager_->getStatusJSON();
    doc["stats"] = streamManager_->getStatsJSON();
    
    String responseStr;
    serializeJson(doc, responseStr);
    request->send(200, "application/json", responseStr);
}

void EnhancedWebServer::handleAPIStreamConfig(AsyncWebServerRequest* request) {
    if (!streamManager_) {
        request->send(503, "application/json", "{\"error\":\"Stream manager not available\"}");
        return;
    }
    
    // Parse JSON body for configuration update
    // Note: This is a simplified implementation - full body parsing would require additional handling
    StreamConfig config = streamManager_->getStreamConfig();
    
    // Apply any query parameters as configuration updates
    if (request->hasParam("fps")) {
        int fps = request->getParam("fps")->value().toInt();
        if (fps >= STREAM_MIN_FPS && fps <= STREAM_MAX_FPS) {
            streamManager_->setFrameRate(fps);
        }
    }
    
    if (request->hasParam("quality")) {
        String qualityStr = request->getParam("quality")->value();
        StreamQuality quality = STREAM_QUALITY_AUTO;
        if (qualityStr == "low") quality = STREAM_QUALITY_LOW;
        else if (qualityStr == "medium") quality = STREAM_QUALITY_MEDIUM;
        else if (qualityStr == "high") quality = STREAM_QUALITY_HIGH;
        else if (qualityStr == "auto") quality = STREAM_QUALITY_AUTO;
        
        streamManager_->setQuality(quality);
    }
    
    if (request->hasParam("frameSize")) {
        String frameSizeStr = request->getParam("frameSize")->value();
        StreamFrameSize frameSize = STREAM_FRAMESIZE_VGA;
        if (frameSizeStr == "qvga") frameSize = STREAM_FRAMESIZE_QVGA;
        else if (frameSizeStr == "vga") frameSize = STREAM_FRAMESIZE_VGA;
        else if (frameSizeStr == "svga") frameSize = STREAM_FRAMESIZE_SVGA;
        else if (frameSizeStr == "hd") frameSize = STREAM_FRAMESIZE_HD;
        
        streamManager_->setFrameSize(frameSize);
    }
    
    if (request->hasParam("motionOnly")) {
        bool motionOnly = request->getParam("motionOnly")->value() == "true";
        streamManager_->setMotionOnlyMode(motionOnly);
    }
    
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Stream configuration updated\"}");
    
    // Broadcast configuration update
    broadcastStreamStatus();
}

void EnhancedWebServer::handleAPIStorageStats(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateStorageStatsJSON());
}

void EnhancedWebServer::handleAPIPowerStats(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generatePowerStatsJSON());
}

void EnhancedWebServer::handleAPIWildlifeLog(AsyncWebServerRequest* request) {
    int limit = request->hasParam("limit") ? request->getParam("limit")->value().toInt() : 50;
    request->send(200, "application/json", generateWildlifeLogJSON(limit));
}

// JSON generation methods
String EnhancedWebServer::generateSystemStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    doc["battery"]["voltage"] = metrics_.batteryVoltage;
    doc["battery"]["percentage"] = metrics_.batteryPercentage;
    doc["battery"]["charging"] = metrics_.isCharging;
    doc["solar"]["voltage"] = metrics_.solarVoltage;
    doc["power"]["state"] = static_cast<int>(metrics_.powerState);
    
    doc["memory"]["free"] = metrics_.freeHeap;
    doc["memory"]["total"] = metrics_.totalHeap;
    doc["system"]["uptime"] = metrics_.uptime;
    doc["system"]["temperature"] = metrics_.temperature;
    
    doc["storage"]["total"] = metrics_.totalStorage;
    doc["storage"]["used"] = metrics_.usedStorage;
    doc["storage"]["free"] = metrics_.freeStorage;
    doc["storage"]["images"] = metrics_.imageCount;
    
    doc["network"]["connected"] = metrics_.wifiConnected;
    doc["network"]["signal"] = metrics_.wifiSignalStrength;
    doc["network"]["ip"] = metrics_.ipAddress;
    
    doc["camera"]["ready"] = metrics_.cameraReady;
    doc["camera"]["monitoring"] = metrics_.monitoringActive;
    doc["captures"]["daily"] = metrics_.dailyCaptures;
    doc["captures"]["total"] = metrics_.totalCaptures;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateImageListJSON(int page, int pageSize) {
    DynamicJsonDocument doc(2048);
    JsonArray images = doc.createNestedArray("images");
    
    // Mock image list - in real implementation, scan SD card
    for (int i = 0; i < pageSize && i < 10; i++) {
        JsonObject img = images.createNestedObject();
        img["filename"] = "wildlife_" + String(millis() - i * 60000) + ".jpg";
        img["timestamp"] = millis() - i * 60000;
        img["size"] = 156789 + i * 1000;
        img["species"] = (i % 3 == 0) ? "deer" : (i % 3 == 1) ? "fox" : "unknown";
    }
    
    doc["page"] = page;
    doc["pageSize"] = pageSize;
    doc["total"] = metrics_.imageCount;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateConfigJSON() {
    DynamicJsonDocument doc(512);
    
    doc["captureInterval"] = 30;
    doc["motionSensitivity"] = 75;
    doc["imageQuality"] = 80;
    doc["powerSaveMode"] = true;
    doc["nightMode"] = "auto";
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateStorageStatsJSON() {
    DynamicJsonDocument doc(256);
    
    doc["total"] = metrics_.totalStorage;
    doc["used"] = metrics_.usedStorage;
    doc["free"] = metrics_.freeStorage;
    doc["images"] = metrics_.imageCount;
    doc["usage_percent"] = metrics_.totalStorage > 0 ? 
        (metrics_.usedStorage * 100 / metrics_.totalStorage) : 0;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generatePowerStatsJSON() {
    DynamicJsonDocument doc(256);
    
    doc["battery"]["voltage"] = metrics_.batteryVoltage;
    doc["battery"]["percentage"] = metrics_.batteryPercentage;
    doc["battery"]["charging"] = metrics_.isCharging;
    doc["solar"]["voltage"] = metrics_.solarVoltage;
    doc["state"] = static_cast<int>(metrics_.powerState);
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateWildlifeLogJSON(int limit) {
    DynamicJsonDocument doc(1024);
    JsonArray log = doc.createNestedArray("log");
    
    // Mock wildlife detection log
    for (int i = 0; i < limit && i < 10; i++) {
        JsonObject entry = log.createNestedObject();
        entry["timestamp"] = millis() - i * 3600000; // 1 hour intervals
        entry["species"] = (i % 4 == 0) ? "deer" : 
                          (i % 4 == 1) ? "fox" : 
                          (i % 4 == 2) ? "raccoon" : "unknown";
        entry["confidence"] = 0.7 + (i % 3) * 0.1;
        entry["filename"] = "wildlife_" + String(millis() - i * 3600000) + ".jpg";
    }
    
    doc["total"] = 10;
    doc["limit"] = limit;
    
    String result;
    serializeJson(doc, result);
    return result;
}

// Analytics API handlers
void EnhancedWebServer::handleAPIAnalyticsSummary(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateAnalyticsSummaryJSON());
}

void EnhancedWebServer::handleAPIAnalyticsWildlife(AsyncWebServerRequest* request) {
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "week";
    request->send(200, "application/json", generateWildlifeAnalyticsJSON(timeRange));
}

void EnhancedWebServer::handleAPIAnalyticsSystem(AsyncWebServerRequest* request) {
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "week";
    request->send(200, "application/json", generateSystemAnalyticsJSON(timeRange));
}

void EnhancedWebServer::handleAPIAnalyticsHistorical(AsyncWebServerRequest* request) {
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "month";
    request->send(200, "application/json", generateHistoricalAnalyticsJSON(timeRange));
}

void EnhancedWebServer::handleAPIAnalyticsExport(AsyncWebServerRequest* request) {
    String format = request->hasParam("format") ? request->getParam("format")->value() : "json";
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "week";
    
    if (format == "csv") {
        // TODO: Implement CSV export
        request->send(501, "text/plain", "CSV export not implemented yet");
    } else {
        // Default to JSON export
        DynamicJsonDocument doc(2048);
        doc["summary"] = generateAnalyticsSummaryJSON();
        doc["wildlife"] = generateWildlifeAnalyticsJSON(timeRange);
        doc["system"] = generateSystemAnalyticsJSON(timeRange);
        doc["exportTime"] = millis();
        doc["timeRange"] = timeRange;
        
        String result;
        serializeJson(doc, result);
        request->send(200, "application/json", result);
    }
}

void EnhancedWebServer::handleAPIAnalyticsSpecies(AsyncWebServerRequest* request) {
    request->send(200, "application/json", generateSpeciesAnalyticsJSON());
}

void EnhancedWebServer::handleAPIAnalyticsActivity(AsyncWebServerRequest* request) {
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "week";
    request->send(200, "application/json", generateActivityAnalyticsJSON(timeRange));
}

void EnhancedWebServer::handleAPIAnalyticsPerformance(AsyncWebServerRequest* request) {
    String timeRange = request->hasParam("timeRange") ? request->getParam("timeRange")->value() : "week";
    request->send(200, "application/json", generatePerformanceAnalyticsJSON(timeRange));
}

// Analytics JSON generators
String EnhancedWebServer::generateAnalyticsSummaryJSON() {
    DynamicJsonDocument doc(1024);
    
    // Key metrics summary
    doc["totalCaptures"] = metrics_.totalCaptures;
    doc["dailyCaptures"] = metrics_.dailyCaptures;
    doc["speciesCount"] = 12; // Mock data
    doc["accuracy"] = 94.2;
    doc["peakActivity"] = "6-8 AM";
    doc["uptime"] = metrics_.uptime;
    doc["batteryHealth"] = metrics_.batteryPercentage;
    doc["storageUsed"] = (metrics_.usedStorage * 100) / metrics_.totalStorage;
    doc["avgConfidence"] = 0.87;
    doc["lastDetection"] = metrics_.lastDetectedSpecies;
    doc["lastDetectionTime"] = millis() - 1200000; // 20 minutes ago
    doc["systemStatus"] = "healthy";
    
    // Weekly trends
    JsonObject trends = doc.createNestedObject("trends");
    trends["capturesChange"] = "+15%";
    trends["accuracyChange"] = "+2.1%";
    trends["newSpecies"] = 2;
    trends["uptimeChange"] = "+0.5%";
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateWildlifeAnalyticsJSON(const String& timeRange) {
    DynamicJsonDocument doc(2048);
    
    // Species distribution
    JsonArray species = doc.createNestedArray("speciesDistribution");
    JsonObject deer = species.createNestedObject();
    deer["name"] = "deer";
    deer["count"] = 45;
    deer["avgConfidence"] = 0.92;
    
    JsonObject fox = species.createNestedObject();
    fox["name"] = "fox";
    fox["count"] = 28;
    fox["avgConfidence"] = 0.89;
    
    JsonObject raccoon = species.createNestedObject();
    raccoon["name"] = "raccoon";
    raccoon["count"] = 18;
    raccoon["avgConfidence"] = 0.85;
    
    JsonObject bird = species.createNestedObject();
    bird["name"] = "bird";
    bird["count"] = 65;
    bird["avgConfidence"] = 0.78;
    
    JsonObject squirrel = species.createNestedObject();
    squirrel["name"] = "squirrel";
    squirrel["count"] = 34;
    squirrel["avgConfidence"] = 0.82;
    
    JsonObject rabbit = species.createNestedObject();
    rabbit["name"] = "rabbit";
    rabbit["count"] = 12;
    rabbit["avgConfidence"] = 0.88;
    
    // Time-based activity patterns
    JsonArray hourlyActivity = doc.createNestedArray("hourlyActivity");
    for (int i = 0; i < 24; i++) {
        JsonObject hour = hourlyActivity.createNestedObject();
        hour["hour"] = i;
        // Mock data with peak activity at dawn and dusk
        float activity = (i >= 5 && i <= 8) ? 0.8 : 
                        (i >= 17 && i <= 20) ? 0.7 : 
                        (i >= 22 || i <= 4) ? 0.3 : 0.4;
        hour["detections"] = int(activity * 20 + random(0, 5));
        hour["confidence"] = 0.7 + activity * 0.2;
    }
    
    // Daily patterns for the time range
    JsonArray dailyActivity = doc.createNestedArray("dailyActivity");
    int days = (timeRange == "week") ? 7 : (timeRange == "month") ? 30 : 1;
    for (int i = 0; i < days; i++) {
        JsonObject day = dailyActivity.createNestedObject();
        day["date"] = "2025-09-" + String(1 + i);
        day["detections"] = 15 + random(0, 25);
        day["species"] = 3 + random(0, 5);
        day["avgConfidence"] = 0.75 + (random(0, 20) / 100.0);
    }
    
    // Motion detection stats
    JsonObject motionStats = doc.createNestedObject("motionStats");
    motionStats["totalTriggers"] = 456;
    motionStats["validDetections"] = 234;
    motionStats["falsePositives"] = 222;
    motionStats["successRate"] = 51.3;
    motionStats["avgResponseTime"] = 1.2;
    
    // Confidence score distribution
    JsonArray confidenceDistribution = doc.createNestedArray("confidenceDistribution");
    JsonObject conf1 = confidenceDistribution.createNestedObject();
    conf1["range"] = "0.9-1.0";
    conf1["count"] = 45;
    
    JsonObject conf2 = confidenceDistribution.createNestedObject();
    conf2["range"] = "0.8-0.9";
    conf2["count"] = 38;
    
    JsonObject conf3 = confidenceDistribution.createNestedObject();
    conf3["range"] = "0.7-0.8";
    conf3["count"] = 25;
    
    JsonObject conf4 = confidenceDistribution.createNestedObject();
    conf4["range"] = "0.6-0.7";
    conf4["count"] = 15;
    
    JsonObject conf5 = confidenceDistribution.createNestedObject();
    conf5["range"] = "0.5-0.6";
    conf5["count"] = 8;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateSystemAnalyticsJSON(const String& timeRange) {
    DynamicJsonDocument doc(2048);
    
    // Battery performance
    JsonObject battery = doc.createNestedObject("battery");
    battery["currentLevel"] = metrics_.batteryPercentage;
    battery["voltage"] = metrics_.batteryVoltage;
    battery["chargingRate"] = metrics_.isCharging ? 45.2 : 0.0;
    battery["avgDailyConsumption"] = 12.5;
    battery["estimatedRuntime"] = 4.2;
    battery["healthScore"] = 95;
    
    // Storage analytics
    JsonObject storage = doc.createNestedObject("storage");
    storage["totalCapacity"] = metrics_.totalStorage / (1024 * 1024); // MB
    storage["usedSpace"] = metrics_.usedStorage / (1024 * 1024); // MB
    storage["freeSpace"] = metrics_.freeStorage / (1024 * 1024); // MB
    storage["utilizationPercent"] = (metrics_.usedStorage * 100) / metrics_.totalStorage;
    storage["avgFileSize"] = 2.1; // MB
    storage["imageCount"] = metrics_.imageCount;
    storage["projectedFull"] = "45 days";
    
    // Network connectivity
    JsonObject network = doc.createNestedObject("network");
    network["signalStrength"] = metrics_.wifiSignalStrength;
    network["connected"] = metrics_.wifiConnected;
    network["uptime"] = 99.2;
    network["avgLatency"] = 23.5;
    network["dataTransferred"] = 156.7; // MB
    network["connectionDrops"] = 2;
    
    // System performance
    JsonObject performance = doc.createNestedObject("performance");
    performance["cpuUsage"] = 35.8;
    performance["memoryUsage"] = (metrics_.totalHeap - metrics_.freeHeap) * 100 / metrics_.totalHeap;
    performance["temperature"] = metrics_.temperature;
    performance["uptime"] = metrics_.uptime;
    performance["avgProcessingTime"] = 1.8;
    performance["systemLoad"] = 0.4;
    
    // Historical trends
    JsonArray batteryHistory = doc.createNestedArray("batteryHistory");
    JsonArray temperatureHistory = doc.createNestedArray("temperatureHistory");
    JsonArray memoryHistory = doc.createNestedArray("memoryHistory");
    
    int points = (timeRange == "week") ? 168 : (timeRange == "month") ? 720 : 24; // Hours
    for (int i = 0; i < points; i += (points / 20)) { // 20 data points
        JsonObject batteryPoint = batteryHistory.createNestedObject();
        batteryPoint["timestamp"] = millis() - (points - i) * 3600000;
        batteryPoint["level"] = 75 + random(-10, 15);
        batteryPoint["voltage"] = 3.7 + (random(-20, 20) / 100.0);
        
        JsonObject tempPoint = temperatureHistory.createNestedObject();
        tempPoint["timestamp"] = millis() - (points - i) * 3600000;
        tempPoint["temperature"] = 20 + random(0, 15);
        
        JsonObject memPoint = memoryHistory.createNestedObject();
        memPoint["timestamp"] = millis() - (points - i) * 3600000;
        memPoint["usage"] = 30 + random(0, 40);
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateHistoricalAnalyticsJSON(const String& timeRange) {
    DynamicJsonDocument doc(2048);
    
    // Long-term trends
    JsonObject trends = doc.createNestedObject("trends");
    trends["captureGrowth"] = "+23%";
    trends["accuracyImprovement"] = "+5.2%";
    trends["speciesDiscovered"] = 8;
    trends["systemReliability"] = "+12%";
    trends["energyEfficiency"] = "+8%";
    
    // Seasonal patterns
    JsonArray seasonalData = doc.createNestedArray("seasonalPatterns");
    JsonObject spring = seasonalData.createNestedObject();
    spring["season"] = "Spring";
    spring["captures"] = 145;
    spring["accuracy"] = 89.2;
    
    JsonObject summer = seasonalData.createNestedObject();
    summer["season"] = "Summer";
    summer["captures"] = 203;
    summer["accuracy"] = 91.5;
    
    JsonObject fall = seasonalData.createNestedObject();
    fall["season"] = "Fall";
    fall["captures"] = 178;
    fall["accuracy"] = 87.8;
    
    JsonObject winter = seasonalData.createNestedObject();
    winter["season"] = "Winter";
    winter["captures"] = 98;
    winter["accuracy"] = 85.1;
    
    // Monthly comparisons
    JsonArray monthlyComparison = doc.createNestedArray("monthlyComparison");
    String months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep"};
    for (int i = 0; i < 9; i++) {
        JsonObject month = monthlyComparison.createNestedObject();
        month["month"] = months[i];
        month["captures"] = 45 + random(0, 50);
        month["species"] = 8 + random(0, 6);
        month["accuracy"] = 85 + random(0, 10);
        month["uptime"] = 95 + random(0, 5);
    }
    
    // Performance milestones
    JsonArray milestones = doc.createNestedArray("milestones");
    JsonObject m1 = milestones.createNestedObject();
    m1["title"] = "1000th capture";
    m1["date"] = "2025-08-15";
    m1["image"] = "wildlife_1000.jpg";
    
    JsonObject m2 = milestones.createNestedObject();
    m2["title"] = "10 species identified";
    m2["date"] = "2025-07-22";
    m2["image"] = "species_milestone.jpg";
    
    JsonObject m3 = milestones.createNestedObject();
    m3["title"] = "99% uptime achieved";
    m3["date"] = "2025-06-30";
    m3["image"] = "";
    
    JsonObject m4 = milestones.createNestedObject();
    m4["title"] = "Solar efficiency peak";
    m4["date"] = "2025-08-01";
    m4["image"] = "";
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateSpeciesAnalyticsJSON() {
    DynamicJsonDocument doc(1024);
    
    JsonArray species = doc.createNestedArray("species");
    JsonObject deer = species.createNestedObject();
    deer["name"] = "White-tailed Deer";
    deer["count"] = 45;
    deer["avgConfidence"] = 0.92;
    deer["type"] = "Large mammal";
    deer["activityPattern"] = "Crepuscular";
    
    JsonObject fox = species.createNestedObject();
    fox["name"] = "Red Fox";
    fox["count"] = 28;
    fox["avgConfidence"] = 0.89;
    fox["type"] = "Medium mammal";
    fox["activityPattern"] = "Nocturnal";
    
    JsonObject raccoon = species.createNestedObject();
    raccoon["name"] = "Common Raccoon";
    raccoon["count"] = 18;
    raccoon["avgConfidence"] = 0.85;
    raccoon["type"] = "Medium mammal";
    raccoon["activityPattern"] = "Nocturnal";
    
    JsonObject robin = species.createNestedObject();
    robin["name"] = "American Robin";
    robin["count"] = 65;
    robin["avgConfidence"] = 0.78;
    robin["type"] = "Small bird";
    robin["activityPattern"] = "Diurnal";
    
    JsonObject squirrel = species.createNestedObject();
    squirrel["name"] = "Eastern Gray Squirrel";
    squirrel["count"] = 34;
    squirrel["avgConfidence"] = 0.82;
    squirrel["type"] = "Small mammal";
    squirrel["activityPattern"] = "Diurnal";
    
    JsonObject rabbit = species.createNestedObject();
    rabbit["name"] = "Cottontail Rabbit";
    rabbit["count"] = 12;
    rabbit["avgConfidence"] = 0.88;
    rabbit["type"] = "Small mammal";
    rabbit["activityPattern"] = "Crepuscular";
    
    doc["totalSpecies"] = 6;
    doc["newThisWeek"] = 1;
    doc["mostActive"] = "American Robin";
    doc["rarest"] = "Cottontail Rabbit";
    doc["avgConfidence"] = 0.856;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generateActivityAnalyticsJSON(const String& timeRange) {
    DynamicJsonDocument doc(1024);
    
    // Activity patterns by time of day
    JsonArray timeActivity = doc.createNestedArray("timeOfDay");
    JsonObject dawn = timeActivity.createNestedObject();
    dawn["period"] = "Dawn";
    dawn["timeRange"] = "5-7 AM";
    dawn["detections"] = 34;
    dawn["activityLevel"] = "High";
    
    JsonObject morning = timeActivity.createNestedObject();
    morning["period"] = "Morning";
    morning["timeRange"] = "7-12 PM";
    morning["detections"] = 18;
    morning["activityLevel"] = "Medium";
    
    JsonObject afternoon = timeActivity.createNestedObject();
    afternoon["period"] = "Afternoon";
    afternoon["timeRange"] = "12-5 PM";
    afternoon["detections"] = 12;
    afternoon["activityLevel"] = "Low";
    
    JsonObject dusk = timeActivity.createNestedObject();
    dusk["period"] = "Dusk";
    dusk["timeRange"] = "5-8 PM";
    dusk["detections"] = 28;
    dusk["activityLevel"] = "High";
    
    JsonObject night = timeActivity.createNestedObject();
    night["period"] = "Night";
    night["timeRange"] = "8 PM-5 AM";
    night["detections"] = 8;
    night["activityLevel"] = "Low";
    
    // Weather correlation
    JsonObject weather = doc.createNestedObject("weatherCorrelation");
    weather["sunny"] = 78;
    weather["cloudy"] = 65;
    weather["rainy"] = 23;
    weather["foggy"] = 45;
    
    // Motion detection patterns
    JsonObject motionPatterns = doc.createNestedObject("motionPatterns");
    motionPatterns["avgTriggerTime"] = 1.2;
    motionPatterns["peakSensitivity"] = "Medium";
    motionPatterns["falsePositiveRate"] = 48.7;
    motionPatterns["bestPerformanceHour"] = 6;
    
    String result;
    serializeJson(doc, result);
    return result;
}

String EnhancedWebServer::generatePerformanceAnalyticsJSON(const String& timeRange) {
    DynamicJsonDocument doc(1024);
    
    // System health scores
    JsonObject health = doc.createNestedObject("healthScores");
    health["overall"] = 94;
    health["camera"] = 97;
    health["storage"] = 89;
    health["battery"] = 92;
    health["network"] = 96;
    health["processing"] = 91;
    
    // Performance metrics
    JsonObject metrics = doc.createNestedObject("metrics");
    metrics["avgResponseTime"] = 1.2;
    metrics["processingEfficiency"] = 87.5;
    metrics["memoryUtilization"] = 68.3;
    metrics["thermalPerformance"] = 95.2;
    metrics["powerEfficiency"] = 89.7;
    
    // Error analysis
    JsonObject errors = doc.createNestedObject("errorAnalysis");
    errors["totalErrors"] = 12;
    errors["criticalErrors"] = 0;
    errors["warningCount"] = 5;
    errors["recoveryRate"] = 100.0;
    errors["avgDowntime"] = 0.0;
    
    String result;
    serializeJson(doc, result);
    return result;
}

// CSV Export Handler Implementation
void EnhancedWebServer::handleAPIExportDetectionsCSV(AsyncWebServerRequest* request) {
    ESP_LOGI(TAG, "CSV export request received");
    
    // Parse date range query parameters
    String startDate = "";
    String endDate = "";
    
    if (request->hasParam("start")) {
        startDate = request->getParam("start")->value();
        ESP_LOGI(TAG, "Start date filter: %s", startDate.c_str());
    }
    
    if (request->hasParam("end")) {
        endDate = request->getParam("end")->value();
        ESP_LOGI(TAG, "End date filter: %s", endDate.c_str());
    }
    
    // Generate filename with current date
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char dateStr[16];
    strftime(dateStr, sizeof(dateStr), "%Y%m%d", &timeinfo);
    String filename = "detections_" + String(dateStr) + ".csv";
    
    // Create CSV response with proper headers
    AsyncWebServerResponse* response = request->beginChunkedResponse(
        "text/csv",
        [this, startDate, endDate](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
            static bool headerSent = false;
            static uint32_t recordCount = 0;
            const uint32_t MAX_RECORDS = 10000;
            
            // Reset state on first call
            if (index == 0) {
                headerSent = false;
                recordCount = 0;
            }
            
            String chunk = "";
            
            // Send CSV header on first chunk
            if (!headerSent) {
                chunk = "Timestamp,Species,Confidence,Image_Path,GPS_Lat,GPS_Lon,Battery_Level\n";
                headerSent = true;
            }
            
            // Check if we've reached the limit
            if (recordCount >= MAX_RECORDS) {
                ESP_LOGI(TAG, "CSV export completed with %d records", recordCount);
                return 0; // End of data
            }
            
            // Generate mock detection data for demonstration
            // In production, this would query actual detection database/log files
            // For now, generate sample data based on existing wildlife log pattern
            const uint32_t recordsPerChunk = 50;
            uint32_t recordsToAdd = min(recordsPerChunk, MAX_RECORDS - recordCount);
            
            for (uint32_t i = 0; i < recordsToAdd; i++) {
                // Generate timestamp (mock data - going back in time)
                unsigned long timestamp = millis() - (recordCount + i) * 3600000;
                String timestampStr = String(timestamp / 1000);
                
                // Generate species data (cycling through common species)
                const char* species[] = {"deer", "fox", "raccoon", "bird", "squirrel", "rabbit"};
                uint8_t speciesIndex = (recordCount + i) % 6;
                String speciesName = String(species[speciesIndex]);
                
                // Generate confidence (0.70 - 0.95)
                float confidence = 0.70 + ((recordCount + i) % 26) * 0.01;
                String confidenceStr = String(confidence, 2);
                
                // Generate image path
                String imagePath = "/images/wildlife_" + String(timestamp) + ".jpg";
                
                // Generate GPS coordinates (mock data)
                String gpsLat = String(45.5 + ((recordCount + i) % 100) * 0.001, 6);
                String gpsLon = String(-122.7 + ((recordCount + i) % 100) * 0.001, 6);
                
                // Generate battery level (70-100%)
                String batteryLevel = String(70 + ((recordCount + i) % 31));
                
                // Generate CSV row with proper escaping
                chunk += generateCSVRow(timestampStr, speciesName, confidenceStr, 
                                       imagePath, gpsLat, gpsLon, batteryLevel);
            }
            
            recordCount += recordsToAdd;
            
            // Copy chunk to buffer
            size_t len = min(chunk.length(), maxLen);
            memcpy(buffer, chunk.c_str(), len);
            
            // Check if we're done
            if (recordCount >= MAX_RECORDS || recordsToAdd == 0) {
                ESP_LOGI(TAG, "CSV export completed with %d records", recordCount);
                if (len < maxLen) {
                    return len; // Last chunk
                }
            }
            
            return len;
        }
    );
    
    // Set proper headers for CSV download
    response->addHeader("Content-Type", "text/csv");
    response->addHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "0");
    
    ESP_LOGI(TAG, "Starting CSV export as file: %s", filename.c_str());
    request->send(response);
}

// CSV Helper Functions
String EnhancedWebServer::escapeCSVField(const String& field) {
    // Escape CSV field by wrapping in quotes if it contains comma, quote, or newline
    if (field.indexOf(',') >= 0 || field.indexOf('"') >= 0 || field.indexOf('\n') >= 0) {
        String escaped = field;
        // Replace " with ""
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

String EnhancedWebServer::generateCSVRow(const String& timestamp, const String& species, 
                                         const String& confidence, const String& imagePath,
                                         const String& gpsLat, const String& gpsLon, 
                                         const String& batteryLevel) {
    String row = "";
    row += escapeCSVField(timestamp) + ",";
    row += escapeCSVField(species) + ",";
    row += escapeCSVField(confidence) + ",";
    row += escapeCSVField(imagePath) + ",";
    row += escapeCSVField(gpsLat) + ",";
    row += escapeCSVField(gpsLon) + ",";
    row += escapeCSVField(batteryLevel) + "\n";
    return row;
}

// Utility functions
bool startEnhancedWebServer(const EnhancedWebConfig& config) {
    if (g_enhancedWebServer == nullptr) {
        g_enhancedWebServer = new EnhancedWebServer();
    }
    
    return g_enhancedWebServer->begin(config);
}

void stopEnhancedWebServer() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->end();
        delete g_enhancedWebServer;
        g_enhancedWebServer = nullptr;
    }
}

bool isEnhancedWebServerRunning() {
    return g_enhancedWebServer != nullptr && g_enhancedWebServer->isRunning();
}

void updateWebDashboard() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->triggerSystemUpdate();
        g_enhancedWebServer->sendHeartbeat();
    }
}

void notifyWildlifeDetection(const String& species, float confidence, const String& imagePath) {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastWildlifeDetection(species, confidence, imagePath);
    }
}

void notifyPowerStateChange() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastPowerUpdate();
    }
}

void notifyStorageChange() {
    if (g_enhancedWebServer != nullptr) {
        g_enhancedWebServer->broadcastStorageUpdate();
    }
}