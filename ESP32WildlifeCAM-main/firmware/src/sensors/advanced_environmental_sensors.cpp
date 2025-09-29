/**
 * @file advanced_environmental_sensors.cpp
 * @brief Advanced Environmental Sensors System Implementation
 */

#include "advanced_environmental_sensors.h"
#include "../debug_utils.h"
#include "../config.h"
#include <math.h>
#include <algorithm>

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

AdvancedEnvironmentalSensors::AdvancedEnvironmentalSensors()
    : oneWire_(ONEWIRE_PIN)
    , ds18b20_(&oneWire_)
    , initialized_(false)
    , low_power_mode_(false)
    , last_reading_time_(0)
    , sensor_errors_(0) {
    
    // Initialize current data structure
    memset(&currentData_, 0, sizeof(AdvancedEnvironmentalData));
    
    // Set default configuration
    config_ = createDefaultSensorConfig();
}

AdvancedEnvironmentalSensors::~AdvancedEnvironmentalSensors() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool AdvancedEnvironmentalSensors::init() {
    DEBUG_PRINTLN("Initializing advanced environmental sensors...");
    
    if (initialized_) {
        DEBUG_PRINTLN("Already initialized");
        return true;
    }
    
    sensor_errors_ = 0;
    bool any_sensor_initialized = false;
    
    // Initialize I2C if not already done
    Wire.begin();
    
    // Initialize individual sensors
    if (config_.enable_bme280 && initializeBME280()) {
        DEBUG_PRINTLN("BME280 initialized successfully");
        any_sensor_initialized = true;
    } else {
        setSensorError(SENSOR_ERROR_BME280);
        DEBUG_PRINTLN("BME280 initialization failed");
    }
    
    if (config_.enable_tsl2591 && initializeTSL2591()) {
        DEBUG_PRINTLN("TSL2591 initialized successfully");
        any_sensor_initialized = true;
    } else {
        setSensorError(SENSOR_ERROR_TSL2591);
        DEBUG_PRINTLN("TSL2591 initialization failed");
    }
    
    if (config_.enable_sgp30 && initializeSGP30()) {
        DEBUG_PRINTLN("SGP30 initialized successfully");
        any_sensor_initialized = true;
    } else {
        setSensorError(SENSOR_ERROR_SGP30);
        DEBUG_PRINTLN("SGP30 initialization failed");
    }
    
    if (config_.enable_ds18b20 && initializeDS18B20()) {
        DEBUG_PRINTLN("DS18B20 initialized successfully");
        any_sensor_initialized = true;
    } else {
        setSensorError(SENSOR_ERROR_DS18B20);
        DEBUG_PRINTLN("DS18B20 initialization failed");
    }
    
    if (config_.enable_max17048 && initializeMAX17048()) {
        DEBUG_PRINTLN("MAX17048 initialized successfully");
        any_sensor_initialized = true;
    } else {
        setSensorError(SENSOR_ERROR_MAX17048);
        DEBUG_PRINTLN("MAX17048 initialization failed");
    }
    
    initialized_ = any_sensor_initialized;
    
    if (initialized_) {
        DEBUG_PRINTF("Advanced environmental sensors initialized with %d errors\n", 
                    __builtin_popcount(sensor_errors_));
        
        // Perform initial reading
        readAllSensors();
    } else {
        DEBUG_PRINTLN("Failed to initialize any environmental sensors");
    }
    
    return initialized_;
}

bool AdvancedEnvironmentalSensors::initializeBME280() {
    if (!bme280_.begin(BME280_I2C_ADDR)) {
        return false;
    }
    
    // Configure BME280 for weather monitoring
    bme280_.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating mode
                       Adafruit_BME280::SAMPLING_X2,      // Temp oversampling
                       Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling
                       Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling
                       Adafruit_BME280::FILTER_X16,       // Filtering
                       Adafruit_BME280::STANDBY_MS_500);  // Standby time
    
    return true;
}

bool AdvancedEnvironmentalSensors::initializeTSL2591() {
    if (!tsl2591_.begin()) {
        return false;
    }
    
    // Configure TSL2591 for wildlife monitoring
    tsl2591_.setGain(TSL2591_GAIN_MED);      // Medium gain for versatility
    tsl2591_.setTiming(TSL2591_INTEGRATIONTIME_300MS);  // Good balance
    
    return true;
}

bool AdvancedEnvironmentalSensors::initializeSGP30() {
    if (!sgp30_.begin()) {
        return false;
    }
    
    // Initialize baseline values (would be loaded from storage in production)
    sgp30_.setIAQBaseline(0x8E68, 0x8F41);
    
    return true;
}

bool AdvancedEnvironmentalSensors::initializeDS18B20() {
    ds18b20_.begin();
    
    // Check if any DS18B20 sensors are connected
    int deviceCount = ds18b20_.getDeviceCount();
    if (deviceCount == 0) {
        return false;
    }
    
    DEBUG_PRINTF("Found %d DS18B20 temperature sensors\n", deviceCount);
    
    // Set resolution for all devices
    ds18b20_.setResolution(12);  // 12-bit resolution
    
    return true;
}

bool AdvancedEnvironmentalSensors::initializeMAX17048() {
    if (!max17048_.begin()) {
        return false;
    }
    
    // Reset to ensure clean state
    max17048_.reset();
    delay(500);
    
    return true;
}

// ===========================
// DATA COLLECTION
// ===========================

bool AdvancedEnvironmentalSensors::readAllSensors() {
    if (!initialized_) {
        return false;
    }
    
    uint32_t current_time = millis();
    
    // Check if enough time has passed since last reading
    if (current_time - last_reading_time_ < config_.fast_interval && last_reading_time_ != 0) {
        return true; // Use cached data
    }
    
    currentData_.timestamp = getCurrentTimestamp();
    bool any_success = false;
    
    // Read basic environmental data (BME280)
    if (config_.enable_bme280 && !hasSensorError(SENSOR_ERROR_BME280)) {
        if (readBME280()) {
            any_success = true;
            clearSensorError(SENSOR_ERROR_BME280);
        } else {
            setSensorError(SENSOR_ERROR_BME280);
        }
    }
    
    // Read light data (TSL2591)
    if (config_.enable_tsl2591 && !hasSensorError(SENSOR_ERROR_TSL2591)) {
        if (readTSL2591()) {
            any_success = true;
            clearSensorError(SENSOR_ERROR_TSL2591);
        } else {
            setSensorError(SENSOR_ERROR_TSL2591);
        }
    }
    
    // Read air quality (SGP30)
    if (config_.enable_sgp30 && !hasSensorError(SENSOR_ERROR_SGP30)) {
        if (readSGP30()) {
            any_success = true;
            clearSensorError(SENSOR_ERROR_SGP30);
        } else {
            setSensorError(SENSOR_ERROR_SGP30);
        }
    }
    
    // Read temperature sensors (DS18B20)
    if (config_.enable_ds18b20 && !hasSensorError(SENSOR_ERROR_DS18B20)) {
        if (readDS18B20()) {
            any_success = true;
            clearSensorError(SENSOR_ERROR_DS18B20);
        } else {
            setSensorError(SENSOR_ERROR_DS18B20);
        }
    }
    
    // Read battery data (MAX17048)
    if (config_.enable_max17048 && !hasSensorError(SENSOR_ERROR_MAX17048)) {
        if (readMAX17048()) {
            any_success = true;
            clearSensorError(SENSOR_ERROR_MAX17048);
        } else {
            setSensorError(SENSOR_ERROR_MAX17048);
        }
    }
    
    if (any_success) {
        // Calculate derived values
        currentData_.dew_point = calculateDewPoint(currentData_.temperature, currentData_.humidity);
        currentData_.heat_index = calculateHeatIndex(currentData_.temperature, currentData_.humidity);
        currentData_.vapor_pressure = calculateVaporPressure(currentData_.temperature, currentData_.humidity);
        
        // Calculate indices
        currentData_.wildlife_activity_index = calculateWildlifeActivityIndex(currentData_);
        currentData_.photography_conditions = calculatePhotographyConditions(currentData_);
        currentData_.comfort_index = calculateComfortIndex(currentData_);
        
        // Update error status
        currentData_.sensor_errors = sensor_errors_;
        
        // Add to history
        addDataToHistory(currentData_);
        
        last_reading_time_ = current_time;
    }
    
    return any_success;
}

bool AdvancedEnvironmentalSensors::readBME280() {
    float temp = bme280_.readTemperature();
    float humidity = bme280_.readHumidity();
    float pressure = bme280_.readPressure() / 100.0F; // Convert Pa to hPa
    
    if (!validateBME280Data(temp, humidity, pressure)) {
        return false;
    }
    
    currentData_.temperature = temp;
    currentData_.humidity = humidity;
    currentData_.pressure = pressure;
    currentData_.bme280_valid = true;
    
    return true;
}

bool AdvancedEnvironmentalSensors::readTSL2591() {
    uint32_t lum = tsl2591_.getFullLuminosity();
    uint16_t ir = lum >> 16;
    uint16_t full = lum & 0xFFFF;
    uint16_t visible = full - ir;
    
    float lux = tsl2591_.calculateLux(full, ir);
    
    if (!validateTSL2591Data(visible, ir, lux)) {
        return false;
    }
    
    currentData_.visible_light = lux;
    currentData_.infrared_light = ir;
    currentData_.full_spectrum_light = full;
    
    return true;
}

bool AdvancedEnvironmentalSensors::readSGP30() {
    if (!sgp30_.IAQmeasure()) {
        return false;
    }
    
    uint16_t tvoc = sgp30_.TVOC;
    uint16_t eco2 = sgp30_.eCO2;
    
    if (!validateSGP30Data(tvoc, eco2)) {
        return false;
    }
    
    currentData_.tvoc_ppb = tvoc;
    currentData_.eco2_ppm = eco2;
    
    return true;
}

bool AdvancedEnvironmentalSensors::readDS18B20() {
    ds18b20_.requestTemperatures();
    
    // Read first sensor as ground temperature
    float ground_temp = ds18b20_.getTempCByIndex(0);
    if (ground_temp == DEVICE_DISCONNECTED_C) {
        return false;
    }
    
    currentData_.ground_temperature = ground_temp;
    
    // Read second sensor as enclosure temperature (if available)
    if (ds18b20_.getDeviceCount() > 1) {
        float enclosure_temp = ds18b20_.getTempCByIndex(1);
        if (enclosure_temp != DEVICE_DISCONNECTED_C) {
            currentData_.enclosure_temperature = enclosure_temp;
        }
    } else {
        currentData_.enclosure_temperature = ground_temp; // Fallback
    }
    
    // Estimate battery temperature (would need dedicated sensor in production)
    currentData_.battery_temperature = currentData_.enclosure_temperature;
    
    return validateTemperatureData(ground_temp);
}

bool AdvancedEnvironmentalSensors::readMAX17048() {
    float voltage = max17048_.getVoltage();
    float percentage = max17048_.getSOC();
    
    if (!validateBatteryData(voltage, percentage)) {
        return false;
    }
    
    currentData_.battery_voltage = voltage;
    currentData_.battery_percentage = percentage;
    
    // Estimate solar voltage (would need dedicated ADC in production)
    // For now, use a simple estimation based on light levels
    if (currentData_.visible_light > 100.0) {
        currentData_.solar_voltage = 3.3 + (currentData_.visible_light / 1000.0);
    } else {
        currentData_.solar_voltage = 0.0;
    }
    
    return true;
}

// ===========================
// DERIVED CALCULATIONS
// ===========================

float AdvancedEnvironmentalSensors::calculateDewPoint(float temp, float humidity) {
    if (humidity <= 0.0 || humidity > 100.0) {
        return NAN;
    }
    
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * temp) / (b + temp)) + log(humidity / 100.0);
    float dew_point = (b * alpha) / (a - alpha);
    
    return dew_point;
}

float AdvancedEnvironmentalSensors::calculateHeatIndex(float temp, float humidity) {
    if (temp < 26.7) { // Below 80°F, heat index is just the temperature
        return temp;
    }
    
    // Convert to Fahrenheit for calculation
    float T = temp * 9.0 / 5.0 + 32.0;
    float RH = humidity;
    
    // Steadman's formula
    float HI = -42.379 + 2.04901523 * T + 10.14333127 * RH 
               - 0.22475541 * T * RH - 6.83783e-3 * T * T
               - 5.481717e-2 * RH * RH + 1.22874e-3 * T * T * RH
               + 8.5282e-4 * T * RH * RH - 1.99e-6 * T * T * RH * RH;
    
    // Convert back to Celsius
    return (HI - 32.0) * 5.0 / 9.0;
}

float AdvancedEnvironmentalSensors::calculateVaporPressure(float temp, float humidity) {
    // Saturation vapor pressure using Antoine equation approximation
    float sat_vapor_pressure = 0.61078 * exp((17.27 * temp) / (temp + 237.3));
    
    // Actual vapor pressure
    return sat_vapor_pressure * (humidity / 100.0);
}

// ===========================
// WILDLIFE ACTIVITY PREDICTION
// ===========================

uint8_t AdvancedEnvironmentalSensors::calculateWildlifeActivityIndex(const AdvancedEnvironmentalData& data) {
    uint8_t activity = 50; // Base activity
    
    // Temperature factor (optimal 10-25°C)
    if (data.temperature >= 10.0 && data.temperature <= 25.0) {
        activity += 20;
    } else if (data.temperature < 0.0 || data.temperature > 35.0) {
        activity = (activity > 20) ? activity - 20 : 0;
    }
    
    // Light factor (dawn/dusk are optimal)
    if (data.visible_light > 10.0 && data.visible_light < 100.0) {
        activity += 15; // Dawn/dusk conditions
    } else if (data.visible_light > 500.0) {
        activity = (activity > 10) ? activity - 10 : 0; // Bright daylight
    }
    
    // Weather stability factor
    if (data.pressure > 1010.0 && data.pressure < 1020.0) {
        activity += 10; // Stable weather
    }
    
    // Humidity factor
    if (data.humidity > 30.0 && data.humidity < 80.0) {
        activity += 5; // Comfortable humidity
    }
    
    return constrain(activity, 0, 100);
}

uint8_t AdvancedEnvironmentalSensors::calculatePhotographyConditions(const AdvancedEnvironmentalData& data) {
    uint8_t conditions = 50; // Base conditions
    
    // Light optimization (100-1000 lux is optimal)
    if (data.visible_light >= 100.0 && data.visible_light <= 1000.0) {
        conditions += 30;
    } else if (data.visible_light < 10.0) {
        conditions = (conditions > 20) ? conditions - 20 : 0; // Too dark
    } else if (data.visible_light > 5000.0) {
        conditions = (conditions > 15) ? conditions - 15 : 0; // Too bright
    }
    
    // Humidity management (condensation risk >95%)
    if (data.humidity > 95.0) {
        conditions = (conditions > 25) ? conditions - 25 : 0; // High condensation risk
    } else if (data.humidity < 20.0) {
        conditions = (conditions > 10) ? conditions - 10 : 0; // Too dry (dust)
    }
    
    // Temperature stability
    if (data.temperature >= 5.0 && data.temperature <= 30.0) {
        conditions += 10; // Good operating range
    }
    
    // Weather stability
    if (data.pressure > 1005.0 && data.pressure < 1025.0) {
        conditions += 10; // Stable weather
    }
    
    return constrain(conditions, 0, 100);
}

uint8_t AdvancedEnvironmentalSensors::calculateComfortIndex(const AdvancedEnvironmentalData& data) {
    uint8_t comfort = 50; // Base comfort
    
    // Temperature comfort (15-25°C optimal)
    float temp_diff = abs(data.temperature - 20.0); // 20°C is ideal
    if (temp_diff <= 5.0) {
        comfort += 20;
    } else if (temp_diff > 15.0) {
        comfort = (comfort > 20) ? comfort - 20 : 0;
    }
    
    // Humidity comfort (40-60% optimal)
    float humidity_diff = abs(data.humidity - 50.0); // 50% is ideal
    if (humidity_diff <= 10.0) {
        comfort += 15;
    } else if (humidity_diff > 30.0) {
        comfort = (comfort > 15) ? comfort - 15 : 0;
    }
    
    // Pressure comfort (1010-1020 hPa optimal)
    if (data.pressure >= 1010.0 && data.pressure <= 1020.0) {
        comfort += 15;
    }
    
    return constrain(comfort, 0, 100);
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

AdvancedEnvironmentalData AdvancedEnvironmentalSensors::getLatestData() const {
    return currentData_;
}

bool AdvancedEnvironmentalSensors::isDataValid() const {
    return initialized_ && (sensor_errors_ != 0xFF);
}

void AdvancedEnvironmentalSensors::cleanup() {
    initialized_ = false;
    dataHistory_.clear();
}

bool AdvancedEnvironmentalSensors::configure(const SensorConfig& config) {
    config_ = config;
    return true;
}

uint32_t AdvancedEnvironmentalSensors::getCurrentTimestamp() const {
    return millis() / 1000; // Convert to seconds
}

// ===========================
// VALIDATION FUNCTIONS
// ===========================

bool AdvancedEnvironmentalSensors::validateBME280Data(float temp, float humidity, float pressure) {
    return (temp >= -40.0 && temp <= 85.0 &&        // BME280 operating range
            humidity >= 0.0 && humidity <= 100.0 &&  // Valid humidity range
            pressure >= 300.0 && pressure <= 1100.0); // Valid pressure range
}

bool AdvancedEnvironmentalSensors::validateTSL2591Data(float visible, float ir, float full) {
    return (visible >= 0.0 && ir >= 0.0 && full >= 0.0 && full <= 65535.0);
}

bool AdvancedEnvironmentalSensors::validateSGP30Data(uint16_t tvoc, uint16_t eco2) {
    return (tvoc <= 60000 && eco2 >= 400 && eco2 <= 60000); // SGP30 valid ranges
}

bool AdvancedEnvironmentalSensors::validateTemperatureData(float temp) {
    return (temp >= -55.0 && temp <= 125.0); // DS18B20 operating range
}

bool AdvancedEnvironmentalSensors::validateBatteryData(float voltage, float percentage) {
    return (voltage >= 2.5 && voltage <= 5.0 &&      // Reasonable battery voltage
            percentage >= 0.0 && percentage <= 100.0); // Valid percentage
}

// ===========================
// ERROR HANDLING
// ===========================

void AdvancedEnvironmentalSensors::setSensorError(uint8_t sensor_flag) {
    sensor_errors_ |= sensor_flag;
}

void AdvancedEnvironmentalSensors::clearSensorError(uint8_t sensor_flag) {
    sensor_errors_ &= ~sensor_flag;
}

bool AdvancedEnvironmentalSensors::hasSensorError(uint8_t sensor_flag) const {
    return (sensor_errors_ & sensor_flag) != 0;
}

uint32_t AdvancedEnvironmentalSensors::getSensorErrors() const {
    return sensor_errors_;
}

void AdvancedEnvironmentalSensors::resetSensorErrors() {
    sensor_errors_ = 0;
}

// ===========================
// DATA HISTORY MANAGEMENT
// ===========================

bool AdvancedEnvironmentalSensors::addDataToHistory(const AdvancedEnvironmentalData& data) {
    dataHistory_.push_back(data);
    limitHistorySize();
    return true;
}

void AdvancedEnvironmentalSensors::limitHistorySize() {
    if (dataHistory_.size() > MAX_HISTORY_SIZE) {
        dataHistory_.erase(dataHistory_.begin(), 
                          dataHistory_.begin() + (dataHistory_.size() - MAX_HISTORY_SIZE));
    }
}

float AdvancedEnvironmentalSensors::constrainFloat(float value, float min_val, float max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

// ===========================
// CONFIGURATION HELPERS
// ===========================

SensorConfig createDefaultSensorConfig() {
    SensorConfig config;
    config.enable_bme280 = BME280_ENABLED;
    config.enable_tsl2591 = TSL2591_ENABLED;
    config.enable_sgp30 = SGP30_ENABLED;
    config.enable_ds18b20 = DS18B20_ENABLED;
    config.enable_max17048 = MAX17048_ENABLED;
    
    config.fast_interval = FAST_SENSOR_INTERVAL;
    config.medium_interval = MEDIUM_SENSOR_INTERVAL;
    config.slow_interval = SLOW_SENSOR_INTERVAL;
    
    config.temp_alert_high = 35.0;
    config.temp_alert_low = -10.0;
    config.humidity_alert_high = 95.0;
    config.pressure_alert_low = 980.0;
    config.battery_alert_low = 20.0;
    
    config.low_power_mode = false;
    config.adaptive_polling = true;
    
    return config;
}

// ===========================
// WILDLIFE ACTIVITY ALGORITHM (STANDALONE)
// ===========================

uint8_t calculateWildlifeActivityIndex(const AdvancedEnvironmentalData& data) {
    uint8_t activity = 50; // Base activity
    
    // Temperature factor (optimal 10-25°C)
    if (data.temperature >= 10.0 && data.temperature <= 25.0) {
        activity += 20;
    } else if (data.temperature < 0.0 || data.temperature > 35.0) {
        activity = (activity > 20) ? activity - 20 : 0;
    }
    
    // Light factor (dawn/dusk are optimal)
    if (data.visible_light > 10.0 && data.visible_light < 100.0) {
        activity += 15; // Dawn/dusk conditions
    } else if (data.visible_light > 500.0) {
        activity = (activity > 10) ? activity - 10 : 0; // Bright daylight
    }
    
    // Weather stability factor
    if (data.pressure > 1010.0 && data.pressure < 1020.0) {
        activity += 10; // Stable weather
    }
    
    // Humidity factor
    if (data.humidity > 30.0 && data.humidity < 80.0) {
        activity += 5; // Comfortable humidity
    }
    
    return constrain(activity, 0, 100);
}