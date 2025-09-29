/**
 * Cloud Configuration Manager - Dynamic Cloud Service Configuration
 * 
 * Provides centralized configuration management for all cloud services
 * with support for dynamic reconfiguration, credential management,
 * and regional optimization.
 * 
 * Features:
 * - Centralized configuration store
 * - Dynamic reconfiguration capabilities
 * - Secure credential management and rotation
 * - Regional cloud optimization
 * - Configuration validation and backup
 * - Environment-specific configurations
 */

#ifndef CLOUD_CONFIG_MANAGER_H
#define CLOUD_CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include "../cloud_integrator.h"

// Configuration Scopes
enum ConfigScope {
    SCOPE_GLOBAL,           // Global system configuration
    SCOPE_SERVICE,          // Service-specific configuration
    SCOPE_PROVIDER,         // Cloud provider configuration
    SCOPE_REGIONAL,         // Regional configuration
    SCOPE_ENVIRONMENT,      // Environment-specific (dev/staging/prod)
    SCOPE_SECURITY          // Security and credential configuration
};

// Configuration Types
enum ConfigType {
    CONFIG_STRING,
    CONFIG_INTEGER,
    CONFIG_FLOAT,
    CONFIG_BOOLEAN,
    CONFIG_JSON,
    CONFIG_ENCRYPTED,       // Encrypted sensitive data
    CONFIG_CREDENTIAL       // Special handling for credentials
};

// Configuration Entry
struct ConfigEntry {
    String key;
    String value;
    ConfigType type;
    ConfigScope scope;
    bool isEncrypted;
    bool isReadOnly;
    bool requiresRestart;
    uint32_t lastModified;
    String description;
    String defaultValue;
    
    ConfigEntry() : 
        key(""), value(""), type(CONFIG_STRING), scope(SCOPE_GLOBAL),
        isEncrypted(false), isReadOnly(false), requiresRestart(false),
        lastModified(0), description(""), defaultValue("") {}
};

// Regional Configuration
struct RegionalConfig {
    String region;
    String primaryEndpoint;
    String backupEndpoint;
    float latencyThreshold;     // Expected latency in ms
    float costMultiplier;       // Regional cost multiplier
    bool isAvailable;
    std::map<String, String> regionalSettings;
    
    RegionalConfig() : 
        region(""), primaryEndpoint(""), backupEndpoint(""),
        latencyThreshold(200.0), costMultiplier(1.0), isAvailable(true) {}
};

// Environment Configuration
enum EnvironmentType {
    ENV_DEVELOPMENT,
    ENV_STAGING,
    ENV_PRODUCTION,
    ENV_TESTING
};

struct EnvironmentConfig {
    EnvironmentType environment;
    String name;
    bool debugMode;
    bool verboseLogging;
    uint32_t resourceLimits;
    std::map<String, String> envSpecificSettings;
    
    EnvironmentConfig() : 
        environment(ENV_PRODUCTION), name("production"), debugMode(false),
        verboseLogging(false), resourceLimits(0) {}
};

// Credential Configuration
struct CredentialConfig {
    String credentialId;
    String provider;
    String accessKey;
    String secretKey;
    String sessionToken;
    uint32_t expiryTime;
    bool autoRotate;
    uint32_t rotationInterval;  // Rotation interval in seconds
    String encryptionKey;
    
    CredentialConfig() : 
        credentialId(""), provider(""), accessKey(""), secretKey(""),
        sessionToken(""), expiryTime(0), autoRotate(false), rotationInterval(86400),
        encryptionKey("") {}
};

// Configuration Validation Result
struct ValidationResult {
    bool isValid;
    std::vector<String> errors;
    std::vector<String> warnings;
    
    ValidationResult() : isValid(true) {}
};

// Configuration Backup
struct ConfigBackup {
    uint32_t timestamp;
    String backupId;
    std::map<String, ConfigEntry> configurations;
    String checksumHash;
    
    ConfigBackup() : timestamp(0), backupId(""), checksumHash("") {}
};

/**
 * Cloud Configuration Manager Class
 * 
 * Centralized configuration management with dynamic updates and security
 */
class CloudConfigManager {
public:
    CloudConfigManager();
    ~CloudConfigManager();
    
    // Initialization and lifecycle
    bool initialize();
    bool loadConfiguration(const String& configPath = "");
    bool saveConfiguration(const String& configPath = "");
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration management
    bool setConfig(const String& key, const String& value, ConfigScope scope = SCOPE_GLOBAL);
    bool setConfig(const String& key, int value, ConfigScope scope = SCOPE_GLOBAL);
    bool setConfig(const String& key, float value, ConfigScope scope = SCOPE_GLOBAL);
    bool setConfig(const String& key, bool value, ConfigScope scope = SCOPE_GLOBAL);
    bool setConfigJson(const String& key, const JsonDocument& value, ConfigScope scope = SCOPE_GLOBAL);
    
    String getConfigString(const String& key, const String& defaultValue = "", ConfigScope scope = SCOPE_GLOBAL) const;
    int getConfigInt(const String& key, int defaultValue = 0, ConfigScope scope = SCOPE_GLOBAL) const;
    float getConfigFloat(const String& key, float defaultValue = 0.0, ConfigScope scope = SCOPE_GLOBAL) const;
    bool getConfigBool(const String& key, bool defaultValue = false, ConfigScope scope = SCOPE_GLOBAL) const;
    bool getConfigJson(const String& key, JsonDocument& value, ConfigScope scope = SCOPE_GLOBAL) const;
    
    bool hasConfig(const String& key, ConfigScope scope = SCOPE_GLOBAL) const;
    bool removeConfig(const String& key, ConfigScope scope = SCOPE_GLOBAL);
    std::vector<String> getConfigKeys(ConfigScope scope = SCOPE_GLOBAL) const;
    
    // Dynamic reconfiguration
    bool enableDynamicConfig(bool enable) { dynamicConfigEnabled_ = enable; return true; }
    bool isDynamicConfigEnabled() const { return dynamicConfigEnabled_; }
    bool applyDynamicChanges();
    std::vector<String> getPendingChanges() const;
    bool rollbackChanges();
    
    // Regional configuration
    bool addRegionalConfig(const RegionalConfig& config);
    bool removeRegionalConfig(const String& region);
    bool updateRegionalConfig(const String& region, const RegionalConfig& config);
    RegionalConfig getRegionalConfig(const String& region) const;
    std::vector<RegionalConfig> getAllRegionalConfigs() const;
    String selectOptimalRegion(const String& currentLocation = "") const;
    
    // Environment configuration
    bool setEnvironment(EnvironmentType environment);
    EnvironmentType getCurrentEnvironment() const { return currentEnvironment_.environment; }
    bool updateEnvironmentConfig(const EnvironmentConfig& config);
    EnvironmentConfig getEnvironmentConfig() const { return currentEnvironment_; }
    
    // Credential management
    bool addCredential(const CredentialConfig& credential);
    bool removeCredential(const String& credentialId);
    bool updateCredential(const String& credentialId, const CredentialConfig& credential);
    CredentialConfig getCredential(const String& credentialId) const;
    std::vector<String> getCredentialIds() const;
    bool rotateCredentials(const String& credentialId = "");
    bool isCredentialExpired(const String& credentialId) const;
    
    // Security features
    bool encryptSensitiveData(bool enable);
    bool setEncryptionKey(const String& key);
    bool encryptValue(const String& value, String& encrypted) const;
    bool decryptValue(const String& encrypted, String& value) const;
    bool validateCredentials() const;
    
    // Configuration validation
    ValidationResult validateConfiguration() const;
    ValidationResult validateConfigEntry(const ConfigEntry& entry) const;
    bool setConfigValidationRules(const String& key, const String& rules);
    
    // Configuration backup and restore
    bool createBackup(const String& backupId = "");
    bool restoreFromBackup(const String& backupId);
    std::vector<String> getAvailableBackups() const;
    bool deleteBackup(const String& backupId);
    bool verifyBackupIntegrity(const String& backupId) const;
    
    // Cloud provider configuration
    bool configureCloudProvider(CloudPlatform platform, const CloudConfig& config);
    CloudConfig getCloudProviderConfig(CloudPlatform platform) const;
    bool isCloudProviderConfigured(CloudPlatform platform) const;
    std::vector<CloudPlatform> getConfiguredProviders() const;
    
    // Service-specific configuration
    bool setServiceConfig(const String& serviceName, const String& key, const String& value);
    String getServiceConfig(const String& serviceName, const String& key, const String& defaultValue = "") const;
    std::map<String, String> getAllServiceConfig(const String& serviceName) const;
    bool removeServiceConfig(const String& serviceName, const String& key);
    
    // Configuration monitoring and notifications
    typedef void (*ConfigChangeCallback)(const String& key, const String& oldValue, const String& newValue, ConfigScope scope);
    typedef void (*CredentialExpiryCallback)(const String& credentialId, uint32_t expiryTime);
    typedef void (*ValidationErrorCallback)(const ValidationResult& result);
    
    void setConfigChangeCallback(ConfigChangeCallback callback) { configChangeCallback_ = callback; }
    void setCredentialExpiryCallback(CredentialExpiryCallback callback) { credentialExpiryCallback_ = callback; }
    void setValidationErrorCallback(ValidationErrorCallback callback) { validationErrorCallback_ = callback; }
    
    // Reporting and diagnostics
    void generateConfigReport(String& report) const;
    void generateSecurityReport(String& report) const;
    void generateRegionalReport(String& report) const;
    
    // Import/Export functionality
    bool exportConfiguration(const String& filePath, ConfigScope scope = SCOPE_GLOBAL) const;
    bool importConfiguration(const String& filePath, bool overwrite = false);
    String exportConfigurationToJson(ConfigScope scope = SCOPE_GLOBAL) const;
    bool importConfigurationFromJson(const String& jsonData, bool overwrite = false);

private:
    // Core state
    bool initialized_;
    bool dynamicConfigEnabled_;
    
    // Configuration storage
    std::map<String, ConfigEntry> configurations_;
    std::map<String, RegionalConfig> regionalConfigs_;
    std::map<String, CredentialConfig> credentials_;
    std::map<String, CloudConfig> providerConfigs_;
    std::map<String, ConfigBackup> backups_;
    
    // Environment and security
    EnvironmentConfig currentEnvironment_;
    String encryptionKey_;
    bool encryptionEnabled_;
    
    // Change tracking
    std::map<String, String> pendingChanges_;
    std::map<String, String> rollbackValues_;
    
    // Validation rules
    std::map<String, String> validationRules_;
    
    // Callbacks
    ConfigChangeCallback configChangeCallback_;
    CredentialExpiryCallback credentialExpiryCallback_;
    ValidationErrorCallback validationErrorCallback_;
    
    // Internal helper methods
    String buildConfigKey(const String& key, ConfigScope scope) const;
    bool storeConfigEntry(const String& key, const String& value, ConfigType type, ConfigScope scope);
    ConfigEntry getConfigEntry(const String& key, ConfigScope scope) const;
    bool validateConfigValue(const String& key, const String& value, ConfigType type) const;
    
    // Encryption helpers
    bool shouldEncryptKey(const String& key) const;
    String generateEncryptionKey() const;
    String calculateChecksum(const std::map<String, ConfigEntry>& configs) const;
    
    // Credential helpers
    bool isCredentialKey(const String& key) const;
    void checkCredentialExpiry();
    bool performCredentialRotation(const String& credentialId);
    
    // Backup helpers
    String generateBackupId() const;
    bool createBackupSnapshot(ConfigBackup& backup) const;
    bool validateBackup(const ConfigBackup& backup) const;
    
    // Regional optimization helpers
    float calculateRegionalScore(const RegionalConfig& regional, const String& currentLocation) const;
    bool isRegionAvailable(const String& region) const;
    
    // Notification helpers
    void notifyConfigChange(const String& key, const String& oldValue, const String& newValue, ConfigScope scope);
    void notifyCredentialExpiry(const String& credentialId);
    void notifyValidationError(const ValidationResult& result);
    
    // JSON serialization helpers
    bool serializeConfigEntry(const ConfigEntry& entry, JsonObject& obj) const;
    bool deserializeConfigEntry(const JsonObject& obj, ConfigEntry& entry) const;
    bool serializeRegionalConfig(const RegionalConfig& config, JsonObject& obj) const;
    bool deserializeRegionalConfig(const JsonObject& obj, RegionalConfig& config) const;
    
    // File I/O helpers
    bool writeConfigFile(const String& filePath, const String& data) const;
    bool readConfigFile(const String& filePath, String& data) const;
    bool ensureConfigDirectory() const;
};

// Global configuration manager instance
extern CloudConfigManager* g_cloudConfigManager;

// Utility functions for easy integration
bool initializeCloudConfigManager();
bool loadCloudConfiguration(const String& configPath = "");
String getCloudConfig(const String& key, const String& defaultValue = "");
bool setCloudConfig(const String& key, const String& value);
bool isCloudConfigured();
void cleanupCloudConfigManager();

#endif // CLOUD_CONFIG_MANAGER_H