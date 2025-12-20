/**
 * @file wildlife_detector.h
 * @brief Foundational AI wildlife detection system
 * @version 1.1.0
 * 
 * This file provides the foundational framework for AI-powered wildlife detection.
 * It integrates with the existing comprehensive AI system in firmware/src/ai/ while
 * providing a simplified, modular interface for basic wildlife classification.
 * 
 * Enhanced with basic AI features:
 * - On-device motion detection with false positive reduction
 * - Simple animal vs. non-animal classification
 * - Size estimation for detected objects
 */

#ifndef WILDLIFE_DETECTOR_H
#define WILDLIFE_DETECTOR_H

#include <stdint.h>
#include <vector>
#include <memory>
#include "basic_ai_features.h"

namespace WildlifeDetection {

/**
 * @brief Wildlife species classification results
 */
enum class SpeciesType {
    UNKNOWN = 0,
    DEER,
    RABBIT,
    RACCOON,
    BIRD,
    SQUIRREL,
    BEAR,
    FOX,
    COYOTE,
    WILDCAT,
    OTHER_MAMMAL,
    OTHER_BIRD,
    HUMAN        // For security/safety purposes
};

/**
 * @brief Confidence levels for detection results
 */
enum class ConfidenceLevel {
    VERY_LOW = 0,    // 0-20%
    LOW,             // 20-40%
    MEDIUM,          // 40-60%
    HIGH,            // 60-80%
    VERY_HIGH        // 80-100%
};

/**
 * @brief Detection result structure
 */
struct DetectionResult {
    SpeciesType species;
    ConfidenceLevel confidence;
    float confidence_score;     // 0.0-1.0
    uint32_t detection_time;    // Timestamp
    uint16_t x, y, width, height; // Bounding box
    bool motion_detected;
    float size_estimate;        // Relative size 0.0-1.0
};

/**
 * @brief Wildlife detector configuration
 */
struct DetectorConfig {
    float confidence_threshold = 0.6f;
    uint32_t detection_interval_ms = 1000;
    bool enable_motion_trigger = true;
    bool enable_species_classification = true;
    bool enable_size_estimation = true;
    uint8_t max_detections_per_frame = 5;
    
    // Basic AI features configuration
    bool enable_false_positive_reduction = true;  ///< Enable false positive reduction
    bool enable_animal_classification = true;      ///< Enable animal vs non-animal classification
    float false_positive_threshold = 0.5f;         ///< Threshold for false positive rejection (0.0-1.0)
    uint8_t motion_history_frames = 5;             ///< Number of frames for motion history
};

/**
 * @brief Main wildlife detection class
 * 
 * Provides a foundational framework for wildlife detection that can be extended
 * with more sophisticated AI models and processing capabilities.
 */
class WildlifeDetector {
public:
    WildlifeDetector();
    ~WildlifeDetector();

    /**
     * @brief Initialize the detector with configuration
     * @param config Detector configuration parameters
     * @return true if initialization successful
     */
    bool initialize(const DetectorConfig& config);

    /**
     * @brief Process a camera frame for wildlife detection
     * @param frame_data Raw image data
     * @param frame_size Size of image data in bytes
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @return Vector of detection results
     */
    std::vector<DetectionResult> detectWildlife(const uint8_t* frame_data, 
                                               size_t frame_size,
                                               uint16_t width, 
                                               uint16_t height);

    /**
     * @brief Check if motion is detected in the current frame
     * @param frame_data Raw image data
     * @param frame_size Size of image data
     * @return true if motion detected
     */
    bool detectMotion(const uint8_t* frame_data, size_t frame_size);

    /**
     * @brief Update detector configuration
     * @param config New configuration
     */
    void updateConfig(const DetectorConfig& config);

    /**
     * @brief Get current configuration
     * @return Current detector configuration
     */
    DetectorConfig getConfig() const { return config_; }

    /**
     * @brief Get detection statistics
     */
    struct Statistics {
        uint32_t total_detections = 0;
        uint32_t species_counts[static_cast<int>(SpeciesType::HUMAN) + 1] = {0};
        uint32_t false_positives = 0;
        uint32_t false_positives_filtered = 0;  ///< False positives caught by basic AI
        uint32_t animal_classifications = 0;     ///< Count of animal classifications
        uint32_t non_animal_classifications = 0; ///< Count of non-animal classifications
        uint32_t processing_time_ms = 0;
        float average_confidence = 0.0f;
    };

    Statistics getStatistics() const { return stats_; }
    void resetStatistics();

    /**
     * @brief Check if detector is initialized and ready
     */
    bool isReady() const { return initialized_; }

    /**
     * @brief Get false positive score for last detection
     * @return Float value 0.0 (definitely real) to 1.0 (definitely false positive)
     */
    float getLastFalsePositiveScore() const { return last_fp_score_; }

    /**
     * @brief Check if last detection was classified as animal
     * @return true if classified as animal
     */
    bool wasLastDetectionAnimal() const { return last_was_animal_; }

    /**
     * @brief Get size category of last detection
     * @return Size category from basic AI
     */
    BasicAI::SizeCategory getLastSizeCategory() const { return last_size_category_; }

private:
    DetectorConfig config_;
    Statistics stats_;
    bool initialized_;
    
    // Frame processing buffers
    std::unique_ptr<uint8_t[]> previous_frame_;
    uint16_t frame_width_;
    uint16_t frame_height_;
    
    // Basic AI processor for enhanced detection
    std::unique_ptr<BasicAI::BasicAIProcessor> ai_processor_;
    
    // Last detection results from basic AI
    float last_fp_score_;
    bool last_was_animal_;
    BasicAI::SizeCategory last_size_category_;
    
    // Internal processing methods
    bool preprocessFrame(const uint8_t* frame_data, size_t frame_size);
    SpeciesType classifyObject(const uint8_t* region_data, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    ConfidenceLevel calculateConfidence(float score);
    float estimateSize(uint16_t width, uint16_t height, uint16_t frame_width, uint16_t frame_height);
    void updateStatistics(const DetectionResult& result);
    
    // Basic AI integration methods
    bool initializeBasicAI();
    bool runBasicAIAnalysis(const uint8_t* frame_data, size_t frame_size, 
                           uint16_t width, uint16_t height,
                           BasicAI::AIAnalysisResult& result);
};

} // namespace WildlifeDetection

#endif // WILDLIFE_DETECTOR_H