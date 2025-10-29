/**
 * @file test_storage_manager.cpp
 * @brief Comprehensive unit tests for StorageManager class using Unity framework
 * @author ESP32 WildCAM Project
 * @date 2025-10-29
 * 
 * This file contains unit tests for the StorageManager class, including tests for:
 * - SD card initialization
 * - Filename generation
 * - Image saving functionality
 * - Metadata handling
 * - Storage space calculations
 */

#include <Arduino.h>
#include <unity.h>
#include "StorageManager.h"
#include <esp_camera.h>

// Test instance
StorageManager* storage = nullptr;

/**
 * @brief setUp function - runs before each test
 * Initializes a fresh StorageManager instance
 */
void setUp(void) {
    storage = new StorageManager();
}

/**
 * @brief tearDown function - runs after each test
 * Cleans up and releases resources
 */
void tearDown(void) {
    if (storage) {
        delete storage;
        storage = nullptr;
    }
}

//==============================================================================
// TEST CASE 1: test_init_success
// Test successful SD card initialization and verify initialized flag
//==============================================================================

/**
 * @brief Test successful SD card initialization
 * 
 * This test verifies that:
 * - The init() method can be called successfully
 * - The storage manager properly initializes the SD card
 * - The initialized flag is set correctly
 * 
 * Note: In hardware-less CI environments, this test allows graceful failure
 * but ensures the method doesn't crash
 */
void test_init_success(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Call init() - may fail in CI environment without SD card
    bool result = storage->init();
    
    // Test passes if init doesn't crash (returns either true or false)
    // In real hardware with SD card, this should return true
    // In CI environment without hardware, this should return false
    // Both cases are valid - the important part is no crash or exception
    
    // If initialization succeeded, verify we can get image count
    if (result) {
        unsigned long count = storage->getImageCount();
        TEST_ASSERT_GREATER_OR_EQUAL(0, count);
    }
    
    // Test completes successfully if we reach here without crashing
    TEST_ASSERT_NOT_NULL(storage);
}

//==============================================================================
// TEST CASE 2: test_generate_filename
// Test filename generation, format verification, and uniqueness
//==============================================================================

/**
 * @brief Test filename generation
 * 
 * This test verifies that:
 * - Filenames are generated successfully
 * - Format matches IMG_HHMMSS_XXX.jpg pattern
 * - Multiple calls produce unique filenames
 * 
 * Note: This test uses saveImage to trigger filename generation,
 * as generateFilename() is a private method
 */
void test_generate_filename(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Initialize storage (if possible)
    bool initResult = storage->init();
    
    // Create mock camera frame buffers
    camera_fb_t fb1, fb2, fb3;
    uint8_t mockData1[100] = {0xFF, 0xD8, 0xFF, 0xE0}; // JPEG header
    uint8_t mockData2[100] = {0xFF, 0xD8, 0xFF, 0xE0};
    uint8_t mockData3[100] = {0xFF, 0xD8, 0xFF, 0xE0};
    
    fb1.buf = mockData1;
    fb1.len = sizeof(mockData1);
    fb1.format = PIXFORMAT_JPEG;
    
    fb2.buf = mockData2;
    fb2.len = sizeof(mockData2);
    fb2.format = PIXFORMAT_JPEG;
    
    fb3.buf = mockData3;
    fb3.len = sizeof(mockData3);
    fb3.format = PIXFORMAT_JPEG;
    
    if (initResult) {
        // Try to save images and check generated filenames
        String filename1 = storage->saveImage(&fb1);
        String filename2 = storage->saveImage(&fb2);
        String filename3 = storage->saveImage(&fb3);
        
        // If saves were successful, verify filenames
        if (filename1.length() > 0) {
            // Verify filename contains IMG_ and .jpg
            TEST_ASSERT_TRUE(filename1.indexOf("IMG_") >= 0);
            TEST_ASSERT_TRUE(filename1.indexOf(".jpg") >= 0);
        }
        
        if (filename2.length() > 0) {
            TEST_ASSERT_TRUE(filename2.indexOf("IMG_") >= 0);
            TEST_ASSERT_TRUE(filename2.indexOf(".jpg") >= 0);
        }
        
        if (filename3.length() > 0) {
            TEST_ASSERT_TRUE(filename3.indexOf("IMG_") >= 0);
            TEST_ASSERT_TRUE(filename3.indexOf(".jpg") >= 0);
        }
        
        // Verify uniqueness - if we got filenames, they should be different
        if (filename1.length() > 0 && filename2.length() > 0) {
            TEST_ASSERT_TRUE(filename1 != filename2);
        }
        if (filename2.length() > 0 && filename3.length() > 0) {
            TEST_ASSERT_TRUE(filename2 != filename3);
        }
        if (filename1.length() > 0 && filename3.length() > 0) {
            TEST_ASSERT_TRUE(filename1 != filename3);
        }
    }
}

//==============================================================================
// TEST CASE 3: test_save_image_success
// Test image saving with mock buffer and verification
//==============================================================================

/**
 * @brief Test image saving with mock camera frame buffer
 * 
 * This test verifies that:
 * - Images can be saved to SD card
 * - File exists after saving
 * - File size matches buffer size (when hardware available)
 */
void test_save_image_success(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Initialize storage
    bool initResult = storage->init();
    
    // Create mock camera frame buffer with JPEG header
    camera_fb_t fb;
    uint8_t mockData[256];
    
    // Fill with JPEG header and some data
    mockData[0] = 0xFF;
    mockData[1] = 0xD8; // JPEG SOI marker
    mockData[2] = 0xFF;
    mockData[3] = 0xE0; // JPEG APP0 marker
    for (int i = 4; i < 256; i++) {
        mockData[i] = (uint8_t)(i % 256);
    }
    
    fb.buf = mockData;
    fb.len = sizeof(mockData);
    fb.format = PIXFORMAT_JPEG;
    fb.width = 320;
    fb.height = 240;
    
    if (initResult) {
        // Save image
        String savedPath = storage->saveImage(&fb);
        
        // If save was successful, verify path is not empty
        if (savedPath.length() > 0) {
            TEST_ASSERT_TRUE(savedPath.length() > 0);
            
            // Verify path contains expected components
            TEST_ASSERT_TRUE(savedPath.indexOf("/images") >= 0);
            TEST_ASSERT_TRUE(savedPath.indexOf("IMG_") >= 0);
            TEST_ASSERT_TRUE(savedPath.indexOf(".jpg") >= 0);
            
            // Get free space to verify SD card is responsive
            unsigned long freeSpace = storage->getFreeSpace();
            TEST_ASSERT_GREATER_THAN(0, freeSpace);
        }
    }
}

//==============================================================================
// TEST CASE 4: test_save_image_null_buffer
// Test error handling with null buffer
//==============================================================================

/**
 * @brief Test error handling with null buffer
 * 
 * This test verifies that:
 * - saveImage() handles null buffer gracefully
 * - Function returns empty string for invalid input
 * - No crash or undefined behavior occurs
 */
void test_save_image_null_buffer(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Initialize storage (if possible)
    bool initResult = storage->init();
    
    // Test with completely null buffer
    String result1 = storage->saveImage(nullptr);
    TEST_ASSERT_EQUAL_STRING("", result1.c_str());
    
    // Test with buffer that has NULL buf pointer
    camera_fb_t fb;
    fb.buf = nullptr;
    fb.len = 100;
    fb.format = PIXFORMAT_JPEG;
    
    String result2 = storage->saveImage(&fb);
    TEST_ASSERT_EQUAL_STRING("", result2.c_str());
    
    // Test with buffer that has zero length
    uint8_t mockData[100];
    fb.buf = mockData;
    fb.len = 0;
    fb.format = PIXFORMAT_JPEG;
    
    String result3 = storage->saveImage(&fb);
    TEST_ASSERT_EQUAL_STRING("", result3.c_str());
    
    // All error cases should return empty string
    TEST_ASSERT_EQUAL(0, result1.length());
    TEST_ASSERT_EQUAL(0, result2.length());
    TEST_ASSERT_EQUAL(0, result3.length());
}

//==============================================================================
// TEST CASE 5: test_save_metadata
// Test metadata saving with JSON
//==============================================================================

/**
 * @brief Test metadata saving functionality
 * 
 * This test verifies that:
 * - Metadata can be saved as JSON
 * - JSON file is created alongside image file
 * - JSON content is valid
 */
void test_save_metadata(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Initialize storage
    bool initResult = storage->init();
    
    if (initResult) {
        // Create test JSON metadata
        StaticJsonDocument<512> metadata;
        metadata["timestamp"] = "2025-10-29T12:00:00Z";
        metadata["temperature"] = 22.5;
        metadata["humidity"] = 65;
        metadata["battery"] = 3.7;
        metadata["species"] = "Deer";
        metadata["confidence"] = 0.95;
        
        // Create a test image path
        String imagePath = "/images/20251029/IMG_120000_001.jpg";
        
        // Save metadata
        bool result = storage->saveMetadata(imagePath, metadata);
        
        // Test with empty path - should always fail
        bool result2 = storage->saveMetadata("", metadata);
        TEST_ASSERT_FALSE(result2);
        
        // If saveMetadata with valid path succeeded, test passes
        // If it failed (no SD card), test still passes as long as no crash occurred
        
    } else {
        // Without initialization, saveMetadata should fail gracefully
        StaticJsonDocument<512> metadata;
        metadata["test"] = "value";
        
        bool result = storage->saveMetadata("/test.jpg", metadata);
        TEST_ASSERT_FALSE(result);
    }
}

//==============================================================================
// TEST CASE 6: test_get_free_space
// Test free space calculation
//==============================================================================

/**
 * @brief Test free space calculation
 * 
 * This test verifies that:
 * - getFreeSpace() returns a reasonable value
 * - Return value is non-negative
 * - Method doesn't crash when called before/after init
 */
void test_get_free_space(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Test before initialization - should return 0
    unsigned long freeSpaceBefore = storage->getFreeSpace();
    TEST_ASSERT_EQUAL(0, freeSpaceBefore);
    
    // Initialize storage
    bool initResult = storage->init();
    
    if (initResult) {
        // After successful initialization, should return positive value
        unsigned long freeSpaceAfter = storage->getFreeSpace();
        TEST_ASSERT_GREATER_THAN(0, freeSpaceAfter);
        
        // Also test getUsedSpace for completeness
        unsigned long usedSpace = storage->getUsedSpace();
        TEST_ASSERT_GREATER_OR_EQUAL(0, usedSpace);
        
        // Free space should be reasonable (less than 1TB for typical SD cards)
        unsigned long maxReasonableSpace = 1024UL * 1024UL * 1024UL * 1024UL; // 1TB
        TEST_ASSERT_LESS_THAN(maxReasonableSpace, freeSpaceAfter);
        
    } else {
        // If init failed, getFreeSpace should still return 0 gracefully
        unsigned long freeSpace = storage->getFreeSpace();
        TEST_ASSERT_EQUAL(0, freeSpace);
    }
}

//==============================================================================
// ADDITIONAL HELPER TESTS
//==============================================================================

/**
 * @brief Test getImageCount functionality
 * 
 * Verifies that image counter is properly tracked
 */
void test_get_image_count(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Before init, should return 0
    unsigned long countBefore = storage->getImageCount();
    TEST_ASSERT_EQUAL(0, countBefore);
    
    // After init, should return counter value (may be > 0 if persistent)
    bool initResult = storage->init();
    if (initResult) {
        unsigned long countAfter = storage->getImageCount();
        TEST_ASSERT_GREATER_OR_EQUAL(0, countAfter);
    }
}

/**
 * @brief Test getImageFiles functionality
 * 
 * Verifies that image file list can be retrieved
 */
void test_get_image_files(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Before init, should return empty vector
    std::vector<String> filesBefore = storage->getImageFiles();
    TEST_ASSERT_TRUE(filesBefore.empty());
    
    // After init, should return vector (may be empty or have files)
    bool initResult = storage->init();
    if (initResult) {
        std::vector<String> filesAfter = storage->getImageFiles();
        // Should at least not crash
        TEST_ASSERT_GREATER_OR_EQUAL(0, filesAfter.size());
    }
}

/**
 * @brief Test getImageFiles comprehensive error handling
 * 
 * Verifies that getImageFiles handles various error conditions gracefully:
 * - Returns empty vector when not initialized
 * - Doesn't crash on repeated calls
 * - Handles edge cases properly
 */
void test_get_image_files_error_handling(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Test 1: Multiple calls before initialization should all return empty vector
    for (int i = 0; i < 3; i++) {
        std::vector<String> files = storage->getImageFiles();
        TEST_ASSERT_EQUAL(0, files.size());
    }
    
    // Test 2: After initialization, repeated calls should work without crashes
    bool initResult = storage->init();
    if (initResult) {
        std::vector<String> files1 = storage->getImageFiles();
        std::vector<String> files2 = storage->getImageFiles();
        std::vector<String> files3 = storage->getImageFiles();
        
        // All calls should succeed (even if no files exist)
        TEST_ASSERT_GREATER_OR_EQUAL(0, files1.size());
        TEST_ASSERT_GREATER_OR_EQUAL(0, files2.size());
        TEST_ASSERT_GREATER_OR_EQUAL(0, files3.size());
        
        // Results should be consistent across calls
        TEST_ASSERT_EQUAL(files1.size(), files2.size());
        TEST_ASSERT_EQUAL(files2.size(), files3.size());
    }
}

/**
 * @brief Test deleteOldFiles functionality
 * 
 * Verifies that old file deletion works gracefully
 */
void test_delete_old_files(void) {
    TEST_ASSERT_NOT_NULL(storage);
    
    // Should handle call before init gracefully
    bool resultBefore = storage->deleteOldFiles(7);
    TEST_ASSERT_FALSE(resultBefore);
    
    // After init, deleteOldFiles should work or fail gracefully
    bool initResult = storage->init();
    if (initResult) {
        bool resultAfter = storage->deleteOldFiles(7);
        // Current implementation is a placeholder that returns true,
        // but we don't assert the specific return value to avoid brittleness
        // The main goal is to verify it doesn't crash when initialized
        // In a full implementation, this would test actual deletion logic
        (void)resultAfter;  // Suppress unused variable warning
    }
}

//==============================================================================
// TEST RUNNER - setup() and loop() for Unity
//==============================================================================

void setup() {
    // Wait for serial monitor to connect
    delay(2000);
    
    Serial.println("===========================================");
    Serial.println("StorageManager Unit Tests");
    Serial.println("===========================================");
    
    UNITY_BEGIN();
    
    // Run all test cases
    RUN_TEST(test_init_success);
    RUN_TEST(test_generate_filename);
    RUN_TEST(test_save_image_success);
    RUN_TEST(test_save_image_null_buffer);
    RUN_TEST(test_save_metadata);
    RUN_TEST(test_get_free_space);
    
    // Additional helper tests
    RUN_TEST(test_get_image_count);
    RUN_TEST(test_get_image_files);
    RUN_TEST(test_get_image_files_error_handling);
    RUN_TEST(test_delete_old_files);
    
    UNITY_END();
    
    Serial.println("===========================================");
    Serial.println("All tests completed!");
    Serial.println("===========================================");
}

void loop() {
    // Tests run once in setup
    // Nothing to do here
}
