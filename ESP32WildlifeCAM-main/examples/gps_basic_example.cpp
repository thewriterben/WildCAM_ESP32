/**
 * @file gps_basic_example.cpp
 * @brief Basic example demonstrating GPS configuration and usage
 * 
 * This example shows how to:
 * 1. Use GPS configuration from config.h
 * 2. Initialize GPS with configured pins
 * 3. Obtain GPS fix and location data
 * 4. Use GPS power management features
 * 
 * Configuration in config.h:
 * - GPS_ENABLED: Enable/disable GPS module
 * - GPS_RX_PIN, GPS_TX_PIN: Serial communication pins
 * - GPS_EN_PIN: Optional enable/power control pin
 * - GPS_BAUD_RATE: Communication baud rate (typically 9600)
 * - GPS_FIX_TIMEOUT: Maximum time to wait for GPS fix
 */

#include "config.h"
#include "gps_handler.h"
#include <Arduino.h>

GPSHandler gps;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=================================");
    Serial.println("GPS Configuration Example");
    Serial.println("=================================");
    
    // Display GPS configuration from config.h
    #if GPS_ENABLED
    Serial.println("GPS is ENABLED");
    Serial.printf("GPS Configuration:\n");
    Serial.printf("  RX Pin: GPIO %d\n", GPS_RX_PIN);
    Serial.printf("  TX Pin: GPIO %d\n", GPS_TX_PIN);
    #ifdef GPS_EN_PIN
    Serial.printf("  EN Pin: GPIO %d\n", GPS_EN_PIN);
    #else
    Serial.println("  EN Pin: Not configured");
    #endif
    Serial.printf("  Baud Rate: %d\n", GPS_BAUD_RATE);
    Serial.printf("  Fix Timeout: %d ms\n", GPS_FIX_TIMEOUT);
    Serial.println();
    
    // Initialize GPS with configuration from config.h
    Serial.println("Initializing GPS...");
    if (gps.initialize()) {
        Serial.println("GPS initialized successfully");
        
        // Wait for GPS fix
        Serial.println("Waiting for GPS fix...");
        if (gps.waitForFix(GPS_FIX_TIMEOUT)) {
            Serial.println("GPS fix acquired!");
            
            // Get current location
            GPSLocation location = gps.getCurrentLocation();
            Serial.printf("Latitude:  %.6f\n", location.latitude);
            Serial.printf("Longitude: %.6f\n", location.longitude);
            Serial.printf("Altitude:  %.1f m\n", location.altitude);
            Serial.println();
            
            // Demonstrate location string format
            String locationStr = gps.getLocationString();
            Serial.printf("Location String: %s\n", locationStr.c_str());
            Serial.println();
            
            // Demonstrate GPS power management
            #if GPS_POWER_SAVE_ENABLED
            Serial.println("GPS Power Management:");
            Serial.println("  Entering sleep mode...");
            gps.enterSleepMode();
            delay(5000);
            
            Serial.println("  Waking up from sleep...");
            gps.exitSleepMode();
            delay(2000);
            #endif
            
            // Demonstrate movement detection
            #if GPS_MOVEMENT_DETECTION
            Serial.println("Movement Detection:");
            Serial.printf("  Threshold: %.1f meters\n", GPS_MOVEMENT_THRESHOLD);
            
            for (int i = 0; i < 5; i++) {
                delay(1000);
                gps.updateLocation();
                
                if (gps.isMoving(GPS_MOVEMENT_THRESHOLD)) {
                    Serial.printf("  [%d] Movement detected!\n", i + 1);
                } else {
                    Serial.printf("  [%d] No movement\n", i + 1);
                }
            }
            #endif
            
        } else {
            Serial.println("Failed to acquire GPS fix within timeout");
            Serial.println("Check GPS module connection and antenna");
        }
        
    } else {
        Serial.println("Failed to initialize GPS");
        Serial.println("Check GPS pin configuration and wiring:");
        Serial.printf("  RX Pin: GPIO %d should connect to GPS TX\n", GPS_RX_PIN);
        Serial.printf("  TX Pin: GPIO %d should connect to GPS RX\n", GPS_TX_PIN);
        #ifdef GPS_EN_PIN
        Serial.printf("  EN Pin: GPIO %d should connect to GPS enable\n", GPS_EN_PIN);
        #endif
    }
    
    #else
    Serial.println("GPS is DISABLED in config.h");
    Serial.println("To enable GPS:");
    Serial.println("  1. Set GPS_ENABLED to true in config.h");
    Serial.println("  2. Verify GPS pins are correctly configured for your board");
    Serial.println("  3. Recompile and upload");
    #endif
    
    Serial.println();
    Serial.println("=================================");
    Serial.println("Example Complete");
    Serial.println("=================================");
}

void loop() {
    // In a real application, you might:
    // 1. Periodically update GPS location
    // 2. Log GPS coordinates with captured images
    // 3. Check for camera movement/theft
    // 4. Use GPS time for accurate timestamps
    
    #if GPS_ENABLED && GPS_PERIODIC_UPDATES
    static unsigned long lastUpdate = 0;
    
    if (millis() - lastUpdate >= GPS_UPDATE_PERIOD) {
        lastUpdate = millis();
        
        if (gps.updateLocation() && gps.isLocationValid()) {
            GPSLocation location = gps.getCurrentLocation();
            Serial.printf("[%lu] GPS: %.6f, %.6f, %.1fm\n",
                         millis(), location.latitude, 
                         location.longitude, location.altitude);
        }
    }
    #endif
    
    delay(1000);
}
