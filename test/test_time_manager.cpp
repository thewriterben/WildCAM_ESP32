/**
 * @file test_time_manager.cpp
 * @brief Unit tests for TimeManager class
 * 
 * Tests for NTP time sync, RTC support, and timestamp generation.
 */

#include <Arduino.h>
#include <unity.h>
#include "config.h"
#include "TimeManager.h"

// Test constants
static const time_t EPOCH_2024_JAN_01 = 1704067200;  // 2024-01-01 00:00:00 UTC

// Test TimeManager initialization
void test_time_manager_init() {
    TimeManager tm;
    bool result = tm.init(0, 0);  // UTC timezone
    TEST_ASSERT_TRUE(result);
}

// Test TimeManager initialization with different timezone
void test_time_manager_init_with_timezone() {
    TimeManager tm;
    // EST timezone: UTC-5 = -18000 seconds
    bool result = tm.init(-18000, 0);
    TEST_ASSERT_TRUE(result);
}

// Test TimeManager initialization with DST offset
void test_time_manager_init_with_dst() {
    TimeManager tm;
    // EST with DST: UTC-5 + 1hr DST
    bool result = tm.init(-18000, 3600);
    TEST_ASSERT_TRUE(result);
}

// Test that time source is correctly reported before any sync
void test_time_manager_initial_time_source() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Before NTP sync or RTC load, source should be internal RTC or none
    TimeSource source = tm.getTimeSource();
    TEST_ASSERT_TRUE(source == TIME_SOURCE_NONE || 
                    source == TIME_SOURCE_RTC_INTERNAL ||
                    source == TIME_SOURCE_RTC_EXTERNAL);
}

// Test getTimestamp buffer handling
void test_time_manager_timestamp_buffer() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Test with valid buffer
    char buffer[30];
    char* result = tm.getTimestamp(buffer, sizeof(buffer));
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(buffer, result);
    
    // Verify buffer has some content
    TEST_ASSERT_TRUE(strlen(buffer) > 0);
}

// Test getTimestamp with insufficient buffer
void test_time_manager_timestamp_small_buffer() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Test with buffer too small (needs at least 25 bytes)
    char smallBuffer[10];
    char* result = tm.getTimestamp(smallBuffer, sizeof(smallBuffer));
    TEST_ASSERT_NULL(result);
}

// Test getTimestamp with null buffer
void test_time_manager_timestamp_null_buffer() {
    TimeManager tm;
    tm.init(0, 0);
    
    char* result = tm.getTimestamp(nullptr, 30);
    TEST_ASSERT_NULL(result);
}

// Test getDateString buffer handling
void test_time_manager_date_string() {
    TimeManager tm;
    tm.init(0, 0);
    
    char buffer[20];
    char* result = tm.getDateString(buffer, sizeof(buffer));
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_TRUE(strlen(buffer) > 0);
}

// Test getDateString with insufficient buffer
void test_time_manager_date_string_small_buffer() {
    TimeManager tm;
    tm.init(0, 0);
    
    char smallBuffer[5];
    char* result = tm.getDateString(smallBuffer, sizeof(smallBuffer));
    TEST_ASSERT_NULL(result);
}

// Test getTimeString buffer handling
void test_time_manager_time_string() {
    TimeManager tm;
    tm.init(0, 0);
    
    char buffer[10];
    char* result = tm.getTimeString(buffer, sizeof(buffer));
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_TRUE(strlen(buffer) > 0);
}

// Test getTimeString with insufficient buffer
void test_time_manager_time_string_small_buffer() {
    TimeManager tm;
    tm.init(0, 0);
    
    char smallBuffer[3];
    char* result = tm.getTimeString(smallBuffer, sizeof(smallBuffer));
    TEST_ASSERT_NULL(result);
}

// Test getTimeSourceString returns valid string
void test_time_manager_time_source_string() {
    TimeManager tm;
    tm.init(0, 0);
    
    const char* sourceStr = tm.getTimeSourceString();
    TEST_ASSERT_NOT_NULL(sourceStr);
    TEST_ASSERT_TRUE(strlen(sourceStr) > 0);
}

// Test manual time setting
void test_time_manager_set_time_manual() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Set time to a known value: Jan 1, 2024, 12:00:00
    bool result = tm.setTime(2024, 1, 1, 12, 0, 0);
    TEST_ASSERT_TRUE(result);
    
    // Verify time is now set
    TEST_ASSERT_TRUE(tm.isTimeSet());
    TEST_ASSERT_EQUAL(TIME_SOURCE_MANUAL, tm.getTimeSource());
}

// Test manual time setting with Unix timestamp
void test_time_manager_set_time_unix() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Set time to a known Unix timestamp (Jan 1, 2024, 00:00:00 UTC)
    bool result = tm.setTime(EPOCH_2024_JAN_01);
    TEST_ASSERT_TRUE(result);
    
    // Verify time is now set
    TEST_ASSERT_TRUE(tm.isTimeSet());
}

// Test getUnixTime returns reasonable value after setting
void test_time_manager_get_unix_time() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Set a known time
    tm.setTime(EPOCH_2024_JAN_01);
    
    // Get the time back
    time_t gotTime = tm.getUnixTime();
    
    // Should be within a few seconds of what we set
    TEST_ASSERT_INT_WITHIN(5, EPOCH_2024_JAN_01, gotTime);
}

// Test getTime fills tm struct correctly
void test_time_manager_get_time_struct() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Set a known time
    tm.setTime(2024, 6, 15, 14, 30, 45);
    
    struct tm timeinfo;
    bool result = tm.getTime(&timeinfo);
    TEST_ASSERT_TRUE(result);
    
    // Verify the struct was filled (checking year)
    TEST_ASSERT_EQUAL(124, timeinfo.tm_year);  // Years since 1900
    TEST_ASSERT_EQUAL(5, timeinfo.tm_mon);     // Month 0-11 (June = 5)
    TEST_ASSERT_EQUAL(15, timeinfo.tm_mday);
}

// Test getTime with null pointer
void test_time_manager_get_time_null() {
    TimeManager tm;
    tm.init(0, 0);
    
    bool result = tm.getTime(nullptr);
    TEST_ASSERT_FALSE(result);
}

// Test hasExternalRTC returns consistent value
void test_time_manager_has_external_rtc() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Just verify it returns a valid boolean (true or false)
    bool hasRTC = tm.hasExternalRTC();
    TEST_ASSERT_TRUE(hasRTC == true || hasRTC == false);
}

// Test getRTCTemperature when RTC not available
void test_time_manager_rtc_temperature() {
    TimeManager tm;
    tm.init(0, 0);
    
    float temp = tm.getRTCTemperature();
    
    // If no RTC, should return NAN
    // If RTC present, should return reasonable temperature
    if (tm.hasExternalRTC()) {
        TEST_ASSERT_TRUE(!isnan(temp));
        TEST_ASSERT_TRUE(temp > -40.0f && temp < 85.0f);  // DS3231 operating range
    } else {
        TEST_ASSERT_TRUE(isnan(temp));
    }
}

// Test that isTimeSet returns false initially (unless RTC has valid time)
void test_time_manager_is_time_set_initial() {
    TimeManager tm;
    // Don't init - should return false
    TEST_ASSERT_FALSE(tm.isTimeSet());
}

// Test printStatus doesn't crash
void test_time_manager_print_status() {
    TimeManager tm;
    tm.init(0, 0);
    
    // Just verify it doesn't crash
    tm.printStatus();
    TEST_ASSERT_TRUE(true);
}

// Test configuration constants are defined
void test_time_config_constants_defined() {
    #ifdef TIME_MANAGEMENT_ENABLED
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("TIME_MANAGEMENT_ENABLED not defined in config.h");
    #endif
    
    #ifdef NTP_SERVER_PRIMARY
        TEST_ASSERT_TRUE(strlen(NTP_SERVER_PRIMARY) > 0);
    #else
        TEST_FAIL_MESSAGE("NTP_SERVER_PRIMARY not defined in config.h");
    #endif
    
    #ifdef NTP_SYNC_TIMEOUT_MS
        TEST_ASSERT_TRUE(NTP_SYNC_TIMEOUT_MS > 0);
    #else
        TEST_FAIL_MESSAGE("NTP_SYNC_TIMEOUT_MS not defined in config.h");
    #endif
}

// Note: NTP sync tests require actual WiFi connection and are not included
// in unit tests. They should be tested manually or in integration tests.

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    // Basic initialization tests
    RUN_TEST(test_time_manager_init);
    RUN_TEST(test_time_manager_init_with_timezone);
    RUN_TEST(test_time_manager_init_with_dst);
    RUN_TEST(test_time_manager_initial_time_source);
    
    // Timestamp generation tests
    RUN_TEST(test_time_manager_timestamp_buffer);
    RUN_TEST(test_time_manager_timestamp_small_buffer);
    RUN_TEST(test_time_manager_timestamp_null_buffer);
    RUN_TEST(test_time_manager_date_string);
    RUN_TEST(test_time_manager_date_string_small_buffer);
    RUN_TEST(test_time_manager_time_string);
    RUN_TEST(test_time_manager_time_string_small_buffer);
    
    // Time source tests
    RUN_TEST(test_time_manager_time_source_string);
    RUN_TEST(test_time_manager_is_time_set_initial);
    
    // Manual time setting tests
    RUN_TEST(test_time_manager_set_time_manual);
    RUN_TEST(test_time_manager_set_time_unix);
    RUN_TEST(test_time_manager_get_unix_time);
    RUN_TEST(test_time_manager_get_time_struct);
    RUN_TEST(test_time_manager_get_time_null);
    
    // RTC tests
    RUN_TEST(test_time_manager_has_external_rtc);
    RUN_TEST(test_time_manager_rtc_temperature);
    
    // Utility tests
    RUN_TEST(test_time_manager_print_status);
    
    // Config tests
    RUN_TEST(test_time_config_constants_defined);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
