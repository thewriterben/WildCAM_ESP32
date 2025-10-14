/**
 * @file ultrasonic_sensor.h
 * @brief HC-SR04 ultrasonic distance sensor implementation
 * @version 1.0.0
 * 
 * Implements the BaseSensor interface for HC-SR04 ultrasonic distance sensor.
 * Provides distance measurements from 2cm to 400cm with ~3mm accuracy.
 */

#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "sensor_interface.h"

/**
 * @brief HC-SR04 ultrasonic distance sensor implementation
 * 
 * Measures distance using ultrasonic sound waves.
 * Range: 2-400cm
 * Accuracy: ~3mm
 * Angle: 15 degrees
 */
class UltrasonicSensor : public BaseSensor {
public:
    /**
     * @brief Constructor
     * @param trigger_pin GPIO pin connected to trigger
     * @param echo_pin GPIO pin connected to echo
     */
    UltrasonicSensor(uint8_t trigger_pin, uint8_t echo_pin);
    
    /**
     * @brief Destructor
     */
    ~UltrasonicSensor() override;
    
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
     * @brief Read distance in centimeters
     */
    float readDistanceCm();
    
    /**
     * @brief Read distance in inches
     */
    float readDistanceInch();
    
    /**
     * @brief Get median distance from multiple readings
     * @param samples Number of samples to take
     */
    float readDistanceMedian(uint8_t samples = 5);
    
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
    uint8_t trigger_pin_;
    uint8_t echo_pin_;
    uint32_t timeout_us_;
    uint32_t consecutive_failures_;
    
    long measurePulse();
    bool validateDistance(float distance);
    float microsecondsToCm(long microseconds);
};

#endif // ULTRASONIC_SENSOR_H
