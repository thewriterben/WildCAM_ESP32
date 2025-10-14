/**
 * @file ultrasonic_sensor.cpp
 * @brief HC-SR04 ultrasonic sensor implementation
 */

#include "ultrasonic_sensor.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trigger_pin, uint8_t echo_pin) 
    : trigger_pin_(trigger_pin), echo_pin_(echo_pin), 
      timeout_us_(30000), consecutive_failures_(0) {
}

UltrasonicSensor::~UltrasonicSensor() {
    cleanup();
}

// ===========================
// IDENTIFICATION
// ===========================

UnifiedSensorType UltrasonicSensor::getSensorType() const {
    return UnifiedSensorType::SENSOR_HC_SR04;
}

const char* UltrasonicSensor::getSensorName() const {
    return "HC-SR04";
}

SensorCapabilities UltrasonicSensor::getCapabilities() const {
    SensorCapabilities caps;
    caps.type = getSensorType();
    caps.name = "HC-SR04";
    caps.manufacturer = "Generic";
    caps.interface = SensorInterface::INTERFACE_DIGITAL;
    caps.i2c_address = 0;
    caps.poll_interval_ms = 60; // Minimum 60ms between measurements
    caps.supports_interrupts = false;
    caps.requires_calibration = false;
    caps.min_value = 2.0f;      // 2cm
    caps.max_value = 400.0f;    // 400cm
    caps.unit = "cm";
    caps.data_size = 4;         // 1 float
    
    return caps;
}

// ===========================
// INITIALIZATION & CONTROL
// ===========================

bool UltrasonicSensor::init() {
    if (status_ == SensorStatus::STATUS_READY) {
        return true;
    }
    
    status_ = SensorStatus::STATUS_INITIALIZING;
    
    // Configure pins
    pinMode(trigger_pin_, OUTPUT);
    pinMode(echo_pin_, INPUT);
    
    // Ensure trigger is low
    digitalWrite(trigger_pin_, LOW);
    delayMicroseconds(2);
    
    // Test sensor
    if (!testCommunication()) {
        snprintf(last_error_, sizeof(last_error_), "Failed to communicate with sensor");
        status_ = SensorStatus::STATUS_DISCONNECTED;
        return false;
    }
    
    status_ = SensorStatus::STATUS_READY;
    consecutive_failures_ = 0;
    last_error_[0] = '\0';
    
    return true;
}

bool UltrasonicSensor::configure(const SensorConfig& config) {
    config_ = config;
    
    // Set timeout based on configuration
    if (config_.timeout_ms > 0) {
        timeout_us_ = config_.timeout_ms * 1000;
    }
    
    return true;
}

bool UltrasonicSensor::isInitialized() const {
    return status_ == SensorStatus::STATUS_READY || 
           status_ == SensorStatus::STATUS_READING;
}

SensorStatus UltrasonicSensor::getStatus() const {
    return status_;
}

void UltrasonicSensor::reset() {
    digitalWrite(trigger_pin_, LOW);
    delayMicroseconds(2);
    consecutive_failures_ = 0;
    status_ = SensorStatus::STATUS_READY;
}

void UltrasonicSensor::cleanup() {
    digitalWrite(trigger_pin_, LOW);
    status_ = SensorStatus::STATUS_UNINITIALIZED;
}

// ===========================
// DATA READING
// ===========================

SensorReading UltrasonicSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.sensor_type = getSensorType();
    reading.status = status_;
    reading.valid = false;
    reading.value_count = 1;
    reading.unit = "cm";
    
    if (!isInitialized()) {
        snprintf(last_error_, sizeof(last_error_), "Sensor not initialized");
        reading.status = SensorStatus::STATUS_ERROR;
        return reading;
    }
    
    status_ = SensorStatus::STATUS_READING;
    
    // Measure distance
    float distance = readDistanceCm();
    
    if (isnan(distance) || !validateDistance(distance)) {
        consecutive_failures_++;
        snprintf(last_error_, sizeof(last_error_), 
                "Failed to read distance (failures: %u)", consecutive_failures_);
        
        if (consecutive_failures_ > 5) {
            status_ = SensorStatus::STATUS_DISCONNECTED;
        } else {
            status_ = SensorStatus::STATUS_ERROR;
        }
        
        reading.status = status_;
        return reading;
    }
    
    // Successful reading
    reading.values[0] = distance;
    reading.valid = true;
    reading.status = SensorStatus::STATUS_READY;
    
    status_ = SensorStatus::STATUS_READY;
    consecutive_failures_ = 0;
    last_reading_ = reading;
    last_read_time_ = reading.timestamp;
    last_error_[0] = '\0';
    
    return reading;
}

bool UltrasonicSensor::dataAvailable() {
    // HC-SR04 needs 60ms minimum between readings
    uint32_t time_since_last = millis() - last_read_time_;
    return time_since_last >= 60;
}

SensorReading UltrasonicSensor::getLastReading() const {
    return last_reading_;
}

uint32_t UltrasonicSensor::getReadingAge() const {
    return millis() - last_read_time_;
}

// ===========================
// SPECIFIC READINGS
// ===========================

float UltrasonicSensor::readDistanceCm() {
    long duration = measurePulse();
    
    if (duration <= 0) {
        return NAN;
    }
    
    return microsecondsToCm(duration);
}

float UltrasonicSensor::readDistanceInch() {
    float cm = readDistanceCm();
    if (isnan(cm)) return NAN;
    return cm / 2.54f;
}

float UltrasonicSensor::readDistanceMedian(uint8_t samples) {
    if (samples == 0) samples = 1;
    if (samples > 10) samples = 10;
    
    float readings[10];
    uint8_t valid_count = 0;
    
    for (uint8_t i = 0; i < samples; i++) {
        float dist = readDistanceCm();
        if (!isnan(dist) && validateDistance(dist)) {
            readings[valid_count++] = dist;
        }
        if (i < samples - 1) {
            delay(60); // Wait between readings
        }
    }
    
    if (valid_count == 0) {
        return NAN;
    }
    
    // Sort readings (simple bubble sort)
    for (uint8_t i = 0; i < valid_count - 1; i++) {
        for (uint8_t j = 0; j < valid_count - i - 1; j++) {
            if (readings[j] > readings[j + 1]) {
                float temp = readings[j];
                readings[j] = readings[j + 1];
                readings[j + 1] = temp;
            }
        }
    }
    
    // Return median
    return readings[valid_count / 2];
}

// ===========================
// DIAGNOSTICS
// ===========================

bool UltrasonicSensor::testCommunication() {
    float distance = readDistanceCm();
    return !isnan(distance);
}

const char* UltrasonicSensor::getLastError() const {
    return last_error_;
}

uint8_t UltrasonicSensor::getHealthScore() const {
    if (status_ == SensorStatus::STATUS_DISCONNECTED) {
        return 0;
    }
    if (status_ == SensorStatus::STATUS_ERROR) {
        return 30;
    }
    if (consecutive_failures_ > 0) {
        return 100 - (consecutive_failures_ * 10);
    }
    return 100;
}

// ===========================
// POWER MANAGEMENT
// ===========================

float UltrasonicSensor::getPowerConsumption() const {
    // HC-SR04: ~15mA @ 5V during measurement = 75mW
    // Average consumption depends on measurement frequency
    return 75.0f;
}

// ===========================
// PRIVATE METHODS
// ===========================

long UltrasonicSensor::measurePulse() {
    // Send trigger pulse
    digitalWrite(trigger_pin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger_pin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger_pin_, LOW);
    
    // Measure echo pulse duration
    long duration = pulseIn(echo_pin_, HIGH, timeout_us_);
    
    return duration;
}

bool UltrasonicSensor::validateDistance(float distance) {
    // HC-SR04 range: 2-400cm
    if (isnan(distance)) return false;
    if (distance < 2.0f || distance > 400.0f) return false;
    return true;
}

float UltrasonicSensor::microsecondsToCm(long microseconds) {
    // Speed of sound: 343 m/s = 0.0343 cm/µs
    // Distance = (time * speed) / 2 (round trip)
    return (float)microseconds / 58.0f;
}
