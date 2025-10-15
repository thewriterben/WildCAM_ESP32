/**
 * @file environmental_suite.cpp
 * @brief Comprehensive Environmental Sensor Suite for Wildlife Monitoring
 * 
 * Implements advanced multi-sensor environmental monitoring including weather,
 * air quality, acoustic analysis, and habitat assessment for wildlife research.
 */

#include "environmental_suite.h"
#include <Arduino.h>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

EnvironmentalSuite::EnvironmentalSuite() 
    : initialized_(false)
    , sensorsActive_(false)
    , lastReading_(0)
    , lastCalibration_(0)
    , dataLoggingEnabled_(true)
    , powerOptimizationEnabled_(false)
{
    memset(&currentData_, 0, sizeof(currentData_));
    memset(&statistics_, 0, sizeof(statistics_));
    memset(&config_, 0, sizeof(config_));
    sensorReadings_.clear();
}

EnvironmentalSuite::~EnvironmentalSuite() {
    cleanup();
}

bool EnvironmentalSuite::initialize(const EnvironmentalConfig& config) {
    config_ = config;
    
    // Initialize weather station
    if (!initializeWeatherStation()) {
        Serial.println("Failed to initialize weather station");
        return false;
    }
    
    // Initialize air quality sensors
    if (!initializeAirQualitySensors()) {
        Serial.println("Failed to initialize air quality sensors");
        return false;
    }
    
    // Initialize acoustic monitoring
    if (!initializeAcousticMonitoring()) {
        Serial.println("Failed to initialize acoustic monitoring");
        return false;
    }
    
    // Initialize soil/habitat sensors
    if (!initializeHabitatSensors()) {
        Serial.println("Failed to initialize habitat sensors");
        return false;
    }
    
    // Initialize UV and light spectrum monitoring
    if (!initializeLightMonitoring()) {
        Serial.println("Failed to initialize light monitoring");
        return false;
    }
    
    // Initialize data logging
    if (!initializeDataLogging()) {
        Serial.println("Failed to initialize data logging");
        return false;
    }
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.totalReadings = 0;
    statistics_.sensorErrors = 0;
    statistics_.calibrationEvents = 0;
    
    // Set timing
    lastReading_ = millis();
    lastCalibration_ = millis();
    
    initialized_ = true;
    Serial.println("Environmental Suite initialized successfully");
    return true;
}

void EnvironmentalSuite::cleanup() {
    // Cleanup individual sensor systems
    weatherStation_.cleanup();
    airQualityMonitor_.cleanup();
    acousticMonitor_.cleanup();
    habitatSensors_.cleanup();
    lightMonitor_.cleanup();
    
    sensorReadings_.clear();
    initialized_ = false;
    sensorsActive_ = false;
}

// ===========================
// MAIN PROCESSING
// ===========================

void EnvironmentalSuite::processEnvironmentalData() {
    if (!initialized_) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Check if it's time for a new reading
    if (currentTime - lastReading_ < config_.readingInterval) {
        return;
    }
    
    sensorsActive_ = true;
    
    // Read all sensors
    if (!readAllSensors()) {
        Serial.println("Error reading environmental sensors");
        statistics_.sensorErrors++;
        sensorsActive_ = false;
        return;
    }
    
    // Process and validate data
    processRawSensorData();
    
    // Apply calibration corrections
    applyCalibrationsCorrections();
    
    // Calculate derived environmental metrics
    calculateDerivedMetrics();
    
    // Store reading in history
    storeCurrentReading();
    
    // Log data if enabled
    if (dataLoggingEnabled_) {
        logEnvironmentalData();
    }
    
    // Update statistics
    updateStatistics();
    
    // Trigger callback if set
    if (dataCallback_) {
        dataCallback_(currentData_);
    }
    
    lastReading_ = currentTime;
    sensorsActive_ = false;
}

bool EnvironmentalSuite::readAllSensors() {
    bool success = true;
    
    // Read weather station data
    if (!weatherStation_.readSensors()) {
        Serial.println("Weather station read failed");
        success = false;
    }
    
    // Read air quality sensors
    if (!airQualityMonitor_.readSensors()) {
        Serial.println("Air quality read failed");
        success = false;
    }
    
    // Read acoustic data
    if (!acousticMonitor_.readAcousticData()) {
        Serial.println("Acoustic monitoring read failed");
        success = false;
    }
    
    // Read habitat sensors
    if (!habitatSensors_.readSensors()) {
        Serial.println("Habitat sensors read failed");
        success = false;
    }
    
    // Read light monitoring data
    if (!lightMonitor_.readLightData()) {
        Serial.println("Light monitoring read failed");
        success = false;
    }
    
    return success;
}

void EnvironmentalSuite::processRawSensorData() {
    // Process weather data
    currentData_.weather = weatherStation_.getWeatherData();
    
    // Process air quality data
    currentData_.airQuality = airQualityMonitor_.getAirQualityData();
    
    // Process acoustic data
    currentData_.acoustic = acousticMonitor_.getAcousticData();
    
    // Process habitat data
    currentData_.habitat = habitatSensors_.getHabitatData();
    
    // Process light data
    currentData_.light = lightMonitor_.getLightData();
    
    // Set timestamp
    currentData_.timestamp = millis();
    currentData_.sensorStatus = getSensorHealthStatus();
}

// ===========================
// CALIBRATION AND VALIDATION
// ===========================

void EnvironmentalSuite::performCalibration() {
    if (!initialized_) {
        return;
    }
    
    Serial.println("Starting environmental sensor calibration...");
    
    // Calibrate weather sensors
    if (weatherStation_.calibrate()) {
        Serial.println("Weather station calibrated");
    } else {
        Serial.println("Weather station calibration failed");
    }
    
    // Calibrate air quality sensors
    if (airQualityMonitor_.calibrate()) {
        Serial.println("Air quality sensors calibrated");
    } else {
        Serial.println("Air quality calibration failed");
    }
    
    // Calibrate acoustic sensors
    if (acousticMonitor_.calibrate()) {
        Serial.println("Acoustic sensors calibrated");
    } else {
        Serial.println("Acoustic calibration failed");
    }
    
    // Calibrate habitat sensors
    if (habitatSensors_.calibrate()) {
        Serial.println("Habitat sensors calibrated");
    } else {
        Serial.println("Habitat calibration failed");
    }
    
    // Calibrate light sensors
    if (lightMonitor_.calibrate()) {
        Serial.println("Light sensors calibrated");
    } else {
        Serial.println("Light calibration failed");
    }
    
    lastCalibration_ = millis();
    statistics_.calibrationEvents++;
    
    Serial.println("Environmental sensor calibration completed");
}

bool EnvironmentalSuite::validateSensorData() {
    bool valid = true;
    
    // Validate temperature readings
    if (currentData_.weather.temperature < -50 || currentData_.weather.temperature > 70) {
        Serial.println("Invalid temperature reading");
        valid = false;
    }
    
    // Validate humidity readings
    if (currentData_.weather.humidity < 0 || currentData_.weather.humidity > 100) {
        Serial.println("Invalid humidity reading");
        valid = false;
    }
    
    // Validate pressure readings
    if (currentData_.weather.pressure < 800 || currentData_.weather.pressure > 1200) {
        Serial.println("Invalid pressure reading");
        valid = false;
    }
    
    // Validate air quality readings
    if (currentData_.airQuality.pm25 < 0 || currentData_.airQuality.pm25 > 1000) {
        Serial.println("Invalid PM2.5 reading");
        valid = false;
    }
    
    // Additional validations for other sensors...
    
    return valid;
}

void EnvironmentalSuite::applyCalibrationsCorrections() {
    // Apply temperature correction
    currentData_.weather.temperature += config_.temperatureOffset;
    
    // Apply humidity correction
    currentData_.weather.humidity *= config_.humidityScale;
    
    // Apply pressure correction
    currentData_.weather.pressure += config_.pressureOffset;
    
    // Apply air quality corrections
    currentData_.airQuality.pm25 *= config_.pm25Scale;
    currentData_.airQuality.co2 += config_.co2Offset;
    
    // Apply light sensor corrections
    currentData_.light.luxLevel *= config_.luxScale;
    currentData_.light.uvIndex *= config_.uvScale;
}

// ===========================
// DERIVED METRICS CALCULATION
// ===========================

void EnvironmentalSuite::calculateDerivedMetrics() {
    // Calculate heat index
    currentData_.derived.heatIndex = calculateHeatIndex(
        currentData_.weather.temperature,
        currentData_.weather.humidity
    );
    
    // Calculate dew point
    currentData_.derived.dewPoint = calculateDewPoint(
        currentData_.weather.temperature,
        currentData_.weather.humidity
    );
    
    // Calculate air quality index
    currentData_.derived.airQualityIndex = calculateAirQualityIndex();
    
    // Calculate comfort index for wildlife
    currentData_.derived.wildlifeComfortIndex = calculateWildlifeComfortIndex();
    
    // Calculate ecosystem health score
    currentData_.derived.ecosystemHealthScore = calculateEcosystemHealthScore();
    
    // Calculate environmental stress factors
    currentData_.derived.environmentalStressScore = calculateEnvironmentalStressScore();
}

float EnvironmentalSuite::calculateHeatIndex(float temperature, float humidity) {
    // Heat index calculation (Rothfusz equation)
    if (temperature < 80.0) {
        return temperature; // Heat index not applicable
    }
    
    float hi = -42.379 + 2.04901523 * temperature + 10.14333127 * humidity
               - 0.22475541 * temperature * humidity
               - 6.83783e-3 * temperature * temperature
               - 5.481717e-2 * humidity * humidity
               + 1.22874e-3 * temperature * temperature * humidity
               + 8.5282e-4 * temperature * humidity * humidity
               - 1.99e-6 * temperature * temperature * humidity * humidity;
    
    return hi;
}

float EnvironmentalSuite::calculateDewPoint(float temperature, float humidity) {
    // Dew point calculation using Magnus formula
    float a = 17.27;
    float b = 237.7;
    
    float alpha = ((a * temperature) / (b + temperature)) + log(humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);
    
    return dewPoint;
}

float EnvironmentalSuite::calculateAirQualityIndex() {
    // Calculate AQI based on PM2.5, PM10, CO2, and VOCs
    float pm25AQI = (currentData_.airQuality.pm25 / 12.0) * 50.0; // US EPA breakpoint
    float co2AQI = (currentData_.airQuality.co2 / 1000.0) * 100.0; // Simplified scale
    float vocAQI = (currentData_.airQuality.vocs / 1000.0) * 100.0; // Simplified scale
    
    // Take the maximum of all AQI components
    float overallAQI = std::max({pm25AQI, co2AQI, vocAQI});
    
    return std::min(overallAQI, 500.0f); // Cap at 500 (hazardous)
}

float EnvironmentalSuite::calculateWildlifeComfortIndex() {
    float comfortIndex = 100.0; // Start with perfect comfort
    
    // Temperature stress
    if (currentData_.weather.temperature < 5 || currentData_.weather.temperature > 30) {
        comfortIndex -= 20.0; // Temperature stress
    }
    
    // Humidity stress
    if (currentData_.weather.humidity < 20 || currentData_.weather.humidity > 80) {
        comfortIndex -= 15.0; // Humidity stress
    }
    
    // Wind stress
    if (currentData_.weather.windSpeed > 10.0) {
        comfortIndex -= 10.0; // High wind stress
    }
    
    // Air quality stress
    if (currentData_.derived.airQualityIndex > 100) {
        comfortIndex -= 25.0; // Poor air quality
    }
    
    // Noise stress
    if (currentData_.acoustic.averageDecibels > 60.0) {
        comfortIndex -= 15.0; // Noise pollution
    }
    
    return std::max(comfortIndex, 0.0f);
}

float EnvironmentalSuite::calculateEcosystemHealthScore() {
    float healthScore = 100.0; // Start with perfect health
    
    // Air quality impact
    healthScore -= (currentData_.derived.airQualityIndex / 500.0) * 30.0;
    
    // Soil health impact
    if (currentData_.habitat.soilMoisture < 20 || currentData_.habitat.soilMoisture > 80) {
        healthScore -= 15.0;
    }
    
    // pH impact
    if (currentData_.habitat.soilPH < 6.0 || currentData_.habitat.soilPH > 8.0) {
        healthScore -= 10.0;
    }
    
    // Light availability impact
    if (currentData_.light.luxLevel < 1000) {
        healthScore -= 10.0; // Low light availability
    }
    
    // Acoustic pollution impact
    if (currentData_.acoustic.averageDecibels > 50.0) {
        healthScore -= (currentData_.acoustic.averageDecibels - 50.0) / 2.0;
    }
    
    return std::max(healthScore, 0.0f);
}

float EnvironmentalSuite::calculateEnvironmentalStressScore() {
    float stressScore = 0.0; // Start with no stress
    
    // Temperature stress
    if (currentData_.weather.temperature < 0 || currentData_.weather.temperature > 35) {
        stressScore += 25.0;
    }
    
    // Extreme weather stress
    if (currentData_.weather.windSpeed > 15.0) {
        stressScore += 20.0;
    }
    
    // Air pollution stress
    stressScore += (currentData_.derived.airQualityIndex / 500.0) * 40.0;
    
    // Noise stress
    if (currentData_.acoustic.averageDecibels > 65.0) {
        stressScore += 15.0;
    }
    
    return std::min(stressScore, 100.0f);
}

// ===========================
// POWER MANAGEMENT
// ===========================

void EnvironmentalSuite::enablePowerOptimization(bool enable) {
    powerOptimizationEnabled_ = enable;
    
    if (enable) {
        // Reduce reading frequency
        config_.readingInterval *= 2;
        
        // Put non-critical sensors to sleep
        enableLowPowerMode();
        
        Serial.println("Environmental sensors power optimization enabled");
    } else {
        // Restore normal operation
        config_.readingInterval = ENVIRONMENTAL_DEFAULT_READING_INTERVAL;
        
        // Wake up all sensors
        disableLowPowerMode();
        
        Serial.println("Environmental sensors full power mode enabled");
    }
}

void EnvironmentalSuite::adjustReadingFrequency(float batteryLevel) {
    if (!powerOptimizationEnabled_) {
        return;
    }
    
    if (batteryLevel < 0.2) {
        // Very low battery - minimal readings
        config_.readingInterval = 300000; // 5 minutes
        enableCriticalSensorsOnly();
    } else if (batteryLevel < 0.5) {
        // Low battery - reduced readings
        config_.readingInterval = 120000; // 2 minutes
        enableEssentialSensorsOnly();
    } else {
        // Normal battery - standard readings
        config_.readingInterval = ENVIRONMENTAL_DEFAULT_READING_INTERVAL;
        enableAllSensors();
    }
}

void EnvironmentalSuite::enableLowPowerMode() {
    // Put sensors into low power mode
    weatherStation_.enableLowPowerMode();
    airQualityMonitor_.enableLowPowerMode();
    acousticMonitor_.enableLowPowerMode();
    habitatSensors_.enableLowPowerMode();
    lightMonitor_.enableLowPowerMode();
}

void EnvironmentalSuite::disableLowPowerMode() {
    // Wake up sensors from low power mode
    weatherStation_.disableLowPowerMode();
    airQualityMonitor_.disableLowPowerMode();
    acousticMonitor_.disableLowPowerMode();
    habitatSensors_.disableLowPowerMode();
    lightMonitor_.disableLowPowerMode();
}

// ===========================
// DATA MANAGEMENT
// ===========================

void EnvironmentalSuite::storeCurrentReading() {
    sensorReadings_.push_back(currentData_);
    
    // Limit history size
    if (sensorReadings_.size() > ENVIRONMENTAL_MAX_HISTORY) {
        sensorReadings_.erase(sensorReadings_.begin());
    }
}

bool EnvironmentalSuite::logEnvironmentalData() {
    // Log data to storage (SD card, flash, etc.)
    // Implementation would write to actual storage
    
    statistics_.dataPointsLogged++;
    return true;
}

std::vector<EnvironmentalData> EnvironmentalSuite::getHistoricalData(
    unsigned long startTime, 
    unsigned long endTime) const {
    
    std::vector<EnvironmentalData> filtered;
    
    for (const auto& reading : sensorReadings_) {
        if (reading.timestamp >= startTime && reading.timestamp <= endTime) {
            filtered.push_back(reading);
        }
    }
    
    return filtered;
}

EnvironmentalTrends EnvironmentalSuite::calculateTrends() const {
    EnvironmentalTrends trends;
    memset(&trends, 0, sizeof(trends));
    
    if (sensorReadings_.size() < 2) {
        return trends; // Need at least 2 points for trends
    }
    
    // Calculate temperature trend
    trends.temperatureTrend = calculateLinearTrend(
        [](const EnvironmentalData& data) { return data.weather.temperature; }
    );
    
    // Calculate humidity trend
    trends.humidityTrend = calculateLinearTrend(
        [](const EnvironmentalData& data) { return data.weather.humidity; }
    );
    
    // Calculate air quality trend
    trends.airQualityTrend = calculateLinearTrend(
        [](const EnvironmentalData& data) { return data.derived.airQualityIndex; }
    );
    
    trends.timestamp = millis();
    
    return trends;
}

// ===========================
// STATISTICS AND MONITORING
// ===========================

EnvironmentalStatistics EnvironmentalSuite::getStatistics() const {
    return statistics_;
}

void EnvironmentalSuite::resetStatistics() {
    memset(&statistics_, 0, sizeof(statistics_));
    statistics_.startTime = millis();
}

uint8_t EnvironmentalSuite::getSensorHealthStatus() const {
    uint8_t status = 0;
    
    if (weatherStation_.isHealthy()) status |= SENSOR_STATUS_WEATHER_OK;
    if (airQualityMonitor_.isHealthy()) status |= SENSOR_STATUS_AIR_QUALITY_OK;
    if (acousticMonitor_.isHealthy()) status |= SENSOR_STATUS_ACOUSTIC_OK;
    if (habitatSensors_.isHealthy()) status |= SENSOR_STATUS_HABITAT_OK;
    if (lightMonitor_.isHealthy()) status |= SENSOR_STATUS_LIGHT_OK;
    
    return status;
}

bool EnvironmentalSuite::isSystemHealthy() const {
    return (getSensorHealthStatus() != 0) && 
           (statistics_.sensorErrors < 10) &&
           (millis() - lastReading_ < config_.readingInterval * 3);
}

void EnvironmentalSuite::updateStatistics() {
    statistics_.totalReadings++;
    statistics_.uptime = millis() - statistics_.startTime;
    
    // Update reading rate
    if (statistics_.uptime > 0) {
        statistics_.readingRate = (statistics_.totalReadings * 1000.0) / statistics_.uptime;
    }
    
    // Update data logging rate
    if (statistics_.totalReadings > 0) {
        statistics_.dataLoggingSuccessRate = 
            (float)statistics_.dataPointsLogged / statistics_.totalReadings;
    }
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

bool EnvironmentalSuite::initializeWeatherStation() {
    return weatherStation_.initialize(config_.weatherConfig);
}

bool EnvironmentalSuite::initializeAirQualitySensors() {
    return airQualityMonitor_.initialize(config_.airQualityConfig);
}

bool EnvironmentalSuite::initializeAcousticMonitoring() {
    return acousticMonitor_.initialize(config_.acousticConfig);
}

bool EnvironmentalSuite::initializeHabitatSensors() {
    return habitatSensors_.initialize(config_.habitatConfig);
}

bool EnvironmentalSuite::initializeLightMonitoring() {
    return lightMonitor_.initialize(config_.lightConfig);
}

bool EnvironmentalSuite::initializeDataLogging() {
    // Initialize data logging system
    return true; // Placeholder
}

void EnvironmentalSuite::enableCriticalSensorsOnly() {
    // Enable only critical sensors for emergency power mode
    weatherStation_.setEnabled(true);
    airQualityMonitor_.setEnabled(false);
    acousticMonitor_.setEnabled(false);
    habitatSensors_.setEnabled(false);
    lightMonitor_.setEnabled(false);
}

void EnvironmentalSuite::enableEssentialSensorsOnly() {
    // Enable essential sensors for low power mode
    weatherStation_.setEnabled(true);
    airQualityMonitor_.setEnabled(true);
    acousticMonitor_.setEnabled(false);
    habitatSensors_.setEnabled(false);
    lightMonitor_.setEnabled(true);
}

void EnvironmentalSuite::enableAllSensors() {
    // Enable all sensors for normal operation
    weatherStation_.setEnabled(true);
    airQualityMonitor_.setEnabled(true);
    acousticMonitor_.setEnabled(true);
    habitatSensors_.setEnabled(true);
    lightMonitor_.setEnabled(true);
}

float EnvironmentalSuite::calculateLinearTrend(
    std::function<float(const EnvironmentalData&)> valueExtractor) const {
    
    if (sensorReadings_.size() < 2) {
        return 0.0; // No trend with insufficient data
    }
    
    // Simple linear regression to calculate trend
    float n = sensorReadings_.size();
    float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    
    for (size_t i = 0; i < sensorReadings_.size(); i++) {
        float x = i; // Time index
        float y = valueExtractor(sensorReadings_[i]);
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate slope (trend)
    float slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    
    return slope;
}