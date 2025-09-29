/**
 * @file power_ai_optimizer.h
 * @brief Advanced Power-AI Optimization System for ESP32 Wildlife Camera
 * 
 * Implements intelligent power management specifically optimized for AI workloads,
 * with dynamic model selection, predictive sleep scheduling, and power-aware
 * inference optimization based on wildlife behavior patterns.
 */

#ifndef POWER_AI_OPTIMIZER_H
#define POWER_AI_OPTIMIZER_H

#include "ai_common.h"
#include "tinyml/inference_engine.h"
#include <vector>
#include <memory>

/**
 * Power-AI Operating Modes
 */
enum class PowerAIMode {
    HIBERNATION,        // Minimal power, motion detection only
    ECO_MONITORING,     // Low power, basic AI every 30s
    BALANCED,           // Medium power, full AI every 10s
    PERFORMANCE,        // High power, full AI every 2s
    RESEARCH,           // Maximum power, continuous AI processing
    ADAPTIVE            // Dynamic mode based on conditions
};

/**
 * AI Workload Priority Levels
 */
enum class AIWorkloadPriority {
    CRITICAL,           // Emergency detection (predators, humans)
    HIGH,               // Species identification
    MEDIUM,             // Behavior analysis
    LOW,                // Data collection
    BACKGROUND          // Model updates, learning
};

/**
 * Power Budget Configuration
 */
struct PowerBudget {
    uint32_t totalBudget_mW;        // Total power budget in milliwatts
    uint32_t aiProcessing_mW;       // Allocated for AI processing
    uint32_t camera_mW;             // Allocated for camera operations
    uint32_t networking_mW;         // Allocated for communication
    uint32_t sensors_mW;            // Allocated for environmental sensors
    uint32_t reserve_mW;            // Emergency reserve
    
    PowerBudget() : totalBudget_mW(1000), aiProcessing_mW(400),
                   camera_mW(300), networking_mW(200),
                   sensors_mW(50), reserve_mW(50) {}
};

/**
 * Wildlife Activity Prediction
 */
struct ActivityPrediction {
    float probabilityScore;         // 0.0 - 1.0 activity probability
    std::vector<SpeciesType> expectedSpecies;
    std::vector<BehaviorType> expectedBehaviors;
    uint32_t nextWakeTime_ms;       // Milliseconds until next expected activity
    float confidenceLevel;          // Confidence in prediction
    
    ActivityPrediction() : probabilityScore(0.0f), nextWakeTime_ms(0),
                          confidenceLevel(0.0f) {}
};

/**
 * Dynamic AI Configuration
 */
struct DynamicAIConfig {
    ModelType primaryModel;
    ModelType fallbackModel;
    float confidenceThreshold;
    uint32_t processingInterval_ms;
    bool useProgressiveInference;
    bool enableEarlyExit;
    uint32_t maxInferenceTime_ms;
    
    DynamicAIConfig() : primaryModel(ModelType::SPECIES_CLASSIFIER),
                       fallbackModel(ModelType::MOTION_DETECTOR),
                       confidenceThreshold(0.6f),
                       processingInterval_ms(5000),
                       useProgressiveInference(true),
                       enableEarlyExit(true),
                       maxInferenceTime_ms(2000) {}
};

/**
 * Power-AI Performance Metrics
 */
struct PowerAIMetrics {
    float averagePowerConsumption_mW;
    float powerEfficiency;          // Detections per mW
    uint32_t totalDetections;
    uint32_t missedDetections;
    float averageResponseTime_ms;
    float batteryLifeExtension_percent;
    uint32_t deepSleepCycles;
    uint32_t powerOptimizationEvents;
    
    PowerAIMetrics() : averagePowerConsumption_mW(0.0f), powerEfficiency(0.0f),
                      totalDetections(0), missedDetections(0),
                      averageResponseTime_ms(0.0f), batteryLifeExtension_percent(0.0f),
                      deepSleepCycles(0), powerOptimizationEvents(0) {}
};

/**
 * Advanced Power-AI Optimization System
 * 
 * Coordinates AI processing with power management to maximize
 * wildlife detection efficiency while extending battery life.
 */
class PowerAIOptimizer {
public:
    PowerAIOptimizer();
    ~PowerAIOptimizer();

    // Core initialization and configuration
    bool init(InferenceEngine* inferenceEngine);
    void configure(const PowerBudget& budget, PowerAIMode mode);
    void cleanup();
    
    // Dynamic power-AI optimization
    bool processFrame(const CameraFrame& frame, AIResult& result);
    void updatePowerState(float batteryVoltage, bool isCharging, float solarVoltage);
    void updateEnvironmentalConditions(float temperature, float humidity, float lightLevel);
    
    // Intelligent sleep scheduling
    uint32_t calculateOptimalSleepDuration();
    bool shouldEnterDeepSleep();
    void scheduleWakeup(uint32_t durationMs);
    ActivityPrediction predictNextActivity();
    
    // Dynamic model selection
    ModelType selectOptimalModel(float batteryLevel, float urgency);
    DynamicAIConfig optimizeAIConfiguration();
    void adaptConfigurationToConditions();
    
    // Power-aware inference strategies
    bool runProgressiveInference(const CameraFrame& frame, AIResult& result);
    bool tryLightweightDetection(const CameraFrame& frame, AIResult& result);
    bool runPowerOptimizedInference(const CameraFrame& frame, ModelType model, AIResult& result);
    
    // Wildlife behavior pattern learning
    void recordWildlifeActivity(const AIResult& result);
    void learnActivityPatterns();
    std::vector<ActivityPrediction> getPredictedActivities(uint32_t lookAheadHours = 24);
    
    // Adaptive power management
    void enableAdaptivePowerManagement(bool enable = true);
    void setPowerBudget(const PowerBudget& budget);
    PowerBudget getCurrentPowerBudget() const;
    void adjustPowerAllocation();
    
    // Emergency and priority handling
    void enableEmergencyMode(bool enable = true);
    void setDetectionPriorities(const std::vector<std::pair<SpeciesType, AIWorkloadPriority>>& priorities);
    bool handleHighPriorityDetection(const AIResult& result);
    
    // Performance monitoring and optimization
    PowerAIMetrics getPerformanceMetrics() const;
    void optimizeBasedOnMetrics();
    void resetMetrics();
    void printOptimizationReport();
    
    // Configuration and tuning
    void setOperatingMode(PowerAIMode mode);
    PowerAIMode getOperatingMode() const;
    void enablePredictiveOptimization(bool enable = true);
    void setActivityPredictionWindow(uint32_t hours);
    
    // Advanced features
    void enableSeasonalAdaptation(bool enable = true);
    void updateSeasonalParameters(uint8_t month, float latitude);
    void enableWeatherAdaptation(bool enable = true);
    void adaptToWeatherConditions(float windSpeed, float precipitation);
    
    // Multi-camera coordination
    void enableMultiCameraCoordination(bool enable = true);
    void shareActivityData(const ActivityPrediction& prediction);
    void coordinatePowerManagement();
    
    // Research and conservation features
    void enableConservationMode(bool enable = true);
    void setConservationPriorities(const std::vector<SpeciesType>& prioritySpecies);
    void generateEnergyEfficiencyReport(const char* filename);
    
    // Debugging and diagnostics
    void enableDebugMode(bool enable = true);
    void printSystemStatus();
    void exportOptimizationData(const char* filename);
    bool runPowerOptimizationBenchmark();

private:
    // Core components
    InferenceEngine* inferenceEngine_;
    bool initialized_;
    
    // Configuration
    PowerBudget powerBudget_;
    PowerAIMode currentMode_;
    DynamicAIConfig aiConfig_;
    bool adaptivePowerManagementEnabled_;
    bool emergencyModeEnabled_;
    bool predictiveOptimizationEnabled_;
    bool debugModeEnabled_;
    
    // Power state tracking
    float currentBatteryVoltage_;
    bool isCharging_;
    float solarVoltage_;
    uint32_t currentPowerConsumption_mW_;
    unsigned long lastPowerUpdate_;
    
    // Environmental conditions
    float currentTemperature_;
    float currentHumidity_;
    float currentLightLevel_;
    float windSpeed_;
    float precipitation_;
    uint8_t currentMonth_;
    float latitude_;
    
    // Activity pattern learning
    struct ActivityRecord {
        SpeciesType species;
        BehaviorType behavior;
        float confidence;
        uint8_t hourOfDay;
        uint8_t dayOfWeek;
        uint8_t month;
        float temperature;
        float lightLevel;
        unsigned long timestamp;
    };
    
    std::vector<ActivityRecord> activityHistory_;
    std::vector<ActivityPrediction> predictedActivities_;
    uint32_t activityPredictionWindow_;
    
    // Power optimization state
    unsigned long lastOptimizationTime_;
    uint32_t optimizationInterval_;
    std::vector<std::pair<SpeciesType, AIWorkloadPriority>> detectionPriorities_;
    std::vector<SpeciesType> conservationPriorities_;
    
    // Performance tracking
    mutable PowerAIMetrics metrics_;
    unsigned long metricsStartTime_;
    
    // Sleep and wake management
    unsigned long lastSleepTime_;
    uint32_t totalSleepTime_;
    std::vector<uint32_t> sleepDurations_;
    
    // Internal optimization functions
    void updatePowerConsumption();
    float calculatePowerEfficiency();
    void optimizePowerAllocation();
    
    // Activity prediction algorithms
    float calculateActivityProbability(uint8_t hour, uint8_t day, uint8_t month);
    std::vector<SpeciesType> predictActiveSpecies(uint8_t hour, float temperature, float light);
    uint32_t calculateOptimalWakeTime();
    
    // Model selection algorithms
    ModelType selectModelBasedOnPower(float batteryLevel);
    ModelType selectModelBasedOnActivity(const ActivityPrediction& prediction);
    bool shouldUseProgressiveInference(float batteryLevel, float urgency);
    
    // Configuration optimization
    void optimizeProcessingInterval();
    void optimizeConfidenceThreshold();
    void optimizeInferenceTimeout();
    
    // Emergency and priority handling
    bool isHighPrioritySpecies(SpeciesType species);
    void handleEmergencyDetection(const AIResult& result);
    void adjustPriorityBasedOnBattery();
    
    // Seasonal and weather adaptation
    void applySeasonalAdjustments();
    void applyWeatherAdjustments();
    float getSeasonalActivityFactor();
    float getWeatherActivityFactor();
    
    // Multi-camera coordination
    void syncWithOtherCameras();
    void shareOptimizationData();
    void coordinateSleepSchedules();
    
    // Utility functions
    void updateMetrics(const AIResult& result, float powerConsumed);
    void cleanupOldActivityRecords();
    bool shouldTriggerOptimization();
    void logOptimizationEvent(const String& event);
    
    // Constants
    static const size_t MAX_ACTIVITY_RECORDS = 10000;
    static const uint32_t DEFAULT_OPTIMIZATION_INTERVAL = 300000; // 5 minutes
    static const float EMERGENCY_BATTERY_THRESHOLD; // 3.2V
    static const float LOW_BATTERY_THRESHOLD; // 3.4V
    static const uint32_t MIN_SLEEP_DURATION = 1000; // 1 second
    static const uint32_t MAX_SLEEP_DURATION = 3600000; // 1 hour
};

// Utility functions
const char* powerAIModeToString(PowerAIMode mode);
const char* aiWorkloadPriorityToString(AIWorkloadPriority priority);
PowerAIMode stringToPowerAIMode(const char* modeStr);
float calculateBatteryLifeExtension(const PowerAIMetrics& metrics);

#endif // POWER_AI_OPTIMIZER_H