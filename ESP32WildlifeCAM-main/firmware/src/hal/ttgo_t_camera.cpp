/**
 * @file ttgo_t_camera.cpp
 * @brief TTGO T-Camera board implementation
 */

#include "ttgo_t_camera.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

TTGOTCAMERA::TTGOTCAMERA() {
    board_type = BOARD_TTGO_T_CAMERA;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool TTGOTCAMERA::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing TTGO T-Camera...");
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Setup display interface
    setupDisplayInterface();
    
    // Detect camera sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect camera sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("TTGO T-Camera initialized successfully");
    return true;
}

bool TTGOTCAMERA::detectSensor() {
    // TTGO T-Camera typically uses OV2640 or OV3660 sensor
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap TTGOTCAMERA::getGPIOMap() const {
    return TTGO_T_CAMERA_GPIO_MAP;
}

CameraConfig TTGOTCAMERA::getCameraConfig() const {
    return TTGO_T_CAMERA_CAMERA_CONFIG;
}

PowerProfile TTGOTCAMERA::getPowerProfile() const {
    return TTGO_T_CAMERA_POWER_PROFILE;
}

DisplayProfile TTGOTCAMERA::getDisplayProfile() const {
    return TTGO_T_CAMERA_DISPLAY_PROFILE;
}

bool TTGOTCAMERA::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    configureWildlifeSettings(sensor);
    
    // TTGO T-Camera specific optimizations for outdoor use
    sensor->set_brightness(sensor, 0);    // Brightness
    sensor->set_contrast(sensor, 0);      // Contrast
    sensor->set_saturation(sensor, 0);    // Saturation
    sensor->set_sharpness(sensor, 1);     // Slight sharpening
    sensor->set_denoise(sensor, 1);       // Enable denoise
    sensor->set_gainceiling(sensor, GAINCEILING_4X);
    sensor->set_quality(sensor, 12);      // JPEG quality
    sensor->set_colorbar(sensor, 0);      // Disable color bar
    sensor->set_whitebal(sensor, 1);      // Auto white balance
    sensor->set_gain_ctrl(sensor, 1);     // Auto gain control
    sensor->set_exposure_ctrl(sensor, 1); // Auto exposure
    sensor->set_hmirror(sensor, 0);       // Horizontal mirror
    sensor->set_vflip(sensor, 0);         // Vertical flip
    
    return true;
}

bool TTGOTCAMERA::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void TTGOTCAMERA::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void TTGOTCAMERA::enterSleepMode() {
    DEBUG_PRINTLN("TTGO T-Camera entering sleep mode");
    
    // Configure wake-up source
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1); // PIR sensor
    esp_sleep_enable_timer_wakeup(300 * 1000000); // 5 minutes
    
    // Turn off camera power
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, HIGH);
    }
    
    esp_deep_sleep_start();
}

void TTGOTCAMERA::exitSleepMode() {
    DEBUG_PRINTLN("TTGO T-Camera exiting sleep mode");
    
    // Power up camera
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, LOW);
        delay(100); // Allow camera to stabilize
    }
}

float TTGOTCAMERA::getBatteryVoltage() {
    // TTGO T-Camera battery voltage monitoring
    int raw = analogRead(35); // Battery voltage pin
    return (raw * 3.3 * 2.0) / 4095.0; // Convert to voltage with divider
}

bool TTGOTCAMERA::hasPSRAM() const {
    return psramFound();
}

uint32_t TTGOTCAMERA::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* TTGOTCAMERA::getChipModel() const {
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

void TTGOTCAMERA::configureWildlifeSettings(sensor_t* sensor) {
    // Optimize for outdoor wildlife photography
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);  // High resolution
    sensor->set_quality(sensor, 12);                // Balance quality vs file size
    sensor->set_brightness(sensor, 0);              // Neutral brightness
    sensor->set_contrast(sensor, 0);                // Neutral contrast
    sensor->set_saturation(sensor, 0);              // Neutral saturation
    sensor->set_sharpness(sensor, 1);               // Slight sharpening for detail
    sensor->set_denoise(sensor, 1);                 // Enable noise reduction
    sensor->set_gainceiling(sensor, GAINCEILING_16X); // Higher gain for low light
    sensor->set_agc_gain(sensor, 0);                // Auto gain
    sensor->set_aec_value(sensor, 0);               // Auto exposure
    sensor->set_aec2(sensor, 0);                    // AEC DSP
    sensor->set_ae_level(sensor, 0);                // AE level
    sensor->set_awb_gain(sensor, 1);                // Auto white balance gain
    sensor->set_wb_mode(sensor, 0);                 // Auto white balance mode
}

bool TTGOTCAMERA::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30); // OV2640 I2C address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV2640 sensor detected");
        return true;
    }
    
    Wire.beginTransmission(0x3C); // Alternative address for OV3660
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV3660 sensor detected");
        sensor_type = SENSOR_OV3660;
        return true;
    }
    
    DEBUG_PRINTLN("Camera sensor not responding");
    return false;
}

void TTGOTCAMERA::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    
    // Set up camera pins
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW); // Power on
    }
    
    if (gpio.reset_pin >= 0) {
        pinMode(gpio.reset_pin, OUTPUT);
        digitalWrite(gpio.reset_pin, HIGH); // Release reset
    }
    
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, LOW); // LED off initially
    }
}

void TTGOTCAMERA::setupPowerPins() {
    // Configure power management pins specific to TTGO T-Camera
    // Enable 3.3V regulator if available
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); // Enable power
}

void TTGOTCAMERA::setupDisplayInterface() {
    // TTGO T-Camera has a built-in display
    DEBUG_PRINTLN("Setting up display interface for TTGO T-Camera");
    
    // Initialize display pins if needed
    // Display is typically on SPI interface
    pinMode(16, OUTPUT); // Display reset
    pinMode(17, OUTPUT); // Display DC
    digitalWrite(16, HIGH); // Release display reset
}