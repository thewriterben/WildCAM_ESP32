#ifndef MOTION_FILTER_H
#define MOTION_FILTER_H

#include <Arduino.h>

// Motion detection statistics structure
struct MotionStats {
    unsigned long lastMotionTime;
    int consecutiveCount;
    float windSpeed;
    float rainfall;
    float temperature;
    bool filteringEnabled;
};

// Motion filter status structure
struct MotionFilterStatus {
    bool initialized;
    unsigned long lastMotionTime;
    bool currentlyDetecting;
    bool weatherSensorActive;
    bool filteringActive;
    int consecutiveMotions;
};

/**
 * @class MotionFilter
 * @brief Intelligent motion detection with weather-based filtering
 * 
 * This class implements motion detection with environmental filtering
 * to eliminate false positives from wind, rain, and other factors.
 */
class MotionFilter {
public:
    /**
     * @brief Constructor
     */
    MotionFilter();

    /**
     * @brief Destructor
     */
    ~MotionFilter();

    /**
     * @brief Initialize motion detection system
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Check if motion is currently detected
     * @return true if motion detected, false otherwise
     */
    bool isMotionDetected();
    
    /**
     * @brief Validate motion against weather conditions and other filters
     * @return true if motion is valid, false if filtered out
     */
    bool isValidMotion();
    
    /**
     * @brief Get current motion detection statistics
     * @return Motion statistics structure
     */
    MotionStats getMotionStats() const;
    
    /**
     * @brief Set motion sensitivity (0-100)
     * @param sensitivity Sensitivity level from 0 (least sensitive) to 100 (most sensitive)
     */
    void setMotionSensitivity(int sensitivity);
    
    /**
     * @brief Enable or disable weather filtering
     * @param enabled true to enable weather filtering, false to disable
     */
    void setWeatherFiltering(bool enabled);
    
    /**
     * @brief Get detailed motion filter status
     * @return Motion filter status structure
     */
    MotionFilterStatus getStatus() const;
    
    /**
     * @brief Reset motion detection statistics
     */
    void resetStats();
    
    /**
     * @brief Cleanup motion filter resources
     */
    void cleanup();

    /**
     * @brief Check if motion filter is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Get current motion sensitivity setting
     * @return Motion sensitivity (0-100)
     */
    int getMotionSensitivity() const { return motionSensitivity; }

    /**
     * @brief Check if weather filtering is enabled
     * @return true if weather filtering is enabled
     */
    bool isWeatherFilteringEnabled() const { return weatherFilteringEnabled; }

private:
    // Member variables
    bool initialized;
    unsigned long lastMotionTime;
    bool motionDetected;
    int consecutiveMotions;
    float currentWindSpeed;
    float currentRainfall;
    float currentTemperature;
    int motionSensitivity;
    bool weatherFilteringEnabled;
    bool weatherSensorInitialized;
    unsigned long lastWeatherReading;
    
    // Private methods
    void updateWeatherData();
    bool isWeatherSuitable();
    static void pirInterrupt();
    float estimateWindSpeed();
    bool isTemperatureStable();
    void applyConfigurationSettings();
    
    // Static instance for interrupt handling
    static MotionFilter* instance;
};

#endif // MOTION_FILTER_H