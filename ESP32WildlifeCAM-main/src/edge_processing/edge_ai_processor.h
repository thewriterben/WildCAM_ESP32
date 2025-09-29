/**
 * @file edge_ai_processor.h
 * @brief Foundational edge AI processing engine for wildlife monitoring
 * @version 1.0.0
 * 
 * This file provides the foundational framework for edge AI processing with
 * real-time wildlife identification. It integrates with the existing edge
 * processing system in src/ai/v2/ while providing a simplified interface
 * for basic edge computing operations.
 */

#ifndef EDGE_AI_PROCESSOR_H
#define EDGE_AI_PROCESSOR_H

#include <stdint.h>
#include <vector>
#include <memory>
#include <functional>

namespace EdgeProcessing {

/**
 * @brief Processing modes for edge AI
 */
enum class ProcessingMode {
    POWER_SAVER = 0,    // Minimal processing for battery life
    BALANCED,           // Balanced performance and power
    PERFORMANCE,        // Maximum accuracy, higher power
    REAL_TIME,          // Optimized for low latency
    CONTINUOUS          // Always-on processing
};

/**
 * @brief AI model types available for edge processing
 */
enum class ModelType {
    MOTION_DETECTION = 0,
    SPECIES_CLASSIFICATION,
    BEHAVIOR_ANALYSIS,
    SIZE_ESTIMATION,
    DANGER_DETECTION,
    CUSTOM_MODEL
};

/**
 * @brief Processing result status
 */
enum class ProcessingStatus {
    SUCCESS = 0,
    NO_DETECTION,
    LOW_CONFIDENCE,
    PROCESSING_ERROR,
    MODEL_NOT_LOADED,
    INSUFFICIENT_MEMORY,
    TIMEOUT
};

/**
 * @brief Edge processing result
 */
struct ProcessingResult {
    ProcessingStatus status;
    ModelType model_used;
    float confidence_score;     // 0.0-1.0
    uint32_t processing_time_ms;
    uint32_t timestamp;
    
    // Detection data
    struct Detection {
        uint16_t x, y, width, height;  // Bounding box
        const char* classification;    // Species or object type
        float confidence;             // Individual detection confidence
        float size_estimate;          // Relative size 0.0-1.0
        bool is_moving;              // Motion detected
    };
    
    std::vector<Detection> detections;
    
    // Metadata
    uint16_t frame_width, frame_height;
    uint32_t frame_size;
    uint8_t battery_impact;     // 0-100, estimated battery usage
};

/**
 * @brief Edge processor configuration
 */
struct EdgeProcessorConfig {
    ProcessingMode mode = ProcessingMode::BALANCED;
    float confidence_threshold = 0.6f;
    uint32_t max_processing_time_ms = 2000;
    uint32_t processing_interval_ms = 1000;
    bool enable_power_optimization = true;
    bool enable_parallel_processing = false;
    uint8_t max_concurrent_models = 2;
    uint32_t memory_limit_kb = 512;
    
    // Model-specific settings
    bool enable_motion_detection = true;
    bool enable_species_classification = true;
    bool enable_behavior_analysis = false;
    bool enable_size_estimation = true;
    bool enable_danger_detection = true;
};

/**
 * @brief Performance metrics for edge processing
 */
struct PerformanceMetrics {
    uint32_t total_frames_processed = 0;
    uint32_t successful_detections = 0;
    uint32_t failed_processing = 0;
    float average_processing_time_ms = 0.0f;
    float average_confidence = 0.0f;
    uint32_t memory_usage_kb = 0;
    uint32_t power_consumption_mw = 0;
    
    // Model-specific metrics
    uint32_t motion_detections = 0;
    uint32_t species_classifications = 0;
    uint32_t behavior_analyses = 0;
    uint32_t danger_alerts = 0;
};

/**
 * @brief Callback function types
 */
using ProcessingCompleteCallback = std::function<void(const ProcessingResult&)>;
using ModelLoadedCallback = std::function<void(ModelType, bool success)>;
using ErrorCallback = std::function<void(const char* error_message)>;
using PowerOptimizationCallback = std::function<void(float battery_level, ProcessingMode recommended_mode)>;

/**
 * @brief Main edge AI processing class
 * 
 * Provides foundational edge AI processing capabilities optimized for
 * low power consumption and minimal latency on ESP32 hardware.
 */
class EdgeAIProcessor {
public:
    EdgeAIProcessor();
    ~EdgeAIProcessor();

    /**
     * @brief Initialize edge processor with configuration
     * @param config Processor configuration
     * @return true if initialization successful
     */
    bool initialize(const EdgeProcessorConfig& config);

    /**
     * @brief Load AI model for edge processing
     * @param model_type Type of model to load
     * @param model_data Binary model data (TensorFlow Lite format)
     * @param model_size Size of model data
     * @return true if model loaded successfully
     */
    bool loadModel(ModelType model_type, const uint8_t* model_data, size_t model_size);

    /**
     * @brief Unload specific model to free memory
     * @param model_type Type of model to unload
     */
    void unloadModel(ModelType model_type);

    /**
     * @brief Process camera frame for wildlife detection and classification
     * @param frame_data Raw image data
     * @param frame_size Size of image data in bytes
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @return Processing result with detections
     */
    ProcessingResult processFrame(const uint8_t* frame_data, 
                                size_t frame_size,
                                uint16_t width, 
                                uint16_t height);

    /**
     * @brief Process frame asynchronously (non-blocking)
     * @param frame_data Raw image data
     * @param frame_size Size of image data
     * @param width Image width
     * @param height Image height
     * @param callback Callback when processing complete
     * @return true if processing started successfully
     */
    bool processFrameAsync(const uint8_t* frame_data, 
                          size_t frame_size,
                          uint16_t width, 
                          uint16_t height,
                          ProcessingCompleteCallback callback);

    /**
     * @brief Update processor configuration
     * @param config New configuration
     */
    void updateConfig(const EdgeProcessorConfig& config);

    /**
     * @brief Get current configuration
     * @return Current processor configuration
     */
    EdgeProcessorConfig getConfig() const { return config_; }

    /**
     * @brief Get performance metrics
     * @return Current performance metrics
     */
    PerformanceMetrics getPerformanceMetrics() const { return metrics_; }

    /**
     * @brief Reset performance metrics
     */
    void resetPerformanceMetrics();

    /**
     * @brief Set callback functions
     */
    void setProcessingCompleteCallback(ProcessingCompleteCallback callback) { 
        processing_complete_callback_ = callback; 
    }
    void setModelLoadedCallback(ModelLoadedCallback callback) { 
        model_loaded_callback_ = callback; 
    }
    void setErrorCallback(ErrorCallback callback) { 
        error_callback_ = callback; 
    }
    void setPowerOptimizationCallback(PowerOptimizationCallback callback) { 
        power_optimization_callback_ = callback; 
    }

    /**
     * @brief Check if processor is initialized and ready
     */
    bool isReady() const { return initialized_; }

    /**
     * @brief Check if specific model is loaded
     * @param model_type Model type to check
     * @return true if model is loaded and ready
     */
    bool isModelLoaded(ModelType model_type) const;

    /**
     * @brief Get available memory for model loading
     * @return Available memory in KB
     */
    uint32_t getAvailableMemory() const;

    /**
     * @brief Optimize processing based on battery level
     * @param battery_level Current battery level (0.0-1.0)
     */
    void optimizeForBattery(float battery_level);

    /**
     * @brief Get recommended processing mode based on current conditions
     * @param battery_level Current battery level
     * @param processing_load Current processing load (0.0-1.0)
     * @return Recommended processing mode
     */
    ProcessingMode getRecommendedMode(float battery_level, float processing_load) const;

private:
    EdgeProcessorConfig config_;
    PerformanceMetrics metrics_;
    bool initialized_;
    
    // Model management
    struct LoadedModel {
        ModelType type;
        std::unique_ptr<uint8_t[]> model_data;
        size_t model_size;
        void* interpreter;  // TensorFlow Lite interpreter
        bool is_ready;
        uint32_t memory_usage_kb;
    };
    
    std::vector<LoadedModel> loaded_models_;
    uint32_t total_memory_used_kb_;
    
    // Processing state
    bool processing_active_;
    uint32_t last_processing_time_;
    
    // Callback functions
    ProcessingCompleteCallback processing_complete_callback_;
    ModelLoadedCallback model_loaded_callback_;
    ErrorCallback error_callback_;
    PowerOptimizationCallback power_optimization_callback_;
    
    // Internal processing methods
    bool preprocessImage(const uint8_t* frame_data, size_t frame_size, 
                        uint16_t width, uint16_t height);
    ProcessingResult runInference(ModelType model_type, const uint8_t* preprocessed_data);
    void postprocessResults(ProcessingResult& result);
    void updateMetrics(const ProcessingResult& result);
    
    // Model management
    LoadedModel* findLoadedModel(ModelType model_type);
    bool validateModelData(const uint8_t* model_data, size_t model_size);
    void optimizeModelExecution(ModelType model_type);
    
    // Power optimization
    void adjustProcessingMode(float battery_level);
    uint32_t estimatePowerConsumption(ProcessingMode mode) const;
    
    // Memory management
    bool checkMemoryAvailable(size_t required_kb);
    void garbageCollectModels();
};

/**
 * @brief Utility functions for edge processing
 */
namespace Utils {
    /**
     * @brief Convert processing mode to string
     */
    const char* processingModeToString(ProcessingMode mode);
    
    /**
     * @brief Convert model type to string
     */
    const char* modelTypeToString(ModelType type);
    
    /**
     * @brief Convert processing status to string
     */
    const char* processingStatusToString(ProcessingStatus status);
    
    /**
     * @brief Calculate detection accuracy score
     */
    float calculateAccuracyScore(const std::vector<ProcessingResult::Detection>& detections);
    
    /**
     * @brief Estimate processing complexity for given configuration
     */
    float estimateProcessingComplexity(const EdgeProcessorConfig& config);
}

} // namespace EdgeProcessing

#endif // EDGE_AI_PROCESSOR_H