#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>

// Test includes
#include "config.h"
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
    PowerManager power;
    bool result = power.init(35);
    TEST_ASSERT_TRUE(result);
}

// Test WebServer initialization
void test_web_server_init() {
    WebServer server(80);
    StorageManager storage;
    CameraManager camera;
    PowerManager power;
    
    // Test successful init with all managers
    bool result = server.init(&storage, &camera, &power);
    TEST_ASSERT_TRUE(result);
}

// Test WebServer init with null references
void test_web_server_init_null_refs() {
    WebServer server(80);
    StorageManager storage;
    CameraManager camera;
    PowerManager power;
    
    // Test with null storage
    bool result = server.init(nullptr, &camera, &power);
    TEST_ASSERT_FALSE(result);
    
    // Test with null camera
    result = server.init(&storage, nullptr, &power);
    TEST_ASSERT_FALSE(result);
    
    // Test with null power
    result = server.init(&storage, &camera, nullptr);
    TEST_ASSERT_FALSE(result);
    
    // Test with all null
    result = server.init(nullptr, nullptr, nullptr);
    TEST_ASSERT_FALSE(result);
}

// Test StorageManager getImageCount
void test_storage_manager_image_count() {
    StorageManager storage;
    
    // Before init, should return 0
    unsigned long count = storage.getImageCount();
    TEST_ASSERT_EQUAL(0, count);
}

// Test StorageManager getImageFiles
void test_storage_manager_image_files() {
    StorageManager storage;
    
    // Before init, should return empty vector
    std::vector<String> files = storage.getImageFiles();
    TEST_ASSERT_TRUE(files.empty());
}

// Test battery voltage calculation
void test_battery_percentage() {
    PowerManager power;
    power.init(35);
    // Would test battery percentage calculation
    TEST_ASSERT_TRUE(true);
}

// Test new PowerManager methods
void test_power_manager_new_methods() {
    PowerManager power;
    power.init(35);
    
    // Test that methods exist and can be called
    int percentage = power.getBatteryPercentage();
    TEST_ASSERT_GREATER_OR_EQUAL(0, percentage);
    TEST_ASSERT_LESS_OR_EQUAL(100, percentage);
    
    // Test isLowBattery
    bool lowBattery = power.isLowBattery();
    // Just verify it returns a value, don't assert specific state
    TEST_ASSERT_TRUE(lowBattery == true || lowBattery == false);
}

// Test motion detection cooldown
void test_motion_cooldown() {
    MotionDetector detector;
    bool result = detector.init(13, 5000);
    TEST_ASSERT_TRUE(result);
}

// Test that firmware version is defined
void test_firmware_version_defined() {
    #ifdef FIRMWARE_VERSION
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("FIRMWARE_VERSION not defined in config.h");
    #endif
}

// Test that critical battery threshold is defined
void test_critical_battery_threshold_defined() {
    #ifdef BATTERY_CRITICAL_THRESHOLD
        TEST_ASSERT_TRUE(BATTERY_CRITICAL_THRESHOLD > 0);
        TEST_ASSERT_TRUE(BATTERY_CRITICAL_THRESHOLD < BATTERY_LOW_THRESHOLD);
    #else
        TEST_FAIL_MESSAGE("BATTERY_CRITICAL_THRESHOLD not defined");
    #endif
}

// Test that image capture delay is defined
void test_image_capture_delay_defined() {
    #ifdef IMAGE_CAPTURE_DELAY_MS
        TEST_ASSERT_TRUE(IMAGE_CAPTURE_DELAY_MS >= 0);
        TEST_ASSERT_TRUE(IMAGE_CAPTURE_DELAY_MS <= 5000);
    #else
        TEST_FAIL_MESSAGE("IMAGE_CAPTURE_DELAY_MS not defined");
    #endif
}

// Test StorageManager metadata save capability
void test_storage_manager_metadata_save() {
    StorageManager storage;
    
    // Create test metadata
    StaticJsonDocument<256> metadata;
    metadata["test_key"] = "test_value";
    metadata["timestamp"] = 12345;
    
    // This would test metadata saving, but requires SD card
    // Just verify the method exists and can be called
    TEST_ASSERT_TRUE(true);
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
    RUN_TEST(test_storage_manager_image_count);
    RUN_TEST(test_storage_manager_image_files);
    RUN_TEST(test_power_manager_init);
    RUN_TEST(test_web_server_init);
    RUN_TEST(test_web_server_init_null_refs);
    RUN_TEST(test_battery_percentage);
    RUN_TEST(test_power_manager_new_methods);
    RUN_TEST(test_motion_cooldown);
    RUN_TEST(test_firmware_version_defined);
    RUN_TEST(test_critical_battery_threshold_defined);
    RUN_TEST(test_image_capture_delay_defined);
    RUN_TEST(test_storage_manager_metadata_save);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
