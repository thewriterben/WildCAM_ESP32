#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>

/**
 * @brief Simple camera management class for ESP32-CAM
 * 
 * Minimal implementation focused on getting camera working
 * with SystemManager integration.
 */
class CameraManager {
public:
    /**
     * @brief Camera capture result structure
     */
    struct CaptureResult {
        bool success;
        size_t imageSize;
        String filename;
        uint32_t captureTime;
        camera_fb_t* frameBuffer;
    };

    CameraManager() : initialized(false) {}
    
    /**
     * @brief Initialize camera with default configuration
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Capture an image to buffer
     * @return Camera frame buffer or nullptr on failure
     */
    camera_fb_t* captureToBuffer();
    
    /**
     * @brief Return frame buffer to system
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Get camera configuration
     * @return Camera configuration structure
     */
    camera_config_t getCameraConfig();
    
    /**
     * @brief Configure camera sensor settings
     */
    void configureSensor();
    
    /**
     * @brief Optimize camera for wildlife photography
     */
    void optimizeForWildlife();
    
    /**
     * @brief Configure advanced GPIO pins
     */
    void configureAdvancedGPIOs();
    
    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Get current configuration as string
     * @return Configuration description
     */
    String getConfiguration() const;

private:
    bool initialized;
    void resetStatistics();
};

#endif // CAMERA_MANAGER_H