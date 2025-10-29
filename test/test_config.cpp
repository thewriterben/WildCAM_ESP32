#include <Arduino.h>
#include <unity.h>
#include "config.h"

/**
 * @file test_config.cpp
 * @brief Unit tests for configuration constants
 * 
 * This test file verifies that all required configuration constants
 * are defined in config.h with correct values and types.
 */

//==============================================================================
// PIN DEFINITIONS TESTS
//==============================================================================

void test_pin_definitions() {
    // Test that pin definitions exist and are valid GPIO numbers
    TEST_ASSERT_EQUAL(13, PIR_SENSOR_PIN);
    TEST_ASSERT_EQUAL(33, BATTERY_MONITOR_PIN);
    TEST_ASSERT_EQUAL(5, SD_CS_PIN);
    TEST_ASSERT_EQUAL(4, FLASH_LED_PIN);
    
    // Verify backward compatibility
    TEST_ASSERT_EQUAL(BATTERY_MONITOR_PIN, BATTERY_ADC_PIN);
}

//==============================================================================
// TIMING CONSTANTS TESTS
//==============================================================================

void test_timing_constants() {
    // Test timing constants have correct default values
    TEST_ASSERT_EQUAL(2000, MOTION_DEBOUNCE_MS);
    TEST_ASSERT_EQUAL(300, DEEP_SLEEP_DURATION_SEC);
    TEST_ASSERT_EQUAL(500, IMAGE_CAPTURE_DELAY_MS);
    TEST_ASSERT_EQUAL(5000, MOTION_COOLDOWN_MS);
    
    // Verify backward compatibility
    TEST_ASSERT_EQUAL(DEEP_SLEEP_DURATION_SEC, DEEP_SLEEP_DURATION);
}

//==============================================================================
// CAMERA SETTINGS TESTS
//==============================================================================

void test_camera_settings() {
    // Test camera settings have correct default values
    TEST_ASSERT_EQUAL(10, DEFAULT_JPEG_QUALITY);
    TEST_ASSERT_EQUAL(FRAMESIZE_UXGA, DEFAULT_FRAME_SIZE);
    TEST_ASSERT_EQUAL(50, MOTION_SENSITIVITY);
    
    // Verify backward compatibility
    TEST_ASSERT_EQUAL(DEFAULT_JPEG_QUALITY, JPEG_QUALITY);
    TEST_ASSERT_EQUAL(DEFAULT_FRAME_SIZE, FRAME_SIZE);
}

//==============================================================================
// POWER MANAGEMENT TESTS
//==============================================================================

void test_power_management() {
    // Test power management thresholds
    TEST_ASSERT_EQUAL_FLOAT(3.3, BATTERY_LOW_THRESHOLD);
    TEST_ASSERT_EQUAL_FLOAT(3.0, BATTERY_CRITICAL_THRESHOLD);
    TEST_ASSERT_EQUAL_FLOAT(3.3, BATTERY_VOLTAGE_MIN);
    TEST_ASSERT_EQUAL_FLOAT(4.2, BATTERY_VOLTAGE_MAX);
    
    // Verify backward compatibility
    TEST_ASSERT_EQUAL_FLOAT(BATTERY_LOW_THRESHOLD, LOW_BATTERY_THRESHOLD);
    
    // Verify critical threshold is lower than low threshold
    TEST_ASSERT_TRUE(BATTERY_CRITICAL_THRESHOLD < BATTERY_LOW_THRESHOLD);
}

//==============================================================================
// STORAGE SETTINGS TESTS
//==============================================================================

void test_storage_settings() {
    // Test storage settings
    TEST_ASSERT_EQUAL(1000, MAX_IMAGES_PER_DAY);
    TEST_ASSERT_EQUAL(7, AUTO_DELETE_DAYS);
    TEST_ASSERT_EQUAL(100000, MAX_IMAGE_SIZE);
}

//==============================================================================
// NETWORK SETTINGS TESTS
//==============================================================================

void test_network_settings() {
    // Test that network settings are defined (not null)
    TEST_ASSERT_NOT_NULL(WIFI_SSID);
    TEST_ASSERT_NOT_NULL(WIFI_PASSWORD);
    TEST_ASSERT_EQUAL(80, WEB_SERVER_PORT);
    TEST_ASSERT_EQUAL(81, WEB_SOCKET_PORT);
}

//==============================================================================
// CAMERA PIN DEFINITIONS TESTS
//==============================================================================

void test_camera_pins() {
    // Test critical camera pins are defined
    TEST_ASSERT_EQUAL(32, PWDN_GPIO_NUM);
    TEST_ASSERT_EQUAL(-1, RESET_GPIO_NUM);
    TEST_ASSERT_EQUAL(0, XCLK_GPIO_NUM);
    TEST_ASSERT_EQUAL(26, SIOD_GPIO_NUM);
    TEST_ASSERT_EQUAL(27, SIOC_GPIO_NUM);
    
    // Test data pins
    TEST_ASSERT_EQUAL(35, Y9_GPIO_NUM);
    TEST_ASSERT_EQUAL(34, Y8_GPIO_NUM);
    TEST_ASSERT_EQUAL(39, Y7_GPIO_NUM);
    TEST_ASSERT_EQUAL(36, Y6_GPIO_NUM);
    TEST_ASSERT_EQUAL(21, Y5_GPIO_NUM);
    TEST_ASSERT_EQUAL(19, Y4_GPIO_NUM);
    TEST_ASSERT_EQUAL(18, Y3_GPIO_NUM);
    TEST_ASSERT_EQUAL(5, Y2_GPIO_NUM);
    
    // Test sync pins
    TEST_ASSERT_EQUAL(25, VSYNC_GPIO_NUM);
    TEST_ASSERT_EQUAL(23, HREF_GPIO_NUM);
    TEST_ASSERT_EQUAL(22, PCLK_GPIO_NUM);
}

//==============================================================================
// CONFIGURATION CONSISTENCY TESTS
//==============================================================================

void test_configuration_consistency() {
    // Test that configuration values are logically consistent
    
    // Battery voltages should be in correct order
    TEST_ASSERT_TRUE(BATTERY_CRITICAL_THRESHOLD < BATTERY_LOW_THRESHOLD);
    TEST_ASSERT_TRUE(BATTERY_LOW_THRESHOLD <= BATTERY_VOLTAGE_MIN);
    TEST_ASSERT_TRUE(BATTERY_VOLTAGE_MIN < BATTERY_VOLTAGE_MAX);
    
    // JPEG quality should be in valid range (1-63)
    TEST_ASSERT_GREATER_THAN(0, DEFAULT_JPEG_QUALITY);
    TEST_ASSERT_LESS_THAN(64, DEFAULT_JPEG_QUALITY);
    
    // Timing values should be positive
    TEST_ASSERT_GREATER_THAN(0, MOTION_DEBOUNCE_MS);
    TEST_ASSERT_GREATER_THAN(0, DEEP_SLEEP_DURATION_SEC);
    TEST_ASSERT_GREATER_THAN(0, IMAGE_CAPTURE_DELAY_MS);
    TEST_ASSERT_GREATER_THAN(0, MOTION_COOLDOWN_MS);
    
    // Storage limits should be positive
    TEST_ASSERT_GREATER_THAN(0, MAX_IMAGES_PER_DAY);
    TEST_ASSERT_GREATER_THAN(0, AUTO_DELETE_DAYS);
    TEST_ASSERT_GREATER_THAN(0, MAX_IMAGE_SIZE);
}

//==============================================================================
// TEST RUNNER
//==============================================================================

void setup() {
    // Wait for serial connection
    delay(2000);
    
    UNITY_BEGIN();
    
    // Run all tests
    RUN_TEST(test_pin_definitions);
    RUN_TEST(test_timing_constants);
    RUN_TEST(test_camera_settings);
    RUN_TEST(test_power_management);
    RUN_TEST(test_storage_settings);
    RUN_TEST(test_network_settings);
    RUN_TEST(test_camera_pins);
    RUN_TEST(test_configuration_consistency);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
