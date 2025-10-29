#include "CameraManager.h"
#include "config.h"

CameraManager::CameraManager() 
    : config(),              // Default-initialize config struct
      initialized(false), 
      jpegQuality(10), 
      frameSize(FRAMESIZE_UXGA),
      flashPin(FLASH_LED_PIN) {
    setupConfig();
}

CameraManager::~CameraManager() {
    deinit();
}

void CameraManager::setupConfig() {
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
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Default settings (will be overridden by init() parameters)
    config.frame_size = frameSize;
    config.jpeg_quality = jpegQuality;
    config.fb_count = 2;
}

bool CameraManager::init(int quality, framesize_t size) {
    // Validate parameters
    if (quality < 1 || quality > 63) {
        Serial.printf("Invalid quality: %d (must be 1-63)\n", quality);
        return false;
    }
    
    // Store parameters
    jpegQuality = quality;
    frameSize = size;
    
    // Update config with parameters
    config.frame_size = frameSize;
    config.jpeg_quality = jpegQuality;
    
    // Initialize flash LED
    pinMode(flashPin, OUTPUT);
    digitalWrite(flashPin, LOW);
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    Serial.println("Camera initialized successfully");
    initialized = true;
    
    // Get sensor for additional configuration
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_vflip(s, 1);       // Flip vertically
        s->set_hmirror(s, 1);     // Mirror horizontally
    }
    
    return true;
}

camera_fb_t* CameraManager::captureImage() {
    if (!initialized) {
        Serial.println("Camera not initialized!");
        return nullptr;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return nullptr;
    }
    
    Serial.printf("Image captured: %d bytes\n", fb->len);
    return fb;
}

void CameraManager::releaseFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool CameraManager::setQuality(int quality) {
    if (quality < 1 || quality > 63) {
        return false;
    }
    
    jpegQuality = quality;
    
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_quality(s, quality);
        return true;
    }
    return false;
}

bool CameraManager::setFrameSize(framesize_t size) {
    frameSize = size;
    
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_framesize(s, size);
        return true;
    }
    return false;
}

void CameraManager::setFlash(bool enable) {
    digitalWrite(flashPin, enable ? HIGH : LOW);
}

bool CameraManager::isInitialized() {
    return initialized;
}

void CameraManager::printStatus() {
    Serial.println("=== Camera Manager Status ===");
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("JPEG Quality: %d (1-63, lower is higher quality)\n", jpegQuality);
    Serial.printf("Frame Size: %d\n", frameSize);
    Serial.printf("Flash Pin: %d\n", flashPin);
    
    if (initialized) {
        sensor_t* s = esp_camera_sensor_get();
        if (s != NULL) {
            Serial.println("Camera Sensor Info:");
            Serial.printf("  Vertical Flip: %s\n", s->status.vflip ? "Yes" : "No");
            Serial.printf("  Horizontal Mirror: %s\n", s->status.hmirror ? "Yes" : "No");
        }
    }
    
    Serial.println("Camera Configuration:");
    Serial.printf("  XCLK Freq: %d Hz\n", config.xclk_freq_hz);
    Serial.printf("  Pixel Format: JPEG\n");
    Serial.printf("  Frame Buffers: %d\n", config.fb_count);
    Serial.printf("  PWDN Pin: %d\n", config.pin_pwdn);
    Serial.printf("  RESET Pin: %d\n", config.pin_reset);
    Serial.println("=============================");
}

void CameraManager::deinit() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
        Serial.println("Camera deinitialized");
    }
}
