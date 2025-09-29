/**
 * @file config_manager.cpp
 * @brief Configuration Manager Implementation
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include "config_manager.h"

ConfigManager::ConfigManager()
    : initialized_(false)
    , lastAutoSave_(0)
    , lastBackup_(0)
    , loadCount_(0)
    , saveCount_(0)
    , backupCount_(0)
    , errorCount_(0) {
}

ConfigManager::~ConfigManager() {
    // Save any pending changes
    if (initialized_) {
        performAutoSave();
    }
}

bool ConfigManager::initialize(const ManagerConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    
    // Initialize storage backends
    if (!isStorageAvailable(config_.primaryBackend)) {
        Serial.printf("ConfigManager: Primary storage backend not available\n");
        errorCount_++;
        return false;
    }
    
    // Load existing configurations
    for (int i = 0; i < 8; i++) { // Number of ConfigSection enum values
        ConfigSection section = static_cast<ConfigSection>(i);
        loadConfig(section);
    }
    
    lastAutoSave_ = millis();
    lastBackup_ = millis();
    initialized_ = true;
    
    Serial.println("ConfigManager: Initialized successfully");
    return true;
}

DynamicJsonDocument ConfigManager::loadConfig(ConfigSection section) {
    loadCount_++;
    
    // Check cache first
    auto cacheIt = configCache_.find(section);
    if (cacheIt != configCache_.end()) {
        return cacheIt->second;
    }
    
    // Load from storage
    String path = getSectionPath(section);
    DynamicJsonDocument config(2048);
    
    if (!loadFromStorage(section, path)) {
        // Load default configuration
        config = getDefaultConfig(section);
        Serial.printf("ConfigManager: Loaded default config for %s\n", getSectionName(section).c_str());
    } else {
        Serial.printf("ConfigManager: Loaded config for %s from %s\n", 
                     getSectionName(section).c_str(), path.c_str());
    }
    
    // Cache the configuration
    configCache_[section] = config;
    lastModified_[section] = millis();
    
    return config;
}

bool ConfigManager::saveConfig(ConfigSection section, const DynamicJsonDocument& config, bool createBackup) {
    saveCount_++;
    
    if (createBackup) {
        this->createBackup();
    }
    
    // Validate configuration
    if (!validateSection(section, config)) {
        Serial.printf("ConfigManager: Invalid configuration for %s\n", getSectionName(section).c_str());
        errorCount_++;
        return false;
    }
    
    String path = getSectionPath(section);
    if (!saveToStorage(section, path, config)) {
        errorCount_++;
        return false;
    }
    
    // Update cache
    configCache_[section] = config;
    lastModified_[section] = millis();
    
    // Trigger change callbacks
    triggerChangeCallbacks(section, "", "");
    
    Serial.printf("ConfigManager: Saved config for %s to %s\n", 
                 getSectionName(section).c_str(), path.c_str());
    
    return true;
}

String ConfigManager::getConfigValue(ConfigSection section, const String& key, const String& defaultValue) {
    DynamicJsonDocument config = loadConfig(section);
    return getNestedValue(config, key, defaultValue);
}

bool ConfigManager::setConfigValue(ConfigSection section, const String& key, const String& value, bool saveImmediately) {
    DynamicJsonDocument config = loadConfig(section);
    setNestedValue(config, key, value);
    
    // Update cache
    configCache_[section] = config;
    lastModified_[section] = millis();
    
    // Trigger callbacks
    triggerChangeCallbacks(section, key, value);
    
    if (saveImmediately) {
        return saveConfig(section, config);
    }
    
    return true;
}

void ConfigManager::registerChangeCallback(ConfigSection section, ConfigChangeCallback callback) {
    changeCallbacks_[section].push_back(callback);
}

bool ConfigManager::createBackup() {
    backupCount_++;
    
    for (const auto& cacheEntry : configCache_) {
        ConfigSection section = cacheEntry.first;
        const DynamicJsonDocument& config = cacheEntry.second;
        
        String backupPath = getBackupPath(section, 0); // Latest backup
        if (!saveToStorage(section, backupPath, config)) {
            errorCount_++;
            Serial.printf("ConfigManager: Failed to backup %s\n", getSectionName(section).c_str());
        }
    }
    
    lastBackup_ = millis();
    Serial.println("ConfigManager: Backup created successfully");
    return true;
}

String ConfigManager::exportConfig(ConfigSection section) {
    if (section == ConfigSection::SYSTEM_SETTINGS) {
        // Export all configurations
        DynamicJsonDocument allConfigs(8192);
        
        for (const auto& cacheEntry : configCache_) {
            String sectionName = getSectionName(cacheEntry.first);
            allConfigs[sectionName] = cacheEntry.second;
        }
        
        return jsonToString(allConfigs);
    } else {
        // Export specific section
        DynamicJsonDocument config = loadConfig(section);
        return jsonToString(config);
    }
}

bool ConfigManager::saveLearnedParameters(const String& parameterId, const DynamicJsonDocument& parameters) {
    DynamicJsonDocument learnedConfig = loadConfig(ConfigSection::LEARNED_PATTERNS);
    learnedConfig[parameterId] = parameters;
    
    return saveConfig(ConfigSection::LEARNED_PATTERNS, learnedConfig);
}

DynamicJsonDocument ConfigManager::loadLearnedParameters(const String& parameterId) {
    DynamicJsonDocument learnedConfig = loadConfig(ConfigSection::LEARNED_PATTERNS);
    
    if (learnedConfig.containsKey(parameterId)) {
        return learnedConfig[parameterId];
    }
    
    return DynamicJsonDocument(512); // Return empty document
}

String ConfigManager::getStatistics() const {
    DynamicJsonDocument stats(1024);
    
    stats["loadCount"] = loadCount_;
    stats["saveCount"] = saveCount_;
    stats["backupCount"] = backupCount_;
    stats["errorCount"] = errorCount_;
    stats["cacheSize"] = configCache_.size();
    stats["lastAutoSave"] = lastAutoSave_;
    stats["lastBackup"] = lastBackup_;
    
    JsonObject sections = stats.createNestedObject("sections");
    for (const auto& entry : lastModified_) {
        String sectionName = getSectionName(entry.first);
        sections[sectionName] = entry.second;
    }
    
    return jsonToString(stats);
}

DynamicJsonDocument ConfigManager::getDefaultConfig(ConfigSection section) {
    DynamicJsonDocument config(2048);
    
    switch (section) {
        case ConfigSection::MOTION_COORDINATOR:
            config["enabled"] = true;
            config["defaultMethod"] = "adaptive";
            config["captureThreshold"] = 0.7;
            config["transmitThreshold"] = 0.8;
            break;
            
        case ConfigSection::ADAPTIVE_PROCESSOR:
            config["enabled"] = true;
            config["activityWindowMinutes"] = 60;
            config["batteryLowThreshold"] = 3.2;
            config["maxProcessingTimeMs"] = 500;
            break;
            
        case ConfigSection::WILDLIFE_ANALYZER:
            config["enabled"] = true;
            config["analysisWindowMs"] = 10000;
            config["wildlifeConfidenceThreshold"] = 0.6;
            config["useTimeOfDayAdaptation"] = true;
            break;
            
        case ConfigSection::ENVIRONMENTAL_SENSORS:
            config["enabled"] = true;
            config["temperatureAdjustment"] = true;
            config["lightLevelAdjustment"] = true;
            config["weatherCompensation"] = true;
            break;
            
        case ConfigSection::POWER_MANAGEMENT:
            config["enabled"] = true;
            config["batteryMonitoring"] = true;
            config["lowPowerThreshold"] = 3.2;
            config["criticalPowerThreshold"] = 3.0;
            break;
            
        case ConfigSection::MEMORY_MANAGEMENT:
            config["enabled"] = true;
            config["autoCleanup"] = true;
            config["cleanupThreshold"] = 80.0;
            config["cleanupInterval"] = 30000;
            break;
            
        case ConfigSection::LEARNED_PATTERNS:
            // Empty by default - will be populated by learning
            config.createNestedObject("patterns");
            break;
            
        case ConfigSection::SYSTEM_SETTINGS:
            config["version"] = "2.0";
            config["debugLevel"] = "info";
            config["enableDetailedLogging"] = false;
            break;
    }
    
    return config;
}

// Private method implementations

String ConfigManager::getSectionPath(ConfigSection section) const {
    return config_.configPath + getSectionName(section) + ".json";
}

String ConfigManager::getSectionName(ConfigSection section) const {
    switch (section) {
        case ConfigSection::MOTION_COORDINATOR: return "motion_coordinator";
        case ConfigSection::ADAPTIVE_PROCESSOR: return "adaptive_processor";
        case ConfigSection::WILDLIFE_ANALYZER: return "wildlife_analyzer";
        case ConfigSection::ENVIRONMENTAL_SENSORS: return "environmental_sensors";
        case ConfigSection::POWER_MANAGEMENT: return "power_management";
        case ConfigSection::MEMORY_MANAGEMENT: return "memory_management";
        case ConfigSection::LEARNED_PATTERNS: return "learned_patterns";
        case ConfigSection::SYSTEM_SETTINGS: return "system_settings";
        default: return "unknown";
    }
}

bool ConfigManager::loadFromStorage(ConfigSection section, const String& path) {
    auto& fs = getFileSystem(config_.primaryBackend);
    
    if (!fs.exists(path)) {
        return false;
    }
    
    File file = fs.open(path, "r");
    if (!file) {
        return false;
    }
    
    String jsonString = file.readString();
    file.close();
    
    DynamicJsonDocument config = stringToJson(jsonString);
    if (config.isNull()) {
        return false;
    }
    
    configCache_[section] = config;
    return true;
}

bool ConfigManager::saveToStorage(ConfigSection section, const String& path, const DynamicJsonDocument& config) {
    auto& fs = getFileSystem(config_.primaryBackend);
    
    // Ensure directory exists
    String dir = path.substring(0, path.lastIndexOf('/'));
    if (!fs.exists(dir)) {
        fs.mkdir(dir);
    }
    
    File file = fs.open(path, "w");
    if (!file) {
        return false;
    }
    
    String jsonString = jsonToString(config);
    size_t written = file.print(jsonString);
    file.close();
    
    return written == jsonString.length();
}

void ConfigManager::triggerChangeCallbacks(ConfigSection section, const String& key, const String& value) {
    auto it = changeCallbacks_.find(section);
    if (it != changeCallbacks_.end()) {
        for (const auto& callback : it->second) {
            callback(getSectionName(section), key, value);
        }
    }
}

bool ConfigManager::isStorageAvailable(StorageBackend backend) const {
    switch (backend) {
        case StorageBackend::LITTLEFS:
            return LittleFS.begin();
        case StorageBackend::SD_CARD:
            return SD_MMC.begin();
        case StorageBackend::BOTH:
            return LittleFS.begin() || SD_MMC.begin();
        default:
            return false;
    }
}

fs::FS& ConfigManager::getFileSystem(StorageBackend backend) {
    switch (backend) {
        case StorageBackend::SD_CARD:
            return SD_MMC;
        default:
            return LittleFS;
    }
}

bool ConfigManager::validateSection(ConfigSection section, const DynamicJsonDocument& config) const {
    // Basic validation - check that required keys exist
    switch (section) {
        case ConfigSection::MOTION_COORDINATOR:
            return config.containsKey("enabled") && config.containsKey("defaultMethod");
        case ConfigSection::ADAPTIVE_PROCESSOR:
            return config.containsKey("enabled") && config.containsKey("maxProcessingTimeMs");
        case ConfigSection::WILDLIFE_ANALYZER:
            return config.containsKey("enabled") && config.containsKey("analysisWindowMs");
        default:
            return true; // Allow any configuration for other sections
    }
}

String ConfigManager::jsonToString(const DynamicJsonDocument& doc) const {
    String output;
    serializeJson(doc, output);
    return output;
}

DynamicJsonDocument ConfigManager::stringToJson(const String& jsonString) const {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (error) {
        Serial.printf("ConfigManager: JSON parse error: %s\n", error.c_str());
        return DynamicJsonDocument(512); // Return empty document
    }
    
    return doc;
}

void ConfigManager::setNestedValue(DynamicJsonDocument& doc, const String& key, const String& value) {
    // Simple implementation - doesn't support dot notation yet
    doc[key] = value;
}

String ConfigManager::getNestedValue(const DynamicJsonDocument& doc, const String& key, const String& defaultValue) const {
    // Simple implementation - doesn't support dot notation yet
    if (doc.containsKey(key)) {
        return doc[key].as<String>();
    }
    return defaultValue;
}