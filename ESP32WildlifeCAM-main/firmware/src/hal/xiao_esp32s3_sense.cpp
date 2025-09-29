/**
 * @file xiao_esp32s3_sense.cpp
 * @brief Seeed Studio XIAO ESP32S3 Sense board implementation
 */

#include "xiao_esp32s3_sense.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

XIAOESP32S3SENSE::XIAOESP32S3SENSE() {
    board_type = BOARD_XIAO_ESP32S3_SENSE;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool XIAOESP32S3SENSE::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing XIAO ESP32S3 Sense...");
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Setup microphone for audio sensing
    setupMicrophone();
    
    // Detect camera sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect camera sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("XIAO ESP32S3 Sense initialized successfully");
    return true;
}

bool XIAOESP32S3SENSE::detectSensor() {
    // XIAO ESP32S3 Sense typically uses OV2640 sensor
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap XIAOESP32S3SENSE::getGPIOMap() const {
    return XIAO_ESP32S3_SENSE_GPIO_MAP;
}

CameraConfig XIAOESP32S3SENSE::getCameraConfig() const {
    return XIAO_ESP32S3_SENSE_CAMERA_CONFIG;
}

PowerProfile XIAOESP32S3SENSE::getPowerProfile() const {
    return XIAO_ESP32S3_SENSE_POWER_PROFILE;
}

bool XIAOESP32S3SENSE::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    configureWildlifeSettings(sensor);
    
    // XIAO ESP32S3 Sense specific optimizations for stealth monitoring
    sensor->set_brightness(sensor, 0);    // Brightness
    sensor->set_contrast(sensor, 1);      // Slight contrast boost
    sensor->set_saturation(sensor, 0);    // Neutral saturation
    sensor->set_sharpness(sensor, 2);     // More sharpening for small size
    sensor->set_denoise(sensor, 1);       // Enable denoise
    sensor->set_gainceiling(sensor, GAINCEILING_8X);
    sensor->set_quality(sensor, 10);      // Higher quality for smaller images
    sensor->set_colorbar(sensor, 0);      // Disable color bar
    sensor->set_whitebal(sensor, 1);      // Auto white balance
    sensor->set_gain_ctrl(sensor, 1);     // Auto gain control
    sensor->set_exposure_ctrl(sensor, 1); // Auto exposure
    sensor->set_hmirror(sensor, 0);       // Horizontal mirror
    sensor->set_vflip(sensor, 0);         // Vertical flip
    
    return true;
}

bool XIAOESP32S3SENSE::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void XIAOESP32S3SENSE::flashLED(bool state) {
    // XIAO ESP32S3 Sense has no built-in flash LED
    // Could use GPIO for external LED if needed
    DEBUG_PRINTLN("XIAO ESP32S3 Sense: No built-in flash LED");
}

void XIAOESP32S3SENSE::enterSleepMode() {
    DEBUG_PRINTLN("XIAO ESP32S3 Sense entering sleep mode");
    
    // Configure wake-up source - ultra low power
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_1, 1); // Motion sensor
    esp_sleep_enable_timer_wakeup(600 * 1000000); // 10 minutes for stealth
    
    // Turn off camera power
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, HIGH);
    }
    
    esp_deep_sleep_start();
}

void XIAOESP32S3SENSE::exitSleepMode() {
    DEBUG_PRINTLN("XIAO ESP32S3 Sense exiting sleep mode");
    
    // Power up camera
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        digitalWrite(gpio.pwdn_pin, LOW);
        delay(50); // Shorter delay for responsiveness
    }
}

float XIAOESP32S3SENSE::getBatteryVoltage() {
    // XIAO ESP32S3 Sense battery voltage monitoring
    int raw = analogRead(A0); // Battery voltage pin
    return (raw * 3.3) / 4095.0; // Convert to voltage
}

bool XIAOESP32S3SENSE::hasPSRAM() const {
    return psramFound();
}

uint32_t XIAOESP32S3SENSE::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* XIAOESP32S3SENSE::getChipModel() const {
    return "ESP32-S3"; // XIAO uses ESP32-S3
}

void XIAOESP32S3SENSE::configureWildlifeSettings(sensor_t* sensor) {
    // Optimize for stealth wildlife monitoring with compact size
    sensor->set_framesize(sensor, FRAMESIZE_QXGA);  // High quality for ESP32-S3
    sensor->set_quality(sensor, 10);                // Higher quality
    sensor->set_brightness(sensor, 0);              // Neutral brightness
    sensor->set_contrast(sensor, 1);                // Slight contrast boost
    sensor->set_saturation(sensor, 0);              // Neutral saturation
    sensor->set_sharpness(sensor, 2);               // More sharpening
    sensor->set_denoise(sensor, 1);                 // Enable noise reduction
    sensor->set_gainceiling(sensor, GAINCEILING_32X); // High gain for low light
    sensor->set_agc_gain(sensor, 0);                // Auto gain
    sensor->set_aec_value(sensor, 0);               // Auto exposure
    sensor->set_aec2(sensor, 0);                    // AEC DSP
    sensor->set_ae_level(sensor, 0);                // AE level
    sensor->set_awb_gain(sensor, 1);                // Auto white balance gain
    sensor->set_wb_mode(sensor, 0);                 // Auto white balance mode
}

bool XIAOESP32S3SENSE::testSensorCommunication() {
    Wire.begin(40, 39); // Custom I2C pins for XIAO
    Wire.beginTransmission(0x30); // OV2640 I2C address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("OV2640 sensor detected on XIAO ESP32S3 Sense");
        return true;
    }
    
    Wire.beginTransmission(0x3C); // Alternative address
    if (Wire.endTransmission() == 0) {
        DEBUG_PRINTLN("Camera sensor detected at alternative address");
        return true;
    }
    
    DEBUG_PRINTLN("Camera sensor not responding on XIAO ESP32S3 Sense");
    return false;
}

void XIAOESP32S3SENSE::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    
    // Set up camera pins for XIAO ESP32S3 Sense
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW); // Power on
    }
    
    if (gpio.reset_pin >= 0) {
        pinMode(gpio.reset_pin, OUTPUT);
        digitalWrite(gpio.reset_pin, HIGH); // Release reset
    }
    
    // No built-in LED on XIAO ESP32S3 Sense
}

void XIAOESP32S3SENSE::setupPowerPins() {
    // Configure ultra-low power mode for stealth monitoring
    DEBUG_PRINTLN("Configuring ultra-low power mode for XIAO ESP32S3 Sense");
    
    // Reduce CPU frequency for power saving
    setCpuFrequencyMhz(80);
    
    // Configure charging status if available
    pinMode(A1, INPUT); // Charging status pin
}

void XIAOESP32S3SENSE::setupMicrophone() {
    // XIAO ESP32S3 Sense has built-in microphone for audio sensing
    DEBUG_PRINTLN("Setting up microphone for audio wildlife detection");
    
    // Configure microphone pins
    pinMode(42, INPUT); // Microphone data pin
    pinMode(41, OUTPUT); // Microphone clock pin
    
    // Initialize I2S for microphone if needed
    // This could be used for wildlife sound detection
}