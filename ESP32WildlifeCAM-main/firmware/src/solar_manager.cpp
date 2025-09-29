/**
 * Solar Manager Module
 * 
 * Manages solar power system including battery monitoring, charging control,
 * and power optimization for the wildlife camera system.
 */

#include "solar_manager.h"
#include "config.h"
#include <Arduino.h>

namespace SolarManager {

// Static variables
static bool initialized = false;
static float batteryVoltage = 0.0;
static float solarVoltage = 0.0;
static bool chargingActive = false;
static unsigned long lastVoltageCheck = 0;
static PowerState currentPowerState = POWER_NORMAL;
static unsigned long totalChargingTime = 0;
static unsigned long chargingStartTime = 0;

// Function prototypes
static float readVoltage(int pin, float voltageDividerRatio = 2.0);
static void updatePowerState();
static void controlCharging();
static void logPowerStatus();

/**
 * Initialize solar power management system
 */
bool init() {
    DEBUG_PRINTLN("Initializing solar power management...");
    
    // Configure ADC pins for voltage monitoring
    pinMode(SOLAR_VOLTAGE_PIN, INPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGING_LED_PIN, OUTPUT);
    
    // Set ADC resolution for better accuracy
    analogReadResolution(ADC_RESOLUTION);  // Configurable ADC resolution
    
    // Initial voltage readings
    update();
    
    initialized = true;
    DEBUG_PRINTLN("Solar power management initialized");
    
    return true;
}

/**
 * Update power system status (call regularly)
 */
void update() {
    if (!initialized) return;
    
    unsigned long now = millis();
    
    // Update voltages at configured interval
    if (now - lastVoltageCheck > VOLTAGE_CHECK_INTERVAL) {
        // Read battery voltage (using configured voltage divider ratio)
        batteryVoltage = readVoltage(BATTERY_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO);
        
        // Read solar panel voltage (using configured voltage divider ratio)
        solarVoltage = readVoltage(SOLAR_VOLTAGE_PIN, VOLTAGE_DIVIDER_RATIO);
        
        // Update power state
        updatePowerState();
        
        // Control charging
        controlCharging();
        
        lastVoltageCheck = now;
        
        // Log status at configured interval
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
float getBatteryVoltage() {
    return batteryVoltage;
}

/**
 * Get current solar panel voltage
 */
float getSolarVoltage() {
    return solarVoltage;
}

/**
 * Check if battery is currently charging
 */
bool isCharging() {
    return chargingActive;
}

/**
 * Get current power state
 */
PowerState getPowerState() {
    return currentPowerState;
}

/**
 * Get battery level as percentage (0-100)
 */
int getBatteryPercentage() {
    // Li-ion battery voltage range: 2.8V (0%) to 4.2V (100%)
    const float minVoltage = 2.8;
    const float maxVoltage = 4.2;
    
    if (batteryVoltage <= minVoltage) return 0;
    if (batteryVoltage >= maxVoltage) return 100;
    
    return (int)((batteryVoltage - minVoltage) / (maxVoltage - minVoltage) * 100);
}

/**
 * Get estimated remaining runtime in hours
 */
float getEstimatedRuntime() {
    // Rough estimation based on battery voltage and average consumption
    const float avgCurrentMa = 50;  // Average current consumption in mA
    const float batteryCapacityMah = 3000;  // Battery capacity in mAh
    
    int batteryPercent = getBatteryPercentage();
    float usableCapacity = batteryCapacityMah * batteryPercent / 100.0;
    
    return usableCapacity / avgCurrentMa;
}

/**
 * Get power system statistics
 */
PowerStats getPowerStats() {
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
bool shouldEnterLowPower() {
    return (currentPowerState == POWER_LOW || currentPowerState == POWER_CRITICAL);
}

/**
 * Read voltage from ADC pin with calibration
 */
static float readVoltage(int pin, float voltageDividerRatio) {
    // Validate inputs
    if (pin < 0 || voltageDividerRatio <= 0) {
        DEBUG_PRINTF("Invalid voltage reading parameters: pin=%d, ratio=%.2f\n", 
                    pin, voltageDividerRatio);
        return 0.0;
    }
    
    // Take multiple readings for better accuracy
    const int numReadings = VOLTAGE_CALIBRATION_SAMPLES;
    long sum = 0;
    
    for (int i = 0; i < numReadings; i++) {
        int reading = analogRead(pin);
        // Validate ADC reading is within expected range
        if (reading < 0 || reading > 4095) {
            DEBUG_PRINTF("Warning: ADC reading out of range: %d\n", reading);
            reading = constrain(reading, 0, 4095);
        }
        sum += reading;
        delay(10);
    }
    
    float avgReading = sum / (float)numReadings;
    
    // Convert ADC reading to voltage
    // ESP32 ADC: 12-bit (0-4095), reference voltage configured
    float voltage = (avgReading * ADC_REFERENCE_VOLTAGE / 4095.0) * voltageDividerRatio;
    
    // Apply reasonable bounds for battery/solar voltages (0-20V max)
    voltage = constrain(voltage, 0.0, 20.0);
    
    return voltage;
}

/**
 * Update current power state based on battery voltage
 */
static void updatePowerState() {
    PowerState newState = currentPowerState;
    
    if (batteryVoltage >= 3.8) {
        newState = POWER_NORMAL;
    } else if (batteryVoltage >= 3.4) {
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
static void controlCharging() {
    bool shouldCharge = false;
    
    // Check if solar voltage is sufficient for charging
    if (solarVoltage >= SOLAR_VOLTAGE_THRESHOLD) {
        // Check if battery needs charging (not fully charged)
        if (batteryVoltage < 4.1) {  // Li-ion full charge is ~4.2V
            shouldCharge = true;
        }
    }
    
    // Update charging state
    if (shouldCharge != chargingActive) {
        chargingActive = shouldCharge;
        DEBUG_PRINTF("Charging %s (Solar: %.2fV, Battery: %.2fV)\n",
                    chargingActive ? "started" : "stopped",
                    solarVoltage, batteryVoltage);
    }
    
    // Update charging LED
    digitalWrite(CHARGING_LED_PIN, chargingActive ? HIGH : LOW);
}

/**
 * Log current power status
 */
static void logPowerStatus() {
    DEBUG_PRINTF("Power Status - Battery: %.2fV (%d%%), Solar: %.2fV, %s, State: %d\n",
                batteryVoltage, getBatteryPercentage(), solarVoltage,
                chargingActive ? "Charging" : "Discharging", currentPowerState);
    
    if (totalChargingTime > 0) {
        DEBUG_PRINTF("Total charging time: %lu minutes\n", totalChargingTime / 60000);
    }
}

/**
 * Enter power saving mode
 */
void enterPowerSaving() {
    DEBUG_PRINTLN("Entering power saving mode...");
    
    // Reduce system clock frequency
    setCpuFrequencyMhz(POWER_SAVE_CPU_FREQUENCY);  // Configurable power saving frequency
    
    // Turn off non-essential peripherals
    // This would typically include disabling WiFi, reducing camera quality, etc.
    
    DEBUG_PRINTLN("Power saving mode active");
}

/**
 * Exit power saving mode
 */
void exitPowerSaving() {
    DEBUG_PRINTLN("Exiting power saving mode...");
    
    // Restore normal system clock
    setCpuFrequencyMhz(NORMAL_CPU_FREQUENCY);
    
    // Re-enable peripherals as needed
    
    DEBUG_PRINTLN("Normal power mode restored");
}

/**
 * Calibrate voltage readings
 */
void calibrateVoltage(float actualBatteryVoltage, float actualSolarVoltage) {
    // This would store calibration factors in EEPROM/Flash
    // For now, just log the calibration values
    
    float batteryError = actualBatteryVoltage - batteryVoltage;
    float solarError = actualSolarVoltage - solarVoltage;
    
    DEBUG_PRINTF("Voltage calibration - Battery error: %.3fV, Solar error: %.3fV\n",
                batteryError, solarError);
    
    // In a real implementation, store calibration factors:
    // batteryCalibrationFactor = actualBatteryVoltage / measuredBatteryVoltage;
    // solarCalibrationFactor = actualSolarVoltage / measuredSolarVoltage;
}

/**
 * Get detailed power system status
 */
PowerSystemStatus getSystemStatus() {
    PowerSystemStatus status;
    status.initialized = initialized;
    status.batteryVoltage = batteryVoltage;
    status.solarVoltage = solarVoltage;
    status.batteryPercentage = getBatteryPercentage();
    status.isCharging = chargingActive;
    status.powerState = currentPowerState;
    status.estimatedRuntime = getEstimatedRuntime();
    status.totalChargingTime = totalChargingTime;
    status.lowPowerMode = (currentPowerState == POWER_LOW || currentPowerState == POWER_CRITICAL);
    
    return status;
}

/**
 * Reset power statistics
 */
void resetStats() {
    totalChargingTime = 0;
    chargingStartTime = 0;
    DEBUG_PRINTLN("Power statistics reset");
}

/**
 * Cleanup solar manager resources
 */
void cleanup() {
    if (initialized) {
        digitalWrite(CHARGING_LED_PIN, LOW);
        initialized = false;
        DEBUG_PRINTLN("Solar power management cleaned up");
    }
}

} // namespace SolarManager