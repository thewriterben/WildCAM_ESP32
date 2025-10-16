/**
 * @file gps_manager.cpp
 * @brief GPS Manager implementation for ESP32 Wildlife Camera
 * 
 * Implements GPS tracking functionality using TinyGPS++ library
 * for parsing NMEA sentences from GPS modules.
 * 
 * @version 1.0.0
 * @date 2025-10-16
 */

#include "gps_manager.h"

// Include config.h for GPS configuration settings
#if __has_include("../config.h")
#include "../config.h"
#else
// Default values if config.h not available
#define GPS_FIX_TIMEOUT 60000
#define GPS_SAVE_LAST_POSITION true
#define GPS_DECIMAL_PLACES 6
#endif

GPSManager::GPSManager() :
    gpsSerial(nullptr),
    initialized(false),
    hasValidFix(false),
    lastFixTime(0),
    lastLatitude(0.0),
    lastLongitude(0.0),
    lastAltitude(0.0) {
}

GPSManager::~GPSManager() {
    if (gpsSerial != nullptr) {
        gpsSerial->end();
        delete gpsSerial;
        gpsSerial = nullptr;
    }
}

bool GPSManager::initialize(int rx_pin, int tx_pin, int baud_rate) {
    // Validate pins
    if (rx_pin < 0 || tx_pin < 0) {
        Serial.println("GPS Manager: Invalid GPIO pins specified");
        return false;
    }

    // Create hardware serial instance (use Serial2 for GPS)
    gpsSerial = new HardwareSerial(2);
    
    if (gpsSerial == nullptr) {
        Serial.println("GPS Manager: Failed to create serial instance");
        return false;
    }

    // Initialize serial communication with GPS module
    gpsSerial->begin(baud_rate, SERIAL_8N1, rx_pin, tx_pin);
    
    // Give GPS module time to start up
    delay(100);

    // Verify serial is working
    if (!gpsSerial) {
        Serial.println("GPS Manager: Serial initialization failed");
        delete gpsSerial;
        gpsSerial = nullptr;
        return false;
    }

    initialized = true;
    Serial.printf("GPS Manager: Initialized successfully (RX: GPIO%d, TX: GPIO%d, Baud: %d)\n", 
                  rx_pin, tx_pin, baud_rate);
    
    return true;
}

bool GPSManager::update() {
    if (!initialized || gpsSerial == nullptr) {
        return false;
    }

    bool newData = false;
    
    // Process all available GPS data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        
        // Feed character to TinyGPS++
        if (gps.encode(c)) {
            newData = true;
        }
    }

    // Update fix status based on location validity
    if (gps.location.isValid()) {
        if (!hasValidFix) {
            Serial.println("GPS Manager: Fix acquired");
        }
        hasValidFix = true;
        lastFixTime = millis();
        
        // Update last known position
        #ifdef GPS_SAVE_LAST_POSITION
        #if GPS_SAVE_LAST_POSITION
        updateLastKnownPosition();
        #endif
        #endif
    } else {
        // Check if fix has timed out
        #ifdef GPS_FIX_TIMEOUT
        if (hasValidFix && (millis() - lastFixTime > GPS_FIX_TIMEOUT)) {
            if (hasValidFix) {
                Serial.println("GPS Manager: Fix lost (timeout)");
            }
            hasValidFix = false;
        }
        #endif
    }

    return newData;
}

float GPSManager::getLatitude() {
    if (hasValidFix && gps.location.isValid()) {
        return (float)gps.location.lat();
    }
    
    // Return last known position if available
    #ifdef GPS_SAVE_LAST_POSITION
    #if GPS_SAVE_LAST_POSITION
    if (lastFixTime > 0) {
        return lastLatitude;
    }
    #endif
    #endif
    
    return 0.0;
}

float GPSManager::getLongitude() {
    if (hasValidFix && gps.location.isValid()) {
        return (float)gps.location.lng();
    }
    
    // Return last known position if available
    #ifdef GPS_SAVE_LAST_POSITION
    #if GPS_SAVE_LAST_POSITION
    if (lastFixTime > 0) {
        return lastLongitude;
    }
    #endif
    #endif
    
    return 0.0;
}

float GPSManager::getAltitude() {
    if (hasValidFix && gps.altitude.isValid()) {
        return (float)gps.altitude.meters();
    }
    
    // Return last known altitude if available
    #ifdef GPS_SAVE_LAST_POSITION
    #if GPS_SAVE_LAST_POSITION
    if (lastFixTime > 0) {
        return lastAltitude;
    }
    #endif
    #endif
    
    return 0.0;
}

bool GPSManager::hasFix() {
    return hasValidFix && gps.location.isValid();
}

int GPSManager::getSatelliteCount() {
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    return 0;
}

unsigned long GPSManager::getTimeSinceLastFix() {
    if (lastFixTime == 0) {
        return 0;
    }
    return millis() - lastFixTime;
}

float GPSManager::getHDOP() {
    if (gps.hdop.isValid()) {
        return (float)gps.hdop.hdop();
    }
    return 99.9; // Return high value if HDOP not available
}

float GPSManager::getSpeedKmph() {
    if (gps.speed.isValid()) {
        return (float)gps.speed.kmph();
    }
    return 0.0;
}

float GPSManager::getCourse() {
    if (gps.course.isValid()) {
        return (float)gps.course.deg();
    }
    return 0.0;
}

bool GPSManager::isInitialized() {
    return initialized;
}

String GPSManager::getLocationString() {
    if (!hasFix() && lastFixTime == 0) {
        return "No GPS fix";
    }

    // Determine number of decimal places
    int decimalPlaces = 6; // Default
    #ifdef GPS_DECIMAL_PLACES
    decimalPlaces = GPS_DECIMAL_PLACES;
    #endif

    float lat = getLatitude();
    float lon = getLongitude();
    float alt = getAltitude();

    String locationStr = String(lat, decimalPlaces) + "," + 
                        String(lon, decimalPlaces);
    
    // Add altitude if enabled
    #ifdef GPS_ALTITUDE_ENABLED
    #if GPS_ALTITUDE_ENABLED
    locationStr += "," + String(alt, 1);
    #endif
    #endif

    // Add fix status indicator if using last known position
    if (!hasValidFix && lastFixTime > 0) {
        locationStr += " (last known)";
    }

    return locationStr;
}

bool GPSManager::waitForFix(unsigned long timeout_ms) {
    if (!initialized) {
        Serial.println("GPS Manager: Not initialized");
        return false;
    }

    Serial.printf("GPS Manager: Waiting for fix (timeout: %lu ms)...\n", timeout_ms);
    
    unsigned long startTime = millis();
    int dotCounter = 0;
    
    while (millis() - startTime < timeout_ms) {
        // Update GPS data
        update();
        
        // Check if we have a fix
        if (hasFix()) {
            Serial.printf("\nGPS Manager: Fix acquired! Lat: %.6f, Lon: %.6f, Alt: %.1fm, Sats: %d\n",
                         getLatitude(), getLongitude(), getAltitude(), getSatelliteCount());
            return true;
        }

        // Print progress dots
        if (++dotCounter % 10 == 0) {
            Serial.print(".");
        }

        delay(100);
    }

    Serial.println("\nGPS Manager: Fix timeout");
    return false;
}

void GPSManager::processGPSData() {
    // This method is for future expansion if needed
    // Currently, data processing is handled in update()
}

void GPSManager::updateLastKnownPosition() {
    if (gps.location.isValid()) {
        lastLatitude = (float)gps.location.lat();
        lastLongitude = (float)gps.location.lng();
    }
    
    if (gps.altitude.isValid()) {
        lastAltitude = (float)gps.altitude.meters();
    }
}
