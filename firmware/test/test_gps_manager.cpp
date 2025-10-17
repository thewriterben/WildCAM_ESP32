/**
 * @file test_gps_manager.cpp
 * @brief Unit tests for GPS Manager
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 */

#include <unity.h>
#include "../src/sensors/gps_manager.h"

GPSManager* testGps = nullptr;

void setUp(void) {
    // Setup before each test
    testGps = new GPSManager();
}

void tearDown(void) {
    // Cleanup after each test
    if (testGps != nullptr) {
        delete testGps;
        testGps = nullptr;
    }
}

/**
 * Test GPS Manager instantiation
 */
void test_gps_manager_instantiation(void) {
    TEST_ASSERT_NOT_NULL(testGps);
    TEST_ASSERT_FALSE(testGps->isInitialized());
    TEST_ASSERT_FALSE(testGps->hasFix());
}

/**
 * Test GPS Manager initialization
 */
void test_gps_manager_initialization(void) {
    // Initialize with test pins (32, 33) and baud rate 9600
    bool result = testGps->initialize(32, 33, 9600);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testGps->isInitialized());
}

/**
 * Test GPS Manager default values
 */
void test_gps_manager_default_values(void) {
    testGps->initialize(32, 33, 9600);
    
    // Before any GPS data, values should be 0 or default
    TEST_ASSERT_EQUAL_FLOAT(0.0, testGps->getLatitude());
    TEST_ASSERT_EQUAL_FLOAT(0.0, testGps->getLongitude());
    TEST_ASSERT_EQUAL_FLOAT(0.0, testGps->getAltitude());
    TEST_ASSERT_EQUAL_INT(0, testGps->getSatelliteCount());
    TEST_ASSERT_FALSE(testGps->hasFix());
}

/**
 * Test GPS location string format
 */
void test_gps_location_string_no_fix(void) {
    testGps->initialize(32, 33, 9600);
    
    String location = testGps->getLocationString();
    TEST_ASSERT_EQUAL_STRING("No GPS fix", location.c_str());
}

/**
 * Test update method returns false when no data
 */
void test_gps_update_no_data(void) {
    testGps->initialize(32, 33, 9600);
    
    // Without actual GPS data, update should return false
    bool result = testGps->update();
    TEST_ASSERT_FALSE(result);
}

/**
 * Test initialization with different baud rates
 */
void test_gps_initialization_different_baud(void) {
    bool result = testGps->initialize(32, 33, 115200);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testGps->isInitialized());
}

/**
 * Test multiple initialization calls
 */
void test_gps_multiple_initialization(void) {
    bool result1 = testGps->initialize(32, 33, 9600);
    TEST_ASSERT_TRUE(result1);
    
    // Second initialization should succeed (already initialized)
    bool result2 = testGps->initialize(32, 33, 9600);
    TEST_ASSERT_TRUE(result2);
}

/**
 * Test update before initialization
 */
void test_gps_update_before_init(void) {
    // Update without initialization should return false
    bool result = testGps->update();
    TEST_ASSERT_FALSE(result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_gps_manager_instantiation);
    RUN_TEST(test_gps_manager_initialization);
    RUN_TEST(test_gps_manager_default_values);
    RUN_TEST(test_gps_location_string_no_fix);
    RUN_TEST(test_gps_update_no_data);
    RUN_TEST(test_gps_initialization_different_baud);
    RUN_TEST(test_gps_multiple_initialization);
    RUN_TEST(test_gps_update_before_init);
    
    UNITY_END();
}

void loop() {
    // Nothing here
}

