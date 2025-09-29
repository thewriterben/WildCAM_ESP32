/**
 * @file espressif_esp32_cam_mb.cpp
 * @brief Espressif ESP32-CAM-MB board implementation
 */

#include "espressif_esp32_cam_mb.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

ESPRESSIF_ESP32CAMMB::ESPRESSIF_ESP32CAMMB() {
    board_type = BOARD_ESPRESSIF_ESP32_CAM_MB;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool ESPRESSIF_ESP32CAMMB::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing Espressif ESP32-CAM-MB...");
    setupOfficialFeatures();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool ESPRESSIF_ESP32CAMMB::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap ESPRESSIF_ESP32CAMMB::getGPIOMap() const {
    return ESPRESSIF_ESP32_CAM_MB_GPIO_MAP;
}

CameraConfig ESPRESSIF_ESP32CAMMB::getCameraConfig() const {
    return ESPRESSIF_ESP32_CAM_MB_CAMERA_CONFIG;
}

PowerProfile ESPRESSIF_ESP32CAMMB::getPowerProfile() const {
    return ESPRESSIF_ESP32_CAM_MB_POWER_PROFILE;
}

bool ESPRESSIF_ESP32CAMMB::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool ESPRESSIF_ESP32CAMMB::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void ESPRESSIF_ESP32CAMMB::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void ESPRESSIF_ESP32CAMMB::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void ESPRESSIF_ESP32CAMMB::exitSleepMode() {
    DEBUG_PRINTLN("Espressif ESP32-CAM-MB exiting sleep mode");
}

float ESPRESSIF_ESP32CAMMB::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool ESPRESSIF_ESP32CAMMB::hasPSRAM() const {
    return psramFound();
}

uint32_t ESPRESSIF_ESP32CAMMB::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* ESPRESSIF_ESP32CAMMB::getChipModel() const {
    return "ESP32";
}

void ESPRESSIF_ESP32CAMMB::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 12);
}

bool ESPRESSIF_ESP32CAMMB::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void ESPRESSIF_ESP32CAMMB::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void ESPRESSIF_ESP32CAMMB::setupPowerPins() {
    DEBUG_PRINTLN("Setting up Espressif ESP32-CAM-MB power management");
}

void ESPRESSIF_ESP32CAMMB::setupOfficialFeatures() {
    DEBUG_PRINTLN("Setting up official Espressif features");
}