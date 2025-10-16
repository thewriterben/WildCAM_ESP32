# GPS Manager Implementation

## Overview
The GPSManager class provides GPS location services for the WildCAM ESP32 platform using the TinyGPS++ library for parsing NMEA sentences from serial GPS modules.

## Features
- Configurable RX/TX pins and baud rate
- Automatic NMEA sentence parsing using TinyGPS++
- GPS fix timeout handling (default: 60 seconds)
- Returns last known position when fix is lost
- Periodic logging of GPS status
- Satellite count tracking

## Files
- `firmware/src/sensors/gps_manager.h` - Header file with class declaration
- `firmware/src/sensors/gps_manager.cpp` - Implementation file

## Usage

### Basic Initialization
```cpp
#include "sensors/gps_manager.h"

GPSManager* gps = new GPSManager();

// Initialize with custom RX/TX pins and baud rate
if (gps->initialize(15, 12, 9600)) {
    Serial.println("GPS initialized successfully");
}
```

### Update GPS Data
```cpp
void loop() {
    // Update GPS data (call frequently to process incoming NMEA sentences)
    gps->update();
    
    // Check if we have a valid fix
    if (gps->hasFix()) {
        float lat = gps->getLatitude();
        float lon = gps->getLongitude();
        float alt = gps->getAltitude();
        int sats = gps->getSatelliteCount();
        
        Serial.printf("Position: %.6f, %.6f, %.1fm (%d satellites)\n",
                     lat, lon, alt, sats);
    }
}
```

### Configure Fix Timeout
```cpp
// Set fix timeout to 2 minutes (120000 ms)
gps->setFixTimeout(120000);
```

## Integration Points

The GPSManager has been integrated into the following components:

### 1. Wildlife Mesh Node (wildlife_mesh_node.ino)
- Initializes GPS manager on startup
- Populates NodeInfo with GPS position if fix is available
- GPS pins: RX=15, TX=12

### 2. Image Mesh (image_mesh.cpp)
- Adds GPS coordinates to image metadata
- Uses global `g_gpsManager` pointer for access
- Automatically includes location if GPS fix is available

### 3. Wildlife Telemetry (wildlife_telemetry.cpp)
- Collects GPS location data for telemetry reports
- Includes satellite count and fix validity
- Reports last known position if fix is lost

## API Reference

### Constructor/Destructor
- `GPSManager()` - Constructor
- `~GPSManager()` - Destructor (cleans up serial resources)

### Initialization
- `bool initialize(int rx_pin, int tx_pin, int baud_rate = 9600)` - Initialize GPS with specified pins and baud rate

### GPS Data Update
- `bool update()` - Read and parse NMEA sentences. Returns true if data was processed.

### Position Access
- `float getLatitude()` - Get latitude in decimal degrees
- `float getLongitude()` - Get longitude in decimal degrees  
- `float getAltitude()` - Get altitude in meters
- `bool hasFix()` - Check if GPS has valid fix
- `int getSatelliteCount()` - Get number of satellites in view

### Configuration
- `void setFixTimeout(unsigned long timeout_ms)` - Set fix timeout duration
- `unsigned long getTimeSinceLastFix()` - Get time since last valid fix

## Dependencies
- TinyGPS++ library (included in platformio.ini)
- Arduino framework
- ESP32 HardwareSerial

## Notes
- Uses HardwareSerial(2) for ESP32 (Serial2)
- Returns last known position when fix times out
- Logs GPS status every 30 seconds when fix is valid
- Logs timeout warnings when fix is lost

## Hardware Requirements
- ESP32-based board
- GPS module with UART/Serial interface
- Proper antenna connection for GPS signal reception
- Common GPS modules: Neo-6M, Neo-7M, Neo-8M

## Example Hardware Connections
```
GPS Module    ESP32
---------     -----
VCC     -->   3.3V or 5V (check module specs)
GND     -->   GND
TX      -->   GPIO 15 (RX_PIN)
RX      -->   GPIO 12 (TX_PIN)
```

## TODO GPS Comments - Resolved
All TODO GPS comments in the codebase have been replaced with GPSManager implementations:
- ✅ wildlife_mesh_node.ino line 351
- ✅ image_mesh.cpp line 130
- ✅ wildlife_telemetry.cpp line 701
