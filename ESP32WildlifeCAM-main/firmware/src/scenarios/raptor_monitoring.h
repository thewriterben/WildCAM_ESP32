/**
 * @file raptor_monitoring.h
 * @brief Core Raptor Monitoring System
 * 
 * Implements specialized monitoring capabilities for birds of prey including
 * scenario-specific configurations, behavior analysis, and optimized detection.
 */

#ifndef RAPTOR_MONITORING_H
#define RAPTOR_MONITORING_H

#include "raptor_configs.h"
#include "../ai/ai_common.h"
#include "../motion_filter.h"
#include "../power_manager.h"
#include "../multi_board/multi_board_system.h"
#include <memory>
#include <vector>

// Forward declarations
class RaptorDetection;
class BehaviorAnalysis;
class AcousticDetection;
class RaptorMotionFilter;

/**
 * Raptor Detection Result
 */
struct RaptorDetectionResult {
    bool raptorDetected;
    RaptorSpecies species;
    RaptorBehavior behavior;
    float confidence;
    
    // Biometric data
    float estimatedWingspan_cm;
    float estimatedBodyLength_cm;
    String colorPattern;
    String flightCharacteristics;
    
    // Location and movement
    struct {
        uint16_t x, y;                          // Center position in image
        uint16_t width, height;                 // Bounding box dimensions
        float velocity_mps;                     // Movement velocity
        float direction_degrees;                // Movement direction
        float altitude_estimate_m;              // Estimated altitude
    } position;
    
    // Temporal data
    unsigned long timestamp;
    unsigned long sequenceId;
    uint16_t frameNumber;
    
    // Environmental context
    struct {
        float windSpeed_mps;
        float temperature_c;
        float pressure_hPa;
        uint8_t cloudCover_percent;
        bool precipitation;
    } environment;
    
    // Audio analysis (if available)
    struct {
        bool callDetected;
        float callFrequency_Hz;
        float callDuration_ms;
        float callIntensity_dB;
        String callType;                        // "alarm", "territorial", "mating", etc.
    } acoustics;
    
    // Quality metrics
    float imageQuality;
    float motionBlur;
    float lighting_quality;
    bool occluded;
};

/**
 * Raptor Monitoring Session
 */
struct RaptorSession {
    RaptorScenario scenario;
    unsigned long startTime;
    unsigned long endTime;
    uint32_t totalDetections;
    uint32_t validDetections;
    uint32_t falsePositives;
    
    // Scenario-specific metrics
    union {
        struct {                                // Nesting monitoring
            uint32_t adultVisits;
            uint32_t feedingEvents;
            uint32_t predatorAlerts;
            uint32_t chickCount;
        } nesting;
        
        struct {                                // Migration tracking
            uint32_t birdsCount;
            uint32_t flocksDetected;
            float averageFlightSpeed_mps;
            float dominantDirection_degrees;
        } migration;
        
        struct {                                // Hunting behavior
            uint32_t huntingAttempts;
            uint32_t successfulHunts;
            uint32_t preySpeciesCount;
            float averageHuntDuration_s;
        } hunting;
        
        struct {                                // Territorial monitoring
            uint32_t territorialDisplays;
            uint32_t intruderEvents;
            uint32_t callsDetected;
            uint8_t individualsIdentified;
        } territorial;
    } metrics;
    
    // Power and performance
    float powerConsumption_mWh;
    float averageProcessingTime_ms;
    uint32_t storageUsed_MB;
    float networkUptime_percent;
};

/**
 * Raptor Monitoring System Main Class
 */
class RaptorMonitoringSystem {
public:
    /**
     * Constructor
     */
    RaptorMonitoringSystem();
    
    /**
     * Destructor
     */
    ~RaptorMonitoringSystem();
    
    /**
     * Initialize the raptor monitoring system
     * @param scenario The monitoring scenario to configure
     * @return true if initialization successful
     */
    bool init(RaptorScenario scenario);
    
    /**
     * Start monitoring session
     * @param duration_hours Session duration in hours (0 = indefinite)
     * @return true if session started successfully
     */
    bool startSession(uint32_t duration_hours = 0);
    
    /**
     * Stop current monitoring session
     * @return Session statistics
     */
    RaptorSession stopSession();
    
    /**
     * Process current frame for raptor detection
     * @param imageData Camera frame data
     * @param imageSize Size of image data
     * @return Detection result
     */
    RaptorDetectionResult processFrame(const uint8_t* imageData, size_t imageSize);
    
    /**
     * Update system state (call regularly in main loop)
     */
    void update();
    
    /**
     * Get current system status
     */
    bool isActive() const { return sessionActive; }
    
    /**
     * Get current scenario
     */
    RaptorScenario getCurrentScenario() const { return currentScenario; }
    
    /**
     * Get session statistics
     */
    RaptorSession getCurrentSession() const { return currentSession; }
    
    /**
     * Configure scenario-specific parameters
     * @param scenario Target scenario
     * @return true if configuration successful
     */
    bool configureScenario(RaptorScenario scenario);
    
    /**
     * Enable/disable acoustic monitoring
     * @param enable true to enable audio processing
     */
    void enableAcousticMonitoring(bool enable);
    
    /**
     * Enable/disable multi-board coordination
     * @param enable true to enable mesh networking
     */
    void enableMeshCoordination(bool enable);
    
    /**
     * Set detection sensitivity
     * @param sensitivity 0-100 sensitivity level
     */
    void setDetectionSensitivity(uint8_t sensitivity);
    
    /**
     * Get power consumption estimate
     * @return Current power consumption in mW
     */
    float getCurrentPowerConsumption() const;
    
    /**
     * Get storage utilization
     * @return Storage usage percentage (0-100)
     */
    uint8_t getStorageUtilization() const;
    
    /**
     * Export session data
     * @param format Export format ("json", "csv", "xml")
     * @return Exported data as string
     */
    String exportSessionData(const String& format = "json") const;
    
    /**
     * Calibrate for specific environment
     * @param calibration_minutes Duration of calibration in minutes
     * @return true if calibration successful
     */
    bool calibrateEnvironment(uint8_t calibration_minutes = 10);
    
    /**
     * Get system health status
     */
    struct SystemHealth {
        bool cameraOperational;
        bool aiSystemOperational;
        bool motionFilterOperational;
        bool powerSystemOperational;
        bool storageOperational;
        bool networkOperational;
        float overallHealth_percent;
    } getSystemHealth() const;

private:
    // Core system state
    bool initialized;
    bool sessionActive;
    RaptorScenario currentScenario;
    RaptorSession currentSession;
    unsigned long lastUpdate;
    
    // Configuration instances
    std::unique_ptr<NestingMonitoringConfig> nestingConfig;
    std::unique_ptr<MigrationTrackingConfig> migrationConfig;
    std::unique_ptr<HuntingBehaviorConfig> huntingConfig;
    std::unique_ptr<TerritorialMonitoringConfig> territorialConfig;
    RaptorDetectionParams detectionParams;
    RaptorPowerConfig powerConfig;
    
    // Component instances
    std::unique_ptr<RaptorDetection> raptorDetector;
    std::unique_ptr<BehaviorAnalysis> behaviorAnalyzer;
    std::unique_ptr<AcousticDetection> acousticDetector;
    std::unique_ptr<RaptorMotionFilter> motionFilter;
    
    // System integrations
    PowerManager* powerManager;
    MultiboardSystem* meshSystem;
    
    // Internal state tracking
    std::vector<RaptorDetectionResult> recentDetections;
    uint32_t sequenceCounter;
    float currentPowerConsumption;
    uint32_t storageUsed;
    
    // Private methods
    bool loadScenarioConfig(RaptorScenario scenario);
    void optimizePowerForScenario();
    void configureMotionDetection();
    void configureCameraSettings();
    void configureNetworking();
    bool validateDetection(const RaptorDetectionResult& result);
    void updateSessionMetrics(const RaptorDetectionResult& result);
    void processAcousticData();
    void managePowerOptimization();
    void handleMeshCoordination();
    String generateMetadataString(const RaptorDetectionResult& result) const;
    bool shouldTriggerCapture() const;
    void cleanup();
    
    // Scenario-specific handlers
    void handleNestingMonitoring(const RaptorDetectionResult& result);
    void handleMigrationTracking(const RaptorDetectionResult& result);
    void handleHuntingBehavior(const RaptorDetectionResult& result);
    void handleTerritorialMonitoring(const RaptorDetectionResult& result);
    
    // Utility methods
    float calculateConfidenceScore(const RaptorDetectionResult& result) const;
    bool isWithinOperatingHours() const;
    void triggerAlert(const String& alertType, const RaptorDetectionResult& result);
    void logDetection(const RaptorDetectionResult& result);
};

/**
 * Utility Functions for Raptor Analysis
 */
namespace RaptorUtils {
    /**
     * Convert raptor species enum to string
     */
    const char* speciesToString(RaptorSpecies species);
    
    /**
     * Convert raptor behavior enum to string
     */
    const char* behaviorToString(RaptorBehavior behavior);
    
    /**
     * Convert scenario enum to string
     */
    const char* scenarioToString(RaptorScenario scenario);
    
    /**
     * Calculate estimated altitude from image data
     */
    float estimateAltitude(uint16_t objectSize_px, float realSize_m, float focalLength_mm);
    
    /**
     * Analyze flight pattern characteristics
     */
    String analyzeFlightPattern(const std::vector<RaptorDetectionResult>& sequence);
    
    /**
     * Generate conservation report
     */
    String generateConservationReport(const RaptorSession& session);
    
    /**
     * Calculate territory boundaries from detection data
     */
    std::vector<std::pair<float, float>> calculateTerritoryBoundaries(
        const std::vector<RaptorDetectionResult>& detections);
}

#endif // RAPTOR_MONITORING_H