/**
 * @file behavior_pattern_detector.h
 * @brief Advanced Behavior Pattern Detection System
 * 
 * Implements sophisticated pattern recognition algorithms for wildlife
 * behavior analysis, including temporal patterns, sequence detection,
 * and pattern classification.
 */

#ifndef BEHAVIOR_PATTERN_DETECTOR_H
#define BEHAVIOR_PATTERN_DETECTOR_H

#include "../ai_common.h"
#include "../behavior_analysis.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Pattern Detection Algorithm Types
 */
enum class PatternAlgorithm {
    SEQUENCE_MATCHING,      // Direct sequence pattern matching
    MARKOV_CHAIN,          // Markov chain behavior transitions
    NEURAL_PATTERN,        // Neural network pattern recognition
    STATISTICAL_ANALYSIS,  // Statistical pattern analysis
    HYBRID_APPROACH        // Combination of multiple algorithms
};

/**
 * Enhanced Behavior Pattern with Predictive Capabilities
 */
struct EnhancedBehaviorPattern {
    // Basic pattern information
    String patternId;
    String patternName;
    std::vector<BehaviorType> sequence;
    float confidence;
    uint32_t observationCount;
    
    // Temporal characteristics
    struct {
        uint32_t averageDuration_s;
        uint32_t minDuration_s;
        uint32_t maxDuration_s;
        float variance;
        
        // Time-based patterns
        bool isCircadian;               // 24-hour pattern
        bool isSeasonal;               // Seasonal pattern
        std::vector<float> hourlyProbability;   // 24 elements [0.0-1.0]
        std::vector<float> monthlyProbability;  // 12 elements [0.0-1.0]
    } temporal;
    
    // Transition probabilities
    std::map<BehaviorType, float> nextBehaviorProbabilities;
    std::map<BehaviorType, uint32_t> transitionCounts;
    
    // Environmental dependencies
    struct {
        bool weatherDependent;
        float temperatureRange[2];      // [min, max] optimal temperature
        float humidityRange[2];         // [min, max] optimal humidity
        std::vector<String> preferredWeatherConditions;
        float lightLevelPreference;     // 0.0 = dark, 1.0 = bright
    } environmental;
    
    // Conservation significance
    struct {
        bool criticalForSurvival;
        bool indicatesHealth;
        bool indicatesStress;
        bool indicatesMating;
        bool indicatesFeeding;
        float conservationImportance;   // 0.0-1.0 importance score
        String conservationNotes;
    } conservation;
    
    // Pattern detection metadata
    unsigned long firstObserved;
    unsigned long lastObserved;
    float detectionAccuracy;
    PatternAlgorithm detectionMethod;
    
    EnhancedBehaviorPattern() : 
        confidence(0.0f), observationCount(0),
        detectionAccuracy(0.0f), detectionMethod(PatternAlgorithm::SEQUENCE_MATCHING) {
        memset(&temporal, 0, sizeof(temporal));
        memset(&environmental, 0, sizeof(environmental));
        memset(&conservation, 0, sizeof(conservation));
        firstObserved = lastObserved = 0;
        
        // Initialize probability arrays
        temporal.hourlyProbability.resize(24, 0.0f);
        temporal.monthlyProbability.resize(12, 0.0f);
    }
};

/**
 * Pattern Detection Configuration
 */
struct PatternDetectionConfig {
    PatternAlgorithm primaryAlgorithm;
    float minimumConfidence;
    uint32_t minimumObservations;
    uint32_t maxPatternLength;
    uint32_t analysisWindowSize;
    
    bool enableTemporalAnalysis;
    bool enableEnvironmentalCorrelation;
    bool enableTransitionAnalysis;
    bool enableStatisticalValidation;
    
    PatternDetectionConfig() :
        primaryAlgorithm(PatternAlgorithm::HYBRID_APPROACH),
        minimumConfidence(0.7f), minimumObservations(5),
        maxPatternLength(10), analysisWindowSize(100),
        enableTemporalAnalysis(true), enableEnvironmentalCorrelation(true),
        enableTransitionAnalysis(true), enableStatisticalValidation(true) {}
};

/**
 * Pattern Detection Result
 */
struct PatternDetectionResult {
    std::vector<EnhancedBehaviorPattern> detectedPatterns;
    std::vector<EnhancedBehaviorPattern> newPatterns;
    std::vector<EnhancedBehaviorPattern> updatedPatterns;
    
    uint32_t totalPatternsAnalyzed;
    uint32_t patternsAboveThreshold;
    float averagePatternConfidence;
    uint32_t processingTime_ms;
    
    PatternDetectionResult() :
        totalPatternsAnalyzed(0), patternsAboveThreshold(0),
        averagePatternConfidence(0.0f), processingTime_ms(0) {}
};

/**
 * Advanced Behavior Pattern Detection System
 * 
 * Implements sophisticated algorithms for detecting, analyzing, and
 * classifying wildlife behavior patterns with high accuracy.
 */
class BehaviorPatternDetector {
public:
    /**
     * Constructor
     */
    BehaviorPatternDetector();
    
    /**
     * Destructor
     */
    ~BehaviorPatternDetector();
    
    /**
     * Initialize pattern detector
     * @param config Detection configuration
     * @return true if initialization successful
     */
    bool init(const PatternDetectionConfig& config = PatternDetectionConfig());
    
    /**
     * Detect patterns in behavior sequence
     * @param behaviors Vector of behavior observations
     * @param environment Environmental context data
     * @return Pattern detection results
     */
    PatternDetectionResult detectPatterns(const std::vector<BehaviorResult>& behaviors,
                                        const EnvironmentalData& environment);
    
    /**
     * Add new behavior observation for pattern learning
     * @param behavior New behavior observation
     * @param environment Environmental context
     */
    void addBehaviorObservation(const BehaviorResult& behavior,
                              const EnvironmentalData& environment);
    
    /**
     * Get all known behavior patterns
     * @return Vector of all detected patterns
     */
    std::vector<EnhancedBehaviorPattern> getAllPatterns() const;
    
    /**
     * Get patterns matching specific criteria
     * @param behaviorType Target behavior type
     * @param minConfidence Minimum confidence threshold
     * @return Matching patterns
     */
    std::vector<EnhancedBehaviorPattern> getPatterns(BehaviorType behaviorType,
                                                   float minConfidence = 0.7f) const;
    
    /**
     * Predict next behavior based on current sequence
     * @param recentBehaviors Recent behavior sequence
     * @param currentEnvironment Current environmental conditions
     * @return Predicted next behavior with confidence
     */
    std::pair<BehaviorType, float> predictNextBehavior(
        const std::vector<BehaviorType>& recentBehaviors,
        const EnvironmentalData& currentEnvironment) const;
    
    /**
     * Analyze behavior transition probabilities
     * @param fromBehavior Source behavior
     * @param environment Environmental context
     * @return Map of transition probabilities to other behaviors
     */
    std::map<BehaviorType, float> getTransitionProbabilities(
        BehaviorType fromBehavior,
        const EnvironmentalData& environment) const;
    
    /**
     * Validate pattern accuracy against observations
     * @param pattern Pattern to validate
     * @param observations Validation observations
     * @return Validation accuracy score
     */
    float validatePattern(const EnhancedBehaviorPattern& pattern,
                         const std::vector<BehaviorResult>& observations) const;
    
    /**
     * Update pattern confidence based on new observations
     * @param patternId Pattern identifier
     * @param wasCorrect Whether pattern prediction was correct
     */
    void updatePatternConfidence(const String& patternId, bool wasCorrect);
    
    /**
     * Get pattern detection statistics
     * @return Detection performance metrics
     */
    AIMetrics getDetectionMetrics() const;
    
    /**
     * Export patterns for research analysis
     * @param filename Output filename
     * @param includeStatistics Include detection statistics
     * @return true if export successful
     */
    bool exportPatterns(const char* filename, bool includeStatistics = true) const;
    
    /**
     * Load patterns from previous session
     * @param filename Input filename
     * @return true if load successful
     */
    bool loadPatterns(const char* filename);
    
    /**
     * Save current patterns to storage
     * @param filename Output filename
     * @return true if save successful
     */
    bool savePatterns(const char* filename) const;
    
    /**
     * Clear all detected patterns
     */
    void clearPatterns();
    
    /**
     * Configure pattern detection parameters
     * @param config New configuration
     */
    void configure(const PatternDetectionConfig& config);

private:
    // Configuration
    PatternDetectionConfig config_;
    bool initialized_;
    
    // Pattern storage
    std::vector<EnhancedBehaviorPattern> detectedPatterns_;
    std::map<String, size_t> patternIndex_;  // Pattern ID to index mapping
    
    // Behavior observation history
    std::vector<BehaviorResult> behaviorHistory_;
    std::vector<EnvironmentalData> environmentHistory_;
    
    // Performance metrics
    mutable AIMetrics detectionMetrics_;
    uint32_t totalDetections_;
    uint32_t successfulDetections_;
    
    // Pattern detection algorithms
    std::vector<EnhancedBehaviorPattern> detectSequencePatterns(
        const std::vector<BehaviorResult>& behaviors) const;
    std::vector<EnhancedBehaviorPattern> detectMarkovPatterns(
        const std::vector<BehaviorResult>& behaviors) const;
    std::vector<EnhancedBehaviorPattern> detectStatisticalPatterns(
        const std::vector<BehaviorResult>& behaviors) const;
    
    // Pattern analysis methods
    void analyzeTemporalCharacteristics(EnhancedBehaviorPattern& pattern,
                                      const std::vector<BehaviorResult>& observations) const;
    void analyzeEnvironmentalDependencies(EnhancedBehaviorPattern& pattern,
                                         const std::vector<EnvironmentalData>& environment) const;
    void calculateTransitionProbabilities(EnhancedBehaviorPattern& pattern,
                                         const std::vector<BehaviorResult>& behaviors) const;
    
    // Utility methods
    bool isSequenceMatch(const std::vector<BehaviorType>& sequence1,
                        const std::vector<BehaviorType>& sequence2,
                        float tolerance = 0.8f) const;
    float calculatePatternSimilarity(const EnhancedBehaviorPattern& pattern1,
                                   const EnhancedBehaviorPattern& pattern2) const;
    String generatePatternId(const std::vector<BehaviorType>& sequence) const;
    void mergePatterns(EnhancedBehaviorPattern& target,
                      const EnhancedBehaviorPattern& source);
    void pruneOldObservations();
    void updateDetectionMetrics(bool success, uint32_t processingTime);
};

#endif // BEHAVIOR_PATTERN_DETECTOR_H