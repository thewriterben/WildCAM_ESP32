/**
 * @file mq_sensor.cpp
 * @brief MQ-series gas sensor implementation
 */

#include "mq_sensor.h"
#include <math.h>

MQSensor::MQSensor(uint8_t pin, MQType type) 
    : pin_(pin), mq_type_(type), r0_(10.0f), rl_(10.0f), 
      calibrated_(false), preheat_time_ms_(20000), consecutive_failures_(0) {
    
    init_time_ = 0;
}

MQSensor::~MQSensor() {
    cleanup();
}

// ===========================
// IDENTIFICATION
// ===========================

UnifiedSensorType MQSensor::getSensorType() const {
    switch (mq_type_) {
        case MQType::MQ2: return UnifiedSensorType::SENSOR_MQ2;
        case MQType::MQ7: return UnifiedSensorType::SENSOR_MQ7;
        case MQType::MQ135: return UnifiedSensorType::SENSOR_MQ135;
        default: return UnifiedSensorType::SENSOR_UNKNOWN;
    }
}

const char* MQSensor::getSensorName() const {
    switch (mq_type_) {
        case MQType::MQ2: return "MQ-2";
        case MQType::MQ7: return "MQ-7";
        case MQType::MQ135: return "MQ-135";
        default: return "Unknown";
    }
}

SensorCapabilities MQSensor::getCapabilities() const {
    SensorCapabilities caps;
    caps.type = getSensorType();
    caps.name = getSensorName();
    caps.manufacturer = "Hanwei Electronics";
    caps.interface = SensorInterface::INTERFACE_ANALOG;
    caps.i2c_address = 0;
    caps.poll_interval_ms = 1000;
    caps.supports_interrupts = false;
    caps.requires_calibration = true;
    caps.min_value = 10.0f;     // Minimum PPM
    caps.max_value = 10000.0f;  // Maximum PPM
    caps.unit = "PPM";
    caps.data_size = 4;         // 1 float
    
    return caps;
}

// ===========================
// INITIALIZATION & CONTROL
// ===========================

bool MQSensor::init() {
    if (status_ == SensorStatus::STATUS_READY) {
        return true;
    }
    
    status_ = SensorStatus::STATUS_INITIALIZING;
    
    // Configure analog pin
    pinMode(pin_, INPUT);
    
    // Record initialization time
    init_time_ = millis();
    
    // MQ sensors require preheat time (typically 20-48 hours for first use,
    // but 20 seconds minimum for warm-up after power on)
    uint32_t elapsed = millis() - init_time_;
    if (elapsed < preheat_time_ms_) {
        snprintf(last_error_, sizeof(last_error_), 
                "Sensor preheating, %lu ms remaining", 
                preheat_time_ms_ - elapsed);
        // We'll still allow initialization but mark as not ready
    }
    
    // Test sensor
    if (!testCommunication()) {
        snprintf(last_error_, sizeof(last_error_), "Failed to read from sensor");
        status_ = SensorStatus::STATUS_ERROR;
        return false;
    }
    
    status_ = SensorStatus::STATUS_READY;
    consecutive_failures_ = 0;
    
    return true;
}

bool MQSensor::configure(const SensorConfig& config) {
    config_ = config;
    
    // Apply calibration values if present
    if (config_.calibration_scale > 0) {
        r0_ = config_.calibration_scale;
        calibrated_ = true;
    }
    
    return true;
}

bool MQSensor::isInitialized() const {
    return status_ == SensorStatus::STATUS_READY || 
           status_ == SensorStatus::STATUS_READING;
}

SensorStatus MQSensor::getStatus() const {
    return status_;
}

void MQSensor::reset() {
    init_time_ = millis();
    consecutive_failures_ = 0;
    status_ = SensorStatus::STATUS_READY;
}

void MQSensor::cleanup() {
    status_ = SensorStatus::STATUS_UNINITIALIZED;
}

// ===========================
// DATA READING
// ===========================

SensorReading MQSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.sensor_type = getSensorType();
    reading.status = status_;
    reading.valid = false;
    reading.value_count = 1;
    reading.unit = "PPM";
    
    if (!isInitialized()) {
        snprintf(last_error_, sizeof(last_error_), "Sensor not initialized");
        reading.status = SensorStatus::STATUS_ERROR;
        return reading;
    }
    
    // Check if sensor has preheated
    uint32_t elapsed = millis() - init_time_;
    if (elapsed < preheat_time_ms_) {
        snprintf(last_error_, sizeof(last_error_), 
                "Sensor still preheating (%lu/%lu ms)", 
                elapsed, preheat_time_ms_);
        reading.status = SensorStatus::STATUS_INITIALIZING;
        return reading;
    }
    
    status_ = SensorStatus::STATUS_READING;
    
    // Read gas concentration
    float ppm = readPPM();
    
    if (isnan(ppm) || !validateReading(ppm)) {
        consecutive_failures_++;
        snprintf(last_error_, sizeof(last_error_), 
                "Failed to read sensor (failures: %u)", consecutive_failures_);
        
        if (consecutive_failures_ > 5) {
            status_ = SensorStatus::STATUS_ERROR;
        }
        
        reading.status = status_;
        return reading;
    }
    
    // Successful reading
    reading.values[0] = ppm;
    reading.valid = true;
    reading.status = SensorStatus::STATUS_READY;
    
    status_ = SensorStatus::STATUS_READY;
    consecutive_failures_ = 0;
    last_reading_ = reading;
    last_read_time_ = reading.timestamp;
    last_error_[0] = '\0';
    
    return reading;
}

bool MQSensor::dataAvailable() {
    // MQ sensors can be read continuously but 1 second is reasonable
    uint32_t time_since_last = millis() - last_read_time_;
    return time_since_last >= 1000;
}

SensorReading MQSensor::getLastReading() const {
    return last_reading_;
}

uint32_t MQSensor::getReadingAge() const {
    return millis() - last_read_time_;
}

// ===========================
// CALIBRATION
// ===========================

bool MQSensor::calibrate() {
    status_ = SensorStatus::STATUS_CALIBRATING;
    
    // Take multiple readings to get average R0 in clean air
    const int num_samples = 10;
    float sum = 0;
    int valid_samples = 0;
    
    for (int i = 0; i < num_samples; i++) {
        float rs = readResistance();
        if (!isnan(rs) && rs > 0) {
            sum += rs;
            valid_samples++;
        }
        delay(500);
    }
    
    if (valid_samples < num_samples / 2) {
        snprintf(last_error_, sizeof(last_error_), "Calibration failed - insufficient valid samples");
        status_ = SensorStatus::STATUS_ERROR;
        return false;
    }
    
    // Calculate R0 (resistance in clean air)
    // For MQ sensors, R0 is the sensor resistance in clean air
    float rs_avg = sum / valid_samples;
    
    // Different sensors have different ratios in clean air
    switch (mq_type_) {
        case MQType::MQ2:
            r0_ = rs_avg / 9.83f;  // Rs/R0 ratio for MQ-2 in clean air
            break;
        case MQType::MQ7:
            r0_ = rs_avg / 27.5f;  // Rs/R0 ratio for MQ-7 in clean air
            break;
        case MQType::MQ135:
            r0_ = rs_avg / 3.6f;   // Rs/R0 ratio for MQ-135 in clean air
            break;
    }
    
    calibrated_ = true;
    status_ = SensorStatus::STATUS_READY;
    
    return true;
}

bool MQSensor::needsCalibration() const {
    return !calibrated_;
}

void MQSensor::setR0(float r0) {
    r0_ = r0;
    calibrated_ = true;
}

// ===========================
// SPECIFIC READINGS
// ===========================

int MQSensor::readRaw() {
    return analogRead(pin_);
}

float MQSensor::readResistance() {
    int raw = readRaw();
    
    // Convert ADC reading to voltage
    // ESP32 ADC: 12-bit (0-4095) for 0-3.3V
    float voltage = ((float)raw / 4095.0f) * 3.3f;
    
    if (voltage <= 0.01f) {
        return NAN; // Avoid division by zero
    }
    
    // Calculate sensor resistance
    // Rs = [(Vc * RL) / Vout] - RL
    float rs = ((3.3f * rl_) / voltage) - rl_;
    
    return rs;
}

float MQSensor::readPPM() {
    float rs = readResistance();
    
    if (isnan(rs) || rs <= 0 || r0_ <= 0) {
        return NAN;
    }
    
    float rs_ratio = rs / r0_;
    return calculatePPM(rs_ratio);
}

bool MQSensor::isGasDetected(float threshold_ppm) {
    float ppm = readPPM();
    if (isnan(ppm)) return false;
    return ppm > threshold_ppm;
}

// ===========================
// DIAGNOSTICS
// ===========================

bool MQSensor::testCommunication() {
    int raw = readRaw();
    return (raw > 0 && raw < 4095); // Valid ADC range
}

const char* MQSensor::getLastError() const {
    return last_error_;
}

uint8_t MQSensor::getHealthScore() const {
    if (!calibrated_) return 50; // Not calibrated
    
    uint32_t elapsed = millis() - init_time_;
    if (elapsed < preheat_time_ms_) {
        return 70; // Still warming up
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

float MQSensor::getPowerConsumption() const {
    // MQ sensors have a heater that consumes power
    // Typical: 150mA @ 5V = 750mW
    return 750.0f;
}

// ===========================
// PRIVATE METHODS
// ===========================

float MQSensor::calculatePPM(float rs_ratio) {
    // Simplified PPM calculation using power law approximation
    // PPM = a * (Rs/R0)^b
    // These are approximate values and should be calibrated for accuracy
    
    float a, b;
    
    switch (mq_type_) {
        case MQType::MQ2:
            // For LPG
            a = 987.99f;
            b = -2.162f;
            break;
        case MQType::MQ7:
            // For CO
            a = 99.042f;
            b = -1.518f;
            break;
        case MQType::MQ135:
            // For CO2
            a = 110.47f;
            b = -2.862f;
            break;
        default:
            return NAN;
    }
    
    return a * pow(rs_ratio, b);
}

bool MQSensor::validateReading(float value) {
    if (isnan(value)) return false;
    if (value < 0 || value > 100000.0f) return false; // Reasonable PPM range
    return true;
}
