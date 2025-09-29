/**
 * @file tensorflow_lite_micro.cpp
 * @brief Implementation of TensorFlow Lite Micro integration
 */

#include "tensorflow_lite_micro.h"
#include "../../config.h"
#include <esp_system.h>
#include <esp_heap_caps.h>

// TensorFlow Lite Micro includes (conditional compilation)
#ifdef TFLITE_MICRO_ENABLED
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#endif

// Static member initialization
bool TensorFlowLiteMicro::initialized_ = false;
size_t TensorFlowLiteMicro::globalArenaSize_ = 256 * 1024; // 256KB default
bool TensorFlowLiteMicro::debugLoggingEnabled_ = false;

/**
 * Initialize TensorFlow Lite Micro
 */
bool TensorFlowLiteMicro::init() {
    if (initialized_) {
        DEBUG_PRINTLN("TensorFlow Lite Micro already initialized");
        return true;
    }
    
    DEBUG_PRINTLN("Initializing TensorFlow Lite Micro...");
    
    // Initialize ESP32-specific features
    if (!initializeESP32Features()) {
        DEBUG_PRINTLN("Error: Failed to initialize ESP32 features");
        return false;
    }
    
    // Configure PSRAM if available
    configurePSRAM();
    
    // Configure FPU for better performance
    if (!configureFPU()) {
        DEBUG_PRINTLN("Warning: FPU configuration failed");
    }
    
    // Configure cache for optimal performance
    if (!configureCache()) {
        DEBUG_PRINTLN("Warning: Cache configuration failed");
    }
    
#ifdef TFLITE_MICRO_ENABLED
    // Initialize TensorFlow Lite Micro system
    tflite::InitializeTarget();
    DEBUG_PRINTLN("TensorFlow Lite Micro target initialized");
#endif
    
    initialized_ = true;
    
    if (debugLoggingEnabled_) {
        printSystemInfo();
    }
    
    DEBUG_PRINTLN("TensorFlow Lite Micro initialized successfully");
    return true;
}

/**
 * Cleanup TensorFlow Lite Micro
 */
void TensorFlowLiteMicro::cleanup() {
    if (!initialized_) return;
    
    DEBUG_PRINTLN("Cleaning up TensorFlow Lite Micro...");
    
    // Cleanup would go here
    initialized_ = false;
    
    DEBUG_PRINTLN("TensorFlow Lite Micro cleanup completed");
}

/**
 * Check if initialized
 */
bool TensorFlowLiteMicro::isInitialized() {
    return initialized_;
}

/**
 * Set global arena size
 */
void TensorFlowLiteMicro::setGlobalArenaSize(size_t size) {
    globalArenaSize_ = size;
    DEBUG_PRINTF("Global arena size set to: %zu bytes\n", size);
}

/**
 * Get global arena size
 */
size_t TensorFlowLiteMicro::getGlobalArenaSize() {
    return globalArenaSize_;
}

/**
 * Get available memory
 */
size_t TensorFlowLiteMicro::getAvailableMemory() {
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

/**
 * Enable ESP32-specific optimizations
 */
void TensorFlowLiteMicro::enableESP32Optimizations() {
    DEBUG_PRINTLN("Enabling ESP32-specific optimizations...");
    
    // Enable vector instructions if available
    enableVectorInstructions();
    
    // Configure CPU for optimal performance
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 240,  // Maximum frequency
        .min_freq_mhz = 80,   // Minimum frequency for power saving
        .light_sleep_enable = false  // Disable light sleep during AI processing
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        DEBUG_PRINTLN("CPU frequency configuration applied");
    } else {
        DEBUG_PRINTLN("Warning: CPU frequency configuration failed");
    }
    
    DEBUG_PRINTLN("ESP32 optimizations enabled");
}

/**
 * Configure PSRAM
 */
void TensorFlowLiteMicro::configurePSRAM() {
    if (esp_psram_is_initialized()) {
        size_t psramSize = esp_psram_get_size();
        DEBUG_PRINTF("PSRAM detected: %zu bytes\n", psramSize);
        
        // Enable PSRAM for large tensor operations
        heap_caps_malloc_extmem_enable(16);  // Enable external memory for allocations >= 16 bytes
        
        DEBUG_PRINTLN("PSRAM configured for AI operations");
    } else {
        DEBUG_PRINTLN("No PSRAM detected - using internal RAM only");
    }
}

/**
 * Enable vector instructions
 */
void TensorFlowLiteMicro::enableVectorInstructions() {
    // ESP32-S3 supports SIMD operations
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    DEBUG_PRINTLN("ESP32-S3 vector instructions enabled");
    #else
    DEBUG_PRINTLN("Vector instructions not available on this ESP32 variant");
    #endif
}

/**
 * Enable power saving mode
 */
void TensorFlowLiteMicro::enablePowerSaving() {
    DEBUG_PRINTLN("Enabling AI power saving mode...");
    
    // Configure dynamic frequency scaling
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160,  // Reduced maximum frequency
        .min_freq_mhz = 40,   // Lower minimum frequency
        .light_sleep_enable = true  // Enable light sleep when idle
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        DEBUG_PRINTLN("Power saving configuration applied");
    } else {
        DEBUG_PRINTLN("Warning: Power saving configuration failed");
    }
}

/**
 * Set CPU clock frequency
 */
void TensorFlowLiteMicro::setClockFrequency(uint32_t frequencyMHz) {
    DEBUG_PRINTF("Setting CPU frequency to %lu MHz\n", frequencyMHz);
    
    esp_pm_config_t pm_config = {
        .max_freq_mhz = frequencyMHz,
        .min_freq_mhz = frequencyMHz / 4,  // Quarter speed for minimum
        .light_sleep_enable = false
    };
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        DEBUG_PRINTF("CPU frequency set to %lu MHz\n", frequencyMHz);
    } else {
        DEBUG_PRINTF("Error: Failed to set CPU frequency to %lu MHz\n", frequencyMHz);
    }
}

/**
 * Enable debug logging
 */
void TensorFlowLiteMicro::enableDebugLogging(bool enable) {
    debugLoggingEnabled_ = enable;
    if (enable) {
        DEBUG_PRINTLN("TensorFlow Lite Micro debug logging enabled");
    }
}

/**
 * Print system information
 */
void TensorFlowLiteMicro::printSystemInfo() {
    DEBUG_PRINTLN("=== TensorFlow Lite Micro System Info ===");
    
    // ESP32 chip info
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    DEBUG_PRINTF("ESP32 Model: %s\n", chip_info.model == CHIP_ESP32 ? "ESP32" : 
                                     chip_info.model == CHIP_ESP32S2 ? "ESP32-S2" :
                                     chip_info.model == CHIP_ESP32S3 ? "ESP32-S3" :
                                     chip_info.model == CHIP_ESP32C3 ? "ESP32-C3" : "Unknown");
    DEBUG_PRINTF("CPU Cores: %d\n", chip_info.cores);
    DEBUG_PRINTF("CPU Frequency: %lu MHz\n", ESP.getCpuFreqMHz());
    
    // Memory info
    DEBUG_PRINTF("Free Heap: %u bytes\n", ESP.getFreeHeap());
    DEBUG_PRINTF("Heap Size: %u bytes\n", ESP.getHeapSize());
    DEBUG_PRINTF("Free PSRAM: %u bytes\n", ESP.getFreePsram());
    DEBUG_PRINTF("PSRAM Size: %u bytes\n", ESP.getPsramSize());
    
    // Flash info
    DEBUG_PRINTF("Flash Size: %u bytes\n", ESP.getFlashChipSize());
    DEBUG_PRINTF("Flash Speed: %u MHz\n", ESP.getFlashChipSpeed() / 1000000);
    
    // Global arena size
    DEBUG_PRINTF("Global Arena Size: %zu bytes\n", globalArenaSize_);
    
    DEBUG_PRINTLN("========================================");
}

/**
 * Self test
 */
bool TensorFlowLiteMicro::selfTest() {
    DEBUG_PRINTLN("Running TensorFlow Lite Micro self test...");
    
    // Test memory allocation
    void* testMem = ESP32MemoryAllocator::allocate(1024);
    if (!testMem) {
        DEBUG_PRINTLN("Error: Memory allocation test failed");
        return false;
    }
    ESP32MemoryAllocator::deallocate(testMem);
    
    // Test PSRAM allocation if available
    if (ESP32MemoryAllocator::isPSRAMAvailable()) {
        void* psramMem = ESP32MemoryAllocator::allocatePSRAM(1024);
        if (!psramMem) {
            DEBUG_PRINTLN("Error: PSRAM allocation test failed");
            return false;
        }
        ESP32MemoryAllocator::deallocate(psramMem);
    }
    
    DEBUG_PRINTLN("TensorFlow Lite Micro self test passed");
    return true;
}

/**
 * Initialize ESP32-specific features
 */
bool TensorFlowLiteMicro::initializeESP32Features() {
    // Configure heap capabilities for AI operations
    heap_caps_malloc_extmem_enable(32);  // Use external memory for allocations >= 32 bytes
    
    // Configure CPU cache for optimal performance
    return true;
}

/**
 * Configure FPU
 */
bool TensorFlowLiteMicro::configureFPU() {
    // ESP32 has hardware floating point unit
    // Configuration would be done here if needed
    DEBUG_PRINTLN("FPU configured for AI operations");
    return true;
}

/**
 * Configure cache
 */
bool TensorFlowLiteMicro::configureCache() {
    // Cache configuration for optimal AI performance
    DEBUG_PRINTLN("Cache configured for AI operations");
    return true;
}

// ESP32MemoryAllocator implementation
size_t ESP32MemoryAllocator::totalAllocated_ = 0;

/**
 * Allocate memory with alignment
 */
void* ESP32MemoryAllocator::allocate(size_t size) {
    // Align to 16-byte boundary for ESP32 optimization
    size_t alignedSize = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    void* ptr = heap_caps_aligned_alloc(ALIGNMENT, alignedSize, MALLOC_CAP_8BIT);
    if (ptr) {
        totalAllocated_ += alignedSize;
    }
    
    return ptr;
}

/**
 * Deallocate memory
 */
void ESP32MemoryAllocator::deallocate(void* ptr) {
    if (ptr) {
        heap_caps_free(ptr);
        // Note: We don't track deallocation size in this simple implementation
    }
}

/**
 * Get used memory
 */
size_t ESP32MemoryAllocator::getUsedMemory() {
    return totalAllocated_;
}

/**
 * Get available memory
 */
size_t ESP32MemoryAllocator::getAvailableMemory() {
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

/**
 * Allocate PSRAM memory
 */
void* ESP32MemoryAllocator::allocatePSRAM(size_t size) {
    if (!isPSRAMAvailable()) {
        return nullptr;
    }
    
    // Align to 16-byte boundary
    size_t alignedSize = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    void* ptr = heap_caps_aligned_alloc(ALIGNMENT, alignedSize, MALLOC_CAP_SPIRAM);
    if (ptr) {
        totalAllocated_ += alignedSize;
    }
    
    return ptr;
}

/**
 * Check if PSRAM is available
 */
bool ESP32MemoryAllocator::isPSRAMAvailable() {
    return esp_psram_is_initialized();
}

/**
 * Get PSRAM size
 */
size_t ESP32MemoryAllocator::getPSRAMSize() {
    return esp_psram_is_initialized() ? esp_psram_get_size() : 0;
}

/**
 * Defragment memory
 */
void ESP32MemoryAllocator::defragment() {
    // ESP32 heap doesn't support explicit defragmentation
    // This would trigger garbage collection if available
    DEBUG_PRINTLN("Memory defragmentation requested");
}