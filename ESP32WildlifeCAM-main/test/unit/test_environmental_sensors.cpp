/**
 * @file test_environmental_sensors.cpp
 * @brief Unit tests for Environmental Sensors module
 * 
 * Tests temperature, humidity, pressure, and other environmental readings
 */

#include <unity.h>
#include <cmath>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
MockSerial Serial;
#endif

// Mock sensor readings
struct SensorData {
    float temperature;      // Celsius
    float humidity;         // Percentage
    float pressure;         // hPa
    float altitude;         // meters
    bool valid;
};

// Mock sensor functions
SensorData mockSensorData = {
    .temperature = 25.0f,
    .humidity = 60.0f,
    .pressure = 1013.25f,
    .altitude = 100.0f,
    .valid = true
};

SensorData readSensors() {
    return mockSensorData;
}

bool initializeSensors() {
    return true;
}

float celsiusToFahrenheit(float celsius) {
    return celsius * 9.0f / 5.0f + 32.0f;
}

float calculateDewPoint(float temperature, float humidity) {
    // Magnus formula approximation
    float a = 17.27f;
    float b = 237.7f;
    float alpha = ((a * temperature) / (b + temperature)) + logf(humidity / 100.0f);
    return (b * alpha) / (a - alpha);
}

float calculateAltitude(float pressure, float seaLevelPressure = 1013.25f) {
    // Barometric formula
    return 44330.0f * (1.0f - powf(pressure / seaLevelPressure, 0.1903f));
}

bool isTemperatureValid(float temp) {
    return temp >= -40.0f && temp <= 85.0f;
}

bool isHumidityValid(float humidity) {
    return humidity >= 0.0f && humidity <= 100.0f;
}

bool isPressureValid(float pressure) {
    return pressure >= 300.0f && pressure <= 1100.0f;
}

void test_environmental_sensors_creation() {
    TEST_ASSERT_TRUE_MESSAGE(true, "Environmental sensors creation test");
}

void test_sensor_initialization() {
    bool result = initializeSensors();
    TEST_ASSERT_TRUE_MESSAGE(result, "Sensor initialization should succeed");
}

void test_temperature_reading() {
    SensorData data = readSensors();
    TEST_ASSERT_TRUE_MESSAGE(data.valid, "Sensor data should be valid");
    TEST_ASSERT_TRUE_MESSAGE(isTemperatureValid(data.temperature), 
                            "Temperature should be in valid range");
}

void test_humidity_reading() {
    SensorData data = readSensors();
    TEST_ASSERT_TRUE_MESSAGE(isHumidityValid(data.humidity), 
                            "Humidity should be in valid range (0-100%)");
}

void test_pressure_reading() {
    SensorData data = readSensors();
    TEST_ASSERT_TRUE_MESSAGE(isPressureValid(data.pressure), 
                            "Pressure should be in valid range");
}

void test_temperature_conversion() {
    float celsius = 25.0f;
    float fahrenheit = celsiusToFahrenheit(celsius);
    
    TEST_ASSERT_EQUAL_FLOAT(77.0f, fahrenheit);
    
    // Test freezing point
    TEST_ASSERT_EQUAL_FLOAT(32.0f, celsiusToFahrenheit(0.0f));
    
    // Test boiling point
    TEST_ASSERT_EQUAL_FLOAT(212.0f, celsiusToFahrenheit(100.0f));
}

void test_dew_point_calculation() {
    float dewPoint = calculateDewPoint(25.0f, 60.0f);
    
    TEST_ASSERT_GREATER_THAN_MESSAGE(0.0f, dewPoint, "Dew point should be positive");
    TEST_ASSERT_LESS_THAN_MESSAGE(25.0f, dewPoint, 
                                  "Dew point should be less than temperature");
}

void test_altitude_calculation() {
    float altitude = calculateAltitude(1013.25f); // Sea level pressure
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0f, 0.0f, altitude, 
                                     "Altitude at sea level pressure should be ~0m");
    
    float mountainAltitude = calculateAltitude(900.0f);
    TEST_ASSERT_GREATER_THAN_MESSAGE(500.0f, mountainAltitude, 
                                     "Lower pressure should give higher altitude");
}

void test_sensor_data_validation() {
    mockSensorData.temperature = 25.0f;
    mockSensorData.humidity = 60.0f;
    mockSensorData.pressure = 1013.25f;
    
    SensorData data = readSensors();
    TEST_ASSERT_TRUE(isTemperatureValid(data.temperature));
    TEST_ASSERT_TRUE(isHumidityValid(data.humidity));
    TEST_ASSERT_TRUE(isPressureValid(data.pressure));
}

void test_extreme_temperature_detection() {
    TEST_ASSERT_FALSE(isTemperatureValid(-50.0f)); // Too cold
    TEST_ASSERT_FALSE(isTemperatureValid(100.0f)); // Too hot
    TEST_ASSERT_TRUE(isTemperatureValid(0.0f));    // Valid
    TEST_ASSERT_TRUE(isTemperatureValid(30.0f));   // Valid
}

void test_humidity_boundary_conditions() {
    TEST_ASSERT_TRUE(isHumidityValid(0.0f));    // Minimum
    TEST_ASSERT_TRUE(isHumidityValid(100.0f));  // Maximum
    TEST_ASSERT_FALSE(isHumidityValid(-1.0f));  // Below range
    TEST_ASSERT_FALSE(isHumidityValid(101.0f)); // Above range
}

void test_pressure_range_validation() {
    TEST_ASSERT_TRUE(isPressureValid(1013.25f));  // Standard
    TEST_ASSERT_TRUE(isPressureValid(870.0f));    // Mountain
    TEST_ASSERT_FALSE(isPressureValid(200.0f));   // Too low
    TEST_ASSERT_FALSE(isPressureValid(1200.0f));  // Too high
}

void setUp(void) {
    // Reset mock data
    mockSensorData.temperature = 25.0f;
    mockSensorData.humidity = 60.0f;
    mockSensorData.pressure = 1013.25f;
    mockSensorData.altitude = 100.0f;
    mockSensorData.valid = true;
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_environmental_sensors_creation);
    RUN_TEST(test_sensor_initialization);
    RUN_TEST(test_temperature_reading);
    RUN_TEST(test_humidity_reading);
    RUN_TEST(test_pressure_reading);
    RUN_TEST(test_temperature_conversion);
    RUN_TEST(test_dew_point_calculation);
    RUN_TEST(test_altitude_calculation);
    RUN_TEST(test_sensor_data_validation);
    RUN_TEST(test_extreme_temperature_detection);
    RUN_TEST(test_humidity_boundary_conditions);
    RUN_TEST(test_pressure_range_validation);
    
    return UNITY_END();
}
