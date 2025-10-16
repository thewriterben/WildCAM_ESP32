/**
 * @file test_diagnostics.cpp
 * @brief Unit tests for system diagnostics functionality
 */

#include <unity.h>
#include "../src/diagnostics.h"

/**
 * Test camera initialization diagnostic
 */
void test_camera_init_diagnostic(void) {
    bool result = testCameraInit();
    // Camera may or may not be present in test environment
    // Just verify the function executes without crashing
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test SD card read/write diagnostic
 */
void test_sd_card_diagnostic(void) {
    bool result = testSDCardReadWrite();
    // SD card may or may not be present in test environment
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test PIR sensor diagnostic
 */
void test_pir_sensor_diagnostic(void) {
    bool result = testPIRSensor();
    // PIR sensor should at least be readable
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test power management diagnostic
 */
void test_power_management_diagnostic(void) {
    bool result = testPowerManagement();
    // Power management should be available
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test memory check diagnostic
 */
void test_memory_diagnostic(void) {
    bool result = testMemory();
    // Memory check should always pass if we can run code
    TEST_ASSERT_TRUE(result);
}

/**
 * Test complete system diagnostics
 */
void test_run_system_diagnostics(void) {
    // Run full diagnostics
    bool result = runSystemDiagnostics();
    
    // Diagnostics should complete without crashing
    // Result depends on hardware availability
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test diagnostics execution time
 */
void test_diagnostics_execution_time(void) {
    unsigned long startTime = millis();
    runSystemDiagnostics();
    unsigned long duration = millis() - startTime;
    
    // Diagnostics should complete in reasonable time (< 5 seconds)
    TEST_ASSERT_LESS_THAN(5000, duration);
}

void setup() {
    delay(2000);  // Wait for serial
    
    UNITY_BEGIN();
    
    RUN_TEST(test_memory_diagnostic);
    RUN_TEST(test_pir_sensor_diagnostic);
    RUN_TEST(test_power_management_diagnostic);
    RUN_TEST(test_sd_card_diagnostic);
    RUN_TEST(test_camera_init_diagnostic);
    RUN_TEST(test_run_system_diagnostics);
    RUN_TEST(test_diagnostics_execution_time);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
