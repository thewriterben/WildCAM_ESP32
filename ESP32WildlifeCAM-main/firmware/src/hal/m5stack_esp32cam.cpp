/**
 * @file m5stack_esp32cam.cpp
 * @brief M5Stack ESP32CAM board implementation
 */

#include "m5stack_esp32cam.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

M5STACKESP32CAM::M5STACKESP32CAM() {
    board_type = BOARD_M5STACK_ESP32CAM;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool M5STACKESP32CAM::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing M5Stack ESP32CAM...");
    setupStackableInterface();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool M5STACKESP32CAM::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap M5STACKESP32CAM::getGPIOMap() const {
    return M5STACK_ESP32CAM_GPIO_MAP;
}

CameraConfig M5STACKESP32CAM::getCameraConfig() const {
    return M5STACK_ESP32CAM_CAMERA_CONFIG;
}

PowerProfile M5STACKESP32CAM::getPowerProfile() const {
    return M5STACK_ESP32CAM_POWER_PROFILE;
}

bool M5STACKESP32CAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool M5STACKESP32CAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void M5STACKESP32CAM::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void M5STACKESP32CAM::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void M5STACKESP32CAM::exitSleepMode() {
    DEBUG_PRINTLN("M5Stack ESP32CAM exiting sleep mode");
}

float M5STACKESP32CAM::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool M5STACKESP32CAM::hasPSRAM() const {
    return psramFound();
}

uint32_t M5STACKESP32CAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* M5STACKESP32CAM::getChipModel() const {
    return "ESP32";
}

void M5STACKESP32CAM::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 12);
}

bool M5STACKESP32CAM::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void M5STACKESP32CAM::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void M5STACKESP32CAM::setupPowerPins() {
    DEBUG_PRINTLN("Setting up M5Stack ESP32CAM power management");
}

void M5STACKESP32CAM::setupStackableInterface() {
    DEBUG_PRINTLN("Setting up M5Stack stackable interface");
}