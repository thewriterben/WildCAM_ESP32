/**
 * @file esp32_s3_cam.cpp
 * @brief ESP32-S3-CAM board implementation
 */

#include "esp32_s3_cam.h"
#include "../config.h"
#include "../configs/sensor_configs.h"
#include "../configs/board_profiles.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <Wire.h>

ESP32S3CAM::ESP32S3CAM() {
    board_type = BOARD_ESP32_S3_CAM;
    sensor_type = SENSOR_UNKNOWN;
    initialized = false;
}

bool ESP32S3CAM::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing ESP32-S3-CAM...");
    
    // Check if we're actually running on ESP32-S3
    if (strstr(getChipModel(), "ESP32-S3") == nullptr) {
        DEBUG_PRINTLN("Warning: Not running on ESP32-S3 chip");
    }
    
    // ESP32-S3-CAM requires PSRAM for optimal operation
    if (!hasPSRAM()) {
        DEBUG_PRINTLN("Warning: ESP32-S3-CAM works best with PSRAM");
    }
    
    // Setup GPIO pins
    if (!setupPins()) {
        DEBUG_PRINTLN("Failed to setup GPIO pins");
        return false;
    }
    
    // Setup power management
    setupPowerManagement();
    
    // Detect camera sensor
    if (!detectSensor()) {
        DEBUG_PRINTLN("Failed to detect camera sensor");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("ESP32-S3-CAM initialized successfully");
    return true;
}

bool ESP32S3CAM::detectSensor() {
    DEBUG_PRINTLN("Detecting camera sensor on ESP32-S3-CAM...");
    
    // Try to detect specific sensor type
    sensor_type = detectSpecificSensor();
    
    if (sensor_type == SENSOR_UNKNOWN) {
        // Default to OV2640 for compatibility
        DEBUG_PRINTLN("Sensor detection inconclusive, defaulting to OV2640");
        sensor_type = SENSOR_OV2640;
    }
    
    const SensorCapabilities* caps = getSensorCapabilities(sensor_type);
    if (caps) {
        DEBUG_PRINTF("Detected sensor: %s (%dx%d)\n", caps->name, caps->max_width, caps->max_height);
    }
    
    return testSensorCommunication();
}

GPIOMap ESP32S3CAM::getGPIOMap() const {
    // Return ESP32-S3-CAM specific GPIO mapping
    return ESP32_S3_CAM_GPIO_MAP;
}

CameraConfig ESP32S3CAM::getCameraConfig() const {
    CameraConfig config = ESP32_S3_CAM_CAMERA_CONFIG;
    
    // Adjust configuration based on detected sensor
    const SensorCapabilities* caps = getSensorCapabilities(sensor_type);
    if (caps) {
        // Use sensor's maximum resolution if smaller than board default
        if (caps->max_width < 2592) { // QXGA width
            if (caps->max_width >= 1600) {
                config.max_framesize = FRAMESIZE_UXGA;
            } else if (caps->max_width >= 800) {
                config.max_framesize = FRAMESIZE_SVGA;
            } else {
                config.max_framesize = FRAMESIZE_VGA;
            }
        }
        
        // Adjust quality based on sensor capabilities
        if (sensor_type == SENSOR_OV5640) {
            config.jpeg_quality = 8;  // Higher quality for 5MP sensor
            config.fb_count = 3;      // More buffers for high resolution
        } else if (sensor_type == SENSOR_OV3660) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
        }
    }
    
    return config;
}

PowerProfile ESP32S3CAM::getPowerProfile() const {
    PowerProfile profile = ESP32_S3_CAM_POWER_PROFILE;
    
    // Adjust power profile based on sensor type
    const SensorCapabilities* caps = getSensorCapabilities(sensor_type);
    if (caps) {
        profile.camera_current_ma = (uint32_t)(caps->power_consumption_mw / 3.3f);
    }
    
    return profile;
}

DisplayProfile ESP32S3CAM::getDisplayProfile() const {
    // ESP32-S3-CAM typically doesn't have built-in display
    // Return no display profile (same as AI-Thinker ESP32-CAM)
    return NO_DISPLAY_PROFILE;
}

bool ESP32S3CAM::configureSensor(sensor_t* sensor) {
    if (!sensor) return false;
    
    DEBUG_PRINTLN("Configuring ESP32-S3-CAM sensor for wildlife photography...");
    
    configureS3OptimizedSettings(sensor);
    return true;
}

bool ESP32S3CAM::setupPins() {
    setupCameraPins();
    return true;
}

void ESP32S3CAM::flashLED(bool state) {
    GPIOMap gpio_map = getGPIOMap();
    if (gpio_map.led_pin >= 0) {
        digitalWrite(gpio_map.led_pin, state ? HIGH : LOW);
    }
}

void ESP32S3CAM::enterSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power down camera if power down pin is available
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, HIGH);
    }
    
    // ESP32-S3 specific sleep optimizations
    DEBUG_PRINTLN("ESP32-S3-CAM entering optimized sleep mode");
}

void ESP32S3CAM::exitSleepMode() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Power up camera if power down pin is available
    if (gpio_map.pwdn_pin >= 0) {
        digitalWrite(gpio_map.pwdn_pin, LOW);
        delay(100); // Allow camera to stabilize
    }
    
    DEBUG_PRINTLN("ESP32-S3-CAM exiting sleep mode");
}

float ESP32S3CAM::getBatteryVoltage() {
    // ESP32-S3-CAM may have battery monitoring capability
    // This would need specific implementation based on hardware design
    return 3.3; // Placeholder
}

bool ESP32S3CAM::hasPSRAM() const {
    return psramFound();
}

uint32_t ESP32S3CAM::getChipId() const {
    uint64_t chip_id = ESP.getEfuseMac();
    return (uint32_t)(chip_id >> 24);
}

const char* ESP32S3CAM::getChipModel() const {
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

void ESP32S3CAM::configureS3OptimizedSettings(sensor_t* sensor) {
    // Get sensor-specific wildlife settings
    const WildlifeSettings* settings = getWildlifeSettings(sensor_type);
    if (!settings) {
        DEBUG_PRINTLN("No specific settings found, using default");
        settings = &OV2640_WILDLIFE_SETTINGS;
    }
    
    DEBUG_PRINTF("Applying ESP32-S3 optimized settings for %s\n", 
                getSensorCapabilities(sensor_type)->name);
    
    // Apply wildlife-optimized settings with ESP32-S3 enhancements
    sensor->set_brightness(sensor, settings->brightness);
    sensor->set_contrast(sensor, settings->contrast + 1); // Slightly higher contrast for S3
    sensor->set_saturation(sensor, settings->saturation);
    sensor->set_special_effect(sensor, 0);
    sensor->set_whitebal(sensor, 1);
    sensor->set_awb_gain(sensor, 1);
    sensor->set_wb_mode(sensor, settings->wb_mode);
    
    // Enhanced exposure settings for ESP32-S3's processing power
    sensor->set_exposure_ctrl(sensor, 1);
    sensor->set_aec2(sensor, 1);  // Enable AEC2 for better performance
    sensor->set_ae_level(sensor, 0);
    sensor->set_aec_value(sensor, settings->aec_value - 50); // Better low light
    sensor->set_gain_ctrl(sensor, 1);
    sensor->set_agc_gain(sensor, settings->agc_gain);
    sensor->set_gainceiling(sensor, settings->gainceiling);
    
    // Enhanced image processing for wildlife
    sensor->set_bpc(sensor, 1);    // Enable bad pixel correction with S3's processing power
    sensor->set_wpc(sensor, 1);
    sensor->set_raw_gma(sensor, 1);
    sensor->set_lenc(sensor, 1);
    
    // Orientation
    sensor->set_hmirror(sensor, settings->hmirror ? 1 : 0);
    sensor->set_vflip(sensor, settings->vflip ? 1 : 0);
    sensor->set_dcw(sensor, 1);
    sensor->set_colorbar(sensor, 0);
    
    // ESP32-S3 specific optimizations
    if (hasPSRAM()) {
        // Take advantage of PSRAM for better image quality
        DEBUG_PRINTLN("PSRAM detected - enabling enhanced quality settings");
        
        // Additional quality improvements possible with PSRAM
        if (sensor_type == SENSOR_OV5640) {
            sensor->set_aec_value(sensor, 150); // Even better low light for 5MP
        }
    }
    
    DEBUG_PRINTLN("ESP32-S3 optimized sensor settings applied");
}

bool ESP32S3CAM::testSensorCommunication() {
    // Test I2C communication with camera sensor
    Wire.begin(getGPIOMap().siod_pin, getGPIOMap().sioc_pin);
    
    // Common camera sensor I2C addresses
    uint8_t sensor_addresses[] = {0x30, 0x3C, 0x21, 0x60};
    
    for (uint8_t addr : sensor_addresses) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            DEBUG_PRINTF("Camera sensor found at I2C address 0x%02X\n", addr);
            return true;
        }
    }
    
    DEBUG_PRINTLN("No camera sensor detected on I2C bus");
    return false;
}

void ESP32S3CAM::setupCameraPins() {
    GPIOMap gpio_map = getGPIOMap();
    
    // Setup LED pin
    if (gpio_map.led_pin >= 0) {
        pinMode(gpio_map.led_pin, OUTPUT);
        digitalWrite(gpio_map.led_pin, LOW);
    }
    
    // Setup power down pin if available
    if (gpio_map.pwdn_pin >= 0) {
        pinMode(gpio_map.pwdn_pin, OUTPUT);
        digitalWrite(gpio_map.pwdn_pin, LOW); // Camera powered on
    }
    
    // Setup reset pin if available
    if (gpio_map.reset_pin >= 0) {
        pinMode(gpio_map.reset_pin, OUTPUT);
        digitalWrite(gpio_map.reset_pin, HIGH); // Camera not in reset
    }
}

void ESP32S3CAM::setupPowerManagement() {
    // ESP32-S3 specific power management setup
    // This could include voltage regulators, power switches, etc.
    DEBUG_PRINTLN("Configuring ESP32-S3 power management");
}

SensorType ESP32S3CAM::detectSpecificSensor() {
    // Try to detect specific sensor by reading manufacturer/product IDs
    // This is a simplified implementation
    
    Wire.begin(getGPIOMap().siod_pin, getGPIOMap().sioc_pin);
    
    // Try OV5640 first (5MP sensor)
    Wire.beginTransmission(0x3C);
    Wire.write(0x30);  // Chip ID high register
    Wire.write(0x0A);
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x3C, 1);
        if (Wire.available()) {
            uint8_t chip_id_high = Wire.read();
            if (chip_id_high == 0x56) { // OV5640 chip ID high byte
                DEBUG_PRINTLN("OV5640 sensor detected");
                return SENSOR_OV5640;
            }
        }
    }
    
    // Try OV3660 (3MP sensor)
    Wire.beginTransmission(0x3C);
    Wire.write(0x30);  // Chip ID high register
    Wire.write(0x0A);
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x3C, 1);
        if (Wire.available()) {
            uint8_t chip_id_high = Wire.read();
            if (chip_id_high == 0x36) { // OV3660 chip ID high byte
                DEBUG_PRINTLN("OV3660 sensor detected");
                return SENSOR_OV3660;
            }
        }
    }
    
    // Try OV2640 (2MP sensor)
    Wire.beginTransmission(0x30);
    Wire.write(0x0A);  // Chip ID high register
    if (Wire.endTransmission() == 0) {
        Wire.requestFrom(0x30, 1);
        if (Wire.available()) {
            uint8_t chip_id_high = Wire.read();
            if (chip_id_high == 0x26) { // OV2640 chip ID high byte
                DEBUG_PRINTLN("OV2640 sensor detected");
                return SENSOR_OV2640;
            }
        }
    }
    
    DEBUG_PRINTLN("Unable to identify specific sensor type");
    return SENSOR_UNKNOWN;
}