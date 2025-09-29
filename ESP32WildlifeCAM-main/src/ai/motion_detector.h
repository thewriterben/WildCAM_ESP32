#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <memory>
#include <vector>
#include <functional>
#include <esp_camera.h>

// Forward declarations
class MotionDetectionManager;
class HybridMotionDetector;

/**
 * @brief Motion sensitivity levels
 */
enum class MotionSensitivity {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    VERY_HIGH = 3
};

/**
 * @brief Motion detection event structure
 */
struct MotionEvent {
    unsigned long timestamp;
    float confidence;
    int x, y;        // Motion detection coordinates
    int width, height; // Motion area dimensions
};

/**
 * @brief Motion detector statistics
 */
struct MotionDetectorStats {
    uint32_t totalDetections;
    float averageConfidence;
    unsigned long lastDetectionTime;
};

/**
 * @class MotionDetector
 * @brief AI-enhanced motion detection system for wildlife cameras
 * 
 * This class provides a high-level interface for motion detection using
 * AI algorithms combined with traditional motion detection methods.
 * It integrates with the existing motion detection infrastructure while
 * providing enhanced accuracy and reduced false positives.
 */
class MotionDetector {
private:
    std::unique_ptr<MotionDetectionManager> motionManager;
    std::unique_ptr<HybridMotionDetector> hybridDetector;
    
    bool initialized;
    float detectionThreshold;
    MotionSensitivity currentSensitivity;
    
    std::vector<MotionEvent> recentEvents;
    std::function<void(const MotionEvent&)> motionCallback;
    
    /**
     * @brief Calculate confidence score for detected motion
     * @param frameBuffer Camera frame buffer
     * @return Confidence score (0.0 to 1.0)
     */
    float calculateConfidence(const camera_fb_t* frameBuffer);
    
    /**
     * @brief Handle motion detection event
     * @param confidence Detection confidence score
     */
    void onMotionDetected(float confidence);

public:
    /**
     * @brief Constructor
     */
    MotionDetector();
    
    /**
     * @brief Destructor
     */
    ~MotionDetector();
    
    /**
     * @brief Initialize the motion detector
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Detect motion in camera frame buffer
     * @param frameBuffer ESP32 camera frame buffer
     * @return true if motion detected, false otherwise
     */
    bool detectMotion(const camera_fb_t* frameBuffer);
    
    /**
     * @brief Detect motion in raw image data
     * @param imageData Raw image data
     * @param dataSize Size of image data
     * @return true if motion detected, false otherwise
     */
    bool detectMotion(const uint8_t* imageData, size_t dataSize);
    
    /**
     * @brief Set motion detection threshold
     * @param threshold Threshold value (0.0 to 1.0)
     */
    void setDetectionThreshold(float threshold);
    
    /**
     * @brief Get current detection threshold
     * @return Current threshold value
     */
    float getDetectionThreshold() const { return detectionThreshold; }
    
    /**
     * @brief Set motion detection sensitivity
     * @param sensitivity Sensitivity level
     */
    void setSensitivity(MotionSensitivity sensitivity);
    
    /**
     * @brief Get current sensitivity setting
     * @return Current sensitivity level
     */
    MotionSensitivity getSensitivity() const { return currentSensitivity; }
    
    /**
     * @brief Set motion detection callback
     * @param callback Function to call when motion is detected
     */
    void setMotionCallback(std::function<void(const MotionEvent&)> callback);
    
    /**
     * @brief Get recent motion events
     * @return Vector of recent motion events
     */
    std::vector<MotionEvent> getRecentEvents() const;
    
    /**
     * @brief Get motion detector statistics
     * @return Statistics structure
     */
    MotionDetectorStats getStatistics() const;
    
    /**
     * @brief Check if detector is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Cleanup and deinitialize
     */
    void cleanup();
};

#endif // MOTION_DETECTOR_H