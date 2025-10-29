#include "power_manager.h"
#include <esp_sleep.h>
#include <esp_pm.h>
#include <driver/rtc_io.h>
#include <driver/adc.h>

bool PowerManager::initialize() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing power management...");

    // Configure ADC for voltage monitoring
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(SOLAR_VOLTAGE_PIN, INPUT);
    
    // Configure charging control pin
    pinMode(CHARGING_CONTROL_PIN, OUTPUT);
    digitalWrite(CHARGING_CONTROL_PIN, LOW);
    
    // Configure LED pins
    pinMode(POWER_LED_PIN, OUTPUT);
    pinMode(CHARGING_LED_PIN, OUTPUT);
    
    // Set ADC resolution
    analogReadResolution(ADC_RESOLUTION);
    
    // Initialize configuration
    initializeDefaultConfig();
    
    // Record boot time
    bootTime = millis();
    lastUpdateTime = bootTime;
    lastBatteryCheck = bootTime;
    
    // Initial battery reading
    update();
    
    initialized = true;
    Serial.println("Power management initialized successfully");
    Serial.printf("Initial battery: %.2fV (%.1f%%)\n", batteryVoltage, batteryPercentage);
    
    return true;
}

void PowerManager::update() {
    if (!initialized) {
        return;
    }

    uint32_t now = millis();
    
    // Update battery readings every 5 seconds
    if (now - lastBatteryCheck >= 5000) {
        batteryVoltage = readBatteryVoltage();
        batteryPercentage = voltageToPercentage(batteryVoltage);
        solarVoltage = readSolarVoltage();
        chargingState = isCharging();
        
        lastBatteryCheck = now;
        
        // Update power state
        updatePowerState();
    }
    
    // Update statistics
    updateStatistics();
    
    // Apply adaptive power management if enabled
    if (powerSavingEnabled) {
        PowerMode optimalMode = calculateOptimalPowerMode();
        if (optimalMode != currentMode) {
            setPowerMode(optimalMode);
        }
    }
    
    lastUpdateTime = now;
}

float PowerManager::getBatteryVoltage() {
    batteryVoltage = readBatteryVoltage();
    return batteryVoltage;
}

float PowerManager::getBatteryPercentage() {
    // Use cached voltage instead of reading again (getBatteryVoltage already updates it)
    // This avoids redundant ADC read
    if (batteryVoltage == 0.0f) {
        // Only read if not yet initialized
        batteryVoltage = readBatteryVoltage();
    }
    batteryPercentage = voltageToPercentage(batteryVoltage);
    return batteryPercentage;
}

void PowerManager::setPowerMode(PowerMode mode) {
    if (currentMode == mode) {
        return;
    }

    currentMode = mode;
    applyPowerMode(mode);
    
    // Use const char* instead of String to avoid heap allocation
    const char* modeStr;
    switch (mode) {
        case PowerMode::MAX_PERFORMANCE: modeStr = "Max Performance"; break;
        case PowerMode::BALANCED: modeStr = "Balanced"; break;
        case PowerMode::ECO_MODE: modeStr = "Eco Mode"; break;
        case PowerMode::SURVIVAL: modeStr = "Survival"; break;
        case PowerMode::HIBERNATION: modeStr = "Hibernation"; break;
        default: modeStr = "Unknown"; break;
    }
    
    Serial.printf("Power mode changed to: %s\n", modeStr);
}

bool PowerManager::shouldEnterDeepSleep() {
    if (!initialized || !powerSavingEnabled) {
        return false;
    }

    // Enter deep sleep in critical battery state
    if (currentState == PowerState::CRITICAL) {
        return true;
    }
    
    // Enter deep sleep in survival mode
    if (currentMode == PowerMode::SURVIVAL && batteryPercentage < 20.0f) {
        return true;
    }
    
    // Adaptive duty cycling
    if (adaptiveDutyCycleEnabled && batteryPercentage < dutyCycleBatteryThreshold) {
        return true;
    }
    
    return false;
}

void PowerManager::prepareForDeepSleep(uint32_t sleepDuration) {
    Serial.println("Preparing for deep sleep...");
    
    // Configure GPIO for low power
    configureGpioForLowPower();
    
    // Disable unused peripherals
    disableUnusedPeripherals();
    
    // Update statistics
    updateStatistics();
    
    // Calculate sleep duration if not provided
    if (sleepDuration == 0) {
        sleepDuration = getOptimalSleepDuration();
    }
    
    Serial.printf("Entering deep sleep for %d seconds\n", sleepDuration / 1000);
}

void PowerManager::enterDeepSleep(uint32_t sleepDuration) {
    prepareForDeepSleep(sleepDuration);
    
    // Configure wake-up sources
    if (sleepDuration > 0) {
        esp_sleep_enable_timer_wakeup(sleepDuration * 1000); // Convert to microseconds
    }
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void PowerManager::configureAdaptiveDutyCycle(bool enable, float batteryThreshold) {
    adaptiveDutyCycleEnabled = enable;
    dutyCycleBatteryThreshold = batteryThreshold;
    
    Serial.printf("Adaptive duty cycle %s (threshold: %.1f%%)\n", 
                 enable ? "enabled" : "disabled", batteryThreshold);
}

uint32_t PowerManager::getOptimalSleepDuration() {
    uint32_t baseSleep = config.deepSleepDuration;
    
    if (batteryPercentage < 10.0f) {
        return baseSleep * 5; // 5x longer sleep in critical state
    } else if (batteryPercentage < 25.0f) {
        return baseSleep * 3; // 3x longer sleep in low battery
    } else if (batteryPercentage < 50.0f) {
        return baseSleep * 2; // 2x longer sleep in moderate battery
    }
    
    return baseSleep;
}

void PowerManager::setCpuFrequency(uint8_t frequencyMHz) {
    setCpuFrequencyMhz(frequencyMHz);
    Serial.printf("CPU frequency set to: %dMHz\n", frequencyMHz);
}

uint8_t PowerManager::getCpuFrequency() {
    return getCpuFrequencyMhz();
}

PowerManager::PowerStats PowerManager::getStatistics() {
    stats.batteryVoltage = batteryVoltage;
    stats.batteryPercentage = batteryPercentage;
    stats.solarVoltage = solarVoltage;
    stats.isCharging = chargingState;
    stats.currentState = currentState;
    stats.currentMode = currentMode;
    stats.uptime = millis() - bootTime;
    
    return stats;
}

void PowerManager::setConfiguration(const PowerConfig& newConfig) {
    config = newConfig;
    Serial.println("Power configuration updated");
}

void PowerManager::resetStatistics() {
    stats = {};
    bootTime = millis();
    totalSleepTime = 0;
    Serial.println("Power statistics reset");
}

bool PowerManager::isCharging() {
    // Check if solar voltage is sufficient and battery is not full
    return (solarVoltage >= SOLAR_VOLTAGE_THRESHOLD) && 
           (batteryVoltage < BATTERY_FULL_VOLTAGE);
}

void PowerManager::setPowerSavingEnabled(bool enable) {
    powerSavingEnabled = enable;
    
    if (enable) {
        // Apply current optimal power mode
        setPowerMode(calculateOptimalPowerMode());
    } else {
        // Set to balanced mode when power saving disabled
        setPowerMode(PowerMode::BALANCED);
    }
    
    Serial.printf("Power saving %s\n", enable ? "enabled" : "disabled");
}

void PowerManager::cleanup() {
    if (initialized) {
        // Turn off LEDs
        digitalWrite(POWER_LED_PIN, LOW);
        digitalWrite(CHARGING_LED_PIN, LOW);
        
        initialized = false;
        Serial.println("Power management cleaned up");
    }
}

float PowerManager::readBatteryVoltage() {
    // Read ADC value and convert to voltage
    int adcValue = analogRead(BATTERY_VOLTAGE_PIN);
    float voltage = (adcValue * ADC_VREF) / (1 << ADC_RESOLUTION);
    
    // Apply voltage divider ratio
    voltage *= BATTERY_VOLTAGE_DIVIDER;
    
    return voltage;
}

float PowerManager::readSolarVoltage() {
    // Read ADC value and convert to voltage
    int adcValue = analogRead(SOLAR_VOLTAGE_PIN);
    float voltage = (adcValue * ADC_VREF) / (1 << ADC_RESOLUTION);
    
    // Apply voltage divider ratio
    voltage *= SOLAR_VOLTAGE_DIVIDER;
    
    return voltage;
}

float PowerManager::voltageToPercentage(float voltage) {
    // Li-ion battery voltage curve (simplified)
    if (voltage >= 4.1f) return 100.0f;
    if (voltage >= 3.9f) return 80.0f + (voltage - 3.9f) * 100.0f;
    if (voltage >= 3.7f) return 40.0f + (voltage - 3.7f) * 200.0f;
    if (voltage >= 3.4f) return 10.0f + (voltage - 3.4f) * 100.0f;
    if (voltage >= 3.0f) return (voltage - 3.0f) * 25.0f;
    return 0.0f;
}

void PowerManager::updatePowerState() {
    PowerState newState = currentState;
    
    if (chargingState) {
        newState = PowerState::CHARGING;
    } else if (batteryPercentage <= (config.criticalBatteryThreshold * 100.0f)) {
        newState = PowerState::CRITICAL;
    } else if (batteryPercentage <= (config.lowBatteryThreshold * 100.0f)) {
        newState = PowerState::LOW_BATTERY;
    } else if (powerSavingEnabled && batteryPercentage < 50.0f) {
        newState = PowerState::POWER_SAVE;
    } else {
        newState = PowerState::NORMAL;
    }
    
    if (newState != currentState) {
        currentState = newState;
        
        // Update LED indicators
        digitalWrite(POWER_LED_PIN, currentState == PowerState::NORMAL ? HIGH : LOW);
        digitalWrite(CHARGING_LED_PIN, currentState == PowerState::CHARGING ? HIGH : LOW);
        
        String stateStr;
        switch (currentState) {
            case PowerState::NORMAL: stateStr = "Normal"; break;
            case PowerState::POWER_SAVE: stateStr = "Power Save"; break;
            case PowerState::LOW_BATTERY: stateStr = "Low Battery"; break;
            case PowerState::CRITICAL: stateStr = "Critical"; break;
            case PowerState::CHARGING: stateStr = "Charging"; break;
        }
        
        Serial.printf("Power state changed to: %s (%.1f%%)\n", stateStr.c_str(), batteryPercentage);
    }
}

void PowerManager::applyPowerMode(PowerMode mode) {
    switch (mode) {
        case PowerMode::MAX_PERFORMANCE:
            setCpuFrequency(config.maxCpuFreq);
            break;
            
        case PowerMode::BALANCED:
            setCpuFrequency(BALANCED_CPU_FREQ_MHZ);
            break;
            
        case PowerMode::ECO_MODE:
            setCpuFrequency(120);
            disableUnusedPeripherals();
            break;
            
        case PowerMode::SURVIVAL:
            setCpuFrequency(config.minCpuFreq);
            disableUnusedPeripherals();
            break;
            
        case PowerMode::HIBERNATION:
            setCpuFrequency(config.minCpuFreq);
            configureGpioForLowPower();
            disableUnusedPeripherals();
            break;
    }
}

PowerManager::PowerMode PowerManager::calculateOptimalPowerMode() {
    if (currentState == PowerState::CRITICAL) {
        return PowerMode::HIBERNATION;
    } else if (currentState == PowerState::LOW_BATTERY) {
        return PowerMode::SURVIVAL;
    } else if (currentState == PowerState::POWER_SAVE) {
        return PowerMode::ECO_MODE;
    } else if (currentState == PowerState::CHARGING) {
        return PowerMode::BALANCED;
    } else {
        return PowerMode::BALANCED;
    }
}

void PowerManager::updateStatistics() {
    uint32_t now = millis();
    stats.uptime = now - bootTime;
    
    // Update sleep statistics
    stats.sleepTime = totalSleepTime;
    
    // Estimate battery life based on current consumption
    // This is a simplified calculation
    if (batteryPercentage > 0) {
        float hoursRemaining = (batteryPercentage / 100.0f) * 24.0f; // Assume 24h at 100%
        stats.estimatedBatteryLife = hoursRemaining * 3600000; // Convert to milliseconds
    }
}

void PowerManager::initializeDefaultConfig() {
    config.lowBatteryThreshold = BATTERY_LOW_THRESHOLD / BATTERY_FULL_VOLTAGE;
    config.criticalBatteryThreshold = BATTERY_CRITICAL_THRESHOLD / BATTERY_FULL_VOLTAGE;
    config.deepSleepDuration = DEEP_SLEEP_DURATION;
    config.adaptiveDutyCycle = ADAPTIVE_DUTY_CYCLE;
    config.solarChargingEnabled = true;
    config.maxCpuFreq = MAX_CPU_FREQ_MHZ;
    config.minCpuFreq = MIN_CPU_FREQ_MHZ;
}

void PowerManager::configureGpioForLowPower() {
    // Configure unused GPIO pins for minimal power consumption
    // This is a simplified implementation
    rtc_gpio_isolate(GPIO_NUM_12);
    rtc_gpio_isolate(GPIO_NUM_15);
}

void PowerManager::disableUnusedPeripherals() {
    // Disable unused peripherals to save power
    // This would include WiFi, Bluetooth, etc.
    // Implementation depends on specific requirements
}