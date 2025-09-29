/**
 * @file ttgo_t_camera_v17.cpp
 * @brief TTGO T-Camera V1.7 board implementation
 */

#include "ttgo_t_camera_v17.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

TTGOTCAMERAV17::TTGOTCAMERAV17() {
    board_type = BOARD_TTGO_T_CAMERA_V17;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool TTGOTCAMERAV17::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing TTGO T-Camera V1.7...");
    setupLatestFeatures();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool TTGOTCAMERAV17::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap TTGOTCAMERAV17::getGPIOMap() const {
    return TTGO_T_CAMERA_V17_GPIO_MAP;
}

CameraConfig TTGOTCAMERAV17::getCameraConfig() const {
    return TTGO_T_CAMERA_V17_CAMERA_CONFIG;
}

PowerProfile TTGOTCAMERAV17::getPowerProfile() const {
    return TTGO_T_CAMERA_V17_POWER_PROFILE;
}

bool TTGOTCAMERAV17::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool TTGOTCAMERAV17::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void TTGOTCAMERAV17::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void TTGOTCAMERAV17::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void TTGOTCAMERAV17::exitSleepMode() {
    DEBUG_PRINTLN("TTGO T-Camera V1.7 exiting sleep mode");
}

float TTGOTCAMERAV17::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool TTGOTCAMERAV17::hasPSRAM() const {
    return psramFound();
}

uint32_t TTGOTCAMERAV17::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* TTGOTCAMERAV17::getChipModel() const {
    return "ESP32";
}

void TTGOTCAMERAV17::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 10); // Higher quality for v1.7
}

bool TTGOTCAMERAV17::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void TTGOTCAMERAV17::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void TTGOTCAMERAV17::setupPowerPins() {
    DEBUG_PRINTLN("Setting up TTGO T-Camera V1.7 power management");
}

void TTGOTCAMERAV17::setupLatestFeatures() {
    DEBUG_PRINTLN("Setting up latest features for TTGO T-Camera V1.7");
}