#ifndef MOTION_DETECTION_H
#define MOTION_DETECTION_H

#include <Arduino.h>
#include <esp_camera.h>
#include "../include/config.h"

/**
 * @brief Frame-based motion detection class
 * 
 * Implements motion detection using frame differencing
 * and block-based analysis for wildlife monitoring.
 */
class MotionDetection {
public:
    /**
     * @brief Motion detection result structure
     */
    struct MotionResult {
        bool motionDetected;
        float motionLevel;          // 0.0 to 1.0
        uint16_t motionBlocks;      // Number of blocks with motion
        uint16_t boundingBoxX;      // Motion bounding box
        uint16_t boundingBoxY;
        uint16_t boundingBoxW;
        uint16_t boundingBoxH;
        uint32_t processTime;       // Processing time in ms
    };

    /**
     * @brief Motion detection statistics
     */
    struct MotionStats {
        uint32_t totalFrames;
        uint32_t motionFrames;
        uint32_t falsePositives;
        float averageMotionLevel;
        uint32_t averageProcessTime;
        float detectionRate;
    };

    /**
     * @brief Initialize motion detection
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Analyze frame for motion
     * @param currentFrame Current camera frame buffer
     * @return Motion detection result
     */
    MotionResult analyzeFrame(camera_fb_t* currentFrame);

    /**
     * @brief Set motion detection sensitivity
     * @param threshold Motion threshold (0-255)
     * @param minBlocks Minimum blocks for motion detection
     */
    void setSensitivity(uint8_t threshold = MOTION_THRESHOLD, 
                       uint16_t minBlocks = MOTION_MIN_AREA);

    /**
     * @brief Enable or disable motion detection
     * @param enable Enable state
     */
    void setEnabled(bool enable) { enabled = enable; }

    /**
     * @brief Check if motion detection is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }

    /**
     * @brief Get motion detection statistics
     * @return Current statistics
     */
    MotionStats getStatistics() const { return stats; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Set reference frame for motion detection
     * @param frame Reference frame buffer
     */
    void setReferenceFrame(camera_fb_t* frame);

    /**
     * @brief Update reference frame automatically
     * @param autoUpdate Enable automatic reference frame update
     */
    void setAutoReferenceUpdate(bool autoUpdate) { autoRefUpdate = autoUpdate; }

    /**
     * @brief Cleanup motion detection resources
     */
    void cleanup();

private:
    bool initialized = false;
    bool enabled = true;
    bool autoRefUpdate = true;
    uint8_t motionThreshold = MOTION_THRESHOLD;
    uint16_t minMotionBlocks = MOTION_MIN_AREA;
    
    uint8_t* referenceFrame = nullptr;
    size_t referenceFrameSize = 0;
    uint16_t frameWidth = 0;
    uint16_t frameHeight = 0;
    
    MotionStats stats = {};
    uint32_t frameCounter = 0;

    /**
     * @brief Calculate frame difference
     * @param current Current frame data
     * @param reference Reference frame data
     * @param size Frame size
     * @return Motion result
     */
    MotionResult calculateFrameDifference(const uint8_t* current, 
                                         const uint8_t* reference, 
                                         size_t size);

    /**
     * @brief Perform block-based motion analysis
     * @param current Current frame data
     * @param reference Reference frame data
     * @param width Frame width
     * @param height Frame height
     * @return Motion result with bounding box
     */
    MotionResult blockBasedAnalysis(const uint8_t* current,
                                   const uint8_t* reference,
                                   uint16_t width,
                                   uint16_t height);

    /**
     * @brief Convert JPEG frame to grayscale
     * @param jpegData JPEG frame data
     * @param jpegSize JPEG frame size
     * @param grayData Output grayscale data
     * @param maxSize Maximum output size
     * @return Actual grayscale data size
     */
    size_t jpegToGrayscale(const uint8_t* jpegData, size_t jpegSize,
                          uint8_t* grayData, size_t maxSize);

    /**
     * @brief Update motion statistics
     * @param result Motion detection result
     */
    void updateStatistics(const MotionResult& result);

    /**
     * @brief Allocate reference frame buffer
     * @param size Required buffer size
     * @return true if allocation successful
     */
    bool allocateReferenceFrame(size_t size);

    /**
     * @brief Simplified motion analysis for performance
     * @param frameData Frame data
     * @param frameSize Frame size
     * @return Motion result
     */
    MotionResult simplifiedMotionAnalysis(const uint8_t* frameData, size_t frameSize);

    /**
     * @brief Calculate data sum for motion comparison
     * @param data Data buffer
     * @param size Data size
     * @return Sum of data values
     */
    uint32_t calculateDataSum(const uint8_t* data, size_t size);
};

#endif // MOTION_DETECTION_H