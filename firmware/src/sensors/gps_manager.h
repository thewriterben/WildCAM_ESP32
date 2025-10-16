/**
 * @file gps_manager.h

 */

#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

/**
 * @class GPSManager

 */
class GPSManager {
public:
    /**

    /**
     * @brief Check if GPS has a valid fix
     * @return true if GPS has valid fix, false otherwise
     */
    bool hasFix();

};

#endif // GPS_MANAGER_H
