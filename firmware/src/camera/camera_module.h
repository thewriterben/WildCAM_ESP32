/**
 * @file camera_module.h
 * @brief Modular Camera Management System for ESP32 Wildlife CAM
 * @author ESP32 Wildlife CAM Project
 * @date 2025-09-01
 * 
 * This module provides a high-level interface for camera operations,
 * abstracting the complexity of camera initialization, configuration,
 * and image capture for wildlife monitoring applications.
 */

#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include <Arduino.h>
#include <esp_camera.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// Forward declarations
class PowerManager;
class StorageManager;

/**
 * @brief Camera operation result codes
 */
enum CameraResult {
    CAMERA_OK = 0,
    CAMERA_ERROR_INIT_FAILED,
    CAMERA_ERROR_NOT_INITIALIZED,
    CAMERA_ERROR_CAPTURE_FAILED,
    CAMERA_ERROR_NO_FRAME,
    CAMERA_ERROR_TIMEOUT,
    CAMERA_ERROR_INSUFFICIENT_MEMORY,
    CAMERA_ERROR_INVALID_CONFIG
};

/**
 * @brief Camera configuration structure
 */
struct CameraConfig {
    framesize_t frame_size;         ///< Image resolution
    pixformat_t pixel_format;       ///< Pixel format (JPEG recommended)
    uint8_t jpeg_quality;           ///< JPEG quality (1-100, lower = better quality)
    uint8_t fb_count;               ///< Frame buffer count
    bool psram_enabled;             ///< Use PSRAM for frame buffers
    uint32_t xclk_freq_hz;          ///< Camera clock frequency
    
    // Wildlife-specific settings
    bool night_mode;                ///< Enable night mode optimizations
    bool flash_enabled;             ///< Enable flash for night captures
    bool auto_exposure;             ///< Enable automatic exposure
    uint8_t brightness;             ///< Brightness adjustment (-2 to 2)
    uint8_t contrast;               ///< Contrast adjustment (-2 to 2)
    uint8_t saturation;             ///< Saturation adjustment (-2 to 2)
    
    // Performance settings
    bool enable_face_detect;        ///< Enable face detection (not for wildlife)
    bool enable_recognition;        ///< Enable recognition features
    uint32_t capture_timeout_ms;    ///< Maximum time to wait for capture
};

/**
 * @brief Camera statistics and performance metrics
 */
struct CameraStats {
    uint32_t total_captures;        ///< Total number of captures attempted
    uint32_t successful_captures;   ///< Number of successful captures
    uint32_t failed_captures;       ///< Number of failed captures
    uint32_t avg_capture_time_ms;   ///< Average capture time in milliseconds
    uint32_t max_capture_time_ms;   ///< Maximum capture time recorded
    uint32_t total_data_captured;   ///< Total bytes of image data captured
    uint32_t last_capture_size;     ///< Size of last captured image
    unsigned long last_capture_timestamp; ///< Timestamp of last capture
    float avg_image_quality;        ///< Average image quality score
};

/**
 * @brief Image metadata structure
 */
struct ImageMetadata {
    unsigned long timestamp;        ///< Capture timestamp
    uint32_t image_size;           ///< Image size in bytes
    uint16_t width;                ///< Image width in pixels
    uint16_t height;               ///< Image height in pixels
    framesize_t frame_size;        ///< Frame size enum
    uint8_t quality;               ///< JPEG quality used
    bool flash_used;               ///< Whether flash was used
    float battery_voltage;         ///< Battery voltage at capture
    int8_t temperature;            ///< Temperature at capture
    uint8_t motion_trigger_level;  ///< Motion detection level that triggered capture
};

/**
 * @brief Camera Module Class
 * 
 * Provides comprehensive camera management for wildlife monitoring,
 * including automatic configuration, error recovery, and performance optimization.
 */
class CameraModule {
public:
    /**
     * @brief Constructor
     */
    CameraModule();
    
    /**
     * @brief Destructor
     */
    ~CameraModule();
    
    /**
     * @brief Initialize camera module
     * @param config Camera configuration
     * @return CameraResult initialization result
     */
    CameraResult initialize(const CameraConfig& config);
    
    /**
     * @brief Deinitialize camera module
     * @return CameraResult deinitialization result
     */
    CameraResult deinitialize();
    
    /**
     * @brief Check if camera is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return m_initialized; }
    
    /**
     * @brief Capture a single image
     * @param metadata Pointer to store image metadata (optional)
     * @return CameraResult capture result
     */
    CameraResult captureImage(ImageMetadata* metadata = nullptr);
    
    /**
     * @brief Capture burst of images
     * @param count Number of images to capture
     * @param interval_ms Interval between captures in milliseconds
     * @return CameraResult capture result
     */
    CameraResult captureBurst(uint8_t count, uint32_t interval_ms = 500);
    
    /**
     * @brief Get the last captured frame buffer
     * @return Pointer to frame buffer or nullptr if none available
     */
    camera_fb_t* getLastFrameBuffer();
    
    /**
     * @brief Return frame buffer to system
     * @param fb Frame buffer to return
     */
    void returnFrameBuffer(camera_fb_t* fb);
    
    /**
     * @brief Update camera configuration
     * @param config New configuration
     * @return CameraResult update result
     */
    CameraResult updateConfiguration(const CameraConfig& config);
    
    /**
     * @brief Get current camera configuration
     * @return Current configuration
     */
    CameraConfig getCurrentConfiguration() const { return m_config; }
    
    /**
     * @brief Get camera statistics
     * @return Camera performance statistics
     */
    CameraStats getStatistics() const { return m_stats; }
    
    /**
     * @brief Reset camera statistics
     */
    void resetStatistics();
    
    /**
     * @brief Test camera functionality
     * @return true if camera is working properly, false otherwise
     */
    bool testCamera();
    
    /**
     * @brief Enable or disable flash
     * @param enabled Flash enabled state
     */
    void setFlashEnabled(bool enabled);
    
    /**
     * @brief Get flash enabled state
     * @return true if flash is enabled, false otherwise
     */
    bool isFlashEnabled() const { return m_config.flash_enabled; }
    
    /**
     * @brief Set night mode
     * @param enabled Night mode enabled state
     */
    void setNightMode(bool enabled);
    
    /**
     * @brief Get night mode state
     * @return true if night mode is enabled, false otherwise
     */
    bool isNightModeEnabled() const { return m_config.night_mode; }
    
    /**
     * @brief Adjust image quality
     * @param quality JPEG quality (1-100, lower = better quality)
     * @return CameraResult adjustment result
     */
    CameraResult adjustQuality(uint8_t quality);
    
    /**
     * @brief Set frame size
     * @param frame_size New frame size
     * @return CameraResult frame size change result
     */
    CameraResult setFrameSize(framesize_t frame_size);
    
    /**
     * @brief Auto-adjust settings based on lighting conditions
     * @return CameraResult adjustment result
     */
    CameraResult autoAdjustSettings();
    
    /**
     * @brief Set power manager reference (for power-aware operations)
     * @param power_manager Pointer to power manager
     */
    void setPowerManager(PowerManager* power_manager);
    
    /**
     * @brief Set storage manager reference (for automatic storage)
     * @param storage_manager Pointer to storage manager
     */
    void setStorageManager(StorageManager* storage_manager);
    
    /**
     * @brief Enable or disable automatic storage of captured images
     * @param enabled Auto-storage enabled state
     */
    void setAutoStorage(bool enabled) { m_auto_storage = enabled; }
    
    /**
     * @brief Get last error message
     * @return Last error message string
     */
    const char* getLastError() const { return m_last_error; }

private:
    // Configuration and state
    CameraConfig m_config;          ///< Current camera configuration
    bool m_initialized;             ///< Initialization state
    bool m_auto_storage;            ///< Automatic storage enabled
    
    // Statistics
    CameraStats m_stats;            ///< Performance statistics
    
    // Frame buffer management
    QueueHandle_t m_frame_queue;    ///< Frame buffer queue
    SemaphoreHandle_t m_capture_mutex; ///< Capture operation mutex
    camera_fb_t* m_last_frame;      ///< Last captured frame buffer
    
    // External dependencies
    PowerManager* m_power_manager;   ///< Power manager reference
    StorageManager* m_storage_manager; ///< Storage manager reference
    
    // Error handling
    char m_last_error[128];         ///< Last error message
    
    // Private methods
    
    /**
     * @brief Convert camera configuration to esp_camera config
     * @param config Input configuration
     * @param camera_config Output esp_camera configuration
     * @return true if conversion successful, false otherwise
     */
    bool convertToEspCameraConfig(const CameraConfig& config, camera_config_t& camera_config);
    
    /**
     * @brief Configure camera sensor settings
     * @return CameraResult configuration result
     */
    CameraResult configureSensor();
    
    /**
     * @brief Apply wildlife-specific optimizations
     * @return CameraResult optimization result
     */
    CameraResult applyWildlifeOptimizations();
    
    /**
     * @brief Update capture statistics
     * @param capture_time_ms Time taken for capture
     * @param image_size Size of captured image
     * @param success Whether capture was successful
     */
    void updateStatistics(uint32_t capture_time_ms, uint32_t image_size, bool success);
    
    /**
     * @brief Set error message
     * @param error Error message
     */
    void setError(const char* error);
    
    /**
     * @brief Check and manage memory usage
     * @return true if memory is sufficient, false otherwise
     */
    bool checkMemoryAvailability();
    
    /**
     * @brief Perform automatic white balance adjustment
     * @return CameraResult adjustment result
     */
    CameraResult performAutoWhiteBalance();
    
    /**
     * @brief Adjust exposure based on lighting conditions
     * @return CameraResult adjustment result
     */
    CameraResult adjustExposure();
    
    /**
     * @brief Calculate image quality score (basic assessment)
     * @param fb Frame buffer to assess
     * @return Quality score (0-100)
     */
    float calculateImageQuality(camera_fb_t* fb);
    
    /**
     * @brief Store captured image using storage manager
     * @param fb Frame buffer to store
     * @param metadata Image metadata
     * @return true if storage successful, false otherwise
     */
    bool storeImage(camera_fb_t* fb, const ImageMetadata& metadata);
};

/**
 * @brief Get default camera configuration for wildlife monitoring
 * @return Default CameraConfig structure
 */
CameraConfig getDefaultWildlifeConfig();

/**
 * @brief Get night mode optimized configuration
 * @return Night mode CameraConfig structure
 */
CameraConfig getNightModeConfig();

/**
 * @brief Get high quality configuration for detailed captures
 * @return High quality CameraConfig structure
 */
CameraConfig getHighQualityConfig();

/**
 * @brief Get power-optimized configuration for extended deployment
 * @return Power-optimized CameraConfig structure
 */
CameraConfig getPowerOptimizedConfig();

/**
 * @brief Convert CameraResult to human-readable string
 * @param result Camera result code
 * @return Human-readable string description
 */
const char* cameraResultToString(CameraResult result);

#endif // CAMERA_MODULE_H