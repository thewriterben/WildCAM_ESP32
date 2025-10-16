/**
 * @file inference_engine.h
 * @brief Enhanced TinyML Inference Engine for ESP32 Wildlife Camera
 * 
 * Provides a unified interface for running TensorFlow Lite models
 * on ESP32 hardware with advanced optimizations for wildlife detection,
 * intelligent caching, and predictive model loading.
 */

#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include "../ai_common.h"
#include <vector>
#include <memory>
#include <set>

// Forward declarations for TensorFlow Lite
namespace tflite {
    class MicroInterpreter;
    class Model;
    class MicroOpResolver;
}

// Forward declarations for AI optimization classes
class ModelCache;
class ModelPredictor;

/**
 * Model Types supported by the inference engine
 */
enum class ModelType {
    SPECIES_CLASSIFIER,
    BEHAVIOR_ANALYZER,
    MOTION_DETECTOR,
    OBJECT_TRACKER,
    THREAT_DETECTOR
};

/**
 * Enhanced TinyML Inference Engine Class
 * 
 * Handles loading, optimizing, and running TensorFlow Lite models
 * optimized for ESP32 hardware constraints with intelligent caching
 * and predictive model management.
 */
class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    // Initialization and model management
    bool init();
    bool loadModel(const uint8_t* modelData, size_t modelSize, ModelType type);
    bool loadModelFromFile(const char* filename, ModelType type);
    void unloadModel(ModelType type);
    bool isModelLoaded(ModelType type) const;
    
    // Enhanced inference operations with caching
    AIResult runInference(const CameraFrame& frame, ModelType type);
    std::vector<AIResult> runMultipleInference(const CameraFrame& frame);
    
    // Predictive model management
    std::vector<ModelType> getPredictedModels(float temperature = 25.0f, float lightLevel = 0.5f);
    void preloadPredictedModels();
    float getPredictedAccuracy(ModelType type);
    
    // Cache management
    float getCacheHitRate() const;
    void clearCache();
    void printCacheStats();
    
    // Performance and optimization
    void enableQuantization(bool enable = true);
    void enablePowerOptimization(bool enable = true);
    void enableIntelligentCaching(bool enable = true);
    void setMemoryLimit(size_t maxMemoryBytes);
    AIMetrics getPerformanceMetrics() const;
    void resetMetrics();
    
    // Model information
    ModelInfo getModelInfo(ModelType type) const;
    std::vector<ModelType> getLoadedModels() const;
    size_t getTotalMemoryUsage() const;
    
    // Configuration
    void setConfidenceThreshold(float threshold);
    float getConfidenceThreshold() const;
    void setMaxInferenceTime(unsigned long maxTimeMs);
    
    // Progressive inference settings
    void enableEarlyExit(bool enable = true);
    void setEarlyExitThreshold(float threshold);
    
    // Environmental adaptation
    void updateEnvironmentalConditions(float temperature, float humidity, float lightLevel);
    void enableContextualConfidence(bool enable = true);
    
    // Debug and diagnostics
    bool testModel(ModelType type);
    void printModelDetails(ModelType type);
    bool validateModelIntegrity(ModelType type);
    void printSystemDiagnostics();

private:
    struct ModelContainer {
        std::unique_ptr<tflite::Model> model;
        std::unique_ptr<tflite::MicroInterpreter> interpreter;
        std::unique_ptr<tflite::MicroOpResolver> resolver;
        uint8_t* tensorArena;
        size_t arenaSize;
        ModelInfo info;
        bool isLoaded;
        unsigned long lastUsed;
        uint32_t usageCount;
        
        ModelContainer() : 
            tensorArena(nullptr), arenaSize(0), isLoaded(false),
            lastUsed(0), usageCount(0) {}
        ~ModelContainer();
    };

    // Model management
    std::vector<std::unique_ptr<ModelContainer>> models_;
    ModelContainer* getModelContainer(ModelType type);
    bool setupInterpreter(ModelContainer* container, const uint8_t* modelData, size_t modelSize);
    
    // Memory management
    size_t maxMemoryLimit_;
    size_t currentMemoryUsage_;
    bool allocateTensorArena(ModelContainer* container, size_t requiredSize);
    void deallocateTensorArena(ModelContainer* container);
    
    // Enhanced preprocessing and postprocessing
    bool preprocessImage(const CameraFrame& frame, float* inputTensor, ModelType type);
    bool preprocessImageEnhanced(const CameraFrame& frame, float* inputTensor, ModelType type);
    bool preprocessForMotionDetection(const CameraFrame& frame, float* inputTensor);
    bool preprocessForSpeciesClassification(const CameraFrame& frame, float* inputTensor);
    bool preprocessForBehaviorAnalysis(const CameraFrame& frame, float* inputTensor);
    
    AIResult postprocessOutput(const float* outputTensor, ModelType type, const CameraFrame& frame);
    AIResult postprocessOutputEnhanced(const float* outputTensor, ModelType type, const CameraFrame& frame);
    
    // Progressive inference
    float quickMotionCheck(const CameraFrame& frame);
    bool shouldUseEarlyExit(ModelType type, float preliminaryConfidence);
    
    // Context-aware processing
    float calculateContextualConfidence(const AIResult& result, const CameraFrame& frame);
    float calculateImageBrightness(const CameraFrame& frame);
    float getTimeOfDayFactor();
    
    // Intelligent caching and prediction
    std::unique_ptr<ModelCache> modelCache_;
    std::unique_ptr<ModelPredictor> modelPredictor_;
    bool intelligentCachingEnabled_;
    
    // Optimization features
    bool quantizationEnabled_;
    bool powerOptimizationEnabled_;
    bool earlyExitEnabled_;
    bool contextualConfidenceEnabled_;
    float confidenceThreshold_;
    float earlyExitThreshold_;
    unsigned long maxInferenceTime_;
    
    // Environmental conditions
    float currentTemperature_;
    float currentHumidity_;
    float currentLightLevel_;
    
    // Performance tracking
    mutable AIMetrics metrics_;
    void updateMetrics(float inferenceTime, bool success);
    
    // Model-specific helpers
    AIResult processSpeciesClassification(const float* output, const CameraFrame& frame);
    AIResult processBehaviorAnalysis(const float* output, const CameraFrame& frame);
    AIResult processMotionDetection(const float* output, const CameraFrame& frame);
    
    // Utility functions
    ModelType getModelTypeFromIndex(size_t index) const;
    size_t getModelIndex(ModelType type) const;
    bool validateModelFormat(const uint8_t* modelData, size_t modelSize);
    
    // Constants
    static const size_t MAX_MODELS = 5;
    static const size_t DEFAULT_ARENA_SIZE = 64 * 1024;  // 64KB
    static const size_t MAX_MEMORY_LIMIT = 512 * 1024;   // 512KB
    static const float DEFAULT_CONFIDENCE_THRESHOLD;
    static const float DEFAULT_EARLY_EXIT_THRESHOLD;
    static const unsigned long DEFAULT_MAX_INFERENCE_TIME;
};

// Utility functions for model management
const char* modelTypeToString(ModelType type);
ModelType stringToModelType(const char* typeStr);
size_t calculateRequiredArenaSize(const uint8_t* modelData, size_t modelSize);

#endif // INFERENCE_ENGINE_H