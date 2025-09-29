/**
 * Cloud Configuration Manager Implementation
 * 
 * Dynamic configuration management with security and validation
 */

#include "cloud_config_manager.h"
#include "../../../../utils/logger.h"
#include <FS.h>
#include <SPIFFS.h>

// Global instance
CloudConfigManager* g_cloudConfigManager = nullptr;

CloudConfigManager::CloudConfigManager() : 
    initialized_(false),
    dynamicConfigEnabled_(true),
    encryptionEnabled_(false),
    configChangeCallback_(nullptr),
    credentialExpiryCallback_(nullptr),
    validationErrorCallback_(nullptr) {
    
    currentEnvironment_.environment = ENV_PRODUCTION;
    currentEnvironment_.name = "production";
}

CloudConfigManager::~CloudConfigManager() {
    cleanup();
}

bool CloudConfigManager::initialize() {
    if (initialized_) {
        Logger::warning("Cloud Configuration Manager already initialized");
        return true;
    }
    
    Logger::info("Initializing Cloud Configuration Manager");
    
    // Initialize SPIFFS for configuration storage
    if (!SPIFFS.begin(true)) {
        Logger::error("Failed to initialize SPIFFS");
        return false;
    }
    
    // Load default configuration
    if (!loadConfiguration()) {
        Logger::warning("Failed to load configuration, using defaults");
        // Create default configuration
        setConfig("system.version", "1.0.0");
        setConfig("cloud.enabled", true);
        setConfig("security.encryption_enabled", true);
    }
    
    initialized_ = true;
    Logger::info("Cloud Configuration Manager initialized successfully");
    return true;
}

bool CloudConfigManager::loadConfiguration(const String& configPath) {
    String path = configPath.isEmpty() ? "/config/cloud_config.json" : configPath;
    
    if (!SPIFFS.exists(path)) {
        Logger::warning("Configuration file not found: " + path);
        return false;
    }
    
    String configData;
    if (!readConfigFile(path, configData)) {
        Logger::error("Failed to read configuration file: " + path);
        return false;
    }
    
    return importConfigurationFromJson(configData, false);
}

bool CloudConfigManager::saveConfiguration(const String& configPath) {
    String path = configPath.isEmpty() ? "/config/cloud_config.json" : configPath;
    
    String configJson = exportConfigurationToJson();
    if (configJson.isEmpty()) {
        Logger::error("Failed to export configuration to JSON");
        return false;
    }
    
    if (!writeConfigFile(path, configJson)) {
        Logger::error("Failed to write configuration file: " + path);
        return false;
    }
    
    Logger::info("Configuration saved to: " + path);
    return true;
}

void CloudConfigManager::cleanup() {
    if (!initialized_) {
        return;
    }
    
    Logger::info("Cleaning up Cloud Configuration Manager");
    
    // Save current configuration
    saveConfiguration();
    
    // Clear all data
    configurations_.clear();
    regionalConfigs_.clear();
    credentials_.clear();
    providerConfigs_.clear();
    backups_.clear();
    pendingChanges_.clear();
    rollbackValues_.clear();
    validationRules_.clear();
    
    initialized_ = false;
    Logger::info("Cloud Configuration Manager cleanup completed");
}

bool CloudConfigManager::setConfig(const String& key, const String& value, ConfigScope scope) {
    return storeConfigEntry(key, value, CONFIG_STRING, scope);
}

bool CloudConfigManager::setConfig(const String& key, int value, ConfigScope scope) {
    return storeConfigEntry(key, String(value), CONFIG_INTEGER, scope);
}

bool CloudConfigManager::setConfig(const String& key, float value, ConfigScope scope) {
    return storeConfigEntry(key, String(value), CONFIG_FLOAT, scope);
}

bool CloudConfigManager::setConfig(const String& key, bool value, ConfigScope scope) {
    return storeConfigEntry(key, String(value ? "true" : "false"), CONFIG_BOOLEAN, scope);
}

String CloudConfigManager::getConfigString(const String& key, const String& defaultValue, ConfigScope scope) const {
    ConfigEntry entry = getConfigEntry(key, scope);
    if (entry.key.isEmpty()) {
        return defaultValue;
    }
    
    if (entry.isEncrypted) {
        String decrypted;
        if (decryptValue(entry.value, decrypted)) {
            return decrypted;
        }
        Logger::error("Failed to decrypt configuration value: " + key);
        return defaultValue;
    }
    
    return entry.value;
}

int CloudConfigManager::getConfigInt(const String& key, int defaultValue, ConfigScope scope) const {
    String value = getConfigString(key, String(defaultValue), scope);
    return value.toInt();
}

float CloudConfigManager::getConfigFloat(const String& key, float defaultValue, ConfigScope scope) const {
    String value = getConfigString(key, String(defaultValue), scope);
    return value.toFloat();
}

bool CloudConfigManager::getConfigBool(const String& key, bool defaultValue, ConfigScope scope) const {
    String value = getConfigString(key, String(defaultValue ? "true" : "false"), scope);
    return value.equalsIgnoreCase("true") || value == "1";
}

bool CloudConfigManager::hasConfig(const String& key, ConfigScope scope) const {
    String configKey = buildConfigKey(key, scope);
    return configurations_.find(configKey) != configurations_.end();
}

bool CloudConfigManager::removeConfig(const String& key, ConfigScope scope) {
    String configKey = buildConfigKey(key, scope);
    auto it = configurations_.find(configKey);
    
    if (it == configurations_.end()) {
        return false;
    }
    
    String oldValue = it->second.value;
    configurations_.erase(it);
    
    // Notify change
    notifyConfigChange(key, oldValue, "", scope);
    
    Logger::info("Configuration removed: " + key);
    return true;
}

std::vector<String> CloudConfigManager::getConfigKeys(ConfigScope scope) const {
    std::vector<String> keys;
    String scopePrefix = buildConfigKey("", scope);
    
    for (const auto& pair : configurations_) {
        if (pair.first.startsWith(scopePrefix)) {
            String key = pair.first.substring(scopePrefix.length());
            keys.push_back(key);
        }
    }
    
    return keys;
}

bool CloudConfigManager::applyDynamicChanges() {
    if (!dynamicConfigEnabled_) {
        Logger::warning("Dynamic configuration is disabled");
        return false;
    }
    
    Logger::info("Applying dynamic configuration changes");
    
    for (const auto& change : pendingChanges_) {
        String key = change.first;
        String newValue = change.second;
        
        // Store rollback value
        if (hasConfig(key)) {
            rollbackValues_[key] = getConfigString(key);
        }
        
        // Apply change
        setConfig(key, newValue);
    }
    
    pendingChanges_.clear();
    Logger::info("Dynamic configuration changes applied");
    return true;
}

std::vector<String> CloudConfigManager::getPendingChanges() const {
    std::vector<String> changes;
    for (const auto& pair : pendingChanges_) {
        changes.push_back(pair.first + " = " + pair.second);
    }
    return changes;
}

bool CloudConfigManager::rollbackChanges() {
    Logger::info("Rolling back configuration changes");
    
    for (const auto& rollback : rollbackValues_) {
        String key = rollback.first;
        String oldValue = rollback.second;
        setConfig(key, oldValue);
    }
    
    rollbackValues_.clear();
    pendingChanges_.clear();
    
    Logger::info("Configuration changes rolled back");
    return true;
}

bool CloudConfigManager::addRegionalConfig(const RegionalConfig& config) {
    if (config.region.isEmpty()) {
        Logger::error("Regional config missing region identifier");
        return false;
    }
    
    regionalConfigs_[config.region] = config;
    Logger::info("Regional configuration added: " + config.region);
    return true;
}

bool CloudConfigManager::removeRegionalConfig(const String& region) {
    auto it = regionalConfigs_.find(region);
    if (it == regionalConfigs_.end()) {
        Logger::warning("Regional configuration not found: " + region);
        return false;
    }
    
    regionalConfigs_.erase(it);
    Logger::info("Regional configuration removed: " + region);
    return true;
}

RegionalConfig CloudConfigManager::getRegionalConfig(const String& region) const {
    auto it = regionalConfigs_.find(region);
    if (it != regionalConfigs_.end()) {
        return it->second;
    }
    return RegionalConfig();
}

std::vector<RegionalConfig> CloudConfigManager::getAllRegionalConfigs() const {
    std::vector<RegionalConfig> configs;
    for (const auto& pair : regionalConfigs_) {
        configs.push_back(pair.second);
    }
    return configs;
}

String CloudConfigManager::selectOptimalRegion(const String& currentLocation) const {
    if (regionalConfigs_.empty()) {
        return "";
    }
    
    String bestRegion;
    float bestScore = -1.0;
    
    for (const auto& pair : regionalConfigs_) {
        const RegionalConfig& config = pair.second;
        if (config.isAvailable) {
            float score = calculateRegionalScore(config, currentLocation);
            if (score > bestScore) {
                bestScore = score;
                bestRegion = config.region;
            }
        }
    }
    
    return bestRegion;
}

bool CloudConfigManager::setEnvironment(EnvironmentType environment) {
    currentEnvironment_.environment = environment;
    
    switch (environment) {
        case ENV_DEVELOPMENT:
            currentEnvironment_.name = "development";
            currentEnvironment_.debugMode = true;
            currentEnvironment_.verboseLogging = true;
            break;
        case ENV_STAGING:
            currentEnvironment_.name = "staging";
            currentEnvironment_.debugMode = false;
            currentEnvironment_.verboseLogging = true;
            break;
        case ENV_PRODUCTION:
            currentEnvironment_.name = "production";
            currentEnvironment_.debugMode = false;
            currentEnvironment_.verboseLogging = false;
            break;
        case ENV_TESTING:
            currentEnvironment_.name = "testing";
            currentEnvironment_.debugMode = true;
            currentEnvironment_.verboseLogging = true;
            break;
    }
    
    Logger::info("Environment changed to: " + currentEnvironment_.name);
    return true;
}

bool CloudConfigManager::addCredential(const CredentialConfig& credential) {
    if (credential.credentialId.isEmpty()) {
        Logger::error("Credential missing ID");
        return false;
    }
    
    CredentialConfig encryptedCredential = credential;
    
    // Encrypt sensitive fields
    if (encryptionEnabled_ && !credential.accessKey.isEmpty()) {
        String encrypted;
        if (encryptValue(credential.accessKey, encrypted)) {
            encryptedCredential.accessKey = encrypted;
        }
    }
    
    if (encryptionEnabled_ && !credential.secretKey.isEmpty()) {
        String encrypted;
        if (encryptValue(credential.secretKey, encrypted)) {
            encryptedCredential.secretKey = encrypted;
        }
    }
    
    credentials_[credential.credentialId] = encryptedCredential;
    Logger::info("Credential added: " + credential.credentialId);
    return true;
}

bool CloudConfigManager::removeCredential(const String& credentialId) {
    auto it = credentials_.find(credentialId);
    if (it == credentials_.end()) {
        Logger::warning("Credential not found: " + credentialId);
        return false;
    }
    
    credentials_.erase(it);
    Logger::info("Credential removed: " + credentialId);
    return true;
}

CredentialConfig CloudConfigManager::getCredential(const String& credentialId) const {
    auto it = credentials_.find(credentialId);
    if (it == credentials_.end()) {
        return CredentialConfig();
    }
    
    CredentialConfig credential = it->second;
    
    // Decrypt sensitive fields
    if (encryptionEnabled_) {
        if (!credential.accessKey.isEmpty()) {
            String decrypted;
            if (decryptValue(credential.accessKey, decrypted)) {
                credential.accessKey = decrypted;
            }
        }
        
        if (!credential.secretKey.isEmpty()) {
            String decrypted;
            if (decryptValue(credential.secretKey, decrypted)) {
                credential.secretKey = decrypted;
            }
        }
    }
    
    return credential;
}

bool CloudConfigManager::encryptSensitiveData(bool enable) {
    encryptionEnabled_ = enable;
    if (enable && encryptionKey_.isEmpty()) {
        encryptionKey_ = generateEncryptionKey();
    }
    Logger::info("Data encryption " + String(enable ? "enabled" : "disabled"));
    return true;
}

bool CloudConfigManager::setEncryptionKey(const String& key) {
    if (key.length() < 16) {
        Logger::error("Encryption key too short (minimum 16 characters)");
        return false;
    }
    
    encryptionKey_ = key;
    Logger::info("Encryption key updated");
    return true;
}

ValidationResult CloudConfigManager::validateConfiguration() const {
    ValidationResult result;
    
    // Validate all configuration entries
    for (const auto& pair : configurations_) {
        ValidationResult entryResult = validateConfigEntry(pair.second);
        result.isValid = result.isValid && entryResult.isValid;
        
        // Merge errors and warnings
        for (const String& error : entryResult.errors) {
            result.errors.push_back(error);
        }
        for (const String& warning : entryResult.warnings) {
            result.warnings.push_back(warning);
        }
    }
    
    // Validate credentials expiry
    for (const auto& pair : credentials_) {
        if (isCredentialExpired(pair.first)) {
            result.warnings.push_back("Credential expired: " + pair.first);
        }
    }
    
    if (!result.isValid && validationErrorCallback_) {
        validationErrorCallback_(result);
    }
    
    return result;
}

ValidationResult CloudConfigManager::validateConfigEntry(const ConfigEntry& entry) const {
    ValidationResult result;
    
    if (entry.key.isEmpty()) {
        result.isValid = false;
        result.errors.push_back("Empty configuration key");
        return result;
    }
    
    // Type-specific validation
    if (!validateConfigValue(entry.key, entry.value, entry.type)) {
        result.isValid = false;
        result.errors.push_back("Invalid value for key: " + entry.key);
    }
    
    // Check validation rules
    auto ruleIt = validationRules_.find(entry.key);
    if (ruleIt != validationRules_.end()) {
        // Apply custom validation rules (simplified implementation)
        // In a full implementation, this would parse and apply complex rules
        if (entry.value.isEmpty()) {
            result.warnings.push_back("Empty value for key: " + entry.key);
        }
    }
    
    return result;
}

String CloudConfigManager::exportConfigurationToJson(ConfigScope scope) const {
    DynamicJsonDocument doc(4096);
    JsonObject root = doc.to<JsonObject>();
    
    String scopePrefix = buildConfigKey("", scope);
    
    for (const auto& pair : configurations_) {
        if (scope == SCOPE_GLOBAL || pair.first.startsWith(scopePrefix)) {
            JsonObject entryObj = root.createNestedObject(pair.first);
            serializeConfigEntry(pair.second, entryObj);
        }
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool CloudConfigManager::importConfigurationFromJson(const String& jsonData, bool overwrite) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        Logger::error("Failed to parse configuration JSON: " + String(error.c_str()));
        return false;
    }
    
    JsonObject root = doc.as<JsonObject>();
    
    for (JsonPair pair : root) {
        String key = pair.key().c_str();
        
        if (!overwrite && hasConfig(key)) {
            Logger::warning("Skipping existing configuration: " + key);
            continue;
        }
        
        JsonObject entryObj = pair.value().as<JsonObject>();
        ConfigEntry entry;
        
        if (deserializeConfigEntry(entryObj, entry)) {
            String configKey = buildConfigKey(entry.key, entry.scope);
            configurations_[configKey] = entry;
        }
    }
    
    Logger::info("Configuration imported from JSON");
    return true;
}

// Private helper methods implementation

String CloudConfigManager::buildConfigKey(const String& key, ConfigScope scope) const {
    String prefix;
    switch (scope) {
        case SCOPE_GLOBAL: prefix = "global."; break;
        case SCOPE_SERVICE: prefix = "service."; break;
        case SCOPE_PROVIDER: prefix = "provider."; break;
        case SCOPE_REGIONAL: prefix = "regional."; break;
        case SCOPE_ENVIRONMENT: prefix = "env."; break;
        case SCOPE_SECURITY: prefix = "security."; break;
    }
    return prefix + key;
}

bool CloudConfigManager::storeConfigEntry(const String& key, const String& value, ConfigType type, ConfigScope scope) {
    if (key.isEmpty()) {
        Logger::error("Configuration key cannot be empty");
        return false;
    }
    
    if (!validateConfigValue(key, value, type)) {
        Logger::error("Invalid configuration value for key: " + key);
        return false;
    }
    
    String configKey = buildConfigKey(key, scope);
    
    // Store old value for change notification
    String oldValue;
    bool hasOldValue = false;
    auto it = configurations_.find(configKey);
    if (it != configurations_.end()) {
        oldValue = it->second.value;
        hasOldValue = true;
    }
    
    // Create configuration entry
    ConfigEntry entry;
    entry.key = key;
    entry.value = value;
    entry.type = type;
    entry.scope = scope;
    entry.isEncrypted = shouldEncryptKey(key);
    entry.lastModified = millis() / 1000;
    
    // Encrypt if needed
    if (entry.isEncrypted && encryptionEnabled_) {
        String encrypted;
        if (encryptValue(value, encrypted)) {
            entry.value = encrypted;
        }
    }
    
    configurations_[configKey] = entry;
    
    // Notify change
    if (hasOldValue && oldValue != value) {
        notifyConfigChange(key, oldValue, value, scope);
    }
    
    Logger::debug("Configuration set: " + key + " = " + value);
    return true;
}

ConfigEntry CloudConfigManager::getConfigEntry(const String& key, ConfigScope scope) const {
    String configKey = buildConfigKey(key, scope);
    auto it = configurations_.find(configKey);
    
    if (it != configurations_.end()) {
        return it->second;
    }
    
    return ConfigEntry();
}

bool CloudConfigManager::validateConfigValue(const String& key, const String& value, ConfigType type) const {
    switch (type) {
        case CONFIG_INTEGER:
            // Check if value is a valid integer
            return value.toInt() != 0 || value == "0";
            
        case CONFIG_FLOAT:
            // Check if value is a valid float
            return value.toFloat() != 0.0 || value == "0.0" || value == "0";
            
        case CONFIG_BOOLEAN:
            // Check if value is a valid boolean
            return value.equalsIgnoreCase("true") || value.equalsIgnoreCase("false") || 
                   value == "1" || value == "0";
            
        case CONFIG_JSON:
            // Validate JSON format
            {
                DynamicJsonDocument doc(1024);
                DeserializationError error = deserializeJson(doc, value);
                return error == DeserializationError::Ok;
            }
            
        default:
            return true; // String and other types are always valid
    }
}

bool CloudConfigManager::shouldEncryptKey(const String& key) const {
    return key.indexOf("key") >= 0 || 
           key.indexOf("secret") >= 0 || 
           key.indexOf("password") >= 0 ||
           key.indexOf("token") >= 0 ||
           key.indexOf("credential") >= 0;
}

String CloudConfigManager::generateEncryptionKey() const {
    // Simple key generation (in production, use cryptographically secure method)
    String key = "WildlifeCam_";
    key += String(millis());
    key += "_EncryptionKey_";
    key += String(random(10000, 99999));
    
    while (key.length() < 32) {
        key += String(random(10, 99));
    }
    
    return key.substring(0, 32);
}

bool CloudConfigManager::encryptValue(const String& value, String& encrypted) const {
    if (encryptionKey_.isEmpty()) {
        Logger::error("Encryption key not set");
        return false;
    }
    
    // Simple XOR encryption (in production, use AES or similar)
    encrypted = "";
    for (int i = 0; i < value.length(); i++) {
        char c = value.charAt(i) ^ encryptionKey_.charAt(i % encryptionKey_.length());
        encrypted += String((int)c, HEX);
    }
    
    return true;
}

bool CloudConfigManager::decryptValue(const String& encrypted, String& value) const {
    if (encryptionKey_.isEmpty()) {
        Logger::error("Encryption key not set");
        return false;
    }
    
    // Reverse XOR encryption
    value = "";
    for (int i = 0; i < encrypted.length(); i += 2) {
        String hexByte = encrypted.substring(i, i + 2);
        int byteValue = strtol(hexByte.c_str(), NULL, 16);
        char c = byteValue ^ encryptionKey_.charAt((i / 2) % encryptionKey_.length());
        value += c;
    }
    
    return true;
}

float CloudConfigManager::calculateRegionalScore(const RegionalConfig& regional, const String& currentLocation) const {
    // Simple scoring based on latency and cost (in production, use actual geolocation)
    float score = 1.0 / regional.latencyThreshold; // Lower latency = higher score
    score = score / regional.costMultiplier; // Lower cost = higher score
    
    // Add location-based scoring if available
    if (!currentLocation.isEmpty() && regional.region.indexOf(currentLocation) >= 0) {
        score *= 1.5; // Boost for regional match
    }
    
    return score;
}

void CloudConfigManager::notifyConfigChange(const String& key, const String& oldValue, const String& newValue, ConfigScope scope) {
    if (configChangeCallback_) {
        configChangeCallback_(key, oldValue, newValue, scope);
    }
    
    Logger::info("Configuration changed: " + key + " from '" + oldValue + "' to '" + newValue + "'");
}

bool CloudConfigManager::writeConfigFile(const String& filePath, const String& data) const {
    // Ensure directory exists
    String dir = filePath.substring(0, filePath.lastIndexOf('/'));
    if (!SPIFFS.exists(dir)) {
        // Create directory (SPIFFS auto-creates)
    }
    
    File file = SPIFFS.open(filePath, "w");
    if (!file) {
        Logger::error("Failed to open file for writing: " + filePath);
        return false;
    }
    
    file.print(data);
    file.close();
    
    return true;
}

bool CloudConfigManager::readConfigFile(const String& filePath, String& data) const {
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        Logger::error("Failed to open file for reading: " + filePath);
        return false;
    }
    
    data = file.readString();
    file.close();
    
    return true;
}

bool CloudConfigManager::serializeConfigEntry(const ConfigEntry& entry, JsonObject& obj) const {
    obj["key"] = entry.key;
    obj["value"] = entry.value;
    obj["type"] = static_cast<int>(entry.type);
    obj["scope"] = static_cast<int>(entry.scope);
    obj["encrypted"] = entry.isEncrypted;
    obj["readonly"] = entry.isReadOnly;
    obj["modified"] = entry.lastModified;
    obj["description"] = entry.description;
    
    return true;
}

bool CloudConfigManager::deserializeConfigEntry(const JsonObject& obj, ConfigEntry& entry) const {
    if (!obj.containsKey("key") || !obj.containsKey("value")) {
        return false;
    }
    
    entry.key = obj["key"].as<String>();
    entry.value = obj["value"].as<String>();
    entry.type = static_cast<ConfigType>(obj["type"].as<int>());
    entry.scope = static_cast<ConfigScope>(obj["scope"].as<int>());
    entry.isEncrypted = obj["encrypted"].as<bool>();
    entry.isReadOnly = obj["readonly"].as<bool>();
    entry.lastModified = obj["modified"].as<uint32_t>();
    entry.description = obj["description"].as<String>();
    
    return true;
}

// Global utility functions implementation

bool initializeCloudConfigManager() {
    if (g_cloudConfigManager != nullptr) {
        Logger::warning("Cloud Configuration Manager already exists");
        return true;
    }
    
    g_cloudConfigManager = new CloudConfigManager();
    if (!g_cloudConfigManager->initialize()) {
        delete g_cloudConfigManager;
        g_cloudConfigManager = nullptr;
        return false;
    }
    
    return true;
}

bool loadCloudConfiguration(const String& configPath) {
    if (g_cloudConfigManager == nullptr) {
        Logger::error("Cloud Configuration Manager not initialized");
        return false;
    }
    
    return g_cloudConfigManager->loadConfiguration(configPath);
}

String getCloudConfig(const String& key, const String& defaultValue) {
    if (g_cloudConfigManager == nullptr) {
        Logger::error("Cloud Configuration Manager not initialized");
        return defaultValue;
    }
    
    return g_cloudConfigManager->getConfigString(key, defaultValue);
}

bool setCloudConfig(const String& key, const String& value) {
    if (g_cloudConfigManager == nullptr) {
        Logger::error("Cloud Configuration Manager not initialized");
        return false;
    }
    
    return g_cloudConfigManager->setConfig(key, value);
}

bool isCloudConfigured() {
    if (g_cloudConfigManager == nullptr) {
        return false;
    }
    
    return g_cloudConfigManager->isInitialized();
}

void cleanupCloudConfigManager() {
    if (g_cloudConfigManager != nullptr) {
        g_cloudConfigManager->cleanup();
        delete g_cloudConfigManager;
        g_cloudConfigManager = nullptr;
    }
}