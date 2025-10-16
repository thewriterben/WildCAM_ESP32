/**
 * @file gps_manager.cpp
 * @brief GPS Manager implementation for ESP32 using TinyGPS++
 * @author WildCAM ESP32 Team
 * @date 2025-10-16
 * 
 * Implementation of GPS location services using TinyGPS++ library
 * for parsing NMEA sentences from serial GPS modules.
 */

#include "gps_manager.h"

// Default fix timeout: 60 seconds
#define DEFAULT_FIX_TIMEOUT 60000

GPSManager::GPSManager() : 
    gpsSerial(nullptr),
    initialized(false),
    lastFixTime(0),
    fixTimeout(DEFAULT_FIX_TIMEOUT),
    lastLatitude(0.0f),
    lastLongitude(0.0f),
    lastAltitude(0.0f) {
}

GPSManager::~GPSManager() {
    if (gpsSerial != nullptr) {
        gpsSerial->end();
        delete gpsSerial;
        gpsSerial = nullptr;
    }
}

bool GPSManager::initialize(int rx_pin, int tx_pin, int baud_rate) {
    if (initialized) {
        Serial.println("[GPSManager] Already initialized");
        return true;
    }
    
    Serial.printf("[GPSManager] Initializing GPS on RX=%d, TX=%d, baud=%d\n", 
                  rx_pin, tx_pin, baud_rate);
    
    // Create hardware serial instance (using Serial2 for ESP32)
    gpsSerial = new HardwareSerial(2);
    
    if (gpsSerial == nullptr) {
        Serial.println("[GPSManager] ERROR: Failed to create serial instance");
        return false;
    }
    
    // Initialize serial communication with GPS module
    gpsSerial->begin(baud_rate, SERIAL_8N1, rx_pin, tx_pin);
    
    // Wait for serial to be ready
    delay(100);
    
    initialized = true;
    Serial.println("[GPSManager] Initialization successful");
    
    return true;
}

bool GPSManager::update() {
    if (!initialized || gpsSerial == nullptr) {
        return false;
    }
    
    bool dataProcessed = false;
    
    // Read available GPS data and feed to TinyGPS++
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        if (gps.encode(c)) {
            dataProcessed = true;
        }
    }
    
    // If we got valid location data, update last known position
    if (gps.location.isValid()) {
        updateLastPosition();
        lastFixTime = millis();
        
        // Log GPS status periodically (every 30 seconds when we have a fix)
        static unsigned long lastLogTime = 0;
        if (millis() - lastLogTime > 30000) {
            Serial.printf("[GPSManager] Fix: Lat=%.6f, Lon=%.6f, Alt=%.1fm, Sats=%d\n",
                         gps.location.lat(), gps.location.lng(), 
                         gps.altitude.meters(), gps.satellites.value());
            lastLogTime = millis();
        }
    }
    
    return dataProcessed;
}

float GPSManager::getLatitude() {
    if (gps.location.isValid() && isFixValid()) {
        return (float)gps.location.lat();
    }
    
    // Return last known position if no current fix
    return lastLatitude;
}

float GPSManager::getLongitude() {
    if (gps.location.isValid() && isFixValid()) {
        return (float)gps.location.lng();
    }
    
    // Return last known position if no current fix
    return lastLongitude;
}

float GPSManager::getAltitude() {
    if (gps.altitude.isValid() && isFixValid()) {
        return (float)gps.altitude.meters();
    }
    
    // Return last known altitude if no current fix
    return lastAltitude;
}

bool GPSManager::hasFix() {
    if (!initialized) {
        return false;
    }
    
    // Check if we have valid location data and fix hasn't timed out
    return gps.location.isValid() && isFixValid();
}

int GPSManager::getSatelliteCount() {
    if (!initialized) {
        return 0;
    }
    
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    
    return 0;
}

void GPSManager::setFixTimeout(unsigned long timeout_ms) {
    fixTimeout = timeout_ms;
    Serial.printf("[GPSManager] Fix timeout set to %lu ms\n", timeout_ms);
}

unsigned long GPSManager::getTimeSinceLastFix() {
    if (lastFixTime == 0) {
        return 0xFFFFFFFF; // Max value if never had a fix
    }
    
    return millis() - lastFixTime;
}

bool GPSManager::isFixValid() {
    // If we never had a fix, it's not valid
    if (lastFixTime == 0) {
        return false;
    }
    
    // Check if fix has timed out
    unsigned long timeSinceFix = millis() - lastFixTime;
    
    if (timeSinceFix > fixTimeout) {
        // Log timeout warning once per timeout period
        static unsigned long lastTimeoutLog = 0;
        if (millis() - lastTimeoutLog > fixTimeout) {
            Serial.printf("[GPSManager] WARNING: GPS fix timeout (%lu ms since last fix)\n", 
                         timeSinceFix);
            lastTimeoutLog = millis();
        }
        return false;
    }
    
    return true;
}

void GPSManager::updateLastPosition() {
    if (gps.location.isValid()) {
        lastLatitude = (float)gps.location.lat();
        lastLongitude = (float)gps.location.lng();
    }
    
    if (gps.altitude.isValid()) {
        lastAltitude = (float)gps.altitude.meters();
    }
}
