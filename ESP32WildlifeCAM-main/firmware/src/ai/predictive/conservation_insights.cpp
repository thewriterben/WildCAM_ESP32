/**
 * @file conservation_insights.cpp
 * @brief Implementation of Conservation Intelligence and Insights Generation
 */

#include "conservation_insights.h"
#include <algorithm>
#include <cmath>

ConservationInsights::ConservationInsights() : 
    initialized_(false), lastEcosystemUpdate_(0),
    totalAssessments_(0), successfulAssessments_(0) {
}

ConservationInsights::~ConservationInsights() {
    // Destructors handle cleanup
}

bool ConservationInsights::init() {
    // Reserve space for data storage
    behaviorObservations_.reserve(10000);
    environmentObservations_.reserve(10000);
    
    // Initialize metrics
    insightsMetrics_ = AIMetrics();
    
    initialized_ = true;
    return true;
}

std::vector<ConservationRecommendation> ConservationInsights::generateConservationAssessment(
    const std::vector<BehaviorResult>& behaviorData,
    const std::vector<EnhancedBehaviorPattern>& patterns,
    const TemporalAnalysisResult& temporal) {
    
    std::vector<ConservationRecommendation> recommendations;
    
    if (!initialized_ || behaviorData.empty()) {
        return recommendations;
    }
    
    uint32_t startTime = millis();
    
    // Analyze population health across species
    std::map<SpeciesType, PopulationAssessment> assessments;
    
    // Group behaviors by species (simplified - would need species classification)
    for (const auto& behavior : behaviorData) {
        SpeciesType species = SpeciesType::UNKNOWN; // Would be determined from context
        if (assessments.find(species) == assessments.end()) {
            assessments[species] = assessPopulationHealth(species, 86400);
        }
    }
    
    // Generate recommendations based on assessments
    for (const auto& assessment : assessments) {
        // Check for critical health indicators
        if (assessment.second.overallHealthScore < 0.3f) {
            ConservationRecommendation recommendation;
            recommendation.priority = ConservationPriority::CRITICAL;
            recommendation.title = "Critical Population Health Alert";
            recommendation.description = "Population health score indicates potential crisis requiring immediate intervention.";
            recommendation.actionItems.push_back("Conduct immediate field assessment");
            recommendation.actionItems.push_back("Check for environmental stressors");
            recommendation.actionItems.push_back("Implement emergency protection measures");
            recommendation.timeframe_days = 1;
            recommendation.expectedImpact = 0.8f;
            recommendation.confidenceLevel = 0.9f;
            recommendations.push_back(recommendation);
        }
        
        // Check stress level trends
        if (assessment.second.stressTrend > 0.2f) {
            ConservationRecommendation recommendation;
            recommendation.priority = ConservationPriority::HIGH;
            recommendation.title = "Increasing Stress Levels Detected";
            recommendation.description = "Wildlife stress indicators show concerning upward trend.";
            recommendation.actionItems.push_back("Identify stress sources");
            recommendation.actionItems.push_back("Reduce human disturbance");
            recommendation.actionItems.push_back("Monitor food availability");
            recommendation.timeframe_days = 7;
            recommendation.expectedImpact = 0.7f;
            recommendation.confidenceLevel = 0.8f;
            recommendations.push_back(recommendation);
        }
        
        // Check behavioral diversity
        if (assessment.second.behavioralDiversity < 1.5f) {
            ConservationRecommendation recommendation;
            recommendation.priority = ConservationPriority::MEDIUM;
            recommendation.title = "Low Behavioral Diversity Warning";
            recommendation.description = "Reduced behavioral variety may indicate habitat degradation.";
            recommendation.actionItems.push_back("Assess habitat quality");
            recommendation.actionItems.push_back("Check food source diversity");
            recommendation.actionItems.push_back("Monitor environmental conditions");
            recommendation.timeframe_days = 14;
            recommendation.expectedImpact = 0.6f;
            recommendation.confidenceLevel = 0.7f;
            recommendations.push_back(recommendation);
        }
    }
    
    // Analyze patterns for conservation insights
    for (const auto& pattern : patterns) {
        if (pattern.conservation.criticalForSurvival && pattern.confidence < 0.5f) {
            ConservationRecommendation recommendation;
            recommendation.priority = ConservationPriority::HIGH;
            recommendation.title = "Critical Behavior Pattern Disruption";
            recommendation.description = "Essential survival behaviors showing reduced frequency.";
            recommendation.actionItems.push_back("Investigate behavior disruption causes");
            recommendation.actionItems.push_back("Protect critical habitat areas");
            recommendation.actionItems.push_back("Reduce environmental disturbances");
            recommendation.timeframe_days = 3;
            recommendation.expectedImpact = 0.8f;
            recommendation.confidenceLevel = pattern.confidence;
            recommendations.push_back(recommendation);
        }
    }
    
    // Analyze temporal patterns for seasonal insights
    if (temporal.circadianPattern.circadianStrength < 0.3f) {
        ConservationRecommendation recommendation;
        recommendation.priority = ConservationPriority::MEDIUM;
        recommendation.title = "Disrupted Daily Activity Patterns";
        recommendation.description = "Natural circadian rhythms appear compromised.";
        recommendation.actionItems.push_back("Reduce artificial lighting during night hours");
        recommendation.actionItems.push_back("Minimize disturbances during peak activity times");
        recommendation.actionItems.push_back("Monitor light pollution sources");
        recommendation.timeframe_days = 30;
        recommendation.expectedImpact = 0.5f;
        recommendation.confidenceLevel = 0.6f;
        recommendations.push_back(recommendation);
    }
    
    // Sort recommendations by priority
    std::sort(recommendations.begin(), recommendations.end(),
             [](const ConservationRecommendation& a, const ConservationRecommendation& b) {
                 return a.priority > b.priority;
             });
    
    uint32_t processingTime = millis() - startTime;
    updateInsightsMetrics(true, processingTime);
    
    return recommendations;
}

PopulationAssessment ConservationInsights::assessPopulationHealth(SpeciesType species,
                                                                uint32_t timeWindow_s) {
    PopulationAssessment assessment;
    assessment.species = species;
    
    if (!initialized_) {
        return assessment;
    }
    
    // Filter behavior data for the specified species and time window
    std::vector<BehaviorResult> speciesData;
    unsigned long cutoffTime = millis() - (timeWindow_s * 1000);
    
    for (const auto& behavior : behaviorObservations_) {
        if (behavior.timestamp >= cutoffTime) {
            // In a real implementation, would filter by species
            speciesData.push_back(behavior);
        }
    }
    
    if (speciesData.empty()) {
        return assessment;
    }
    
    // Analyze population trends
    analyzePopulationTrends(assessment, speciesData);
    
    // Calculate health indicators
    calculateHealthIndicators(assessment, speciesData);
    
    // Assess stress levels
    assessStressLevels(assessment, speciesData);
    
    // Detect breeding activity
    detectBreedingActivity(assessment, speciesData);
    
    // Calculate behavioral diversity
    std::map<BehaviorType, uint32_t> behaviorCounts;
    for (const auto& behavior : speciesData) {
        behaviorCounts[behavior.primaryBehavior]++;
    }
    
    // Shannon diversity index
    float diversity = 0.0f;
    float total = speciesData.size();
    for (const auto& count : behaviorCounts) {
        float proportion = count.second / total;
        if (proportion > 0) {
            diversity -= proportion * log2f(proportion);
        }
    }
    assessment.behavioralDiversity = diversity;
    assessment.uniqueBehaviorsObserved = behaviorCounts.size();
    
    // Identify dominant and rare behaviors
    for (const auto& count : behaviorCounts) {
        float frequency = count.second / total;
        if (frequency > 0.2f) {
            assessment.dominantBehaviors.push_back(count.first);
        } else if (frequency < 0.05f) {
            assessment.rareBehaviors.push_back(count.first);
        }
    }
    
    // Calculate overall health score
    float healthScore = 0.0f;
    healthScore += (assessment.behavioralDiversity / 3.0f) * 0.3f; // Diversity component
    healthScore += (1.0f - assessment.avgStressLevel) * 0.3f;      // Stress component
    healthScore += assessment.reproductiveSuccess * 0.2f;         // Reproduction component
    healthScore += std::min(1.0f, assessment.activePopulationCount / 10.0f) * 0.2f; // Population size
    
    assessment.overallHealthScore = std::max(0.0f, std::min(1.0f, healthScore));
    
    return assessment;
}

EcosystemHealth ConservationInsights::assessEcosystemHealth(uint32_t timeWindow_s) {
    // Check if we need to update cached assessment
    unsigned long now = millis();
    if (now - lastEcosystemUpdate_ < 3600000) { // Update every hour
        return cachedEcosystemHealth_;
    }
    
    EcosystemHealth ecosystem;
    
    if (!initialized_ || behaviorObservations_.empty()) {
        return ecosystem;
    }
    
    // Get recent behavior data
    std::vector<BehaviorResult> recentData;
    unsigned long cutoffTime = now - (timeWindow_s * 1000);
    
    for (const auto& behavior : behaviorObservations_) {
        if (behavior.timestamp >= cutoffTime) {
            recentData.push_back(behavior);
        }
    }
    
    if (recentData.empty()) {
        return ecosystem;
    }
    
    // Calculate biodiversity index
    ecosystem.biodiversityIndex = calculateSpeciesDiversity(recentData);
    
    // Assess habitat quality based on behavior patterns
    calculateHabitatMetrics(ecosystem, recentData);
    
    // Analyze species interactions
    analyzeSpeciesInteractions(ecosystem, recentData);
    
    // Assess environmental threats
    std::vector<EnvironmentalData> recentEnv;
    for (const auto& env : environmentObservations_) {
        if (env.timestamp >= cutoffTime) {
            recentEnv.push_back(env);
        }
    }
    assessEnvironmentalThreats(ecosystem, recentEnv);
    
    // Calculate environmental stability
    if (!recentEnv.empty()) {
        float tempVariance = 0.0f;
        float humidityVariance = 0.0f;
        float meanTemp = 0.0f;
        float meanHumidity = 0.0f;
        
        for (const auto& env : recentEnv) {
            meanTemp += env.temperature;
            meanHumidity += env.humidity;
        }
        meanTemp /= recentEnv.size();
        meanHumidity /= recentEnv.size();
        
        for (const auto& env : recentEnv) {
            tempVariance += (env.temperature - meanTemp) * (env.temperature - meanTemp);
            humidityVariance += (env.humidity - meanHumidity) * (env.humidity - meanHumidity);
        }
        tempVariance /= recentEnv.size();
        humidityVariance /= recentEnv.size();
        
        // Lower variance = higher stability
        float tempStability = 1.0f / (1.0f + tempVariance / 100.0f);
        float humidityStability = 1.0f / (1.0f + humidityVariance / 1000.0f);
        ecosystem.environmentalStability = (tempStability + humidityStability) / 2.0f;
    }
    
    cachedEcosystemHealth_ = ecosystem;
    lastEcosystemUpdate_ = now;
    
    return ecosystem;
}

std::vector<MigrationPrediction> ConservationInsights::predictMigration(SpeciesType species,
                                                                       uint8_t monthsAhead) {
    std::vector<MigrationPrediction> predictions;
    
    if (!initialized_) {
        return predictions;
    }
    
    // Analyze historical movement patterns
    MigrationPrediction prediction;
    prediction.species = species;
    
    if (analyzeHistoricalMigration(species, prediction)) {
        predictions.push_back(prediction);
    }
    
    return predictions;
}

FeedingOptimization ConservationInsights::generateFeedingOptimization(SpeciesType species) {
    FeedingOptimization optimization;
    optimization.species = species;
    
    if (!initialized_) {
        return optimization;
    }
    
    // Get feeding behavior data for the species
    std::vector<BehaviorResult> feedingData;
    for (const auto& behavior : behaviorObservations_) {
        if (behavior.primaryBehavior == BehaviorType::FEEDING) {
            feedingData.push_back(behavior);
        }
    }
    
    if (feedingData.empty()) {
        return optimization;
    }
    
    // Analyze optimal feeding times
    analyzeOptimalFeedingTimes(optimization, feedingData);
    
    // Generate location recommendations
    generateLocationRecommendations(optimization, feedingData);
    
    return optimization;
}

std::vector<ConservationRecommendation> ConservationInsights::detectConservationAlerts(
    const std::vector<BehaviorResult>& behaviorData) {
    
    std::vector<ConservationRecommendation> alerts;
    
    if (!initialized_ || behaviorData.empty()) {
        return alerts;
    }
    
    // Check for immediate threats or unusual patterns
    float avgStress = 0.0f;
    uint32_t highStressCount = 0;
    
    for (const auto& behavior : behaviorData) {
        avgStress += behavior.stress_level;
        if (behavior.stress_level > 0.8f) {
            highStressCount++;
        }
    }
    avgStress /= behaviorData.size();
    
    // High stress alert
    if (avgStress > 0.7f || highStressCount > behaviorData.size() * 0.3f) {
        ConservationRecommendation alert;
        alert.priority = ConservationPriority::EMERGENCY;
        alert.title = "Emergency: Extreme Stress Levels Detected";
        alert.description = "Wildlife showing signs of severe distress requiring immediate action.";
        alert.actionItems.push_back("Identify and remove immediate threats");
        alert.actionItems.push_back("Implement emergency protection protocols");
        alert.actionItems.push_back("Contact wildlife management authorities");
        alert.timeframe_days = 0; // Immediate action
        alert.expectedImpact = 0.9f;
        alert.confidenceLevel = 0.95f;
        alerts.push_back(alert);
    }
    
    // Population decline alert
    if (behaviorData.size() < 5) { // Very low activity might indicate population decline
        ConservationRecommendation alert;
        alert.priority = ConservationPriority::HIGH;
        alert.title = "Potential Population Decline Alert";
        alert.description = "Significantly reduced wildlife activity detected.";
        alert.actionItems.push_back("Conduct population count assessment");
        alert.actionItems.push_back("Check for environmental changes");
        alert.actionItems.push_back("Investigate potential threats");
        alert.timeframe_days = 2;
        alert.expectedImpact = 0.8f;
        alert.confidenceLevel = 0.7f;
        alerts.push_back(alert);
    }
    
    return alerts;
}

float ConservationInsights::calculateBiodiversityIndex(uint32_t timeWindow_s) {
    return calculateSpeciesDiversity(behaviorObservations_);
}

float ConservationInsights::assessHabitatQuality(const std::vector<EnhancedBehaviorPattern>& patterns) {
    if (patterns.empty()) {
        return 0.0f;
    }
    
    float qualityScore = 0.0f;
    uint32_t criticalPatterns = 0;
    uint32_t healthyPatterns = 0;
    
    for (const auto& pattern : patterns) {
        if (pattern.conservation.criticalForSurvival) {
            criticalPatterns++;
            if (pattern.confidence > 0.7f) {
                qualityScore += 0.3f; // High weight for critical behaviors
            }
        }
        
        if (pattern.conservation.indicatesHealth && pattern.confidence > 0.6f) {
            healthyPatterns++;
            qualityScore += 0.1f;
        }
        
        if (pattern.conservation.indicatesStress && pattern.confidence > 0.6f) {
            qualityScore -= 0.2f; // Penalty for stress indicators
        }
    }
    
    // Normalize score
    float maxPossibleScore = criticalPatterns * 0.3f + patterns.size() * 0.1f;
    if (maxPossibleScore > 0) {
        qualityScore = std::max(0.0f, qualityScore / maxPossibleScore);
    }
    
    return std::min(1.0f, qualityScore);
}

void ConservationInsights::updateBehaviorObservations(const BehaviorResult& behavior,
                                                     const EnvironmentalData& environment) {
    if (!initialized_) {
        return;
    }
    
    behaviorObservations_.push_back(behavior);
    environmentObservations_.push_back(environment);
    
    // Limit data size for memory management
    const size_t maxObservations = 10000;
    if (behaviorObservations_.size() > maxObservations) {
        behaviorObservations_.erase(behaviorObservations_.begin());
        environmentObservations_.erase(environmentObservations_.begin());
    }
}

void ConservationInsights::clearInsightsData() {
    behaviorObservations_.clear();
    environmentObservations_.clear();
    populationAssessments_.clear();
    cachedEcosystemHealth_ = EcosystemHealth();
    lastEcosystemUpdate_ = 0;
    insightsMetrics_ = AIMetrics();
    totalAssessments_ = 0;
    successfulAssessments_ = 0;
}

AIMetrics ConservationInsights::getInsightsMetrics() const {
    return insightsMetrics_;
}

// Private methods implementation
void ConservationInsights::analyzePopulationTrends(PopulationAssessment& assessment,
                                                  const std::vector<BehaviorResult>& data) {
    if (data.empty()) {
        return;
    }
    
    // Estimate population based on observation frequency
    // This is simplified - real implementation would use mark-recapture or other methods
    assessment.activePopulationCount = data.size(); // Very rough estimate
    
    // Analyze trend by comparing first half to second half of data
    if (data.size() >= 10) {
        size_t midPoint = data.size() / 2;
        float earlyActivity = 0.0f;
        float recentActivity = 0.0f;
        
        for (size_t i = 0; i < midPoint; i++) {
            earlyActivity += data[i].activity_level;
        }
        for (size_t i = midPoint; i < data.size(); i++) {
            recentActivity += data[i].activity_level;
        }
        
        earlyActivity /= midPoint;
        recentActivity /= (data.size() - midPoint);
        
        assessment.populationTrend = recentActivity - earlyActivity;
        assessment.confidenceInterval = 0.7f; // Simplified confidence measure
    }
}

void ConservationInsights::calculateHealthIndicators(PopulationAssessment& assessment,
                                                    const std::vector<BehaviorResult>& data) {
    // Activity level indicator
    HealthIndicator activityIndicator;
    activityIndicator.indicatorName = "Average Activity Level";
    
    float totalActivity = 0.0f;
    for (const auto& behavior : data) {
        totalActivity += behavior.activity_level;
    }
    activityIndicator.currentValue = data.size() > 0 ? totalActivity / data.size() : 0.0f;
    activityIndicator.historicalAverage = 0.6f; // Typical baseline
    activityIndicator.threshold_warning = 0.4f;
    activityIndicator.threshold_critical = 0.2f;
    
    if (activityIndicator.currentValue < activityIndicator.threshold_critical) {
        activityIndicator.priority = ConservationPriority::CRITICAL;
        activityIndicator.recommendation = "Immediate intervention required - extremely low activity levels";
    } else if (activityIndicator.currentValue < activityIndicator.threshold_warning) {
        activityIndicator.priority = ConservationPriority::HIGH;
        activityIndicator.recommendation = "Monitor closely - activity levels below normal";
    } else {
        activityIndicator.priority = ConservationPriority::LOW;
        activityIndicator.recommendation = "Activity levels within normal range";
    }
    
    activityIndicator.isImproving = activityIndicator.currentValue > activityIndicator.historicalAverage;
    activityIndicator.isDeteriorating = activityIndicator.currentValue < activityIndicator.historicalAverage * 0.8f;
    
    assessment.healthIndicators.push_back(activityIndicator);
}

void ConservationInsights::assessStressLevels(PopulationAssessment& assessment,
                                             const std::vector<BehaviorResult>& data) {
    if (data.empty()) {
        return;
    }
    
    float totalStress = 0.0f;
    uint32_t stressCount = 0;
    
    for (const auto& behavior : data) {
        if (behavior.stress_level > 0.0f) {
            totalStress += behavior.stress_level;
            stressCount++;
        }
    }
    
    if (stressCount > 0) {
        assessment.avgStressLevel = totalStress / stressCount;
        
        // Analyze stress trend (simplified)
        if (data.size() >= 10) {
            size_t midPoint = data.size() / 2;
            float earlyStress = 0.0f;
            float recentStress = 0.0f;
            uint32_t earlyCount = 0;
            uint32_t recentCount = 0;
            
            for (size_t i = 0; i < midPoint; i++) {
                if (data[i].stress_level > 0.0f) {
                    earlyStress += data[i].stress_level;
                    earlyCount++;
                }
            }
            for (size_t i = midPoint; i < data.size(); i++) {
                if (data[i].stress_level > 0.0f) {
                    recentStress += data[i].stress_level;
                    recentCount++;
                }
            }
            
            if (earlyCount > 0 && recentCount > 0) {
                earlyStress /= earlyCount;
                recentStress /= recentCount;
                assessment.stressTrend = recentStress - earlyStress;
            }
        }
        
        // Identify stress factors (simplified)
        if (assessment.avgStressLevel > 0.7f) {
            assessment.stressFactors.push_back("High environmental stress detected");
        }
        if (assessment.stressTrend > 0.1f) {
            assessment.stressFactors.push_back("Increasing stress trend observed");
        }
    }
}

void ConservationInsights::detectBreedingActivity(PopulationAssessment& assessment,
                                                 const std::vector<BehaviorResult>& data) {
    uint32_t matingBehaviors = 0;
    uint32_t socialBehaviors = 0;
    
    for (const auto& behavior : data) {
        if (behavior.primaryBehavior == BehaviorType::MATING) {
            matingBehaviors++;
            assessment.breedingActivityDetected = true;
        }
        if (behavior.primaryBehavior == BehaviorType::SOCIAL) {
            socialBehaviors++;
        }
    }
    
    if (matingBehaviors > 0) {
        assessment.reproductiveSuccess = std::min(1.0f, static_cast<float>(matingBehaviors) / data.size() * 10.0f);
        
        // Estimate breeding season (simplified)
        uint8_t currentMonth = (millis() / (86400000 * 30)) % 12 + 1;
        assessment.breedingSeasonMonth = currentMonth;
    }
}

float ConservationInsights::calculateSpeciesDiversity(const std::vector<BehaviorResult>& data) {
    if (data.empty()) {
        return 0.0f;
    }
    
    // In a real implementation, this would count actual species
    // For now, we'll use behavioral diversity as a proxy
    std::map<BehaviorType, uint32_t> behaviorCounts;
    for (const auto& behavior : data) {
        behaviorCounts[behavior.primaryBehavior]++;
    }
    
    // Shannon diversity index
    float diversity = 0.0f;
    float total = data.size();
    for (const auto& count : behaviorCounts) {
        float proportion = count.second / total;
        if (proportion > 0) {
            diversity -= proportion * log2f(proportion);
        }
    }
    
    return diversity;
}

void ConservationInsights::analyzeSpeciesInteractions(EcosystemHealth& ecosystem,
                                                     const std::vector<BehaviorResult>& data) {
    // Analyze group behaviors for species interactions
    for (const auto& behavior : data) {
        if (behavior.isGroupBehavior && behavior.animalCount > 1) {
            EcosystemHealth::SpeciesInteraction interaction;
            interaction.species1 = SpeciesType::UNKNOWN; // Would need species identification
            interaction.species2 = SpeciesType::UNKNOWN;
            interaction.interactionStrength = behavior.confidence;
            
            // Determine interaction type based on behavior
            switch (behavior.primaryBehavior) {
                case BehaviorType::AGGRESSIVE:
                    interaction.isCompetitive = true;
                    break;
                case BehaviorType::SOCIAL:
                    interaction.isSymbiotic = true;
                    break;
                case BehaviorType::FEEDING:
                    interaction.isCompetitive = true; // Competition for resources
                    break;
                default:
                    break;
            }
            
            ecosystem.interactions.push_back(interaction);
        }
    }
}

void ConservationInsights::assessEnvironmentalThreats(EcosystemHealth& ecosystem,
                                                     const std::vector<EnvironmentalData>& environment) {
    if (environment.empty()) {
        return;
    }
    
    // Analyze temperature extremes
    float minTemp = environment[0].temperature;
    float maxTemp = environment[0].temperature;
    for (const auto& env : environment) {
        minTemp = std::min(minTemp, env.temperature);
        maxTemp = std::max(maxTemp, env.temperature);
    }
    
    if (maxTemp - minTemp > 30.0f) { // High temperature variation
        EcosystemHealth::ThreatAssessment threat;
        threat.threatType = "Temperature Extremes";
        threat.severity = 0.7f;
        threat.probability = 0.8f;
        threat.evidence = "High temperature variation detected";
        ecosystem.threats.push_back(threat);
    }
    
    // Check for unusual humidity patterns
    float avgHumidity = 0.0f;
    for (const auto& env : environment) {
        avgHumidity += env.humidity;
    }
    avgHumidity /= environment.size();
    
    if (avgHumidity < 30.0f || avgHumidity > 90.0f) {
        EcosystemHealth::ThreatAssessment threat;
        threat.threatType = "Humidity Stress";
        threat.severity = 0.5f;
        threat.probability = 0.6f;
        threat.evidence = "Unusual humidity levels detected";
        ecosystem.threats.push_back(threat);
    }
}

void ConservationInsights::calculateHabitatMetrics(EcosystemHealth& ecosystem,
                                                  const std::vector<BehaviorResult>& data) {
    if (data.empty()) {
        return;
    }
    
    // Calculate habitat quality based on behavior patterns
    float naturalBehaviors = 0.0f;
    float stressedBehaviors = 0.0f;
    
    for (const auto& behavior : data) {
        switch (behavior.primaryBehavior) {
            case BehaviorType::FEEDING:
            case BehaviorType::RESTING:
            case BehaviorType::GROOMING:
                naturalBehaviors++;
                break;
            case BehaviorType::ALERT:
            case BehaviorType::AGGRESSIVE:
                if (behavior.stress_level > 0.5f) {
                    stressedBehaviors++;
                }
                break;
            default:
                break;
        }
    }
    
    float total = data.size();
    ecosystem.habitatQuality = total > 0 ? 
        (naturalBehaviors / total) * (1.0f - (stressedBehaviors / total)) : 0.0f;
    ecosystem.habitatQuality = std::max(0.0f, std::min(1.0f, ecosystem.habitatQuality));
}

bool ConservationInsights::analyzeHistoricalMigration(SpeciesType species, MigrationPrediction& prediction) {
    // Simplified migration analysis based on movement patterns
    uint32_t movementBehaviors = 0;
    
    for (const auto& behavior : behaviorObservations_) {
        if (behavior.primaryBehavior == BehaviorType::MOVING) {
            movementBehaviors++;
        }
    }
    
    if (movementBehaviors > behaviorObservations_.size() * 0.3f) {
        prediction.migrationExpected = true;
        prediction.confidence = 0.7f;
        prediction.predictedStartMonth = (millis() / (86400000 * 30)) % 12 + 1;
        prediction.predictedEndMonth = (prediction.predictedStartMonth + 2) % 12 + 1;
        prediction.estimatedPopulationSize = static_cast<float>(behaviorObservations_.size());
        
        prediction.triggerFactors.push_back("Increased movement activity detected");
        prediction.monitoringRecommendations.push_back("Monitor key migration corridors");
        prediction.habitatPreparation.push_back("Ensure food and water availability along routes");
        
        return true;
    }
    
    return false;
}

void ConservationInsights::analyzeOptimalFeedingTimes(FeedingOptimization& optimization,
                                                     const std::vector<BehaviorResult>& data) {
    // Analyze feeding behavior by time of day
    std::vector<uint32_t> hourlyFeedingCounts(24, 0);
    std::vector<float> hourlyEffectiveness(24, 0.0f);
    
    for (const auto& behavior : data) {
        uint8_t hour = (behavior.timestamp / 3600000) % 24;
        hourlyFeedingCounts[hour]++;
        hourlyEffectiveness[hour] += behavior.confidence;
    }
    
    // Calculate optimal times
    for (int hour = 0; hour < 24; hour++) {
        if (hourlyFeedingCounts[hour] > 0) {
            FeedingOptimization::OptimalTime optimalTime;
            optimalTime.hour = hour;
            optimalTime.probability = static_cast<float>(hourlyFeedingCounts[hour]) / data.size();
            optimalTime.effectiveness = hourlyEffectiveness[hour] / hourlyFeedingCounts[hour];
            
            if (optimalTime.probability > 0.05f && optimalTime.effectiveness > 0.6f) {
                optimization.dailyOptimalTimes.push_back(optimalTime);
            }
        }
    }
    
    // Sort by effectiveness
    std::sort(optimization.dailyOptimalTimes.begin(), optimization.dailyOptimalTimes.end(),
             [](const FeedingOptimization::OptimalTime& a, const FeedingOptimization::OptimalTime& b) {
                 return a.effectiveness > b.effectiveness;
             });
}

void ConservationInsights::generateLocationRecommendations(FeedingOptimization& optimization,
                                                          const std::vector<BehaviorResult>& data) {
    // Generate generic location recommendations
    FeedingOptimization::LocationRecommendation recommendation;
    recommendation.locationDescription = "Areas with high feeding activity frequency";
    recommendation.suitability = 0.8f;
    recommendation.reasons.push_back("Consistent feeding behavior observed");
    recommendation.reasons.push_back("Low stress levels during feeding");
    
    optimization.recommendedLocations.push_back(recommendation);
}

ConservationPriority ConservationInsights::calculatePriority(float severity, float urgency) {
    float combined = (severity + urgency) / 2.0f;
    
    if (combined >= 0.9f) return ConservationPriority::EMERGENCY;
    if (combined >= 0.7f) return ConservationPriority::CRITICAL;
    if (combined >= 0.5f) return ConservationPriority::HIGH;
    if (combined >= 0.3f) return ConservationPriority::MEDIUM;
    return ConservationPriority::LOW;
}

void ConservationInsights::updateInsightsMetrics(bool success, uint32_t processingTime) {
    totalAssessments_++;
    if (success) {
        successfulAssessments_++;
    }
    
    insightsMetrics_.totalInferences = totalAssessments_;
    insightsMetrics_.successfulInferences = successfulAssessments_;
    insightsMetrics_.inferenceTime = processingTime;
}

void ConservationInsights::pruneOldObservations() {
    // Keep only recent observations to manage memory
    const size_t maxObservations = 10000;
    if (behaviorObservations_.size() > maxObservations) {
        size_t removeCount = behaviorObservations_.size() - maxObservations;
        behaviorObservations_.erase(behaviorObservations_.begin(), 
                                   behaviorObservations_.begin() + removeCount);
        environmentObservations_.erase(environmentObservations_.begin(),
                                      environmentObservations_.begin() + removeCount);
    }
}