/**
 * @file temporal_sequence_analyzer.cpp
 * @brief Implementation of Temporal Sequence Analysis for Wildlife Behavior
 */

#include "temporal_sequence_analyzer.h"
#include <algorithm>
#include <cmath>

TemporalSequenceAnalyzer::TemporalSequenceAnalyzer() : 
    initialized_(false), lastCircadianUpdate_(0), lastSeasonalUpdate_(0),
    totalAnalyses_(0), successfulAnalyses_(0) {
}

TemporalSequenceAnalyzer::~TemporalSequenceAnalyzer() {
    // Destructors handle cleanup
}

bool TemporalSequenceAnalyzer::init(const TemporalAnalysisConfig& config) {
    config_ = config;
    
    // Reserve space for data storage
    behaviorHistory_.reserve(10000);
    environmentHistory_.reserve(10000);
    timestampHistory_.reserve(10000);
    
    // Initialize metrics
    analysisMetrics_ = AIMetrics();
    
    initialized_ = true;
    return true;
}

void TemporalSequenceAnalyzer::addObservation(const BehaviorResult& behavior,
                                            const EnvironmentalData& environment) {
    if (!initialized_) {
        return;
    }
    
    // Add to history
    behaviorHistory_.push_back(behavior);
    environmentHistory_.push_back(environment);
    timestampHistory_.push_back(behavior.timestamp);
    
    // Update circadian and seasonal data
    updateCircadianData(behavior);
    updateSeasonalData(behavior);
    
    // Prune old data to manage memory
    pruneOldData();
}

TemporalAnalysisResult TemporalSequenceAnalyzer::analyzeSequences(uint32_t timeWindow_s) {
    TemporalAnalysisResult result;
    
    if (!initialized_) {
        return result;
    }
    
    uint32_t startTime = millis();
    
    // Extract sequences for different window types
    result.sequences = extractSequences(config_.shortTerm, timeWindow_s);
    
    // Add medium and long term sequences if sufficient data
    if (hasSufficientData(1)) {
        auto mediumSequences = extractSequences(config_.mediumTerm, timeWindow_s);
        result.sequences.insert(result.sequences.end(), mediumSequences.begin(), mediumSequences.end());
    }
    
    if (hasSufficientData(2)) {
        auto longSequences = extractSequences(config_.longTerm, timeWindow_s);
        result.sequences.insert(result.sequences.end(), longSequences.begin(), longSequences.end());
    }
    
    // Analyze circadian patterns if enabled
    if (config_.enableCircadianAnalysis && behaviorHistory_.size() >= config_.circadianDataPoints) {
        result.circadianPattern = analyzeCircadianPattern(3);
    }
    
    // Analyze seasonal patterns if enabled
    if (config_.enableSeasonalAnalysis && behaviorHistory_.size() >= config_.seasonalDataPoints) {
        result.seasonalPattern = analyzeSeasonalPattern(3);
    }
    
    // Analyze trends
    if (behaviorHistory_.size() >= 10) {
        analyzeTrends(result);
    }
    
    // Assess data quality and prediction readiness
    result.sufficientDataForPrediction = hasSufficientData(0) && behaviorHistory_.size() >= 20;
    result.dataQualityScore = calculateDataQuality();
    result.predictionConfidence = result.sufficientDataForPrediction ? 
        std::min(static_cast<float>(behaviorHistory_.size()) / 100.0f, 1.0f) : 0.0f;
    
    uint32_t processingTime = millis() - startTime;
    updateAnalysisMetrics(true, processingTime);
    
    return result;
}

TemporalSequence TemporalSequenceAnalyzer::getSequence(const TemporalWindow& window,
                                                     unsigned long endTime) {
    TemporalSequence sequence;
    sequence.window = window;
    
    if (!initialized_ || behaviorHistory_.empty()) {
        return sequence;
    }
    
    if (endTime == 0) {
        endTime = millis();
    }
    
    unsigned long startTime = endTime - (window.windowSize_s * 1000);
    sequence.startTime = startTime;
    sequence.endTime = endTime;
    
    // Extract behaviors in time window
    for (size_t i = 0; i < behaviorHistory_.size(); i++) {
        unsigned long behaviorTime = timestampHistory_[i];
        if (behaviorTime >= startTime && behaviorTime <= endTime) {
            sequence.behaviors.push_back(behaviorHistory_[i]);
        }
    }
    
    // Analyze sequence characteristics
    analyzeSequenceStatistics(sequence);
    calculateBehaviorEntropy(sequence);
    
    return sequence;
}

std::pair<BehaviorType, float> TemporalSequenceAnalyzer::predictNextInSequence(
    const TemporalSequence& recentSequence,
    uint32_t timeHorizon_s) {
    
    if (recentSequence.behaviors.empty()) {
        return {BehaviorType::UNKNOWN, 0.0f};
    }
    
    // Get recent behavior types
    std::vector<BehaviorType> recentTypes;
    for (const auto& behavior : recentSequence.behaviors) {
        recentTypes.push_back(behavior.primaryBehavior);
    }
    
    // Build transition matrix from historical data
    auto transitionMatrix = getTransitionMatrix(86400); // 24 hours of data
    
    // Find most likely next behavior
    BehaviorType lastBehavior = recentTypes.back();
    float bestProbability = 0.0f;
    BehaviorType predictedBehavior = BehaviorType::UNKNOWN;
    
    for (const auto& transition : transitionMatrix) {
        if (transition.first.first == lastBehavior) {
            if (transition.second > bestProbability) {
                bestProbability = transition.second;
                predictedBehavior = transition.first.second;
            }
        }
    }
    
    // Adjust confidence based on sequence coherence
    float confidence = bestProbability * recentSequence.sequenceCoherence;
    
    return {predictedBehavior, confidence};
}

CircadianAnalysis TemporalSequenceAnalyzer::analyzeCircadianPattern(uint32_t minDays) {
    // Check if we need to update cached analysis
    unsigned long now = millis();
    if (now - lastCircadianUpdate_ < 3600000) { // Update every hour
        return cachedCircadianAnalysis_;
    }
    
    CircadianAnalysis analysis;
    
    if (behaviorHistory_.size() < minDays * 24) { // Rough estimate
        return analysis;
    }
    
    // Calculate activity by hour
    std::vector<uint32_t> hourCounts(24, 0);
    std::vector<std::vector<uint32_t>> hourBehaviorCounts(24, std::vector<uint32_t>(11, 0));
    
    for (size_t i = 0; i < behaviorHistory_.size(); i++) {
        uint8_t hour = getHourOfDay(timestampHistory_[i]);
        BehaviorType behavior = behaviorHistory_[i].primaryBehavior;
        
        hourCounts[hour]++;
        if (static_cast<int>(behavior) < 11) {
            hourBehaviorCounts[hour][static_cast<int>(behavior)]++;
        }
    }
    
    // Calculate activity levels and probabilities
    uint32_t maxCount = *std::max_element(hourCounts.begin(), hourCounts.end());
    for (int hour = 0; hour < 24; hour++) {
        analysis.hourlyActivity[hour] = maxCount > 0 ? 
            static_cast<float>(hourCounts[hour]) / maxCount : 0.0f;
        
        // Calculate behavior probabilities for this hour
        uint32_t totalBehaviors = hourCounts[hour];
        for (int behavior = 0; behavior < 11; behavior++) {
            analysis.hourlyBehaviorProb[hour][behavior] = totalBehaviors > 0 ?
                static_cast<float>(hourBehaviorCounts[hour][behavior]) / totalBehaviors : 0.0f;
        }
    }
    
    // Identify peak periods
    identifyActivityPeaks(analysis);
    
    // Calculate circadian metrics
    calculateCircadianMetrics(analysis);
    
    cachedCircadianAnalysis_ = analysis;
    lastCircadianUpdate_ = now;
    
    return analysis;
}

SeasonalAnalysis TemporalSequenceAnalyzer::analyzeSeasonalPattern(uint32_t minMonths) {
    // Check if we need to update cached analysis
    unsigned long now = millis();
    if (now - lastSeasonalUpdate_ < 86400000) { // Update daily
        return cachedSeasonalAnalysis_;
    }
    
    SeasonalAnalysis analysis;
    
    if (behaviorHistory_.size() < minMonths * 30) { // Rough estimate
        return analysis;
    }
    
    // Calculate activity by month
    std::vector<uint32_t> monthCounts(12, 0);
    std::vector<std::vector<uint32_t>> monthBehaviorCounts(12, std::vector<uint32_t>(11, 0));
    
    for (size_t i = 0; i < behaviorHistory_.size(); i++) {
        uint8_t month = getMonthOfYear(timestampHistory_[i]);
        BehaviorType behavior = behaviorHistory_[i].primaryBehavior;
        
        if (month >= 1 && month <= 12) {
            monthCounts[month - 1]++;
            if (static_cast<int>(behavior) < 11) {
                monthBehaviorCounts[month - 1][static_cast<int>(behavior)]++;
            }
        }
    }
    
    // Calculate activity levels and probabilities
    uint32_t maxCount = *std::max_element(monthCounts.begin(), monthCounts.end());
    for (int month = 0; month < 12; month++) {
        analysis.monthlyActivity[month] = maxCount > 0 ?
            static_cast<float>(monthCounts[month]) / maxCount : 0.0f;
        
        // Calculate behavior probabilities for this month
        uint32_t totalBehaviors = monthCounts[month];
        for (int behavior = 0; behavior < 11; behavior++) {
            analysis.monthlyBehaviorProb[month][behavior] = totalBehaviors > 0 ?
                static_cast<float>(monthBehaviorCounts[month][behavior]) / totalBehaviors : 0.0f;
        }
    }
    
    // Identify migration and breeding patterns
    identifyMigrationPatterns(analysis);
    identifyBreedingSeasons(analysis);
    
    // Calculate seasonal metrics
    calculateSeasonalMetrics(analysis);
    
    cachedSeasonalAnalysis_ = analysis;
    lastSeasonalUpdate_ = now;
    
    return analysis;
}

std::vector<uint32_t> TemporalSequenceAnalyzer::detectTemporalAnomalies(
    const TemporalSequence& sequence, float threshold) {
    
    std::vector<uint32_t> anomalies;
    
    if (sequence.behaviors.size() < 3) {
        return anomalies;
    }
    
    // Calculate expected behavior patterns
    std::map<BehaviorType, float> behaviorFrequencies;
    for (const auto& behavior : sequence.behaviors) {
        behaviorFrequencies[behavior.primaryBehavior]++;
    }
    
    // Normalize frequencies
    float total = sequence.behaviors.size();
    for (auto& freq : behaviorFrequencies) {
        freq.second /= total;
    }
    
    // Detect anomalies based on unexpected behaviors or timing
    for (size_t i = 0; i < sequence.behaviors.size(); i++) {
        const auto& behavior = sequence.behaviors[i];
        
        // Check behavior frequency anomaly
        float expectedFreq = behaviorFrequencies[behavior.primaryBehavior];
        if (expectedFreq < (1.0f - threshold)) {
            anomalies.push_back(i);
            continue;
        }
        
        // Check timing anomaly (unusually long or short duration)
        if (i > 0) {
            uint32_t timeDiff = behavior.timestamp - sequence.behaviors[i-1].timestamp;
            float avgTimeDiff = (sequence.endTime - sequence.startTime) / sequence.behaviors.size();
            
            if (timeDiff > avgTimeDiff * 3 || timeDiff < avgTimeDiff / 3) {
                anomalies.push_back(i);
            }
        }
    }
    
    return anomalies;
}

float TemporalSequenceAnalyzer::calculateSequenceCoherence(const TemporalSequence& sequence) const {
    if (sequence.behaviors.size() < 2) {
        return 0.0f;
    }
    
    // Calculate coherence based on behavior transitions and timing consistency
    float transitionCoherence = 0.0f;
    float timingCoherence = 0.0f;
    
    // Analyze transitions
    std::map<std::pair<BehaviorType, BehaviorType>, uint32_t> transitions;
    std::vector<uint32_t> timeDiffs;
    
    for (size_t i = 0; i < sequence.behaviors.size() - 1; i++) {
        BehaviorType from = sequence.behaviors[i].primaryBehavior;
        BehaviorType to = sequence.behaviors[i + 1].primaryBehavior;
        transitions[{from, to}]++;
        
        uint32_t timeDiff = sequence.behaviors[i + 1].timestamp - sequence.behaviors[i].timestamp;
        timeDiffs.push_back(timeDiff);
    }
    
    // Calculate transition coherence (how predictable transitions are)
    if (!transitions.empty()) {
        uint32_t totalTransitions = sequence.behaviors.size() - 1;
        uint32_t maxTransitionCount = 0;
        for (const auto& trans : transitions) {
            maxTransitionCount = std::max(maxTransitionCount, trans.second);
        }
        transitionCoherence = static_cast<float>(maxTransitionCount) / totalTransitions;
    }
    
    // Calculate timing coherence (how consistent timing is)
    if (!timeDiffs.empty()) {
        // Calculate coefficient of variation
        float mean = 0.0f;
        for (uint32_t diff : timeDiffs) {
            mean += diff;
        }
        mean /= timeDiffs.size();
        
        float variance = 0.0f;
        for (uint32_t diff : timeDiffs) {
            variance += (diff - mean) * (diff - mean);
        }
        variance /= timeDiffs.size();
        
        float stdDev = sqrt(variance);
        float cv = mean > 0 ? stdDev / mean : 1.0f;
        timingCoherence = std::max(0.0f, 1.0f - cv); // Lower CV = higher coherence
    }
    
    return (transitionCoherence + timingCoherence) / 2.0f;
}

std::map<std::pair<BehaviorType, BehaviorType>, float> TemporalSequenceAnalyzer::getTransitionMatrix(
    uint32_t timeWindow_s) {
    
    std::map<std::pair<BehaviorType, BehaviorType>, float> transitionMatrix;
    
    if (behaviorHistory_.size() < 2) {
        return transitionMatrix;
    }
    
    // Get recent behaviors within time window
    unsigned long cutoffTime = millis() - (timeWindow_s * 1000);
    std::vector<BehaviorResult> recentBehaviors;
    
    for (size_t i = 0; i < behaviorHistory_.size(); i++) {
        if (timestampHistory_[i] >= cutoffTime) {
            recentBehaviors.push_back(behaviorHistory_[i]);
        }
    }
    
    if (recentBehaviors.size() < 2) {
        return transitionMatrix;
    }
    
    // Count transitions
    std::map<std::pair<BehaviorType, BehaviorType>, uint32_t> transitionCounts;
    std::map<BehaviorType, uint32_t> stateCounts;
    
    for (size_t i = 0; i < recentBehaviors.size() - 1; i++) {
        BehaviorType from = recentBehaviors[i].primaryBehavior;
        BehaviorType to = recentBehaviors[i + 1].primaryBehavior;
        
        transitionCounts[{from, to}]++;
        stateCounts[from]++;
    }
    
    // Calculate probabilities
    for (const auto& transition : transitionCounts) {
        BehaviorType from = transition.first.first;
        uint32_t count = transition.second;
        uint32_t totalFromState = stateCounts[from];
        
        float probability = static_cast<float>(count) / totalFromState;
        transitionMatrix[transition.first] = probability;
    }
    
    return transitionMatrix;
}

void TemporalSequenceAnalyzer::clearData() {
    behaviorHistory_.clear();
    environmentHistory_.clear();
    timestampHistory_.clear();
    cachedCircadianAnalysis_ = CircadianAnalysis();
    cachedSeasonalAnalysis_ = SeasonalAnalysis();
    lastCircadianUpdate_ = 0;
    lastSeasonalUpdate_ = 0;
    analysisMetrics_ = AIMetrics();
    totalAnalyses_ = 0;
    successfulAnalyses_ = 0;
}

void TemporalSequenceAnalyzer::configure(const TemporalAnalysisConfig& config) {
    config_ = config;
}

bool TemporalSequenceAnalyzer::hasSufficientData(uint8_t analysisType) const {
    switch (analysisType) {
        case 0: // Short-term
            return behaviorHistory_.size() >= 5;
        case 1: // Medium-term
            return behaviorHistory_.size() >= 20;
        case 2: // Long-term
            return behaviorHistory_.size() >= 100;
        default:
            return false;
    }
}

// Private methods implementation
std::vector<TemporalSequence> TemporalSequenceAnalyzer::extractSequences(
    const TemporalWindow& window, uint32_t timeWindow_s) {
    
    std::vector<TemporalSequence> sequences;
    
    if (behaviorHistory_.empty()) {
        return sequences;
    }
    
    unsigned long currentTime = millis();
    unsigned long analysisEnd = timeWindow_s > 0 ? 
        currentTime - (timeWindow_s * 1000) : timestampHistory_.front();
    
    // Extract sequences using sliding window
    for (unsigned long windowStart = timestampHistory_.back(); 
         windowStart >= analysisEnd; 
         windowStart -= (window.stepSize_s * 1000)) {
        
        TemporalSequence sequence = getSequence(window, windowStart + (window.windowSize_s * 1000));
        
        if (sequence.behaviors.size() >= window.minObservations) {
            sequences.push_back(sequence);
        }
        
        if (!window.overlapping) {
            windowStart -= (window.windowSize_s * 1000) - (window.stepSize_s * 1000);
        }
    }
    
    return sequences;
}

void TemporalSequenceAnalyzer::analyzeSequenceStatistics(TemporalSequence& sequence) {
    if (sequence.behaviors.empty()) {
        return;
    }
    
    // Calculate dominant behavior
    std::map<BehaviorType, uint32_t> behaviorCounts;
    uint32_t totalTransitions = 0;
    float totalActivity = 0.0f;
    
    for (const auto& behavior : sequence.behaviors) {
        behaviorCounts[behavior.primaryBehavior]++;
        totalActivity += behavior.activity_level;
    }
    
    // Find dominant behavior
    uint32_t maxCount = 0;
    for (const auto& count : behaviorCounts) {
        if (count.second > maxCount) {
            maxCount = count.second;
            sequence.dominantBehavior = count.first;
        }
    }
    
    // Count transitions
    for (size_t i = 0; i < sequence.behaviors.size() - 1; i++) {
        if (sequence.behaviors[i].primaryBehavior != sequence.behaviors[i + 1].primaryBehavior) {
            totalTransitions++;
        }
    }
    
    sequence.behaviorTransitions = totalTransitions;
    sequence.activityLevel = sequence.behaviors.size() > 0 ? 
        totalActivity / sequence.behaviors.size() : 0.0f;
    
    // Calculate sequence coherence
    sequence.sequenceCoherence = calculateSequenceCoherence(sequence);
}

void TemporalSequenceAnalyzer::calculateBehaviorEntropy(TemporalSequence& sequence) {
    if (sequence.behaviors.empty()) {
        return;
    }
    
    // Calculate behavior distribution
    std::map<BehaviorType, uint32_t> behaviorCounts;
    for (const auto& behavior : sequence.behaviors) {
        behaviorCounts[behavior.primaryBehavior]++;
    }
    
    // Calculate entropy
    float entropy = 0.0f;
    float total = sequence.behaviors.size();
    for (const auto& count : behaviorCounts) {
        float probability = count.second / total;
        if (probability > 0) {
            entropy -= probability * log2f(probability);
        }
    }
    
    sequence.statistics.entropy = entropy;
    sequence.statistics.predictability = entropy > 0 ? 1.0f - (entropy / log2f(behaviorCounts.size())) : 1.0f;
}

void TemporalSequenceAnalyzer::updateCircadianData(const BehaviorResult& behavior) {
    // This would update internal circadian tracking data
    // For now, data is updated when analyzeCircadianPattern is called
}

void TemporalSequenceAnalyzer::updateSeasonalData(const BehaviorResult& behavior) {
    // This would update internal seasonal tracking data
    // For now, data is updated when analyzeSeasonalPattern is called
}

void TemporalSequenceAnalyzer::identifyActivityPeaks(CircadianAnalysis& analysis) {
    // Find peaks in hourly activity
    for (int hour = 0; hour < 24; hour++) {
        bool isPeak = true;
        float currentActivity = analysis.hourlyActivity[hour];
        
        // Check if this hour has higher activity than neighbors
        int prevHour = (hour + 23) % 24;
        int nextHour = (hour + 1) % 24;
        
        if (currentActivity <= analysis.hourlyActivity[prevHour] ||
            currentActivity <= analysis.hourlyActivity[nextHour] ||
            currentActivity < 0.3f) { // Minimum threshold for peak
            isPeak = false;
        }
        
        if (isPeak) {
            CircadianAnalysis::ActivityPeak peak;
            peak.startHour = hour;
            peak.endHour = hour; // Simplified - could extend to neighboring hours
            peak.intensity = currentActivity;
            
            // Find primary behavior for this peak
            float maxProb = 0.0f;
            for (int behavior = 0; behavior < 11; behavior++) {
                if (analysis.hourlyBehaviorProb[hour][behavior] > maxProb) {
                    maxProb = analysis.hourlyBehaviorProb[hour][behavior];
                    peak.primaryBehavior = static_cast<BehaviorType>(behavior);
                }
            }
            
            analysis.peakPeriods.push_back(peak);
        }
    }
}

void TemporalSequenceAnalyzer::calculateCircadianMetrics(CircadianAnalysis& analysis) {
    // Calculate circadian strength (how pronounced the 24h pattern is)
    float meanActivity = 0.0f;
    for (int hour = 0; hour < 24; hour++) {
        meanActivity += analysis.hourlyActivity[hour];
    }
    meanActivity /= 24.0f;
    
    float variance = 0.0f;
    for (int hour = 0; hour < 24; hour++) {
        variance += (analysis.hourlyActivity[hour] - meanActivity) * 
                   (analysis.hourlyActivity[hour] - meanActivity);
    }
    variance /= 24.0f;
    
    analysis.circadianStrength = sqrt(variance) / meanActivity;
    
    // Determine activity pattern type
    float dayActivity = 0.0f; // 6 AM to 6 PM
    float nightActivity = 0.0f; // 6 PM to 6 AM
    float dawnActivity = (analysis.hourlyActivity[5] + analysis.hourlyActivity[6] + analysis.hourlyActivity[7]) / 3.0f;
    float duskActivity = (analysis.hourlyActivity[17] + analysis.hourlyActivity[18] + analysis.hourlyActivity[19]) / 3.0f;
    
    for (int hour = 6; hour < 18; hour++) {
        dayActivity += analysis.hourlyActivity[hour];
    }
    for (int hour = 18; hour < 24; hour++) {
        nightActivity += analysis.hourlyActivity[hour];
    }
    for (int hour = 0; hour < 6; hour++) {
        nightActivity += analysis.hourlyActivity[hour];
    }
    
    dayActivity /= 12.0f;
    nightActivity /= 12.0f;
    
    analysis.isDiurnal = dayActivity > nightActivity * 1.5f;
    analysis.isNocturnal = nightActivity > dayActivity * 1.5f;
    analysis.isCrepuscular = (dawnActivity > dayActivity * 1.2f) || (duskActivity > dayActivity * 1.2f);
}

uint8_t TemporalSequenceAnalyzer::getHourOfDay(unsigned long timestamp) const {
    // Convert timestamp to hour of day (0-23)
    // This is a simplified implementation
    return (timestamp / 3600000) % 24;
}

uint8_t TemporalSequenceAnalyzer::getMonthOfYear(unsigned long timestamp) const {
    // Convert timestamp to month of year (1-12)
    // This is a simplified implementation - would need proper date handling
    return ((timestamp / (86400000 * 30)) % 12) + 1;
}

void TemporalSequenceAnalyzer::identifyMigrationPatterns(SeasonalAnalysis& analysis) {
    // Look for seasonal movement patterns
    // This is a simplified implementation
    for (int month = 0; month < 12; month++) {
        // Check for significant changes in activity that might indicate migration
        int nextMonth = (month + 1) % 12;
        float activityChange = fabs(analysis.monthlyActivity[nextMonth] - analysis.monthlyActivity[month]);
        
        if (activityChange > 0.5f) { // Significant change threshold
            SeasonalAnalysis::MigrationPattern pattern;
            pattern.startMonth = month + 1;
            pattern.endMonth = nextMonth + 1;
            pattern.intensity = activityChange;
            pattern.isInbound = analysis.monthlyActivity[nextMonth] > analysis.monthlyActivity[month];
            
            analysis.migrationPatterns.push_back(pattern);
            analysis.hasMigration = true;
        }
    }
}

void TemporalSequenceAnalyzer::identifyBreedingSeasons(SeasonalAnalysis& analysis) {
    // Look for mating behavior peaks that indicate breeding seasons
    for (int month = 0; month < 12; month++) {
        // Check for mating behavior probability
        int matingBehaviorIndex = static_cast<int>(BehaviorType::MATING);
        if (matingBehaviorIndex < 11) {
            float matingProb = analysis.monthlyBehaviorProb[month][matingBehaviorIndex];
            
            if (matingProb > 0.1f) { // Significant mating activity
                analysis.breedingSeason.peakMonth = month + 1;
                analysis.breedingSeason.intensity = matingProb;
                analysis.breedingSeason.durationMonths = 1; // Simplified
                analysis.hasBreedingSeason = true;
            }
        }
    }
}

void TemporalSequenceAnalyzer::calculateSeasonalMetrics(SeasonalAnalysis& analysis) {
    // Calculate how strong seasonal patterns are
    float meanActivity = 0.0f;
    for (int month = 0; month < 12; month++) {
        meanActivity += analysis.monthlyActivity[month];
    }
    meanActivity /= 12.0f;
    
    float variance = 0.0f;
    for (int month = 0; month < 12; month++) {
        variance += (analysis.monthlyActivity[month] - meanActivity) * 
                   (analysis.monthlyActivity[month] - meanActivity);
    }
    variance /= 12.0f;
    
    analysis.seasonalStrength = meanActivity > 0 ? sqrt(variance) / meanActivity : 0.0f;
}

void TemporalSequenceAnalyzer::analyzeTrends(TemporalAnalysisResult& result) {
    // Simple trend analysis - check if activity is increasing or decreasing
    if (behaviorHistory_.size() < 10) {
        return;
    }
    
    // Compare recent activity to earlier activity
    size_t halfPoint = behaviorHistory_.size() / 2;
    float earlyActivity = 0.0f;
    float recentActivity = 0.0f;
    
    for (size_t i = 0; i < halfPoint; i++) {
        earlyActivity += behaviorHistory_[i].activity_level;
    }
    for (size_t i = halfPoint; i < behaviorHistory_.size(); i++) {
        recentActivity += behaviorHistory_[i].activity_level;
    }
    
    earlyActivity /= halfPoint;
    recentActivity /= (behaviorHistory_.size() - halfPoint);
    
    float change = recentActivity - earlyActivity;
    result.trends.activityIncreasing = change > 0.1f;
    result.trends.activityDecreasing = change < -0.1f;
    result.trends.trendStrength = fabs(change);
}

uint32_t TemporalSequenceAnalyzer::calculateDataQuality() {
    if (behaviorHistory_.empty()) {
        return 0;
    }
    
    uint32_t quality = 0;
    
    // Check data quantity
    if (behaviorHistory_.size() >= 100) quality += 25;
    else if (behaviorHistory_.size() >= 50) quality += 15;
    else if (behaviorHistory_.size() >= 10) quality += 5;
    
    // Check data diversity (different behavior types)
    std::set<BehaviorType> uniqueBehaviors;
    for (const auto& behavior : behaviorHistory_) {
        uniqueBehaviors.insert(behavior.primaryBehavior);
    }
    if (uniqueBehaviors.size() >= 5) quality += 25;
    else if (uniqueBehaviors.size() >= 3) quality += 15;
    else if (uniqueBehaviors.size() >= 2) quality += 10;
    
    // Check temporal span
    if (!timestampHistory_.empty()) {
        uint32_t span = timestampHistory_.back() - timestampHistory_.front();
        uint32_t days = span / 86400000; // Convert to days
        if (days >= 7) quality += 25;
        else if (days >= 3) quality += 15;
        else if (days >= 1) quality += 10;
    }
    
    // Check data consistency (confidence levels)
    float avgConfidence = 0.0f;
    for (const auto& behavior : behaviorHistory_) {
        avgConfidence += behavior.confidence;
    }
    avgConfidence /= behaviorHistory_.size();
    if (avgConfidence >= 0.8f) quality += 25;
    else if (avgConfidence >= 0.6f) quality += 15;
    else if (avgConfidence >= 0.4f) quality += 10;
    
    return quality;
}

void TemporalSequenceAnalyzer::pruneOldData() {
    // Keep only recent data to manage memory
    const size_t maxHistory = 10000;
    if (behaviorHistory_.size() > maxHistory) {
        size_t removeCount = behaviorHistory_.size() - maxHistory;
        behaviorHistory_.erase(behaviorHistory_.begin(), behaviorHistory_.begin() + removeCount);
        environmentHistory_.erase(environmentHistory_.begin(), environmentHistory_.begin() + removeCount);
        timestampHistory_.erase(timestampHistory_.begin(), timestampHistory_.begin() + removeCount);
    }
}

void TemporalSequenceAnalyzer::updateAnalysisMetrics(bool success, uint32_t processingTime) {
    totalAnalyses_++;
    if (success) {
        successfulAnalyses_++;
    }
    
    analysisMetrics_.totalInferences = totalAnalyses_;
    analysisMetrics_.successfulInferences = successfulAnalyses_;
    analysisMetrics_.inferenceTime = processingTime;
}

AIMetrics TemporalSequenceAnalyzer::getAnalysisMetrics() const {
    return analysisMetrics_;
}