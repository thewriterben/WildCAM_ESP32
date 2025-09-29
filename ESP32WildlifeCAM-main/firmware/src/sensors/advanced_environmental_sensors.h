/**
 * @file advanced_environmental_sensors.h
 * @brief Advanced Environmental Sensors System for ESP32WildlifeCAM
 * 
 * Comprehensive environmental monitoring with multi-sensor integration,
 * wildlife activity prediction, and photography condition assessment.
 */

#ifndef ADVANCED_ENVIRONMENTAL_SENSORS_H
#define ADVANCED_ENVIRONMENTAL_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SGP30.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

// ===========================
// SENSOR CONFIGURATION
// ===========================

// Sensor enable flags
#define BME280_ENABLED          true
#define TSL2591_ENABLED         true
#define SGP30_ENABLED           true
#define DS18B20_ENABLED         true
#define MAX17048_ENABLED        true

// I2C addresses
#define BME280_I2C_ADDR         0x76
#define TSL2591_I2C_ADDR        0x29
#define SGP30_I2C_ADDR          0x58
#define MAX17048_I2C_ADDR       0x36

// OneWire pin for DS18B20
#define ONEWIRE_PIN             33

// Polling intervals (milliseconds)
#define FAST_SENSOR_INTERVAL    5000    // 5 seconds
#define MEDIUM_SENSOR_INTERVAL  30000   // 30 seconds
#define SLOW_SENSOR_INTERVAL    300000  // 5 minutes

// ===========================
// DATA STRUCTURES
// ===========================

/**
 * Advanced Environmental Data Structure
 * 
 * Comprehensive environmental data with derived calculations
 * and wildlife/photography assessment indices.
 */
struct AdvancedEnvironmentalData {
    uint32_t timestamp;
    
    // Basic environmental (BME280)
    float temperature;              // Temperature in Celsius
    float humidity;                 // Relative humidity %
    float pressure;                 // Atmospheric pressure hPa
    bool bme280_valid;              // BME280 sensor validity
    
    // Advanced temperature monitoring (DS18B20)
    float ground_temperature;       // Ground temperature
    float enclosure_temperature;    // Enclosure temperature
    float battery_temperature;      // Battery temperature
    
    // Light monitoring (TSL2591)
    float visible_light;            // Visible light in lux
    float infrared_light;           // IR component
    float full_spectrum_light;      // Full spectrum
    
    // Air quality (SGP30)
    uint16_t tvoc_ppb;             // Total VOC in ppb
    uint16_t eco2_ppm;             // Equivalent CO2 in ppm
    
    // Power monitoring (MAX17048)
    float battery_voltage;          // Battery voltage
    float battery_percentage;       // Battery percentage
    float solar_voltage;            // Solar panel voltage
    
    // Derived values
    float dew_point;               // Dew point temperature
    float heat_index;              // Heat index
    float vapor_pressure;          // Vapor pressure
    
    // Wildlife/photography indices (0-100%)
    uint8_t wildlife_activity_index;    // Wildlife activity prediction
    uint8_t photography_conditions;     // Photography conditions assessment
    uint8_t comfort_index;             // Overall environmental comfort
    
    // Diagnostics
    uint32_t sensor_errors;        // Sensor error flags
    float sensor_drift[10];        // Sensor drift tracking
};

/**
 * Sensor Configuration Structure
 */
struct SensorConfig {
    bool enable_bme280;
    bool enable_tsl2591;
    bool enable_sgp30;
    bool enable_ds18b20;
    bool enable_max17048;
    
    uint32_t fast_interval;
    uint32_t medium_interval;
    uint32_t slow_interval;
    
    // Alert thresholds
    float temp_alert_high;
    float temp_alert_low;
    float humidity_alert_high;
    float pressure_alert_low;
    float battery_alert_low;
    
    // Power optimization
    bool low_power_mode;
    bool adaptive_polling;
};

// ===========================
// SENSOR ERROR FLAGS
// ===========================

#define SENSOR_ERROR_BME280     0x01
#define SENSOR_ERROR_TSL2591    0x02
#define SENSOR_ERROR_SGP30      0x04
#define SENSOR_ERROR_DS18B20    0x08
#define SENSOR_ERROR_MAX17048   0x10
#define SENSOR_ERROR_I2C        0x20

// ===========================
// ADVANCED ENVIRONMENTAL SENSORS CLASS
// ===========================

class AdvancedEnvironmentalSensors {
public:
    AdvancedEnvironmentalSensors();
    ~AdvancedEnvironmentalSensors();
    
    // Initialization and configuration
    bool init();
    bool configure(const SensorConfig& config);
    void cleanup();
    
    // Main data collection
    bool readAllSensors();
    AdvancedEnvironmentalData getLatestData() const;
    bool isDataValid() const;
    
    // Individual sensor access
    bool readBME280();
    bool readTSL2591();
    bool readSGP30();
    bool readDS18B20();
    bool readMAX17048();
    
    // Derived calculations
    float calculateDewPoint(float temp, float humidity);
    float calculateHeatIndex(float temp, float humidity);
    float calculateVaporPressure(float temp, float humidity);
    float calculateAltitude(float pressure, float sea_level_pressure = 1013.25);
    
    // Wildlife activity prediction
    uint8_t calculateWildlifeActivityIndex(const AdvancedEnvironmentalData& data);
    uint8_t calculatePhotographyConditions(const AdvancedEnvironmentalData& data);
    uint8_t calculateComfortIndex(const AdvancedEnvironmentalData& data);
    
    // Environmental alerts
    bool checkTemperatureAlerts(float temperature);
    bool checkHumidityAlerts(float humidity);
    bool checkPressureAlerts(float pressure);
    bool checkBatteryAlerts(float battery_percentage);
    bool checkAirQualityAlerts(uint16_t tvoc, uint16_t eco2);
    
    // Sensor health and diagnostics
    bool performSensorDiagnostics();
    uint32_t getSensorErrors() const;
    bool isSensorHealthy(uint8_t sensor_id) const;
    void resetSensorErrors();
    
    // Power management
    bool enterLowPowerMode();
    bool exitLowPowerMode();
    bool isLowPowerMode() const;
    
    // Configuration management
    SensorConfig getConfig() const;
    bool setConfig(const SensorConfig& config);
    bool saveConfig();
    bool loadConfig();
    
    // Calibration and drift compensation
    bool calibrateSensors();
    bool updateDriftCompensation();
    float getSensorDrift(uint8_t sensor_id) const;
    
    // Data history and trends
    bool addDataToHistory(const AdvancedEnvironmentalData& data);
    std::vector<AdvancedEnvironmentalData> getDataHistory(uint32_t duration_minutes = 60) const;
    float calculateTrend(const char* parameter, uint32_t duration_minutes = 30) const;
    
    // Utility methods
    String getStatusString() const;
    void printDiagnostics() const;
    bool exportData(const String& filename, uint32_t duration_hours = 24) const;

private:
    // Sensor instances
    Adafruit_BME280 bme280_;
    Adafruit_TSL2591 tsl2591_;
    Adafruit_SGP30 sgp30_;
    OneWire oneWire_;
    DallasTemperature ds18b20_;
    SFE_MAX1704X max17048_;
    
    // Current data and configuration
    AdvancedEnvironmentalData currentData_;
    SensorConfig config_;
    
    // State tracking
    bool initialized_;
    bool low_power_mode_;
    uint32_t last_reading_time_;
    uint32_t sensor_errors_;
    
    // Data history for trends
    std::vector<AdvancedEnvironmentalData> dataHistory_;
    static const size_t MAX_HISTORY_SIZE = 288; // 24 hours at 5-minute intervals
    
    // Internal methods
    bool initializeBME280();
    bool initializeTSL2591();
    bool initializeSGP30();
    bool initializeDS18B20();
    bool initializeMAX17048();
    
    // Data validation
    bool validateBME280Data(float temp, float humidity, float pressure);
    bool validateTSL2591Data(float visible, float ir, float full);
    bool validateSGP30Data(uint16_t tvoc, uint16_t eco2);
    bool validateTemperatureData(float temp);
    bool validateBatteryData(float voltage, float percentage);
    
    // Error handling
    void setSensorError(uint8_t sensor_flag);
    void clearSensorError(uint8_t sensor_flag);
    bool hasSensorError(uint8_t sensor_flag) const;
    
    // Utility functions
    uint32_t getCurrentTimestamp() const;
    void limitHistorySize();
    float constrainFloat(float value, float min_val, float max_val);
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function
AdvancedEnvironmentalSensors* createAdvancedEnvironmentalSensors();

// Configuration helpers
SensorConfig createDefaultSensorConfig();
SensorConfig createLowPowerSensorConfig();
SensorConfig createResearchSensorConfig();

// Wildlife activity prediction algorithm
uint8_t calculateWildlifeActivityIndex(const AdvancedEnvironmentalData& data);

// Photography conditions assessment
bool environmentallyAwareCameraCapture();
uint8_t assessPhotographyConditions(const AdvancedEnvironmentalData& data);

// Environmental filtering for motion detection
bool shouldFilterMotionByEnvironment(const AdvancedEnvironmentalData& data);
float getEnvironmentalMotionThreshold(const AdvancedEnvironmentalData& data);

// Data analysis utilities
float calculateMovingAverage(const std::vector<float>& values, size_t window_size = 5);
bool detectEnvironmentalAnomaly(const AdvancedEnvironmentalData& current, 
                               const std::vector<AdvancedEnvironmentalData>& history);

// Conversion utilities
String formatEnvironmentalData(const AdvancedEnvironmentalData& data);
String formatSensorDiagnostics(uint32_t error_flags);

#endif // ADVANCED_ENVIRONMENTAL_SENSORS_H