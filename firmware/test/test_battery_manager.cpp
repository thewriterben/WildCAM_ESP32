/**
 * @file test_battery_manager.cpp
 * @brief Unit tests for advanced battery management system
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 */

#include <unity.h>
#include "../power/battery_manager.h"

// Mock pins for testing
static const int TEST_VOLTAGE_PIN = 34;
static const int TEST_CURRENT_PIN = 35;
static const int TEST_TEMP_PIN = 32;
static const int TEST_CHARGE_PIN = 25;

static BatteryManager* battery_manager = nullptr;

void setUp(void) {
    // Initialize battery manager before each test
    if (battery_manager == nullptr) {
        battery_manager = new BatteryManager(TEST_VOLTAGE_PIN, TEST_CURRENT_PIN,
                                              TEST_TEMP_PIN, TEST_CHARGE_PIN);
        battery_manager->begin();
    }
}

void tearDown(void) {
    // Cleanup after all tests
}

/**
 * Test battery manager initialization
 */
void test_battery_manager_init(void) {
    BatteryManager bm(TEST_VOLTAGE_PIN);
    bool result = bm.begin();
    TEST_ASSERT_TRUE(result);
}

/**
 * Test default Li-ion configuration
 */
void test_default_lithium_ion_config(void) {
    BatteryConfig config = BatteryManager::getDefaultConfig(BatteryChemistry::LITHIUM_ION);
    
    TEST_ASSERT_EQUAL(BatteryChemistry::LITHIUM_ION, config.chemistry);
    TEST_ASSERT_EQUAL(1, config.cell_count);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 4.2f, config.charge_voltage);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 3.0f, config.discharge_cutoff);
    TEST_ASSERT_TRUE(config.nominal_capacity > 0);
}

/**
 * Test default LiFePO4 configuration
 */
void test_default_lifepo4_config(void) {
    BatteryConfig config = BatteryManager::getDefaultConfig(BatteryChemistry::LIFEPO4);
    
    TEST_ASSERT_EQUAL(BatteryChemistry::LIFEPO4, config.chemistry);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 3.65f, config.charge_voltage);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 2.5f, config.discharge_cutoff);
}

/**
 * Test default lead-acid configuration
 */
void test_default_lead_acid_config(void) {
    BatteryConfig config = BatteryManager::getDefaultConfig(BatteryChemistry::LEAD_ACID);
    
    TEST_ASSERT_EQUAL(BatteryChemistry::LEAD_ACID, config.chemistry);
    TEST_ASSERT_EQUAL(6, config.cell_count);  // 12V battery
}

/**
 * Test voltage reading returns valid range
 */
void test_voltage_reading_valid_range(void) {
    float voltage = battery_manager->getVoltage();
    
    // Voltage should be in reasonable range for Li-ion
    TEST_ASSERT_TRUE(voltage >= 0.0f);
    TEST_ASSERT_TRUE(voltage <= 5.0f);  // Single cell max with margin
}

/**
 * Test state of charge is within valid bounds
 */
void test_soc_valid_bounds(void) {
    float soc = battery_manager->getStateOfCharge();
    
    TEST_ASSERT_TRUE(soc >= 0.0f);
    TEST_ASSERT_TRUE(soc <= 100.0f);
}

/**
 * Test state of health is within valid bounds
 */
void test_soh_valid_bounds(void) {
    float soh = battery_manager->getStateOfHealth();
    
    TEST_ASSERT_TRUE(soh >= 0.0f);
    TEST_ASSERT_TRUE(soh <= 100.0f);
}

/**
 * Test charging enable/disable
 */
void test_charging_enable_disable(void) {
    battery_manager->setChargingEnabled(true);
    TEST_ASSERT_TRUE(battery_manager->isChargingEnabled());
    
    battery_manager->setChargingEnabled(false);
    TEST_ASSERT_FALSE(battery_manager->isChargingEnabled());
    
    // Re-enable for other tests
    battery_manager->setChargingEnabled(true);
}

/**
 * Test fault detection - no fault on init
 */
void test_no_initial_fault(void) {
    BatteryManager bm(TEST_VOLTAGE_PIN);
    bm.begin();
    
    TEST_ASSERT_FALSE(bm.hasFault());
    TEST_ASSERT_EQUAL(BatteryFault::NONE, bm.getFault());
}

/**
 * Test fault clear functionality
 */
void test_fault_clear(void) {
    battery_manager->clearFault();
    TEST_ASSERT_FALSE(battery_manager->hasFault());
}

/**
 * Test initial charging stage is IDLE
 */
void test_initial_charging_stage_idle(void) {
    BatteryManager bm(TEST_VOLTAGE_PIN);
    bm.begin();
    
    TEST_ASSERT_EQUAL(ChargingStage::IDLE, bm.getChargingStage());
}

/**
 * Test status structure is populated
 */
void test_status_structure(void) {
    BatteryStatus status = battery_manager->getStatus();
    
    // All fields should be accessible
    TEST_ASSERT_TRUE(status.voltage >= 0);
    TEST_ASSERT_TRUE(status.state_of_charge >= 0);
    TEST_ASSERT_TRUE(status.state_of_health >= 0);
}

/**
 * Test configuration update
 */
void test_config_update(void) {
    BatteryConfig new_config = BatteryManager::getDefaultConfig(BatteryChemistry::LIFEPO4);
    battery_manager->setConfiguration(new_config);
    
    BatteryConfig current_config = battery_manager->getConfiguration();
    TEST_ASSERT_EQUAL(BatteryChemistry::LIFEPO4, current_config.chemistry);
}

/**
 * Test max charge current setting
 */
void test_max_charge_current_setting(void) {
    float test_current = 500.0f;
    battery_manager->setMaxChargeCurrent(test_current);
    
    BatteryConfig config = battery_manager->getConfiguration();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, test_current, config.max_charge_current);
}

/**
 * Test cycle count starts at zero
 */
void test_initial_cycle_count(void) {
    BatteryManager bm(TEST_VOLTAGE_PIN);
    bm.begin();
    
    TEST_ASSERT_EQUAL(0, bm.getCycleCount());
}

/**
 * Test update function doesn't crash
 */
void test_update_no_crash(void) {
    // Call update multiple times
    for (int i = 0; i < 10; i++) {
        battery_manager->update();
        delay(10);
    }
    TEST_PASS();
}

/**
 * Test temperature reading returns reasonable value
 */
void test_temperature_reading(void) {
    float temp = battery_manager->getTemperature();
    
    // Temperature should be in reasonable range
    TEST_ASSERT_TRUE(temp >= -40.0f);
    TEST_ASSERT_TRUE(temp <= 85.0f);
}

/**
 * Test battery health enum values
 */
void test_battery_health_values(void) {
    BatteryHealth health = battery_manager->getHealth();
    
    // Health should be one of the defined values
    TEST_ASSERT_TRUE(
        health == BatteryHealth::EXCELLENT ||
        health == BatteryHealth::GOOD ||
        health == BatteryHealth::FAIR ||
        health == BatteryHealth::POOR ||
        health == BatteryHealth::REPLACE ||
        health == BatteryHealth::UNKNOWN
    );
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_battery_manager_init);
    
    // Configuration tests
    RUN_TEST(test_default_lithium_ion_config);
    RUN_TEST(test_default_lifepo4_config);
    RUN_TEST(test_default_lead_acid_config);
    RUN_TEST(test_config_update);
    RUN_TEST(test_max_charge_current_setting);
    
    // Reading tests
    RUN_TEST(test_voltage_reading_valid_range);
    RUN_TEST(test_soc_valid_bounds);
    RUN_TEST(test_soh_valid_bounds);
    RUN_TEST(test_temperature_reading);
    
    // State tests
    RUN_TEST(test_charging_enable_disable);
    RUN_TEST(test_no_initial_fault);
    RUN_TEST(test_fault_clear);
    RUN_TEST(test_initial_charging_stage_idle);
    RUN_TEST(test_initial_cycle_count);
    RUN_TEST(test_battery_health_values);
    
    // Status tests
    RUN_TEST(test_status_structure);
    
    // Operational tests
    RUN_TEST(test_update_no_crash);
    
    UNITY_END();
    
    // Cleanup
    if (battery_manager != nullptr) {
        delete battery_manager;
        battery_manager = nullptr;
    }
}

void loop() {
    // Nothing here
}
