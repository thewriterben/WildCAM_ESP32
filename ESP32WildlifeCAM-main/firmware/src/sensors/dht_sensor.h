/**
 * @file dht_sensor.h
 * @brief DHT11/DHT22 temperature and humidity sensor implementation
 * @version 1.0.0
 * 
 * Implements the BaseSensor interface for DHT11 and DHT22 sensors.
 * These sensors provide temperature and humidity readings via a 
 * single-wire digital interface.
 */

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include "sensor_interface.h"
#include <DHT.h>

/**
 * @brief DHT sensor type specification
 */
enum class DHTType {
    DHT11 = 11,
    DHT22 = 22
};

/**
 * @brief DHT sensor implementation
 * 
 * Provides temperature and humidity readings from DHT11/DHT22 sensors.
 * DHT11: 0-50°C, 20-80% RH, ±2°C, ±5% RH accuracy
 * DHT22: -40-80°C, 0-100% RH, ±0.5°C, ±2% RH accuracy
 */
class DHTSensor : public BaseSensor {
public:
    /**
     * @brief Constructor
     * @param pin GPIO pin connected to DHT sensor
     * @param type DHT sensor type (DHT11 or DHT22)
     */
    DHTSensor(uint8_t pin, DHTType type = DHTType::DHT22);
    
    /**
     * @brief Destructor
     */
    ~DHTSensor() override;
    
    // ===========================
    // IDENTIFICATION
    // ===========================
    
    UnifiedSensorType getSensorType() const override;
    const char* getSensorName() const override;
    SensorCapabilities getCapabilities() const override;
    
    // ===========================
    // INITIALIZATION & CONTROL
    // ===========================
    
    bool init() override;
    bool configure(const SensorConfig& config) override;
    bool isInitialized() const override;
    SensorStatus getStatus() const override;
    void reset() override;
    void cleanup() override;
    
    // ===========================
    // DATA READING
    // ===========================
    
    SensorReading read() override;
    bool dataAvailable() override;
    SensorReading getLastReading() const override;
    uint32_t getReadingAge() const override;
    
    // ===========================
    // SPECIFIC READINGS
    // ===========================
    
    /**
     * @brief Read temperature in Celsius
     */
    float readTemperature();
    
    /**
     * @brief Read temperature in Fahrenheit
     */
    float readTemperatureF();
    
    /**
     * @brief Read relative humidity
     */
    float readHumidity();
    
    /**
     * @brief Compute heat index in Celsius
     */
    float computeHeatIndex();
    
    /**
     * @brief Compute heat index in Fahrenheit
     */
    float computeHeatIndexF();
    
    // ===========================
    // DIAGNOSTICS
    // ===========================
    
    bool testCommunication() override;
    const char* getLastError() const override;
    uint8_t getHealthScore() const override;
    
    // ===========================
    // POWER MANAGEMENT
    // ===========================
    
    float getPowerConsumption() const override;

private:
    DHT* dht_;
    uint8_t pin_;
    DHTType dht_type_;
    uint32_t min_sample_interval_ms_;
    uint32_t consecutive_failures_;
    
    bool validateReading(float temp, float humidity);
};

#endif // DHT_SENSOR_H
