/**
 * @file lilygo_t_camera_plus.cpp
 * @brief LilyGO T-Camera Plus board implementation
 */

#include "lilygo_t_camera_plus.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

LILYGOTCAMERAPLUS::LILYGOTCAMERAPLUS() {
    board_type = BOARD_LILYGO_T_CAMERA_PLUS;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool LILYGOTCAMERAPLUS::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing LilyGO T-Camera Plus...");
    setupEnhancedFeatures();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool LILYGOTCAMERAPLUS::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap LILYGOTCAMERAPLUS::getGPIOMap() const {
    return LILYGO_T_CAMERA_PLUS_GPIO_MAP;
}

CameraConfig LILYGOTCAMERAPLUS::getCameraConfig() const {
    return LILYGO_T_CAMERA_PLUS_CAMERA_CONFIG;
}

PowerProfile LILYGOTCAMERAPLUS::getPowerProfile() const {
    return LILYGO_T_CAMERA_PLUS_POWER_PROFILE;
}

DisplayProfile LILYGOTCAMERAPLUS::getDisplayProfile() const {
    return LILYGO_T_CAMERA_PLUS_DISPLAY_PROFILE;
}

bool LILYGOTCAMERAPLUS::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool LILYGOTCAMERAPLUS::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void LILYGOTCAMERAPLUS::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void LILYGOTCAMERAPLUS::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void LILYGOTCAMERAPLUS::exitSleepMode() {
    DEBUG_PRINTLN("LilyGO T-Camera Plus exiting sleep mode");
}

float LILYGOTCAMERAPLUS::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool LILYGOTCAMERAPLUS::hasPSRAM() const {
    return psramFound();
}

uint32_t LILYGOTCAMERAPLUS::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* LILYGOTCAMERAPLUS::getChipModel() const {
    return "ESP32";
}

void LILYGOTCAMERAPLUS::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 10);
}

bool LILYGOTCAMERAPLUS::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void LILYGOTCAMERAPLUS::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void LILYGOTCAMERAPLUS::setupPowerPins() {
    DEBUG_PRINTLN("Setting up LilyGO T-Camera Plus power management");
}

void LILYGOTCAMERAPLUS::setupEnhancedFeatures() {
    DEBUG_PRINTLN("Setting up enhanced features for LilyGO T-Camera Plus");
}