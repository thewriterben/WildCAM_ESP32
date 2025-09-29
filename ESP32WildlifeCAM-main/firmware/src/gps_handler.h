/**
 * @file gps_handler.h
 * @brief Header file for GPS/GNSS location services
 */

#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

struct GPSLocation {
    double latitude;
    double longitude;
    float altitude;
};

class GPSHandler {
public:
    GPSHandler();
    
    // Initialization
    bool initialize();
    bool waitForFix(unsigned long timeoutMs = 60000);
    
    // Location services
    bool updateLocation();
    GPSLocation getCurrentLocation();
    bool isLocationValid();
    String getLocationString();
    
    // Movement detection
    bool isMoving(double threshold = 5.0); // 5 meter threshold
    
    // Distance calculations
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    
    // Power management
    void enterSleepMode();
    void exitSleepMode();
    
    // Configuration
    void setFixTimeout(unsigned long timeoutMs);
    unsigned long getLastFixTime();

private:
    SoftwareSerial gpsSerial;
    bool isInitialized;
    bool hasValidFix;
    GPSLocation currentLocation;
    unsigned long lastFixTime;
    unsigned long fixTimeout;
    
    // Internal methods
    bool configureGPS();
    bool parseGGA(const String& sentence);
    bool parseRMC(const String& sentence);
    double parseCoordinate(const String& coord);
    void updateGPSTime(const String& timeStr, const String& dateStr);
    double radians(double degrees);
};

#endif // GPS_HANDLER_H