/**
 * @file logging_example.cpp
 * @brief Example demonstrating the WildCAM ESP32 logging system
 * 
 * This example shows how to use the comprehensive logging system
 * with different log levels, timestamps, and configuration options.
 */

#include <Arduino.h>
#include "config.h"
#include "Logger.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=================================");
    Serial.println("  Logging System Example");
    Serial.println("=================================\n");
    
    // Example 1: Initialize with default settings (INFO level, Serial only)
    Serial.println("Example 1: Default initialization");
    Logger::init(LOG_INFO, true, false, "/system.log");
    
    // Example 2: Log messages at different levels
    Serial.println("\nExample 2: Different log levels");
    LOG_DEBUG("This is a debug message - will be filtered out");
    LOG_INFO("This is an info message");
    LOG_WARN("This is a warning message");
    LOG_ERROR("This is an error message");
    
    delay(2000);
    
    // Example 3: Change log level to DEBUG
    Serial.println("\nExample 3: Change to DEBUG level");
    Logger::setLogLevel(LOG_DEBUG);
    LOG_DEBUG("Now debug messages are visible");
    LOG_INFO("Info messages still visible");
    
    delay(2000);
    
    // Example 4: Format strings with variables
    Serial.println("\nExample 4: Format strings");
    int sensorValue = 42;
    float temperature = 25.5;
    const char* status = "OK";
    
    LOG_INFO("Sensor reading: %d", sensorValue);
    LOG_INFO("Temperature: %.1f°C", temperature);
    LOG_INFO("Status: %s", status);
    LOG_WARN("Warning: Temperature %.1f°C exceeds threshold", temperature);
    
    delay(2000);
    
    // Example 5: Simulate a capture sequence with logging
    Serial.println("\nExample 5: Simulated capture sequence");
    
    LOG_INFO("Motion detected - starting capture sequence");
    LOG_DEBUG("Stabilizing camera for 500ms");
    delay(500);
    
    LOG_INFO("Capturing image...");
    // Simulate capture
    delay(100);
    
    bool captureSuccess = true;
    if (captureSuccess) {
        LOG_INFO("Image captured successfully (size: 45678 bytes)");
        LOG_INFO("Saving to SD card...");
        LOG_INFO("Image saved: /IMG_0001.jpg");
        LOG_DEBUG("Metadata saved");
    } else {
        LOG_ERROR("Failed to capture image");
    }
    
    delay(2000);
    
    // Example 6: Error level filtering
    Serial.println("\nExample 6: ERROR level only");
    Logger::setLogLevel(LOG_ERROR);
    
    LOG_DEBUG("Debug - filtered");
    LOG_INFO("Info - filtered");
    LOG_WARN("Warning - filtered");
    LOG_ERROR("Only errors visible at this level");
    
    delay(2000);
    
    // Example 7: Disable and re-enable serial output
    Serial.println("\nExample 7: Toggle serial output");
    Logger::setLogLevel(LOG_INFO);
    LOG_INFO("This message is visible");
    
    Serial.println("Disabling serial logging...");
    Logger::setSerialOutput(false);
    LOG_INFO("This message is NOT visible on serial");
    
    Serial.println("Re-enabling serial logging...");
    Logger::setSerialOutput(true);
    LOG_INFO("This message is visible again");
    
    delay(2000);
    
    // Example 8: Battery monitoring with logging
    Serial.println("\nExample 8: Battery monitoring simulation");
    float batteryVoltage = 3.95;
    int batteryPercent = 85;
    
    LOG_INFO("Battery check: %.2fV (%d%%)", batteryVoltage, batteryPercent);
    
    batteryVoltage = 3.4;
    batteryPercent = 35;
    LOG_WARN("Battery low: %.2fV (%d%%)", batteryVoltage, batteryPercent);
    
    batteryVoltage = 3.0;
    batteryPercent = 5;
    LOG_ERROR("Battery critical: %.2fV (%d%%)", batteryVoltage, batteryPercent);
    
    delay(2000);
    
    Serial.println("\n=================================");
    Serial.println("  Example Complete!");
    Serial.println("=================================\n");
}

void loop() {
    // Example 9: Periodic logging
    static unsigned long lastLog = 0;
    
    if (millis() - lastLog > 5000) {
        LOG_INFO("Periodic status check - uptime: %lu ms", millis());
        lastLog = millis();
    }
    
    delay(100);
}
