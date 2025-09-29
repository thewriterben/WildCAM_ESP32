/**
 * @file conservation_insights.h
 * @brief Conservation Intelligence and Insights Generation
 * 
 * Provides conservation-focused analytics, population health monitoring,
 * and actionable insights for wildlife management and research.
 */

#ifndef CONSERVATION_INSIGHTS_H
#define CONSERVATION_INSIGHTS_H

#include "../ai_common.h"
#include "../behavior_analysis.h"
#include "behavior_pattern_detector.h"
#include "temporal_sequence_analyzer.h"
#include <vector>
#include <map>

/**
 * Conservation Priority Level
 */
enum class ConservationPriority {
    LOW = 0,                // Routine monitoring
    MEDIUM = 1,             // Increased attention needed
    HIGH = 2,               // Intervention may be required
    CRITICAL = 3,           // Immediate action needed
    EMERGENCY = 4           // Crisis situation
};

/**
 * Population Health Indicator
 */
struct HealthIndicator {
    String indicatorName;
    float currentValue;
    float historicalAverage;
    float threshold_warning;
    float threshold_critical;
    bool isImproving;
    bool isDeteriorating;
    
    ConservationPriority priority;
    String recommendation;
    
    HealthIndicator() : 
        currentValue(0.0f), historicalAverage(0.0f),
        threshold_warning(0.0f), threshold_critical(0.0f),
        isImproving(false), isDeteriorating(false),
        priority(ConservationPriority::LOW) {}
};

/**
 * Species Population Assessment
 */
struct PopulationAssessment {
    SpeciesType species;
    uint32_t estimatedPopulation;
    float populationTrend;              // Positive = increasing, negative = decreasing
    float confidenceInterval;           // Statistical confidence in estimate
    
    // Health metrics
    std::vector<HealthIndicator> healthIndicators;
    float overallHealthScore;           // 0.0-1.0 composite health score
    
    // Behavioral diversity
    float behavioralDiversity;          // Shannon diversity of behaviors
    uint32_t uniqueBehaviorsObserved;
    std::vector<BehaviorType> dominantBehaviors;
    std::vector<BehaviorType> rareBehaviors;
    
    // Stress indicators
    float avgStressLevel;
    float stressTrend;                  // Trend in stress over time
    std::vector<String> stressFactors;
    
    // Reproductive health
    bool breedingActivityDetected;
    float reproductiveSuccess;          // Estimated based on behavior
    uint8_t breedingSeasonMonth;
    
    PopulationAssessment() : 
        species(SpeciesType::UNKNOWN), estimatedPopulation(0),
        populationTrend(0.0f), confidenceInterval(0.0f),
        overallHealthScore(0.0f), behavioralDiversity(0.0f),
        uniqueBehaviorsObserved(0), avgStressLevel(0.0f),
        stressTrend(0.0f), breedingActivityDetected(false),
        reproductiveSuccess(0.0f), breedingSeasonMonth(0) {}
};

/**
 * Ecosystem Health Assessment
 */
struct EcosystemHealth {
    // Overall ecosystem metrics
    float biodiversityIndex;            // Species diversity in area
    float habitatQuality;              // Quality assessment based on behavior
    float environmentalStability;      // Stability of environmental conditions
    
    // Species interactions
    struct SpeciesInteraction {
        SpeciesType species1;
        SpeciesType species2;
        float interactionStrength;      // Strength of interaction
        bool isCompetitive;
        bool isPredatorPrey;
        bool isSymbiotic;
        
        SpeciesInteraction() : 
            species1(SpeciesType::UNKNOWN), species2(SpeciesType::UNKNOWN),
            interactionStrength(0.0f), isCompetitive(false),
            isPredatorPrey(false), isSymbiotic(false) {}
    };
    std::vector<SpeciesInteraction> interactions;
    
    // Threat assessment
    struct ThreatAssessment {
        String threatType;              // Human, climate, disease, etc.
        float severity;                 // 0.0-1.0 severity level
        float probability;              // 0.0-1.0 probability of occurrence
        String evidence;                // Evidence supporting threat
        std::vector<String> affectedSpecies;
        
        ThreatAssessment() : severity(0.0f), probability(0.0f) {}
    };
    std::vector<ThreatAssessment> threats;
    
    EcosystemHealth() : 
        biodiversityIndex(0.0f), habitatQuality(0.0f),
        environmentalStability(0.0f) {}
};

/**
 * Conservation Recommendation
 */
struct ConservationRecommendation {
    ConservationPriority priority;
    String title;
    String description;
    std::vector<String> actionItems;
    
    // Implementation details
    float estimatedCost;                // Relative cost estimate
    uint32_t timeframe_days;           // Recommended implementation timeframe
    float expectedImpact;              // Expected positive impact (0.0-1.0)
    
    // Supporting data
    std::vector<String> evidenceSources;
    float confidenceLevel;             // Confidence in recommendation
    
    ConservationRecommendation() : 
        priority(ConservationPriority::LOW), estimatedCost(0.0f),
        timeframe_days(0), expectedImpact(0.0f), confidenceLevel(0.0f) {}
};

/**
 * Migration Prediction
 */
struct MigrationPrediction {
    SpeciesType species;
    bool migrationExpected;
    uint8_t predictedStartMonth;
    uint8_t predictedEndMonth;
    float confidence;
    
    // Migration characteristics
    bool isInboundMigration;           // Arriving vs departing
    float estimatedPopulationSize;
    std::vector<String> triggerFactors; // Weather, food, etc.
    
    // Recommendations
    std::vector<String> monitoringRecommendations;
    std::vector<String> habitatPreparation;
    
    MigrationPrediction() : 
        species(SpeciesType::UNKNOWN), migrationExpected(false),
        predictedStartMonth(0), predictedEndMonth(0), confidence(0.0f),
        isInboundMigration(false), estimatedPopulationSize(0.0f) {}
};

/**
 * Feeding Optimization Data
 */
struct FeedingOptimization {
    SpeciesType species;
    
    // Optimal feeding times
    struct OptimalTime {
        uint8_t hour;                   // Hour of day (0-23)
        float probability;              // Probability of feeding
        float effectiveness;            // How effective feeding is at this time
        
        OptimalTime() : hour(0), probability(0.0f), effectiveness(0.0f) {}
    };
    std::vector<OptimalTime> dailyOptimalTimes;
    std::vector<OptimalTime> seasonalOptimalTimes;
    
    // Location recommendations
    struct LocationRecommendation {
        String locationDescription;
        float suitability;              // 0.0-1.0 suitability score
        std::vector<String> reasons;
        
        LocationRecommendation() : suitability(0.0f) {}
    };
    std::vector<LocationRecommendation> recommendedLocations;
    
    // Environmental conditions
    struct ConditionPreference {
        String conditionType;           // Weather, temperature, etc.
        String optimalValue;
        float importance;               // How important this condition is
        
        ConditionPreference() : importance(0.0f) {}
    };
    std::vector<ConditionPreference> optimalConditions;
    
    FeedingOptimization() : species(SpeciesType::UNKNOWN) {}
};

/**
 * Conservation Insights Generator
 * 
 * Analyzes wildlife behavior data to generate actionable conservation
 * insights, population health assessments, and management recommendations.
 */
class ConservationInsights {
public:
    /**
     * Constructor
     */
    ConservationInsights();
    
    /**
     * Destructor
     */
    ~ConservationInsights();
    
    /**
     * Initialize conservation insights system
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * Generate comprehensive conservation assessment
     * @param behaviorData Recent behavior observations
     * @param patterns Detected behavior patterns
     * @param temporal Temporal analysis results
     * @return Complete conservation assessment
     */
    std::vector<ConservationRecommendation> generateConservationAssessment(
        const std::vector<BehaviorResult>& behaviorData,
        const std::vector<EnhancedBehaviorPattern>& patterns,
        const TemporalAnalysisResult& temporal);
    
    /**
     * Assess population health for specific species
     * @param species Target species
     * @param timeWindow_s Analysis time window
     * @return Population health assessment
     */
    PopulationAssessment assessPopulationHealth(SpeciesType species,
                                              uint32_t timeWindow_s = 86400);
    
    /**
     * Generate ecosystem health report
     * @param timeWindow_s Analysis time window
     * @return Ecosystem health assessment
     */
    EcosystemHealth assessEcosystemHealth(uint32_t timeWindow_s = 604800); // 1 week default
    
    /**
     * Predict migration patterns
     * @param species Target species (UNKNOWN for all species)
     * @param monthsAhead Number of months to predict
     * @return Migration predictions
     */
    std::vector<MigrationPrediction> predictMigration(SpeciesType species = SpeciesType::UNKNOWN,
                                                     uint8_t monthsAhead = 6);
    
    /**
     * Generate feeding optimization recommendations
     * @param species Target species
     * @return Feeding optimization data
     */
    FeedingOptimization generateFeedingOptimization(SpeciesType species);
    
    /**
     * Detect conservation alerts
     * @param behaviorData Recent behavior observations
     * @return High-priority conservation alerts
     */
    std::vector<ConservationRecommendation> detectConservationAlerts(
        const std::vector<BehaviorResult>& behaviorData);
    
    /**
     * Calculate biodiversity metrics
     * @param timeWindow_s Analysis time window
     * @return Biodiversity index and related metrics
     */
    float calculateBiodiversityIndex(uint32_t timeWindow_s = 86400);
    
    /**
     * Assess habitat quality based on behavior patterns
     * @param patterns Detected behavior patterns
     * @return Habitat quality score (0.0-1.0)
     */
    float assessHabitatQuality(const std::vector<EnhancedBehaviorPattern>& patterns);
    
    /**
     * Generate research recommendations
     * @param currentData Available behavior data
     * @return Research priorities and recommendations
     */
    std::vector<String> generateResearchRecommendations(
        const std::vector<BehaviorResult>& currentData);
    
    /**
     * Export conservation data for research
     * @param filename Output filename
     * @param includeRawData Include raw behavioral data
     * @return true if export successful
     */
    bool exportConservationData(const char* filename, bool includeRawData = false);
    
    /**
     * Update behavior observations for insights generation
     * @param behavior New behavior observation
     * @param environment Environmental context
     */
    void updateBehaviorObservations(const BehaviorResult& behavior,
                                  const EnvironmentalData& environment);
    
    /**
     * Get conservation performance metrics
     * @return Insights generation metrics
     */
    AIMetrics getInsightsMetrics() const;
    
    /**
     * Clear all conservation data
     */
    void clearInsightsData();

private:
    bool initialized_;
    
    // Data storage
    std::vector<BehaviorResult> behaviorObservations_;
    std::vector<EnvironmentalData> environmentObservations_;
    std::map<SpeciesType, PopulationAssessment> populationAssessments_;
    
    // Cached assessments
    EcosystemHealth cachedEcosystemHealth_;
    unsigned long lastEcosystemUpdate_;
    
    // Performance metrics
    mutable AIMetrics insightsMetrics_;
    uint32_t totalAssessments_;
    uint32_t successfulAssessments_;
    
    // Analysis methods
    void analyzePopulationTrends(PopulationAssessment& assessment,
                               const std::vector<BehaviorResult>& data);
    void calculateHealthIndicators(PopulationAssessment& assessment,
                                 const std::vector<BehaviorResult>& data);
    void assessStressLevels(PopulationAssessment& assessment,
                          const std::vector<BehaviorResult>& data);
    void detectBreedingActivity(PopulationAssessment& assessment,
                              const std::vector<BehaviorResult>& data);
    
    // Ecosystem analysis
    void analyzeSpeciesInteractions(EcosystemHealth& ecosystem,
                                  const std::vector<BehaviorResult>& data);
    void assessEnvironmentalThreats(EcosystemHealth& ecosystem,
                                   const std::vector<EnvironmentalData>& environment);
    void calculateHabitatMetrics(EcosystemHealth& ecosystem,
                               const std::vector<BehaviorResult>& data);
    
    // Migration analysis
    bool analyzeHistoricalMigration(SpeciesType species, MigrationPrediction& prediction);
    void correlateEnvironmentalFactors(MigrationPrediction& prediction,
                                     const std::vector<EnvironmentalData>& environment);
    
    // Feeding optimization
    void analyzeOptimalFeedingTimes(FeedingOptimization& optimization,
                                  const std::vector<BehaviorResult>& data);
    void generateLocationRecommendations(FeedingOptimization& optimization,
                                       const std::vector<BehaviorResult>& data);
    
    // Utility methods
    ConservationPriority calculatePriority(float severity, float urgency);
    float calculateSpeciesDiversity(const std::vector<BehaviorResult>& data);
    float calculateBehavioralEntropy(const std::vector<BehaviorType>& behaviors);
    void updateInsightsMetrics(bool success, uint32_t processingTime);
    void pruneOldObservations();
};

#endif // CONSERVATION_INSIGHTS_H