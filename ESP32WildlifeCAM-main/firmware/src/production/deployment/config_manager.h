/**
 * Configuration Manager - Centralized Configuration Deployment
 * 
 * Manages configuration deployment and versioning across wildlife camera networks.
 * Supports hierarchical configuration, environment-specific settings, and rollback.
 * 
 * Features:
 * - Centralized configuration management
 * - Version control and rollback capabilities
 * - Environment-specific configuration profiles
 * - Real-time configuration updates
 * - Configuration validation and testing
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <map>
#include <vector>

// Configuration Types
enum ConfigType {
    CONFIG_SYSTEM,         // System-level configuration
    CONFIG_CAMERA,         // Camera and imaging settings
    CONFIG_AI,             // AI and ML model parameters
    CONFIG_POWER,          // Power management settings
    CONFIG_NETWORK,        // Network and communication
    CONFIG_SECURITY,       // Security and authentication
    CONFIG_DEPLOYMENT,     // Deployment-specific settings
    CONFIG_CUSTOM          // Custom application settings
};

// Configuration Scope
enum ConfigScope {
    SCOPE_GLOBAL,          // Applies to entire network
    SCOPE_SITE,            // Applies to deployment site
    SCOPE_GROUP,           // Applies to device group
    SCOPE_DEVICE           // Applies to specific device
};

// Configuration Status
enum ConfigStatus {
    CONFIG_UNKNOWN,
    CONFIG_PENDING,        // Waiting for deployment
    CONFIG_DEPLOYING,      // Currently being deployed
    CONFIG_ACTIVE,         // Currently active configuration
    CONFIG_FAILED,         // Deployment failed
    CONFIG_ROLLED_BACK     // Configuration was rolled back
};

// Configuration Entry Structure
struct ConfigEntry {
    String key;
    String value;
    ConfigType type;
    ConfigScope scope;
    String description;
    bool required;
    String defaultValue;
    String validationRule;
    uint32_t timestamp;
    String version;
    
    ConfigEntry() : 
        key(""), value(""), type(CONFIG_SYSTEM), scope(SCOPE_DEVICE),
        description(""), required(false), defaultValue(""),
        validationRule(""), timestamp(0), version("") {}
};

// Configuration Profile
struct ConfigProfile {
    String profileId;
    String name;
    String description;
    String environment;  // development, staging, production
    std::vector<ConfigEntry> entries;
    uint32_t version;
    uint32_t timestamp;
    String checksum;
    ConfigStatus status;
    
    ConfigProfile() : 
        profileId(""), name(""), description(""), environment(""),
        version(0), timestamp(0), checksum(""), status(CONFIG_UNKNOWN) {}
};

// Configuration Change Event
struct ConfigChangeEvent {
    String configKey;
    String oldValue;
    String newValue;
    ConfigType type;
    uint32_t timestamp;
    String source;
    bool rollback;
    
    ConfigChangeEvent() : 
        configKey(""), oldValue(""), newValue(""), type(CONFIG_SYSTEM),
        timestamp(0), source(""), rollback(false) {}
};

// Configuration Validation Result
struct ValidationResult {
    bool valid;
    String errorMessage;
    std::vector<String> warnings;
    std::vector<String> suggestions;
    
    ValidationResult() : valid(true), errorMessage("") {}
};

/**
 * Configuration Manager Class
 * 
 * Manages the complete configuration lifecycle from deployment to rollback
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Initialization and setup
    bool init(const String& deviceId, const String& networkId);
    void cleanup();
    bool loadDefaultConfiguration();
    bool saveConfiguration();
    
    // Configuration management
    bool setConfig(const String& key, const String& value, ConfigType type = CONFIG_SYSTEM);
    String getConfig(const String& key, const String& defaultValue = "") const;
    bool hasConfig(const String& key) const;
    bool removeConfig(const String& key);
    
    // Type-specific getters with validation
    int getInt(const String& key, int defaultValue = 0) const;
    float getFloat(const String& key, float defaultValue = 0.0) const;
    bool getBool(const String& key, bool defaultValue = false) const;
    String getString(const String& key, const String& defaultValue = "") const;
    
    // Profile management
    bool loadProfile(const String& profileId);
    bool saveProfile(const ConfigProfile& profile);
    bool activateProfile(const String& profileId);
    String getActiveProfileId() const { return activeProfileId_; }
    std::vector<String> getAvailableProfiles() const;
    
    // Configuration deployment
    bool deployConfiguration(const ConfigProfile& profile);
    bool validateConfiguration(const ConfigProfile& profile, ValidationResult& result);
    bool rollbackConfiguration();
    bool canRollback() const { return !rollbackProfile_.profileId.isEmpty(); }
    
    // Network synchronization
    bool syncWithServer(const String& serverURL);
    bool publishConfiguration();
    bool subscribeToUpdates();
    
    // Version control
    uint32_t getCurrentVersion() const { return currentVersion_; }
    String getVersionHistory() const;
    bool revertToVersion(uint32_t version);
    
    // Monitoring and events
    ConfigStatus getStatus() const { return status_; }
    std::vector<ConfigChangeEvent> getChangeHistory() const { return changeHistory_; }
    void clearChangeHistory() { changeHistory_.clear(); }
    
    // Environment management
    bool setEnvironment(const String& environment);
    String getEnvironment() const { return environment_; }
    bool isProductionEnvironment() const { return environment_ == "production"; }
    
    // Configuration categories
    void setCameraConfig(const String& key, const String& value) { setConfig(key, value, CONFIG_CAMERA); }
    void setAIConfig(const String& key, const String& value) { setConfig(key, value, CONFIG_AI); }
    void setPowerConfig(const String& key, const String& value) { setConfig(key, value, CONFIG_POWER); }
    void setNetworkConfig(const String& key, const String& value) { setConfig(key, value, CONFIG_NETWORK); }
    void setSecurityConfig(const String& key, const String& value) { setConfig(key, value, CONFIG_SECURITY); }
    
    // Utility functions
    String exportConfiguration(ConfigType type = CONFIG_SYSTEM) const;
    bool importConfiguration(const String& jsonConfig);
    void printConfiguration() const;
    String generateChecksum() const;
    
    // Event callbacks
    typedef void (*ConfigChangeCallback)(const ConfigChangeEvent& event);
    typedef void (*ConfigErrorCallback)(const String& error);
    typedef void (*ConfigSyncCallback)(bool success, const String& message);
    
    void setChangeCallback(ConfigChangeCallback callback) { changeCallback_ = callback; }
    void setErrorCallback(ConfigErrorCallback callback) { errorCallback_ = callback; }
    void setSyncCallback(ConfigSyncCallback callback) { syncCallback_ = callback; }

private:
    // Core state
    String deviceId_;
    String networkId_;
    String environment_;
    bool initialized_;
    ConfigStatus status_;
    uint32_t currentVersion_;
    String activeProfileId_;
    
    // Configuration storage
    std::map<String, ConfigEntry> configMap_;
    ConfigProfile activeProfile_;
    ConfigProfile rollbackProfile_;
    std::vector<ConfigProfile> profileHistory_;
    std::vector<ConfigChangeEvent> changeHistory_;
    
    // Callbacks
    ConfigChangeCallback changeCallback_;
    ConfigErrorCallback errorCallback_;
    ConfigSyncCallback syncCallback_;
    
    // File system paths
    String configFilePath_;
    String profilesPath_;
    String historyPath_;
    
    // Internal methods
    bool loadFromFile(const String& filename);
    bool saveToFile(const String& filename) const;
    ConfigEntry createConfigEntry(const String& key, const String& value, ConfigType type);
    bool validateConfigEntry(const ConfigEntry& entry, String& error) const;
    void recordChange(const String& key, const String& oldValue, const String& newValue, ConfigType type);
    void notifyChange(const ConfigChangeEvent& event);
    void notifyError(const String& error);
    void incrementVersion();
    
    // Validation helpers
    bool validateKey(const String& key) const;
    bool validateValue(const String& value, const ConfigEntry& entry) const;
    bool validateType(const String& value, ConfigType type) const;
    
    // Serialization
    bool serializeProfile(const ConfigProfile& profile, JsonDocument& doc) const;
    bool deserializeProfile(const JsonDocument& doc, ConfigProfile& profile) const;
    String configTypeToString(ConfigType type) const;
    ConfigType stringToConfigType(const String& typeStr) const;
    String configScopeToString(ConfigScope scope) const;
    ConfigScope stringToConfigScope(const String& scopeStr) const;
};

// Global configuration manager instance
extern ConfigManager* g_configManager;

// Utility functions for easy integration
bool initializeConfigManager(const String& deviceId, const String& networkId);
bool deployConfig(const String& key, const String& value, ConfigType type = CONFIG_SYSTEM);
String getConfigValue(const String& key, const String& defaultValue = "");
bool loadConfigProfile(const String& profileId);
void cleanupConfigManager();

// Quick access to common configuration categories
namespace CameraConfig {
    void setResolution(const String& resolution);
    void setQuality(int quality);
    void setFrameRate(int fps);
    String getResolution();
    int getQuality();
    int getFrameRate();
}

namespace AIConfig {
    void setModel(const String& modelPath);
    void setThreshold(float threshold);
    void setBatchSize(int batchSize);
    String getModel();
    float getThreshold();
    int getBatchSize();
}

namespace PowerConfig {
    void setSleepMode(const String& mode);
    void setBatteryThreshold(float threshold);
    void setSolarOptimization(bool enabled);
    String getSleepMode();
    float getBatteryThreshold();
    bool getSolarOptimization();
}

#endif // CONFIG_MANAGER_H