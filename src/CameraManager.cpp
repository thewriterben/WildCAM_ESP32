#include "CameraManager.h"
#include "config.h"

// Store constant strings in PROGMEM to save RAM
static const char TAG_INIT[] PROGMEM = "Initializing AI-Thinker ESP32-CAM...";
static const char TAG_CONFIG[] PROGMEM = "Configuration: Quality=%d, FrameSize=%d\n";
static const char TAG_ERROR_INIT[] PROGMEM = "ERROR: Camera initialization failed!";
static const char TAG_ERROR_CODE[] PROGMEM = "Error code: 0x%x\n";
static const char TAG_ERROR_INVALID_QUALITY[] PROGMEM = "ERROR: Invalid quality: %d (must be 1-63)\n";
static const char TAG_SUCCESS_INIT[] PROGMEM = "Camera initialized successfully";
static const char TAG_SENSOR_CONFIG[] PROGMEM = "Camera sensor configured: vflip=1, hmirror=1";
static const char TAG_WARN_SENSOR[] PROGMEM = "WARNING: Could not get camera sensor for configuration";
static const char TAG_ERROR_NOT_INIT[] PROGMEM = "ERROR: Camera not initialized!";
static const char TAG_ERROR_CAPTURE_NULL[] PROGMEM = "ERROR: Camera capture failed - frame buffer is NULL";
static const char TAG_ERROR_CAPTURE_ZERO[] PROGMEM = "ERROR: Camera capture failed - buffer length is 0";
static const char TAG_CAPTURE_INFO[] PROGMEM = "Image captured: %d bytes, %dx%d pixels\n";
static const char TAG_DEINIT[] PROGMEM = "Camera deinitialized";

// Error troubleshooting strings in PROGMEM
static const char ERR_INVALID_ARG_REASON[] PROGMEM = "Reason: Invalid argument provided to camera init";
static const char ERR_INVALID_ARG_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Check frame size and quality parameters are valid\n  - Verify pin assignments match your hardware";
static const char ERR_INVALID_STATE_REASON[] PROGMEM = "Reason: Camera driver already initialized or in invalid state";
static const char ERR_INVALID_STATE_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Call deinit() before reinitializing\n  - Reset the ESP32 if problem persists";
static const char ERR_NO_MEM_REASON[] PROGMEM = "Reason: Out of memory - PSRAM may not be available";
static const char ERR_NO_MEM_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Check that PSRAM is enabled in platformio.ini\n  - Try lowering frame size (e.g., FRAMESIZE_SVGA)\n  - Increase JPEG quality value (lower compression)";
static const char ERR_NOT_FOUND_REASON[] PROGMEM = "Reason: Camera sensor not found - check hardware connections";
static const char ERR_NOT_FOUND_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Verify camera module is properly seated\n  - Check I2C pins (SDA/SCL) connections\n  - Ensure camera power supply is stable";
static const char ERR_NOT_SUPPORTED_REASON[] PROGMEM = "Reason: Operation not supported by camera sensor";
static const char ERR_NOT_SUPPORTED_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Verify you're using a compatible camera module\n  - Check frame size is supported by your sensor";
static const char ERR_TIMEOUT_REASON[] PROGMEM = "Reason: Camera initialization timeout - check I2C communication";
static const char ERR_TIMEOUT_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Verify I2C pins (GPIO26/27) are not conflicting\n  - Check for loose connections on camera module\n  - Reduce I2C bus speed if interference is present";
static const char ERR_UNKNOWN_REASON[] PROGMEM = "Reason: Unknown error (0x%x)\n";
static const char ERR_UNKNOWN_TROUBLE[] PROGMEM = "Troubleshooting:\n  - Check all camera pins are correctly connected\n  - Ensure camera module is properly seated\n  - Try a different camera module to isolate hardware issue";

// Status strings in PROGMEM
static const char STATUS_HEADER[] PROGMEM = "=== Camera Manager Status ===";
static const char STATUS_INIT[] PROGMEM = "Initialized: %s\n";
static const char STATUS_QUALITY[] PROGMEM = "JPEG Quality: %d (1-63, lower is higher quality)\n";
static const char STATUS_FRAMESIZE[] PROGMEM = "Frame Size: %d\n";
static const char STATUS_FLASH[] PROGMEM = "Flash Pin: %d\n";
static const char STATUS_SENSOR[] PROGMEM = "Camera Sensor Info:";
static const char STATUS_VFLIP[] PROGMEM = "  Vertical Flip: %s\n";
static const char STATUS_HMIRROR[] PROGMEM = "  Horizontal Mirror: %s\n";
static const char STATUS_CONFIG[] PROGMEM = "Camera Configuration:";
static const char STATUS_XCLK[] PROGMEM = "  XCLK Freq: %d Hz\n";
static const char STATUS_PIXFMT[] PROGMEM = "  Pixel Format: JPEG\n";
static const char STATUS_FBCOUNT[] PROGMEM = "  Frame Buffers: %d\n";
static const char STATUS_PWDN[] PROGMEM = "  PWDN Pin: %d\n";
static const char STATUS_RESET[] PROGMEM = "  RESET Pin: %d\n";
static const char STATUS_FOOTER[] PROGMEM = "=============================";

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
        Serial.printf_P(TAG_ERROR_INVALID_QUALITY, quality);
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
    
    Serial.println(FPSTR(TAG_INIT));
    Serial.printf_P(TAG_CONFIG, quality, size);
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println(FPSTR(TAG_ERROR_INIT));
        Serial.printf_P(TAG_ERROR_CODE, err);
        
        // Print detailed error messages with context-specific troubleshooting
        switch (err) {
            case ESP_ERR_INVALID_ARG:
                Serial.println(FPSTR(ERR_INVALID_ARG_REASON));
                Serial.println(FPSTR(ERR_INVALID_ARG_TROUBLE));
                break;
            case ESP_ERR_INVALID_STATE:
                Serial.println(FPSTR(ERR_INVALID_STATE_REASON));
                Serial.println(FPSTR(ERR_INVALID_STATE_TROUBLE));
                break;
            case ESP_ERR_NO_MEM:
                Serial.println(FPSTR(ERR_NO_MEM_REASON));
                Serial.println(FPSTR(ERR_NO_MEM_TROUBLE));
                break;
            case ESP_ERR_NOT_FOUND:
                Serial.println(FPSTR(ERR_NOT_FOUND_REASON));
                Serial.println(FPSTR(ERR_NOT_FOUND_TROUBLE));
                break;
            case ESP_ERR_NOT_SUPPORTED:
                Serial.println(FPSTR(ERR_NOT_SUPPORTED_REASON));
                Serial.println(FPSTR(ERR_NOT_SUPPORTED_TROUBLE));
                break;
            case ESP_ERR_TIMEOUT:
                Serial.println(FPSTR(ERR_TIMEOUT_REASON));
                Serial.println(FPSTR(ERR_TIMEOUT_TROUBLE));
                break;
            default:
                Serial.printf_P(ERR_UNKNOWN_REASON, err);
                Serial.println(FPSTR(ERR_UNKNOWN_TROUBLE));
                break;
        }
        
        return false;
    }
    
    Serial.println(FPSTR(TAG_SUCCESS_INIT));
    initialized = true;
    
    // Get sensor for additional configuration
    sensor_t* s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_vflip(s, 1);       // Flip vertically
        s->set_hmirror(s, 1);     // Mirror horizontally
        Serial.println(FPSTR(TAG_SENSOR_CONFIG));
    } else {
        Serial.println(FPSTR(TAG_WARN_SENSOR));
    }
    
    return true;
}

camera_fb_t* CameraManager::captureImage() {
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return nullptr;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println(FPSTR(TAG_ERROR_CAPTURE_NULL));
        return nullptr;
    }
    
    // Verify buffer length is valid
    if (fb->len == 0) {
        Serial.println(FPSTR(TAG_ERROR_CAPTURE_ZERO));
        esp_camera_fb_return(fb);
        return nullptr;
    }
    
    // Print capture info with size and dimensions
    Serial.printf_P(TAG_CAPTURE_INFO, fb->len, fb->width, fb->height);
    
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
    Serial.println(FPSTR(STATUS_HEADER));
    Serial.printf_P(STATUS_INIT, initialized ? "Yes" : "No");
    Serial.printf_P(STATUS_QUALITY, jpegQuality);
    Serial.printf_P(STATUS_FRAMESIZE, frameSize);
    Serial.printf_P(STATUS_FLASH, flashPin);
    
    if (initialized) {
        sensor_t* s = esp_camera_sensor_get();
        if (s != NULL) {
            Serial.println(FPSTR(STATUS_SENSOR));
            Serial.printf_P(STATUS_VFLIP, s->status.vflip ? "Yes" : "No");
            Serial.printf_P(STATUS_HMIRROR, s->status.hmirror ? "Yes" : "No");
        }
    }
    
    Serial.println(FPSTR(STATUS_CONFIG));
    Serial.printf_P(STATUS_XCLK, config.xclk_freq_hz);
    Serial.println(FPSTR(STATUS_PIXFMT));
    Serial.printf_P(STATUS_FBCOUNT, config.fb_count);
    Serial.printf_P(STATUS_PWDN, config.pin_pwdn);
    Serial.printf_P(STATUS_RESET, config.pin_reset);
    Serial.println(FPSTR(STATUS_FOOTER));
}

void CameraManager::deinit() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
        Serial.println(FPSTR(TAG_DEINIT));
    }
}
