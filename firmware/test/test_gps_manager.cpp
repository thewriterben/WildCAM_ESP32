/**
 * @file test_gps_manager.cpp
 * @brief Unit tests for GPS Manager
 */

#include <unity.h>
#include "../src/sensors/gps_manager.h"

// Mock GPS manager instance
GPSManager* testGPS = nullptr;

void setUp(void) {
    // Initialize before each test
    if (testGPS == nullptr) {
        testGPS = new GPSManager();
    }
}

void tearDown(void) {
    // Cleanup after each test
    // Don't delete testGPS as we reuse it across tests
}

/**
 * Test GPS Manager creation
 */
void test_gpsManagerCreation(void) {
    TEST_ASSERT_NOT_NULL(testGPS);
    TEST_ASSERT_FALSE(testGPS->isInitialized());
}

/**
 * Test GPS Manager initialization with valid pins
 */
void test_gpsManagerInitialization(void) {
    // Initialize with test pins (32, 33)
    bool result = testGPS->initialize(32, 33, 9600);
    
    // Should succeed
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testGPS->isInitialized());
}

/**
 * Test GPS Manager initialization with invalid pins
 */
void test_gpsManagerInvalidPins(void) {
    GPSManager* invalidGPS = new GPSManager();
    
    // Try to initialize with invalid pins
    bool result = invalidGPS->initialize(-1, -1, 9600);
    
    // Should fail
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(invalidGPS->isInitialized());
    
    delete invalidGPS;
}

/**
 * Test GPS fix status before fix acquired
 */
void test_gpsFixStatusNoFix(void) {
    TEST_ASSERT_FALSE(testGPS->hasFix());
}

/**
 * Test GPS getters return zero/default values without fix
 */
void test_gpsGettersNoFix(void) {
    float lat = testGPS->getLatitude();
    float lon = testGPS->getLongitude();
    float alt = testGPS->getAltitude();
    int sats = testGPS->getSatelliteCount();
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, lat);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, lon);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, alt);
    TEST_ASSERT_EQUAL_INT(0, sats);
}

/**
 * Test GPS update method doesn't crash
 */
void test_gpsUpdate(void) {
    // Update should not crash even without GPS data
    bool result = testGPS->update();
    
    // Should return false since no GPS data available in test
    TEST_ASSERT_FALSE(result);
}

/**
 * Test GPS location string without fix
 */
void test_gpsLocationStringNoFix(void) {
    String locationStr = testGPS->getLocationString();
    
    // Should return "No GPS fix" when no fix
    TEST_ASSERT_TRUE(locationStr.indexOf("No GPS fix") >= 0);
}

/**
 * Test GPS HDOP without fix
 */
void test_gpsHDOPNoFix(void) {
    float hdop = testGPS->getHDOP();
    
    // Should return high value (99.9) when no fix
    TEST_ASSERT_EQUAL_FLOAT(99.9f, hdop);
}

/**
 * Test GPS speed without fix
 */
void test_gpsSpeedNoFix(void) {
    float speed = testGPS->getSpeedKmph();
    
    // Should return 0.0 when no fix
    TEST_ASSERT_EQUAL_FLOAT(0.0f, speed);
}

/**
 * Test GPS course without fix
 */
void test_gpsCourseNoFix(void) {
    float course = testGPS->getCourse();
    
    // Should return 0.0 when no fix
    TEST_ASSERT_EQUAL_FLOAT(0.0f, course);
}

/**
 * Test GPS time since last fix
 */
void test_gpsTimeSinceLastFix(void) {
    unsigned long time = testGPS->getTimeSinceLastFix();
    
    // Should return 0 when no fix ever acquired
    TEST_ASSERT_EQUAL_UINT32(0, time);
}

/**
 * Test wait for fix with short timeout (should timeout)
 */
void test_gpsWaitForFixTimeout(void) {
    // Wait for fix with 100ms timeout (should timeout since no GPS connected)
    bool result = testGPS->waitForFix(100);
    
    // Should return false (timeout)
    TEST_ASSERT_FALSE(result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_gpsManagerCreation);
    RUN_TEST(test_gpsManagerInitialization);
    RUN_TEST(test_gpsManagerInvalidPins);
    RUN_TEST(test_gpsFixStatusNoFix);
    RUN_TEST(test_gpsGettersNoFix);
    RUN_TEST(test_gpsUpdate);
    RUN_TEST(test_gpsLocationStringNoFix);
    RUN_TEST(test_gpsHDOPNoFix);
    RUN_TEST(test_gpsSpeedNoFix);
    RUN_TEST(test_gpsCourseNoFix);
    RUN_TEST(test_gpsTimeSinceLastFix);
    RUN_TEST(test_gpsWaitForFixTimeout);
    
    // Cleanup
    if (testGPS != nullptr) {
        delete testGPS;
        testGPS = nullptr;
    }
    
    UNITY_END();
}

void loop() {
    // Nothing here
}
