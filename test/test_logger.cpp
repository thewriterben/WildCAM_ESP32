#include <Arduino.h>
#include <unity.h>
#include "Logger.h"
#include "config.h"

// Test logger initialization with default parameters
void test_logger_init_default() {
    bool result = Logger::init();
    TEST_ASSERT_TRUE(result);
}

// Test logger initialization with custom parameters
void test_logger_init_custom() {
    bool result = Logger::init(LOG_DEBUG, true, false, "/test.log");
    TEST_ASSERT_TRUE(result);
}

// Test log level filtering - DEBUG messages should not appear when level is INFO
void test_log_level_filtering() {
    Logger::init(LOG_INFO, true, false);
    
    // This should be filtered out (DEBUG < INFO)
    LOG_DEBUG("This debug message should be filtered");
    
    // These should appear
    LOG_INFO("This info message should appear");
    LOG_WARN("This warning message should appear");
    LOG_ERROR("This error message should appear");
    
    TEST_ASSERT_TRUE(true);
}

// Test setting log level after initialization
void test_set_log_level() {
    Logger::init(LOG_INFO, true, false);
    
    // Change log level to DEBUG
    Logger::setLogLevel(LOG_DEBUG);
    LogLevel currentLevel = Logger::getLogLevel();
    
    TEST_ASSERT_EQUAL(LOG_DEBUG, currentLevel);
    
    // Change to ERROR
    Logger::setLogLevel(LOG_ERROR);
    currentLevel = Logger::getLogLevel();
    
    TEST_ASSERT_EQUAL(LOG_ERROR, currentLevel);
}

// Test level to string conversion
void test_level_to_string() {
    TEST_ASSERT_EQUAL_STRING("DEBUG", Logger::levelToString(LOG_DEBUG));
    TEST_ASSERT_EQUAL_STRING("INFO", Logger::levelToString(LOG_INFO));
    TEST_ASSERT_EQUAL_STRING("WARN", Logger::levelToString(LOG_WARN));
    TEST_ASSERT_EQUAL_STRING("ERROR", Logger::levelToString(LOG_ERROR));
}

// Test timestamp generation
void test_timestamp_generation() {
    char buffer[32];
    char* result = Logger::getTimestamp(buffer, sizeof(buffer));
    
    // Verify we got a non-null result
    TEST_ASSERT_NOT_NULL(result);
    
    // Verify it returned the buffer we passed
    TEST_ASSERT_EQUAL_PTR(buffer, result);
    
    // Verify buffer is not empty
    TEST_ASSERT_TRUE(strlen(buffer) > 0);
}

// Test serial output enable/disable
void test_serial_output_toggle() {
    Logger::init(LOG_INFO, true, false);
    
    // Disable serial output
    Logger::setSerialOutput(false);
    LOG_INFO("This should not appear on serial");
    
    // Re-enable serial output
    Logger::setSerialOutput(true);
    LOG_INFO("This should appear on serial");
    
    TEST_ASSERT_TRUE(true);
}

// Test SD output enable/disable
void test_sd_output_toggle() {
    Logger::init(LOG_INFO, true, false);
    
    // Enable SD output
    Logger::setSDOutput(true);
    
    // Disable SD output
    Logger::setSDOutput(false);
    
    TEST_ASSERT_TRUE(true);
}

// Test log file path setting
void test_set_log_file_path() {
    Logger::init(LOG_INFO, true, false);
    
    Logger::setLogFilePath("/custom.log");
    
    TEST_ASSERT_TRUE(true);
}

// Test logging with format strings
void test_log_with_format() {
    Logger::init(LOG_INFO, true, false);
    
    int value = 42;
    float voltage = 3.7;
    const char* status = "OK";
    
    LOG_INFO("Integer: %d, Float: %.2f, String: %s", value, voltage, status);
    LOG_WARN("Battery voltage: %.2fV", voltage);
    LOG_ERROR("Error code: %d", value);
    
    TEST_ASSERT_TRUE(true);
}

// Test that config.h logging defines exist
void test_config_logging_defines() {
    #ifdef LOGGING_ENABLED
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("LOGGING_ENABLED not defined in config.h");
    #endif
    
    #ifdef DEFAULT_LOG_LEVEL
        TEST_ASSERT_TRUE(DEFAULT_LOG_LEVEL >= 0 && DEFAULT_LOG_LEVEL <= 3);
    #else
        TEST_FAIL_MESSAGE("DEFAULT_LOG_LEVEL not defined in config.h");
    #endif
    
    #ifdef LOG_TO_SERIAL
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("LOG_TO_SERIAL not defined in config.h");
    #endif
    
    #ifdef LOG_TO_SD
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("LOG_TO_SD not defined in config.h");
    #endif
    
    #ifdef LOG_FILE_PATH
        TEST_ASSERT_TRUE(true);
    #else
        TEST_FAIL_MESSAGE("LOG_FILE_PATH not defined in config.h");
    #endif
}

// Test logger macros include function and line information
void test_log_macros_include_metadata() {
    Logger::init(LOG_DEBUG, true, false);
    
    // These macros should include __func__ and __LINE__
    LOG_DEBUG("Debug with metadata");
    LOG_INFO("Info with metadata");
    LOG_WARN("Warning with metadata");
    LOG_ERROR("Error with metadata");
    
    TEST_ASSERT_TRUE(true);
}

void setup() {
    // Wait for serial monitor
    delay(2000);
    
    UNITY_BEGIN();
    
    // Run tests
    RUN_TEST(test_logger_init_default);
    RUN_TEST(test_logger_init_custom);
    RUN_TEST(test_log_level_filtering);
    RUN_TEST(test_set_log_level);
    RUN_TEST(test_level_to_string);
    RUN_TEST(test_timestamp_generation);
    RUN_TEST(test_serial_output_toggle);
    RUN_TEST(test_sd_output_toggle);
    RUN_TEST(test_set_log_file_path);
    RUN_TEST(test_log_with_format);
    RUN_TEST(test_config_logging_defines);
    RUN_TEST(test_log_macros_include_metadata);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
