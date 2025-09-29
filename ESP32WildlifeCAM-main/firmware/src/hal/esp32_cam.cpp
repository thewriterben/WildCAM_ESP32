/**
 * @file esp32_cam.cpp
 * @brief AI-Thinker ESP32-CAM board implementation
 */

#include "esp32_cam.h"
#include "../config.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

ESP32CAM::ESP32CAM() {
    board_type = BOARD_AI_THINKER_ESP32_CAM;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool ESP32CAM::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing AI-Thinker ESP32-CAM...");
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Detect camera sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect camera sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("AI-Thinker ESP32-CAM initialized successfully");
    return true;
}

bool ESP32CAM::detectSensor() {
    // For AI-Thinker ESP32-CAM, typically OV2640
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap ESP32CAM::getGPIOMap() const {
    GPIOMap gpio_map;
    
    // AI-Thinker ESP32-CAM pin definitions
    gpio_map.pwdn_pin = 32;
    gpio_map.reset_pin = -1;
    gpio_map.xclk_pin = 0;
    gpio_map.siod_pin = 26;
    gpio_map.sioc_pin = 27;
    
    gpio_map.y9_pin = 35;
    gpio_map.y8_pin = 34;
    gpio_map.y7_pin = 39;
    gpio_map.y6_pin = 36;
    gpio_map.y5_pin = 21;
    gpio_map.y4_pin = 19;
    gpio_map.y3_pin = 18;
    gpio_map.y2_pin = 5;
    gpio_map.vsync_pin = 25;
    gpio_map.href_pin = 23;
    gpio_map.pclk_pin = 22;
    
    gpio_map.led_pin = 4;
    gpio_map.flash_pin = 4; // Same as LED for AI-Thinker
    
    return gpio_map;
}

CameraConfig ESP32CAM::getCameraConfig() const {
    CameraConfig config;
    
    config.max_framesize = FRAMESIZE_UXGA; // 1600x1200
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = 12;
    config.fb_count = hasPSRAM() ? 2 : 1;
    config.xclk_freq_hz = 20000000;
    config.psram_required = false; // Works with or without PSRAM
    config.has_flash = true;
    
    return config;
}

PowerProfile ESP32CAM::getPowerProfile() const {
    PowerProfile profile;
    
    profile.sleep_current_ua = 10000;    // 10mA in light sleep
    profile.active_current_ma = 120;     // 120mA active
    profile.camera_current_ma = 200;     // 200mA with camera
    profile.has_external_power = false;  // No built-in power management
    profile.supports_deep_sleep = true;  // ESP32 supports deep sleep
    profile.min_voltage = 3.0;           // 3.0V minimum
    profile.max_voltage = 3.6;           // 3.6V maximum
    
    return profile;
}

DisplayProfile ESP32CAM::getDisplayProfile() const {
    return NO_DISPLAY_PROFILE;  // ESP32-CAM has no built-in display
}

bool ESP32CAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    DEBUG_PRINTLN("Configuring camera sensor for wildlife photography...");
    
    configureWildlifeSettings(sensor);
    return true;
}

bool ESP32CAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void ESP32CAM::flashLED(bool state) {
    GPIOMap gpio_map = getGPIOMap();
    if (gpio_map.led_pin >= 0) {
        digitalWrite(gpio_map.led_pin, state ? HIGH : LOW);
    }
}

void ESP32CAM::enterSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power down camera
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, HIGH);
    }
    
    DEBUG_PRINTLN("ESP32-CAM entering sleep mode");
}

void ESP32CAM::exitSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power up camera
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, LOW);
        delay(100); // Allow camera to stabilize
    }
    
    DEBUG_PRINTLN("ESP32-CAM exiting sleep mode");
}

float ESP32CAM::getBatteryVoltage() {
    // AI-Thinker doesn't have built-in battery monitoring
    // This would need external voltage divider
    return 3.3; // Placeholder
}

bool ESP32CAM::hasPSRAM() const {
    return psramFound();
}

uint32_t ESP32CAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* ESP32CAM::getChipModel() const {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    switch(chip_info.model) {
        case CHIP_ESP32: return "ESP32";
        case CHIP_ESP32S2: return "ESP32-S2";
        case CHIP_ESP32S3: return "ESP32-S3";
        case CHIP_ESP32C3: return "ESP32-C3";
        default: return "Unknown";
    }
}

void ESP32CAM::configureWildlifeSettings(sensor_t* sensor) {
    // Get sensor-specific wildlife settings
    const WildlifeSettings* settings = getWildlifeSettings(sensor_type);
    if (!settings) {
        DEBUG_PRINTLN("No specific settings found, using default OV2640 settings");
        settings = &OV2640_WILDLIFE_SETTINGS;
    }
    
    DEBUG_PRINTF("Applying %s wildlife settings\n", getSensorCapabilities(sensor_type)->name);
    
    // Apply wildlife-optimized settings
    sensor->set_brightness(sensor, settings->brightness);
    sensor->set_contrast(sensor, settings->contrast);
    sensor->set_saturation(sensor, settings->saturation);
    sensor->set_special_effect(sensor, 0); // 0=No Effect
    sensor->set_whitebal(sensor, 1);       // Enable auto white balance
    sensor->set_awb_gain(sensor, 1);       // Enable AWB gain
    sensor->set_wb_mode(sensor, settings->wb_mode);
    
    // Exposure and gain settings optimized for sensor
    sensor->set_exposure_ctrl(sensor, 1);  // Enable exposure control
    sensor->set_aec2(sensor, 0);           // Disable AEC2
    sensor->set_ae_level(sensor, 0);       // Auto exposure level
    sensor->set_aec_value(sensor, settings->aec_value);
    sensor->set_gain_ctrl(sensor, 1);      // Enable gain control
    sensor->set_agc_gain(sensor, settings->agc_gain);
    sensor->set_gainceiling(sensor, settings->gainceiling);
    
    // Image enhancement for wildlife photography
    sensor->set_bpc(sensor, 0);            // Disable bad pixel correction (can cause artifacts)
    sensor->set_wpc(sensor, 1);            // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, 1);           // Enable lens correction
    
    // Orientation settings
    sensor->set_hmirror(sensor, settings->hmirror ? 1 : 0);
    sensor->set_vflip(sensor, settings->vflip ? 1 : 0);
    sensor->set_dcw(sensor, 1);            // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);       // Disable color bar test pattern
    
    DEBUG_PRINTLN("Wildlife-optimized sensor settings applied");
}

bool ESP32CAM::testSensorCommunication() {
    // Test I2C communication with camera sensor
    Wire.begin(SIOD_GPIO_NUM, SIOC_GPIO_NUM);
    delay(100); // Allow I2C to stabilize
    
    // Try to communicate with OV2640 sensor (default for AI-Thinker ESP32-CAM)
    const uint8_t OV2640_I2C_ADDR = 0x30;
    const uint8_t OV2640_CHIP_ID_HIGH = 0x0A;
    const uint8_t OV2640_CHIP_ID_LOW = 0x0B;
    
    // Read chip ID to verify sensor communication
    Wire.beginTransmission(OV2640_I2C_ADDR);
    Wire.write(OV2640_CHIP_ID_HIGH);
    if (Wire.endTransmission() != 0) {
        DEBUG_PRINTLN("Failed to write to sensor register");
        return false;
    }
    
    Wire.requestFrom(OV2640_I2C_ADDR, (uint8_t)1);
    if (Wire.available() < 1) {
        DEBUG_PRINTLN("No response from sensor");
        return false;
    }
    
    uint8_t chip_id_high = Wire.read();
    
    Wire.beginTransmission(OV2640_I2C_ADDR);
    Wire.write(OV2640_CHIP_ID_LOW);
    if (Wire.endTransmission() != 0) {
        DEBUG_PRINTLN("Failed to read chip ID low");
        return false;
    }
    
    Wire.requestFrom(OV2640_I2C_ADDR, (uint8_t)1);
    if (Wire.available() < 1) {
        DEBUG_PRINTLN("No response for chip ID low");
        return false;
    }
    
    uint8_t chip_id_low = Wire.read();
    
    // OV2640 chip ID should be 0x26XX
    if (chip_id_high == 0x26) {
        DEBUG_PRINTF("OV2640 sensor detected (ID: 0x%02X%02X)\n", chip_id_high, chip_id_low);
        return true;
    } else {
        DEBUG_PRINTF("Unexpected sensor chip ID: 0x%02X%02X\n", chip_id_high, chip_id_low);
        // Still return true as other sensors might work
        return true;
    }
}

void ESP32CAM::setupCameraPins() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Setup LED pin
    if (gpio_map.led_pin >= 0) {
        pinMode(gpio_map.led_pin, OUTPUT);
        digitalWrite(gpio_map.led_pin, LOW);
    }
    
    // Setup power down pin
    if (gpio_map.pwdn_pin >= 0) {
        pinMode(gpio_map.pwdn_pin, OUTPUT);
        digitalWrite(gpio_map.pwdn_pin, LOW); // Camera powered on
    }
}

void ESP32CAM::setupPowerPins() {
    // AI-Thinker doesn't have special power management pins
    // Power management would be handled externally
}