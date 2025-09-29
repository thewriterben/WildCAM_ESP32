/**
 * @file ttgo_t_journal.cpp
 * @brief TTGO T-Journal board implementation
 */

#include "ttgo_t_journal.h"
#include "../config.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

TTGOTJOURNAL::TTGOTJOURNAL() {
    board_type = BOARD_TTGO_T_JOURNAL;
    sensor_type = SENSOR_OV2640;
    initialized = false;
}

bool TTGOTJOURNAL::init() {
    if (initialized) return true;
    DEBUG_PRINTLN("Initializing TTGO T-Journal...");
    setupEPaperDisplay();
    if (!setupPins()) return false;
    if (!detectSensor()) return false;
    initialized = true;
    return true;
}

bool TTGOTJOURNAL::detectSensor() {
    sensor_type = SENSOR_OV2640;
    return testSensorCommunication();
}

GPIOMap TTGOTJOURNAL::getGPIOMap() const {
    return TTGO_T_JOURNAL_GPIO_MAP;
}

CameraConfig TTGOTJOURNAL::getCameraConfig() const {
    return TTGO_T_JOURNAL_CAMERA_CONFIG;
}

PowerProfile TTGOTJOURNAL::getPowerProfile() const {
    return TTGO_T_JOURNAL_POWER_PROFILE;
}

bool TTGOTJOURNAL::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    configureWildlifeSettings(sensor);
    return true;
}

bool TTGOTJOURNAL::setupPins() {
    setupCameraPins();
    setupPowerPins();
    return true;
}

void TTGOTJOURNAL::flashLED(bool state) {
    GPIOMap gpio = getGPIOMap();
    if (gpio.led_pin >= 0) {
        pinMode(gpio.led_pin, OUTPUT);
        digitalWrite(gpio.led_pin, state ? HIGH : LOW);
    }
}

void TTGOTJOURNAL::enterSleepMode() {
    esp_sleep_enable_timer_wakeup(300 * 1000000);
    esp_deep_sleep_start();
}

void TTGOTJOURNAL::exitSleepMode() {
    DEBUG_PRINTLN("TTGO T-Journal exiting sleep mode");
}

float TTGOTJOURNAL::getBatteryVoltage() {
    int raw = analogRead(35);
    return (raw * 3.3 * 2.0) / 4095.0;
}

bool TTGOTJOURNAL::hasPSRAM() const {
    return psramFound();
}

uint32_t TTGOTJOURNAL::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* TTGOTJOURNAL::getChipModel() const {
    return "ESP32";
}

void TTGOTJOURNAL::configureWildlifeSettings(sensor_t* sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_UXGA);
    sensor->set_quality(sensor, 12);
}

bool TTGOTJOURNAL::testSensorCommunication() {
    Wire.begin();
    Wire.beginTransmission(0x30);
    return (Wire.endTransmission() == 0);
}

void TTGOTJOURNAL::setupCameraPins() {
    GPIOMap gpio = getGPIOMap();
    if (gpio.pwdn_pin >= 0) {
        pinMode(gpio.pwdn_pin, OUTPUT);
        digitalWrite(gpio.pwdn_pin, LOW);
    }
}

void TTGOTJOURNAL::setupPowerPins() {
    DEBUG_PRINTLN("Setting up TTGO T-Journal power management");
}

void TTGOTJOURNAL::setupEPaperDisplay() {
    DEBUG_PRINTLN("Setting up E-paper display for TTGO T-Journal");
}