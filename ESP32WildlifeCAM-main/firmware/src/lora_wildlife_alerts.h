/**
 * @file lora_wildlife_alerts.h
 * @brief Wildlife detection alert system via LoRa mesh network
 * 
 * Integrates dangerous species detection with LoRa mesh networking
 * to provide real-time alerts across the camera network.
 */

#ifndef LORA_WILDLIFE_ALERTS_H
#define LORA_WILDLIFE_ALERTS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "lora_mesh.h"
#include "../../src/ai/dangerous_species_alert.h"

/**
 * @brief Alert transmission priority
 */
enum class AlertTransmissionPriority {
    LOW = 0,        // Standard transmission, can be queued
    NORMAL = 1,     // Normal priority, transmit soon
    HIGH = 2,       // High priority, transmit immediately
    CRITICAL = 3    // Critical priority, bypass queue
};

/**
 * @brief Alert transmission status
 */
enum class AlertTransmissionStatus {
    PENDING = 0,
    TRANSMITTING = 1,
    TRANSMITTED = 2,
    FAILED = 3,
    ACKNOWLEDGED = 4
};

/**
 * @brief Wildlife alert message structure for LoRa transmission
 */
struct WildlifeAlertMessage {
    uint32_t alertId;               // Unique alert identifier
    uint32_t sourceNodeId;          // Node that detected the species
    uint32_t timestamp;             // Detection timestamp
    
    // Species information
    uint8_t speciesType;            // Species enum value
    char speciesName[32];           // Species name
    float confidence;               // Detection confidence
    
    // Alert information
    uint8_t alertPriority;          // Alert priority level
    bool requiresImmediate;         // Immediate notification flag
    
    // Location (if available)
    float latitude;
    float longitude;
    
    // Transmission metadata
    uint8_t transmissionPriority;
    uint8_t transmissionStatus;
    uint8_t retryCount;
    uint32_t lastTransmitTime;
};

/**
 * @brief Wildlife alert transmission statistics
 */
struct WildlifeAlertStats {
    uint32_t totalAlertsSent;
    uint32_t criticalAlertsSent;
    uint32_t successfulTransmissions;
    uint32_t failedTransmissions;
    uint32_t averageTransmitTime;
    uint32_t alertsInQueue;
};

namespace LoRaWildlifeAlerts {
    /**
     * @brief Initialize wildlife alert system
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * @brief Send wildlife alert over LoRa mesh
     * @param alert Dangerous species alert to transmit
     * @return true if alert queued/sent successfully
     */
    bool sendAlert(const DangerousSpeciesAlert& alert);
    
    /**
     * @brief Process incoming wildlife alerts
     * Called by LoRa mesh receive handler
     */
    void processIncomingAlert(const String& message);
    
    /**
     * @brief Set GPS coordinates for this node
     * @param lat Latitude
     * @param lon Longitude
     */
    void setLocation(float lat, float lon);
    
    /**
     * @brief Get alert transmission statistics
     * @return Alert statistics
     */
    WildlifeAlertStats getStatistics();
    
    /**
     * @brief Set alert callback for received alerts
     * @param callback Function to call when alert received
     */
    void setAlertCallback(void (*callback)(const WildlifeAlertMessage&));
    
    /**
     * @brief Force transmission of pending alerts
     * Useful for critical situations
     */
    void flushPendingAlerts();
    
    /**
     * @brief Enable or disable alert system
     * @param enable Enable state
     */
    void setEnabled(bool enable);
    
    /**
     * @brief Check if alert system is enabled
     * @return true if enabled
     */
    bool isEnabled();
    
    /**
     * @brief Cleanup alert system resources
     */
    void cleanup();
}

#endif // LORA_WILDLIFE_ALERTS_H
