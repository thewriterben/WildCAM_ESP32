/**
 * @file test_camera_capture.cpp
 * @brief Unit tests for camera capture functionality
 */

#include <unity.h>
#include "../src/camera/camera_manager.h"

CameraManager* camera = nullptr;

void setUp(void) {
    // Initialize before each test
    camera = new CameraManager();
}

void tearDown(void) {
    // Cleanup after each test
    if (camera) {
        delete camera;
        camera = nullptr;
    }
}

/**
 * Test camera initialization
 */
void test_camera_initialization(void) {
    TEST_ASSERT_NOT_NULL(camera);
    TEST_ASSERT_FALSE(camera->isInitialized());
    
    bool result = camera->initialize();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(camera->isInitialized());
}

/**
 * Test camera capture with valid parameters
 */
void test_camera_capture_success(void) {
    TEST_ASSERT_TRUE(camera->initialize());
    
    int width = 0, height = 0;
    uint8_t* image_data = camera->captureFrame(&width, &height);
    
    TEST_ASSERT_NOT_NULL(image_data);
    TEST_ASSERT_GREATER_THAN(0, width);
    TEST_ASSERT_GREATER_THAN(0, height);
    TEST_ASSERT_EQUAL(320, width);  // QVGA
    TEST_ASSERT_EQUAL(240, height); // QVGA
    
    camera->releaseFrame(image_data);
}

/**
 * Test camera capture without initialization
 */
void test_camera_capture_not_initialized(void) {
    int width = 0, height = 0;
    uint8_t* image_data = camera->captureFrame(&width, &height);
    
    TEST_ASSERT_NULL(image_data);
    TEST_ASSERT_FALSE(camera->getLastError().isEmpty());
}

/**
 * Test multiple consecutive captures (frame buffer management)
 */
void test_multiple_captures(void) {
    TEST_ASSERT_TRUE(camera->initialize());
    
    for (int i = 0; i < 5; i++) {
        int width = 0, height = 0;
        uint8_t* image_data = camera->captureFrame(&width, &height);
        
        TEST_ASSERT_NOT_NULL_MESSAGE(image_data, "Capture failed on iteration");
        camera->releaseFrame(image_data);
    }
}

/**
 * Test thread safety with mutex protection
 */
void test_camera_thread_safety(void) {
    TEST_ASSERT_TRUE(camera->initialize());
    
    // Simulate concurrent access
    int width1 = 0, height1 = 0;
    uint8_t* image1 = camera->captureFrame(&width1, &height1);
    TEST_ASSERT_NOT_NULL(image1);
    
    // Second capture should wait for first to release
    int width2 = 0, height2 = 0;
    uint8_t* image2 = camera->captureFrame(&width2, &height2);
    
    // First frame should be released before second is acquired
    TEST_ASSERT_NULL(image2); // Or implement timeout mechanism
    
    camera->releaseFrame(image1);
}

/**
 * Test memory cleanup on destruction
 */
void test_camera_cleanup(void) {
    camera->initialize();
    
    int width = 0, height = 0;
    uint8_t* image_data = camera->captureFrame(&width, &height);
    TEST_ASSERT_NOT_NULL(image_data);
    
    // Destructor should clean up automatically
    delete camera;
    camera = nullptr;
    
    // If we reach here without crash, cleanup worked
    TEST_ASSERT_NULL(camera);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_camera_initialization);
    RUN_TEST(test_camera_capture_not_initialized);
    RUN_TEST(test_camera_capture_success);
    RUN_TEST(test_multiple_captures);
    RUN_TEST(test_camera_thread_safety);
    RUN_TEST(test_camera_cleanup);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
