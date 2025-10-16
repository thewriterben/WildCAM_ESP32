/**
 * @file power_management_example.cpp
 * @brief Example usage of power management stub functions
 * 
 * This example demonstrates how to use the power management functions
 * in the WildCAM ESP32 firmware.
 */

#include <Arduino.h>
#include "../src/power/power_management.h"
#include "../utils/logger.h"

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    Logger::info("=== Power Management Example ===");
    
    // Initialize power management
    Logger::info("Initializing power management...");
    bool initialized = initializePowerManagement();
    if (!initialized) {
        Logger::warning("Power management initialization returned false (stub)");
    }
    
    // Read battery voltage
    Logger::info("Reading battery voltage...");
    float batteryVoltage = getBatteryVoltage();
    Logger::info("Battery voltage: %.2f V", batteryVoltage);
    
    // Read solar voltage
    Logger::info("Reading solar voltage...");
    float solarVoltage = getSolarVoltage();
    Logger::info("Solar voltage: %.2f V", solarVoltage);
    
    // Check charging status
    Logger::info("Checking charging status...");
    bool charging = isCharging();
    Logger::info("Charging: %s", charging ? "Yes" : "No");
    
    // Enable power saving mode
    Logger::info("Enabling power saving mode...");
    enablePowerSaving();
    
    Logger::info("=== Example Complete ===");
}

void loop() {
    // Monitor battery status every 10 seconds
    static unsigned long lastCheck = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastCheck >= 10000) {
        Logger::info("--- Battery Status Update ---");
        
        float batteryVoltage = getBatteryVoltage();
        Logger::info("Battery: %.2f V", batteryVoltage);
        
        float solarVoltage = getSolarVoltage();
        Logger::info("Solar: %.2f V", solarVoltage);
        
        bool charging = isCharging();
        Logger::info("Charging: %s", charging ? "Yes" : "No");
        
        lastCheck = currentTime;
    }
    
    delay(100);
}
