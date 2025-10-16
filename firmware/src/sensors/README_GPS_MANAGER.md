# GPS Manager for ESP32 WildCAM

## Overview

The GPS Manager provides location tracking capabilities for the ESP32 Wildlife Camera system. It uses the TinyGPS++ library to parse NMEA sentences from GPS modules and provides an easy-to-use interface for accessing location data.

## Features

- **Easy Initialization**: Simple setup with configurable RX/TX pins and baud rate
- **NMEA Parsing**: Uses TinyGPS++ for robust NMEA sentence parsing
- **Fix Status Tracking**: Monitors GPS fix status and satellite count
- **Last Known Position**: Maintains last known position when fix is lost
- **Rich Data Access**: Provides latitude, longitude, altitude, speed, course, and HDOP
- **Non-blocking Updates**: Update method doesn't block, suitable for main loop
- **Timeout Support**: Optional blocking wait for fix with timeout

## Configuration

GPS settings are defined in `config.h`:

```cpp
// GPS Module Enable/Disable
#define GPS_ENABLED true                 // Enable GPS module for location tracking

// GPS Serial Communication Pins
#define GPS_RX_PIN 32                    // GPS module TX -> ESP32 RX (GPIO 32)
#define GPS_TX_PIN 33                    // GPS module RX -> ESP32 TX (GPIO 33)

// GPS Module Configuration
#define GPS_BAUD_RATE 9600               // GPS serial baud rate
#define GPS_UPDATE_RATE 1000             // ms - GPS position update rate
#define GPS_FIX_TIMEOUT 60000            // ms - timeout waiting for GPS fix

// GPS Feature Configuration
#define GPS_USE_TINYGPS_PLUS true        // Use TinyGPS++ library
#define GPS_SAVE_LAST_POSITION true      // Save last known position on fix loss
#define GPS_ALTITUDE_ENABLED true        // Include altitude in GPS data
#define GPS_SATELLITE_COUNT_ENABLED true // Track satellites in view
```

### Pin Configuration Notes

- **GPIO 32 (RX)**: Receives data from GPS module TX
- **GPIO 33 (TX)**: Sends data to GPS module RX
- These pins are available on AI-Thinker ESP32-CAM and don't conflict with camera pins
- GPIO 25 can optionally be used for GPS power control

### Supported GPS Modules

The GPS Manager works with any GPS module that outputs NMEA sentences over serial UART:

- **Recommended**: 
  - NEO-6M / NEO-7M / NEO-8M (u-blox)
  - GT-U7 / GY-GPS6MV2
  - PA1010D (Adafruit Mini GPS)
  
- **Baud Rate**: Most modules use 9600 baud (configurable)
- **Protocol**: Standard NMEA 0183 sentences (GGA, RMC, etc.)

## Usage

### Basic Setup

```cpp
#include "sensors/gps_manager.h"
#include "config.h"

// Global GPS manager instance
GPSManager* gpsManager = nullptr;

void setup() {
    Serial.begin(115200);
    
    // Create GPS manager
    gpsManager = new GPSManager();
    
    // Initialize with config values
    if (gpsManager->initialize(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE)) {
        Serial.println("GPS initialized successfully!");
    }
}

void loop() {
    // Update GPS data (call regularly)
    gpsManager->update();
    
    // Check for fix and use data
    if (gpsManager->hasFix()) {
        float lat = gpsManager->getLatitude();
        float lon = gpsManager->getLongitude();
        // Use GPS data...
    }
}
```

### Waiting for Fix

```cpp
// Block until fix acquired or timeout
if (gpsManager->waitForFix(60000)) {  // 60 second timeout
    Serial.println("GPS fix acquired!");
    Serial.printf("Location: %.6f, %.6f\n", 
                  gpsManager->getLatitude(), 
                  gpsManager->getLongitude());
}
```

### Getting GPS Data

```cpp
// Location
float latitude = gpsManager->getLatitude();      // Decimal degrees
float longitude = gpsManager->getLongitude();    // Decimal degrees
float altitude = gpsManager->getAltitude();      // Meters above sea level

// Status
bool hasFix = gpsManager->hasFix();              // Fix status
int satellites = gpsManager->getSatelliteCount(); // Number of satellites

// Movement
float speed = gpsManager->getSpeedKmph();        // Speed in km/h
float course = gpsManager->getCourse();          // Course in degrees (0-360)

// Quality
float hdop = gpsManager->getHDOP();              // Horizontal dilution of precision

// Formatted string
String location = gpsManager->getLocationString(); // "lat,lon,alt"
```

### Image Tagging Example

```cpp
void captureImage() {
    // Capture image with camera
    camera_fb_t* fb = esp_camera_fb_get();
    
    // Add GPS metadata if available
    if (gpsManager->hasFix()) {
        ImageMetadata metadata;
        metadata.latitude = gpsManager->getLatitude();
        metadata.longitude = gpsManager->getLongitude();
        metadata.altitude = gpsManager->getAltitude();
        metadata.satellites = gpsManager->getSatelliteCount();
        
        // Save image with metadata
        saveImageWithMetadata(fb, metadata);
    } else {
        // Save without GPS data
        saveImage(fb);
    }
    
    esp_camera_fb_return(fb);
}
```

## API Reference

### Initialization

#### `bool initialize(int rx_pin, int tx_pin, int baud_rate = 9600)`
Initialize GPS module with specified pins and baud rate.

**Parameters:**
- `rx_pin`: GPIO pin for RX (GPS TX -> ESP32 RX)
- `tx_pin`: GPIO pin for TX (GPS RX -> ESP32 TX)
- `baud_rate`: Serial baud rate (default: 9600)

**Returns:** `true` if successful, `false` otherwise

### Update

#### `bool update()`
Read and parse incoming GPS data. Call this regularly in main loop.

**Returns:** `true` if new data was processed, `false` otherwise

### Location Access

#### `float getLatitude()`
Get current latitude in decimal degrees (-90.0 to +90.0).
Returns last known position if fix is lost and `GPS_SAVE_LAST_POSITION` is enabled.

#### `float getLongitude()`
Get current longitude in decimal degrees (-180.0 to +180.0).
Returns last known position if fix is lost and `GPS_SAVE_LAST_POSITION` is enabled.

#### `float getAltitude()`
Get altitude in meters above sea level.
Returns last known altitude if fix is lost and `GPS_SAVE_LAST_POSITION` is enabled.

### Status

#### `bool hasFix()`
Check if GPS has a valid fix.

**Returns:** `true` if valid fix, `false` otherwise

#### `int getSatelliteCount()`
Get number of satellites currently in view.

**Returns:** Number of satellites (0-255)

#### `unsigned long getTimeSinceLastFix()`
Get time elapsed since last valid GPS fix.

**Returns:** Milliseconds since last fix, or 0 if no fix ever obtained

#### `bool isInitialized()`
Check if GPS manager is initialized.

**Returns:** `true` if initialized, `false` otherwise

### Additional Data

#### `float getHDOP()`
Get horizontal dilution of precision. Lower values indicate better precision.

**Returns:** HDOP value (typical range: 1-20)

#### `float getSpeedKmph()`
Get speed over ground in kilometers per hour.

**Returns:** Speed in km/h

#### `float getCourse()`
Get course over ground in degrees.

**Returns:** Course (0-360 degrees)

#### `String getLocationString()`
Get formatted location string.

**Returns:** String in format "lat,lon,alt" or "No GPS fix" if no fix

### Utility

#### `bool waitForFix(unsigned long timeout_ms = 60000)`
Block and wait for GPS fix with timeout.

**Parameters:**
- `timeout_ms`: Maximum time to wait in milliseconds (default: 60000)

**Returns:** `true` if fix acquired, `false` if timeout

## Integration with WildCAM Firmware

### Main Application

The GPS Manager is integrated into the main firmware and made available as a global instance:

```cpp
// In main.cpp or similar
GPSManager* gpsManager = nullptr;

void setup() {
    // Initialize GPS if enabled
    #if GPS_ENABLED
    gpsManager = new GPSManager();
    gpsManager->initialize(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD_RATE);
    #endif
}
```

### Image Mesh Integration

The `image_mesh.cpp` module uses GPS data for image metadata:

```cpp
// Automatically adds GPS coordinates to image metadata
if (gpsManager != nullptr && gpsManager->hasFix()) {
    metadata.latitude = gpsManager->getLatitude();
    metadata.longitude = gpsManager->getLongitude();
}
```

### Wildlife Telemetry Integration

The `wildlife_telemetry.cpp` module uses GPS for location tracking:

```cpp
// Collects complete GPS data for telemetry
location.latitude = gpsManager->getLatitude();
location.longitude = gpsManager->getLongitude();
location.altitude = gpsManager->getAltitude();
location.satellites = gpsManager->getSatelliteCount();
location.fixValid = gpsManager->hasFix();
```

## Hardware Setup

### Wiring

Connect GPS module to ESP32-CAM:

```
GPS Module    ESP32-CAM
----------    ---------
VCC       ->  3.3V or 5V (check module specs)
GND       ->  GND
TX        ->  GPIO 32 (GPS_RX_PIN)
RX        ->  GPIO 33 (GPS_TX_PIN)
```

### Power Considerations

- GPS modules typically draw 20-40mA when acquiring fix
- After fix, power consumption drops to 15-25mA
- Consider adding a power control circuit on GPIO 25 for battery operation
- Most GPS modules work with 3.3V logic levels (ESP32 compatible)

### Antenna Placement

- GPS requires clear view of sky for best performance
- Ceramic patch antennas work well for stationary installations
- External active antennas provide better performance in challenging locations
- Keep antenna away from metal objects and RF interference sources

## Performance

### Fix Time

- **Cold start**: 30-60 seconds (no almanac data)
- **Warm start**: 25-35 seconds (recent almanac)
- **Hot start**: 1-10 seconds (recent fix and valid almanac)

### Accuracy

- **Horizontal**: Typically 2-5 meters (depends on satellite count and HDOP)
- **Vertical**: Typically 5-10 meters
- **Speed**: ±0.1 m/s
- **Time**: ±1 microsecond when synced

### Update Rate

- Default: 1 Hz (1 update per second)
- Configurable up to 5-10 Hz on newer modules
- ESP32 can easily handle 1 Hz updates without performance impact

## Troubleshooting

### No GPS Fix

1. **Check antenna placement**: Needs clear view of sky
2. **Verify wiring**: TX/RX pins might be swapped
3. **Check baud rate**: Most modules use 9600, some use 115200
4. **Wait longer**: Initial fix can take 60+ seconds
5. **Check power**: GPS modules need stable 3.3V/5V supply

### Incorrect Data

1. **Verify pin configuration**: Check GPS_RX_PIN and GPS_TX_PIN in config.h
2. **Check NMEA output**: Use serial monitor to verify GPS is sending data
3. **Verify module type**: Some modules need configuration commands

### High Power Consumption

1. **Use power control pin**: Implement GPS sleep when not needed
2. **Reduce update rate**: Lower update frequency saves power
3. **Use assisted GPS**: A-GPS can reduce fix time and power

## Future Enhancements

Potential future improvements:

- [ ] A-GPS support for faster fix
- [ ] GPS power management (sleep/wake cycles)
- [ ] Geofencing capabilities
- [ ] Motion detection using GPS
- [ ] Support for GLONASS/Galileo/BeiDou
- [ ] Dead reckoning during GPS outages

## Dependencies

- **TinyGPS++**: NMEA parsing library (included in platformio.ini)
- **HardwareSerial**: ESP32 hardware serial (built-in)
- **Arduino**: Arduino framework for ESP32

## License

Part of the ESP32 WildCAM project. See main project LICENSE file.

## Support

For issues or questions:
- Check the examples in `firmware/examples/`
- Review test cases in `firmware/test/test_gps_manager.cpp`
- Open an issue on the project repository
