/**
 * @file raptor_configs.h
 * @brief Raptor-specific monitoring configurations
 * 
 * Defines specialized configurations for different raptor monitoring scenarios
 * optimized for birds of prey behavior, habitat requirements, and conservation research.
 */

#ifndef RAPTOR_CONFIGS_H
#define RAPTOR_CONFIGS_H

#include "../config.h"
#include <Arduino.h>

/**
 * Raptor Monitoring Scenarios
 */
enum class RaptorScenario {
    NESTING_MONITORING,      // Ultra-low disturbance nesting site monitoring
    MIGRATION_TRACKING,      // Seasonal migration pattern documentation
    HUNTING_BEHAVIOR,        // Predation pattern and prey selection analysis
    TERRITORIAL_BOUNDARY,    // Territorial dispute and boundary monitoring
    GENERAL_MONITORING       // General-purpose wildlife monitoring
};

/**
 * Raptor Species Categories for Specialized Detection
 */
enum class RaptorSpecies {
    EAGLE,                   // Eagles (Bald, Golden, etc.)
    HAWK,                    // Hawks (Red-tailed, Cooper's, etc.)
    FALCON,                  // Falcons (Peregrine, Kestrel, etc.)
    HARRIER,                 // Harriers (Northern Harrier)
    KITE,                    // Kites (Mississippi Kite, etc.)
    BUZZARD,                 // Buzzards (Rough-legged, etc.)
    UNKNOWN_RAPTOR           // Unidentified raptor
};

/**
 * Raptor Behavior Classifications
 */
enum class RaptorBehavior {
    SOARING,                 // High-altitude soaring/circling
    HUNTING,                 // Active hunting behavior
    PERCHING,                // Perched observation/rest
    FEEDING,                 // Feeding on prey
    TERRITORIAL_DISPLAY,     // Territorial behavior/calls
    NESTING_ACTIVITY,        // Nest building/maintenance
    PARENTAL_CARE,           // Feeding chicks/protection
    MIGRATION_FLIGHT,        // Directional migration movement
    COURTSHIP               // Mating displays and courtship
};

/**
 * Configuration for Raptor Nesting Monitoring
 * Ultra-low disturbance operation for breeding season monitoring
 */
struct NestingMonitoringConfig {
    // Stealth operation settings
    bool stealthMode = true;
    bool silentOperation = true;
    bool disableLEDs = true;
    bool disableAudibleAlerts = true;
    
    // Power optimization for remote locations
    uint32_t deepSleepDuration_ms = 300000;     // 5 minutes deep sleep
    float batteryThreshold_min = 3.0f;          // Minimum operating voltage
    bool solarOptimization = true;
    
    // Camera settings optimized for nesting
    uint16_t captureInterval_ms = 1800000;      // 30 minute intervals
    uint8_t imageQuality = 8;                   // High quality for detail
    bool timelapseMode = true;
    uint16_t burstCount = 3;                    // 3 images per trigger
    
    // Motion detection tuned for adults/chicks
    uint8_t motionSensitivity = 25;             // Low sensitivity to avoid disturbance
    uint16_t motionCooldown_ms = 60000;         // 1 minute cooldown
    bool weatherFiltering = true;
    
    // AI detection parameters
    float raptorConfidenceThreshold = 0.85f;
    bool enableChickDetection = true;
    bool enableParentDetection = true;
    bool enablePredatorDetection = true;
    
    // Storage optimization
    uint32_t maxStorageDays = 60;               // 60 days of storage
    bool compressImages = true;
    bool prioritizeRaptorImages = true;
};

/**
 * Configuration for Migration Route Tracking
 * Multi-camera coordination for flyway documentation
 */
struct MigrationTrackingConfig {
    // Network coordination
    bool meshNetworkEnabled = true;
    uint8_t maxMeshNodes = 20;
    bool timeSync = true;
    uint32_t syncInterval_ms = 30000;           // 30 second sync
    
    // High-speed capture for flight
    uint16_t burstMode = true;
    uint16_t burstCount = 10;                   // 10 rapid shots
    uint16_t burstInterval_ms = 200;            // 200ms between shots
    uint8_t captureFrameRate = 15;              // 15 FPS for sequences
    
    // Weather integration
    bool weatherDataCollection = true;
    bool windSpeedTracking = true;
    bool barometricTracking = true;
    float migrationTriggerPressure = 1015.0f;  // Typical migration pressure
    
    // Motion detection for flocks
    uint8_t motionSensitivity = 70;             // High sensitivity for distant birds
    bool flockDetection = true;
    uint16_t multiTargetTracking = 50;          // Track up to 50 birds
    
    // AI species identification
    float speciesConfidenceThreshold = 0.75f;
    bool enableFlightPatternAnalysis = true;
    bool enableFormationAnalysis = true;
    bool enableCountingAlgorithm = true;
    
    // Data storage and transmission
    bool realTimeTransmission = true;
    uint32_t dataRetention_hours = 168;         // 7 days retention
    bool prioritySpeciesAlert = true;
};

/**
 * Configuration for Hunting Behavior Analysis
 * High-speed capture for predation documentation
 */
struct HuntingBehaviorConfig {
    // High-speed capture system
    bool highSpeedMode = true;
    uint8_t maxFrameRate = 30;                  // 30 FPS capability
    uint16_t preTriggerBuffer_ms = 2000;        // 2 second pre-trigger
    uint16_t postTriggerCapture_ms = 10000;     // 10 second post-trigger
    
    // Advanced motion prediction
    bool motionPrediction = true;
    uint8_t predictionHorizon_ms = 500;         // 500ms prediction
    bool autoFocus = true;
    bool autoExposure = true;
    
    // Multi-zone detection
    uint8_t detectionZones = 9;                 // 3x3 grid detection
    bool territoryMapping = true;
    bool huntingGroundAnalysis = true;
    
    // Behavior classification
    bool preyIdentification = true;
    bool successRateTracking = true;
    bool huntingTechniqueAnalysis = true;
    float behaviorConfidenceThreshold = 0.80f;
    
    // Extended storage for sequences
    uint32_t maxSequenceLength_s = 300;         // 5 minute sequences
    bool sequenceCompression = false;           // No compression for analysis
    uint32_t storageCapacity_GB = 128;          // Large storage requirement
    
    // Power management for intensive operation
    bool adaptivePowerMode = true;
    uint16_t huntingSeasonBoost = true;         // Increased activity during hunting season
};

/**
 * Configuration for Territorial Boundary Monitoring
 * Multi-modal detection for territorial behavior
 */
struct TerritorialMonitoringConfig {
    // Multi-zone boundary detection
    uint8_t boundaryZones = 16;                 // 4x4 grid for boundaries
    bool zoneCrossingDetection = true;
    bool territorialMapGeneration = true;
    
    // Acoustic integration for calls
    bool audioRecording = true;
    uint32_t audioSampleRate = 44100;           // High quality audio
    uint16_t audioBuffer_s = 30;                // 30 second buffer
    bool callDetection = true;
    bool callClassification = true;
    
    // Individual identification
    bool individualTracking = true;
    bool biometricAnalysis = true;              // Size, markings, flight pattern
    uint8_t maxTrackedIndividuals = 20;
    
    // Inter-species interaction analysis
    bool multiSpeciesDetection = true;
    bool aggressionDetection = true;
    bool displacementTracking = true;
    
    // Long-term pattern analysis
    uint32_t analysisWindow_days = 30;          // 30-day analysis window
    bool seasonalAdaptation = true;
    bool territoryEvolutionTracking = true;
    
    // Alert system
    bool territorialConflictAlerts = true;
    bool newIndividualAlerts = true;
    float alertConfidenceThreshold = 0.85f;
    
    // Data collection
    uint32_t continuousRecording_hours = 24;    // 24/7 operation
    bool prioritizeInteractions = true;
    bool metadataEnrichment = true;
};

/**
 * Generic Raptor Detection Parameters
 */
struct RaptorDetectionParams {
    // Size filtering (pixels)
    uint16_t minRaptorSize_px = 50;
    uint16_t maxRaptorSize_px = 2000;
    
    // Flight pattern characteristics
    float minWingspan_ratio = 4.0f;             // Wingspan to body ratio
    float soaringDetection_threshold = 0.7f;
    float glideDetection_threshold = 0.6f;
    
    // Confidence thresholds
    float raptorClassification_threshold = 0.80f;
    float behaviorClassification_threshold = 0.75f;
    float speciesIdentification_threshold = 0.85f;
    
    // False positive filtering
    bool windFilterEnabled = true;
    bool shadowFilterEnabled = true;
    bool aircraftFilterEnabled = true;
    float aircraftRejection_threshold = 0.95f;
};

/**
 * Power Optimization for Extended Deployment
 */
struct RaptorPowerConfig {
    // Scenario-specific power profiles
    float nestingPowerBudget_mW = 150.0f;       // Ultra-low power for nesting
    float migrationPowerBudget_mW = 300.0f;     // Moderate power for tracking
    float huntingPowerBudget_mW = 500.0f;       // Higher power for high-speed
    float territorialPowerBudget_mW = 250.0f;   // Balanced for 24/7 operation
    
    // Adaptive power management
    bool activityBasedScaling = true;
    bool weatherBasedScaling = true;
    bool seasonalOptimization = true;
    
    // Solar integration
    float solarEfficiency_factor = 0.85f;
    uint16_t solarPanel_watts = 10;             // 10W panel recommendation
    uint32_t batteryCapacity_mAh = 5000;        // 5Ah battery recommendation
    
    // Sleep optimization
    uint32_t deepSleep_base_ms = 180000;        // 3 minute base interval
    uint32_t lightSleep_duration_ms = 10000;    // 10 second light sleep
    bool motionWakeup = true;
    bool scheduledWakeup = true;
};

#endif // RAPTOR_CONFIGS_H