/**
 * @file tensorflow_lite_implementation.h
 * @brief Complete TensorFlow Lite Micro integration for ESP32 Wildlife Camera
 * 
 * This file provides a high-level interface for wildlife detection using
 * TensorFlow Lite Micro, integrating with the existing camera and AI systems.
 * 
 * Phase 1 Core AI Enhancement Integration
 */

#ifndef TENSORFLOW_LITE_IMPLEMENTATION_H
#define TENSORFLOW_LITE_IMPLEMENTATION_H

#include "ai_common.h"
#include "tinyml/tensorflow_lite_micro.h"

// TensorFlow Lite Micro includes (conditional compilation)
#ifdef TFLITE_MICRO_ENABLED
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#endif

/**
 * Wildlife Detection Model Types
 */
enum WildlifeModelType {
    MODEL_SPECIES_CLASSIFIER,
    MODEL_BEHAVIOR_ANALYZER,
    MODEL_MOTION_DETECTOR,
    MODEL_HUMAN_DETECTOR,
    MODEL_COUNT
};

/**
 * Model Information Structure
 */
struct ModelInfo {
    WildlifeModelType type;
    String name;
    String filename;
    size_t size;
    uint32_t version;
    bool loaded;
    float accuracy;
    uint32_t inputWidth;
    uint32_t inputHeight;
    uint32_t channels;
    
    ModelInfo() : type(MODEL_SPECIES_CLASSIFIER), name(""), filename(""), 
                  size(0), version(0), loaded(false), accuracy(0.0f),
                  inputWidth(224), inputHeight(224), channels(3) {}
};

/**
 * Inference Result Structure
 */
struct InferenceResult {
    float confidence;
    String className;
    uint32_t classIndex;
    uint32_t inferenceTime;
    bool valid;
    
    InferenceResult() : confidence(0.0f), className(""), classIndex(0), 
                       inferenceTime(0), valid(false) {}
};

/**
 * TensorFlow Lite Implementation Manager
 * 
 * Provides high-level interface for wildlife detection using TensorFlow Lite Micro
 */
class TensorFlowLiteImplementation {
public:
    TensorFlowLiteImplementation();
    ~TensorFlowLiteImplementation();
    
    // Initialization and cleanup
    bool init();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Model management
    bool loadModel(WildlifeModelType type, const char* modelPath);
    bool loadModelsFromDirectory(const char* directory);
    bool unloadModel(WildlifeModelType type);
    void unloadAllModels();
    bool isModelLoaded(WildlifeModelType type) const;
    ModelInfo getModelInfo(WildlifeModelType type) const;
    std::vector<ModelInfo> getLoadedModels() const;
    
    // Inference operations
    InferenceResult runInference(WildlifeModelType type, const uint8_t* imageData, 
                                uint32_t width, uint32_t height, uint32_t channels = 3);
    InferenceResult runInference(WildlifeModelType type, const CameraFrame& frame);
    
    // Batch inference for multiple models
    std::vector<InferenceResult> runMultiModelInference(const uint8_t* imageData,
                                                       uint32_t width, uint32_t height);
    
    // Image preprocessing
    bool preprocessImage(const uint8_t* inputImage, uint32_t inputWidth, uint32_t inputHeight,
                        uint8_t* outputImage, uint32_t outputWidth, uint32_t outputHeight);
    bool resizeImage(const uint8_t* input, uint32_t inputW, uint32_t inputH,
                    uint8_t* output, uint32_t outputW, uint32_t outputH);
    bool normalizeImage(uint8_t* imageData, uint32_t width, uint32_t height);
    
    // Performance monitoring
    void enablePerformanceMonitoring(bool enable) { performanceMonitoring_ = enable; }
    uint32_t getAverageInferenceTime(WildlifeModelType type) const;
    float getModelAccuracy(WildlifeModelType type) const;
    size_t getMemoryUsage() const;
    
    // Configuration
    void setConfidenceThreshold(float threshold) { confidenceThreshold_ = threshold; }
    float getConfidenceThreshold() const { return confidenceThreshold_; }
    void setMaxInferenceTime(uint32_t timeMs) { maxInferenceTime_ = timeMs; }
    uint32_t getMaxInferenceTime() const { return maxInferenceTime_; }
    
    // Wildlife-specific methods
    InferenceResult detectSpecies(const CameraFrame& frame);
    InferenceResult analyzeBehavior(const CameraFrame& frame);
    InferenceResult detectMotion(const CameraFrame& frame);
    InferenceResult detectHuman(const CameraFrame& frame);
    
    // Model validation
    bool validateModel(const char* modelPath);
    bool benchmarkModel(WildlifeModelType type, uint32_t iterations = 10);
    
private:
#ifdef TFLITE_MICRO_ENABLED
    // TensorFlow Lite Micro components
    struct ModelComponents {
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
        tflite::MicroMutableOpResolver<10>* resolver;
        uint8_t* tensorArena;
        size_t arenaSize;
        TfLiteTensor* inputTensor;
        TfLiteTensor* outputTensor;
        
        ModelComponents() : model(nullptr), interpreter(nullptr), resolver(nullptr),
                           tensorArena(nullptr), arenaSize(0), inputTensor(nullptr), outputTensor(nullptr) {}
    };
    
    ModelComponents models_[MODEL_COUNT];
#endif
    
    // State variables
    bool initialized_;
    float confidenceThreshold_;
    uint32_t maxInferenceTime_;
    bool performanceMonitoring_;
    
    // Performance tracking
    struct PerformanceMetrics {
        uint32_t totalInferences;
        uint32_t totalTime;
        uint32_t minTime;
        uint32_t maxTime;
        
        PerformanceMetrics() : totalInferences(0), totalTime(0), minTime(UINT32_MAX), maxTime(0) {}
    };
    
    PerformanceMetrics performance_[MODEL_COUNT];
    
    // Model information
    ModelInfo modelInfo_[MODEL_COUNT];
    
    // Private methods
    bool initializeModel(WildlifeModelType type, const uint8_t* modelData, size_t modelSize);
    void cleanupModel(WildlifeModelType type);
    bool prepareInputTensor(WildlifeModelType type, const uint8_t* imageData, 
                           uint32_t width, uint32_t height, uint32_t channels);
    InferenceResult processOutputTensor(WildlifeModelType type);
    void updatePerformanceMetrics(WildlifeModelType type, uint32_t inferenceTime);
    
    // Constants
    static const size_t DEFAULT_ARENA_SIZE = 512 * 1024; // 512KB
    static const float DEFAULT_CONFIDENCE_THRESHOLD = 0.7f;
    static const uint32_t DEFAULT_MAX_INFERENCE_TIME = 5000; // 5 seconds
};

// Global instance
extern TensorFlowLiteImplementation* g_tensorflowImplementation;

// Utility functions
bool initializeTensorFlowLite();
void cleanupTensorFlowLite();
bool loadWildlifeModels(const char* modelsDirectory);

#endif // TENSORFLOW_LITE_IMPLEMENTATION_H