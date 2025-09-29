/**
 * @file camera_handler.h
 * @brief Main camera handler interface for ESP32WildlifeCAM Step 1.4
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file provides the Camera Driver interface as specified in the
 * Unified Development Plan Step 1.4. It includes conflict-aware initialization
 * and integration with the GPIO manager and board detection system.
 */

#ifndef SRC_CAMERA_HANDLER_H
#define SRC_CAMERA_HANDLER_H

#include <Arduino.h>
#include <esp_camera.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "camera_config.h"

// Forward declarations
class CameraBoard;
struct GPIOMap;

/**
 * @class CameraHandler
 * @brief Critical Camera Driver implementation for Step 1.4
 * 
 * This class implements the camera driver system as outlined in the Unified
 * Development Plan. It provides conflict-aware initialization, PSRAM optimization,
 * and proper frame buffer management to prevent memory leaks.
 */
class CameraHandler {
private:
    camera_config_t config;
    QueueHandle_t frame_queue;
    bool initialized;
    CameraBoard* board_instance;
    
public:
    /**
     * @brief Initialize camera with user configuration
     * @param user_config Camera configuration parameters
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t initialize(const CameraConfig& user_config);
    
    /**
     * @brief Capture frame with timeout
     * @param timeout_ms Maximum time to wait for capture in milliseconds
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t captureFrame(uint32_t timeout_ms);
    
    /**
     * @brief Get frame buffer from queue
     * @return Pointer to frame buffer or nullptr if none available
     */
    camera_fb_t* getFrameBuffer();
    
    /**
     * @brief Return frame buffer to system
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Deinitialize camera and cleanup resources
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t deinitialize();
    
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
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Get initialization result with diagnostic information
     * @return CameraInitResult structure with detailed status
     */
    CameraInitResult getInitializationResult() const;
    
    /**
     * @brief Get capture statistics
     * @return CaptureStats structure with performance metrics
     */
    CaptureStats getCaptureStats() const;
    
    /**
     * @brief Test camera functionality
     * @return true if camera test passed, false otherwise
     */
    bool testCamera();
    
    /**
     * @brief Constructor
     */
    CameraHandler();
    
    /**
     * @brief Destructor
     */
    ~CameraHandler();
    
private:
    // Internal state tracking
    CaptureStats stats;
    CameraInitResult init_result;
    uint32_t last_capture_time;
    
    // Private methods
    esp_err_t setupPins();
    esp_err_t configureSensor();
    bool detectHardwareCapabilities();
    void updateCaptureStats(uint32_t capture_time, size_t image_size, bool success);
    void logDiagnosticInfo() const;
};

#endif // SRC_CAMERA_HANDLER_H