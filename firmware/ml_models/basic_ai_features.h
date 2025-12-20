/**
 * @file basic_ai_features.h
 * @brief Basic AI features for ESP32 Wildlife Camera
 * @version 1.0.0
 * 
 * Implements on-device AI capabilities:
 * - Motion detection with false positive reduction
 * - Simple animal vs. non-animal classification
 * - Size estimation for detected objects
 * 
 * Designed for resource-constrained ESP32 devices with minimal memory
 * footprint and efficient processing.
 */

#ifndef BASIC_AI_FEATURES_H
#define BASIC_AI_FEATURES_H

#include <stdint.h>
#include <stddef.h>

namespace BasicAI {

//==============================================================================
// Constants
//==============================================================================

/** Maximum number of motion regions to track */
static const uint8_t MAX_MOTION_REGIONS = 10;

/** Motion history buffer size for false positive reduction */
static const uint8_t MOTION_HISTORY_SIZE = 5;

/** Default motion detection threshold (0-255) */
static const uint8_t DEFAULT_MOTION_THRESHOLD = 25;

/** Minimum pixels for valid motion region */
static const uint16_t MIN_MOTION_PIXELS = 100;

/** Classification confidence threshold */
static const float CLASSIFICATION_THRESHOLD = 0.5f;

//==============================================================================
// Enumerations
//==============================================================================

/**
 * @brief Classification result for animal vs non-animal
 */
enum class ClassificationType {
    UNKNOWN = 0,        ///< Unable to classify
    ANIMAL,             ///< Classified as animal (wildlife)
    NON_ANIMAL,         ///< Classified as non-animal (vegetation, shadows, etc.)
    HUMAN,              ///< Classified as human
    VEHICLE             ///< Classified as vehicle
};

/**
 * @brief Size category for detected objects
 */
enum class SizeCategory {
    TINY = 0,           ///< Very small (birds, rodents) - <5% frame area
    SMALL,              ///< Small (rabbits, squirrels) - 5-15% frame area
    MEDIUM,             ///< Medium (foxes, raccoons) - 15-30% frame area
    LARGE,              ///< Large (deer, large dogs) - 30-50% frame area
    VERY_LARGE          ///< Very large (bears, moose) - >50% frame area
};

/**
 * @brief Motion detection confidence level
 */
enum class MotionConfidence {
    NONE = 0,           ///< No motion detected
    LOW,                ///< Low confidence - might be noise
    MEDIUM,             ///< Medium confidence - likely real motion
    HIGH,               ///< High confidence - definitely real motion
    VERY_HIGH           ///< Very high confidence - strong, consistent motion
};

//==============================================================================
// Data Structures
//==============================================================================

/**
 * @brief Bounding box for detected regions
 */
struct BoundingBox {
    uint16_t x;         ///< X coordinate (top-left)
    uint16_t y;         ///< Y coordinate (top-left)
    uint16_t width;     ///< Width in pixels
    uint16_t height;    ///< Height in pixels
};

/**
 * @brief Motion detection result for a single region
 */
struct MotionRegion {
    BoundingBox bbox;           ///< Bounding box of motion region
    float intensity;            ///< Motion intensity (0.0-1.0)
    uint32_t pixel_count;       ///< Number of pixels with motion
    uint32_t timestamp;         ///< Detection timestamp (ms)
    bool is_valid;              ///< Whether this region is valid
};

/**
 * @brief Result of motion detection with false positive analysis
 */
struct MotionDetectionResult {
    bool motion_detected;               ///< Whether motion was detected
    MotionConfidence confidence;        ///< Confidence level
    uint8_t region_count;               ///< Number of motion regions
    MotionRegion regions[MAX_MOTION_REGIONS]; ///< Motion regions
    float false_positive_score;         ///< Likelihood of false positive (0.0-1.0)
    uint32_t processing_time_us;        ///< Processing time in microseconds
};

/**
 * @brief Result of animal vs non-animal classification
 */
struct ClassificationResult {
    ClassificationType classification;  ///< Classification type
    float confidence;                   ///< Classification confidence (0.0-1.0)
    float animal_score;                 ///< Score for animal classification
    float non_animal_score;             ///< Score for non-animal classification
    uint32_t processing_time_us;        ///< Processing time in microseconds
};

/**
 * @brief Result of size estimation
 */
struct SizeEstimationResult {
    SizeCategory category;              ///< Size category
    float relative_size;                ///< Relative size (0.0-1.0) of frame area
    uint32_t estimated_pixels;          ///< Estimated object size in pixels
    float width_ratio;                  ///< Width relative to frame width
    float height_ratio;                 ///< Height relative to frame height
};

/**
 * @brief Combined AI analysis result
 */
struct AIAnalysisResult {
    MotionDetectionResult motion;       ///< Motion detection results
    ClassificationResult classification; ///< Classification results
    SizeEstimationResult size;          ///< Size estimation results
    bool is_valid_wildlife_event;       ///< Overall determination
    float overall_confidence;           ///< Overall confidence score
    uint32_t total_processing_time_us;  ///< Total processing time
};

/**
 * @brief Configuration for basic AI features
 */
struct BasicAIConfig {
    // Motion detection settings
    uint8_t motion_threshold;           ///< Pixel difference threshold (0-255)
    uint8_t min_motion_frames;          ///< Minimum consecutive frames for valid motion
    bool enable_temporal_filtering;     ///< Enable temporal filtering for noise reduction
    float noise_reduction_level;        ///< Noise reduction strength (0.0-1.0)
    
    // Classification settings
    float animal_confidence_threshold;  ///< Minimum confidence for animal classification
    bool enable_human_detection;        ///< Enable human detection
    bool enable_vehicle_detection;      ///< Enable vehicle detection
    
    // Size estimation settings
    bool enable_size_estimation;        ///< Enable size estimation
    float min_object_size;              ///< Minimum object size (fraction of frame)
    float max_object_size;              ///< Maximum object size (fraction of frame)
    
    // Performance settings
    bool fast_mode;                     ///< Use fast processing (lower accuracy)
    uint16_t processing_width;          ///< Processing resolution width
    uint16_t processing_height;         ///< Processing resolution height
};

//==============================================================================
// BasicAIProcessor Class
//==============================================================================

/**
 * @class BasicAIProcessor
 * @brief On-device AI processor for wildlife detection
 * 
 * Provides lightweight AI processing capabilities for ESP32 devices:
 * - Frame differencing for motion detection
 * - Texture and edge analysis for classification
 * - Blob analysis for size estimation
 * 
 * Optimized for low memory usage and fast processing.
 */
class BasicAIProcessor {
public:
    BasicAIProcessor();
    ~BasicAIProcessor();

    /**
     * @brief Initialize the AI processor
     * @param config Configuration parameters
     * @return true if initialization successful
     */
    bool initialize(const BasicAIConfig& config);

    /**
     * @brief Check if processor is initialized
     * @return true if initialized and ready
     */
    bool isInitialized() const { return initialized_; }

    /**
     * @brief Update configuration
     * @param config New configuration
     */
    void updateConfig(const BasicAIConfig& config);

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    BasicAIConfig getConfig() const { return config_; }

    //--------------------------------------------------------------------------
    // Motion Detection with False Positive Reduction
    //--------------------------------------------------------------------------

    /**
     * @brief Detect motion with false positive reduction
     * @param current_frame Current frame data (grayscale or RGB)
     * @param frame_size Size of frame data in bytes
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     * @param channels Number of color channels (1 for grayscale, 3 for RGB)
     * @return Motion detection result
     */
    MotionDetectionResult detectMotion(
        const uint8_t* current_frame,
        size_t frame_size,
        uint16_t width,
        uint16_t height,
        uint8_t channels = 1
    );

    /**
     * @brief Check if motion is likely a false positive
     * @param result Motion detection result to analyze
     * @return Float score (0.0 = definitely real, 1.0 = definitely false positive)
     */
    float analyzeFalsePositive(const MotionDetectionResult& result);

    //--------------------------------------------------------------------------
    // Animal vs Non-Animal Classification
    //--------------------------------------------------------------------------

    /**
     * @brief Classify region as animal or non-animal
     * @param frame_data Frame data (grayscale or RGB)
     * @param frame_size Size of frame data
     * @param width Frame width
     * @param height Frame height
     * @param roi Region of interest to classify
     * @param channels Number of color channels
     * @return Classification result
     */
    ClassificationResult classifyRegion(
        const uint8_t* frame_data,
        size_t frame_size,
        uint16_t width,
        uint16_t height,
        const BoundingBox& roi,
        uint8_t channels = 1
    );

    /**
     * @brief Simple animal detection using texture analysis
     * @param frame_data Frame data
     * @param width Frame width
     * @param height Frame height
     * @param channels Number of color channels
     * @return true if animal-like characteristics detected
     */
    bool hasAnimalCharacteristics(
        const uint8_t* frame_data,
        uint16_t width,
        uint16_t height,
        uint8_t channels = 1
    );

    //--------------------------------------------------------------------------
    // Size Estimation
    //--------------------------------------------------------------------------

    /**
     * @brief Estimate size of detected object
     * @param bbox Bounding box of detected object
     * @param frame_width Total frame width
     * @param frame_height Total frame height
     * @return Size estimation result
     */
    SizeEstimationResult estimateSize(
        const BoundingBox& bbox,
        uint16_t frame_width,
        uint16_t frame_height
    );

    /**
     * @brief Get size category from relative size
     * @param relative_size Size relative to frame (0.0-1.0)
     * @return Size category enum
     */
    static SizeCategory getSizeCategory(float relative_size);

    //--------------------------------------------------------------------------
    // Combined Analysis
    //--------------------------------------------------------------------------

    /**
     * @brief Perform complete AI analysis on frame
     * @param current_frame Current frame data
     * @param frame_size Size of frame data
     * @param width Frame width
     * @param height Frame height
     * @param channels Number of color channels
     * @return Combined AI analysis result
     */
    AIAnalysisResult analyzeFrame(
        const uint8_t* current_frame,
        size_t frame_size,
        uint16_t width,
        uint16_t height,
        uint8_t channels = 1
    );

    //--------------------------------------------------------------------------
    // Statistics and Diagnostics
    //--------------------------------------------------------------------------

    /**
     * @brief Get processing statistics
     */
    struct Statistics {
        uint32_t total_frames_processed;
        uint32_t motion_detections;
        uint32_t false_positives_filtered;
        uint32_t animal_classifications;
        uint32_t non_animal_classifications;
        uint32_t average_processing_time_us;
    };

    /**
     * @brief Get current statistics
     * @return Processing statistics
     */
    Statistics getStatistics() const { return stats_; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Reset internal state (clears frame history)
     */
    void reset();

private:
    BasicAIConfig config_;          ///< Configuration
    bool initialized_;              ///< Initialization state
    Statistics stats_;              ///< Processing statistics

    // Frame buffers for motion detection
    uint8_t* previous_frame_;       ///< Previous frame for differencing
    uint8_t* diff_frame_;           ///< Frame difference buffer
    size_t frame_buffer_size_;      ///< Size of frame buffers
    uint16_t current_width_;        ///< Current frame width
    uint16_t current_height_;       ///< Current frame height

    // Motion history for false positive reduction
    struct MotionHistoryEntry {
        uint32_t timestamp;
        float intensity;
        uint8_t region_count;
        bool valid;
    };
    MotionHistoryEntry motion_history_[MOTION_HISTORY_SIZE];
    uint8_t history_index_;

    // Internal processing methods
    
    /**
     * @brief Calculate frame difference
     */
    void calculateFrameDifference(
        const uint8_t* current,
        const uint8_t* previous,
        uint8_t* diff,
        size_t size,
        uint8_t threshold
    );

    /**
     * @brief Find connected regions in difference frame
     */
    uint8_t findMotionRegions(
        const uint8_t* diff_frame,
        uint16_t width,
        uint16_t height,
        MotionRegion* regions,
        uint8_t max_regions
    );

    /**
     * @brief Calculate texture features for classification
     */
    float calculateTextureScore(
        const uint8_t* region_data,
        uint16_t width,
        uint16_t height
    );

    /**
     * @brief Calculate edge density for classification
     */
    float calculateEdgeDensity(
        const uint8_t* region_data,
        uint16_t width,
        uint16_t height
    );

    /**
     * @brief Analyze motion consistency across history
     */
    float analyzeMotionConsistency();

    /**
     * @brief Update motion history
     */
    void updateMotionHistory(const MotionDetectionResult& result);

    /**
     * @brief Convert multi-channel to grayscale if needed
     */
    void convertToGrayscale(
        const uint8_t* input,
        uint8_t* output,
        size_t pixel_count,
        uint8_t channels
    );

    /**
     * @brief Allocate frame buffers
     */
    bool allocateBuffers(size_t size);

    /**
     * @brief Free frame buffers
     */
    void freeBuffers();

    /**
     * @brief Update statistics after processing
     */
    void updateStatistics(uint32_t processing_time_us, bool motion_detected, 
                         bool is_animal, bool was_false_positive);
};

//==============================================================================
// Default Configurations
//==============================================================================

/**
 * @brief Get default configuration for general wildlife monitoring
 */
BasicAIConfig getDefaultConfig();

/**
 * @brief Get configuration optimized for battery-powered operation
 */
BasicAIConfig getLowPowerConfig();

/**
 * @brief Get configuration optimized for accuracy
 */
BasicAIConfig getHighAccuracyConfig();

//==============================================================================
// Utility Functions
//==============================================================================

/**
 * @brief Convert classification type to string
 */
const char* classificationTypeToString(ClassificationType type);

/**
 * @brief Convert size category to string
 */
const char* sizeCategoryToString(SizeCategory category);

/**
 * @brief Convert motion confidence to string
 */
const char* motionConfidenceToString(MotionConfidence confidence);

} // namespace BasicAI

#endif // BASIC_AI_FEATURES_H
