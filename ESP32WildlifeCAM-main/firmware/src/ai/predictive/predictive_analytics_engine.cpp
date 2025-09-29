/**
 * @file predictive_analytics_engine.cpp
 * @brief Implementation of Predictive Wildlife Behavior Analytics Engine
 */

#include "predictive_analytics_engine.h"
#include "../tinyml/inference_engine.h"

PredictiveAnalyticsEngine::PredictiveAnalyticsEngine() : 
    initialized_(false), totalPredictions_(0), accuratePredictions_(0) {
}

PredictiveAnalyticsEngine::~PredictiveAnalyticsEngine() {
    // Destructors of unique_ptr members will be called automatically
}

bool PredictiveAnalyticsEngine::init(const PredictiveConfig& config) {
    config_ = config;
    
    // Initialize core components
    patternDetector_ = std::make_unique<BehaviorPatternDetector>();
    if (!patternDetector_->init()) {
        return false;
    }
    
    sequenceAnalyzer_ = std::make_unique<TemporalSequenceAnalyzer>();
    if (!sequenceAnalyzer_->init()) {
        return false;
    }
    
    conservationAnalyzer_ = std::make_unique<ConservationInsights>();
    if (!conservationAnalyzer_->init()) {
        return false;
    }
    
    behaviorDatabase_ = std::make_unique<BehaviorDatabase>();
    if (!behaviorDatabase_->init(512, true)) { // 512KB, compression enabled
        return false;
    }
    
    patternCache_ = std::make_unique<PatternCache>();
    if (!patternCache_->init()) {
        return false;
    }
    
    // Initialize metrics
    predictionMetrics_ = AIMetrics();
    
    initialized_ = true;
    return true;
}

PredictiveAnalysisResult PredictiveAnalyticsEngine::analyzeBehavior(
    const BehaviorResult& behavior,
    const EnvironmentalData& environment) {
    
    PredictiveAnalysisResult result;
    
    if (!initialized_) {
        return result;
    }
    
    uint32_t startTime = millis();
    
    // Store behavior in database
    behaviorDatabase_->storeBehavior(behavior, environment);
    
    // Update sequence analyzer
    sequenceAnalyzer_->addObservation(behavior, environment);
    
    // Update pattern detector
    patternDetector_->addBehaviorObservation(behavior, environment);
    
    // Set current behavior
    result.currentBehavior = behavior;
    result.timestamp = behavior.timestamp;
    
    if (!isReadyForPredictions()) {
        result.analysisDuration_ms = millis() - startTime;
        return result;
    }
    
    // Get recent behavior history
    std::vector<BehaviorResult> recentBehaviors = 
        behaviorDatabase_->getRecentBehaviors(SpeciesType::UNKNOWN, config_.shortTermWindow_s, 20);
    
    // Generate behavior prediction if enabled
    if (config_.enableBehaviorPrediction && recentBehaviors.size() >= 3) {
        result.nextBehaviorPrediction = generateBehaviorPrediction(behavior, recentBehaviors);
    }
    
    // Detect patterns if enabled
    if (config_.enablePatternDetection) {
        PatternDetectionResult patternResult = 
            patternDetector_->detectPatterns(recentBehaviors, environment);
        result.matchedPatterns = patternResult.detectedPatterns;
        
        // Update pattern cache
        updatePatternCache(patternResult.detectedPatterns);
    }
    
    // Anomaly detection if enabled
    if (config_.enableAnomalyDetection && !result.matchedPatterns.empty()) {
        bool isAnomalous = detectBehaviorAnomaly(behavior, result.matchedPatterns);
        result.nextBehaviorPrediction.isAnomalous = isAnomalous;
    }
    
    // Generate conservation insights if enabled
    if (config_.enableConservationInsights) {
        TemporalAnalysisResult temporalResult = sequenceAnalyzer_->analyzeSequences(config_.mediumTermWindow_s);
        
        std::vector<ConservationRecommendation> recommendations = 
            conservationAnalyzer_->generateConservationAssessment(
                recentBehaviors, result.matchedPatterns, temporalResult);
        
        if (!recommendations.empty()) {
            result.conservationInsights.criticalBehaviorDetected = 
                recommendations[0].priority >= ConservationPriority::HIGH;
            result.conservationInsights.conservationPriority = 
                static_cast<float>(recommendations[0].priority) / 4.0f;
            result.conservationInsights.conservationRecommendation = recommendations[0].title;
        }
        
        // Get population metrics
        result.populationMetrics = getPopulationHealth(config_.mediumTermWindow_s);
    }
    
    // Calculate prediction accuracy
    if (totalPredictions_ > 0) {
        result.predictionAccuracy = static_cast<float>(accuratePredictions_) / totalPredictions_;
    }
    
    // Power optimization if enabled
    if (config_.enablePowerOptimization) {
        optimizeForPowerConsumption();
    }
    
    result.analysisDuration_ms = millis() - startTime;
    
    // Update metrics
    predictionMetrics_.totalInferences++;
    predictionMetrics_.inferenceTime = result.analysisDuration_ms;
    
    return result;
}

PredictionResult PredictiveAnalyticsEngine::generateBehaviorPrediction(
    const BehaviorResult& current,
    const std::vector<BehaviorResult>& history) {
    
    PredictionResult prediction;
    
    if (history.size() < 2) {
        return prediction;
    }
    
    // Extract behavior sequence
    std::vector<BehaviorType> sequence;
    for (const auto& behavior : history) {
        sequence.push_back(behavior.primaryBehavior);
    }
    
    // Use pattern detector for prediction
    auto nextBehavior = patternDetector_->predictNextBehavior(sequence, EnvironmentalData());
    
    prediction.predictedBehavior = nextBehavior.first;
    prediction.confidence = nextBehavior.second;
    
    // Estimate time to next behavior based on patterns
    if (prediction.confidence > config_.predictionConfidenceThreshold) {
        // Simple estimation based on recent behavior durations
        uint32_t avgDuration = 0;
        for (const auto& behavior : history) {
            avgDuration += behavior.duration;
        }
        if (history.size() > 0) {
            avgDuration /= history.size();
            prediction.timeToNext_s = avgDuration;
        }
    }
    
    totalPredictions_++;
    
    return prediction;
}

bool PredictiveAnalyticsEngine::detectBehaviorAnomaly(
    const BehaviorResult& behavior,
    const std::vector<BehaviorPattern>& patterns) {
    
    if (patterns.empty()) {
        return false;
    }
    
    // Check if current behavior fits any known patterns
    for (const auto& pattern : patterns) {
        bool matchesPattern = false;
        for (const auto& expectedBehavior : pattern.sequence) {
            if (expectedBehavior == behavior.primaryBehavior) {
                matchesPattern = true;
                break;
            }
        }
        
        if (matchesPattern && pattern.confidence > config_.anomalyDetectionThreshold) {
            return false; // Not anomalous if it matches a high-confidence pattern
        }
    }
    
    // If confidence is very low or doesn't match any pattern, consider anomalous
    return behavior.confidence < 0.3f || patterns.empty();
}

void PredictiveAnalyticsEngine::updatePatternCache(
    const std::vector<BehaviorPattern>& newPatterns) {
    
    // Convert BehaviorPattern to EnhancedBehaviorPattern and cache
    for (const auto& pattern : newPatterns) {
        EnhancedBehaviorPattern enhancedPattern;
        enhancedPattern.patternName = pattern.patternName;
        enhancedPattern.sequence = pattern.sequence;
        enhancedPattern.confidence = pattern.confidence;
        enhancedPattern.observationCount = pattern.observationCount;
        enhancedPattern.temporal.averageDuration_s = pattern.averageDuration_s;
        enhancedPattern.conservation.criticalForSurvival = pattern.criticalForSurvival;
        enhancedPattern.conservation.indicatesHealth = pattern.indicatesHealth;
        enhancedPattern.conservation.indicatesStress = pattern.indicatesStress;
        
        patternCache_->addPattern(enhancedPattern);
    }
}

PopulationHealthMetrics PredictiveAnalyticsEngine::getPopulationHealth(uint32_t timeWindow_s) {
    PopulationHealthMetrics metrics;
    
    if (!initialized_) {
        return metrics;
    }
    
    // Get recent behavior data
    std::vector<BehaviorResult> recentBehaviors = 
        behaviorDatabase_->getRecentBehaviors(SpeciesType::UNKNOWN, timeWindow_s, 1000);
    
    if (recentBehaviors.empty()) {
        return metrics;
    }
    
    // Calculate overall health score based on behavior diversity and stress levels
    std::map<BehaviorType, uint32_t> behaviorCounts;
    float totalStress = 0.0f;
    uint32_t validStressMeasurements = 0;
    
    for (const auto& behavior : recentBehaviors) {
        behaviorCounts[behavior.primaryBehavior]++;
        if (behavior.stress_level > 0.0f) {
            totalStress += behavior.stress_level;
            validStressMeasurements++;
        }
    }
    
    // Calculate behavioral diversity (Shannon index)
    float diversity = 0.0f;
    uint32_t totalBehaviors = recentBehaviors.size();
    for (const auto& pair : behaviorCounts) {
        float proportion = static_cast<float>(pair.second) / totalBehaviors;
        if (proportion > 0) {
            diversity -= proportion * log2f(proportion);
        }
    }
    
    metrics.diversityIndex = diversity;
    metrics.activePopulationCount = recentBehaviors.size(); // Simplified estimate
    
    // Calculate stress trend (simplified)
    if (validStressMeasurements > 0) {
        float avgStress = totalStress / validStressMeasurements;
        metrics.stressLevelTrend = avgStress;
    }
    
    // Overall health score combines diversity and low stress
    metrics.overallHealthScore = (diversity / 3.0f) * (1.0f - metrics.stressLevelTrend);
    metrics.overallHealthScore = constrain(metrics.overallHealthScore, 0.0f, 1.0f);
    
    return metrics;
}

void PredictiveAnalyticsEngine::updatePredictionModels(
    const BehaviorResult& actualBehavior,
    const PredictionResult& previousPrediction) {
    
    if (previousPrediction.predictedBehavior == BehaviorType::UNKNOWN) {
        return;
    }
    
    // Check if prediction was accurate
    bool wasAccurate = (actualBehavior.primaryBehavior == previousPrediction.predictedBehavior);
    
    if (wasAccurate) {
        accuratePredictions_++;
    }
    
    // Update pattern detector confidence
    // This would need the pattern ID that made the prediction
    // For now, we'll just track overall accuracy
    
    validatePredictionAccuracy(actualBehavior, previousPrediction);
}

void PredictiveAnalyticsEngine::validatePredictionAccuracy(
    const BehaviorResult& actual,
    const PredictionResult& predicted) {
    
    // Update prediction metrics
    predictionMetrics_.successfulInferences++;
    
    // Time-based accuracy validation
    if (predicted.timeToNext_s > 0) {
        // Check if timing was reasonably accurate (within 20% tolerance)
        uint32_t actualDuration = actual.duration;
        uint32_t predictedDuration = predicted.timeToNext_s;
        float error = abs(static_cast<int>(actualDuration - predictedDuration)) / 
                     static_cast<float>(predictedDuration);
        
        if (error < 0.2f) { // Within 20% tolerance
            predictionMetrics_.totalInferences++; // Count as successful timing prediction
        }
    }
}

std::vector<uint32_t> PredictiveAnalyticsEngine::predictOptimalFeedingTimes(
    SpeciesType species, uint8_t daysAhead) {
    
    std::vector<uint32_t> optimalTimes;
    
    if (!initialized_ || !conservationAnalyzer_) {
        return optimalTimes;
    }
    
    FeedingOptimization feedingData = conservationAnalyzer_->generateFeedingOptimization(species);
    
    // Convert feeding optimization data to time predictions
    for (const auto& optimalTime : feedingData.dailyOptimalTimes) {
        if (optimalTime.effectiveness > 0.7f) { // High effectiveness threshold
            // Generate times for the requested number of days
            uint32_t baseTime = millis() / 1000; // Current time in seconds
            uint32_t secondsPerDay = 86400;
            
            for (uint8_t day = 0; day < daysAhead; day++) {
                uint32_t dayStart = baseTime + (day * secondsPerDay);
                // Set to the optimal hour
                uint32_t optimalTimeOfDay = (dayStart / secondsPerDay) * secondsPerDay + 
                                          (optimalTime.hour * 3600);
                optimalTimes.push_back(optimalTimeOfDay);
            }
        }
    }
    
    return optimalTimes;
}

PredictionResult PredictiveAnalyticsEngine::predictMigrationPattern(SpeciesType species) {
    PredictionResult prediction;
    
    if (!initialized_ || !conservationAnalyzer_) {
        return prediction;
    }
    
    // Get migration predictions from conservation analyzer
    std::vector<MigrationPrediction> migrations = 
        conservationAnalyzer_->predictMigration(species, 6);
    
    if (!migrations.empty() && migrations[0].migrationExpected) {
        prediction.predictedBehavior = BehaviorType::MOVING; // Migration is movement
        prediction.confidence = migrations[0].confidence;
        prediction.environmentInfluenced = !migrations[0].triggerFactors.empty();
        prediction.environmentalFactor = 0.8f; // Migration is highly environment-dependent
    }
    
    return prediction;
}

bool PredictiveAnalyticsEngine::exportPredictionData(const char* filename, bool includeRawData) {
    if (!initialized_) {
        return false;
    }
    
    return behaviorDatabase_->exportDatabase(filename, "json");
}

AIMetrics PredictiveAnalyticsEngine::getPredictionMetrics() const {
    return predictionMetrics_;
}

void PredictiveAnalyticsEngine::resetPredictiveModels() {
    if (!initialized_) {
        return;
    }
    
    behaviorDatabase_->clearDatabase();
    patternCache_->clearCache();
    patternDetector_->clearPatterns();
    sequenceAnalyzer_->clearData();
    conservationAnalyzer_->clearInsightsData();
    
    totalPredictions_ = 0;
    accuratePredictions_ = 0;
    predictionMetrics_ = AIMetrics();
}

void PredictiveAnalyticsEngine::configure(const PredictiveConfig& config) {
    config_ = config;
    
    // Reconfigure sub-components if needed
    if (initialized_) {
        // Update configuration for sub-components
        TemporalAnalysisConfig temporalConfig;
        temporalConfig.shortTerm = TemporalWindow(config.shortTermWindow_s, 60, 3, true);
        temporalConfig.mediumTerm = TemporalWindow(config.mediumTermWindow_s, 300, 5, true);
        temporalConfig.longTerm = TemporalWindow(config.longTermWindow_s, 1800, 10, false);
        
        sequenceAnalyzer_->configure(temporalConfig);
    }
}

bool PredictiveAnalyticsEngine::isReadyForPredictions() const {
    if (!initialized_) {
        return false;
    }
    
    // Check if we have sufficient data for meaningful predictions
    return behaviorDatabase_->hasSufficientData(SpeciesType::UNKNOWN, 10, 3600) &&
           sequenceAnalyzer_->hasSufficientData(0); // Short-term analysis
}

void PredictiveAnalyticsEngine::optimizeForPowerConsumption() {
    // Reduce analysis frequency or complexity based on power state
    // This is a placeholder for power optimization logic
    
    if (patternCache_) {
        // Optimize cache to reduce memory usage
        patternCache_->optimizeCache();
    }
    
    if (behaviorDatabase_) {
        // Remove old data to save memory and processing time
        uint32_t oneWeekAgo = (millis() / 1000) - (7 * 24 * 3600);
        behaviorDatabase_->optimizeStorage(oneWeekAgo);
    }
}