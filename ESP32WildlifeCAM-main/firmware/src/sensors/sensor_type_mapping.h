/**
 * @file sensor_type_mapping.h
 * @brief Mapping between old and new sensor type definitions
 * @version 1.0.0
 * 
 * This file provides compatibility between existing sensor type enums
 * and the new unified sensor type system. It ensures backward compatibility
 * while allowing gradual migration to the new abstraction layer.
 */

#ifndef SENSOR_TYPE_MAPPING_H
#define SENSOR_TYPE_MAPPING_H

#include "sensor_interface.h"
#include "../configs/sensor_configs.h"

/**
 * @brief Convert HAL SensorType to UnifiedSensorType
 * 
 * Converts the camera sensor types used in HAL (Hardware Abstraction Layer)
 * to the new unified sensor type enumeration.
 * 
 * @param hal_type Camera sensor type from HAL
 * @return Equivalent unified sensor type
 */
inline UnifiedSensorType halSensorToUnified(SensorType hal_type) {
    switch (hal_type) {
        case SENSOR_OV2640:
            return UnifiedSensorType::SENSOR_CAMERA_OV2640;
        case SENSOR_OV3660:
            return UnifiedSensorType::SENSOR_CAMERA_OV3660;
        case SENSOR_OV5640:
            return UnifiedSensorType::SENSOR_CAMERA_OV5640;
        case SENSOR_GC032A:
            return UnifiedSensorType::SENSOR_CAMERA_GC032A;
        case SENSOR_SC030IOT:
            return UnifiedSensorType::SENSOR_CAMERA_SC030IOT;
        case SENSOR_UNKNOWN:
        default:
            return UnifiedSensorType::SENSOR_UNKNOWN;
    }
}

/**
 * @brief Convert UnifiedSensorType to HAL SensorType
 * 
 * Converts unified sensor type back to HAL camera sensor type.
 * Only works for camera sensors.
 * 
 * @param unified_type Unified sensor type
 * @return Equivalent HAL sensor type (SENSOR_UNKNOWN if not a camera)
 */
inline SensorType unifiedToHalSensor(UnifiedSensorType unified_type) {
    switch (unified_type) {
        case UnifiedSensorType::SENSOR_CAMERA_OV2640:
            return SENSOR_OV2640;
        case UnifiedSensorType::SENSOR_CAMERA_OV3660:
            return SENSOR_OV3660;
        case UnifiedSensorType::SENSOR_CAMERA_OV5640:
            return SENSOR_OV5640;
        case UnifiedSensorType::SENSOR_CAMERA_GC032A:
            return SENSOR_GC032A;
        case UnifiedSensorType::SENSOR_CAMERA_SC030IOT:
            return SENSOR_SC030IOT;
        default:
            return SENSOR_UNKNOWN;
    }
}

/**
 * @brief Check if unified sensor type is a camera
 * 
 * @param type Unified sensor type to check
 * @return true if sensor is a camera type
 */
inline bool isUnifiedSensorCamera(UnifiedSensorType type) {
    return type >= UnifiedSensorType::SENSOR_CAMERA_OV2640 &&
           type <= UnifiedSensorType::SENSOR_CAMERA_SC030IOT;
}

/**
 * @brief Check if unified sensor type is environmental
 * 
 * @param type Unified sensor type to check
 * @return true if sensor is an environmental sensor
 */
inline bool isUnifiedSensorEnvironmental(UnifiedSensorType type) {
    return type >= UnifiedSensorType::SENSOR_BME280 &&
           type <= UnifiedSensorType::SENSOR_SGP30;
}

/**
 * @brief Check if unified sensor type is motion-based
 * 
 * @param type Unified sensor type to check
 * @return true if sensor is a motion sensor
 */
inline bool isUnifiedSensorMotion(UnifiedSensorType type) {
    return type >= UnifiedSensorType::SENSOR_PIR &&
           type <= UnifiedSensorType::SENSOR_MLX90640;
}

/**
 * @brief Get sensor category name
 * 
 * @param type Unified sensor type
 * @return String describing sensor category
 */
inline const char* getUnifiedSensorCategory(UnifiedSensorType type) {
    if (isUnifiedSensorCamera(type)) {
        return "Camera";
    } else if (isUnifiedSensorEnvironmental(type)) {
        return "Environmental";
    } else if (isUnifiedSensorMotion(type)) {
        return "Motion";
    } else if (type >= UnifiedSensorType::SENSOR_HC_SR04 &&
               type <= UnifiedSensorType::SENSOR_VL53L1X) {
        return "Distance";
    } else if (type >= UnifiedSensorType::SENSOR_MQ2 &&
               type <= UnifiedSensorType::SENSOR_MQ7) {
        return "Gas";
    } else if (type >= UnifiedSensorType::SENSOR_MICROPHONE_ANALOG &&
               type <= UnifiedSensorType::SENSOR_MICROPHONE_PDM) {
        return "Acoustic";
    } else if (type >= UnifiedSensorType::SENSOR_MAX17048 &&
               type <= UnifiedSensorType::SENSOR_INA219) {
        return "Power";
    } else if (type >= UnifiedSensorType::SENSOR_GPS &&
               type <= UnifiedSensorType::SENSOR_GYROSCOPE) {
        return "Positioning";
    }
    return "Unknown";
}

/**
 * @brief Sensor integration helper structure
 * 
 * Tracks which sensors are available and their configuration
 * in the WildCAM system.
 */
struct SensorIntegration {
    // Camera sensors
    bool has_camera;
    UnifiedSensorType camera_type;
    
    // Environmental sensors
    bool has_temperature;
    bool has_humidity;
    bool has_pressure;
    bool has_light;
    bool has_air_quality;
    
    // Motion sensors
    bool has_pir;
    bool has_thermal;
    
    // Distance sensors
    bool has_ultrasonic;
    
    // Gas sensors
    bool has_gas_sensor;
    
    // Additional sensors
    bool has_gps;
    bool has_battery_monitor;
    
    /**
     * @brief Initialize from configuration
     */
    void initialize() {
        has_camera = false;
        camera_type = UnifiedSensorType::SENSOR_UNKNOWN;
        has_temperature = false;
        has_humidity = false;
        has_pressure = false;
        has_light = false;
        has_air_quality = false;
        has_pir = false;
        has_thermal = false;
        has_ultrasonic = false;
        has_gas_sensor = false;
        has_gps = false;
        has_battery_monitor = false;
    }
    
    /**
     * @brief Register a detected sensor
     */
    void registerSensor(UnifiedSensorType type) {
        if (isUnifiedSensorCamera(type)) {
            has_camera = true;
            camera_type = type;
        } else {
            switch (type) {
                case UnifiedSensorType::SENSOR_DHT11:
                case UnifiedSensorType::SENSOR_DHT22:
                case UnifiedSensorType::SENSOR_DS18B20:
                case UnifiedSensorType::SENSOR_BME280:
                    has_temperature = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_BH1750:
                case UnifiedSensorType::SENSOR_TSL2591:
                    has_light = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_SGP30:
                case UnifiedSensorType::SENSOR_MQ2:
                case UnifiedSensorType::SENSOR_MQ7:
                case UnifiedSensorType::SENSOR_MQ135:
                    has_air_quality = true;
                    has_gas_sensor = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_PIR:
                    has_pir = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_MLX90640:
                    has_thermal = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_HC_SR04:
                case UnifiedSensorType::SENSOR_VL53L0X:
                case UnifiedSensorType::SENSOR_VL53L1X:
                    has_ultrasonic = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_GPS:
                    has_gps = true;
                    break;
                    
                case UnifiedSensorType::SENSOR_MAX17048:
                case UnifiedSensorType::SENSOR_INA219:
                    has_battery_monitor = true;
                    break;
                    
                default:
                    break;
            }
        }
        
        // DHT and BME280 also provide humidity
        if (type == UnifiedSensorType::SENSOR_DHT11 ||
            type == UnifiedSensorType::SENSOR_DHT22 ||
            type == UnifiedSensorType::SENSOR_BME280) {
            has_humidity = true;
        }
        
        // BME280 also provides pressure
        if (type == UnifiedSensorType::SENSOR_BME280) {
            has_pressure = true;
        }
    }
    
    /**
     * @brief Get count of registered sensors
     */
    int getSensorCount() const {
        int count = 0;
        if (has_camera) count++;
        if (has_temperature) count++;
        if (has_humidity) count++;
        if (has_pressure) count++;
        if (has_light) count++;
        if (has_air_quality) count++;
        if (has_pir) count++;
        if (has_thermal) count++;
        if (has_ultrasonic) count++;
        if (has_gas_sensor) count++;
        if (has_gps) count++;
        if (has_battery_monitor) count++;
        return count;
    }
    
    /**
     * @brief Print sensor summary
     */
    void printSummary() const {
        Serial.println("=== Sensor Integration Summary ===");
        Serial.printf("Total sensors: %d\n\n", getSensorCount());
        
        if (has_camera) {
            Serial.printf("Camera: %s\n", sensorTypeToString(camera_type));
        }
        if (has_temperature) Serial.println("✓ Temperature sensor");
        if (has_humidity) Serial.println("✓ Humidity sensor");
        if (has_pressure) Serial.println("✓ Pressure sensor");
        if (has_light) Serial.println("✓ Light sensor");
        if (has_air_quality) Serial.println("✓ Air quality sensor");
        if (has_pir) Serial.println("✓ PIR motion sensor");
        if (has_thermal) Serial.println("✓ Thermal imaging");
        if (has_ultrasonic) Serial.println("✓ Distance sensor");
        if (has_gas_sensor) Serial.println("✓ Gas sensor");
        if (has_gps) Serial.println("✓ GPS");
        if (has_battery_monitor) Serial.println("✓ Battery monitor");
        
        Serial.println("==================================\n");
    }
};

#endif // SENSOR_TYPE_MAPPING_H
