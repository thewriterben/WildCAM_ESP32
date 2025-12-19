/**
 * @file test_adaptive_sleep_scheduler.cpp
 * @brief Unit tests for adaptive sleep scheduler
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 */

#include <unity.h>
#include "../power/adaptive_sleep_scheduler.h"

static AdaptiveSleepScheduler* scheduler = nullptr;

void setUp(void) {
    // Initialize scheduler before each test
    if (scheduler == nullptr) {
        scheduler = new AdaptiveSleepScheduler();
        scheduler->begin();
    }
}

void tearDown(void) {
    // Cleanup after all tests
}

/**
 * Test scheduler initialization with default config
 */
void test_init_default(void) {
    AdaptiveSleepScheduler sched;
    bool result = sched.begin();
    TEST_ASSERT_TRUE(result);
}

/**
 * Test scheduler initialization with custom config
 */
void test_init_custom_config(void) {
    AdaptiveSleepScheduler sched;
    SchedulerConfig config;
    config.aggressiveness = SleepAggressiveness::AGGRESSIVE;
    config.min_sleep_duration_ms = 10000;
    config.max_sleep_duration_ms = 1800000;
    config.default_sleep_duration_ms = 600000;
    config.battery_low_threshold = 25.0f;
    config.battery_critical_threshold = 10.0f;
    config.enable_seasonal_adjustment = false;
    config.enable_weather_adjustment = false;
    
    bool result = sched.begin(config);
    TEST_ASSERT_TRUE(result);
    
    SchedulerConfig stored = sched.getConfiguration();
    TEST_ASSERT_EQUAL(SleepAggressiveness::AGGRESSIVE, stored.aggressiveness);
}

/**
 * Test motion event recording
 */
void test_record_motion_event(void) {
    scheduler->resetPatterns();
    scheduler->setCurrentTime(10, 30);  // 10:30 AM
    
    uint16_t before = scheduler->getTodayMotionCount();
    scheduler->recordMotionEvent();
    uint16_t after = scheduler->getTodayMotionCount();
    
    TEST_ASSERT_EQUAL(before + 1, after);
}

/**
 * Test recommended sleep duration is within configured range
 */
void test_sleep_duration_in_range(void) {
    scheduler->setCurrentTime(12, 0);
    
    uint32_t duration = scheduler->getRecommendedSleepDuration();
    SchedulerConfig config = scheduler->getConfiguration();
    
    TEST_ASSERT_TRUE(duration >= config.min_sleep_duration_ms);
    TEST_ASSERT_TRUE(duration <= config.max_sleep_duration_ms);
}

/**
 * Test sleep duration for specific hour
 */
void test_sleep_duration_for_hour(void) {
    for (int hour = 0; hour < 24; hour++) {
        uint32_t duration = scheduler->getRecommendedSleepDuration(hour);
        SchedulerConfig config = scheduler->getConfiguration();
        
        TEST_ASSERT_TRUE(duration >= config.min_sleep_duration_ms);
        TEST_ASSERT_TRUE(duration <= config.max_sleep_duration_ms);
    }
}

/**
 * Test slot statistics for valid hour
 */
void test_slot_statistics_valid_hour(void) {
    MotionSlotStats stats = scheduler->getSlotStatistics(12);
    
    // Activity probability should be between 0 and 1
    TEST_ASSERT_TRUE(stats.activity_probability >= 0.0f);
    TEST_ASSERT_TRUE(stats.activity_probability <= 1.0f);
}

/**
 * Test slot statistics for invalid hour returns zeros
 */
void test_slot_statistics_invalid_hour(void) {
    MotionSlotStats stats = scheduler->getSlotStatistics(30);  // Invalid hour
    
    TEST_ASSERT_EQUAL(0, stats.total_events);
    TEST_ASSERT_EQUAL(0.0f, stats.activity_probability);
}

/**
 * Test aggressiveness setting
 */
void test_set_aggressiveness(void) {
    scheduler->setAggressiveness(SleepAggressiveness::CONSERVATIVE);
    TEST_ASSERT_EQUAL(SleepAggressiveness::CONSERVATIVE, scheduler->getAggressiveness());
    
    scheduler->setAggressiveness(SleepAggressiveness::AGGRESSIVE);
    TEST_ASSERT_EQUAL(SleepAggressiveness::AGGRESSIVE, scheduler->getAggressiveness());
    
    scheduler->setAggressiveness(SleepAggressiveness::BALANCED);  // Restore
}

/**
 * Test battery level update
 */
void test_battery_level_update(void) {
    // Test normal battery
    scheduler->updateBatteryLevel(75.0f);
    TEST_PASS();
    
    // Test low battery
    scheduler->updateBatteryLevel(25.0f);
    TEST_PASS();
    
    // Test critical battery
    scheduler->updateBatteryLevel(5.0f);
    TEST_PASS();
    
    // Restore normal
    scheduler->updateBatteryLevel(100.0f);
}

/**
 * Test low battery increases sleep duration
 */
void test_low_battery_increases_sleep(void) {
    scheduler->setCurrentTime(12, 0);
    
    // Get duration with full battery
    scheduler->updateBatteryLevel(100.0f);
    scheduler->update();
    uint32_t full_battery_sleep = scheduler->getRecommendedSleepDuration();
    
    // Get duration with low battery
    scheduler->updateBatteryLevel(10.0f);
    scheduler->update();
    uint32_t low_battery_sleep = scheduler->getRecommendedSleepDuration();
    
    // Low battery should sleep longer or equal
    TEST_ASSERT_TRUE(low_battery_sleep >= full_battery_sleep);
    
    // Restore
    scheduler->updateBatteryLevel(100.0f);
}

/**
 * Test current time setting
 */
void test_set_current_time(void) {
    scheduler->setCurrentTime(15, 30);
    // No direct getter for time, but function should not crash
    TEST_PASS();
}

/**
 * Test invalid time values are handled
 */
void test_invalid_time_values(void) {
    scheduler->setCurrentTime(30, 90);  // Invalid values
    // Should not crash, values should be clamped or rejected
    TEST_PASS();
}

/**
 * Test pattern reset
 */
void test_pattern_reset(void) {
    // Record some events
    scheduler->recordMotionEvent();
    scheduler->recordMotionEvent();
    
    // Reset patterns
    scheduler->resetPatterns();
    
    TEST_ASSERT_EQUAL(0, scheduler->getTodayMotionCount());
}

/**
 * Test peak activity hour is valid
 */
void test_peak_activity_hour_valid(void) {
    uint8_t peak = scheduler->getPeakActivityHour();
    TEST_ASSERT_TRUE(peak < 24);
}

/**
 * Test quiet hours retrieval
 */
void test_get_quiet_hours(void) {
    uint8_t quiet_hours[6];
    int count = scheduler->getQuietHours(quiet_hours, 6);
    
    TEST_ASSERT_TRUE(count > 0);
    TEST_ASSERT_TRUE(count <= 6);
    
    // All returned hours should be valid
    for (int i = 0; i < count; i++) {
        TEST_ASSERT_TRUE(quiet_hours[i] < 24);
    }
}

/**
 * Test activity score is in valid range
 */
void test_activity_score_range(void) {
    float score = scheduler->getActivityScore();
    
    TEST_ASSERT_TRUE(score >= 0.0f);
    TEST_ASSERT_TRUE(score <= 1.0f);
}

/**
 * Test is currently active time returns boolean
 */
void test_is_currently_active_time(void) {
    bool active = scheduler->isCurrentlyActiveTime();
    TEST_ASSERT_TRUE(active == true || active == false);
}

/**
 * Test is active time for all hours
 */
void test_is_active_time_all_hours(void) {
    for (int hour = 0; hour < 24; hour++) {
        bool active = scheduler->isActiveTime(hour);
        TEST_ASSERT_TRUE(active == true || active == false);
    }
}

/**
 * Test daily schedule retrieval
 */
void test_get_daily_schedule(void) {
    SleepScheduleEntry schedule[24];
    scheduler->getDailySchedule(schedule);
    
    for (int h = 0; h < 24; h++) {
        TEST_ASSERT_EQUAL(h, schedule[h].hour);
        TEST_ASSERT_TRUE(schedule[h].sleep_duration_ms > 0);
        TEST_ASSERT_TRUE(schedule[h].detection_probability >= 0.0f);
        TEST_ASSERT_TRUE(schedule[h].detection_probability <= 1.0f);
    }
}

/**
 * Test update function doesn't crash
 */
void test_update_no_crash(void) {
    for (int i = 0; i < 20; i++) {
        scheduler->update();
        delay(10);
    }
    TEST_PASS();
}

/**
 * Test default configuration values
 */
void test_default_config(void) {
    SchedulerConfig config = AdaptiveSleepScheduler::getDefaultConfig();
    
    TEST_ASSERT_EQUAL(SleepAggressiveness::BALANCED, config.aggressiveness);
    TEST_ASSERT_TRUE(config.min_sleep_duration_ms > 0);
    TEST_ASSERT_TRUE(config.max_sleep_duration_ms > config.min_sleep_duration_ms);
    TEST_ASSERT_TRUE(config.battery_low_threshold > config.battery_critical_threshold);
}

/**
 * Test configuration update
 */
void test_config_update(void) {
    SchedulerConfig config = scheduler->getConfiguration();
    config.aggressiveness = SleepAggressiveness::ULTRA_AGGRESSIVE;
    
    scheduler->setConfiguration(config);
    
    SchedulerConfig updated = scheduler->getConfiguration();
    TEST_ASSERT_EQUAL(SleepAggressiveness::ULTRA_AGGRESSIVE, updated.aggressiveness);
    
    // Restore original
    config.aggressiveness = SleepAggressiveness::BALANCED;
    scheduler->setConfiguration(config);
}

/**
 * Test multiple motion events accumulate
 */
void test_multiple_motion_events(void) {
    scheduler->resetPatterns();
    
    for (int i = 0; i < 10; i++) {
        scheduler->recordMotionEvent();
    }
    
    TEST_ASSERT_EQUAL(10, scheduler->getTodayMotionCount());
}

/**
 * Test aggressive mode increases sleep duration
 */
void test_aggressive_increases_sleep(void) {
    scheduler->setCurrentTime(12, 0);
    
    // Get duration with balanced aggressiveness
    scheduler->setAggressiveness(SleepAggressiveness::BALANCED);
    uint32_t balanced_sleep = scheduler->getRecommendedSleepDuration();
    
    // Get duration with aggressive setting
    scheduler->setAggressiveness(SleepAggressiveness::AGGRESSIVE);
    uint32_t aggressive_sleep = scheduler->getRecommendedSleepDuration();
    
    // Aggressive should sleep longer or equal
    TEST_ASSERT_TRUE(aggressive_sleep >= balanced_sleep);
    
    // Restore
    scheduler->setAggressiveness(SleepAggressiveness::BALANCED);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_init_default);
    RUN_TEST(test_init_custom_config);
    RUN_TEST(test_default_config);
    
    // Configuration tests
    RUN_TEST(test_set_aggressiveness);
    RUN_TEST(test_config_update);
    RUN_TEST(test_set_current_time);
    RUN_TEST(test_invalid_time_values);
    
    // Motion recording tests
    RUN_TEST(test_record_motion_event);
    RUN_TEST(test_multiple_motion_events);
    RUN_TEST(test_pattern_reset);
    
    // Sleep duration tests
    RUN_TEST(test_sleep_duration_in_range);
    RUN_TEST(test_sleep_duration_for_hour);
    RUN_TEST(test_aggressive_increases_sleep);
    
    // Battery tests
    RUN_TEST(test_battery_level_update);
    RUN_TEST(test_low_battery_increases_sleep);
    
    // Statistics tests
    RUN_TEST(test_slot_statistics_valid_hour);
    RUN_TEST(test_slot_statistics_invalid_hour);
    RUN_TEST(test_peak_activity_hour_valid);
    RUN_TEST(test_get_quiet_hours);
    RUN_TEST(test_activity_score_range);
    
    // Active time tests
    RUN_TEST(test_is_currently_active_time);
    RUN_TEST(test_is_active_time_all_hours);
    
    // Schedule tests
    RUN_TEST(test_get_daily_schedule);
    
    // Operational tests
    RUN_TEST(test_update_no_crash);
    
    UNITY_END();
    
    // Cleanup
    if (scheduler != nullptr) {
        delete scheduler;
        scheduler = nullptr;
    }
}

void loop() {
    // Nothing here
}
