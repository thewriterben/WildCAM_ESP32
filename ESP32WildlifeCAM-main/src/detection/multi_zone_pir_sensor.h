#ifndef MULTI_ZONE_PIR_SENSOR_H
#define MULTI_ZONE_PIR_SENSOR_H

#include <Arduino.h>
#include <vector>
#include "pir_sensor.h"
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief Multi-zone PIR sensor management class
 * 
 * Extends the existing PIR sensor functionality to support multiple
 * PIR sensors for zone-based motion detection.
 */
class MultiZonePIRSensor {
public:
    /**
     * @brief PIR zone configuration
     */
    struct PIRZone {
        uint8_t zoneId;
        uint8_t pin;
        String name;
        float sensitivity;
        bool enabled;
        uint8_t priority;           // Zone priority (0=highest, 255=lowest)
        
        // Zone statistics
        struct {
            uint32_t detectionCount;
            uint32_t falsePositiveCount;
            unsigned long lastDetection;
            float averageConfidence;
            bool currentlyActive;
        } stats;
        
        PIRZone() : zoneId(0), pin(0), sensitivity(0.5f), enabled(true), priority(128) {
            stats = {};
        }
    };

    /**
     * @brief Multi-zone detection result
     */
    struct MultiZoneResult {
        bool motionDetected;
        uint8_t activeZoneCount;
        uint8_t highestPriorityZone;
        float overallConfidence;
        std::vector<uint8_t> activeZones;
        uint32_t processTime;
        String description;
    };

    /**
     * @brief Initialize multi-zone PIR system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Add a PIR zone
     * @param zoneId Zone identifier
     * @param pin GPIO pin for PIR sensor
     * @param name Zone name/description
     * @param sensitivity Zone sensitivity (0.0-1.0)
     * @param priority Zone priority (0=highest)
     * @return true if zone added successfully
     */
    bool addZone(uint8_t zoneId, uint8_t pin, const String& name, 
                 float sensitivity = 0.5f, uint8_t priority = 128);

    /**
     * @brief Remove a PIR zone
     * @param zoneId Zone identifier to remove
     * @return true if zone removed successfully
     */
    bool removeZone(uint8_t zoneId);

    /**
     * @brief Check for motion across all zones
     * @return Multi-zone detection result
     */
    MultiZoneResult detectMotion();

    /**
     * @brief Enable/disable a specific zone
     * @param zoneId Zone identifier
     * @param enabled Enable state
     * @return true if successful
     */
    bool setZoneEnabled(uint8_t zoneId, bool enabled);

    /**
     * @brief Set zone sensitivity
     * @param zoneId Zone identifier
     * @param sensitivity Sensitivity level (0.0-1.0)
     * @return true if successful
     */
    bool setZoneSensitivity(uint8_t zoneId, float sensitivity);

    /**
     * @brief Set zone priority
     * @param zoneId Zone identifier
     * @param priority Priority level (0=highest)
     * @return true if successful
     */
    bool setZonePriority(uint8_t zoneId, uint8_t priority);

    /**
     * @brief Get zone configuration
     * @param zoneId Zone identifier
     * @return Zone configuration or nullptr if not found
     */
    const PIRZone* getZone(uint8_t zoneId) const;

    /**
     * @brief Get all configured zones
     * @return Vector of all zones
     */
    const std::vector<PIRZone>& getAllZones() const { return zones; }

    /**
     * @brief Get zone count
     * @return Number of configured zones
     */
    size_t getZoneCount() const { return zones.size(); }

    /**
     * @brief Reset all zone statistics
     */
    void resetStatistics();

    /**
     * @brief Enable/disable multi-zone system
     * @param enabled Enable state
     */
    void setEnabled(bool enabled) { this->enabled = enabled; }

    /**
     * @brief Check if multi-zone system is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Cleanup multi-zone PIR resources
     */
    void cleanup();

    /**
     * @brief Configure zones for common wildlife monitoring scenarios
     */
    bool configureDefaultZones();

private:
    bool initialized = false;
    bool enabled = true;
    std::vector<PIRZone> zones;
    std::vector<PIRSensor> pirSensors;  // Individual PIR sensor instances
    
    /**
     * @brief Find zone by ID
     * @param zoneId Zone identifier
     * @return Iterator to zone or end() if not found
     */
    std::vector<PIRZone>::iterator findZone(uint8_t zoneId);
    std::vector<PIRZone>::const_iterator findZone(uint8_t zoneId) const;

    /**
     * @brief Initialize PIR sensor for a zone
     * @param zone Zone configuration
     * @return true if successful
     */
    bool initializePIRForZone(const PIRZone& zone);

    /**
     * @brief Calculate overall confidence from zone results
     * @param activeZones List of active zones
     * @return Combined confidence score
     */
    float calculateOverallConfidence(const std::vector<uint8_t>& activeZones);

    /**
     * @brief Update zone statistics
     * @param zoneId Zone that detected motion
     * @param confidence Detection confidence
     */
    void updateZoneStatistics(uint8_t zoneId, float confidence);

    /**
     * @brief Generate result description
     * @param result Detection result
     * @return Human-readable description
     */
    String generateResultDescription(const MultiZoneResult& result);
};

#endif // MULTI_ZONE_PIR_SENSOR_H