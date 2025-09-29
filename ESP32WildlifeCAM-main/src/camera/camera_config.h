/**
 * @file camera_config.h
 * @brief Camera configuration structures and definitions
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file defines configuration structures and types for camera operations
 * as part of the Camera Driver Implementation Step 1.4
 */

#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#include <esp_camera.h>
#include <Arduino.h>

/**
 * Camera configuration structure for wildlife monitoring
 * Extends the standard esp_camera config with wildlife-specific settings
 */
struct CameraConfig {
    // Basic camera settings
    framesize_t max_framesize;      ///< Maximum supported frame size
    pixformat_t pixel_format;       ///< Pixel format (JPEG, RGB565, etc.)
    uint8_t jpeg_quality;           ///< JPEG quality (1-63, lower = higher quality)
    uint8_t fb_count;               ///< Frame buffer count
    uint32_t xclk_freq_hz;          ///< Clock frequency
    camera_grab_mode_t grab_mode;   ///< Frame grab mode
    
    // Hardware requirements
    bool psram_required;            ///< PSRAM requirement
    bool has_flash;                 ///< Flash capability
    
    // Wildlife-specific settings
    bool motion_detection_enabled;  ///< Enable motion detection optimization
    bool night_mode;                ///< Night mode for low-light conditions
    bool burst_mode;                ///< Burst capture mode
    uint8_t capture_interval_sec;   ///< Interval between captures in seconds
    
    // Power management
    bool low_power_mode;            ///< Enable low power mode
    uint8_t sleep_delay_sec;        ///< Sleep delay after capture
    
    // Storage preferences
    bool save_metadata;             ///< Save image metadata as JSON
    bool compress_images;           ///< Enable additional compression
    
    /**
     * Default configuration for AI-Thinker ESP32-CAM
     */
    static CameraConfig getDefaultAIThinkerConfig() {
        CameraConfig config;
        config.max_framesize = FRAMESIZE_UXGA;  // 1600x1200
        config.pixel_format = PIXFORMAT_JPEG;
        config.jpeg_quality = 10;               // High quality
        config.fb_count = 2;                    // Double buffering
        config.xclk_freq_hz = 20000000;         // 20MHz
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.psram_required = false;          // Works without PSRAM
        config.has_flash = true;
        config.motion_detection_enabled = true;
        config.night_mode = false;
        config.burst_mode = false;
        config.capture_interval_sec = 0;        // On-demand
        config.low_power_mode = true;
        config.sleep_delay_sec = 5;
        config.save_metadata = true;
        config.compress_images = false;
        return config;
    }
    
    /**
     * High performance configuration for boards with PSRAM
     */
    static CameraConfig getHighPerformanceConfig() {
        CameraConfig config;
        config.max_framesize = FRAMESIZE_QXGA;  // 2048x1536
        config.pixel_format = PIXFORMAT_JPEG;
        config.jpeg_quality = 8;                // Very high quality
        config.fb_count = 3;                    // Triple buffering
        config.xclk_freq_hz = 24000000;         // 24MHz
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.psram_required = true;
        config.has_flash = true;
        config.motion_detection_enabled = true;
        config.night_mode = false;
        config.burst_mode = true;
        config.capture_interval_sec = 0;
        config.low_power_mode = false;
        config.sleep_delay_sec = 1;
        config.save_metadata = true;
        config.compress_images = false;
        return config;
    }
    
    /**
     * Low power configuration for battery operation
     */
    static CameraConfig getLowPowerConfig() {
        CameraConfig config;
        config.max_framesize = FRAMESIZE_SVGA;  // 800x600
        config.pixel_format = PIXFORMAT_JPEG;
        config.jpeg_quality = 15;               // Lower quality for smaller files
        config.fb_count = 1;                    // Single buffer
        config.xclk_freq_hz = 10000000;         // 10MHz - lower power
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.psram_required = false;
        config.has_flash = false;
        config.motion_detection_enabled = true;
        config.night_mode = true;
        config.burst_mode = false;
        config.capture_interval_sec = 60;       // 1 minute intervals
        config.low_power_mode = true;
        config.sleep_delay_sec = 30;
        config.save_metadata = false;           // Skip metadata to save power
        config.compress_images = true;
        return config;
    }
};

/**
 * Camera initialization result structure
 */
struct CameraInitResult {
    esp_err_t error_code;           ///< ESP error code
    bool initialized;               ///< Initialization success flag
    const char* error_message;      ///< Human-readable error message
    uint32_t init_time_ms;          ///< Initialization time in milliseconds
    
    // Hardware detection results
    bool psram_detected;            ///< PSRAM availability
    size_t psram_size;              ///< PSRAM size in bytes
    bool sensor_detected;           ///< Camera sensor detection
    const char* sensor_name;        ///< Detected sensor name
    const char* board_name;         ///< Detected board name
};

/**
 * Frame capture statistics
 */
struct CaptureStats {
    uint32_t total_captures;        ///< Total number of captures
    uint32_t successful_captures;   ///< Successful captures
    uint32_t failed_captures;       ///< Failed captures
    uint32_t avg_capture_time_ms;   ///< Average capture time
    uint32_t min_capture_time_ms;   ///< Minimum capture time
    uint32_t max_capture_time_ms;   ///< Maximum capture time
    uint32_t last_capture_time_ms;  ///< Last capture time
    size_t avg_image_size;          ///< Average image size in bytes
    size_t total_data_captured;     ///< Total data captured in bytes
};

#endif // CAMERA_CONFIG_H