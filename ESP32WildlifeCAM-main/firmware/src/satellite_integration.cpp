/**
 * @file satellite_integration.cpp
 * @brief Implementation of satellite communication integration for wildlife camera
 */

#include "satellite_integration.h"
#include <Arduino.h>

// Global instance
WildlifeCameraSatelliteIntegration satelliteIntegration;

WildlifeCameraSatelliteIntegration::WildlifeCameraSatelliteIntegration() :
    lastStatusUpdate(0),
    lastConfigCheck(0),
    emergencyMode(false),
    lowPowerMode(false) {
    
    createDefaultConfig();
}

bool WildlifeCameraSatelliteIntegration::initializeSatelliteComm() {
    Serial.println("Initializing satellite communication...");
    
    // Configure the satellite communication module
    if (!satelliteComm.configure(currentConfig)) {
        Serial.println("Failed to configure satellite communication");
        return false;
    }
    
    // Initialize the specific satellite module
    if (currentConfig.enabled) {
        if (satelliteComm.initialize(currentConfig.module)) {
            Serial.println("Satellite communication initialized successfully");
            logSatelliteActivity("INITIALIZED");
            return true;
        } else {
            Serial.println("Failed to initialize satellite module");
            return false;
        }
    }
    
    Serial.println("Satellite communication disabled in configuration");
    return true;
}

void WildlifeCameraSatelliteIntegration::configureForEnvironment(bool remoteLocation) {
    if (remoteLocation) {
        // Optimize for remote deployment
        currentConfig.enabled = true;
        currentConfig.transmissionInterval = 7200; // 2 hours
        currentConfig.maxDailyMessages = 12;
        currentConfig.costOptimization = true;
        currentConfig.useScheduledTransmission = true;
        currentConfig.emergencyThreshold = 85; // More conservative
    } else {
        // Optimize for accessible locations
        currentConfig.transmissionInterval = 3600; // 1 hour
        currentConfig.maxDailyMessages = 24;
        currentConfig.emergencyThreshold = 90;
    }
    
    satelliteComm.updateConfiguration(currentConfig);
    Serial.printf("Configured for %s deployment\n", remoteLocation ? "remote" : "accessible");
}

void WildlifeCameraSatelliteIntegration::handleWildlifeDetection(const String& species, float confidence, 
                                                                 const uint8_t* imageData, size_t imageSize) {
    if (!currentConfig.enabled) return;
    
    // Create wildlife alert message
    String alertMessage = createWildlifeAlertMessage(species, confidence);
    
    // Determine priority based on species and confidence
    SatelliteMessagePriority priority = SAT_PRIORITY_NORMAL;
    if (confidence > 0.9) {
        priority = SAT_PRIORITY_HIGH;
    }
    if (species.indexOf("ENDANGERED") != -1 || species.indexOf("RARE") != -1) {
        priority = SAT_PRIORITY_HIGH;
    }
    
    // Send the alert
    if (satelliteComm.sendWildlifeAlert(alertMessage)) {
        logSatelliteActivity("WILDLIFE_ALERT_SENT: " + species);
    }
    
    // Optionally send compressed thumbnail for high-confidence detections
    if (confidence > 0.8 && imageData && imageSize > 0) {
        uint8_t thumbnailData[SATELLITE_THUMBNAIL_MAX_SIZE];
        size_t thumbnailSize = dataCompression.optimizeForSatellite(
            imageData, imageSize, thumbnailData, sizeof(thumbnailData)
        );
        
        if (thumbnailSize > 0) {
            satelliteComm.sendThumbnail(thumbnailData, thumbnailSize);
            logSatelliteActivity("THUMBNAIL_SENT");
        }
    }
}

void WildlifeCameraSatelliteIntegration::sendPeriodicStatusUpdate() {
    if (!currentConfig.enabled) return;
    
    unsigned long now = millis();
    if (now - lastStatusUpdate > (currentConfig.transmissionInterval * 1000)) {
        
        if (satelliteComm.sendStatusReport()) {
            logSatelliteActivity("STATUS_UPDATE_SENT");
            lastStatusUpdate = now;
        }
        
        // Also try to send any stored messages
        satelliteComm.sendStoredMessages();
    }
}

void WildlifeCameraSatelliteIntegration::handleEmergencyCondition(const String& condition) {
    if (!currentConfig.enabled) return;
    
    emergencyMode = true;
    satelliteComm.enableEmergencyMode();
    
    String emergencyMessage = "EMERGENCY:" + condition + ",TIME:" + String(millis());
    
    if (satelliteComm.sendEmergencyAlert(emergencyMessage)) {
        logSatelliteActivity("EMERGENCY_SENT: " + condition);
    }
}

void WildlifeCameraSatelliteIntegration::updatePowerStatus(uint8_t batteryLevel, bool isCharging) {
    if (!currentConfig.enabled) return;
    
    // Update satellite communication power optimization
    satelliteComm.optimizePowerConsumption(batteryLevel);
    
    // Handle low battery emergency
    if (batteryLevel < currentConfig.emergencyThreshold && !emergencyMode) {
        handleEmergencyCondition("LOW_BATTERY:" + String(batteryLevel) + "%");
    }
    
    // Handle critical battery level
    if (batteryLevel < 10) {
        satelliteComm.enterSleepMode();
        lowPowerMode = true;
        logSatelliteActivity("LOW_POWER_MODE_ENABLED");
    } else if (lowPowerMode && batteryLevel > 20) {
        satelliteComm.exitSleepMode();
        lowPowerMode = false;
        logSatelliteActivity("LOW_POWER_MODE_DISABLED");
    }
}

void WildlifeCameraSatelliteIntegration::enterLowPowerMode() {
    if (currentConfig.enabled) {
        satelliteComm.enterSleepMode();
        lowPowerMode = true;
        logSatelliteActivity("SLEEP_MODE_ENABLED");
    }
}

void WildlifeCameraSatelliteIntegration::exitLowPowerMode() {
    if (currentConfig.enabled && lowPowerMode) {
        satelliteComm.exitSleepMode();
        lowPowerMode = false;
        logSatelliteActivity("SLEEP_MODE_DISABLED");
    }
}

void WildlifeCameraSatelliteIntegration::updateSatelliteConfig(const SatelliteConfig& newConfig) {
    currentConfig = newConfig;
    satelliteComm.updateConfiguration(currentConfig);
    logSatelliteActivity("CONFIG_UPDATED");
}

void WildlifeCameraSatelliteIntegration::checkForRemoteConfigUpdates() {
    if (!currentConfig.enabled) return;
    
    unsigned long now = millis();
    if (now - lastConfigCheck > 3600000) { // Check hourly
        if (satelliteComm.receiveConfigUpdate()) {
            logSatelliteActivity("REMOTE_CONFIG_RECEIVED");
        }
        lastConfigCheck = now;
    }
}

bool WildlifeCameraSatelliteIntegration::isSatelliteEnabled() const {
    return currentConfig.enabled;
}

float WildlifeCameraSatelliteIntegration::getTodaySatelliteCost() const {
    return satelliteComm.getTodayCost();
}

size_t WildlifeCameraSatelliteIntegration::getPendingMessageCount() const {
    return satelliteComm.getStoredMessageCount();
}

void WildlifeCameraSatelliteIntegration::createDefaultConfig() {
    currentConfig.enabled = false; // Disabled by default
    currentConfig.module = MODULE_SWARM; // Default to Swarm (lower cost)
    currentConfig.transmissionInterval = 3600; // 1 hour
    currentConfig.emergencyInterval = 300; // 5 minutes
    currentConfig.maxDailyMessages = 24;
    currentConfig.emergencyThreshold = 90;
    currentConfig.costOptimization = true;
    currentConfig.autoPassPrediction = true;
    currentConfig.maxDailyCost = 25.0;
    currentConfig.prioritizeEmergency = true;
    currentConfig.enableMessageCompression = true;
    currentConfig.useScheduledTransmission = true;
}

String WildlifeCameraSatelliteIntegration::createWildlifeAlertMessage(const String& species, float confidence) {
    String message = "WILDLIFE:";
    message += "SPECIES=" + species + ",";
    message += "CONFIDENCE=" + String(confidence, 2) + ",";
    message += "TIME=" + String(millis()) + ",";
    message += "BATTERY=" + String(analogRead(A0) * 3.3 / 4095.0, 1) + "V";
    
    return message;
}

bool WildlifeCameraSatelliteIntegration::shouldSendImmediately(const String& messageType) {
    if (messageType.indexOf("EMERGENCY") != -1) return true;
    if (messageType.indexOf("ENDANGERED") != -1) return true;
    if (emergencyMode) return true;
    
    return false;
}

void WildlifeCameraSatelliteIntegration::logSatelliteActivity(const String& activity) {
    Serial.printf("[SAT_INT] %s at %lu ms\n", activity.c_str(), millis());
}