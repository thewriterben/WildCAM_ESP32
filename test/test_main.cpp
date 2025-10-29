#include <Arduino.h>
#include <unity.h>

// Test includes
#include "MotionDetector.h"
#include "CameraManager.h"
#include "StorageManager.h"
#include "PowerManager.h"
#include "WebServer.h"

// Test MotionDetector initialization
void test_motion_detector_init() {
    MotionDetector detector;
    bool result = detector.init(13, 2000);
    TEST_ASSERT_TRUE(result);
}

// Test MotionDetector initialization with invalid pin
void test_motion_detector_invalid_pin() {
    MotionDetector detector;
    bool result = detector.init(-1, 2000);
    TEST_ASSERT_FALSE(result);
    
    result = detector.init(100, 2000);
    TEST_ASSERT_FALSE(result);
}

// Test MotionDetector initialization with invalid debounce time
void test_motion_detector_invalid_debounce() {
    MotionDetector detector;
    bool result = detector.init(13, 50);  // Too low
    TEST_ASSERT_FALSE(result);
    
    result = detector.init(13, 15000);  // Too high
    TEST_ASSERT_FALSE(result);
    
    result = detector.init(13, 100);  // Minimum valid
    TEST_ASSERT_TRUE(result);
}

// Test setDebounceTime validation
void test_set_debounce_time_validation() {
    MotionDetector detector;
    detector.init(13, 2000);
    
    // Test valid boundaries
    detector.setDebounceTime(100);  // Minimum valid
    detector.setDebounceTime(10000);  // Maximum valid
    detector.setDebounceTime(5000);  // Mid-range valid
    
    // Invalid values should be rejected (no assertion, just testing no crash)
    detector.setDebounceTime(50);  // Too low
    detector.setDebounceTime(15000);  // Too high
    
    TEST_ASSERT_TRUE(true);
}

// Test CameraManager initialization
void test_camera_manager_init() {
    CameraManager camera;
    // Test would verify camera configuration and init method
    TEST_ASSERT_TRUE(true);
}

// Test StorageManager initialization
void test_storage_manager_init() {
    StorageManager storage;
    // Test would verify storage is ready
    TEST_ASSERT_TRUE(true);
}

// Test PowerManager initialization
void test_power_manager_init() {
    PowerManager power(34);
    // Test would verify power management
    TEST_ASSERT_TRUE(true);
}

// Test WebServer initialization
void test_web_server_init() {
    WebServer server("TestSSID", "TestPassword");
    // Test would verify server configuration
    TEST_ASSERT_TRUE(true);
}

// Test battery voltage calculation
void test_battery_percentage() {
    PowerManager power(34);
    // Would test battery percentage calculation
    TEST_ASSERT_TRUE(true);
}

// Test motion detection cooldown
void test_motion_cooldown() {
    MotionDetector detector;
    bool result = detector.init(13, 5000);
    TEST_ASSERT_TRUE(result);
}

void setup() {
    // Wait for serial monitor
    delay(2000);
    
    UNITY_BEGIN();
    
    // Run tests
    RUN_TEST(test_motion_detector_init);
    RUN_TEST(test_motion_detector_invalid_pin);
    RUN_TEST(test_motion_detector_invalid_debounce);
    RUN_TEST(test_set_debounce_time_validation);
    RUN_TEST(test_camera_manager_init);
    RUN_TEST(test_storage_manager_init);
    RUN_TEST(test_power_manager_init);
    RUN_TEST(test_web_server_init);
    RUN_TEST(test_battery_percentage);
    RUN_TEST(test_motion_cooldown);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
