/**
 * @file test_camera_module.cpp
 * @brief Unit tests for CameraModule captureImage() function
 * @author ESP32 Wildlife CAM Project
 * @date 2025-10-16
 */

#include <unity.h>
#include "../src/camera/camera_module.h"

CameraModule* camera_module = nullptr;

void setUp(void) {
    // Initialize before each test
    camera_module = new CameraModule();
}

void tearDown(void) {
    // Cleanup after each test
    if (camera_module) {
        camera_module->deinitialize();
        delete camera_module;
        camera_module = nullptr;
    }
}

/**
 * Test captureImage() when camera is not initialized
 * Expected: Should return CAMERA_ERROR_NOT_INITIALIZED
 */
void test_captureImage_not_initialized(void) {
    TEST_ASSERT_NOT_NULL(camera_module);
    TEST_ASSERT_FALSE(camera_module->isInitialized());
    
    // Attempt to capture without initialization
    CameraResult result = camera_module->captureImage();
    
    TEST_ASSERT_EQUAL(CAMERA_ERROR_NOT_INITIALIZED, result);
    TEST_ASSERT_NOT_NULL(camera_module->getLastError());
}

/**
 * Test captureImage() with valid initialization
 * Expected: Should return CAMERA_OK and populate metadata
 */
void test_captureImage_success(void) {
    // Initialize camera with default config
    CameraConfig config = getDefaultWildlifeConfig();
    CameraResult init_result = camera_module->initialize(config);
    
    // Skip test if initialization fails (hardware not available)
    if (init_result != CAMERA_OK) {
        TEST_IGNORE_MESSAGE("Camera hardware not available");
        return;
    }
    
    TEST_ASSERT_TRUE(camera_module->isInitialized());
    
    // Capture image with metadata
    ImageMetadata metadata;
    memset(&metadata, 0, sizeof(metadata));
    CameraResult result = camera_module->captureImage(&metadata);
    
    TEST_ASSERT_EQUAL(CAMERA_OK, result);
    TEST_ASSERT_GREATER_THAN(0, metadata.image_size);
    TEST_ASSERT_GREATER_THAN(0, metadata.width);
    TEST_ASSERT_GREATER_THAN(0, metadata.height);
    TEST_ASSERT_GREATER_THAN(0, metadata.timestamp);
}

/**
 * Test captureImage() without metadata parameter
 * Expected: Should return CAMERA_OK without crashing
 */
void test_captureImage_no_metadata(void) {
    // Initialize camera with default config
    CameraConfig config = getDefaultWildlifeConfig();
    CameraResult init_result = camera_module->initialize(config);
    
    // Skip test if initialization fails (hardware not available)
    if (init_result != CAMERA_OK) {
        TEST_IGNORE_MESSAGE("Camera hardware not available");
        return;
    }
    
    TEST_ASSERT_TRUE(camera_module->isInitialized());
    
    // Capture image without metadata
    CameraResult result = camera_module->captureImage();
    
    TEST_ASSERT_EQUAL(CAMERA_OK, result);
}

/**
 * Test captureImage() statistics update
 * Expected: Should update capture statistics correctly
 */
void test_captureImage_statistics(void) {
    // Initialize camera with default config
    CameraConfig config = getDefaultWildlifeConfig();
    CameraResult init_result = camera_module->initialize(config);
    
    // Skip test if initialization fails (hardware not available)
    if (init_result != CAMERA_OK) {
        TEST_IGNORE_MESSAGE("Camera hardware not available");
        return;
    }
    
    // Reset statistics
    camera_module->resetStatistics();
    CameraStats stats_before = camera_module->getStatistics();
    TEST_ASSERT_EQUAL(0, stats_before.total_captures);
    TEST_ASSERT_EQUAL(0, stats_before.successful_captures);
    
    // Capture image
    CameraResult result = camera_module->captureImage();
    TEST_ASSERT_EQUAL(CAMERA_OK, result);
    
    // Check statistics were updated
    CameraStats stats_after = camera_module->getStatistics();
    TEST_ASSERT_EQUAL(1, stats_after.total_captures);
    TEST_ASSERT_EQUAL(1, stats_after.successful_captures);
    TEST_ASSERT_GREATER_THAN(0, stats_after.last_capture_size);
    TEST_ASSERT_GREATER_THAN(0, stats_after.last_capture_timestamp);
}

/**
 * Test multiple consecutive captures
 * Expected: Should successfully capture multiple images
 */
void test_captureImage_multiple_captures(void) {
    // Initialize camera with default config
    CameraConfig config = getDefaultWildlifeConfig();
    CameraResult init_result = camera_module->initialize(config);
    
    // Skip test if initialization fails (hardware not available)
    if (init_result != CAMERA_OK) {
        TEST_IGNORE_MESSAGE("Camera hardware not available");
        return;
    }
    
    // Capture multiple images
    const uint8_t num_captures = 3;
    for (uint8_t i = 0; i < num_captures; i++) {
        // Return previous frame buffer if exists
        camera_fb_t* fb = camera_module->getLastFrameBuffer();
        if (fb != nullptr) {
            camera_module->returnFrameBuffer(fb);
        }
        
        CameraResult result = camera_module->captureImage();
        TEST_ASSERT_EQUAL_MESSAGE(CAMERA_OK, result, "Capture failed on iteration");
    }
    
    // Verify statistics
    CameraStats stats = camera_module->getStatistics();
    TEST_ASSERT_EQUAL(num_captures, stats.total_captures);
    TEST_ASSERT_EQUAL(num_captures, stats.successful_captures);
}

/**
 * Test error message logging
 * Expected: Should set appropriate error message on failure
 */
void test_captureImage_error_logging(void) {
    TEST_ASSERT_NOT_NULL(camera_module);
    TEST_ASSERT_FALSE(camera_module->isInitialized());
    
    // Attempt capture without initialization
    CameraResult result = camera_module->captureImage();
    
    TEST_ASSERT_EQUAL(CAMERA_ERROR_NOT_INITIALIZED, result);
    
    // Check that error message was set
    const char* error = camera_module->getLastError();
    TEST_ASSERT_NOT_NULL(error);
    TEST_ASSERT_GREATER_THAN(0, strlen(error));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_captureImage_not_initialized);
    RUN_TEST(test_captureImage_success);
    RUN_TEST(test_captureImage_no_metadata);
    RUN_TEST(test_captureImage_statistics);
    RUN_TEST(test_captureImage_multiple_captures);
    RUN_TEST(test_captureImage_error_logging);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
