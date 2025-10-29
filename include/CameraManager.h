/**
 * @file CameraManager.h
 * @brief Camera management class for ESP32-CAM operations
 * 
 * This class provides a comprehensive interface for managing camera operations
 * on the AI-Thinker ESP32-CAM board with RAII-style resource management.
 * Supports configurable image quality settings and various frame sizes.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>

/**
 * @class CameraManager
 * @brief Manages ESP32-CAM camera operations with RAII-style resource management
 * 
 * This class handles camera initialization, configuration, image capture,
 * and resource cleanup for the AI-Thinker ESP32-CAM board. It provides
 * automatic resource management through constructor/destructor pattern.
 * 
 * Key Features:
 * - AI-Thinker ESP32-CAM board support
 * - Configurable JPEG quality (1-63, lower is higher quality)
 * - Multiple frame size support (QVGA to UXGA)
 * - Automatic resource cleanup via RAII
 * - Flash LED control
 * - Status reporting
 * 
 * Example usage:
 * @code
 * CameraManager camera;
 * if (camera.init(10, FRAMESIZE_UXGA)) {
 *     camera_fb_t* fb = camera.captureImage();
 *     if (fb) {
 *         // Process image...
 *         camera.releaseFrameBuffer(fb);
 *     }
 *     camera.printStatus();
 * }
 * @endcode
 */
class CameraManager {
private:
    bool initialized;           ///< Camera initialization status
    camera_config_t config;     ///< Camera hardware configuration
    int jpegQuality;            ///< JPEG quality (1-63, lower is higher quality)
    framesize_t frameSize;      ///< Image resolution/frame size
    int flashPin;               ///< GPIO pin for flash LED
    
    /**
     * @brief Setup camera configuration for AI-Thinker ESP32-CAM
     * 
     * Configures all camera pins and default settings for the AI-Thinker
     * ESP32-CAM board including GPIO assignments, clock frequency, and
     * pixel format.
     */
    void setupConfig();

public:
    /**
     * @brief Default constructor
     * 
     * Initializes the camera manager with default settings.
     * Camera is not active until init() is called.
     */
    CameraManager();
    
    /**
     * @brief Destructor with automatic resource cleanup (RAII)
     * 
     * Automatically deinitializes the camera hardware when the object
     * goes out of scope, ensuring proper resource cleanup.
     */
    ~CameraManager();
    
    /**
     * @brief Initialize camera with specified quality and frame size
     * 
     * @param quality JPEG quality setting (1-63, lower is higher quality, default: 10)
     * @param size Frame size/resolution (default: FRAMESIZE_UXGA)
     * @return true if initialization successful, false otherwise
     * 
     * Initializes the camera hardware with the specified quality and frame size.
     * This must be called before capturing images.
     */
    bool init(int quality = 10, framesize_t size = FRAMESIZE_UXGA);
    
    /**
     * @brief Capture an image and return frame buffer
     * 
     * @return Pointer to camera frame buffer, or nullptr on failure
     * 
     * Captures a single image frame. The caller is responsible for
     * releasing the frame buffer using releaseFrameBuffer().
     */
    camera_fb_t* captureImage();
    
    /**
     * @brief Release a frame buffer back to the camera driver
     * 
     * @param fb Pointer to frame buffer to release
     * 
     * Releases the frame buffer memory back to the camera driver.
     * Always call this after processing captured images.
     */
    void releaseFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Set JPEG quality for captured images
     * 
     * @param quality JPEG quality value (1-63, lower is higher quality)
     * @return true if quality was set successfully, false otherwise
     * 
     * Changes the JPEG compression quality. Lower values produce higher
     * quality images but larger file sizes.
     */
    bool setQuality(int quality);
    
    /**
     * @brief Set frame size/resolution for captured images
     * 
     * @param size Frame size enumeration (e.g., FRAMESIZE_UXGA, FRAMESIZE_VGA)
     * @return true if frame size was set successfully, false otherwise
     * 
     * Changes the image resolution. Higher resolutions produce more detailed
     * images but consume more memory and processing time.
     */
    bool setFrameSize(framesize_t size);
    
    /**
     * @brief Print current camera status to Serial
     * 
     * Outputs detailed status information including initialization state,
     * current quality setting, frame size, and configuration parameters.
     * Useful for debugging and monitoring.
     */
    void printStatus();
    
    /**
     * @brief Enable or disable flash LED
     * 
     * @param enable true to turn flash on, false to turn off
     * 
     * Controls the built-in flash LED for low-light photography.
     */
    void setFlash(bool enable);
    
    /**
     * @brief Check if camera is initialized
     * 
     * @return true if camera is initialized and ready, false otherwise
     */
    bool isInitialized();
    
    /**
     * @brief Manually deinitialize camera to save power
     * 
     * Releases camera resources and powers down the camera module.
     * Camera must be reinitialized with init() before use.
     */
    void deinit();
};

#endif // CAMERA_MANAGER_H
