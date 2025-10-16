/**
 * @file gps_manager_example.cpp
 * @brief Example usage of GPSManager class
 * 
 * This example demonstrates how to use the GPSManager class to:
 * - Initialize GPS module
 * - Read GPS data
 * - Handle GPS fix timeout
 * - Access location information
 */

#include <Arduino.h>
#include "sensors/gps_manager.h"

// GPS configuration
#define GPS_RX_PIN 15
#define GPS_TX_PIN 12
#define GPS_BAUD_RATE 9600

// Create GPS manager instance
GPSManager* gps = nullptr;

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("========================================");
    Serial.println("GPSManager Example");
    Serial.println("========================================");
    
    // Create and initialize GPS manager
    gps = new GPSManager();
    
    if (!gps->initialize(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE)) {
        Serial.println("ERROR: Failed to initialize GPS manager!");
        while (1) delay(1000);
    }
    
    // Configure fix timeout (2 minutes)
    gps->setFixTimeout(120000);
    
    Serial.println("GPS manager initialized successfully");
    Serial.println("Waiting for GPS fix...");
    Serial.println();
}

void loop() {
    static unsigned long lastPrintTime = 0;
    
    // Update GPS data (call frequently)
    gps->update();
    
    // Print GPS status every 5 seconds
    if (millis() - lastPrintTime > 5000) {
        lastPrintTime = millis();
        
        Serial.println("----------------------------------------");
        
        if (gps->hasFix()) {
            // We have a valid GPS fix
            float lat = gps->getLatitude();
            float lon = gps->getLongitude();
            float alt = gps->getAltitude();
            int sats = gps->getSatelliteCount();
            unsigned long timeSinceFix = gps->getTimeSinceLastFix();
            
            Serial.println("GPS Status: FIX ACQUIRED");
            Serial.printf("Latitude:    %.6f°\n", lat);
            Serial.printf("Longitude:   %.6f°\n", lon);
            Serial.printf("Altitude:    %.1f m\n", alt);
            Serial.printf("Satellites:  %d\n", sats);
            Serial.printf("Fix age:     %lu ms\n", timeSinceFix);
            
            // Example: Google Maps link
            Serial.printf("Maps link:   https://maps.google.com/?q=%.6f,%.6f\n", lat, lon);
            
        } else {
            // No fix available
            Serial.println("GPS Status: NO FIX");
            
            unsigned long timeSinceFix = gps->getTimeSinceLastFix();
            if (timeSinceFix == 0xFFFFFFFF) {
                Serial.println("Never had a fix");
            } else {
                Serial.printf("Time since last fix: %lu ms\n", timeSinceFix);
                
                // Show last known position
                float lat = gps->getLatitude();
                float lon = gps->getLongitude();
                float alt = gps->getAltitude();
                
                Serial.println("Last known position:");
                Serial.printf("  Latitude:  %.6f°\n", lat);
                Serial.printf("  Longitude: %.6f°\n", lon);
                Serial.printf("  Altitude:  %.1f m\n", alt);
            }
            
            int sats = gps->getSatelliteCount();
            Serial.printf("Satellites visible: %d\n", sats);
        }
        
        Serial.println("----------------------------------------");
        Serial.println();
    }
    
    // Small delay to prevent overwhelming the serial output
    delay(100);
}
