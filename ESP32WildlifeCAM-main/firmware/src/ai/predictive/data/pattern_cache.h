/**
 * @file pattern_cache.h
 * @brief Fast Pattern Lookup and Caching System
 * 
 * Implements high-performance caching for behavior patterns to enable
 * real-time pattern matching and prediction with minimal latency.
 */

#ifndef PATTERN_CACHE_H
#define PATTERN_CACHE_H

#include "../../ai_common.h"
#include "../behavior_pattern_detector.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Pattern Cache Entry
 */
struct PatternCacheEntry {
    String patternId;
    EnhancedBehaviorPattern pattern;
    
    // Cache metadata
    uint32_t accessCount;
    uint32_t lastAccessed;
    uint32_t cacheTimestamp;
    float relevanceScore;              // How relevant this pattern is
    
    // Performance data
    uint32_t matchCount;               // Number of successful matches
    uint32_t missCount;                // Number of failed matches
    float avgMatchTime_ms;             // Average matching time
    
    PatternCacheEntry() : 
        accessCount(0), lastAccessed(0), cacheTimestamp(0),
        relevanceScore(0.0f), matchCount(0), missCount(0),
        avgMatchTime_ms(0.0f) {}
};

/**
 * Pattern Lookup Index for Fast Retrieval
 */
struct PatternIndex {
    // Behavior-based index
    std::map<BehaviorType, std::vector<size_t>> behaviorIndex;
    
    // Sequence-based index
    std::map<String, std::vector<size_t>> sequenceIndex;
    
    // Temporal-based index
    std::map<uint8_t, std::vector<size_t>> hourIndex;     // By hour of day
    std::map<uint8_t, std::vector<size_t>> monthIndex;    // By month
    
    // Species-based index
    std::map<SpeciesType, std::vector<size_t>> speciesIndex;
    
    // Confidence-based index
    std::map<uint8_t, std::vector<size_t>> confidenceIndex; // Binned confidence
};

/**
 * Cache Performance Metrics
 */
struct CacheMetrics {
    uint32_t totalAccesses;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    float hitRate;
    
    float avgLookupTime_ms;
    float maxLookupTime_ms;
    float minLookupTime_ms;
    
    uint32_t totalPatterns;
    uint32_t activePatterns;           // Patterns accessed recently
    uint32_t memoryUsage_bytes;
    float memoryEfficiency;            // Memory usage vs storage efficiency
    
    CacheMetrics() : 
        totalAccesses(0), cacheHits(0), cacheMisses(0), hitRate(0.0f),
        avgLookupTime_ms(0.0f), maxLookupTime_ms(0.0f), minLookupTime_ms(0.0f),
        totalPatterns(0), activePatterns(0), memoryUsage_bytes(0),
        memoryEfficiency(0.0f) {}
};

/**
 * Cache Configuration
 */
struct CacheConfig {
    uint32_t maxPatterns;              // Maximum patterns to cache
    uint32_t maxMemoryKB;              // Maximum memory usage
    uint32_t timeToLive_s;             // Pattern TTL in seconds
    float relevanceThreshold;          // Minimum relevance to keep in cache
    
    bool enableLRU;                    // Enable LRU eviction
    bool enableRelevanceScoring;       // Enable relevance-based eviction
    bool enablePredictiveLoading;      // Preload related patterns
    
    CacheConfig() : 
        maxPatterns(50), maxMemoryKB(128), timeToLive_s(3600),
        relevanceThreshold(0.1f), enableLRU(true),
        enableRelevanceScoring(true), enablePredictiveLoading(true) {}
};

/**
 * Pattern Matching Request
 */
struct PatternMatchRequest {
    std::vector<BehaviorType> sequence;
    EnvironmentalData environment;
    SpeciesType species;
    float minConfidence;
    uint32_t maxMatches;
    bool exactMatch;                   // Require exact sequence match
    
    PatternMatchRequest() : 
        species(SpeciesType::UNKNOWN), minConfidence(0.7f),
        maxMatches(10), exactMatch(false) {}
};

/**
 * Pattern Match Result
 */
struct PatternMatchResult {
    std::vector<EnhancedBehaviorPattern> matchedPatterns;
    std::vector<float> matchScores;    // Confidence scores for each match
    std::vector<String> matchReasons;  // Why each pattern matched
    
    uint32_t totalCandidates;          // Total patterns considered
    uint32_t processingTime_ms;        // Time to complete matching
    bool fromCache;                    // Whether result was cached
    
    PatternMatchResult() : 
        totalCandidates(0), processingTime_ms(0), fromCache(false) {}
};

/**
 * Fast Pattern Lookup and Caching System
 * 
 * High-performance caching system for behavior patterns optimized
 * for real-time pattern matching and prediction operations.
 */
class PatternCache {
public:
    /**
     * Constructor
     */
    PatternCache();
    
    /**
     * Destructor
     */
    ~PatternCache();
    
    /**
     * Initialize pattern cache
     * @param config Cache configuration
     * @return true if initialization successful
     */
    bool init(const CacheConfig& config = CacheConfig());
    
    /**
     * Add pattern to cache
     * @param pattern Pattern to cache
     * @return true if pattern added successfully
     */
    bool addPattern(const EnhancedBehaviorPattern& pattern);
    
    /**
     * Remove pattern from cache
     * @param patternId Pattern identifier
     * @return true if pattern removed
     */
    bool removePattern(const String& patternId);
    
    /**
     * Update existing pattern in cache
     * @param pattern Updated pattern
     * @return true if pattern updated
     */
    bool updatePattern(const EnhancedBehaviorPattern& pattern);
    
    /**
     * Find matching patterns
     * @param request Pattern matching request
     * @return Pattern match results
     */
    PatternMatchResult findMatches(const PatternMatchRequest& request);
    
    /**
     * Get pattern by ID
     * @param patternId Pattern identifier
     * @return Pointer to pattern or nullptr if not found
     */
    const EnhancedBehaviorPattern* getPattern(const String& patternId);
    
    /**
     * Get patterns by behavior type
     * @param behaviorType Target behavior type
     * @param minConfidence Minimum confidence threshold
     * @return Vector of matching patterns
     */
    std::vector<EnhancedBehaviorPattern> getPatternsByBehavior(
        BehaviorType behaviorType,
        float minConfidence = 0.7f);
    
    /**
     * Get patterns by sequence
     * @param sequence Behavior sequence to match
     * @param exactMatch Require exact sequence match
     * @return Vector of matching patterns
     */
    std::vector<EnhancedBehaviorPattern> getPatternsBySequence(
        const std::vector<BehaviorType>& sequence,
        bool exactMatch = false);
    
    /**
     * Get patterns relevant to current context
     * @param currentBehavior Current behavior
     * @param environment Environmental context
     * @param timeContext Time of day (hour)
     * @return Vector of relevant patterns
     */
    std::vector<EnhancedBehaviorPattern> getContextualPatterns(
        BehaviorType currentBehavior,
        const EnvironmentalData& environment,
        uint8_t timeContext);
    
    /**
     * Predict next pattern based on current context
     * @param recentBehaviors Recent behavior sequence
     * @param environment Current environment
     * @return Predicted pattern and confidence
     */
    std::pair<EnhancedBehaviorPattern, float> predictNextPattern(
        const std::vector<BehaviorType>& recentBehaviors,
        const EnvironmentalData& environment);
    
    /**
     * Preload related patterns for predictive caching
     * @param currentPattern Current pattern being accessed
     */
    void preloadRelatedPatterns(const EnhancedBehaviorPattern& currentPattern);
    
    /**
     * Optimize cache performance
     * @return Number of patterns removed during optimization
     */
    uint32_t optimizeCache();
    
    /**
     * Get cache performance metrics
     * @return Current cache metrics
     */
    CacheMetrics getMetrics() const;
    
    /**
     * Clear all cached patterns
     */
    void clearCache();
    
    /**
     * Export cache contents
     * @param filename Output filename
     * @param includeMetrics Include performance metrics
     * @return true if export successful
     */
    bool exportCache(const char* filename, bool includeMetrics = true);
    
    /**
     * Import cache contents
     * @param filename Input filename
     * @param clearExisting Clear existing cache before import
     * @return true if import successful
     */
    bool importCache(const char* filename, bool clearExisting = false);
    
    /**
     * Validate cache integrity
     * @return true if cache is consistent
     */
    bool validateCache() const;
    
    /**
     * Configure cache parameters
     * @param config New cache configuration
     */
    void configure(const CacheConfig& config);
    
    /**
     * Get cache utilization statistics
     * @return Current cache utilization percentage
     */
    float getCacheUtilization() const;

private:
    // Configuration
    CacheConfig config_;
    bool initialized_;
    
    // Cache storage
    std::vector<PatternCacheEntry> cachedPatterns_;
    PatternIndex patternIndex_;
    
    // Performance tracking
    mutable CacheMetrics metrics_;
    
    // Cache management
    std::map<String, size_t> patternIdIndex_;  // Pattern ID to index mapping
    std::vector<size_t> accessOrder_;         // LRU tracking
    
    // Internal methods
    void updateIndex(const EnhancedBehaviorPattern& pattern, size_t index);
    void removeFromIndex(const String& patternId, size_t index);
    void updateAccessOrder(size_t index);
    void evictPatterns();
    void evictLRU();
    void evictByRelevance();
    
    // Pattern matching algorithms
    float calculatePatternMatch(const EnhancedBehaviorPattern& pattern,
                              const PatternMatchRequest& request) const;
    float calculateSequenceMatch(const std::vector<BehaviorType>& sequence1,
                               const std::vector<BehaviorType>& sequence2,
                               bool exactMatch = false) const;
    float calculateEnvironmentalMatch(const EnhancedBehaviorPattern& pattern,
                                    const EnvironmentalData& environment) const;
    float calculateTemporalMatch(const EnhancedBehaviorPattern& pattern,
                               uint8_t currentHour) const;
    
    // Relevance scoring
    void updateRelevanceScores();
    float calculateRelevanceScore(const PatternCacheEntry& entry) const;
    
    // Predictive loading
    std::vector<String> identifyRelatedPatterns(const EnhancedBehaviorPattern& pattern) const;
    void loadPredictivePatterns(const std::vector<String>& patternIds);
    
    // Utility methods
    String generateSequenceHash(const std::vector<BehaviorType>& sequence) const;
    uint8_t getConfidenceBin(float confidence) const;
    size_t findPatternIndex(const String& patternId) const;
    bool hasMemorySpace(size_t requiredBytes) const;
    void updateMetrics(bool hit, uint32_t processingTime);
};

#endif // PATTERN_CACHE_H