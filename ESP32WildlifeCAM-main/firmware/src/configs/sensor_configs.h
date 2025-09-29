/**
 * @file sensor_configs.h
 * @brief Camera sensor configuration profiles
 */

#ifndef SENSOR_CONFIGS_H
#define SENSOR_CONFIGS_H

#include "../hal/camera_board.h"

// Sensor capability structure
struct SensorCapabilities {
    const char* name;
    uint16_t max_width;
    uint16_t max_height;
    uint8_t max_fps;
    bool supports_night_mode;
    bool has_autofocus;
    uint8_t min_light_lux;
    float power_consumption_mw;
};

// OV2640 Configuration (2MP)
const SensorCapabilities OV2640_CAPABILITIES = {
    .name = "OV2640",
    .max_width = 1600,
    .max_height = 1200,
    .max_fps = 30,
    .supports_night_mode = false,
    .has_autofocus = false,
    .min_light_lux = 10,
    .power_consumption_mw = 120
};

// OV3660 Configuration (3MP)
const SensorCapabilities OV3660_CAPABILITIES = {
    .name = "OV3660",
    .max_width = 2048,
    .max_height = 1536,
    .max_fps = 15,
    .supports_night_mode = true,
    .has_autofocus = false,
    .min_light_lux = 5,
    .power_consumption_mw = 150
};

// OV5640 Configuration (5MP)
const SensorCapabilities OV5640_CAPABILITIES = {
    .name = "OV5640",
    .max_width = 2592,
    .max_height = 1944,
    .max_fps = 15,
    .supports_night_mode = true,
    .has_autofocus = true,
    .min_light_lux = 3,
    .power_consumption_mw = 200
};

// GC032A Configuration (VGA)
const SensorCapabilities GC032A_CAPABILITIES = {
    .name = "GC032A",
    .max_width = 640,
    .max_height = 480,
    .max_fps = 60,
    .supports_night_mode = false,
    .has_autofocus = false,
    .min_light_lux = 15,
    .power_consumption_mw = 80
};

// HM01B0 Configuration (QVGA Monochrome)
const SensorCapabilities HM01B0_CAPABILITIES = {
    .name = "HM01B0",
    .max_width = 320,
    .max_height = 240,
    .max_fps = 60,
    .supports_night_mode = true,
    .has_autofocus = false,
    .min_light_lux = 1,
    .power_consumption_mw = 50
};

// SC031GS Configuration (VGA)
const SensorCapabilities SC031GS_CAPABILITIES = {
    .name = "SC031GS",
    .max_width = 640,
    .max_height = 480,
    .max_fps = 120,
    .supports_night_mode = true,
    .has_autofocus = false,
    .min_light_lux = 2,
    .power_consumption_mw = 70
};

// Wildlife photography settings for different sensors
struct WildlifeSettings {
    int8_t brightness;        // -2 to 2
    int8_t contrast;          // -2 to 2
    int8_t saturation;        // -2 to 2
    uint8_t wb_mode;          // 0-Auto, 1-Sunny, 2-Cloudy, 3-Office, 4-Home
    uint16_t aec_value;       // 0 to 1200
    uint8_t agc_gain;         // 0 to 30
    gainceiling_t gainceiling; // 0 to 6
    bool hmirror;             // Horizontal mirror
    bool vflip;               // Vertical flip
};

// Optimized settings for OV2640 wildlife photography
const WildlifeSettings OV2640_WILDLIFE_SETTINGS = {
    .brightness = 0,
    .contrast = 0,
    .saturation = 0,
    .wb_mode = 0,           // Auto white balance
    .aec_value = 300,
    .agc_gain = 0,
    .gainceiling = (gainceiling_t)0,
    .hmirror = false,
    .vflip = false
};

// Optimized settings for OV3660 wildlife photography
const WildlifeSettings OV3660_WILDLIFE_SETTINGS = {
    .brightness = 0,
    .contrast = 1,          // Slightly higher contrast
    .saturation = 0,
    .wb_mode = 0,
    .aec_value = 250,       // Lower for better light sensitivity
    .agc_gain = 5,          // Higher gain for low light
    .gainceiling = (gainceiling_t)2,
    .hmirror = false,
    .vflip = false
};

// Optimized settings for OV5640 wildlife photography
const WildlifeSettings OV5640_WILDLIFE_SETTINGS = {
    .brightness = 0,
    .contrast = 1,
    .saturation = 1,        // Enhanced saturation for wildlife colors
    .wb_mode = 0,
    .aec_value = 200,       // Excellent low light performance
    .agc_gain = 10,
    .gainceiling = (gainceiling_t)4,
    .hmirror = false,
    .vflip = false
};

// Power-optimized settings for ultra-low power sensors
const WildlifeSettings LOW_POWER_WILDLIFE_SETTINGS = {
    .brightness = -1,       // Slightly darker to save power
    .contrast = 0,
    .saturation = -1,       // Reduced processing
    .wb_mode = 1,           // Fixed sunny mode (less processing)
    .aec_value = 400,       // Higher exposure value
    .agc_gain = 0,          // No additional gain
    .gainceiling = (gainceiling_t)0,
    .hmirror = false,
    .vflip = false
};

// Function to get sensor capabilities by type
inline const SensorCapabilities* getSensorCapabilities(SensorType sensor_type) {
    switch (sensor_type) {
        case SENSOR_OV2640: return &OV2640_CAPABILITIES;
        case SENSOR_OV3660: return &OV3660_CAPABILITIES;
        case SENSOR_OV5640: return &OV5640_CAPABILITIES;
        case SENSOR_GC032A: return &GC032A_CAPABILITIES;
        case SENSOR_HM01B0: return &HM01B0_CAPABILITIES;
        case SENSOR_SC031GS: return &SC031GS_CAPABILITIES;
        default: return nullptr;
    }
}

// Function to get wildlife settings by sensor type
inline const WildlifeSettings* getWildlifeSettings(SensorType sensor_type) {
    switch (sensor_type) {
        case SENSOR_OV2640: return &OV2640_WILDLIFE_SETTINGS;
        case SENSOR_OV3660: return &OV3660_WILDLIFE_SETTINGS;
        case SENSOR_OV5640: return &OV5640_WILDLIFE_SETTINGS;
        case SENSOR_GC032A:
        case SENSOR_HM01B0:
        case SENSOR_SC031GS:
            return &LOW_POWER_WILDLIFE_SETTINGS;
        default: return &OV2640_WILDLIFE_SETTINGS; // Default fallback
    }
}

#endif // SENSOR_CONFIGS_H