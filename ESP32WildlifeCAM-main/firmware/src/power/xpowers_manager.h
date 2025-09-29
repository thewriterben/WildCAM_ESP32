/**
 * @file xpowers_manager.h
 * @brief XPowersLib Advanced Power Management for ESP32 Wildlife Camera
 * 
 * Enhanced power management using XPowersLib for precise battery monitoring,
 * solar charging optimization, and advanced power control features.
 * 
 * Phase 1 Core AI Enhancement Integration
 */

#ifndef XPOWERS_MANAGER_H
#define XPOWERS_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

// XPowersLib integration
#ifdef XPOWERS_ENABLED
#include "XPowersLib.h"
#endif

/**
 * Power Management Status
 */
enum XPowerStatus {
    XPOWER_UNKNOWN,
    XPOWER_INITIALIZING,
    XPOWER_NORMAL,
    XPOWER_LOW_BATTERY,
    XPOWER_CHARGING,
    XPOWER_FULL,
    XPOWER_ERROR
};

/**
 * Charging Mode
 */
enum ChargingMode {
    CHARGE_AUTO,        // Automatic charging based on conditions
    CHARGE_FAST,        // Fast charging mode
    CHARGE_STANDARD,    // Standard charging mode
    CHARGE_TRICKLE,     // Trickle charging mode
    CHARGE_DISABLED     // Charging disabled
};

/**
 * Advanced Power Metrics
 */
struct XPowerMetrics {
    // Battery measurements
    float batteryVoltage;           // Battery voltage (V)
    float batteryCurrent;           // Battery current (mA)
    float batteryPower;             // Battery power (mW)
    float batteryTemperature;       // Battery temperature (°C)
    uint8_t batteryPercentage;      // Battery capacity (%)
    bool batteryConnected;          // Battery connection status
    
    // Charging measurements
    float chargeCurrent;            // Charging current (mA)
    float chargeVoltage;            // Charging voltage (V)
    float chargePower;              // Charging power (mW)
    bool isCharging;                // Charging status
    
    // Solar/VBUS measurements
    float solarVoltage;             // Solar/VBUS voltage (V)
    float solarCurrent;             // Solar/VBUS current (mA)
    float solarPower;               // Solar/VBUS power (mW)
    bool solarConnected;            // Solar/VBUS connection status
    
    // System measurements
    float systemVoltage;            // System voltage (V)
    float systemCurrent;            // System current consumption (mA)
    float systemPower;              // System power consumption (mW)
    
    // Status and health
    XPowerStatus status;
    bool healthy;
    uint32_t timestamp;
    
    XPowerMetrics() : batteryVoltage(0), batteryCurrent(0), batteryPower(0), batteryTemperature(25.0f),
                     batteryPercentage(0), batteryConnected(false), chargeCurrent(0), chargeVoltage(0),
                     chargePower(0), isCharging(false), solarVoltage(0), solarCurrent(0), solarPower(0),
                     solarConnected(false), systemVoltage(0), systemCurrent(0), systemPower(0),
                     status(XPOWER_UNKNOWN), healthy(false), timestamp(0) {}
};

/**
 * Charging Configuration
 */
struct ChargingConfig {
    ChargingMode mode;
    uint16_t currentLimit;          // Charging current limit (mA)
    float voltageLimit;             // Charging voltage limit (V)
    float terminationCurrent;       // Charge termination current (mA)
    float temperatureThreshold;     // Temperature threshold for charging (°C)
    bool enabled;
    
    ChargingConfig() : mode(CHARGE_AUTO), currentLimit(500), voltageLimit(4.2f),
                      terminationCurrent(50), temperatureThreshold(45.0f), enabled(true) {}
};

/**
 * XPowersLib Advanced Power Manager
 * 
 * Provides comprehensive power management using XPowersLib for:
 * - Precise battery monitoring and health assessment
 * - Optimized solar charging with MPPT-like behavior
 * - Advanced power control and protection
 * - Temperature monitoring and thermal management
 */
class XPowersManager {
public:
    XPowersManager();
    ~XPowersManager();
    
    // Initialization and configuration
    bool init(uint8_t sdaPin = 21, uint8_t sclPin = 22);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    bool hasXPowersChip() const { return xpowersAvailable_; }
    uint8_t getChipModel() const { return chipModel_; }
    String getChipName() const;
    
    // Power measurements
    XPowerMetrics getPowerMetrics();
    void updateMeasurements();
    bool isMeasurementValid() const { return measurementValid_; }
    uint32_t getLastUpdateTime() const { return lastUpdateTime_; }
    
    // Battery management
    float getBatteryVoltage();
    float getBatteryCurrent();
    float getBatteryPower();
    float getBatteryTemperature();
    uint8_t getBatteryPercentage();
    bool isBatteryConnected();
    bool isBatteryLow();
    bool isBatteryCritical();
    void enableBatteryMonitoring(bool enable);
    
    // Charging management
    bool configureCharging(const ChargingConfig& config);
    ChargingConfig getChargingConfig() const { return chargingConfig_; }
    bool isCharging();
    float getChargingCurrent();
    float getChargingVoltage();
    void enableCharging(bool enable);
    void setChargingCurrent(uint16_t currentMA);
    void setChargingVoltage(float voltage);
    bool optimizeCharging();
    
    // Solar power management
    float getSolarVoltage();
    float getSolarCurrent();
    float getSolarPower();
    bool isSolarConnected();
    bool optimizeSolarCharging();
    void enableSolarMonitoring(bool enable);
    
    // Power optimization
    void enterLowPowerMode();
    void exitLowPowerMode();
    void enablePowerSaving(bool enable);
    void setSystemVoltage(float voltage);
    void enableBoostConverter(bool enable);
    
    // Protection and safety
    void enableOverchargeProtection(bool enable);
    void enableOverdischargeProtection(bool enable);
    void enableTemperatureProtection(bool enable);
    void setLowBatteryThreshold(float voltage);
    void setCriticalBatteryThreshold(float voltage);
    bool isOvertemperature();
    
    // Power control
    void enableOutput(bool enable);
    void enableGPIO(uint8_t gpio, bool enable);
    void enableLDO(uint8_t ldo, bool enable, float voltage = 3.3f);
    void enableDCDC(uint8_t dcdc, bool enable, float voltage = 3.3f);
    
    // Interrupts and events
    void enableInterrupts(bool enable);
    void clearInterrupts();
    uint32_t getInterruptStatus();
    bool hasLowBatteryInterrupt();
    bool hasChargingInterrupt();
    bool hasTemperatureInterrupt();
    
    // Wakeup and sleep
    void enableWakeup(bool enable);
    void setWakeupSource(uint8_t source);
    void enterSleepMode();
    void enterShutdownMode();
    
    // Calibration and diagnostics
    bool calibrateBatteryGauge();
    bool performSelfTest();
    void resetToDefaults();
    String getDiagnosticInfo();
    
    // Configuration
    void setUpdateInterval(uint32_t intervalMs) { updateInterval_ = intervalMs; }
    uint32_t getUpdateInterval() const { return updateInterval_; }
    void enableAutoOptimization(bool enable) { autoOptimization_ = enable; }
    bool isAutoOptimizationEnabled() const { return autoOptimization_; }
    
    // Callbacks
    typedef void (*PowerEventCallback)(XPowerStatus status, const XPowerMetrics& metrics);
    void setPowerEventCallback(PowerEventCallback callback) { powerEventCallback_ = callback; }
    
private:
#ifdef XPOWERS_ENABLED
    XPowersPPM* xpowers_;
#endif
    
    // State variables
    bool initialized_;
    bool xpowersAvailable_;
    uint8_t chipModel_;
    uint8_t i2cAddress_;
    
    // Configuration
    ChargingConfig chargingConfig_;
    uint32_t updateInterval_;
    bool autoOptimization_;
    bool lowPowerMode_;
    
    // Measurements
    XPowerMetrics currentMetrics_;
    XPowerMetrics previousMetrics_;
    bool measurementValid_;
    uint32_t lastUpdateTime_;
    
    // Thresholds
    float lowBatteryThreshold_;
    float criticalBatteryThreshold_;
    float temperatureThreshold_;
    
    // Callbacks
    PowerEventCallback powerEventCallback_;
    
    // Private methods
    bool detectXPowersChip();
    bool initializeChip();
    void configureDefaults();
    bool readBatteryMeasurements(XPowerMetrics& metrics);
    bool readChargingMeasurements(XPowerMetrics& metrics);
    bool readSolarMeasurements(XPowerMetrics& metrics);
    bool readSystemMeasurements(XPowerMetrics& metrics);
    void updateStatus(XPowerMetrics& metrics);
    void checkThresholds(const XPowerMetrics& metrics);
    void triggerPowerEvent(XPowerStatus status);
    
    // Optimization algorithms
    void optimizeForBatteryLife();
    void optimizeForSolarConditions();
    void adjustChargingParameters();
    
    // Constants
    static const uint8_t XPOWERS_ADDRESSES[];
    static const uint8_t NUM_ADDRESSES;
    static const uint32_t DEFAULT_UPDATE_INTERVAL = 5000; // 5 seconds
    static const float DEFAULT_LOW_BATTERY_THRESHOLD = 3.3f;
    static const float DEFAULT_CRITICAL_BATTERY_THRESHOLD = 3.0f;
    static const float DEFAULT_TEMPERATURE_THRESHOLD = 60.0f;
};

// Global instance
extern XPowersManager* g_xpowersManager;

// Utility functions
bool initializeXPowers(uint8_t sdaPin = 21, uint8_t sclPin = 22);
void cleanupXPowers();
XPowerMetrics getSystemPowerMetrics();
bool optimizeSystemPower();

#endif // XPOWERS_MANAGER_H