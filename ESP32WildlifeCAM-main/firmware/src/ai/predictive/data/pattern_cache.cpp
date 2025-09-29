/**
 * @file pattern_cache.cpp
 * @brief Implementation of Fast Pattern Lookup and Caching System
 */

#include "pattern_cache.h"
#include <algorithm>
#include <cmath>

PatternCache::PatternCache() : initialized_(false) {
}

PatternCache::~PatternCache() {
    // Destructors handle cleanup
}

bool PatternCache::init(const CacheConfig& config) {
    config_ = config;
    
    // Reserve space for cache storage
    cachedPatterns_.reserve(config.maxPatterns);
    accessOrder_.reserve(config.maxPatterns);
    
    // Initialize metrics
    metrics_ = CacheMetrics();
    
    initialized_ = true;
    return true;
}

bool PatternCache::addPattern(const EnhancedBehaviorPattern& pattern) {
    if (!initialized_) {
        return false;
    }
    
    // Check if pattern already exists
    if (patternIdIndex_.find(pattern.patternId) != patternIdIndex_.end()) {
        return updatePattern(pattern);
    }
    
    // Check memory constraints
    if (cachedPatterns_.size() >= config_.maxPatterns) {
        evictPatterns();
    }
    
    // Create cache entry
    PatternCacheEntry entry;
    entry.patternId = pattern.patternId;
    entry.pattern = pattern;
    entry.cacheTimestamp = millis();
    entry.relevanceScore = calculateRelevanceScore(entry);
    
    // Add to cache
    cachedPatterns_.push_back(entry);
    size_t index = cachedPatterns_.size() - 1;
    patternIdIndex_[pattern.patternId] = index;
    
    // Update indices
    updateIndex(pattern, index);
    updateAccessOrder(index);
    
    metrics_.totalPatterns++;
    return true;
}

bool PatternCache::removePattern(const String& patternId) {
    if (!initialized_) {
        return false;
    }
    
    auto it = patternIdIndex_.find(patternId);
    if (it == patternIdIndex_.end()) {
        return false;
    }
    
    size_t index = it->second;
    
    // Remove from indices
    removeFromIndex(patternId, index);
    
    // Remove from cache
    cachedPatterns_.erase(cachedPatterns_.begin() + index);
    patternIdIndex_.erase(it);
    
    // Update remaining indices
    for (auto& pair : patternIdIndex_) {
        if (pair.second > index) {
            pair.second--;
        }
    }
    
    metrics_.totalPatterns--;
    return true;
}

bool PatternCache::updatePattern(const EnhancedBehaviorPattern& pattern) {
    if (!initialized_) {
        return false;
    }
    
    auto it = patternIdIndex_.find(pattern.patternId);
    if (it == patternIdIndex_.end()) {
        return addPattern(pattern);
    }
    
    size_t index = it->second;
    PatternCacheEntry& entry = cachedPatterns_[index];
    
    // Update pattern data
    entry.pattern = pattern;
    entry.relevanceScore = calculateRelevanceScore(entry);
    
    // Update access order
    updateAccessOrder(index);
    
    return true;
}

PatternMatchResult PatternCache::findMatches(const PatternMatchRequest& request) {
    PatternMatchResult result;
    
    if (!initialized_) {
        return result;
    }
    
    uint32_t startTime = millis();
    metrics_.totalAccesses++;
    
    std::vector<std::pair<float, size_t>> candidates; // Score, index pairs
    
    // Search through cached patterns
    for (size_t i = 0; i < cachedPatterns_.size(); i++) {
        const EnhancedBehaviorPattern& pattern = cachedPatterns_[i].pattern;
        
        // Filter by species if specified
        if (request.species != SpeciesType::UNKNOWN) {
            // Would need species information in pattern
        }
        
        // Calculate match score
        float matchScore = calculatePatternMatch(pattern, request);
        
        if (matchScore >= request.minConfidence) {
            candidates.push_back({matchScore, i});
            updateAccessOrder(i);
        }
    }
    
    // Sort by match score
    std::sort(candidates.begin(), candidates.end(),
             [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Prepare results
    size_t maxResults = std::min(static_cast<size_t>(request.maxMatches), candidates.size());
    for (size_t i = 0; i < maxResults; i++) {
        size_t patternIndex = candidates[i].second;
        result.matchedPatterns.push_back(cachedPatterns_[patternIndex].pattern);
        result.matchScores.push_back(candidates[i].first);
        result.matchReasons.push_back("Sequence and environmental match");
    }
    
    result.totalCandidates = cachedPatterns_.size();
    result.processingTime_ms = millis() - startTime;
    result.fromCache = true;
    
    bool hit = !result.matchedPatterns.empty();
    updateMetrics(hit, result.processingTime_ms);
    
    return result;
}

const EnhancedBehaviorPattern* PatternCache::getPattern(const String& patternId) {
    if (!initialized_) {
        return nullptr;
    }
    
    auto it = patternIdIndex_.find(patternId);
    if (it == patternIdIndex_.end()) {
        return nullptr;
    }
    
    size_t index = it->second;
    updateAccessOrder(index);
    
    return &cachedPatterns_[index].pattern;
}

std::vector<EnhancedBehaviorPattern> PatternCache::getPatternsByBehavior(
    BehaviorType behaviorType, float minConfidence) {
    
    std::vector<EnhancedBehaviorPattern> matches;
    
    if (!initialized_) {
        return matches;
    }
    
    // Use behavior index if available
    auto indexIt = patternIndex_.behaviorIndex.find(behaviorType);
    if (indexIt != patternIndex_.behaviorIndex.end()) {
        for (size_t index : indexIt->second) {
            if (index < cachedPatterns_.size() && 
                cachedPatterns_[index].pattern.confidence >= minConfidence) {
                matches.push_back(cachedPatterns_[index].pattern);
                updateAccessOrder(index);
            }
        }
    }
    
    return matches;
}

std::vector<EnhancedBehaviorPattern> PatternCache::getPatternsBySequence(
    const std::vector<BehaviorType>& sequence, bool exactMatch) {
    
    std::vector<EnhancedBehaviorPattern> matches;
    
    if (!initialized_ || sequence.empty()) {
        return matches;
    }
    
    String sequenceHash = generateSequenceHash(sequence);
    
    // Check sequence index
    auto indexIt = patternIndex_.sequenceIndex.find(sequenceHash);
    if (indexIt != patternIndex_.sequenceIndex.end()) {
        for (size_t index : indexIt->second) {
            if (index < cachedPatterns_.size()) {
                const EnhancedBehaviorPattern& pattern = cachedPatterns_[index].pattern;
                
                if (exactMatch) {
                    if (pattern.sequence == sequence) {
                        matches.push_back(pattern);
                        updateAccessOrder(index);
                    }
                } else {
                    float similarity = calculateSequenceMatch(pattern.sequence, sequence, false);
                    if (similarity >= 0.7f) {
                        matches.push_back(pattern);
                        updateAccessOrder(index);
                    }
                }
            }
        }
    }
    
    return matches;
}

std::vector<EnhancedBehaviorPattern> PatternCache::getContextualPatterns(
    BehaviorType currentBehavior,
    const EnvironmentalData& environment,
    uint8_t timeContext) {
    
    std::vector<EnhancedBehaviorPattern> contextualPatterns;
    
    if (!initialized_) {
        return contextualPatterns;
    }
    
    // Get patterns for current behavior
    auto behaviorPatterns = getPatternsByBehavior(currentBehavior, 0.5f);
    
    // Filter by environmental and temporal context
    for (const auto& pattern : behaviorPatterns) {
        float environmentalMatch = calculateEnvironmentalMatch(pattern, environment);
        float temporalMatch = calculateTemporalMatch(pattern, timeContext);
        
        float contextScore = (environmentalMatch + temporalMatch) / 2.0f;
        if (contextScore >= 0.6f) {
            contextualPatterns.push_back(pattern);
        }
    }
    
    return contextualPatterns;
}

std::pair<EnhancedBehaviorPattern, float> PatternCache::predictNextPattern(
    const std::vector<BehaviorType>& recentBehaviors,
    const EnvironmentalData& environment) {
    
    EnhancedBehaviorPattern emptyPattern;
    
    if (!initialized_ || recentBehaviors.empty()) {
        return {emptyPattern, 0.0f};
    }
    
    float bestScore = 0.0f;
    EnhancedBehaviorPattern bestPattern;
    
    // Look for patterns that start with the recent behavior sequence
    for (const auto& entry : cachedPatterns_) {
        const EnhancedBehaviorPattern& pattern = entry.pattern;
        
        if (pattern.sequence.size() <= recentBehaviors.size()) {
            continue;
        }
        
        // Check if recent behaviors match the beginning of this pattern
        bool matches = true;
        size_t checkLength = std::min(recentBehaviors.size(), pattern.sequence.size() - 1);
        
        for (size_t i = 0; i < checkLength; i++) {
            size_t recentIndex = recentBehaviors.size() - checkLength + i;
            if (recentBehaviors[recentIndex] != pattern.sequence[i]) {
                matches = false;
                break;
            }
        }
        
        if (matches) {
            float environmentalScore = calculateEnvironmentalMatch(pattern, environment);
            float totalScore = pattern.confidence * environmentalScore;
            
            if (totalScore > bestScore) {
                bestScore = totalScore;
                bestPattern = pattern;
            }
        }
    }
    
    return {bestPattern, bestScore};
}

uint32_t PatternCache::optimizeCache() {
    if (!initialized_) {
        return 0;
    }
    
    uint32_t removedCount = 0;
    
    // Update relevance scores
    updateRelevanceScores();
    
    // Remove expired patterns
    unsigned long now = millis();
    auto it = cachedPatterns_.begin();
    while (it != cachedPatterns_.end()) {
        if (config_.timeToLive_s > 0 && 
            (now - it->cacheTimestamp) > (config_.timeToLive_s * 1000)) {
            
            // Remove from indices
            removeFromIndex(it->patternId, std::distance(cachedPatterns_.begin(), it));
            it = cachedPatterns_.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    // Remove low relevance patterns if over capacity
    if (cachedPatterns_.size() > config_.maxPatterns * 0.8f) {
        evictByRelevance();
    }
    
    // Update metrics
    metrics_.totalPatterns = cachedPatterns_.size();
    
    return removedCount;
}

void PatternCache::clearCache() {
    cachedPatterns_.clear();
    patternIdIndex_.clear();
    patternIndex_ = PatternIndex();
    accessOrder_.clear();
    metrics_ = CacheMetrics();
}

CacheMetrics PatternCache::getMetrics() const {
    // Update real-time metrics
    const_cast<PatternCache*>(this)->updateMetrics(false, 0);
    return metrics_;
}

float PatternCache::getCacheUtilization() const {
    if (config_.maxPatterns == 0) {
        return 0.0f;
    }
    return static_cast<float>(cachedPatterns_.size()) / config_.maxPatterns;
}

// Private methods implementation
void PatternCache::updateIndex(const EnhancedBehaviorPattern& pattern, size_t index) {
    // Update behavior index
    for (const auto& behavior : pattern.sequence) {
        patternIndex_.behaviorIndex[behavior].push_back(index);
    }
    
    // Update sequence index
    String sequenceHash = generateSequenceHash(pattern.sequence);
    patternIndex_.sequenceIndex[sequenceHash].push_back(index);
    
    // Update temporal indices
    for (size_t hour = 0; hour < pattern.temporal.hourlyProbability.size(); hour++) {
        if (pattern.temporal.hourlyProbability[hour] > 0.5f) {
            patternIndex_.hourIndex[hour].push_back(index);
        }
    }
    
    for (size_t month = 0; month < pattern.temporal.monthlyProbability.size(); month++) {
        if (pattern.temporal.monthlyProbability[month] > 0.5f) {
            patternIndex_.monthIndex[month].push_back(index);
        }
    }
    
    // Update confidence index
    uint8_t confidenceBin = getConfidenceBin(pattern.confidence);
    patternIndex_.confidenceIndex[confidenceBin].push_back(index);
}

void PatternCache::removeFromIndex(const String& patternId, size_t index) {
    // Remove from all indices - simplified implementation
    // In a real system, would need to track which indices contain this pattern
    
    for (auto& behaviorPair : patternIndex_.behaviorIndex) {
        auto& indices = behaviorPair.second;
        indices.erase(std::remove(indices.begin(), indices.end(), index), indices.end());
    }
    
    for (auto& sequencePair : patternIndex_.sequenceIndex) {
        auto& indices = sequencePair.second;
        indices.erase(std::remove(indices.begin(), indices.end(), index), indices.end());
    }
    
    for (auto& hourPair : patternIndex_.hourIndex) {
        auto& indices = hourPair.second;
        indices.erase(std::remove(indices.begin(), indices.end(), index), indices.end());
    }
    
    for (auto& monthPair : patternIndex_.monthIndex) {
        auto& indices = monthPair.second;
        indices.erase(std::remove(indices.begin(), indices.end(), index), indices.end());
    }
    
    for (auto& confidencePair : patternIndex_.confidenceIndex) {
        auto& indices = confidencePair.second;
        indices.erase(std::remove(indices.begin(), indices.end(), index), indices.end());
    }
}

void PatternCache::updateAccessOrder(size_t index) {
    if (!config_.enableLRU) {
        return;
    }
    
    // Remove from current position
    accessOrder_.erase(std::remove(accessOrder_.begin(), accessOrder_.end(), index), 
                      accessOrder_.end());
    
    // Add to end (most recently used)
    accessOrder_.push_back(index);
    
    // Update access count
    if (index < cachedPatterns_.size()) {
        cachedPatterns_[index].accessCount++;
        cachedPatterns_[index].lastAccessed = millis();
    }
}

void PatternCache::evictPatterns() {
    if (config_.enableLRU) {
        evictLRU();
    } else if (config_.enableRelevanceScoring) {
        evictByRelevance();
    } else {
        // Remove oldest pattern
        if (!cachedPatterns_.empty()) {
            removePattern(cachedPatterns_[0].patternId);
        }
    }
}

void PatternCache::evictLRU() {
    if (accessOrder_.empty()) {
        return;
    }
    
    // Remove least recently used pattern
    size_t lruIndex = accessOrder_[0];
    if (lruIndex < cachedPatterns_.size()) {
        removePattern(cachedPatterns_[lruIndex].patternId);
    }
}

void PatternCache::evictByRelevance() {
    if (cachedPatterns_.empty()) {
        return;
    }
    
    // Find pattern with lowest relevance score
    float minRelevance = 1.0f;
    size_t minIndex = 0;
    
    for (size_t i = 0; i < cachedPatterns_.size(); i++) {
        if (cachedPatterns_[i].relevanceScore < minRelevance) {
            minRelevance = cachedPatterns_[i].relevanceScore;
            minIndex = i;
        }
    }
    
    if (minRelevance < config_.relevanceThreshold) {
        removePattern(cachedPatterns_[minIndex].patternId);
    }
}

float PatternCache::calculatePatternMatch(const EnhancedBehaviorPattern& pattern,
                                        const PatternMatchRequest& request) const {
    
    if (request.sequence.empty()) {
        return 0.0f;
    }
    
    // Calculate sequence match
    float sequenceMatch = calculateSequenceMatch(pattern.sequence, request.sequence, request.exactMatch);
    
    // Calculate environmental match
    float environmentalMatch = calculateEnvironmentalMatch(pattern, request.environment);
    
    // Combine scores
    float totalMatch = (sequenceMatch * 0.7f) + (environmentalMatch * 0.3f);
    
    return totalMatch;
}

float PatternCache::calculateSequenceMatch(const std::vector<BehaviorType>& sequence1,
                                         const std::vector<BehaviorType>& sequence2,
                                         bool exactMatch) const {
    
    if (exactMatch) {
        return (sequence1 == sequence2) ? 1.0f : 0.0f;
    }
    
    if (sequence1.empty() || sequence2.empty()) {
        return 0.0f;
    }
    
    // Calculate similarity based on common elements
    size_t matches = 0;
    size_t maxLength = std::max(sequence1.size(), sequence2.size());
    size_t minLength = std::min(sequence1.size(), sequence2.size());
    
    for (size_t i = 0; i < minLength; i++) {
        if (i < sequence1.size() && i < sequence2.size() && sequence1[i] == sequence2[i]) {
            matches++;
        }
    }
    
    return static_cast<float>(matches) / maxLength;
}

float PatternCache::calculateEnvironmentalMatch(const EnhancedBehaviorPattern& pattern,
                                              const EnvironmentalData& environment) const {
    
    if (!pattern.environmental.weatherDependent) {
        return 1.0f; // No environmental dependency
    }
    
    float score = 1.0f;
    
    // Check temperature range
    if (environment.temperature < pattern.environmental.temperatureRange[0] ||
        environment.temperature > pattern.environmental.temperatureRange[1]) {
        score *= 0.5f;
    }
    
    // Check humidity range
    if (environment.humidity < pattern.environmental.humidityRange[0] ||
        environment.humidity > pattern.environmental.humidityRange[1]) {
        score *= 0.7f;
    }
    
    return score;
}

float PatternCache::calculateTemporalMatch(const EnhancedBehaviorPattern& pattern,
                                         uint8_t currentHour) const {
    
    if (currentHour >= 24 || currentHour >= pattern.temporal.hourlyProbability.size()) {
        return 0.5f; // Default match
    }
    
    return pattern.temporal.hourlyProbability[currentHour];
}

void PatternCache::updateRelevanceScores() {
    for (auto& entry : cachedPatterns_) {
        entry.relevanceScore = calculateRelevanceScore(entry);
    }
}

float PatternCache::calculateRelevanceScore(const PatternCacheEntry& entry) const {
    float score = 0.0f;
    
    // Base confidence score
    score += entry.pattern.confidence * 0.4f;
    
    // Access frequency score
    float accessFrequency = static_cast<float>(entry.accessCount);
    score += std::min(accessFrequency / 10.0f, 0.3f);
    
    // Recency score
    unsigned long timeSinceAccess = millis() - entry.lastAccessed;
    float recencyScore = 1.0f / (1.0f + timeSinceAccess / 3600000.0f); // Decay over hours
    score += recencyScore * 0.3f;
    
    return std::min(score, 1.0f);
}

String PatternCache::generateSequenceHash(const std::vector<BehaviorType>& sequence) const {
    String hash = "";
    for (const auto& behavior : sequence) {
        hash += String(static_cast<int>(behavior)) + "_";
    }
    return hash;
}

uint8_t PatternCache::getConfidenceBin(float confidence) const {
    return static_cast<uint8_t>(confidence * 10.0f); // 0-10 bins
}

size_t PatternCache::findPatternIndex(const String& patternId) const {
    auto it = patternIdIndex_.find(patternId);
    return (it != patternIdIndex_.end()) ? it->second : SIZE_MAX;
}

bool PatternCache::hasMemorySpace(size_t requiredBytes) const {
    // Simplified memory check
    return cachedPatterns_.size() < config_.maxPatterns;
}

void PatternCache::updateMetrics(bool hit, uint32_t processingTime) {
    if (hit) {
        metrics_.cacheHits++;
    } else {
        metrics_.cacheMisses++;
    }
    
    if (metrics_.totalAccesses > 0) {
        metrics_.hitRate = static_cast<float>(metrics_.cacheHits) / metrics_.totalAccesses;
    }
    
    // Update timing metrics
    if (processingTime > 0) {
        if (metrics_.minLookupTime_ms == 0.0f || processingTime < metrics_.minLookupTime_ms) {
            metrics_.minLookupTime_ms = processingTime;
        }
        if (processingTime > metrics_.maxLookupTime_ms) {
            metrics_.maxLookupTime_ms = processingTime;
        }
        
        metrics_.avgLookupTime_ms = (metrics_.avgLookupTime_ms + processingTime) / 2.0f;
    }
    
    // Update other metrics
    metrics_.totalPatterns = cachedPatterns_.size();
    metrics_.memoryUsage_bytes = cachedPatterns_.size() * sizeof(PatternCacheEntry);
    metrics_.memoryEfficiency = metrics_.totalPatterns > 0 ? 
        static_cast<float>(metrics_.cacheHits) / metrics_.totalPatterns : 0.0f;
}