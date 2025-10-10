#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief Camera management class for ESP32-CAM
 * 
 * Handles camera initialization, configuration, image capture,
 * and optimization for wildlife monitoring applications.
 */
class CameraManager {
public:
    /**
     * @brief Camera capture profiles for different use cases
     */
    enum class CameraProfile {
        HIGH_QUALITY,    // Maximum resolution and quality for final captures
        BALANCED,        // Good balance of quality and speed
        FAST_CAPTURE     // Lower quality but fast for motion detection
    };

    /**
     * @brief Camera capture result structure
     */
    struct CaptureResult {
        bool success;
        size_t imageSize;
        String filename;
        uint32_t captureTime;
        camera_fb_t* frameBuffer;
        CameraProfile profileUsed;  // Profile used for this capture
    };

    /**
     * @brief Camera statistics structure
     */
    struct CameraStats {
        uint32_t totalCaptures;
        uint32_t successfulCaptures;
        uint32_t failedCaptures;
        uint32_t averageCaptureTime;
        size_t averageImageSize;
        float successRate;
    };

    /**
     * @brief Burst capture configuration
     */
    struct BurstConfig {
        uint8_t count = BURST_DEFAULT_COUNT;           // Number of images in burst
        uint16_t intervalMs = BURST_DEFAULT_INTERVAL_MS; // Interval between captures (ms)
        uint8_t quality = CAMERA_JPEG_QUALITY;         // JPEG quality for burst images
        bool saveIndividual = true;                     // Save individual images
        bool createSequence = false;                    // Create sequence metadata
    };

    /**
     * @brief Burst capture result
     */
    struct BurstResult {
        bool success;
        uint8_t capturedCount;
        String sequenceId;
        uint32_t totalTime;
        size_t totalSize;
        String folder;
        String filenames[BURST_MAX_COUNT];
    };

    /**
     * @brief Video recording configuration
     */
    struct VideoConfig {
        uint16_t durationSeconds = VIDEO_DEFAULT_DURATION_S; // Recording duration
        uint8_t frameRate = VIDEO_FRAME_RATE;               // Frames per second
        uint8_t quality = VIDEO_QUALITY;                    // Video quality
        framesize_t frameSize = FRAMESIZE_VGA;              // Video frame size
        bool audioEnabled = false;                          // Audio recording (future)
    };

    /**
     * @brief Video recording result
     */
    struct VideoResult {
        bool success;
        String filename;
        uint32_t duration;
        uint32_t frameCount;
        size_t fileSize;
        uint32_t recordingTime;
    };

    /**
     * @brief Time-lapse configuration
     */
    struct TimeLapseConfig {
        uint32_t intervalSeconds = TIMELAPSE_DEFAULT_INTERVAL_S; // Interval between captures
        uint32_t totalDuration = 0;                             // Total duration (0 = indefinite)
        uint16_t maxImages = 1000;                              // Maximum images to capture
        bool autoCleanup = true;                                // Auto-delete old images
        String folder = "/timelapse";                           // Storage folder
    };

    /**
     * @brief Time-lapse result
     */
    struct TimeLapseResult {
        bool success;
        uint32_t capturedImages;
        String sequenceId;
        uint32_t totalDuration;
        size_t totalSize;
        String folder;
    };

    /**
     * @brief Initialize camera system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Capture image and save to SD card
     * @param folder Target folder path
     * @return Capture result with details
     */
    CaptureResult captureImage(const String& folder = IMAGE_FOLDER);

    /**
     * @brief Capture image to frame buffer (no save)
     * @return Frame buffer pointer (must be returned with returnFrameBuffer)
     */
    camera_fb_t* captureToBuffer();

    /**
     * @brief Return frame buffer to free memory
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);

    /**
     * @brief Save frame buffer to file
     * @param fb Frame buffer to save
     * @param folder Target folder
     * @param filename Custom filename (optional)
     * @return Saved filename or empty string on failure
     */
    String saveFrameBuffer(camera_fb_t* fb, const String& folder, const String& filename = "");

    /**
     * @brief Save image metadata with power telemetry
     * @param imageFilename Image filename
     * @param fb Frame buffer
     * @param profile Profile used
     */
    void saveImageMetadata(const String& imageFilename, camera_fb_t* fb, CameraProfile profile);

    /**
     * @brief Configure camera settings for specific conditions
     * @param brightness Brightness adjustment (-2 to 2)
     * @param contrast Contrast adjustment (-2 to 2)
     * @param saturation Saturation adjustment (-2 to 2)
     */
    void configureSensor(int brightness = 0, int contrast = 0, int saturation = 0);

    /**
     * @brief Optimize camera settings for wildlife photography
     * @param lightLevel Current light level (0-100)
     * @param motionLevel Expected motion level (0-100)
     */
    void optimizeForWildlife(int lightLevel = 50, int motionLevel = 50);

    /**
     * @brief Enable/disable night mode optimizations
     * @param enable Enable night mode
     */
    void setNightMode(bool enable);

    /**
     * @brief Set camera profile
     * @param profile Camera profile to use
     */
    void setCameraProfile(CameraProfile profile);

    /**
     * @brief Get current camera profile
     * @return Current camera profile
     */
    CameraProfile getCameraProfile() const { return currentProfile; }

    /**
     * @brief Capture image with specific profile
     * @param profile Profile to use for capture
     * @param folder Target folder path
     * @return Capture result with details
     */
    CaptureResult captureWithProfile(CameraProfile profile, const String& folder = IMAGE_FOLDER);

    /**
     * @brief Capture burst of images
     * @param config Burst capture configuration
     * @param folder Target folder path
     * @return Burst capture result
     */
    BurstResult captureBurst(const BurstConfig& config, const String& folder = IMAGE_FOLDER);

    /**
     * @brief Record video clip
     * @param config Video recording configuration
     * @param folder Target folder path
     * @return Video recording result
     */
    VideoResult recordVideo(const VideoConfig& config, const String& folder = "/videos");

    /**
     * @brief Start time-lapse capture
     * @param config Time-lapse configuration
     * @return Time-lapse result (updated as capture progresses)
     */
    TimeLapseResult startTimeLapse(const TimeLapseConfig& config);

    /**
     * @brief Stop active time-lapse capture
     * @return Final time-lapse result
     */
    TimeLapseResult stopTimeLapse();

    /**
     * @brief Check if time-lapse is currently active
     * @return true if time-lapse is running
     */
    bool isTimeLapseActive() const { return timeLapseActive; }

    /**
     * @brief Update time-lapse (call periodically)
     * @return Updated time-lapse result
     */
    TimeLapseResult updateTimeLapse();

    /**
     * @brief Control IR LED for night vision
     * @param enable Enable IR LED
     */
    void setIRLED(bool enable);

    /**
     * @brief Get current light level from sensor
     * @return Light level (0-100)
     */
    uint8_t getLightLevel() const;

    /**
     * @brief Auto-adjust night mode based on light level
     */
    void autoAdjustNightMode();

    /**
     * @brief Check if camera is initialized and ready
     * @return true if camera ready
     */
    bool isReady() const { return initialized; }

    /**
     * @brief Get camera statistics
     * @return Current camera statistics
     */
    CameraStats getStatistics() const { return stats; }

    /**
     * @brief Reset camera statistics
     */
    void resetStatistics();

    /**
     * @brief Get current camera configuration
     * @return Camera configuration string
     */
    String getConfiguration() const;

    /**
     * @brief Cleanup camera resources
     */
    void cleanup();

private:
    bool initialized = false;
    CameraStats stats = {};
    uint32_t captureCounter = 0;
    bool nightModeEnabled = false;
    bool irLedEnabled = false;
    CameraProfile currentProfile = CameraProfile::BALANCED;  // Default profile
    
    // Time-lapse state
    bool timeLapseActive = false;
    TimeLapseConfig currentTimeLapseConfig = {};
    TimeLapseResult currentTimeLapseResult = {};
    uint32_t lastTimeLapseCapture = 0;

    /**
     * @brief Generate unique filename for image
     * @param folder Target folder
     * @param extension File extension
     * @return Generated filename
     */
    String generateFilename(const String& folder, const String& extension = ".jpg");

    /**
     * @brief Create directory if it doesn't exist
     * @param path Directory path
     * @return true if directory exists or was created
     */
    bool ensureDirectory(const String& path);

    /**
     * @brief Update capture statistics
     * @param success Whether capture was successful
     * @param captureTime Time taken for capture
     * @param imageSize Size of captured image
     */
    void updateStats(bool success, uint32_t captureTime, size_t imageSize);

    /**
     * @brief Configure camera pins for ESP32-CAM
     * @return Camera configuration structure
     */
    camera_config_t getCameraConfig();

    /**
     * @brief Generate burst sequence ID
     * @return Unique sequence identifier
     */
    String generateBurstSequenceId();

    /**
     * @brief Save burst metadata file
     * @param result Burst result with metadata
     * @param folder Target folder
     * @return true if metadata saved successfully
     */
    bool saveBurstMetadata(const BurstResult& result, const String& folder);

    /**
     * @brief Configure GPIO pins for advanced features
     */
    void configureAdvancedGPIOs();

    /**
     * @brief Apply camera profile settings
     * @param profile Profile to apply
     */
    void applyProfile(CameraProfile profile);

    /**
     * @brief Generate video filename with codec extension
     * @param folder Target folder
     * @return Generated video filename
     */
    String generateVideoFilename(const String& folder);

    /**
     * @brief Initialize video encoder (MJPEG)
     * @param config Video configuration
     * @return true if encoder initialized successfully
     */
    bool initVideoEncoder(const VideoConfig& config);

    /**
     * @brief Write video frame to file
     * @param fb Frame buffer
     * @param file Output file handle
     * @return true if frame written successfully
     */
    bool writeVideoFrame(camera_fb_t* fb, File& file);
};

#endif // CAMERA_MANAGER_H