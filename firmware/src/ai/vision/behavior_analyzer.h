/**
 * @file behavior_analyzer.h
 * @brief AI-powered wildlife behavior analysis
 * 
 * Provides real-time behavior classification and pattern analysis
 * for wildlife monitoring and research applications.
 */

#ifndef BEHAVIOR_ANALYZER_H
#define BEHAVIOR_ANALYZER_H

#include "../ai_common.h"


/**
 * Detailed Behavior Analysis Result
 */
struct BehaviorResult {
    BehaviorType primaryBehavior;
    BehaviorType secondaryBehavior;  // Secondary behavior if applicable
    float confidence;
    ConfidenceLevel confidenceLevel;
    float activity_level;            // 0.0 to 1.0
    float stress_level;              // 0.0 to 1.0 (estimated)
    uint16_t duration;               // Behavior duration in seconds
    bool isRepeatedBehavior;         // Pattern detection
    unsigned long timestamp;
    
    // Context information
    bool isGroupBehavior;            // Multiple animals involved
    uint8_t animalCount;             // Number of animals in behavior
    bool hasHumanInteraction;        // Human presence affecting behavior
    
    BehaviorResult() : 
        primaryBehavior(BehaviorType::UNKNOWN),
        secondaryBehavior(BehaviorType::UNKNOWN),
        confidence(0.0f),
        confidenceLevel(ConfidenceLevel::VERY_LOW),
        activity_level(0.0f), stress_level(0.0f),
        duration(0), isRepeatedBehavior(false),
        timestamp(0), isGroupBehavior(false),
        animalCount(1), hasHumanInteraction(false) {}
};

/**
 * Behavior Pattern Detection
 */
struct BehaviorPattern {
    String patternName;
    std::vector<BehaviorType> sequence;
    float frequency;                 // How often this pattern occurs
    uint32_t occurrences;           // Total occurrences observed
    unsigned long avgDuration;       // Average duration of pattern
    float confidence;               // Pattern reliability
    bool isSeasonalPattern;         // Varies by season
    bool isTimeOfDayPattern;        // Varies by time of day
};

/**
 * Wildlife Behavior Analyzer
 * 
 * Advanced AI system for analyzing animal behavior patterns,
 * stress indicators, and social interactions.
 */
class BehaviorAnalyzer {
public:
    BehaviorAnalyzer();
    ~BehaviorAnalyzer();

    // Initialization and configuration
    bool init();
    bool loadBehaviorModel(const char* modelPath);
    bool loadSpeciesSpecificModel(SpeciesType species, const char* modelPath);
    
    // Behavior analysis operations
    BehaviorResult analyzeBehavior(const CameraFrame& frame, SpeciesType species = SpeciesType::UNKNOWN);
    std::vector<BehaviorResult> analyzeGroupBehavior(const CameraFrame& frame, const std::vector<SpeciesResult>& animals);
    BehaviorResult analyzeTemporalBehavior(const std::vector<CameraFrame>& frameSequence, SpeciesType species);
    
    // Pattern detection
    void enablePatternDetection(bool enable = true);
    std::vector<BehaviorPattern> detectPatterns(uint32_t observationPeriodDays = 7);
    BehaviorPattern getMostCommonPattern(SpeciesType species = SpeciesType::UNKNOWN);
    
    // Stress and health analysis
    void enableStressDetection(bool enable = true);
    float analyzeStressLevel(const CameraFrame& frame, SpeciesType species);
    bool detectAbnormalBehavior(const BehaviorResult& result, SpeciesType species);
    
    // Social behavior analysis
    void enableSocialAnalysis(bool enable = true);
    BehaviorResult analyzeSocialInteraction(const CameraFrame& frame, const std::vector<SpeciesResult>& animals);
    bool detectAggressiveBehavior(const CameraFrame& frame);
    bool detectMatingBehavior(const CameraFrame& frame, SpeciesType species);
    
    // Temporal analysis
    void enableTemporalTracking(bool enable = true);
    void setObservationWindow(uint32_t windowSizeFrames);
    std::vector<BehaviorResult> getRecentBehaviors(uint32_t timeWindowMinutes = 60);
    
    // Environmental context
    void setEnvironmentalContext(float temperature, float humidity, int lightLevel);
    void enableWeatherCorrelation(bool enable = true);
    BehaviorResult analyzeWeatherInfluence(const BehaviorResult& behavior);
    
    // Learning and adaptation
    void enableContinuousLearning(bool enable = true);
    void updateBehaviorModel(const CameraFrame& frame, BehaviorType correctBehavior);
    bool trainFromLocalObservations();
    
    // Configuration and thresholds
    void setConfidenceThreshold(float threshold);
    void setActivityThreshold(float threshold);
    void setStressThreshold(float threshold);
    void enableSpeciesSpecificAnalysis(bool enable = true);
    
    // Performance monitoring
    AIMetrics getBehaviorMetrics() const;
    float getBehaviorAccuracy() const;
    void resetMetrics();
    
    // Data export and research
    void enableResearchMode(bool enable = true);
    void exportBehaviorData(const char* filename);
    void exportPatternData(const char* filename);
    std::vector<BehaviorResult> getBehaviorHistory(uint32_t hours = 24);
    
    // Alert system
    void enableAlerts(bool enable = true);
    void setAlertCallback(void (*callback)(BehaviorType behavior, float confidence));
    void addBehaviorAlert(BehaviorType behavior, float minConfidence);
    
    // Debug and diagnostics
    void enableDetailedLogging(bool enable = true);
    bool validateBehaviorModel();
    void printBehaviorStatistics();

private:
    // Core components
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    
    // Configuration
    float confidenceThreshold_;
    float activityThreshold_;
    float stressThreshold_;
    bool patternDetectionEnabled_;
    bool stressDetectionEnabled_;
    bool socialAnalysisEnabled_;
    bool temporalTrackingEnabled_;
    bool weatherCorrelationEnabled_;
    bool continuousLearningEnabled_;
    bool speciesSpecificAnalysisEnabled_;
    bool researchModeEnabled_;
    bool alertsEnabled_;
    bool detailedLoggingEnabled_;
    
    // Temporal analysis
    uint32_t observationWindow_;
    std::queue<BehaviorResult> recentBehaviors_;
    std::vector<CameraFrame> frameHistory_;
    
    // Pattern detection
    std::vector<BehaviorPattern> detectedPatterns_;
    void updatePatterns(const BehaviorResult& newBehavior);
    bool isPatternMatch(const std::vector<BehaviorType>& sequence, const BehaviorPattern& pattern);
    
    // Environmental context
    struct EnvironmentalData {
        float temperature;
        float humidity;
        int lightLevel;
        unsigned long timestamp;
    };
    EnvironmentalData currentEnvironment_;
    
    // Alert system
    struct BehaviorAlert {
        BehaviorType behavior;
        float minConfidence;
        void (*callback)(BehaviorType, float);
    };
    std::vector<BehaviorAlert> behaviorAlerts_;
    void checkAlerts(const BehaviorResult& result);
    
    // Performance tracking
    mutable AIMetrics behaviorMetrics_;
    uint32_t totalAnalyses_;
    uint32_t correctAnalyses_;
    
    // Preprocessing and postprocessing
    bool preprocessForBehaviorAnalysis(const CameraFrame& frame, float* inputTensor);
    BehaviorResult postprocessBehaviorOutput(const float* output, const CameraFrame& frame);
    
    // Stress analysis
    float calculateStressIndicators(const CameraFrame& frame, SpeciesType species);
    bool analyzePostureStress(const CameraFrame& frame);
    bool analyzeMovementPatternStress(const std::vector<CameraFrame>& frames);
    
    // Social behavior analysis
    BehaviorResult analyzeDominanceHierarchy(const std::vector<SpeciesResult>& animals);
    bool detectPlayBehavior(const CameraFrame& frame, const std::vector<SpeciesResult>& animals);
    bool detectParentalBehavior(const CameraFrame& frame, SpeciesType species);
    
    // Utility functions
    BehaviorType getBehaviorFromModelOutput(int classIndex);
    float calculateBehaviorTransitionProbability(BehaviorType from, BehaviorType to);
    bool isCompatibleBehavior(BehaviorType primary, BehaviorType secondary);
    float getSpeciesTypicalActivityLevel(SpeciesType species, BehaviorType behavior);
    
    // Model management
    std::map<SpeciesType, std::unique_ptr<InferenceEngine>> speciesSpecificModels_;
    bool loadSpeciesModel(SpeciesType species);
    
    // Constants
    static const float DEFAULT_CONFIDENCE_THRESHOLD;
    static const float DEFAULT_ACTIVITY_THRESHOLD;
    static const float DEFAULT_STRESS_THRESHOLD;
    static const uint32_t DEFAULT_OBSERVATION_WINDOW;
    static const uint32_t MAX_PATTERN_LENGTH = 10;
    static const uint32_t MAX_RECENT_BEHAVIORS = 1000;
};

// Utility functions
const char* getBehaviorDescription(BehaviorType behavior);
bool isActiveBehavior(BehaviorType behavior);
bool isPassiveBehavior(BehaviorType behavior);
float getBehaviorEnergyLevel(BehaviorType behavior);
bool isStressIndicator(BehaviorType behavior);

#endif // BEHAVIOR_ANALYZER_H