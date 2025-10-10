/**
 * @file power_manager.h
 * @brief Core Power Management System for ESP32 Wildlife Camera
 * @author thewriterben
 * @date 2025-01-27
 * @version 0.1.0
 * 
 * Comprehensive power management system for battery monitoring, solar charging,
 * and power optimization specifically designed for AI-Thinker ESP32-CAM constraints.
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_adc_cal.h>

/**
 * Power Management Status
 */
enum PowerStatus {
    POWER_UNKNOWN,
    POWER_NORMAL,
    POWER_LOW_BATTERY,
    POWER_CRITICAL_BATTERY,
    POWER_CHARGING,
    POWER_FULLY_CHARGED,
    POWER_ERROR
};

/**
 * Power State for system operation
 */
enum PowerState {
    STATE_NORMAL_OPERATION,
    STATE_POWER_SAVING,
    STATE_LOW_POWER,
    STATE_DEEP_SLEEP,
    STATE_EMERGENCY_SHUTDOWN
};

/**
 * Battery Health Status
 */
enum BatteryHealth {
    BATTERY_HEALTHY,
    BATTERY_DEGRADED,
    BATTERY_CRITICAL,
    BATTERY_DISCONNECTED
};

/**
 * Solar Charging Status
 */
enum SolarStatus {
    SOLAR_UNKNOWN,
    SOLAR_AVAILABLE,
    SOLAR_CHARGING,
    SOLAR_INSUFFICIENT,
    SOLAR_DISCONNECTED
};

/**
 * Power Metrics Structure
 */
struct PowerMetrics {
    // Battery measurements
    float batteryVoltage;           // Battery voltage (V)
    float batteryPercentage;        // Battery capacity (%)
    BatteryHealth batteryHealth;    // Battery health status
    
    // Solar measurements  
    float solarVoltage;             // Solar panel voltage (V)
    SolarStatus solarStatus;        // Solar charging status
    bool isCharging;                // Charging active flag
    
    // System power
    PowerStatus powerStatus;        // Overall power status
    PowerState powerState;          // Current power state
    unsigned long lastUpdate;       // Last measurement timestamp
    
    // Power consumption estimates
    float estimatedRuntimeHours;    // Estimated runtime remaining
    bool lowPowerWarning;           // Low power warning flag
    bool criticalPowerWarning;      // Critical power warning flag
    
    PowerMetrics() : batteryVoltage(0.0f), batteryPercentage(0), batteryHealth(BATTERY_DISCONNECTED),
                    solarVoltage(0.0f), solarStatus(SOLAR_UNKNOWN), isCharging(false),
                    powerStatus(POWER_UNKNOWN), powerState(STATE_NORMAL_OPERATION), lastUpdate(0),
                    estimatedRuntimeHours(0.0f), lowPowerWarning(false), criticalPowerWarning(false) {}
};

/**
 * Power Hardware Configuration
 * Describes the available hardware capabilities for power monitoring
 */
struct PowerHardware {
    bool hasCurrentSensor;          // INA219 or similar current sensor available
    bool hasVoltageDivider;         // Hardware voltage divider for monitoring
    bool hasSolarInput;             // Solar panel input available
    bool hasChargingController;     // Hardware charging controller present
    
    PowerHardware() : hasCurrentSensor(false), hasVoltageDivider(true),
                     hasSolarInput(false), hasChargingController(false) {}
};

/**
 * Power Management Configuration
 */
struct PowerConfig {
    // Hardware capabilities
    PowerHardware hardware;         // Hardware configuration
    
    // GPIO pins for monitoring
    uint8_t batteryPin;             // GPIO pin for battery voltage (ADC1_CH7 = GPIO 33)
    uint8_t solarPin;               // GPIO pin for solar voltage (GPIO 32, shared with PWDN)
    uint8_t chargingIndicatorPin;   // GPIO pin for charging LED
    
    // Voltage thresholds
    float lowBatteryThreshold;      // Low battery warning threshold (V)
    float criticalBatteryThreshold; // Critical battery threshold (V)
    float fullChargeVoltage;        // Full charge voltage (V)
    float minSolarVoltage;          // Minimum solar voltage for charging (V)
    
    // ADC calibration
    float batteryVoltageMultiplier; // Voltage divider multiplier for battery
    float solarVoltageMultiplier;   // Voltage divider multiplier for solar
    uint16_t adcResolution;         // ADC resolution bits
    
    // Power saving settings
    uint32_t measurementInterval;   // Measurement interval (ms)
    uint32_t lowPowerInterval;      // Low power measurement interval (ms)
    bool enableDeepSleep;           // Enable automatic deep sleep
    uint32_t deepSleepThreshold;    // Deep sleep threshold (ms)
    
    PowerConfig() : batteryPin(33), solarPin(32), chargingIndicatorPin(16),
                   lowBatteryThreshold(3.3f), criticalBatteryThreshold(3.0f),
                   fullChargeVoltage(4.2f), minSolarVoltage(4.5f),
                   batteryVoltageMultiplier(2.0f), solarVoltageMultiplier(2.0f),
                   adcResolution(12), measurementInterval(5000), lowPowerInterval(30000),
                   enableDeepSleep(true), deepSleepThreshold(600000) {} // 10 minutes
};

/**
 * Core Power Management Class
 * 
 * Handles all power-related operations including:
 * - Battery voltage monitoring and health assessment
 * - Solar panel voltage monitoring and charging detection
 * - Power state management and optimization
 * - Deep sleep coordination
 * - Low battery warning system
 * - GPIO conflict resolution for AI-Thinker ESP32-CAM
 */
class PowerManager {
public:
    PowerManager();
    ~PowerManager();
    
    // Initialization and configuration
    bool initialize(const PowerConfig& config = PowerConfig());
    void cleanup();
    bool isInitialized() const { return m_initialized; }
    
    // Power measurements
    PowerMetrics getPowerMetrics();
    void updateMeasurements();
    bool areMeasurementsValid() const { return m_measurementsValid; }
    
    // Battery management
    float getBatteryVoltage();
    float getBatteryPercentage();
    BatteryHealth getBatteryHealth();
    bool isBatteryLow();
    bool isBatteryCritical();
    float getEstimatedRuntimeHours();
    
    // Solar charging management
    float getSolarVoltage();
    SolarStatus getSolarStatus();
    bool isCharging();
    bool isSolarAvailable();
    
    // Power state management
    PowerStatus getPowerStatus();
    PowerState getPowerState();
    void setPowerState(PowerState state);
    bool enterLowPowerMode();
    bool exitLowPowerMode();
    bool enterDeepSleep(uint32_t sleepTimeMs = 0);
    
    // Power optimization
    void optimizePowerConsumption();
    void enablePowerSaving(bool enable);
    bool isPowerSavingEnabled() const { return m_powerSavingEnabled; }
    
    // Warning system
    bool hasLowPowerWarning() const { return m_currentMetrics.lowPowerWarning; }
    bool hasCriticalPowerWarning() const { return m_currentMetrics.criticalPowerWarning; }
    void clearWarnings();
    
    // Configuration
    void setConfig(const PowerConfig& config);
    PowerConfig getConfig() const { return m_config; }
    void setMeasurementInterval(uint32_t intervalMs);
    
    // Calibration
    void calibrateBatteryVoltage(float actualVoltage);
    void calibrateSolarVoltage(float actualVoltage);
    
    // System integration
    void onCameraActivation();
    void onCameraDeactivation();
    void onNetworkActivity();
    void onSensorActivity();
    
    // Emergency handling
    void handleEmergencyShutdown();
    bool isEmergencyShutdownRequired();
    
private:
    // Configuration and state
    PowerConfig m_config;
    bool m_initialized;
    bool m_measurementsValid;
    bool m_powerSavingEnabled;
    
    // ADC calibration
    esp_adc_cal_characteristics_t m_adcChars;
    bool m_adcCalibrated;
    
    // Current measurements
    PowerMetrics m_currentMetrics;
    PowerMetrics m_previousMetrics;
    unsigned long m_lastMeasurementTime;
    
    // Power state tracking
    PowerState m_targetPowerState;
    unsigned long m_stateChangeTime;
    bool m_inLowPowerMode;
    
    // Activity tracking for power optimization
    unsigned long m_lastCameraActivity;
    unsigned long m_lastNetworkActivity;
    unsigned long m_lastSensorActivity;
    
    // Private methods
    bool initializeADC();
    bool initializeGPIO();
    float readBatteryVoltage();
    float readSolarVoltage();
    float calculateBatteryPercentage(float voltage);
    BatteryHealth assessBatteryHealth(float voltage, float percentage);
    SolarStatus assessSolarStatus(float voltage);
    PowerStatus determinePowerStatus();
    
    void updatePowerState();
    void checkPowerThresholds();
    void updateWarnings();
    void logPowerMetrics();
    
    bool configureDeepSleep();
    void shutdownNonEssentialSystems();
    void restoreNormalOperation();
    
    // GPIO conflict resolution for AI-Thinker ESP32-CAM
    bool resolvePinConflicts();
    bool isSolarPinShared() const;
    void handleSharedPinAccess();
    
    // Constants
    static const float BATTERY_MIN_VOLTAGE = 2.8f;
    static const float BATTERY_MAX_VOLTAGE = 4.2f;
    static const float ADC_REFERENCE_VOLTAGE = 3.3f;
    static const uint16_t ADC_MAX_VALUE = 4095; // 12-bit ADC
    static const uint32_t MEASUREMENT_TIMEOUT_MS = 100;
    static const uint8_t VOLTAGE_SAMPLES = 10; // Number of samples for averaging
};

// Global power manager instance declaration
extern PowerManager* g_powerManager;

// Utility functions
bool initializePowerManagement(const PowerConfig& config = PowerConfig());
void cleanupPowerManagement();
PowerMetrics getSystemPowerMetrics();
bool isSystemPowerHealthy();
void logPowerStatus();

#endif // POWER_MANAGER_H