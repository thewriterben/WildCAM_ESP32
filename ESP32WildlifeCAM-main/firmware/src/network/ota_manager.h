/**
 * @file ota_manager.h
 * @brief Enhanced OTA Update Manager with AsyncElegantOTA Integration
 * 
 * Simplified OTA management for network integration, focusing on
 * web-based updates and easy integration with existing WiFi systems.
 * 
 * Phase 1 Core AI Enhancement Integration
 */

#ifndef NETWORK_OTA_MANAGER_H
#define NETWORK_OTA_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Update.h>

// AsyncElegantOTA integration for Phase 1 enhancement
#ifdef OTA_ENABLED
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#endif

/**
 * OTA Status
 */
enum NetworkOTAStatus {
    NET_OTA_IDLE,
    NET_OTA_STARTING,
    NET_OTA_RUNNING,
    NET_OTA_UPDATING,
    NET_OTA_SUCCESS,
    NET_OTA_FAILED,
    NET_OTA_DISABLED
};

/**
 * OTA Configuration
 */
struct NetworkOTAConfig {
    bool enabled;
    uint16_t port;
    String username;
    String password;
    String path;
    bool requireAuth;
    bool autoReboot;
    uint32_t timeoutMs;
    
    NetworkOTAConfig() : enabled(true), port(80), username("admin"), password("wildlife"),
                        path("/update"), requireAuth(true), autoReboot(true), timeoutMs(30000) {}
};

/**
 * Network OTA Manager
 * 
 * Simplified OTA manager for network integration with focus on:
 * - Easy web-based firmware updates
 * - Integration with existing WiFi systems
 * - Minimal configuration required
 * - Status monitoring and progress tracking
 */
class NetworkOTAManager {
public:
    NetworkOTAManager();
    ~NetworkOTAManager();
    
    // Initialization
    bool init(const NetworkOTAConfig& config = NetworkOTAConfig());
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Web server integration
    bool begin(AsyncWebServer* server = nullptr);
    bool begin(uint16_t port, const String& username = "admin", const String& password = "wildlife");
    void end();
    
    // Status and monitoring
    NetworkOTAStatus getStatus() const { return status_; }
    String getStatusString() const;
    float getProgress() const { return progress_; }
    String getLastError() const { return lastError_; }
    bool isRunning() const { return status_ == NET_OTA_RUNNING; }
    
    // Configuration
    void setCredentials(const String& username, const String& password);
    void setPath(const String& path);
    void setPort(uint16_t port);
    void enableAuth(bool enable);
    void enableAutoReboot(bool enable);
    
    // Information
    String getUpdateURL() const;
    String getCurrentVersion() const;
    uint32_t getFreeSpace() const;
    bool hasEnoughSpace(size_t requiredSize) const;
    
    // Event callbacks
    typedef std::function<void()> OTACallback;
    typedef std::function<void(size_t current, size_t total)> OTAProgressCallback;
    typedef std::function<void(bool success)> OTAEndCallback;
    
    void onStart(OTACallback callback) { onStartCallback_ = callback; }
    void onProgress(OTAProgressCallback callback) { onProgressCallback_ = callback; }
    void onEnd(OTAEndCallback callback) { onEndCallback_ = callback; }
    void onError(OTACallback callback) { onErrorCallback_ = callback; }
    
    // Static utility methods
    static String getChipInfo();
    static String getFirmwareInfo();
    static bool validateFirmware(const uint8_t* data, size_t length);

private:
    // Configuration
    NetworkOTAConfig config_;
    bool initialized_;
    bool serverOwned_;
    
    // State
    NetworkOTAStatus status_;
    float progress_;
    String lastError_;
    uint32_t startTime_;
    
#ifdef OTA_ENABLED
    // AsyncElegantOTA components
    AsyncWebServer* webServer_;
#endif
    
    // Callbacks
    OTACallback onStartCallback_;
    OTAProgressCallback onProgressCallback_;
    OTAEndCallback onEndCallback_;
    OTACallback onErrorCallback_;
    
    // Private methods
    void updateStatus(NetworkOTAStatus status);
    void setError(const String& error);
    void setupOTAHandlers();
    void handleOTAStart();
    void handleOTAProgress(size_t current, size_t total);
    void handleOTAEnd(bool success);
    String generateInfoPage() const;
};

// Global instance
extern NetworkOTAManager* g_networkOTAManager;

// Utility functions
bool initializeNetworkOTA(const NetworkOTAConfig& config = NetworkOTAConfig());
void cleanupNetworkOTA();
bool startOTAServer(uint16_t port = 80, const String& username = "admin", const String& password = "wildlife");
void stopOTAServer();
NetworkOTAStatus getNetworkOTAStatus();
String getOTAUpdateURL();

#endif // NETWORK_OTA_MANAGER_H