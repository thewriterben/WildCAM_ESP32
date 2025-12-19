/**
 * @file test_sensor_manager.cpp
 * @brief Unit tests for SensorManager class
 * 
 * Tests BME280, GPS, and Light sensor functionality.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include <unity.h>
#include "SensorManager.h"
#include "config.h"

// Test instance
static SensorManager* testSensors = nullptr;

void setUp(void) {
    testSensors = new SensorManager();
}

void tearDown(void) {
    if (testSensors != nullptr) {
        delete testSensors;
        testSensors = nullptr;
    }
}

//==============================================================================
// INITIALIZATION TESTS
//==============================================================================

/**
 * @brief Test SensorManager default construction
 */
void test_sensor_manager_default_construction(void) {
    TEST_ASSERT_NOT_NULL(testSensors);
    
    // Initially no sensors should be available
    TEST_ASSERT_FALSE(testSensors->isBME280Available());
    TEST_ASSERT_FALSE(testSensors->isGPSAvailable());
    TEST_ASSERT_FALSE(testSensors->isLightSensorAvailable());
}

/**
 * @brief Test I2C initialization
 */
void test_i2c_initialization(void) {
    // I2C init should succeed with valid pins
    TEST_ASSERT_TRUE(testSensors->initI2C(21, 22));
    
    // Re-initialization should also return true (idempotent)
    TEST_ASSERT_TRUE(testSensors->initI2C(21, 22));
}

/**
 * @brief Test sensor init without any sensors enabled
 */
void test_sensor_init_no_sensors(void) {
    // Without any hardware, init() should return false
    // but not crash or cause errors
    bool result = testSensors->init();
    
    // Result depends on whether sensors are configured
    // In test environment without hardware, this may return false
    // This test just ensures no crashes occur
    TEST_ASSERT_TRUE(true);
}

//==============================================================================
// BME280 TESTS
//==============================================================================

/**
 * @brief Test BME280 read when not initialized
 */
void test_bme280_read_not_initialized(void) {
    EnvironmentalData data = testSensors->readEnvironmental();
    
    // Should return invalid data when not initialized
    TEST_ASSERT_FALSE(data.valid);
}

/**
 * @brief Test environmental data structure
 */
void test_environmental_data_structure(void) {
    EnvironmentalData data;
    
    // Test default/zero initialization
    memset(&data, 0, sizeof(data));
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.temperature);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.humidity);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.pressure);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.altitude);
    TEST_ASSERT_FALSE(data.valid);
    TEST_ASSERT_EQUAL_UINT32(0, data.timestamp);
}

/**
 * @brief Test sea level pressure setting
 */
void test_set_sea_level_pressure(void) {
    // Setting pressure should not cause any issues
    testSensors->setSeaLevelPressure(1013.25);
    testSensors->setSeaLevelPressure(1000.0);
    testSensors->setSeaLevelPressure(1025.0);
    
    // No crash = success
    TEST_ASSERT_TRUE(true);
}

//==============================================================================
// GPS TESTS
//==============================================================================

/**
 * @brief Test GPS read when not initialized
 */
void test_gps_read_not_initialized(void) {
    GPSData data = testSensors->readGPS();
    
    // Should return no fix when not initialized
    TEST_ASSERT_FALSE(data.hasFix);
}

/**
 * @brief Test GPS data structure
 */
void test_gps_data_structure(void) {
    GPSData data;
    
    // Test default/zero initialization
    memset(&data, 0, sizeof(data));
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.latitude);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.longitude);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.altitude);
    TEST_ASSERT_EQUAL_INT(0, data.satellites);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.hdop);
    TEST_ASSERT_FALSE(data.hasFix);
    TEST_ASSERT_EQUAL_UINT32(0, data.timestamp);
}

/**
 * @brief Test GPS location string when not initialized
 */
void test_gps_location_string_not_initialized(void) {
    String locationStr = testSensors->getLocationString();
    
    // Should return "No GPS" when not initialized or no fix
    TEST_ASSERT_EQUAL_STRING("No GPS", locationStr.c_str());
}

/**
 * @brief Test GPS availability check
 */
void test_gps_availability(void) {
    TEST_ASSERT_FALSE(testSensors->isGPSAvailable());
    TEST_ASSERT_FALSE(testSensors->hasGPSFix());
}

//==============================================================================
// LIGHT SENSOR TESTS
//==============================================================================

/**
 * @brief Test light sensor read when not initialized
 */
void test_light_read_not_initialized(void) {
    LightData data = testSensors->readLight();
    
    // Should return invalid data when not initialized
    TEST_ASSERT_FALSE(data.valid);
}

/**
 * @brief Test light data structure
 */
void test_light_data_structure(void) {
    LightData data;
    
    // Test default/zero initialization
    memset(&data, 0, sizeof(data));
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, data.lux);
    TEST_ASSERT_FALSE(data.isDaytime);
    TEST_ASSERT_FALSE(data.valid);
    TEST_ASSERT_EQUAL_UINT32(0, data.timestamp);
}

/**
 * @brief Test daytime threshold setting
 */
void test_set_daytime_threshold(void) {
    // Setting threshold should not cause any issues
    testSensors->setDaytimeThreshold(50.0);
    testSensors->setDaytimeThreshold(100.0);
    testSensors->setDaytimeThreshold(200.0);
    
    // No crash = success
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief Test light condition enum values
 */
void test_light_condition_enum(void) {
    // Test that enum values are distinct
    TEST_ASSERT_NOT_EQUAL((int)LightCondition::NIGHT, (int)LightCondition::TWILIGHT);
    TEST_ASSERT_NOT_EQUAL((int)LightCondition::TWILIGHT, (int)LightCondition::CLOUDY);
    TEST_ASSERT_NOT_EQUAL((int)LightCondition::CLOUDY, (int)LightCondition::DAYLIGHT);
    TEST_ASSERT_NOT_EQUAL((int)LightCondition::DAYLIGHT, (int)LightCondition::BRIGHT_SUN);
}

/**
 * @brief Test light condition when not initialized
 */
void test_light_condition_not_initialized(void) {
    // Should default to DAYLIGHT when sensor not available
    LightCondition condition = testSensors->getLightCondition();
    TEST_ASSERT_EQUAL((int)LightCondition::DAYLIGHT, (int)condition);
}

//==============================================================================
// UTILITY TESTS
//==============================================================================

/**
 * @brief Test print status doesn't crash
 */
void test_print_status_no_crash(void) {
    // Should not crash even with no sensors initialized
    testSensors->printStatus();
    
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief Test GPS update when not initialized
 */
void test_gps_update_not_initialized(void) {
    // Should return false when not initialized
    bool result = testSensors->updateGPS();
    TEST_ASSERT_FALSE(result);
}

//==============================================================================
// MAIN TEST RUNNER
//==============================================================================

void setup() {
    delay(2000);  // Allow serial to settle
    
    UNITY_BEGIN();
    
    // Run initialization tests
    RUN_TEST(test_sensor_manager_default_construction);
    RUN_TEST(test_i2c_initialization);
    RUN_TEST(test_sensor_init_no_sensors);
    
    // Run BME280 tests
    RUN_TEST(test_bme280_read_not_initialized);
    RUN_TEST(test_environmental_data_structure);
    RUN_TEST(test_set_sea_level_pressure);
    
    // Run GPS tests
    RUN_TEST(test_gps_read_not_initialized);
    RUN_TEST(test_gps_data_structure);
    RUN_TEST(test_gps_location_string_not_initialized);
    RUN_TEST(test_gps_availability);
    
    // Run Light sensor tests
    RUN_TEST(test_light_read_not_initialized);
    RUN_TEST(test_light_data_structure);
    RUN_TEST(test_set_daytime_threshold);
    RUN_TEST(test_light_condition_enum);
    RUN_TEST(test_light_condition_not_initialized);
    
    // Run utility tests
    RUN_TEST(test_print_status_no_crash);
    RUN_TEST(test_gps_update_not_initialized);
    
    UNITY_END();
}

void loop() {
    // Nothing to do in loop for unit tests
}
