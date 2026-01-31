#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>

/**
 * @brief Structure to hold energy accounting statistics
 * 
 * Tracks power usage and battery consumption for detailed analytics.
 */
struct EnergyStats {
    unsigned long activeTimeMs;      ///< Total active (non-sleep) time in milliseconds
    unsigned long sleepTimeMs;       ///< Total deep sleep time in milliseconds
    unsigned long captureCount;      ///< Number of image captures
    unsigned long transmitCount;     ///< Number of data transmissions
    float energyConsumedMah;         ///< Estimated energy consumed in mAh
    float peakCurrentMa;             ///< Peak current draw estimate in mA
    float averageCurrentMa;          ///< Average current draw estimate in mA
    unsigned long lastUpdateTime;    ///< Timestamp of last update
    float batteryCapacityMah;        ///< Battery capacity in mAh
    float estimatedRemainingHours;   ///< Estimated remaining runtime in hours
    bool isValid;                    ///< Whether statistics are valid
};

/**
 * @brief Power consumption profile for different operations
 */
struct PowerProfile {
    float idleCurrentMa;             ///< Current draw when idle (mA)
    float captureCurrentMa;          ///< Current draw during image capture (mA)
    float transmitCurrentMa;         ///< Current draw during WiFi transmission (mA)
    float deepSleepCurrentUa;        ///< Current draw during deep sleep (uA)
    unsigned long captureDurationMs; ///< Average capture duration (ms)
    unsigned long transmitDurationMs;///< Average transmission duration (ms)
};

class PowerManager {
private:
    int batteryPin;
    float batteryVoltage;
    bool lowPowerMode;
    const float BATTERY_MIN = 3.3;
    const float BATTERY_MAX = 4.2;
    
    // Energy accounting members
    EnergyStats energyStats;
    PowerProfile powerProfile;
    unsigned long sessionStartTime;
    unsigned long lastVoltageReadTime;
    float initialVoltage;
    bool energyTrackingEnabled;
    
    /**
     * @brief Update energy consumption estimates based on activity
     */
    void updateEnergyEstimate();

public:
    PowerManager();
    
    bool init(int batteryMonitorPin = 35);
    float getBatteryVoltage();
    int getBatteryPercentage();
    bool isLowBattery();
    void enterDeepSleep(uint64_t sleepTimeSeconds);
    void configureWakeOnMotion(int pirPin);
    void configureWakeOnTimer(uint64_t seconds);
    void printPowerStatus();
    
    // ============================================================================
    // Energy Accounting Features (In Progress → Complete)
    // ============================================================================
    
    /**
     * @brief Enable or disable energy tracking
     * 
     * When enabled, the PowerManager tracks power consumption and estimates
     * remaining battery life based on usage patterns.
     * 
     * @param enable true to enable energy tracking, false to disable
     */
    void setEnergyTrackingEnabled(bool enable);
    
    /**
     * @brief Check if energy tracking is enabled
     * 
     * @return bool true if energy tracking is enabled
     */
    bool isEnergyTrackingEnabled() const;
    
    /**
     * @brief Record an image capture event for energy accounting
     * 
     * Adds the power consumption of an image capture operation to
     * the energy accounting statistics.
     */
    void recordCapture();
    
    /**
     * @brief Record a data transmission event for energy accounting
     * 
     * Adds the power consumption of a data transmission operation to
     * the energy accounting statistics.
     */
    void recordTransmission();
    
    /**
     * @brief Record deep sleep period for energy accounting
     * 
     * Updates the energy accounting with time spent in deep sleep.
     * Should be called after waking from deep sleep.
     * 
     * @param sleepDurationMs Duration of deep sleep in milliseconds
     */
    void recordSleepPeriod(unsigned long sleepDurationMs);
    
    /**
     * @brief Get current energy accounting statistics
     * 
     * Returns detailed statistics about power consumption and
     * estimated remaining battery life.
     * 
     * @return EnergyStats Current energy statistics
     */
    EnergyStats getEnergyStats() const;
    
    /**
     * @brief Reset energy accounting statistics
     * 
     * Clears all energy accounting data and resets counters.
     * Useful when starting a new monitoring session or replacing battery.
     */
    void resetEnergyStats();
    
    /**
     * @brief Set the battery capacity for runtime estimation
     * 
     * @param capacityMah Battery capacity in milliamp-hours
     */
    void setBatteryCapacity(float capacityMah);
    
    /**
     * @brief Get estimated remaining runtime in hours
     * 
     * Calculates the estimated remaining runtime based on current
     * battery level and average power consumption.
     * 
     * @return float Estimated remaining hours, or -1 if cannot estimate
     */
    float getEstimatedRemainingHours();
    
    /**
     * @brief Set the power profile for energy estimation
     * 
     * Configures the power consumption values for different operations.
     * 
     * @param profile Power profile configuration
     */
    void setPowerProfile(const PowerProfile& profile);
    
    /**
     * @brief Get the current power profile
     * 
     * @return PowerProfile Current power profile configuration
     */
    PowerProfile getPowerProfile() const;
    
    /**
     * @brief Print detailed energy accounting report
     * 
     * Outputs a comprehensive report of energy usage statistics
     * to the Serial monitor.
     */
    void printEnergyReport();
};

#endif // POWER_MANAGER_H
