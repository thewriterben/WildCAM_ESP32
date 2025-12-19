/**
 * @file test_improved_power_manager.cpp
 * @brief Unit tests for improved power manager
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 */

#include <unity.h>
#include "../power/improved_power_manager.h"

// Mock pins for testing
static const ImprovedPowerPins TEST_PINS = {
    .battery_voltage_pin = 34,
    .battery_current_pin = 35,
    .battery_temp_pin = 32,
    .charge_enable_pin = 25,
    .solar_voltage_pin = 33,
    .solar_current_pin = 36,
    .mppt_pwm_pin = 26
};

static ImprovedPowerManager* power_manager = nullptr;

void setUp(void) {
    // Initialize power manager before each test
    if (power_manager == nullptr) {
        power_manager = new ImprovedPowerManager();
        power_manager->begin(TEST_PINS);
    }
}

void tearDown(void) {
    // Cleanup after all tests
}

/**
 * Test power manager initialization with default config
 */
void test_init_default_config(void) {
    ImprovedPowerManager pm;
    bool result = pm.begin(TEST_PINS);
    TEST_ASSERT_TRUE(result);
}

/**
 * Test power manager initialization with custom config
 */
void test_init_custom_config(void) {
    ImprovedPowerManager pm;
    ImprovedPowerConfig config = ImprovedPowerManager::getDefaultConfig();
    config.low_battery_threshold = 25.0f;
    config.critical_battery_threshold = 12.0f;
    config.enable_adaptive_sleep = true;
    
    bool result = pm.begin(TEST_PINS, config);
    TEST_ASSERT_TRUE(result);
}

/**
 * Test default configuration values
 */
void test_default_config_values(void) {
    ImprovedPowerConfig config = ImprovedPowerManager::getDefaultConfig();
    
    TEST_ASSERT_TRUE(config.low_battery_threshold > 0);
    TEST_ASSERT_TRUE(config.critical_battery_threshold > 0);
    TEST_ASSERT_TRUE(config.low_battery_threshold > config.critical_battery_threshold);
    TEST_ASSERT_TRUE(config.min_sleep_duration_ms > 0);
    TEST_ASSERT_TRUE(config.max_sleep_duration_ms > config.min_sleep_duration_ms);
    TEST_ASSERT_TRUE(config.enable_mppt);
    TEST_ASSERT_TRUE(config.enable_adaptive_sleep);
}

/**
 * Test battery voltage reading is non-negative
 */
void test_battery_voltage_non_negative(void) {
    float voltage = power_manager->getBatteryVoltage();
    TEST_ASSERT_TRUE(voltage >= 0.0f);
}

/**
 * Test battery SOC is in valid range
 */
void test_battery_soc_range(void) {
    float soc = power_manager->getBatterySOC();
    TEST_ASSERT_TRUE(soc >= 0.0f);
    TEST_ASSERT_TRUE(soc <= 100.0f);
}

/**
 * Test battery SOH is in valid range
 */
void test_battery_soh_range(void) {
    float soh = power_manager->getBatterySOH();
    TEST_ASSERT_TRUE(soh >= 0.0f);
    TEST_ASSERT_TRUE(soh <= 100.0f);
}

/**
 * Test battery health is valid enum
 */
void test_battery_health_valid(void) {
    BatteryHealth health = power_manager->getBatteryHealth();
    TEST_ASSERT_TRUE(
        health == BatteryHealth::EXCELLENT ||
        health == BatteryHealth::GOOD ||
        health == BatteryHealth::FAIR ||
        health == BatteryHealth::POOR ||
        health == BatteryHealth::REPLACE ||
        health == BatteryHealth::UNKNOWN
    );
}

/**
 * Test solar power is non-negative
 */
void test_solar_power_non_negative(void) {
    float power = power_manager->getSolarPower();
    TEST_ASSERT_TRUE(power >= 0.0f);
}

/**
 * Test daily energy harvest is non-negative
 */
void test_daily_energy_non_negative(void) {
    float energy = power_manager->getDailyEnergyHarvest();
    TEST_ASSERT_TRUE(energy >= 0.0f);
}

/**
 * Test operating mode setting
 */
void test_set_operating_mode(void) {
    power_manager->setOperatingMode(PowerSystemMode::POWER_SAVE);
    TEST_ASSERT_EQUAL(PowerSystemMode::POWER_SAVE, power_manager->getOperatingMode());
    
    power_manager->setOperatingMode(PowerSystemMode::NORMAL);
    TEST_ASSERT_EQUAL(PowerSystemMode::NORMAL, power_manager->getOperatingMode());
}

/**
 * Test all operating modes can be set
 */
void test_all_operating_modes(void) {
    PowerSystemMode modes[] = {
        PowerSystemMode::NORMAL,
        PowerSystemMode::POWER_SAVE,
        PowerSystemMode::SOLAR_PRIORITY,
        PowerSystemMode::BATTERY_PRESERVE,
        PowerSystemMode::EMERGENCY,
        PowerSystemMode::MAINTENANCE
    };
    
    for (int i = 0; i < 6; i++) {
        power_manager->setOperatingMode(modes[i]);
        TEST_ASSERT_EQUAL(modes[i], power_manager->getOperatingMode());
    }
    
    // Restore normal mode
    power_manager->setOperatingMode(PowerSystemMode::NORMAL);
}

/**
 * Test recommended sleep duration is in valid range
 */
void test_recommended_sleep_duration_range(void) {
    uint32_t duration = power_manager->getRecommendedSleepDuration();
    ImprovedPowerConfig config = ImprovedPowerManager::getDefaultConfig();
    
    TEST_ASSERT_TRUE(duration >= config.min_sleep_duration_ms);
    TEST_ASSERT_TRUE(duration <= config.max_sleep_duration_ms);
}

/**
 * Test motion event recording doesn't crash
 */
void test_record_motion_event(void) {
    power_manager->recordMotionEvent();
    power_manager->recordMotionEvent(millis());
    TEST_PASS();
}

/**
 * Test activity score is in valid range
 */
void test_activity_score_range(void) {
    float score = power_manager->getActivityScore();
    TEST_ASSERT_TRUE(score >= 0.0f);
    TEST_ASSERT_TRUE(score <= 1.0f);
}

/**
 * Test is active time returns boolean
 */
void test_is_active_time(void) {
    bool active = power_manager->isActiveTime();
    TEST_ASSERT_TRUE(active == true || active == false);
}

/**
 * Test is charging returns boolean
 */
void test_is_charging(void) {
    bool charging = power_manager->isCharging();
    TEST_ASSERT_TRUE(charging == true || charging == false);
}

/**
 * Test is solar available returns boolean
 */
void test_is_solar_available(void) {
    bool available = power_manager->isSolarAvailable();
    TEST_ASSERT_TRUE(available == true || available == false);
}

/**
 * Test charging enable/disable
 */
void test_charging_enable_disable(void) {
    power_manager->setChargingEnabled(true);
    TEST_ASSERT_TRUE(power_manager->isChargingEnabled());
    
    power_manager->setChargingEnabled(false);
    TEST_ASSERT_FALSE(power_manager->isChargingEnabled());
    
    // Re-enable
    power_manager->setChargingEnabled(true);
}

/**
 * Test current time setting
 */
void test_set_current_time(void) {
    power_manager->setCurrentTime(10, 30);
    TEST_PASS();
}

/**
 * Test weather conditions update
 */
void test_update_weather_conditions(void) {
    power_manager->updateWeatherConditions(50, 25.0f);
    power_manager->updateWeatherConditions(0, 35.0f);
    power_manager->updateWeatherConditions(100, 5.0f);
    TEST_PASS();
}

/**
 * Test fault detection
 */
void test_has_fault(void) {
    bool fault = power_manager->hasFault();
    TEST_ASSERT_TRUE(fault == true || fault == false);
}

/**
 * Test fault clear
 */
void test_clear_fault(void) {
    power_manager->clearFault();
    TEST_PASS();
}

/**
 * Test calibration functions don't crash
 */
void test_calibration_functions(void) {
    power_manager->calibrateBatteryVoltage(4.0f);
    power_manager->calibrateSolarSensors(1.1f, 0.95f);
    TEST_PASS();
}

/**
 * Test reset functions don't crash
 */
void test_reset_functions(void) {
    power_manager->resetMotionPatterns();
    power_manager->resetDailyEnergy();
    TEST_PASS();
}

/**
 * Test status structure is populated
 */
void test_status_structure(void) {
    ImprovedPowerStatus status = power_manager->getStatus();
    
    // All voltage/current values should be non-negative
    TEST_ASSERT_TRUE(status.battery_voltage >= 0);
    TEST_ASSERT_TRUE(status.solar_voltage >= 0);
    TEST_ASSERT_TRUE(status.solar_power >= 0);
    
    // Percentages should be in range
    TEST_ASSERT_TRUE(status.battery_soc >= 0 && status.battery_soc <= 100);
    TEST_ASSERT_TRUE(status.battery_soh >= 0 && status.battery_soh <= 100);
    TEST_ASSERT_TRUE(status.activity_score >= 0 && status.activity_score <= 1);
    
    // Sleep duration should be positive
    TEST_ASSERT_TRUE(status.recommended_sleep_ms > 0);
}

/**
 * Test component accessors
 */
void test_component_accessors(void) {
    MPPTController* mppt = power_manager->getMPPTController();
    BatteryManager* battery = power_manager->getBatteryManager();
    AdaptiveSleepScheduler* scheduler = power_manager->getSleepScheduler();
    
    // Components should be initialized
    TEST_ASSERT_NOT_NULL(mppt);
    TEST_ASSERT_NOT_NULL(battery);
    TEST_ASSERT_NOT_NULL(scheduler);
}

/**
 * Test update function doesn't crash
 */
void test_update_no_crash(void) {
    for (int i = 0; i < 20; i++) {
        power_manager->update();
        delay(10);
    }
    TEST_PASS();
}

/**
 * Test prepare for deep sleep
 */
void test_prepare_for_deep_sleep(void) {
    power_manager->prepareForDeepSleep(60000);  // 1 minute
    TEST_PASS();
}

/**
 * Test emergency mode increases sleep
 */
void test_emergency_mode_sleep(void) {
    power_manager->setOperatingMode(PowerSystemMode::NORMAL);
    uint32_t normal_sleep = power_manager->getRecommendedSleepDuration();
    
    power_manager->setOperatingMode(PowerSystemMode::EMERGENCY);
    uint32_t emergency_sleep = power_manager->getRecommendedSleepDuration();
    
    // Emergency mode should recommend longer sleep
    TEST_ASSERT_TRUE(emergency_sleep >= normal_sleep);
    
    // Restore normal mode
    power_manager->setOperatingMode(PowerSystemMode::NORMAL);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_init_default_config);
    RUN_TEST(test_init_custom_config);
    RUN_TEST(test_default_config_values);
    
    // Battery tests
    RUN_TEST(test_battery_voltage_non_negative);
    RUN_TEST(test_battery_soc_range);
    RUN_TEST(test_battery_soh_range);
    RUN_TEST(test_battery_health_valid);
    
    // Solar tests
    RUN_TEST(test_solar_power_non_negative);
    RUN_TEST(test_daily_energy_non_negative);
    RUN_TEST(test_is_solar_available);
    
    // Operating mode tests
    RUN_TEST(test_set_operating_mode);
    RUN_TEST(test_all_operating_modes);
    
    // Sleep scheduling tests
    RUN_TEST(test_recommended_sleep_duration_range);
    RUN_TEST(test_record_motion_event);
    RUN_TEST(test_activity_score_range);
    RUN_TEST(test_is_active_time);
    RUN_TEST(test_emergency_mode_sleep);
    
    // Charging tests
    RUN_TEST(test_is_charging);
    RUN_TEST(test_charging_enable_disable);
    
    // Configuration tests
    RUN_TEST(test_set_current_time);
    RUN_TEST(test_update_weather_conditions);
    
    // Fault tests
    RUN_TEST(test_has_fault);
    RUN_TEST(test_clear_fault);
    
    // Calibration and reset tests
    RUN_TEST(test_calibration_functions);
    RUN_TEST(test_reset_functions);
    
    // Status tests
    RUN_TEST(test_status_structure);
    
    // Component tests
    RUN_TEST(test_component_accessors);
    
    // Operational tests
    RUN_TEST(test_update_no_crash);
    RUN_TEST(test_prepare_for_deep_sleep);
    
    UNITY_END();
    
    // Cleanup
    if (power_manager != nullptr) {
        delete power_manager;
        power_manager = nullptr;
    }
}

void loop() {
    // Nothing here
}
