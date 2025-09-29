/**
 * @file m5stack_timer_cam.cpp
 * @brief M5Stack Timer Camera board implementation
 */

#include "m5stack_timer_cam.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

M5StackTimerCAM::M5StackTimerCAM() {
    board_type = BOARD_M5STACK_TIMER_CAM;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool M5StackTimerCAM::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing M5Stack Timer Camera...");
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Setup battery management for outdoor deployment
    setupBatteryManagement();
    
    // Detect camera sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect camera sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("M5Stack Timer Camera initialized successfully");
    return true;
}

bool M5StackTimerCAM::detectSensor() {
    // M5Stack Timer Camera typically uses OV2640 sensor
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap M5StackTimerCAM::getGPIOMap() const {
    return M5STACK_TIMER_CAM_GPIO_MAP;
}

CameraConfig M5StackTimerCAM::getCameraConfig() const {
    return M5STACK_TIMER_CAM_CAMERA_CONFIG;
}

PowerProfile M5StackTimerCAM::getPowerProfile() const {
    return M5STACK_TIMER_CAM_POWER_PROFILE;
}

bool M5StackTimerCAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    configureWildlifeSettings(sensor);
    
    // M5Stack specific optimizations for battery life
    sensor->set_brightness(sensor, 0);    // Brightness
    sensor->set_contrast(sensor, 0);      // Contrast
    sensor->set_saturation(sensor, 0);    // Saturation
    sensor->set_sharpness(sensor, 0);     // Sharpness
    sensor->set_denoise(sensor, 1);       // Denoise
    sensor->set_gainceiling(sensor, GAINCEILING_2X);
    sensor->set_quality(sensor, 12);       // JPEG quality
    sensor->set_colorbar(sensor, 0);      // Disable color bar
    sensor->set_whitebal(sensor, 1);      // Auto white balance
    sensor->set_gain_ctrl(sensor, 1);     // Auto gain control
    sensor->set_exposure_ctrl(sensor, 1); // Auto exposure
    sensor->set_hmirror(sensor, 0);       // Horizontal mirror
    sensor->set_vflip(sensor, 0);         // Vertical flip
    
    return true;
}

bool M5StackTimerCAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void M5StackTimerCAM::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void M5StackTimerCAM::enterSleepMode() {
    DEBUG_PRINTLN("M5Stack Timer Camera entering sleep mode");
    
    // Configure wake-up source (PIR sensor, timer, etc.)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1); // PIR sensor
    esp_sleep_enable_timer_wakeup(300 * 1000000); // 5 minutes
    
    // Turn off camera power
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, HIGH);
    }
    
    esp_deep_sleep_start();
}

void M5StackTimerCAM::exitSleepMode() {
    DEBUG_PRINTLN("M5Stack Timer Camera exiting sleep mode");
    
    // Power up camera
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, LOW);
        delay(100); // Allow camera to stabilize
    }
}

float M5StackTimerCAM::getBatteryVoltage() {
    // M5Stack Timer Camera has built-in battery monitoring
    // Reading from analog pin for battery voltage divider
    int raw = analogRead(35); // Battery voltage pin
    return (raw * 3.3 * 2.0) / 4095.0; // Convert to voltage with divider
}

bool M5StackTimerCAM::hasPSRAM() const {
    return psramFound();
}

uint32_t M5StackTimerCAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* M5StackTimerCAM::getChipModel() const {
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

void M5StackTimerCAM::configureWildlifeSettings(sensor_t* sensor) {
    // Optimize for outdoor wildlife photography
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);  // High resolution for detail
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

bool M5StackTimerCAM::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30); // OV2640 I2C address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV2640 sensor detected");
        return true;
    }
    
    Wire.beginTransmission(0x3C); // Alternative address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("Camera sensor detected at alternative address");
        return true;
    }
    
    DEBUG_PRINTLN("Camera sensor not responding");
    return false;
}

void M5StackTimerCAM::setupCameraPins() {
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

void M5StackTimerCAM::setupPowerPins() {
    // Configure power management pins specific to M5Stack
    // Enable 3.3V regulator if available
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); // Enable power
}

void M5StackTimerCAM::setupBatteryManagement() {
    // M5Stack Timer Camera specific battery management
    DEBUG_PRINTLN("Setting up battery management for extended field deployment");
    
    // Configure battery voltage monitoring
    pinMode(35, INPUT); // Battery voltage ADC pin
    
    // Set up low power modes
    setCpuFrequencyMhz(80); // Reduce CPU frequency for power saving
    
    // Configure charging LED if available
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW); // Charging indicator off initially
}