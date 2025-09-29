/**
 * @file tensorflow_lite_esp32.h
 * @brief ESP32-specific TensorFlow Lite implementation for wildlife classification
 * 
 * Provides optimized TensorFlow Lite Micro interface for ESP32-S3 with memory constraints.
 * Designed for production-ready wildlife detection with minimal resource usage.
 */

#ifndef TENSORFLOW_LITE_ESP32_H
#define TENSORFLOW_LITE_ESP32_H

#include <stdint.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Constants for ESP32-S3 optimization
#define MAX_ARENA_SIZE          (2 * 1024 * 1024)  // 2MB maximum
#define DEFAULT_ARENA_SIZE      (512 * 1024)       // 512KB default
#define MAX_CONCURRENT_MODELS   3                  // Maximum models loaded simultaneously
#define MAX_OUTPUT_CLASSES      50                 // Maximum species classes

/**
 * Model handle structure for managing loaded models
 */
typedef struct {
    uint8_t id;                    // Slot ID
    const uint8_t* modelData;      // Pointer to model data
    size_t modelSize;              // Size of model in bytes
    uint8_t* arenaPtr;             // Pointer to tensor arena
    size_t arenaSize;              // Size of tensor arena  
    bool active;                   // Whether slot is active
} ModelHandle;

/**
 * Inference result structure
 */
typedef struct {
    bool success;                              // Whether inference succeeded
    uint8_t detectedClass;                     // Most confident class ID
    float maxConfidence;                       // Maximum confidence score
    float confidenceScores[MAX_OUTPUT_CLASSES]; // All confidence scores
    uint8_t numClasses;                        // Number of output classes
    uint32_t inferenceTimeMs;                  // Inference time in milliseconds
} InferenceResult;

/**
 * Performance statistics structure
 */
typedef struct {
    uint32_t totalInferences;      // Total inferences performed
    uint32_t totalInferenceTime;   // Total inference time (ms)
    uint32_t averageInferenceTime; // Average inference time (ms)
    uint32_t minInferenceTime;     // Minimum inference time (ms)
    uint32_t maxInferenceTime;     // Maximum inference time (ms)
} PerformanceStats;

/**
 * TensorFlow Lite ESP32 Implementation Class
 * 
 * Provides hardware-optimized TensorFlow Lite Micro implementation
 * specifically designed for ESP32-S3 wildlife classification.
 */
class TensorFlowLiteESP32 {
public:
    TensorFlowLiteESP32();
    ~TensorFlowLiteESP32();

    /**
     * Initialize TensorFlow Lite ESP32 implementation
     * @param arenaSize Size of tensor arena in bytes (default: DEFAULT_ARENA_SIZE)
     * @return true if initialization successful
     */
    bool initialize(size_t arenaSize = DEFAULT_ARENA_SIZE);

    /**
     * Clean up resources and shut down
     */
    void cleanup();

    /**
     * Load a TensorFlow Lite model
     * @param modelData Pointer to model data in memory
     * @param modelSize Size of model data in bytes
     * @param handle Pointer to model handle structure (output)
     * @return true if model loaded successfully
     */
    bool loadModel(const uint8_t* modelData, size_t modelSize, ModelHandle* handle);

    /**
     * Unload a previously loaded model
     * @param handle Pointer to model handle
     */
    void unloadModel(ModelHandle* handle);

    /**
     * Run inference on loaded model
     * @param handle Pointer to model handle
     * @param inputData Input tensor data
     * @param inputSize Size of input data
     * @return Inference result structure
     */
    InferenceResult runInference(ModelHandle* handle, const float* inputData, size_t inputSize);

    /**
     * Get current memory usage
     * @return Current memory usage in bytes
     */
    size_t getMemoryUsage() const;

    /**
     * Get available free memory
     * @return Free memory in bytes
     */
    size_t getFreeMemory() const;

    /**
     * Get performance statistics
     * @return Performance statistics structure
     */
    PerformanceStats getPerformanceStats() const;

    /**
     * Reset performance statistics
     */
    void resetPerformanceStats();

    /**
     * Check if system is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_;                      // Initialization status
    size_t currentArenaSize_;              // Current arena size
    size_t maxArenaSize_;                  // Maximum allowed arena size
    uint8_t* tensorArena_;                 // Pointer to tensor arena
    uint8_t activeModels_;                 // Number of active models
    ModelHandle modelSlots_[MAX_CONCURRENT_MODELS]; // Model storage slots
    PerformanceStats performance_;         // Performance tracking

    /**
     * Simulate inference for development/testing
     * @param inputData Input data
     * @param inputSize Input size
     * @param result Result structure to populate
     */
    void simulateInference(const float* inputData, size_t inputSize, InferenceResult* result);

    /**
     * Update performance metrics
     * @param inferenceTime Time taken for inference
     */
    void updatePerformanceMetrics(uint32_t inferenceTime);
};

// Global instance and initialization functions
extern TensorFlowLiteESP32* g_tflite_esp32;

/**
 * Initialize global TensorFlow Lite ESP32 instance
 * @param arenaSize Size of tensor arena
 * @return true if initialization successful
 */
bool initializeTensorFlowLiteESP32(size_t arenaSize = DEFAULT_ARENA_SIZE);

/**
 * Clean up global TensorFlow Lite ESP32 instance
 */
void cleanupTensorFlowLiteESP32();

#endif // TENSORFLOW_LITE_ESP32_H