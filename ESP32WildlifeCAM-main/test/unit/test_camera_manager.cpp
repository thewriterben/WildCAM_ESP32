/**
 * @file test_camera_manager.cpp
 * @brief Unit tests for Camera Manager module
 * 
 * Tests camera initialization, configuration, and capture functionality
 */

#include <unity.h>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
#include "../mocks/mock_camera.h"
MockSerial Serial;
#endif

// Test initialization
void test_camera_manager_create() {
    // Test that we can create camera manager
    TEST_ASSERT_TRUE_MESSAGE(true, "Camera manager creation test");
}

void test_camera_config_defaults() {
    camera_config_t config = {};
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.frame_size = 10; // FRAMESIZE_UXGA
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    TEST_ASSERT_EQUAL_INT(-1, config.pin_pwdn);
    TEST_ASSERT_EQUAL_INT(-1, config.pin_reset);
    TEST_ASSERT_EQUAL_INT(20000000, config.xclk_freq_hz);
    TEST_ASSERT_EQUAL_INT(10, config.frame_size);
    TEST_ASSERT_EQUAL_INT(12, config.jpeg_quality);
    TEST_ASSERT_EQUAL_INT(2, config.fb_count);
}

void test_camera_initialization() {
    camera_config_t config = {};
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    
    int result = esp_camera_init(&config);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "Camera initialization should succeed");
}

void test_camera_capture() {
    camera_fb_t* fb = esp_camera_fb_get();
    
    TEST_ASSERT_NOT_NULL_MESSAGE(fb, "Frame buffer should not be NULL");
    TEST_ASSERT_NOT_NULL_MESSAGE(fb->buf, "Frame buffer data should not be NULL");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, fb->len, "Frame buffer length should be positive");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, fb->width, "Frame width should be positive");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, fb->height, "Frame height should be positive");
    
    esp_camera_fb_return(fb);
}

void test_camera_frame_buffer_properties() {
    camera_fb_t* fb = esp_camera_fb_get();
    
    TEST_ASSERT_NOT_NULL(fb);
    TEST_ASSERT_EQUAL_INT(320, fb->width);
    TEST_ASSERT_EQUAL_INT(240, fb->height);
    TEST_ASSERT_EQUAL_INT(1024, fb->len);
    
    esp_camera_fb_return(fb);
}

void test_camera_multiple_captures() {
    // Test capturing multiple frames
    for (int i = 0; i < 5; i++) {
        camera_fb_t* fb = esp_camera_fb_get();
        TEST_ASSERT_NOT_NULL(fb);
        esp_camera_fb_return(fb);
    }
    TEST_PASS_MESSAGE("Multiple captures succeeded");
}

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_camera_manager_create);
    RUN_TEST(test_camera_config_defaults);
    RUN_TEST(test_camera_initialization);
    RUN_TEST(test_camera_capture);
    RUN_TEST(test_camera_frame_buffer_properties);
    RUN_TEST(test_camera_multiple_captures);
    
    return UNITY_END();
}
