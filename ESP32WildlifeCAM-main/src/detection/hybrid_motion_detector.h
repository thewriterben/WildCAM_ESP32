#ifndef HYBRID_MOTION_DETECTOR_H
#define HYBRID_MOTION_DETECTOR_H

#include "pir_sensor.h"
#include "motion_detection.h"
#include "../camera/camera_manager.h"
#include "../include/config.h"

/**
 * @brief Hybrid motion detector combining PIR and frame-based detection
 * 
 * Provides intelligent motion detection by combining:
 * - PIR sensor for instant motion detection
 * - Frame-based analysis for motion verification
 * - False positive filtering and confidence scoring
 */
class HybridMotionDetector {
public:
    /**
     * @brief Motion detection confidence levels
     */
    enum class ConfidenceLevel {
        NONE = 0,
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
        VERY_HIGH = 4
    };

    /**
     * @brief Hybrid motion detection result
     */
    struct HybridResult {
        bool motionDetected;
        ConfidenceLevel confidence;
        float confidenceScore;      // 0.0 to 1.0
        bool pirTriggered;
        bool frameMotionDetected;
        MotionDetection::MotionResult frameResult;
        uint32_t totalProcessTime;
        String description;
    };

    /**
     * @brief Hybrid detector statistics
     */
    struct HybridStats {
        uint32_t totalDetections;
        uint32_t pirOnlyDetections;
        uint32_t frameOnlyDetections;
        uint32_t combinedDetections;
        uint32_t falsePositives;
        float averageConfidence;
        uint32_t averageProcessTime;
    };

    /**
     * @brief Initialize hybrid motion detector
     * @param cameraManager Camera manager instance
     * @return true if initialization successful
     */
    bool initialize(CameraManager* cameraManager);

    /**
     * @brief Check for motion using hybrid approach
     * @return Hybrid detection result
     */
    HybridResult detectMotion();

    /**
     * @brief Set motion detection mode
     * @param useHybrid Use hybrid detection (PIR + frame analysis)
     * @param pirOnly Use PIR only mode
     * @param frameOnly Use frame analysis only mode
     */
    void setDetectionMode(bool useHybrid = true, bool pirOnly = false, bool frameOnly = false);

    /**
     * @brief Configure confidence thresholds
     * @param pirWeight Weight of PIR detection (0.0 to 1.0)
     * @param frameWeight Weight of frame detection (0.0 to 1.0)
     * @param confirmationTime Time to wait for confirmation (ms)
     */
    void configureWeights(float pirWeight = 0.6f, float frameWeight = 0.4f, 
                         uint32_t confirmationTime = MOTION_CONFIRMATION_TIME);

    /**
     * @brief Enable or disable the detector
     * @param enable Enable state
     */
    void setEnabled(bool enable);

    /**
     * @brief Check if detector is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Get detector statistics
     * @return Current statistics
     */
    HybridStats getStatistics() const { return stats; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Configure for low power operation
     * @param lowPower Enable low power mode
     */
    void setLowPowerMode(bool lowPower);

    /**
     * @brief Get current confidence level description
     * @param level Confidence level
     * @return Human-readable description
     */
    static String getConfidenceDescription(ConfidenceLevel level);

    /**
     * @brief Cleanup hybrid detector resources
     */
    void cleanup();

private:
    bool initialized = false;
    bool enabled = true;
    bool hybridMode = true;
    bool pirOnlyMode = false;
    bool frameOnlyMode = false;
    bool lowPowerMode = false;

    // Component instances
    PIRSensor pirSensor;
    MotionDetection motionDetection;
    CameraManager* camera = nullptr;

    // Configuration
    float pirWeight = 0.6f;
    float frameWeight = 0.4f;
    uint32_t confirmationTime = MOTION_CONFIRMATION_TIME;

    // Statistics
    HybridStats stats = {};

    /**
     * @brief Perform PIR-based detection
     * @return true if PIR detected motion
     */
    bool detectPIRMotion();

    /**
     * @brief Perform frame-based detection
     * @return Frame motion detection result
     */
    MotionDetection::MotionResult detectFrameMotion();

    /**
     * @brief Calculate confidence score from PIR and frame results
     * @param pirDetected PIR detection state
     * @param frameResult Frame detection result
     * @return Combined confidence score
     */
    float calculateConfidenceScore(bool pirDetected, const MotionDetection::MotionResult& frameResult);

    /**
     * @brief Convert confidence score to confidence level
     * @param score Confidence score (0.0 to 1.0)
     * @return Confidence level
     */
    ConfidenceLevel scoreToConfidenceLevel(float score);

    /**
     * @brief Update detector statistics
     * @param result Hybrid detection result
     */
    void updateStatistics(const HybridResult& result);

    /**
     * @brief Generate result description
     * @param result Hybrid detection result
     * @return Human-readable description
     */
    String generateDescription(const HybridResult& result);
};

#endif // HYBRID_MOTION_DETECTOR_H