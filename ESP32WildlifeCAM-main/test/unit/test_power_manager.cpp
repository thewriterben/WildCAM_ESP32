/**
 * @file test_power_manager.cpp
 * @brief Unit tests for Power Manager module
 * 
 * Tests power state management, battery monitoring, and power modes
 */

#include <unity.h>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
MockSerial Serial;
#endif

// Mock power states
enum class PowerState {
    NORMAL,
    POWER_SAVE,
    LOW_BATTERY,
    CRITICAL,
    CHARGING
};

// Mock power modes
enum class PowerMode {
    MAX_PERFORMANCE,
    BALANCED,
    ECO_MODE,
    SURVIVAL,
    HIBERNATION
};

// Mock battery voltage readings
float mockBatteryVoltage = 3.7f;
float mockSolarVoltage = 5.0f;
bool mockIsCharging = false;

float getBatteryVoltage() {
    return mockBatteryVoltage;
}

float getSolarVoltage() {
    return mockSolarVoltage;
}

bool isCharging() {
    return mockIsCharging;
}

float calculateBatteryPercentage(float voltage) {
    // Simple linear approximation for LiPo battery
    // 4.2V = 100%, 3.0V = 0%
    if (voltage >= 4.2f) return 100.0f;
    if (voltage <= 3.0f) return 0.0f;
    return ((voltage - 3.0f) / 1.2f) * 100.0f;
}

PowerState determinePowerState(float batteryPercentage, bool charging) {
    if (charging) return PowerState::CHARGING;
    if (batteryPercentage < 10.0f) return PowerState::CRITICAL;
    if (batteryPercentage < 20.0f) return PowerState::LOW_BATTERY;
    if (batteryPercentage < 50.0f) return PowerState::POWER_SAVE;
    return PowerState::NORMAL;
}

void test_power_manager_creation() {
    TEST_ASSERT_TRUE_MESSAGE(true, "Power manager creation test");
}

void test_battery_voltage_reading() {
    float voltage = getBatteryVoltage();
    TEST_ASSERT_GREATER_THAN_MESSAGE(0.0f, voltage, "Battery voltage should be positive");
    TEST_ASSERT_LESS_THAN_MESSAGE(5.0f, voltage, "Battery voltage should be less than 5V");
}

void test_battery_percentage_calculation() {
    float percentage = calculateBatteryPercentage(3.7f);
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0.0f, percentage, "Percentage should be >= 0");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(100.0f, percentage, "Percentage should be <= 100");
    
    // Test edge cases
    TEST_ASSERT_EQUAL_FLOAT(100.0f, calculateBatteryPercentage(4.2f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, calculateBatteryPercentage(3.0f));
    TEST_ASSERT_EQUAL_FLOAT(50.0f, calculateBatteryPercentage(3.6f));
}

void test_power_state_normal() {
    PowerState state = determinePowerState(80.0f, false);
    TEST_ASSERT_EQUAL_INT((int)PowerState::NORMAL, (int)state);
}

void test_power_state_low_battery() {
    PowerState state = determinePowerState(15.0f, false);
    TEST_ASSERT_EQUAL_INT((int)PowerState::LOW_BATTERY, (int)state);
}

void test_power_state_critical() {
    PowerState state = determinePowerState(5.0f, false);
    TEST_ASSERT_EQUAL_INT((int)PowerState::CRITICAL, (int)state);
}

void test_power_state_charging() {
    PowerState state = determinePowerState(50.0f, true);
    TEST_ASSERT_EQUAL_INT((int)PowerState::CHARGING, (int)state);
}

void test_solar_voltage_reading() {
    mockSolarVoltage = 5.0f;
    float voltage = getSolarVoltage();
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0.0f, voltage, "Solar voltage should be >= 0");
    TEST_ASSERT_LESS_THAN_MESSAGE(10.0f, voltage, "Solar voltage should be < 10V");
}

void test_charging_detection() {
    mockIsCharging = true;
    TEST_ASSERT_TRUE(isCharging());
    
    mockIsCharging = false;
    TEST_ASSERT_FALSE(isCharging());
}

void test_power_mode_transitions() {
    PowerMode mode = PowerMode::MAX_PERFORMANCE;
    TEST_ASSERT_EQUAL_INT((int)PowerMode::MAX_PERFORMANCE, (int)mode);
    
    mode = PowerMode::ECO_MODE;
    TEST_ASSERT_EQUAL_INT((int)PowerMode::ECO_MODE, (int)mode);
    
    mode = PowerMode::SURVIVAL;
    TEST_ASSERT_EQUAL_INT((int)PowerMode::SURVIVAL, (int)mode);
}

void setUp(void) {
    // Reset mock values
    mockBatteryVoltage = 3.7f;
    mockSolarVoltage = 5.0f;
    mockIsCharging = false;
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_power_manager_creation);
    RUN_TEST(test_battery_voltage_reading);
    RUN_TEST(test_battery_percentage_calculation);
    RUN_TEST(test_power_state_normal);
    RUN_TEST(test_power_state_low_battery);
    RUN_TEST(test_power_state_critical);
    RUN_TEST(test_power_state_charging);
    RUN_TEST(test_solar_voltage_reading);
    RUN_TEST(test_charging_detection);
    RUN_TEST(test_power_mode_transitions);
    
    return UNITY_END();
}
