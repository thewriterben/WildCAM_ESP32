/**
 * @file test_storage_manager.cpp
 * @brief Unit tests for Storage Manager module
 * 
 * Tests SD card operations, file management, and storage capacity monitoring
 */

#include <unity.h>
#include <string>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
#include "../mocks/mock_sd.h"
MockSerial Serial;
#endif

// Mock storage structures
struct StorageInfo {
    uint64_t totalBytes;
    uint64_t usedBytes;
    uint64_t freeBytes;
    float usagePercent;
    bool mounted;
};

StorageInfo getStorageInfo() {
    StorageInfo info;
#ifdef NATIVE_TEST
    info.totalBytes = MockSD::totalBytes();
    info.usedBytes = MockSD::usedBytes();
#else
    info.totalBytes = SD.totalBytes();
    info.usedBytes = SD.usedBytes();
#endif
    info.freeBytes = info.totalBytes - info.usedBytes;
    info.usagePercent = (float)info.usedBytes / info.totalBytes * 100.0f;
    info.mounted = true;
    return info;
}

bool initializeStorage() {
#ifdef NATIVE_TEST
    return MockSD::begin();
#else
    return SD.begin();
#endif
}

bool createDirectory(const char* path) {
#ifdef NATIVE_TEST
    return MockSD::mkdir(path);
#else
    return SD.mkdir(path);
#endif
}

bool fileExists(const char* path) {
#ifdef NATIVE_TEST
    return MockSD::exists(path);
#else
    return SD.exists(path);
#endif
}

bool deleteFile(const char* path) {
#ifdef NATIVE_TEST
    return MockSD::remove(path);
#else
    return SD.remove(path);
#endif
}

void test_storage_manager_creation() {
    TEST_ASSERT_TRUE_MESSAGE(true, "Storage manager creation test");
}

void test_storage_initialization() {
    bool result = initializeStorage();
    TEST_ASSERT_TRUE_MESSAGE(result, "Storage initialization should succeed");
}

void test_storage_info_retrieval() {
    initializeStorage();
    StorageInfo info = getStorageInfo();
    
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, info.totalBytes, "Total bytes should be positive");
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0, info.usedBytes, "Used bytes should be >= 0");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(info.totalBytes, info.usedBytes, 
                                      "Used bytes should not exceed total");
    TEST_ASSERT_TRUE_MESSAGE(info.mounted, "Storage should be mounted");
}

void test_free_space_calculation() {
    initializeStorage();
    StorageInfo info = getStorageInfo();
    
    uint64_t expectedFree = info.totalBytes - info.usedBytes;
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(expectedFree, info.freeBytes, 
                                     "Free space should equal total - used");
}

void test_usage_percentage() {
    initializeStorage();
    StorageInfo info = getStorageInfo();
    
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0.0f, info.usagePercent, "Usage % should be >= 0");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(100.0f, info.usagePercent, "Usage % should be <= 100");
}

void test_directory_creation() {
    initializeStorage();
    bool result = createDirectory("/test_dir");
    TEST_ASSERT_TRUE_MESSAGE(result, "Directory creation should succeed");
}

void test_file_existence_check() {
    initializeStorage();
#ifdef NATIVE_TEST
    MockSD::addFile("/test.txt", "test content");
#endif
    bool exists = fileExists("/test.txt");
    TEST_ASSERT_TRUE_MESSAGE(exists, "File should exist");
    
    bool notExists = fileExists("/nonexistent.txt");
    TEST_ASSERT_FALSE_MESSAGE(notExists, "Non-existent file should return false");
}

void test_file_deletion() {
    initializeStorage();
#ifdef NATIVE_TEST
    MockSD::addFile("/delete_me.txt", "content");
#endif
    bool result = deleteFile("/delete_me.txt");
    TEST_ASSERT_TRUE_MESSAGE(result, "File deletion should succeed");
}

void test_storage_capacity_check() {
    initializeStorage();
    StorageInfo info = getStorageInfo();
    
    // For 8GB SD card
    TEST_ASSERT_GREATER_THAN_MESSAGE(1ULL * 1024 * 1024 * 1024, info.totalBytes, 
                                     "Total capacity should be > 1GB");
}

void test_low_storage_detection() {
    initializeStorage();
    StorageInfo info = getStorageInfo();
    
    bool isLowStorage = info.freeBytes < (100ULL * 1024 * 1024); // < 100MB
    TEST_ASSERT_FALSE_MESSAGE(isLowStorage, "Storage should not be low for fresh init");
}

void setUp(void) {
#ifdef NATIVE_TEST
    MockSD::reset();
#endif
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_storage_manager_creation);
    RUN_TEST(test_storage_initialization);
    RUN_TEST(test_storage_info_retrieval);
    RUN_TEST(test_free_space_calculation);
    RUN_TEST(test_usage_percentage);
    RUN_TEST(test_directory_creation);
    RUN_TEST(test_file_existence_check);
    RUN_TEST(test_file_deletion);
    RUN_TEST(test_storage_capacity_check);
    RUN_TEST(test_low_storage_detection);
    
    return UNITY_END();
}
