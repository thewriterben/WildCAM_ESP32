/**
 * @file predictive_analytics_engine.h
 * @brief Predictive Wildlife Behavior Analytics Engine
 * 
 * Implements advanced predictive analytics capabilities for wildlife behavior
 * monitoring, including behavior prediction, pattern recognition, and
 * conservation insights generation.
 */

#ifndef PREDICTIVE_ANALYTICS_ENGINE_H
#define PREDICTIVE_ANALYTICS_ENGINE_H

#include "../ai_common.h"
#include "../vision/behavior_analyzer.h"
#include "behavior_pattern_detector.h"
#include "temporal_sequence_analyzer.h"
#include "conservation_insights.h"
#include "data/behavior_database.h"
#include "data/pattern_cache.h"
#include <memory>
#include <vector>

/**
 * Prediction Result Structure
 */
struct PredictionResult {
    BehaviorType predictedBehavior;
    float confidence;
    uint32_t timeToNext_s;              // Predicted time to next behavior
    std::vector<BehaviorType> sequence; // Predicted behavior sequence
    float patternConfidence;            // Confidence in pattern match
    bool isAnomalous;                   // Indicates unusual behavior
    
    // Environmental influence
    bool environmentInfluenced;         // Weather/time affecting prediction
    float environmentalFactor;          // Strength of environmental influence
    
    PredictionResult() : 
        predictedBehavior(BehaviorType::UNKNOWN),
        confidence(0.0f), timeToNext_s(0),
        patternConfidence(0.0f), isAnomalous(false),
        environmentInfluenced(false), environmentalFactor(0.0f) {}
};

/**
 * Wildlife Population Health Metrics
 */
struct PopulationHealthMetrics {
    float overallHealthScore;           // 0.0-1.0 population health
    float stressLevelTrend;            // Trending stress indicator
    uint32_t activePopulationCount;    // Estimated active animals
    float diversityIndex;              // Behavior diversity metric
    
    // Activity patterns
    struct {
        float peakActivityHours[24];   // Activity by hour of day
        float seasonalActivity[12];    // Activity by month
        float migrationIndicator;      // Migration behavior strength
    } activityMetrics;
    
    PopulationHealthMetrics() : 
        overallHealthScore(0.0f), stressLevelTrend(0.0f),
        activePopulationCount(0), diversityIndex(0.0f) {
        memset(&activityMetrics, 0, sizeof(activityMetrics));
    }
};

/**
 * Enhanced Wildlife Analysis Result with Predictions
 */
struct PredictiveAnalysisResult {
    // Original analysis data
    BehaviorResult currentBehavior;
    
    // Predictive analytics
    PredictionResult nextBehaviorPrediction;
    std::vector<BehaviorPattern> matchedPatterns;
    PopulationHealthMetrics populationMetrics;
    
    // Conservation insights
    struct {
        bool criticalBehaviorDetected;
        bool feedingOpportunityOptimal;
        bool migrationEventPredicted;
        float conservationPriority;     // 0.0-1.0 priority score
        String conservationRecommendation;
    } conservationInsights;
    
    // Analytics metadata
    uint32_t analysisDuration_ms;
    float predictionAccuracy;           // Historical accuracy for this pattern
    unsigned long timestamp;
    
    PredictiveAnalysisResult() : 
        analysisDuration_ms(0), predictionAccuracy(0.0f), timestamp(0) {
        memset(&conservationInsights, 0, sizeof(conservationInsights));
        conservationInsights.conservationPriority = 0.0f;
    }
};

/**
 * Predictive Analytics Configuration
 */
struct PredictiveConfig {
    bool enableBehaviorPrediction;
    bool enablePatternDetection;
    bool enableAnomalyDetection;
    bool enableConservationInsights;
    
    // Analysis windows
    uint32_t shortTermWindow_s;         // For immediate predictions
    uint32_t mediumTermWindow_s;        // For pattern detection
    uint32_t longTermWindow_s;          // For population analysis
    
    // Thresholds
    float predictionConfidenceThreshold;
    float anomalyDetectionThreshold;
    float patternMatchThreshold;
    
    // Performance limits
    uint32_t maxPredictionHistory;
    uint32_t maxPatternCache;
    bool enablePowerOptimization;
    
    PredictiveConfig() :
        enableBehaviorPrediction(true), enablePatternDetection(true),
        enableAnomalyDetection(true), enableConservationInsights(true),
        shortTermWindow_s(300), mediumTermWindow_s(3600), longTermWindow_s(86400),
        predictionConfidenceThreshold(0.7f), anomalyDetectionThreshold(0.8f),
        patternMatchThreshold(0.8f), maxPredictionHistory(1000),
        maxPatternCache(50), enablePowerOptimization(true) {}
};

/**
 * Main Predictive Analytics Engine
 * 
 * Coordinates all predictive analytics components to provide comprehensive
 * wildlife behavior prediction and conservation insights.
 */
class PredictiveAnalyticsEngine {
public:
    /**
     * Constructor
     */
    PredictiveAnalyticsEngine();
    
    /**
     * Destructor
     */
    ~PredictiveAnalyticsEngine();
    
    /**
     * Initialize predictive analytics engine
     * @param config Configuration parameters
     * @return true if initialization successful
     */
    bool init(const PredictiveConfig& config = PredictiveConfig());
    
    /**
     * Process new behavior data and generate predictions
     * @param behavior Current behavior analysis result
     * @param environment Environmental context
     * @return Comprehensive predictive analysis result
     */
    PredictiveAnalysisResult analyzeBehavior(const BehaviorResult& behavior,
                                           const EnvironmentalData& environment);
    
    /**
     * Update behavior prediction models with validation data
     * @param actualBehavior Observed behavior for model validation
     * @param previousPrediction Previous prediction to validate
     */
    void updatePredictionModels(const BehaviorResult& actualBehavior,
                               const PredictionResult& previousPrediction);
    
    /**
     * Get population health metrics
     * @param timeWindow_s Analysis time window in seconds
     * @return Population health assessment
     */
    PopulationHealthMetrics getPopulationHealth(uint32_t timeWindow_s = 86400);
    
    /**
     * Predict optimal feeding times for wildlife
     * @param species Target species type
     * @param daysAhead Number of days to predict
     * @return Optimal feeding time predictions
     */
    std::vector<uint32_t> predictOptimalFeedingTimes(SpeciesType species, uint8_t daysAhead = 7);
    
    /**
     * Detect migration patterns and predict movement
     * @param species Target species for migration analysis
     * @return Migration prediction data
     */
    PredictionResult predictMigrationPattern(SpeciesType species);
    
    /**
     * Export prediction data for research
     * @param filename Output filename
     * @param includeRawData Include raw behavioral data
     * @return true if export successful
     */
    bool exportPredictionData(const char* filename, bool includeRawData = false);
    
    /**
     * Get prediction accuracy statistics
     * @return Current prediction accuracy metrics
     */
    AIMetrics getPredictionMetrics() const;
    
    /**
     * Reset all predictive models and data
     */
    void resetPredictiveModels();
    
    /**
     * Configure prediction parameters
     * @param config New configuration
     */
    void configure(const PredictiveConfig& config);
    
    /**
     * Check if system is ready for predictions
     * @return true if sufficient data available for predictions
     */
    bool isReadyForPredictions() const;

private:
    // Core components
    std::unique_ptr<BehaviorPatternDetector> patternDetector_;
    std::unique_ptr<TemporalSequenceAnalyzer> sequenceAnalyzer_;
    std::unique_ptr<ConservationInsights> conservationAnalyzer_;
    std::unique_ptr<BehaviorDatabase> behaviorDatabase_;
    std::unique_ptr<PatternCache> patternCache_;
    
    // Configuration
    PredictiveConfig config_;
    bool initialized_;
    
    // Performance tracking
    mutable AIMetrics predictionMetrics_;
    uint32_t totalPredictions_;
    uint32_t accuratePredictions_;
    
    // Internal analysis methods
    PredictionResult generateBehaviorPrediction(const BehaviorResult& current,
                                              const std::vector<BehaviorResult>& history);
    bool detectBehaviorAnomaly(const BehaviorResult& behavior,
                              const std::vector<BehaviorPattern>& patterns);
    void updatePatternCache(const std::vector<BehaviorPattern>& newPatterns);
    void validatePredictionAccuracy(const BehaviorResult& actual,
                                   const PredictionResult& predicted);
    
    // Utility methods
    float calculateEnvironmentalInfluence(const EnvironmentalData& environment,
                                         const BehaviorPattern& pattern);
    uint32_t estimateTimeToNextBehavior(const BehaviorPattern& pattern);
    void optimizeForPowerConsumption();
};

#endif // PREDICTIVE_ANALYTICS_ENGINE_H