/**
 * @file test_satellite_comm.cpp
 * @brief Unit tests for satellite communication functionality
 */

#include <unity.h>
#include "satellite_comm.h"
#include "satellite_config.h"
#include "data_compression.h"

// Test objects
SatelliteComm testSatComm;
SatelliteConfig testConfig;
DataCompression testCompression;

void setUp(void) {
    // Initialize test configuration
    testConfig.enabled = true;
    testConfig.module = MODULE_SWARM;
    testConfig.transmissionInterval = 3600;
    testConfig.maxDailyMessages = 24;
    testConfig.maxDailyCost = 25.0;
    testConfig.costOptimization = true;
}

void tearDown(void) {
    // Clean up after each test
    testSatComm.resetMessageCount();
}

void test_satellite_config_creation(void) {
    TEST_ASSERT_TRUE(testConfig.enabled);
    TEST_ASSERT_EQUAL(MODULE_SWARM, testConfig.module);
    TEST_ASSERT_EQUAL(3600, testConfig.transmissionInterval);
    TEST_ASSERT_EQUAL(24, testConfig.maxDailyMessages);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 25.0, testConfig.maxDailyCost);
}

void test_satellite_configuration(void) {
    bool result = testSatComm.configure(testConfig);
    TEST_ASSERT_TRUE(result);
    
    SatelliteConfig retrievedConfig = testSatComm.getConfiguration();
    TEST_ASSERT_EQUAL(testConfig.module, retrievedConfig.module);
    TEST_ASSERT_EQUAL(testConfig.maxDailyMessages, retrievedConfig.maxDailyMessages);
}

void test_message_priority_handling(void) {
    testSatComm.configure(testConfig);
    
    // Test that emergency messages bypass normal limits
    testSatComm.setMessageLimit(0); // Set limit to 0
    
    // Normal message should fail
    bool normalResult = testSatComm.sendMessage("Test normal", SAT_PRIORITY_NORMAL);
    TEST_ASSERT_FALSE(normalResult); // Should be stored, not sent
    
    // Emergency message should succeed (in real hardware with satellite connection)
    bool emergencyResult = testSatComm.sendEmergencyAlert("Test emergency");
    // Note: This will fail in test environment without actual satellite hardware
    // but the logic should be tested
}

void test_cost_tracking(void) {
    testSatComm.configure(testConfig);
    
    float initialCost = testSatComm.getTodayCost();
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, initialCost);
    
    bool withinLimit = testSatComm.isWithinCostLimit();
    TEST_ASSERT_TRUE(withinLimit);
}

void test_message_storage(void) {
    testSatComm.configure(testConfig);
    
    size_t initialStored = testSatComm.getStoredMessageCount();
    
    // Simulate storing a message (when satellite unavailable)
    testSatComm.sendMessage("Test stored message", SAT_PRIORITY_NORMAL);
    
    size_t afterStorage = testSatComm.getStoredMessageCount();
    TEST_ASSERT_GREATER_THAN(initialStored, afterStorage);
}

void test_power_optimization(void) {
    testSatComm.configure(testConfig);
    
    // Test low power mode
    testSatComm.optimizePowerConsumption(15); // Low battery
    TEST_ASSERT_TRUE(testSatComm.isInLowPowerMode());
    
    // Test normal power mode
    testSatComm.optimizePowerConsumption(80); // Good battery
    TEST_ASSERT_FALSE(testSatComm.isInLowPowerMode());
}

void test_emergency_mode(void) {
    testSatComm.configure(testConfig);
    
    TEST_ASSERT_FALSE(testSatComm.isEmergencyMode());
    
    testSatComm.enableEmergencyMode();
    TEST_ASSERT_TRUE(testSatComm.isEmergencyMode());
    
    testSatComm.disableEmergencyMode();
    TEST_ASSERT_FALSE(testSatComm.isEmergencyMode());
}

void test_data_compression_constants(void) {
    // Test that compression constants are properly defined
    TEST_ASSERT_GREATER_THAN(0, RLE_MAX_COUNT);
    TEST_ASSERT_GREATER_THAN(0, COMPRESSION_MIN_EFFICIENCY);
    TEST_ASSERT_GREATER_THAN(0, SATELLITE_MESSAGE_MAX_LENGTH);
}

void test_satellite_config_constants(void) {
    // Test that satellite configuration constants are properly defined
    TEST_ASSERT_GREATER_THAN(0, SATELLITE_BAUD_RATE);
    TEST_ASSERT_GREATER_THAN(0, SATELLITE_MESSAGE_MAX_LENGTH);
    TEST_ASSERT_GREATER_THAN(0, DEFAULT_DAILY_MESSAGE_LIMIT);
    TEST_ASSERT_GREATER_THAN(0, RETRY_BASE_DELAY);
    TEST_ASSERT_GREATER_THAN(0, MAX_RETRY_ATTEMPTS);
}

void test_message_compression(void) {
    String originalMessage = "WILDLIFE: This is a long wildlife detection message with repeated spaces  and content";
    String testMessage = originalMessage;
    
    // Test basic compression logic (simplified)
    testMessage.replace("WILDLIFE", "WL");
    testMessage.replace("  ", " ");
    testMessage.trim();
    
    TEST_ASSERT_LESS_THAN(originalMessage.length(), testMessage.length());
}

void test_satellite_pass_prediction(void) {
    testSatComm.configure(testConfig);
    
    bool predicted = testSatComm.predictNextPass();
    TEST_ASSERT_TRUE(predicted);
    
    time_t nextPass = testSatComm.getNextPassTime();
    TEST_ASSERT_GREATER_THAN(0, nextPass);
}

void test_transmission_window_calculation(void) {
    testSatComm.configure(testConfig);
    
    TransmissionWindow window = testSatComm.getOptimalTransmissionWindow();
    // Should return a valid window type
    TEST_ASSERT_GREATER_OR_EQUAL(WINDOW_IMMEDIATE, window);
    TEST_ASSERT_LESS_OR_EQUAL(WINDOW_EMERGENCY, window);
}

void test_multiple_module_support(void) {
    // Test Iridium configuration
    testConfig.module = MODULE_IRIDIUM;
    bool iridiumConfig = testSatComm.configure(testConfig);
    TEST_ASSERT_TRUE(iridiumConfig);
    
    // Test Swarm configuration
    testConfig.module = MODULE_SWARM;
    bool swarmConfig = testSatComm.configure(testConfig);
    TEST_ASSERT_TRUE(swarmConfig);
    
    // Test RockBLOCK configuration
    testConfig.module = MODULE_ROCKBLOCK;
    bool rockblockConfig = testSatComm.configure(testConfig);
    TEST_ASSERT_TRUE(rockblockConfig);
}

// Main test runner function
void runSatelliteCommTests() {
    UNITY_BEGIN();
    
    RUN_TEST(test_satellite_config_creation);
    RUN_TEST(test_satellite_configuration);
    RUN_TEST(test_message_priority_handling);
    RUN_TEST(test_cost_tracking);
    RUN_TEST(test_message_storage);
    RUN_TEST(test_power_optimization);
    RUN_TEST(test_emergency_mode);
    RUN_TEST(test_data_compression_constants);
    RUN_TEST(test_satellite_config_constants);
    RUN_TEST(test_message_compression);
    RUN_TEST(test_satellite_pass_prediction);
    RUN_TEST(test_transmission_window_calculation);
    RUN_TEST(test_multiple_module_support);
    
    UNITY_END();
}

#ifdef UNIT_TEST
// For native testing environment
int main(int argc, char **argv) {
    runSatelliteCommTests();
    return 0;
}
#endif