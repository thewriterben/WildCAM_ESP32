/**
 * @file gps_manager.h
 * @brief GPS Manager for ESP32 WildCAM - Location tracking using TinyGPS++
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 */

#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

/**
 * @class GPSManager
 * @brief Manages GPS module communication and location data
 * 
 * Provides an easy-to-use interface for GPS modules that output NMEA sentences.
 * Handles initialization, data parsing, and provides access to location data.
 * Maintains last known position when GPS fix is lost.
 */
class GPSManager {
public:
    /**
     * @brief Constructor
     */
    GPSManager();
    
    /**
     * @brief Destructor
     */
    ~GPSManager();
    
    /**
     * @brief Initialize GPS module
     * @param rx_pin GPIO pin for RX (GPS TX -> ESP32 RX)
     * @param tx_pin GPIO pin for TX (GPS RX -> ESP32 TX)
     * @param baud_rate Serial baud rate (default: 9600)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int rx_pin, int tx_pin, int baud_rate = 9600);
    
    /**
     * @brief Update GPS data - call regularly in main loop
     * @return true if new data was processed, false otherwise
     */
    bool update();
    
    /**
     * @brief Check if GPS module is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;
    
    /**
     * @brief Check if GPS has a valid fix
     * @return true if valid fix, false otherwise
     */
    bool hasFix() const;
    
    /**
     * @brief Wait for GPS fix with timeout
     * @param timeout_ms Maximum time to wait in milliseconds (default: 60000)
     * @return true if fix acquired, false if timeout
     */
    bool waitForFix(unsigned long timeout_ms = 60000);
    
    /**
     * @brief Get current latitude
     * @return Latitude in decimal degrees (-90.0 to +90.0)
     */
    float getLatitude() const;
    
    /**
     * @brief Get current longitude
     * @return Longitude in decimal degrees (-180.0 to +180.0)
     */
    float getLongitude() const;
    
    /**
     * @brief Get current altitude
     * @return Altitude in meters above sea level
     */
    float getAltitude() const;
    
    /**
     * @brief Get number of satellites in view
     * @return Number of satellites (0-255)
     */
    int getSatelliteCount() const;
    
    /**
     * @brief Get horizontal dilution of precision
     * @return HDOP value (lower is better, typical range: 1-20)
     */
    float getHDOP() const;
    
    /**
     * @brief Get speed over ground
     * @return Speed in kilometers per hour
     */
    float getSpeedKmph() const;
    
    /**
     * @brief Get course over ground
     * @return Course in degrees (0-360)
     */
    float getCourse() const;
    
    /**
     * @brief Get time since last valid fix
     * @return Milliseconds since last fix, or 0 if no fix ever obtained
     */
    unsigned long getTimeSinceLastFix() const;
    
    /**
     * @brief Get formatted location string
     * @return String in format "lat,lon,alt" or "No GPS fix"
     */
    String getLocationString() const;
    
    /**
     * @brief Print GPS status to serial for debugging
     */
    void printStatus() const;

private:
    TinyGPSPlus gps;                    ///< TinyGPS++ parser instance
    HardwareSerial* gpsSerial;          ///< Hardware serial for GPS communication
    
    bool initialized;                    ///< Initialization status flag
    bool fixAcquired;                    ///< Current fix status
    unsigned long lastFixTime;           ///< Timestamp of last valid fix
    
    // Last known position (cached when fix is lost)
    float lastLatitude;                  ///< Last known latitude
    float lastLongitude;                 ///< Last known longitude
    float lastAltitude;                  ///< Last known altitude
    int lastSatellites;                  ///< Last satellite count
    float lastHDOP;                      ///< Last HDOP value
    
    /**
     * @brief Update cached position data
     */
    void updateLastKnownPosition();
};

#endif // GPS_MANAGER_H
