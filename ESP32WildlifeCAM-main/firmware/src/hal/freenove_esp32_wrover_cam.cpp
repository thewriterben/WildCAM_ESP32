/**
 * @file freenove_esp32_wrover_cam.cpp
 * @brief Freenove ESP32-WROVER-CAM board implementation
 */

#include "freenove_esp32_wrover_cam.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

FREENOVESP32WROVERCAM::FREENOVESP32WROVERCAM() {
    board_type = BOARD_FREENOVE_ESP32_WROVER_CAM;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool FREENOVESP32WROVERCAM::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing Freenove ESP32-WROVER-CAM...");
    setupPSRAMOptimizations();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool FREENOVESP32WROVERCAM::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap FREENOVESP32WROVERCAM::getGPIOMap() const {
    return FREENOVE_ESP32_WROVER_CAM_GPIO_MAP;
}

CameraConfig FREENOVESP32WROVERCAM::getCameraConfig() const {
    return FREENOVE_ESP32_WROVER_CAM_CAMERA_CONFIG;
}

PowerProfile FREENOVESP32WROVERCAM::getPowerProfile() const {
    return FREENOVE_ESP32_WROVER_CAM_POWER_PROFILE;
}

bool FREENOVESP32WROVERCAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool FREENOVESP32WROVERCAM::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void FREENOVESP32WROVERCAM::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void FREENOVESP32WROVERCAM::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void FREENOVESP32WROVERCAM::exitSleepMode() {
    DEBUG_PRINTLN("Freenove ESP32-WROVER-CAM exiting sleep mode");
}

float FREENOVESP32WROVERCAM::getBatteryVoltage() {
    int raw = analogRead(A0);
    return (raw * 3.3) / 4095.0;
}

bool FREENOVESP32WROVERCAM::hasPSRAM() const {
    return psramFound();
}

uint32_t FREENOVESP32WROVERCAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* FREENOVESP32WROVERCAM::getChipModel() const {
    return "ESP32";
}

void FREENOVESP32WROVERCAM::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 12);
}

bool FREENOVESP32WROVERCAM::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void FREENOVESP32WROVERCAM::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void FREENOVESP32WROVERCAM::setupPowerPins() {
    DEBUG_PRINTLN("Setting up Freenove power management");
}

void FREENOVESP32WROVERCAM::setupPSRAMOptimizations() {
    DEBUG_PRINTLN("Setting up PSRAM optimizations for Freenove");
}