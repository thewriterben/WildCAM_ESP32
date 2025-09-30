/**
 * @file test_advanced_species_detection.cpp
 * @brief Test suite for Advanced Species Detection system
 * 
 * Validates the functionality of the advanced AI species detection
 * module for ESP32-S3 edge computing.
 */

#include <Arduino.h>
#include "../firmware/src/ai/advanced_species_detection.h"
#include <cassert>
#include <iostream>

// Test result tracking
struct TestStats {
    uint32_t totalTests = 0;
    uint32_t passedTests = 0;
    uint32_t failedTests = 0;
};

TestStats testStats;

// Test utilities
void assertTrue(bool condition, const char* message) {
    testStats.totalTests++;
    if (condition) {
        testStats.passedTests++;
        std::cout << "✓ PASS: " << message << std::endl;
    } else {
        testStats.failedTests++;
        std::cerr << "✗ FAIL: " << message << std::endl;
    }
}

void assertEqual(float expected, float actual, float tolerance, const char* message) {
    testStats.totalTests++;
    if (abs(expected - actual) <= tolerance) {
        testStats.passedTests++;
        std::cout << "✓ PASS: " << message << std::endl;
    } else {
        testStats.failedTests++;
        std::cerr << "✗ FAIL: " << message 
                  << " (expected: " << expected << ", got: " << actual << ")" << std::endl;
    }
}

// Mock camera frame for testing
CameraFrame createMockFrame(uint16_t width = 224, uint16_t height = 224) {
    CameraFrame frame;
    frame.width = width;
    frame.height = height;
    frame.length = width * height * 3; // RGB
    frame.data = new uint8_t[frame.length];
    
    // Fill with mock data
    for (size_t i = 0; i < frame.length; i++) {
        frame.data[i] = random(0, 256);
    }
    
    frame.timestamp = millis();
    frame.format = 1; // RGB
    
    return frame;
}

void deleteMockFrame(CameraFrame& frame) {
    if (frame.data) {
        delete[] frame.data;
        frame.data = nullptr;
    }
}

// ===========================================
// TEST CASES
// ===========================================

/**
 * Test: Basic initialization
 */
void testBasicInitialization() {
    std::cout << "\n=== Test: Basic Initialization ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    
    // Should not be initialized yet
    assertTrue(!detector.isInitialized(), 
              "Detector should not be initialized before init() call");
    
    // Configure for testing
    AdvancedDetectionConfig config;
    config.modelPath = "test_model.tflite";
    config.requiresPSRAM = false; // Allow testing without PSRAM
    config.requiresESP32S3 = false; // Allow testing on other platforms
    
    // Initialize
    bool initResult = detector.initialize(config);
    assertTrue(initResult || !psramFound(), 
              "Detector initialization should succeed (or skip if no PSRAM)");
    
    std::cout << "Basic initialization test completed" << std::endl;
}

/**
 * Test: Configuration validation
 */
void testConfigurationValidation() {
    std::cout << "\n=== Test: Configuration Validation ===" << std::endl;
    
    AdvancedDetectionConfig config;
    
    // Validate default configuration
    assertTrue(config.confidenceThreshold >= 0.0f && config.confidenceThreshold <= 1.0f,
              "Default confidence threshold should be in valid range [0, 1]");
    
    assertTrue(config.maxInferenceTime_ms > 0,
              "Max inference time should be positive");
    
    assertEqual(0.80f, config.confidenceThreshold, 0.01f,
               "Default confidence threshold should be 80%");
    
    // Test custom configuration
    config.confidenceThreshold = 0.90f;
    config.maxInferenceTime_ms = 3000;
    config.enableEdgeOptimization = false;
    
    assertEqual(0.90f, config.confidenceThreshold, 0.01f,
               "Custom confidence threshold should be set correctly");
    
    std::cout << "Configuration validation test completed" << std::endl;
}

/**
 * Test: Species detection
 */
void testSpeciesDetection() {
    std::cout << "\n=== Test: Species Detection ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping detection test - initialization failed" << std::endl;
        return;
    }
    
    // Create mock frame
    CameraFrame frame = createMockFrame();
    
    // Run detection
    AdvancedSpeciesResult result = detector.detectSpecies(frame);
    
    // Validate result structure
    assertTrue(result.primarySpecies != AdvancedSpeciesType::UNKNOWN || 
              result.primaryConfidence < config.confidenceThreshold,
              "Detection should identify species or have low confidence");
    
    assertTrue(result.primaryConfidence >= 0.0f && result.primaryConfidence <= 1.0f,
              "Primary confidence should be in valid range");
    
    assertTrue(result.secondaryConfidence >= 0.0f && result.secondaryConfidence <= 1.0f,
              "Secondary confidence should be in valid range");
    
    assertTrue(result.primaryConfidence >= result.secondaryConfidence,
              "Primary confidence should be >= secondary confidence");
    
    assertTrue(result.inferenceTime_ms < config.maxInferenceTime_ms + 1000,
              "Inference time should be within reasonable bounds");
    
    // Cleanup
    deleteMockFrame(frame);
    detector.shutdown();
    
    std::cout << "Species detection test completed" << std::endl;
}

/**
 * Test: Edge optimization
 */
void testEdgeOptimization() {
    std::cout << "\n=== Test: Edge Optimization ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    config.enableEdgeOptimization = true;
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping edge optimization test - initialization failed" << std::endl;
        return;
    }
    
    // Test different inference modes
    detector.setInferenceMode("fast");
    detector.setInferenceMode("balanced");
    detector.setInferenceMode("accurate");
    
    // Test power optimization
    bool powerOptResult = detector.optimizeForPower(0.8f); // 80% battery
    assertTrue(powerOptResult, "Power optimization should succeed");
    
    powerOptResult = detector.optimizeForPower(0.3f); // 30% battery
    assertTrue(powerOptResult, "Power optimization should succeed for low battery");
    
    powerOptResult = detector.optimizeForPower(0.1f); // 10% battery (critical)
    assertTrue(powerOptResult, "Power optimization should succeed for critical battery");
    
    detector.shutdown();
    
    std::cout << "Edge optimization test completed" << std::endl;
}

/**
 * Test: Taxonomic hierarchy
 */
void testTaxonomicHierarchy() {
    std::cout << "\n=== Test: Taxonomic Hierarchy ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    config.useHierarchicalClassification = true;
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping taxonomy test - initialization failed" << std::endl;
        return;
    }
    
    // Get taxonomy for known species
    TaxonomicInfo taxonomy = detector.getTaxonomy(AdvancedSpeciesType::WHITE_TAILED_DEER);
    
    assertTrue(taxonomy.commonName.length() > 0,
              "Common name should be populated");
    
    assertTrue(taxonomy.family.length() > 0,
              "Family should be populated");
    
    // Test species grouping by family
    std::vector<AdvancedSpeciesType> cervidae = detector.getSpeciesByFamily("Cervidae");
    assertTrue(cervidae.size() > 0,
              "Should find species in Cervidae family");
    
    detector.shutdown();
    
    std::cout << "Taxonomic hierarchy test completed" << std::endl;
}

/**
 * Test: Performance metrics
 */
void testPerformanceMetrics() {
    std::cout << "\n=== Test: Performance Metrics ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping metrics test - initialization failed" << std::endl;
        return;
    }
    
    // Run several detections
    CameraFrame frame = createMockFrame();
    for (int i = 0; i < 5; i++) {
        detector.detectSpecies(frame);
    }
    
    // Get metrics
    AIMetrics metrics = detector.getPerformanceMetrics();
    uint32_t totalDetections = detector.getTotalDetections();
    
    assertTrue(totalDetections == 5,
              "Should have recorded 5 detections");
    
    assertTrue(metrics.totalInferences == 5,
              "Metrics should show 5 inferences");
    
    assertTrue(metrics.inferenceTime > 0.0f,
              "Average inference time should be positive");
    
    // Reset metrics
    detector.resetMetrics();
    metrics = detector.getPerformanceMetrics();
    
    assertTrue(metrics.totalInferences == 0,
              "Metrics should be reset to zero");
    
    // Cleanup
    deleteMockFrame(frame);
    detector.shutdown();
    
    std::cout << "Performance metrics test completed" << std::endl;
}

/**
 * Test: Multiple species detection
 */
void testMultipleSpeciesDetection() {
    std::cout << "\n=== Test: Multiple Species Detection ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping multiple detection test - initialization failed" << std::endl;
        return;
    }
    
    CameraFrame frame = createMockFrame();
    
    // Detect multiple species
    std::vector<AdvancedSpeciesResult> results = detector.detectMultipleSpecies(frame);
    
    assertTrue(results.size() >= 0,
              "Should return valid results vector");
    
    // Validate each result
    for (const auto& result : results) {
        assertTrue(result.primaryConfidence >= 0.0f && result.primaryConfidence <= 1.0f,
                  "Each result should have valid confidence");
    }
    
    deleteMockFrame(frame);
    detector.shutdown();
    
    std::cout << "Multiple species detection test completed" << std::endl;
}

/**
 * Test: Model information
 */
void testModelInformation() {
    std::cout << "\n=== Test: Model Information ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    config.modelPath = "wildlife_research_v3.tflite";
    
    if (!detector.initialize(config)) {
        std::cout << "Skipping model info test - initialization failed" << std::endl;
        return;
    }
    
    ModelInfo modelInfo = detector.getModelInfo();
    
    assertTrue(modelInfo.name.length() > 0,
              "Model name should be populated");
    
    assertTrue(modelInfo.accuracy >= 0.0f && modelInfo.accuracy <= 1.0f,
              "Model accuracy should be in valid range");
    
    assertTrue(modelInfo.isLoaded,
              "Model should be marked as loaded");
    
    detector.shutdown();
    
    std::cout << "Model information test completed" << std::endl;
}

/**
 * Test: Self-test functionality
 */
void testSelfTest() {
    std::cout << "\n=== Test: Self-Test Functionality ===" << std::endl;
    
    AdvancedSpeciesDetector detector;
    
    // Self-test should fail before initialization
    bool selfTestResult = detector.runSelfTest();
    assertTrue(!selfTestResult,
              "Self-test should fail before initialization");
    
    // Initialize
    AdvancedDetectionConfig config;
    config.requiresPSRAM = false;
    config.requiresESP32S3 = false;
    
    if (detector.initialize(config)) {
        // Self-test should pass after initialization
        selfTestResult = detector.runSelfTest();
        assertTrue(selfTestResult,
                  "Self-test should pass after successful initialization");
    }
    
    detector.shutdown();
    
    std::cout << "Self-test functionality test completed" << std::endl;
}

/**
 * Test: Utility functions
 */
void testUtilityFunctions() {
    std::cout << "\n=== Test: Utility Functions ===" << std::endl;
    
    // Test species to string conversion
    const char* name = advancedSpeciesToString(AdvancedSpeciesType::WHITE_TAILED_DEER);
    assertTrue(strlen(name) > 0,
              "Species name should be non-empty");
    
    // Test size estimation
    float size = getTypicalSpeciesSize(AdvancedSpeciesType::WHITE_TAILED_DEER);
    assertTrue(size > 0.0f,
              "Species size should be positive");
    
    // Test conservation status
    String status = getConservationStatus(AdvancedSpeciesType::WHITE_TAILED_DEER);
    assertTrue(status.length() > 0,
              "Conservation status should be non-empty");
    
    // Test endangered species check
    bool isEndangered = isEndangeredSpecies(AdvancedSpeciesType::GRAY_WOLF);
    // Gray Wolf is considered endangered in many regions
    assertTrue(isEndangered,
              "Gray Wolf should be marked as endangered");
    
    std::cout << "Utility functions test completed" << std::endl;
}

// ===========================================
// MAIN TEST RUNNER
// ===========================================

void runAllTests() {
    std::cout << "\n" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Advanced Species Detection Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Run all tests
    testBasicInitialization();
    testConfigurationValidation();
    testSpeciesDetection();
    testEdgeOptimization();
    testTaxonomicHierarchy();
    testPerformanceMetrics();
    testMultipleSpeciesDetection();
    testModelInformation();
    testSelfTest();
    testUtilityFunctions();
    
    // Print summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Total tests:  " << testStats.totalTests << std::endl;
    std::cout << "Passed:       " << testStats.passedTests << " ✓" << std::endl;
    std::cout << "Failed:       " << testStats.failedTests << " ✗" << std::endl;
    
    float successRate = (float)testStats.passedTests / testStats.totalTests * 100.0f;
    std::cout << "Success rate: " << successRate << "%" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (testStats.failedTests == 0) {
        std::cout << "\n✓ All tests PASSED!" << std::endl;
    } else {
        std::cout << "\n✗ Some tests FAILED!" << std::endl;
    }
}

// Arduino setup and loop for embedded testing
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    std::cout << "Starting Advanced Species Detection tests..." << std::endl;
    runAllTests();
}

void loop() {
    // Tests run once in setup
    delay(10000);
}

// Native C++ main for desktop testing
#ifndef ARDUINO
int main() {
    runAllTests();
    return (testStats.failedTests == 0) ? 0 : 1;
}
#endif
