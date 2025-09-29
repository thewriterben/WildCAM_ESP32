/**
 * OTA Manager - Over-the-Air Update System
 * 
 * Provides secure, reliable firmware updates for wildlife camera networks.
 * Supports staged rollouts, A/B testing, and automatic rollback capabilities.
 * Enhanced with AsyncElegantOTA for web-based updates (Phase 1).
 * 
 * Features:
 * - Secure OTA updates with signature verification
 * - Web-based update interface via AsyncElegantOTA
 * - Staged deployment across network nodes
 * - Automatic health monitoring and rollback
 * - Bandwidth-efficient delta updates
 * - Update scheduling and coordination
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <Update.h>
#include <WiFi.h>
#include <HTTPUpdate.h>
#include <esp_ota_ops.h>
#include <esp_app_desc.h>

// AsyncElegantOTA integration for Phase 1 enhancement
#ifdef OTA_ENABLED
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#endif

// OTA Update Status
enum OTAStatus {
    OTA_IDLE,
    OTA_CHECKING,
    OTA_DOWNLOADING,
    OTA_INSTALLING,
    OTA_VERIFYING,
    OTA_COMPLETE,
    OTA_FAILED,
    OTA_ROLLED_BACK
};

// Update Priority Levels
enum UpdatePriority {
    PRIORITY_CRITICAL = 0,    // Security fixes, immediate deployment
    PRIORITY_HIGH = 1,        // Bug fixes, deploy within 24h
    PRIORITY_NORMAL = 2,      // Features, deploy within week
    PRIORITY_LOW = 3          // Optimizations, deploy when convenient
};

// Deployment Stage Configuration
enum DeploymentStage {
    STAGE_CANARY = 0,        // 5% of nodes for initial testing
    STAGE_PILOT = 1,         // 25% of nodes for broader testing
    STAGE_PRODUCTION = 2     // Full deployment to all nodes
};

// OTA Configuration Structure
struct OTAConfig {
    String updateServerURL;
    String deviceID;
    String networkID;
    String currentVersion;
    String targetVersion;
    uint32_t checkInterval;
    bool autoUpdate;
    bool stagedDeployment;
    DeploymentStage currentStage;
    UpdatePriority minPriority;
    uint32_t rollbackTimeout;
    bool signatureVerification;
    
    OTAConfig() : 
        updateServerURL(""),
        deviceID(""),
        networkID(""),
        currentVersion(""),
        targetVersion(""),
        checkInterval(3600000),  // 1 hour
        autoUpdate(false),
        stagedDeployment(true),
        currentStage(STAGE_CANARY),
        minPriority(PRIORITY_NORMAL),
        rollbackTimeout(300000), // 5 minutes
        signatureVerification(true) {}
};

// Update Package Information
struct UpdatePackage {
    String version;
    String description;
    uint32_t size;
    String checksum;
    String signature;
    UpdatePriority priority;
    DeploymentStage targetStage;
    String downloadURL;
    bool deltaUpdate;
    String baseVersion;
    uint32_t timestamp;
    
    UpdatePackage() : 
        version(""),
        description(""),
        size(0),
        checksum(""),
        signature(""),
        priority(PRIORITY_NORMAL),
        targetStage(STAGE_CANARY),
        downloadURL(""),
        deltaUpdate(false),
        baseVersion(""),
        timestamp(0) {}
};

// OTA Health Metrics
struct OTAHealthMetrics {
    uint32_t successfulUpdates;
    uint32_t failedUpdates;
    uint32_t rolledBackUpdates;
    uint32_t totalUpdateTime;
    uint32_t averageUpdateTime;
    float successRate;
    uint32_t lastUpdateTimestamp;
    String lastUpdateVersion;
    bool systemHealthy;
    
    OTAHealthMetrics() : 
        successfulUpdates(0),
        failedUpdates(0),
        rolledBackUpdates(0),
        totalUpdateTime(0),
        averageUpdateTime(0),
        successRate(100.0),
        lastUpdateTimestamp(0),
        lastUpdateVersion(""),
        systemHealthy(true) {}
};

/**
 * OTA Manager Class
 * 
 * Manages the complete OTA update lifecycle from discovery to deployment
 */
class OTAManager {
public:
    OTAManager();
    ~OTAManager();
    
    // Initialization and configuration
    bool init(const OTAConfig& config);
    void cleanup();
    bool configure(const OTAConfig& config);
    OTAConfig getConfiguration() const { return config_; }
    
    // Update discovery and management
    bool checkForUpdates();
    bool hasAvailableUpdate() const { return hasAvailableUpdate_; }
    UpdatePackage getAvailableUpdate() const { return availableUpdate_; }
    bool shouldUpdate() const;
    
    // Update execution
    bool startUpdate(bool forceUpdate = false);
    bool pauseUpdate();
    bool resumeUpdate();
    bool cancelUpdate();
    
    // Status and monitoring
    OTAStatus getStatus() const { return status_; }
    float getProgress() const { return progress_; }
    String getStatusMessage() const { return statusMessage_; }
    OTAHealthMetrics getHealthMetrics() const { return healthMetrics_; }
    
    // Rollback capabilities
    bool canRollback() const;
    bool initiateRollback();
    bool confirmUpdate();
    
    // Network coordination
    bool registerForUpdates(const String& networkID);
    bool reportStatus(const String& status);
    bool coordinateWithNetwork();
    
    // Security features
    bool verifySignature(const uint8_t* data, size_t length, const String& signature);
    bool verifyChecksum(const uint8_t* data, size_t length, const String& checksum);
    bool validateUpdatePackage(const UpdatePackage& package);
    
    // Utility functions
    String getCurrentVersion() const;
    String getBootPartition() const;
    uint32_t getFreeSpace() const;
    bool isUpdateInProgress() const { return status_ != OTA_IDLE && status_ != OTA_COMPLETE; }
    
    // Event callbacks
    typedef void (*OTAProgressCallback)(float progress, const String& message);
    typedef void (*OTAStatusCallback)(OTAStatus status, const String& message);
    typedef void (*OTAErrorCallback)(int errorCode, const String& errorMessage);
    
    void setProgressCallback(OTAProgressCallback callback) { progressCallback_ = callback; }
    void setStatusCallback(OTAStatusCallback callback) { statusCallback_ = callback; }
    void setErrorCallback(OTAErrorCallback callback) { errorCallback_ = callback; }

#ifdef OTA_ENABLED
    // AsyncElegantOTA Web Interface Integration - Phase 1 Enhancement
    bool initWebOTA(AsyncWebServer* server);
    bool startWebOTA(uint16_t port = 80, const String& username = "admin", const String& password = "wildlife");
    void stopWebOTA();
    bool isWebOTARunning() const { return webOTARunning_; }
    String getWebOTAURL() const;
    
    // Web interface configuration
    void setWebOTACredentials(const String& username, const String& password);
    void enableWebOTAAuth(bool enable) { webOTAAuthEnabled_ = enable; }
    void setWebOTAPath(const String& path) { webOTAPath_ = path; }
    
    // Web OTA callbacks
    void onWebOTAStart(std::function<void()> callback) { webOTAStartCallback_ = callback; }
    void onWebOTAProgress(std::function<void(size_t current, size_t total)> callback) { webOTAProgressCallback_ = callback; }
    void onWebOTAEnd(std::function<void(bool success)> callback) { webOTAEndCallback_ = callback; }
#endif

private:
    // Core components
    OTAConfig config_;
    OTAStatus status_;
    UpdatePackage availableUpdate_;
    OTAHealthMetrics healthMetrics_;
    
    // State management
    bool initialized_;
    bool hasAvailableUpdate_;
    float progress_;
    String statusMessage_;
    uint32_t updateStartTime_;
    uint32_t rollbackTimeout_;
    
    // Network communication
    WiFiClient otaClient_;
    HTTPUpdate httpUpdate_;
    
    // Callbacks
    OTAProgressCallback progressCallback_;
    OTAStatusCallback statusCallback_;
    OTAErrorCallback errorCallback_;
    
#ifdef OTA_ENABLED
    // AsyncElegantOTA Web Interface state
    AsyncWebServer* webOTAServer_;
    bool webOTARunning_;
    bool webOTAAuthEnabled_;
    String webOTAUsername_;
    String webOTAPassword_;
    String webOTAPath_;
    uint16_t webOTAPort_;
    
    // Web OTA callbacks
    std::function<void()> webOTAStartCallback_;
    std::function<void(size_t, size_t)> webOTAProgressCallback_;
    std::function<void(bool)> webOTAEndCallback_;
#endif
    
    // Internal methods
    bool downloadUpdate(const UpdatePackage& package);
    bool installUpdate(const uint8_t* data, size_t length);
    bool verifyInstallation();
    void updateProgress(float progress, const String& message);
    void updateStatus(OTAStatus status, const String& message);
    void reportError(int errorCode, const String& errorMessage);
    
    // Staged deployment helpers
    bool isEligibleForStage(DeploymentStage stage) const;
    DeploymentStage calculateDeploymentStage() const;
    bool waitForStageApproval(DeploymentStage stage);
    
    // Health monitoring
    void monitorUpdateHealth();
    bool checkSystemHealth() const;
    void updateHealthMetrics(bool successful);
    
    // Security helpers
    bool initializeCrypto();
    bool loadPublicKey();
    String calculateChecksum(const uint8_t* data, size_t length);
};

// Global OTA manager instance
extern OTAManager* g_otaManager;

// Utility functions for easy integration
bool initializeOTA(const OTAConfig& config);
bool checkAndUpdateFirmware();
bool forceUpdate(const String& version = "");
void cleanupOTA();

// OTA status monitoring
OTAStatus getOTAStatus();
float getOTAProgress();
String getOTAMessage();

#endif // OTA_MANAGER_H