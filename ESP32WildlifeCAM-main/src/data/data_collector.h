#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include "../include/config.h"
#include "../ai/wildlife_classifier.h"
#include "../power/power_manager.h"

/**
 * @brief Intelligent data collection and organization system
 * 
 * Manages data collection, metadata generation, species-based organization,
 * and storage optimization for wildlife monitoring.
 */
class DataCollector {
public:
    /**
     * @brief Data collection event types
     */
    enum class EventType {
        MOTION_TRIGGER = 0,
        AI_CLASSIFICATION = 1,
        SCHEDULED_CAPTURE = 2,
        MANUAL_TRIGGER = 3,
        SYSTEM_EVENT = 4
    };

    /**
     * @brief Data collection result
     */
    struct CollectionResult {
        bool success;
        String imageFilename;
        String metadataFilename;
        size_t imageSize;
        size_t metadataSize;
        EventType eventType;
        uint32_t processingTime;
    };

    /**
     * @brief Data collection statistics
     */
    struct CollectionStats {
        uint32_t totalCollections;
        uint32_t successfulCollections;
        uint32_t failedCollections;
        uint64_t totalDataSize;
        uint32_t speciesCounts[51];
        uint32_t eventTypeCounts[5];
        float successRate;
        uint32_t averageProcessingTime;
    };

    /**
     * @brief Metadata structure for collected data
     */
    struct DataMetadata {
        // Timestamp and identification
        uint32_t timestamp;
        String dateTime;
        String filename;
        EventType eventType;
        
        // Environmental data
        float batteryVoltage;
        float batteryPercentage;
        float solarVoltage;
        bool isCharging;
        float temperature;
        float humidity;
        float pressure;
        
        // Motion detection data
        bool motionDetected;
        float motionLevel;
        String motionSource; // PIR, Frame, Hybrid
        
        // AI classification data
        WildlifeClassifier::SpeciesType species;
        float speciesConfidence;
        String speciesName;
        uint8_t animalCount;
        bool isDangerous;
        
        // Image data
        size_t imageSize;
        uint16_t imageWidth;
        uint16_t imageHeight;
        uint8_t jpegQuality;
        
        // System data
        uint32_t uptime;
        uint8_t cpuFrequency;
        size_t freeHeap;
        PowerManager::PowerState powerState;
    };

    /**
     * @brief Initialize data collector
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Collect data from motion event
     * @param imageData Image data buffer
     * @param imageSize Image size
     * @param motionLevel Motion detection level
     * @param motionSource Source of motion detection
     * @return Collection result
     */
    CollectionResult collectMotionData(const uint8_t* imageData, size_t imageSize,
                                     float motionLevel, const String& motionSource);

    /**
     * @brief Collect data from AI classification
     * @param imageData Image data buffer
     * @param imageSize Image size
     * @param classificationResult AI classification result
     * @return Collection result
     */
    CollectionResult collectClassificationData(const uint8_t* imageData, size_t imageSize,
                                             const WildlifeClassifier::ClassificationResult& classificationResult);

    /**
     * @brief Collect scheduled data
     * @param imageData Image data buffer
     * @param imageSize Image size
     * @return Collection result
     */
    CollectionResult collectScheduledData(const uint8_t* imageData, size_t imageSize);

    /**
     * @brief Save metadata to JSON file
     * @param metadata Metadata structure
     * @param filename Output filename
     * @return true if successful
     */
    bool saveMetadata(const DataMetadata& metadata, const String& filename);

    /**
     * @brief Organize data by species
     * @param imageFilename Image filename
     * @param species Species type
     * @return true if successful
     */
    bool organizeBySpecies(const String& imageFilename, WildlifeClassifier::SpeciesType species);

    /**
     * @brief Organize data by date
     * @param imageFilename Image filename
     * @return true if successful
     */
    bool organizeByDate(const String& imageFilename);

    /**
     * @brief Get data collection statistics
     * @return Current statistics
     */
    CollectionStats getStatistics() const { return stats; }

    /**
     * @brief Reset collection statistics
     */
    void resetStatistics();

    /**
     * @brief Set species folder organization
     * @param enable Enable species-based organization
     */
    void setSpeciesOrganization(bool enable) { speciesOrganizationEnabled = enable; }

    /**
     * @brief Set time-based folder organization
     * @param enable Enable time-based organization
     */
    void setTimeOrganization(bool enable) { timeOrganizationEnabled = enable; }

    /**
     * @brief Enable or disable data collector
     * @param enable Enable state
     */
    void setEnabled(bool enable) { enabled = enable; }

    /**
     * @brief Check if data collector is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Cleanup data collector resources
     */
    void cleanup();

private:
    bool initialized = false;
    bool enabled = true;
    bool speciesOrganizationEnabled = SPECIES_FOLDERS_ENABLED;
    bool timeOrganizationEnabled = TIME_BASED_FOLDERS;
    
    CollectionStats stats = {};
    uint32_t collectionCounter = 0;

    /**
     * @brief Generate unique filename for data
     * @param prefix Filename prefix
     * @param extension File extension
     * @return Generated filename
     */
    String generateFilename(const String& prefix, const String& extension);

    /**
     * @brief Create metadata structure
     * @param eventType Event type
     * @param imageSize Image size
     * @return Metadata structure
     */
    DataMetadata createMetadata(EventType eventType, size_t imageSize);

    /**
     * @brief Save image data to file
     * @param imageData Image data buffer
     * @param imageSize Image size
     * @param filename Output filename
     * @return true if successful
     */
    bool saveImageData(const uint8_t* imageData, size_t imageSize, const String& filename);

    /**
     * @brief Update collection statistics
     * @param result Collection result
     */
    void updateStatistics(const CollectionResult& result);

    /**
     * @brief Get species folder name
     * @param species Species type
     * @return Folder name
     */
    String getSpeciesFolder(WildlifeClassifier::SpeciesType species);

    /**
     * @brief Get date folder name
     * @return Date-based folder name
     */
    String getDateFolder();

    /**
     * @brief Ensure directory exists
     * @param path Directory path
     * @return true if directory exists or was created
     */
    bool ensureDirectory(const String& path);

    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp
     */
    String getCurrentTimestamp();

    /**
     * @brief Get environmental data (placeholder)
     * @param temperature Output temperature
     * @param humidity Output humidity
     * @param pressure Output pressure
     */
    void getEnvironmentalData(float& temperature, float& humidity, float& pressure);
};

#endif // DATA_COLLECTOR_H