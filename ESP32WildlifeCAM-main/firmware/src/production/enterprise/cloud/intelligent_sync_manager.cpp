/**
 * Intelligent Sync Manager Implementation
 * 
 * Provides intelligent synchronization of wildlife monitoring data
 * with adaptive algorithms based on connection quality.
 */

#include "intelligent_sync_manager.h"

// Global instance
IntelligentSyncManager* g_intelligentSyncManager = nullptr;

IntelligentSyncManager::IntelligentSyncManager() :
    initialized_(false),
    offlineMode_(false),
    isPaused_(false),
    lastQualityCheck_(0),
    bandwidthStrategy_(BANDWIDTH_ADAPTIVE),
    bandwidthLimit_(0),
    conflictResolution_(CONFLICT_NEWEST_WINS) {
}

IntelligentSyncManager::~IntelligentSyncManager() {
    cleanup();
}

bool IntelligentSyncManager::initialize(const SyncConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    initialized_ = true;
    
    return true;
}

void IntelligentSyncManager::cleanup() {
    while (!syncQueue_.empty()) {
        syncQueue_.pop();
    }
    
    syncItems_.clear();
    scheduledSyncs_.clear();
    pendingConflicts_.clear();
    
    initialized_ = false;
}

bool IntelligentSyncManager::addSyncItem(const SyncItem& item) {
    if (!initialized_) {
        return false;
    }
    
    syncItems_[item.itemId] = item;
    syncQueue_.push(item);
    
    return true;
}

bool IntelligentSyncManager::removeSyncItem(const String& itemId) {
    if (syncItems_.find(itemId) != syncItems_.end()) {
        syncItems_.erase(itemId);
        return true;
    }
    return false;
}

bool IntelligentSyncManager::updateSyncItem(const String& itemId, const SyncItem& item) {
    if (syncItems_.find(itemId) == syncItems_.end()) {
        return false;
    }
    
    syncItems_[itemId] = item;
    return true;
}

SyncItem IntelligentSyncManager::getSyncItem(const String& itemId) const {
    auto it = syncItems_.find(itemId);
    if (it != syncItems_.end()) {
        return it->second;
    }
    return SyncItem();
}

bool IntelligentSyncManager::syncNow() {
    if (!initialized_ || isPaused_) {
        return false;
    }
    
    return processSync();
}

bool IntelligentSyncManager::syncByPriority(SyncPriority minPriority) {
    if (!initialized_ || isPaused_) {
        return false;
    }
    
    processHighPriorityUploads();
    return true;
}

bool IntelligentSyncManager::scheduleSyncAt(const String& itemId, uint32_t timestamp) {
    if (!initialized_) {
        return false;
    }
    
    scheduledSyncs_[itemId] = timestamp;
    return true;
}

bool IntelligentSyncManager::cancelScheduledSync(const String& itemId) {
    if (scheduledSyncs_.find(itemId) != scheduledSyncs_.end()) {
        scheduledSyncs_.erase(itemId);
        return true;
    }
    return false;
}

bool IntelligentSyncManager::setOfflineMode(bool offline) {
    offlineMode_ = offline;
    return true;
}

bool IntelligentSyncManager::isOfflineMode() const {
    return offlineMode_;
}

void IntelligentSyncManager::enableAdaptiveSync(bool enable) {
    config_.enableAdaptiveSync = enable;
}

void IntelligentSyncManager::setBandwidthLimit(uint32_t bytesPerSecond) {
    bandwidthLimit_ = bytesPerSecond;
}

void IntelligentSyncManager::setBandwidthStrategy(BandwidthStrategy strategy) {
    bandwidthStrategy_ = strategy;
}

ConnectionMetrics IntelligentSyncManager::measureConnectionQuality() {
    ConnectionMetrics metrics;
    
    // Measure current connection quality
    metrics.bandwidth = 1000000; // 1 MB/s placeholder
    metrics.latency = 50; // 50ms placeholder
    metrics.packetLoss = 0.01f; // 1% placeholder
    metrics.stability = 0.95f;
    metrics.signalStrength = 85;
    metrics.isMetered = false;
    metrics.lastMeasurement = millis();
    
    connectionMetrics_ = metrics;
    lastQualityCheck_ = millis();
    
    return metrics;
}

ConnectionMetrics IntelligentSyncManager::getConnectionMetrics() const {
    return connectionMetrics_;
}

bool IntelligentSyncManager::shouldSyncNow() const {
    if (isPaused_ || offlineMode_) {
        return false;
    }
    
    if (syncQueue_.empty()) {
        return false;
    }
    
    // Check if connection quality is sufficient
    if (connectionMetrics_.stability < 0.5f) {
        return false;
    }
    
    return true;
}

uint32_t IntelligentSyncManager::getQueueSize() const {
    return syncQueue_.size();
}

std::vector<SyncItem> IntelligentSyncManager::getPendingSyncs() const {
    std::vector<SyncItem> items;
    std::priority_queue<SyncItem> tempQueue = syncQueue_;
    
    while (!tempQueue.empty()) {
        items.push_back(tempQueue.top());
        tempQueue.pop();
    }
    
    return items;
}

bool IntelligentSyncManager::clearSyncQueue() {
    while (!syncQueue_.empty()) {
        syncQueue_.pop();
    }
    return true;
}

void IntelligentSyncManager::setConflictResolution(ConflictResolution resolution) {
    conflictResolution_ = resolution;
}

bool IntelligentSyncManager::hasConflicts() const {
    return !pendingConflicts_.empty();
}

std::vector<SyncItem> IntelligentSyncManager::getConflicts() const {
    std::vector<SyncItem> conflicts;
    
    for (const auto& pair : pendingConflicts_) {
        conflicts.push_back(pair.second.first);
    }
    
    return conflicts;
}

bool IntelligentSyncManager::resolveConflict(const String& itemId, bool keepLocal) {
    auto it = pendingConflicts_.find(itemId);
    if (it == pendingConflicts_.end()) {
        return false;
    }
    
    SyncItem item = keepLocal ? it->second.first : it->second.second;
    syncItems_[itemId] = item;
    pendingConflicts_.erase(itemId);
    
    return true;
}

bool IntelligentSyncManager::enableDeltaSync(bool enable) {
    config_.enableDeltaSync = enable;
    return true;
}

bool IntelligentSyncManager::enableCompression(bool enable) {
    config_.enableCompression = enable;
    return true;
}

bool IntelligentSyncManager::addSelectiveSyncPattern(const String& pattern) {
    selectivePatterns_.push_back(pattern);
    return true;
}

bool IntelligentSyncManager::removeSelectiveSyncPattern(const String& pattern) {
    for (auto it = selectivePatterns_.begin(); it != selectivePatterns_.end(); ++it) {
        if (*it == pattern) {
            selectivePatterns_.erase(it);
            return true;
        }
    }
    return false;
}

std::vector<String> IntelligentSyncManager::getSelectivePatterns() const {
    return selectivePatterns_;
}

bool IntelligentSyncManager::connectToCloudProvider(CloudPlatform platform) {
    return true;
}

bool IntelligentSyncManager::syncWithMultipleClouds(const std::vector<CloudPlatform>& platforms) {
    return true;
}

bool IntelligentSyncManager::setCloudSyncPriority(CloudPlatform platform, uint32_t priority) {
    return true;
}

bool IntelligentSyncManager::enableIncrementalBackup(bool enable) {
    return true;
}

bool IntelligentSyncManager::createSyncSnapshot(const String& snapshotId) {
    return true;
}

bool IntelligentSyncManager::restoreFromSnapshot(const String& snapshotId) {
    return true;
}

bool IntelligentSyncManager::optimizeSyncPaths() {
    return true;
}

bool IntelligentSyncManager::predictSyncNeeds(JsonDocument& predictions) {
    return true;
}

void IntelligentSyncManager::generateSyncReport(String& report) {
    report = "Intelligent Sync Manager Report\n";
    report += "Queue Size: " + String(getQueueSize()) + "\n";
    report += "Offline Mode: " + String(offlineMode_ ? "Yes" : "No") + "\n";
    report += "Paused: " + String(isPaused_ ? "Yes" : "No") + "\n";
}

void IntelligentSyncManager::generatePerformanceReport(String& report) {
    report = "Performance Report\n";
    report += "Bandwidth: " + String(connectionMetrics_.bandwidth) + " bytes/sec\n";
    report += "Latency: " + String(connectionMetrics_.latency) + " ms\n";
}

void IntelligentSyncManager::generateConflictReport(String& report) {
    report = "Conflict Report\n";
    report += "Pending Conflicts: " + String(pendingConflicts_.size()) + "\n";
}

bool IntelligentSyncManager::pauseSynchronization() {
    isPaused_ = true;
    return true;
}

bool IntelligentSyncManager::resumeSynchronization() {
    isPaused_ = false;
    return true;
}

bool IntelligentSyncManager::emergencySync(SyncPriority minPriority) {
    return syncByPriority(minPriority);
}

bool IntelligentSyncManager::processSync() {
    if (syncQueue_.empty()) {
        return true;
    }
    
    // Measure connection quality before sync
    measureConnectionQuality();
    
    // Process items in priority order
    while (!syncQueue_.empty() && shouldSyncNow()) {
        SyncItem item = syncQueue_.top();
        syncQueue_.pop();
        
        if (!shouldSkipItem(item)) {
            // Process sync item
            syncItems_[item.itemId].lastSynced = millis();
        }
    }
    
    return true;
}

void IntelligentSyncManager::processHighPriorityUploads() {
    std::vector<SyncItem> highPriorityItems;
    
    for (const auto& pair : syncItems_) {
        if (pair.second.priority <= PRIORITY_HIGH) {
            highPriorityItems.push_back(pair.second);
        }
    }
    
    for (const SyncItem& item : highPriorityItems) {
        syncQueue_.push(item);
    }
}

bool IntelligentSyncManager::shouldSkipItem(const SyncItem& item) {
    // Skip if recently synced
    if (millis() - item.lastSynced < 60000) { // 1 minute
        return true;
    }
    
    // Skip if retry count exceeded
    if (item.retryCount >= item.maxRetries) {
        return true;
    }
    
    return false;
}
