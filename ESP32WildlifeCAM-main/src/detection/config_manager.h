/**
 * @file config_manager.h
 * @brief Configuration Manager for Motion Detection System
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <map>
#include <functional>

/**
 * @brief Configuration Manager for persistent configuration storage
 * 
 * Manages configuration persistence, live updates, and learned parameters
 * for the motion detection system.
 */
class ConfigManager {
public:
    /**
     * @brief Configuration change callback type
     */
    typedef std::function<void(const String& section, const String& key, const String& value)> ConfigChangeCallback;

    /**
     * @brief Storage backend types
     */
    enum class StorageBackend {
        LITTLEFS,       // Internal flash storage
        SD_CARD,        // SD card storage  
        BOTH            // Redundant storage on both
    };

    /**
     * @brief Configuration section types
     */
    enum class ConfigSection {
        MOTION_COORDINATOR,
        ADAPTIVE_PROCESSOR,
        WILDLIFE_ANALYZER,
        ENVIRONMENTAL_SENSORS,
        POWER_MANAGEMENT,
        MEMORY_MANAGEMENT,
        LEARNED_PATTERNS,
        SYSTEM_SETTINGS
    };

    /**
     * @brief Configuration manager settings
     */
    struct ManagerConfig {
        StorageBackend primaryBackend = StorageBackend::LITTLEFS;
        StorageBackend backupBackend = StorageBackend::SD_CARD;
        String configPath = "/config/";
        String backupPath = "/backup/";
        bool enableAutoSave = true;
        uint32_t autoSaveIntervalMs = 300000; // 5 minutes
        bool enableBackup = true;
        uint32_t backupIntervalMs = 3600000;  // 1 hour
        bool enableVersioning = true;
        uint32_t maxVersions = 5;
    };

    /**
     * Constructor
     */
    ConfigManager();

    /**
     * Destructor
     */
    ~ConfigManager();

    /**
     * @brief Initialize configuration manager
     * @param config Manager configuration
     * @return true if initialization successful
     */
    bool initialize(const ManagerConfig& config = ManagerConfig());

    /**
     * @brief Load configuration from storage
     * @param section Configuration section to load
     * @return JSON document with configuration
     */
    DynamicJsonDocument loadConfig(ConfigSection section);

    /**
     * @brief Save configuration to storage
     * @param section Configuration section to save
     * @param config JSON configuration document
     * @param createBackup Create backup before saving
     * @return true if save successful
     */
    bool saveConfig(ConfigSection section, const DynamicJsonDocument& config, bool createBackup = false);

    /**
     * @brief Get configuration value
     * @param section Configuration section
     * @param key Configuration key (supports dot notation for nested values)
     * @param defaultValue Default value if key not found
     * @return Configuration value as string
     */
    String getConfigValue(ConfigSection section, const String& key, const String& defaultValue = "");

    /**
     * @brief Set configuration value
     * @param section Configuration section
     * @param key Configuration key (supports dot notation for nested values)
     * @param value Value to set
     * @param saveImmediately Save to storage immediately
     * @return true if value set successfully
     */
    bool setConfigValue(ConfigSection section, const String& key, const String& value, bool saveImmediately = false);

    /**
     * @brief Register callback for configuration changes
     * @param section Section to monitor
     * @param callback Function to call when section changes
     */
    void registerChangeCallback(ConfigSection section, ConfigChangeCallback callback);

    /**
     * @brief Create backup of all configurations
     * @return true if backup successful
     */
    bool createBackup();

    /**
     * @brief Restore configuration from backup
     * @param version Version to restore (0 = latest)
     * @return true if restore successful
     */
    bool restoreFromBackup(uint32_t version = 0);

    /**
     * @brief Export configuration to JSON string
     * @param section Section to export, or all sections if not specified
     * @return JSON string with configuration
     */
    String exportConfig(ConfigSection section = ConfigSection::SYSTEM_SETTINGS);

    /**
     * @brief Import configuration from JSON string
     * @param jsonConfig JSON configuration string
     * @param section Section to import to
     * @return true if import successful
     */
    bool importConfig(const String& jsonConfig, ConfigSection section);

    /**
     * @brief Save learned parameters
     * @param parameterId Unique identifier for the parameter
     * @param parameters JSON document with learned data
     * @return true if save successful
     */
    bool saveLearnedParameters(const String& parameterId, const DynamicJsonDocument& parameters);

    /**
     * @brief Load learned parameters
     * @param parameterId Unique identifier for the parameter
     * @return JSON document with learned data
     */
    DynamicJsonDocument loadLearnedParameters(const String& parameterId);

    /**
     * @brief Get configuration manager statistics
     * @return JSON string with statistics
     */
    String getStatistics() const;

    /**
     * @brief Validate configuration integrity
     * @return true if all configurations are valid
     */
    bool validateIntegrity();

    /**
     * @brief Reset configuration to defaults
     * @param section Section to reset, or all sections if not specified
     * @return true if reset successful
     */
    bool resetToDefaults(ConfigSection section = ConfigSection::SYSTEM_SETTINGS);

    /**
     * @brief Get default configuration for section
     * @param section Configuration section
     * @return JSON document with default configuration
     */
    static DynamicJsonDocument getDefaultConfig(ConfigSection section);

private:
    // Configuration state
    ManagerConfig config_;
    bool initialized_;
    
    // In-memory configuration cache
    std::map<ConfigSection, DynamicJsonDocument> configCache_;
    std::map<ConfigSection, uint32_t> lastModified_;
    
    // Change callbacks
    std::map<ConfigSection, std::vector<ConfigChangeCallback>> changeCallbacks_;
    
    // Auto-save state
    uint32_t lastAutoSave_;
    uint32_t lastBackup_;
    
    // Statistics
    mutable uint32_t loadCount_;
    mutable uint32_t saveCount_;
    mutable uint32_t backupCount_;
    mutable uint32_t errorCount_;
    
    // Helper methods
    String getSectionPath(ConfigSection section) const;
    String getSectionName(ConfigSection section) const;
    String getBackupPath(ConfigSection section, uint32_t version) const;
    
    bool loadFromStorage(ConfigSection section, const String& path);
    bool saveToStorage(ConfigSection section, const String& path, const DynamicJsonDocument& config);
    
    void triggerChangeCallbacks(ConfigSection section, const String& key, const String& value);
    void performAutoSave();
    void performAutoBackup();
    
    bool isStorageAvailable(StorageBackend backend) const;
    fs::FS& getFileSystem(StorageBackend backend);
    
    // Configuration validation
    bool validateSection(ConfigSection section, const DynamicJsonDocument& config) const;
    
    // Utility methods
    String jsonToString(const DynamicJsonDocument& doc) const;
    DynamicJsonDocument stringToJson(const String& jsonString) const;
    void setNestedValue(DynamicJsonDocument& doc, const String& key, const String& value);
    String getNestedValue(const DynamicJsonDocument& doc, const String& key, const String& defaultValue) const;
};

#endif // CONFIG_MANAGER_H