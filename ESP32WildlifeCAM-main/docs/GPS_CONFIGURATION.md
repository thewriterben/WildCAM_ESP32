# GPS Configuration Guide

## Overview

The WildCAM ESP32 firmware includes comprehensive GPS/GNSS support for location tracking, movement detection, and accurate time synchronization. All GPS settings are centrally configured in `config.h`.

## Configuration Options

### Basic GPS Settings

```cpp
// Enable or disable GPS module
#define GPS_ENABLED true

// GPS module baud rate (typically 9600 for NMEA)
#define GPS_BAUD_RATE 9600

// GPS update interval in milliseconds
#define GPS_UPDATE_INTERVAL 1000

// Timeout waiting for GPS fix (60 seconds)
#define GPS_FIX_TIMEOUT 60000

// Cold start timeout for GPS module (120 seconds)
#define GPS_COLD_START_TIMEOUT 120000
```

### Pin Configuration

GPS pins are automatically configured based on your board type and feature conflicts.

#### AI-Thinker ESP32-CAM

```cpp
#define GPS_RX_PIN 33    // GPIO 33 - Available, no camera conflict
#define GPS_TX_PIN 13    // GPIO 13 - Available (strapping pin)
#define GPS_EN_PIN 12    // GPIO 12 - Conflicts with SD_CS when SD enabled
```

**Pin Conflict Warnings:**
- ❌ Do NOT use GPIO 32 (camera PWDN pin)
- ❌ Do NOT use GPIO 25 (camera VSYNC pin)
- ⚠️ GPIO 12 conflicts with SD_CS when SD card is enabled
- ⚠️ GPIO 13 is a strapping pin (affects boot mode if held low)

#### LilyGO T-Camera Plus S3

```cpp
#define GPS_RX_PIN 16    // GPIO 16 - Available on ESP32-S3
#define GPS_TX_PIN 15    // GPIO 15 - Available on ESP32-S3
#define GPS_EN_PIN 10    // GPIO 10 - GPS enable/power pin
```

#### Other Boards

Default pin configuration:
```cpp
#define GPS_RX_PIN 16    // Default RX pin
#define GPS_TX_PIN 17    // Default TX pin
#define GPS_EN_PIN 18    // Default enable pin
```

### GPS Features

```cpp
// Log GPS coordinates with captured images
#define GPS_LOCATION_LOGGING true

// Detect camera movement/theft
#define GPS_MOVEMENT_DETECTION true

// Movement detection threshold in meters
#define GPS_MOVEMENT_THRESHOLD 5.0

// Enable geofencing alerts
#define GPS_GEOFENCING_ENABLED false

// Use GPS for accurate time synchronization
#define GPS_TIME_SYNC_ENABLED true
```

### Power Management

```cpp
// Enable GPS power saving mode
#define GPS_POWER_SAVE_ENABLED true

// Put GPS in standby after obtaining fix
#define GPS_STANDBY_AFTER_FIX true

// Wake GPS periodically for position updates
#define GPS_PERIODIC_UPDATES true

// Period for GPS updates in power save mode (5 minutes)
#define GPS_UPDATE_PERIOD 300000
```

### Data Storage

```cpp
// Embed GPS coordinates in JPEG EXIF data
#define GPS_EMBED_IN_EXIF true

// Log GPS track for camera movement history
#define GPS_LOG_TRACK true

// Interval for GPS track logging (1 minute)
#define GPS_TRACK_LOG_INTERVAL 60000
```

## Hardware Connection

### Typical GPS Module Wiring

```
GPS Module        ESP32-CAM (AI-Thinker)
----------        -----------------------
VCC       <-->    3.3V or 5V (check module specs)
GND       <-->    GND
TX        <-->    GPIO 33 (GPS_RX_PIN)
RX        <-->    GPIO 13 (GPS_TX_PIN)
EN/PWR    <-->    GPIO 12 (GPS_EN_PIN) [Optional]
```

### Supported GPS Modules

- **NEO-6M**: Basic GPS, cold start ~30s
- **NEO-M8N**: Enhanced GPS with better accuracy
- **NEO-M9N**: Latest generation with faster acquisition
- **PA1010D**: Ultra-low power GPS module
- **GT-U7**: Budget-friendly GPS module

Most GPS modules use NMEA protocol at 9600 baud, which is the default configuration.

## Usage Example

```cpp
#include "config.h"
#include "gps_handler.h"

GPSHandler gps;

void setup() {
    Serial.begin(115200);
    
    // Initialize GPS with configuration from config.h
    if (gps.initialize()) {
        Serial.println("GPS initialized");
        
        // Wait for GPS fix
        if (gps.waitForFix(GPS_FIX_TIMEOUT)) {
            // Get current location
            GPSLocation location = gps.getCurrentLocation();
            Serial.printf("Location: %.6f, %.6f\n", 
                         location.latitude, location.longitude);
        }
    }
}

void loop() {
    // Update GPS location
    if (gps.updateLocation() && gps.isLocationValid()) {
        GPSLocation loc = gps.getCurrentLocation();
        
        // Check for movement
        if (gps.isMoving(GPS_MOVEMENT_THRESHOLD)) {
            Serial.println("Camera moved!");
        }
        
        // Enter power save mode after fix
        if (GPS_STANDBY_AFTER_FIX) {
            gps.enterSleepMode();
        }
    }
    
    delay(GPS_UPDATE_PERIOD);
}
```

## Pin Conflict Resolution

### AI-Thinker ESP32-CAM Limitations

The AI-Thinker ESP32-CAM has limited available GPIO pins due to camera usage. GPS pins were carefully selected to avoid conflicts:

**Available Pins for GPS:**
- GPIO 33 ✅ (GPS_RX_PIN)
- GPIO 13 ⚠️ (GPS_TX_PIN - strapping pin)
- GPIO 12 ⚠️ (GPS_EN_PIN - conflicts with SD)

**Unavailable Pins (Camera):**
- GPIO 0, 4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 34, 35, 36, 39

### Resolving Conflicts

If you experience pin conflicts:

1. **SD Card Conflict (GPIO 12)**:
   ```cpp
   // Option 1: Disable SD card to use GPS_EN_PIN
   #define SD_CARD_ENABLED false
   
   // Option 2: Don't use GPS_EN_PIN (GPS always powered)
   // Comment out GPS_EN_PIN definition
   ```

2. **Strapping Pin Issue (GPIO 13)**:
   - Ensure GPS module doesn't pull GPIO 13 low during boot
   - Add 10kΩ pull-up resistor to GPIO 13
   - Or use alternative pin if available

3. **Alternative Pin Assignments**:
   ```cpp
   // When SD card is disabled:
   #define GPS_EN_PIN 14    // Use SD_CLK_PIN
   #define GPS_TX_PIN 2     // Use SD_MISO_PIN
   ```

## Troubleshooting

### GPS Not Initializing

- Check wiring connections
- Verify GPS module power (3.3V or 5V based on module)
- Ensure GPS_ENABLED is true in config.h
- Check Serial Monitor for initialization messages

### No GPS Fix

- GPS needs clear view of sky (4+ satellites)
- Cold start can take 30-60 seconds
- Increase GPS_FIX_TIMEOUT if needed
- Check GPS module LED indicator (should blink when searching)

### Incorrect Location

- Wait for better fix (more satellites)
- Check antenna connection
- Verify coordinate parsing (latitude/longitude format)

### Boot Issues

- GPIO 13 strapping pin conflict
- Add pull-up resistor to GPIO 13
- Ensure GPS module doesn't interfere with boot

## Advanced Features

### Movement Detection

```cpp
// Set custom movement threshold
gps.setMovementThreshold(10.0);  // 10 meters

// Check if camera moved
if (gps.isMoving()) {
    Serial.println("Theft alert!");
}
```

### Distance Calculations

```cpp
// Calculate distance between two points
double dist = gps.calculateDistance(
    lat1, lon1,  // First location
    lat2, lon2   // Second location
);
Serial.printf("Distance: %.2f meters\n", dist);
```

### Power Management

```cpp
// Enter sleep mode when not needed
gps.enterSleepMode();

// Wake up for location update
gps.exitSleepMode();
gps.updateLocation();
```

## Integration with Other Features

### Image Metadata

GPS coordinates can be automatically embedded in captured images:

```cpp
#define GPS_EMBED_IN_EXIF true
```

### Time Synchronization

GPS provides accurate time for timestamping:

```cpp
#define GPS_TIME_SYNC_ENABLED true
```

### Movement Logging

Track camera position history:

```cpp
#define GPS_LOG_TRACK true
#define GPS_TRACK_LOG_INTERVAL 60000  // 1 minute
```

## See Also

- `examples/gps_basic_example.cpp` - Basic GPS usage
- `src/gps_handler.h` - GPS handler API documentation
- `test/test_gps_config.cpp` - GPS configuration tests
- `config.h` - Complete configuration reference
