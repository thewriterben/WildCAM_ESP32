# WildCAM ESP32 Sensor Integration Guide

This guide describes the sensor abstraction layer and supported sensor types in the WildCAM ESP32 firmware.

## Overview

The sensor abstraction layer provides a unified interface for working with different types of sensors. This makes it easy to add new sensors, swap sensor types, and manage multiple sensors in a consistent way.

## Architecture

### Base Sensor Interface (`sensor_interface.h`)

All sensors implement the `BaseSensor` abstract base class which provides:

- **Identification**: Get sensor type, name, and capabilities
- **Initialization**: Initialize and configure sensors
- **Data Reading**: Read sensor values in a standardized format
- **Calibration**: Calibrate sensors that require it
- **Power Management**: Control sensor power modes
- **Diagnostics**: Test communication and get health status

### Sensor Registry

The `SensorRegistry` class manages multiple sensors:

```cpp
SensorRegistry registry;

// Register sensors
registry.registerSensor(dhtSensor);
registry.registerSensor(ultrasonicSensor);

// Initialize all sensors
registry.initAll();

// Read all sensors at once
std::vector<SensorReading> readings = registry.readAll();
```

### Sensor Factory

The `SensorFactory` can auto-detect and create sensor instances:

```cpp
// Auto-detect all available sensors
std::vector<BaseSensor*> sensors = SensorFactory::detectSensors();

// Check if specific sensor is available
if (SensorFactory::isSensorAvailable(UnifiedSensorType::SENSOR_DHT22)) {
    BaseSensor* sensor = SensorFactory::createSensor(UnifiedSensorType::SENSOR_DHT22);
}
```

## Supported Sensor Types

### Environmental Sensors

#### DHT11/DHT22 Temperature & Humidity
- **Interface**: Digital (single-wire)
- **Measurements**: Temperature (-40°C to 80°C), Humidity (0-100% RH)
- **Accuracy**: DHT22: ±0.5°C, ±2% RH
- **Power**: ~5mW
- **Class**: `DHTSensor`

```cpp
DHTSensor dht(15, DHTType::DHT22);  // GPIO 15, DHT22 type
dht.init();

SensorReading reading = dht.read();
float temp = reading.values[0];      // Temperature in °C
float humidity = reading.values[1];   // Relative humidity in %
float heatIndex = dht.computeHeatIndex();
```

#### BME280 (Already Supported)
- **Interface**: I2C (0x76 or 0x77)
- **Measurements**: Temperature, Humidity, Pressure
- **See**: `advanced_environmental_sensors.h`

#### BH1750 Light Sensor (Already Supported)
- **Interface**: I2C (0x23)
- **Measurements**: Light intensity in lux
- **See**: System manager I2C detection

### Distance Sensors

#### HC-SR04 Ultrasonic Distance
- **Interface**: Digital (Trigger + Echo pins)
- **Range**: 2cm to 400cm
- **Accuracy**: ±3mm
- **Power**: ~75mW during measurement
- **Class**: `UltrasonicSensor`

```cpp
UltrasonicSensor ultrasonic(12, 14);  // Trigger: GPIO 12, Echo: GPIO 14
ultrasonic.init();

SensorReading reading = ultrasonic.read();
float distance_cm = reading.values[0];

// Get median of multiple readings for better accuracy
float median = ultrasonic.readDistanceMedian(5);
```

### Gas Sensors

#### MQ-2 Smoke & Gas Detector
- **Interface**: Analog
- **Detects**: Smoke, LPG, CO, Alcohol, Propane, Hydrogen
- **Range**: 100-10,000 PPM
- **Requires**: 20-48 hour initial preheat, calibration in clean air
- **Power**: ~750mW (heater)
- **Class**: `MQSensor`

```cpp
MQSensor mq2(34, MQType::MQ2);  // GPIO 34 (analog), MQ-2 type
mq2.init();

// Calibrate in clean air
if (mq2.needsCalibration()) {
    mq2.calibrate();
}

SensorReading reading = mq2.read();
float ppm = reading.values[0];
```

#### MQ-7 Carbon Monoxide
- **Interface**: Analog
- **Detects**: Carbon Monoxide (CO)
- **Range**: 10-1,000 PPM CO
- **Class**: `MQSensor`

```cpp
MQSensor mq7(34, MQType::MQ7);
```

#### MQ-135 Air Quality
- **Interface**: Analog
- **Detects**: CO2, NH3, NOx, Alcohol, Benzene, Smoke
- **Range**: 10-10,000 PPM
- **Class**: `MQSensor`

```cpp
MQSensor mq135(34, MQType::MQ135);
```

### Motion Sensors

#### PIR (Passive Infrared) - Already Supported
- **Interface**: Digital
- **Detection**: Motion via infrared
- **See**: System manager initialization

#### MLX90640 Thermal Camera - Already Supported
- **Interface**: I2C
- **Resolution**: 32x24 thermal array
- **See**: Swarm sensors integration

### Camera Sensors - Already Supported
- OV2640, OV3660, OV5640, GC032A, SC030IOT
- **See**: `esp32_s3_cam.cpp`

## Usage Examples

### Basic Sensor Usage

```cpp
#include "sensors/dht_sensor.h"

// Create sensor
DHTSensor dht(15, DHTType::DHT22);

// Initialize
if (!dht.init()) {
    Serial.println("Failed to init DHT");
    Serial.println(dht.getLastError());
    return;
}

// Configure
SensorConfig config;
config.enabled = true;
config.poll_interval_ms = 2000;
dht.configure(config);

// Read in loop
void loop() {
    if (dht.dataAvailable()) {
        SensorReading reading = dht.read();
        
        if (reading.valid) {
            Serial.print("Temp: ");
            Serial.print(reading.values[0]);
            Serial.println(" °C");
        }
    }
    delay(100);
}
```

### Multi-Sensor Integration

See the complete example in `examples/multi_sensor_integration/multi_sensor_integration.ino`

```cpp
SensorRegistry registry;

// Register multiple sensors
registry.registerSensor(new DHTSensor(15, DHTType::DHT22));
registry.registerSensor(new UltrasonicSensor(12, 14));
registry.registerSensor(new MQSensor(34, MQType::MQ135));

// Initialize all
registry.initAll();

// Read all sensors
std::vector<SensorReading> readings = registry.readAll();

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
```

## Adding New Sensor Types

To add a new sensor type:

1. **Add to UnifiedSensorType enum** in `sensor_interface.h`:
   ```cpp
   enum class UnifiedSensorType {
       // ... existing types
       SENSOR_YOUR_NEW_SENSOR,
   };
   ```

2. **Create sensor class** inheriting from `BaseSensor`:
   ```cpp
   class YourSensor : public BaseSensor {
   public:
       // Implement required virtual functions
       UnifiedSensorType getSensorType() const override;
       bool init() override;
       SensorReading read() override;
       // ... etc
   };
   ```

3. **Add to SensorFactory** in `sensor_interface.cpp`:
   ```cpp
   BaseSensor* SensorFactory::createSensor(UnifiedSensorType type) {
       switch(type) {
           case UnifiedSensorType::SENSOR_YOUR_NEW_SENSOR:
               return new YourSensor();
           // ... existing cases
       }
   }
   ```

4. **Update utility functions** to handle new sensor name/type strings

5. **Add example** in `examples/` directory

## Configuration

### SensorConfig Structure

```cpp
struct SensorConfig {
    bool enabled;                   // Enable/disable sensor
    uint16_t poll_interval_ms;      // Minimum time between readings
    bool use_interrupts;            // Use interrupt-driven reading
    uint8_t interrupt_pin;          // GPIO for interrupt
    float calibration_offset;       // Offset calibration
    float calibration_scale;        // Scale calibration
    bool low_power_mode;            // Enable low power mode
    uint32_t timeout_ms;            // Read timeout
};
```

### Sensor Capabilities

Each sensor reports its capabilities:

```cpp
SensorCapabilities caps = sensor->getCapabilities();

Serial.println(caps.name);              // e.g., "DHT22"
Serial.println(caps.manufacturer);      // e.g., "Aosong"
Serial.println(caps.interface);         // I2C, SPI, Analog, etc.
Serial.println(caps.poll_interval_ms);  // Recommended poll rate
Serial.println(caps.requires_calibration); // true/false
```

## Power Management

Sensors support power management:

```cpp
// Put sensor to sleep
sensor->sleep();

// Wake sensor
sensor->wake();

// Get power consumption
float power_mw = sensor->getPowerConsumption();
```

## Diagnostics

### Health Monitoring

```cpp
// Test sensor communication
if (!sensor->testCommunication()) {
    Serial.println("Sensor not responding");
}

// Get health score (0-100)
uint8_t health = sensor->getHealthScore();

// Get status
SensorStatus status = sensor->getStatus();
```

### Error Handling

```cpp
SensorReading reading = sensor->read();

if (!reading.valid) {
    Serial.print("Error: ");
    Serial.println(sensor->getLastError());
    Serial.print("Status: ");
    Serial.println(statusToString(reading.status));
}
```

## Pin Assignments

Common GPIO pin assignments for ESP32-S3-CAM:

| Sensor Type | Default Pins | Notes |
|------------|--------------|-------|
| DHT11/22   | GPIO 15      | Any digital GPIO |
| HC-SR04    | Trigger: GPIO 12, Echo: GPIO 14 | Any digital GPIOs |
| MQ Sensors | GPIO 34-39   | Analog input pins |
| I2C Sensors| SDA: GPIO 1, SCL: GPIO 2 | Shared I2C bus |
| PIR        | GPIO 33      | Any digital GPIO |

**Important**: Avoid GPIOs used by camera interface (see `esp32_s3_cam_config.h`)

## Integration with Existing Systems

The new sensor abstraction integrates with:

- **Advanced Environmental Sensors**: `advanced_environmental_sensors.h` can use the new interface
- **Swarm Sensors**: `swarm_sensors.h` sensor types map to unified types
- **System Manager**: `system_manager.cpp` I2C detection can use sensor factory
- **Camera Boards**: HAL boards can register camera sensors in the registry

## Performance Considerations

- **Polling Rates**: Respect minimum poll intervals to avoid errors
- **I2C Bus**: Multiple I2C sensors share the bus; coordinate access
- **Power**: Gas sensors consume significant power (~750mW heater)
- **Memory**: Each sensor instance uses ~200-500 bytes RAM
- **Calibration**: MQ sensors need 20-48 hours initial preheat for accuracy

## Future Enhancements

Planned additions:
- VL53L0X/VL53L1X time-of-flight distance sensors
- I2S/PDM microphone modules for acoustic monitoring
- GPS module integration
- IMU (accelerometer/gyroscope) support
- Additional environmental sensors (pressure, UV, etc.)
- Sensor data fusion algorithms
- Machine learning-based sensor anomaly detection

## References

- Base Interface: `firmware/src/sensors/sensor_interface.h`
- DHT Sensor: `firmware/src/sensors/dht_sensor.h`
- Ultrasonic: `firmware/src/sensors/ultrasonic_sensor.h`
- Gas Sensors: `firmware/src/sensors/mq_sensor.h`
- Example: `examples/multi_sensor_integration/`
- Advanced Environmental: `firmware/src/sensors/advanced_environmental_sensors.h`
