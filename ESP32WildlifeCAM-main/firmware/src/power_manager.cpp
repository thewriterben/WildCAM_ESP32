/**
 * Power Manager Module
 * 
 * Manages solar power system including battery monitoring, charging control,
 * and power optimization for the wildlife camera system.
 */

#include "power_manager.h"
#include "config.h"
#include <Arduino.h>

// Static instance for backward compatibility
namespace SolarManager {
    PowerManager* instance = nullptr;
}

// Constructor
PowerManager::PowerManager() 
    : initialized(false), batteryVoltage(0.0), solarVoltage(0.0),
      chargingActive(false), lastVoltageCheck(0), currentPowerState(POWER_NORMAL),
      totalChargingTime(0), chargingStartTime(0), lowPowerMode(false),
      batteryCalibrationOffset(0.0), solarCalibrationOffset(0.0),
      advancedOptimizer(nullptr), optimizationsEnabled(false) {
    applyConfigurationSettings();
}

// Destructor
PowerManager::~PowerManager() {
    cleanup();
}

/**
 * Apply configuration settings from config.h
 */
void PowerManager::applyConfigurationSettings() {
    // Configuration settings are applied through config.h defines
    // This method can be extended for runtime configuration
}

/**
 * Initialize power management system
 */
bool PowerManager::init() {
    DEBUG_PRINTLN("Initializing power management system...");
    
    // Configure ADC pins for voltage monitoring
    pinMode(SOLAR_VOLTAGE_PIN, INPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGING_LED_PIN, OUTPUT);
    
    // Set ADC resolution for better accuracy
    analogReadResolution(ADC_RESOLUTION);
    
    // Initial voltage readings
    update();
    
    initialized = true;
    DEBUG_PRINTLN("Power management system initialized");
    
    return true;
}

/**
 * Update power system status (call regularly)
 */
void PowerManager::update() {
    if (!initialized) return;
    
    unsigned long now = millis();
    
    // Update voltages every configured interval
    if (now - lastVoltageCheck > VOLTAGE_CHECK_INTERVAL) {
        // Read battery voltage
        batteryVoltage = readVoltage(BATTERY_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO) + batteryCalibrationOffset;
        
        // Read solar panel voltage
        solarVoltage = readVoltage(SOLAR_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO) + solarCalibrationOffset;
        
        // Update power state
        updatePowerState();
        
        // Control charging
        controlCharging();
        
        lastVoltageCheck = now;
        
        // Log status every configured interval
        static unsigned long lastLog = 0;
        if (now - lastLog > POWER_LOG_INTERVAL) {
            logPowerStatus();
            lastLog = now;
        }
    }
    
    // Update charging time tracking
    if (chargingActive) {
        if (chargingStartTime == 0) {
            chargingStartTime = now;
        }
    } else {
        if (chargingStartTime != 0) {
            totalChargingTime += now - chargingStartTime;
            chargingStartTime = 0;
        }
    }
}

/**
 * Get current battery voltage
 */
float PowerManager::getBatteryVoltage() const {
    return batteryVoltage;
}

/**
 * Get current solar panel voltage
 */
float PowerManager::getSolarVoltage() const {
    return solarVoltage;
}

/**
 * Check if battery is currently charging
 */
bool PowerManager::isCharging() const {
    return chargingActive;
}

/**
 * Get current power state
 */
PowerState PowerManager::getPowerState() const {
    return currentPowerState;
}

/**
 * Get battery level as percentage (0-100)
 */
int PowerManager::getBatteryPercentage() const {
    // Convert voltage to percentage (simple linear approximation)
    float percentage = ((batteryVoltage - BATTERY_CRITICAL_THRESHOLD) / 
                       (BATTERY_FULL_VOLTAGE - BATTERY_CRITICAL_THRESHOLD)) * 100.0;
    
    return CLAMP((int)percentage, 0, 100);
}

/**
 * Get estimated remaining runtime in hours
 */
float PowerManager::getEstimatedRuntime() const {
    // Simple estimation based on current battery level and average consumption
    // This is a rough estimate and would need calibration based on actual usage
    int batteryPercent = getBatteryPercentage();
    
    if (batteryPercent > 80) return 48.0;  // 2 days
    if (batteryPercent > 60) return 24.0;  // 1 day
    if (batteryPercent > 40) return 12.0;  // 12 hours
    if (batteryPercent > 20) return 6.0;   // 6 hours
    if (batteryPercent > 10) return 2.0;   // 2 hours
    
    return 0.5; // 30 minutes minimum
}

/**
 * Get power system statistics
 */
PowerStats PowerManager::getPowerStats() const {
    PowerStats stats;
    stats.batteryVoltage = batteryVoltage;
    stats.solarVoltage = solarVoltage;
    stats.batteryPercentage = getBatteryPercentage();
    stats.isCharging = chargingActive;
    stats.powerState = currentPowerState;
    stats.totalChargingTime = totalChargingTime;
    stats.estimatedRuntime = getEstimatedRuntime();
    
    return stats;
}

/**
 * Check if system should enter low power mode
 */
bool PowerManager::shouldEnterLowPower() const {
    return (currentPowerState <= POWER_LOW) || 
           (batteryVoltage < BATTERY_LOW_THRESHOLD && !chargingActive);
}

/**
 * Enter power saving mode
 */
void PowerManager::enterPowerSaving() {
    if (lowPowerMode) return;
    
    DEBUG_PRINTLN("Entering power saving mode...");
    
    // Reduce system clock frequency
    setCpuFrequencyMhz(LOW_POWER_CPU_FREQ);
    
    // Turn off non-essential peripherals
    digitalWrite(CHARGING_LED_PIN, LOW);
    
    lowPowerMode = true;
    DEBUG_PRINTLN("Power saving mode active");
}

/**
 * Exit power saving mode
 */
void PowerManager::exitPowerSaving() {
    if (!lowPowerMode) return;
    
    DEBUG_PRINTLN("Exiting power saving mode...");
    
    // Restore normal system clock frequency
    setCpuFrequencyMhz(240); // Normal ESP32 frequency
    
    lowPowerMode = false;
    DEBUG_PRINTLN("Normal power mode restored");
}

/**
 * Calibrate voltage readings with known values
 */
void PowerManager::calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage) {
    float measuredBattery = readVoltage(BATTERY_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO);
    float measuredSolar = readVoltage(SOLAR_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO);
    
    batteryCalibrationOffset = actualBatteryVoltage - measuredBattery;
    solarCalibrationOffset = actualSolarVoltage - measuredSolar;
    
    DEBUG_PRINTF("Voltage calibration: Battery offset: %.3f, Solar offset: %.3f\n", 
                 batteryCalibrationOffset, solarCalibrationOffset);
}

/**
 * Get detailed power system status
 */
PowerSystemStatus PowerManager::getSystemStatus() const {
    PowerSystemStatus status;
    status.initialized = initialized;
    status.batteryVoltage = batteryVoltage;
    status.solarVoltage = solarVoltage;
    status.batteryPercentage = getBatteryPercentage();
    status.isCharging = chargingActive;
    status.powerState = currentPowerState;
    status.estimatedRuntime = getEstimatedRuntime();
    status.totalChargingTime = totalChargingTime;
    status.lowPowerMode = lowPowerMode;
    
    return status;
}

/**
 * Reset power statistics
 */
void PowerManager::resetStats() {
    totalChargingTime = 0;
    chargingStartTime = 0;
    batteryCalibrationOffset = 0.0;
    solarCalibrationOffset = 0.0;
    
    DEBUG_PRINTLN("Power statistics reset");
}

/**
 * Cleanup power manager resources
 */
void PowerManager::cleanup() {
    if (initialized) {
        digitalWrite(CHARGING_LED_PIN, LOW);
        exitPowerSaving();
        initialized = false;
        DEBUG_PRINTLN("Power manager deinitialized");
    }
}

/**
 * Read voltage from ADC pin with calibration
 */
float PowerManager::readVoltage(int pin, float voltageDividerRatio) {
    // Take multiple readings for better accuracy
    long sum = 0;
    
    for (int i = 0; i < VOLTAGE_READINGS_COUNT; i++) {
        sum += analogRead(pin);
        delay(VOLTAGE_READING_DELAY);
    }
    
    float avgReading = sum / (float)VOLTAGE_READINGS_COUNT;
    
    // Convert ADC reading to voltage
    float voltage = (avgReading * ADC_REFERENCE_VOLTAGE / ((1 << ADC_RESOLUTION) - 1)) * voltageDividerRatio;
    
    return voltage;
}

/**
 * Update current power state based on battery voltage
 */
void PowerManager::updatePowerState() {
    PowerState newState = currentPowerState;
    
    if (batteryVoltage >= BATTERY_NORMAL_VOLTAGE) {
        newState = POWER_NORMAL;
    } else if (batteryVoltage >= BATTERY_GOOD_VOLTAGE) {
        newState = POWER_GOOD;
    } else if (batteryVoltage >= BATTERY_LOW_THRESHOLD) {
        newState = POWER_LOW;
    } else {
        newState = POWER_CRITICAL;
    }
    
    if (newState != currentPowerState) {
        DEBUG_PRINTF("Power state changed: %d -> %d (%.2fV)\n", 
                    currentPowerState, newState, batteryVoltage);
        currentPowerState = newState;
    }
}

/**
 * Control charging based on solar and battery conditions
 */
void PowerManager::controlCharging() {
    bool shouldCharge = (solarVoltage >= SOLAR_CHARGING_VOLTAGE_MIN) && 
                       (batteryVoltage < BATTERY_FULL_VOLTAGE);
    
    if (shouldCharge != chargingActive) {
        chargingActive = shouldCharge;
        digitalWrite(CHARGING_LED_PIN, chargingActive ? HIGH : LOW);
        
        DEBUG_PRINTF("Charging %s (Solar: %.2fV, Battery: %.2fV)\n", 
                    chargingActive ? "started" : "stopped", solarVoltage, batteryVoltage);
    }
}

/**
 * Log current power status
 */
void PowerManager::logPowerStatus() {
    DEBUG_PRINTF("Power Status - Battery: %.2fV (%d%%), Solar: %.2fV, %s, State: %d\n",
                batteryVoltage, getBatteryPercentage(), solarVoltage,
                chargingActive ? "Charging" : "Discharging", currentPowerState);
    
    if (totalChargingTime > 0) {
        DEBUG_PRINTF("Total charging time: %lu minutes\n", totalChargingTime / 60000);
    }
    
    if (lowPowerMode) {
        DEBUG_PRINTLN("System in low power mode");
    }
}

// Backward compatibility namespace implementation
namespace SolarManager {
    bool init() {
        if (!instance) {
            instance = new PowerManager();
        }
        return instance->init();
    }
    
    void update() {
        if (instance) instance->update();
    }
    
    float getBatteryVoltage() {
        return instance ? instance->getBatteryVoltage() : 0.0;
    }
    
    float getSolarVoltage() {
        return instance ? instance->getSolarVoltage() : 0.0;
    }
    
    bool isCharging() {
        return instance ? instance->isCharging() : false;
    }
    
    PowerState getPowerState() {
        return instance ? instance->getPowerState() : POWER_CRITICAL;
    }
    
    int getBatteryPercentage() {
        return instance ? instance->getBatteryPercentage() : 0;
    }
    
    float getEstimatedRuntime() {
        return instance ? instance->getEstimatedRuntime() : 0.0;
    }
    
    PowerStats getPowerStats() {
        if (instance) {
            return instance->getPowerStats();
        }
        PowerStats empty = {};
        return empty;
    }
    
    bool shouldEnterLowPower() {
        return instance ? instance->shouldEnterLowPower() : true;
    }
    
    void enterPowerSaving() {
        if (instance) instance->enterPowerSaving();
    }
    
    void exitPowerSaving() {
        if (instance) instance->exitPowerSaving();
    }
    
    void calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage) {
        if (instance) instance->calibrateVoltage(actualBatteryVoltage, actualSolarVoltage);
    }
    
    PowerSystemStatus getSystemStatus() {
        if (instance) {
            return instance->getSystemStatus();
        }
        PowerSystemStatus empty = {};
        return empty;
    }
    
    void resetStats() {
        if (instance) instance->resetStats();
    }
    
    void cleanup() {
        if (instance) {
            instance->cleanup();
            delete instance;
            instance = nullptr;
        }
    }
}

// Advanced optimization methods implementation
bool PowerManager::enableAdvancedOptimizations() {
    if (optimizationsEnabled) return true;
    
    #ifdef OPTIMIZED_HEAP_SIZE
    // Only enable if optimizations are compiled in
    advancedOptimizer = nullptr; // Would create AdvancedPowerManager instance
    optimizationsEnabled = true;
    
    DEBUG_PRINTLN("Advanced power optimizations enabled");
    return true;
    #else
    DEBUG_PRINTLN("Advanced optimizations not available - compile with optimization flags");
    return false;
    #endif
}

float PowerManager::getOptimizationRecommendations() const {
    if (!optimizationsEnabled) return 0.0f;
    
    float potentialSavings = 0.0f;
    
    // Calculate potential power savings based on current state
    if (currentPowerState == POWER_LOW || currentPowerState == POWER_CRITICAL) {
        potentialSavings += 50.0f; // 50mA potential savings in low power mode
    }
    
    if (!chargingActive && batteryVoltage < 3.5f) {
        potentialSavings += 30.0f; // 30mA additional savings when not charging
    }
    
    return potentialSavings;
}