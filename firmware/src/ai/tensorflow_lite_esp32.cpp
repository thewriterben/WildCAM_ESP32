/**
 * @file tensorflow_lite_esp32.cpp
 * @brief ESP32-specific TensorFlow Lite implementation for wildlife classification
 * 
 * Optimized TensorFlow Lite Micro implementation for ESP32-S3 with 8MB PSRAM constraint.
 * Focuses on minimal memory usage and efficient inference for wildlife detection.
 */

#include "tensorflow_lite_esp32.h"
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_log.h>

static const char* TAG = "TFLite_ESP32";

// Global instance
TensorFlowLiteESP32* g_tflite_esp32 = nullptr;

TensorFlowLiteESP32::TensorFlowLiteESP32() 
    : initialized_(false)
    , currentArenaSize_(0)
    , maxArenaSize_(MAX_ARENA_SIZE)
    , tensorArena_(nullptr)
    , activeModels_(0) {
    
    memset(&performance_, 0, sizeof(performance_));
}

TensorFlowLiteESP32::~TensorFlowLiteESP32() {
    cleanup();
}

bool TensorFlowLiteESP32::initialize(size_t arenaSize) {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing TensorFlow Lite ESP32 with arena size: %zu bytes", arenaSize);

    // Validate arena size constraints
    if (arenaSize > maxArenaSize_) {
        ESP_LOGE(TAG, "Arena size %zu exceeds maximum %zu", arenaSize, maxArenaSize_);
        return false;
    }

    // Allocate tensor arena preferring PSRAM
    tensorArena_ = static_cast<uint8_t*>(heap_caps_malloc(arenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!tensorArena_) {
        // Fallback to internal RAM
        tensorArena_ = static_cast<uint8_t*>(heap_caps_malloc(arenaSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }

    if (!tensorArena_) {
        ESP_LOGE(TAG, "Failed to allocate tensor arena of size %zu", arenaSize);
        return false;
    }

    currentArenaSize_ = arenaSize;
    initialized_ = true;

    ESP_LOGI(TAG, "TensorFlow Lite ESP32 initialized successfully");
    ESP_LOGI(TAG, "Arena allocated at: %p (PSRAM: %s)", 
             tensorArena_, heap_caps_get_free_size(MALLOC_CAP_SPIRAM) > 0 ? "yes" : "no");

    return true;
}

void TensorFlowLiteESP32::cleanup() {
    if (tensorArena_) {
        heap_caps_free(tensorArena_);
        tensorArena_ = nullptr;
    }
    
    currentArenaSize_ = 0;
    activeModels_ = 0;
    initialized_ = false;
    
    ESP_LOGI(TAG, "TensorFlow Lite ESP32 cleaned up");
}

bool TensorFlowLiteESP32::loadModel(const uint8_t* modelData, size_t modelSize, ModelHandle* handle) {
    if (!initialized_ || !modelData || !handle) {
        ESP_LOGE(TAG, "Invalid parameters for model loading");
        return false;
    }

    if (activeModels_ >= MAX_CONCURRENT_MODELS) {
        ESP_LOGE(TAG, "Maximum concurrent models reached");
        return false;
    }

    // Initialize model handle
    memset(handle, 0, sizeof(ModelHandle));
    handle->modelData = modelData;
    handle->modelSize = modelSize;
    handle->arenaPtr = tensorArena_;
    handle->arenaSize = currentArenaSize_;

    // Store model in available slot
    for (int i = 0; i < MAX_CONCURRENT_MODELS; i++) {
        if (!modelSlots_[i].active) {
            modelSlots_[i] = *handle;
            modelSlots_[i].active = true;
            handle->id = i;
            activeModels_++;
            
            ESP_LOGI(TAG, "Model loaded in slot %d, size: %zu bytes", i, modelSize);
            return true;
        }
    }

    ESP_LOGE(TAG, "No available model slots");
    return false;
}

void TensorFlowLiteESP32::unloadModel(ModelHandle* handle) {
    if (!handle || handle->id >= MAX_CONCURRENT_MODELS) {
        return;
    }

    if (modelSlots_[handle->id].active) {
        modelSlots_[handle->id].active = false;
        activeModels_--;
        ESP_LOGI(TAG, "Model unloaded from slot %d", handle->id);
    }

    memset(handle, 0, sizeof(ModelHandle));
}

InferenceResult TensorFlowLiteESP32::runInference(ModelHandle* handle, const float* inputData, size_t inputSize) {
    InferenceResult result = {};
    result.success = false;

    if (!initialized_ || !handle || !inputData) {
        ESP_LOGE(TAG, "Invalid parameters for inference");
        return result;
    }

    if (!modelSlots_[handle->id].active) {
        ESP_LOGE(TAG, "Model slot %d is not active", handle->id);
        return result;
    }

    uint32_t startTime = esp_timer_get_time() / 1000; // Convert to milliseconds

    // TODO: Actual TensorFlow Lite inference would go here
    // For now, simulate inference with placeholder logic
    simulateInference(inputData, inputSize, &result);

    uint32_t endTime = esp_timer_get_time() / 1000;
    result.inferenceTimeMs = endTime - startTime;
    result.success = true;

    // Update performance metrics
    updatePerformanceMetrics(result.inferenceTimeMs);

    ESP_LOGD(TAG, "Inference completed in %lu ms", result.inferenceTimeMs);
    return result;
}

size_t TensorFlowLiteESP32::getMemoryUsage() const {
    return currentArenaSize_;
}

size_t TensorFlowLiteESP32::getFreeMemory() const {
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

PerformanceStats TensorFlowLiteESP32::getPerformanceStats() const {
    return performance_;
}

void TensorFlowLiteESP32::resetPerformanceStats() {
    memset(&performance_, 0, sizeof(performance_));
}

void TensorFlowLiteESP32::simulateInference(const float* inputData, size_t inputSize, InferenceResult* result) {
    // Placeholder inference simulation for wildlife classification
    // In production, this would be replaced with actual TensorFlow Lite inference
    
    result->confidenceScores[0] = 0.85f; // High confidence for detected species
    result->confidenceScores[1] = 0.12f; // Low confidence for alternative
    result->confidenceScores[2] = 0.03f; // Very low confidence
    
    result->detectedClass = 0; // Most confident class
    result->maxConfidence = result->confidenceScores[0];
    result->numClasses = 3;
    
    // Simulate processing delay based on input size
    vTaskDelay(pdMS_TO_TICKS(inputSize / 1000)); // Simple delay simulation
}

void TensorFlowLiteESP32::updatePerformanceMetrics(uint32_t inferenceTime) {
    performance_.totalInferences++;
    performance_.totalInferenceTime += inferenceTime;
    performance_.averageInferenceTime = performance_.totalInferenceTime / performance_.totalInferences;
    
    if (inferenceTime > performance_.maxInferenceTime) {
        performance_.maxInferenceTime = inferenceTime;
    }
    
    if (performance_.minInferenceTime == 0 || inferenceTime < performance_.minInferenceTime) {
        performance_.minInferenceTime = inferenceTime;
    }
}

// Global initialization function
bool initializeTensorFlowLiteESP32(size_t arenaSize) {
    if (!g_tflite_esp32) {
        g_tflite_esp32 = new TensorFlowLiteESP32();
    }
    
    return g_tflite_esp32->initialize(arenaSize);
}

void cleanupTensorFlowLiteESP32() {
    if (g_tflite_esp32) {
        delete g_tflite_esp32;
        g_tflite_esp32 = nullptr;
    }
}