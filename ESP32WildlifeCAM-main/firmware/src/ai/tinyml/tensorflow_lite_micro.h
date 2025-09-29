/**
 * @file tensorflow_lite_micro.h
 * @brief TensorFlow Lite Micro integration for ESP32
 * 
 * Provides TensorFlow Lite Micro functionality specifically optimized
 * for ESP32 wildlife camera applications.
 */

#ifndef TENSORFLOW_LITE_MICRO_H
#define TENSORFLOW_LITE_MICRO_H

#include "../ai_common.h"

// TensorFlow Lite Micro includes (conditional compilation)
#ifdef TFLITE_MICRO_ENABLED
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#endif

/**
 * TensorFlow Lite Micro Manager
 * 
 * Handles the initialization and management of TensorFlow Lite Micro
 * environment on ESP32 with optimizations for wildlife detection.
 */
class TensorFlowLiteMicro {
public:
    static bool init();
    static void cleanup();
    static bool isInitialized();
    
    // Memory management
    static void setGlobalArenaSize(size_t size);
    static size_t getGlobalArenaSize();
    static size_t getAvailableMemory();
    
    // ESP32-specific optimizations
    static void enableESP32Optimizations();
    static void configurePSRAM();
    static void enableVectorInstructions();
    
    // Power management
    static void enablePowerSaving();
    static void setClockFrequency(uint32_t frequencyMHz);
    
    // Debugging and diagnostics
    static void enableDebugLogging(bool enable = true);
    static void printSystemInfo();
    static bool selfTest();

private:
    static bool initialized_;
    static size_t globalArenaSize_;
    static bool debugLoggingEnabled_;
    
    // ESP32-specific initialization
    static bool initializeESP32Features();
    static bool configureFPU();
    static bool configureCache();
};

// Operator resolver for wildlife detection models
class WildlifeOpResolver {
public:
    WildlifeOpResolver();
    
    // Add operators commonly used in wildlife detection models
    void addCommonOps();
    void addConvolutionOps();
    void addPoolingOps();
    void addActivationOps();
    void addNormalizationOps();
    
    // Get the resolver for use with interpreter
    void* getResolver();

private:
    void* resolver_;
};

// Memory allocator optimized for ESP32
class ESP32MemoryAllocator {
public:
    static void* allocate(size_t size);
    static void deallocate(void* ptr);
    static size_t getUsedMemory();
    static size_t getAvailableMemory();
    static void defragment();
    
    // PSRAM-specific functions
    static void* allocatePSRAM(size_t size);
    static bool isPSRAMAvailable();
    static size_t getPSRAMSize();

private:
    static size_t totalAllocated_;
    static const size_t ALIGNMENT = 16;  // 16-byte alignment for ESP32
};

#endif // TENSORFLOW_LITE_MICRO_H