/**
 * Intelligent Sync Manager - Adaptive Cloud Synchronization
 * 
 * Provides intelligent synchronization of wildlife monitoring data
 * with adaptive algorithms based on connection quality, data priority,
 * and system constraints.
 * 
 * Features:
 * - Bandwidth-adaptive synchronization
 * - Priority-based data queuing
 * - Offline-first operation with intelligent queuing
 * - Connection quality assessment
 * - Delta synchronization for efficiency
 * - Conflict resolution and data integrity
 */

#ifndef INTELLIGENT_SYNC_MANAGER_H
#define INTELLIGENT_SYNC_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <queue>
#include <map>
#include "../cloud_integrator.h"

// Sync Priority Levels
enum SyncPriority {
    PRIORITY_CRITICAL = 1,    // Emergency/security events
    PRIORITY_HIGH = 2,        // Wildlife detection events
    PRIORITY_MEDIUM = 3,      // Regular environmental data
    PRIORITY_LOW = 4,         // Historical/backup data
    PRIORITY_BACKGROUND = 5   // Non-essential data
};

// Sync Types
enum SyncType {
    SYNC_FULL,               // Complete data synchronization
    SYNC_INCREMENTAL,        // Only changed data
    SYNC_DELTA,              // Delta-based optimization
    SYNC_COMPRESSED,         // Compressed data transfer
    SYNC_METADATA_ONLY,      // Metadata synchronization only
    SYNC_SELECTIVE           // User-defined selective sync
};

// Connection Quality Metrics
struct ConnectionMetrics {
    uint32_t bandwidth;          // Available bandwidth in bytes/sec
    uint32_t latency;            // Network latency in milliseconds
    float packetLoss;            // Packet loss percentage
    float stability;             // Connection stability score (0-1)
    uint32_t signalStrength;     // Signal strength percentage
    bool isMetered;              // Whether connection is metered
    uint32_t lastMeasurement;    // Last measurement timestamp
    
    ConnectionMetrics() : 
        bandwidth(0), latency(0), packetLoss(0.0), stability(0.0),
        signalStrength(0), isMetered(false), lastMeasurement(0) {}
};

// Sync Item
struct SyncItem {
    String itemId;
    String localPath;
    String remotePath;
    SyncPriority priority;
    SyncType syncType;
    uint32_t dataSize;
    uint32_t lastModified;
    uint32_t lastSynced;
    String checksum;
    bool isCompressed;
    bool isEncrypted;
    uint32_t retryCount;
    uint32_t maxRetries;
    String errorMessage;
    uint32_t estimatedSyncTime;
    std::map<String, String> metadata;
    
    SyncItem() : 
        itemId(""), localPath(""), remotePath(""), priority(PRIORITY_MEDIUM),
        syncType(SYNC_INCREMENTAL), dataSize(0), lastModified(0), lastSynced(0),
        checksum(""), isCompressed(false), isEncrypted(false), retryCount(0),
        maxRetries(3), errorMessage(""), estimatedSyncTime(0) {}
};

// Sync Configuration
struct SyncConfig {
    bool enableAdaptiveSync;        // Enable adaptive sync algorithms
    bool enableDeltaSync;           // Enable delta synchronization
    bool enableCompression;         // Enable data compression
    bool enableEncryption;          // Enable data encryption
    uint32_t syncInterval;          // Base sync interval in seconds
    uint32_t maxBatchSize;          // Maximum items per sync batch
    uint32_t maxBandwidthUsage;     // Maximum bandwidth usage percentage
    uint32_t qualityThreshold;      // Minimum quality for sync
    uint32_t retryInterval;         // Retry interval for failed syncs
    bool enableOfflineQueue;        // Enable offline queuing
    uint32_t maxQueueSize;          // Maximum queue size
    bool prioritizeByType;          // Prioritize by data type
    
    SyncConfig() : 
        enableAdaptiveSync(true), enableDeltaSync(true), enableCompression(true),
        enableEncryption(true), syncInterval(300), maxBatchSize(10),
        maxBandwidthUsage(80), qualityThreshold(50), retryInterval(60),
        enableOfflineQueue(true), maxQueueSize(1000), prioritizeByType(true) {}
};

// Sync Statistics
struct SyncStats {
    uint32_t totalItemsSynced;
    uint32_t totalDataSynced;       // Total data in bytes
    uint32_t successfulSyncs;
    uint32_t failedSyncs;
    uint32_t averageSyncTime;       // Average sync time in milliseconds
    uint32_t totalBandwidthUsed;    // Total bandwidth used
    float syncEfficiency;           // Sync efficiency percentage
    uint32_t queuedItems;
    uint32_t conflictsResolved;
    uint32_t lastSyncTime;
    std::map<SyncPriority, uint32_t> priorityStats;
    
    SyncStats() : 
        totalItemsSynced(0), totalDataSynced(0), successfulSyncs(0), failedSyncs(0),
        averageSyncTime(0), totalBandwidthUsed(0), syncEfficiency(0.0),
        queuedItems(0), conflictsResolved(0), lastSyncTime(0) {}
};

// Conflict Resolution Strategy
enum ConflictResolution {
    RESOLVE_LOCAL_WINS,      // Local version takes precedence
    RESOLVE_REMOTE_WINS,     // Remote version takes precedence
    RESOLVE_NEWER_WINS,      // Newer version takes precedence
    RESOLVE_LARGER_WINS,     // Larger file takes precedence
    RESOLVE_MANUAL,          // Manual conflict resolution required
    RESOLVE_MERGE            // Attempt to merge changes
};

// Bandwidth Adaptation Strategy
enum BandwidthStrategy {
    BANDWIDTH_CONSERVATIVE,  // Conservative bandwidth usage
    BANDWIDTH_BALANCED,      // Balanced bandwidth usage
    BANDWIDTH_AGGRESSIVE,    // Aggressive bandwidth usage
    BANDWIDTH_ADAPTIVE       // Dynamically adaptive
};

/**
 * Intelligent Sync Manager Class
 * 
 * Advanced synchronization with adaptive algorithms and optimization
 */
class IntelligentSyncManager {
public:
    IntelligentSyncManager();
    ~IntelligentSyncManager();
    
    // Initialization and configuration
    bool initialize(const SyncConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    bool updateConfiguration(const SyncConfig& config);
    SyncConfig getConfiguration() const { return config_; }
    
    // Connection management and monitoring
    bool assessConnectionQuality();
    ConnectionMetrics getConnectionMetrics() const { return connectionMetrics_; }
    bool isConnectionSuitable(SyncPriority minPriority = PRIORITY_LOW) const;
    void setConnectionQualityThreshold(uint32_t threshold) { config_.qualityThreshold = threshold; }
    
    // Sync queue management
    bool addToSyncQueue(const SyncItem& item);
    bool removeFromSyncQueue(const String& itemId);
    bool prioritizeQueueItem(const String& itemId, SyncPriority newPriority);
    std::vector<SyncItem> getSyncQueue() const;
    void clearSyncQueue();
    uint32_t getQueueSize() const;
    
    // Synchronization operations
    bool performSync();
    bool syncItem(const String& itemId);
    bool syncByPriority(SyncPriority priority);
    bool syncBatch(const std::vector<String>& itemIds);
    bool forceSyncAll();
    
    // Adaptive synchronization
    bool enableAdaptiveMode(bool enable);
    bool isAdaptiveModeEnabled() const { return config_.enableAdaptiveSync; }
    void setBandwidthStrategy(BandwidthStrategy strategy);
    BandwidthStrategy getBandwidthStrategy() const { return bandwidthStrategy_; }
    
    // Delta synchronization
    bool enableDeltaSync(bool enable);
    bool calculateDelta(const String& itemId, String& deltaData);
    bool applyDelta(const String& itemId, const String& deltaData);
    bool generateDeltaManifest(JsonDocument& manifest);
    
    // Compression and optimization
    bool enableCompression(bool enable);
    bool compressData(const String& data, String& compressed);
    bool decompressData(const String& compressed, String& data);
    uint32_t estimateCompressionRatio(const String& data);
    
    // Conflict resolution
    void setConflictResolution(ConflictResolution strategy);
    ConflictResolution getConflictResolution() const { return conflictResolution_; }
    bool resolveConflict(const String& itemId, const SyncItem& localItem, const SyncItem& remoteItem);
    std::vector<String> getPendingConflicts() const;
    
    // Offline operation
    bool enableOfflineMode(bool enable);
    bool isOfflineModeEnabled() const { return offlineMode_; }
    bool queueForOfflineSync(const SyncItem& item);
    bool processOfflineQueue();
    uint32_t getOfflineQueueSize() const;
    
    // Selective synchronization
    bool setSelectiveSync(const std::vector<String>& patterns);
    std::vector<String> getSelectiveSync() const { return selectivePatterns_; }
    bool matchesSelectivePattern(const String& path) const;
    bool addSelectivePattern(const String& pattern);
    bool removeSelectivePattern(const String& pattern);
    
    // Bandwidth management
    bool setBandwidthLimit(uint32_t limitBytesPerSecond);
    uint32_t getBandwidthLimit() const { return bandwidthLimit_; }
    uint32_t getCurrentBandwidthUsage() const;
    bool throttleBandwidth(bool enable);
    
    // Data integrity and validation
    bool validateSyncItem(const String& itemId);
    bool verifyDataIntegrity(const String& itemId);
    String calculateChecksum(const String& data);
    bool compareChecksums(const String& itemId, const String& remoteChecksum);
    
    // Progress tracking and monitoring
    float getSyncProgress() const;
    uint32_t getEstimatedTimeRemaining() const;
    SyncStats getSyncStatistics() const;
    void resetStatistics();
    
    // Sync scheduling
    bool scheduleSync(const String& itemId, uint32_t scheduledTime);
    bool cancelScheduledSync(const String& itemId);
    std::vector<String> getScheduledSyncs() const;
    bool processScheduledSyncs();
    
    // Event handling and callbacks
    typedef void (*SyncProgressCallback)(const String& itemId, float progress);
    typedef void (*SyncCompleteCallback)(const String& itemId, bool success, const String& error);
    typedef void (*ConflictDetectedCallback)(const String& itemId, const SyncItem& local, const SyncItem& remote);
    typedef void (*ConnectionChangeCallback)(const ConnectionMetrics& metrics);
    
    void setSyncProgressCallback(SyncProgressCallback callback) { syncProgressCallback_ = callback; }
    void setSyncCompleteCallback(SyncCompleteCallback callback) { syncCompleteCallback_ = callback; }
    void setConflictDetectedCallback(ConflictDetectedCallback callback) { conflictCallback_ = callback; }
    void setConnectionChangeCallback(ConnectionChangeCallback callback) { connectionCallback_ = callback; }
    
    // Integration with cloud services
    bool connectToCloudProvider(CloudPlatform platform);
    bool syncWithMultipleClouds(const std::vector<CloudPlatform>& platforms);
    bool setCloudSyncPriority(CloudPlatform platform, uint32_t priority);
    
    // Advanced features
    bool enableIncrementalBackup(bool enable);
    bool createSyncSnapshot(const String& snapshotId);
    bool restoreFromSnapshot(const String& snapshotId);
    bool optimizeSyncPaths();
    bool predictSyncNeeds(JsonDocument& predictions);
    
    // Reporting and diagnostics
    void generateSyncReport(String& report);
    void generatePerformanceReport(String& report);
    void generateConflictReport(String& report);
    
    // Emergency operations
    bool pauseSynchronization();
    bool resumeSynchronization();
    bool isPaused() const { return isPaused_; }
    bool emergencySync(SyncPriority minPriority = PRIORITY_CRITICAL);

private:
    // Core state
    bool initialized_;
    bool offlineMode_;
    bool isPaused_;
    SyncConfig config_;
    
    // Connection monitoring
    ConnectionMetrics connectionMetrics_;
    uint32_t lastQualityCheck_;
    BandwidthStrategy bandwidthStrategy_;
    uint32_t bandwidthLimit_;
    
    // Sync queue and management
    std::priority_queue<SyncItem> syncQueue_;
    std::map<String, SyncItem> syncItems_;
    std::map<String, uint32_t> scheduledSyncs_;
    std::queue<SyncItem> offlineQueue_;
    
    // Conflict resolution
    ConflictResolution conflictResolution_;
    std::map<String, std::pair<SyncItem, SyncItem>> pendingConflicts_;
    
    // Selective sync
    std::vector<String> selectivePatterns_;
    
    // Statistics and monitoring
    SyncStats syncStats_;
    uint32_t currentSyncStartTime_;
    std::map<String, float> syncProgress_;
    
    // Event callbacks
    SyncProgressCallback syncProgressCallback_;
    SyncCompleteCallback syncCompleteCallback_;
    ConflictDetectedCallback conflictCallback_;
    ConnectionChangeCallback connectionCallback_;
    
    // Internal helper methods
    bool processSyncQueue();
    bool syncSingleItem(const SyncItem& item);
    bool uploadItem(const SyncItem& item);
    bool downloadItem(const SyncItem& item);
    
    // Connection quality assessment
    uint32_t measureBandwidth();
    uint32_t measureLatency();
    float measurePacketLoss();
    float calculateStabilityScore();
    bool updateConnectionMetrics();
    
    // Adaptive algorithms
    void adaptSyncStrategy();
    void adjustBatchSize();
    void adjustSyncInterval();
    SyncType selectOptimalSyncType(const SyncItem& item);
    
    // Delta synchronization helpers
    bool generateDelta(const String& oldData, const String& newData, String& delta);
    bool applyDeltaPatch(const String& originalData, const String& delta, String& newData);
    bool isDeltaSyncBeneficial(const SyncItem& item);
    
    // Compression helpers
    bool shouldCompressItem(const SyncItem& item);
    uint32_t estimateTransferTime(uint32_t dataSize, bool compressed = false);
    
    // Conflict resolution helpers
    bool detectConflict(const SyncItem& local, const SyncItem& remote);
    bool attemptAutomaticResolution(const String& itemId, const SyncItem& local, const SyncItem& remote);
    SyncItem selectConflictWinner(const SyncItem& local, const SyncItem& remote);
    
    // Queue management helpers
    void sortQueueByPriority();
    void optimizeQueueOrder();
    bool shouldSkipItem(const SyncItem& item);
    void cleanupExpiredItems();
    
    // Bandwidth management helpers
    bool isWithinBandwidthLimit(uint32_t additionalUsage);
    void throttleTransfer(uint32_t bytesToTransfer);
    void updateBandwidthUsage(uint32_t bytesTransferred);
    
    // Statistics helpers
    void updateSyncStatistics(const SyncItem& item, bool success, uint32_t syncTime);
    void calculateEfficiencyMetrics();
    void trackPriorityDistribution();
    
    // Notification helpers
    void notifySyncProgress(const String& itemId, float progress);
    void notifySyncComplete(const String& itemId, bool success, const String& error = "");
    void notifyConflictDetected(const String& itemId, const SyncItem& local, const SyncItem& remote);
    void notifyConnectionChange();
    
    // Cloud integration helpers
    bool initializeCloudConnection();
    bool testCloudConnectivity();
    bool syncToCloud(const SyncItem& item);
    bool syncFromCloud(const SyncItem& item);
    
    // Utility methods
    String generateItemId() const;
    String getSyncTypeName(SyncType type) const;
    String getPriorityName(SyncPriority priority) const;
    uint32_t getCurrentTimestamp() const;
    bool validateSyncConfiguration() const;
};

// Global intelligent sync manager instance
extern IntelligentSyncManager* g_intelligentSyncManager;

// Utility functions for easy integration
bool initializeIntelligentSync(const SyncConfig& config);
bool addToIntelligentSync(const String& localPath, const String& remotePath, SyncPriority priority = PRIORITY_MEDIUM);
bool triggerIntelligentSync();
bool isIntelligentSyncHealthy();
void cleanupIntelligentSync();

#endif // INTELLIGENT_SYNC_MANAGER_H