#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>

class PowerManager {
private:
    int batteryPin;
    float batteryVoltageMin;
    float batteryVoltageMax;
    float lowBatteryThreshold;
    unsigned long deepSleepDuration;
    
    // Calculate voltage from ADC reading
    float calculateVoltage(int adcValue);

public:
    PowerManager(int pin = 33);
    
    // Initialize power management
    bool begin();
    
    // Read battery voltage
    float getBatteryVoltage();
    
    // Get battery percentage (0-100)
    int getBatteryPercentage();
    
    // Check if battery is low
    bool isBatteryLow();
    
    // Enter deep sleep mode
    void enterDeepSleep(unsigned long seconds = 60);
    
    // Configure wakeup sources
    void configureWakeup(int pirPin);
    
    // Set low battery threshold
    void setLowBatteryThreshold(float voltage);
    
    // Get power consumption estimate
    float getEstimatedPowerConsumption();
    
    // Enable/disable peripheral power
    void setPeripheralPower(bool enable);
};

#endif // POWER_MANAGER_H
