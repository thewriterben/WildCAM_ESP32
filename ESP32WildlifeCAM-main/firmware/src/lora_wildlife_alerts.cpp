/**
 * @file lora_wildlife_alerts.cpp
 * @brief Implementation of wildlife alert system via LoRa mesh
 */

#include "lora_wildlife_alerts.h"
#include "config.h"

namespace LoRaWildlifeAlerts {

// Configuration
static bool initialized = false;
static bool enabled = true;
static uint32_t nextAlertId = 1;

// Location
static float currentLatitude = 0.0f;
static float currentLongitude = 0.0f;

// Statistics
static WildlifeAlertStats stats = {};

// Alert queue
#define ALERT_QUEUE_SIZE 10
static WildlifeAlertMessage alertQueue[ALERT_QUEUE_SIZE];
static uint8_t queueCount = 0;

// Alert callback
static void (*alertCallback)(const WildlifeAlertMessage&) = nullptr;

// Internal functions
static String serializeAlert(const WildlifeAlertMessage& alert);
static WildlifeAlertMessage deserializeAlert(const String& message);
static bool transmitAlert(WildlifeAlertMessage& alert);
static void queueAlert(const WildlifeAlertMessage& alert);
static void processAlertQueue();

/**
 * Initialize wildlife alert system
 */
bool init() {
    if (initialized) {
        return true;
    }
    
    Serial.println("Initializing wildlife alert system...");
    
    // Reset statistics
    memset(&stats, 0, sizeof(stats));
    
    // Clear alert queue
    queueCount = 0;
    
    initialized = true;
    enabled = true;
    
    Serial.println("Wildlife alert system initialized");
    return true;
}

/**
 * Send wildlife alert over LoRa mesh
 */
bool sendAlert(const DangerousSpeciesAlert& alert) {
    if (!initialized || !enabled) {
        return false;
    }
    
    Serial.println("Sending wildlife alert via LoRa mesh...");
    
    // Create alert message
    WildlifeAlertMessage message;
    message.alertId = nextAlertId++;
    message.sourceNodeId = NODE_ID;
    message.timestamp = alert.timestamp;
    
    // Species information
    message.speciesType = static_cast<uint8_t>(alert.species);
    strncpy(message.speciesName, alert.speciesName.c_str(), sizeof(message.speciesName) - 1);
    message.speciesName[sizeof(message.speciesName) - 1] = '\0';
    message.confidence = alert.confidence;
    
    // Alert information
    message.alertPriority = static_cast<uint8_t>(alert.priority);
    message.requiresImmediate = alert.requiresImmediate;
    
    // Location
    message.latitude = currentLatitude;
    message.longitude = currentLongitude;
    
    // Transmission metadata
    switch (alert.priority) {
        case AlertPriority::CRITICAL:
            message.transmissionPriority = static_cast<uint8_t>(AlertTransmissionPriority::CRITICAL);
            break;
        case AlertPriority::HIGH:
            message.transmissionPriority = static_cast<uint8_t>(AlertTransmissionPriority::HIGH);
            break;
        default:
            message.transmissionPriority = static_cast<uint8_t>(AlertTransmissionPriority::NORMAL);
            break;
    }
    
    message.transmissionStatus = static_cast<uint8_t>(AlertTransmissionStatus::PENDING);
    message.retryCount = 0;
    message.lastTransmitTime = 0;
    
    // For critical alerts, try immediate transmission
    if (message.transmissionPriority == static_cast<uint8_t>(AlertTransmissionPriority::CRITICAL)) {
        if (transmitAlert(message)) {
            stats.totalAlertsSent++;
            stats.criticalAlertsSent++;
            stats.successfulTransmissions++;
            Serial.printf("CRITICAL ALERT transmitted immediately: %s\n", alert.alertMessage.c_str());
            return true;
        }
    }
    
    // Queue for transmission
    queueAlert(message);
    stats.totalAlertsSent++;
    if (message.alertPriority == static_cast<uint8_t>(AlertPriority::CRITICAL)) {
        stats.criticalAlertsSent++;
    }
    
    Serial.printf("Wildlife alert queued: %s (ID: %d)\n", alert.speciesName.c_str(), message.alertId);
    return true;
}

/**
 * Process incoming wildlife alerts
 */
void processIncomingAlert(const String& message) {
    if (!initialized || !enabled) {
        return;
    }
    
    Serial.println("Processing incoming wildlife alert...");
    
    WildlifeAlertMessage alert = deserializeAlert(message);
    
    if (alert.alertId > 0) {
        Serial.printf("ALERT RECEIVED: %s detected at node %d (confidence: %.1f%%)\n",
                     alert.speciesName, alert.sourceNodeId, alert.confidence * 100.0f);
        
        // Call user callback if registered
        if (alertCallback) {
            alertCallback(alert);
        }
        
        // If high priority, display warning
        if (alert.alertPriority >= static_cast<uint8_t>(AlertPriority::HIGH)) {
            Serial.println("⚠️  WARNING: Dangerous species detected in area!");
            Serial.printf("Species: %s | Location: %.6f, %.6f\n", 
                         alert.speciesName, alert.latitude, alert.longitude);
        }
        
        // Re-broadcast critical alerts to ensure network coverage
        if (alert.alertPriority == static_cast<uint8_t>(AlertPriority::CRITICAL) && 
            alert.retryCount < 3) {
            alert.retryCount++;
            String rebroadcast = serializeAlert(alert);
            LoraMesh::queueMessage(rebroadcast);
            Serial.println("Re-broadcasting critical alert");
        }
    }
}

/**
 * Set GPS coordinates for this node
 */
void setLocation(float lat, float lon) {
    currentLatitude = lat;
    currentLongitude = lon;
    Serial.printf("Location set: %.6f, %.6f\n", lat, lon);
}

/**
 * Get alert transmission statistics
 */
WildlifeAlertStats getStatistics() {
    stats.alertsInQueue = queueCount;
    return stats;
}

/**
 * Set alert callback
 */
void setAlertCallback(void (*callback)(const WildlifeAlertMessage&)) {
    alertCallback = callback;
    Serial.println("Alert callback registered");
}

/**
 * Force transmission of pending alerts
 */
void flushPendingAlerts() {
    Serial.printf("Flushing %d pending alerts...\n", queueCount);
    
    while (queueCount > 0) {
        if (transmitAlert(alertQueue[0])) {
            // Shift queue
            for (uint8_t i = 1; i < queueCount; i++) {
                alertQueue[i-1] = alertQueue[i];
            }
            queueCount--;
        } else {
            break; // Stop if transmission fails
        }
        delay(100); // Small delay between transmissions
    }
}

/**
 * Enable or disable alert system
 */
void setEnabled(bool enable) {
    enabled = enable;
    Serial.printf("Wildlife alert system %s\n", enable ? "enabled" : "disabled");
}

/**
 * Check if enabled
 */
bool isEnabled() {
    return enabled;
}

/**
 * Cleanup resources
 */
void cleanup() {
    if (initialized) {
        queueCount = 0;
        alertCallback = nullptr;
        initialized = false;
        Serial.println("Wildlife alert system cleaned up");
    }
}

// Internal function implementations

/**
 * Serialize alert to JSON string
 */
static String serializeAlert(const WildlifeAlertMessage& alert) {
    DynamicJsonDocument doc(512);
    
    doc["type"] = "wildlife_alert";
    doc["alert_id"] = alert.alertId;
    doc["node_id"] = alert.sourceNodeId;
    doc["timestamp"] = alert.timestamp;
    
    doc["species_type"] = alert.speciesType;
    doc["species_name"] = alert.speciesName;
    doc["confidence"] = alert.confidence;
    
    doc["priority"] = alert.alertPriority;
    doc["immediate"] = alert.requiresImmediate;
    
    doc["lat"] = alert.latitude;
    doc["lon"] = alert.longitude;
    
    doc["retry"] = alert.retryCount;
    
    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * Deserialize alert from JSON string
 */
static WildlifeAlertMessage deserializeAlert(const String& message) {
    WildlifeAlertMessage alert;
    memset(&alert, 0, sizeof(alert));
    
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.printf("Failed to parse alert: %s\n", error.c_str());
        return alert;
    }
    
    if (doc["type"] != "wildlife_alert") {
        return alert;
    }
    
    alert.alertId = doc["alert_id"] | 0;
    alert.sourceNodeId = doc["node_id"] | 0;
    alert.timestamp = doc["timestamp"] | 0;
    
    alert.speciesType = doc["species_type"] | 0;
    const char* name = doc["species_name"] | "";
    strncpy(alert.speciesName, name, sizeof(alert.speciesName) - 1);
    alert.confidence = doc["confidence"] | 0.0f;
    
    alert.alertPriority = doc["priority"] | 0;
    alert.requiresImmediate = doc["immediate"] | false;
    
    alert.latitude = doc["lat"] | 0.0f;
    alert.longitude = doc["lon"] | 0.0f;
    
    alert.retryCount = doc["retry"] | 0;
    
    return alert;
}

/**
 * Transmit alert via LoRa
 */
static bool transmitAlert(WildlifeAlertMessage& alert) {
    uint32_t startTime = millis();
    
    String message = serializeAlert(alert);
    bool success = LoraMesh::queueMessage(message);
    
    if (success) {
        alert.transmissionStatus = static_cast<uint8_t>(AlertTransmissionStatus::TRANSMITTED);
        alert.lastTransmitTime = millis();
        
        uint32_t transmitTime = millis() - startTime;
        stats.averageTransmitTime = (stats.averageTransmitTime * stats.successfulTransmissions + transmitTime) / 
                                    (stats.successfulTransmissions + 1);
        stats.successfulTransmissions++;
    } else {
        alert.transmissionStatus = static_cast<uint8_t>(AlertTransmissionStatus::FAILED);
        stats.failedTransmissions++;
    }
    
    return success;
}

/**
 * Queue alert for transmission
 */
static void queueAlert(const WildlifeAlertMessage& alert) {
    if (queueCount >= ALERT_QUEUE_SIZE) {
        Serial.println("Alert queue full, dropping oldest alert");
        // Shift queue
        for (uint8_t i = 1; i < queueCount; i++) {
            alertQueue[i-1] = alertQueue[i];
        }
        queueCount--;
    }
    
    alertQueue[queueCount] = alert;
    queueCount++;
}

/**
 * Process alert queue (should be called periodically)
 */
static void processAlertQueue() {
    if (queueCount == 0) {
        return;
    }
    
    // Try to transmit first alert in queue
    if (transmitAlert(alertQueue[0])) {
        // Shift queue
        for (uint8_t i = 1; i < queueCount; i++) {
            alertQueue[i-1] = alertQueue[i];
        }
        queueCount--;
    }
}

} // namespace LoRaWildlifeAlerts
