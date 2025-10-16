/**
 * @file gps_manager_example.cpp
 * @brief Example usage of GPSManager for ESP32 Wildlife Camera
 * 
 * This example demonstrates how to initialize and use the GPSManager
 * to track wildlife camera location.
 * 
 * @version 1.0.0
 * @date 2025-10-16
 */

#include <Arduino.h>
#include "sensors/gps_manager.h"
#include "config.h"

// Global GPS manager instance
GPSManager* gpsManager = nullptr;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== ESP32 WildCAM GPS Manager Example ===\n");
    
    // Check if GPS is enabled in config
    #ifdef GPS_ENABLED
    #if GPS_ENABLED
        Serial.println("GPS is enabled in configuration");
        
        // Create GPS manager instance
        gpsManager = new GPSManager();
        
        if (gpsManager == nullptr) {
            Serial.println("ERROR: Failed to create GPSManager instance");
            return;
        }
        
        // Initialize GPS with pins from config.h
        Serial.printf("Initializing GPS on pins RX:%d TX:%d Baud:%d\n", 
                     GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);
        
        bool initialized = gpsManager->initialize(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);
        
        if (initialized) {
            Serial.println("GPS Manager initialized successfully!");
            
            // Wait for GPS fix (optional)
            #ifdef GPS_FIX_TIMEOUT
            Serial.printf("Waiting for GPS fix (timeout: %d ms)...\n", GPS_FIX_TIMEOUT);
            
            if (gpsManager->waitForFix(GPS_FIX_TIMEOUT)) {
                Serial.println("GPS fix acquired!");
                
                // Print GPS data
                Serial.printf("  Latitude:  %.6f\n", gpsManager->getLatitude());
                Serial.printf("  Longitude: %.6f\n", gpsManager->getLongitude());
                Serial.printf("  Altitude:  %.1f m\n", gpsManager->getAltitude());
                Serial.printf("  Satellites: %d\n", gpsManager->getSatelliteCount());
                Serial.printf("  HDOP: %.2f\n", gpsManager->getHDOP());
                Serial.printf("  Location String: %s\n", gpsManager->getLocationString().c_str());
            } else {
                Serial.println("GPS fix timeout - will use last known position when available");
            }
            #endif
        } else {
            Serial.println("ERROR: GPS Manager initialization failed");
        }
    #else
        Serial.println("GPS is disabled in configuration");
    #endif
    #else
        Serial.println("GPS_ENABLED not defined in config.h");
    #endif
}

void loop() {
    // Update GPS data regularly
    if (gpsManager != nullptr && gpsManager->isInitialized()) {
        // Update GPS (call this regularly in main loop)
        gpsManager->update();
        
        // Check if we have a fix
        if (gpsManager->hasFix()) {
            // Print GPS data every 10 seconds
            static unsigned long lastPrint = 0;
            if (millis() - lastPrint > 10000) {
                lastPrint = millis();
                
                Serial.println("\n--- GPS Update ---");
                Serial.printf("Latitude:   %.6f\n", gpsManager->getLatitude());
                Serial.printf("Longitude:  %.6f\n", gpsManager->getLongitude());
                Serial.printf("Altitude:   %.1f m\n", gpsManager->getAltitude());
                Serial.printf("Satellites: %d\n", gpsManager->getSatelliteCount());
                Serial.printf("HDOP:       %.2f\n", gpsManager->getHDOP());
                Serial.printf("Speed:      %.1f km/h\n", gpsManager->getSpeedKmph());
                Serial.printf("Course:     %.1f degrees\n", gpsManager->getCourse());
                Serial.printf("Fix time:   %lu ms ago\n", gpsManager->getTimeSinceLastFix());
                
                // Get formatted location string
                String locationStr = gpsManager->getLocationString();
                Serial.printf("Location:   %s\n", locationStr.c_str());
            }
        } else {
            // No fix - print status every 30 seconds
            static unsigned long lastNoFixPrint = 0;
            if (millis() - lastNoFixPrint > 30000) {
                lastNoFixPrint = millis();
                
                Serial.println("\n--- GPS Status ---");
                Serial.println("No GPS fix");
                Serial.printf("Satellites: %d\n", gpsManager->getSatelliteCount());
                Serial.printf("Time since last fix: %lu ms\n", gpsManager->getTimeSinceLastFix());
                
                // Still print last known position if available
                String locationStr = gpsManager->getLocationString();
                if (locationStr.indexOf("No GPS fix") < 0) {
                    Serial.printf("Last known location: %s\n", locationStr.c_str());
                }
            }
        }
    }
    
    // Small delay to prevent overwhelming serial output
    delay(100);
}

/**
 * Example function showing how to use GPS data in image capture
 */
void captureImageWithGPS() {
    // Example: Capture image and tag with GPS coordinates
    
    if (gpsManager != nullptr && gpsManager->hasFix()) {
        // Get GPS data for image metadata
        float latitude = gpsManager->getLatitude();
        float longitude = gpsManager->getLongitude();
        float altitude = gpsManager->getAltitude();
        
        Serial.println("\nCapturing image with GPS data:");
        Serial.printf("  GPS: %.6f, %.6f (alt: %.1fm)\n", latitude, longitude, altitude);
        
        // TODO: Add your image capture code here
        // Example: camera.capture();
        // Example: saveImageWithMetadata(latitude, longitude, altitude);
        
    } else {
        Serial.println("\nCapturing image without GPS data (no fix)");
        
        // Still capture image, just without GPS coordinates
        // TODO: Add your image capture code here
    }
}

/**
 * Example function showing how to check if camera has moved
 */
bool hasCameraMoved(float thresholdMeters = 10.0) {
    static float lastLat = 0.0;
    static float lastLon = 0.0;
    static bool hasLastPosition = false;
    
    if (gpsManager == nullptr || !gpsManager->hasFix()) {
        return false;
    }
    
    float currentLat = gpsManager->getLatitude();
    float currentLon = gpsManager->getLongitude();
    
    if (!hasLastPosition) {
        lastLat = currentLat;
        lastLon = currentLon;
        hasLastPosition = true;
        return false;
    }
    
    // Calculate distance using Haversine formula (simplified)
    const float earthRadius = 6371000.0; // meters
    float dLat = (currentLat - lastLat) * PI / 180.0;
    float dLon = (currentLon - lastLon) * PI / 180.0;
    
    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(lastLat * PI / 180.0) * cos(currentLat * PI / 180.0) *
              sin(dLon / 2) * sin(dLon / 2);
    
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    float distance = earthRadius * c;
    
    if (distance > thresholdMeters) {
        Serial.printf("Camera moved %.1f meters\n", distance);
        lastLat = currentLat;
        lastLon = currentLon;
        return true;
    }
    
    return false;
}
