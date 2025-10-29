#include "PowerManager.h"
#include "config.h"

PowerManager::PowerManager()
    : batteryPin(35),
      batteryVoltage(0.0),
      lowPowerMode(false) {
}

void PowerManager::init(int batteryMonitorPin) {
    batteryPin = batteryMonitorPin;
    
    // Configure ADC for battery voltage reading
    pinMode(batteryPin, INPUT);
    
    // Configure ADC attenuation for full range
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
    
    Serial.println("Power Manager initialized");
}

float PowerManager::getBatteryVoltage() {
    int adcValue = analogRead(batteryPin);
    
    // ESP32 ADC: 0-4095 maps to 0-3.3V (with 11dB attenuation)
    // Adjust for voltage divider if present
    const float ADC_MAX = 4095.0;
    const float ADC_VREF = 3.3;
    const float VOLTAGE_DIVIDER_RATIO = 2.0;  // Adjust based on your hardware
    
    batteryVoltage = (adcValue / ADC_MAX) * ADC_VREF * VOLTAGE_DIVIDER_RATIO;
    
    Serial.printf("Battery ADC: %d, Voltage: %.2fV\n", adcValue, batteryVoltage);
    return batteryVoltage;
}

int PowerManager::getBatteryPercentage() {
    float voltage = getBatteryVoltage();
    
    // Clamp voltage to min/max range
    if (voltage < BATTERY_MIN) {
        return 0;
    }
    if (voltage > BATTERY_MAX) {
        return 100;
    }
    
    // Linear interpolation
    float percentage = ((voltage - BATTERY_MIN) / 
                       (BATTERY_MAX - BATTERY_MIN)) * 100.0;
    
    return (int)percentage;
}

bool PowerManager::isLowBattery() {
    float voltage = getBatteryVoltage();
    return voltage < LOW_BATTERY_THRESHOLD;
}

void PowerManager::enterDeepSleep(uint64_t sleepTimeSeconds) {
    Serial.printf("Entering deep sleep for %llu seconds...\n", sleepTimeSeconds);
    Serial.flush();
    
    // Configure wake up timer
    esp_sleep_enable_timer_wakeup(sleepTimeSeconds * 1000000ULL);  // Convert to microseconds
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void PowerManager::configureWakeOnMotion(int pirPin) {
    // Configure external wakeup from PIR sensor
    esp_sleep_enable_ext0_wakeup((gpio_num_t)pirPin, HIGH);
    
    Serial.printf("Configured wakeup on motion (pin %d)\n", pirPin);
}

void PowerManager::configureWakeOnTimer(uint64_t seconds) {
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Convert to microseconds
    
    Serial.printf("Configured wakeup on timer (%llu seconds)\n", seconds);
}

void PowerManager::printPowerStatus() {
    Serial.println("========== Power Status ==========");
    Serial.printf("Battery Voltage: %.2fV\n", getBatteryVoltage());
    Serial.printf("Battery Percentage: %d%%\n", getBatteryPercentage());
    Serial.printf("Low Battery: %s\n", isLowBattery() ? "YES" : "NO");
    Serial.printf("Low Power Mode: %s\n", lowPowerMode ? "ENABLED" : "DISABLED");
    Serial.printf("Battery Min: %.2fV\n", BATTERY_MIN);
    Serial.printf("Battery Max: %.2fV\n", BATTERY_MAX);
    Serial.println("==================================");
}
