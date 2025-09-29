/**
 * @file behavior_pattern_detector.cpp
 * @brief Implementation of Advanced Behavior Pattern Detection System
 */

#include "behavior_pattern_detector.h"
#include <algorithm>
#include <cmath>

BehaviorPatternDetector::BehaviorPatternDetector() : 
    initialized_(false), totalDetections_(0), successfulDetections_(0) {
}

BehaviorPatternDetector::~BehaviorPatternDetector() {
    // Destructors handle cleanup
}

bool BehaviorPatternDetector::init(const PatternDetectionConfig& config) {
    config_ = config;
    
    // Initialize storage
    detectedPatterns_.reserve(100);
    behaviorHistory_.reserve(1000);
    environmentHistory_.reserve(1000);
    
    // Initialize metrics
    detectionMetrics_ = AIMetrics();
    
    initialized_ = true;
    return true;
}

PatternDetectionResult BehaviorPatternDetector::detectPatterns(
    const std::vector<BehaviorResult>& behaviors,
    const EnvironmentalData& environment) {
    
    PatternDetectionResult result;
    
    if (!initialized_ || behaviors.empty()) {
        return result;
    }
    
    uint32_t startTime = millis();
    
    // Use different algorithms based on configuration
    std::vector<EnhancedBehaviorPattern> newPatterns;
    
    switch (config_.primaryAlgorithm) {
        case PatternAlgorithm::SEQUENCE_MATCHING:
            newPatterns = detectSequencePatterns(behaviors);
            break;
        case PatternAlgorithm::MARKOV_CHAIN:
            newPatterns = detectMarkovPatterns(behaviors);
            break;
        case PatternAlgorithm::STATISTICAL_ANALYSIS:
            newPatterns = detectStatisticalPatterns(behaviors);
            break;
        case PatternAlgorithm::HYBRID_APPROACH:
            // Combine multiple approaches
            auto seq = detectSequencePatterns(behaviors);
            auto markov = detectMarkovPatterns(behaviors);
            newPatterns.insert(newPatterns.end(), seq.begin(), seq.end());
            newPatterns.insert(newPatterns.end(), markov.begin(), markov.end());
            break;
        default:
            newPatterns = detectSequencePatterns(behaviors);
            break;
    }
    
    // Filter patterns by confidence threshold
    for (auto& pattern : newPatterns) {
        if (pattern.confidence >= config_.minimumConfidence) {
            // Check if pattern already exists
            auto existing = std::find_if(detectedPatterns_.begin(), detectedPatterns_.end(),
                [&pattern](const EnhancedBehaviorPattern& p) {
                    return isSequenceMatch(p.sequence, pattern.sequence, 0.9f);
                });
            
            if (existing != detectedPatterns_.end()) {
                // Update existing pattern
                mergePatterns(*existing, pattern);
                result.updatedPatterns.push_back(*existing);
            } else {
                // Add new pattern
                pattern.patternId = generatePatternId(pattern.sequence);
                detectedPatterns_.push_back(pattern);
                patternIndex_[pattern.patternId] = detectedPatterns_.size() - 1;
                result.newPatterns.push_back(pattern);
            }
        }
    }
    
    // Copy all patterns above threshold
    for (const auto& pattern : detectedPatterns_) {
        if (pattern.confidence >= config_.minimumConfidence) {
            result.detectedPatterns.push_back(pattern);
        }
    }
    
    // Update result statistics
    result.totalPatternsAnalyzed = newPatterns.size();
    result.patternsAboveThreshold = result.detectedPatterns.size();
    
    if (!result.detectedPatterns.empty()) {
        float totalConfidence = 0.0f;
        for (const auto& pattern : result.detectedPatterns) {
            totalConfidence += pattern.confidence;
        }
        result.averagePatternConfidence = totalConfidence / result.detectedPatterns.size();
    }
    
    result.processingTime_ms = millis() - startTime;
    updateDetectionMetrics(true, result.processingTime_ms);
    
    return result;
}

void BehaviorPatternDetector::addBehaviorObservation(const BehaviorResult& behavior,
                                                   const EnvironmentalData& environment) {
    if (!initialized_) {
        return;
    }
    
    // Add to history
    behaviorHistory_.push_back(behavior);
    environmentHistory_.push_back(environment);
    
    // Limit history size for memory management
    const size_t maxHistory = 1000;
    if (behaviorHistory_.size() > maxHistory) {
        behaviorHistory_.erase(behaviorHistory_.begin());
        environmentHistory_.erase(environmentHistory_.begin());
    }
}

std::vector<EnhancedBehaviorPattern> BehaviorPatternDetector::getAllPatterns() const {
    return detectedPatterns_;
}

std::vector<EnhancedBehaviorPattern> BehaviorPatternDetector::getPatterns(
    BehaviorType behaviorType, float minConfidence) const {
    
    std::vector<EnhancedBehaviorPattern> matchingPatterns;
    
    for (const auto& pattern : detectedPatterns_) {
        if (pattern.confidence >= minConfidence) {
            // Check if pattern contains the behavior type
            for (const auto& behavior : pattern.sequence) {
                if (behavior == behaviorType) {
                    matchingPatterns.push_back(pattern);
                    break;
                }
            }
        }
    }
    
    return matchingPatterns;
}

std::pair<BehaviorType, float> BehaviorPatternDetector::predictNextBehavior(
    const std::vector<BehaviorType>& recentBehaviors,
    const EnvironmentalData& currentEnvironment) const {
    
    if (!initialized_ || recentBehaviors.empty()) {
        return {BehaviorType::UNKNOWN, 0.0f};
    }
    
    float bestConfidence = 0.0f;
    BehaviorType predictedBehavior = BehaviorType::UNKNOWN;
    
    // Look for patterns that match the recent behavior sequence
    for (const auto& pattern : detectedPatterns_) {
        if (pattern.sequence.size() <= recentBehaviors.size()) {
            continue; // Pattern too short to predict next
        }
        
        // Check if recent behaviors match the beginning of this pattern
        bool matches = true;
        size_t matchLength = std::min(recentBehaviors.size(), pattern.sequence.size() - 1);
        
        for (size_t i = 0; i < matchLength; i++) {
            size_t recentIndex = recentBehaviors.size() - matchLength + i;
            if (recentBehaviors[recentIndex] != pattern.sequence[i]) {
                matches = false;
                break;
            }
        }
        
        if (matches && pattern.confidence > bestConfidence) {
            bestConfidence = pattern.confidence;
            predictedBehavior = pattern.sequence[matchLength];
        }
    }
    
    return {predictedBehavior, bestConfidence};
}

void BehaviorPatternDetector::clearPatterns() {
    detectedPatterns_.clear();
    patternIndex_.clear();
    behaviorHistory_.clear();
    environmentHistory_.clear();
    detectionMetrics_ = AIMetrics();
    totalDetections_ = 0;
    successfulDetections_ = 0;
}

// Private methods implementation
std::vector<EnhancedBehaviorPattern> BehaviorPatternDetector::detectSequencePatterns(
    const std::vector<BehaviorResult>& behaviors) const {
    
    std::vector<EnhancedBehaviorPattern> patterns;
    
    if (behaviors.size() < 2) {
        return patterns;
    }
    
    // Look for repeating sequences of different lengths
    for (size_t patternLength = 2; patternLength <= config_.maxPatternLength && 
         patternLength <= behaviors.size() / 2; patternLength++) {
        
        for (size_t start = 0; start <= behaviors.size() - (patternLength * 2); start++) {
            std::vector<BehaviorType> sequence;
            
            // Extract potential pattern
            for (size_t i = 0; i < patternLength; i++) {
                sequence.push_back(behaviors[start + i].primaryBehavior);
            }
            
            // Look for repetitions
            uint32_t repetitions = 1;
            for (size_t pos = start + patternLength; 
                 pos <= behaviors.size() - patternLength; pos += patternLength) {
                
                bool matches = true;
                for (size_t i = 0; i < patternLength; i++) {
                    if (behaviors[pos + i].primaryBehavior != sequence[i]) {
                        matches = false;
                        break;
                    }
                }
                
                if (matches) {
                    repetitions++;
                } else {
                    break;
                }
            }
            
            // If we found repetitions, create a pattern
            if (repetitions >= config_.minimumObservations) {
                EnhancedBehaviorPattern pattern;
                pattern.sequence = sequence;
                pattern.confidence = std::min(static_cast<float>(repetitions) / 10.0f, 1.0f);
                pattern.observationCount = repetitions;
                pattern.detectionMethod = PatternAlgorithm::SEQUENCE_MATCHING;
                pattern.detectionAccuracy = pattern.confidence;
                
                // Calculate temporal characteristics
                analyzeTemporalCharacteristics(pattern, behaviors);
                
                patterns.push_back(pattern);
            }
        }
    }
    
    return patterns;
}

std::vector<EnhancedBehaviorPattern> BehaviorPatternDetector::detectMarkovPatterns(
    const std::vector<BehaviorResult>& behaviors) const {
    
    std::vector<EnhancedBehaviorPattern> patterns;
    
    if (behaviors.size() < 3) {
        return patterns;
    }
    
    // Build transition matrix
    std::map<std::pair<BehaviorType, BehaviorType>, uint32_t> transitions;
    std::map<BehaviorType, uint32_t> stateCounts;
    
    for (size_t i = 0; i < behaviors.size() - 1; i++) {
        BehaviorType from = behaviors[i].primaryBehavior;
        BehaviorType to = behaviors[i + 1].primaryBehavior;
        
        transitions[{from, to}]++;
        stateCounts[from]++;
    }
    
    // Create patterns from high-probability transitions
    for (const auto& transition : transitions) {
        BehaviorType from = transition.first.first;
        BehaviorType to = transition.first.second;
        uint32_t count = transition.second;
        
        float probability = static_cast<float>(count) / stateCounts[from];
        
        if (probability >= 0.7f && count >= config_.minimumObservations) {
            EnhancedBehaviorPattern pattern;
            pattern.sequence = {from, to};
            pattern.confidence = probability;
            pattern.observationCount = count;
            pattern.detectionMethod = PatternAlgorithm::MARKOV_CHAIN;
            pattern.detectionAccuracy = probability;
            
            // Add transition probabilities
            pattern.nextBehaviorProbabilities[to] = probability;
            pattern.transitionCounts[to] = count;
            
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<EnhancedBehaviorPattern> BehaviorPatternDetector::detectStatisticalPatterns(
    const std::vector<BehaviorResult>& behaviors) const {
    
    std::vector<EnhancedBehaviorPattern> patterns;
    
    // Count behavior frequencies
    std::map<BehaviorType, uint32_t> behaviorCounts;
    for (const auto& behavior : behaviors) {
        behaviorCounts[behavior.primaryBehavior]++;
    }
    
    // Find dominant behaviors (statistical significance)
    for (const auto& count : behaviorCounts) {
        float frequency = static_cast<float>(count.second) / behaviors.size();
        
        if (frequency >= 0.3f && count.second >= config_.minimumObservations) {
            EnhancedBehaviorPattern pattern;
            pattern.sequence = {count.first}; // Single behavior pattern
            pattern.confidence = frequency;
            pattern.observationCount = count.second;
            pattern.detectionMethod = PatternAlgorithm::STATISTICAL_ANALYSIS;
            pattern.detectionAccuracy = frequency;
            
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

void BehaviorPatternDetector::analyzeTemporalCharacteristics(
    EnhancedBehaviorPattern& pattern,
    const std::vector<BehaviorResult>& observations) const {
    
    if (observations.empty()) return;
    
    // Calculate average duration
    uint32_t totalDuration = 0;
    uint32_t count = 0;
    
    for (const auto& obs : observations) {
        if (std::find(pattern.sequence.begin(), pattern.sequence.end(), 
                     obs.primaryBehavior) != pattern.sequence.end()) {
            totalDuration += obs.duration;
            count++;
        }
    }
    
    if (count > 0) {
        pattern.temporal.averageDuration_s = totalDuration / count;
        pattern.temporal.minDuration_s = pattern.temporal.averageDuration_s / 2;
        pattern.temporal.maxDuration_s = pattern.temporal.averageDuration_s * 2;
    }
}

bool BehaviorPatternDetector::isSequenceMatch(
    const std::vector<BehaviorType>& sequence1,
    const std::vector<BehaviorType>& sequence2,
    float tolerance) const {
    
    if (sequence1.size() != sequence2.size()) {
        return false;
    }
    
    size_t matches = 0;
    for (size_t i = 0; i < sequence1.size(); i++) {
        if (sequence1[i] == sequence2[i]) {
            matches++;
        }
    }
    
    float similarity = static_cast<float>(matches) / sequence1.size();
    return similarity >= tolerance;
}

String BehaviorPatternDetector::generatePatternId(const std::vector<BehaviorType>& sequence) const {
    String id = "PAT_";
    for (const auto& behavior : sequence) {
        id += String(static_cast<int>(behavior)) + "_";
    }
    id += String(millis());
    return id;
}

void BehaviorPatternDetector::mergePatterns(EnhancedBehaviorPattern& target,
                                          const EnhancedBehaviorPattern& source) {
    // Update observation count and confidence
    uint32_t totalObs = target.observationCount + source.observationCount;
    target.confidence = (target.confidence * target.observationCount + 
                        source.confidence * source.observationCount) / totalObs;
    target.observationCount = totalObs;
    
    // Update last observed time
    target.lastObserved = millis();
}

void BehaviorPatternDetector::updateDetectionMetrics(bool success, uint32_t processingTime) {
    totalDetections_++;
    if (success) {
        successfulDetections_++;
    }
    
    detectionMetrics_.totalInferences = totalDetections_;
    detectionMetrics_.successfulInferences = successfulDetections_;
    detectionMetrics_.inferenceTime = processingTime;
    
    if (totalDetections_ > 0) {
        // Update average processing time
        detectionMetrics_.preprocessingTime = 
            (detectionMetrics_.preprocessingTime + processingTime) / 2.0f;
    }
}

AIMetrics BehaviorPatternDetector::getDetectionMetrics() const {
    return detectionMetrics_;
}