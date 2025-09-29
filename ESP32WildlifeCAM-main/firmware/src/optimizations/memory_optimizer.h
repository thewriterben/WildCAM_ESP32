#ifndef MEMORY_OPTIMIZER_H
#define MEMORY_OPTIMIZER_H

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <vector>

/**
 * Memory Optimizer - Critical heap management for ESP32WildlifeCAM
 * 
 * Implements buffer pools, memory alignment, and fragmentation prevention
 * to ensure stable operation during extended wildlife monitoring.
 */
class MemoryOptimizer {
public:
    // Critical heap management
    static void* allocateAligned(size_t size, size_t alignment = 32);
    static void* allocateDMA(size_t size);
    static void defragmentHeap();
    static size_t getFragmentationLevel();
    
    // Smart buffer pools for camera frames and network messages
    class BufferPool {
        uint8_t* pool;
        std::vector<bool> allocated;
        size_t blockSize;
        size_t blockCount;
        size_t usedCount;
        
    public:
        BufferPool(size_t blockSize, size_t count);
        ~BufferPool();
        uint8_t* acquire();
        void release(uint8_t* ptr);
        float utilizationPercent() const;
        size_t getAvailableBlocks() const;
        bool init();
        void cleanup();
    };
    
    // Memory-mapped analytics storage with compression
    class CircularAnalyticsBuffer {
        uint8_t* buffer;
        size_t capacity;
        size_t writePos;
        size_t readPos;
        bool overflow;
        bool compressed;
        
    public:
        CircularAnalyticsBuffer(size_t capacity);
        ~CircularAnalyticsBuffer();
        bool write(const void* data, size_t size);
        bool read(void* data, size_t size);
        void compress(); // LZ4 compression on-the-fly
        size_t getUsedSpace() const;
        bool init();
        void cleanup();
    };
    
    // Initialize memory optimization system
    static bool init();
    static void cleanup();
    static BufferPool* getImagePool();
    static BufferPool* getMessagePool();
    static CircularAnalyticsBuffer* getAnalyticsBuffer();

private:
    static BufferPool* imagePool;
    static BufferPool* messagePool;
    static CircularAnalyticsBuffer* analyticsBuffer;
    static bool initialized;
};

// CRITICAL: Replace std::vector with fixed-size arrays in hot paths
template<typename T, size_t N>
class FixedVector {
    T data[N];
    size_t size_;
    
public:
    FixedVector() : size_(0) {}
    
    void push_back(const T& item) { 
        if(size_ < N) data[size_++] = item; 
    }
    
    T& operator[](size_t i) { 
        return data[i]; 
    }
    
    const T& operator[](size_t i) const { 
        return data[i]; 
    }
    
    size_t size() const { 
        return size_; 
    }
    
    void clear() { 
        size_ = 0; 
    }
    
    bool full() const { 
        return size_ >= N; 
    }
    
    T* begin() { 
        return data; 
    }
    
    T* end() { 
        return data + size_; 
    }
    
    const T* begin() const { 
        return data; 
    }
    
    const T* end() const { 
        return data + size_; 
    }
};

#endif // MEMORY_OPTIMIZER_H