/**
 * @file mppt_controller.h
 * @brief Maximum Power Point Tracking controller for solar energy harvesting
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

#ifndef MPPT_CONTROLLER_H
#define MPPT_CONTROLLER_H

#include <Arduino.h>

/**
 * @brief MPPT algorithm types
 */
enum class MPPTAlgorithm {
    PERTURB_OBSERVE,    // Simple P&O algorithm
    INCREMENTAL_CONDUCTANCE, // IC algorithm for better efficiency
    CONSTANT_VOLTAGE    // CV algorithm for simple implementation
};

/**
 * @brief Solar power system status
 */
struct SolarPowerStatus {
    float solar_voltage;        // Solar panel voltage (V)
    float solar_current;        // Solar panel current (mA)
    float solar_power;          // Solar panel power (mW)
    float battery_voltage;      // Battery voltage (V)
    float battery_current;      // Battery charging current (mA)
    float charge_efficiency;    // Charging efficiency (%)
    float daily_energy_harvest; // Daily energy harvested (Wh)
    bool is_charging;          // Battery charging status
    bool is_daylight;          // Daylight detection
    int cloud_cover_estimate;  // Cloud cover estimate (0-100%)
};

/**
 * @brief Weather-adaptive power management
 */
class MPPTController {
private:
    // Hardware pins
    int solar_voltage_pin_;
    int solar_current_pin_;
    int battery_voltage_pin_;
    int battery_current_pin_;
    int pwm_control_pin_;
    
    // MPPT parameters
    MPPTAlgorithm algorithm_;
    float reference_voltage_;
    float max_power_voltage_;
    float perturb_step_;
    float efficiency_threshold_;
    
    // Power tracking variables
    float previous_power_;
    float previous_voltage_;
    float previous_current_;
    bool perturb_direction_;
    
    // Weather adaptation
    float cloud_factor_;
    float temperature_coefficient_;
    unsigned long last_mppt_update_;
    unsigned long mppt_update_interval_;
    
    // Energy accounting
    float daily_energy_counter_;
    unsigned long energy_reset_time_;
    
    // ADC calibration factors
    float voltage_calibration_factor_;
    float current_calibration_factor_;
    
public:
    /**
     * @brief Initialize MPPT controller
     * @param solar_v_pin Solar voltage ADC pin
     * @param solar_c_pin Solar current ADC pin  
     * @param battery_v_pin Battery voltage ADC pin
     * @param battery_c_pin Battery current ADC pin
     * @param pwm_pin PWM control pin for charge controller
     * @param algorithm MPPT algorithm to use
     */
    MPPTController(int solar_v_pin, int solar_c_pin, int battery_v_pin, 
                   int battery_c_pin, int pwm_pin, MPPTAlgorithm algorithm = MPPTAlgorithm::PERTURB_OBSERVE);
    
    /**
     * @brief Initialize the MPPT system
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Main MPPT control loop - call regularly
     */
    void update();
    
    /**
     * @brief Get current solar power system status
     * @return SolarPowerStatus structure with current readings
     */
    SolarPowerStatus getStatus();
    
    /**
     * @brief Set weather adaptation parameters
     * @param cloud_cover Cloud cover percentage (0-100)
     * @param temperature Ambient temperature (°C)
     */
    void updateWeatherConditions(int cloud_cover, float temperature);
    
    /**
     * @brief Enable/disable weather-adaptive charging
     * @param enabled Enable weather adaptation
     */
    void setWeatherAdaptation(bool enabled) { 
        mppt_update_interval_ = enabled ? 1000 : 5000; 
    }
    
    /**
     * @brief Set MPPT algorithm
     * @param algorithm Algorithm to use
     */
    void setAlgorithm(MPPTAlgorithm algorithm) { algorithm_ = algorithm; }
    
    /**
     * @brief Calibrate voltage and current sensors
     * @param v_factor Voltage calibration factor
     * @param c_factor Current calibration factor
     */
    void calibrateSensors(float v_factor, float c_factor);
    
    /**
     * @brief Get daily energy harvest total
     * @return Daily energy in Wh
     */
    float getDailyEnergyHarvest() const { return daily_energy_counter_; }
    
    /**
     * @brief Reset daily energy counter
     */
    void resetDailyEnergy();
    
    /**
     * @brief Check if system is in optimal power point
     * @return true if at or near MPP
     */
    bool isAtMaxPowerPoint() const;
    
    /**
     * @brief Get charging efficiency
     * @return Efficiency percentage (0-100)
     */
    float getChargingEfficiency() const;
    
    /**
     * @brief Enable/disable MPPT tracking
     * @param enabled Enable MPPT
     */
    void setMPPTEnabled(bool enabled);

private:
    /**
     * @brief Read solar panel voltage
     * @return Voltage in volts
     */
    float readSolarVoltage();
    
    /**
     * @brief Read solar panel current
     * @return Current in milliamps
     */
    float readSolarCurrent();
    
    /**
     * @brief Read battery voltage
     * @return Voltage in volts
     */
    float readBatteryVoltage();
    
    /**
     * @brief Read battery current
     * @return Current in milliamps
     */
    float readBatteryCurrent();
    
    /**
     * @brief Apply PWM control signal
     * @param duty_cycle PWM duty cycle (0-255)
     */
    void setPWMDutyCycle(int duty_cycle);
    
    /**
     * @brief Execute Perturb & Observe algorithm
     */
    void executeP_O_Algorithm();
    
    /**
     * @brief Execute Incremental Conductance algorithm
     */
    void executeIC_Algorithm();
    
    /**
     * @brief Execute Constant Voltage algorithm
     */
    void executeCV_Algorithm();
    
    /**
     * @brief Update energy accounting
     * @param power Current power in watts
     */
    void updateEnergyAccounting(float power);
    
    /**
     * @brief Detect daylight conditions
     * @return true if sufficient light for charging
     */
    bool isDaylight();
    
    /**
     * @brief Estimate cloud cover from power variations
     * @return Cloud cover percentage (0-100)
     */
    int estimateCloudCover();
};

#endif // MPPT_CONTROLLER_H