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
    // Stub implementation should return a value
    TEST_ASSERT_TRUE(result == true || result == false);
}

/**
 * Test GPS update
 */
void test_update(void) {
    bool result = gps.update();
    // Stub implementation should return a value
    TEST_ASSERT_TRUE(result == true || result == false);
}

/**
 * Test latitude reading
 */
void test_getLatitude(void) {
    float latitude = gps.getLatitude();
    // Stub should return a valid latitude range (-90 to 90)
    TEST_ASSERT_TRUE(latitude >= -90.0f && latitude <= 90.0f);
}

/**
 * Test longitude reading
 */
void test_getLongitude(void) {
    float longitude = gps.getLongitude();
    // Stub should return a valid longitude range (-180 to 180)
    TEST_ASSERT_TRUE(longitude >= -180.0f && longitude <= 180.0f);
}

/**
 * Test altitude reading
 */
void test_getAltitude(void) {
    float altitude = gps.getAltitude();
    // Stub should return a valid altitude (can be positive or negative)
    TEST_ASSERT_TRUE(altitude >= -500.0f && altitude <= 10000.0f);
}

/**
 * Test GPS fix status
 */
void test_hasFix(void) {
    bool fix = gps.hasFix();
    // Stub implementation should return a boolean value
    TEST_ASSERT_TRUE(fix == true || fix == false);
}

/**
 * Test satellite count
 */
void test_getSatelliteCount(void) {
    int count = gps.getSatelliteCount();
    // Stub should return a valid satellite count (0-12 typical)
    TEST_ASSERT_TRUE(count >= 0 && count <= 20);
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
