/**
 * @file behavior_analysis.h
 * @brief Behavioral Pattern Recognition for Raptor Monitoring
 * 
 * Implements advanced behavioral analysis algorithms for bird of prey
 * monitoring including activity classification, temporal pattern analysis,
 * and conservation-relevant behavior identification.
 */

#ifndef BEHAVIOR_ANALYSIS_H
#define BEHAVIOR_ANALYSIS_H

#include "../scenarios/raptor_configs.h"
#include "raptor_detection.h"
#include <vector>
#include <memory>
#include <deque>

/**
 * Temporal Analysis Window Types
 */
enum class AnalysisWindow {
    IMMEDIATE,              // Current frame only
    SHORT_TERM,             // 5-30 seconds
    MEDIUM_TERM,            // 1-10 minutes  
    LONG_TERM,              // Hours to days
    SEASONAL                // Weeks to months
};

/**
 * Behavior Confidence Levels
 */
enum class BehaviorConfidence {
    LOW = 1,                // 50-65% confidence
    MEDIUM = 2,             // 65-80% confidence
    HIGH = 3,               // 80-95% confidence
    VERY_HIGH = 4           // 95%+ confidence
};

/**
 * Activity State Classification
 */
enum class ActivityState {
    RESTING,                // Perched or still
    ALERT,                  // Perched but vigilant
    PREENING,               // Grooming behavior
    FORAGING,               // Searching for food
    HUNTING,                // Active predation
    FEEDING,                // Consuming prey
    TERRITORIAL,            // Territorial behavior/calls
    COURTSHIP,              // Mating displays
    NESTING,                // Nest-related activities
    PARENTAL,               // Caring for young
    MIGRATION,              // Directional movement
    SOCIAL,                 // Group interactions
    THREAT_RESPONSE,        // Response to danger
    UNKNOWN                 // Unclassified behavior
};

/**
 * Behavioral Event Structure
 */
struct BehaviorEvent {
    ActivityState activity;
    RaptorBehavior behaviorType;
    BehaviorConfidence confidence;
    unsigned long startTime;
    unsigned long duration_ms;
    
    // Spatial information
    struct {
        float x, y;                     // Center coordinates (normalized 0-1)
        float boundingBox[4];           // [x1, y1, x2, y2]
        float movementDistance_px;
        float averageVelocity_pxps;     // Pixels per second
    } spatial;
    
    // Environmental context
    struct {
        float windSpeed_mps;
        float temperature_c;
        float lightLevel_lux;
        bool precipitation;
        String weatherCondition;
    } environment;
    
    // Associated data
    std::vector<uint32_t> imageFrameIds;
    std::vector<float> audioFeatures;
    String notes;
    bool validated;
    
    BehaviorEvent() : activity(ActivityState::UNKNOWN), 
                     behaviorType(RaptorBehavior::UNKNOWN), 
                     confidence(BehaviorConfidence::LOW),
                     startTime(0), duration_ms(0), validated(false) {}
};

/**
 * Behavior Pattern Recognition Result
 */
struct BehaviorAnalysisResult {
    // Primary classification
    ActivityState primaryActivity;
    RaptorBehavior primaryBehavior;
    BehaviorConfidence overallConfidence;
    
    // Temporal characteristics
    bool isSequentialBehavior;          // Part of longer sequence
    bool isRepeatingPattern;            // Repetitive behavior
    uint32_t cycleLength_s;             // If repetitive, cycle duration
    
    // Activity transitions
    ActivityState previousActivity;
    float transitionProbability;
    bool abnormalTransition;
    
    // Behavioral complexity
    struct {
        uint8_t complexity_score;       // 1-10 scale
        bool multiModal;                // Uses multiple sensory inputs
        bool socialComponent;           // Involves other individuals
        bool environmentalTrigger;      // Triggered by environment
    } characteristics;
    
    // Conservation relevance
    struct {
        bool conservationSignificant;
        String significance;            // Description of importance
        bool requiresAlert;
        uint8_t priority_level;         // 1-5 priority scale
    } conservation;
    
    // Quality metrics
    float dataQuality;
    float spatialAccuracy;
    float temporalAccuracy;
    uint32_t supportingFrames;
    
    BehaviorAnalysisResult() : primaryActivity(ActivityState::UNKNOWN),
                              primaryBehavior(RaptorBehavior::UNKNOWN),
                              overallConfidence(BehaviorConfidence::LOW),
                              isSequentialBehavior(false), isRepeatingPattern(false),
                              cycleLength_s(0), transitionProbability(0.0f),
                              abnormalTransition(false), dataQuality(0.0f),
                              spatialAccuracy(0.0f), temporalAccuracy(0.0f),
                              supportingFrames(0) {}
};

/**
 * Long-term Behavior Pattern
 */
struct BehaviorPattern {
    String patternName;
    std::vector<ActivityState> sequence;
    uint32_t averageDuration_s;
    float confidence;
    uint32_t observationCount;
    
    // Temporal characteristics
    struct {
        bool dailyPattern;
        bool seasonalPattern;
        std::vector<uint8_t> preferredHours;   // 0-23 hour preferences
        std::vector<uint8_t> preferredMonths;  // 1-12 month preferences
        float circadianAlignment;
    } temporal;
    
    // Environmental dependencies
    struct {
        bool weatherDependent;
        std::vector<String> preferredConditions;
        float temperatureRange[2];      // [min, max] in Celsius
        float windSpeedRange[2];        // [min, max] in m/s
    } environmental;
    
    // Conservation implications
    bool criticalForSurvival;
    bool indicatesHealth;
    bool indicatesStress;
    String conservationNotes;
    
    BehaviorPattern() : averageDuration_s(0), confidence(0.0f), 
                       observationCount(0), criticalForSurvival(false),
                       indicatesHealth(false), indicatesStress(false) {}
};

/**
 * Main Behavior Analysis System
 */
class BehaviorAnalysis {
public:
    /**
     * Constructor
     */
    BehaviorAnalysis();
    
    /**
     * Destructor
     */
    ~BehaviorAnalysis();
    
    /**
     * Initialize behavior analysis system
     * @param scenario Target monitoring scenario
     * @return true if initialization successful
     */
    bool init(RaptorScenario scenario);
    
    /**
     * Analyze current behavior from detection data
     * @param detection Current raptor detection result
     * @param window Analysis window type
     * @return Behavior analysis result
     */
    BehaviorAnalysisResult analyzeBehavior(const struct RaptorDetectionResult& detection, 
                                          AnalysisWindow window = AnalysisWindow::SHORT_TERM);
    
    /**
     * Update behavior analysis with new detection
     * @param detection New detection data
     */
    void updateAnalysis(const struct RaptorDetectionResult& detection);
    
    /**
     * Analyze behavior sequence over time window
     * @param detections Vector of recent detections
     * @param windowSize_s Analysis window in seconds
     * @return Comprehensive behavior analysis
     */
    BehaviorAnalysisResult analyzeSequence(const std::vector<struct RaptorDetectionResult>& detections,
                                          uint32_t windowSize_s);
    
    /**
     * Detect long-term behavior patterns
     * @param minObservations Minimum observations to establish pattern
     * @return Vector of detected patterns
     */
    std::vector<BehaviorPattern> detectPatterns(uint32_t minObservations = 5);
    
    /**
     * Classify activity state from single detection
     * @param detection Detection data
     * @return Activity classification
     */
    ActivityState classifyActivity(const struct RaptorDetectionResult& detection);
    
    /**
     * Predict next likely behavior
     * @param currentBehavior Current behavior state
     * @param timeHorizon_s Prediction horizon in seconds
     * @return Predicted behavior with confidence
     */
    std::pair<RaptorBehavior, float> predictNextBehavior(RaptorBehavior currentBehavior, 
                                                         uint32_t timeHorizon_s = 60);
    
    /**
     * Generate behavior summary for session
     * @return Comprehensive behavior summary
     */
    String generateBehaviorSummary() const;
    
    /**
     * Export behavior data for research
     * @param format Export format ("csv", "json", "xml")
     * @return Formatted behavior data
     */
    String exportBehaviorData(const String& format = "csv") const;
    
    /**
     * Set analysis sensitivity
     * @param sensitivity 0-100 sensitivity level
     */
    void setAnalysisSensitivity(uint8_t sensitivity);
    
    /**
     * Enable/disable real-time alerts
     * @param enable true to enable behavior alerts
     */
    void enableBehaviorAlerts(bool enable);
    
    /**
     * Get current analysis statistics
     */
    struct AnalysisStats {
        uint32_t totalObservations;
        uint32_t behaviorEvents;
        uint32_t patternsDetected;
        float averageConfidence;
        uint32_t conservationAlerts;
        float analysisUptime_percent;
    } getAnalysisStats() const;
    
    /**
     * Calibrate behavior baselines for environment
     * @param calibrationPeriod_hours Hours of baseline data collection
     * @return true if calibration successful
     */
    bool calibrateBaselines(uint8_t calibrationPeriod_hours = 24);
    
    /**
     * Check if analysis system is ready
     */
    bool isReady() const { return initialized && calibrated; }

private:
    // Core system state
    bool initialized;
    bool calibrated;
    RaptorScenario currentScenario;
    uint8_t analysisSensitivity;
    bool alertsEnabled;
    
    // Detection history management
    std::deque<struct RaptorDetectionResult> detectionHistory;
    std::deque<BehaviorEvent> behaviorHistory;
    std::vector<BehaviorPattern> detectedPatterns;
    static const size_t MAX_HISTORY_SIZE = 1000;
    
    // Analysis parameters
    struct {
        uint32_t shortTermWindow_s;
        uint32_t mediumTermWindow_s;
        uint32_t longTermWindow_s;
        float confidenceThreshold;
        float patternDetectionThreshold;
    } analysisParams;
    
    // Statistical baselines
    struct {
        float baselineActivityLevel;
        std::vector<float> hourlyActivity;      // 24-hour activity profile
        std::vector<float> behaviorFrequency;   // Frequency of each behavior type
        bool baselinesEstablished;
    } baselines;
    
    // Performance tracking
    struct {
        uint32_t totalAnalyses;
        float totalAnalysisTime_ms;
        uint32_t patternDetections;
        uint32_t alertsGenerated;
        unsigned long lastAnalysisTime;
    } performanceStats;
    
    // Analysis algorithms
    ActivityState analyzeMovementPattern(const std::vector<struct RaptorDetectionResult>& sequence);
    ActivityState analyzePerchingBehavior(const struct RaptorDetectionResult& detection);
    ActivityState analyzeFlightBehavior(const struct RaptorDetectionResult& detection);
    ActivityState analyzeInteractionBehavior(const std::vector<struct RaptorDetectionResult>& sequence);
    
    // Pattern recognition
    bool isHuntingSequence(const std::vector<struct RaptorDetectionResult>& sequence);
    bool isTerritorialDisplay(const std::vector<struct RaptorDetectionResult>& sequence);
    bool isCourtshipBehavior(const std::vector<struct RaptorDetectionResult>& sequence);
    bool isParentalBehavior(const std::vector<struct RaptorDetectionResult>& sequence);
    
    // Temporal analysis
    bool detectCyclicPattern(const std::vector<BehaviorEvent>& events, uint32_t& cycleLength);
    float calculateTransitionProbability(ActivityState from, ActivityState to);
    bool isAbnormalTransition(ActivityState from, ActivityState to, unsigned long timeGap_ms);
    
    // Environmental correlation
    float correlateWithWeather(const BehaviorEvent& behavior);
    float correlateWithTimeOfDay(const BehaviorEvent& behavior);
    float correlateWithSeason(const BehaviorEvent& behavior);
    
    // Feature extraction
    std::vector<float> extractSpatialFeatures(const struct RaptorDetectionResult& detection);
    std::vector<float> extractTemporalFeatures(const std::vector<struct RaptorDetectionResult>& sequence);
    std::vector<float> extractMotionFeatures(const std::vector<struct RaptorDetectionResult>& sequence);
    
    // Classification algorithms
    ActivityState classifyUsingMovement(const std::vector<float>& features);
    ActivityState classifyUsingPosture(const std::vector<float>& features);
    ActivityState classifyUsingEnvironment(const std::vector<float>& features);
    
    // Pattern matching
    float matchPattern(const std::vector<ActivityState>& observed, const BehaviorPattern& pattern);
    bool validatePattern(const BehaviorPattern& pattern);
    void updatePatternStatistics(BehaviorPattern& pattern, const BehaviorEvent& event);
    
    // Alert generation
    void checkConservationAlerts(const BehaviorAnalysisResult& result);
    void generateAlert(const String& alertType, const BehaviorAnalysisResult& result);
    bool shouldAlert(const BehaviorAnalysisResult& result);
    
    // Utility methods
    void cleanupOldData();
    void updatePerformanceMetrics(float analysisTime_ms);
    float calculateOverallConfidence(const std::vector<float>& confidences);
    String behaviorToDescription(const BehaviorAnalysisResult& result);
    void logBehaviorEvent(const BehaviorEvent& event);
    
    // Scenario-specific analysis
    void configureForNestingMonitoring();
    void configureForMigrationTracking();
    void configureForHuntingAnalysis();
    void configureForTerritorialMonitoring();
};

/**
 * Behavior Analysis Utilities
 */
namespace BehaviorUtils {
    /**
     * Convert activity state to string
     */
    const char* activityToString(ActivityState activity);
    
    /**
     * Convert confidence level to string
     */
    const char* confidenceToString(BehaviorConfidence confidence);
    
    /**
     * Calculate behavior complexity score
     */
    uint8_t calculateComplexityScore(const BehaviorAnalysisResult& result);
    
    /**
     * Generate conservation assessment
     */
    String generateConservationAssessment(const std::vector<BehaviorPattern>& patterns);
    
    /**
     * Calculate behavioral diversity index
     */
    float calculateBehavioralDiversity(const std::vector<BehaviorEvent>& events);
    
    /**
     * Identify stress indicators in behavior
     */
    std::vector<String> identifyStressIndicators(const std::vector<BehaviorEvent>& events);
}

#endif // BEHAVIOR_ANALYSIS_H