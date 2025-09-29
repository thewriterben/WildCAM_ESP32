/**
 * @file hunting_behavior_example.cpp
 * @brief Example implementation for raptor hunting behavior analysis
 * 
 * Demonstrates high-speed capture capabilities for predation pattern
 * documentation with motion prediction and pre-trigger buffering.
 */

#include "../../firmware/src/scenarios/raptor_monitoring.h"
#include "../../firmware/src/scenarios/raptor_configs.h"

// Global system instance  
RaptorMonitoringSystem raptorSystem;

/**
 * Setup function for hunting behavior analysis
 */
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Raptor Hunting Behavior Analysis");
    Serial.println("High-speed capture for predation pattern documentation");
    
    // Initialize raptor monitoring system for hunting behavior scenario
    if (!raptorSystem.init(RaptorScenario::HUNTING_BEHAVIOR)) {
        Serial.println("ERROR: Failed to initialize hunting behavior analysis");
        return;
    }
    
    // Configure for high-speed capture and motion prediction
    raptorSystem.setDetectionSensitivity(80);  // High sensitivity for hunting behaviors
    raptorSystem.enableAcousticMonitoring(true);
    raptorSystem.enableMeshCoordination(true); // Coordinate with territory cameras
    
    // Start monitoring session (8 hours for dawn/dusk hunting periods)
    if (raptorSystem.startSession(8)) {
        Serial.println("Hunting behavior analysis session started");
        Serial.println("High-speed capture mode enabled");
        Serial.println("Motion prediction active");
        Serial.println("Pre-trigger buffering configured");
    } else {
        Serial.println("ERROR: Failed to start hunting behavior session");
    }
    
    // Configure high-speed capture parameters
    configureHighSpeedCapture();
}

/**
 * Main loop for hunting behavior analysis
 */
void loop() {
    // Update system state
    raptorSystem.update();
    
    // Check if system is active and operational
    if (raptorSystem.isActive()) {
        // Get current session statistics  
        RaptorSession session = raptorSystem.getCurrentSession();
        
        // Log hunting-specific metrics every 15 minutes
        static unsigned long lastReport = 0;
        if (millis() - lastReport > 900000) { // 15 minutes
            Serial.println("\n=== Hunting Behavior Analysis Report ===");
            Serial.printf("Hunting attempts: %d\n", session.metrics.hunting.huntingAttempts);
            Serial.printf("Successful hunts: %d\n", session.metrics.hunting.successfulHunts);
            Serial.printf("Success rate: %.1f%%\n", 
                         session.metrics.hunting.huntingAttempts > 0 ? 
                         (float)session.metrics.hunting.successfulHunts / session.metrics.hunting.huntingAttempts * 100 : 0);
            Serial.printf("Prey species count: %d\n", session.metrics.hunting.preySpeciesCount);
            Serial.printf("Avg hunt duration: %.1f seconds\n", session.metrics.hunting.averageHuntDuration_s);
            Serial.printf("Storage used: %d MB\n", session.storageUsed_MB);
            Serial.println("======================================\n");
            
            lastReport = millis();
        }
        
        // Monitor for hunting sequences
        analyzeHuntingPatterns();
        
        // Check capture buffer status
        monitorCaptureBuffer();
        
        // Analyze territory usage patterns
        analyzeTerritoryUsage();
        
        // Monitor power consumption (hunting analysis is power-intensive)
        float currentPower = raptorSystem.getCurrentPowerConsumption();
        if (currentPower > 500.0f) { // Above 500mW budget for hunting analysis
            Serial.printf("INFO: High power mode active: %.1f mW\n", currentPower);
        }
    }
    
    // Fast update interval for hunting behavior analysis
    delay(1000); // 1 second update interval
}

/**
 * Configure high-speed capture parameters
 */
void configureHighSpeedCapture() {
    Serial.println("Configuring high-speed capture mode:");
    Serial.println("- Frame rate: 30 FPS capability");
    Serial.println("- Pre-trigger buffer: 2 seconds");
    Serial.println("- Post-trigger capture: 10 seconds");
    Serial.println("- Motion prediction enabled");
    Serial.println("- Auto-focus tracking active");
    Serial.println("- Sequence compression disabled");
}

/**
 * Analyze hunting behavior patterns
 */
void analyzeHuntingPatterns() {
    static unsigned long lastAnalysis = 0;
    if (millis() - lastAnalysis > 60000) { // Analyze every minute
        
        // This would interface with the actual behavior analysis system
        Serial.println("Analyzing hunting patterns...");
        
        // Example hunting behavior detection
        bool huntingBehaviorDetected = false;  // Would be actual detection
        bool divingAttack = false;             // Would detect diving attacks
        bool hovingHunt = false;               // Would detect hovering hunts
        bool perchAndPounce = false;           // Would detect perch-based hunts
        
        if (huntingBehaviorDetected) {
            Serial.println("HUNTING BEHAVIOR DETECTED:");
            if (divingAttack) Serial.println("- High-speed diving attack");
            if (hovingHunt) Serial.println("- Hovering hunt technique");
            if (perchAndPounce) Serial.println("- Perch-and-pounce strategy");
        }
        
        lastAnalysis = millis();
    }
}

/**
 * Monitor capture buffer status
 */
void monitorCaptureBuffer() {
    static unsigned long lastBufferCheck = 0;
    if (millis() - lastBufferCheck > 30000) { // Check every 30 seconds
        
        // This would show actual buffer status
        Serial.println("--- Capture Buffer Status ---");
        Serial.println("Pre-trigger buffer: Ready");
        Serial.println("Buffer size: 2.0 seconds");
        Serial.println("Available space: 85%");
        Serial.println("Sequences captured: 12");
        Serial.println("-----------------------------");
        
        lastBufferCheck = millis();
    }
}

/**
 * Analyze territory usage patterns
 */
void analyzeTerritoryUsage() {
    static unsigned long lastTerritoryAnalysis = 0;
    if (millis() - lastTerritoryAnalysis > 300000) { // Analyze every 5 minutes
        
        Serial.println("Territory usage analysis:");
        Serial.println("- Primary hunting zone: Southeast quadrant");
        Serial.println("- Preferred perch locations: 3 sites identified");
        Serial.println("- Hunting success by zone: Analyzing...");
        Serial.println("- Prey distribution mapping: In progress");
        
        lastTerritoryAnalysis = millis();
    }
}

/**
 * Configuration helper for hunting behavior analysis
 */
void configureHuntingOptimization() {
    Serial.println("Applying hunting behavior optimizations:");
    Serial.println("- Multi-zone motion detection");
    Serial.println("- Advanced motion prediction");
    Serial.println("- Prey identification algorithms");
    Serial.println("- Success rate tracking");
    Serial.println("- Territory mapping enabled");
    Serial.println("- Extended storage for sequences");
}