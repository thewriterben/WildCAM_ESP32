/**
 * @file solar_power_system.h
 * @brief Integrated solar power system combining MPPT and battery management
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 3.0.0
 * 
 * This module provides a unified interface for solar-powered operation:
 * - MPPT solar charging with multiple algorithm support
 * - Advanced battery management with multi-stage charging
 * - Automatic power optimization based on conditions
 * - Energy statistics and reporting
 */

#ifndef SOLAR_POWER_SYSTEM_H
#define SOLAR_POWER_SYSTEM_H

#include <Arduino.h>
#include "mppt_controller.h"
#include "battery_manager.h"

/**
 * @brief Solar power system operating mode
 */
enum class SolarPowerMode {
    AUTO,               // Automatic mode selection
    MAXIMUM_HARVEST,    // Prioritize solar energy collection
    BATTERY_PRESERVE,   // Prioritize battery longevity
    BALANCED,           // Balance between harvest and preservation
    EMERGENCY           // Low power emergency mode
};

/**
 * @brief Comprehensive power system status
 */
struct PowerSystemStatus {
    // Solar panel data
    float solar_voltage;
    float solar_current;
    float solar_power;
    bool solar_available;
    int cloud_cover;
    
    // Battery data
    float battery_voltage;
    float battery_current;
    float battery_soc;
    float battery_soh;
    float battery_temperature;
    ChargingStage charging_stage;
    BatteryHealth battery_health;
    bool battery_charging;
    
    // System data
    float charge_efficiency;
    float daily_energy_harvest;
    float total_energy_harvest;
    SolarPowerMode operating_mode;
    MPPTAlgorithm mppt_algorithm;
    
    // Time estimates
    float time_to_full;
    float time_to_empty;
    
    // Faults
    bool has_fault;
    BatteryFault battery_fault;
};

/**
 * @brief Pin configuration for solar power system
 */
struct SolarPowerPins {
    int solar_voltage;      // Solar panel voltage ADC pin
    int solar_current;      // Solar panel current ADC pin
    int battery_voltage;    // Battery voltage ADC pin
    int battery_current;    // Battery current ADC pin (optional: -1)
    int battery_temp;       // Battery temperature ADC pin (optional: -1)
    int mppt_pwm;           // MPPT PWM control pin
    int charge_enable;      // Charge enable GPIO pin (optional: -1)
};

/**
 * @brief Integrated solar power system class
 */
class SolarPowerSystem {
public:
    /**
     * @brief Constructor with pin configuration
     * @param pins Pin configuration structure
     */
    explicit SolarPowerSystem(const SolarPowerPins& pins);
    
    /**
     * @brief Destructor
     */
    ~SolarPowerSystem();
    
    /**
     * @brief Initialize the solar power system
     * @param battery_config Battery configuration
     * @param mppt_algorithm MPPT algorithm to use
     * @return true if initialization successful
     */
    bool begin(const BatteryConfig& battery_config = BatteryConfig(),
               MPPTAlgorithm mppt_algorithm = MPPTAlgorithm::PERTURB_OBSERVE);
    
    /**
     * @brief Initialize with default configuration
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Main update loop - call regularly (every 100ms recommended)
     */
    void update();
    
    /**
     * @brief Get comprehensive system status
     * @return PowerSystemStatus structure
     */
    PowerSystemStatus getStatus() const;
    
    /**
     * @brief Set operating mode
     * @param mode Desired operating mode
     */
    void setMode(SolarPowerMode mode);
    
    /**
     * @brief Get current operating mode
     * @return Current SolarPowerMode
     */
    SolarPowerMode getMode() const { return operating_mode_; }
    
    /**
     * @brief Set MPPT algorithm
     * @param algorithm MPPT algorithm to use
     */
    void setMPPTAlgorithm(MPPTAlgorithm algorithm);
    
    /**
     * @brief Enable or disable solar charging
     * @param enable true to enable
     */
    void setChargingEnabled(bool enable);
    
    /**
     * @brief Check if charging is enabled
     * @return true if enabled
     */
    bool isChargingEnabled() const;
    
    /**
     * @brief Get solar power in milliwatts
     * @return Solar power in mW
     */
    float getSolarPower() const;
    
    /**
     * @brief Get battery state of charge
     * @return SOC percentage (0-100)
     */
    float getBatterySOC() const;
    
    /**
     * @brief Get battery voltage
     * @return Battery voltage in V
     */
    float getBatteryVoltage() const;
    
    /**
     * @brief Check if solar panel is producing power
     * @return true if solar power available
     */
    bool isSolarAvailable() const;
    
    /**
     * @brief Check if battery is charging
     * @return true if charging
     */
    bool isCharging() const;
    
    /**
     * @brief Get daily energy harvest
     * @return Energy in Wh
     */
    float getDailyEnergyHarvest() const;
    
    /**
     * @brief Reset daily energy counter
     */
    void resetDailyEnergy();
    
    /**
     * @brief Get total energy harvested since boot
     * @return Energy in Wh
     */
    float getTotalEnergyHarvest() const { return total_energy_harvest_; }
    
    /**
     * @brief Check if system has any fault
     * @return true if fault present
     */
    bool hasFault() const;
    
    /**
     * @brief Clear any fault conditions
     */
    void clearFault();
    
    /**
     * @brief Update weather conditions for MPPT optimization
     * @param cloud_cover Cloud cover percentage (0-100)
     * @param temperature Ambient temperature in °C
     */
    void updateWeatherConditions(int cloud_cover, float temperature);
    
    /**
     * @brief Calibrate solar voltage sensor
     * @param known_voltage Known reference voltage
     */
    void calibrateSolarVoltage(float known_voltage);
    
    /**
     * @brief Calibrate solar current sensor
     * @param known_current Known reference current
     */
    void calibrateSolarCurrent(float known_current);
    
    /**
     * @brief Calibrate battery voltage sensor
     * @param known_voltage Known reference voltage
     */
    void calibrateBatteryVoltage(float known_voltage);
    
    /**
     * @brief Get MPPT controller instance
     * @return Pointer to MPPTController
     */
    MPPTController* getMPPTController() { return mppt_controller_; }
    
    /**
     * @brief Get battery manager instance
     * @return Pointer to BatteryManager
     */
    BatteryManager* getBatteryManager() { return battery_manager_; }
    
    /**
     * @brief Print status to Serial
     */
    void printStatus() const;

private:
    // Component instances
    MPPTController* mppt_controller_;
    BatteryManager* battery_manager_;
    
    // Pin configuration
    SolarPowerPins pins_;
    
    // Operating state
    SolarPowerMode operating_mode_;
    bool initialized_;
    bool charging_enabled_;
    
    // Energy tracking
    float total_energy_harvest_;
    
    // Weather conditions
    int cloud_cover_;
    float ambient_temperature_;
    
    // Timing
    unsigned long last_update_;
    unsigned long last_mode_evaluation_;
    
    // Internal methods
    void optimizeForMode();
    void evaluateAutoMode();
    SolarPowerMode determineOptimalMode() const;
};

#endif // SOLAR_POWER_SYSTEM_H
