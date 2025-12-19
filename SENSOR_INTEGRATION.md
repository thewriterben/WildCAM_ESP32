# WildCAM ESP32 - Sensor Integration Guide

This guide covers the integration of additional environmental sensors with the WildCAM ESP32 wildlife camera system.

## Supported Sensors

| Sensor | Type | Interface | Purpose |
|--------|------|-----------|---------|
| **BME280** | Environmental | I2C | Temperature, humidity, pressure |
| **GPS (NEO-6M)** | Location | UART | Geo-tagging images |
| **BH1750** | Light | I2C | Day/night detection |
| **LDR/Photoresistor** | Light | Analog | Simple light level |

## Quick Start

### 1. Enable Sensors in Configuration

Edit `include/config.h` to enable the sensors you want to use:

```cpp
// Enable BME280 temperature/humidity/pressure sensor
#define SENSOR_BME280_ENABLED

// Enable GPS location tagging (optional)
#define SENSOR_GPS_ENABLED

// Enable light sensor for day/night detection
#define SENSOR_LIGHT_ENABLED
```

### 2. Build with Sensor Support

Use the pre-configured PlatformIO environments:

```bash
# Basic sensors (BME280 + Light)
pio run -e esp32cam_sensors

# All sensors including GPS
pio run -e esp32cam_sensors_gps
```

### 3. Wiring

See the [Wiring Diagrams](#wiring-diagrams) section below.

---

## BME280 Environmental Sensor

The BME280 provides accurate temperature, humidity, and atmospheric pressure readings. This data is automatically included in image metadata.

### Features

- **Temperature:** -40°C to +85°C, ±1°C accuracy
- **Humidity:** 0-100% RH, ±3% accuracy
- **Pressure:** 300-1100 hPa, ±1 hPa accuracy
- **Altitude:** Calculated from pressure (requires sea-level calibration)

### Wiring (I2C)

| BME280 Pin | ESP32-CAM Pin | Notes |
|------------|---------------|-------|
| VCC | 3.3V | **Do not use 5V!** |
| GND | GND | |
| SDA | GPIO 14 | I2C Data |
| SCL | GPIO 15 | I2C Clock |

### Configuration

```cpp
// In config.h
#define SENSOR_BME280_ENABLED
#define SENSOR_BME280_ADDRESS 0x76    // Use 0x77 if SDO pin is high
#define SENSOR_I2C_SDA_PIN 14
#define SENSOR_I2C_SCL_PIN 15
```

### Metadata Output

When BME280 is enabled, image metadata includes:

```json
{
  "environment": {
    "temperature": 23.5,
    "humidity": 65.2,
    "pressure": 1013.25,
    "altitude": 120.5
  }
}
```

---

## GPS Location Tagging

Add GPS coordinates to captured wildlife images for mapping and spatial analysis.

### Supported Modules

- NEO-6M, NEO-7M, NEO-8M (most common)
- ATGM336H
- Any NMEA-compatible GPS module

### Features

- Latitude/longitude in decimal degrees
- Altitude above sea level
- Satellite count and signal quality (HDOP)
- Automatic fix detection

### Wiring (UART)

| GPS Pin | ESP32-CAM Pin | Notes |
|---------|---------------|-------|
| VCC | 3.3V or 5V | Check module specs |
| GND | GND | |
| TX | GPIO 16 | GPS TX → ESP32 RX |
| RX | GPIO 17 | Optional (for configuration) |

### Configuration

```cpp
// In config.h
#define SENSOR_GPS_ENABLED
#define SENSOR_GPS_RX_PIN 16
#define SENSOR_GPS_TX_PIN 17
#define SENSOR_GPS_BAUD 9600    // Most GPS modules use 9600
```

### Getting a GPS Fix

GPS modules require a clear view of the sky for satellite reception:

- **Cold start:** 1-5 minutes for first fix
- **Warm start:** 30 seconds - 1 minute
- **Hot start:** < 10 seconds

The system will continue to capture images even without GPS fix. When fix is available, coordinates are added to metadata.

### Metadata Output

When GPS has a valid fix, image metadata includes:

```json
{
  "gps": {
    "latitude": 45.123456,
    "longitude": -122.654321,
    "altitude": 150.5,
    "satellites": 8
  }
}
```

---

## Light Sensor (Day/Night Detection)

Detect ambient light levels for adaptive camera behavior.

### Option 1: BH1750 (I2C) - Recommended

Digital light sensor with precise lux readings.

#### Wiring

| BH1750 Pin | ESP32-CAM Pin | Notes |
|------------|---------------|-------|
| VCC | 3.3V | |
| GND | GND | |
| SDA | GPIO 14 | Shared I2C bus |
| SCL | GPIO 15 | Shared I2C bus |
| ADDR | GND | Address 0x23 (or VCC for 0x5C) |

#### Configuration

```cpp
// In config.h
#define SENSOR_LIGHT_ENABLED
#define SENSOR_BH1750_ADDRESS 0x23
// Don't define SENSOR_LIGHT_ANALOG_PIN to use I2C
```

### Option 2: LDR/Photoresistor (Analog)

Simple analog light sensing using a photoresistor.

#### Wiring

```
3.3V ─────┬───── LDR ───────┬──── GND
          │                  │
          └── GPIO 32 ──────┘
               (through 10kΩ)
```

#### Configuration

```cpp
// In config.h
#define SENSOR_LIGHT_ENABLED
#define SENSOR_LIGHT_ANALOG_PIN 32
```

### Light Conditions

| Lux Level | Condition | Notes |
|-----------|-----------|-------|
| < 10 | Night | Very dark |
| 10 - 100 | Twilight | Dawn/dusk |
| 100 - 1000 | Cloudy | Overcast day |
| 1000 - 10000 | Daylight | Normal day |
| > 10000 | Bright Sun | Direct sunlight |

### Metadata Output

```json
{
  "light": {
    "lux": 5420.5,
    "is_daytime": true
  }
}
```

---

## Wiring Diagrams

### Full Sensor Suite Wiring

```
                   ESP32-CAM
                  ┌─────────┐
                  │         │
        3.3V ────►│ 3V3     │
        GND  ────►│ GND     │
                  │         │
  BME280/BH1750   │         │
  SDA  ─────────►│ GPIO 14 │──── I2C SDA
  SCL  ─────────►│ GPIO 15 │──── I2C SCL
                  │         │
  GPS Module      │         │
  TX   ─────────►│ GPIO 16 │──── UART RX
  RX   ◄─────────│ GPIO 17 │──── UART TX (optional)
                  │         │
  PIR Sensor      │         │
  OUT  ─────────►│ GPIO 13 │──── Motion detect
                  │         │
                  └─────────┘
```

### I2C Address Reference

| Device | Default Address | Alternate |
|--------|-----------------|-----------|
| BME280 | 0x76 | 0x77 |
| BH1750 | 0x23 | 0x5C |

---

## API Reference

### SensorManager Class

```cpp
#include "SensorManager.h"

SensorManager sensors;

// Initialize all enabled sensors
bool success = sensors.init();

// Read environmental data (BME280)
EnvironmentalData env = sensors.readEnvironmental();
if (env.valid) {
    Serial.printf("Temperature: %.1f°C\n", env.temperature);
    Serial.printf("Humidity: %.1f%%\n", env.humidity);
    Serial.printf("Pressure: %.1f hPa\n", env.pressure);
}

// Read GPS data
GPSData gps = sensors.readGPS();
if (gps.hasFix) {
    Serial.printf("Location: %.6f, %.6f\n", gps.latitude, gps.longitude);
}

// Read light level
LightData light = sensors.readLight();
if (light.valid) {
    Serial.printf("Light: %.1f lux (%s)\n", 
                  light.lux, light.isDaytime ? "Day" : "Night");
}

// Get current light condition
LightCondition condition = sensors.getLightCondition();
// Returns: NIGHT, TWILIGHT, CLOUDY, DAYLIGHT, or BRIGHT_SUN

// Print sensor status
sensors.printStatus();
```

### Data Structures

```cpp
// Environmental data from BME280
struct EnvironmentalData {
    float temperature;      // Celsius
    float humidity;         // Percentage
    float pressure;         // hPa
    float altitude;         // Meters
    bool valid;
    unsigned long timestamp;
};

// GPS location data
struct GPSData {
    float latitude;         // Decimal degrees
    float longitude;        // Decimal degrees
    float altitude;         // Meters
    int satellites;
    float hdop;
    bool hasFix;
    unsigned long timestamp;
};

// Light sensor data
struct LightData {
    float lux;
    bool isDaytime;
    bool valid;
    unsigned long timestamp;
};
```

---

## Troubleshooting

### BME280 Not Detected

1. Check wiring - ensure VCC is 3.3V, not 5V
2. Verify I2C address (try both 0x76 and 0x77)
3. Check for I2C conflicts with other devices
4. Use I2C scanner to verify device presence

### GPS Not Getting Fix

1. Move to open area with clear sky view
2. Wait 2-5 minutes for cold start
3. Check antenna connection (external antenna may help)
4. Verify baud rate setting (default 9600)

### Light Sensor Readings Incorrect

**BH1750:**
- Check I2C wiring and address
- Verify sensor is not covered or obstructed

**LDR:**
- Verify voltage divider resistor value
- Calibrate in known lighting conditions
- Check ADC pin assignment

### I2C Bus Issues

- Only use 3.3V for I2C devices
- Add 4.7kΩ pull-up resistors on SDA/SCL if needed
- Keep I2C wires short (< 30cm)
- Don't share I2C bus with noisy devices

---

## Power Considerations

| Sensor | Typical Current | Sleep Mode |
|--------|-----------------|------------|
| BME280 | 1.8 µA (sleep), 350 µA (measuring) | Yes |
| BH1750 | 120 µA (active), 1 µA (sleep) | Yes |
| NEO-6M GPS | 45 mA (tracking), 11 mA (standby) | Partial |
| LDR | < 1 mA | N/A |

**Tips for Battery Operation:**

- Disable unused sensors to save power
- Use GPS only when needed (disable with `SENSOR_GPS_ENABLED`)
- BME280 and BH1750 have minimal impact on battery life
- Consider GPS backup mode for faster subsequent fixes

---

## Example: Complete Sensor Setup

```cpp
// main.cpp
#include "SensorManager.h"

SensorManager sensors;

void setup() {
    Serial.begin(115200);
    
    // Initialize sensor manager
    if (sensors.init()) {
        Serial.println("Sensors initialized");
        sensors.printStatus();
    }
    
    // Set custom thresholds
    sensors.setDaytimeThreshold(50.0);  // 50 lux threshold
    sensors.setSeaLevelPressure(1013.25);  // For altitude calculation
}

void loop() {
    // Update GPS continuously
    sensors.updateGPS();
    
    // Read sensors every minute
    static unsigned long lastRead = 0;
    if (millis() - lastRead > 60000) {
        EnvironmentalData env = sensors.readEnvironmental();
        GPSData gps = sensors.readGPS();
        LightData light = sensors.readLight();
        
        // Use sensor data...
        
        lastRead = millis();
    }
}
```

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024 | Initial sensor support (BME280, GPS, Light) |

---

## See Also

- [Main README](README.md) - Project overview
- [SENSOR_SUPPORT_SUMMARY.md](SENSOR_SUPPORT_SUMMARY.md) - Detailed sensor framework
- [config.h](include/config.h) - Configuration options
- [Hardware Requirements](HARDWARE_REQUIREMENTS.md) - Complete component list
