/**
 * @file ai_validation_test.cpp
 * @brief Validation tests for AI wildlife detection system
 * 
 * This file contains unit tests and integration tests for the AI wildlife
 * detection pipeline to ensure proper functionality.
 */

#include <Arduino.h>
#include "../firmware/src/ai/tensorflow_lite_implementation.h"
#include "../src/ai/wildlife_classifier.h"
#include <esp_heap_caps.h>

// Test configuration
#define TEST_IMAGE_WIDTH 224
#define TEST_IMAGE_HEIGHT 224
#define TEST_IMAGE_CHANNELS 3
#define TEST_ITERATIONS 5

/**
 * Test Results Structure
 */
struct TestResult {
    bool passed;
    String description;
    uint32_t executionTime;
    String errorMessage;
    
    TestResult(bool p = false, String desc = "", uint32_t time = 0, String error = "") 
        : passed(p), description(desc), executionTime(time), errorMessage(error) {}
};

class AIValidationTest {
private:
    WildlifeClassifier classifier;
    uint32_t testCount = 0;
    uint32_t passedTests = 0;
    
public:
    /**
     * Initialize test system
     */
    bool initialize() {
        Serial.println("=== AI Wildlife Detection Validation Tests ===");
        Serial.println();
        
        // Check PSRAM availability
        if (!psramFound()) {
            Serial.println("WARNING: PSRAM not found, some tests may fail");
        } else {
            Serial.printf("PSRAM available: %d bytes\n", ESP.getPsramSize());
        }
        
        return true;
    }
    
    /**
     * Run all validation tests
     */
    void runAllTests() {
        Serial.println("Starting AI validation tests...\n");
        
        // Core system tests
        runTest(testTensorFlowLiteInitialization());
        runTest(testWildlifeClassifierInitialization());
        runTest(testMemoryAllocation());
        runTest(testModelValidation());
        
        // Inference tests
        runTest(testBasicInference());
        runTest(testSpeciesClassification());
        runTest(testDangerousSpeciesDetection());
        runTest(testPerformanceBenchmark());
        
        // Memory management tests
        runTest(testMemoryOptimization());
        runTest(testFragmentationHandling());
        
        // Print final results
        printTestSummary();
    }
    
private:
    /**
     * Run individual test and track results
     */
    void runTest(TestResult result) {
        testCount++;
        Serial.printf("Test %d: %s\n", testCount, result.description.c_str());
        
        if (result.passed) {
            Serial.printf("  ✅ PASSED (%d ms)\n", result.executionTime);
            passedTests++;
        } else {
            Serial.printf("  ❌ FAILED: %s\n", result.errorMessage.c_str());
        }
        Serial.println();
    }
    
    /**
     * Test TensorFlow Lite initialization
     */
    TestResult testTensorFlowLiteInitialization() {
        uint32_t startTime = millis();
        
        bool success = initializeTensorFlowLite();
        
        uint32_t duration = millis() - startTime;
        
        if (!success) {
            return TestResult(false, "TensorFlow Lite Initialization", duration, 
                            "Failed to initialize TensorFlow Lite system");
        }
        
        // Check global instance
        if (!g_tensorflowImplementation) {
            return TestResult(false, "TensorFlow Lite Initialization", duration,
                            "Global TensorFlow Lite instance not created");
        }
        
        if (!g_tensorflowImplementation->isInitialized()) {
            return TestResult(false, "TensorFlow Lite Initialization", duration,
                            "TensorFlow Lite instance not properly initialized");
        }
        
        return TestResult(true, "TensorFlow Lite Initialization", duration);
    }
    
    /**
     * Test wildlife classifier initialization
     */
    TestResult testWildlifeClassifierInitialization() {
        uint32_t startTime = millis();
        
        bool success = classifier.initialize();
        
        uint32_t duration = millis() - startTime;
        
        if (!success) {
            return TestResult(false, "Wildlife Classifier Initialization", duration,
                            "Failed to initialize wildlife classifier");
        }
        
        if (!classifier.isEnabled()) {
            return TestResult(false, "Wildlife Classifier Initialization", duration,
                            "Classifier not enabled after initialization");
        }
        
        return TestResult(true, "Wildlife Classifier Initialization", duration);
    }
    
    /**
     * Test memory allocation for AI operations
     */
    TestResult testMemoryAllocation() {
        uint32_t startTime = millis();
        
        size_t initialFreeHeap = ESP.getFreeHeap();
        size_t initialFreePsram = ESP.getFreePsram();
        
        // Test large memory allocation (simulate model loading)
        size_t testSize = 512 * 1024; // 512KB
        void* testBuffer = NULL;
        
        if (psramFound()) {
            testBuffer = ps_malloc(testSize);
        } else {
            testBuffer = malloc(testSize);
        }
        
        uint32_t duration = millis() - startTime;
        
        if (!testBuffer) {
            return TestResult(false, "Memory Allocation Test", duration,
                            "Failed to allocate 512KB test buffer");
        }
        
        // Fill buffer to ensure it's actually allocated
        memset(testBuffer, 0xAA, testSize);
        
        // Check memory usage
        size_t freeHeapAfter = ESP.getFreeHeap();
        size_t freePsramAfter = ESP.getFreePsram();
        
        // Clean up
        free(testBuffer);
        
        // Verify memory was actually allocated
        bool memoryChanged = (initialFreeHeap != freeHeapAfter) || (initialFreePsram != freePsramAfter);
        
        if (!memoryChanged) {
            return TestResult(false, "Memory Allocation Test", duration,
                            "Memory allocation did not affect heap/PSRAM usage");
        }
        
        return TestResult(true, "Memory Allocation Test", duration);
    }
    
    /**
     * Test model validation functionality
     */
    TestResult testModelValidation() {
        uint32_t startTime = millis();
        
        if (!g_tensorflowImplementation) {
            return TestResult(false, "Model Validation Test", 0,
                            "TensorFlow Lite not initialized");
        }
        
        // Test validation of non-existent model (should fail gracefully)
        bool nonExistentResult = g_tensorflowImplementation->validateModel("/nonexistent/model.tflite");
        
        if (nonExistentResult) {
            return TestResult(false, "Model Validation Test", millis() - startTime,
                            "Validation should fail for non-existent model");
        }
        
        // Test validation with empty path (should fail gracefully)
        bool emptyPathResult = g_tensorflowImplementation->validateModel("");
        
        if (emptyPathResult) {
            return TestResult(false, "Model Validation Test", millis() - startTime,
                            "Validation should fail for empty path");
        }
        
        uint32_t duration = millis() - startTime;
        return TestResult(true, "Model Validation Test", duration);
    }
    
    /**
     * Test basic inference functionality
     */
    TestResult testBasicInference() {
        uint32_t startTime = millis();
        
        // Create test image data
        size_t imageSize = TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT * TEST_IMAGE_CHANNELS;
        uint8_t* testImage = (uint8_t*)malloc(imageSize);
        
        if (!testImage) {
            return TestResult(false, "Basic Inference Test", millis() - startTime,
                            "Failed to allocate test image data");
        }
        
        // Fill with pattern data
        for (size_t i = 0; i < imageSize; i++) {
            testImage[i] = (i % 256);
        }
        
        // Test inference (will use simplified classification if no models loaded)
        WildlifeClassifier::ClassificationResult result = classifier.classifyImage(
            testImage, imageSize, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        
        free(testImage);
        
        uint32_t duration = millis() - startTime;
        
        // Check if we got a valid result (even from simplified classification)
        if (result.inferenceTime == 0) {
            return TestResult(false, "Basic Inference Test", duration,
                            "Inference time should be non-zero");
        }
        
        if (result.speciesName.length() == 0) {
            return TestResult(false, "Basic Inference Test", duration,
                            "Species name should not be empty");
        }
        
        return TestResult(true, "Basic Inference Test", duration);
    }
    
    /**
     * Test species classification functionality
     */
    TestResult testSpeciesClassification() {
        uint32_t startTime = millis();
        
        // Test species name mapping
        String deerName = WildlifeClassifier::getSpeciesName(WildlifeClassifier::SpeciesType::WHITE_TAILED_DEER);
        if (deerName != "White-tailed Deer") {
            return TestResult(false, "Species Classification Test", millis() - startTime,
                            "Incorrect species name mapping for deer");
        }
        
        String bearName = WildlifeClassifier::getSpeciesName(WildlifeClassifier::SpeciesType::BLACK_BEAR);
        if (bearName != "Black Bear") {
            return TestResult(false, "Species Classification Test", millis() - startTime,
                            "Incorrect species name mapping for bear");
        }
        
        String unknownName = WildlifeClassifier::getSpeciesName(WildlifeClassifier::SpeciesType::UNKNOWN);
        if (unknownName != "Unknown Species") {
            return TestResult(false, "Species Classification Test", millis() - startTime,
                            "Incorrect species name mapping for unknown");
        }
        
        uint32_t duration = millis() - startTime;
        return TestResult(true, "Species Classification Test", duration);
    }
    
    /**
     * Test dangerous species detection
     */
    TestResult testDangerousSpeciesDetection() {
        uint32_t startTime = millis();
        
        // Test dangerous species identification
        bool bearIsDangerous = WildlifeClassifier::isDangerousSpecies(WildlifeClassifier::SpeciesType::BLACK_BEAR);
        bool wolfIsDangerous = WildlifeClassifier::isDangerousSpecies(WildlifeClassifier::SpeciesType::GRAY_WOLF);
        bool lionIsDangerous = WildlifeClassifier::isDangerousSpecies(WildlifeClassifier::SpeciesType::MOUNTAIN_LION);
        
        if (!bearIsDangerous || !wolfIsDangerous || !lionIsDangerous) {
            return TestResult(false, "Dangerous Species Detection Test", millis() - startTime,
                            "Known dangerous species not correctly identified");
        }
        
        // Test non-dangerous species
        bool deerIsDangerous = WildlifeClassifier::isDangerousSpecies(WildlifeClassifier::SpeciesType::WHITE_TAILED_DEER);
        bool turkeyIsDangerous = WildlifeClassifier::isDangerousSpecies(WildlifeClassifier::SpeciesType::WILD_TURKEY);
        
        if (deerIsDangerous || turkeyIsDangerous) {
            return TestResult(false, "Dangerous Species Detection Test", millis() - startTime,
                            "Non-dangerous species incorrectly identified as dangerous");
        }
        
        uint32_t duration = millis() - startTime;
        return TestResult(true, "Dangerous Species Detection Test", duration);
    }
    
    /**
     * Test performance benchmarking
     */
    TestResult testPerformanceBenchmark() {
        uint32_t startTime = millis();
        
        if (!g_tensorflowImplementation) {
            return TestResult(false, "Performance Benchmark Test", millis() - startTime,
                            "TensorFlow Lite not initialized");
        }
        
        // Enable performance monitoring
        g_tensorflowImplementation->enablePerformanceMonitoring(true);
        
        // Run multiple inferences to test performance tracking
        size_t imageSize = TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT * TEST_IMAGE_CHANNELS;
        uint8_t* testImage = (uint8_t*)malloc(imageSize);
        
        if (!testImage) {
            return TestResult(false, "Performance Benchmark Test", millis() - startTime,
                            "Failed to allocate test image for benchmarking");
        }
        
        // Fill with pattern data
        for (size_t i = 0; i < imageSize; i++) {
            testImage[i] = (i % 256);
        }
        
        uint32_t totalTime = 0;
        for (int i = 0; i < TEST_ITERATIONS; i++) {
            uint32_t iterStart = millis();
            WildlifeClassifier::ClassificationResult result = classifier.classifyImage(
                testImage, imageSize, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
            totalTime += (millis() - iterStart);
        }
        
        free(testImage);
        
        uint32_t avgTime = totalTime / TEST_ITERATIONS;
        uint32_t duration = millis() - startTime;
        
        // Check if average time is reasonable (should be under 5 seconds per inference)
        if (avgTime > 5000) {
            return TestResult(false, "Performance Benchmark Test", duration,
                            "Average inference time too high: " + String(avgTime) + "ms");
        }
        
        return TestResult(true, "Performance Benchmark Test", duration);
    }
    
    /**
     * Test memory optimization integration
     */
    TestResult testMemoryOptimization() {
        uint32_t startTime = millis();
        
        size_t initialFreeHeap = ESP.getFreeHeap();
        
        // Simulate memory-intensive AI operations
        for (int i = 0; i < 3; i++) {
            size_t testSize = 100 * 1024; // 100KB allocations
            void* testBuffer = malloc(testSize);
            
            if (testBuffer) {
                memset(testBuffer, 0x55, testSize);
                free(testBuffer);
            }
        }
        
        size_t finalFreeHeap = ESP.getFreeHeap();
        uint32_t duration = millis() - startTime;
        
        // Check for significant memory leaks
        int32_t memoryDifference = initialFreeHeap - finalFreeHeap;
        
        if (memoryDifference > 50 * 1024) { // Allow 50KB tolerance
            return TestResult(false, "Memory Optimization Test", duration,
                            "Potential memory leak detected: " + String(memoryDifference) + " bytes");
        }
        
        return TestResult(true, "Memory Optimization Test", duration);
    }
    
    /**
     * Test fragmentation handling
     */
    TestResult testFragmentationHandling() {
        uint32_t startTime = millis();
        
        // Create fragmented memory pattern
        void* fragments[10];
        size_t fragmentSize = 10 * 1024; // 10KB fragments
        
        // Allocate fragments
        for (int i = 0; i < 10; i++) {
            fragments[i] = malloc(fragmentSize);
            if (fragments[i]) {
                memset(fragments[i], 0x77, fragmentSize);
            }
        }
        
        // Free every other fragment to create fragmentation
        for (int i = 1; i < 10; i += 2) {
            if (fragments[i]) {
                free(fragments[i]);
                fragments[i] = NULL;
            }
        }
        
        // Try to allocate larger block
        size_t largeSize = 50 * 1024; // 50KB
        void* largeBuffer = malloc(largeSize);
        
        // Clean up remaining fragments
        for (int i = 0; i < 10; i += 2) {
            if (fragments[i]) {
                free(fragments[i]);
            }
        }
        
        if (largeBuffer) {
            free(largeBuffer);
        }
        
        uint32_t duration = millis() - startTime;
        
        // The test passes if we don't crash during fragmentation
        return TestResult(true, "Fragmentation Handling Test", duration);
    }
    
    /**
     * Print test summary
     */
    void printTestSummary() {
        Serial.println("=== Test Summary ===");
        Serial.printf("Total tests: %d\n", testCount);
        Serial.printf("Passed: %d\n", passedTests);
        Serial.printf("Failed: %d\n", testCount - passedTests);
        Serial.printf("Success rate: %.1f%%\n", (float)passedTests / testCount * 100);
        
        if (passedTests == testCount) {
            Serial.println("🎉 ALL TESTS PASSED! AI system is ready.");
        } else {
            Serial.println("⚠️  Some tests failed. Check the output above for details.");
        }
        
        Serial.println();
        
        // Print memory status
        Serial.println("=== Memory Status ===");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        if (psramFound()) {
            Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        }
        Serial.printf("Minimum free heap: %d bytes\n", ESP.getMinFreeHeap());
        
        Serial.println("\n=== AI Wildlife Detection Validation Complete ===");
    }
};

// Global test instance
AIValidationTest aiTest;

void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial monitor
    
    // Initialize and run tests
    if (aiTest.initialize()) {
        aiTest.runAllTests();
    } else {
        Serial.println("ERROR: Failed to initialize test system");
    }
}

void loop() {
    // Tests run only once in setup()
    delay(10000);
    Serial.println("Tests completed. Reboot to run again.");
}