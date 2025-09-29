/**
 * @file esp_eye.cpp
 * @brief ESP-EYE board implementation
 */

#include "esp_eye.h"
#include "../config.h"
#include "../configs/sensor_configs.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

ESPEYE::ESPEYE() {
    board_type = BOARD_ESP_EYE;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool ESPEYE::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing ESP-EYE...");
    
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
    DEBUG_PRINTLN("ESP-EYE initialized successfully");
    return true;
}

bool ESPEYE::detectSensor() {
    // ESP-EYE typically uses OV2640, but let's detect the actual sensor
    sensor_type = detectSpecificSensor();
    if (sensor_type == SENSOR_UNKNOWN) {
        // Fallback to OV2640 if detection fails
        sensor_type = SENSOR_OV2640;
    }
    return testSensorCommunication();
}

GPIOMap ESPEYE::getGPIOMap() const {
    // Return the ESP-EYE GPIO configuration from board profiles
    return ESP_EYE_GPIO_MAP;
}

CameraConfig ESPEYE::getCameraConfig() const {
    // Return the ESP-EYE camera configuration from board profiles
    return ESP_EYE_CAMERA_CONFIG;
}

PowerProfile ESPEYE::getPowerProfile() const {
    // Return the ESP-EYE power profile from board profiles
    return ESP_EYE_POWER_PROFILE;
}

DisplayProfile ESPEYE::getDisplayProfile() const {
    return NO_DISPLAY_PROFILE;  // ESP-EYE has no built-in display for now
}

bool ESPEYE::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    DEBUG_PRINTLN("Configuring ESP-EYE camera sensor for wildlife photography...");
    
    configureEspEyeSettings(sensor);
    return true;
}

bool ESPEYE::setupPins() {
    setupCameraPins();
    setupPowerManagement();
    return true;
}

void ESPEYE::flashLED(bool state) {
    GPIOMap gpio_map = getGPIOMap();
    if (gpio_map.led_pin >= 0) {
        digitalWrite(gpio_map.led_pin, state ? HIGH : LOW);
    }
}

void ESPEYE::enterSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power down camera
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, HIGH);
    }
    
    DEBUG_PRINTLN("ESP-EYE entering sleep mode");
}

void ESPEYE::exitSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power up camera
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, LOW);
        delay(100); // Allow camera to stabilize
    }
    
    DEBUG_PRINTLN("ESP-EYE exiting sleep mode");
}

float ESPEYE::getBatteryVoltage() {
    // ESP-EYE has advanced power management
    // This would read from the power management circuit
    return 3.3; // Placeholder - would need actual implementation
}

bool ESPEYE::hasPSRAM() const {
    return psramFound();
}

uint32_t ESPEYE::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* ESPEYE::getChipModel() const {
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

void ESPEYE::configureEspEyeSettings(sensor_t* sensor) {
    // Get sensor-specific wildlife settings
    const WildlifeSettings* settings = getWildlifeSettings(sensor_type);
    if (!settings) {
        DEBUG_PRINTLN("No specific settings found, using default OV2640 settings");
        settings = &OV2640_WILDLIFE_SETTINGS;
    }
    
    DEBUG_PRINTF("Applying %s wildlife settings for ESP-EYE\n", getSensorCapabilities(sensor_type)->name);
    
    // Apply wildlife-optimized settings
    sensor->set_brightness(sensor, settings->brightness);
    sensor->set_contrast(sensor, settings->contrast);
    sensor->set_saturation(sensor, settings->saturation);
    sensor->set_special_effect(sensor, 0); // 0=No Effect
    sensor->set_whitebal(sensor, 1);       // Enable auto white balance
    sensor->set_awb_gain(sensor, 1);       // Enable AWB gain
    sensor->set_wb_mode(sensor, settings->wb_mode);
    
    // Exposure and gain settings optimized for ESP-EYE
    sensor->set_exposure_ctrl(sensor, 1);  // Enable exposure control
    sensor->set_aec2(sensor, 0);           // Disable AEC2
    sensor->set_ae_level(sensor, 0);       // Auto exposure level
    sensor->set_aec_value(sensor, settings->aec_value);
    sensor->set_gain_ctrl(sensor, 1);      // Enable gain control
    sensor->set_agc_gain(sensor, settings->agc_gain);
    sensor->set_gainceiling(sensor, settings->gainceiling);
    
    // Image enhancement for wildlife photography
    sensor->set_bpc(sensor, 0);            // Disable bad pixel correction
    sensor->set_wpc(sensor, 1);            // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, 1);           // Enable lens correction
    
    // ESP-EYE specific orientation (different from AI-Thinker)
    sensor->set_hmirror(sensor, settings->hmirror ? 1 : 0);
    sensor->set_vflip(sensor, settings->vflip ? 1 : 0);
    sensor->set_dcw(sensor, 1);            // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);       // Disable color bar test pattern
    
    DEBUG_PRINTLN("ESP-EYE wildlife-optimized sensor settings applied");
}

bool ESPEYE::testSensorCommunication() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Test I2C communication with camera sensor using ESP-EYE pins
    Wire.begin(gpio_map.siod_pin, gpio_map.sioc_pin);
    delay(100); // Allow I2C to stabilize
    
    // Try to communicate with OV2640 sensor (common for ESP-EYE)
    const uint8_t OV2640_I2C_ADDR = 0x30;
    const uint8_t OV2640_CHIP_ID_HIGH = 0x0A;
    
    // Read chip ID to verify sensor communication
    Wire.beginTransmission(OV2640_I2C_ADDR);
    Wire.write(OV2640_CHIP_ID_HIGH);
    if (Wire.endTransmission() != 0) {
        DEBUG_PRINTLN("ESP-EYE: Failed to write to sensor register");
        return false;
    }
    
    Wire.requestFrom(OV2640_I2C_ADDR, (uint8_t)1);
    if (Wire.available() < 1) {
        DEBUG_PRINTLN("ESP-EYE: No response from sensor");
        return false;
    }
    
    uint8_t chip_id_high = Wire.read();
    
    // OV2640 chip ID high byte should be 0x26
    if (chip_id_high == 0x26) {
        DEBUG_PRINTLN("ESP-EYE: OV2640 sensor detected");
        sensor_type = SENSOR_OV2640;
        return true;
    }
    
    DEBUG_PRINTF("ESP-EYE: Unknown sensor chip ID: 0x%02X\n", chip_id_high);
    return false;
}

void ESPEYE::setupCameraPins() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Configure camera control pins
    if (gpio_map.pwdn_pin >= 0) {
        pinMode(gpio_map.pwdn_pin, OUTPUT);
        digitalWrite(gpio_map.pwdn_pin, LOW); // Camera active
    }
    
    if (gpio_map.reset_pin >= 0) {
        pinMode(gpio_map.reset_pin, OUTPUT);
        digitalWrite(gpio_map.reset_pin, HIGH); // Not in reset
    }
    
    // Configure LED/flash pin
    if (gpio_map.led_pin >= 0) {
        pinMode(gpio_map.led_pin, OUTPUT);
        digitalWrite(gpio_map.led_pin, LOW); // LED off
    }
    
    DEBUG_PRINTLN("ESP-EYE camera pins configured");
}

void ESPEYE::setupPowerManagement() {
    // ESP-EYE has more advanced power management than AI-Thinker
    // Configure any power management specific pins here
    
    DEBUG_PRINTLN("ESP-EYE power management configured");
}

SensorType ESPEYE::detectSpecificSensor() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Initialize I2C for sensor detection
    Wire.begin(gpio_map.siod_pin, gpio_map.sioc_pin);
    delay(100);
    
    // Try to detect OV2640 first (most common)
    Wire.beginTransmission(0x30); // OV2640 I2C address
    Wire.write(0x0A); // Chip ID register
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x30, (uint8_t)1);
        if (Wire.available()) {
            uint8_t chip_id = Wire.read();
            if (chip_id == 0x26) {
                DEBUG_PRINTLN("ESP-EYE: OV2640 sensor detected");
                return SENSOR_OV2640;
            }
        }
    }
    
    // Could add detection for other sensors here
    DEBUG_PRINTLN("ESP-EYE: Sensor detection failed, assuming OV2640");
    return SENSOR_OV2640;
}