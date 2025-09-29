#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

// Forward declaration for advanced power optimization
class AdvancedPowerManager;

// Power state enumeration
enum PowerState {
    POWER_CRITICAL = 0,  // Battery critically low
    POWER_LOW = 1,       // Battery low
    POWER_GOOD = 2,      // Battery good
    POWER_NORMAL = 3     // Battery normal/full
};

// Power statistics structure
struct PowerStats {
    float batteryVoltage;
    float solarVoltage;
    int batteryPercentage;
    bool isCharging;
    PowerState powerState;
    unsigned long totalChargingTime;
    float estimatedRuntime;
};

// Power system status structure
struct PowerSystemStatus {
    bool initialized;
    float batteryVoltage;
    float solarVoltage;
    int batteryPercentage;
    bool isCharging;
    PowerState powerState;
    float estimatedRuntime;
    unsigned long totalChargingTime;
    bool lowPowerMode;
};

/**
 * @class PowerManager
 * @brief Manages solar power, battery monitoring, and power optimization
 * 
 * This class encapsulates all power-related functionality for the wildlife
 * monitoring system, including battery voltage monitoring, solar charging
 * management, and power state optimization.
 */
class PowerManager {
public:
    /**
     * @brief Constructor
     */
    PowerManager();

    /**
     * @brief Destructor
     */
    ~PowerManager();

    /**
     * @brief Initialize power management system
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Update power system status (call regularly)
     */
    void update();
    
    /**
     * @brief Get current battery voltage
     * @return Battery voltage in volts
     */
    float getBatteryVoltage() const;
    
    /**
     * @brief Get current solar panel voltage
     * @return Solar panel voltage in volts
     */
    float getSolarVoltage() const;
    
    /**
     * @brief Check if battery is currently charging
     * @return true if charging, false otherwise
     */
    bool isCharging() const;
    
    /**
     * @brief Get current power state
     * @return Current power state enum
     */
    PowerState getPowerState() const;
    
    /**
     * @brief Get battery level as percentage (0-100)
     * @return Battery charge percentage
     */
    int getBatteryPercentage() const;
    
    /**
     * @brief Get estimated remaining runtime in hours
     * @return Estimated runtime in hours
     */
    float getEstimatedRuntime() const;
    
    /**
     * @brief Get power system statistics
     * @return Power statistics structure
     */
    PowerStats getPowerStats() const;
    
    /**
     * @brief Check if system should enter low power mode
     * @return true if low power mode recommended
     */
    bool shouldEnterLowPower() const;
    
    /**
     * @brief Enter power saving mode
     */
    void enterPowerSaving();
    
    /**
     * @brief Exit power saving mode
     */
    void exitPowerSaving();
    
    /**
     * @brief Calibrate voltage readings with known values
     * @param actualBatteryVoltage Actual battery voltage measured externally
     * @param actualSolarVoltage Actual solar voltage measured externally
     */
    void calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage);
    
    /**
     * @brief Get detailed power system status
     * @return Power system status structure
     */
    PowerSystemStatus getSystemStatus() const;
    
    /**
     * @brief Reset power statistics
     */
    void resetStats();
    
    /**
     * @brief Cleanup power manager resources
     */
    void cleanup();

    /**
     * @brief Check if power manager is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Check if currently in low power mode
     * @return true if in low power mode, false otherwise
     */
    bool isInLowPowerMode() const { return lowPowerMode; }

    /**
     * @brief Enable advanced power optimizations
     * @return true if optimization enabled successfully
     */
    bool enableAdvancedOptimizations();

    /**
     * @brief Get power consumption optimization recommendations
     * @return recommended power savings in mA
     */
    float getOptimizationRecommendations() const;

private:
    // Member variables
    bool initialized;
    float batteryVoltage;
    float solarVoltage;
    bool chargingActive;
    unsigned long lastVoltageCheck;
    PowerState currentPowerState;
    unsigned long totalChargingTime;
    unsigned long chargingStartTime;
    bool lowPowerMode;
    float batteryCalibrationOffset;
    float solarCalibrationOffset;
    
    // Advanced optimization support
    AdvancedPowerManager* advancedOptimizer;
    bool optimizationsEnabled;
    
    // Private methods
    float readVoltage(int pin, float voltageDividerRatio = VOLTAGE_DIVIDER_RATIO);
    void updatePowerState();
    void controlCharging();
    void logPowerStatus();
    void applyConfigurationSettings();
};

// For backward compatibility with existing code
namespace SolarManager {
    extern PowerManager* instance;
    
    bool init();
    void update();
    float getBatteryVoltage();
    float getSolarVoltage();
    bool isCharging();
    PowerState getPowerState();
    int getBatteryPercentage();
    float getEstimatedRuntime();
    PowerStats getPowerStats();
    bool shouldEnterLowPower();
    void enterPowerSaving();
    void exitPowerSaving();
    void calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage);
    PowerSystemStatus getSystemStatus();
    void resetStats();
    void cleanup();
}

#endif // POWER_MANAGER_H