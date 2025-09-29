/**
 * @file ai_wildlife_system.h
 * @brief Enhanced AI Wildlife Monitoring System Integration
 * 
 * Integrates all AI/ML components for comprehensive wildlife monitoring
 * with the existing ESP32 Wildlife Camera system, including advanced
 * optimizations, multi-modal processing, and intelligent power management.
 */

#ifndef AI_WILDLIFE_SYSTEM_H
#define AI_WILDLIFE_SYSTEM_H

#include "ai_common.h"
#include "tinyml/inference_engine.h"
#include "vision/species_classifier.h"
#include "vision/behavior_analyzer.h"
#include "power_ai_optimizer.h"
#include "ai_benchmark_optimizer.h"
#include "multimodal_ai_system.h"
#include "predictive/predictive_analytics_engine.h"
#include <memory>

// Forward declarations
class ModelCache;
class ModelPredictor;

// Forward declaration for predictive analytics
struct PredictiveAnalysisResult;

/**
 * Enhanced Wildlife Analysis Result
 */
struct WildlifeAnalysisResult {
    SpeciesResult species;
    BehaviorResult behavior;
    bool motionDetected;
    float overallConfidence;
    bool threatDetected;
    bool humanPresenceDetected;
    unsigned long processingTime;
    unsigned long timestamp;
    
    // Enhanced multi-modal results
    MultiModalResult multiModalResult;
    AudioFeatures audioAnalysis;
    EnvironmentalContext environmentalContext;
    
    // Power and performance metrics
    float powerConsumption_mW;
    ModelType usedModel;
    bool wasFromCache;
    float cacheHitRate;
    
    // Advanced analysis
    float stressLevel;
    float biodiversityIndex;
    std::vector<String> behaviorIndicators;
    String analysisNotes;
    
    // Predictive Analytics Results
    struct PredictiveData {
        BehaviorType predictedNextBehavior;
        float predictionConfidence;
        uint32_t timeToNextBehavior_s;
        bool behaviorAnomalyDetected;
        float populationHealthScore;
        bool migrationEventPredicted;
        String conservationRecommendation;
        float conservationPriority;
        std::vector<uint32_t> optimalFeedingTimes;
        
        PredictiveData() : 
            predictedNextBehavior(BehaviorType::UNKNOWN),
            predictionConfidence(0.0f), timeToNextBehavior_s(0),
            behaviorAnomalyDetected(false), populationHealthScore(0.0f),
            migrationEventPredicted(false), conservationPriority(0.0f) {}
    } predictiveData;
    
    WildlifeAnalysisResult() : 
        motionDetected(false), overallConfidence(0.0f),
        threatDetected(false), humanPresenceDetected(false),
        processingTime(0), timestamp(0),
        powerConsumption_mW(0.0f),
        usedModel(ModelType::SPECIES_CLASSIFIER),
        wasFromCache(false), cacheHitRate(0.0f),
        stressLevel(0.0f), biodiversityIndex(0.0f) {}
};

/**
 * Enhanced AI Processing Configuration
 */
struct AIProcessingConfig {
    bool enableSpeciesClassification;
    bool enableBehaviorAnalysis;
    bool enableMotionDetection;
    bool enableThreatDetection;
    bool enableHumanDetection;
    bool enableRealTimeProcessing;
    bool enableBatchProcessing;
    bool enableMultiModalProcessing;
    bool enablePowerOptimization;
    bool enableIntelligentCaching;
    bool enablePredictiveProcessing;
    bool enableEnvironmentalAdaptation;
    
    // Predictive Analytics Configuration
    bool enablePredictiveAnalytics;
    bool enableBehaviorPrediction;
    bool enablePatternDetection;
    bool enableAnomalyDetection;
    bool enableConservationInsights;
    
    float confidenceThreshold;
    uint32_t processingInterval;     // ms between AI analyses
    PowerAIMode powerMode;
    OptimizationTarget optimizationTarget;
    
    AIProcessingConfig() : 
        enableSpeciesClassification(true),
        enableBehaviorAnalysis(true),
        enableMotionDetection(true),
        enableThreatDetection(false),
        enableHumanDetection(true),
        enableRealTimeProcessing(true),
        enableBatchProcessing(false),
        enableMultiModalProcessing(true),
        enablePowerOptimization(true),
        enableIntelligentCaching(true),
        enablePredictiveProcessing(true),
        enableEnvironmentalAdaptation(true),
        enablePredictiveAnalytics(true),
        enableBehaviorPrediction(true),
        enablePatternDetection(true),
        enableAnomalyDetection(true),
        enableConservationInsights(true),
        enablePredictiveProcessing(true),
        enableEnvironmentalAdaptation(true),
        confidenceThreshold(0.6f),
        processingInterval(2000),
        powerMode(PowerAIMode::ADAPTIVE),
        optimizationTarget(OptimizationTarget::BALANCED_PERFORMANCE) {}
};

/**
 * Intelligent Trigger System
 */
class IntelligentTrigger {
public:
    IntelligentTrigger();
    
    bool init();
    bool shouldCapture(const WildlifeAnalysisResult& analysis);
    void learnFromCapture(const WildlifeAnalysisResult& analysis, bool wasUseful);
    float predictAnimalProbability();
    void optimizeForSpecies(SpeciesType target);
    void setTimeBasedTriggers(bool enable = true);
    void setBehaviorBasedTriggers(bool enable = true);
    
    // Power optimization
    void enablePowerOptimization(bool enable = true);
    unsigned long getPredictedSleepTime();
    bool shouldWakeUp();

private:
    struct TriggerPattern {
        SpeciesType species;
        BehaviorType behavior;
        uint8_t hourOfDay;
        float probability;
        uint32_t successCount;
        uint32_t totalCount;
    };
    
    std::vector<TriggerPattern> learnedPatterns_;
    bool timeBasedTriggersEnabled_;
    bool behaviorBasedTriggersEnabled_;
    bool powerOptimizationEnabled_;
    
    void updatePatterns(const WildlifeAnalysisResult& analysis, bool success);
    float calculateTriggerProbability(const WildlifeAnalysisResult& analysis);
};

/**
 * Enhanced AI Wildlife Monitoring System
 * 
 * Coordinates all AI components and integrates with the existing
 * camera system for intelligent wildlife monitoring with advanced
 * optimizations, multi-modal processing, and power management.
 */
class AIWildlifeSystem {
public:
    AIWildlifeSystem();
    ~AIWildlifeSystem();

    // System initialization and configuration
    bool init();
    bool loadModels(const char* modelsDirectory);
    void configure(const AIProcessingConfig& config);
    void cleanup();
    bool isSystemReady() const;
    
    // Enhanced processing functions
    WildlifeAnalysisResult analyzeFrame(const CameraFrame& frame);
    WildlifeAnalysisResult analyzeFrameWithAudio(const CameraFrame& frame, 
                                                const float* audioData, 
                                                size_t audioLength);
    WildlifeAnalysisResult analyzeFrameSequence(const std::vector<CameraFrame>& frames);
    WildlifeAnalysisResult analyzeMultiModal(const CameraFrame& frame,
                                           const float* audioData,
                                           size_t audioLength,
                                           const EnvironmentalContext& environment);
    bool processMotionEvent(const CameraFrame& frame);
    
    // Integration with existing system
    bool shouldTriggerCapture();
    void onImageCaptured(const CameraFrame& frame, const String& filename);
    void onMotionDetected();
    void updatePowerState(float batteryVoltage, bool isCharging);
    void updateEnvironmentalConditions(const EnvironmentalContext& environment);
    
    // Intelligent scheduling and power management
    void enableIntelligentScheduling(bool enable = true);
    void enablePowerOptimization(bool enable = true);
    unsigned long getNextWakeTime();
    bool shouldEnterDeepSleep();
    
    // Predictive Analytics Interface
    bool enablePredictiveAnalytics(bool enable = true);
    PredictiveAnalysisResult generatePredictiveAnalysis(const BehaviorResult& behavior,
                                                       const EnvironmentalData& environment);
    PopulationHealthMetrics getPopulationHealth(uint32_t timeWindow_s = 86400);
    std::vector<uint32_t> predictOptimalFeedingTimes(SpeciesType species, uint8_t daysAhead = 7);
    PredictionResult predictMigrationPattern(SpeciesType species);
    bool exportPredictionData(const char* filename, bool includeRawData = false);
    AIMetrics getPredictionMetrics() const;
    void resetPredictiveModels();
    void optimizeForBatteryLife();
    PowerAIMode getCurrentPowerMode() const;
    
    // Learning and adaptation
    void enableContinuousLearning(bool enable = true);
    void updateFromUserFeedback(const String& imageFilename, SpeciesType correctSpecies);
    void shareDataForFederatedLearning();
    void enableEnvironmentalAdaptation(bool enable = true);
    
    // Performance monitoring and optimization
    AIMetrics getSystemMetrics() const;
    PowerAIMetrics getPowerMetrics() const;
    String getSystemStatus() const;
    void printSystemStatistics();
    void runPerformanceBenchmark();
    void optimizeSystemPerformance();
    
    // Configuration and settings
    void setConfidenceThreshold(float threshold);
    void enablePowerOptimization(bool enable = true);
    void setProcessingPriority(ModelType priority);
    void setOptimizationTarget(OptimizationTarget target);
    void setPowerMode(PowerAIMode mode);
    
    // Model management
    bool updateModel(ModelType type, const char* newModelPath);
    std::vector<ModelType> getLoadedModels() const;
    bool validateAllModels();
    void enableModelCaching(bool enable = true);
    float getCacheHitRate() const;
    
    // Multi-modal processing
    void enableMultiModalProcessing(bool enable = true);
    void enableAudioProcessing(bool enable = true);
    void configureSensorFusion(const SensorFusionConfig& config);
    MultiModalResult getLastMultiModalResult() const;
    
    // Advanced features
    void enableEcosystemMonitoring(bool enable = true);
    void enableThreatDetection(bool enable = true);
    void setConservationTargets(const std::vector<SpeciesType>& targets);
    std::vector<SpeciesType> getPredictedActiveSpecies() const;
    float calculateBiodiversityIndex() const;
    
    // Research and data export
    void enableResearchMode(bool enable = true);
    void exportAnalysisData(const char* filename);
    void exportLearningData(const char* filename);
    void exportPerformanceReport(const char* filename);
    void generateEcosystemReport(const char* filename);

private:
    // Core AI components
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    std::unique_ptr<WildlifeClassifier> speciesClassifier_;
    std::unique_ptr<BehaviorAnalyzer> behaviorAnalyzer_;
    std::unique_ptr<IntelligentTrigger> intelligentTrigger_;
    
    // Enhanced optimization components
    std::unique_ptr<PowerAIOptimizer> powerOptimizer_;
    std::unique_ptr<AIBenchmarkOptimizer> benchmarkOptimizer_;
    std::unique_ptr<MultiModalAISystem> multiModalSystem_;
    
    // Predictive Analytics Engine
    std::unique_ptr<PredictiveAnalyticsEngine> predictiveEngine_;
    
    // Configuration
    AIProcessingConfig config_;
    bool systemInitialized_;
    bool intelligentSchedulingEnabled_;
    bool continuousLearningEnabled_;
    bool researchModeEnabled_;
    bool multiModalProcessingEnabled_;
    bool powerOptimizationEnabled_;
    
    // Performance tracking
    mutable AIMetrics systemMetrics_;
    std::vector<WildlifeAnalysisResult> recentAnalyses_;
    
    // Power management
    float batteryVoltage_;
    bool isCharging_;
    unsigned long lastProcessingTime_;
    EnvironmentalContext currentEnvironment_;
    
    // Processing optimization
    unsigned long lastAnalysisTime_;
    ModelType processingPriority_;
    OptimizationTarget optimizationTarget_;
    PowerAIMode currentPowerMode_;
    
    // Learning data
    struct LearningRecord {
        String imageFilename;
        WildlifeAnalysisResult analysis;
        SpeciesType correctSpecies;
        BehaviorType correctBehavior;
        bool userVerified;
        unsigned long timestamp;
    };
    std::vector<LearningRecord> learningData_;
    
    // Internal processing functions
    bool initializeAIComponents();
    WildlifeAnalysisResult runFullAnalysis(const CameraFrame& frame);
    WildlifeAnalysisResult runOptimizedAnalysis(const CameraFrame& frame);
    WildlifeAnalysisResult runMultiModalAnalysis(const CameraFrame& frame,
                                                const float* audioData,
                                                size_t audioLength,
                                                const EnvironmentalContext& environment);
    
    // Power-optimized processing
    WildlifeAnalysisResult runLowPowerAnalysis(const CameraFrame& frame);
    bool shouldRunFullAnalysis();
    void adjustProcessingBasedOnPower();
    
    // Learning and adaptation
    void processLearningData();
    void updateModelsFromLearning();
    void saveLearningData();
    void loadLearningData();
    
    // Environmental adaptation
    void adaptToEnvironmentalConditions();
    void updateSpeciesPredictions();
    void adjustConfidenceThresholds();
    
    // Utility functions
    void updateSystemMetrics(const WildlifeAnalysisResult& result);
    void logAnalysisResult(const WildlifeAnalysisResult& result);
    bool isHighPriorityDetection(const WildlifeAnalysisResult& result);
    void triggerConservationAlert(const WildlifeAnalysisResult& result);
    
    // Constants
    static const size_t MAX_RECENT_ANALYSES = 100;
    static const size_t MAX_LEARNING_RECORDS = 1000;
    static const unsigned long MIN_PROCESSING_INTERVAL = 1000; // 1 second
    static const float LOW_BATTERY_THRESHOLD = 3.4f; // Volts
    static const float EMERGENCY_BATTERY_THRESHOLD = 3.2f; // Volts
};

// Global AI system instance (singleton pattern)
extern AIWildlifeSystem* g_aiSystem;

// Integration functions for existing code
bool initializeAISystem();
void cleanupAISystem();
bool isAISystemReady();
WildlifeAnalysisResult analyzeCurrentFrame(camera_fb_t* fb);
bool shouldAITriggerCapture();
void notifyAIOfImageCapture(const String& filename);

#endif // AI_WILDLIFE_SYSTEM_H