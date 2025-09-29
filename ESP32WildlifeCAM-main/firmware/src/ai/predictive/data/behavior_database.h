/**
 * @file behavior_database.h
 * @brief Efficient Temporal Behavior Data Storage System
 * 
 * Implements an efficient storage system for temporal behavior data
 * optimized for ESP32 memory constraints and fast retrieval.
 */

#ifndef BEHAVIOR_DATABASE_H
#define BEHAVIOR_DATABASE_H

#include "../../ai_common.h"
#include "../../behavior_analysis.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Compressed Behavior Record for Storage Efficiency
 */
struct CompressedBehaviorRecord {
    uint32_t timestamp;                 // Unix timestamp
    uint8_t behaviorType;              // Compressed behavior type
    uint8_t confidence;                // Confidence (0-255, scaled from 0.0-1.0)
    uint8_t duration;                  // Duration in minutes (0-255)
    uint8_t activityLevel;             // Activity level (0-255)
    uint8_t stressLevel;               // Stress level (0-255)
    uint8_t animalCount;               // Number of animals
    uint8_t flags;                     // Packed boolean flags
    
    // Environmental context (compressed)
    uint8_t temperature;               // Temperature offset from baseline
    uint8_t humidity;                  // Humidity percentage
    uint8_t lightLevel;                // Light level (0-255)
    
    CompressedBehaviorRecord() : 
        timestamp(0), behaviorType(0), confidence(0), duration(0),
        activityLevel(0), stressLevel(0), animalCount(1), flags(0),
        temperature(128), humidity(128), lightLevel(128) {}
    
    // Conversion methods
    static CompressedBehaviorRecord fromBehaviorResult(const BehaviorResult& behavior,
                                                      const EnvironmentalData& environment);
    BehaviorResult toBehaviorResult() const;
};

/**
 * Time-based Data Index for Fast Retrieval
 */
struct TimeIndex {
    uint32_t startTime;
    uint32_t endTime;
    uint32_t recordCount;
    uint32_t dataOffset;               // Offset in storage
    
    TimeIndex() : startTime(0), endTime(0), recordCount(0), dataOffset(0) {}
};

/**
 * Species-based Data Organization
 */
struct SpeciesDataIndex {
    SpeciesType species;
    std::vector<TimeIndex> timeIndices;
    uint32_t totalRecords;
    uint32_t firstTimestamp;
    uint32_t lastTimestamp;
    
    SpeciesDataIndex() : 
        species(SpeciesType::UNKNOWN), totalRecords(0),
        firstTimestamp(0), lastTimestamp(0) {}
};

/**
 * Database Query Parameters
 */
struct QueryParameters {
    SpeciesType species;               // Target species (UNKNOWN = all)
    uint32_t startTime;                // Query start time (0 = earliest)
    uint32_t endTime;                  // Query end time (0 = latest)
    BehaviorType behaviorType;         // Target behavior (UNKNOWN = all)
    float minConfidence;               // Minimum confidence threshold
    uint32_t maxResults;               // Maximum number of results
    bool sortByTime;                   // Sort results by timestamp
    
    QueryParameters() : 
        species(SpeciesType::UNKNOWN), startTime(0), endTime(0),
        behaviorType(BehaviorType::UNKNOWN), minConfidence(0.0f),
        maxResults(1000), sortByTime(true) {}
};

/**
 * Database Statistics
 */
struct DatabaseStats {
    uint32_t totalRecords;
    uint32_t totalSpecies;
    uint32_t uniqueBehaviors;
    uint32_t memoryUsage_bytes;
    uint32_t compressionRatio;         // Percentage of original size
    
    // Time range
    uint32_t oldestRecord;
    uint32_t newestRecord;
    uint32_t dataSpan_days;
    
    // Performance metrics
    float avgQueryTime_ms;
    uint32_t totalQueries;
    uint32_t cacheHits;
    float cacheHitRate;
    
    DatabaseStats() : 
        totalRecords(0), totalSpecies(0), uniqueBehaviors(0),
        memoryUsage_bytes(0), compressionRatio(0), oldestRecord(0),
        newestRecord(0), dataSpan_days(0), avgQueryTime_ms(0.0f),
        totalQueries(0), cacheHits(0), cacheHitRate(0.0f) {}
};

/**
 * Behavior Database Storage System
 * 
 * Efficient storage and retrieval system for temporal behavior data
 * optimized for ESP32 memory constraints and predictive analytics needs.
 */
class BehaviorDatabase {
public:
    /**
     * Constructor
     */
    BehaviorDatabase();
    
    /**
     * Destructor
     */
    ~BehaviorDatabase();
    
    /**
     * Initialize behavior database
     * @param maxMemoryKB Maximum memory usage in KB
     * @param enableCompression Enable data compression
     * @return true if initialization successful
     */
    bool init(uint32_t maxMemoryKB = 512, bool enableCompression = true);
    
    /**
     * Store behavior observation
     * @param behavior Behavior observation
     * @param environment Environmental context
     * @return true if storage successful
     */
    bool storeBehavior(const BehaviorResult& behavior,
                      const EnvironmentalData& environment);
    
    /**
     * Query behavior data
     * @param params Query parameters
     * @return Vector of matching behavior records
     */
    std::vector<BehaviorResult> queryBehaviors(const QueryParameters& params);
    
    /**
     * Get behavior sequence for time window
     * @param species Target species
     * @param startTime Window start time
     * @param endTime Window end time
     * @return Chronological sequence of behaviors
     */
    std::vector<BehaviorResult> getBehaviorSequence(SpeciesType species,
                                                   uint32_t startTime,
                                                   uint32_t endTime);
    
    /**
     * Get recent behaviors
     * @param species Target species
     * @param timeWindow_s Time window in seconds
     * @param maxCount Maximum number of records
     * @return Recent behavior records
     */
    std::vector<BehaviorResult> getRecentBehaviors(SpeciesType species = SpeciesType::UNKNOWN,
                                                  uint32_t timeWindow_s = 3600,
                                                  uint32_t maxCount = 100);
    
    /**
     * Get behavior patterns by frequency
     * @param species Target species
     * @param timeWindow_s Analysis time window
     * @return Map of behavior types to frequency counts
     */
    std::map<BehaviorType, uint32_t> getBehaviorFrequencies(SpeciesType species,
                                                           uint32_t timeWindow_s = 86400);
    
    /**
     * Get temporal behavior distribution
     * @param species Target species
     * @param behaviorType Target behavior
     * @param granularity Time granularity (0=hourly, 1=daily, 2=monthly)
     * @return Time-based behavior distribution
     */
    std::vector<std::pair<uint32_t, uint32_t>> getTemporalDistribution(
        SpeciesType species,
        BehaviorType behaviorType,
        uint8_t granularity = 0);
    
    /**
     * Calculate behavior transition probabilities
     * @param species Target species
     * @param timeWindow_s Analysis time window
     * @return Transition probability matrix
     */
    std::map<std::pair<BehaviorType, BehaviorType>, float> getTransitionProbabilities(
        SpeciesType species,
        uint32_t timeWindow_s = 86400);
    
    /**
     * Export database to file
     * @param filename Output filename
     * @param format Export format ("json", "csv", "binary")
     * @return true if export successful
     */
    bool exportDatabase(const char* filename, const char* format = "json");
    
    /**
     * Import database from file
     * @param filename Input filename
     * @param append Append to existing data
     * @return true if import successful
     */
    bool importDatabase(const char* filename, bool append = false);
    
    /**
     * Optimize database storage
     * @param removeOlderThan Remove records older than timestamp (0 = no removal)
     * @return Number of records removed
     */
    uint32_t optimizeStorage(uint32_t removeOlderThan = 0);
    
    /**
     * Get database statistics
     * @return Current database statistics
     */
    DatabaseStats getStatistics() const;
    
    /**
     * Clear all data
     */
    void clearDatabase();
    
    /**
     * Check if database has sufficient data for analysis
     * @param species Target species
     * @param minRecords Minimum required records
     * @param timeSpan_s Minimum time span in seconds
     * @return true if sufficient data available
     */
    bool hasSufficientData(SpeciesType species = SpeciesType::UNKNOWN,
                          uint32_t minRecords = 10,
                          uint32_t timeSpan_s = 3600) const;
    
    /**
     * Get memory usage information
     * @return Current memory usage in bytes
     */
    uint32_t getMemoryUsage() const;
    
    /**
     * Defragment database storage
     * @return Amount of memory freed in bytes
     */
    uint32_t defragmentDatabase();

private:
    // Configuration
    uint32_t maxMemoryKB_;
    bool enableCompression_;
    bool initialized_;
    
    // Data storage
    std::vector<CompressedBehaviorRecord> behaviorRecords_;
    std::map<SpeciesType, SpeciesDataIndex> speciesIndices_;
    std::vector<TimeIndex> timeIndices_;
    
    // Query cache
    struct QueryCache {
        QueryParameters params;
        std::vector<BehaviorResult> results;
        uint32_t timestamp;
        uint32_t hitCount;
        
        QueryCache() : timestamp(0), hitCount(0) {}
    };
    std::vector<QueryCache> queryCache_;
    uint32_t maxCacheSize_;
    
    // Performance tracking
    mutable DatabaseStats stats_;
    uint32_t totalQueries_;
    float totalQueryTime_;
    
    // Internal methods
    void updateIndices(const CompressedBehaviorRecord& record, SpeciesType species);
    std::vector<BehaviorResult> executeQuery(const QueryParameters& params);
    bool isQueryCached(const QueryParameters& params, std::vector<BehaviorResult>& results);
    void cacheQuery(const QueryParameters& params, const std::vector<BehaviorResult>& results);
    void pruneCache();
    void updateStats();
    
    // Compression methods
    uint8_t compressFloat(float value, float min_val = 0.0f, float max_val = 1.0f) const;
    float decompressFloat(uint8_t compressed, float min_val = 0.0f, float max_val = 1.0f) const;
    uint8_t compressTemperature(float temperature) const;
    float decompressTemperature(uint8_t compressed) const;
    
    // Storage optimization
    void removeOldRecords(uint32_t olderThan);
    void compactStorage();
    bool hasMemorySpace(uint32_t requiredBytes) const;
};

#endif // BEHAVIOR_DATABASE_H