# WildCAM ESP32 Sensor Examples

This directory contains examples demonstrating the use of the sensor abstraction layer and various sensor types.

## Available Examples

### 1. Basic Sensor Reading (`basic_sensor_reading/`)

**Purpose**: Demonstrates the simplest way to initialize and read from a single sensor.

**Hardware Required**:
- ESP32 board
- DHT22 temperature/humidity sensor
- Connecting wires

**Wiring**:
```
DHT22 VCC  → ESP32 3.3V
DHT22 GND  → ESP32 GND
DHT22 DATA → ESP32 GPIO 15
```

**What it demonstrates**:
- Creating a sensor instance
- Initializing a sensor
- Configuring sensor parameters
- Reading sensor values
- Displaying sensor capabilities
- Environmental assessment

**Key Code**:
```cpp
DHTSensor dht(15, DHTType::DHT22);
dht.init();

SensorReading reading = dht.read();
float temperature = reading.values[0];
float humidity = reading.values[1];
```

### 2. Multi-Sensor Integration (`multi_sensor_integration/`)

**Purpose**: Shows how to work with multiple sensors simultaneously using the sensor registry.

**Hardware Required**:
- ESP32 board
- DHT22 temperature/humidity sensor
- HC-SR04 ultrasonic distance sensor
- MQ-135 air quality sensor
- Connecting wires

**Wiring**:
```
DHT22:
  VCC  → ESP32 3.3V
  GND  → ESP32 GND
  DATA → ESP32 GPIO 15

HC-SR04:
  VCC     → ESP32 5V
  GND     → ESP32 GND
  TRIGGER → ESP32 GPIO 12
  ECHO    → ESP32 GPIO 14

MQ-135:
  VCC  → ESP32 5V
  GND  → ESP32 GND
  AOUT → ESP32 GPIO 34
```

**What it demonstrates**:
- Registering multiple sensors
- Using SensorRegistry for management
- Reading all sensors together
- Sensor fusion for wildlife detection
- Multi-modal environmental monitoring
- Health monitoring across sensors

**Key Code**:
```cpp
SensorRegistry registry;

registry.registerSensor(new DHTSensor(15, DHTType::DHT22));
registry.registerSensor(new UltrasonicSensor(12, 14));
registry.registerSensor(new MQSensor(34, MQType::MQ135));

registry.initAll();
std::vector<SensorReading> readings = registry.readAll();
```

## Common Pin Configurations

### ESP32-CAM / ESP32-S3-CAM

**Available GPIO pins** (not used by camera):
- GPIO 1, 2, 3 (with caution - strapping pins)
- GPIO 12, 13, 14, 15, 16
- GPIO 33 (if not used for LED)
- GPIO 34, 35, 36, 39 (input only, ADC)

**Recommended sensor pins**:
- DHT sensors: GPIO 15
- PIR sensor: GPIO 13
- Ultrasonic: Trigger=GPIO 12, Echo=GPIO 14
- Analog sensors (MQ): GPIO 34-39

**I2C bus** (shared for environmental sensors):
- SDA: GPIO 1
- SCL: GPIO 2

### Standard ESP32

More flexible GPIO options available. See board documentation.

## Sensor Types Reference

### Temperature & Humidity
- **DHT11**: Budget option, ±2°C, ±5% RH
- **DHT22**: Better accuracy, ±0.5°C, ±2% RH
- **BME280**: I2C, includes pressure sensor

### Distance Measurement
- **HC-SR04**: 2-400cm range, ultrasonic
- **VL53L0X**: 30-2000mm, laser ToF (I2C)

### Gas & Air Quality
- **MQ-2**: Smoke, LPG, CO
- **MQ-7**: Carbon monoxide
- **MQ-135**: General air quality, CO2
- **SGP30**: TVOC, eCO2 (I2C)

### Motion Detection
- **PIR**: Passive infrared motion
- **RCWL-0516**: Microwave motion
- **MLX90640**: Thermal imaging (I2C)

### Light Sensing
- **BH1750**: Digital light intensity (I2C)
- **TSL2591**: Advanced light sensor (I2C)
- **LDR**: Simple analog light sensor

## Building and Uploading

### Arduino IDE

1. Install required libraries:
   - DHT sensor library by Adafruit
   - Adafruit Unified Sensor
   - (Other sensors as needed)

2. Open example `.ino` file
3. Select your board (ESP32 Dev Module or ESP32-S3)
4. Select COM port
5. Click Upload

### PlatformIO

1. Open example directory in PlatformIO
2. Ensure `platformio.ini` has required libraries
3. Build and upload:
   ```bash
   pio run -t upload
   pio device monitor
   ```

## Troubleshooting

### Sensor Not Detected

**DHT Sensors**:
- Check wiring (VCC, GND, DATA)
- Verify GPIO pin number in code
- Add 10kΩ pull-up resistor between DATA and VCC if needed
- Wait 2 seconds after power-on

**HC-SR04**:
- Requires 5V power (not 3.3V)
- Check both trigger and echo connections
- Ensure no obstacles blocking sensor

**MQ Gas Sensors**:
- Require 20-48 hour preheat for first use
- Need 20-60 second warm-up after power on
- Calibrate in clean air
- Check analog pin supports ADC

**I2C Sensors**:
- Verify I2C address (use I2C scanner)
- Check SDA/SCL connections
- Add pull-up resistors (4.7kΩ) if needed
- Ensure only one device per address

### Reading Errors

**Invalid readings**:
```cpp
if (!reading.valid) {
    Serial.println(sensor->getLastError());
    Serial.println(statusToString(reading.status));
}
```

**Check sensor health**:
```cpp
uint8_t health = sensor->getHealthScore();
if (health < 50) {
    // Sensor may be failing or disconnected
}
```

### Performance Issues

- Respect minimum poll intervals (see `getCapabilities()`)
- Don't block in sensor reading loops
- Use interrupts where supported
- Consider low-power modes for battery operation

## Adding Your Own Examples

To create a new example:

1. Create directory: `examples/your_example_name/`
2. Create `.ino` file with same name
3. Include sensor headers:
   ```cpp
   #include "../../firmware/src/sensors/sensor_interface.h"
   #include "../../firmware/src/sensors/your_sensor.h"
   ```
4. Follow existing example structure
5. Document hardware requirements
6. Add to this README

## Integration with Main Firmware

To use sensors in main WildCAM firmware:

```cpp
#include "firmware/src/sensors/sensor_interface.h"
#include "firmware/src/sensors/dht_sensor.h"

// In your main setup
SensorRegistry globalRegistry;

void setupSensors() {
    // Register environmental sensors
    if (config.enable_dht) {
        auto dht = new DHTSensor(config.dht_pin, DHTType::DHT22);
        dht->init();
        globalRegistry.registerSensor(dht);
    }
    
    // More sensors...
}

// In your main loop
void monitorEnvironment() {
    auto readings = globalRegistry.readAll();
    
    for (auto& reading : readings) {
        if (reading.valid) {
            // Process sensor data
            logSensorData(reading);
            checkAlertConditions(reading);
        }
    }
}
```

## Wildlife Monitoring Applications

### Motion-Triggered Recording
Combine ultrasonic or PIR sensors with camera:
```cpp
if (ultrasonic.readDistanceCm() < 200.0f) {
    camera.capture();
}
```

### Environmental Correlation
Associate weather with wildlife activity:
```cpp
if (wildlife_detected) {
    auto env = dht.read();
    logActivity(animal_type, env.values[0], env.values[1]);
}
```

### Air Quality Monitoring
Detect fires or pollution:
```cpp
if (mq2.readPPM() > SMOKE_THRESHOLD) {
    sendFireAlert();
}
```

## Additional Resources

- **Sensor Interface Documentation**: `firmware/src/sensors/README_SENSORS.md`
- **Hardware Guides**: `hardware/` directory
- **Main Firmware Integration**: `firmware/ESP32_WildlifeCam_Pro.ino`
- **Board Configurations**: `hardware/esp32_s3_cam/esp32_s3_cam_config.h`

## Support

For issues or questions:
1. Check troubleshooting section above
2. Review sensor documentation
3. Check pin assignments for your board
4. Verify library versions
5. Open GitHub issue with details

## License

These examples are part of WildCAM ESP32 project and follow the same license as the main project.
