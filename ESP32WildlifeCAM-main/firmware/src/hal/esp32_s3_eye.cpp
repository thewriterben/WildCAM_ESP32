/**
 * @file esp32_s3_eye.cpp
 * @brief ESP32-S3-EYE board implementation
 */

#include "esp32_s3_eye.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

ESP32S3EYE::ESP32S3EYE() {
    board_type = BOARD_ESP32_S3_EYE;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool ESP32S3EYE::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing ESP32-S3-EYE...");
    
    // Setup AI acceleration
    setupAIAcceleration();
    
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
    DEBUG_PRINTLN("ESP32-S3-EYE initialized successfully");
    return true;
}

bool ESP32S3EYE::detectSensor() {
    // ESP32-S3-EYE typically uses OV3660 or OV5640 sensor
    sensor_type = SENSOR_OV3660;
    return testSensorCommunication();
}

GPIOMap ESP32S3EYE::getGPIOMap() const {
    return ESP32_S3_EYE_GPIO_MAP;
}

CameraConfig ESP32S3EYE::getCameraConfig() const {
    return ESP32_S3_EYE_CAMERA_CONFIG;
}

PowerProfile ESP32S3EYE::getPowerProfile() const {
    return ESP32_S3_EYE_POWER_PROFILE;
}

bool ESP32S3EYE::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    configureWildlifeSettings(sensor);
    
    // ESP32-S3-EYE specific optimizations for AI vision
    sensor->set_framesize(sensor, FRAMESIZE_QXGA);  // High resolution for AI
    sensor->set_quality(sensor, 10);                // High quality
    sensor->set_brightness(sensor, 0);
    sensor->set_contrast(sensor, 0);
    sensor->set_saturation(sensor, 0);
    sensor->set_sharpness(sensor, 1);
    sensor->set_denoise(sensor, 1);
    sensor->set_gainceiling(sensor, GAINCEILING_8X);
    sensor->set_whitebal(sensor, 1);
    sensor->set_gain_ctrl(sensor, 1);
    sensor->set_exposure_ctrl(sensor, 1);
    
    return true;
}

bool ESP32S3EYE::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void ESP32S3EYE::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void ESP32S3EYE::enterSleepMode() {
    DEBUG_PRINTLN("ESP32-S3-EYE entering sleep mode");
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void ESP32S3EYE::exitSleepMode() {
    DEBUG_PRINTLN("ESP32-S3-EYE exiting sleep mode");
}

float ESP32S3EYE::getBatteryVoltage() {
    int raw = analogRead(A0);
    return (raw * 3.3) / 4095.0;
}

bool ESP32S3EYE::hasPSRAM() const {
    return psramFound();
}

uint32_t ESP32S3EYE::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* ESP32S3EYE::getChipModel() const {
    return "ESP32-S3";
}

void ESP32S3EYE::configureWildlifeSettings(sensor_t* sensor) {
    // Optimize for AI-powered wildlife detection
    sensor->set_framesize(sensor, FRAMESIZE_QXGA);
    sensor->set_quality(sensor, 10);
    sensor->set_brightness(sensor, 0);
    sensor->set_contrast(sensor, 0);
    sensor->set_saturation(sensor, 0);
    sensor->set_sharpness(sensor, 1);
    sensor->set_denoise(sensor, 1);
    sensor->set_gainceiling(sensor, GAINCEILING_16X);
    sensor->set_awb_gain(sensor, 1);
    sensor->set_wb_mode(sensor, 0);
}

bool ESP32S3EYE::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x3C); // OV3660 address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV3660 sensor detected on ESP32-S3-EYE");
        return true;
    }
    return false;
}

void ESP32S3EYE::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
    if (gpio.reset_pin >= 0) {
        pinMode(gpio.reset_pin, OUTPUT);
        digitalWrite(gpio.reset_pin, HIGH);
    }
}

void ESP32S3EYE::setupPowerPins() {
    DEBUG_PRINTLN("Setting up ESP32-S3-EYE power management");
}

void ESP32S3EYE::setupAIAcceleration() {
    DEBUG_PRINTLN("Setting up AI acceleration for ESP32-S3-EYE");
    // Configure AI acceleration features
}