/**
 * @file sensor_interface.cpp
 * @brief Implementation of sensor interface utilities
 */

#include "sensor_interface.h"

// ===========================
// UTILITY FUNCTION IMPLEMENTATIONS
// ===========================

const char* sensorTypeToString(UnifiedSensorType type) {
    switch (type) {
        // Camera sensors
        case UnifiedSensorType::SENSOR_CAMERA_OV2640: return "OV2640";
        case UnifiedSensorType::SENSOR_CAMERA_OV3660: return "OV3660";
        case UnifiedSensorType::SENSOR_CAMERA_OV5640: return "OV5640";
        case UnifiedSensorType::SENSOR_CAMERA_GC032A: return "GC032A";
        case UnifiedSensorType::SENSOR_CAMERA_SC030IOT: return "SC030IOT";
        
        // Environmental sensors
        case UnifiedSensorType::SENSOR_BME280: return "BME280";
        case UnifiedSensorType::SENSOR_DHT11: return "DHT11";
        case UnifiedSensorType::SENSOR_DHT22: return "DHT22";
        case UnifiedSensorType::SENSOR_DS18B20: return "DS18B20";
        case UnifiedSensorType::SENSOR_BH1750: return "BH1750";
        case UnifiedSensorType::SENSOR_TSL2591: return "TSL2591";
        case UnifiedSensorType::SENSOR_SGP30: return "SGP30";
        
        // Motion sensors
        case UnifiedSensorType::SENSOR_PIR: return "PIR";
        case UnifiedSensorType::SENSOR_RCWL0516: return "RCWL-0516";
        case UnifiedSensorType::SENSOR_MLX90640: return "MLX90640";
        
        // Distance sensors
        case UnifiedSensorType::SENSOR_HC_SR04: return "HC-SR04";
        case UnifiedSensorType::SENSOR_VL53L0X: return "VL53L0X";
        case UnifiedSensorType::SENSOR_VL53L1X: return "VL53L1X";
        
        // Gas sensors
        case UnifiedSensorType::SENSOR_MQ2: return "MQ-2";
        case UnifiedSensorType::SENSOR_MQ135: return "MQ-135";
        case UnifiedSensorType::SENSOR_MQ7: return "MQ-7";
        
        // Acoustic sensors
        case UnifiedSensorType::SENSOR_MICROPHONE_ANALOG: return "Analog Microphone";
        case UnifiedSensorType::SENSOR_MICROPHONE_I2S: return "I2S Microphone";
        case UnifiedSensorType::SENSOR_MICROPHONE_PDM: return "PDM Microphone";
        
        // Power monitoring
        case UnifiedSensorType::SENSOR_MAX17048: return "MAX17048";
        case UnifiedSensorType::SENSOR_INA219: return "INA219";
        
        // Other sensors
        case UnifiedSensorType::SENSOR_GPS: return "GPS";
        case UnifiedSensorType::SENSOR_MAGNETOMETER: return "Magnetometer";
        case UnifiedSensorType::SENSOR_ACCELEROMETER: return "Accelerometer";
        case UnifiedSensorType::SENSOR_GYROSCOPE: return "Gyroscope";
        
        case UnifiedSensorType::SENSOR_UNKNOWN:
        default: return "Unknown";
    }
}

const char* interfaceTypeToString(SensorInterface interface) {
    switch (interface) {
        case SensorInterface::INTERFACE_I2C: return "I2C";
        case SensorInterface::INTERFACE_SPI: return "SPI";
        case SensorInterface::INTERFACE_UART: return "UART";
        case SensorInterface::INTERFACE_ANALOG: return "Analog";
        case SensorInterface::INTERFACE_DIGITAL: return "Digital";
        case SensorInterface::INTERFACE_ONEWIRE: return "OneWire";
        case SensorInterface::INTERFACE_I2S: return "I2S";
        case SensorInterface::INTERFACE_PDM: return "PDM";
        case SensorInterface::INTERFACE_CAMERA: return "Camera";
        case SensorInterface::INTERFACE_UNKNOWN:
        default: return "Unknown";
    }
}

const char* statusToString(SensorStatus status) {
    switch (status) {
        case SensorStatus::STATUS_UNINITIALIZED: return "Uninitialized";
        case SensorStatus::STATUS_INITIALIZING: return "Initializing";
        case SensorStatus::STATUS_READY: return "Ready";
        case SensorStatus::STATUS_READING: return "Reading";
        case SensorStatus::STATUS_ERROR: return "Error";
        case SensorStatus::STATUS_DISCONNECTED: return "Disconnected";
        case SensorStatus::STATUS_CALIBRATING: return "Calibrating";
        case SensorStatus::STATUS_SLEEPING: return "Sleeping";
        default: return "Unknown";
    }
}

// ===========================
// SENSOR FACTORY IMPLEMENTATION
// ===========================

BaseSensor* SensorFactory::createSensor(UnifiedSensorType type) {
    // Note: Caller is responsible for deleting the returned sensor
    switch (type) {
        // DHT sensors - requires pin configuration
        case UnifiedSensorType::SENSOR_DHT11:
        case UnifiedSensorType::SENSOR_DHT22:
            // Cannot auto-create without pin information
            return nullptr;
            
        // Ultrasonic sensor - requires pin configuration
        case UnifiedSensorType::SENSOR_HC_SR04:
            return nullptr;
            
        // MQ gas sensors - requires pin configuration
        case UnifiedSensorType::SENSOR_MQ2:
        case UnifiedSensorType::SENSOR_MQ7:
        case UnifiedSensorType::SENSOR_MQ135:
            return nullptr;
            
        // I2C sensors could be auto-created if address is known
        case UnifiedSensorType::SENSOR_BME280:
        case UnifiedSensorType::SENSOR_BH1750:
        case UnifiedSensorType::SENSOR_TSL2591:
        case UnifiedSensorType::SENSOR_SGP30:
        case UnifiedSensorType::SENSOR_MAX17048:
            // Would require I2C library integration
            return nullptr;
            
        default:
            return nullptr;
    }
}

std::vector<BaseSensor*> SensorFactory::detectSensors() {
    std::vector<BaseSensor*> detected_sensors;
    
    // Auto-detection for I2C sensors
    Wire.begin();
    
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        
        if (error == 0) {
            // Device found at this address
            // Try to identify the sensor type based on address
            switch (address) {
                case 0x76:
                case 0x77:
                    // BME280/BMP280 - already supported in advanced_environmental_sensors
                    break;
                case 0x23:
                    // BH1750 light sensor - already supported
                    break;
                case 0x29:
                    // TSL2591 light sensor - already supported
                    break;
                case 0x58:
                    // SGP30 air quality - already supported
                    break;
                case 0x36:
                    // MAX17048 fuel gauge - already supported
                    break;
                // Additional I2C devices can be added here
            }
        }
    }
    
    // Note: GPIO-based sensors (DHT, ultrasonic, PIR, etc.) require
    // configuration to know which pins to probe, so they cannot be
    // fully auto-detected without user configuration
    
    return detected_sensors;
}

bool SensorFactory::isSensorAvailable(UnifiedSensorType type) {
    // Check if sensor type can be detected
    switch (type) {
        // I2C sensors - check bus
        case UnifiedSensorType::SENSOR_BME280:
            return checkI2CAddress(0x76) || checkI2CAddress(0x77);
        case UnifiedSensorType::SENSOR_BH1750:
            return checkI2CAddress(0x23);
        case UnifiedSensorType::SENSOR_TSL2591:
            return checkI2CAddress(0x29);
        case UnifiedSensorType::SENSOR_SGP30:
            return checkI2CAddress(0x58);
        case UnifiedSensorType::SENSOR_MAX17048:
            return checkI2CAddress(0x36);
            
        // GPIO-based sensors cannot be detected without configuration
        default:
            return false;
    }
}

// Helper function to check I2C address
static bool checkI2CAddress(byte address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

// ===========================
// SENSOR REGISTRY IMPLEMENTATION
// ===========================

SensorRegistry::SensorRegistry() {
}

SensorRegistry::~SensorRegistry() {
    cleanup();
}

bool SensorRegistry::initAll() {
    bool all_success = true;
    
    for (auto sensor : sensors_) {
        if (sensor && !sensor->init()) {
            all_success = false;
        }
    }
    
    return all_success;
}

bool SensorRegistry::registerSensor(BaseSensor* sensor) {
    if (!sensor) {
        return false;
    }
    
    // Check if sensor already registered
    for (auto s : sensors_) {
        if (s && s->getSensorType() == sensor->getSensorType()) {
            return false; // Already registered
        }
    }
    
    sensors_.push_back(sensor);
    return true;
}

bool SensorRegistry::unregisterSensor(UnifiedSensorType type) {
    for (auto it = sensors_.begin(); it != sensors_.end(); ++it) {
        if (*it && (*it)->getSensorType() == type) {
            delete *it;
            sensors_.erase(it);
            return true;
        }
    }
    return false;
}

BaseSensor* SensorRegistry::getSensor(UnifiedSensorType type) {
    for (auto sensor : sensors_) {
        if (sensor && sensor->getSensorType() == type) {
            return sensor;
        }
    }
    return nullptr;
}

std::vector<BaseSensor*> SensorRegistry::getAllSensors() {
    return sensors_;
}

std::vector<SensorReading> SensorRegistry::readAll() {
    std::vector<SensorReading> readings;
    
    for (auto sensor : sensors_) {
        if (sensor && sensor->isInitialized()) {
            readings.push_back(sensor->read());
        }
    }
    
    return readings;
}

void SensorRegistry::cleanup() {
    for (auto sensor : sensors_) {
        if (sensor) {
            sensor->cleanup();
            delete sensor;
        }
    }
    sensors_.clear();
}
