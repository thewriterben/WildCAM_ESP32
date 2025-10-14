/**
 * @file mq_sensor.h
 * @brief MQ-series gas sensor implementation (MQ-2, MQ-7, MQ-135)
 * @version 1.0.0
 * 
 * Implements the BaseSensor interface for MQ-series gas sensors.
 * These sensors detect various gases through analog readings.
 */

#ifndef MQ_SENSOR_H
#define MQ_SENSOR_H

#include "sensor_interface.h"

/**
 * @brief MQ sensor type specification
 */
enum class MQType {
    MQ2,    // Smoke, LPG, CO, Alcohol, Propane, Hydrogen
    MQ7,    // Carbon Monoxide
    MQ135   // Air quality (CO2, NH3, NOx, Alcohol, Benzene, Smoke)
};

/**
 * @brief MQ gas sensor implementation
 * 
 * Provides gas concentration readings from MQ-series sensors.
 * MQ-2: Detects smoke and combustible gases
 * MQ-7: Detects carbon monoxide
 * MQ-135: Detects air quality and various gases
 */
class MQSensor : public BaseSensor {
public:
    /**
     * @brief Constructor
     * @param pin Analog GPIO pin connected to sensor
     * @param type MQ sensor type
     */
    MQSensor(uint8_t pin, MQType type);
    
    /**
     * @brief Destructor
     */
    ~MQSensor() override;
    
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
    // CALIBRATION
    // ===========================
    
    bool calibrate() override;
    bool needsCalibration() const override;
    
    /**
     * @brief Set R0 resistance value from calibration
     */
    void setR0(float r0);
    
    /**
     * @brief Get R0 resistance value
     */
    float getR0() const { return r0_; }
    
    // ===========================
    // SPECIFIC READINGS
    // ===========================
    
    /**
     * @brief Read raw analog value
     */
    int readRaw();
    
    /**
     * @brief Read sensor resistance
     */
    float readResistance();
    
    /**
     * @brief Read gas concentration in PPM
     */
    float readPPM();
    
    /**
     * @brief Check if gas detected above threshold
     */
    bool isGasDetected(float threshold_ppm = 100.0f);
    
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
    uint8_t pin_;
    MQType mq_type_;
    float r0_;                  // Calibration resistance in clean air
    float rl_;                  // Load resistance (typically 10kΩ)
    bool calibrated_;
    uint32_t preheat_time_ms_;  // Sensor preheat time
    uint32_t init_time_;
    uint32_t consecutive_failures_;
    
    float calculatePPM(float rs_ratio);
    bool validateReading(float value);
};

#endif // MQ_SENSOR_H
