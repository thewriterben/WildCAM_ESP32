/**
 * @file improved_power_manager.h
 * @brief Improved power management system with MPPT, advanced battery analytics,
 *        and adaptive sleep scheduling
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 1.0.0
 * 
 * This module provides an integrated power management solution combining:
 * - MPPT solar charge controller support
 * - Advanced battery analytics (SOC, SOH, multi-stage charging)
 * - Adaptive sleep schedules based on motion patterns
 */

#ifndef IMPROVED_POWER_MANAGER_H
#define IMPROVED_POWER_MANAGER_H

#include <Arduino.h>
#include "mppt_controller.h"
#include "battery_manager.h"
#include "solar_power_system.h"
#include "adaptive_sleep_scheduler.h"

/**
 * @brief Power system operating mode
 */
enum class PowerSystemMode {
    NORMAL,             // Normal operation
    POWER_SAVE,         // Power saving mode
    SOLAR_PRIORITY,     // Maximize solar harvesting
    BATTERY_PRESERVE,   // Preserve battery longevity
    EMERGENCY,          // Critical battery - minimal operation
    MAINTENANCE         // Maintenance/calibration mode
};

/**
 * @brief Comprehensive power system status
 */
struct ImprovedPowerStatus {
    // Battery metrics
    float battery_voltage;
    float battery_current;
    float battery_soc;          // State of Charge (%)
    float battery_soh;          // State of Health (%)
    float battery_temperature;
    BatteryHealth battery_health;
    ChargingStage charging_stage;
    int cycle_count;
    
    // Solar metrics
    float solar_voltage;
    float solar_current;
    float solar_power;
    bool solar_available;
    float daily_energy_harvest;
    float charge_efficiency;
    int cloud_cover_estimate;
    
    // System metrics
    PowerSystemMode operating_mode;
    uint32_t recommended_sleep_ms;
    bool is_active_time;
    float activity_score;
    
    // Time estimates
    float time_to_full_min;
    float time_to_empty_min;
    
    // Fault status
    bool has_fault;
    BatteryFault battery_fault;
};

/**
 * @brief Power system pin configuration
 */
struct ImprovedPowerPins {
    // Battery monitoring
    int battery_voltage_pin;
    int battery_current_pin;
    int battery_temp_pin;
    int charge_enable_pin;
    
    // Solar monitoring
    int solar_voltage_pin;
    int solar_current_pin;
    int mppt_pwm_pin;
};

/**
 * @brief Power system configuration
 */
struct ImprovedPowerConfig {
    // Battery thresholds
    float low_battery_threshold;        // Percentage
    float critical_battery_threshold;   // Percentage
    float full_charge_threshold;        // Percentage
    
    // Sleep configuration
    uint32_t min_sleep_duration_ms;
    uint32_t max_sleep_duration_ms;
    uint32_t default_sleep_duration_ms;
    
    // Solar configuration
    bool enable_mppt;
    MPPTAlgorithm mppt_algorithm;
    
    // Battery configuration
    BatteryChemistry battery_chemistry;
    int battery_cell_count;
    float battery_capacity_mah;
    
    // Power saving
    bool enable_adaptive_sleep;
    bool enable_auto_mode_switching;
    SleepAggressiveness sleep_aggressiveness;
};

/**
 * @brief Improved power management system
 */
class ImprovedPowerManager {
public:
    /**
     * @brief Constructor
     */
    ImprovedPowerManager();

    /**
     * @brief Destructor
     */
    ~ImprovedPowerManager();

    /**
     * @brief Initialize with pin and system configuration
     * @param pins Pin configuration
     * @param config System configuration
     * @return true if initialization successful
     */
    bool begin(const ImprovedPowerPins& pins, const ImprovedPowerConfig& config);

    /**
     * @brief Initialize with default configuration
     * @param pins Pin configuration
     * @return true if initialization successful
     */
    bool begin(const ImprovedPowerPins& pins);

    /**
     * @brief Main update loop - call regularly (every 100ms recommended)
     */
    void update();

    /**
     * @brief Get comprehensive power status
     * @return ImprovedPowerStatus structure
     */
    ImprovedPowerStatus getStatus() const;

    /**
     * @brief Get current battery voltage
     * @return Battery voltage in volts
     */
    float getBatteryVoltage() const;

    /**
     * @brief Get battery state of charge
     * @return SOC percentage (0-100)
     */
    float getBatterySOC() const;

    /**
     * @brief Get battery state of health
     * @return SOH percentage (0-100)
     */
    float getBatterySOH() const;

    /**
     * @brief Get battery health assessment
     * @return BatteryHealth enum
     */
    BatteryHealth getBatteryHealth() const;

    /**
     * @brief Check if battery is charging
     * @return true if charging
     */
    bool isCharging() const;

    /**
     * @brief Get solar power in milliwatts
     * @return Solar power in mW
     */
    float getSolarPower() const;

    /**
     * @brief Check if solar power is available
     * @return true if solar power available
     */
    bool isSolarAvailable() const;

    /**
     * @brief Get daily solar energy harvest
     * @return Energy in Wh
     */
    float getDailyEnergyHarvest() const;

    /**
     * @brief Set operating mode
     * @param mode Desired operating mode
     */
    void setOperatingMode(PowerSystemMode mode);

    /**
     * @brief Get current operating mode
     * @return Current PowerSystemMode
     */
    PowerSystemMode getOperatingMode() const { return operating_mode_; }

    /**
     * @brief Get recommended sleep duration based on patterns and battery
     * @return Sleep duration in milliseconds
     */
    uint32_t getRecommendedSleepDuration();

    /**
     * @brief Record motion event for pattern learning
     * @param timestamp Event timestamp (0 = now)
     */
    void recordMotionEvent(unsigned long timestamp = 0);

    /**
     * @brief Check if current time is typically active
     * @return true if typically active time
     */
    bool isActiveTime() const;

    /**
     * @brief Get activity score for current time
     * @return Activity score (0.0-1.0)
     */
    float getActivityScore() const;

    /**
     * @brief Enter deep sleep mode
     * @param duration_ms Sleep duration (0 = use recommended)
     */
    void enterDeepSleep(uint32_t duration_ms = 0);

    /**
     * @brief Prepare for deep sleep (save state, configure wakeup)
     * @param duration_ms Sleep duration
     */
    void prepareForDeepSleep(uint32_t duration_ms);

    /**
     * @brief Set current time for scheduling
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     */
    void setCurrentTime(uint8_t hour, uint8_t minute);

    /**
     * @brief Update weather conditions for MPPT optimization
     * @param cloud_cover Cloud cover percentage (0-100)
     * @param temperature Temperature in Celsius
     */
    void updateWeatherConditions(int cloud_cover, float temperature);

    /**
     * @brief Enable or disable charging
     * @param enable Enable charging
     */
    void setChargingEnabled(bool enable);

    /**
     * @brief Check if charging is enabled
     * @return true if enabled
     */
    bool isChargingEnabled() const;

    /**
     * @brief Check if system has any fault
     * @return true if fault present
     */
    bool hasFault() const;

    /**
     * @brief Clear fault conditions
     */
    void clearFault();

    /**
     * @brief Calibrate battery voltage sensor
     * @param known_voltage Known reference voltage
     */
    void calibrateBatteryVoltage(float known_voltage);

    /**
     * @brief Calibrate solar sensors
     * @param voltage_factor Voltage calibration factor
     * @param current_factor Current calibration factor
     */
    void calibrateSolarSensors(float voltage_factor, float current_factor);

    /**
     * @brief Reset motion patterns
     */
    void resetMotionPatterns();

    /**
     * @brief Reset daily energy counter
     */
    void resetDailyEnergy();

    /**
     * @brief Get MPPT controller for advanced access
     * @return Pointer to MPPTController (or nullptr)
     */
    MPPTController* getMPPTController() { 
        return solar_system_ ? solar_system_->getMPPTController() : nullptr; 
    }

    /**
     * @brief Get battery manager for advanced access
     * @return Pointer to BatteryManager (or nullptr)
     */
    BatteryManager* getBatteryManager() { 
        return solar_system_ ? solar_system_->getBatteryManager() : nullptr; 
    }

    /**
     * @brief Get sleep scheduler for advanced access
     * @return Pointer to AdaptiveSleepScheduler (or nullptr)
     */
    AdaptiveSleepScheduler* getSleepScheduler() { return sleep_scheduler_; }

    /**
     * @brief Print comprehensive status to Serial
     */
    void printStatus() const;

    /**
     * @brief Print daily schedule to Serial
     */
    void printDailySchedule() const;

    /**
     * @brief Get default configuration
     * @return Default ImprovedPowerConfig
     */
    static ImprovedPowerConfig getDefaultConfig();

private:
    // Component instances
    SolarPowerSystem* solar_system_;
    AdaptiveSleepScheduler* sleep_scheduler_;
    
    // Configuration
    ImprovedPowerPins pins_;
    ImprovedPowerConfig config_;
    
    // State
    bool initialized_;
    PowerSystemMode operating_mode_;
    
    // Timing
    unsigned long last_update_;
    unsigned long last_mode_evaluation_;
    
    // Cached values
    uint8_t current_hour_;
    uint8_t current_minute_;
    
    /**
     * @brief Initialize sub-components
     * @return true if all components initialized
     */
    bool initializeComponents();

    /**
     * @brief Evaluate and update operating mode
     */
    void evaluateOperatingMode();

    /**
     * @brief Apply mode-specific optimizations
     */
    void applyModeOptimizations();

    /**
     * @brief Determine optimal mode based on conditions
     * @return Recommended PowerSystemMode
     */
    PowerSystemMode determineOptimalMode() const;

    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();
};

#endif // IMPROVED_POWER_MANAGER_H
