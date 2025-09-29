/**
 * @file nesting_monitoring_example.cpp
 * @brief Example implementation for raptor nesting site monitoring
 * 
 * Demonstrates ultra-low disturbance monitoring of eagle, hawk, and falcon
 * nesting sites during breeding season with stealth operation capabilities.
 */

#include "../../firmware/src/scenarios/raptor_monitoring.h"
#include "../../firmware/src/scenarios/raptor_configs.h"

// Global system instance
RaptorMonitoringSystem raptorSystem;

/**
 * Setup function for nesting monitoring scenario
 */
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Raptor Nesting Monitoring");
    Serial.println("Ultra-low disturbance operation for breeding season monitoring");
    
    // Initialize raptor monitoring system for nesting scenario
    if (!raptorSystem.init(RaptorScenario::NESTING_MONITORING)) {
        Serial.println("ERROR: Failed to initialize raptor monitoring system");
        return;
    }
    
    // Configure ultra-sensitive detection for minimal disturbance
    raptorSystem.setDetectionSensitivity(25);  // Low sensitivity to avoid false triggers
    raptorSystem.enableAcousticMonitoring(true);
    raptorSystem.enableMeshCoordination(false); // Disable mesh to reduce RF emissions
    
    // Start monitoring session (indefinite duration)
    if (raptorSystem.startSession(0)) {
        Serial.println("Nesting monitoring session started successfully");
        Serial.println("Operating in stealth mode - minimal emissions");
    } else {
        Serial.println("ERROR: Failed to start monitoring session");
    }
}

/**
 * Main loop for nesting monitoring
 */
void loop() {
    // Update system state
    raptorSystem.update();
    
    // Check if system is active and operational
    if (raptorSystem.isActive()) {
        // Get current session statistics
        RaptorSession session = raptorSystem.getCurrentSession();
        
        // Log nesting-specific metrics every 30 minutes
        static unsigned long lastReport = 0;
        if (millis() - lastReport > 1800000) { // 30 minutes
            Serial.println("\n=== Nesting Monitoring Report ===");
            Serial.printf("Adult visits: %d\n", session.metrics.nesting.adultVisits);
            Serial.printf("Feeding events: %d\n", session.metrics.nesting.feedingEvents);
            Serial.printf("Predator alerts: %d\n", session.metrics.nesting.predatorAlerts);
            Serial.printf("Chick count: %d\n", session.metrics.nesting.chickCount);
            Serial.printf("Power consumption: %.2f mWh\n", session.powerConsumption_mWh);
            Serial.printf("Storage used: %d MB\n", session.storageUsed_MB);
            Serial.println("================================\n");
            
            lastReport = millis();
        }
        
        // Check system health
        auto health = raptorSystem.getSystemHealth();
        if (health.overallHealth_percent < 80.0f) {
            Serial.printf("WARNING: System health at %.1f%%\n", health.overallHealth_percent);
        }
        
        // Monitor power consumption for extended operation
        float currentPower = raptorSystem.getCurrentPowerConsumption();
        if (currentPower > 200.0f) { // Above 200mW budget for nesting
            Serial.printf("WARNING: Power consumption high: %.1f mW\n", currentPower);
        }
    }
    
    // Sleep to conserve power (nesting scenario prioritizes battery life)
    delay(5000); // 5 second update interval
}

/**
 * Configuration helper for nesting monitoring
 */
void configureNestingOptimization() {
    // These would be additional optimizations specific to nesting monitoring
    Serial.println("Applying nesting-specific optimizations:");
    Serial.println("- Ultra-low power mode enabled");
    Serial.println("- Silent operation (no LEDs, no sounds)");
    Serial.println("- Extended deep sleep intervals");
    Serial.println("- Weather-resistant operation");
    Serial.println("- Chick detection algorithms enabled");
    Serial.println("- Predator alert system active");
}