/**
 * @file basic_sensor_reading.ino
 * @brief Basic example of reading a single sensor
 * 
 * This simple example shows how to initialize and read from a DHT22
 * temperature and humidity sensor using the sensor abstraction layer.
 */

#include "../../firmware/src/sensors/sensor_interface.h"
#include "../../firmware/src/sensors/dht_sensor.h"

// Pin configuration
#define DHT_PIN 15

// Sensor instance
DHTSensor* tempHumiditySensor = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n====================================");
    Serial.println("WildCAM Basic Sensor Reading Example");
    Serial.println("====================================\n");
    
    // Create and initialize DHT22 sensor
    Serial.println("Initializing DHT22 sensor on GPIO 15...");
    tempHumiditySensor = new DHTSensor(DHT_PIN, DHTType::DHT22);
    
    if (tempHumiditySensor->init()) {
        Serial.println("✓ Sensor initialized successfully!\n");
        
        // Print sensor capabilities
        printSensorInfo(tempHumiditySensor);
        
        // Configure sensor
        SensorConfig config;
        config.enabled = true;
        config.poll_interval_ms = 2000;  // Read every 2 seconds minimum
        config.low_power_mode = false;
        
        tempHumiditySensor->configure(config);
        
        Serial.println("\nStarting measurements...\n");
    } else {
        Serial.println("✗ Failed to initialize sensor");
        Serial.print("Error: ");
        Serial.println(tempHumiditySensor->getLastError());
    }
}

void loop() {
    if (!tempHumiditySensor || !tempHumiditySensor->isInitialized()) {
        delay(1000);
        return;
    }
    
    // Check if sensor is ready for new reading
    if (tempHumiditySensor->dataAvailable()) {
        // Read sensor
        SensorReading reading = tempHumiditySensor->read();
        
        // Display results
        Serial.println("=== Sensor Reading ===");
        Serial.printf("Time: %lu ms\n", reading.timestamp);
        Serial.printf("Status: %s\n", statusToString(reading.status));
        
        if (reading.valid) {
            // Extract temperature and humidity
            float temperature = reading.values[0];
            float humidity = reading.values[1];
            
            Serial.printf("Temperature: %.1f °C (%.1f °F)\n", 
                         temperature, 
                         temperature * 9.0f / 5.0f + 32.0f);
            Serial.printf("Humidity: %.1f %%\n", humidity);
            
            // Calculate and display heat index
            float heatIndex = tempHumiditySensor->computeHeatIndex();
            Serial.printf("Heat Index: %.1f °C (%.1f °F)\n", 
                         heatIndex,
                         heatIndex * 9.0f / 5.0f + 32.0f);
            
            // Sensor health
            uint8_t health = tempHumiditySensor->getHealthScore();
            Serial.printf("Sensor Health: %d%%\n", health);
            
            // Environmental assessment
            assessEnvironment(temperature, humidity);
        } else {
            Serial.println("Reading invalid");
            if (strlen(tempHumiditySensor->getLastError()) > 0) {
                Serial.print("Error: ");
                Serial.println(tempHumiditySensor->getLastError());
            }
        }
        
        Serial.println();
    }
    
    // Small delay to prevent overwhelming the serial output
    delay(100);
}

/**
 * @brief Print sensor information and capabilities
 */
void printSensorInfo(BaseSensor* sensor) {
    if (!sensor) return;
    
    SensorCapabilities caps = sensor->getCapabilities();
    
    Serial.println("--- Sensor Information ---");
    Serial.printf("Name: %s\n", caps.name);
    Serial.printf("Manufacturer: %s\n", caps.manufacturer);
    Serial.printf("Interface: %s\n", interfaceTypeToString(caps.interface));
    Serial.printf("Range: %.1f to %.1f %s\n", 
                 caps.min_value, caps.max_value, caps.unit);
    Serial.printf("Recommended Poll Interval: %d ms\n", caps.poll_interval_ms);
    Serial.printf("Requires Calibration: %s\n", 
                 caps.requires_calibration ? "Yes" : "No");
    Serial.printf("Power Consumption: %.1f mW\n", 
                 sensor->getPowerConsumption());
    Serial.println();
}

/**
 * @brief Assess environmental conditions for wildlife
 */
void assessEnvironment(float temperature, float humidity) {
    Serial.println("--- Environmental Assessment ---");
    
    // Temperature assessment
    if (temperature < 0) {
        Serial.println("⚠ Freezing conditions");
    } else if (temperature > 35) {
        Serial.println("⚠ Very hot conditions");
    } else if (temperature >= 15 && temperature <= 25) {
        Serial.println("✓ Optimal temperature for wildlife activity");
    }
    
    // Humidity assessment
    if (humidity < 30) {
        Serial.println("⚠ Low humidity - dry conditions");
    } else if (humidity > 80) {
        Serial.println("⚠ High humidity - damp conditions");
    } else if (humidity >= 40 && humidity <= 70) {
        Serial.println("✓ Comfortable humidity level");
    }
    
    // Combined assessment for wildlife photography
    if (temperature >= 10 && temperature <= 30 && 
        humidity >= 30 && humidity <= 80) {
        Serial.println("📷 Good conditions for wildlife photography");
    }
}
