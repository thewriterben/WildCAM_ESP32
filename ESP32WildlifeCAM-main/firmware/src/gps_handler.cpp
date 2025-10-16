/**
 * @file gps_handler.cpp
 * @brief GPS/GNSS handler for location services and satellite timing
 * 
 * Provides GPS location data for wildlife camera images and
 * satellite pass timing optimization.
 */

#include "gps_handler.h"
#include "config.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <math.h>

// GPS pin definitions are now configured in config.h
// GPS_RX_PIN, GPS_TX_PIN, GPS_EN_PIN are board-specific

GPSHandler::GPSHandler() :
#if GPS_ENABLED
    gpsSerial(GPS_RX_PIN, GPS_TX_PIN),
#else
    gpsSerial(-1, -1),  // Dummy initialization when GPS disabled
#endif
    isInitialized(false),
    hasValidFix(false),
    currentLocation{0, 0, 0},
    lastFixTime(0),
    fixTimeout(GPS_FIX_TIMEOUT) {  // Use timeout from config.h
}

bool GPSHandler::initialize() {
#if !GPS_ENABLED
    Serial.println("GPS module is disabled in config.h");
    return false;
#else
    // Initialize GPS serial communication
    gpsSerial.begin(GPS_BAUD_RATE);
    
    // Enable GPS module if enable pin is defined
    #ifdef GPS_EN_PIN
    pinMode(GPS_EN_PIN, OUTPUT);
    digitalWrite(GPS_EN_PIN, HIGH);
    delay(1000);
    #endif
    
    // Configure GPS module
    if (configureGPS()) {
        isInitialized = true;
        Serial.println("GPS handler initialized successfully");
        Serial.printf("GPS Pins - RX: %d, TX: %d", GPS_RX_PIN, GPS_TX_PIN);
        #ifdef GPS_EN_PIN
        Serial.printf(", EN: %d", GPS_EN_PIN);
        #endif
        Serial.println();
        return true;
    }
    
    Serial.println("Failed to initialize GPS module");
    return false;
#endif
}

bool GPSHandler::configureGPS() {
    // Send configuration commands to GPS module
    // Set output rate to 1Hz
    gpsSerial.println("$PMTK220,1000*1F");
    delay(500);
    
    // Set NMEA output to include GGA and RMC sentences
    gpsSerial.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
    delay(500);
    
    // Enable DGPS
    gpsSerial.println("$PMTK301,2*2E");
    delay(500);
    
    return true;
}

bool GPSHandler::waitForFix(unsigned long timeoutMs) {
    unsigned long startTime = millis();
    
    Serial.println("Waiting for GPS fix...");
    
    while (millis() - startTime < timeoutMs) {
        if (updateLocation()) {
            if (hasValidFix) {
                Serial.printf("GPS fix acquired: %.6f, %.6f (altitude: %.1fm)\n",
                             currentLocation.latitude, currentLocation.longitude, 
                             currentLocation.altitude);
                return true;
            }
        }
        delay(1000);
    }
    
    Serial.println("GPS fix timeout");
    return false;
}

bool GPSHandler::updateLocation() {
    if (!isInitialized) {
        return false;
    }
    
    while (gpsSerial.available()) {
        String sentence = gpsSerial.readStringUntil('\n');
        sentence.trim();
        
        if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA")) {
            return parseGGA(sentence);
        } else if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
            return parseRMC(sentence);
        }
    }
    
    // Check if fix has timed out
    if (hasValidFix && (millis() - lastFixTime > fixTimeout)) {
        hasValidFix = false;
        Serial.println("GPS fix lost (timeout)");
    }
    
    return hasValidFix;
}

bool GPSHandler::parseGGA(const String& sentence) {
    // Parse GGA sentence: $GPGGA,time,lat,N/S,lon,E/W,quality,numSV,HDOP,alt,M,geoid,M,dgps,checksum
    
    int commaIndex[14];
    int commaCount = 0;
    
    // Find all comma positions
    for (int i = 0; i < sentence.length() && commaCount < 14; i++) {
        if (sentence.charAt(i) == ',') {
            commaIndex[commaCount++] = i;
        }
    }
    
    if (commaCount < 13) {
        return false; // Invalid sentence
    }
    
    // Extract quality indicator
    String qualityStr = sentence.substring(commaIndex[5] + 1, commaIndex[6]);
    int quality = qualityStr.toInt();
    
    if (quality == 0) {
        hasValidFix = false;
        return false; // No fix
    }
    
    // Extract latitude
    String latStr = sentence.substring(commaIndex[1] + 1, commaIndex[2]);
    String latDirStr = sentence.substring(commaIndex[2] + 1, commaIndex[3]);
    
    if (latStr.length() > 0) {
        double lat = parseCoordinate(latStr);
        if (latDirStr == "S") lat = -lat;
        currentLocation.latitude = lat;
    }
    
    // Extract longitude
    String lonStr = sentence.substring(commaIndex[3] + 1, commaIndex[4]);
    String lonDirStr = sentence.substring(commaIndex[4] + 1, commaIndex[5]);
    
    if (lonStr.length() > 0) {
        double lon = parseCoordinate(lonStr);
        if (lonDirStr == "W") lon = -lon;
        currentLocation.longitude = lon;
    }
    
    // Extract altitude
    String altStr = sentence.substring(commaIndex[8] + 1, commaIndex[9]);
    if (altStr.length() > 0) {
        currentLocation.altitude = altStr.toFloat();
    }
    
    hasValidFix = true;
    lastFixTime = millis();
    
    return true;
}

bool GPSHandler::parseRMC(const String& sentence) {
    // Parse RMC sentence for date/time information
    // $GPRMC,time,status,lat,N/S,lon,E/W,speed,course,date,magvar,E/W,checksum
    
    int commaIndex[12];
    int commaCount = 0;
    
    // Find all comma positions
    for (int i = 0; i < sentence.length() && commaCount < 12; i++) {
        if (sentence.charAt(i) == ',') {
            commaIndex[commaCount++] = i;
        }
    }
    
    if (commaCount < 11) {
        return false;
    }
    
    // Extract status
    String statusStr = sentence.substring(commaIndex[1] + 1, commaIndex[2]);
    if (statusStr != "A") {
        hasValidFix = false;
        return false; // Invalid fix
    }
    
    // Extract date and time
    String timeStr = sentence.substring(commaIndex[0] + 1, commaIndex[1]);
    String dateStr = sentence.substring(commaIndex[8] + 1, commaIndex[9]);
    
    if (timeStr.length() >= 6 && dateStr.length() >= 6) {
        updateGPSTime(timeStr, dateStr);
    }
    
    return hasValidFix;
}

double GPSHandler::parseCoordinate(const String& coord) {
    if (coord.length() < 4) return 0.0;
    
    // Format: DDMM.MMMM (degrees and minutes)
    int dotIndex = coord.indexOf('.');
    if (dotIndex < 3) return 0.0;
    
    String degreesStr = coord.substring(0, dotIndex - 2);
    String minutesStr = coord.substring(dotIndex - 2);
    
    double degrees = degreesStr.toFloat();
    double minutes = minutesStr.toFloat();
    
    return degrees + (minutes / 60.0);
}

void GPSHandler::updateGPSTime(const String& timeStr, const String& dateStr) {
    // Extract time components (HHMMSS)
    if (timeStr.length() >= 6) {
        int hours = timeStr.substring(0, 2).toInt();
        int minutes = timeStr.substring(2, 4).toInt();
        int seconds = timeStr.substring(4, 6).toInt();
        
        Serial.printf("GPS Time: %02d:%02d:%02d\n", hours, minutes, seconds);
    }
    
    // Extract date components (DDMMYY)
    if (dateStr.length() >= 6) {
        int day = dateStr.substring(0, 2).toInt();
        int month = dateStr.substring(2, 4).toInt();
        int year = 2000 + dateStr.substring(4, 6).toInt();
        
        Serial.printf("GPS Date: %02d/%02d/%04d\n", day, month, year);
    }
}

GPSLocation GPSHandler::getCurrentLocation() {
    updateLocation();
    return currentLocation;
}

bool GPSHandler::isLocationValid() {
    updateLocation();
    return hasValidFix;
}

String GPSHandler::getLocationString() {
    if (!hasValidFix) {
        return "No GPS fix";
    }
    
    return String(currentLocation.latitude, 6) + "," + 
           String(currentLocation.longitude, 6) + "," + 
           String(currentLocation.altitude, 1);
}

double GPSHandler::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula for calculating distance between two GPS coordinates
    const double earthRadius = 6371000; // Earth radius in meters
    
    double dLat = radians(lat2 - lat1);
    double dLon = radians(lon2 - lon1);
    
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(radians(lat1)) * cos(radians(lat2)) *
               sin(dLon / 2) * sin(dLon / 2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double distance = earthRadius * c;
    
    return distance;
}

double GPSHandler::radians(double degrees) {
    return degrees * PI / 180.0;
}

bool GPSHandler::isMoving(double threshold) {
    static GPSLocation lastLocation = {0, 0, 0};
    static bool hasLastLocation = false;
    
    if (!hasValidFix) {
        return false;
    }
    
    if (!hasLastLocation) {
        lastLocation = currentLocation;
        hasLastLocation = true;
        return false;
    }
    
    double distance = calculateDistance(lastLocation.latitude, lastLocation.longitude,
                                       currentLocation.latitude, currentLocation.longitude);
    
    lastLocation = currentLocation;
    
    return distance > threshold;
}

void GPSHandler::enterSleepMode() {
    if (isInitialized) {
        // Put GPS module into standby mode
        gpsSerial.println("$PMTK161,0*28");
        delay(100);
        
        // Disable GPS power if enable pin is defined
        #ifdef GPS_EN_PIN
        digitalWrite(GPS_EN_PIN, LOW);
        #endif
    }
}

void GPSHandler::exitSleepMode() {
    if (isInitialized) {
        // Enable GPS power if enable pin is defined
        #ifdef GPS_EN_PIN
        digitalWrite(GPS_EN_PIN, HIGH);
        delay(1000);
        #endif
        
        // Wake up GPS module
        gpsSerial.println("$PMTK010,001*2E");
        delay(100);
        
        // Reconfigure GPS
        configureGPS();
    }
}

void GPSHandler::setFixTimeout(unsigned long timeoutMs) {
    fixTimeout = timeoutMs;
}

unsigned long GPSHandler::getLastFixTime() {
    return lastFixTime;
}