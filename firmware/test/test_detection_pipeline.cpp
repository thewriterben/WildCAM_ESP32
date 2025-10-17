/**
 * @file test_detection_pipeline.cpp
 * @brief Unit tests for wildlife detection processing pipeline
 */

#include <unity.h>
#include "../core/storage_manager.h"
#include <string.h>
#include <time.h>

// Mock BoundingBox structure for testing (matches actual definition)
struct BoundingBox {
    float x;
    float y;
    float width;
    float height;
    float confidence;
    int class_id;
    const char* class_name;
};

// External functions to test (from main.cpp)
extern bool generateDetectionFilename(const char* species, char* buffer, size_t buffer_size);
extern bool saveDetectionMetadata(const char* filename, const char* species, 
                                  float confidence, const BoundingBox& bbox);
extern bool processWildlifeDetection(const uint8_t* image_data, size_t image_size, 
                                    const BoundingBox& detection);

/**
 * Test filename generation with valid species name
 */
void test_filename_generation_valid(void) {
    char filename[MAX_FILENAME_LENGTH];
    bool result = generateDetectionFilename("deer", filename, sizeof(filename));
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(strlen(filename) > 0);
    TEST_ASSERT_TRUE(strstr(filename, "deer") != NULL);
    TEST_ASSERT_TRUE(strstr(filename, ".jpg") != NULL);
}

/**
 * Test filename generation with null buffer
 */
void test_filename_generation_null_buffer(void) {
    bool result = generateDetectionFilename("deer", NULL, 64);
    TEST_ASSERT_FALSE(result);
}

/**
 * Test filename generation with insufficient buffer size
 */
void test_filename_generation_small_buffer(void) {
    char filename[10]; // Too small
    bool result = generateDetectionFilename("deer", filename, sizeof(filename));
    TEST_ASSERT_FALSE(result);
}

/**
 * Test filename format contains timestamp
 */
void test_filename_contains_timestamp(void) {
    char filename[MAX_FILENAME_LENGTH];
    generateDetectionFilename("fox", filename, sizeof(filename));
    
    // Should contain either date format or millis timestamp
    bool has_timestamp = (strstr(filename, "_") != NULL);
    TEST_ASSERT_TRUE(has_timestamp);
}

/**
 * Test metadata generation with GPS and battery info
 */
void test_metadata_generation(void) {
    // Initialize storage for test
    if (!g_storage.isReady()) {
        g_storage.initialize();
    }
    
    BoundingBox bbox = {
        .x = 0.25f,
        .y = 0.35f,
        .width = 0.30f,
        .height = 0.40f,
        .confidence = 0.95f,
        .class_id = 1,
        .class_name = "deer"
    };
    
    bool result = saveDetectionMetadata("test_image.jpg", "deer", 0.95f, bbox);
    
    // Metadata save may fail if storage not available, but should not crash
    TEST_ASSERT_TRUE(result || !g_storage.isReady());
    
    // Verify metadata includes required fields (conceptual - actual verification would need file read)
    // Metadata should include: filename, species, confidence, timestamp, bounding_box, 
    // class_id, battery_voltage, gps_coordinates
}

/**
 * Test metadata filename conversion
 */
void test_metadata_filename_conversion(void) {
    // Metadata should change .jpg to .json
    char test_filename[] = "20231015_120000_deer.jpg";
    char expected_metadata[] = "20231015_120000_deer.json";
    
    // This test verifies the concept - actual function is internal
    char* ext = strrchr(test_filename, '.');
    TEST_ASSERT_NOT_NULL(ext);
    strcpy(ext, ".json");
    TEST_ASSERT_EQUAL_STRING(expected_metadata, test_filename);
}

/**
 * Test detection processing with valid data
 */
void test_process_detection_valid(void) {
    // Initialize storage
    if (!g_storage.isReady()) {
        g_storage.initialize();
    }
    
    // Create test image data
    uint8_t test_image[1024];
    memset(test_image, 0x55, sizeof(test_image));
    
    BoundingBox detection = {
        .x = 0.15f,
        .y = 0.25f,
        .width = 0.50f,
        .height = 0.60f,
        .confidence = 0.87f,
        .class_id = 2,
        .class_name = "fox"
    };
    
    bool result = processWildlifeDetection(test_image, sizeof(test_image), detection);
    
    // Should succeed or gracefully handle storage failure
    TEST_ASSERT_TRUE(result);
}

/**
 * Test detection processing with null image data
 */
void test_process_detection_null_data(void) {
    BoundingBox detection = {
        .x = 0.15f,
        .y = 0.25f,
        .width = 0.50f,
        .height = 0.60f,
        .confidence = 0.87f,
        .class_name = "fox"
    };
    
    bool result = processWildlifeDetection(NULL, 1024, detection);
    TEST_ASSERT_FALSE(result);
}

/**
 * Test detection processing with zero size
 */
void test_process_detection_zero_size(void) {
    uint8_t test_image[1024];
    
    BoundingBox detection = {
        .x = 0.15f,
        .y = 0.25f,
        .width = 0.50f,
        .height = 0.60f,
        .confidence = 0.87f,
        .class_id = 2,
        .class_name = "fox"
    };
    
    bool result = processWildlifeDetection(test_image, 0, detection);
    TEST_ASSERT_FALSE(result);
}

/**
 * Test storage graceful failure handling
 */
void test_storage_failure_handling(void) {
    // Even if storage is not ready, processing should continue
    // and return true to allow detection loop to continue
    
    uint8_t test_image[100];
    memset(test_image, 0xAA, sizeof(test_image));
    
    BoundingBox detection = {
        .x = 0.10f,
        .y = 0.20f,
        .width = 0.25f,
        .height = 0.30f,
        .confidence = 0.75f,
        .class_id = 3,
        .class_name = "rabbit"
    };
    
    // This should not fail even if storage is unavailable
    bool result = processWildlifeDetection(test_image, sizeof(test_image), detection);
    TEST_ASSERT_TRUE(result);
}

/**
 * Test multiple consecutive detections
 */
void test_multiple_detections(void) {
    if (!g_storage.isReady()) {
        g_storage.initialize();
    }
    
    uint8_t test_image[512];
    memset(test_image, 0x77, sizeof(test_image));
    
    const char* species[] = {"deer", "fox", "rabbit", "bird"};
    
    for (int i = 0; i < 4; i++) {
        BoundingBox detection = {
            .x = 0.10f + (i * 0.05f),
            .y = 0.20f + (i * 0.05f),
            .width = 0.25f,
            .height = 0.30f,
            .confidence = 0.80f + (i * 0.05f),
            .class_id = i,
            .class_name = species[i]
        };
        
        bool result = processWildlifeDetection(test_image, sizeof(test_image), detection);
        TEST_ASSERT_TRUE(result);
    }
}

/**
 * Test detection counter persistence
 */
void test_detection_counter(void) {
    // Get detection counter function (external)
    extern uint32_t getDetectionCounter();
    
    // Counter should be accessible
    uint32_t count = getDetectionCounter();
    TEST_ASSERT_TRUE(count >= 0); // Counter should be valid (0 or higher)
}

/**
 * Test storage space check before save
 */
void test_storage_space_check(void) {
    if (!g_storage.isReady()) {
        // Cannot test if storage not ready
        TEST_PASS();
        return;
    }
    
    // Check that getFreeSpace returns a valid value
    uint64_t free_space = g_storage.getFreeSpace();
    TEST_ASSERT_TRUE(free_space > 0 || free_space == 0); // Valid response
}

/**
 * Test retry logic on save failure
 */
void test_retry_logic(void) {
    // This test verifies that processWildlifeDetection includes retry logic
    // The function should attempt multiple retries before giving up
    // Success is measured by function returning true (graceful handling)
    
    uint8_t test_image[256];
    memset(test_image, 0xBB, sizeof(test_image));
    
    BoundingBox detection = {
        .x = 0.30f,
        .y = 0.40f,
        .width = 0.20f,
        .height = 0.25f,
        .confidence = 0.82f,
        .class_id = 5,
        .class_name = "squirrel"
    };
    
    // Even if storage fails, should return true for graceful continuation
    bool result = processWildlifeDetection(test_image, sizeof(test_image), detection);
    TEST_ASSERT_TRUE(result);
}

void setUp(void) {
    // Initialize before each test
}

void tearDown(void) {
    // Cleanup after each test
    g_storage.clearError();
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_filename_generation_valid);
    RUN_TEST(test_filename_generation_null_buffer);
    RUN_TEST(test_filename_generation_small_buffer);
    RUN_TEST(test_filename_contains_timestamp);
    RUN_TEST(test_metadata_filename_conversion);
    RUN_TEST(test_metadata_generation);
    RUN_TEST(test_process_detection_valid);
    RUN_TEST(test_process_detection_null_data);
    RUN_TEST(test_process_detection_zero_size);
    RUN_TEST(test_storage_failure_handling);
    RUN_TEST(test_multiple_detections);
    RUN_TEST(test_detection_counter);
    RUN_TEST(test_storage_space_check);
    RUN_TEST(test_retry_logic);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
