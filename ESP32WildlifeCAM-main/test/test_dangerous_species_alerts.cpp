/**
 * @file test_dangerous_species_alerts.cpp
 * @brief Test suite for dangerous species alert system
 * 
 * Validates the functionality of the dangerous species detection
 * and alert system including priority levels and alert generation.
 */

#include <Arduino.h>
#include "../src/ai/dangerous_species_alert.h"
#include "../src/ai/wildlife_classifier.h"

// Test configuration
#define TEST_TIMEOUT 30000  // 30 seconds

/**
 * Test result structure
 */
struct TestResult {
    bool passed;
    String testName;
    uint32_t executionTime;
    String errorMessage;
    
    TestResult(bool p = false, String name = "", uint32_t time = 0, String error = "")
        : passed(p), testName(name), executionTime(time), errorMessage(error) {}
};

/**
 * Test dangerous species alert system
 */
class DangerousSpeciesAlertTest {
private:
    DangerousSpeciesAlertSystem alertSystem;
    WildlifeClassifier classifier;
    uint32_t testCount = 0;
    uint32_t passedTests = 0;
    
public:
    /**
     * Initialize test system
     */
    bool initialize() {
        Serial.println("=== Dangerous Species Alert System Tests ===");
        Serial.println();
        
        // Initialize alert system
        if (!alertSystem.initialize()) {
            Serial.println("ERROR: Failed to initialize alert system");
            return false;
        }
        
        Serial.println("Alert system initialized successfully");
        return true;
    }
    
    /**
     * Run all tests
     */
    void runAllTests() {
        Serial.println("Starting dangerous species alert tests...\n");
        
        // Core functionality tests
        runTest(testAlertSystemInitialization());
        runTest(testDangerousSpeciesDetection());
        runTest(testAlertPriorityLevels());
        runTest(testConsecutiveDetections());
        runTest(testAlertCooldown());
        runTest(testConfidenceThreshold());
        
        // Alert generation tests
        runTest(testCriticalAlertGeneration());
        runTest(testHighPriorityAlert());
        runTest(testLowPriorityAlert());
        
        // Integration tests
        runTest(testMultipleSpeciesAlerts());
        runTest(testAlertMessageFormat());
        
        // Print summary
        printTestSummary();
    }
    
private:
    /**
     * Run individual test
     */
    void runTest(TestResult result) {
        testCount++;
        Serial.printf("Test %d: %s\n", testCount, result.testName.c_str());
        
        if (result.passed) {
            Serial.printf("  ✅ PASSED (%d ms)\n", result.executionTime);
            passedTests++;
        } else {
            Serial.printf("  ❌ FAILED: %s\n", result.errorMessage.c_str());
        }
        Serial.println();
    }
    
    /**
     * Test: Alert system initialization
     */
    TestResult testAlertSystemInitialization() {
        uint32_t startTime = millis();
        
        DangerousSpeciesAlertSystem testSystem;
        if (!testSystem.initialize()) {
            return TestResult(false, "Alert System Initialization", millis() - startTime,
                            "Failed to initialize alert system");
        }
        
        if (!testSystem.isEnabled()) {
            return TestResult(false, "Alert System Initialization", millis() - startTime,
                            "Alert system not enabled after initialization");
        }
        
        return TestResult(true, "Alert System Initialization", millis() - startTime);
    }
    
    /**
     * Test: Dangerous species detection
     */
    TestResult testDangerousSpeciesDetection() {
        uint32_t startTime = millis();
        
        // Test that dangerous species are correctly identified
        bool bearDangerous = WildlifeClassifier::isDangerousSpecies(
            WildlifeClassifier::SpeciesType::BLACK_BEAR);
        bool wolfDangerous = WildlifeClassifier::isDangerousSpecies(
            WildlifeClassifier::SpeciesType::GRAY_WOLF);
        bool lionDangerous = WildlifeClassifier::isDangerousSpecies(
            WildlifeClassifier::SpeciesType::MOUNTAIN_LION);
        
        if (!bearDangerous || !wolfDangerous || !lionDangerous) {
            return TestResult(false, "Dangerous Species Detection", millis() - startTime,
                            "Known dangerous species not identified correctly");
        }
        
        // Test that non-dangerous species are correctly identified
        bool deerDangerous = WildlifeClassifier::isDangerousSpecies(
            WildlifeClassifier::SpeciesType::WHITE_TAILED_DEER);
        bool rabbitDangerous = WildlifeClassifier::isDangerousSpecies(
            WildlifeClassifier::SpeciesType::RABBIT);
        
        if (deerDangerous || rabbitDangerous) {
            return TestResult(false, "Dangerous Species Detection", millis() - startTime,
                            "Non-dangerous species incorrectly marked as dangerous");
        }
        
        return TestResult(true, "Dangerous Species Detection", millis() - startTime);
    }
    
    /**
     * Test: Alert priority levels
     */
    TestResult testAlertPriorityLevels() {
        uint32_t startTime = millis();
        
        // Test that priorities are assigned correctly
        AlertPriority bearPriority = DangerousSpeciesAlertSystem::getAlertPriority(
            WildlifeClassifier::SpeciesType::BLACK_BEAR);
        
        if (bearPriority != AlertPriority::CRITICAL) {
            return TestResult(false, "Alert Priority Levels", millis() - startTime,
                            "Bear should have CRITICAL priority");
        }
        
        AlertPriority wolfPriority = DangerousSpeciesAlertSystem::getAlertPriority(
            WildlifeClassifier::SpeciesType::GRAY_WOLF);
        
        if (wolfPriority != AlertPriority::HIGH) {
            return TestResult(false, "Alert Priority Levels", millis() - startTime,
                            "Wolf should have HIGH priority");
        }
        
        AlertPriority coyotePriority = DangerousSpeciesAlertSystem::getAlertPriority(
            WildlifeClassifier::SpeciesType::COYOTE);
        
        if (coyotePriority == AlertPriority::CRITICAL) {
            return TestResult(false, "Alert Priority Levels", millis() - startTime,
                            "Coyote should not have CRITICAL priority");
        }
        
        return TestResult(true, "Alert Priority Levels", millis() - startTime);
    }
    
    /**
     * Test: Consecutive detections requirement
     */
    TestResult testConsecutiveDetections() {
        uint32_t startTime = millis();
        
        DangerousSpeciesAlertSystem testSystem;
        testSystem.initialize();
        testSystem.setMinConsecutiveDetections(2);
        
        // Create a dangerous species detection
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::BLACK_BEAR;
        result.speciesName = "Black Bear";
        result.confidence = 0.85f;
        result.isValid = true;
        
        // First detection - should not trigger alert
        DangerousSpeciesAlert* alert1 = testSystem.processClassification(result);
        if (alert1 != nullptr) {
            return TestResult(false, "Consecutive Detections", millis() - startTime,
                            "Alert triggered on first detection (should require 2)");
        }
        
        // Second consecutive detection - should trigger alert
        DangerousSpeciesAlert* alert2 = testSystem.processClassification(result);
        if (alert2 == nullptr) {
            return TestResult(false, "Consecutive Detections", millis() - startTime,
                            "Alert not triggered after 2 consecutive detections");
        }
        
        return TestResult(true, "Consecutive Detections", millis() - startTime);
    }
    
    /**
     * Test: Alert cooldown mechanism
     */
    TestResult testAlertCooldown() {
        uint32_t startTime = millis();
        
        DangerousSpeciesAlertSystem testSystem;
        testSystem.initialize();
        testSystem.setMinConsecutiveDetections(1);
        
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::BLACK_BEAR;
        result.speciesName = "Black Bear";
        result.confidence = 0.85f;
        result.isValid = true;
        
        // First alert
        DangerousSpeciesAlert* alert1 = testSystem.processClassification(result);
        if (alert1 == nullptr) {
            return TestResult(false, "Alert Cooldown", millis() - startTime,
                            "First alert not generated");
        }
        
        // Immediate second detection - should be in cooldown
        DangerousSpeciesAlert* alert2 = testSystem.processClassification(result);
        if (alert2 != nullptr) {
            return TestResult(false, "Alert Cooldown", millis() - startTime,
                            "Alert generated during cooldown period");
        }
        
        return TestResult(true, "Alert Cooldown", millis() - startTime);
    }
    
    /**
     * Test: Confidence threshold
     */
    TestResult testConfidenceThreshold() {
        uint32_t startTime = millis();
        
        DangerousSpeciesAlertSystem testSystem;
        testSystem.initialize();
        testSystem.setConfidenceThreshold(0.75f);
        testSystem.setMinConsecutiveDetections(1);
        
        // Low confidence detection
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::BLACK_BEAR;
        result.speciesName = "Black Bear";
        result.confidence = 0.65f;  // Below threshold
        result.isValid = true;
        
        DangerousSpeciesAlert* alert = testSystem.processClassification(result);
        if (alert != nullptr) {
            return TestResult(false, "Confidence Threshold", millis() - startTime,
                            "Alert generated with confidence below threshold");
        }
        
        return TestResult(true, "Confidence Threshold", millis() - startTime);
    }
    
    /**
     * Test: Critical alert generation
     */
    TestResult testCriticalAlertGeneration() {
        uint32_t startTime = millis();
        
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::BLACK_BEAR;
        result.speciesName = "Black Bear";
        result.confidence = 0.92f;
        result.isValid = true;
        
        alertSystem.clearAlertHistory();
        alertSystem.setMinConsecutiveDetections(1);
        
        DangerousSpeciesAlert* alert = alertSystem.processClassification(result);
        
        if (alert == nullptr) {
            return TestResult(false, "Critical Alert Generation", millis() - startTime,
                            "Failed to generate critical alert for bear");
        }
        
        if (alert->priority != AlertPriority::CRITICAL) {
            return TestResult(false, "Critical Alert Generation", millis() - startTime,
                            "Bear alert should have CRITICAL priority");
        }
        
        if (!alert->requiresImmediate) {
            return TestResult(false, "Critical Alert Generation", millis() - startTime,
                            "High confidence bear detection should require immediate alert");
        }
        
        return TestResult(true, "Critical Alert Generation", millis() - startTime);
    }
    
    /**
     * Test: High priority alert
     */
    TestResult testHighPriorityAlert() {
        uint32_t startTime = millis();
        
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::GRAY_WOLF;
        result.speciesName = "Gray Wolf";
        result.confidence = 0.88f;
        result.isValid = true;
        
        alertSystem.clearAlertHistory();
        alertSystem.setMinConsecutiveDetections(1);
        
        DangerousSpeciesAlert* alert = alertSystem.processClassification(result);
        
        if (alert == nullptr) {
            return TestResult(false, "High Priority Alert", millis() - startTime,
                            "Failed to generate alert for wolf");
        }
        
        if (alert->priority != AlertPriority::HIGH) {
            return TestResult(false, "High Priority Alert", millis() - startTime,
                            "Wolf alert should have HIGH priority");
        }
        
        return TestResult(true, "High Priority Alert", millis() - startTime);
    }
    
    /**
     * Test: Low priority alert
     */
    TestResult testLowPriorityAlert() {
        uint32_t startTime = millis();
        
        WildlifeClassifier::ClassificationResult result;
        result.species = WildlifeClassifier::SpeciesType::COYOTE;
        result.speciesName = "Coyote";
        result.confidence = 0.78f;
        result.isValid = true;
        
        alertSystem.clearAlertHistory();
        alertSystem.setMinConsecutiveDetections(1);
        
        DangerousSpeciesAlert* alert = alertSystem.processClassification(result);
        
        if (alert == nullptr) {
            return TestResult(false, "Low Priority Alert", millis() - startTime,
                            "Failed to generate alert for coyote");
        }
        
        if (alert->priority == AlertPriority::CRITICAL) {
            return TestResult(false, "Low Priority Alert", millis() - startTime,
                            "Coyote alert should not have CRITICAL priority");
        }
        
        return TestResult(true, "Low Priority Alert", millis() - startTime);
    }
    
    /**
     * Test: Multiple species alerts
     */
    TestResult testMultipleSpeciesAlerts() {
        uint32_t startTime = millis();
        
        alertSystem.clearAlertHistory();
        alertSystem.setMinConsecutiveDetections(1);
        
        // Detect bear
        WildlifeClassifier::ClassificationResult bearResult;
        bearResult.species = WildlifeClassifier::SpeciesType::BLACK_BEAR;
        bearResult.speciesName = "Black Bear";
        bearResult.confidence = 0.85f;
        bearResult.isValid = true;
        
        DangerousSpeciesAlert* bearAlert = alertSystem.processClassification(bearResult);
        if (bearAlert == nullptr) {
            return TestResult(false, "Multiple Species Alerts", millis() - startTime,
                            "Failed to generate bear alert");
        }
        
        uint32_t totalAlerts = alertSystem.getTotalAlerts();
        uint32_t criticalAlerts = alertSystem.getCriticalAlerts();
        
        if (totalAlerts != 1 || criticalAlerts != 1) {
            return TestResult(false, "Multiple Species Alerts", millis() - startTime,
                            "Incorrect alert statistics");
        }
        
        return TestResult(true, "Multiple Species Alerts", millis() - startTime);
    }
    
    /**
     * Test: Alert message format
     */
    TestResult testAlertMessageFormat() {
        uint32_t startTime = millis();
        
        String message = DangerousSpeciesAlertSystem::getAlertMessage(
            WildlifeClassifier::SpeciesType::BLACK_BEAR, 0.92f);
        
        if (message.length() == 0) {
            return TestResult(false, "Alert Message Format", millis() - startTime,
                            "Alert message is empty");
        }
        
        if (message.indexOf("Black Bear") < 0) {
            return TestResult(false, "Alert Message Format", millis() - startTime,
                            "Alert message missing species name");
        }
        
        if (message.indexOf("CRITICAL") < 0 && message.indexOf("HIGH") < 0) {
            return TestResult(false, "Alert Message Format", millis() - startTime,
                            "Alert message missing priority indicator");
        }
        
        return TestResult(true, "Alert Message Format", millis() - startTime);
    }
    
    /**
     * Print test summary
     */
    void printTestSummary() {
        Serial.println("================================");
        Serial.println("Test Summary");
        Serial.println("================================");
        Serial.printf("Total Tests: %d\n", testCount);
        Serial.printf("Passed: %d\n", passedTests);
        Serial.printf("Failed: %d\n", testCount - passedTests);
        Serial.printf("Success Rate: %.1f%%\n", (passedTests * 100.0f) / testCount);
        Serial.println("================================");
        
        if (passedTests == testCount) {
            Serial.println("✅ ALL TESTS PASSED!");
        } else {
            Serial.println("❌ SOME TESTS FAILED");
        }
    }
};

// Test entry point
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    delay(2000); // Wait for serial monitor
    
    DangerousSpeciesAlertTest test;
    if (test.initialize()) {
        test.runAllTests();
    } else {
        Serial.println("ERROR: Failed to initialize test system");
    }
}

void loop() {
    // Tests run once in setup()
    delay(1000);
}
