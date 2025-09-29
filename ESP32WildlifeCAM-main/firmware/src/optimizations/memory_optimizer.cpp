#include "memory_optimizer.h"
#include "../config.h"

// Configuration constants
#define IMAGE_BUFFER_POOL_SIZE 5           // Pre-allocated image buffers
#define MESSAGE_BUFFER_POOL_SIZE 20        // Pre-allocated message buffers
#define ANALYTICS_CIRCULAR_BUFFER_SIZE (64 * 1024) // 64KB circular buffer
#define IMAGE_BUFFER_SIZE (1024 * 1024)    // 1MB per image buffer
#define MESSAGE_BUFFER_SIZE (1024)         // 1KB per message buffer

// Static member definitions
MemoryOptimizer::BufferPool* MemoryOptimizer::imagePool = nullptr;
MemoryOptimizer::BufferPool* MemoryOptimizer::messagePool = nullptr;
MemoryOptimizer::CircularAnalyticsBuffer* MemoryOptimizer::analyticsBuffer = nullptr;
bool MemoryOptimizer::initialized = false;

// MemoryOptimizer static methods
void* MemoryOptimizer::allocateAligned(size_t size, size_t alignment) {
    void* ptr = heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_8BIT);
    if (!ptr && esp_psram_is_initialized()) {
        // Fallback to PSRAM if internal memory fails
        ptr = heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_SPIRAM);
    }
    return ptr;
}

void* MemoryOptimizer::allocateDMA(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_DMA);
}

void MemoryOptimizer::defragmentHeap() {
    // ESP32 doesn't support explicit defragmentation
    // This triggers internal heap management optimizations
    heap_caps_check_integrity_all(true);
}

size_t MemoryOptimizer::getFragmentationLevel() {
    size_t totalFree = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    
    if (totalFree == 0) return 100; // 100% fragmented
    
    // Calculate fragmentation as percentage
    // 0% = no fragmentation, 100% = completely fragmented
    return 100 - ((largestBlock * 100) / totalFree);
}

bool MemoryOptimizer::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing Memory Optimizer...");
    
    // Create image buffer pool
    imagePool = new BufferPool(IMAGE_BUFFER_SIZE, IMAGE_BUFFER_POOL_SIZE);
    if (!imagePool || !imagePool->init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize image buffer pool");
        cleanup();
        return false;
    }
    
    // Create message buffer pool  
    messagePool = new BufferPool(MESSAGE_BUFFER_SIZE, MESSAGE_BUFFER_POOL_SIZE);
    if (!messagePool || !messagePool->init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize message buffer pool");
        cleanup();
        return false;
    }
    
    // Create analytics circular buffer
    analyticsBuffer = new CircularAnalyticsBuffer(ANALYTICS_CIRCULAR_BUFFER_SIZE);
    if (!analyticsBuffer || !analyticsBuffer->init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize analytics buffer");
        cleanup();
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Memory Optimizer initialized successfully");
    return true;
}

void MemoryOptimizer::cleanup() {
    if (imagePool) {
        imagePool->cleanup();
        delete imagePool;
        imagePool = nullptr;
    }
    
    if (messagePool) {
        messagePool->cleanup();
        delete messagePool;
        messagePool = nullptr;
    }
    
    if (analyticsBuffer) {
        analyticsBuffer->cleanup();
        delete analyticsBuffer;
        analyticsBuffer = nullptr;
    }
    
    initialized = false;
}

MemoryOptimizer::BufferPool* MemoryOptimizer::getImagePool() {
    return imagePool;
}

MemoryOptimizer::BufferPool* MemoryOptimizer::getMessagePool() {
    return messagePool;
}

MemoryOptimizer::CircularAnalyticsBuffer* MemoryOptimizer::getAnalyticsBuffer() {
    return analyticsBuffer;
}

// BufferPool implementation
MemoryOptimizer::BufferPool::BufferPool(size_t blockSize, size_t count) 
    : pool(nullptr), blockSize(blockSize), blockCount(count), usedCount(0) {
    allocated.resize(count, false);
}

MemoryOptimizer::BufferPool::~BufferPool() {
    cleanup();
}

bool MemoryOptimizer::BufferPool::init() {
    size_t totalSize = blockSize * blockCount;
    
    // Try to allocate in PSRAM first for large pools
    if (esp_psram_is_initialized() && totalSize > 32768) {
        pool = (uint8_t*)heap_caps_malloc(totalSize, MALLOC_CAP_SPIRAM);
    }
    
    // Fallback to internal memory
    if (!pool) {
        pool = (uint8_t*)heap_caps_malloc(totalSize, MALLOC_CAP_8BIT);
    }
    
    if (!pool) {
        DEBUG_PRINTF("ERROR: Failed to allocate buffer pool (%zu bytes)\n", totalSize);
        return false;
    }
    
    DEBUG_PRINTF("Buffer pool allocated: %zu blocks of %zu bytes\n", blockCount, blockSize);
    return true;
}

void MemoryOptimizer::BufferPool::cleanup() {
    if (pool) {
        heap_caps_free(pool);
        pool = nullptr;
    }
    usedCount = 0;
    std::fill(allocated.begin(), allocated.end(), false);
}

uint8_t* MemoryOptimizer::BufferPool::acquire() {
    for (size_t i = 0; i < blockCount; i++) {
        if (!allocated[i]) {
            allocated[i] = true;
            usedCount++;
            return pool + (i * blockSize);
        }
    }
    return nullptr; // No available blocks
}

void MemoryOptimizer::BufferPool::release(uint8_t* ptr) {
    if (!ptr || !pool) return;
    
    // Calculate block index
    size_t offset = ptr - pool;
    size_t blockIndex = offset / blockSize;
    
    if (blockIndex < blockCount && allocated[blockIndex]) {
        allocated[blockIndex] = false;
        usedCount--;
    }
}

float MemoryOptimizer::BufferPool::utilizationPercent() const {
    return blockCount > 0 ? (float(usedCount) * 100.0f / float(blockCount)) : 0.0f;
}

size_t MemoryOptimizer::BufferPool::getAvailableBlocks() const {
    return blockCount - usedCount;
}

// CircularAnalyticsBuffer implementation
MemoryOptimizer::CircularAnalyticsBuffer::CircularAnalyticsBuffer(size_t capacity)
    : buffer(nullptr), capacity(capacity), writePos(0), readPos(0), 
      overflow(false), compressed(false) {
}

MemoryOptimizer::CircularAnalyticsBuffer::~CircularAnalyticsBuffer() {
    cleanup();
}

bool MemoryOptimizer::CircularAnalyticsBuffer::init() {
    // Prefer PSRAM for analytics buffer if available
    if (esp_psram_is_initialized()) {
        buffer = (uint8_t*)heap_caps_malloc(capacity, MALLOC_CAP_SPIRAM);
    }
    
    if (!buffer) {
        buffer = (uint8_t*)heap_caps_malloc(capacity, MALLOC_CAP_8BIT);
    }
    
    if (!buffer) {
        DEBUG_PRINTF("ERROR: Failed to allocate analytics buffer (%zu bytes)\n", capacity);
        return false;
    }
    
    DEBUG_PRINTF("Analytics buffer allocated: %zu bytes\n", capacity);
    return true;
}

void MemoryOptimizer::CircularAnalyticsBuffer::cleanup() {
    if (buffer) {
        heap_caps_free(buffer);
        buffer = nullptr;
    }
    writePos = 0;
    readPos = 0;
    overflow = false;
    compressed = false;
}

bool MemoryOptimizer::CircularAnalyticsBuffer::write(const void* data, size_t size) {
    if (!buffer || !data || size == 0) return false;
    
    const uint8_t* src = (const uint8_t*)data;
    
    for (size_t i = 0; i < size; i++) {
        buffer[writePos] = src[i];
        writePos = (writePos + 1) % capacity;
        
        // Check for buffer overflow
        if (writePos == readPos) {
            overflow = true;
            readPos = (readPos + 1) % capacity; // Advance read position
        }
    }
    
    return true;
}

bool MemoryOptimizer::CircularAnalyticsBuffer::read(void* data, size_t size) {
    if (!buffer || !data || size == 0 || readPos == writePos) return false;
    
    uint8_t* dest = (uint8_t*)data;
    size_t bytesRead = 0;
    
    while (bytesRead < size && readPos != writePos) {
        dest[bytesRead] = buffer[readPos];
        readPos = (readPos + 1) % capacity;
        bytesRead++;
    }
    
    return bytesRead > 0;
}

void MemoryOptimizer::CircularAnalyticsBuffer::compress() {
    // Placeholder for LZ4 compression implementation
    // This would compress the buffer contents in-place
    compressed = true;
}

size_t MemoryOptimizer::CircularAnalyticsBuffer::getUsedSpace() const {
    if (writePos >= readPos) {
        return writePos - readPos;
    } else {
        return capacity - readPos + writePos;
    }
}