/**
 * @file test_gps_manager.cpp
 * @brief Unit tests for GPS manager stub functions
 */

#include <unity.h>
#include "../src/sensors/gps_manager.h"

GPSManager gps;

void setUp(void) {
    // Initialize before each test
}

void tearDown(void) {
    // Cleanup after each test
}

/**
 * Test GPS initialization
 */
void test_initialize(void) {
    // Test with typical GPS pins and baud rate
    bool result = gps.initialize(16, 17, 9600);
    // Stub returns false
    TEST_ASSERT_FALSE(result);
}

/**
 * Test GPS update
 */
void test_update(void) {
    bool result = gps.update();
    // Stub returns false
    TEST_ASSERT_FALSE(result);
}

/**
 * Test latitude reading
 */
void test_getLatitude(void) {
    float latitude = gps.getLatitude();
    // Stub returns 0.0f
    TEST_ASSERT_EQUAL_FLOAT(0.0f, latitude);
}

/**
 * Test longitude reading
 */
void test_getLongitude(void) {
    float longitude = gps.getLongitude();
    // Stub returns 0.0f
    TEST_ASSERT_EQUAL_FLOAT(0.0f, longitude);
}

/**
 * Test altitude reading
 */
void test_getAltitude(void) {
    float altitude = gps.getAltitude();
    // Stub returns 0.0f (sea level)
    TEST_ASSERT_EQUAL_FLOAT(0.0f, altitude);
}

/**
 * Test GPS fix status
 */
void test_hasFix(void) {
    bool fix = gps.hasFix();
    // Stub returns false
    TEST_ASSERT_FALSE(fix);
}

/**
 * Test satellite count
 */
void test_getSatelliteCount(void) {
    int count = gps.getSatelliteCount();
    // Stub returns 0
    TEST_ASSERT_EQUAL_INT(0, count);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_initialize);
    RUN_TEST(test_update);
    RUN_TEST(test_getLatitude);
    RUN_TEST(test_getLongitude);
    RUN_TEST(test_getAltitude);
    RUN_TEST(test_hasFix);
    RUN_TEST(test_getSatelliteCount);
    
    UNITY_END();
}

void loop() {
    // Nothing here
}
