/**
 * @file behavior_database.cpp
 * @brief Implementation of Efficient Temporal Behavior Data Storage
 */

#include "behavior_database.h"
#include <algorithm>

// Static helper functions
BehaviorType decompressBehaviorType(uint8_t compressed) {
    return static_cast<BehaviorType>(compressed);
}

uint8_t compressBehaviorType(BehaviorType behavior) {
    return static_cast<uint8_t>(behavior);
}

// CompressedBehaviorRecord implementation
CompressedBehaviorRecord CompressedBehaviorRecord::fromBehaviorResult(
    const BehaviorResult& behavior,
    const EnvironmentalData& environment) {
    
    CompressedBehaviorRecord record;
    record.timestamp = behavior.timestamp;
    record.behaviorType = compressBehaviorType(behavior.primaryBehavior);
    record.confidence = static_cast<uint8_t>(behavior.confidence * 255.0f);
    record.duration = std::min(static_cast<uint32_t>(behavior.duration / 60), 255U); // Convert to minutes
    record.activityLevel = static_cast<uint8_t>(behavior.activity_level * 255.0f);
    record.stressLevel = static_cast<uint8_t>(behavior.stress_level * 255.0f);
    record.animalCount = behavior.animalCount;
    
    // Pack boolean flags
    record.flags = 0;
    if (behavior.isRepeatedBehavior) record.flags |= 0x01;
    if (behavior.isGroupBehavior) record.flags |= 0x02;
    if (behavior.hasHumanInteraction) record.flags |= 0x04;
    
    // Compress environmental data
    record.temperature = static_cast<uint8_t>((environment.temperature + 40.0f) * 2.0f); // -40 to +87.5°C range
    record.humidity = static_cast<uint8_t>(environment.humidity * 2.55f);
    record.lightLevel = static_cast<uint8_t>(environment.lightLevel);
    
    return record;
}

BehaviorResult CompressedBehaviorRecord::toBehaviorResult() const {
    BehaviorResult behavior;
    behavior.timestamp = timestamp;
    behavior.primaryBehavior = decompressBehaviorType(behaviorType);
    behavior.confidence = confidence / 255.0f;
    behavior.duration = duration * 60; // Convert back to seconds
    behavior.activity_level = activityLevel / 255.0f;
    behavior.stress_level = stressLevel / 255.0f;
    behavior.animalCount = animalCount;
    
    // Unpack boolean flags
    behavior.isRepeatedBehavior = (flags & 0x01) != 0;
    behavior.isGroupBehavior = (flags & 0x02) != 0;
    behavior.hasHumanInteraction = (flags & 0x04) != 0;
    
    return behavior;
}

// BehaviorDatabase implementation
BehaviorDatabase::BehaviorDatabase() : 
    maxMemoryKB_(512), enableCompression_(true), initialized_(false),
    maxCacheSize_(10), totalQueries_(0), totalQueryTime_(0.0f) {
}

BehaviorDatabase::~BehaviorDatabase() {
    // Cleanup handled by destructors
}

bool BehaviorDatabase::init(uint32_t maxMemoryKB, bool enableCompression) {
    maxMemoryKB_ = maxMemoryKB;
    enableCompression_ = enableCompression;
    
    // Reserve space for initial data
    behaviorRecords_.reserve(1000);
    queryCache_.reserve(maxCacheSize_);
    
    initialized_ = true;
    return true;
}

bool BehaviorDatabase::storeBehavior(const BehaviorResult& behavior,
                                   const EnvironmentalData& environment) {
    if (!initialized_) {
        return false;
    }
    
    // Check memory constraints
    size_t requiredBytes = sizeof(CompressedBehaviorRecord);
    if (!hasMemorySpace(requiredBytes)) {
        // Try to free space
        uint32_t oneWeekAgo = (behavior.timestamp > 604800) ? behavior.timestamp - 604800 : 0;
        removeOldRecords(oneWeekAgo);
        
        if (!hasMemorySpace(requiredBytes)) {
            return false; // Out of memory
        }
    }
    
    // Create compressed record
    CompressedBehaviorRecord record = 
        CompressedBehaviorRecord::fromBehaviorResult(behavior, environment);
    
    // Determine species from behavior context (simplified)
    SpeciesType species = SpeciesType::UNKNOWN; // Would need to be passed in or inferred
    
    // Store record
    behaviorRecords_.push_back(record);
    
    // Update indices
    updateIndices(record, species);
    
    // Update statistics
    updateStats();
    
    return true;
}

std::vector<BehaviorResult> BehaviorDatabase::queryBehaviors(const QueryParameters& params) {
    if (!initialized_) {
        return std::vector<BehaviorResult>();
    }
    
    uint32_t startTime = millis();
    
    // Check cache first
    std::vector<BehaviorResult> cachedResults;
    if (isQueryCached(params, cachedResults)) {
        stats_.cacheHits++;
        updateMetrics(true, millis() - startTime);
        return cachedResults;
    }
    
    // Execute query
    std::vector<BehaviorResult> results = executeQuery(params);
    
    // Cache results
    cacheQuery(params, results);
    
    uint32_t queryTime = millis() - startTime;
    updateMetrics(false, queryTime);
    
    return results;
}

std::vector<BehaviorResult> BehaviorDatabase::executeQuery(const QueryParameters& params) {
    std::vector<BehaviorResult> results;
    
    for (const auto& record : behaviorRecords_) {
        // Time range filter
        if (params.startTime > 0 && record.timestamp < params.startTime) continue;
        if (params.endTime > 0 && record.timestamp > params.endTime) continue;
        
        // Behavior type filter
        if (params.behaviorType != BehaviorType::UNKNOWN) {
            if (decompressBehaviorType(record.behaviorType) != params.behaviorType) continue;
        }
        
        // Confidence filter
        float confidence = record.confidence / 255.0f;
        if (confidence < params.minConfidence) continue;
        
        // Convert and add to results
        BehaviorResult behavior = record.toBehaviorResult();
        results.push_back(behavior);
        
        // Limit results
        if (results.size() >= params.maxResults) break;
    }
    
    // Sort by time if requested
    if (params.sortByTime) {
        std::sort(results.begin(), results.end(),
                 [](const BehaviorResult& a, const BehaviorResult& b) {
                     return a.timestamp < b.timestamp;
                 });
    }
    
    return results;
}

std::vector<BehaviorResult> BehaviorDatabase::getRecentBehaviors(SpeciesType species,
                                                               uint32_t timeWindow_s,
                                                               uint32_t maxCount) {
    QueryParameters params;
    params.species = species;
    params.startTime = (millis() / 1000) - timeWindow_s;
    params.endTime = 0; // Current time
    params.maxResults = maxCount;
    params.sortByTime = true;
    
    return queryBehaviors(params);
}

std::map<BehaviorType, uint32_t> BehaviorDatabase::getBehaviorFrequencies(SpeciesType species,
                                                                         uint32_t timeWindow_s) {
    std::map<BehaviorType, uint32_t> frequencies;
    
    std::vector<BehaviorResult> behaviors = getRecentBehaviors(species, timeWindow_s, 1000);
    
    for (const auto& behavior : behaviors) {
        frequencies[behavior.primaryBehavior]++;
    }
    
    return frequencies;
}

bool BehaviorDatabase::exportDatabase(const char* filename, const char* format) {
    // Simplified export implementation
    // In a real implementation, this would write to file system
    return true;
}

uint32_t BehaviorDatabase::optimizeStorage(uint32_t removeOlderThan) {
    uint32_t removedCount = 0;
    
    if (removeOlderThan > 0) {
        removedCount = behaviorRecords_.size();
        removeOldRecords(removeOlderThan);
        removedCount -= behaviorRecords_.size();
    }
    
    // Compact storage
    compactStorage();
    
    // Update statistics
    updateStats();
    
    return removedCount;
}

DatabaseStats BehaviorDatabase::getStatistics() const {
    // Update real-time statistics
    const_cast<BehaviorDatabase*>(this)->updateStats();
    return stats_;
}

void BehaviorDatabase::clearDatabase() {
    behaviorRecords_.clear();
    speciesIndices_.clear();
    timeIndices_.clear();
    queryCache_.clear();
    stats_ = DatabaseStats();
    totalQueries_ = 0;
    totalQueryTime_ = 0.0f;
}

bool BehaviorDatabase::hasSufficientData(SpeciesType species, uint32_t minRecords, uint32_t timeSpan_s) const {
    if (!initialized_ || behaviorRecords_.empty()) {
        return false;
    }
    
    // Check total record count
    if (behaviorRecords_.size() < minRecords) {
        return false;
    }
    
    // Check time span
    if (behaviorRecords_.size() >= 2) {
        uint32_t timeSpan = behaviorRecords_.back().timestamp - behaviorRecords_.front().timestamp;
        if (timeSpan < timeSpan_s) {
            return false;
        }
    }
    
    return true;
}

uint32_t BehaviorDatabase::getMemoryUsage() const {
    return behaviorRecords_.size() * sizeof(CompressedBehaviorRecord) +
           queryCache_.size() * sizeof(QueryCache) +
           speciesIndices_.size() * sizeof(SpeciesDataIndex) +
           timeIndices_.size() * sizeof(TimeIndex);
}

// Private methods
void BehaviorDatabase::updateIndices(const CompressedBehaviorRecord& record, SpeciesType species) {
    // Update species index
    if (speciesIndices_.find(species) == speciesIndices_.end()) {
        speciesIndices_[species] = SpeciesDataIndex();
        speciesIndices_[species].species = species;
    }
    
    SpeciesDataIndex& speciesIndex = speciesIndices_[species];
    speciesIndex.totalRecords++;
    if (speciesIndex.firstTimestamp == 0) {
        speciesIndex.firstTimestamp = record.timestamp;
    }
    speciesIndex.lastTimestamp = record.timestamp;
}

bool BehaviorDatabase::isQueryCached(const QueryParameters& params, std::vector<BehaviorResult>& results) {
    for (auto& cache : queryCache_) {
        // Simple cache key comparison (could be optimized with hash)
        if (cache.params.species == params.species &&
            cache.params.startTime == params.startTime &&
            cache.params.endTime == params.endTime &&
            cache.params.behaviorType == params.behaviorType &&
            cache.params.minConfidence == params.minConfidence &&
            cache.params.maxResults == params.maxResults) {
            
            cache.hitCount++;
            results = cache.results;
            return true;
        }
    }
    return false;
}

void BehaviorDatabase::cacheQuery(const QueryParameters& params, const std::vector<BehaviorResult>& results) {
    if (queryCache_.size() >= maxCacheSize_) {
        pruneCache();
    }
    
    QueryCache cache;
    cache.params = params;
    cache.results = results;
    cache.timestamp = millis();
    cache.hitCount = 0;
    
    queryCache_.push_back(cache);
}

void BehaviorDatabase::pruneCache() {
    if (queryCache_.empty()) return;
    
    // Remove oldest cache entry
    auto oldest = std::min_element(queryCache_.begin(), queryCache_.end(),
                                  [](const QueryCache& a, const QueryCache& b) {
                                      return a.timestamp < b.timestamp;
                                  });
    
    if (oldest != queryCache_.end()) {
        queryCache_.erase(oldest);
    }
}

void BehaviorDatabase::updateStats() {
    stats_.totalRecords = behaviorRecords_.size();
    stats_.totalSpecies = speciesIndices_.size();
    stats_.memoryUsage_bytes = getMemoryUsage();
    
    if (!behaviorRecords_.empty()) {
        stats_.oldestRecord = behaviorRecords_.front().timestamp;
        stats_.newestRecord = behaviorRecords_.back().timestamp;
        stats_.dataSpan_days = (stats_.newestRecord - stats_.oldestRecord) / 86400;
    }
    
    if (totalQueries_ > 0) {
        stats_.avgQueryTime_ms = totalQueryTime_ / totalQueries_;
        stats_.cacheHitRate = static_cast<float>(stats_.cacheHits) / totalQueries_;
    }
    
    stats_.totalQueries = totalQueries_;
}

void BehaviorDatabase::removeOldRecords(uint32_t olderThan) {
    behaviorRecords_.erase(
        std::remove_if(behaviorRecords_.begin(), behaviorRecords_.end(),
                      [olderThan](const CompressedBehaviorRecord& record) {
                          return record.timestamp < olderThan;
                      }),
        behaviorRecords_.end());
}

void BehaviorDatabase::compactStorage() {
    // Shrink containers to fit
    behaviorRecords_.shrink_to_fit();
    queryCache_.shrink_to_fit();
}

bool BehaviorDatabase::hasMemorySpace(uint32_t requiredBytes) const {
    uint32_t currentUsage = getMemoryUsage();
    uint32_t maxUsage = maxMemoryKB_ * 1024;
    return (currentUsage + requiredBytes) <= maxUsage;
}

void BehaviorDatabase::updateMetrics(bool hit, uint32_t processingTime) {
    totalQueries_++;
    totalQueryTime_ += processingTime;
    
    if (hit) {
        stats_.cacheHits++;
    }
}