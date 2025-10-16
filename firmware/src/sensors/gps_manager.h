/**
 * @file gps_manager.h
 * @brief GPS Manager for ESP32 Wildlife Camera
 * 
 * Provides GPS location tracking and management for wildlife monitoring.
 * Uses TinyGPS++ library for NMEA sentence parsing.
 * 
 * Features:
 * - Initialize GPS module with configurable pins and baud rate
 * - Read and parse GPS data from NMEA sentences
 * - Track GPS fix status and satellite count
 * - Store last known position on fix loss
 * - Provide latitude, longitude, and altitude data
 * 
 * @version 1.0.0
 * @date 2025-10-16
 */

#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

/**
 * @class GPSManager
 * @brief Manages GPS module communication and data processing
 */
class GPSManager {
public:
    /**
     * @brief Constructor for GPSManager
     */
    GPSManager();

    /**
     * @brief Destructor for GPSManager
     */
    ~GPSManager();

    /**
     * @brief Initialize GPS module with specified pins and baud rate
     * @param rx_pin GPIO pin for RX (GPS TX -> ESP32 RX)
     * @param tx_pin GPIO pin for TX (GPS RX -> ESP32 TX)
     * @param baud_rate Serial communication baud rate (default: 9600)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int rx_pin, int tx_pin, int baud_rate = 9600);

    /**
     * @brief Update GPS data by reading and parsing incoming NMEA sentences
     * Should be called regularly in main loop to process GPS data
     * @return true if new valid data was processed, false otherwise
     */
    bool update();

    /**
     * @brief Get current latitude in decimal degrees
     * @return Latitude (-90.0 to +90.0), or last known position if fix lost
     */
    float getLatitude();

    /**
     * @brief Get current longitude in decimal degrees
     * @return Longitude (-180.0 to +180.0), or last known position if fix lost
     */
    float getLongitude();

    /**
     * @brief Get current altitude in meters above sea level
     * @return Altitude in meters, or last known altitude if fix lost
     */
    float getAltitude();

    /**
     * @brief Check if GPS has a valid fix
     * @return true if GPS has valid fix, false otherwise
     */
    bool hasFix();

    /**
     * @brief Get number of satellites currently in view
     * @return Number of satellites (0-255)
     */
    int getSatelliteCount();

    /**
     * @brief Get time since last valid GPS fix
     * @return Milliseconds since last fix, or 0 if no fix ever obtained
     */
    unsigned long getTimeSinceLastFix();

    /**
     * @brief Get GPS horizontal dilution of precision (HDOP)
     * Lower values indicate better precision
     * @return HDOP value (typical range: 1-20)
     */
    float getHDOP();

    /**
     * @brief Get speed over ground in km/h
     * @return Speed in kilometers per hour
     */
    float getSpeedKmph();

    /**
     * @brief Get course over ground in degrees
     * @return Course in degrees (0-360)
     */
    float getCourse();

    /**
     * @brief Check if GPS module is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized();

    /**
     * @brief Get formatted location string
     * @return String in format "lat,lon,alt" or "No GPS fix" if no fix
     */
    String getLocationString();

    /**
     * @brief Wait for GPS fix with timeout
     * Blocks until fix is acquired or timeout expires
     * @param timeout_ms Maximum time to wait in milliseconds
     * @return true if fix acquired, false if timeout
     */
    bool waitForFix(unsigned long timeout_ms = 60000);

private:
    TinyGPSPlus gps;                    ///< TinyGPS++ parser object
    HardwareSerial* gpsSerial;          ///< Hardware serial interface
    
    bool initialized;                   ///< Initialization status
    bool hasValidFix;                   ///< Current GPS fix status
    unsigned long lastFixTime;          ///< Timestamp of last valid fix
    
    // Last known position (used when fix is lost)
    float lastLatitude;                 ///< Last known latitude
    float lastLongitude;                ///< Last known longitude
    float lastAltitude;                 ///< Last known altitude
    
    /**
     * @brief Process incoming GPS data from serial
     * @return true if new data was processed
     */
    bool processGPSData();

    /**
     * @brief Update last known position from current GPS data
     */
    void updateLastKnownPosition();
};

#endif // GPS_MANAGER_H
