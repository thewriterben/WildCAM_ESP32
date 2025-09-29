/**
 * @file test_motion_enhancements.cpp
 * @brief Test suite for motion detection enhancements
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include <Arduino.h>
#include "../src/detection/adaptive_processor.h"
#include "../src/detection/wildlife_analyzer.h"
#include "../src/detection/motion_coordinator.h"

// Test results tracking
struct TestResults {
    uint32_t totalTests = 0;
    uint32_t passedTests = 0;
    uint32_t failedTests = 0;
};

TestResults g_testResults;

// Test helper macros
#define TEST_ASSERT(condition, testName) \
    do { \
        g_testResults.totalTests++; \
        if (condition) { \
            g_testResults.passedTests++; \
            Serial.printf("✓ PASS: %s\n", testName); \
        } else { \
            g_testResults.failedTests++; \
            Serial.printf("✗ FAIL: %s\n", testName); \
        } \
    } while(0)

#define TEST_ASSERT_NEAR(actual, expected, tolerance, testName) \
    TEST_ASSERT(abs((actual) - (expected)) <= (tolerance), testName)

// Test functions
void testAdaptiveProcessor() {
    Serial.println("\n=== Testing Adaptive Processor ===");
    
    AdaptiveProcessor processor;
    
    // Test initialization
    TEST_ASSERT(processor.initialize(), "AdaptiveProcessor initialization");
    
    // Test processing decision
    auto decision = processor.getProcessingDecision(3.7f, 20.0f, 0.5f);
    TEST_ASSERT(decision.level != AdaptiveProcessor::ProcessingLevel::MINIMAL, "Processing level not minimal with good conditions");
    
    // Test activity tracking
    processor.updateActivity(true, 0.8f, 300);
    processor.updateActivity(true, 0.7f, 250);
    processor.updateActivity(false, 0.0f, 200);
    
    auto activityLevel = processor.getCurrentActivityLevel();
    TEST_ASSERT(activityLevel != AdaptiveProcessor::ActivityLevel::DORMANT, "Activity level should not be dormant after detections");
    
    // Test ROI update
    processor.updateROI(150, 120, 50, 40, 0.8f);
    auto roi = processor.getCurrentROI();
    TEST_ASSERT(roi.confidence > 0.0f, "ROI confidence should increase after update");
    
    // Test configuration JSON
    String config = processor.getConfigJSON();
    TEST_ASSERT(config.length() > 10, "Configuration JSON should be generated");
    TEST_ASSERT(processor.loadConfigJSON(config), "Configuration JSON should load successfully");
    
    Serial.printf("Adaptive Processor Tests: %d/%d passed\n", 
                  g_testResults.passedTests - (g_testResults.totalTests - 7), 7);
}

void testWildlifeAnalyzer() {
    Serial.println("\n=== Testing Wildlife Analyzer ===");
    
    WildlifeAnalyzer analyzer;
    
    // Test initialization
    TEST_ASSERT(analyzer.initialize(), "WildlifeAnalyzer initialization");
    
    // Test motion data addition
    WildlifeAnalyzer::MotionDataPoint dataPoint;
    dataPoint.timestamp = millis();
    dataPoint.x = 160;
    dataPoint.y = 120;
    dataPoint.width = 50;
    dataPoint.height = 40;
    dataPoint.intensity = 0.8f;
    dataPoint.confidence = 0.7f;
    
    analyzer.addMotionData(dataPoint);
    
    // Add more data points to simulate movement
    for (int i = 0; i < 10; i++) {
        dataPoint.timestamp = millis() + i * 100;
        dataPoint.x += 5; // Moving right
        dataPoint.y += 2; // Moving down slightly
        analyzer.addMotionData(dataPoint);
    }
    
    // Test pattern analysis
    auto result = analyzer.analyzePattern(12, 20.0f, 0.5f);
    TEST_ASSERT(result.primaryPattern != WildlifeAnalyzer::MovementPattern::UNKNOWN, "Pattern should be classified");
    TEST_ASSERT(result.confidence > 0.0f, "Analysis should have confidence > 0");
    
    // Test quick analysis
    std::vector<WildlifeAnalyzer::MotionDataPoint> recentData;
    recentData.push_back(dataPoint);
    dataPoint.x += 10;
    recentData.push_back(dataPoint);
    
    auto quickResult = analyzer.quickAnalysis(recentData);
    TEST_ASSERT(quickResult.primaryPattern != WildlifeAnalyzer::MovementPattern::UNKNOWN, "Quick analysis should classify pattern");
    
    // Test learning
    WildlifeAnalyzer::MovementCharacteristics characteristics;
    characteristics.speed = 15.0f;
    characteristics.size = 0.2f;
    characteristics.direction = 0.5f;
    
    analyzer.learnPattern(WildlifeAnalyzer::MovementPattern::MEDIUM_MAMMAL, characteristics, 0.9f);
    
    // Test configuration
    String config = analyzer.getConfigJSON();
    TEST_ASSERT(config.length() > 10, "Configuration JSON should be generated");
    TEST_ASSERT(analyzer.loadConfigJSON(config), "Configuration JSON should load successfully");
    
    // Test statistics
    String stats = analyzer.getAnalysisStats();
    TEST_ASSERT(stats.length() > 10, "Statistics should be generated");
    
    Serial.printf("Wildlife Analyzer Tests: %d/%d passed\n", 
                  g_testResults.passedTests - (g_testResults.totalTests - 14), 7);
}

void testMotionCoordinator() {
    Serial.println("\n=== Testing Motion Coordinator ===");
    
    MotionCoordinator coordinator;
    
    // Test initialization (without camera manager for testing)
    TEST_ASSERT(coordinator.initialize(nullptr), "MotionCoordinator initialization");
    
    // Test detection method setting
    coordinator.setDetectionMethod(MotionCoordinator::DetectionMethod::ADAPTIVE);
    TEST_ASSERT(coordinator.getCurrentMethod() == MotionCoordinator::DetectionMethod::ADAPTIVE, 
                "Detection method should be set to adaptive");
    
    // Test environmental conditions update
    MotionCoordinator::EnvironmentalConditions conditions;
    conditions.batteryVoltage = 3.8f;
    conditions.temperature = 25.0f;
    conditions.lightLevel = 0.6f;
    conditions.currentHour = 14;
    
    coordinator.updateEnvironmentalConditions(conditions);
    
    // Test motion detection (this will use fallback behavior without full system)
    auto result = coordinator.detectMotion(nullptr, conditions);
    TEST_ASSERT(result.timestamp > 0, "Detection result should have timestamp");
    
    // Test configuration
    String config = coordinator.getConfigJSON();
    TEST_ASSERT(config.length() > 10, "Configuration JSON should be generated");
    TEST_ASSERT(coordinator.loadConfigJSON(config), "Configuration JSON should load successfully");
    
    // Test statistics
    auto stats = coordinator.getStatistics();
    TEST_ASSERT(stats.lastResetTime > 0, "Statistics should have reset time");
    
    // Test status report
    String status = coordinator.getStatusReport();
    TEST_ASSERT(status.length() > 10, "Status report should be generated");
    
    // Test feature toggles
    coordinator.setWildlifeAnalysisEnabled(false);
    coordinator.setWildlifeAnalysisEnabled(true);
    
    coordinator.setPerformanceOptimizationEnabled(false);
    coordinator.setPerformanceOptimizationEnabled(true);
    
    Serial.printf("Motion Coordinator Tests: %d/%d passed\n", 
                  g_testResults.passedTests - (g_testResults.totalTests - 21), 7);
}

void testJSONConfiguration() {
    Serial.println("\n=== Testing JSON Configuration ===");
    
    // Test loading configuration from file system
    // This is a simplified test since we don't have full file system access
    
    String testConfig = R"({
        "enabled": true,
        "activityWindowMinutes": 120,
        "batteryLowThreshold": 3.1
    })";
    
    AdaptiveProcessor processor;
    processor.initialize();
    
    TEST_ASSERT(processor.loadConfigJSON(testConfig), "JSON configuration should load");
    
    String retrievedConfig = processor.getConfigJSON();
    TEST_ASSERT(retrievedConfig.indexOf("enabled") > 0, "Retrieved config should contain enabled field");
    
    Serial.printf("JSON Configuration Tests: %d/%d passed\n", 
                  g_testResults.passedTests - (g_testResults.totalTests - 28), 2);
}

void testIntegration() {
    Serial.println("\n=== Testing Integration ===");
    
    // Test that all components can work together
    AdaptiveProcessor processor;
    WildlifeAnalyzer analyzer;
    MotionCoordinator coordinator;
    
    TEST_ASSERT(processor.initialize(), "Processor initialization in integration test");
    TEST_ASSERT(analyzer.initialize(), "Analyzer initialization in integration test");
    TEST_ASSERT(coordinator.initialize(nullptr), "Coordinator initialization in integration test");
    
    // Simulate a detection workflow
    MotionCoordinator::EnvironmentalConditions conditions;
    conditions.batteryVoltage = 3.7f;
    conditions.temperature = 20.0f;
    conditions.lightLevel = 0.5f;
    conditions.currentHour = 18; // Evening
    
    // Test environmental updates
    processor.updateActivity(true, 0.8f, 300);
    
    WildlifeAnalyzer::MotionDataPoint dataPoint;
    dataPoint.timestamp = millis();
    dataPoint.x = 160;
    dataPoint.y = 120;
    dataPoint.width = 60;
    dataPoint.height = 45;
    dataPoint.intensity = 0.8f;
    dataPoint.confidence = 0.7f;
    analyzer.addMotionData(dataPoint);
    
    auto result = coordinator.detectMotion(nullptr, conditions);
    TEST_ASSERT(result.timestamp > 0, "Integrated detection should produce result");
    
    Serial.printf("Integration Tests: %d/%d passed\n", 
                  g_testResults.passedTests - (g_testResults.totalTests - 31), 4);
}

void runAllTests() {
    Serial.println("\n======================================");
    Serial.println("Motion Detection Enhancement Test Suite");
    Serial.println("======================================");
    
    // Reset test results
    g_testResults = TestResults{};
    
    // Run all test suites
    testAdaptiveProcessor();
    testWildlifeAnalyzer();
    testMotionCoordinator();
    testJSONConfiguration();
    testIntegration();
    
    // Print overall results
    Serial.println("\n======================================");
    Serial.println("           TEST SUMMARY");
    Serial.println("======================================");
    Serial.printf("Total Tests:  %d\n", g_testResults.totalTests);
    Serial.printf("Passed:       %d\n", g_testResults.passedTests);
    Serial.printf("Failed:       %d\n", g_testResults.failedTests);
    Serial.printf("Success Rate: %.1f%%\n", 
                  (float)g_testResults.passedTests / g_testResults.totalTests * 100.0f);
    
    if (g_testResults.failedTests == 0) {
        Serial.println("🎉 ALL TESTS PASSED! 🎉");
    } else {
        Serial.println("⚠️  Some tests failed - check implementation");
    }
    Serial.println("======================================");
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for serial connection
    
    Serial.println("ESP32WildlifeCAM Motion Detection Enhancement Test");
    Serial.println("Starting test suite...");
    
    runAllTests();
}

void loop() {
    // Test complete - just wait
    delay(10000);
}