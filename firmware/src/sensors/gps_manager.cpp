/**
 * @file gps_manager.cpp
 * @brief GPS Manager implementation for ESP32 WildCAM
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 */

#include "gps_manager.h"

GPSManager::GPSManager()
    : gpsSerial(nullptr),
      initialized(false),
      fixAcquired(false),
      lastFixTime(0),
      lastLatitude(0.0),
      lastLongitude(0.0),
      lastAltitude(0.0),
      lastSatellites(0),
      lastHDOP(0.0)
{
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
        Serial.println("[GPS] Already initialized");
        return true;
    }
    
    Serial.println("[GPS] Initializing GPS Manager...");
    Serial.printf("[GPS] RX Pin: %d, TX Pin: %d, Baud Rate: %d\n", rx_pin, tx_pin, baud_rate);
    
    // Create and configure hardware serial for GPS (UART1)
    gpsSerial = new HardwareSerial(1);
    gpsSerial->begin(baud_rate, SERIAL_8N1, rx_pin, tx_pin);
    
    // Small delay to allow serial to stabilize
    delay(100);
    
    initialized = true;
    Serial.println("[GPS] GPS Manager initialized successfully");
    
    return true;
}

bool GPSManager::update() {
    if (!initialized || gpsSerial == nullptr) {
        return false;
    }
    
    bool newData = false;
    
    // Read all available GPS data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        if (gps.encode(c)) {
            newData = true;
        }
    }
    
    // Update fix status based on location validity
    if (gps.location.isValid() && gps.location.isUpdated()) {
        if (!fixAcquired) {
            Serial.println("[GPS] GPS fix acquired!");
        }
        fixAcquired = true;
        lastFixTime = millis();
        
        // Update cached position
        updateLastKnownPosition();
        
        return true;
    } else if (fixAcquired && (millis() - lastFixTime > 5000)) {
        // Lost fix if no update for 5 seconds
        Serial.println("[GPS] GPS fix lost");
        fixAcquired = false;
    }
    
    return newData;
}

bool GPSManager::isInitialized() const {
    return initialized;
}

bool GPSManager::hasFix() const {
    return fixAcquired && gps.location.isValid();
}

bool GPSManager::waitForFix(unsigned long timeout_ms) {
    if (!initialized) {
        Serial.println("[GPS] ERROR: GPS not initialized");
        return false;
    }
    
    Serial.printf("[GPS] Waiting for GPS fix (timeout: %lu ms)...\n", timeout_ms);
    unsigned long startTime = millis();
    
    while ((millis() - startTime) < timeout_ms) {
        update();
        
        if (hasFix()) {
            Serial.println("[GPS] GPS fix acquired!");
            return true;
        }
        
        // Print status every 5 seconds
        if ((millis() - startTime) % 5000 < 100) {
            Serial.printf("[GPS] Still waiting... Satellites: %d\n", getSatelliteCount());
        }
        
        delay(100);
    }
    
    Serial.println("[GPS] Timeout waiting for GPS fix");
    return false;
}

float GPSManager::getLatitude() const {
    if (gps.location.isValid()) {
        return (float)gps.location.lat();
    }
    return lastLatitude;
}

float GPSManager::getLongitude() const {
    if (gps.location.isValid()) {
        return (float)gps.location.lng();
    }
    return lastLongitude;
}

float GPSManager::getAltitude() const {
    if (gps.altitude.isValid()) {
        return (float)gps.altitude.meters();
    }
    return lastAltitude;
}

int GPSManager::getSatelliteCount() const {
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    return lastSatellites;
}

float GPSManager::getHDOP() const {
    if (gps.hdop.isValid()) {
        return (float)gps.hdop.hdop();
    }
    return lastHDOP;
}

float GPSManager::getSpeedKmph() const {
    if (gps.speed.isValid()) {
        return (float)gps.speed.kmph();
    }
    return 0.0;
}

float GPSManager::getCourse() const {
    if (gps.course.isValid()) {
        return (float)gps.course.deg();
    }
    return 0.0;
}

unsigned long GPSManager::getTimeSinceLastFix() const {
    if (lastFixTime == 0) {
        return 0;
    }
    return millis() - lastFixTime;
}

String GPSManager::getLocationString() const {
    if (hasFix() || (lastLatitude != 0.0 && lastLongitude != 0.0)) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.6f,%.6f,%.1f", 
                 getLatitude(), getLongitude(), getAltitude());
        return String(buffer);
    }
    return "No GPS fix";
}

void GPSManager::printStatus() const {
    Serial.println("========== GPS Status ==========");
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("Fix Status: %s\n", hasFix() ? "Valid" : "Invalid");
    
    if (gps.location.isValid()) {
        Serial.printf("Latitude: %.6f°\n", gps.location.lat());
        Serial.printf("Longitude: %.6f°\n", gps.location.lng());
        Serial.printf("Location Age: %lu ms\n", gps.location.age());
    } else {
        Serial.printf("Last Known Latitude: %.6f°\n", lastLatitude);
        Serial.printf("Last Known Longitude: %.6f°\n", lastLongitude);
    }
    
    if (gps.altitude.isValid()) {
        Serial.printf("Altitude: %.1f m\n", gps.altitude.meters());
    } else {
        Serial.printf("Last Known Altitude: %.1f m\n", lastAltitude);
    }
    
    if (gps.satellites.isValid()) {
        Serial.printf("Satellites: %d\n", gps.satellites.value());
    } else {
        Serial.printf("Last Satellites: %d\n", lastSatellites);
    }
    
    if (gps.hdop.isValid()) {
        Serial.printf("HDOP: %.2f\n", gps.hdop.hdop());
    } else {
        Serial.printf("Last HDOP: %.2f\n", lastHDOP);
    }
    
    if (gps.speed.isValid()) {
        Serial.printf("Speed: %.2f km/h\n", gps.speed.kmph());
    }
    
    if (gps.course.isValid()) {
        Serial.printf("Course: %.2f°\n", gps.course.deg());
    }
    
    if (lastFixTime > 0) {
        Serial.printf("Time Since Last Fix: %lu ms\n", getTimeSinceLastFix());
    }
    
    Serial.printf("Characters Processed: %lu\n", gps.charsProcessed());
    Serial.printf("Sentences with Fix: %lu\n", gps.sentencesWithFix());
    Serial.printf("Failed Checksum: %lu\n", gps.failedChecksum());
    Serial.println("================================");
}

void GPSManager::updateLastKnownPosition() {
    if (gps.location.isValid()) {
        lastLatitude = (float)gps.location.lat();
        lastLongitude = (float)gps.location.lng();
    }
    
    if (gps.altitude.isValid()) {
        lastAltitude = (float)gps.altitude.meters();
    }
    
    if (gps.satellites.isValid()) {
        lastSatellites = gps.satellites.value();
    }
    
    if (gps.hdop.isValid()) {
        lastHDOP = (float)gps.hdop.hdop();
    }
}
