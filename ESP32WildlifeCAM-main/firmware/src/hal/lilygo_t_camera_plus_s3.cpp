/**
 * @file lilygo_t_camera_plus_s3.cpp
 * @brief LilyGO T-Camera Plus S3 OV5640 V1.1 board implementation
 */

#include "lilygo_t_camera_plus_s3.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include "../configs/sensor_configs.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <Wire.h>

LILYGOTCAMERAPLASS3::LILYGOTCAMERAPLASS3() {
    board_type = BOARD_LILYGO_T_CAMERA_PLUS_S3;
    sensor_type = SENSOR_OV5640;
    initialized = false;
}

bool LILYGOTCAMERAPLASS3::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing LilyGO T-Camera Plus S3 OV5640 V1.1...");
    
    // Validate S3 hardware first
    if (!validateS3Hardware()) {
        DEBUG_PRINTLN("Hardware validation failed - not a valid S3 board");
        return false;
    }
    
    // Setup S3-specific features
    setupS3Features();
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Detect OV5640 sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect OV5640 sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("LilyGO T-Camera Plus S3 OV5640 V1.1 initialized successfully");
    return true;
}

bool LILYGOTCAMERAPLASS3::detectSensor() {
    // Set expected sensor type
    sensor_type = SENSOR_OV5640;
    
    // Test OV5640 communication
    return testOV5640Communication();
}

GPIOMap LILYGOTCAMERAPLASS3::getGPIOMap() const {
    return LILYGO_T_CAMERA_PLUS_S3_GPIO_MAP;
}

CameraConfig LILYGOTCAMERAPLASS3::getCameraConfig() const {
    return LILYGO_T_CAMERA_PLUS_S3_CAMERA_CONFIG;
}

PowerProfile LILYGOTCAMERAPLASS3::getPowerProfile() const {
    return LILYGO_T_CAMERA_PLUS_S3_POWER_PROFILE;
}

DisplayProfile LILYGOTCAMERAPLASS3::getDisplayProfile() const {
    return LILYGO_T_CAMERA_PLUS_S3_DISPLAY_PROFILE;
}

bool LILYGOTCAMERAPLASS3::configureSensor(sensor_t* sensor) {
    if (!sensor) {
        DEBUG_PRINTLN("Invalid sensor pointer");
        return false;
    }
    
    // Apply OV5640-specific wildlife optimizations
    configureOV5640Settings(sensor);
    
    return true;
}

bool LILYGOTCAMERAPLASS3::setupPins() {
    setupCameraPins();
    setupPowerPins();
    setupDisplayPins();
    return true;
}

void LILYGOTCAMERAPLASS3::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void LILYGOTCAMERAPLASS3::enterSleepMode() {
    DEBUG_PRINTLN("Entering deep sleep mode");
    
    // Configure wake-up timer (5 minutes for wildlife monitoring)
    esp_sleep_enable_timer_wakeup(300 * 1000000ULL);
    
    // Configure wake-up on GPIO (PIR sensor, if connected)
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_6, 1);
    
    // Start deep sleep
    esp_deep_sleep_start();
}

void LILYGOTCAMERAPLASS3::exitSleepMode() {
    DEBUG_PRINTLN("LilyGO T-Camera Plus S3 exiting sleep mode");
    
    // Re-initialize any necessary peripherals after wake
    setupS3Features();
}

float LILYGOTCAMERAPLASS3::getBatteryVoltage() {
    // Battery voltage monitoring on GPIO 8
    int raw = analogRead(8);
    // Convert to voltage with appropriate scaling factor
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool LILYGOTCAMERAPLASS3::hasPSRAM() const {
    return psramFound();
}

uint32_t LILYGOTCAMERAPLASS3::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* LILYGOTCAMERAPLASS3::getChipModel() const {
    return "ESP32-S3";
}

void LILYGOTCAMERAPLASS3::configureOV5640Settings(sensor_t* sensor) {
    DEBUG_PRINTLN("Configuring OV5640 sensor for wildlife photography");
    
    // Get OV5640 wildlife settings
    const WildlifeSettings* settings = getWildlifeSettings(SENSOR_OV5640);
    if (!settings) {
        DEBUG_PRINTLN("Warning: No OV5640 wildlife settings found, using defaults");
        return;
    }
    
    // Apply wildlife optimizations for OV5640
    sensor->set_brightness(sensor, settings->brightness);
    sensor->set_contrast(sensor, settings->contrast);
    sensor->set_saturation(sensor, settings->saturation);
    sensor->set_whitebal(sensor, settings->wb_mode == 0);
    sensor->set_awb_gain(sensor, settings->wb_mode == 0);
    sensor->set_aec_value(sensor, settings->aec_value);
    sensor->set_agc_gain(sensor, settings->agc_gain);
    sensor->set_gainceiling(sensor, settings->gainceiling);
    sensor->set_hmirror(sensor, settings->hmirror);
    sensor->set_vflip(sensor, settings->vflip);
    
    // OV5640-specific optimizations for wildlife
    sensor->set_framesize(sensor, FRAMESIZE_QXGA);  // Use high resolution
    sensor->set_quality(sensor, 8);                 // Good quality for wildlife
    
    DEBUG_PRINTLN("OV5640 wildlife optimization complete");
}

bool LILYGOTCAMERAPLASS3::testOV5640Communication() {
    DEBUG_PRINTLN("Testing OV5640 communication...");
    
    GPIOMap gpio = getGPIOMap();
    
    // Initialize I2C for sensor communication
    Wire.begin(gpio.siod_pin, gpio.sioc_pin);
    Wire.setClock(100000); // 100kHz for OV5640
    
    // OV5640 I2C address is 0x3C (7-bit addressing)
    Wire.beginTransmission(0x3C);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        DEBUG_PRINTLN("OV5640 sensor detected successfully");
        return true;
    } else {
        DEBUG_PRINTLN("Failed to detect OV5640 sensor");
        return false;
    }
}

void LILYGOTCAMERAPLASS3::setupCameraPins() {
    DEBUG_PRINTLN("Setting up camera pins for T-Camera Plus S3");
    
    GPIOMap gpio = getGPIOMap();
    
    // Camera pins are configured automatically by esp_camera_init()
    // But we can set up any additional camera-related pins here
    
    // Setup XCLK pin if needed
    if (gpio.xclk_pin >= 0) {
        pinMode(gpio.xclk_pin, OUTPUT);
    }
    
    // Setup power down pin (if used)
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW); // Enable camera
    }
    
    // Setup reset pin (if used)
    if (gpio.reset_pin >= 0) {
        pinMode(gpio.reset_pin, OUTPUT);
        digitalWrite(gpio.reset_pin, HIGH); // Normal operation
    }
}

void LILYGOTCAMERAPLASS3::setupPowerPins() {
    DEBUG_PRINTLN("Setting up power management for T-Camera Plus S3");
    
    // Setup battery voltage monitoring
    pinMode(8, INPUT);
    
    // Setup any other power-related pins
    // (implementation depends on specific board design)
}

void LILYGOTCAMERAPLASS3::setupDisplayPins() {
    DEBUG_PRINTLN("Setting up display pins for T-Camera Plus S3");
    
    DisplayProfile display = getDisplayProfile();
    
    if (display.has_display) {
        // Setup TFT display pins
        if (display.display_cs_pin >= 0) {
            pinMode(display.display_cs_pin, OUTPUT);
            digitalWrite(display.display_cs_pin, HIGH);
        }
        
        if (display.display_dc_pin >= 0) {
            pinMode(display.display_dc_pin, OUTPUT);
        }
        
        if (display.display_rst_pin >= 0) {
            pinMode(display.display_rst_pin, OUTPUT);
            digitalWrite(display.display_rst_pin, HIGH);
        }
        
        if (display.display_bl_pin >= 0) {
            pinMode(display.display_bl_pin, OUTPUT);
            digitalWrite(display.display_bl_pin, HIGH); // Enable backlight
        }
    }
}

void LILYGOTCAMERAPLASS3::setupS3Features() {
    DEBUG_PRINTLN("Setting up ESP32-S3 specific features");
    
    // Disable brownout detector for better low-power operation
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    // Configure CPU frequency for optimal performance
    setCpuFrequencyMhz(240);
    
    // Enable PSRAM if available
    if (hasPSRAM()) {
        DEBUG_PRINTLN("PSRAM detected and enabled");
    } else {
        DEBUG_PRINTLN("Warning: No PSRAM detected - may limit high resolution capability");
    }
}

bool LILYGOTCAMERAPLASS3::validateS3Hardware() {
    // Check if we're running on ESP32-S3
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    if (chip_info.model != CHIP_ESP32S3) {
        DEBUG_PRINTLN("Not running on ESP32-S3 hardware");
        return false;
    }
    
    DEBUG_PRINTLN("ESP32-S3 hardware validation passed");
    return true;
}