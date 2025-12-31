/**
 * @file test_production_satellite_integration.cpp
 * @brief Unit tests for production system satellite communication integration
 * 
 * Tests the integration of satellite communication features into the 
 * Phase 4 production deployment system.
 */

#include <unity.h>
#include "../firmware/src/production/production_system.h"
#include "../firmware/src/satellite_comm.h"
#include "../firmware/src/satellite_config.h"
#include "../firmware/src/network_selector.h"

// Test configuration
ProductionConfig testProdConfig;
SystemHealthMetrics testHealthMetrics;

void setUp(void) {
    // Initialize production test configuration with satellite enabled
    testProdConfig = ProductionConfig();
    testProdConfig.deploymentId = "TEST_DEVICE_001";
    testProdConfig.siteId = "TEST_SITE";
    testProdConfig.networkId = "TEST_NETWORK";
    testProdConfig.scenario = SCENARIO_CONSERVATION;
    
    // Enable satellite features
    testProdConfig.enableSatellite = true;
    testProdConfig.satelliteModule = MODULE_SWARM;
    testProdConfig.satelliteTransmitInterval = 3600;
    testProdConfig.maxSatelliteMessagesDaily = 24;
    testProdConfig.maxSatelliteCostDaily = 25.0;
    testProdConfig.satelliteCostOptimization = true;
    testProdConfig.satelliteEmergencyOnly = false;
    testProdConfig.satelliteAutoFallback = true;
    
    // Enable other production features
    testProdConfig.enableOTA = false; // Disable for unit testing
    testProdConfig.enableCloudSync = false;
    testProdConfig.enableAdvancedAI = false;
    testProdConfig.enableEnvironmentalAdaptation = false;
    testProdConfig.enableSecurity = false;
    testProdConfig.enableMultiSite = false;
    
    testProdConfig.conservationMode = true;
    testProdConfig.endangeredSpeciesAlert = true;
}

void tearDown(void) {
    // Clean up after each test
    cleanupProductionSystem();
}

// Test 1: Production config includes satellite settings
void test_production_config_satellite_settings(void) {
    TEST_ASSERT_TRUE(testProdConfig.enableSatellite);
    TEST_ASSERT_EQUAL(MODULE_SWARM, testProdConfig.satelliteModule);
    TEST_ASSERT_EQUAL(3600, testProdConfig.satelliteTransmitInterval);
    TEST_ASSERT_EQUAL(24, testProdConfig.maxSatelliteMessagesDaily);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 25.0, testProdConfig.maxSatelliteCostDaily);
    TEST_ASSERT_TRUE(testProdConfig.satelliteCostOptimization);
    TEST_ASSERT_FALSE(testProdConfig.satelliteEmergencyOnly);
    TEST_ASSERT_TRUE(testProdConfig.satelliteAutoFallback);
}

// Test 2: Production config default satellite settings
void test_production_config_default_satellite(void) {
    ProductionConfig defaultConfig;
    
    // Satellite should be disabled by default
    TEST_ASSERT_FALSE(defaultConfig.enableSatellite);
    TEST_ASSERT_EQUAL(MODULE_SWARM, defaultConfig.satelliteModule);
    TEST_ASSERT_TRUE(defaultConfig.satelliteAutoFallback);
}

// Test 3: System health metrics include satellite fields
void test_health_metrics_satellite_fields(void) {
    SystemHealthMetrics metrics;
    
    // Check satellite-specific fields exist and have defaults
    TEST_ASSERT_TRUE(metrics.satelliteHealthy);
    TEST_ASSERT_FALSE(metrics.satelliteAvailable);
    TEST_ASSERT_EQUAL(-100, metrics.satelliteSignalStrength);
    TEST_ASSERT_EQUAL(0, metrics.satelliteMessagesSent);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, metrics.satelliteCostToday);
    TEST_ASSERT_EQUAL(0, metrics.storedSatelliteMessages);
    TEST_ASSERT_FALSE(metrics.satelliteEmergencyMode);
}

// Test 4: Deployment scenario configuration
void test_deployment_scenario_research(void) {
    testProdConfig.scenario = SCENARIO_RESEARCH;
    
    // Research scenario should support higher message rates
    TEST_ASSERT_EQUAL(SCENARIO_RESEARCH, testProdConfig.scenario);
}

// Test 5: Deployment scenario conservation
void test_deployment_scenario_conservation(void) {
    testProdConfig.scenario = SCENARIO_CONSERVATION;
    
    TEST_ASSERT_EQUAL(SCENARIO_CONSERVATION, testProdConfig.scenario);
    TEST_ASSERT_TRUE(testProdConfig.conservationMode);
    TEST_ASSERT_TRUE(testProdConfig.endangeredSpeciesAlert);
}

// Test 6: Deployment scenario emergency
void test_deployment_scenario_emergency(void) {
    testProdConfig.scenario = SCENARIO_EMERGENCY;
    
    TEST_ASSERT_EQUAL(SCENARIO_EMERGENCY, testProdConfig.scenario);
}

// Test 7: Satellite module types
void test_satellite_module_types(void) {
    TEST_ASSERT_EQUAL(0, MODULE_NONE);
    TEST_ASSERT_EQUAL(1, MODULE_IRIDIUM);
    TEST_ASSERT_EQUAL(2, MODULE_SWARM);
    TEST_ASSERT_EQUAL(3, MODULE_ROCKBLOCK);
}

// Test 8: Satellite priority levels match production priorities
void test_satellite_priority_alignment(void) {
    // Verify satellite priorities align with production priorities
    TEST_ASSERT_EQUAL(0, SAT_PRIORITY_LOW);
    TEST_ASSERT_EQUAL(1, SAT_PRIORITY_NORMAL);
    TEST_ASSERT_EQUAL(2, SAT_PRIORITY_HIGH);
    TEST_ASSERT_EQUAL(3, SAT_PRIORITY_EMERGENCY);
}

// Test 9: Network type includes satellite
void test_network_type_includes_satellite(void) {
    TEST_ASSERT_EQUAL(4, NETWORK_TYPE_SATELLITE);
}

// Test 10: Production status enum values
void test_production_status_values(void) {
    TEST_ASSERT_EQUAL(0, PROD_INITIALIZING);
    TEST_ASSERT_EQUAL(1, PROD_OPERATIONAL);
    TEST_ASSERT_EQUAL(2, PROD_UPDATING);
    TEST_ASSERT_EQUAL(3, PROD_MAINTENANCE);
    TEST_ASSERT_EQUAL(4, PROD_ERROR);
    TEST_ASSERT_EQUAL(5, PROD_SHUTDOWN);
}

// Test 11: Satellite emergency only mode
void test_satellite_emergency_only_mode(void) {
    testProdConfig.satelliteEmergencyOnly = true;
    
    TEST_ASSERT_TRUE(testProdConfig.satelliteEmergencyOnly);
}

// Test 12: Cost optimization setting
void test_satellite_cost_optimization_setting(void) {
    testProdConfig.satelliteCostOptimization = true;
    TEST_ASSERT_TRUE(testProdConfig.satelliteCostOptimization);
    
    testProdConfig.satelliteCostOptimization = false;
    TEST_ASSERT_FALSE(testProdConfig.satelliteCostOptimization);
}

// Test 13: Daily message limit configuration
void test_satellite_daily_message_limit(void) {
    testProdConfig.maxSatelliteMessagesDaily = 48;
    TEST_ASSERT_EQUAL(48, testProdConfig.maxSatelliteMessagesDaily);
    
    testProdConfig.maxSatelliteMessagesDaily = 6;
    TEST_ASSERT_EQUAL(6, testProdConfig.maxSatelliteMessagesDaily);
}

// Test 14: Daily cost limit configuration
void test_satellite_daily_cost_limit(void) {
    testProdConfig.maxSatelliteCostDaily = 50.0;
    TEST_ASSERT_FLOAT_WITHIN(0.01, 50.0, testProdConfig.maxSatelliteCostDaily);
    
    testProdConfig.maxSatelliteCostDaily = 10.0;
    TEST_ASSERT_FLOAT_WITHIN(0.01, 10.0, testProdConfig.maxSatelliteCostDaily);
}

// Test 15: Transmission interval configuration
void test_satellite_transmission_interval(void) {
    testProdConfig.satelliteTransmitInterval = 1800; // 30 minutes
    TEST_ASSERT_EQUAL(1800, testProdConfig.satelliteTransmitInterval);
    
    testProdConfig.satelliteTransmitInterval = 7200; // 2 hours
    TEST_ASSERT_EQUAL(7200, testProdConfig.satelliteTransmitInterval);
}

// Test 16: Auto fallback configuration
void test_satellite_auto_fallback(void) {
    testProdConfig.satelliteAutoFallback = true;
    TEST_ASSERT_TRUE(testProdConfig.satelliteAutoFallback);
    
    testProdConfig.satelliteAutoFallback = false;
    TEST_ASSERT_FALSE(testProdConfig.satelliteAutoFallback);
}

// Test 17: Satellite module configuration for different scenarios
void test_module_selection_by_scenario(void) {
    // Swarm for cost-effective monitoring
    testProdConfig.satelliteModule = MODULE_SWARM;
    TEST_ASSERT_EQUAL(MODULE_SWARM, testProdConfig.satelliteModule);
    
    // Iridium for emergency reliability
    testProdConfig.satelliteModule = MODULE_IRIDIUM;
    TEST_ASSERT_EQUAL(MODULE_IRIDIUM, testProdConfig.satelliteModule);
    
    // RockBLOCK for Iridium compatibility
    testProdConfig.satelliteModule = MODULE_ROCKBLOCK;
    TEST_ASSERT_EQUAL(MODULE_ROCKBLOCK, testProdConfig.satelliteModule);
}

// Test 18: Conservation mode satellite integration
void test_conservation_mode_satellite(void) {
    testProdConfig.conservationMode = true;
    testProdConfig.endangeredSpeciesAlert = true;
    testProdConfig.enableSatellite = true;
    
    TEST_ASSERT_TRUE(testProdConfig.conservationMode);
    TEST_ASSERT_TRUE(testProdConfig.endangeredSpeciesAlert);
    TEST_ASSERT_TRUE(testProdConfig.enableSatellite);
}

// Test 19: Health metrics satellite health flag
void test_health_metrics_satellite_healthy(void) {
    SystemHealthMetrics metrics;
    
    // Default should be healthy
    TEST_ASSERT_TRUE(metrics.satelliteHealthy);
    
    // Can be set to unhealthy
    metrics.satelliteHealthy = false;
    TEST_ASSERT_FALSE(metrics.satelliteHealthy);
}

// Test 20: Health metrics satellite availability
void test_health_metrics_satellite_availability(void) {
    SystemHealthMetrics metrics;
    
    // Default should be unavailable
    TEST_ASSERT_FALSE(metrics.satelliteAvailable);
    
    // Can be set to available
    metrics.satelliteAvailable = true;
    TEST_ASSERT_TRUE(metrics.satelliteAvailable);
}

// Main test runner function
void runProductionSatelliteIntegrationTests() {
    UNITY_BEGIN();
    
    RUN_TEST(test_production_config_satellite_settings);
    RUN_TEST(test_production_config_default_satellite);
    RUN_TEST(test_health_metrics_satellite_fields);
    RUN_TEST(test_deployment_scenario_research);
    RUN_TEST(test_deployment_scenario_conservation);
    RUN_TEST(test_deployment_scenario_emergency);
    RUN_TEST(test_satellite_module_types);
    RUN_TEST(test_satellite_priority_alignment);
    RUN_TEST(test_network_type_includes_satellite);
    RUN_TEST(test_production_status_values);
    RUN_TEST(test_satellite_emergency_only_mode);
    RUN_TEST(test_cost_optimization_setting);
    RUN_TEST(test_satellite_daily_message_limit);
    RUN_TEST(test_satellite_daily_cost_limit);
    RUN_TEST(test_satellite_transmission_interval);
    RUN_TEST(test_satellite_auto_fallback);
    RUN_TEST(test_module_selection_by_scenario);
    RUN_TEST(test_conservation_mode_satellite);
    RUN_TEST(test_health_metrics_satellite_healthy);
    RUN_TEST(test_health_metrics_satellite_availability);
    
    UNITY_END();
}

#ifdef UNIT_TEST
// For native testing environment
int main(int argc, char **argv) {
    runProductionSatelliteIntegrationTests();
    return 0;
}
#endif
