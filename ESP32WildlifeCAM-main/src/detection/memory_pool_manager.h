/**
 * @file memory_pool_manager.h
 * @brief Memory Pool Manager for Frame Buffer Management
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#ifndef MEMORY_POOL_MANAGER_H
#define MEMORY_POOL_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include <esp_heap_caps.h>

/**
 * @brief Memory Pool Manager for efficient frame buffer allocation
 * 
 * Manages memory pools for camera frame buffers, motion detection buffers,
 * and analysis data to optimize memory usage and reduce fragmentation.
 */
class MemoryPoolManager {
public:
    /**
     * @brief Memory pool types
     */
    enum class PoolType {
        FRAME_BUFFER,       // Camera frame buffers (large, infrequent)
        ANALYSIS_BUFFER,    // Motion analysis buffers (medium, frequent)
        TEMP_BUFFER,        // Temporary processing buffers (small, very frequent)
        CONFIG_BUFFER       // Configuration and metadata (small, rare)
    };

    /**
     * @brief Buffer allocation result
     */
    struct BufferAllocation {
        void* buffer = nullptr;
        size_t size = 0;
        PoolType poolType;
        uint32_t allocationId = 0;
        uint32_t timestamp = 0;
        bool isValid() const { return buffer != nullptr; }
    };

    /**
     * @brief Memory pool configuration
     */
    struct PoolConfig {
        size_t blockSize;           // Size of each block in the pool
        uint32_t blockCount;        // Number of blocks in the pool
        bool usePSRAM;              // Use PSRAM if available
        uint32_t alignment;         // Memory alignment requirement
        String name;                // Pool name for debugging
    };

    /**
     * @brief Memory pool statistics
     */
    struct PoolStats {
        uint32_t totalBlocks = 0;
        uint32_t usedBlocks = 0;
        uint32_t peakUsage = 0;
        uint32_t allocations = 0;
        uint32_t deallocations = 0;
        uint32_t failures = 0;
        size_t totalMemory = 0;
        size_t usedMemory = 0;
        float fragmentationRatio = 0.0f;
        uint32_t lastResetTime = 0;
    };

    /**
     * Constructor
     */
    MemoryPoolManager();

    /**
     * Destructor
     */
    ~MemoryPoolManager();

    /**
     * @brief Initialize memory pool manager
     * @param enablePSRAM Use PSRAM for large allocations if available
     * @return true if initialization successful
     */
    bool initialize(bool enablePSRAM = true);

    /**
     * @brief Create a memory pool
     * @param poolType Type of pool to create
     * @param config Pool configuration
     * @return true if pool created successfully
     */
    bool createPool(PoolType poolType, const PoolConfig& config);

    /**
     * @brief Allocate buffer from pool
     * @param poolType Pool to allocate from
     * @param size Required buffer size (must be <= pool block size)
     * @return Buffer allocation result
     */
    BufferAllocation allocate(PoolType poolType, size_t size = 0);

    /**
     * @brief Deallocate buffer back to pool
     * @param allocation Buffer allocation to return
     * @return true if deallocation successful
     */
    bool deallocate(const BufferAllocation& allocation);

    /**
     * @brief Get memory pool statistics
     * @param poolType Pool to get statistics for
     * @return Pool statistics
     */
    PoolStats getPoolStats(PoolType poolType) const;

    /**
     * @brief Get overall memory statistics
     * @return Combined statistics for all pools
     */
    PoolStats getOverallStats() const;

    /**
     * @brief Reset pool statistics
     * @param poolType Pool to reset, or all pools if not specified
     */
    void resetStats(PoolType poolType = PoolType::FRAME_BUFFER);

    /**
     * @brief Defragment memory pools
     * @return Number of blocks moved during defragmentation
     */
    uint32_t defragment();

    /**
     * @brief Check memory pool health
     * @return true if all pools are healthy
     */
    bool checkHealth() const;

    /**
     * @brief Get memory usage report
     * @return JSON string with detailed memory usage
     */
    String getMemoryReport() const;

    /**
     * @brief Configure automatic garbage collection
     * @param enabled Enable automatic cleanup
     * @param thresholdPercent Trigger cleanup when usage exceeds this percentage
     * @param intervalMs Interval between cleanup checks
     */
    void configureAutoCleanup(bool enabled, float thresholdPercent = 80.0f, uint32_t intervalMs = 30000);

    /**
     * @brief Force garbage collection
     * @return Number of buffers cleaned up
     */
    uint32_t garbageCollect();

    /**
     * @brief Get recommended pool configurations for ESP32 wildlife camera
     * @return Vector of recommended pool configurations
     */
    static std::vector<std::pair<PoolType, PoolConfig>> getRecommendedPools();

private:
    /**
     * @brief Individual memory pool implementation
     */
    class MemoryPool {
    public:
        MemoryPool(const PoolConfig& config);
        ~MemoryPool();
        
        bool initialize();
        BufferAllocation allocate(size_t size);
        bool deallocate(uint32_t allocationId);
        PoolStats getStats() const;
        void resetStats();
        bool isHealthy() const;
        uint32_t cleanup();
        
    private:
        PoolConfig config_;
        uint8_t* poolMemory_;
        std::vector<bool> blockAllocated_;
        std::vector<uint32_t> allocationIds_;
        std::vector<uint32_t> allocationTimes_;
        PoolStats stats_;
        uint32_t nextAllocationId_;
        bool initialized_;
        
        uint32_t findFreeBlock();
        void updateStats();
    };

    // Pool instances
    std::map<PoolType, std::unique_ptr<MemoryPool>> pools_;
    
    // Configuration
    bool initialized_;
    bool psramEnabled_;
    
    // Auto cleanup configuration
    bool autoCleanupEnabled_;
    float cleanupThreshold_;
    uint32_t cleanupInterval_;
    uint32_t lastCleanupTime_;
    
    // Helper methods
    String getPoolTypeName(PoolType poolType) const;
    size_t getTotalSystemMemory() const;
    size_t getAvailableSystemMemory() const;
    void performAutoCleanup();
};

#endif // MEMORY_POOL_MANAGER_H