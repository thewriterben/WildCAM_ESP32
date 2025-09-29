/**
 * @file migration_tracking_example.cpp
 * @brief Example implementation for raptor migration route tracking
 * 
 * Demonstrates multi-camera mesh network coordination for documenting
 * seasonal migration patterns along known flyways with synchronized capture.
 */

#include "../../firmware/src/scenarios/raptor_monitoring.h"
#include "../../firmware/src/scenarios/raptor_configs.h"

// Global system instance
RaptorMonitoringSystem raptorSystem;

/**
 * Setup function for migration tracking scenario
 */
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Raptor Migration Tracking");
    Serial.println("Multi-camera coordination for flyway documentation");
    
    // Initialize raptor monitoring system for migration scenario
    if (!raptorSystem.init(RaptorScenario::MIGRATION_TRACKING)) {
        Serial.println("ERROR: Failed to initialize raptor migration tracking");
        return;
    }
    
    // Configure for high-speed burst capture and mesh coordination
    raptorSystem.setDetectionSensitivity(70);  // High sensitivity for distant birds
    raptorSystem.enableAcousticMonitoring(true);
    raptorSystem.enableMeshCoordination(true); // Enable mesh for multi-camera sync
    
    // Start monitoring session (24 hours for daily migration pattern)
    if (raptorSystem.startSession(24)) {
        Serial.println("Migration tracking session started successfully");
        Serial.println("Multi-camera mesh coordination enabled");
        Serial.println("High-speed burst mode active");
    } else {
        Serial.println("ERROR: Failed to start migration tracking session");
    }
    
    // Display mesh network status
    displayMeshStatus();
}

/**
 * Main loop for migration tracking
 */
void loop() {
    // Update system state
    raptorSystem.update();
    
    // Check if system is active and operational
    if (raptorSystem.isActive()) {
        // Get current session statistics
        RaptorSession session = raptorSystem.getCurrentSession();
        
        // Log migration-specific metrics every 10 minutes
        static unsigned long lastReport = 0;
        if (millis() - lastReport > 600000) { // 10 minutes
            Serial.println("\n=== Migration Tracking Report ===");
            Serial.printf("Birds counted: %d\n", session.metrics.migration.birdsCount);
            Serial.printf("Flocks detected: %d\n", session.metrics.migration.flocksDetected);
            Serial.printf("Avg flight speed: %.2f m/s\n", session.metrics.migration.averageFlightSpeed_mps);
            Serial.printf("Dominant direction: %.1f degrees\n", session.metrics.migration.dominantDirection_degrees);
            Serial.printf("Network uptime: %.1f%%\n", session.networkUptime_percent);
            Serial.println("===============================\n");
            
            lastReport = millis();
        }
        
        // Monitor mesh network status
        static unsigned long lastMeshCheck = 0;
        if (millis() - lastMeshCheck > 30000) { // Check every 30 seconds
            displayMeshStatus();
            lastMeshCheck = millis();
        }
        
        // Check for weather conditions that trigger migration
        checkMigrationTriggers();
        
        // Monitor storage utilization (migration tracking generates lots of data)
        uint8_t storageUsed = raptorSystem.getStorageUtilization();
        if (storageUsed > 85) {
            Serial.printf("WARNING: Storage utilization high: %d%%\n", storageUsed);
        }
    }
    
    // Faster update interval for migration tracking
    delay(2000); // 2 second update interval
}

/**
 * Display mesh network status
 */
void displayMeshStatus() {
    Serial.println("--- Mesh Network Status ---");
    // This would show actual mesh network information
    Serial.println("Node ID: 1");
    Serial.println("Connected nodes: 3");
    Serial.println("Coordinator: Node 2");
    Serial.println("Network health: Good");
    Serial.println("Time sync: Active");
    Serial.println("---------------------------");
}

/**
 * Check for environmental triggers that indicate migration activity
 */
void checkMigrationTriggers() {
    // This would integrate with weather sensors and environmental data
    static bool triggerReported = false;
    
    // Example conditions that trigger increased migration monitoring
    bool barometricChange = false;  // Would check actual barometric pressure
    bool temperatureDrop = false;   // Would check temperature trends
    bool windConditions = false;    // Would check wind speed/direction
    
    if ((barometricChange || temperatureDrop || windConditions) && !triggerReported) {
        Serial.println("MIGRATION TRIGGER DETECTED:");
        if (barometricChange) Serial.println("- Barometric pressure change");
        if (temperatureDrop) Serial.println("- Temperature drop");
        if (windConditions) Serial.println("- Favorable wind conditions");
        Serial.println("Increasing monitoring intensity");
        
        // Increase sensitivity and capture rate
        raptorSystem.setDetectionSensitivity(85);
        triggerReported = true;
    }
}

/**
 * Configuration helper for migration tracking
 */
void configureMigrationOptimization() {
    Serial.println("Applying migration-specific optimizations:");
    Serial.println("- High-speed burst capture enabled");
    Serial.println("- Multi-camera mesh coordination");
    Serial.println("- Weather data integration");
    Serial.println("- Flock detection algorithms");
    Serial.println("- Species identification for flyways");
    Serial.println("- Real-time data transmission");
}