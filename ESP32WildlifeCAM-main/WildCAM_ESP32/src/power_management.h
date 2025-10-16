/**
 * @file power_management.h
 * @brief Advanced Power Management System for ESP32 Wildlife CAM
 * @author ESP32 Wildlife CAM Project
 * @date 2025-09-01
 * 
 * This module provides comprehensive power management for extended field deployment,
 * including battery monitoring, solar charging optimization, and intelligent power
 * conservation strategies.
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

/**
 * @brief Power management states
 */
enum PowerState {
    POWER_STATE_NORMAL = 0,         ///< Normal operation mode
    POWER_STATE_CONSERVATION,       ///< Power conservation mode
    POWER_STATE_EMERGENCY,          ///< Emergency low power mode
    POWER_STATE_CHARGING,           ///< Actively charging
    POWER_STATE_SLEEP,              ///< Sleep mode
    POWER_STATE_DEEP_SLEEP,         ///< Deep sleep mode
    POWER_STATE_CRITICAL            ///< Critical battery level
};

/**
 * @brief Power source types
 */
enum PowerSource {
    POWER_SOURCE_BATTERY = 0,       ///< Running on battery
    POWER_SOURCE_SOLAR,             ///< Charging from solar
    POWER_SOURCE_EXTERNAL,          ///< External power source
    POWER_SOURCE_UNKNOWN            ///< Unknown power source
};

/**
 * @brief Power consumption profiles
 */
enum PowerProfile {
    POWER_PROFILE_PERFORMANCE = 0,  ///< High performance, higher consumption
    POWER_PROFILE_BALANCED,         ///< Balanced performance and efficiency
    POWER_PROFILE_EFFICIENCY,       ///< Efficiency optimized
    POWER_PROFILE_SURVIVAL          ///< Maximum battery life
};

/**
 * @brief Battery information structure
 */
struct BatteryInfo {
    float voltage;                  ///< Current battery voltage (V)
    float current;                  ///< Current draw (mA, positive = discharge)
    uint8_t percentage;             ///< Battery percentage (0-100)
    uint32_t capacity_mah;          ///< Battery capacity (mAh)
    uint32_t remaining_mah;         ///< Remaining capacity (mAh)
    float temperature;              ///< Battery temperature (°C)
    uint32_t cycle_count;           ///< Number of charge cycles
    bool is_charging;               ///< Whether battery is charging
    uint32_t time_to_empty_hours;   ///< Estimated time to empty (hours)
    uint32_t time_to_full_hours;    ///< Estimated time to full charge (hours)
};

/**
 * @brief Solar charging information
 */
struct SolarInfo {
    float voltage;                  ///< Solar panel voltage (V)
    float current;                  ///< Solar panel current (mA)
    float power;                    ///< Solar panel power (mW)
    bool is_active;                 ///< Whether solar panel is actively charging
    float efficiency;               ///< Charging efficiency (%)
    uint32_t daily_energy_mwh;      ///< Daily energy harvested (mWh)
    uint32_t total_energy_mwh;      ///< Total energy harvested (mWh)
    float peak_power_today;         ///< Peak power today (mW)
    uint8_t sunlight_hours;         ///< Estimated sunlight hours today
};

/**
 * @brief Power consumption statistics
 */
struct PowerStats {
    float avg_current_ma;           ///< Average current consumption (mA)
    float peak_current_ma;          ///< Peak current consumption (mA)
    uint32_t total_energy_consumed; ///< Total energy consumed (mWh)
    uint32_t uptime_hours;          ///< Total uptime (hours)
    uint32_t sleep_time_hours;      ///< Total sleep time (hours)
    float efficiency_rating;        ///< Power efficiency rating (%)
    uint32_t wake_events;           ///< Number of wake events
    uint32_t emergency_shutdowns;   ///< Number of emergency shutdowns
};

/**
 * @brief Power configuration structure
 */
struct PowerConfig {
    // Voltage thresholds
    float critical_voltage;         ///< Critical battery voltage (V)
    float low_voltage;              ///< Low battery voltage (V)
    float full_voltage;             ///< Full battery voltage (V)
    
    // Current limits
    float max_discharge_current;    ///< Maximum discharge current (mA)
    float max_charge_current;       ///< Maximum charge current (mA)
    
    // Sleep configuration
    uint32_t sleep_timeout_ms;      ///< Timeout before sleep (ms)
    uint32_t deep_sleep_timeout_ms; ///< Timeout before deep sleep (ms)
    bool wake_on_motion;            ///< Wake on motion detection
    bool wake_on_timer;             ///< Wake on timer
    uint32_t timer_wake_interval_ms; ///< Timer wake interval (ms)
    
    // Solar charging
    bool solar_enabled;             ///< Solar charging enabled
    float solar_max_voltage;        ///< Maximum solar voltage (V)
    float solar_mppt_voltage;       ///< MPPT voltage setpoint (V)
    
    // Power profiles
    PowerProfile profile;           ///< Active power profile
    bool adaptive_profile;          ///< Enable adaptive profile switching
    
    // Safety settings
    bool over_discharge_protection; ///< Over-discharge protection
    bool over_charge_protection;    ///< Over-charge protection
    bool temperature_protection;    ///< Temperature protection
    float max_temperature;          ///< Maximum operating temperature (°C)
};

/**
 * @brief Power Management Class
 * 
 * Comprehensive power management system for wildlife monitoring applications,
 * providing intelligent power conservation, battery monitoring, and solar charging.
 */
class PowerManager {
public:
    /**
     * @brief Constructor
     */
    PowerManager();
    
    /**
     * @brief Destructor
     * Ensures proper cleanup of RTOS resources (semaphores, tasks)
     */
    ~PowerManager();
    
    /**
     * @brief Initialize power management system
     * @param config Power configuration
     * @return true if successful, false otherwise
     */
    bool initialize(const PowerConfig& config);
    
    /**
     * @brief Update power management (call regularly)
     * @note Thread-safe: Protected by internal mutex
     */
    void update();
    
    /**
     * @brief Get current battery information
     * @return BatteryInfo structure
     * @note Thread-safe: Returns copy of internal state
     */
    BatteryInfo getBatteryInfo() const;
    
    /**
     * @brief Get solar charging information
     * @return SolarInfo structure
     */
    SolarInfo getSolarInfo() const;
    
    /**
     * @brief Get power consumption statistics
     * @return PowerStats structure
     */
    PowerStats getPowerStats() const;
    
    /**
     * @brief Get current power state
     * @return Current PowerState
     */
    PowerState getPowerState() const { return m_power_state; }
    
    /**
     * @brief Get current power source
     * @return Current PowerSource
     */
    PowerSource getPowerSource() const { return m_power_source; }
    
    /**
     * @brief Set power profile
     * @param profile Power profile to use
     */
    void setPowerProfile(PowerProfile profile);
    
    /**
     * @brief Get current power profile
     * @return Current PowerProfile
     */
    PowerProfile getPowerProfile() const { return m_config.profile; }
    
    /**
     * @brief Enter sleep mode
     * @param duration_ms Sleep duration in milliseconds (0 = indefinite)
     * @return true if sleep entered successfully
     */
    bool enterSleep(uint32_t duration_ms = 0);
    
    /**
     * @brief Enter deep sleep mode
     * @param duration_ms Deep sleep duration in milliseconds (0 = indefinite)
     * @return true if deep sleep entered successfully
     */
    bool enterDeepSleep(uint32_t duration_ms = 0);
    
    /**
     * @brief Wake from sleep mode
     */
    void wakeFromSleep();
    
    /**
     * @brief Check if system should enter sleep mode
     * @return true if sleep recommended
     */
    bool shouldEnterSleep() const;
    
    /**
     * @brief Check if system should enter deep sleep mode
     * @return true if deep sleep recommended
     */
    bool shouldEnterDeepSleep() const;
    
    /**
     * @brief Enable or disable solar charging
     * @param enabled Solar charging enabled state
     */
    void setSolarChargingEnabled(bool enabled);
    
    /**
     * @brief Check if solar charging is enabled
     * @return true if enabled, false otherwise
     */
    bool isSolarChargingEnabled() const { return m_config.solar_enabled; }
    
    /**
     * @brief Get estimated runtime in current conditions
     * @return Estimated runtime in hours
     */
    uint32_t getEstimatedRuntimeHours() const;
    
    /**
     * @brief Get estimated charge time
     * @return Estimated charge time in hours
     */
    uint32_t getEstimatedChargeTimeHours() const;
    
    /**
     * @brief Check if battery is critical
     * @return true if battery is at critical level
     */
    bool isBatteryCritical() const;
    
    /**
     * @brief Check if battery is low
     * @return true if battery is at low level
     */
    bool isBatteryLow() const;
    
    /**
     * @brief Check if battery is charging
     * @return true if battery is charging
     */
    bool isCharging() const;
    
    /**
     * @brief Get power consumption for specific component
     * @param component Component name
     * @return Current consumption in mA
     */
    float getComponentPowerConsumption(const char* component) const;
    
    /**
     * @brief Enable power monitoring for component
     * @param component Component name
     * @param enabled Monitoring enabled state
     */
    void setComponentMonitoring(const char* component, bool enabled);
    
    /**
     * @brief Trigger emergency shutdown
     * @param reason Shutdown reason
     */
    void emergencyShutdown(const char* reason);
    
    /**
     * @brief Reset power statistics
     */
    void resetStatistics();
    
    /**
     * @brief Update power configuration
     * @param config New configuration
     * @return true if update successful
     */
    bool updateConfiguration(const PowerConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current PowerConfig
     */
    PowerConfig getCurrentConfiguration() const { return m_config; }
    
    /**
     * @brief Calibrate battery measurement
     * @param known_voltage Known battery voltage for calibration
     * @return true if calibration successful
     */
    bool calibrateBatteryMeasurement(float known_voltage);
    
    /**
     * @brief Get last error message
     * @return Last error message
     */
    const char* getLastError() const { return m_last_error; }

private:
    // Configuration and state
    PowerConfig m_config;           ///< Power configuration
    PowerState m_power_state;       ///< Current power state
    PowerSource m_power_source;     ///< Current power source
    bool m_initialized;             ///< Initialization state
    
    // Battery monitoring
    BatteryInfo m_battery_info;     ///< Current battery information
    SolarInfo m_solar_info;         ///< Current solar information
    PowerStats m_power_stats;       ///< Power statistics
    
    // Timing and control
    unsigned long m_last_update;    ///< Last update timestamp
    unsigned long m_sleep_timer;    ///< Sleep timer
    unsigned long m_stats_reset_time; ///< Statistics reset time
    SemaphoreHandle_t m_power_mutex; ///< Power state mutex
    
    // Measurement calibration
    float m_voltage_calibration;    ///< Voltage measurement calibration factor
    float m_current_calibration;    ///< Current measurement calibration factor
    
    // Error handling
    char m_last_error[128];         ///< Last error message
    
    // Private methods
    
    /**
     * @brief Read battery voltage
     * @return Battery voltage in volts
     */
    float readBatteryVoltage();
    
    /**
     * @brief Read battery current
     * @return Battery current in mA (positive = discharge)
     */
    float readBatteryCurrent();
    
    /**
     * @brief Read solar panel voltage
     * @return Solar voltage in volts
     */
    float readSolarVoltage();
    
    /**
     * @brief Read solar panel current
     * @return Solar current in mA
     */
    float readSolarCurrent();
    
    /**
     * @brief Calculate battery percentage
     * @param voltage Battery voltage
     * @return Battery percentage (0-100)
     */
    uint8_t calculateBatteryPercentage(float voltage);
    
    /**
     * @brief Update power state based on conditions
     */
    void updatePowerState();
    
    /**
     * @brief Update power source detection
     */
    void updatePowerSource();
    
    /**
     * @brief Update battery information
     */
    void updateBatteryInfo();
    
    /**
     * @brief Update solar information
     */
    void updateSolarInfo();
    
    /**
     * @brief Update power statistics
     */
    void updatePowerStats();
    
    /**
     * @brief Apply power profile settings
     * @param profile Power profile to apply
     */
    void applyPowerProfile(PowerProfile profile);
    
    /**
     * @brief Perform adaptive profile switching
     */
    void performAdaptiveProfileSwitch();
    
    /**
     * @brief Check safety conditions
     * @return true if safe to operate
     */
    bool checkSafetyConditions();
    
    /**
     * @brief Set error message
     * @param error Error message
     */
    void setError(const char* error);
    
    /**
     * @brief Configure wake sources for sleep modes
     */
    void configureWakeSources();
    
    /**
     * @brief Log power event
     * @param event Event description
     */
    void logPowerEvent(const char* event);
};

/**
 * @brief Get default power configuration
 * @return Default PowerConfig structure
 */
PowerConfig getDefaultPowerConfig();

/**
 * @brief Get power-optimized configuration for extended deployment
 * @return Power-optimized PowerConfig structure
 */
PowerConfig getPowerOptimizedConfig();

/**
 * @brief Get emergency power configuration
 * @return Emergency PowerConfig structure
 */
PowerConfig getEmergencyPowerConfig();

/**
 * @brief Convert PowerState to human-readable string
 * @param state Power state
 * @return Human-readable string
 */
const char* powerStateToString(PowerState state);

/**
 * @brief Convert PowerSource to human-readable string
 * @param source Power source
 * @return Human-readable string
 */
const char* powerSourceToString(PowerSource source);

/**
 * @brief Convert PowerProfile to human-readable string
 * @param profile Power profile
 * @return Human-readable string
 */
const char* powerProfileToString(PowerProfile profile);

#endif // POWER_MANAGEMENT_H