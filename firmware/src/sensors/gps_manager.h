/**
 * @file gps_manager.h
 * @brief GPS Manager class for ESP32 using TinyGPS++
 * @author WildCAM ESP32 Team
 * @date 2025-10-16
 * 
 * Provides GPS location services using TinyGPS++ library for parsing
 * NMEA sentences from serial GPS modules.
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
 * This class handles GPS module initialization, NMEA sentence parsing,
 * and provides access to location data with timeout handling.
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
     * @brief Initialize GPS module with specified pins and baud rate
     * @param rx_pin RX pin number for GPS serial communication
     * @param tx_pin TX pin number for GPS serial communication
     * @param baud_rate Baud rate for GPS serial communication (default: 9600)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int rx_pin, int tx_pin, int baud_rate = 9600);
    
    /**
     * @brief Update GPS data by reading and parsing NMEA sentences
     * @return true if GPS data was updated, false otherwise
     */
    bool update();
    
    /**
     * @brief Get current latitude
     * @return Latitude in decimal degrees (returns last known if no fix)
     */
    float getLatitude();
    
    /**
     * @brief Get current longitude
     * @return Longitude in decimal degrees (returns last known if no fix)
     */
    float getLongitude();
    
    /**
     * @brief Get current altitude
     * @return Altitude in meters (returns last known if no fix)
     */
    float getAltitude();
    
    /**
     * @brief Check if GPS has a valid fix
     * @return true if GPS has valid fix, false otherwise
     */
    bool hasFix();
    
    /**
     * @brief Get number of satellites in view
     * @return Number of satellites
     */
    int getSatelliteCount();
    
    /**
     * @brief Set fix timeout duration
     * @param timeout_ms Timeout in milliseconds
     */
    void setFixTimeout(unsigned long timeout_ms);
    
    /**
     * @brief Get time since last valid fix
     * @return Time in milliseconds since last fix
     */
    unsigned long getTimeSinceLastFix();

private:
    TinyGPSPlus gps;                    ///< TinyGPS++ parser instance
    HardwareSerial* gpsSerial;          ///< Hardware serial for GPS communication
    
    bool initialized;                    ///< Initialization status flag
    unsigned long lastFixTime;           ///< Timestamp of last valid fix
    unsigned long fixTimeout;            ///< Fix timeout duration in milliseconds
    
    float lastLatitude;                  ///< Last known latitude
    float lastLongitude;                 ///< Last known longitude
    float lastAltitude;                  ///< Last known altitude
    
    /**
     * @brief Check if fix has timed out
     * @return true if fix is still valid, false if timed out
     */
    bool isFixValid();
    
    /**
     * @brief Update last known position from GPS
     */
    void updateLastPosition();
};

#endif // GPS_MANAGER_H
