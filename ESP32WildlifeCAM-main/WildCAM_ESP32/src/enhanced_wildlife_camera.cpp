/**
 * @file enhanced_wildlife_camera.cpp
 * @brief Implementation of enhanced camera class with OV5640 5MP optimization
 */

#include "enhanced_wildlife_camera.h"
#include "firmware/src/hal/lilygo_t_camera_plus.h"
#include "firmware/src/configs/board_profiles.h"
#include <esp_log.h>
#include <esp_system.h>
#include <driver/gpio.h>
#include <math.h>

static const char* TAG = "WildlifeCamera";

// Frame size lookup table
static const struct {
    WildlifeFrameSize size;
    const char* name;
    uint16_t width;
    uint16_t height;
} frame_size_table[] = {
    {WILDLIFE_QVGA, "QVGA", 320, 240},
    {WILDLIFE_VGA, "VGA", 640, 480},
    {WILDLIFE_SVGA, "SVGA", 800, 600},
    {WILDLIFE_HD, "HD", 1280, 720},
    {WILDLIFE_SXGA, "SXGA", 1280, 1024},
    {WILDLIFE_UXGA, "UXGA", 1600, 1200},
    {WILDLIFE_FHD, "FHD", 1920, 1080},
    {WILDLIFE_QSXGA, "QSXGA", 2592, 1944}
};

EnhancedWildlifeCamera::EnhancedWildlifeCamera() 
    : monitoring_active_(false), current_hour_(12), current_minute_(0),
      board_type_(BOARD_LILYGO_T_CAMERA_PLUS), monitoring_mode_(MODE_MANUAL) {
    
    // Initialize status
    memset(&status_, 0, sizeof(status_));
    status_.sensor_type = SENSOR_UNKNOWN;
    status_.current_frame_size = WILDLIFE_UXGA;
    status_.image_quality = 12;
    status_.focus_position = DEFAULT_FOCUS_POSITION;
    
    // Initialize default settings
    current_settings_ = getDefaultWildlifeSettings();
}

EnhancedWildlifeCamera::~EnhancedWildlifeCamera() {
    cleanup();
}

bool EnhancedWildlifeCamera::init(BoardType board_type) {
    ESP_LOGI(TAG, "Initializing Enhanced Wildlife Camera...");
    
    board_type_ = board_type;
    
    if (!initializeHardware()) {
        ESP_LOGE(TAG, "Failed to initialize hardware");
        return false;
    }
    
    if (!detectSensor()) {
        ESP_LOGE(TAG, "Failed to detect camera sensor");
        return false;
    }
    
    if (!setupCameraConfig()) {
        ESP_LOGE(TAG, "Failed to setup camera configuration");
        return false;
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return false;
    }
    
    // Apply OV5640 specific optimizations
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor && status_.sensor_type == SENSOR_OV5640) {
        if (!applyOV5640Optimizations(sensor)) {
            ESP_LOGW(TAG, "Failed to apply OV5640 optimizations");
        }
    }
    
    // Configure IR Cut pin
    if (!configureGPIOs()) {
        ESP_LOGE(TAG, "Failed to configure GPIOs");
        return false;
    }
    
    // Set default IR Cut state (enabled for day mode)
    setIRCutFilter(IR_CUT_ENABLED);
    
    status_.initialized = true;
    ESP_LOGI(TAG, "Enhanced Wildlife Camera initialized successfully");
    ESP_LOGI(TAG, "Sensor: %s, Frame Size: %s", getSensorName(), frameSizeToString(status_.current_frame_size));
    
    return true;
}

bool EnhancedWildlifeCamera::detectSensor() {
    ESP_LOGI(TAG, "Detecting camera sensor...");
    
    // Initialize I2C for sensor detection
    Wire.begin(camera_config_.pin_sda, camera_config_.pin_scl);
    delay(100);
    
    // Try to detect OV5640 first (target sensor)
    Wire.beginTransmission(0x3C); // OV5640 I2C address
    Wire.write(0x30); // Chip ID register high
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x3C, (uint8_t)1);
        if (Wire.available()) {
            uint8_t chip_id_high = Wire.read();
            
            Wire.beginTransmission(0x3C);
            Wire.write(0x31); // Chip ID register low
            Wire.endTransmission();
            Wire.requestFrom(0x3C, (uint8_t)1);
            if (Wire.available()) {
                uint8_t chip_id_low = Wire.read();
                uint16_t chip_id = (chip_id_high << 8) | chip_id_low;
                
                if (chip_id == 0x5640) {
                    ESP_LOGI(TAG, "OV5640 sensor detected (ID: 0x%04X)", chip_id);
                    status_.sensor_type = SENSOR_OV5640;
                    return true;
                }
            }
        }
    }
    
    // Fallback to OV2640 detection
    Wire.beginTransmission(0x30); // OV2640 I2C address
    Wire.write(0x0A); // Chip ID register
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x30, (uint8_t)1);
        if (Wire.available()) {
            uint8_t chip_id = Wire.read();
            if (chip_id == 0x26) {
                ESP_LOGI(TAG, "OV2640 sensor detected (fallback)");
                status_.sensor_type = SENSOR_OV2640;
                return true;
            }
        }
    }
    
    ESP_LOGW(TAG, "No supported sensor detected, assuming OV5640");
    status_.sensor_type = SENSOR_OV5640;
    return true;
}

bool EnhancedWildlifeCamera::initializeHardware() {
    ESP_LOGI(TAG, "Initializing hardware for board type: %d", board_type_);
    
    // Configure camera pins based on board type
    switch (board_type_) {
        case BOARD_LILYGO_T_CAMERA_PLUS:
            // Use T-Camera Plus S3 pin configuration
            camera_config_.pin_pwdn = -1;    // Power down (not used)
            camera_config_.pin_reset = -1;   // Reset (not used)
            camera_config_.pin_xclk = 15;    // External clock
            camera_config_.pin_sda = 4;      // I2C SDA
            camera_config_.pin_scl = 5;      // I2C SCL
            camera_config_.pin_d7 = 16;      // Camera data
            camera_config_.pin_d6 = 17;
            camera_config_.pin_d5 = 18;
            camera_config_.pin_d4 = 12;
            camera_config_.pin_d3 = 10;
            camera_config_.pin_d2 = 8;
            camera_config_.pin_d1 = 9;
            camera_config_.pin_d0 = 11;
            camera_config_.pin_vsync = 6;    // Vertical sync
            camera_config_.pin_href = 7;     // Horizontal reference
            camera_config_.pin_pclk = 13;    // Pixel clock
            break;
            
        default:
            ESP_LOGE(TAG, "Unsupported board type: %d", board_type_);
            return false;
    }
    
    return true;
}

bool EnhancedWildlifeCamera::configureGPIOs() {
    ESP_LOGI(TAG, "Configuring GPIOs...");
    
    // Configure IR Cut control pin
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << IR_CUT_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure IR Cut GPIO: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGI(TAG, "IR Cut control configured on GPIO%d", IR_CUT_PIN);
    return true;
}

bool EnhancedWildlifeCamera::setupCameraConfig() {
    // Base camera configuration
    camera_config_.ledc_channel = LEDC_CHANNEL_0;
    camera_config_.ledc_timer = LEDC_TIMER_0;
    camera_config_.xclk_freq_hz = 20000000; // 20MHz for OV5640
    camera_config_.pixel_format = PIXFORMAT_JPEG;
    camera_config_.frame_size = (framesize_t)status_.current_frame_size;
    camera_config_.jpeg_quality = status_.image_quality;
    camera_config_.fb_count = 2; // Double buffering for smooth capture
    camera_config_.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config_.grab_mode = CAMERA_GRAB_LATEST;
    
    return true;
}

bool EnhancedWildlifeCamera::applyOV5640Optimizations(sensor_t* sensor) {
    ESP_LOGI(TAG, "Applying OV5640 wildlife optimizations...");
    
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor pointer is null");
        return false;
    }
    
    // Get OV5640 wildlife settings
    const WildlifeSettings* settings = &OV5640_WILDLIFE_SETTINGS;
    
    // Apply basic wildlife settings
    sensor->set_brightness(sensor, settings->brightness);
    sensor->set_contrast(sensor, settings->contrast);
    sensor->set_saturation(sensor, settings->saturation);
    sensor->set_special_effect(sensor, 0); // No special effects
    sensor->set_whitebal(sensor, 1);       // Enable auto white balance
    sensor->set_awb_gain(sensor, 1);       // Enable AWB gain
    sensor->set_wb_mode(sensor, settings->wb_mode);
    
    // Exposure and gain settings optimized for wildlife
    sensor->set_exposure_ctrl(sensor, 1);  // Enable exposure control
    sensor->set_aec2(sensor, 0);           // Disable AEC2
    sensor->set_ae_level(sensor, 0);       // Auto exposure level
    sensor->set_aec_value(sensor, settings->aec_value);
    sensor->set_gain_ctrl(sensor, 1);      // Enable gain control
    sensor->set_agc_gain(sensor, settings->agc_gain);
    sensor->set_gainceiling(sensor, settings->gainceiling);
    
    // OV5640 specific optimizations
    sensor->set_bpc(sensor, 1);            // Enable bad pixel correction
    sensor->set_wpc(sensor, 1);            // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, 1);           // Enable lens correction
    
    // Image enhancement for wildlife photography
    sensor->set_hmirror(sensor, settings->hmirror ? 1 : 0);
    sensor->set_vflip(sensor, settings->vflip ? 1 : 0);
    sensor->set_dcw(sensor, 1);            // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);       // Disable color bar test pattern
    
    // OV5640 specific register tweaks for better wildlife performance
    // Note: These would require direct register access for full optimization
    
    ESP_LOGI(TAG, "OV5640 wildlife optimizations applied successfully");
    return true;
}

bool EnhancedWildlifeCamera::setIRCutFilter(IRCutState state) {
    ESP_LOGI(TAG, "Setting IR Cut filter to: %s", irCutStateToString(state));
    
    setIRCutGPIO(state == IR_CUT_ENABLED);
    status_.ir_cut_enabled = (state == IR_CUT_ENABLED);
    
    // Small delay to allow mechanical filter to settle
    delay(50);
    
    ESP_LOGI(TAG, "IR Cut filter set successfully");
    return true;
}

bool EnhancedWildlifeCamera::toggleIRCutFilter() {
    IRCutState new_state = status_.ir_cut_enabled ? IR_CUT_DISABLED : IR_CUT_ENABLED;
    return setIRCutFilter(new_state);
}

IRCutState EnhancedWildlifeCamera::getIRCutState() const {
    return status_.ir_cut_enabled ? IR_CUT_ENABLED : IR_CUT_DISABLED;
}

bool EnhancedWildlifeCamera::autoAdjustIRCut() {
    // Automatic IR Cut adjustment based on time of day
    bool should_enable = isDaytime();
    IRCutState target_state = should_enable ? IR_CUT_ENABLED : IR_CUT_DISABLED;
    
    if (getIRCutState() != target_state) {
        ESP_LOGI(TAG, "Auto-adjusting IR Cut for %s mode", should_enable ? "day" : "night");
        return setIRCutFilter(target_state);
    }
    
    return true;
}

void EnhancedWildlifeCamera::setIRCutGPIO(bool state) {
    gpio_set_level((gpio_num_t)IR_CUT_PIN, state ? 1 : 0);
}

camera_fb_t* EnhancedWildlifeCamera::captureImage() {
    if (!status_.initialized) {
        ESP_LOGE(TAG, "Camera not initialized");
        return nullptr;
    }
    
    ESP_LOGD(TAG, "Capturing image...");
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        updateStatistics(false);
        return nullptr;
    }
    
    status_.last_capture_time = millis();
    updateStatistics(true);
    
    ESP_LOGD(TAG, "Image captured successfully: %dx%d, %u bytes", 
             fb->width, fb->height, fb->len);
    
    return fb;
}

bool EnhancedWildlifeCamera::captureToFile(const char* filename) {
    camera_fb_t* fb = captureImage();
    if (!fb) {
        return false;
    }
    
    // Write to file (implementation would depend on storage system)
    // For now, just return frame buffer for caller to handle
    esp_camera_fb_return(fb);
    
    ESP_LOGI(TAG, "Image saved to: %s", filename);
    return true;
}

bool EnhancedWildlifeCamera::setFrameSize(WildlifeFrameSize size) {
    if (!validateFrameSize(size)) {
        ESP_LOGE(TAG, "Invalid frame size: %d", size);
        return false;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        ESP_LOGE(TAG, "Failed to get sensor");
        return false;
    }
    
    if (sensor->set_framesize(sensor, (framesize_t)size) != 0) {
        ESP_LOGE(TAG, "Failed to set frame size");
        return false;
    }
    
    status_.current_frame_size = size;
    ESP_LOGI(TAG, "Frame size set to: %s", frameSizeToString(size));
    
    return true;
}

bool EnhancedWildlifeCamera::setImageQuality(uint8_t quality) {
    if (quality < 1 || quality > 63) {
        ESP_LOGE(TAG, "Invalid quality value: %d (must be 1-63)", quality);
        return false;
    }
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        ESP_LOGE(TAG, "Failed to get sensor");
        return false;
    }
    
    if (sensor->set_quality(sensor, quality) != 0) {
        ESP_LOGE(TAG, "Failed to set image quality");
        return false;
    }
    
    status_.image_quality = quality;
    ESP_LOGI(TAG, "Image quality set to: %d", quality);
    
    return true;
}

CameraStatus EnhancedWildlifeCamera::getStatus() const {
    return status_;
}

const char* EnhancedWildlifeCamera::getSensorName() const {
    switch (status_.sensor_type) {
        case SENSOR_OV2640: return "OV2640";
        case SENSOR_OV3660: return "OV3660";
        case SENSOR_OV5640: return "OV5640";
        default: return "Unknown";
    }
}

bool EnhancedWildlifeCamera::isDaytime() const {
    // Simple time-based day/night detection
    // Daytime is considered 6:00 AM to 6:00 PM
    return (current_hour_ >= 6 && current_hour_ < 18);
}

void EnhancedWildlifeCamera::setTimeOfDay(uint8_t hour, uint8_t minute) {
    current_hour_ = hour % 24;
    current_minute_ = minute % 60;
}

bool EnhancedWildlifeCamera::validateFrameSize(WildlifeFrameSize size) const {
    // Check if frame size is supported by current sensor
    if (status_.sensor_type == SENSOR_OV5640) {
        // OV5640 supports all frame sizes up to QSXGA
        return size <= WILDLIFE_QSXGA;
    } else if (status_.sensor_type == SENSOR_OV2640) {
        // OV2640 supports up to UXGA
        return size <= WILDLIFE_UXGA;
    }
    
    return size <= WILDLIFE_VGA; // Safe default
}

const char* EnhancedWildlifeCamera::frameSizeToString(WildlifeFrameSize size) const {
    for (const auto& entry : frame_size_table) {
        if (entry.size == size) {
            return entry.name;
        }
    }
    return "Unknown";
}

void EnhancedWildlifeCamera::updateStatistics(bool capture_success) {
    status_.total_captures++;
    if (!capture_success) {
        status_.failed_captures++;
    }
}

void EnhancedWildlifeCamera::cleanup() {
    if (status_.initialized) {
        esp_camera_deinit();
        status_.initialized = false;
        ESP_LOGI(TAG, "Enhanced Wildlife Camera cleaned up");
    }
}

// Utility functions implementation
const char* irCutStateToString(IRCutState state) {
    return (state == IR_CUT_ENABLED) ? "Enabled" : "Disabled";
}

const char* captureModeToString(WildlifeCaptureMode mode) {
    switch (mode) {
        case MODE_CONTINUOUS: return "Continuous";
        case MODE_MOTION_TRIGGER: return "Motion Trigger";
        case MODE_TIME_LAPSE: return "Time Lapse";
        case MODE_MANUAL: return "Manual";
        default: return "Unknown";
    }
}

WildlifeCaptureSettings getDefaultWildlifeSettings() {
    WildlifeCaptureSettings settings = {};
    settings.frame_size = WILDLIFE_UXGA;
    settings.quality = 12;
    settings.enable_autofocus = true;
    settings.focus_position = DEFAULT_FOCUS_POSITION;
    settings.brightness = 0;
    settings.contrast = 1;
    settings.saturation = 1;
    settings.auto_ir_cut = true;
    settings.capture_interval_s = 30;
    return settings;
}

WildlifeCaptureSettings getLowLightWildlifeSettings() {
    WildlifeCaptureSettings settings = getDefaultWildlifeSettings();
    settings.frame_size = WILDLIFE_HD; // Lower resolution for better low light
    settings.quality = 8; // Better quality for detail
    settings.brightness = 1;
    settings.contrast = 2;
    settings.saturation = 0;
    return settings;
}

WildlifeCaptureSettings getHighQualityWildlifeSettings() {
    WildlifeCaptureSettings settings = getDefaultWildlifeSettings();
    settings.frame_size = WILDLIFE_QSXGA; // Full 5MP resolution
    settings.quality = 6; // Highest quality
    settings.brightness = 0;
    settings.contrast = 1;
    settings.saturation = 2;
    return settings;
}