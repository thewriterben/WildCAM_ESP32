/**
 * @file temporal_sequence_analyzer.h
 * @brief Temporal Sequence Analysis for Wildlife Behavior
 * 
 * Implements sliding window algorithms and temporal analysis for
 * understanding wildlife behavior patterns over time.
 */

#ifndef TEMPORAL_SEQUENCE_ANALYZER_H
#define TEMPORAL_SEQUENCE_ANALYZER_H

#include "../ai_common.h"
#include "../behavior_analysis.h"
#include <vector>
#include <deque>
#include <map>

/**
 * Temporal Analysis Window Configuration
 */
struct TemporalWindow {
    uint32_t windowSize_s;              // Window size in seconds
    uint32_t stepSize_s;                // Step size for sliding window
    uint32_t minObservations;           // Minimum observations per window
    bool overlapping;                   // Whether windows can overlap
    
    TemporalWindow() : 
        windowSize_s(300), stepSize_s(60), minObservations(3), overlapping(true) {}
    
    TemporalWindow(uint32_t window, uint32_t step, uint32_t minObs, bool overlap) :
        windowSize_s(window), stepSize_s(step), minObservations(minObs), overlapping(overlap) {}
};

/**
 * Temporal Behavior Sequence
 */
struct TemporalSequence {
    std::vector<BehaviorResult> behaviors;
    TemporalWindow window;
    unsigned long startTime;
    unsigned long endTime;
    
    // Sequence characteristics
    float sequenceCoherence;            // How consistent the sequence is
    float activityLevel;               // Overall activity in sequence
    BehaviorType dominantBehavior;     // Most frequent behavior
    uint32_t behaviorTransitions;      // Number of behavior changes
    
    // Statistical properties
    struct {
        float mean_duration;
        float std_duration;
        float entropy;                  // Behavioral entropy
        float predictability;          // How predictable the sequence is
    } statistics;
    
    TemporalSequence() : 
        startTime(0), endTime(0), sequenceCoherence(0.0f),
        activityLevel(0.0f), dominantBehavior(BehaviorType::UNKNOWN),
        behaviorTransitions(0) {
        memset(&statistics, 0, sizeof(statistics));
    }
};

/**
 * Circadian Rhythm Analysis
 */
struct CircadianAnalysis {
    // 24-hour activity pattern
    float hourlyActivity[24];           // Activity level by hour
    float hourlyBehaviorProb[24][11];   // Behavior probability by hour (11 behavior types)
    
    // Peak activity periods
    struct ActivityPeak {
        uint8_t startHour;
        uint8_t endHour;
        float intensity;
        BehaviorType primaryBehavior;
        
        ActivityPeak() : startHour(0), endHour(0), intensity(0.0f),
                        primaryBehavior(BehaviorType::UNKNOWN) {}
    };
    std::vector<ActivityPeak> peakPeriods;
    
    // Circadian metrics
    float circadianStrength;            // How strong the 24h pattern is
    float circadianPhase;              // Phase shift from sunrise
    bool isDiurnal;                    // Day-active pattern
    bool isNocturnal;                  // Night-active pattern
    bool isCrepuscular;                // Dawn/dusk active pattern
    
    CircadianAnalysis() : 
        circadianStrength(0.0f), circadianPhase(0.0f),
        isDiurnal(false), isNocturnal(false), isCrepuscular(false) {
        memset(hourlyActivity, 0, sizeof(hourlyActivity));
        memset(hourlyBehaviorProb, 0, sizeof(hourlyBehaviorProb));
    }
};

/**
 * Seasonal Pattern Analysis
 */
struct SeasonalAnalysis {
    // Monthly activity patterns
    float monthlyActivity[12];          // Activity level by month
    float monthlyBehaviorProb[12][11];  // Behavior probability by month
    
    // Migration indicators
    struct MigrationPattern {
        uint8_t startMonth;
        uint8_t endMonth;
        float intensity;
        BehaviorType associatedBehavior;
        bool isInbound;                 // Arrival vs departure
        
        MigrationPattern() : startMonth(0), endMonth(0), intensity(0.0f),
                            associatedBehavior(BehaviorType::UNKNOWN), isInbound(false) {}
    };
    std::vector<MigrationPattern> migrationPatterns;
    
    // Breeding/mating seasons
    struct BreedingSeason {
        uint8_t peakMonth;
        uint8_t durationMonths;
        float intensity;
        
        BreedingSeason() : peakMonth(0), durationMonths(0), intensity(0.0f) {}
    };
    BreedingSeason breedingSeason;
    
    // Seasonal characteristics
    float seasonalStrength;             // How strong seasonal patterns are
    bool hasMigration;                 // Shows migration behavior
    bool hasBreedingSeason;            // Shows breeding behavior
    
    SeasonalAnalysis() : 
        seasonalStrength(0.0f), hasMigration(false), hasBreedingSeason(false) {
        memset(monthlyActivity, 0, sizeof(monthlyActivity));
        memset(monthlyBehaviorProb, 0, sizeof(monthlyBehaviorProb));
    }
};

/**
 * Temporal Analysis Configuration
 */
struct TemporalAnalysisConfig {
    // Window configurations
    TemporalWindow shortTerm;           // 5-30 seconds
    TemporalWindow mediumTerm;          // 1-10 minutes
    TemporalWindow longTerm;            // Hours to days
    
    // Analysis parameters
    float coherenceThreshold;           // Minimum coherence for valid sequence
    float activityThreshold;           // Minimum activity level
    uint32_t maxSequenceLength;        // Maximum behaviors per sequence
    
    // Circadian analysis
    bool enableCircadianAnalysis;
    uint32_t circadianDataPoints;      // Minimum data points for analysis
    
    // Seasonal analysis
    bool enableSeasonalAnalysis;
    uint32_t seasonalDataPoints;       // Minimum data points for analysis
    
    TemporalAnalysisConfig() :
        shortTerm(30, 5, 3, true),
        mediumTerm(600, 60, 5, true),
        longTerm(3600, 300, 10, false),
        coherenceThreshold(0.6f), activityThreshold(0.1f),
        maxSequenceLength(100), enableCircadianAnalysis(true),
        circadianDataPoints(50), enableSeasonalAnalysis(true),
        seasonalDataPoints(200) {}
};

/**
 * Temporal Analysis Result
 */
struct TemporalAnalysisResult {
    std::vector<TemporalSequence> sequences;
    CircadianAnalysis circadianPattern;
    SeasonalAnalysis seasonalPattern;
    
    // Trend analysis
    struct {
        bool activityIncreasing;
        bool activityDecreasing;
        float trendStrength;            // 0.0-1.0 strength of trend
        BehaviorType emergingBehavior;  // New behavior appearing
        BehaviorType decliningBehavior; // Behavior becoming less common
    } trends;
    
    // Prediction readiness
    bool sufficientDataForPrediction;
    float predictionConfidence;
    uint32_t dataQualityScore;         // 0-100 quality assessment
    
    TemporalAnalysisResult() : 
        sufficientDataForPrediction(false), predictionConfidence(0.0f),
        dataQualityScore(0) {
        memset(&trends, 0, sizeof(trends));
        trends.emergingBehavior = trends.decliningBehavior = BehaviorType::UNKNOWN;
    }
};

/**
 * Temporal Sequence Analyzer
 * 
 * Analyzes wildlife behavior sequences over time using sliding window
 * algorithms to identify patterns, trends, and temporal relationships.
 */
class TemporalSequenceAnalyzer {
public:
    /**
     * Constructor
     */
    TemporalSequenceAnalyzer();
    
    /**
     * Destructor
     */
    ~TemporalSequenceAnalyzer();
    
    /**
     * Initialize temporal analyzer
     * @param config Analysis configuration
     * @return true if initialization successful
     */
    bool init(const TemporalAnalysisConfig& config = TemporalAnalysisConfig());
    
    /**
     * Add behavior observation to temporal analysis
     * @param behavior Behavior observation
     * @param environment Environmental context
     */
    void addObservation(const BehaviorResult& behavior,
                       const EnvironmentalData& environment);
    
    /**
     * Analyze temporal patterns in behavior data
     * @param timeWindow_s Analysis time window (0 = all data)
     * @return Temporal analysis results
     */
    TemporalAnalysisResult analyzeSequences(uint32_t timeWindow_s = 0);
    
    /**
     * Get behavior sequence for specific time window
     * @param window Temporal window configuration
     * @param endTime End time for analysis (0 = current time)
     * @return Temporal sequence
     */
    TemporalSequence getSequence(const TemporalWindow& window,
                                unsigned long endTime = 0);
    
    /**
     * Predict next behavior in sequence
     * @param recentSequence Recent behavior sequence
     * @param timeHorizon_s Prediction time horizon
     * @return Predicted behavior and confidence
     */
    std::pair<BehaviorType, float> predictNextInSequence(
        const TemporalSequence& recentSequence,
        uint32_t timeHorizon_s = 300);
    
    /**
     * Analyze circadian behavior patterns
     * @param minDays Minimum days of data required
     * @return Circadian pattern analysis
     */
    CircadianAnalysis analyzeCircadianPattern(uint32_t minDays = 3);
    
    /**
     * Analyze seasonal behavior patterns
     * @param minMonths Minimum months of data required
     * @return Seasonal pattern analysis
     */
    SeasonalAnalysis analyzeSeasonalPattern(uint32_t minMonths = 3);
    
    /**
     * Detect behavior anomalies in temporal sequence
     * @param sequence Sequence to analyze
     * @param threshold Anomaly detection threshold
     * @return Vector of anomalous behavior indices
     */
    std::vector<uint32_t> detectTemporalAnomalies(const TemporalSequence& sequence,
                                                 float threshold = 0.8f);
    
    /**
     * Calculate sequence coherence score
     * @param sequence Temporal sequence
     * @return Coherence score (0.0-1.0)
     */
    float calculateSequenceCoherence(const TemporalSequence& sequence) const;
    
    /**
     * Get behavior transition matrix
     * @param timeWindow_s Analysis time window
     * @return Transition probability matrix
     */
    std::map<std::pair<BehaviorType, BehaviorType>, float> getTransitionMatrix(
        uint32_t timeWindow_s = 86400);
    
    /**
     * Export temporal analysis data
     * @param filename Output filename
     * @param includeRawData Include raw observations
     * @return true if export successful
     */
    bool exportTemporalData(const char* filename, bool includeRawData = false);
    
    /**
     * Get analysis performance metrics
     * @return Performance metrics
     */
    AIMetrics getAnalysisMetrics() const;
    
    /**
     * Clear all temporal data
     */
    void clearData();
    
    /**
     * Configure analysis parameters
     * @param config New configuration
     */
    void configure(const TemporalAnalysisConfig& config);
    
    /**
     * Get data sufficiency for analysis type
     * @param analysisType Type of analysis (0=short, 1=medium, 2=long)
     * @return true if sufficient data available
     */
    bool hasSufficientData(uint8_t analysisType) const;

private:
    // Configuration
    TemporalAnalysisConfig config_;
    bool initialized_;
    
    // Data storage
    std::deque<BehaviorResult> behaviorHistory_;
    std::deque<EnvironmentalData> environmentHistory_;
    std::deque<unsigned long> timestampHistory_;
    
    // Cached analysis results
    CircadianAnalysis cachedCircadianAnalysis_;
    SeasonalAnalysis cachedSeasonalAnalysis_;
    unsigned long lastCircadianUpdate_;
    unsigned long lastSeasonalUpdate_;
    
    // Performance metrics
    mutable AIMetrics analysisMetrics_;
    uint32_t totalAnalyses_;
    uint32_t successfulAnalyses_;
    
    // Analysis methods
    std::vector<TemporalSequence> extractSequences(const TemporalWindow& window,
                                                  uint32_t timeWindow_s);
    void analyzeSequenceStatistics(TemporalSequence& sequence);
    void calculateBehaviorEntropy(TemporalSequence& sequence);
    
    // Circadian analysis methods
    void updateCircadianData(const BehaviorResult& behavior);
    void calculateCircadianMetrics(CircadianAnalysis& analysis);
    void identifyActivityPeaks(CircadianAnalysis& analysis);
    
    // Seasonal analysis methods
    void updateSeasonalData(const BehaviorResult& behavior);
    void calculateSeasonalMetrics(SeasonalAnalysis& analysis);
    void identifyMigrationPatterns(SeasonalAnalysis& analysis);
    void identifyBreedingSeasons(SeasonalAnalysis& analysis);
    
    // Utility methods
    void pruneOldData();
    uint8_t getHourOfDay(unsigned long timestamp) const;
    uint8_t getMonthOfYear(unsigned long timestamp) const;
    float calculatePredictability(const std::vector<BehaviorType>& sequence) const;
    void updateAnalysisMetrics(bool success, uint32_t processingTime);
};

#endif // TEMPORAL_SEQUENCE_ANALYZER_H