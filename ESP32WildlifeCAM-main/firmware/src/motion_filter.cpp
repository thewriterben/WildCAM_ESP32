/**
 * Motion Filter Module
 * 
 * Implements intelligent motion detection with weather-based filtering
 * to eliminate false positives from wind, rain, and other environmental factors.
 */

#include "motion_filter.h"
#include "config.h"
#include <Arduino.h>
#include <cmath>  // For fabs() function

#ifdef BME280_ENABLED
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#endif

// Environmental integration
extern AdvancedEnvironmentalData getLatestEnvironmentalData();
extern bool shouldFilterMotionByEnvironment(const AdvancedEnvironmentalData& data);
extern float getEnvironmentalMotionThreshold(const AdvancedEnvironmentalData& data);
extern bool validateMotionWithEnvironment(bool motion_detected, uint16_t motion_confidence);

// Static instance for interrupt handling
MotionFilter* MotionFilter::instance = nullptr;

// Constructor
MotionFilter::MotionFilter() 
    : initialized(false), lastMotionTime(0), motionDetected(false),
      consecutiveMotions(0), currentWindSpeed(0.0), currentRainfall(0.0),
      currentTemperature(20.0), motionSensitivity(MOTION_SENSITIVITY),
      weatherFilteringEnabled(WEATHER_FILTERING_ENABLED), weatherSensorInitialized(false),
      lastWeatherReading(0) {
    instance = this;
    applyConfigurationSettings();
}

// Destructor
MotionFilter::~MotionFilter() {
    cleanup();
    instance = nullptr;
}

/**
 * Apply configuration settings from config.h
 */
void MotionFilter::applyConfigurationSettings() {
    motionSensitivity = MOTION_SENSITIVITY;
    weatherFilteringEnabled = WEATHER_FILTERING_ENABLED;
}

/**
 * Initialize motion detection system
 */
bool MotionFilter::init() {
    DEBUG_PRINTLN("Initializing motion filter system...");
    
    // Configure PIR sensor pin
    pinMode(PIR_PIN, INPUT);
    
    // Attach interrupt for motion detection
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, PIR_TRIGGER_MODE);
    
    #ifdef BME280_ENABLED
    // Initialize BME280 weather sensor if enabled
    if (BME280_ENABLED) {
        Wire.begin(BME280_SDA, BME280_SCL);
        
        static Adafruit_BME280 bme;
        weatherSensorInitialized = bme.begin(BME280_ADDRESS);
        
        if (weatherSensorInitialized) {
            DEBUG_PRINTLN("BME280 weather sensor initialized");
            
            // Configure BME280 settings
            bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating Mode
                           Adafruit_BME280::SAMPLING_X2,      // Temp. oversampling
                           Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling
                           Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling
                           Adafruit_BME280::FILTER_X16,       // Filtering
                           Adafruit_BME280::STANDBY_MS_500);  // Standby time
        } else {
            DEBUG_PRINTLN("Warning: BME280 initialization failed - using estimated values");
        }
    }
    #endif
    
    initialized = true;
    DEBUG_PRINTLN("Motion filter system initialized");
    
    return true;
}

/**
 * Check if motion is currently detected
 */
bool MotionFilter::isMotionDetected() {
    if (!initialized) return false;
    
    // Check if enough time has passed since last detection (debounce)
    if (millis() - lastMotionTime < PIR_DEBOUNCE_TIME) {
        return false;
    }
    
    // Update motion state
    bool currentMotion = motionDetected;
    motionDetected = false;  // Reset flag
    
    return currentMotion;
}

/**
 * Validate motion against weather conditions and other filters
 */
bool MotionFilter::isValidMotion() {
    if (!weatherFilteringEnabled) {
        return true;  // Accept all motion if filtering disabled
    }
    
    // Update weather data
    updateWeatherData();
    
    // Get environmental data for enhanced filtering
    AdvancedEnvironmentalData envData = getLatestEnvironmentalData();
    
    // Check if motion should be filtered by environmental conditions
    if (shouldFilterMotionByEnvironment(envData)) {
        DEBUG_PRINTLN("Motion filtered: environmental conditions");
        return false;
    }
    
    // Check weather conditions
    if (!isWeatherSuitable()) {
        DEBUG_PRINTLN("Motion filtered: unsuitable weather conditions");
        return false;
    }
    
    // Check temperature stability for PIR compensation
    if (TEMP_COMP_ENABLED && !isTemperatureStable()) {
        DEBUG_PRINTLN("Motion filtered: temperature fluctuation");
        return false;
    }
    
    // Environmental motion threshold adjustment
    float envThreshold = getEnvironmentalMotionThreshold(envData);
    uint16_t adjustedThreshold = (uint16_t)(MOTION_CONSECUTIVE_THRESHOLD * envThreshold);
    
    // Consecutive motion validation with environmental adjustment
    consecutiveMotions++;
    if (consecutiveMotions < adjustedThreshold) {
        DEBUG_PRINTF("Motion filtered: awaiting confirmation (%d/%d)\n", 
                    consecutiveMotions, adjustedThreshold);
        return false;
    }
    
    // Enhanced validation with environmental context
    uint16_t motion_confidence = 75; // Default confidence level
    if (!validateMotionWithEnvironment(true, motion_confidence)) {
        DEBUG_PRINTLN("Motion filtered: environmental validation failed");
        return false;
    }
    
    consecutiveMotions = 0;  // Reset counter
    DEBUG_PRINTF("Motion validated: environmental conditions favorable (wildlife activity: %d%%)\n", 
                envData.wildlife_activity_index);
    return true;
}

/**
 * Get current motion detection statistics
 */
MotionStats MotionFilter::getMotionStats() const {
    MotionStats stats;
    stats.lastMotionTime = lastMotionTime;
    stats.consecutiveCount = consecutiveMotions;
    stats.windSpeed = currentWindSpeed;
    stats.rainfall = currentRainfall;
    stats.temperature = currentTemperature;
    stats.filteringEnabled = weatherFilteringEnabled;
    
    return stats;
}

/**
 * Set motion sensitivity (0-100)
 */
void MotionFilter::setMotionSensitivity(int sensitivity) {
    motionSensitivity = CLAMP(sensitivity, 0, 100);
    
    DEBUG_PRINTF("Motion sensitivity set to %d%%\n", motionSensitivity);
}

/**
 * Enable or disable weather filtering
 */
void MotionFilter::setWeatherFiltering(bool enabled) {
    weatherFilteringEnabled = enabled;
    DEBUG_PRINTF("Weather filtering %s\n", enabled ? "enabled" : "disabled");
}

/**
 * Get detailed motion filter status
 */
MotionFilterStatus MotionFilter::getStatus() const {
    MotionFilterStatus status;
    status.initialized = initialized;
    status.lastMotionTime = lastMotionTime;
    status.currentlyDetecting = (millis() - lastMotionTime) < PIR_DEBOUNCE_TIME;
    status.weatherSensorActive = weatherSensorInitialized;
    status.filteringActive = weatherFilteringEnabled;
    status.consecutiveMotions = consecutiveMotions;
    
    return status;
}

/**
 * Reset motion detection statistics
 */
void MotionFilter::resetStats() {
    consecutiveMotions = 0;
    lastMotionTime = 0;
    currentWindSpeed = 0.0;
    currentRainfall = 0.0;
    currentTemperature = 20.0;
    DEBUG_PRINTLN("Motion detection statistics reset");
}

/**
 * Cleanup motion filter resources
 */
void MotionFilter::cleanup() {
    if (initialized) {
        detachInterrupt(digitalPinToInterrupt(PIR_PIN));
        initialized = false;
        DEBUG_PRINTLN("Motion filter system cleaned up");
    }
}

/**
 * PIR interrupt handler (static)
 */
void IRAM_ATTR MotionFilter::pirInterrupt() {
    if (instance) {
        instance->motionDetected = true;
        instance->lastMotionTime = millis();
    }
}

/**
 * Update weather data from sensors
 */
void MotionFilter::updateWeatherData() {
    unsigned long now = millis();
    
    // Only update weather data at configured intervals
    if (now - lastWeatherReading < WEATHER_READING_INTERVAL) {
        return;
    }
    
    lastWeatherReading = now;
    
    #ifdef BME280_ENABLED
    if (weatherSensorInitialized) {
        static Adafruit_BME280 bme;
        
        currentTemperature = bme.readTemperature();
        float pressure = bme.readPressure() / 100.0F;  // Convert to hPa
        float humidity = bme.readHumidity();
        
        // Estimate rainfall from humidity and pressure changes
        static float lastPressure = pressure;
        float pressureDelta = pressure - lastPressure;
        lastPressure = pressure;
        
        // Simple rainfall estimation (would need calibration)
        if (humidity > 85 && pressureDelta < -0.5) {
            currentRainfall = (90 - humidity) * 0.1;  // Rough estimate
        } else {
            currentRainfall = 0.0;
        }
        
        DEBUG_PRINTF("Weather: T=%.1f°C, H=%.1f%%, P=%.1fhPa, Rain=%.1fmm/h\n",
                    currentTemperature, humidity, pressure, currentRainfall);
    } else
    #endif
    {
        // Fallback: use default values or simple estimations
        currentTemperature = 20.0;  // Default temperature
        currentRainfall = 0.0;      // Assume no rain
    }
    
    // Estimate wind speed from motion sensor sensitivity
    currentWindSpeed = estimateWindSpeed();
}

/**
 * Check if weather conditions are suitable for motion detection
 */
bool MotionFilter::isWeatherSuitable() {
    // Check wind threshold
    if (currentWindSpeed > WIND_THRESHOLD) {
        DEBUG_PRINTF("Wind too strong: %.1f km/h (threshold: %.1f)\n", 
                    currentWindSpeed, WIND_THRESHOLD);
        return false;
    }
    
    // Check rainfall threshold
    if (currentRainfall > RAIN_THRESHOLD) {
        DEBUG_PRINTF("Rain too heavy: %.1f mm/h (threshold: %.1f)\n", 
                    currentRainfall, RAIN_THRESHOLD);
        return false;
    }
    
    return true;
}

/**
 * Estimate wind speed from motion sensor behavior
 */
float MotionFilter::estimateWindSpeed() {
    static unsigned long lastWindCheck = 0;
    static int falseMotionCount = 0;
    
    unsigned long now = millis();
    
    // Count false motions over the last minute
    if (now - lastWindCheck > 60000) {  // Every minute
        float estimatedWind = falseMotionCount * 2.5;  // Rough correlation
        falseMotionCount = 0;
        lastWindCheck = now;
        return estimatedWind;
    }
    
    // Increment false motion counter for consecutive quick triggers
    if (consecutiveMotions > 5) {
        falseMotionCount++;
    }
    
    return currentWindSpeed;  // Return last calculated value
}

/**
 * Check if temperature is stable enough for reliable PIR operation
 */
bool MotionFilter::isTemperatureStable() {
    static float lastTemperature = currentTemperature;
    static unsigned long lastTempCheck = 0;
    
    unsigned long now = millis();
    
    // Check temperature change rate
    if (now - lastTempCheck > 30000) {  // Every 30 seconds
        float tempDelta = fabs(currentTemperature - lastTemperature);
        lastTemperature = currentTemperature;
        lastTempCheck = now;
        
        // PIR sensors can be affected by rapid temperature changes
        if (tempDelta > TEMP_STABILITY_THRESHOLD) {
            DEBUG_PRINTF("Rapid temperature change: %.1f°C\n", tempDelta);
            return false;
        }
    }
    
    return true;
}