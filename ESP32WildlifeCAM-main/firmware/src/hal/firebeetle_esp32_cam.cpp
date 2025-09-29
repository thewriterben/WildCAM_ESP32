/**
 * @file firebeetle_esp32_cam.cpp
 * @brief DFRobot FireBeetle ESP32-CAM board implementation
 */

#include "firebeetle_esp32_cam.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

FIREBEETLEESP32CAM::FIREBEETLEESP32CAM() {
    board_type = BOARD_FIREBEETLE_ESP32_CAM;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool FIREBEETLEESP32CAM::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing FireBeetle ESP32-CAM...");
    
    // Setup low power mode first
    setupLowPowerMode();
    
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
    DEBUG_PRINTLN("FireBeetle ESP32-CAM initialized successfully");
    return true;
}

bool FIREBEETLEESP32CAM::detectSensor() {
    // FireBeetle ESP32-CAM typically uses OV2640 sensor
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap FIREBEETLEESP32CAM::getGPIOMap() const {
    return FIREBEETLE_ESP32_CAM_GPIO_MAP;
}

CameraConfig FIREBEETLEESP32CAM::getCameraConfig() const {
    return FIREBEETLE_ESP32_CAM_CAMERA_CONFIG;
}

PowerProfile FIREBEETLEESP32CAM::getPowerProfile() const {
    return FIREBEETLE_ESP32_CAM_POWER_PROFILE;
}

bool FIREBEETLEESP32CAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    configureWildlifeSettings(sensor);
    
    // FireBeetle specific optimizations for extreme low power
    sensor->set_brightness(sensor, 0);    // Brightness
    sensor->set_contrast(sensor, 0);      // Contrast
    sensor->set_saturation(sensor, -1);   // Slightly reduced saturation for power
    sensor->set_sharpness(sensor, 0);     // Minimal processing for power saving
    sensor->set_denoise(sensor, 1);       // Enable denoise
    sensor->set_gainceiling(sensor, GAINCEILING_2X); // Lower gain for power
    sensor->set_quality(sensor, 15);      // Lower quality for power saving
    sensor->set_colorbar(sensor, 0);      // Disable color bar
    sensor->set_whitebal(sensor, 1);      // Auto white balance
    sensor->set_gain_ctrl(sensor, 1);     // Auto gain control
    sensor->set_exposure_ctrl(sensor, 1); // Auto exposure
    sensor->set_hmirror(sensor, 0);       // Horizontal mirror
    sensor->set_vflip(sensor, 0);         // Vertical flip
    
    return true;
}

bool FIREBEETLEESP32CAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void FIREBEETLEESP32CAM::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void FIREBEETLEESP32CAM::enterSleepMode() {
    DEBUG_PRINTLN("FireBeetle ESP32-CAM entering ultra-low power sleep mode");
    
    // Configure wake-up source for extended battery life
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1); // PIR sensor
    esp_sleep_enable_timer_wakeup(900 * 1000000); // 15 minutes for extreme power saving
    
    // Turn off camera power
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, HIGH);
    }
    
    // Turn off all unnecessary peripherals
    esp_wifi_stop();
    esp_bt_controller_disable();
    
    esp_deep_sleep_start();
}

void FIREBEETLEESP32CAM::exitSleepMode() {
    DEBUG_PRINTLN("FireBeetle ESP32-CAM exiting sleep mode");
    
    // Power up camera
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, LOW);
        delay(200); // Allow extra time for stable power-up
    }
}

float FIREBEETLEESP32CAM::getBatteryVoltage() {
    // FireBeetle has built-in battery management
    int raw = analogRead(A0); // Battery voltage pin
    return (raw * 3.3 * 2.0) / 4095.0; // Convert to voltage with divider
}

bool FIREBEETLEESP32CAM::hasPSRAM() const {
    return psramFound();
}

uint32_t FIREBEETLEESP32CAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* FIREBEETLEESP32CAM::getChipModel() const {
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

void FIREBEETLEESP32CAM::configureWildlifeSettings(sensor_t* sensor) {
    // Optimize for extended battery life in wildlife monitoring
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);  // Good resolution
    sensor->set_quality(sensor, 15);                // Lower quality for power
    sensor->set_brightness(sensor, 0);              // Neutral brightness
    sensor->set_contrast(sensor, 0);                // Minimal processing
    sensor->set_saturation(sensor, -1);             // Reduced saturation
    sensor->set_sharpness(sensor, 0);               // Minimal processing
    sensor->set_denoise(sensor, 1);                 // Keep noise reduction
    sensor->set_gainceiling(sensor, GAINCEILING_4X); // Moderate gain
    sensor->set_agc_gain(sensor, 0);                // Auto gain
    sensor->set_aec_value(sensor, 0);               // Auto exposure
    sensor->set_aec2(sensor, 0);                    // AEC DSP
    sensor->set_ae_level(sensor, 0);                // AE level
    sensor->set_awb_gain(sensor, 1);                // Auto white balance gain
    sensor->set_wb_mode(sensor, 0);                 // Auto white balance mode
}

bool FIREBEETLEESP32CAM::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30); // OV2640 I2C address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV2640 sensor detected on FireBeetle");
        return true;
    }
    
    Wire.beginTransmission(0x3C); // Alternative address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("Camera sensor detected at alternative address");
        return true;
    }
    
    DEBUG_PRINTLN("Camera sensor not responding on FireBeetle");
    return false;
}

void FIREBEETLEESP32CAM::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    
    // Set up camera pins for FireBeetle
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

void FIREBEETLEESP32CAM::setupPowerPins() {
    // Configure power management pins specific to FireBeetle
    DEBUG_PRINTLN("Setting up FireBeetle power management");
    
    // Enable low power regulator
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); // Enable power
    
    // Configure charging LED
    pinMode(A1, INPUT); // Charging status
}

void FIREBEETLEESP32CAM::setupLowPowerMode() {
    // FireBeetle specific low power optimizations
    DEBUG_PRINTLN("Configuring FireBeetle for ultra-low power consumption");
    
    // Reduce CPU frequency to minimum
    setCpuFrequencyMhz(40); // Lowest stable frequency for camera operation
    
    // Disable unnecessary peripherals
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM); // Maximum power saving
    
    // Configure ADC for battery monitoring
    analogSetAttenuation(ADC_11db); // For battery voltage measurement
    analogSetWidth(12); // 12-bit resolution
    
    // Setup power management interrupt
    pinMode(16, INPUT_PULLUP); // Power button/wake pin
}