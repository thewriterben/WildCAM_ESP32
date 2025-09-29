/**
 * @file m5stack_unitcam.cpp
 * @brief M5Stack UnitCAM board implementation
 */

#include "m5stack_unitcam.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

M5STACKUNITCAM::M5STACKUNITCAM() {
    board_type = BOARD_M5STACK_UNITCAM;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool M5STACKUNITCAM::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing M5Stack UnitCAM...");
    setupTinyFormFactor();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool M5STACKUNITCAM::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap M5STACKUNITCAM::getGPIOMap() const {
    return M5STACK_UNITCAM_GPIO_MAP;
}

CameraConfig M5STACKUNITCAM::getCameraConfig() const {
    return M5STACK_UNITCAM_CAMERA_CONFIG;
}

PowerProfile M5STACKUNITCAM::getPowerProfile() const {
    return M5STACK_UNITCAM_POWER_PROFILE;
}

bool M5STACKUNITCAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool M5STACKUNITCAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void M5STACKUNITCAM::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void M5STACKUNITCAM::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(600 * 1000000); // Longer for discrete monitoring
    esp_deep_sleep_start();
}

void M5STACKUNITCAM::exitSleepMode() {
    DEBUG_PRINTLN("M5Stack UnitCAM exiting sleep mode");
}

float M5STACKUNITCAM::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool M5STACKUNITCAM::hasPSRAM() const {
    return psramFound();
}

uint32_t M5STACKUNITCAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* M5STACKUNITCAM::getChipModel() const {
    return "ESP32";
}

void M5STACKUNITCAM::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_SVGA); // Smaller for tiny form factor
    sensor->set_quality(sensor, 12);
}

bool M5STACKUNITCAM::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void M5STACKUNITCAM::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void M5STACKUNITCAM::setupPowerPins() {
    DEBUG_PRINTLN("Setting up M5Stack UnitCAM power management");
}

void M5STACKUNITCAM::setupTinyFormFactor() {
    DEBUG_PRINTLN("Setting up tiny form factor optimizations for M5Stack UnitCAM");
}