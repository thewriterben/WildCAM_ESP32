/**
 * @file battery_manager.h
 * @brief Advanced battery management system for solar-powered wildlife camera
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 3.0.0
 * 
 * Features:
 * - Multi-stage charging (Bulk, Absorption, Float)
 * - Battery health monitoring and state of charge estimation
 * - Temperature-compensated charging
 * - Over-voltage, under-voltage, and over-current protection
 * - Cell balancing support (for multi-cell batteries)
 */

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>

/**
 * @brief Battery chemistry types
 */
enum class BatteryChemistry {
    LITHIUM_ION,        // Li-ion (3.7V nominal)
    LITHIUM_POLYMER,    // LiPo (3.7V nominal)
    LIFEPO4,            // LiFePO4 (3.2V nominal)
    LEAD_ACID,          // Lead-acid (12V nominal)
    NIMH                // NiMH (1.2V per cell)
};

/**
 * @brief Charging stage enumeration
 */
enum class ChargingStage {
    IDLE,               // Not charging
    PRECHARGE,          // Pre-conditioning deeply discharged battery
    BULK,               // Constant current charging
    ABSORPTION,         // Constant voltage, decreasing current
    FLOAT,              // Maintenance charging
    EQUALIZATION,       // Cell balancing (for lead-acid)
    FAULT               // Charging fault detected
};

/**
 * @brief Battery health status
 */
enum class BatteryHealth {
    EXCELLENT,          // > 90% capacity
    GOOD,               // 70-90% capacity
    FAIR,               // 50-70% capacity
    POOR,               // 30-50% capacity
    REPLACE,            // < 30% capacity
    UNKNOWN             // Not yet determined
};

/**
 * @brief Battery fault types
 */
enum class BatteryFault {
    NONE,               // No fault
    OVER_VOLTAGE,       // Voltage too high
    UNDER_VOLTAGE,      // Voltage too low
    OVER_CURRENT,       // Current too high
    OVER_TEMPERATURE,   // Temperature too high
    UNDER_TEMPERATURE,  // Temperature too low
    CELL_IMBALANCE,     // Cell voltage mismatch
    COMMUNICATION       // Sensor communication error
};

/**
 * @brief Battery status structure
 */
struct BatteryStatus {
    float voltage;                  // Current voltage (V)
    float current;                  // Current flow (mA, positive = charging)
    float temperature;              // Battery temperature (°C)
    float state_of_charge;          // State of charge (0-100%)
    float state_of_health;          // State of health (0-100%)
    float capacity_remaining;       // Remaining capacity (mAh)
    float time_to_full;             // Estimated time to full charge (minutes)
    float time_to_empty;            // Estimated time to empty (minutes)
    int cycle_count;                // Charge cycle count
    ChargingStage charging_stage;   // Current charging stage
    BatteryHealth health;           // Overall health assessment
    BatteryFault fault;             // Current fault, if any
    bool is_charging;               // Whether battery is charging
    bool is_balancing;              // Whether cells are being balanced
};

/**
 * @brief Battery configuration structure
 */
struct BatteryConfig {
    BatteryChemistry chemistry;     // Battery chemistry type
    int cell_count;                 // Number of cells in series
    float nominal_capacity;         // Nominal capacity (mAh)
    float charge_voltage;           // Target charge voltage per cell (V)
    float float_voltage;            // Float charge voltage per cell (V)
    float discharge_cutoff;         // Low voltage cutoff per cell (V)
    float max_charge_current;       // Maximum charge current (mA)
    float max_discharge_current;    // Maximum discharge current (mA)
    float precharge_threshold;      // Voltage below which precharge is needed (V)
    float precharge_current;        // Precharge current (mA)
    float absorption_current_threshold; // Current below which to enter float (mA)
    float temp_compensation;        // Temperature compensation (mV/°C)
    float max_temperature;          // Maximum safe temperature (°C)
    float min_temperature;          // Minimum charging temperature (°C)
};

/**
 * @brief Advanced battery management class
 */
class BatteryManager {
public:
    /**
     * @brief Constructor with pin configuration
     * @param voltage_pin ADC pin for voltage measurement
     * @param current_pin ADC pin for current measurement (or -1 if not available)
     * @param temp_pin ADC pin for temperature sensor (or -1 if not available)
     * @param charge_enable_pin GPIO pin to enable/disable charging
     */
    BatteryManager(int voltage_pin, int current_pin = -1, 
                   int temp_pin = -1, int charge_enable_pin = -1);
    
    /**
     * @brief Initialize battery manager
     * @param config Battery configuration
     * @return true if initialization successful
     */
    bool begin(const BatteryConfig& config);
    
    /**
     * @brief Initialize with default Li-ion configuration
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Main update loop - call regularly (every 100ms recommended)
     */
    void update();
    
    /**
     * @brief Get current battery status
     * @return BatteryStatus structure with all readings
     */
    BatteryStatus getStatus() const;
    
    /**
     * @brief Get current state of charge
     * @return State of charge percentage (0-100)
     */
    float getStateOfCharge() const;
    
    /**
     * @brief Get current battery voltage
     * @return Voltage in volts
     */
    float getVoltage() const;
    
    /**
     * @brief Get current charging current
     * @return Current in mA (positive = charging)
     */
    float getCurrent() const;
    
    /**
     * @brief Get battery temperature
     * @return Temperature in °C
     */
    float getTemperature() const;
    
    /**
     * @brief Get current charging stage
     * @return ChargingStage enum value
     */
    ChargingStage getChargingStage() const { return charging_stage_; }
    
    /**
     * @brief Check if battery is currently charging
     * @return true if charging
     */
    bool isCharging() const;
    
    /**
     * @brief Check if battery has any active fault
     * @return true if fault detected
     */
    bool hasFault() const { return current_fault_ != BatteryFault::NONE; }
    
    /**
     * @brief Get current fault type
     * @return BatteryFault enum value
     */
    BatteryFault getFault() const { return current_fault_; }
    
    /**
     * @brief Clear fault condition (after addressing cause)
     */
    void clearFault();
    
    /**
     * @brief Enable or disable charging
     * @param enable true to enable charging
     */
    void setChargingEnabled(bool enable);
    
    /**
     * @brief Check if charging is enabled
     * @return true if charging is enabled
     */
    bool isChargingEnabled() const { return charging_enabled_; }
    
    /**
     * @brief Set maximum charge current
     * @param current_ma Maximum charge current in mA
     */
    void setMaxChargeCurrent(float current_ma);
    
    /**
     * @brief Get estimated time to full charge
     * @return Time in minutes, or -1 if not charging
     */
    float getTimeToFull() const;
    
    /**
     * @brief Get estimated time to empty
     * @return Time in minutes at current discharge rate
     */
    float getTimeToEmpty() const;
    
    /**
     * @brief Get battery health assessment
     * @return BatteryHealth enum value
     */
    BatteryHealth getHealth() const { return battery_health_; }
    
    /**
     * @brief Get estimated state of health percentage
     * @return State of health (0-100%)
     */
    float getStateOfHealth() const { return state_of_health_; }
    
    /**
     * @brief Get charge cycle count
     * @return Number of full charge cycles
     */
    int getCycleCount() const { return cycle_count_; }
    
    /**
     * @brief Calibrate voltage measurement
     * @param known_voltage Known reference voltage
     */
    void calibrateVoltage(float known_voltage);
    
    /**
     * @brief Calibrate current measurement
     * @param known_current Known reference current
     */
    void calibrateCurrent(float known_current);
    
    /**
     * @brief Start cell balancing (if supported)
     */
    void startBalancing();
    
    /**
     * @brief Stop cell balancing
     */
    void stopBalancing();
    
    /**
     * @brief Set configuration
     * @param config New battery configuration
     */
    void setConfiguration(const BatteryConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current BatteryConfig
     */
    BatteryConfig getConfiguration() const { return config_; }
    
    /**
     * @brief Get default configuration for battery chemistry
     * @param chemistry Battery chemistry type
     * @return Default BatteryConfig for the chemistry
     */
    static BatteryConfig getDefaultConfig(BatteryChemistry chemistry);

private:
    // Pin configuration
    int voltage_pin_;
    int current_pin_;
    int temp_pin_;
    int charge_enable_pin_;
    
    // Battery configuration
    BatteryConfig config_;
    
    // Current state
    float voltage_;
    float current_;
    float temperature_;
    float state_of_charge_;
    float state_of_health_;
    ChargingStage charging_stage_;
    BatteryHealth battery_health_;
    BatteryFault current_fault_;
    
    // Charging control
    bool charging_enabled_;
    bool balancing_active_;
    float target_charge_current_;
    
    // Calibration
    float voltage_calibration_;
    float current_calibration_;
    float current_offset_;
    
    // State tracking
    int cycle_count_;
    float coulomb_counter_;
    float capacity_used_;
    unsigned long last_update_;
    unsigned long stage_start_time_;
    unsigned long last_full_charge_time_;
    
    // Averaging buffers
    static const int AVG_BUFFER_SIZE = 16;
    float voltage_buffer_[AVG_BUFFER_SIZE];
    float current_buffer_[AVG_BUFFER_SIZE];
    int buffer_index_;
    
    // Internal methods
    float readVoltage();
    float readCurrent();
    float readTemperature();
    float calculateSOC();
    void updateChargingStage();
    void checkProtection();
    BatteryHealth assessHealth();
    void updateCoulombCounter(float current, float dt);
    float getTemperatureCompensatedVoltage(float base_voltage);
};

#endif // BATTERY_MANAGER_H
