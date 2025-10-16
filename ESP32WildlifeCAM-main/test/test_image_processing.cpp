/**
 * @file test_image_processing.cpp
 * @brief Unit tests for JPEG decoding and image processing
 * 
 * Tests for image_utils module including:
 * - JPEG decoding with TJpgDec
 * - Bilinear interpolation for image resizing
 * - Memory management and error handling
 */

#include <Arduino.h>
#include <unity.h>
#include "../src/utils/image_utils.h"

// Test configuration
#define TEST_WIDTH 320
#define TEST_HEIGHT 240
#define TEST_SMALL_WIDTH 160
#define TEST_SMALL_HEIGHT 120

// Helper function to create a simple test JPEG header
// This is a minimal valid JPEG for testing purposes
const uint8_t test_jpeg_data[] = {
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46, 0x49, 0x46, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0xDB, 0x00, 0x43,
    0x00, 0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09,
    0x09, 0x08, 0x0A, 0x0C, 0x14, 0x0D, 0x0C, 0x0B, 0x0B, 0x0C, 0x19, 0x12,
    0x13, 0x0F, 0x14, 0x1D, 0x1A, 0x1F, 0x1E, 0x1D, 0x1A, 0x1C, 0x1C, 0x20,
    0x24, 0x2E, 0x27, 0x20, 0x22, 0x2C, 0x23, 0x1C, 0x1C, 0x28, 0x37, 0x29,
    0x2C, 0x30, 0x31, 0x34, 0x34, 0x34, 0x1F, 0x27, 0x39, 0x3D, 0x38, 0x32,
    0x3C, 0x2E, 0x33, 0x34, 0x32, 0xFF, 0xC0, 0x00, 0x0B, 0x08, 0x00, 0x01,
    0x00, 0x01, 0x01, 0x01, 0x11, 0x00, 0xFF, 0xC4, 0x00, 0x14, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x03, 0xFF, 0xDA, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00,
    0x3F, 0x00, 0xFE, 0x8A, 0x28, 0xFF, 0xD9
};
const size_t test_jpeg_size = sizeof(test_jpeg_data);

/**
 * Test: JPEG decoding with valid data
 */
void test_jpeg_decode_valid() {
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    bool result = ImageUtils::decodeJPEG(test_jpeg_data, test_jpeg_size, 
                                         &width, &height, &rgbData);
    
    TEST_ASSERT_TRUE_MESSAGE(result, "JPEG decode should succeed");
    TEST_ASSERT_NOT_NULL_MESSAGE(rgbData, "RGB buffer should be allocated");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, width, "Width should be positive");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, height, "Height should be positive");
    
    // Clean up
    ImageUtils::freeDecodedBuffer(rgbData);
}

/**
 * Test: JPEG decoding with null data
 */
void test_jpeg_decode_null_data() {
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    bool result = ImageUtils::decodeJPEG(nullptr, 100, &width, &height, &rgbData);
    
    TEST_ASSERT_FALSE_MESSAGE(result, "Decode should fail with null data");
    TEST_ASSERT_NULL_MESSAGE(rgbData, "RGB buffer should not be allocated");
}

/**
 * Test: JPEG decoding with zero size
 */
void test_jpeg_decode_zero_size() {
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    bool result = ImageUtils::decodeJPEG(test_jpeg_data, 0, &width, &height, &rgbData);
    
    TEST_ASSERT_FALSE_MESSAGE(result, "Decode should fail with zero size");
    TEST_ASSERT_NULL_MESSAGE(rgbData, "RGB buffer should not be allocated");
}

/**
 * Test: JPEG decoding with null output pointers
 */
void test_jpeg_decode_null_outputs() {
    bool result1 = ImageUtils::decodeJPEG(test_jpeg_data, test_jpeg_size, 
                                          nullptr, nullptr, nullptr);
    TEST_ASSERT_FALSE_MESSAGE(result1, "Decode should fail with all null outputs");
    
    uint16_t width;
    bool result2 = ImageUtils::decodeJPEG(test_jpeg_data, test_jpeg_size, 
                                          &width, nullptr, nullptr);
    TEST_ASSERT_FALSE_MESSAGE(result2, "Decode should fail with null height output");
}

/**
 * Test: Free decoded buffer
 */
void test_free_decoded_buffer() {
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    bool result = ImageUtils::decodeJPEG(test_jpeg_data, test_jpeg_size, 
                                         &width, &height, &rgbData);
    
    if (result && rgbData) {
        // Should not crash
        ImageUtils::freeDecodedBuffer(rgbData);
    }
    
    // Should not crash with null pointer
    ImageUtils::freeDecodedBuffer(nullptr);
    
    TEST_ASSERT_TRUE_MESSAGE(true, "Free should handle null gracefully");
}

/**
 * Test: Image scaling with bilinear interpolation - downscaling
 */
void test_image_scaling_downscale() {
    // Create a test source image with a simple pattern
    const uint16_t srcWidth = 100;
    const uint16_t srcHeight = 100;
    const uint16_t dstWidth = 50;
    const uint16_t dstHeight = 50;
    
    size_t srcSize = srcWidth * srcHeight * 3;
    size_t dstSize = dstWidth * dstHeight * 3;
    
    uint8_t* srcData = (uint8_t*)malloc(srcSize);
    uint8_t* dstData = (uint8_t*)malloc(dstSize);
    
    TEST_ASSERT_NOT_NULL_MESSAGE(srcData, "Source buffer allocation failed");
    TEST_ASSERT_NOT_NULL_MESSAGE(dstData, "Destination buffer allocation failed");
    
    // Fill source with gradient pattern
    for (uint16_t y = 0; y < srcHeight; y++) {
        for (uint16_t x = 0; x < srcWidth; x++) {
            size_t idx = (y * srcWidth + x) * 3;
            srcData[idx + 0] = (uint8_t)(x * 255 / srcWidth);      // R gradient
            srcData[idx + 1] = (uint8_t)(y * 255 / srcHeight);     // G gradient
            srcData[idx + 2] = 128;                                 // B constant
        }
    }
    
    bool result = ImageUtils::scaleImage(srcData, srcWidth, srcHeight,
                                         dstData, dstWidth, dstHeight);
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Image scaling should succeed");
    
    // Verify that interpolation produces smooth results
    // Check a few pixels to ensure they're reasonable
    uint8_t r0 = dstData[0];  // Top-left should be dark
    uint8_t r_last = dstData[(dstHeight - 1) * dstWidth * 3];  // Bottom-left should be brighter
    
    TEST_ASSERT_LESS_THAN_MESSAGE(50, r0, "Top-left R should be low");
    TEST_ASSERT_GREATER_THAN_MESSAGE(200, r_last, "Bottom-left R should be higher");
    
    free(srcData);
    free(dstData);
}

/**
 * Test: Image scaling with bilinear interpolation - upscaling
 */
void test_image_scaling_upscale() {
    const uint16_t srcWidth = 50;
    const uint16_t srcHeight = 50;
    const uint16_t dstWidth = 100;
    const uint16_t dstHeight = 100;
    
    size_t srcSize = srcWidth * srcHeight * 3;
    size_t dstSize = dstWidth * dstHeight * 3;
    
    uint8_t* srcData = (uint8_t*)malloc(srcSize);
    uint8_t* dstData = (uint8_t*)malloc(dstSize);
    
    TEST_ASSERT_NOT_NULL_MESSAGE(srcData, "Source buffer allocation failed");
    TEST_ASSERT_NOT_NULL_MESSAGE(dstData, "Destination buffer allocation failed");
    
    // Fill source with a checkerboard pattern
    for (uint16_t y = 0; y < srcHeight; y++) {
        for (uint16_t x = 0; x < srcWidth; x++) {
            size_t idx = (y * srcWidth + x) * 3;
            uint8_t color = ((x / 10) + (y / 10)) % 2 ? 255 : 0;
            srcData[idx + 0] = color;
            srcData[idx + 1] = color;
            srcData[idx + 2] = color;
        }
    }
    
    bool result = ImageUtils::scaleImage(srcData, srcWidth, srcHeight,
                                         dstData, dstWidth, dstHeight);
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Image upscaling should succeed");
    
    free(srcData);
    free(dstData);
}

/**
 * Test: Image scaling with null pointers
 */
void test_image_scaling_null_pointers() {
    uint8_t dummy[100];
    
    bool result1 = ImageUtils::scaleImage(nullptr, 10, 10, dummy, 5, 5);
    TEST_ASSERT_FALSE_MESSAGE(result1, "Scaling should fail with null source");
    
    bool result2 = ImageUtils::scaleImage(dummy, 10, 10, nullptr, 5, 5);
    TEST_ASSERT_FALSE_MESSAGE(result2, "Scaling should fail with null destination");
}

/**
 * Test: Image scaling with zero dimensions
 */
void test_image_scaling_zero_dimensions() {
    uint8_t src[100], dst[100];
    
    bool result1 = ImageUtils::scaleImage(src, 0, 10, dst, 5, 5);
    TEST_ASSERT_FALSE_MESSAGE(result1, "Scaling should fail with zero width");
    
    bool result2 = ImageUtils::scaleImage(src, 10, 0, dst, 5, 5);
    TEST_ASSERT_FALSE_MESSAGE(result2, "Scaling should fail with zero height");
    
    bool result3 = ImageUtils::scaleImage(src, 10, 10, dst, 0, 5);
    TEST_ASSERT_FALSE_MESSAGE(result3, "Scaling should fail with zero dst width");
}

/**
 * Test: Bilinear interpolation quality
 * Verify that bilinear interpolation produces smoother results than nearest-neighbor
 */
void test_bilinear_interpolation_quality() {
    // Create a simple 2x2 image with distinct colors
    const uint16_t srcWidth = 2;
    const uint16_t srcHeight = 2;
    const uint16_t dstWidth = 4;
    const uint16_t dstHeight = 4;
    
    uint8_t srcData[2 * 2 * 3] = {
        // Top-left: black (0,0,0), Top-right: red (255,0,0)
        0, 0, 0,    255, 0, 0,
        // Bottom-left: green (0,255,0), Bottom-right: blue (0,0,255)
        0, 255, 0,  0, 0, 255
    };
    
    uint8_t dstData[4 * 4 * 3];
    
    bool result = ImageUtils::scaleImage(srcData, srcWidth, srcHeight,
                                         dstData, dstWidth, dstHeight);
    
    TEST_ASSERT_TRUE_MESSAGE(result, "Scaling should succeed");
    
    // The center pixel should be a blend of all four colors (roughly gray)
    // Since bilinear interpolation averages, we expect intermediate values
    size_t centerIdx = (2 * dstWidth + 2) * 3;  // Pixel at (2,2)
    
    // Each channel should have some value (not pure 0 or 255)
    uint8_t r = dstData[centerIdx + 0];
    uint8_t g = dstData[centerIdx + 1];
    uint8_t b = dstData[centerIdx + 2];
    
    // With bilinear interpolation, the center should be a mix
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, r, "Center pixel R should have some red");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, g, "Center pixel G should have some green");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, b, "Center pixel B should have some blue");
}

/**
 * Test: Memory management - no leaks
 */
void test_memory_management() {
    uint32_t freeBefore = ESP.getFreeHeap();
    
    for (int i = 0; i < 10; i++) {
        uint16_t width, height;
        uint8_t* rgbData = nullptr;
        
        bool result = ImageUtils::decodeJPEG(test_jpeg_data, test_jpeg_size, 
                                             &width, &height, &rgbData);
        
        if (result && rgbData) {
            ImageUtils::freeDecodedBuffer(rgbData);
        }
    }
    
    uint32_t freeAfter = ESP.getFreeHeap();
    
    // Allow for some variation, but memory should not leak significantly
    int32_t diff = freeBefore - freeAfter;
    TEST_ASSERT_LESS_THAN_MESSAGE(1000, abs(diff), "Memory leak detected");
}

/**
 * Setup function - called before each test
 */
void setUp(void) {
    // Initialize serial for debugging
    if (!Serial) {
        Serial.begin(115200);
        delay(100);
    }
}

/**
 * Teardown function - called after each test
 */
void tearDown(void) {
    // Clean up if needed
    delay(100);
}

/**
 * Main test runner
 */
void setup() {
    delay(2000);  // Wait for serial monitor
    
    Serial.println("\n=== Image Processing Unit Tests ===\n");
    
    UNITY_BEGIN();
    
    // JPEG decoding tests
    RUN_TEST(test_jpeg_decode_valid);
    RUN_TEST(test_jpeg_decode_null_data);
    RUN_TEST(test_jpeg_decode_zero_size);
    RUN_TEST(test_jpeg_decode_null_outputs);
    RUN_TEST(test_free_decoded_buffer);
    
    // Image scaling tests
    RUN_TEST(test_image_scaling_downscale);
    RUN_TEST(test_image_scaling_upscale);
    RUN_TEST(test_image_scaling_null_pointers);
    RUN_TEST(test_image_scaling_zero_dimensions);
    RUN_TEST(test_bilinear_interpolation_quality);
    
    // Memory management tests
    RUN_TEST(test_memory_management);
    
    UNITY_END();
}

void loop() {
    // Nothing to do in loop
}
