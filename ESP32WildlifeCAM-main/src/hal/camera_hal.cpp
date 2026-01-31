/**
 * @file camera_hal.cpp
 * @brief Hardware abstraction layer for camera interfaces
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 * 
 * Unified camera interface supporting all ESP32 variants with
 * optimized settings for wildlife monitoring applications.
 */

#include "camera_hal.h"
#include "../../utils/logger.h"

namespace Wildlife {
namespace HAL {

CameraHAL::CameraHAL() : 
    camera_initialized_(false),
    current_board_(BoardType::UNKNOWN),
    image_quality_(ImageQuality::HIGH),
    resolution_(Resolution::HD) {
}

CameraHAL::~CameraHAL() {
    if (camera_initialized_) {
        deinitialize();
    }
}

bool CameraHAL::initialize(BoardType board_type) {
    current_board_ = board_type;
    
    Logger::info("Initializing camera HAL for board: %s", getBoardName(board_type));
    
    // Get board-specific configuration
    CameraConfig config = getBoardCameraConfig(board_type);
    
    return initializeCamera(config);
}

bool CameraHAL::initializeCamera(const CameraConfig& config) {
    camera_config_t esp_config;
    
    // Convert our config to ESP32 camera config
    esp_config.ledc_channel = config.ledc_channel;
    esp_config.ledc_timer = config.ledc_timer;
    esp_config.pin_d0 = config.pin_d0;
    esp_config.pin_d1 = config.pin_d1;
    esp_config.pin_d2 = config.pin_d2;
    esp_config.pin_d3 = config.pin_d3;
    esp_config.pin_d4 = config.pin_d4;
    esp_config.pin_d5 = config.pin_d5;
    esp_config.pin_d6 = config.pin_d6;
    esp_config.pin_d7 = config.pin_d7;
    esp_config.pin_xclk = config.pin_xclk;
    esp_config.pin_pclk = config.pin_pclk;
    esp_config.pin_vsync = config.pin_vsync;
    esp_config.pin_href = config.pin_href;
    esp_config.pin_sscb_sda = config.pin_sda;
    esp_config.pin_sscb_scl = config.pin_scl;
    esp_config.pin_pwdn = config.pin_pwdn;
    esp_config.pin_reset = config.pin_reset;
    esp_config.xclk_freq_hz = config.xclk_freq_hz;
    esp_config.pixel_format = static_cast<pixformat_t>(config.pixel_format);
    esp_config.frame_size = static_cast<framesize_t>(config.frame_size);
    esp_config.jpeg_quality = config.jpeg_quality;
    esp_config.fb_count = config.fb_count;
    esp_config.fb_location = CAMERA_FB_IN_PSRAM;
    esp_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&esp_config);
    if (err != ESP_OK) {
        Logger::error("Camera initialization failed: 0x%x", err);
        return false;
    }
    
    // Configure sensor for wildlife monitoring
    if (!configureSensorForWildlife()) {
        Logger::warning("Wildlife-specific sensor configuration failed");
    }
    
    camera_initialized_ = true;
    Logger::info("Camera HAL initialized successfully");
    return true;
}

bool CameraHAL::configureSensorForWildlife() {
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        Logger::error("Failed to get camera sensor");
        return false;
    }
    
    // Optimize sensor settings for wildlife photography
    sensor->set_brightness(sensor, 0);     // 0 = normal brightness
    sensor->set_contrast(sensor, 0);       // 0 = normal contrast
    sensor->set_saturation(sensor, 0);     // 0 = normal saturation
    sensor->set_special_effect(sensor, 0); // 0 = no special effect
    sensor->set_whitebal(sensor, 1);       // Enable white balance
    sensor->set_awb_gain(sensor, 1);       // Enable AWB gain
    sensor->set_wb_mode(sensor, 0);        // Auto white balance mode
    sensor->set_exposure_ctrl(sensor, 1);  // Enable exposure control
    sensor->set_aec2(sensor, 0);           // Disable AEC2
    sensor->set_ae_level(sensor, 0);       // Normal AE level
    sensor->set_aec_value(sensor, 300);    // Auto exposure value
    sensor->set_gain_ctrl(sensor, 1);      // Enable gain control
    sensor->set_agc_gain(sensor, 0);       // Auto gain
    sensor->set_gainceiling(sensor, static_cast<gainceiling_t>(6)); // Max gain
    sensor->set_bpc(sensor, 0);            // Disable black pixel correction
    sensor->set_wpc(sensor, 1);            // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, 1);           // Enable lens correction
    sensor->set_hmirror(sensor, 0);        // No horizontal mirror
    sensor->set_vflip(sensor, 0);          // No vertical flip
    sensor->set_dcw(sensor, 1);            // Enable downsize
    sensor->set_colorbar(sensor, 0);       // Disable color bar
    
    Logger::info("Sensor configured for wildlife monitoring");
    return true;
}

CameraHAL::CameraConfig CameraHAL::getBoardCameraConfig(BoardType board_type) {
    CameraConfig config;
    
    switch (board_type) {
        case BoardType::AI_THINKER_CAM:
            return getAIThinkerConfig();
        case BoardType::ESP32_S3_CAM:
            return getESP32S3Config();
        case BoardType::ESP32_C3_CAM:
            return getESP32C3Config();
        case BoardType::ESP32_C6_CAM:
            return getESP32C6Config();
        case BoardType::ESP32_S2_CAM:
            return getESP32S2Config();
        case BoardType::TTGO_T_CAMERA:
            return getTTGOConfig();
        default:
            Logger::warning("Unknown board type, using default config");
            return getDefaultConfig();
    }
}

CameraHAL::CameraConfig CameraHAL::getESP32C3Config() {
    CameraConfig config;
    
    // ESP32-C3 optimized for ultra-low power
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sda = 26;
    config.pin_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 10000000; // Lower frequency for power saving
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_HD);
    config.jpeg_quality = 12; // Lower quality for faster processing
    config.fb_count = 1; // Single buffer for memory conservation
    
    return config;
}

CameraHAL::CameraConfig CameraHAL::getESP32C6Config() {
    CameraConfig config;
    
    // ESP32-C6 optimized for WiFi 6 and high performance
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 11;
    config.pin_d1 = 9;
    config.pin_d2 = 8;
    config.pin_d3 = 10;
    config.pin_d4 = 12;
    config.pin_d5 = 18;
    config.pin_d6 = 17;
    config.pin_d7 = 16;
    config.pin_xclk = 15;
    config.pin_pclk = 13;
    config.pin_vsync = 6;
    config.pin_href = 7;
    config.pin_sda = 5;
    config.pin_scl = 4;
    config.pin_pwdn = 14;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000; // Higher frequency for better quality
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_UXGA);
    config.jpeg_quality = 8; // Higher quality
    config.fb_count = 2; // Double buffering
    
    return config;
}

CameraHAL::CameraConfig CameraHAL::getESP32S2Config() {
    CameraConfig config;
    
    // ESP32-S2 optimized for single-core high performance
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sda = 26;
    config.pin_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 24000000; // High frequency for performance
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_UXGA);
    config.jpeg_quality = 6; // High quality
    config.fb_count = 2; // Double buffering
    
    return config;
}

bool CameraHAL::captureImage(ImageBuffer& buffer) {
    if (!camera_initialized_) {
        Logger::error("Camera not initialized");
        return false;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Logger::error("Camera capture failed");
        return false;
    }
    
    buffer.data = fb->buf;
    buffer.length = fb->len;
    buffer.format = static_cast<ImageFormat>(fb->format);
    buffer.width = fb->width;
    buffer.height = fb->height;
    buffer.timestamp = millis();
    
    Logger::info("Image captured: %zu bytes, %dx%d", buffer.length, buffer.width, buffer.height);
    return true;
}

void CameraHAL::releaseImageBuffer() {
    esp_camera_fb_return(esp_camera_fb_get());
}

bool CameraHAL::setResolution(Resolution resolution) {
    if (!camera_initialized_) {
        return false;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        return false;
    }
    
    framesize_t frame_size;
    switch (resolution) {
        case Resolution::QVGA:  frame_size = FRAMESIZE_QVGA; break;
        case Resolution::VGA:   frame_size = FRAMESIZE_VGA; break;
        case Resolution::HD:    frame_size = FRAMESIZE_HD; break;
        case Resolution::UXGA:  frame_size = FRAMESIZE_UXGA; break;
        default: return false;
    }
    
    int result = sensor->set_framesize(sensor, frame_size);
    if (result == 0) {
        resolution_ = resolution;
        Logger::info("Resolution set to %s", getResolutionName(resolution));
        return true;
    }
    
    return false;
}

bool CameraHAL::setImageQuality(ImageQuality quality) {
    if (!camera_initialized_) {
        return false;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        return false;
    }
    
    int jpeg_quality;
    switch (quality) {
        case ImageQuality::LOW:    jpeg_quality = 20; break;
        case ImageQuality::MEDIUM: jpeg_quality = 12; break;
        case ImageQuality::HIGH:   jpeg_quality = 8; break;
        case ImageQuality::ULTRA:  jpeg_quality = 4; break;
        default: return false;
    }
    
    int result = sensor->set_quality(sensor, jpeg_quality);
    if (result == 0) {
        image_quality_ = quality;
        Logger::info("Image quality set to %s", getQualityName(quality));
        return true;
    }
    
    return false;
}

bool CameraHAL::enableNightMode(bool enable) {
    if (!camera_initialized_) {
        return false;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        return false;
    }
    
    if (enable) {
        // Configure for night/low-light wildlife photography
        sensor->set_gain_ctrl(sensor, 1);
        sensor->set_agc_gain(sensor, 30);  // Maximum gain
        sensor->set_gainceiling(sensor, static_cast<gainceiling_t>(6));
        sensor->set_aec_value(sensor, 1200); // Longer exposure
        sensor->set_ae_level(sensor, 2);     // Bright AE level
        Logger::info("Night mode enabled");
    } else {
        // Return to normal settings
        sensor->set_agc_gain(sensor, 0);     // Auto gain
        sensor->set_aec_value(sensor, 300);  // Normal exposure
        sensor->set_ae_level(sensor, 0);     // Normal AE level
        Logger::info("Night mode disabled");
    }
    
    return true;
}

bool CameraHAL::deinitialize() {
    if (camera_initialized_) {
        esp_camera_deinit();
        camera_initialized_ = false;
        Logger::info("Camera HAL deinitialized");
    }
    return true;
}

// Helper functions for default configurations
CameraHAL::CameraConfig CameraHAL::getAIThinkerConfig() {
    // Implementation for AI-Thinker ESP32-CAM
    CameraConfig config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sda = 26;
    config.pin_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_UXGA);
    config.jpeg_quality = 10;
    config.fb_count = 2;
    return config;
}

CameraHAL::CameraConfig CameraHAL::getESP32S3Config() {
    // ESP32-S3-CAM optimized configuration with enhanced performance
    CameraConfig config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    
    // ESP32-S3-CAM specific pin mapping (matches board_profiles.h ESP32_S3_CAM_GPIO_MAP)
    config.pin_d0 = 15;   // Y2 pin
    config.pin_d1 = 17;   // Y3 pin
    config.pin_d2 = 18;   // Y4 pin
    config.pin_d3 = 16;   // Y5 pin
    config.pin_d4 = 14;   // Y6 pin
    config.pin_d5 = 12;   // Y7 pin
    config.pin_d6 = 11;   // Y8 pin
    config.pin_d7 = 48;   // Y9 pin
    config.pin_xclk = 10; // External clock
    config.pin_pclk = 13; // Pixel clock
    config.pin_vsync = 38; // VSYNC
    config.pin_href = 47;  // HREF
    config.pin_sda = 40;   // I2C SDA (SCCB)
    config.pin_scl = 39;   // I2C SCL (SCCB)
    config.pin_pwdn = -1;  // Power down pin (not used on S3)
    config.pin_reset = -1; // Reset pin (not used on S3)
    
    // ESP32-S3 performance optimizations
    config.xclk_freq_hz = 24000000; // Higher clock for better performance (24MHz)
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_UXGA); // 1600x1200 for quality
    config.jpeg_quality = 8;  // Higher quality with S3 processing power
    config.fb_count = 3;      // Triple buffering with PSRAM for smooth operation
    
    return config;
}

CameraHAL::CameraConfig CameraHAL::getTTGOConfig() {
    // TTGO T-Camera configuration
    CameraConfig config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    
    // TTGO T-Camera specific pin mapping (matches board_profiles.h TTGO_T_CAMERA_GPIO_MAP)
    config.pin_d0 = 5;    // Y2 pin
    config.pin_d1 = 14;   // Y3 pin
    config.pin_d2 = 4;    // Y4 pin
    config.pin_d3 = 15;   // Y5 pin
    config.pin_d4 = 18;   // Y6 pin
    config.pin_d5 = 23;   // Y7 pin
    config.pin_d6 = 36;   // Y8 pin
    config.pin_d7 = 39;   // Y9 pin
    config.pin_xclk = 32; // External clock
    config.pin_pclk = 19; // Pixel clock
    config.pin_vsync = 27; // VSYNC
    config.pin_href = 25;  // HREF
    config.pin_sda = 13;   // I2C SDA (SCCB)
    config.pin_scl = 12;   // I2C SCL (SCCB)
    config.pin_pwdn = 26;  // Power down pin
    config.pin_reset = -1; // Reset pin (not used)
    
    // TTGO T-Camera settings
    config.xclk_freq_hz = 20000000; // 20MHz standard clock
    config.pixel_format = static_cast<int>(PIXFORMAT_JPEG);
    config.frame_size = static_cast<int>(FRAMESIZE_UXGA);
    config.jpeg_quality = 12; // Standard quality
    config.fb_count = 2;      // Double buffering with PSRAM
    
    return config;
}

CameraHAL::CameraConfig CameraHAL::getDefaultConfig() {
    return getAIThinkerConfig();
}

const char* CameraHAL::getBoardName(BoardType type) {
    switch (type) {
        case BoardType::AI_THINKER_CAM: return "AI-Thinker ESP32-CAM";
        case BoardType::ESP32_S3_CAM: return "ESP32-S3-CAM";
        case BoardType::ESP32_C3_CAM: return "ESP32-C3-CAM";
        case BoardType::ESP32_C6_CAM: return "ESP32-C6-CAM";
        case BoardType::ESP32_S2_CAM: return "ESP32-S2-CAM";
        case BoardType::TTGO_T_CAMERA: return "TTGO T-Camera";
        default: return "Unknown";
    }
}

const char* CameraHAL::getResolutionName(Resolution resolution) {
    switch (resolution) {
        case Resolution::QVGA: return "QVGA (320x240)";
        case Resolution::VGA: return "VGA (640x480)";
        case Resolution::HD: return "HD (1280x720)";
        case Resolution::UXGA: return "UXGA (1600x1200)";
        default: return "Unknown";
    }
}

const char* CameraHAL::getQualityName(ImageQuality quality) {
    switch (quality) {
        case ImageQuality::LOW: return "Low";
        case ImageQuality::MEDIUM: return "Medium";
        case ImageQuality::HIGH: return "High";
        case ImageQuality::ULTRA: return "Ultra";
        default: return "Unknown";
    }
}

} // namespace HAL
} // namespace Wildlife