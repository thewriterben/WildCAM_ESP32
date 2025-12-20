/**
 * @file test_basic_ai_features.cpp
 * @brief Unit tests for basic AI features module
 */

#include <unity.h>
#include "../ml_models/basic_ai_features.h"
#include <cstring>
#include <cstdlib>

using namespace BasicAI;

// Test configuration
static BasicAIProcessor* processor = nullptr;
static const uint16_t TEST_WIDTH = 160;
static const uint16_t TEST_HEIGHT = 120;
static const size_t TEST_FRAME_SIZE = TEST_WIDTH * TEST_HEIGHT;

/**
 * Set up test fixture
 */
void setUp(void) {
    processor = new BasicAIProcessor();
}

/**
 * Tear down test fixture
 */
void tearDown(void) {
    if (processor) {
        delete processor;
        processor = nullptr;
    }
}

//==============================================================================
// Configuration Tests
//==============================================================================

/**
 * Test default configuration creation
 */
void test_default_config(void) {
    BasicAIConfig config = getDefaultConfig();
    
    TEST_ASSERT_EQUAL(DEFAULT_MOTION_THRESHOLD, config.motion_threshold);
    TEST_ASSERT_EQUAL(2, config.min_motion_frames);
    TEST_ASSERT_TRUE(config.enable_temporal_filtering);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.3f, config.noise_reduction_level);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, CLASSIFICATION_THRESHOLD, config.animal_confidence_threshold);
    TEST_ASSERT_TRUE(config.enable_human_detection);
    TEST_ASSERT_TRUE(config.enable_vehicle_detection);
    TEST_ASSERT_TRUE(config.enable_size_estimation);
    TEST_ASSERT_EQUAL(320, config.processing_width);
    TEST_ASSERT_EQUAL(240, config.processing_height);
}

/**
 * Test low power configuration
 */
void test_low_power_config(void) {
    BasicAIConfig config = getLowPowerConfig();
    
    TEST_ASSERT_TRUE(config.fast_mode);
    TEST_ASSERT_EQUAL(160, config.processing_width);
    TEST_ASSERT_EQUAL(120, config.processing_height);
    TEST_ASSERT_FALSE(config.enable_temporal_filtering);
    TEST_ASSERT_EQUAL(1, config.min_motion_frames);
}

/**
 * Test high accuracy configuration
 */
void test_high_accuracy_config(void) {
    BasicAIConfig config = getHighAccuracyConfig();
    
    TEST_ASSERT_EQUAL(20, config.motion_threshold);
    TEST_ASSERT_EQUAL(3, config.min_motion_frames);
    TEST_ASSERT_FALSE(config.fast_mode);
    TEST_ASSERT_EQUAL(640, config.processing_width);
    TEST_ASSERT_EQUAL(480, config.processing_height);
}

//==============================================================================
// Initialization Tests
//==============================================================================

/**
 * Test processor initialization with valid config
 */
void test_processor_initialization(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    
    bool result = processor->initialize(config);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(processor->isInitialized());
}

/**
 * Test processor initialization with invalid config
 */
void test_processor_initialization_invalid_config(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = 0;  // Invalid
    config.processing_height = 0;
    
    bool result = processor->initialize(config);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(processor->isInitialized());
}

/**
 * Test processor configuration update
 */
void test_config_update(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    
    processor->initialize(config);
    
    BasicAIConfig new_config = getLowPowerConfig();
    processor->updateConfig(new_config);
    
    BasicAIConfig retrieved = processor->getConfig();
    TEST_ASSERT_TRUE(retrieved.fast_mode);
}

//==============================================================================
// Motion Detection Tests
//==============================================================================

/**
 * Test motion detection with no motion (identical frames)
 */
void test_motion_detection_no_motion(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = false;
    processor->initialize(config);
    
    // Create two identical frames
    uint8_t* frame1 = new uint8_t[TEST_FRAME_SIZE];
    uint8_t* frame2 = new uint8_t[TEST_FRAME_SIZE];
    memset(frame1, 128, TEST_FRAME_SIZE);
    memset(frame2, 128, TEST_FRAME_SIZE);
    
    // First frame to establish baseline
    processor->detectMotion(frame1, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    
    // Second frame should show no motion
    MotionDetectionResult result = processor->detectMotion(
        frame2, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1
    );
    
    TEST_ASSERT_FALSE(result.motion_detected);
    TEST_ASSERT_EQUAL(MotionConfidence::NONE, result.confidence);
    TEST_ASSERT_EQUAL(0, result.region_count);
    
    delete[] frame1;
    delete[] frame2;
}

/**
 * Test motion detection with significant motion
 */
void test_motion_detection_with_motion(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = false;
    processor->initialize(config);
    
    // Create two different frames
    uint8_t* frame1 = new uint8_t[TEST_FRAME_SIZE];
    uint8_t* frame2 = new uint8_t[TEST_FRAME_SIZE];
    memset(frame1, 50, TEST_FRAME_SIZE);
    memset(frame2, 50, TEST_FRAME_SIZE);
    
    // Add a "moving object" in frame2 (bright region)
    for (int y = 40; y < 80; y++) {
        for (int x = 40; x < 100; x++) {
            frame2[y * TEST_WIDTH + x] = 200;
        }
    }
    
    // First frame to establish baseline
    processor->detectMotion(frame1, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    
    // Second frame should show motion
    MotionDetectionResult result = processor->detectMotion(
        frame2, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1
    );
    
    TEST_ASSERT_TRUE(result.motion_detected);
    TEST_ASSERT_GREATER_THAN(0, result.region_count);
    TEST_ASSERT_NOT_EQUAL(MotionConfidence::NONE, result.confidence);
    
    delete[] frame1;
    delete[] frame2;
}

/**
 * Test motion detection with RGB input
 */
void test_motion_detection_rgb_input(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = false;
    processor->initialize(config);
    
    size_t rgb_size = TEST_FRAME_SIZE * 3;
    uint8_t* frame1 = new uint8_t[rgb_size];
    uint8_t* frame2 = new uint8_t[rgb_size];
    
    // Initialize with uniform color
    for (size_t i = 0; i < rgb_size; i += 3) {
        frame1[i] = 100;     // R
        frame1[i+1] = 100;   // G
        frame1[i+2] = 100;   // B
        frame2[i] = 100;
        frame2[i+1] = 100;
        frame2[i+2] = 100;
    }
    
    // Add motion in frame2
    for (int y = 40; y < 80; y++) {
        for (int x = 40; x < 100; x++) {
            size_t idx = (y * TEST_WIDTH + x) * 3;
            frame2[idx] = 255;
            frame2[idx+1] = 255;
            frame2[idx+2] = 255;
        }
    }
    
    // Process frames
    processor->detectMotion(frame1, rgb_size, TEST_WIDTH, TEST_HEIGHT, 3);
    MotionDetectionResult result = processor->detectMotion(
        frame2, rgb_size, TEST_WIDTH, TEST_HEIGHT, 3
    );
    
    TEST_ASSERT_TRUE(result.motion_detected);
    
    delete[] frame1;
    delete[] frame2;
}

/**
 * Test false positive score calculation
 */
void test_false_positive_analysis(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = true;
    processor->initialize(config);
    
    // Create frames with motion
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    memset(frame, 100, TEST_FRAME_SIZE);
    
    // Process multiple frames to build history
    for (int i = 0; i < 5; i++) {
        // Add some variation
        for (int y = 40; y < 60; y++) {
            for (int x = 40; x < 60; x++) {
                frame[y * TEST_WIDTH + x] = 100 + (i % 2) * 100;
            }
        }
        processor->detectMotion(frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    }
    
    // Final detection
    MotionDetectionResult result = processor->detectMotion(
        frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1
    );
    
    // False positive score should be calculated
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, result.false_positive_score);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, result.false_positive_score);
    
    delete[] frame;
}

//==============================================================================
// Classification Tests
//==============================================================================

/**
 * Test classification with animal-like characteristics
 */
void test_classification_animal_characteristics(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    // Create frame with texture (simulating fur/feathers)
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    
    // Generate texture pattern
    for (int y = 0; y < TEST_HEIGHT; y++) {
        for (int x = 0; x < TEST_WIDTH; x++) {
            // Create moderate texture variation
            int noise = (rand() % 40) - 20;
            int pattern = ((x + y) % 10 < 5) ? 30 : 0;
            frame[y * TEST_WIDTH + x] = static_cast<uint8_t>(
                std::min(255, std::max(0, 100 + noise + pattern))
            );
        }
    }
    
    BoundingBox roi = {20, 20, 100, 80};
    ClassificationResult result = processor->classifyRegion(
        frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, roi, 1
    );
    
    // Should get a valid classification result
    TEST_ASSERT_NOT_EQUAL(ClassificationType::UNKNOWN, result.classification);
    TEST_ASSERT_GREATER_THAN(0.0f, result.confidence);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, result.confidence);
    
    delete[] frame;
}

/**
 * Test classification with non-animal characteristics
 */
void test_classification_non_animal_characteristics(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    // Create uniform frame (simulating shadow or flat surface)
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    memset(frame, 128, TEST_FRAME_SIZE);
    
    BoundingBox roi = {20, 20, 100, 80};
    ClassificationResult result = processor->classifyRegion(
        frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, roi, 1
    );
    
    // Should classify as non-animal due to lack of texture
    // Note: With uniform input, may classify as non-animal or unknown
    TEST_ASSERT_GREATER_THAN(0.0f, result.non_animal_score);
    
    delete[] frame;
}

/**
 * Test classification with invalid ROI
 */
void test_classification_invalid_roi(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    memset(frame, 128, TEST_FRAME_SIZE);
    
    // Invalid ROI (out of bounds)
    BoundingBox roi = {TEST_WIDTH + 10, TEST_HEIGHT + 10, 100, 80};
    ClassificationResult result = processor->classifyRegion(
        frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, roi, 1
    );
    
    // Should return unknown classification
    TEST_ASSERT_EQUAL(ClassificationType::UNKNOWN, result.classification);
    
    delete[] frame;
}

//==============================================================================
// Size Estimation Tests
//==============================================================================

/**
 * Test size category determination
 */
void test_size_category_determination(void) {
    TEST_ASSERT_EQUAL(SizeCategory::TINY, BasicAIProcessor::getSizeCategory(0.03f));
    TEST_ASSERT_EQUAL(SizeCategory::SMALL, BasicAIProcessor::getSizeCategory(0.10f));
    TEST_ASSERT_EQUAL(SizeCategory::MEDIUM, BasicAIProcessor::getSizeCategory(0.25f));
    TEST_ASSERT_EQUAL(SizeCategory::LARGE, BasicAIProcessor::getSizeCategory(0.40f));
    TEST_ASSERT_EQUAL(SizeCategory::VERY_LARGE, BasicAIProcessor::getSizeCategory(0.60f));
}

/**
 * Test size estimation calculation
 */
void test_size_estimation(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    // Small object
    BoundingBox small_bbox = {0, 0, 20, 15};
    SizeEstimationResult small_result = processor->estimateSize(
        small_bbox, TEST_WIDTH, TEST_HEIGHT
    );
    
    TEST_ASSERT_EQUAL(SizeCategory::TINY, small_result.category);
    float expected_small = (20.0f * 15.0f) / (TEST_WIDTH * TEST_HEIGHT);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected_small, small_result.relative_size);
    
    // Medium object
    BoundingBox medium_bbox = {0, 0, 60, 50};
    SizeEstimationResult medium_result = processor->estimateSize(
        medium_bbox, TEST_WIDTH, TEST_HEIGHT
    );
    
    TEST_ASSERT_EQUAL(SizeCategory::MEDIUM, medium_result.category);
    
    // Large object
    BoundingBox large_bbox = {0, 0, 100, 80};
    SizeEstimationResult large_result = processor->estimateSize(
        large_bbox, TEST_WIDTH, TEST_HEIGHT
    );
    
    TEST_ASSERT_EQUAL(SizeCategory::LARGE, large_result.category);
}

/**
 * Test size estimation with edge cases
 */
void test_size_estimation_edge_cases(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    // Zero-size bbox
    BoundingBox zero_bbox = {0, 0, 0, 0};
    SizeEstimationResult zero_result = processor->estimateSize(
        zero_bbox, TEST_WIDTH, TEST_HEIGHT
    );
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, zero_result.relative_size);
    
    // Full-frame bbox
    BoundingBox full_bbox = {0, 0, TEST_WIDTH, TEST_HEIGHT};
    SizeEstimationResult full_result = processor->estimateSize(
        full_bbox, TEST_WIDTH, TEST_HEIGHT
    );
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, full_result.relative_size);
}

//==============================================================================
// Combined Analysis Tests
//==============================================================================

/**
 * Test complete frame analysis
 */
void test_complete_frame_analysis(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = false;
    processor->initialize(config);
    
    // Create test frames
    uint8_t* frame1 = new uint8_t[TEST_FRAME_SIZE];
    uint8_t* frame2 = new uint8_t[TEST_FRAME_SIZE];
    memset(frame1, 100, TEST_FRAME_SIZE);
    memset(frame2, 100, TEST_FRAME_SIZE);
    
    // Add "animal" with texture in frame2
    for (int y = 30; y < 90; y++) {
        for (int x = 30; x < 110; x++) {
            int noise = (rand() % 30) - 15;
            int pattern = ((x + y) % 8 < 4) ? 20 : 0;
            frame2[y * TEST_WIDTH + x] = static_cast<uint8_t>(
                std::min(255, std::max(0, 150 + noise + pattern))
            );
        }
    }
    
    // Establish baseline
    processor->analyzeFrame(frame1, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    
    // Analyze frame with object
    AIAnalysisResult result = processor->analyzeFrame(
        frame2, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1
    );
    
    // Verify all components of analysis
    TEST_ASSERT_TRUE(result.motion.motion_detected);
    TEST_ASSERT_GREATER_THAN(0.0f, result.overall_confidence);
    TEST_ASSERT_GREATER_THAN(0, result.total_processing_time_us);
    
    // Size should be estimated
    TEST_ASSERT_GREATER_THAN(0.0f, result.size.relative_size);
    
    delete[] frame1;
    delete[] frame2;
}

//==============================================================================
// Statistics Tests
//==============================================================================

/**
 * Test statistics tracking
 */
void test_statistics_tracking(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    config.enable_temporal_filtering = false;
    processor->initialize(config);
    
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    memset(frame, 100, TEST_FRAME_SIZE);
    
    // Process several frames
    for (int i = 0; i < 5; i++) {
        processor->detectMotion(frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    }
    
    BasicAIProcessor::Statistics stats = processor->getStatistics();
    TEST_ASSERT_EQUAL(5, stats.total_frames_processed);
    TEST_ASSERT_GREATER_THAN(0, stats.average_processing_time_us);
    
    delete[] frame;
}

/**
 * Test statistics reset
 */
void test_statistics_reset(void) {
    BasicAIConfig config = getDefaultConfig();
    config.processing_width = TEST_WIDTH;
    config.processing_height = TEST_HEIGHT;
    processor->initialize(config);
    
    uint8_t* frame = new uint8_t[TEST_FRAME_SIZE];
    memset(frame, 100, TEST_FRAME_SIZE);
    
    // Process some frames
    processor->detectMotion(frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    processor->detectMotion(frame, TEST_FRAME_SIZE, TEST_WIDTH, TEST_HEIGHT, 1);
    
    // Reset and verify
    processor->resetStatistics();
    BasicAIProcessor::Statistics stats = processor->getStatistics();
    
    TEST_ASSERT_EQUAL(0, stats.total_frames_processed);
    TEST_ASSERT_EQUAL(0, stats.motion_detections);
    
    delete[] frame;
}

//==============================================================================
// Utility Function Tests
//==============================================================================

/**
 * Test string conversion functions
 */
void test_string_conversions(void) {
    // Classification type
    TEST_ASSERT_EQUAL_STRING("Unknown", classificationTypeToString(ClassificationType::UNKNOWN));
    TEST_ASSERT_EQUAL_STRING("Animal", classificationTypeToString(ClassificationType::ANIMAL));
    TEST_ASSERT_EQUAL_STRING("Non-Animal", classificationTypeToString(ClassificationType::NON_ANIMAL));
    TEST_ASSERT_EQUAL_STRING("Human", classificationTypeToString(ClassificationType::HUMAN));
    TEST_ASSERT_EQUAL_STRING("Vehicle", classificationTypeToString(ClassificationType::VEHICLE));
    
    // Size category
    TEST_ASSERT_EQUAL_STRING("Tiny", sizeCategoryToString(SizeCategory::TINY));
    TEST_ASSERT_EQUAL_STRING("Small", sizeCategoryToString(SizeCategory::SMALL));
    TEST_ASSERT_EQUAL_STRING("Medium", sizeCategoryToString(SizeCategory::MEDIUM));
    TEST_ASSERT_EQUAL_STRING("Large", sizeCategoryToString(SizeCategory::LARGE));
    TEST_ASSERT_EQUAL_STRING("Very Large", sizeCategoryToString(SizeCategory::VERY_LARGE));
    
    // Motion confidence
    TEST_ASSERT_EQUAL_STRING("None", motionConfidenceToString(MotionConfidence::NONE));
    TEST_ASSERT_EQUAL_STRING("Low", motionConfidenceToString(MotionConfidence::LOW));
    TEST_ASSERT_EQUAL_STRING("Medium", motionConfidenceToString(MotionConfidence::MEDIUM));
    TEST_ASSERT_EQUAL_STRING("High", motionConfidenceToString(MotionConfidence::HIGH));
    TEST_ASSERT_EQUAL_STRING("Very High", motionConfidenceToString(MotionConfidence::VERY_HIGH));
}

//==============================================================================
// Test Runner
//==============================================================================

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Configuration tests
    RUN_TEST(test_default_config);
    RUN_TEST(test_low_power_config);
    RUN_TEST(test_high_accuracy_config);
    
    // Initialization tests
    RUN_TEST(test_processor_initialization);
    RUN_TEST(test_processor_initialization_invalid_config);
    RUN_TEST(test_config_update);
    
    // Motion detection tests
    RUN_TEST(test_motion_detection_no_motion);
    RUN_TEST(test_motion_detection_with_motion);
    RUN_TEST(test_motion_detection_rgb_input);
    RUN_TEST(test_false_positive_analysis);
    
    // Classification tests
    RUN_TEST(test_classification_animal_characteristics);
    RUN_TEST(test_classification_non_animal_characteristics);
    RUN_TEST(test_classification_invalid_roi);
    
    // Size estimation tests
    RUN_TEST(test_size_category_determination);
    RUN_TEST(test_size_estimation);
    RUN_TEST(test_size_estimation_edge_cases);
    
    // Combined analysis tests
    RUN_TEST(test_complete_frame_analysis);
    
    // Statistics tests
    RUN_TEST(test_statistics_tracking);
    RUN_TEST(test_statistics_reset);
    
    // Utility tests
    RUN_TEST(test_string_conversions);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
