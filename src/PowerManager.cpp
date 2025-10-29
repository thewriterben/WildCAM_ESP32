#include "PowerManager.h"
#include "config.h"

PowerManager::PowerManager(int pin)
    : batteryPin(pin),
      batteryVoltageMin(BATTERY_VOLTAGE_MIN),
      batteryVoltageMax(BATTERY_VOLTAGE_MAX),
      lowBatteryThreshold(LOW_BATTERY_THRESHOLD),
      deepSleepDuration(DEEP_SLEEP_DURATION) {
}

bool PowerManager::begin() {
    // Configure ADC for battery voltage reading
    pinMode(batteryPin, INPUT);
    
    // Configure ADC attenuation for full range
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
    
    Serial.println("Power Manager initialized");
    return true;
}

float PowerManager::calculateVoltage(int adcValue) {
    // ESP32 ADC: 0-4095 maps to 0-3.3V (with 11dB attenuation)
    // Adjust for voltage divider if present
    const float ADC_MAX = 4095.0;
    const float ADC_VREF = 3.3;
    const float VOLTAGE_DIVIDER_RATIO = 2.0;  // Adjust based on your hardware
    
    float voltage = (adcValue / ADC_MAX) * ADC_VREF * VOLTAGE_DIVIDER_RATIO;
    return voltage;
}

float PowerManager::getBatteryVoltage() {
    int adcValue = analogRead(batteryPin);
    float voltage = calculateVoltage(adcValue);
    
    Serial.printf("Battery ADC: %d, Voltage: %.2fV\n", adcValue, voltage);
    return voltage;
}

int PowerManager::getBatteryPercentage() {
    float voltage = getBatteryVoltage();
    
    // Clamp voltage to min/max range
    if (voltage < batteryVoltageMin) {
        return 0;
    }
    if (voltage > batteryVoltageMax) {
        return 100;
    }
    
    // Linear interpolation
    float percentage = ((voltage - batteryVoltageMin) / 
                       (batteryVoltageMax - batteryVoltageMin)) * 100.0;
    
    return (int)percentage;
}

bool PowerManager::isBatteryLow() {
    float voltage = getBatteryVoltage();
    return voltage < lowBatteryThreshold;
}

void PowerManager::enterDeepSleep(unsigned long seconds) {
    Serial.printf("Entering deep sleep for %lu seconds...\n", seconds);
    Serial.flush();
    
    // Configure wake up timer
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Convert to microseconds
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void PowerManager::configureWakeup(int pirPin) {
    // Configure external wakeup from PIR sensor
    esp_sleep_enable_ext0_wakeup((gpio_num_t)pirPin, HIGH);
    
    Serial.printf("Configured wakeup on pin %d\n", pirPin);
}

void PowerManager::setLowBatteryThreshold(float voltage) {
    lowBatteryThreshold = voltage;
    Serial.printf("Low battery threshold set to: %.2fV\n", voltage);
}

float PowerManager::getEstimatedPowerConsumption() {
    // Estimated power consumption in mA
    // This is a simplified estimation
    float consumption = 0.0;
    
    // Base ESP32 consumption: ~80mA active, ~0.01mA deep sleep
    consumption += 80.0;  // Active mode
    
    // Camera: ~100-150mA when active
    // Add other peripherals as needed
    
    return consumption;
}

void PowerManager::setPeripheralPower(bool enable) {
    // Control power to peripherals if hardware supports it
    // This is a placeholder for future hardware implementations
    Serial.printf("Peripheral power: %s\n", enable ? "ON" : "OFF");
}
