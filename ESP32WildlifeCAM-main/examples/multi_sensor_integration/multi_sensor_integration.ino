/**
 * @file multi_sensor_integration.ino
 * @brief Example demonstrating integration of multiple sensor types
 * 
 * This example shows how to use the sensor abstraction layer to work
 * with multiple sensor types including DHT, ultrasonic, and gas sensors.
 */

#include "../../firmware/src/sensors/sensor_interface.h"
#include "../../firmware/src/sensors/dht_sensor.h"
#include "../../firmware/src/sensors/ultrasonic_sensor.h"
#include "../../firmware/src/sensors/mq_sensor.h"

// Pin definitions - adjust based on your hardware
#define DHT_PIN         15
#define TRIGGER_PIN     12
#define ECHO_PIN        14
#define MQ_PIN          34

// Sensor instances
DHTSensor* dhtSensor = nullptr;
UltrasonicSensor* ultrasonicSensor = nullptr;
MQSensor* mqSensor = nullptr;

// Sensor registry
SensorRegistry registry;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n==================================");
    Serial.println("WildCAM Multi-Sensor Integration");
    Serial.println("==================================\n");
    
    // Initialize DHT22 sensor
    Serial.println("Initializing DHT22 sensor...");
    dhtSensor = new DHTSensor(DHT_PIN, DHTType::DHT22);
    if (dhtSensor->init()) {
        Serial.println("✓ DHT22 initialized successfully");
        registry.registerSensor(dhtSensor);
        
        // Configure sensor
        SensorConfig config;
        config.enabled = true;
        config.poll_interval_ms = 2000;
        config.low_power_mode = false;
        dhtSensor->configure(config);
    } else {
        Serial.println("✗ DHT22 initialization failed");
        Serial.print("  Error: ");
        Serial.println(dhtSensor->getLastError());
    }
    
    // Initialize ultrasonic sensor
    Serial.println("\nInitializing HC-SR04 ultrasonic sensor...");
    ultrasonicSensor = new UltrasonicSensor(TRIGGER_PIN, ECHO_PIN);
    if (ultrasonicSensor->init()) {
        Serial.println("✓ HC-SR04 initialized successfully");
        registry.registerSensor(ultrasonicSensor);
        
        SensorConfig config;
        config.enabled = true;
        config.poll_interval_ms = 100;
        config.timeout_ms = 30;
        ultrasonicSensor->configure(config);
    } else {
        Serial.println("✗ HC-SR04 initialization failed");
        Serial.print("  Error: ");
        Serial.println(ultrasonicSensor->getLastError());
    }
    
    // Initialize MQ-135 gas sensor
    Serial.println("\nInitializing MQ-135 gas sensor...");
    mqSensor = new MQSensor(MQ_PIN, MQType::MQ135);
    if (mqSensor->init()) {
        Serial.println("✓ MQ-135 initialized successfully");
        registry.registerSensor(mqSensor);
        
        // Calibrate sensor if needed
        if (mqSensor->needsCalibration()) {
            Serial.println("  Calibrating MQ-135 in clean air...");
            if (mqSensor->calibrate()) {
                Serial.print("  ✓ Calibration complete. R0 = ");
                Serial.println(mqSensor->getR0());
            } else {
                Serial.println("  ✗ Calibration failed");
            }
        }
        
        SensorConfig config;
        config.enabled = true;
        config.poll_interval_ms = 1000;
        mqSensor->configure(config);
    } else {
        Serial.println("✗ MQ-135 initialization failed");
        Serial.print("  Error: ");
        Serial.println(mqSensor->getLastError());
    }
    
    Serial.println("\n==================================");
    Serial.println("Sensor initialization complete");
    Serial.printf("Active sensors: %d\n", registry.getSensorCount());
    Serial.println("==================================\n");
    
    delay(1000);
}

void loop() {
    static unsigned long lastReadTime = 0;
    unsigned long currentTime = millis();
    
    // Read all sensors every 5 seconds
    if (currentTime - lastReadTime >= 5000) {
        lastReadTime = currentTime;
        
        Serial.println("\n========== Sensor Readings ==========");
        Serial.printf("Timestamp: %lu ms\n\n", currentTime);
        
        // Read DHT sensor
        if (dhtSensor && dhtSensor->isInitialized()) {
            Serial.println("DHT22 Temperature/Humidity Sensor:");
            SensorReading reading = dhtSensor->read();
            
            if (reading.valid) {
                Serial.printf("  Temperature: %.1f °C\n", reading.values[0]);
                Serial.printf("  Humidity: %.1f %%\n", reading.values[1]);
                Serial.printf("  Heat Index: %.1f °C\n", dhtSensor->computeHeatIndex());
                Serial.printf("  Health: %d%%\n", dhtSensor->getHealthScore());
            } else {
                Serial.printf("  Status: %s\n", statusToString(reading.status));
                if (strlen(dhtSensor->getLastError()) > 0) {
                    Serial.printf("  Error: %s\n", dhtSensor->getLastError());
                }
            }
        }
        
        // Read ultrasonic sensor
        if (ultrasonicSensor && ultrasonicSensor->isInitialized()) {
            Serial.println("\nHC-SR04 Ultrasonic Distance Sensor:");
            SensorReading reading = ultrasonicSensor->read();
            
            if (reading.valid) {
                Serial.printf("  Distance: %.1f cm (%.1f in)\n", 
                             reading.values[0], 
                             reading.values[0] / 2.54f);
                Serial.printf("  Health: %d%%\n", ultrasonicSensor->getHealthScore());
                
                // Wildlife detection logic
                if (reading.values[0] < 100.0f) {
                    Serial.println("  ⚠ Possible wildlife detected!");
                }
            } else {
                Serial.printf("  Status: %s\n", statusToString(reading.status));
                if (strlen(ultrasonicSensor->getLastError()) > 0) {
                    Serial.printf("  Error: %s\n", ultrasonicSensor->getLastError());
                }
            }
        }
        
        // Read gas sensor
        if (mqSensor && mqSensor->isInitialized()) {
            Serial.println("\nMQ-135 Air Quality Sensor:");
            SensorReading reading = mqSensor->read();
            
            if (reading.valid) {
                Serial.printf("  Gas Level: %.1f PPM\n", reading.values[0]);
                Serial.printf("  Health: %d%%\n", mqSensor->getHealthScore());
                
                // Air quality assessment
                if (reading.values[0] > 1000.0f) {
                    Serial.println("  ⚠ Poor air quality detected!");
                } else if (reading.values[0] > 500.0f) {
                    Serial.println("  ⚡ Moderate air quality");
                } else {
                    Serial.println("  ✓ Good air quality");
                }
            } else {
                Serial.printf("  Status: %s\n", statusToString(reading.status));
                if (strlen(mqSensor->getLastError()) > 0) {
                    Serial.printf("  Error: %s\n", mqSensor->getLastError());
                }
            }
        }
        
        Serial.println("\n====================================\n");
    }
    
    // Check for wildlife detection conditions
    checkWildlifeConditions();
    
    delay(100);
}

/**
 * @brief Check environmental conditions for wildlife activity
 */
void checkWildlifeConditions() {
    static unsigned long lastCheck = 0;
    static bool lastDetection = false;
    
    if (millis() - lastCheck < 1000) {
        return; // Check once per second
    }
    lastCheck = millis();
    
    bool wildlifeDetected = false;
    String detectionReason = "";
    
    // Check distance sensor for movement
    if (ultrasonicSensor && ultrasonicSensor->isInitialized()) {
        SensorReading reading = ultrasonicSensor->getLastReading();
        if (reading.valid && reading.values[0] < 200.0f) {
            wildlifeDetected = true;
            detectionReason = "proximity";
        }
    }
    
    // Log detection events
    if (wildlifeDetected && !lastDetection) {
        Serial.println("\n!!! WILDLIFE DETECTION EVENT !!!");
        Serial.printf("Reason: %s\n", detectionReason.c_str());
        Serial.printf("Time: %lu ms\n\n", millis());
        
        // Here you could trigger camera capture, send alerts, etc.
    }
    
    lastDetection = wildlifeDetected;
}

/**
 * @brief Print sensor capabilities
 */
void printSensorCapabilities(BaseSensor* sensor) {
    if (!sensor) return;
    
    SensorCapabilities caps = sensor->getCapabilities();
    
    Serial.printf("Sensor: %s\n", caps.name);
    Serial.printf("  Manufacturer: %s\n", caps.manufacturer);
    Serial.printf("  Interface: %s\n", interfaceTypeToString(caps.interface));
    Serial.printf("  Range: %.1f - %.1f %s\n", 
                 caps.min_value, caps.max_value, caps.unit);
    Serial.printf("  Poll Interval: %d ms\n", caps.poll_interval_ms);
    Serial.printf("  Calibration Required: %s\n", 
                 caps.requires_calibration ? "Yes" : "No");
}
