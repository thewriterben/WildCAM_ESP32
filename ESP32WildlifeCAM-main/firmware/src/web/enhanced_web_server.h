/**
 * @file enhanced_web_server.h
 * @brief Enhanced Web Server with Real-Time Dashboard Support
 * 
 * Extends the existing AsyncWebServer implementation to provide:
 * - WebSocket support for real-time updates
 * - Modern responsive dashboard interface
 * - Enhanced API endpoints for wildlife monitoring
 * - Integration with power, AI, and storage systems
 * 
 * Builds upon existing web server patterns in the codebase.
 */

#ifndef ENHANCED_WEB_SERVER_H
#define ENHANCED_WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <SD_MMC.h>

// Integration with existing systems
#include "../power_manager.h"
#include "../../src/data/storage_manager.h"
#include "../../src/ai/wildlife_classifier.h"
#include "../../src/streaming/stream_manager.h"
#include "../i18n/language_manager.h"
#include "web_language_integration.h"

/**
 * WebSocket message types for real-time updates
 */
enum class WSMessageType {
    SYSTEM_STATUS = 0,
    WILDLIFE_DETECTION = 1,
    POWER_UPDATE = 2,
    STORAGE_UPDATE = 3,
    CAMERA_STATUS = 4,
    ERROR_ALERT = 5,
    HEARTBEAT = 6,
    STREAM_STATUS = 7
};

/**
 * Enhanced Web Server Configuration
 */
struct EnhancedWebConfig {
    uint16_t port = 80;
    String dashboardPath = "/";
    String apiBasePath = "/api";
    String wsPath = "/ws";
    String streamPath = "/stream";
    bool enableAuth = false;
    String username = "admin";
    String password = "wildlife";
    uint32_t wsHeartbeatInterval = 30000; // 30 seconds
    uint32_t systemUpdateInterval = 5000;  // 5 seconds
    bool enableCORS = true;
    
    EnhancedWebConfig() {}
};

/**
 * System metrics for real-time dashboard
 */
struct SystemMetrics {
    // Power system
    float batteryVoltage = 0.0;
    float solarVoltage = 0.0;
    int batteryPercentage = 0;
    bool isCharging = false;
    PowerState powerState = POWER_NORMAL;
    
    // Memory and system
    uint32_t freeHeap = 0;
    uint32_t totalHeap = 0;
    uint32_t uptime = 0;
    float temperature = 0.0;
    
    // Storage
    uint64_t totalStorage = 0;
    uint64_t usedStorage = 0;
    uint64_t freeStorage = 0;
    uint32_t imageCount = 0;
    
    // Network
    bool wifiConnected = false;
    int wifiSignalStrength = 0;
    String ipAddress = "";
    
    // Camera and AI
    bool cameraReady = false;
    bool monitoringActive = false;
    uint32_t dailyCaptures = 0;
    uint32_t totalCaptures = 0;
    String lastDetectedSpecies = "";
    float lastDetectionConfidence = 0.0;
    
    SystemMetrics() {}
};

/**
 * Enhanced Web Server Class
 * 
 * Provides a modern web dashboard with real-time updates for wildlife monitoring.
 * Integrates seamlessly with existing AsyncWebServer implementations.
 */
class EnhancedWebServer {
public:
    EnhancedWebServer();
    ~EnhancedWebServer();
    
    // Initialization and lifecycle
    bool begin(const EnhancedWebConfig& config = EnhancedWebConfig());
    void end();
    bool isRunning() const { return running_; }
    
    // Server and WebSocket access
    AsyncWebServer* getServer() { return &server_; }
    AsyncWebSocket* getWebSocket() { return &webSocket_; }
    
    // Stream manager integration
    void setStreamManager(StreamManager* streamManager) { streamManager_ = streamManager; }
    StreamManager* getStreamManager() { return streamManager_; }
    
    // Language integration
    void setLanguageIntegration(WebLanguageIntegration* languageIntegration) { languageIntegration_ = languageIntegration; }
    WebLanguageIntegration* getLanguageIntegration() { return languageIntegration_; }
    
    // Real-time updates
    void broadcastSystemStatus();
    void broadcastWildlifeDetection(const String& species, float confidence, const String& imagePath);
    void broadcastPowerUpdate();
    void broadcastStorageUpdate();
    void broadcastCameraStatus();
    void broadcastErrorAlert(const String& error);
    void broadcastStreamStatus(); // Added for streaming updates
    
    // Metrics and status
    void updateSystemMetrics();
    SystemMetrics getSystemMetrics() const { return metrics_; }
    uint32_t getConnectedClients() const;
    
    // Configuration
    void setUpdateInterval(uint32_t intervalMs) { config_.systemUpdateInterval = intervalMs; }
    void enableAuthentication(bool enable, const String& username = "", const String& password = "");
    void setCORSEnabled(bool enable) { config_.enableCORS = enable; }
    
    // Event callbacks for integration
    typedef std::function<void(const String&)> LogCallback;
    typedef std::function<void()> SystemUpdateCallback;
    
    void onLog(LogCallback callback) { logCallback_ = callback; }
    void onSystemUpdate(SystemUpdateCallback callback) { systemUpdateCallback_ = callback; }
    
    // Manual update triggers (for external integration)
    void triggerSystemUpdate() { updateSystemMetrics(); broadcastSystemStatus(); }
    void triggerPowerUpdate() { broadcastPowerUpdate(); }
    void triggerStorageUpdate() { broadcastStorageUpdate(); }

private:
    // Core components
    AsyncWebServer server_;
    AsyncWebSocket webSocket_;
    EnhancedWebConfig config_;
    SystemMetrics metrics_;
    bool running_;
    
    // External components
    StreamManager* streamManager_;
    WebLanguageIntegration* languageIntegration_;
    
    // Timing
    unsigned long lastSystemUpdate_;
    unsigned long lastHeartbeat_;
    
    // Callbacks
    LogCallback logCallback_;
    SystemUpdateCallback systemUpdateCallback_;
    
    // Setup methods
    void setupRoutes();
    void setupWebSocket();
    void setupAPIEndpoints();
    void setupStaticFiles();
    void setupCORS();
    
    // WebSocket handlers
    void onWSEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, 
                   AwsEventType type, void* arg, uint8_t* data, size_t len);
    void handleWSMessage(AsyncWebSocketClient* client, const String& message);
    void sendWSMessage(WSMessageType type, const JsonDocument& data);
    void sendHeartbeat();
    
    // API handlers
    void handleDashboard(AsyncWebServerRequest* request);
    void handleAPIStatus(AsyncWebServerRequest* request);
    void handleAPISystemMetrics(AsyncWebServerRequest* request);
    void handleAPIImageList(AsyncWebServerRequest* request);
    void handleAPIImageThumbnail(AsyncWebServerRequest* request);
    void handleAPICapture(AsyncWebServerRequest* request);
    void handleAPIConfig(AsyncWebServerRequest* request);
    void handleAPIConfigUpdate(AsyncWebServerRequest* request);
    void handleAPIStream(AsyncWebServerRequest* request);
    void handleAPIStreamStart(AsyncWebServerRequest* request);
    void handleAPIStreamStop(AsyncWebServerRequest* request);
    void handleAPIStreamStats(AsyncWebServerRequest* request);
    void handleAPIStreamConfig(AsyncWebServerRequest* request);
    void handleAPIStorageStats(AsyncWebServerRequest* request);
    void handleAPIPowerStats(AsyncWebServerRequest* request);
    void handleAPIWildlifeLog(AsyncWebServerRequest* request);
    
    // Analytics API endpoints
    void handleAPIAnalyticsSummary(AsyncWebServerRequest* request);
    void handleAPIAnalyticsWildlife(AsyncWebServerRequest* request);
    void handleAPIAnalyticsSystem(AsyncWebServerRequest* request);
    void handleAPIAnalyticsHistorical(AsyncWebServerRequest* request);
    void handleAPIAnalyticsExport(AsyncWebServerRequest* request);
    void handleAPIAnalyticsSpecies(AsyncWebServerRequest* request);
    void handleAPIAnalyticsActivity(AsyncWebServerRequest* request);
    void handleAPIAnalyticsPerformance(AsyncWebServerRequest* request);
    
    // CSV Export endpoint
    void handleAPIExportDetectionsCSV(AsyncWebServerRequest* request);
    
    // Helper methods
    String generateSystemStatusJSON();
    String generateImageListJSON(int page = 0, int pageSize = 20);
    String generateConfigJSON();
    String generateStorageStatsJSON();
    String generatePowerStatsJSON();
    String generateWildlifeLogJSON(int limit = 50);
    
    // Analytics JSON generators
    String generateAnalyticsSummaryJSON();
    String generateWildlifeAnalyticsJSON(const String& timeRange = "week");
    String generateSystemAnalyticsJSON(const String& timeRange = "week");
    String generateHistoricalAnalyticsJSON(const String& timeRange = "month");
    String generateSpeciesAnalyticsJSON();
    String generateActivityAnalyticsJSON(const String& timeRange = "week");
    String generatePerformanceAnalyticsJSON(const String& timeRange = "week");
    
    // CSV generation helpers
    String escapeCSVField(const String& field);
    String generateCSVRow(const String& timestamp, const String& species, 
                         const String& confidence, const String& imagePath,
                         const String& gpsLat, const String& gpsLon, 
                         const String& batteryLevel);
    
    bool authenticateRequest(AsyncWebServerRequest* request);
    void logMessage(const String& message);
    void updatePowerMetrics();
    void updateStorageMetrics();
    void updateNetworkMetrics();
    void updateCameraMetrics();
};

// Global instance for easy integration
extern EnhancedWebServer* g_enhancedWebServer;

// Utility functions for easy integration with existing code
bool startEnhancedWebServer(const EnhancedWebConfig& config = EnhancedWebConfig());
void stopEnhancedWebServer();
bool isEnhancedWebServerRunning();
void updateWebDashboard();
void notifyWildlifeDetection(const String& species, float confidence, const String& imagePath);
void notifyPowerStateChange();
void notifyStorageChange();

#endif // ENHANCED_WEB_SERVER_H