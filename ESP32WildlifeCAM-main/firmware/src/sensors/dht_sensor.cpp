/**
 * @file dht_sensor.cpp
 * @brief DHT11/DHT22 sensor implementation
 */

#include "dht_sensor.h"

DHTSensor::DHTSensor(uint8_t pin, DHTType type) 
    : pin_(pin), dht_type_(type), consecutive_failures_(0) {
    
    // Set minimum sampling interval based on sensor type
    if (dht_type_ == DHTType::DHT11) {
        min_sample_interval_ms_ = 1000; // DHT11: 1 second minimum
    } else {
        min_sample_interval_ms_ = 2000; // DHT22: 2 seconds minimum
    }
    
    // Create DHT object
    dht_ = new DHT(pin_, static_cast<uint8_t>(dht_type_));
}

DHTSensor::~DHTSensor() {
    cleanup();
}

// ===========================
// IDENTIFICATION
// ===========================

UnifiedSensorType DHTSensor::getSensorType() const {
    return dht_type_ == DHTType::DHT11 ? 
        UnifiedSensorType::SENSOR_DHT11 : 
        UnifiedSensorType::SENSOR_DHT22;
}

const char* DHTSensor::getSensorName() const {
    return dht_type_ == DHTType::DHT11 ? "DHT11" : "DHT22";
}

SensorCapabilities DHTSensor::getCapabilities() const {
    SensorCapabilities caps;
    caps.type = getSensorType();
    caps.name = getSensorName();
    caps.manufacturer = "Aosong";
    caps.interface = SensorInterface::INTERFACE_DIGITAL;
    caps.i2c_address = 0;
    caps.poll_interval_ms = min_sample_interval_ms_;
    caps.supports_interrupts = false;
    caps.requires_calibration = false;
    caps.data_size = 8; // 2 floats (temp, humidity)
    
    if (dht_type_ == DHTType::DHT11) {
        caps.min_value = 0.0f;
        caps.max_value = 50.0f;
        caps.unit = "°C/% RH";
    } else {
        caps.min_value = -40.0f;
        caps.max_value = 80.0f;
        caps.unit = "°C/% RH";
    }
    
    return caps;
}

// ===========================
// INITIALIZATION & CONTROL
// ===========================

bool DHTSensor::init() {
    if (status_ == SensorStatus::STATUS_READY) {
        return true;
    }
    
    status_ = SensorStatus::STATUS_INITIALIZING;
    
    if (!dht_) {
        snprintf(last_error_, sizeof(last_error_), "DHT object not created");
        status_ = SensorStatus::STATUS_ERROR;
        return false;
    }
    
    // Initialize DHT sensor
    dht_->begin();
    
    // Wait for sensor to stabilize
    delay(min_sample_interval_ms_);
    
    // Test initial reading
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

bool DHTSensor::configure(const SensorConfig& config) {
    config_ = config;
    
    // Ensure poll interval is not too fast
    if (config_.poll_interval_ms < min_sample_interval_ms_) {
        config_.poll_interval_ms = min_sample_interval_ms_;
    }
    
    return true;
}

bool DHTSensor::isInitialized() const {
    return status_ == SensorStatus::STATUS_READY || 
           status_ == SensorStatus::STATUS_READING;
}

SensorStatus DHTSensor::getStatus() const {
    return status_;
}

void DHTSensor::reset() {
    if (dht_) {
        dht_->begin();
        delay(min_sample_interval_ms_);
    }
    consecutive_failures_ = 0;
    status_ = SensorStatus::STATUS_READY;
}

void DHTSensor::cleanup() {
    if (dht_) {
        delete dht_;
        dht_ = nullptr;
    }
    status_ = SensorStatus::STATUS_UNINITIALIZED;
}

// ===========================
// DATA READING
// ===========================

SensorReading DHTSensor::read() {
    SensorReading reading;
    reading.timestamp = millis();
    reading.sensor_type = getSensorType();
    reading.status = status_;
    reading.valid = false;
    reading.value_count = 2;
    reading.unit = "°C/% RH";
    
    // Check if sensor is initialized
    if (!isInitialized()) {
        snprintf(last_error_, sizeof(last_error_), "Sensor not initialized");
        reading.status = SensorStatus::STATUS_ERROR;
        return reading;
    }
    
    // Check minimum sample interval
    uint32_t time_since_last = millis() - last_read_time_;
    if (time_since_last < min_sample_interval_ms_) {
        // Return last reading if too soon
        return last_reading_;
    }
    
    status_ = SensorStatus::STATUS_READING;
    
    // Read temperature and humidity
    float humidity = dht_->readHumidity();
    float temperature = dht_->readTemperature();
    
    // Check if readings are valid
    if (isnan(temperature) || isnan(humidity)) {
        consecutive_failures_++;
        snprintf(last_error_, sizeof(last_error_), 
                "Failed to read from sensor (failures: %u)", consecutive_failures_);
        
        if (consecutive_failures_ > 5) {
            status_ = SensorStatus::STATUS_DISCONNECTED;
        } else {
            status_ = SensorStatus::STATUS_ERROR;
        }
        
        reading.status = status_;
        return reading;
    }
    
    // Validate readings
    if (!validateReading(temperature, humidity)) {
        consecutive_failures_++;
        snprintf(last_error_, sizeof(last_error_), "Invalid sensor reading");
        status_ = SensorStatus::STATUS_ERROR;
        reading.status = status_;
        return reading;
    }
    
    // Successful reading
    reading.values[0] = temperature;
    reading.values[1] = humidity;
    reading.valid = true;
    reading.status = SensorStatus::STATUS_READY;
    
    status_ = SensorStatus::STATUS_READY;
    consecutive_failures_ = 0;
    last_reading_ = reading;
    last_read_time_ = reading.timestamp;
    last_error_[0] = '\0';
    
    return reading;
}

bool DHTSensor::dataAvailable() {
    uint32_t time_since_last = millis() - last_read_time_;
    return time_since_last >= min_sample_interval_ms_;
}

SensorReading DHTSensor::getLastReading() const {
    return last_reading_;
}

uint32_t DHTSensor::getReadingAge() const {
    return millis() - last_read_time_;
}

// ===========================
// SPECIFIC READINGS
// ===========================

float DHTSensor::readTemperature() {
    if (!dht_) return NAN;
    return dht_->readTemperature();
}

float DHTSensor::readTemperatureF() {
    if (!dht_) return NAN;
    return dht_->readTemperature(true);
}

float DHTSensor::readHumidity() {
    if (!dht_) return NAN;
    return dht_->readHumidity();
}

float DHTSensor::computeHeatIndex() {
    if (!dht_) return NAN;
    float temp = readTemperature();
    float hum = readHumidity();
    if (isnan(temp) || isnan(hum)) return NAN;
    return dht_->computeHeatIndex(temp, hum, false);
}

float DHTSensor::computeHeatIndexF() {
    if (!dht_) return NAN;
    float temp = readTemperatureF();
    float hum = readHumidity();
    if (isnan(temp) || isnan(hum)) return NAN;
    return dht_->computeHeatIndex(temp, hum, true);
}

// ===========================
// DIAGNOSTICS
// ===========================

bool DHTSensor::testCommunication() {
    if (!dht_) return false;
    
    float temp = dht_->readTemperature();
    float hum = dht_->readHumidity();
    
    return !isnan(temp) && !isnan(hum);
}

const char* DHTSensor::getLastError() const {
    return last_error_;
}

uint8_t DHTSensor::getHealthScore() const {
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

float DHTSensor::getPowerConsumption() const {
    // DHT11: ~0.5mA @ 3.3V = 1.65mW
    // DHT22: ~1.5mA @ 3.3V = 4.95mW
    if (dht_type_ == DHTType::DHT11) {
        return 1.65f;
    } else {
        return 4.95f;
    }
}

// ===========================
// PRIVATE METHODS
// ===========================

bool DHTSensor::validateReading(float temp, float humidity) {
    // Check for NaN
    if (isnan(temp) || isnan(humidity)) {
        return false;
    }
    
    // Validate ranges based on sensor type
    if (dht_type_ == DHTType::DHT11) {
        // DHT11: 0-50°C, 20-80% RH
        if (temp < -5.0f || temp > 60.0f) return false;
        if (humidity < 10.0f || humidity > 90.0f) return false;
    } else {
        // DHT22: -40-80°C, 0-100% RH
        if (temp < -50.0f || temp > 90.0f) return false;
        if (humidity < 0.0f || humidity > 100.0f) return false;
    }
    
    return true;
}
