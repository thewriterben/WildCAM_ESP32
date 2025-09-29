/**
 * @file territorial_monitoring_example.cpp
 * @brief Example implementation for raptor territorial boundary monitoring
 * 
 * Demonstrates multi-modal detection for territorial behavior including
 * acoustic integration, individual identification, and boundary mapping.
 */

#include "../../firmware/src/scenarios/raptor_monitoring.h"
#include "../../firmware/src/scenarios/raptor_configs.h"

// Global system instance
RaptorMonitoringSystem raptorSystem;

/**
 * Setup function for territorial boundary monitoring
 */
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Raptor Territorial Monitoring");
    Serial.println("Multi-modal detection for territorial behavior analysis");
    
    // Initialize raptor monitoring system for territorial scenario
    if (!raptorSystem.init(RaptorScenario::TERRITORIAL_BOUNDARY)) {
        Serial.println("ERROR: Failed to initialize territorial monitoring");
        return;
    }
    
    // Configure for multi-modal detection and boundary mapping
    raptorSystem.setDetectionSensitivity(75);  // Balanced sensitivity
    raptorSystem.enableAcousticMonitoring(true);  // Critical for territorial calls
    raptorSystem.enableMeshCoordination(true);    // Coordinate boundary cameras
    
    // Start monitoring session (continuous for 24/7 operation)
    if (raptorSystem.startSession(0)) {
        Serial.println("Territorial monitoring session started");
        Serial.println("24/7 continuous operation enabled");
        Serial.println("Multi-zone boundary detection active");
        Serial.println("Acoustic call detection enabled");
        Serial.println("Individual identification system ready");
    } else {
        Serial.println("ERROR: Failed to start territorial monitoring session");
    }
    
    // Configure territorial detection zones
    configureTerritorialZones();
}

/**
 * Main loop for territorial monitoring
 */
void loop() {
    // Update system state
    raptorSystem.update();
    
    // Check if system is active and operational
    if (raptorSystem.isActive()) {
        // Get current session statistics
        RaptorSession session = raptorSystem.getCurrentSession();
        
        // Log territorial-specific metrics every 20 minutes
        static unsigned long lastReport = 0;
        if (millis() - lastReport > 1200000) { // 20 minutes
            Serial.println("\n=== Territorial Monitoring Report ===");
            Serial.printf("Territorial displays: %d\n", session.metrics.territorial.territorialDisplays);
            Serial.printf("Intruder events: %d\n", session.metrics.territorial.intruderEvents);
            Serial.printf("Calls detected: %d\n", session.metrics.territorial.callsDetected);
            Serial.printf("Individuals identified: %d\n", session.metrics.territorial.individualsIdentified);
            Serial.printf("Network uptime: %.1f%%\n", session.networkUptime_percent);
            Serial.println("====================================\n");
            
            lastReport = millis();
        }
        
        // Monitor territorial boundaries
        monitorTerritorialBoundaries();
        
        // Analyze acoustic activity
        analyzeAcousticActivity();
        
        // Track individual raptors
        trackIndividualRaptors();
        
        // Detect territorial conflicts
        detectTerritorialConflicts();
        
        // Monitor inter-species interactions
        monitorInterSpeciesInteractions();
    }
    
    // Moderate update interval for territorial monitoring
    delay(3000); // 3 second update interval
}

/**
 * Configure territorial detection zones
 */
void configureTerritorialZones() {
    Serial.println("Configuring territorial detection zones:");
    Serial.println("- Zone 1: North boundary (4x4 grid)");
    Serial.println("- Zone 2: East boundary (4x4 grid)");
    Serial.println("- Zone 3: South boundary (4x4 grid)");
    Serial.println("- Zone 4: West boundary (4x4 grid)");
    Serial.println("- Zone 5: Central territory (core area)");
    Serial.println("- Acoustic monitoring: 360-degree coverage");
    Serial.println("- Individual ID tracking: Active");
}

/**
 * Monitor territorial boundaries
 */
void monitorTerritorialBoundaries() {
    static unsigned long lastBoundaryCheck = 0;
    if (millis() - lastBoundaryCheck > 60000) { // Check every minute
        
        Serial.println("Territorial boundary status:");
        
        // This would interface with actual boundary detection system
        bool northBoundaryActivity = false;    // Would be actual detection
        bool eastBoundaryActivity = false;
        bool southBoundaryActivity = true;     // Example: activity detected
        bool westBoundaryActivity = false;
        bool centralTerritoryActivity = true;
        
        if (northBoundaryActivity) Serial.println("- North boundary: Activity detected");
        if (eastBoundaryActivity) Serial.println("- East boundary: Activity detected");
        if (southBoundaryActivity) {
            Serial.println("- South boundary: ACTIVITY DETECTED");
            Serial.println("  -> Possible territorial incursion");
        }
        if (westBoundaryActivity) Serial.println("- West boundary: Activity detected");
        if (centralTerritoryActivity) {
            Serial.println("- Central territory: Active");
            Serial.println("  -> Resident raptor present");
        }
        
        lastBoundaryCheck = millis();
    }
}

/**
 * Analyze acoustic activity for territorial calls
 */
void analyzeAcousticActivity() {
    static unsigned long lastAcousticAnalysis = 0;
    if (millis() - lastAcousticAnalysis > 45000) { // Analyze every 45 seconds
        
        // This would interface with actual acoustic detection system
        bool territorialCallDetected = true;   // Example: call detected
        bool alarmCallDetected = false;
        bool aggressiveCallDetected = false;
        bool contactCallDetected = false;
        
        if (territorialCallDetected || alarmCallDetected || aggressiveCallDetected || contactCallDetected) {
            Serial.println("ACOUSTIC ACTIVITY DETECTED:");
            if (territorialCallDetected) {
                Serial.println("- Territorial call: Red-tailed Hawk");
                Serial.println("  -> Frequency: 2.1 kHz, Duration: 1.2s");
                Serial.println("  -> Confidence: 89%");
            }
            if (alarmCallDetected) Serial.println("- Alarm call detected");
            if (aggressiveCallDetected) Serial.println("- Aggressive vocalization");
            if (contactCallDetected) Serial.println("- Contact call (pair communication)");
        }
        
        lastAcousticAnalysis = millis();
    }
}

/**
 * Track individual raptors for territorial analysis
 */
void trackIndividualRaptors() {
    static unsigned long lastIndividualTracking = 0;
    if (millis() - lastIndividualTracking > 120000) { // Track every 2 minutes
        
        Serial.println("Individual tracking update:");
        Serial.println("- Individual A: Resident adult male");
        Serial.println("  -> Location: Central perch");
        Serial.println("  -> Activity: Territorial patrol");
        Serial.println("  -> Confidence: 94%");
        
        Serial.println("- Individual B: Adult female");
        Serial.println("  -> Location: East boundary");
        Serial.println("  -> Activity: Foraging");
        Serial.println("  -> Confidence: 87%");
        
        // Example of intruder detection
        bool intruderDetected = true; // Example detection
        if (intruderDetected) {
            Serial.println("- INTRUDER DETECTED: Unknown adult");
            Serial.println("  -> Location: South boundary");
            Serial.println("  -> Behavior: Territorial challenge");
            Serial.println("  -> Alert level: HIGH");
        }
        
        lastIndividualTracking = millis();
    }
}

/**
 * Detect territorial conflicts
 */
void detectTerritorialConflicts() {
    static unsigned long lastConflictCheck = 0;
    if (millis() - lastConflictCheck > 180000) { // Check every 3 minutes
        
        // This would analyze territorial behavior patterns
        bool conflictDetected = false;    // Would be actual analysis
        bool aggressiveInteraction = false;
        bool chaseSequence = false;
        bool territorialDisplay = true;   // Example: display detected
        
        if (conflictDetected || aggressiveInteraction || chaseSequence || territorialDisplay) {
            Serial.println("TERRITORIAL BEHAVIOR ANALYSIS:");
            if (territorialDisplay) {
                Serial.println("- Territorial display detected");
                Serial.println("  -> Type: Aerial display");
                Serial.println("  -> Duration: 45 seconds");
                Serial.println("  -> Participants: 2 individuals");
            }
            if (chaseSequence) Serial.println("- Chase sequence in progress");
            if (aggressiveInteraction) Serial.println("- Aggressive interaction detected");
        }
        
        lastConflictCheck = millis();
    }
}

/**
 * Monitor inter-species interactions
 */
void monitorInterSpeciesInteractions() {
    static unsigned long lastInterSpeciesCheck = 0;
    if (millis() - lastInterSpeciesCheck > 240000) { // Check every 4 minutes
        
        Serial.println("Inter-species interaction analysis:");
        
        // Example of mixed species detection
        bool mixedSpeciesDetected = false;  // Would be actual detection
        bool competitiveInteraction = false;
        bool neutralCoexistence = true;     // Example: peaceful coexistence
        
        if (mixedSpeciesDetected || competitiveInteraction || neutralCoexistence) {
            if (neutralCoexistence) {
                Serial.println("- Red-tailed Hawk and American Kestrel");
                Serial.println("  -> Interaction: Neutral coexistence");
                Serial.println("  -> Territory overlap: Minimal");
                Serial.println("  -> Competition level: Low");
            }
            if (competitiveInteraction) Serial.println("- Competitive interaction detected");
        } else {
            Serial.println("- Single species present (Red-tailed Hawk)");
        }
        
        lastInterSpeciesCheck = millis();
    }
}

/**
 * Configuration helper for territorial monitoring
 */
void configureTerritorialOptimization() {
    Serial.println("Applying territorial monitoring optimizations:");
    Serial.println("- Multi-zone boundary detection (16 zones)");
    Serial.println("- Acoustic call classification enabled");
    Serial.println("- Individual identification and tracking");
    Serial.println("- Territorial conflict detection");
    Serial.println("- Inter-species interaction analysis");
    Serial.println("- 24/7 continuous operation");
    Serial.println("- Boundary evolution tracking");
}