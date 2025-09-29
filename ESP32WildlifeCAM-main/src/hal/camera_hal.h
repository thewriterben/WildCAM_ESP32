/**
 * @file camera_hal.h
 * @brief Hardware abstraction layer for camera interfaces
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef CAMERA_HAL_H
#define CAMERA_HAL_H

#include <Arduino.h>
#include <esp_camera.h>
#include "../../include/boards/board_detection.h"

namespace Wildlife {
namespace HAL {

using BoardType = Hardware::BoardType;

/**
 * Image resolution options
 */
enum class Resolution {
    QVGA,   // 320x240
    VGA,    // 640x480
    HD,     // 1280x720
    UXGA    // 1600x1200
};

/**
 * Image quality levels
 */
enum class ImageQuality {
    LOW,     // Fast capture, larger file
    MEDIUM,  // Balanced quality/speed
    HIGH,    // Good quality, slower
    ULTRA    // Best quality, slowest
};

/**
 * Image format enumeration
 */
enum class ImageFormat {
    JPEG,
    RGB565,
    YUV422,
    GRAYSCALE
};

/**
 * Image buffer structure
 */
struct ImageBuffer {
    uint8_t* data = nullptr;
    size_t length = 0;
    ImageFormat format = ImageFormat::JPEG;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t timestamp = 0;
};

/**
 * Camera configuration structure
 */
struct CameraConfig {
    // LEDC configuration
    int ledc_channel = 0;
    int ledc_timer = 0;
    
    // Camera data pins
    int pin_d0 = -1;
    int pin_d1 = -1;
    int pin_d2 = -1;
    int pin_d3 = -1;
    int pin_d4 = -1;
    int pin_d5 = -1;
    int pin_d6 = -1;
    int pin_d7 = -1;
    
    // Camera control pins
    int pin_xclk = -1;
    int pin_pclk = -1;
    int pin_vsync = -1;
    int pin_href = -1;
    int pin_sda = -1;
    int pin_scl = -1;
    int pin_pwdn = -1;
    int pin_reset = -1;
    
    // Camera settings
    int xclk_freq_hz = 20000000;
    int pixel_format = 0; // PIXFORMAT_JPEG
    int frame_size = 0;   // FRAMESIZE_UXGA
    int jpeg_quality = 10;
    int fb_count = 2;
};

/**
 * Hardware Abstraction Layer for Camera
 * 
 * Provides unified camera interface across all ESP32 variants
 * with optimized configurations for wildlife monitoring.
 */
class CameraHAL {
public:
    CameraHAL();
    ~CameraHAL();
    
    // Initialization and configuration
    bool initialize(BoardType board_type);
    bool deinitialize();
    
    // Image capture
    bool captureImage(ImageBuffer& buffer);
    void releaseImageBuffer();
    
    // Camera settings
    bool setResolution(Resolution resolution);
    bool setImageQuality(ImageQuality quality);
    bool enableNightMode(bool enable);
    
    // Status queries
    bool isInitialized() const { return camera_initialized_; }
    BoardType getCurrentBoard() const { return current_board_; }
    Resolution getCurrentResolution() const { return resolution_; }
    ImageQuality getCurrentQuality() const { return image_quality_; }
    
    // Utility functions
    static const char* getBoardName(BoardType type);
    static const char* getResolutionName(Resolution resolution);
    static const char* getQualityName(ImageQuality quality);
    
private:
    bool camera_initialized_;
    BoardType current_board_;
    Resolution resolution_;
    ImageQuality image_quality_;
    
    // Board-specific configurations
    CameraConfig getBoardCameraConfig(BoardType board_type);
    CameraConfig getAIThinkerConfig();
    CameraConfig getESP32S3Config();
    CameraConfig getESP32C3Config();
    CameraConfig getESP32C6Config();
    CameraConfig getESP32S2Config();
    CameraConfig getTTGOConfig();
    CameraConfig getDefaultConfig();
    
    // Camera initialization helpers
    bool initializeCamera(const CameraConfig& config);
    bool configureSensorForWildlife();
};

} // namespace HAL
} // namespace Wildlife

#endif // CAMERA_HAL_H