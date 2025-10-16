/**
 * @file time_manager_example.cpp
 * @brief Example demonstrating the usage of time manager functions
 * 
 * This example shows how to use the time manager stub implementation
 * for the WildCAM ESP32 project.
 */

#include <Arduino.h>
#include "../src/utils/time_manager.h"
#include "../utils/logger.h"

void setup() {
    // Initialize Serial for logging
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Logger::info("Time Manager Example Starting...");
    
    // 1. Initialize the time manager
    if (initializeTimeManager()) {
        Logger::info("Time manager initialized successfully!");
    } else {
        Logger::error("Failed to initialize time manager");
        return;
    }
    
    // 2. Set timezone (e.g., UTC-5 for EST)
    setTimezone(-5);
    
    // 3. Try to sync with NTP (will return false in stub)
    if (syncWithNTP()) {
        Logger::info("NTP sync successful");
    } else {
        Logger::warning("NTP sync not available (stub implementation)");
    }
    
    // 4. Try to sync with RTC (will return false in stub)
    if (syncWithRTC()) {
        Logger::info("RTC sync successful");
    } else {
        Logger::warning("RTC sync not available (stub implementation)");
    }
    
    // 5. Get current time
    time_t currentTime = getCurrentTime();
    Logger::info("Current Unix timestamp: %ld", (long)currentTime);
    
    // 6. Get formatted time
    String formattedTime = getFormattedTime();
    Logger::info("Formatted time: %s", formattedTime.c_str());
    
    Logger::info("Time Manager Example Complete!");
}

void loop() {
    // Print time every 10 seconds
    static unsigned long lastPrint = 0;
    unsigned long now = millis();
    
    if (now - lastPrint >= 10000) {
        lastPrint = now;
        
        String currentTime = getFormattedTime();
        Logger::info("Current time: %s", currentTime.c_str());
    }
    
    delay(100);
}
