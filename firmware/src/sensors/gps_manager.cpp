/**
 * @file gps_manager.cpp
 */

#include "gps_manager.h"


}

GPSManager::~GPSManager() {
    if (gpsSerial != nullptr) {
        gpsSerial->end();
        delete gpsSerial;
        gpsSerial = nullptr;
    }
}

bool GPSManager::initialize(int rx_pin, int tx_pin, int baud_rate) {

    
    return true;
}

bool GPSManager::update() {
    if (!initialized || gpsSerial == nullptr) {
        return false;
    }

    if (gps.location.isValid()) {
        lastLatitude = (float)gps.location.lat();
        lastLongitude = (float)gps.location.lng();
    }
    
    if (gps.altitude.isValid()) {
        lastAltitude = (float)gps.altitude.meters();
    }
}
