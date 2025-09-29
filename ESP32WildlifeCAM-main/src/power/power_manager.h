#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief Power management class for ESP32-CAM wildlife camera
 * 
 * Manages battery monitoring, power saving modes, CPU frequency scaling,
 * and deep sleep functionality for extended battery life.
 */
class PowerManager {
public:
    /**
     * @brief Power states
     */
    enum class PowerState {
        NORMAL,         // Normal operation
        POWER_SAVE,     // Reduced power consumption
        LOW_BATTERY,    // Low battery warning state
        CRITICAL,       // Critical battery state
        CHARGING        // Charging state
    };

    /**
     * @brief Power modes for different operation scenarios
     */
    enum class PowerMode {
        MAX_PERFORMANCE,    // Maximum performance (240MHz)
        BALANCED,          // Balanced performance (160MHz)
        ECO_MODE,          // Economy mode (120MHz)
        SURVIVAL,          // Survival mode (80MHz)
        HIBERNATION        // Deep sleep preparation
    };

    /**
     * @brief Power statistics
     */
    struct PowerStats {
        float batteryVoltage;
        float batteryPercentage;
        float solarVoltage;
        bool isCharging;
        PowerState currentState;
        PowerMode currentMode;
        uint32_t uptime;
        uint32_t sleepTime;
        uint32_t totalSleepCycles;
        float averagePowerConsumption;
        uint32_t estimatedBatteryLife;
    };

    /**
     * @brief Power configuration
     */
    struct PowerConfig {
        float lowBatteryThreshold;
        float criticalBatteryThreshold;
        uint32_t deepSleepDuration;
        bool adaptiveDutyCycle;
        bool solarChargingEnabled;
        uint8_t maxCpuFreq;
        uint8_t minCpuFreq;
    };

    /**
     * @brief Initialize power management system
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Update power system (call regularly)
     */
    void update();

    /**
     * @brief Get current battery voltage
     * @return Battery voltage in volts
     */
    float getBatteryVoltage();

    /**
     * @brief Get current battery percentage
     * @return Battery percentage (0-100)
     */
    float getBatteryPercentage();

    /**
     * @brief Get current power state
     * @return Current power state
     */
    PowerState getPowerState() const { return currentState; }

    /**
     * @brief Set power mode
     * @param mode Target power mode
     */
    void setPowerMode(PowerMode mode);

    /**
     * @brief Get current power mode
     * @return Current power mode
     */
    PowerMode getPowerMode() const { return currentMode; }

    /**
     * @brief Check if system should enter deep sleep
     * @return true if deep sleep recommended
     */
    bool shouldEnterDeepSleep();

    /**
     * @brief Prepare for deep sleep
     * @param sleepDuration Sleep duration in milliseconds
     */
    void prepareForDeepSleep(uint32_t sleepDuration = 0);

    /**
     * @brief Enter deep sleep mode
     * @param sleepDuration Sleep duration in milliseconds
     */
    void enterDeepSleep(uint32_t sleepDuration = 0);

    /**
     * @brief Configure adaptive duty cycling
     * @param enable Enable adaptive duty cycling
     * @param batteryThreshold Battery threshold for activation
     */
    void configureAdaptiveDutyCycle(bool enable, float batteryThreshold = 50.0f);

    /**
     * @brief Get optimal sleep duration based on battery level
     * @return Recommended sleep duration in milliseconds
     */
    uint32_t getOptimalSleepDuration();

    /**
     * @brief Set CPU frequency
     * @param frequencyMHz CPU frequency in MHz
     */
    void setCpuFrequency(uint8_t frequencyMHz);

    /**
     * @brief Get current CPU frequency
     * @return CPU frequency in MHz
     */
    uint8_t getCpuFrequency();

    /**
     * @brief Get power statistics
     * @return Current power statistics
     */
    PowerStats getStatistics();

    /**
     * @brief Get power configuration
     * @return Current power configuration
     */
    PowerConfig getConfiguration() const { return config; }

    /**
     * @brief Set power configuration
     * @param newConfig New power configuration
     */
    void setConfiguration(const PowerConfig& newConfig);

    /**
     * @brief Reset power statistics
     */
    void resetStatistics();

    /**
     * @brief Check if system is charging
     * @return true if charging
     */
    bool isCharging();

    /**
     * @brief Enable or disable power saving features
     * @param enable Enable power saving
     */
    void setPowerSavingEnabled(bool enable);

    /**
     * @brief Cleanup power management resources
     */
    void cleanup();

private:
    bool initialized = false;
    bool powerSavingEnabled = true;
    PowerState currentState = PowerState::NORMAL;
    PowerMode currentMode = PowerMode::BALANCED;
    
    PowerConfig config;
    PowerStats stats = {};
    
    uint32_t lastUpdateTime = 0;
    uint32_t lastBatteryCheck = 0;
    uint32_t bootTime = 0;
    uint32_t totalSleepTime = 0;
    
    // Battery monitoring
    float batteryVoltage = 0.0f;
    float batteryPercentage = 0.0f;
    float solarVoltage = 0.0f;
    bool chargingState = false;
    
    // Adaptive duty cycling
    bool adaptiveDutyCycleEnabled = false;
    float dutyCycleBatteryThreshold = 50.0f;
    uint32_t adaptiveSleepDuration = 0;

    /**
     * @brief Read battery voltage from ADC
     * @return Raw battery voltage
     */
    float readBatteryVoltage();

    /**
     * @brief Read solar voltage from ADC
     * @return Solar panel voltage
     */
    float readSolarVoltage();

    /**
     * @brief Convert battery voltage to percentage
     * @param voltage Battery voltage
     * @return Battery percentage (0-100)
     */
    float voltageToPercentage(float voltage);

    /**
     * @brief Update power state based on battery level
     */
    void updatePowerState();

    /**
     * @brief Apply power mode settings
     * @param mode Power mode to apply
     */
    void applyPowerMode(PowerMode mode);

    /**
     * @brief Calculate optimal power mode based on conditions
     * @return Recommended power mode
     */
    PowerMode calculateOptimalPowerMode();

    /**
     * @brief Update statistics
     */
    void updateStatistics();

    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();

    /**
     * @brief Configure GPIO for low power operation
     */
    void configureGpioForLowPower();

    /**
     * @brief Disable unused peripherals
     */
    void disableUnusedPeripherals();
};

#endif // POWER_MANAGER_H