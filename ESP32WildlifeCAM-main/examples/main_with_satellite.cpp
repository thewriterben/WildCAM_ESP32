/**
 * @file main_with_satellite.cpp
 * @brief Example integration of satellite communication in wildlife camera main loop
 * 
 * This example shows how to integrate satellite communication capabilities
 * into the main ESP32 Wildlife Camera application.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>

// Include wildlife camera core components
#include "config.h"
#include "camera_handler.h"
#include "motion_filter.h"
#include "power_manager.h"

// Include satellite communication integration
#include "satellite_integration.h"

// Global objects
PowerManager powerManager;
CameraHandler cameraHandler;
MotionFilter motionFilter;

// Timing variables
unsigned long lastStatusUpdate = 0;
unsigned long lastConfigCheck = 0;
unsigned long lastPowerCheck = 0;

// Configuration
const unsigned long STATUS_UPDATE_INTERVAL = 3600000;  // 1 hour
const unsigned long CONFIG_CHECK_INTERVAL = 3600000;   // 1 hour  
const unsigned long POWER_CHECK_INTERVAL = 60000;      // 1 minute

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera with Satellite Communication");
    
    // Initialize core systems
    powerManager.initialize();
    cameraHandler.initialize();
    motionFilter.initialize();
    
    // Initialize satellite communication
    if (SATELLITE_INIT()) {
        Serial.println("Satellite communication initialized successfully");
        
        // Configure for remote deployment
        satelliteIntegration.configureForEnvironment(true); // Remote location
    } else {
        Serial.println("Satellite communication initialization failed - continuing without satellite");
    }
    
    Serial.println("System initialization complete");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Handle motion detection and wildlife monitoring
    handleWildlifeMonitoring();
    
    // Handle power management and satellite power optimization
    handlePowerManagement(currentTime);
    
    // Handle satellite communication tasks
    handleSatelliteCommunication(currentTime);
    
    // Handle system maintenance
    handleSystemMaintenance(currentTime);
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}

void handleWildlifeMonitoring() {
    // Check for motion detection
    if (motionFilter.isMotionDetected()) {
        Serial.println("Motion detected - capturing image");
        
        // Capture image
        camera_fb_t* frameBuffer = cameraHandler.captureImage();
        if (frameBuffer) {
            // Process image for wildlife detection (simplified)
            String detectedSpecies = processImageForWildlife(frameBuffer);
            float confidence = calculateDetectionConfidence(frameBuffer);
            
            // Send satellite alert if wildlife detected with high confidence
            if (confidence > 0.7 && detectedSpecies != "NONE") {
                SATELLITE_WILDLIFE_ALERT(detectedSpecies, confidence, frameBuffer->buf, frameBuffer->len);
                
                Serial.printf("Wildlife alert sent: %s (confidence: %.2f)\n", 
                             detectedSpecies.c_str(), confidence);
            }
            
            // Release frame buffer
            esp_camera_fb_return(frameBuffer);
        }
        
        // Reset motion detection
        motionFilter.resetMotionDetection();
    }
}

void handlePowerManagement(unsigned long currentTime) {
    if (currentTime - lastPowerCheck > POWER_CHECK_INTERVAL) {
        // Get current power status
        uint8_t batteryLevel = powerManager.getBatteryPercentage();
        bool isCharging = powerManager.isCharging();
        
        // Update satellite communication power management
        SATELLITE_POWER_UPDATE(batteryLevel, isCharging);
        
        // Handle emergency low battery condition
        if (batteryLevel < 15 && !isCharging) {
            SATELLITE_EMERGENCY("CRITICAL_BATTERY:" + String(batteryLevel) + "%");
            
            // Enter power saving mode
            satelliteIntegration.enterLowPowerMode();
            powerManager.enterLowPowerMode();
            
            Serial.println("Entering emergency power saving mode");
        }
        
        // Handle system recovery from low power
        if (batteryLevel > 30 && isCharging) {
            satelliteIntegration.exitLowPowerMode();
            powerManager.exitLowPowerMode();
        }
        
        lastPowerCheck = currentTime;
    }
}

void handleSatelliteCommunication(unsigned long currentTime) {
    // Send periodic status updates
    if (currentTime - lastStatusUpdate > STATUS_UPDATE_INTERVAL) {
        satelliteIntegration.sendPeriodicStatusUpdate();
        lastStatusUpdate = currentTime;
        
        Serial.printf("Status update sent. Cost today: $%.2f, Pending messages: %d\n",
                     satelliteIntegration.getTodaySatelliteCost(),
                     satelliteIntegration.getPendingMessageCount());
    }
    
    // Check for remote configuration updates
    if (currentTime - lastConfigCheck > CONFIG_CHECK_INTERVAL) {
        satelliteIntegration.checkForRemoteConfigUpdates();
        lastConfigCheck = currentTime;
    }
}

void handleSystemMaintenance(unsigned long currentTime) {
    // Handle system errors and diagnostics
    if (powerManager.hasErrors()) {
        String errorMessage = "SYSTEM_ERROR:" + powerManager.getErrorDescription();
        SATELLITE_EMERGENCY(errorMessage);
        
        Serial.println("System error reported via satellite");
    }
    
    // Handle storage management
    if (cameraHandler.isStorageFull()) {
        SATELLITE_EMERGENCY("STORAGE_FULL");
        
        // Clear old files or implement storage management
        cameraHandler.cleanupOldFiles();
    }
    
    // Handle temperature monitoring
    float temperature = powerManager.getTemperature();
    if (temperature > 70.0 || temperature < -20.0) {
        SATELLITE_EMERGENCY("TEMP_ALERT:" + String(temperature, 1) + "C");
    }
}

String processImageForWildlife(camera_fb_t* frameBuffer) {
    // Simplified wildlife detection (replace with actual AI/ML processing)
    // This would typically use TensorFlow Lite or other ML framework
    
    // For demonstration, return random species based on image characteristics
    if (frameBuffer->len > 50000) {
        // Larger images might indicate larger animals
        return "DEER";
    } else if (frameBuffer->len > 30000) {
        return "RABBIT";
    } else if (frameBuffer->len > 20000) {
        return "BIRD";
    }
    
    return "NONE";
}

float calculateDetectionConfidence(camera_fb_t* frameBuffer) {
    // Simplified confidence calculation (replace with actual AI processing)
    // This would typically be output from ML model
    
    // For demonstration, base confidence on image size and quality
    float confidence = 0.5;
    
    if (frameBuffer->len > 40000) {
        confidence += 0.3;
    }
    
    // Add some randomness to simulate real detection
    confidence += (random(0, 20) / 100.0);
    
    return min(confidence, 1.0);
}

// Example satellite configuration based on deployment scenario
void configureSatelliteForDeployment() {
    SatelliteConfig config;
    
    // Get current configuration
    config = satelliteIntegration.getConfiguration();
    
    // Modify for specific deployment needs
    if (isRemoteDeployment()) {
        // Remote area - optimize for cost and power
        config.transmissionInterval = 7200;      // 2 hours
        config.maxDailyMessages = 12;            // Limit messages
        config.maxDailyCost = 15.0;              // Lower budget
        config.emergencyThreshold = 85;          // Conservative power
        config.useScheduledTransmission = true;  // Wait for optimal windows
    } else {
        // Accessible area - more frequent updates
        config.transmissionInterval = 1800;      // 30 minutes  
        config.maxDailyMessages = 48;            // More messages
        config.maxDailyCost = 25.0;              // Higher budget
        config.emergencyThreshold = 90;          // Normal power
    }
    
    // Apply configuration
    satelliteIntegration.updateSatelliteConfig(config);
}

bool isRemoteDeployment() {
    // Determine if this is a remote deployment based on various factors
    // This could check GPS location, network availability, etc.
    
    // For example, check if WiFi networks are available
    int networkCount = WiFi.scanNetworks();
    return networkCount == 0; // No WiFi = remote location
}

// Example usage in setup() for different deployment scenarios:
void exampleDeploymentSetup() {
    // Example 1: Research station deployment
    if (DEPLOYMENT_TYPE == "RESEARCH") {
        SatelliteConfig researchConfig;
        researchConfig.enabled = true;
        researchConfig.module = MODULE_SWARM;     // Lower cost
        researchConfig.transmissionInterval = 3600;
        researchConfig.maxDailyMessages = 24;
        researchConfig.costOptimization = true;
        satelliteIntegration.updateSatelliteConfig(researchConfig);
    }
    
    // Example 2: Emergency monitoring deployment  
    if (DEPLOYMENT_TYPE == "EMERGENCY") {
        SatelliteConfig emergencyConfig;
        emergencyConfig.enabled = true;
        emergencyConfig.module = MODULE_IRIDIUM;  // More reliable
        emergencyConfig.transmissionInterval = 900; // 15 minutes
        emergencyConfig.maxDailyMessages = 96;
        emergencyConfig.prioritizeEmergency = true;
        satelliteIntegration.updateSatelliteConfig(emergencyConfig);
    }
    
    // Example 3: Educational deployment
    if (DEPLOYMENT_TYPE == "EDUCATION") {
        SatelliteConfig eduConfig;
        eduConfig.enabled = false; // Satellite disabled for cost
        // Use only WiFi/LoRa for educational deployments
        satelliteIntegration.updateSatelliteConfig(eduConfig);
    }
}