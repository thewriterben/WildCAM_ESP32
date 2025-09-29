#ifndef WILDLIFE_CLASSIFIER_H
#define WILDLIFE_CLASSIFIER_H

#include <Arduino.h>
#include <esp_camera.h>
#include "../include/config.h"

// Forward declarations
struct InferenceResult;
struct CameraFrame;

/**
 * @brief Wildlife species classification using TensorFlow Lite
 * 
 * Provides AI-powered species identification for wildlife monitoring
 * with optimized inference for ESP32 platform.
 */
class WildlifeClassifier {
public:
    /**
     * @brief Species types supported by the classifier
     */
    enum class SpeciesType {
        UNKNOWN = 0,
        WHITE_TAILED_DEER = 1,
        BLACK_BEAR = 2,
        RED_FOX = 3,
        GRAY_WOLF = 4,
        MOUNTAIN_LION = 5,
        ELK = 6,
        MOOSE = 7,
        RACCOON = 8,
        COYOTE = 9,
        BOBCAT = 10,
        WILD_TURKEY = 11,
        BALD_EAGLE = 12,
        RED_TAILED_HAWK = 13,
        GREAT_BLUE_HERON = 14,
        HUMAN = 50
    };

    /**
     * @brief Classification confidence levels
     */
    enum class ConfidenceLevel {
        VERY_LOW = 0,
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
        VERY_HIGH = 4
    };

    /**
     * @brief Classification result structure
     */
    struct ClassificationResult {
        SpeciesType species;
        float confidence;
        ConfidenceLevel confidenceLevel;
        uint32_t inferenceTime;
        bool isValid;
        String speciesName;
        uint8_t animalCount;
        float boundingBoxes[10][4]; // Up to 10 detected animals (x, y, w, h)
    };

    /**
     * @brief Classifier statistics
     */
    struct ClassifierStats {
        uint32_t totalInferences;
        uint32_t successfulInferences;
        uint32_t averageInferenceTime;
        float averageConfidence;
        uint32_t speciesDetectionCounts[51]; // Count for each species
    };

    /**
     * @brief Initialize wildlife classifier
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Classify wildlife in camera frame
     * @param frame Camera frame buffer
     * @return Classification result
     */
    ClassificationResult classifyFrame(camera_fb_t* frame);

    /**
     * @brief Classify wildlife from image data
     * @param imageData Image data buffer
     * @param imageSize Image data size
     * @param width Image width
     * @param height Image height
     * @return Classification result
     */
    ClassificationResult classifyImage(const uint8_t* imageData, size_t imageSize, 
                                     uint16_t width, uint16_t height);

    /**
     * @brief Set classification confidence threshold
     * @param threshold Minimum confidence for valid detection (0.0 to 1.0)
     */
    void setConfidenceThreshold(float threshold);

    /**
     * @brief Enable or disable the classifier
     * @param enable Enable state
     */
    void setEnabled(bool enable) { enabled = enable; }

    /**
     * @brief Check if classifier is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Get classifier statistics
     * @return Current statistics
     */
    ClassifierStats getStatistics() const { return stats; }

    /**
     * @brief Reset classifier statistics
     */
    void resetStatistics();

    /**
     * @brief Get species name from type
     * @param species Species type
     * @return Human-readable species name
     */
    static String getSpeciesName(SpeciesType species);

    /**
     * @brief Get confidence level description
     * @param level Confidence level
     * @return Human-readable description
     */
    static String getConfidenceLevelDescription(ConfidenceLevel level);

    /**
     * @brief Check if species is dangerous/predator
     * @param species Species type
     * @return true if species is potentially dangerous
     */
    static bool isDangerousSpecies(SpeciesType species);

    /**
     * @brief Cleanup classifier resources
     */
    void cleanup();

private:
    bool initialized = false;
    bool enabled = true;
    float confidenceThreshold = SPECIES_CONFIDENCE_THRESHOLD;
    
    ClassifierStats stats = {};
    
    // Model and inference data (placeholders for TensorFlow Lite integration)
    void* modelBuffer = nullptr;
    void* interpreter = nullptr;
    void* tensorArena = nullptr;
    size_t arenaSize = TFLITE_ARENA_SIZE;

    /**
     * @brief Load TensorFlow Lite model
     * @return true if model loaded successfully
     */
    bool loadModel();

    /**
     * @brief Preprocess image for inference
     * @param imageData Input image data
     * @param imageSize Input image size
     * @param inputTensor Output tensor buffer
     * @return true if preprocessing successful
     */
    bool preprocessImage(const uint8_t* imageData, size_t imageSize, float* inputTensor);

    /**
     * @brief Run inference on preprocessed data
     * @param inputTensor Input tensor data
     * @param outputTensor Output tensor buffer
     * @return true if inference successful
     */
    bool runInference(const float* inputTensor, float* outputTensor);

    /**
     * @brief Process inference output to classification result
     * @param outputTensor Model output tensor
     * @return Classification result
     */
    ClassificationResult processOutput(const float* outputTensor);

    /**
     * @brief Convert confidence score to confidence level
     * @param confidence Confidence score (0.0 to 1.0)
     * @return Confidence level
     */
    ConfidenceLevel scoreToConfidenceLevel(float confidence);

    /**
     * @brief Update classifier statistics
     * @param result Classification result
     */
    void updateStatistics(const ClassificationResult& result);

    /**
     * @brief Simplified classification for demonstration
     * @param imageData Input image data
     * @param imageSize Input image size
     * @return Classification result
     */
    ClassificationResult performSimplifiedClassification(const uint8_t* imageData, size_t imageSize);
    
    /**
     * @brief Convert TensorFlow Lite result to classifier result format
     * @param tfResult TensorFlow Lite inference result
     * @return Classification result
     */
    ClassificationResult convertTensorFlowResult(const InferenceResult& tfResult);
};

#endif // WILDLIFE_CLASSIFIER_H