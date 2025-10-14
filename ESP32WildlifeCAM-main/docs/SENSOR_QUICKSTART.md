# WildCAM Sensor Integration Quick Start Guide

Get started with adding environmental, distance, and gas sensors to your WildCAM ESP32 wildlife monitoring system in minutes!

## Quick Overview

The new sensor abstraction layer lets you easily add various sensor types to enhance your wildlife monitoring capabilities:

- 🌡️ **Temperature/Humidity**: DHT11, DHT22, BME280
- 📏 **Distance**: HC-SR04 ultrasonic, VL53L0X laser
- 💨 **Gas/Air Quality**: MQ-2, MQ-7, MQ-135, SGP30
- 🔦 **Light**: BH1750, TSL2591
- ⚡ **Power**: MAX17048 battery monitor
- 📍 **Motion**: PIR, thermal imaging

## 5-Minute Setup: DHT22 Temperature Sensor

### What You Need
- ESP32-CAM or ESP32-S3-CAM board
- DHT22 sensor
- 3 jumper wires

### Wiring
```
DHT22 Pin 1 (VCC)  → ESP32 3.3V
DHT22 Pin 2 (DATA) → ESP32 GPIO 15
DHT22 Pin 4 (GND)  → ESP32 GND
```

### Code
```cpp
#include "firmware/src/sensors/sensor_interface.h"
#include "firmware/src/sensors/dht_sensor.h"

DHTSensor dht(15, DHTType::DHT22);

void setup() {
    Serial.begin(115200);
    dht.init();
}

void loop() {
    if (dht.dataAvailable()) {
        SensorReading reading = dht.read();
        if (reading.valid) {
            Serial.printf("Temp: %.1f°C, Humidity: %.1f%%\n", 
                         reading.values[0], reading.values[1]);
        }
    }
    delay(2000);
}
```

That's it! You now have temperature and humidity monitoring.

## 10-Minute Setup: Wildlife Proximity Detection

Add ultrasonic distance sensing to detect when animals approach your camera.

### What You Need
- ESP32-CAM board (with DHT22 from above)
- HC-SR04 ultrasonic sensor
- 4 jumper wires

### Additional Wiring
```
HC-SR04 VCC     → ESP32 5V
HC-SR04 GND     → ESP32 GND
HC-SR04 TRIGGER → ESP32 GPIO 12
HC-SR04 ECHO    → ESP32 GPIO 14
```

### Enhanced Code
```cpp
#include "firmware/src/sensors/sensor_interface.h"
#include "firmware/src/sensors/dht_sensor.h"
#include "firmware/src/sensors/ultrasonic_sensor.h"

DHTSensor dht(15, DHTType::DHT22);
UltrasonicSensor ultrasonic(12, 14);  // trigger, echo
SensorRegistry sensors;

void setup() {
    Serial.begin(115200);
    
    // Register sensors
    dht.init();
    ultrasonic.init();
    sensors.registerSensor(&dht);
    sensors.registerSensor(&ultrasonic);
    
    Serial.println("Wildlife monitoring active!");
}

void loop() {
    // Check for wildlife proximity
    SensorReading distance = ultrasonic.read();
    if (distance.valid && distance.values[0] < 100.0f) {
        Serial.println("🦌 Wildlife detected!");
        
        // Capture environment data
        SensorReading env = dht.read();
        if (env.valid) {
            Serial.printf("Environment: %.1f°C, %.1f%% RH\n",
                         env.values[0], env.values[1]);
        }
        
        // Here you would trigger camera capture
    }
    
    delay(100);
}
```

Now your camera knows when animals are nearby and can capture environmental context!

## 15-Minute Setup: Complete Environmental Station

Build a full monitoring station with air quality sensing.

### What You Need
- Previous components (DHT22 + HC-SR04)
- MQ-135 air quality sensor
- 3 jumper wires

### Additional Wiring
```
MQ-135 VCC  → ESP32 5V
MQ-135 GND  → ESP32 GND
MQ-135 AOUT → ESP32 GPIO 34 (analog pin)
```

### Complete Code
```cpp
#include "firmware/src/sensors/sensor_interface.h"
#include "firmware/src/sensors/dht_sensor.h"
#include "firmware/src/sensors/ultrasonic_sensor.h"
#include "firmware/src/sensors/mq_sensor.h"

DHTSensor dht(15, DHTType::DHT22);
UltrasonicSensor ultrasonic(12, 14);
MQSensor airQuality(34, MQType::MQ135);
SensorRegistry sensors;

void setup() {
    Serial.begin(115200);
    
    // Initialize all sensors
    dht.init();
    ultrasonic.init();
    airQuality.init();
    
    // Calibrate air quality sensor in clean air
    if (airQuality.needsCalibration()) {
        Serial.println("Calibrating air quality sensor...");
        airQuality.calibrate();
        Serial.println("Calibration complete!");
    }
    
    // Register sensors
    sensors.registerSensor(&dht);
    sensors.registerSensor(&ultrasonic);
    sensors.registerSensor(&airQuality);
    
    Serial.printf("Monitoring with %d sensors\n", sensors.getSensorCount());
}

void loop() {
    // Read all sensors at once
    auto readings = sensors.readAll();
    
    Serial.println("=== Environmental Status ===");
    for (const auto& reading : readings) {
        if (reading.valid) {
            Serial.print(sensorTypeToString(reading.sensor_type));
            Serial.print(": ");
            for (int i = 0; i < reading.value_count; i++) {
                Serial.print(reading.values[i]);
                Serial.print(" ");
            }
            Serial.println(reading.unit);
        }
    }
    Serial.println();
    
    delay(5000);
}
```

## Common Sensor Configurations

### Research Station (Maximum Data)
```
✓ DHT22 (temp/humidity)
✓ BME280 (temp/humidity/pressure) 
✓ BH1750 (light)
✓ HC-SR04 (distance)
✓ MQ-135 (air quality)
✓ MAX17048 (battery)
```

### Budget Setup
```
✓ DHT11 (basic temp/humidity)
✓ PIR (motion)
✓ Analog light sensor
```

### Long-Range Detection
```
✓ HC-SR04 ultrasonic (up to 4m)
✓ PIR motion
✓ DHT22 (environment)
```

### Fire Monitoring
```
✓ MQ-2 (smoke detection)
✓ DHT22 (temperature)
✓ Thermal camera (MLX90640)
```

## Sensor Selection Guide

### Temperature Sensors
| Sensor | Accuracy | Range | Cost | When to Use |
|--------|----------|-------|------|-------------|
| DHT11 | ±2°C | 0-50°C | $ | Budget projects |
| DHT22 | ±0.5°C | -40-80°C | $$ | Most wildlife monitoring |
| DS18B20 | ±0.5°C | -55-125°C | $$ | Water/extreme temps |
| BME280 | ±1°C | -40-85°C | $$$ | Also need pressure/altitude |

### Distance Sensors
| Sensor | Range | Accuracy | When to Use |
|--------|-------|----------|-------------|
| HC-SR04 | 2-400cm | ±3mm | Wide-beam detection |
| VL53L0X | 3-200cm | ±3% | Precise targeting |
| VL53L1X | 4-400cm | ±2% | Long-range precise |

### Gas Sensors
| Sensor | Detects | Applications |
|--------|---------|--------------|
| MQ-2 | Smoke, LPG, CO | Fire detection |
| MQ-7 | Carbon monoxide | Safety monitoring |
| MQ-135 | Air quality, CO2 | General air quality |
| SGP30 | TVOC, eCO2 | Indoor air quality |

## GPIO Pin Reference

### ESP32-S3-CAM Safe Pins
| Pin | Type | Recommended Use |
|-----|------|-----------------|
| GPIO 1 | I2C | SDA for I2C sensors |
| GPIO 2 | I2C | SCL for I2C sensors |
| GPIO 12-16 | Digital | DHT, PIR, ultrasonic |
| GPIO 33 | Digital | General digital I/O |
| GPIO 34-39 | Analog Input | MQ sensors, LDR |

**Avoid**: GPIOs used by camera (see hardware docs)

## Troubleshooting

### Sensor Not Detected
```cpp
if (!sensor.init()) {
    Serial.print("Error: ");
    Serial.println(sensor.getLastError());
    
    // Check health
    Serial.printf("Health: %d%%\n", sensor.getHealthScore());
    
    // Test communication
    if (!sensor.testCommunication()) {
        Serial.println("Check wiring!");
    }
}
```

### Invalid Readings
```cpp
SensorReading reading = sensor.read();
if (!reading.valid) {
    Serial.printf("Status: %s\n", statusToString(reading.status));
    // STATUS_DISCONNECTED: Check wiring
    // STATUS_ERROR: Sensor malfunction
    // STATUS_INITIALIZING: Wait for warmup
}
```

### DHT Sensor Issues
- Add 10kΩ pull-up resistor on data line
- Wait 2 seconds between readings (DHT22)
- Power cycle if stuck in error state

### HC-SR04 Issues
- Needs 5V power (not 3.3V)
- Keep sensor clear of obstacles
- 60ms minimum between readings

### MQ Sensor Issues
- Requires 20-48 hour initial preheat
- Needs 20-60 second warm-up after power on
- Must calibrate in clean air
- Consumes ~750mW (plan power budget)

## Power Considerations

### Current Draw
| Sensor | Active | Sleep | Notes |
|--------|--------|-------|-------|
| DHT22 | 1.5mA | - | No sleep mode |
| HC-SR04 | 15mA | - | Pulse only |
| MQ-135 | 150mA | - | Heater always on |
| BME280 | 0.7mA | 0.1μA | Sleep capable |

### Battery Life Tips
1. Use sensors with sleep modes when possible
2. Reduce MQ sensor poll frequency
3. Power sensors via GPIO for on-demand operation
4. Use I2C sensors (lower power)

## Complete Examples

Full working examples are in `examples/` directory:
- `basic_sensor_reading/` - Simple single sensor
- `multi_sensor_integration/` - Multiple sensors working together

## Next Steps

1. **Test Your Setup**: Run examples to verify wiring
2. **Customize Thresholds**: Adjust detection values for your use case
3. **Add Data Logging**: Save readings to SD card
4. **Integrate with Camera**: Trigger captures on events
5. **Add Connectivity**: Send data via WiFi/LoRa

## Additional Resources

- **Full API Documentation**: `firmware/src/sensors/README_SENSORS.md`
- **Example Guide**: `examples/README_SENSOR_EXAMPLES.md`
- **Hardware Specs**: Individual sensor datasheets
- **Community**: GitHub issues for support

## Ready-Made Configurations

### Copy-Paste Sensor Setup

**Wildlife Trail Camera**:
```cpp
DHTSensor dht(15, DHTType::DHT22);           // Environment
UltrasonicSensor proximity(12, 14);          // Trigger
PIR motion(13);                               // Motion backup
```

**Fire Watch Station**:
```cpp
DHTSensor dht(15, DHTType::DHT22);           // Temperature
MQSensor smoke(34, MQType::MQ2);             // Smoke detection
BH1750 light;                                 // Light level
```

**Research Station**:
```cpp
BME280 environment;                           // Temp/Humidity/Pressure
TSL2591 light;                                // Light spectrum
SGP30 airQuality;                             // Air quality
MAX17048 battery;                             // Power monitoring
```

## Get Started Now!

1. Pick a sensor configuration above
2. Wire it up (5 minutes)
3. Copy example code
4. Upload and test
5. Start monitoring wildlife!

Questions? Check the full documentation or open a GitHub issue.

Happy monitoring! 🦌📷🌲
