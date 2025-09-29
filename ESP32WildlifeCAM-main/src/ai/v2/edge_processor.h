/**
 * @file edge_processor.h
 * @brief Optimized Edge AI Processing Engine Header
 * 
 * Defines the enhanced edge AI processing system for real-time wildlife
 * detection with 95%+ accuracy and power optimization features.
 */

#ifndef EDGE_PROCESSOR_H
#define EDGE_PROCESSOR_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include <functional>

// TensorFlow Lite includes
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Forward declarations
class EdgeAIPerformanceMonitor;

// ===========================
// CONSTANTS
// ===========================

#define EDGE_TENSOR_ARENA_SIZE          200000  // 200KB for model operations
#define EDGE_MAX_DETECTION_QUEUE        10      // Maximum detections in queue
#define EDGE_DEFAULT_INFERENCE_INTERVAL 1000    // Default inference interval (1 sec)
#define EDGE_DEFAULT_CONFIDENCE_THRESHOLD 0.7   // Default confidence threshold

// ===========================
// ENUMERATIONS
// ===========================

/**
 * @brief Image format types
 */
typedef enum {
    IMAGE_FORMAT_RGB888 = 0,        // 24-bit RGB
    IMAGE_FORMAT_RGB565 = 1,        // 16-bit RGB
    IMAGE_FORMAT_JPEG = 2,          // JPEG compressed
    IMAGE_FORMAT_GRAYSCALE = 3      // 8-bit grayscale
} ImageFormat;

/**
 * @brief Error codes for edge processing
 */
typedef enum {
    EDGE_ERROR_NONE = 0,            // No error
    EDGE_ERROR_NOT_INITIALIZED = 1, // Processor not initialized
    EDGE_ERROR_MODEL_NOT_LOADED = 2, // AI model not loaded
    EDGE_ERROR_PREPROCESSING_FAILED = 3, // Image preprocessing failed
    EDGE_ERROR_INFERENCE_FAILED = 4, // Inference execution failed
    EDGE_ERROR_MEMORY_ALLOCATION = 5, // Memory allocation failed
    EDGE_ERROR_INVALID_INPUT = 6    // Invalid input parameters
} EdgeProcessorError;

// ===========================
// DATA STRUCTURES
// ===========================

/**
 * @brief Bounding box coordinates
 */
struct BoundingBox {
    uint16_t x;                     // Top-left x coordinate
    uint16_t y;                     // Top-left y coordinate
    uint16_t width;                 // Bounding box width
    uint16_t height;                // Bounding box height
};

/**
 * @brief Wildlife detection result
 */
struct WildlifeDetectionResult {
    BoundingBox boundingBox;        // Detection bounding box
    float confidence;               // Detection confidence (0.0-1.0)
    uint16_t speciesId;             // Species identifier
    float speciesConfidence;        // Species classification confidence
    uint8_t behaviorClass;          // Behavior classification
    float behaviorConfidence;       // Behavior confidence
    unsigned long timestamp;        // Detection timestamp
    uint32_t frameId;               // Frame sequence number
    EdgeProcessorError error;       // Error status
    
    // Extended wildlife-specific data
    float environmentalScore;       // Environmental context score
    bool isMoving;                  // Movement detection flag
    float sizeScore;                // Size reasonableness score
    uint8_t qualityFlags;           // Detection quality flags
};

/**
 * @brief Edge processor configuration
 */
struct EdgeProcessorConfig {
    uint16_t inputWidth;            // Model input width
    uint16_t inputHeight;           // Model input height
    float confidenceThreshold;      // Minimum confidence threshold
    uint32_t inferenceInterval;     // Time between inferences (ms)
    bool powerOptimizationEnabled;  // Power optimization flag
    bool temporalFilteringEnabled;  // Temporal filtering flag
    bool environmentalContextEnabled; // Environmental context flag
    uint8_t maxDetectionsPerFrame;  // Maximum detections per frame
    
    // Wildlife-specific parameters
    float minWildlifeSize;          // Minimum wildlife size ratio
    float maxWildlifeSize;          // Maximum wildlife size ratio
    bool speciesClassificationEnabled; // Species ID flag
    bool behaviorAnalysisEnabled;   // Behavior analysis flag
};

/**
 * @brief Processing statistics
 */
struct EdgeProcessorStatistics {
    unsigned long startTime;        // System start time
    uint32_t totalInferences;       // Total inferences performed
    uint32_t wildlifeDetections;    // Total wildlife detections
    uint32_t falsePositives;        // Estimated false positives
    uint32_t inferenceErrors;       // Inference execution errors
    float averageInferenceTime;     // Average inference time (ms)
    float frameRate;                // Current frame rate (fps)
    float accuracyScore;            // Estimated accuracy score
    uint32_t memoryUsage;           // Current memory usage (bytes)
    
    // Wildlife-specific statistics
    uint32_t speciesClassified;     // Total species classifications
    uint32_t behaviorsAnalyzed;     // Total behavior analyses
    float wildlifeAccuracy;         // Wildlife detection accuracy
    uint32_t temporalFiltered;      // Detections filtered by temporal logic
};

// ===========================
// CALLBACK TYPES
// ===========================

typedef std::function<void(const WildlifeDetectionResult&)> DetectionCallback;
typedef std::function<void(const EdgeProcessorStatistics&)> StatisticsCallback;

// ===========================
// EDGE PROCESSOR CLASS
// ===========================

/**
 * @brief Optimized edge AI processing engine for wildlife monitoring
 * 
 * Implements real-time wildlife detection, species identification, and
 * behavior analysis with power optimization and high accuracy features.
 */
class EdgeProcessor {
public:
    // Constructor and destructor
    EdgeProcessor();
    ~EdgeProcessor();
    
    // Initialization and configuration
    bool initialize(const EdgeProcessorConfig& config);
    void cleanup();
    
    // Main processing methods
    void processFrame();
    WildlifeDetectionResult processImage(const uint8_t* imageData, 
                                       size_t imageSize, 
                                       ImageFormat format);
    
    // Model management
    bool loadAIModels();
    bool reloadModel(const char* modelPath);
    bool isModelLoaded() const { return modelLoaded_; }
    
    // Configuration
    void setConfig(const EdgeProcessorConfig& config) { config_ = config; }
    EdgeProcessorConfig getConfig() const { return config_; }
    void setConfidenceThreshold(float threshold) { config_.confidenceThreshold = threshold; }
    void setInferenceInterval(uint32_t interval) { config_.inferenceInterval = interval; }
    
    // Power management
    void enablePowerOptimization(bool enable);
    void adjustInferenceFrequency(float batteryLevel);
    void enableLowPowerMode();
    void disableLowPowerMode();
    
    // Statistics and monitoring
    EdgeProcessorStatistics getStatistics() const;
    void resetStatistics();
    float getAverageInferenceTime() const;
    float getAccuracyScore() const;
    uint32_t getFramesProcessed() const;
    
    // Callbacks
    void setDetectionCallback(DetectionCallback callback) { detectionCallback_ = callback; }
    void setStatisticsCallback(StatisticsCallback callback) { statisticsCallback_ = callback; }
    
    // Wildlife-specific methods
    void enableSpeciesClassification(bool enable) { config_.speciesClassificationEnabled = enable; }
    void enableBehaviorAnalysis(bool enable) { config_.behaviorAnalysisEnabled = enable; }
    // Enhanced edge AI optimization features
    void enableMultiCoreProcessing(bool enable);
    void enableSIMDOptimizations(bool enable);
    void enablePipelineParallelism(bool enable);
    void enableBatchProcessing(bool enable);
    void setMemoryPoolSize(size_t pool_size);
    
    // Dynamic model selection and optimization
    void enableDynamicModelSelection(bool enable);
    bool loadModelVariant(const String& variant_name, const uint8_t* model_data, size_t model_size);
    void selectOptimalModel(float battery_level, float performance_requirement, 
                          const String& environmental_conditions);
    
    // Edge learning and adaptation
    void enableEdgeLearning(bool enable);
    void addTrainingSample(const uint8_t* image_data, const String& ground_truth_label);
    bool performLocalModelAdaptation();
    void shareModelUpdates(); // For federated learning
    
    // Multi-modal processing
    bool processMultiModal(const uint8_t* image_data, const float* audio_features,
                         const float* sensor_data, WildlifeDetectionResult& result);
    void enableMultiModalFusion(bool enable);
    
    // Behavioral pattern recognition
    bool detectBehaviorPattern(const std::vector<WildlifeDetectionResult>& recent_detections,
                             String& detected_pattern);
    void enableBehaviorTracking(bool enable);
    
    // Population counting
    bool estimatePopulationCount(const WildlifeDetectionResult& detection,
                               uint8_t& estimated_count, float& confidence);
    void enablePopulationCounting(bool enable);
    
    // Environmental context integration
    void updateEnvironmentalContext(float temperature, float humidity, float light_level,
                                  uint8_t time_of_day, uint8_t season);
    void enableEnvironmentalAdaptation(bool enable);
    
    // Real-time performance monitoring
    void enablePerformanceMonitoring(bool enable);
    void setPerformanceThresholds(float min_fps, float max_inference_time_ms,
                                float min_accuracy_percent);
    std::vector<String> getPerformanceAlerts();
    float getSystemHealthScore();
    
    // Confidence-based processing
    void enableConfidenceBasedProcessing(bool enable);
    void setConfidenceThresholds(float detection_threshold, float species_threshold,
                               float behavior_threshold);
    
    // Temporal consistency
    void enableTemporalConsistency(bool enable);
    void setTemporalWindow(uint8_t frame_count);
    String applyTemporalSmoothing(const String& current_prediction, float confidence);
    
    // Advanced features
    std::vector<WildlifeDetectionResult> getDetectionHistory() const { return detectionQueue_; }
    void clearDetectionHistory() { detectionQueue_.clear(); }
    bool isProcessingActive() const { return processingActive_; }

private:
    // Configuration and state
    EdgeProcessorConfig config_;
    bool initialized_;
    bool modelLoaded_;
    bool processingActive_;
    bool powerOptimizationEnabled_;
    
    // Enhanced edge AI features
    bool multi_core_enabled_;
    bool simd_enabled_;
    bool pipeline_enabled_;
    bool batch_processing_enabled_;
    bool dynamic_model_selection_enabled_;
    bool edge_learning_enabled_;
    bool multi_modal_enabled_;
    bool behavior_tracking_enabled_;
    bool population_counting_enabled_;
    bool environmental_adaptation_enabled_;
    bool performance_monitoring_enabled_;
    bool confidence_based_processing_enabled_;
    bool temporal_consistency_enabled_;
    
    // Timing and frame tracking
    unsigned long lastInference_;
    uint32_t frameCount_;
    
    // TensorFlow Lite components
    const tflite::Model* model_;
    std::unique_ptr<tflite::MicroInterpreter> interpreter_;
    tflite::ErrorReporter* errorReporter_;
    uint8_t* tensorArena_;
    size_t tensor_arena_size_;
    
    // Memory pool management
    struct MemoryPool {
        uint8_t* pool_data;
        size_t pool_size;
        std::vector<bool> block_allocated;
        size_t block_size;
        size_t total_blocks;
    } memory_pool_;
    
    // Model variants for dynamic selection
    struct ModelVariant {
        String name;
        const uint8_t* model_data;
        size_t model_size;
        float accuracy_rating;
        float speed_rating;
        float power_rating;
    };
    std::vector<ModelVariant> loaded_models_;
    String current_model_variant_;
    
    // Edge learning data
    struct TrainingSample {
        std::vector<float> features;
        String label;
        float confidence;
        unsigned long timestamp;
    };
    std::vector<TrainingSample> training_samples_;
    static const size_t MAX_TRAINING_SAMPLES = 1000;
    
    // Temporal consistency
    struct TemporalFrame {
        String prediction;
        float confidence;
        std::vector<float> features;
        unsigned long timestamp;
    };
    std::vector<TemporalFrame> temporal_history_;
    uint8_t temporal_window_size_;
    
    // Environmental context
    struct EnvironmentalContext {
        float temperature;
        float humidity;
        float light_level;
        uint8_t time_of_day;
        uint8_t season;
        unsigned long last_update;
    } environmental_context_;
    
    // Performance monitoring
    class EdgeAIPerformanceMonitor* performance_monitor_;
    
    // Multi-modal processing
    struct MultiModalData {
        std::vector<float> visual_features;
        std::vector<float> audio_features;
        std::vector<float> sensor_features;
        unsigned long timestamp;
    } current_multimodal_data_;
    
    // Behavioral pattern tracking
    struct BehaviorPattern {
        String pattern_name;
        std::vector<String> sequence;
        float confidence;
        uint32_t occurrence_count;
    };
    std::vector<BehaviorPattern> detected_patterns_;
    
    // Image processing
    float* preprocessedImage_;
    
    // Detection results
    std::vector<WildlifeDetectionResult> detectionQueue_;
    
    // Statistics
    EdgeProcessorStatistics statistics_;
    
    // Callbacks
    DetectionCallback detectionCallback_;
    StatisticsCallback statisticsCallback_;
    
    // External model data (would be loaded from flash/SD)
    extern const unsigned char wildlife_detection_model_data[];
    
    // Enhanced private methods - Multi-core and SIMD processing
    bool initializeMultiCoreProcessing();
    bool initializeSIMDProcessing();
    void processFrameMultiCore();
    void processFrameSIMD();
    static void inferenceTask(void* parameters);
    
    // Enhanced private methods - Memory pool management
    bool initializeMemoryPool(size_t pool_size);
    void cleanupMemoryPool();
    uint8_t* allocateFromPool(size_t size);
    void releaseToPool(uint8_t* ptr);
    
    // Enhanced private methods - Dynamic model management
    bool switchToModel(const String& model_name);
    ModelVariant* findOptimalModel(float battery_level, float performance_requirement);
    void evaluateModelPerformance(const String& model_name, float accuracy, float speed, float power);
    
    // Enhanced private methods - Edge learning
    void extractFeaturesForLearning(const uint8_t* image_data, std::vector<float>& features);
    bool shouldTriggerAdaptation();
    void cleanupOldTrainingSamples();
    
    // Enhanced private methods - Multi-modal fusion
    std::vector<float> fuseMultiModalFeatures(const std::vector<float>& visual,
                                            const std::vector<float>& audio,
                                            const std::vector<float>& sensor);
    
    // Enhanced private methods - Behavioral pattern detection
    void updateBehaviorHistory(const WildlifeDetectionResult& result);
    bool matchBehaviorPattern(const std::vector<String>& recent_behaviors,
                            BehaviorPattern& matched_pattern);
    
    // Enhanced private methods - Population counting
    std::vector<BoundingBox> detectIndividuals(const WildlifeDetectionResult& result);
    uint8_t countNonOverlappingDetections(const std::vector<BoundingBox>& detections);
    
    // Enhanced private methods - Environmental adaptation
    void applyEnvironmentalFiltering(WildlifeDetectionResult& result);
    float calculateEnvironmentalWeight(const String& species);
    
    // Enhanced private methods - Temporal consistency
    void updateTemporalHistory(const String& prediction, float confidence,
                             const std::vector<float>& features);
    String getTemporalConsensus();
    float calculateTemporalConfidence();
    
    // Enhanced private methods - Confidence-based processing
    bool shouldSkipInference(float motion_confidence);
    bool shouldProcessFullPipeline(float detection_confidence);
    
    // Enhanced private methods - Performance optimization
    void optimizeBasedOnPerformance();
    void adjustProcessingParameters();
    bool checkPerformanceThresholds();
    
    // Private methods - Initialization
    bool initializeTensorFlowLite();
    bool initializeImageProcessing();
    
    // Private methods - Model management
    bool loadWildlifeDetectionModel();
    bool loadSpeciesClassificationModel();
    bool loadBehaviorAnalysisModel();
    void addOperationsToResolver(tflite::MicroMutableOpResolver<10>& resolver);
    bool verifyModelTensors();
    
    // Private methods - Image preprocessing
    bool captureAndPreprocessFrame();
    bool preprocessImage(const uint8_t* imageData, size_t imageSize, ImageFormat format);
    bool preprocessCurrentFrame();
    bool convertImageFormat(const uint8_t* imageData, size_t imageSize, ImageFormat format);
    bool resizeImage();
    bool normalizeImage();
    bool applyWildlifePreprocessing();
    
    // Private methods - Inference
    WildlifeDetectionResult runInference();
    void extractInferenceResults(WildlifeDetectionResult& result);
    WildlifeDetectionResult optimizeForWildlife(const WildlifeDetectionResult& result);
    WildlifeDetectionResult applyAdvancedFiltering(const WildlifeDetectionResult& result);
    
    // Private methods - Image processing helpers
    bool convertRGB565ToRGB888(const uint8_t* rgb565Data, size_t dataSize);
    bool convertJPEGToRGB888(const uint8_t* jpegData, size_t dataSize);
    bool enhanceContrast();
    bool reduceNoise();
    bool enhanceEdges();
    
    // Private methods - Wildlife-specific filtering
    WildlifeDetectionResult applyTemporalFiltering(const WildlifeDetectionResult& result);
    WildlifeDetectionResult applySizeFiltering(const WildlifeDetectionResult& result);
    WildlifeDetectionResult applyMovementFiltering(const WildlifeDetectionResult& result);
    WildlifeDetectionResult applyEnvironmentalContext(const WildlifeDetectionResult& result);
    
    // Private methods - Validation and quality checks
    bool isConsistentWithPreviousDetections(const WildlifeDetectionResult& result);
    bool isSizeReasonable(const BoundingBox& bbox);
    bool isNearImageEdge(const BoundingBox& bbox);
    uint16_t findMaxConfidenceClass(float* data, int numClasses);
    
    // Private methods - Utilities
    void processDetectionResult(const WildlifeDetectionResult& result);
    void updateInferenceStatistics(unsigned long inferenceTime);
    uint32_t calculateMemoryUsage();
};

// ===========================
// INLINE IMPLEMENTATIONS
// ===========================

inline bool EdgeProcessor::isModelLoaded() const {
    return modelLoaded_;
}

inline void EdgeProcessor::setConfig(const EdgeProcessorConfig& config) {
    config_ = config;
}

inline EdgeProcessorConfig EdgeProcessor::getConfig() const {
    return config_;
}

inline void EdgeProcessor::setConfidenceThreshold(float threshold) {
    config_.confidenceThreshold = threshold;
}

inline void EdgeProcessor::setInferenceInterval(uint32_t interval) {
    config_.inferenceInterval = interval;
}

inline void EdgeProcessor::setDetectionCallback(DetectionCallback callback) {
    detectionCallback_ = callback;
}

inline void EdgeProcessor::setStatisticsCallback(StatisticsCallback callback) {
    statisticsCallback_ = callback;
}

inline void EdgeProcessor::enableSpeciesClassification(bool enable) {
    config_.speciesClassificationEnabled = enable;
}

inline void EdgeProcessor::enableBehaviorAnalysis(bool enable) {
    config_.behaviorAnalysisEnabled = enable;
}

inline void EdgeProcessor::enableTemporalFiltering(bool enable) {
    config_.temporalFilteringEnabled = enable;
}

inline std::vector<WildlifeDetectionResult> EdgeProcessor::getDetectionHistory() const {
    return detectionQueue_;
}

inline void EdgeProcessor::clearDetectionHistory() {
    detectionQueue_.clear();
}

inline bool EdgeProcessor::isProcessingActive() const {
    return processingActive_;
}

#endif // EDGE_PROCESSOR_H