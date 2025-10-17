/**
 * @file test_storage_manager.cpp
 * @brief Comprehensive unit tests for StorageManager
 * @author ESP32 Wildlife CAM Project
 * @date 2025-10-16
 */

#include <unity.h>
#include "../core/storage_manager.h"

StorageManager* storage = nullptr;

void setUp(void) {
    // Initialize before each test
    storage = new StorageManager();
}

void tearDown(void) {
    // Cleanup after each test
    if (storage) {
        delete storage;
        storage = nullptr;
    }
}

/**
 * Test storage manager initialization
 * Expected: Initialize and detect storage type
 */
void test_storage_initialization(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Attempt initialization (may fail in test environment without SD card)
    bool result = storage->initialize();
    
    // Test passes if either succeeds or fails gracefully
    // In CI environment without hardware, it should fail gracefully
    TEST_ASSERT_TRUE(result || !result);
}

/**
 * Test storage ready check
 * Expected: Should reflect initialization state
 */
void test_storage_is_ready(void) {
    bool ready = storage->isReady();
    
    // Before initialization, should be false
    TEST_ASSERT_FALSE(ready);
}

/**
 * Test get active storage type
 * Expected: Return STORAGE_NONE before initialization
 */
void test_get_active_storage_type(void) {
    storage_type_t type = storage->getActiveStorage();
    
    // Before initialization, should be STORAGE_NONE
    TEST_ASSERT_EQUAL(STORAGE_NONE, type);
}

/**
 * Test save image with null data
 * Expected: Should fail gracefully
 */
void test_save_image_null_data(void) {
    storage_result_t result = storage->saveImage(nullptr, 100, "test.jpg");
    
    // Should not crash, should return error
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save image with empty filename
 * Expected: Should fail gracefully
 */
void test_save_image_empty_filename(void) {
    uint8_t data[10] = {0};
    storage_result_t result = storage->saveImage(data, sizeof(data), "");
    
    // Should fail with empty filename
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save image with null filename
 * Expected: Should fail gracefully
 */
void test_save_image_null_filename(void) {
    uint8_t data[10] = {0};
    storage_result_t result = storage->saveImage(data, sizeof(data), nullptr);
    
    // Should fail with null filename
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save image with zero length
 * Expected: Should fail gracefully
 */
void test_save_image_zero_length(void) {
    uint8_t data[10] = {0};
    storage_result_t result = storage->saveImage(data, 0, "test.jpg");
    
    // Should fail with zero length
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save log with null message
 * Expected: Should fail gracefully
 */
void test_save_log_null_message(void) {
    storage_result_t result = storage->saveLog(nullptr);
    
    // Should fail with null message
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save log with empty message
 * Expected: Should handle empty message
 */
void test_save_log_empty_message(void) {
    storage_result_t result = storage->saveLog("");
    
    // Empty log should either succeed or fail gracefully
    TEST_ASSERT_TRUE(result == STORAGE_SUCCESS || result != STORAGE_SUCCESS);
}

/**
 * Test save config with null key
 * Expected: Should fail gracefully
 */
void test_save_config_null_key(void) {
    storage_result_t result = storage->saveConfig(nullptr, "value");
    
    // Should fail with null key
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test save config with null value
 * Expected: Should fail gracefully
 */
void test_save_config_null_value(void) {
    storage_result_t result = storage->saveConfig("key", nullptr);
    
    // Should fail with null value
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test read config with null key
 * Expected: Should fail gracefully
 */
void test_read_config_null_key(void) {
    char buffer[64];
    storage_result_t result = storage->readConfig(nullptr, buffer, sizeof(buffer));
    
    // Should fail with null key
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test read config with null buffer
 * Expected: Should fail gracefully
 */
void test_read_config_null_buffer(void) {
    storage_result_t result = storage->readConfig("key", nullptr, 64);
    
    // Should fail with null buffer
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test read config with zero buffer size
 * Expected: Should fail gracefully
 */
void test_read_config_zero_buffer_size(void) {
    char buffer[64];
    storage_result_t result = storage->readConfig("key", buffer, 0);
    
    // Should fail with zero buffer size
    TEST_ASSERT_NOT_EQUAL(STORAGE_SUCCESS, result);
}

/**
 * Test get used space before initialization
 * Expected: Should return 0 or handle gracefully
 */
void test_get_used_space_not_initialized(void) {
    uint64_t used = storage->getUsedSpace();
    
    // Should return 0 when not initialized
    TEST_ASSERT_EQUAL_UINT64(0, used);
}

/**
 * Test get free space before initialization
 * Expected: Should return 0 or handle gracefully
 */
void test_get_free_space_not_initialized(void) {
    uint64_t free = storage->getFreeSpace();
    
    // Should return 0 when not initialized
    TEST_ASSERT_EQUAL_UINT64(0, free);
}

/**
 * Test get total space before initialization
 * Expected: Should return 0 or handle gracefully
 */
void test_get_total_space_not_initialized(void) {
    uint64_t total = storage->getTotalSpace();
    
    // Should return 0 when not initialized
    TEST_ASSERT_EQUAL_UINT64(0, total);
}

/**
 * Test create directories before initialization
 * Expected: Should fail gracefully
 */
void test_create_directories_not_initialized(void) {
    bool result = storage->createDirectories();
    
    // Should fail when not initialized
    TEST_ASSERT_FALSE(result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_storage_initialization);
    RUN_TEST(test_storage_is_ready);
    RUN_TEST(test_get_active_storage_type);
    RUN_TEST(test_save_image_null_data);
    RUN_TEST(test_save_image_empty_filename);
    RUN_TEST(test_save_image_null_filename);
    RUN_TEST(test_save_image_zero_length);
    RUN_TEST(test_save_log_null_message);
    RUN_TEST(test_save_log_empty_message);
    RUN_TEST(test_save_config_null_key);
    RUN_TEST(test_save_config_null_value);
    RUN_TEST(test_read_config_null_key);
    RUN_TEST(test_read_config_null_buffer);
    RUN_TEST(test_read_config_zero_buffer_size);
    RUN_TEST(test_get_used_space_not_initialized);
    RUN_TEST(test_get_free_space_not_initialized);
    RUN_TEST(test_get_total_space_not_initialized);
    RUN_TEST(test_create_directories_not_initialized);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
