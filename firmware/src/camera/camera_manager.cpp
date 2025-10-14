/**
 * @file camera_manager.cpp
 * @brief Lightweight camera manager implementation
 */

#include "camera_manager.h"
#include "../../utils/logger.h"

// AI-Thinker ESP32-CAM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

CameraManager::CameraManager() 
    : m_initialized(false), m_currentFrameBuffer(nullptr), m_lastError("") {
    m_mutex = xSemaphoreCreateMutex();
}

CameraManager::~CameraManager() {
    if (m_currentFrameBuffer) {
        esp_camera_fb_return(m_currentFrameBuffer);
        m_currentFrameBuffer = nullptr;
    }
    if (m_mutex) {
        vSemaphoreDelete(m_mutex);
    }
}

camera_config_t CameraManager::getCameraConfig() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565; // RGB565 for AI processing
    config.frame_size = FRAMESIZE_QVGA;     // 320x240 for YOLO-tiny
    config.jpeg_quality = 12;
    config.fb_count = 2;                     // Double buffering
    config.grab_mode = CAMERA_GRAB_LATEST;   // Always get latest frame
    
    return config;
}

bool CameraManager::initialize() {
    if (m_initialized) {
        Logger::warning("Camera already initialized");
        return true;
    }
    
    Logger::info("Initializing camera...");
    
    camera_config_t config = getCameraConfig();
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        setError("Camera init failed");
        Logger::error("Camera init failed: 0x%x", err);
        return false;
    }
    
    // Get sensor and configure
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        setError("Failed to get camera sensor");
        Logger::error("Failed to get camera sensor");
        esp_camera_deinit();
        return false;
    }
    
    configureSensor();
    
    // Test capture to verify camera is working
    camera_fb_t* test_fb = esp_camera_fb_get();
    if (!test_fb) {
        setError("Test capture failed");
        Logger::error("Camera test capture failed");
        esp_camera_deinit();
        return false;
    }
    esp_camera_fb_return(test_fb);
    
    m_initialized = true;
    Logger::info("Camera initialized successfully (QVGA RGB565)");
    return true;
}

void CameraManager::configureSensor() {
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return;
    
    // Wildlife-optimized settings for AI detection
    sensor->set_brightness(sensor, 0);     // -2 to 2
    sensor->set_contrast(sensor, 0);       // -2 to 2
    sensor->set_saturation(sensor, 0);     // -2 to 2
    sensor->set_special_effect(sensor, 0); // No special effects
    sensor->set_whitebal(sensor, 1);       // Enable white balance
    sensor->set_awb_gain(sensor, 1);       // Enable auto white balance gain
    sensor->set_wb_mode(sensor, 0);        // Auto white balance mode
    sensor->set_exposure_ctrl(sensor, 1);  // Enable auto exposure
    sensor->set_aec2(sensor, 1);           // Enable AEC sensor
    sensor->set_ae_level(sensor, 0);       // -2 to 2
    sensor->set_aec_value(sensor, 300);    // 0 to 1200
    sensor->set_gain_ctrl(sensor, 1);      // Enable auto gain
    sensor->set_agc_gain(sensor, 0);       // 0 to 30
    sensor->set_gainceiling(sensor, (gainceiling_t)0); // GAINCEILING_2X
    sensor->set_bpc(sensor, 0);            // Black pixel correction
    sensor->set_wpc(sensor, 1);            // White pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, 1);           // Enable lens correction
    sensor->set_hmirror(sensor, 0);        // No horizontal mirror
    sensor->set_vflip(sensor, 0);          // No vertical flip
    sensor->set_dcw(sensor, 1);            // Enable downsize
    sensor->set_colorbar(sensor, 0);       // Disable color bar test pattern
}

uint8_t* CameraManager::captureFrame(int* width, int* height) {
    if (!m_initialized) {
        setError("Camera not initialized");
        return nullptr;
    }
    
    // Thread-safe capture
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        setError("Failed to acquire camera mutex");
        return nullptr;
    }
    
    // Clear previous frame if exists
    if (m_currentFrameBuffer) {
        esp_camera_fb_return(m_currentFrameBuffer);
        m_currentFrameBuffer = nullptr;
    }
    
    // Capture new frame
    m_currentFrameBuffer = esp_camera_fb_get();
    xSemaphoreGive(m_mutex);
    
    if (!m_currentFrameBuffer) {
        setError("Frame capture failed");
        return nullptr;
    }
    
    if (!m_currentFrameBuffer->buf || m_currentFrameBuffer->len == 0) {
        setError("Invalid frame buffer");
        esp_camera_fb_return(m_currentFrameBuffer);
        m_currentFrameBuffer = nullptr;
        return nullptr;
    }
    
    // Set output parameters
    if (width) *width = m_currentFrameBuffer->width;
    if (height) *height = m_currentFrameBuffer->height;
    
    return m_currentFrameBuffer->buf;
}

void CameraManager::releaseFrame(uint8_t* image_data) {
    if (xSemaphoreTake(m_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if (m_currentFrameBuffer && m_currentFrameBuffer->buf == image_data) {
            esp_camera_fb_return(m_currentFrameBuffer);
            m_currentFrameBuffer = nullptr;
        }
        xSemaphoreGive(m_mutex);
    }
}

void CameraManager::setError(const char* error) {
    m_lastError = String(error);
}
