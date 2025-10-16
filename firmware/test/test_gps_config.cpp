/**
 * @file test_gps_config.cpp
 * @brief Unit tests for GPS configuration settings
 * 
 * Tests verify that GPS configuration values from config.h are
 * correctly defined and accessible to the GPS handler.
 */

#include <unity.h>
#include "../src/config.h"

void setUp(void) {
    // Initialize before each test
}

void tearDown(void) {
    // Cleanup after each test
}

/**
 * Test that GPS_ENABLED is defined
 */
void test_gps_enabled_is_defined(void) {
    #ifdef GPS_ENABLED
    TEST_PASS();
    #else
    TEST_FAIL_MESSAGE("GPS_ENABLED is not defined");
    #endif
}

/**
 * Test that GPS baud rate is defined and valid
 */
void test_gps_baud_rate_defined(void) {
    #ifdef GPS_BAUD_RATE
    TEST_ASSERT_GREATER_THAN(0, GPS_BAUD_RATE);
    TEST_ASSERT_EQUAL(9600, GPS_BAUD_RATE);
    #else
    TEST_FAIL_MESSAGE("GPS_BAUD_RATE is not defined");
    #endif
}

/**
 * Test that GPS timeout values are defined and reasonable
 */
void test_gps_timeout_values(void) {
    #ifdef GPS_FIX_TIMEOUT
    TEST_ASSERT_GREATER_THAN(0, GPS_FIX_TIMEOUT);
    TEST_ASSERT_EQUAL(60000, GPS_FIX_TIMEOUT);
    #else
    TEST_FAIL_MESSAGE("GPS_FIX_TIMEOUT is not defined");
    #endif
    
    #ifdef GPS_COLD_START_TIMEOUT
    TEST_ASSERT_GREATER_THAN(0, GPS_COLD_START_TIMEOUT);
    TEST_ASSERT_EQUAL(120000, GPS_COLD_START_TIMEOUT);
    #else
    TEST_FAIL_MESSAGE("GPS_COLD_START_TIMEOUT is not defined");
    #endif
}

/**
 * Test that GPS pins are defined when GPS is enabled
 */
void test_gps_pins_defined_when_enabled(void) {
    #if GPS_ENABLED
        #ifdef GPS_RX_PIN
        TEST_ASSERT_GREATER_OR_EQUAL(0, GPS_RX_PIN);
        #else
        TEST_FAIL_MESSAGE("GPS_RX_PIN is not defined when GPS_ENABLED is true");
        #endif
        
        #ifdef GPS_TX_PIN
        TEST_ASSERT_GREATER_OR_EQUAL(0, GPS_TX_PIN);
        #else
        TEST_FAIL_MESSAGE("GPS_TX_PIN is not defined when GPS_ENABLED is true");
        #endif
        
        // GPS_EN_PIN is optional, so we don't require it
        TEST_PASS_MESSAGE("GPS pins are properly defined");
    #else
        TEST_PASS_MESSAGE("GPS is disabled, pin tests skipped");
    #endif
}

/**
 * Test AI-Thinker specific GPS pin assignments
 */
void test_ai_thinker_gps_pins(void) {
    #if defined(CAMERA_MODEL_AI_THINKER) && GPS_ENABLED
        // Verify correct pin assignments for AI-Thinker board
        TEST_ASSERT_EQUAL(33, GPS_RX_PIN);  // Should use GPIO 33
        TEST_ASSERT_EQUAL(13, GPS_TX_PIN);  // Should use GPIO 13
        
        #ifdef GPS_EN_PIN
        TEST_ASSERT_EQUAL(12, GPS_EN_PIN);  // Should use GPIO 12
        #endif
        
        TEST_PASS_MESSAGE("AI-Thinker GPS pins correctly configured");
    #else
        TEST_PASS_MESSAGE("Not AI-Thinker board or GPS disabled");
    #endif
}

/**
 * Test GPS pins don't conflict with critical camera pins
 */
void test_gps_no_camera_conflicts(void) {
    #if defined(CAMERA_MODEL_AI_THINKER) && GPS_ENABLED
        // GPS pins should NOT be:
        // - GPIO 32 (PWDN_GPIO_NUM)
        // - GPIO 25 (VSYNC_GPIO_NUM)
        
        TEST_ASSERT_NOT_EQUAL(32, GPS_RX_PIN);
        TEST_ASSERT_NOT_EQUAL(32, GPS_TX_PIN);
        TEST_ASSERT_NOT_EQUAL(25, GPS_RX_PIN);
        TEST_ASSERT_NOT_EQUAL(25, GPS_TX_PIN);
        
        #ifdef GPS_EN_PIN
        TEST_ASSERT_NOT_EQUAL(32, GPS_EN_PIN);
        TEST_ASSERT_NOT_EQUAL(25, GPS_EN_PIN);
        #endif
        
        TEST_PASS_MESSAGE("GPS pins do not conflict with critical camera pins");
    #else
        TEST_PASS_MESSAGE("Not AI-Thinker board or GPS disabled");
    #endif
}

/**
 * Test GPS feature flags are defined
 */
void test_gps_feature_flags(void) {
    #ifdef GPS_LOCATION_LOGGING
    TEST_PASS();
    #else
    TEST_FAIL_MESSAGE("GPS_LOCATION_LOGGING is not defined");
    #endif
    
    #ifdef GPS_MOVEMENT_DETECTION
    TEST_PASS();
    #else
    TEST_FAIL_MESSAGE("GPS_MOVEMENT_DETECTION is not defined");
    #endif
    
    #ifdef GPS_POWER_SAVE_ENABLED
    TEST_PASS();
    #else
    TEST_FAIL_MESSAGE("GPS_POWER_SAVE_ENABLED is not defined");
    #endif
}

/**
 * Test GPS movement threshold is reasonable
 */
void test_gps_movement_threshold(void) {
    #ifdef GPS_MOVEMENT_THRESHOLD
    TEST_ASSERT_GREATER_THAN(0.0f, GPS_MOVEMENT_THRESHOLD);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, GPS_MOVEMENT_THRESHOLD);
    #else
    TEST_FAIL_MESSAGE("GPS_MOVEMENT_THRESHOLD is not defined");
    #endif
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_gps_enabled_is_defined);
    RUN_TEST(test_gps_baud_rate_defined);
    RUN_TEST(test_gps_timeout_values);
    RUN_TEST(test_gps_pins_defined_when_enabled);
    RUN_TEST(test_ai_thinker_gps_pins);
    RUN_TEST(test_gps_no_camera_conflicts);
    RUN_TEST(test_gps_feature_flags);
    RUN_TEST(test_gps_movement_threshold);
    
    UNITY_END();
}

void loop() {
    // Nothing here
}
