/**
 * @file memory_pool_manager.cpp
 * @brief Memory Pool Manager Implementation
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include "memory_pool_manager.h"
#include <ArduinoJson.h>
#include <algorithm>

MemoryPoolManager::MemoryPoolManager()
    : initialized_(false)
    , psramEnabled_(false)
    , autoCleanupEnabled_(false)
    , cleanupThreshold_(80.0f)
    , cleanupInterval_(30000)
    , lastCleanupTime_(0) {
}

MemoryPoolManager::~MemoryPoolManager() {
    // Cleanup all pools
    pools_.clear();
}

bool MemoryPoolManager::initialize(bool enablePSRAM) {
    if (initialized_) {
        return true;
    }
    
    psramEnabled_ = enablePSRAM && esp_psram_is_initialized();
    
    // Create recommended memory pools
    auto recommendedPools = getRecommendedPools();
    for (const auto& poolPair : recommendedPools) {
        if (!createPool(poolPair.first, poolPair.second)) {
            Serial.printf("MemoryPoolManager: Failed to create %s pool\n", 
                         getPoolTypeName(poolPair.first).c_str());
            // Continue with other pools
        }
    }
    
    // Configure automatic cleanup
    configureAutoCleanup(true, 75.0f, 30000);
    
    initialized_ = true;
    Serial.println("MemoryPoolManager: Initialized successfully");
    
    return true;
}

bool MemoryPoolManager::createPool(PoolType poolType, const PoolConfig& config) {
    if (pools_.find(poolType) != pools_.end()) {
        Serial.printf("MemoryPoolManager: Pool %s already exists\n", 
                     getPoolTypeName(poolType).c_str());
        return false;
    }
    
    auto pool = std::make_unique<MemoryPool>(config);
    if (!pool->initialize()) {
        Serial.printf("MemoryPoolManager: Failed to initialize %s pool\n", 
                     getPoolTypeName(poolType).c_str());
        return false;
    }
    
    pools_[poolType] = std::move(pool);
    Serial.printf("MemoryPoolManager: Created %s pool (%d blocks of %d bytes)\n",
                 getPoolTypeName(poolType).c_str(), config.blockCount, config.blockSize);
    
    return true;
}

MemoryPoolManager::BufferAllocation MemoryPoolManager::allocate(PoolType poolType, size_t size) {
    auto it = pools_.find(poolType);
    if (it == pools_.end()) {
        Serial.printf("MemoryPoolManager: Pool %s not found\n", getPoolTypeName(poolType).c_str());
        return BufferAllocation{};
    }
    
    auto allocation = it->second->allocate(size);
    allocation.poolType = poolType;
    
    // Trigger auto cleanup if needed
    if (autoCleanupEnabled_) {
        uint32_t now = millis();
        if (now - lastCleanupTime_ > cleanupInterval_) {
            performAutoCleanup();
            lastCleanupTime_ = now;
        }
    }
    
    return allocation;
}

bool MemoryPoolManager::deallocate(const BufferAllocation& allocation) {
    if (!allocation.isValid()) {
        return false;
    }
    
    auto it = pools_.find(allocation.poolType);
    if (it == pools_.end()) {
        return false;
    }
    
    return it->second->deallocate(allocation.allocationId);
}

MemoryPoolManager::PoolStats MemoryPoolManager::getPoolStats(PoolType poolType) const {
    auto it = pools_.find(poolType);
    if (it == pools_.end()) {
        return PoolStats{};
    }
    
    return it->second->getStats();
}

MemoryPoolManager::PoolStats MemoryPoolManager::getOverallStats() const {
    PoolStats overall;
    
    for (const auto& poolPair : pools_) {
        PoolStats poolStats = poolPair.second->getStats();
        overall.totalBlocks += poolStats.totalBlocks;
        overall.usedBlocks += poolStats.usedBlocks;
        overall.peakUsage += poolStats.peakUsage;
        overall.allocations += poolStats.allocations;
        overall.deallocations += poolStats.deallocations;
        overall.failures += poolStats.failures;
        overall.totalMemory += poolStats.totalMemory;
        overall.usedMemory += poolStats.usedMemory;
    }
    
    // Calculate overall fragmentation
    if (overall.totalMemory > 0) {
        overall.fragmentationRatio = (float)overall.usedMemory / overall.totalMemory;
    }
    
    return overall;
}

String MemoryPoolManager::getMemoryReport() const {
    DynamicJsonDocument doc(2048);
    
    doc["systemMemory"]["total"] = getTotalSystemMemory();
    doc["systemMemory"]["available"] = getAvailableSystemMemory();
    doc["systemMemory"]["psramEnabled"] = psramEnabled_;
    
    JsonObject pools = doc.createNestedObject("pools");
    for (const auto& poolPair : pools_) {
        String poolName = getPoolTypeName(poolPair.first);
        JsonObject poolStats = pools.createNestedObject(poolName);
        
        PoolStats stats = poolPair.second->getStats();
        poolStats["totalBlocks"] = stats.totalBlocks;
        poolStats["usedBlocks"] = stats.usedBlocks;
        poolStats["peakUsage"] = stats.peakUsage;
        poolStats["allocations"] = stats.allocations;
        poolStats["deallocations"] = stats.deallocations;
        poolStats["failures"] = stats.failures;
        poolStats["totalMemory"] = stats.totalMemory;
        poolStats["usedMemory"] = stats.usedMemory;
        poolStats["fragmentationRatio"] = stats.fragmentationRatio;
    }
    
    PoolStats overall = getOverallStats();
    JsonObject overallStats = doc.createNestedObject("overall");
    overallStats["totalBlocks"] = overall.totalBlocks;
    overallStats["usedBlocks"] = overall.usedBlocks;
    overallStats["utilizationPercent"] = overall.totalBlocks > 0 ? 
        (float)overall.usedBlocks / overall.totalBlocks * 100.0f : 0.0f;
    overallStats["totalMemory"] = overall.totalMemory;
    overallStats["usedMemory"] = overall.usedMemory;
    
    String output;
    serializeJson(doc, output);
    return output;
}

std::vector<std::pair<MemoryPoolManager::PoolType, MemoryPoolManager::PoolConfig>> 
MemoryPoolManager::getRecommendedPools() {
    std::vector<std::pair<PoolType, PoolConfig>> recommendations;
    
    // Frame buffer pool (large buffers for camera frames)
    PoolConfig frameConfig;
    frameConfig.blockSize = 100 * 1024; // 100KB per frame buffer
    frameConfig.blockCount = 3;         // 3 frame buffers
    frameConfig.usePSRAM = true;        // Use PSRAM for large buffers
    frameConfig.alignment = 4;
    frameConfig.name = "FrameBuffers";
    recommendations.push_back({PoolType::FRAME_BUFFER, frameConfig});
    
    // Analysis buffer pool (medium buffers for motion analysis)
    PoolConfig analysisConfig;
    analysisConfig.blockSize = 32 * 1024; // 32KB per analysis buffer
    analysisConfig.blockCount = 4;        // 4 analysis buffers
    analysisConfig.usePSRAM = false;      // Use internal RAM for faster access
    analysisConfig.alignment = 4;
    analysisConfig.name = "AnalysisBuffers";
    recommendations.push_back({PoolType::ANALYSIS_BUFFER, analysisConfig});
    
    // Temporary buffer pool (small buffers for temporary processing)
    PoolConfig tempConfig;
    tempConfig.blockSize = 4 * 1024;  // 4KB per temp buffer
    tempConfig.blockCount = 8;        // 8 temp buffers
    tempConfig.usePSRAM = false;      // Use internal RAM
    tempConfig.alignment = 4;
    tempConfig.name = "TempBuffers";
    recommendations.push_back({PoolType::TEMP_BUFFER, tempConfig});
    
    // Configuration buffer pool (small buffers for config data)
    PoolConfig configConfig;
    configConfig.blockSize = 2 * 1024; // 2KB per config buffer
    configConfig.blockCount = 4;       // 4 config buffers
    configConfig.usePSRAM = false;     // Use internal RAM
    configConfig.alignment = 4;
    configConfig.name = "ConfigBuffers";
    recommendations.push_back({PoolType::CONFIG_BUFFER, configConfig});
    
    return recommendations;
}

void MemoryPoolManager::configureAutoCleanup(bool enabled, float thresholdPercent, uint32_t intervalMs) {
    autoCleanupEnabled_ = enabled;
    cleanupThreshold_ = thresholdPercent;
    cleanupInterval_ = intervalMs;
    lastCleanupTime_ = millis();
    
    Serial.printf("MemoryPoolManager: Auto cleanup %s (threshold: %.1f%%, interval: %dms)\n",
                 enabled ? "enabled" : "disabled", thresholdPercent, intervalMs);
}

uint32_t MemoryPoolManager::garbageCollect() {
    uint32_t totalCleaned = 0;
    
    for (auto& poolPair : pools_) {
        totalCleaned += poolPair.second->cleanup();
    }
    
    if (totalCleaned > 0) {
        Serial.printf("MemoryPoolManager: Garbage collection freed %d buffers\n", totalCleaned);
    }
    
    return totalCleaned;
}

String MemoryPoolManager::getPoolTypeName(PoolType poolType) const {
    switch (poolType) {
        case PoolType::FRAME_BUFFER: return "FrameBuffer";
        case PoolType::ANALYSIS_BUFFER: return "AnalysisBuffer";
        case PoolType::TEMP_BUFFER: return "TempBuffer";
        case PoolType::CONFIG_BUFFER: return "ConfigBuffer";
        default: return "Unknown";
    }
}

size_t MemoryPoolManager::getTotalSystemMemory() const {
    return ESP.getHeapSize() + (psramEnabled_ ? ESP.getPsramSize() : 0);
}

size_t MemoryPoolManager::getAvailableSystemMemory() const {
    return ESP.getFreeHeap() + (psramEnabled_ ? ESP.getFreePsram() : 0);
}

void MemoryPoolManager::performAutoCleanup() {
    PoolStats overall = getOverallStats();
    
    if (overall.totalBlocks == 0) return;
    
    float utilizationPercent = (float)overall.usedBlocks / overall.totalBlocks * 100.0f;
    
    if (utilizationPercent > cleanupThreshold_) {
        Serial.printf("MemoryPoolManager: Auto cleanup triggered (utilization: %.1f%%)\n", utilizationPercent);
        garbageCollect();
    }
}

// MemoryPool implementation
MemoryPoolManager::MemoryPool::MemoryPool(const PoolConfig& config)
    : config_(config)
    , poolMemory_(nullptr)
    , nextAllocationId_(1)
    , initialized_(false) {
}

MemoryPoolManager::MemoryPool::~MemoryPool() {
    if (poolMemory_) {
        heap_caps_free(poolMemory_);
    }
}

bool MemoryPoolManager::MemoryPool::initialize() {
    if (initialized_) {
        return true;
    }
    
    size_t totalSize = config_.blockSize * config_.blockCount;
    
    // Allocate memory based on configuration
    uint32_t caps = MALLOC_CAP_8BIT;
    if (config_.usePSRAM && esp_psram_is_initialized()) {
        caps |= MALLOC_CAP_SPIRAM;
    } else {
        caps |= MALLOC_CAP_INTERNAL;
    }
    
    poolMemory_ = (uint8_t*)heap_caps_aligned_alloc(config_.alignment, totalSize, caps);
    if (!poolMemory_) {
        Serial.printf("MemoryPool: Failed to allocate %d bytes for %s\n", 
                     totalSize, config_.name.c_str());
        return false;
    }
    
    // Initialize tracking structures
    blockAllocated_.resize(config_.blockCount, false);
    allocationIds_.resize(config_.blockCount, 0);
    allocationTimes_.resize(config_.blockCount, 0);
    
    // Initialize statistics
    stats_.totalBlocks = config_.blockCount;
    stats_.totalMemory = totalSize;
    stats_.lastResetTime = millis();
    
    initialized_ = true;
    return true;
}

MemoryPoolManager::BufferAllocation MemoryPoolManager::MemoryPool::allocate(size_t size) {
    if (!initialized_ || size > config_.blockSize) {
        stats_.failures++;
        return BufferAllocation{};
    }
    
    uint32_t blockIndex = findFreeBlock();
    if (blockIndex >= config_.blockCount) {
        stats_.failures++;
        return BufferAllocation{};
    }
    
    // Mark block as allocated
    blockAllocated_[blockIndex] = true;
    allocationIds_[blockIndex] = nextAllocationId_;
    allocationTimes_[blockIndex] = millis();
    
    // Create allocation result
    BufferAllocation allocation;
    allocation.buffer = poolMemory_ + (blockIndex * config_.blockSize);
    allocation.size = size > 0 ? size : config_.blockSize;
    allocation.allocationId = nextAllocationId_;
    allocation.timestamp = allocationTimes_[blockIndex];
    
    nextAllocationId_++;
    stats_.allocations++;
    updateStats();
    
    return allocation;
}

bool MemoryPoolManager::MemoryPool::deallocate(uint32_t allocationId) {
    for (uint32_t i = 0; i < config_.blockCount; i++) {
        if (blockAllocated_[i] && allocationIds_[i] == allocationId) {
            blockAllocated_[i] = false;
            allocationIds_[i] = 0;
            allocationTimes_[i] = 0;
            stats_.deallocations++;
            updateStats();
            return true;
        }
    }
    
    return false;
}

MemoryPoolManager::PoolStats MemoryPoolManager::MemoryPool::getStats() const {
    return stats_;
}

uint32_t MemoryPoolManager::MemoryPool::findFreeBlock() {
    for (uint32_t i = 0; i < config_.blockCount; i++) {
        if (!blockAllocated_[i]) {
            return i;
        }
    }
    return config_.blockCount; // No free block found
}

void MemoryPoolManager::MemoryPool::updateStats() {
    stats_.usedBlocks = 0;
    for (bool allocated : blockAllocated_) {
        if (allocated) {
            stats_.usedBlocks++;
        }
    }
    
    stats_.peakUsage = std::max(stats_.peakUsage, stats_.usedBlocks);
    stats_.usedMemory = stats_.usedBlocks * config_.blockSize;
    
    if (stats_.totalBlocks > 0) {
        stats_.fragmentationRatio = (float)stats_.usedBlocks / stats_.totalBlocks;
    }
}

uint32_t MemoryPoolManager::MemoryPool::cleanup() {
    uint32_t cleaned = 0;
    uint32_t currentTime = millis();
    uint32_t maxAge = 300000; // 5 minutes
    
    for (uint32_t i = 0; i < config_.blockCount; i++) {
        if (blockAllocated_[i] && (currentTime - allocationTimes_[i]) > maxAge) {
            blockAllocated_[i] = false;
            allocationIds_[i] = 0;
            allocationTimes_[i] = 0;
            cleaned++;
        }
    }
    
    if (cleaned > 0) {
        updateStats();
    }
    
    return cleaned;
}