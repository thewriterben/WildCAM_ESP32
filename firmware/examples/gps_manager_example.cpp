/**
 * @file gps_manager_example.cpp
 * @brief Example demonstrating GPS Manager usage
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * This example shows how to:
 * - Initialize the GPS manager
 * - Wait for GPS fix
 * - Read location data
 * - Monitor GPS status
 */

#include <Arduino.h>
#include "sensors/gps_manager.h"
#include "config.h"

// Global GPS manager instance
GPSManager* gpsManager = nullptr;

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n====================================");
    Serial.println("GPS Manager Example");
    Serial.println("====================================\n");
    
    // Create and initialize GPS manager
    gpsManager = new GPSManager();
    
    if (!gpsManager->initialize(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE)) {
        Serial.println("ERROR: Failed to initialize GPS");
        return;
    }
    
    Serial.println("GPS initialized successfully!");
    Serial.printf("Pins - RX: GPIO%d, TX: GPIO%d\n", GPS_RX_PIN, GPS_TX_PIN);
    Serial.printf("Baud Rate: %d\n\n", GPS_BAUD_RATE);
    
    // Wait for GPS fix (60 second timeout)
    Serial.println("Waiting for GPS fix...");
    Serial.println("(Make sure GPS module has clear view of sky)\n");
    
    if (gpsManager->waitForFix(GPS_FIX_TIMEOUT)) {
        Serial.println("\n✓ GPS fix acquired!");
        Serial.println("\nInitial GPS Data:");
        gpsManager->printStatus();
    } else {
        Serial.println("\n✗ Failed to acquire GPS fix within timeout");
        Serial.println("GPS may still work, continuing anyway...");
    }
    
    Serial.println("\n====================================");
    Serial.println("Entering main loop - updating every second");
    Serial.println("====================================\n");
}

void loop() {
    // Update GPS data
    gpsManager->update();
    
    // Print location data every second
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 1000) {
        lastPrint = millis();
        
        Serial.println("\n--- GPS Update ---");
        
        if (gpsManager->hasFix()) {
            // GPS has fix - print current data
            Serial.printf("Fix Status: VALID (⚡ Active)\n");
            Serial.printf("Location: %.6f°, %.6f°\n", 
                         gpsManager->getLatitude(), 
                         gpsManager->getLongitude());
            Serial.printf("Altitude: %.1f m\n", gpsManager->getAltitude());
            Serial.printf("Satellites: %d\n", gpsManager->getSatelliteCount());
            Serial.printf("HDOP: %.2f\n", gpsManager->getHDOP());
            Serial.printf("Speed: %.2f km/h\n", gpsManager->getSpeedKmph());
            Serial.printf("Course: %.2f°\n", gpsManager->getCourse());
            Serial.printf("Location String: %s\n", gpsManager->getLocationString().c_str());
        } else {
            // No fix - print last known position if available
            Serial.printf("Fix Status: INVALID (⏳ Searching...)\n");
            
            if (gpsManager->getTimeSinceLastFix() > 0) {
                Serial.printf("Time since last fix: %lu ms\n", 
                             gpsManager->getTimeSinceLastFix());
                Serial.printf("Last known position: %.6f°, %.6f°\n", 
                             gpsManager->getLatitude(), 
                             gpsManager->getLongitude());
            } else {
                Serial.println("No fix ever acquired");
            }
            
            Serial.printf("Satellites visible: %d\n", gpsManager->getSatelliteCount());
        }
        
        // Print detailed status every 10 seconds
        static unsigned long lastDetailedPrint = 0;
        if (millis() - lastDetailedPrint >= 10000) {
            lastDetailedPrint = millis();
            Serial.println("\n--- Detailed GPS Status ---");
            gpsManager->printStatus();
        }
    }
    
    delay(100);  // Small delay to prevent overwhelming serial output
}
