/**
 * @file inference_engine.h
 * @brief Optimized inference engine for ESP32 wildlife classification
 * 
 * Provides efficient TensorFlow Lite inference with memory optimization,
 * preprocessing pipeline, and real-time performance for wildlife detection.
 */

#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include "tensorflow_lite_esp32.h"
#include "model_manager.h"
#include <stdint.h>
#include <stddef.h>

// Input image specifications
#define INPUT_WIDTH 224
#define INPUT_HEIGHT 224
#define INPUT_CHANNELS 3
#define INPUT_SIZE (INPUT_WIDTH * INPUT_HEIGHT * INPUT_CHANNELS)

// Output specifications
#define MAX_DETECTIONS 10
#define MAX_SPECIES_CLASSES 50
#define MAX_BEHAVIOR_CLASSES 12

/**
 * Preprocessing options
 */
typedef struct {
    bool normalizePixels;     // Normalize pixel values to 0-1 range
    bool subtractMean;        // Subtract ImageNet mean
    bool resizeImage;         // Resize to model input size
    float meanRGB[3];         // RGB mean values for normalization
    float stdRGB[3];          // RGB standard deviation values
} PreprocessingConfig;

/**
 * Detection result structure
 */
typedef struct {
    uint8_t classId;          // Detected class ID
    float confidence;         // Detection confidence (0-1)
    float bbox[4];            // Bounding box [x, y, width, height] (normalized)
    uint32_t timestamp;       // Detection timestamp
} Detection;

/**
 * Species classification result
 */
typedef struct {
    uint8_t speciesId;        // Species class ID
    float confidence;         // Classification confidence
    char speciesName[32];     // Species name
    uint8_t behaviorId;       // Detected behavior ID
    float behaviorConfidence; // Behavior confidence
    uint32_t inferenceTime;   // Inference time in milliseconds
    bool valid;               // Whether result is valid
} SpeciesResult;

/**
 * Inference Engine Class
 * 
 * Handles the complete inference pipeline from raw image data to 
 * wildlife classification results with optimized performance.
 */
class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    /**
     * Initialize the inference engine
     * @param modelManager Pointer to model manager instance
     * @return true if initialization successful
     */
    bool initialize(ModelManager* modelManager);

    /**
     * Shutdown the inference engine
     */
    void shutdown();

    /**
     * Set preprocessing configuration
     * @param config Preprocessing configuration
     */
    void setPreprocessingConfig(const PreprocessingConfig& config);

    /**
     * Preprocess raw image data for inference
     * @param imageData Raw image data (RGB or grayscale)
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels (1 or 3)
     * @param outputBuffer Output buffer for preprocessed data
     * @return true if preprocessing successful
     */
    bool preprocessImage(const uint8_t* imageData, uint16_t width, uint16_t height, 
                        uint8_t channels, float* outputBuffer);

    /**
     * Run species classification inference
     * @param imageData Preprocessed image data
     * @param result Output result structure
     * @return true if inference successful
     */
    bool classifySpecies(const float* imageData, SpeciesResult* result);

    /**
     * Run behavior analysis inference
     * @param imageSequence Sequence of preprocessed images for temporal analysis
     * @param sequenceLength Number of images in sequence
     * @param result Output result structure
     * @return true if inference successful
     */
    bool analyzeBehavior(const float* imageSequence, uint8_t sequenceLength, SpeciesResult* result);

    /**
     * Run motion detection inference
     * @param currentFrame Current frame
     * @param previousFrame Previous frame
     * @param detections Output array for detections
     * @param maxDetections Maximum number of detections
     * @return Number of detections found
     */
    uint8_t detectMotion(const float* currentFrame, const float* previousFrame, 
                        Detection* detections, uint8_t maxDetections);

    /**
     * Run complete wildlife analysis pipeline
     * @param imageData Raw image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @param result Output result structure
     * @return true if analysis successful
     */
    bool analyzeWildlife(const uint8_t* imageData, uint16_t width, uint16_t height, 
                        uint8_t channels, SpeciesResult* result);

    /**
     * Get inference performance statistics
     * @return Performance statistics
     */
    PerformanceStats getPerformanceStats() const;

    /**
     * Reset performance statistics
     */
    void resetPerformanceStats();

    /**
     * Get current memory usage
     * @return Memory usage in bytes
     */
    size_t getMemoryUsage() const;

    /**
     * Check if inference engine is ready
     * @return true if ready for inference
     */
    bool isReady() const { return initialized_ && modelManager_ != nullptr; }

private:
    bool initialized_;                    // Initialization status
    ModelManager* modelManager_;          // Model manager instance
    TensorFlowLiteESP32* tfliteEngine_;   // TensorFlow Lite engine
    ModelHandle speciesModel_;            // Species classification model handle
    ModelHandle behaviorModel_;           // Behavior analysis model handle
    ModelHandle motionModel_;             // Motion detection model handle
    PreprocessingConfig preprocessConfig_; // Preprocessing configuration
    float* preprocessBuffer_;             // Buffer for preprocessed data
    float* previousFrameBuffer_;          // Buffer for previous frame (motion detection)
    PerformanceStats performanceStats_;   // Performance tracking

    /**
     * Load required models
     * @return true if models loaded successfully
     */
    bool loadModels();

    /**
     * Unload all models
     */
    void unloadModels();

    /**
     * Resize image using bilinear interpolation
     * @param input Input image data
     * @param inputWidth Input width
     * @param inputHeight Input height
     * @param output Output image data
     * @param outputWidth Output width
     * @param outputHeight Output height
     * @param channels Number of channels
     */
    void resizeImage(const uint8_t* input, uint16_t inputWidth, uint16_t inputHeight,
                    float* output, uint16_t outputWidth, uint16_t outputHeight, uint8_t channels);

    /**
     * Normalize pixel values
     * @param data Image data to normalize
     * @param size Size of data array
     * @param config Preprocessing configuration
     */
    void normalizePixels(float* data, size_t size, const PreprocessingConfig& config);

    /**
     * Apply non-maximum suppression to detections
     * @param detections Array of detections
     * @param numDetections Number of detections
     * @param iouThreshold IoU threshold for suppression
     * @return Number of detections after suppression
     */
    uint8_t applyNMS(Detection* detections, uint8_t numDetections, float iouThreshold);

    /**
     * Calculate IoU between two bounding boxes
     * @param box1 First bounding box
     * @param box2 Second bounding box
     * @return IoU value (0-1)
     */
    float calculateIoU(const float* box1, const float* box2);

    /**
     * Update performance statistics
     * @param inferenceTime Time taken for inference
     * @param success Whether inference was successful
     */
    void updatePerformanceStats(uint32_t inferenceTime, bool success);
};

#endif // INFERENCE_ENGINE_H