#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include <Arduino.h>
#include <esp_camera.h>
#include "hal/camera_board.h"
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Lighting condition enumeration
enum LightingCondition {
    LIGHTING_BRIGHT = 0,
    LIGHTING_NORMAL = 1,
    LIGHTING_DIM = 2,
    LIGHTING_DARK = 3
};

// Camera status structure
struct CameraStatus {
    bool initialized;
    bool sensorDetected;
    int imageCount;
    esp_err_t lastError;
    BoardType boardType;
    SensorType sensorType;
    const char* boardName;
    const char* sensorName;
};

/**
 * @class CameraHandler
 * @brief Manages ESP32 camera initialization, configuration, and image capture
 * 
 * This class encapsulates all camera-related functionality for the wildlife
 * monitoring system, providing a clean interface for camera operations.
 */
class CameraHandler {
public:
    /**
     * @brief Constructor
     */
    CameraHandler();

    /**
     * @brief Destructor
     */
    ~CameraHandler();

    /**
     * @brief Initialize camera with optimal settings for wildlife photography
     * Uses HAL for automatic board detection and configuration
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Initialize camera with specific board type
     * @param boardType Specific board type to use
     * @return true if initialization successful, false otherwise
     */
    bool init(BoardType boardType);
    
    /**
     * @brief Initialize camera with user-provided configuration
     * @param user_config Camera configuration to apply
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initialize(const CameraConfig& user_config);
    
    /**
     * @brief Validate pin assignment for conflict-free operation
     * @return true if pin assignments are valid, false otherwise
     */
    bool validatePinAssignment();
    
    /**
     * @brief Initialize camera with conflict checking
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initializeWithConflictCheck();
    
    /**
     * @brief Get the current camera board instance
     * @return Pointer to camera board, or nullptr if not initialized
     */
    CameraBoard* getBoard();
    
    /**
     * @brief Capture a single image
     * @return Camera frame buffer pointer, or nullptr on failure
     */
    camera_fb_t* captureImage();
    
    /**
     * @brief Capture frame with timeout
     * @param timeout_ms Timeout in milliseconds for frame capture
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t captureFrame(uint32_t timeout_ms);
    
    /**
     * @brief Get frame buffer from internal queue
     * @return Camera frame buffer pointer, or nullptr if none available
     */
    camera_fb_t* getFrameBuffer();
    
    /**
     * @brief Return frame buffer to system
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Deinitialize camera and clean up resources
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitialize();
    
    /**
     * @brief Save image to SD card with timestamp and metadata
     * @param fb Camera frame buffer
     * @param folder Target folder path
     * @return Filename of saved image, or empty string on failure
     */
    String saveImage(camera_fb_t* fb, const char* folder);
    
    /**
     * @brief Generate timestamped filename
     * @param folder Target folder path
     * @return Generated filename
     */
    String generateFilename(const char* folder);
    
    /**
     * @brief Save image metadata as JSON
     * @param imageFilename Path to image file
     * @param fb Camera frame buffer
     */
    void saveImageMetadata(const String& imageFilename, camera_fb_t* fb);
    
    /**
     * @brief Get camera status information
     * @return Camera status structure
     */
    CameraStatus getStatus() const;
    
    /**
     * @brief Take a test image and return basic info
     * @return true if test successful, false otherwise
     */
    bool testCamera();
    
    /**
     * @brief Adjust camera settings for different lighting conditions
     * @param condition Lighting condition to optimize for
     */
    void adjustForLighting(LightingCondition condition);
    
    /**
     * @brief Flash camera LED briefly
     */
    void flashLED();
    
    /**
     * @brief Cleanup camera resources
     */
    void cleanup();

    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Get total number of images captured
     * @return Image counter value
     */
    int getImageCount() const { return imageCounter; }

    /**
     * @brief Capture image with environmental awareness
     * @return Camera frame buffer pointer, or nullptr on failure
     */
    camera_fb_t* captureImageWithEnvironmentalAwareness();
    
    /**
     * @brief Adjust camera settings based on environmental conditions
     * @param env_data Environmental data for optimization
     * @return true if settings were adjusted, false otherwise
     */
    bool adjustForEnvironmentalConditions();
    
    /**
     * @brief Check if environmental conditions are suitable for photography
     * @return true if conditions are good, false otherwise
     */
    bool areEnvironmentalConditionsSuitable();

private:
    // Member variables
    camera_config_t camera_config;
    bool initialized;
    int imageCounter;
    std::unique_ptr<CameraBoard> board;
    QueueHandle_t frame_queue;  // Frame queue for buffer management
    
    // Private methods
    bool initializeCamera();
    void configureSensorSettings(sensor_t* sensor);
    void applyConfigurationSettings();
};

#endif // CAMERA_HANDLER_H