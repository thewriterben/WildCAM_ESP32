/**
 * @file test_jpeg_decoder.cpp
 * @brief Unit tests for JPEG decoding functionality
 */

#include <unity.h>
#include "../src/utils/jpeg_decoder.h"
#include <cstring>

// Simple 2x2 pixel test pattern in RGB888
static const uint8_t test_pattern_rgb[12] = {
    255, 0, 0,    // Red
    0, 255, 0,    // Green
    0, 0, 255,    // Blue
    255, 255, 0   // Yellow
};

/**
 * Test decoding with null parameters
 */
void test_jpeg_decode_null_parameters(void) {
    uint8_t dummy_data[10] = {0};
    uint8_t* output = nullptr;
    int width = 0, height = 0;
    
    // Test null jpeg_data
    TEST_ASSERT_FALSE(decodeJPEG(nullptr, 10, &output, &width, &height));
    
    // Test zero size
    TEST_ASSERT_FALSE(decodeJPEG(dummy_data, 0, &output, &width, &height));
    
    // Test null output pointer
    TEST_ASSERT_FALSE(decodeJPEG(dummy_data, 10, nullptr, &width, &height));
    
    // Test null width pointer
    TEST_ASSERT_FALSE(decodeJPEG(dummy_data, 10, &output, nullptr, &height));
    
    // Test null height pointer
    TEST_ASSERT_FALSE(decodeJPEG(dummy_data, 10, &output, &width, nullptr));
}

/**
 * Test decoding with invalid JPEG data
 */
void test_jpeg_decode_invalid_data(void) {
    uint8_t invalid_data[20] = {0x00, 0x01, 0x02, 0x03}; // Not a JPEG
    uint8_t* output = nullptr;
    int width = 0, height = 0;
    
    bool result = decodeJPEG(invalid_data, sizeof(invalid_data), &output, &width, &height);
    
    // Should fail gracefully without crashing
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_NULL(output);
}

/**
 * Test free with null buffer (should not crash)
 */
void test_free_decoded_buffer_null(void) {
    // Should handle null gracefully
    freeDecodedBuffer(nullptr);
    TEST_ASSERT(true); // If we reach here, test passed
}

/**
 * Test image resize with null parameters
 */
void test_resize_null_parameters(void) {
    uint8_t* output = nullptr;
    
    // Test null input
    TEST_ASSERT_FALSE(resizeImageBilinear(nullptr, 10, 10, 5, 5, &output));
    
    // Test null output pointer
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 10, 5, 5, nullptr));
}

/**
 * Test image resize with invalid dimensions
 */
void test_resize_invalid_dimensions(void) {
    uint8_t* output = nullptr;
    
    // Test zero input width
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 0, 10, 5, 5, &output));
    
    // Test zero input height
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 0, 5, 5, &output));
    
    // Test zero target width
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 10, 0, 5, &output));
    
    // Test zero target height
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 10, 5, 0, &output));
    
    // Test negative dimensions
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, -10, 10, 5, 5, &output));
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, -10, 5, 5, &output));
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 10, -5, 5, &output));
    TEST_ASSERT_FALSE(resizeImageBilinear(test_pattern_rgb, 10, 10, 5, -5, &output));
}

/**
 * Test image resize - same size (identity)
 */
void test_resize_same_size(void) {
    uint8_t* output = nullptr;
    
    bool result = resizeImageBilinear(test_pattern_rgb, 2, 2, 2, 2, &output);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(output);
    
    // Check that output matches input (should be identical)
    for (int i = 0; i < 12; i++) {
        TEST_ASSERT_EQUAL_UINT8(test_pattern_rgb[i], output[i]);
    }
    
    freeDecodedBuffer(output);
}

/**
 * Test image resize - upscaling
 */
void test_resize_upscale(void) {
    uint8_t* output = nullptr;
    
    // Upscale 2x2 to 4x4
    bool result = resizeImageBilinear(test_pattern_rgb, 2, 2, 4, 4, &output);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(output);
    
    // Check corners should be close to original values
    // Top-left pixel (red)
    TEST_ASSERT_EQUAL_UINT8(255, output[0]);
    TEST_ASSERT_EQUAL_UINT8(0, output[1]);
    TEST_ASSERT_EQUAL_UINT8(0, output[2]);
    
    // Top-right pixel (green) at position (3, 0)
    int top_right = (0 * 4 + 3) * 3;
    TEST_ASSERT_EQUAL_UINT8(0, output[top_right]);
    TEST_ASSERT_EQUAL_UINT8(255, output[top_right + 1]);
    TEST_ASSERT_EQUAL_UINT8(0, output[top_right + 2]);
    
    // Bottom-left pixel (blue) at position (0, 3)
    int bottom_left = (3 * 4 + 0) * 3;
    TEST_ASSERT_EQUAL_UINT8(0, output[bottom_left]);
    TEST_ASSERT_EQUAL_UINT8(0, output[bottom_left + 1]);
    TEST_ASSERT_EQUAL_UINT8(255, output[bottom_left + 2]);
    
    // Bottom-right pixel (yellow) at position (3, 3)
    int bottom_right = (3 * 4 + 3) * 3;
    TEST_ASSERT_EQUAL_UINT8(255, output[bottom_right]);
    TEST_ASSERT_EQUAL_UINT8(255, output[bottom_right + 1]);
    TEST_ASSERT_EQUAL_UINT8(0, output[bottom_right + 2]);
    
    freeDecodedBuffer(output);
}

/**
 * Test image resize - downscaling
 */
void test_resize_downscale(void) {
    // Create a simple 4x4 test pattern
    uint8_t input_4x4[48]; // 4x4 RGB888
    
    // Fill with a gradient pattern
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = (y * 4 + x) * 3;
            input_4x4[idx] = (x + y) * 30;     // R increases
            input_4x4[idx + 1] = 100;          // G constant
            input_4x4[idx + 2] = (3 - x) * 40; // B decreases
        }
    }
    
    uint8_t* output = nullptr;
    
    // Downscale 4x4 to 2x2
    bool result = resizeImageBilinear(input_4x4, 4, 4, 2, 2, &output);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(output);
    
    // Just verify output buffer is allocated correctly
    // The actual pixel values will be interpolated
    TEST_ASSERT_NOT_NULL(output);
    
    freeDecodedBuffer(output);
}

/**
 * Test image resize - non-square dimensions
 */
void test_resize_non_square(void) {
    // Create a 3x2 test image
    uint8_t input_3x2[18] = {
        255, 0, 0,    0, 255, 0,    0, 0, 255,  // Row 1
        255, 255, 0,  255, 0, 255,  0, 255, 255 // Row 2
    };
    
    uint8_t* output = nullptr;
    
    // Resize to 6x4
    bool result = resizeImageBilinear(input_3x2, 3, 2, 6, 4, &output);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(output);
    
    // Expected output size is 6 * 4 * 3 = 72 bytes
    // Just verify it doesn't crash and buffer is allocated
    
    freeDecodedBuffer(output);
}

/**
 * Test memory management - multiple alloc/free cycles
 */
void test_memory_management_cycles(void) {
    uint8_t* output = nullptr;
    
    // Perform multiple resize operations
    for (int i = 0; i < 5; i++) {
        bool result = resizeImageBilinear(test_pattern_rgb, 2, 2, 4, 4, &output);
        TEST_ASSERT_TRUE(result);
        TEST_ASSERT_NOT_NULL(output);
        freeDecodedBuffer(output);
        output = nullptr;
    }
    
    TEST_ASSERT_NULL(output);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_jpeg_decode_null_parameters);
    RUN_TEST(test_jpeg_decode_invalid_data);
    RUN_TEST(test_free_decoded_buffer_null);
    RUN_TEST(test_resize_null_parameters);
    RUN_TEST(test_resize_invalid_dimensions);
    RUN_TEST(test_resize_same_size);
    RUN_TEST(test_resize_upscale);
    RUN_TEST(test_resize_downscale);
    RUN_TEST(test_resize_non_square);
    RUN_TEST(test_memory_management_cycles);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
