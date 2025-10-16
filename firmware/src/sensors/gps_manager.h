/**
 * @file gps_manager.h
 * @brief GPS integration module for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides GPS functionality for location tracking and satellite positioning
 */

#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>

/**
 * @class GPSManager
 * @brief GPS sensor manager for location tracking and satellite positioning
 */
class GPSManager {
public:
    /**
     * @brief Initialize GPS module with specified pins and baud rate
     * @param rx_pin RX pin number for GPS serial communication
     * @param tx_pin TX pin number for GPS serial communication
     * @param baud_rate Baud rate for GPS serial communication (typically 9600)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int rx_pin, int tx_pin, int baud_rate);
    
    /**
     * @brief Update GPS data by reading and parsing from GPS module
     * @return true if new valid data received, false otherwise
     */
    bool update();
    
    /**
     * @brief Get current latitude coordinate
     * @return Latitude in decimal degrees (positive = North, negative = South)
     */
    float getLatitude();
    
    /**
     * @brief Get current longitude coordinate
     * @return Longitude in decimal degrees (positive = East, negative = West)
     */
    float getLongitude();
    
    /**
     * @brief Get current altitude above sea level
     * @return Altitude in meters
     */
    float getAltitude();
    
    /**
     * @brief Check if GPS has valid position fix
     * @return true if GPS has valid fix, false otherwise
     */
    bool hasFix();
    
    /**
     * @brief Get number of satellites currently in view
     * @return Number of satellites being tracked
     */
    int getSatelliteCount();
};

#endif // GPS_MANAGER_H
