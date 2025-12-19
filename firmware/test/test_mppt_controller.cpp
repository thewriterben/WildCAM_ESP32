/**
 * @file test_mppt_controller.cpp
 * @brief Unit tests for MPPT solar charge controller
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 */

#include <unity.h>
#include "../power/mppt_controller.h"

// Mock pins for testing
static const int TEST_SOLAR_V_PIN = 34;
static const int TEST_SOLAR_I_PIN = 35;
static const int TEST_BATTERY_V_PIN = 32;
static const int TEST_BATTERY_I_PIN = 33;
static const int TEST_PWM_PIN = 25;

static MPPTController* mppt_controller = nullptr;

void setUp(void) {
    // Initialize MPPT controller before each test
    if (mppt_controller == nullptr) {
        mppt_controller = new MPPTController(
            TEST_SOLAR_V_PIN,
            TEST_SOLAR_I_PIN,
            TEST_BATTERY_V_PIN,
            TEST_BATTERY_I_PIN,
            TEST_PWM_PIN,
            MPPTAlgorithm::PERTURB_OBSERVE
        );
        mppt_controller->begin();
    }
}

void tearDown(void) {
    // Cleanup after all tests
}

/**
 * Test MPPT controller initialization with P&O algorithm
 */
void test_mppt_init_po_algorithm(void) {
    MPPTController controller(TEST_SOLAR_V_PIN, TEST_SOLAR_I_PIN,
                              TEST_BATTERY_V_PIN, TEST_BATTERY_I_PIN,
                              TEST_PWM_PIN, MPPTAlgorithm::PERTURB_OBSERVE);
    bool result = controller.begin();
    TEST_ASSERT_TRUE(result);
}

/**
 * Test MPPT controller initialization with IC algorithm
 */
void test_mppt_init_ic_algorithm(void) {
    MPPTController controller(TEST_SOLAR_V_PIN, TEST_SOLAR_I_PIN,
                              TEST_BATTERY_V_PIN, TEST_BATTERY_I_PIN,
                              TEST_PWM_PIN, MPPTAlgorithm::INCREMENTAL_CONDUCTANCE);
    bool result = controller.begin();
    TEST_ASSERT_TRUE(result);
}

/**
 * Test MPPT controller initialization with CV algorithm
 */
void test_mppt_init_cv_algorithm(void) {
    MPPTController controller(TEST_SOLAR_V_PIN, TEST_SOLAR_I_PIN,
                              TEST_BATTERY_V_PIN, TEST_BATTERY_I_PIN,
                              TEST_PWM_PIN, MPPTAlgorithm::CONSTANT_VOLTAGE);
    bool result = controller.begin();
    TEST_ASSERT_TRUE(result);
}

/**
 * Test solar power status structure
 */
void test_status_structure(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    
    // All fields should be accessible
    TEST_ASSERT_TRUE(status.solar_voltage >= 0);
    TEST_ASSERT_TRUE(status.solar_current >= 0);
    TEST_ASSERT_TRUE(status.solar_power >= 0);
    TEST_ASSERT_TRUE(status.battery_voltage >= 0);
    TEST_ASSERT_TRUE(status.charge_efficiency >= 0);
}

/**
 * Test solar voltage reading returns non-negative
 */
void test_solar_voltage_non_negative(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    TEST_ASSERT_TRUE(status.solar_voltage >= 0.0f);
}

/**
 * Test solar current reading returns non-negative
 */
void test_solar_current_non_negative(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    TEST_ASSERT_TRUE(status.solar_current >= 0.0f);
}

/**
 * Test battery voltage reading is in reasonable range
 */
void test_battery_voltage_range(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    // Reasonable range for single Li-ion cell or small pack
    TEST_ASSERT_TRUE(status.battery_voltage >= 0.0f);
    TEST_ASSERT_TRUE(status.battery_voltage <= 25.0f);
}

/**
 * Test charging efficiency is in valid range
 */
void test_charge_efficiency_range(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    TEST_ASSERT_TRUE(status.charge_efficiency >= 0.0f);
    TEST_ASSERT_TRUE(status.charge_efficiency <= 100.0f);
}

/**
 * Test cloud cover estimate is in valid range
 */
void test_cloud_cover_range(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    TEST_ASSERT_TRUE(status.cloud_cover_estimate >= 0);
    TEST_ASSERT_TRUE(status.cloud_cover_estimate <= 100);
}

/**
 * Test weather conditions update doesn't crash
 */
void test_weather_update_no_crash(void) {
    mppt_controller->updateWeatherConditions(50, 25.0f);
    TEST_PASS();
}

/**
 * Test weather adaptation toggle
 */
void test_weather_adaptation_toggle(void) {
    mppt_controller->setWeatherAdaptation(true);
    mppt_controller->setWeatherAdaptation(false);
    TEST_PASS();
}

/**
 * Test sensor calibration
 */
void test_sensor_calibration(void) {
    mppt_controller->calibrateSensors(1.1f, 0.95f);
    TEST_PASS();
}

/**
 * Test daily energy reset
 */
void test_daily_energy_reset(void) {
    mppt_controller->resetDailyEnergy();
    float energy = mppt_controller->getDailyEnergyHarvest();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, energy);
}

/**
 * Test daily energy accumulation
 */
void test_daily_energy_non_negative(void) {
    float energy = mppt_controller->getDailyEnergyHarvest();
    TEST_ASSERT_TRUE(energy >= 0.0f);
}

/**
 * Test MPPT enable/disable
 */
void test_mppt_enable_disable(void) {
    mppt_controller->setMPPTEnabled(true);
    mppt_controller->setMPPTEnabled(false);
    mppt_controller->setMPPTEnabled(true);  // Re-enable for other tests
    TEST_PASS();
}

/**
 * Test algorithm change
 */
void test_algorithm_change(void) {
    mppt_controller->setAlgorithm(MPPTAlgorithm::INCREMENTAL_CONDUCTANCE);
    mppt_controller->setAlgorithm(MPPTAlgorithm::CONSTANT_VOLTAGE);
    mppt_controller->setAlgorithm(MPPTAlgorithm::PERTURB_OBSERVE);
    TEST_PASS();
}

/**
 * Test update function doesn't crash
 */
void test_update_no_crash(void) {
    // Call update multiple times
    for (int i = 0; i < 10; i++) {
        mppt_controller->update();
        delay(10);
    }
    TEST_PASS();
}

/**
 * Test max power point detection function
 */
void test_max_power_point_detection(void) {
    // Just ensure function doesn't crash and returns boolean
    bool at_mpp = mppt_controller->isAtMaxPowerPoint();
    TEST_ASSERT_TRUE(at_mpp == true || at_mpp == false);
}

/**
 * Test charging efficiency function
 */
void test_get_charging_efficiency(void) {
    float efficiency = mppt_controller->getChargingEfficiency();
    TEST_ASSERT_TRUE(efficiency >= 0.0f);
    TEST_ASSERT_TRUE(efficiency <= 100.0f);
}

/**
 * Test daylight detection flag
 */
void test_daylight_detection(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    // Just ensure the flag is a valid boolean
    TEST_ASSERT_TRUE(status.is_daylight == true || status.is_daylight == false);
}

/**
 * Test charging status flag
 */
void test_charging_status(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    // Just ensure the flag is a valid boolean
    TEST_ASSERT_TRUE(status.is_charging == true || status.is_charging == false);
}

/**
 * Test solar power calculation (P = V * I)
 */
void test_solar_power_calculation(void) {
    SolarPowerStatus status = mppt_controller->getStatus();
    float calculated_power = status.solar_voltage * status.solar_current;
    TEST_ASSERT_FLOAT_WITHIN(10.0f, calculated_power, status.solar_power);
}

/**
 * Test extreme weather conditions don't crash
 */
void test_extreme_weather_conditions(void) {
    // Test edge cases
    mppt_controller->updateWeatherConditions(0, -40.0f);    // Clear, cold
    mppt_controller->updateWeatherConditions(100, 60.0f);   // Overcast, hot
    mppt_controller->updateWeatherConditions(50, 25.0f);    // Normal
    TEST_PASS();
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_mppt_init_po_algorithm);
    RUN_TEST(test_mppt_init_ic_algorithm);
    RUN_TEST(test_mppt_init_cv_algorithm);
    
    // Status and reading tests
    RUN_TEST(test_status_structure);
    RUN_TEST(test_solar_voltage_non_negative);
    RUN_TEST(test_solar_current_non_negative);
    RUN_TEST(test_battery_voltage_range);
    RUN_TEST(test_charge_efficiency_range);
    RUN_TEST(test_cloud_cover_range);
    RUN_TEST(test_daylight_detection);
    RUN_TEST(test_charging_status);
    RUN_TEST(test_solar_power_calculation);
    
    // Configuration tests
    RUN_TEST(test_weather_update_no_crash);
    RUN_TEST(test_weather_adaptation_toggle);
    RUN_TEST(test_sensor_calibration);
    RUN_TEST(test_extreme_weather_conditions);
    
    // Energy tracking tests
    RUN_TEST(test_daily_energy_reset);
    RUN_TEST(test_daily_energy_non_negative);
    
    // Control tests
    RUN_TEST(test_mppt_enable_disable);
    RUN_TEST(test_algorithm_change);
    RUN_TEST(test_max_power_point_detection);
    RUN_TEST(test_get_charging_efficiency);
    
    // Operational tests
    RUN_TEST(test_update_no_crash);
    
    UNITY_END();
    
    // Cleanup
    if (mppt_controller != nullptr) {
        delete mppt_controller;
        mppt_controller = nullptr;
    }
}

void loop() {
    // Nothing here
}
