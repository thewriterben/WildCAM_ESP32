#ifndef ADVANCED_MOTION_DETECTION_H
#define ADVANCED_MOTION_DETECTION_H

#include "motion_detection.h"
#include <vector>
#include <memory>

/**
 * @brief Advanced motion detection with enhanced algorithms
 * 
 * Extends basic motion detection with:
 * - Background subtraction
 * - Movement vector analysis
 * - Object size filtering
 * - Machine learning false positive reduction
 */
class AdvancedMotionDetection : public MotionDetection {
public:
    /**
     * @brief Movement vector structure
     */
    struct MotionVector {
        float dx, dy;           // Movement direction
        float magnitude;        // Movement speed
        uint16_t x, y;         // Vector origin point
        float confidence;       // Vector reliability
    };

    /**
     * @brief Enhanced motion result with advanced analysis
     */
    struct AdvancedMotionResult : public MotionResult {
        // Background subtraction results
        float backgroundSimilarity;
        bool backgroundUpdated;
        
        // Movement analysis
        std::vector<MotionVector> motionVectors;
        float averageSpeed;
        float dominantDirection;  // In radians
        
        // Object filtering
        uint16_t filteredObjects;
        uint16_t minObjectSize;
        uint16_t maxObjectSize;
        
        // Machine learning results
        float mlConfidence;
        float adaptiveThreshold;
        bool falsePositivePrediction;
        
        // Analytics
        uint32_t dwellTime;     // Time object present in scene
        bool isNewObject;       // First detection of this object
    };

    /**
     * @brief Background model configuration
     */
    struct BackgroundConfig {
        float learningRate = 0.1f;      // Background adaptation rate
        float updateThreshold = 0.3f;   // Threshold for background updates
        uint32_t updateInterval = 5000; // Minimum time between updates (ms)
        bool adaptiveUpdate = true;     // Enable adaptive update rate
        uint32_t maxFramesWithoutUpdate = 100; // Force update after this many frames
    };

    /**
     * @brief Object size filter configuration
     */
    struct SizeFilterConfig {
        uint16_t minObjectSize = 25;    // Minimum object size in pixels
        uint16_t maxObjectSize = 5000;  // Maximum object size in pixels
        bool enableFiltering = true;    // Enable size-based filtering
        float sizeConfidenceWeight = 0.3f; // Weight of size in confidence calculation
    };

    /**
     * @brief Machine learning configuration
     */
    struct MLConfig {
        bool enableLearning = true;     // Enable adaptive learning
        float learningRate = 0.05f;     // Learning adaptation rate
        uint32_t trainingPeriod = 7200000; // Training period (2 hours in ms)
        float confidenceThreshold = 0.6f;  // ML confidence threshold
        uint32_t patternMemorySize = 1000;  // Number of patterns to remember
    };

    /**
     * @brief Initialize advanced motion detection
     * @return true if initialization successful
     */
    bool initialize() override;

    /**
     * @brief Analyze frame with advanced algorithms
     * @param currentFrame Current camera frame buffer
     * @return Advanced motion detection result
     */
    AdvancedMotionResult analyzeFrameAdvanced(camera_fb_t* currentFrame);

    /**
     * @brief Configure background subtraction
     * @param config Background model configuration
     */
    void configureBackground(const BackgroundConfig& config);

    /**
     * @brief Configure object size filtering
     * @param config Size filter configuration
     */
    void configureSizeFilter(const SizeFilterConfig& config);

    /**
     * @brief Configure machine learning parameters
     * @param config ML configuration
     */
    void configureML(const MLConfig& config);

    /**
     * @brief Force background model update
     */
    void updateBackgroundModel();

    /**
     * @brief Reset machine learning adaptation
     */
    void resetMLAdaptation();

    /**
     * @brief Get current adaptive threshold
     * @return Current ML-adjusted threshold
     */
    float getAdaptiveThreshold() const { return adaptiveThreshold; }

    /**
     * @brief Get background model similarity
     * @return Similarity to current background model (0-1)
     */
    float getBackgroundSimilarity() const { return backgroundSimilarity; }

    /**
     * @brief Enable/disable specific advanced features
     */
    void setBackgroundSubtractionEnabled(bool enabled) { backgroundEnabled = enabled; }
    void setVectorAnalysisEnabled(bool enabled) { vectorAnalysisEnabled = enabled; }
    void setSizeFilteringEnabled(bool enabled) { sizeFilteringEnabled = enabled; }
    void setMLFilteringEnabled(bool enabled) { mlFilteringEnabled = enabled; }

    /**
     * @brief Cleanup advanced motion detection resources
     */
    void cleanup() override;

private:
    // Configuration
    BackgroundConfig bgConfig;
    SizeFilterConfig sizeConfig;
    MLConfig mlConfig;

    // Feature enable flags
    bool backgroundEnabled = true;
    bool vectorAnalysisEnabled = true;
    bool sizeFilteringEnabled = true;
    bool mlFilteringEnabled = true;

    // Background model
    std::unique_ptr<uint8_t[]> backgroundModel;
    std::unique_ptr<float[]> backgroundVariance;
    uint32_t backgroundFrameCount = 0;
    uint32_t lastBackgroundUpdate = 0;
    float backgroundSimilarity = 0.0f;

    // Motion vectors
    std::vector<MotionVector> currentVectors;
    std::vector<MotionVector> previousVectors;

    // Machine learning state
    float adaptiveThreshold = 0.5f;
    std::vector<float> falsePositivePatterns;
    std::vector<float> truePositivePatterns;
    uint32_t trainingStartTime = 0;

    // Analytics state
    uint32_t objectFirstSeen = 0;
    bool objectTracked = false;

    /**
     * @brief Perform background subtraction
     * @param currentFrame Current frame data
     * @param width Frame width
     * @param height Frame height
     * @return Background subtraction result
     */
    float performBackgroundSubtraction(const uint8_t* currentFrame, 
                                     uint16_t width, uint16_t height);

    /**
     * @brief Update background model with current frame
     * @param currentFrame Current frame data
     * @param width Frame width
     * @param height Frame height
     */
    void updateBackground(const uint8_t* currentFrame, 
                         uint16_t width, uint16_t height);

    /**
     * @brief Analyze movement vectors
     * @param currentFrame Current frame data
     * @param previousFrame Previous frame data
     * @param width Frame width
     * @param height Frame height
     * @return Vector of detected motion vectors
     */
    std::vector<MotionVector> analyzeMotionVectors(const uint8_t* currentFrame,
                                                  const uint8_t* previousFrame,
                                                  uint16_t width, uint16_t height);

    /**
     * @brief Filter objects by size
     * @param motionRegions Detected motion regions
     * @return Filtered motion regions
     */
    std::vector<MotionVector> filterObjectsBySize(const std::vector<MotionVector>& motionRegions);

    /**
     * @brief Apply machine learning false positive filtering
     * @param result Current detection result
     * @return ML-adjusted confidence and false positive prediction
     */
    std::pair<float, bool> applyMLFiltering(const AdvancedMotionResult& result);

    /**
     * @brief Update machine learning model with new data
     * @param features Feature vector from current detection
     * @param isMotion True if this was confirmed motion
     */
    void updateMLModel(const std::vector<float>& features, bool isMotion);

    /**
     * @brief Extract features for machine learning
     * @param result Current detection result
     * @return Feature vector
     */
    std::vector<float> extractMLFeatures(const AdvancedMotionResult& result);

    /**
     * @brief Calculate motion vector confidence
     * @param vector Motion vector to evaluate
     * @return Confidence score (0-1)
     */
    float calculateVectorConfidence(const MotionVector& vector);

    /**
     * @brief Calculate dominant motion direction
     * @param vectors List of motion vectors
     * @return Dominant direction in radians
     */
    float calculateDominantDirection(const std::vector<MotionVector>& vectors);

    /**
     * @brief Calculate average motion speed
     * @param vectors List of motion vectors
     * @return Average speed
     */
    float calculateAverageSpeed(const std::vector<MotionVector>& vectors);

    /**
     * @brief Allocate background model buffers
     * @param width Frame width
     * @param height Frame height
     * @return true if allocation successful
     */
    bool allocateBackgroundModel(uint16_t width, uint16_t height);
};

#endif // ADVANCED_MOTION_DETECTION_H