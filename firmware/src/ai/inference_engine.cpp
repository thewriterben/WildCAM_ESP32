/**
 * @file inference_engine.cpp
 * @brief Optimized inference engine implementation for ESP32 wildlife classification
 */

#include "inference_engine.h"
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <string.h>

static const char* TAG = "InferenceEngine";

// Default preprocessing configuration
static const PreprocessingConfig DEFAULT_PREPROCESS_CONFIG = {
    .normalizePixels = true,
    .subtractMean = true,
    .resizeImage = true,
    .meanRGB = {0.485f, 0.456f, 0.406f},  // ImageNet mean
    .stdRGB = {0.229f, 0.224f, 0.225f}    // ImageNet std
};

InferenceEngine::InferenceEngine()
    : initialized_(false)
    , modelManager_(nullptr)
    , tfliteEngine_(nullptr)
    , preprocessBuffer_(nullptr)
    , previousFrameBuffer_(nullptr) {
    
    memset(&speciesModel_, 0, sizeof(speciesModel_));
    memset(&behaviorModel_, 0, sizeof(behaviorModel_));
    memset(&motionModel_, 0, sizeof(motionModel_));
    memset(&performanceStats_, 0, sizeof(performanceStats_));
    
    preprocessConfig_ = DEFAULT_PREPROCESS_CONFIG;
}

InferenceEngine::~InferenceEngine() {
    shutdown();
}

bool InferenceEngine::initialize(ModelManager* modelManager) {
    if (initialized_) {
        ESP_LOGW(TAG, "Already initialized");
        return true;
    }

    if (!modelManager) {
        ESP_LOGE(TAG, "Model manager is null");
        return false;
    }

    ESP_LOGI(TAG, "Initializing Inference Engine");

    modelManager_ = modelManager;

    // Initialize TensorFlow Lite ESP32 engine
    if (!initializeTensorFlowLiteESP32()) {
        ESP_LOGE(TAG, "Failed to initialize TensorFlow Lite ESP32");
        return false;
    }

    tfliteEngine_ = g_tflite_esp32;

    // Allocate preprocessing buffer
    preprocessBuffer_ = (float*)heap_caps_malloc(INPUT_SIZE * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!preprocessBuffer_) {
        preprocessBuffer_ = (float*)heap_caps_malloc(INPUT_SIZE * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    if (!preprocessBuffer_) {
        ESP_LOGE(TAG, "Failed to allocate preprocessing buffer");
        return false;
    }

    // Allocate previous frame buffer for motion detection
    previousFrameBuffer_ = (float*)heap_caps_malloc(INPUT_SIZE * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!previousFrameBuffer_) {
        previousFrameBuffer_ = (float*)heap_caps_malloc(INPUT_SIZE * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    if (!previousFrameBuffer_) {
        ESP_LOGE(TAG, "Failed to allocate previous frame buffer");
        heap_caps_free(preprocessBuffer_);
        preprocessBuffer_ = nullptr;
        return false;
    }

    // Load required models
    if (!loadModels()) {
        ESP_LOGE(TAG, "Failed to load models");
        shutdown();
        return false;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "Inference Engine initialized successfully");

    return true;
}

void InferenceEngine::shutdown() {
    if (!initialized_) {
        return;
    }

    ESP_LOGI(TAG, "Shutting down Inference Engine");

    unloadModels();

    if (preprocessBuffer_) {
        heap_caps_free(preprocessBuffer_);
        preprocessBuffer_ = nullptr;
    }

    if (previousFrameBuffer_) {
        heap_caps_free(previousFrameBuffer_);
        previousFrameBuffer_ = nullptr;
    }

    cleanupTensorFlowLiteESP32();
    tfliteEngine_ = nullptr;
    modelManager_ = nullptr;
    initialized_ = false;

    ESP_LOGI(TAG, "Inference Engine shut down");
}

bool InferenceEngine::loadModels() {
    ESP_LOGI(TAG, "Loading inference models");

    // Load species classifier model
    if (!modelManager_->loadModel(MODEL_SPECIES_CLASSIFIER)) {
        ESP_LOGE(TAG, "Failed to load species classifier model");
        return false;
    }

    // Load behavior analyzer model
    if (!modelManager_->loadModel(MODEL_BEHAVIOR_ANALYZER)) {
        ESP_LOGE(TAG, "Failed to load behavior analyzer model");
        return false;
    }

    // Load motion detector model
    if (!modelManager_->loadModel(MODEL_MOTION_DETECTOR)) {
        ESP_LOGE(TAG, "Failed to load motion detector model");
        return false;
    }

    // Initialize model handles (placeholder data for now)
    const ModelInfo* speciesInfo = modelManager_->getModelInfo(MODEL_SPECIES_CLASSIFIER);
    const ModelInfo* behaviorInfo = modelManager_->getModelInfo(MODEL_BEHAVIOR_ANALYZER);
    const ModelInfo* motionInfo = modelManager_->getModelInfo(MODEL_MOTION_DETECTOR);

    if (!speciesInfo || !behaviorInfo || !motionInfo) {
        ESP_LOGE(TAG, "Failed to get model information");
        return false;
    }

    ESP_LOGI(TAG, "Models loaded successfully");
    return true;
}

void InferenceEngine::unloadModels() {
    if (modelManager_) {
        modelManager_->unloadModel(MODEL_SPECIES_CLASSIFIER);
        modelManager_->unloadModel(MODEL_BEHAVIOR_ANALYZER);
        modelManager_->unloadModel(MODEL_MOTION_DETECTOR);
    }

    if (tfliteEngine_) {
        tfliteEngine_->unloadModel(&speciesModel_);
        tfliteEngine_->unloadModel(&behaviorModel_);
        tfliteEngine_->unloadModel(&motionModel_);
    }
}

void InferenceEngine::setPreprocessingConfig(const PreprocessingConfig& config) {
    preprocessConfig_ = config;
    ESP_LOGI(TAG, "Preprocessing configuration updated");
}

bool InferenceEngine::preprocessImage(const uint8_t* imageData, uint16_t width, uint16_t height, 
                                     uint8_t channels, float* outputBuffer) {
    if (!imageData || !outputBuffer) {
        ESP_LOGE(TAG, "Invalid input parameters for preprocessing");
        return false;
    }

    ESP_LOGD(TAG, "Preprocessing image: %dx%dx%d", width, height, channels);

    // Resize image if needed
    if (preprocessConfig_.resizeImage && (width != INPUT_WIDTH || height != INPUT_HEIGHT)) {
        resizeImage(imageData, width, height, outputBuffer, INPUT_WIDTH, INPUT_HEIGHT, channels);
    } else {
        // Direct copy and convert to float
        for (int i = 0; i < INPUT_SIZE; i++) {
            outputBuffer[i] = (float)imageData[i];
        }
    }

    // Normalize pixels
    if (preprocessConfig_.normalizePixels || preprocessConfig_.subtractMean) {
        normalizePixels(outputBuffer, INPUT_SIZE, preprocessConfig_);
    }

    return true;
}

bool InferenceEngine::classifySpecies(const float* imageData, SpeciesResult* result) {
    if (!initialized_ || !imageData || !result) {
        ESP_LOGE(TAG, "Invalid parameters for species classification");
        return false;
    }

    uint32_t startTime = esp_timer_get_time() / 1000;

    // Run inference using TensorFlow Lite ESP32
    InferenceResult inferenceResult = tfliteEngine_->runInference(&speciesModel_, imageData, INPUT_SIZE);

    if (!inferenceResult.success) {
        ESP_LOGE(TAG, "Species classification inference failed");
        return false;
    }

    // Process results
    result->speciesId = inferenceResult.detectedClass;
    result->confidence = inferenceResult.maxConfidence;
    result->inferenceTime = inferenceResult.inferenceTimeMs;
    result->valid = true;

    // Map species ID to name (simplified mapping)
    snprintf(result->speciesName, sizeof(result->speciesName), "Species_%d", result->speciesId);

    uint32_t endTime = esp_timer_get_time() / 1000;
    updatePerformanceStats(endTime - startTime, true);

    ESP_LOGD(TAG, "Species classification: ID=%d, Confidence=%.3f, Time=%lums", 
             result->speciesId, result->confidence, result->inferenceTime);

    return true;
}

bool InferenceEngine::analyzeBehavior(const float* imageSequence, uint8_t sequenceLength, SpeciesResult* result) {
    if (!initialized_ || !imageSequence || !result || sequenceLength == 0) {
        ESP_LOGE(TAG, "Invalid parameters for behavior analysis");
        return false;
    }

    uint32_t startTime = esp_timer_get_time() / 1000;

    // For now, analyze only the last frame (temporal analysis would require model changes)
    const float* lastFrame = imageSequence + (sequenceLength - 1) * INPUT_SIZE;
    
    InferenceResult inferenceResult = tfliteEngine_->runInference(&behaviorModel_, lastFrame, INPUT_SIZE);

    if (!inferenceResult.success) {
        ESP_LOGE(TAG, "Behavior analysis inference failed");
        return false;
    }

    // Process results
    result->behaviorId = inferenceResult.detectedClass;
    result->behaviorConfidence = inferenceResult.maxConfidence;
    result->inferenceTime = inferenceResult.inferenceTimeMs;
    result->valid = true;

    uint32_t endTime = esp_timer_get_time() / 1000;
    updatePerformanceStats(endTime - startTime, true);

    ESP_LOGD(TAG, "Behavior analysis: ID=%d, Confidence=%.3f, Time=%lums", 
             result->behaviorId, result->behaviorConfidence, result->inferenceTime);

    return true;
}

uint8_t InferenceEngine::detectMotion(const float* currentFrame, const float* previousFrame, 
                                     Detection* detections, uint8_t maxDetections) {
    if (!initialized_ || !currentFrame || !previousFrame || !detections) {
        ESP_LOGE(TAG, "Invalid parameters for motion detection");
        return 0;
    }

    uint32_t startTime = esp_timer_get_time() / 1000;

    // Simple motion detection - calculate frame difference
    float* frameDiff = (float*)heap_caps_malloc(INPUT_SIZE * sizeof(float), MALLOC_CAP_INTERNAL);
    if (!frameDiff) {
        ESP_LOGE(TAG, "Failed to allocate frame difference buffer");
        return 0;
    }

    // Calculate frame difference
    for (int i = 0; i < INPUT_SIZE; i++) {
        frameDiff[i] = fabs(currentFrame[i] - previousFrame[i]);
    }

    InferenceResult inferenceResult = tfliteEngine_->runInference(&motionModel_, frameDiff, INPUT_SIZE);
    heap_caps_free(frameDiff);

    if (!inferenceResult.success) {
        ESP_LOGE(TAG, "Motion detection inference failed");
        return 0;
    }

    // Create simple detection result
    uint8_t numDetections = (inferenceResult.maxConfidence > 0.5f) ? 1 : 0;
    
    if (numDetections > 0 && maxDetections > 0) {
        detections[0].classId = 0; // Motion class
        detections[0].confidence = inferenceResult.maxConfidence;
        detections[0].bbox[0] = 0.2f; // x
        detections[0].bbox[1] = 0.2f; // y  
        detections[0].bbox[2] = 0.6f; // width
        detections[0].bbox[3] = 0.6f; // height
        detections[0].timestamp = esp_timer_get_time() / 1000;
    }

    uint32_t endTime = esp_timer_get_time() / 1000;
    updatePerformanceStats(endTime - startTime, true);

    ESP_LOGD(TAG, "Motion detection: %d detections found", numDetections);
    return numDetections;
}

bool InferenceEngine::analyzeWildlife(const uint8_t* imageData, uint16_t width, uint16_t height, 
                                     uint8_t channels, SpeciesResult* result) {
    if (!initialized_ || !imageData || !result) {
        ESP_LOGE(TAG, "Invalid parameters for wildlife analysis");
        return false;
    }

    // Preprocess image
    if (!preprocessImage(imageData, width, height, channels, preprocessBuffer_)) {
        ESP_LOGE(TAG, "Image preprocessing failed");
        return false;
    }

    // Run species classification
    if (!classifySpecies(preprocessBuffer_, result)) {
        ESP_LOGE(TAG, "Species classification failed");
        return false;
    }

    // Run behavior analysis if species detected with high confidence
    if (result->confidence > 0.7f) {
        analyzeBehavior(preprocessBuffer_, 1, result);
    }

    return true;
}

void InferenceEngine::resizeImage(const uint8_t* input, uint16_t inputWidth, uint16_t inputHeight,
                                 float* output, uint16_t outputWidth, uint16_t outputHeight, uint8_t channels) {
    float scaleX = (float)inputWidth / outputWidth;
    float scaleY = (float)inputHeight / outputHeight;

    for (int y = 0; y < outputHeight; y++) {
        for (int x = 0; x < outputWidth; x++) {
            // Simple nearest neighbor interpolation
            int srcX = (int)(x * scaleX);
            int srcY = (int)(y * scaleY);
            
            // Clamp to input bounds
            srcX = (srcX >= inputWidth) ? inputWidth - 1 : srcX;
            srcY = (srcY >= inputHeight) ? inputHeight - 1 : srcY;

            for (int c = 0; c < channels; c++) {
                int srcIdx = (srcY * inputWidth + srcX) * channels + c;
                int dstIdx = (y * outputWidth + x) * channels + c;
                output[dstIdx] = (float)input[srcIdx];
            }
        }
    }
}

void InferenceEngine::normalizePixels(float* data, size_t size, const PreprocessingConfig& config) {
    if (config.normalizePixels) {
        // Normalize to 0-1 range
        for (size_t i = 0; i < size; i++) {
            data[i] /= 255.0f;
        }
    }

    if (config.subtractMean) {
        // Subtract mean and divide by std (per channel)
        for (int i = 0; i < INPUT_HEIGHT * INPUT_WIDTH; i++) {
            for (int c = 0; c < INPUT_CHANNELS; c++) {
                int idx = i * INPUT_CHANNELS + c;
                data[idx] = (data[idx] - config.meanRGB[c]) / config.stdRGB[c];
            }
        }
    }
}

PerformanceStats InferenceEngine::getPerformanceStats() const {
    return performanceStats_;
}

void InferenceEngine::resetPerformanceStats() {
    memset(&performanceStats_, 0, sizeof(performanceStats_));
}

size_t InferenceEngine::getMemoryUsage() const {
    size_t usage = 0;
    if (preprocessBuffer_) usage += INPUT_SIZE * sizeof(float);
    if (previousFrameBuffer_) usage += INPUT_SIZE * sizeof(float);
    if (tfliteEngine_) usage += tfliteEngine_->getMemoryUsage();
    return usage;
}

void InferenceEngine::updatePerformanceStats(uint32_t inferenceTime, bool success) {
    performanceStats_.totalInferences++;
    if (success) {
        performanceStats_.totalInferenceTime += inferenceTime;
        performanceStats_.averageInferenceTime = performanceStats_.totalInferenceTime / performanceStats_.totalInferences;
        
        if (inferenceTime > performanceStats_.maxInferenceTime) {
            performanceStats_.maxInferenceTime = inferenceTime;
        }
        
        if (performanceStats_.minInferenceTime == 0 || inferenceTime < performanceStats_.minInferenceTime) {
            performanceStats_.minInferenceTime = inferenceTime;
        }
    }
}