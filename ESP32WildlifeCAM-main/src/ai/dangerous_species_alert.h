/**
 * @file dangerous_species_alert.h
 * @brief Dangerous species detection and alert system
 * 
 * Implements real-time alerts for dangerous wildlife species detected
 * by the AI classification system. Provides enhanced safety features
 * for wildlife monitoring in areas with predators.
 */

#ifndef DANGEROUS_SPECIES_ALERT_H
#define DANGEROUS_SPECIES_ALERT_H

#include <Arduino.h>
#include "wildlife_classifier.h"

/**
 * @brief Alert priority levels for dangerous species
 */
enum class AlertPriority {
    LOW = 0,       // Low priority - non-aggressive species (coyote, bobcat)
    MEDIUM = 1,    // Medium priority - potentially dangerous
    HIGH = 2,      // High priority - dangerous predators (wolf, mountain lion)
    CRITICAL = 3   // Critical priority - large dangerous animals (bear)
};

/**
 * @brief Alert type for dangerous species detection
 */
struct DangerousSpeciesAlert {
    WildlifeClassifier::SpeciesType species;
    String speciesName;
    float confidence;
    AlertPriority priority;
    uint32_t timestamp;
    bool requiresImmediate;     // Requires immediate notification
    String alertMessage;
    uint32_t detectionCount;    // Number of consecutive detections
    bool alertSent;             // Whether alert has been sent
};

/**
 * @brief Dangerous species alert system
 * 
 * Monitors wildlife classifications and generates alerts for dangerous species.
 * Integrates with notification systems (LoRa, WiFi, satellite) to send
 * real-time warnings to field personnel and monitoring stations.
 */
class DangerousSpeciesAlertSystem {
public:
    /**
     * @brief Initialize the alert system
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Process classification result and generate alerts
     * @param result Classification result from wildlife classifier
     * @return Alert structure if dangerous species detected, nullptr otherwise
     */
    DangerousSpeciesAlert* processClassification(const WildlifeClassifier::ClassificationResult& result);
    
    /**
     * @brief Get alert priority for a species
     * @param species Species type
     * @return Alert priority level
     */
    static AlertPriority getAlertPriority(WildlifeClassifier::SpeciesType species);
    
    /**
     * @brief Check if species requires immediate alert
     * @param species Species type
     * @param confidence Detection confidence
     * @return true if immediate alert required
     */
    static bool requiresImmediateAlert(WildlifeClassifier::SpeciesType species, float confidence);
    
    /**
     * @brief Get alert message for species
     * @param species Species type
     * @param confidence Detection confidence
     * @return Alert message string
     */
    static String getAlertMessage(WildlifeClassifier::SpeciesType species, float confidence);
    
    /**
     * @brief Set minimum confidence threshold for alerts
     * @param threshold Confidence threshold (0.0 to 1.0)
     */
    void setConfidenceThreshold(float threshold);
    
    /**
     * @brief Set minimum consecutive detections for alert
     * @param count Number of consecutive detections required
     */
    void setMinConsecutiveDetections(uint32_t count);
    
    /**
     * @brief Get total alerts generated
     * @return Number of alerts
     */
    uint32_t getTotalAlerts() const { return totalAlerts; }
    
    /**
     * @brief Get critical alerts generated
     * @return Number of critical alerts
     */
    uint32_t getCriticalAlerts() const { return criticalAlerts; }
    
    /**
     * @brief Clear alert history
     */
    void clearAlertHistory();
    
    /**
     * @brief Enable or disable the alert system
     * @param enable Enable state
     */
    void setEnabled(bool enable) { enabled = enable; }
    
    /**
     * @brief Check if alert system is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

private:
    bool initialized = false;
    bool enabled = true;
    float confidenceThreshold = 0.70f;
    uint32_t minConsecutiveDetections = 2;
    
    // Statistics
    uint32_t totalAlerts = 0;
    uint32_t criticalAlerts = 0;
    
    // Alert tracking
    WildlifeClassifier::SpeciesType lastDetectedSpecies = WildlifeClassifier::SpeciesType::UNKNOWN;
    uint32_t consecutiveDetections = 0;
    uint32_t lastAlertTime = 0;
    static const uint32_t ALERT_COOLDOWN_MS = 60000; // 60 seconds cooldown between alerts
    
    DangerousSpeciesAlert currentAlert = {};
    
    /**
     * @brief Check if in alert cooldown period
     * @return true if in cooldown
     */
    bool isInCooldown();
    
    /**
     * @brief Update consecutive detection count
     * @param species Currently detected species
     * @return Current consecutive count
     */
    uint32_t updateConsecutiveCount(WildlifeClassifier::SpeciesType species);
    
    /**
     * @brief Generate alert for dangerous species
     * @param result Classification result
     * @return Generated alert
     */
    DangerousSpeciesAlert generateAlert(const WildlifeClassifier::ClassificationResult& result);
};

#endif // DANGEROUS_SPECIES_ALERT_H
