/**
 * @file sensor_interface.h
 * @brief Base interface for all sensor types in WildCAM ESP32
 * @version 1.0.0
 * 
 * Provides a unified abstraction layer for different sensor types including
 * camera, environmental, motion, acoustic, and other sensor modalities.
 */

#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <Arduino.h>
#include <vector>

// ===========================
// SENSOR TYPE DEFINITIONS
// ===========================

/**
 * @brief Unified sensor type enumeration
 * Combines camera, environmental, and other sensor types
 */
enum class UnifiedSensorType {
    // Camera sensors
    SENSOR_CAMERA_OV2640 = 0,
    SENSOR_CAMERA_OV3660,
    SENSOR_CAMERA_OV5640,
    SENSOR_CAMERA_GC032A,
    SENSOR_CAMERA_SC030IOT,
    
    // Environmental sensors
    SENSOR_BME280,              // Temperature, humidity, pressure
    SENSOR_DHT11,               // Temperature, humidity
    SENSOR_DHT22,               // Temperature, humidity (higher accuracy)
    SENSOR_DS18B20,             // Temperature (OneWire)
    SENSOR_BH1750,              // Light intensity
    SENSOR_TSL2591,             // Advanced light sensor
    SENSOR_SGP30,               // Air quality (TVOC, eCO2)
    
    // Motion sensors
    SENSOR_PIR,                 // Passive infrared motion
    SENSOR_RCWL0516,            // Microwave motion
    SENSOR_MLX90640,            // Thermal imaging
    
    // Distance sensors
    SENSOR_HC_SR04,             // Ultrasonic distance
    SENSOR_VL53L0X,             // Time-of-flight laser distance
    SENSOR_VL53L1X,             // Long-range ToF
    
    // Gas sensors
    SENSOR_MQ2,                 // Smoke, LPG, CO
    SENSOR_MQ135,               // Air quality, CO2
    SENSOR_MQ7,                 // Carbon monoxide
    
    // Acoustic sensors
    SENSOR_MICROPHONE_ANALOG,   // Analog microphone
    SENSOR_MICROPHONE_I2S,      // I2S digital microphone
    SENSOR_MICROPHONE_PDM,      // PDM microphone
    
    // Power monitoring
    SENSOR_MAX17048,            // Battery fuel gauge
    SENSOR_INA219,              // Current/voltage monitor
    
    // Other sensors
    SENSOR_GPS,                 // GPS module
    SENSOR_MAGNETOMETER,        // Compass
    SENSOR_ACCELEROMETER,       // Motion/orientation
    SENSOR_GYROSCOPE,           // Angular velocity
    
    SENSOR_UNKNOWN = 255
};

/**
 * @brief Sensor communication interface type
 */
enum class SensorInterface {
    INTERFACE_I2C = 0,
    INTERFACE_SPI,
    INTERFACE_UART,
    INTERFACE_ANALOG,
    INTERFACE_DIGITAL,
    INTERFACE_ONEWIRE,
    INTERFACE_I2S,
    INTERFACE_PDM,
    INTERFACE_CAMERA,
    INTERFACE_UNKNOWN
};

/**
 * @brief Sensor status
 */
enum class SensorStatus {
    STATUS_UNINITIALIZED = 0,
    STATUS_INITIALIZING,
    STATUS_READY,
    STATUS_READING,
    STATUS_ERROR,
    STATUS_DISCONNECTED,
    STATUS_CALIBRATING,
    STATUS_SLEEPING
};

/**
 * @brief Sensor capabilities
 */
struct SensorCapabilities {
    UnifiedSensorType type;
    const char* name;
    const char* manufacturer;
    SensorInterface interface;
    uint8_t i2c_address;        // For I2C sensors (0 if not applicable)
    uint16_t poll_interval_ms;  // Recommended polling interval
    bool supports_interrupts;
    bool requires_calibration;
    float min_value;
    float max_value;
    const char* unit;
    uint8_t data_size;          // Size of data in bytes
};

/**
 * @brief Sensor configuration
 */
struct SensorConfig {
    bool enabled;
    uint16_t poll_interval_ms;
    bool use_interrupts;
    uint8_t interrupt_pin;
    float calibration_offset;
    float calibration_scale;
    bool low_power_mode;
    uint32_t timeout_ms;
};

/**
 * @brief Generic sensor reading structure
 */
struct SensorReading {
    uint32_t timestamp;
    UnifiedSensorType sensor_type;
    SensorStatus status;
    float values[8];            // Up to 8 sensor values
    uint8_t value_count;
    const char* unit;
    bool valid;
};

// ===========================
// BASE SENSOR INTERFACE
// ===========================

/**
 * @brief Abstract base class for all sensors
 * 
 * Provides a unified interface for sensor initialization, reading,
 * and management across different sensor types.
 */
class BaseSensor {
public:
    virtual ~BaseSensor() = default;
    
    // ===========================
    // IDENTIFICATION
    // ===========================
    
    /**
     * @brief Get sensor type
     */
    virtual UnifiedSensorType getSensorType() const = 0;
    
    /**
     * @brief Get sensor name
     */
    virtual const char* getSensorName() const = 0;
    
    /**
     * @brief Get sensor capabilities
     */
    virtual SensorCapabilities getCapabilities() const = 0;
    
    // ===========================
    // INITIALIZATION & CONTROL
    // ===========================
    
    /**
     * @brief Initialize the sensor
     * @return true if initialization successful
     */
    virtual bool init() = 0;
    
    /**
     * @brief Configure the sensor
     * @param config Sensor configuration
     * @return true if configuration successful
     */
    virtual bool configure(const SensorConfig& config) = 0;
    
    /**
     * @brief Check if sensor is initialized
     */
    virtual bool isInitialized() const = 0;
    
    /**
     * @brief Get current sensor status
     */
    virtual SensorStatus getStatus() const = 0;
    
    /**
     * @brief Reset the sensor
     */
    virtual void reset() = 0;
    
    /**
     * @brief Cleanup sensor resources
     */
    virtual void cleanup() = 0;
    
    // ===========================
    // DATA READING
    // ===========================
    
    /**
     * @brief Read sensor data
     * @return Sensor reading structure
     */
    virtual SensorReading read() = 0;
    
    /**
     * @brief Check if new data is available
     * @return true if new data ready to be read
     */
    virtual bool dataAvailable() = 0;
    
    /**
     * @brief Get last reading without triggering new read
     */
    virtual SensorReading getLastReading() const = 0;
    
    /**
     * @brief Get reading age in milliseconds
     */
    virtual uint32_t getReadingAge() const = 0;
    
    // ===========================
    // CALIBRATION
    // ===========================
    
    /**
     * @brief Calibrate the sensor
     * @return true if calibration successful
     */
    virtual bool calibrate() { return true; }
    
    /**
     * @brief Check if sensor needs calibration
     */
    virtual bool needsCalibration() const { return false; }
    
    // ===========================
    // POWER MANAGEMENT
    // ===========================
    
    /**
     * @brief Enter low power mode
     */
    virtual void sleep() {}
    
    /**
     * @brief Wake from low power mode
     */
    virtual void wake() {}
    
    /**
     * @brief Get estimated power consumption in mW
     */
    virtual float getPowerConsumption() const { return 0.0f; }
    
    // ===========================
    // DIAGNOSTICS
    // ===========================
    
    /**
     * @brief Test sensor communication
     * @return true if sensor responds correctly
     */
    virtual bool testCommunication() = 0;
    
    /**
     * @brief Get last error message
     */
    virtual const char* getLastError() const { return ""; }
    
    /**
     * @brief Get sensor health score (0-100)
     */
    virtual uint8_t getHealthScore() const { return 100; }

protected:
    SensorStatus status_ = SensorStatus::STATUS_UNINITIALIZED;
    SensorConfig config_;
    SensorReading last_reading_;
    uint32_t last_read_time_ = 0;
    char last_error_[128] = "";
};

// ===========================
// SENSOR FACTORY
// ===========================

/**
 * @brief Factory for creating sensor instances
 */
class SensorFactory {
public:
    /**
     * @brief Create a sensor instance based on type
     * @param type Sensor type to create
     * @return Pointer to sensor instance or nullptr if failed
     */
    static BaseSensor* createSensor(UnifiedSensorType type);
    
    /**
     * @brief Auto-detect and create available sensors
     * @return Vector of detected sensor instances
     */
    static std::vector<BaseSensor*> detectSensors();
    
    /**
     * @brief Check if a sensor type is available
     * @param type Sensor type to check
     * @return true if sensor is detected
     */
    static bool isSensorAvailable(UnifiedSensorType type);
};

// ===========================
// SENSOR REGISTRY
// ===========================

/**
 * @brief Registry for managing multiple sensors
 */
class SensorRegistry {
public:
    SensorRegistry();
    ~SensorRegistry();
    
    /**
     * @brief Initialize all registered sensors
     */
    bool initAll();
    
    /**
     * @brief Register a sensor
     */
    bool registerSensor(BaseSensor* sensor);
    
    /**
     * @brief Unregister a sensor
     */
    bool unregisterSensor(UnifiedSensorType type);
    
    /**
     * @brief Get sensor by type
     */
    BaseSensor* getSensor(UnifiedSensorType type);
    
    /**
     * @brief Get all registered sensors
     */
    std::vector<BaseSensor*> getAllSensors();
    
    /**
     * @brief Read all sensors
     */
    std::vector<SensorReading> readAll();
    
    /**
     * @brief Get count of registered sensors
     */
    size_t getSensorCount() const { return sensors_.size(); }
    
    /**
     * @brief Cleanup all sensors
     */
    void cleanup();

private:
    std::vector<BaseSensor*> sensors_;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

/**
 * @brief Convert sensor type to string
 */
const char* sensorTypeToString(UnifiedSensorType type);

/**
 * @brief Convert interface type to string
 */
const char* interfaceTypeToString(SensorInterface interface);

/**
 * @brief Convert status to string
 */
const char* statusToString(SensorStatus status);

#endif // SENSOR_INTERFACE_H
