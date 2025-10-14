/**
 * @file camera_manager.h
 * @brief Lightweight camera manager for WildCAM ESP32 v2.0
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides thread-safe camera capture functionality for AI processing pipeline
 */

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>

/**
 * @class CameraManager
 * @brief Thread-safe camera frame manager for multi-core processing
 */
class CameraManager {
public:
    CameraManager();
    ~CameraManager();
    
    /**
     * @brief Initialize camera with AI-Thinker configuration
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Capture frame to buffer (thread-safe)
     * @param width Output image width
     * @param height Output image height
     * @return Pointer to image data or nullptr on failure
     */
    uint8_t* captureFrame(int* width, int* height);
    
    /**
     * @brief Release frame buffer (must be called after captureFrame)
     * @param image_data Pointer to image data from captureFrame
     */
    void releaseFrame(uint8_t* image_data);
    
    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }
    
    /**
     * @brief Get last error message
     * @return Error string or empty if no error
     */
    String getLastError() const { return m_lastError; }
    
private:
    bool m_initialized;
    camera_fb_t* m_currentFrameBuffer;
    String m_lastError;
    SemaphoreHandle_t m_mutex;
    
    /**
     * @brief Get camera configuration for AI-Thinker board
     * @return Camera configuration structure
     */
    camera_config_t getCameraConfig();
    
    /**
     * @brief Configure sensor for wildlife detection
     */
    void configureSensor();
    
    /**
     * @brief Set error message
     */
    void setError(const char* error);
};

#endif // CAMERA_MANAGER_H
