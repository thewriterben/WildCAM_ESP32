#include "CameraManager.h"
#include "config.h"

CameraManager::CameraManager() 
    : config(),
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
    
    // Enable PSRAM for larger frame buffers
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
}

bool CameraManager::init(int quality, framesize_t size) {
    // Validate parameters
    if (quality < 1 || quality > 63) {
        Serial.printf("ERROR: Invalid quality: %d (must be 1-63)\n", quality);
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
    
    Serial.println("Initializing AI-Thinker ESP32-CAM...");
    Serial.printf("Configuration: Quality=%d, FrameSize=%d\n", quality, size);
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println("ERROR: Camera initialization failed!");
        Serial.printf("Error code: 0x%x\n", err);
        
        // Print detailed error messages with context-specific troubleshooting
        switch (err) {
            case ESP_ERR_INVALID_ARG:
                Serial.println("Reason: Invalid argument provided to camera init");
                Serial.println("Troubleshooting:");
                Serial.println("  - Check frame size and quality parameters are valid");
                Serial.println("  - Verify pin assignments match your hardware");
                break;
            case ESP_ERR_INVALID_STATE:
                Serial.println("Reason: Camera driver already initialized or in invalid state");
                Serial.println("Troubleshooting:");
                Serial.println("  - Call deinit() before reinitializing");
                Serial.println("  - Reset the ESP32 if problem persists");
                break;
            case ESP_ERR_NO_MEM:
                Serial.println("Reason: Out of memory - PSRAM may not be available");
                Serial.println("Troubleshooting:");
                Serial.println("  - Check that PSRAM is enabled in platformio.ini");
                Serial.println("  - Try lowering frame size (e.g., FRAMESIZE_SVGA)");
                Serial.println("  - Increase JPEG quality value (lower compression)");
                break;
            case ESP_ERR_NOT_FOUND:
                Serial.println("Reason: Camera sensor not found - check hardware connections");
                Serial.println("Troubleshooting:");
                Serial.println("  - Verify camera module is properly seated");
                Serial.println("  - Check I2C pins (SDA/SCL) connections");
                Serial.println("  - Ensure camera power supply is stable");
                break;
            case ESP_ERR_NOT_SUPPORTED:
                Serial.println("Reason: Operation not supported by camera sensor");
                Serial.println("Troubleshooting:");
                Serial.println("  - Verify you're using a compatible camera module");
                Serial.println("  - Check frame size is supported by your sensor");
                break;
            case ESP_ERR_TIMEOUT:
                Serial.println("Reason: Camera initialization timeout - check I2C communication");
                Serial.println("Troubleshooting:");
                Serial.println("  - Verify I2C pins (GPIO26/27) are not conflicting");
                Serial.println("  - Check for loose connections on camera module");
                Serial.println("  - Reduce I2C bus speed if interference is present");
                break;
            default:
                Serial.printf("Reason: Unknown error (0x%x)\n", err);
                Serial.println("Troubleshooting:");
                Serial.println("  - Check all camera pins are correctly connected");
                Serial.println("  - Ensure camera module is properly seated");
                Serial.println("  - Try a different camera module to isolate hardware issue");
                break;
        }
        
        return false;
    }
    
    Serial.println("Camera initialized successfully");
    initialized = true;
    
    // Get sensor for additional configuration
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_vflip(s, 1);       // Flip vertically
        s->set_hmirror(s, 1);     // Mirror horizontally
        Serial.println("Camera sensor configured: vflip=1, hmirror=1");
    } else {
        Serial.println("WARNING: Could not get camera sensor for configuration");
    }
    
    return true;
}

camera_fb_t* CameraManager::captureImage() {
    if (!initialized) {
        Serial.println("ERROR: Camera not initialized!");
        return nullptr;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("ERROR: Camera capture failed - frame buffer is NULL");
        return nullptr;
    }
    
    // Verify buffer length is valid
    if (fb->len == 0) {
        Serial.println("ERROR: Camera capture failed - buffer length is 0");
        esp_camera_fb_return(fb);
        return nullptr;
    }
    
    // Print capture info with size and dimensions
    Serial.printf("Image captured: %d bytes, %dx%d pixels\n", 
                  fb->len, fb->width, fb->height);
    
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
