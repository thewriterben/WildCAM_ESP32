/**
 * @file servo_system_test.cpp
 * @brief Test Suite for Servo Control System
 * 
 * Tests the pan/tilt servo system integration with the wildlife camera.
 */

#include "../src/servo/servo_controller.h"
#include "../src/servo/pan_tilt_manager.h"
#include "../src/servo/servo_integration.h"
#include "../include/config.h"
#include <Arduino.h>

// Test configuration
const bool RUN_HARDWARE_TESTS = false; // Set to true only if servos are connected

/**
 * Test Results Structure
 */
struct TestResults {
    uint32_t tests_run = 0;
    uint32_t tests_passed = 0;
    uint32_t tests_failed = 0;
    
    void recordTest(bool passed) {
        tests_run++;
        if (passed) {
            tests_passed++;
        } else {
            tests_failed++;
        }
    }
    
    void printSummary() {
        Serial.printf("\n=== Test Summary ===\n");
        Serial.printf("Tests Run: %d\n", tests_run);
        Serial.printf("Passed: %d\n", tests_passed);
        Serial.printf("Failed: %d\n", tests_failed);
        Serial.printf("Success Rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0f);
    }
};

TestResults test_results;

/**
 * Test servo configuration creation
 */
bool testServoConfigCreation() {
    Serial.println("Testing servo configuration creation...");
    
    // Test pan configuration
    ServoConfig pan_config = ServoUtils::createPanConfig(16);
    bool pan_valid = (pan_config.pin == 16 && 
                     pan_config.min_angle == 0.0f && 
                     pan_config.max_angle == 180.0f &&
                     pan_config.center_angle == 90.0f);
    
    // Test tilt configuration
    ServoConfig tilt_config = ServoUtils::createTiltConfig(17);
    bool tilt_valid = (tilt_config.pin == 17 && 
                      tilt_config.min_angle == 45.0f && 
                      tilt_config.max_angle == 135.0f &&
                      tilt_config.center_angle == 90.0f);
    
    // Test angle validation
    bool angle_validation = ServoUtils::isValidAngle(90.0f, pan_config) &&
                           !ServoUtils::isValidAngle(200.0f, pan_config) &&
                           ServoUtils::isValidAngle(90.0f, tilt_config) &&
                           !ServoUtils::isValidAngle(30.0f, tilt_config);
    
    bool passed = pan_valid && tilt_valid && angle_validation;
    Serial.printf("Servo config test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test servo controller creation (without hardware)
 */
bool testServoControllerCreation() {
    Serial.println("Testing servo controller creation...");
    
    ServoConfig config = ServoUtils::createPanConfig(16);
    ServoController controller(config);
    
    // Test initial state
    ServoStatus status = controller.getStatus();
    bool initial_state = (!status.initialized && 
                         !status.attached && 
                         !status.moving &&
                         status.current_angle == 90.0f);
    
    // Test configuration
    ServoConfig retrieved_config = controller.getConfig();
    bool config_match = (retrieved_config.pin == config.pin &&
                        retrieved_config.center_angle == config.center_angle);
    
    bool passed = initial_state && config_match;
    Serial.printf("Servo controller test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test pan/tilt position utilities
 */
bool testPanTiltPositions() {
    Serial.println("Testing pan/tilt position utilities...");
    
    // Test position creation
    PanTiltPosition center = PanTiltUtils::createCenterPosition();
    bool center_valid = (center.pan_angle == 90.0f && center.tilt_angle == 90.0f);
    
    PanTiltPosition left = PanTiltUtils::createLookLeftPosition();
    bool left_valid = (left.pan_angle == 135.0f && left.tilt_angle == 90.0f);
    
    // Test motion to position conversion
    PanTiltPosition motion_pos = PanTiltUtils::motionToPosition(0.5f, 0.5f);
    bool motion_valid = (motion_pos.pan_angle == 90.0f && motion_pos.tilt_angle == 90.0f);
    
    // Test distance calculation
    float distance = PanTiltUtils::calculateDistance(center, left);
    bool distance_valid = (distance == 45.0f);
    
    // Test interpolation
    PanTiltPosition mid = PanTiltUtils::interpolatePosition(center, left, 0.5f);
    bool interpolation_valid = (mid.pan_angle == 112.5f && mid.tilt_angle == 90.0f);
    
    bool passed = center_valid && left_valid && motion_valid && distance_valid && interpolation_valid;
    Serial.printf("Pan/tilt position test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test pan/tilt manager creation (without hardware)
 */
bool testPanTiltManagerCreation() {
    Serial.println("Testing pan/tilt manager creation...");
    
    PanTiltManager manager;
    
    // Test initial state
    bool initial_state = (!manager.isMoving() && 
                         !manager.isScanning() && 
                         !manager.isTracking() &&
                         !manager.isPowerEnabled());
    
    // Test position functions
    PanTiltPosition current = manager.getCurrentPosition();
    bool position_valid = (current.pan_angle == 90.0f && current.tilt_angle == 90.0f);
    
    // Test scan positions
    manager.addScanPosition(PanTiltUtils::createCenterPosition());
    manager.addScanPosition(PanTiltUtils::createLookLeftPosition());
    bool scan_positions = (manager.getScanPositionsCount() == 2);
    
    manager.clearScanPositions();
    bool clear_positions = (manager.getScanPositionsCount() == 0);
    
    bool passed = initial_state && position_valid && scan_positions && clear_positions;
    Serial.printf("Pan/tilt manager test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test servo integration configuration
 */
bool testServoIntegrationConfig() {
    Serial.println("Testing servo integration configuration...");
    
    ServoIntegrationConfig config;
    
    // Test default configuration values
    bool defaults_valid = (config.servo_enabled == PAN_TILT_ENABLED &&
                          config.auto_tracking_enabled == WILDLIFE_TRACKING_ENABLED &&
                          config.auto_scanning_enabled == AUTO_SCANNING_ENABLED &&
                          config.tracking_threshold == TRACKING_MIN_CONFIDENCE);
    
    // Test configuration modification
    config.servo_enabled = false;
    config.tracking_threshold = 0.8f;
    bool modification_valid = (!config.servo_enabled && config.tracking_threshold == 0.8f);
    
    bool passed = defaults_valid && modification_valid;
    Serial.printf("Servo integration config test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test hardware initialization (only if hardware is available)
 */
bool testHardwareInitialization() {
    if (!RUN_HARDWARE_TESTS) {
        Serial.println("Skipping hardware tests (RUN_HARDWARE_TESTS = false)");
        return true; // Skip test but don't fail
    }
    
    Serial.println("Testing hardware initialization...");
    
    PanTiltManager manager;
    
    // Test initialization
    bool init_success = manager.initialize(PAN_SERVO_PIN, TILT_SERVO_PIN);
    if (!init_success) {
        Serial.printf("Hardware init test: FAIL (initialization failed)\n");
        return false;
    }
    
    // Test power state
    bool power_enabled = manager.isPowerEnabled();
    
    // Test movement (if servos are connected)
    bool move_success = manager.moveToHome(false);
    delay(1000);
    
    // Test position reading
    PanTiltPosition pos = manager.getCurrentPosition();
    bool position_reasonable = (pos.pan_angle >= 85.0f && pos.pan_angle <= 95.0f &&
                               pos.tilt_angle >= 85.0f && pos.tilt_angle <= 95.0f);
    
    // Cleanup
    manager.cleanup();
    
    bool passed = init_success && power_enabled && move_success && position_reasonable;
    Serial.printf("Hardware init test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test scanning pattern generation
 */
bool testScanPatternGeneration() {
    Serial.println("Testing scan pattern generation...");
    
    PanTiltManager manager;
    
    // Test horizontal sweep configuration
    ScanConfig horizontal_config;
    horizontal_config.pattern = ScanPattern::HORIZONTAL_SWEEP;
    horizontal_config.pan_step = 30.0f;
    
    // Test grid scan configuration
    ScanConfig grid_config;
    grid_config.pattern = ScanPattern::GRID_SCAN;
    grid_config.pan_step = 45.0f;
    grid_config.tilt_step = 30.0f;
    
    // Test configuration validity
    bool config_valid = (horizontal_config.pattern == ScanPattern::HORIZONTAL_SWEEP &&
                        grid_config.pattern == ScanPattern::GRID_SCAN);
    
    // Test custom positions
    manager.addScanPosition(PanTiltUtils::createCenterPosition());
    manager.addScanPosition(PanTiltUtils::createLookLeftPosition());
    manager.addScanPosition(PanTiltUtils::createLookRightPosition());
    
    bool custom_positions = (manager.getScanPositionsCount() == 3);
    
    bool passed = config_valid && custom_positions;
    Serial.printf("Scan pattern test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Test tracking target structures
 */
bool testTrackingTargets() {
    Serial.println("Testing tracking target structures...");
    
    // Test target creation
    TrackingTarget target;
    target.pan_angle = 120.0f;
    target.tilt_angle = 80.0f;
    target.confidence = 0.85f;
    target.active = true;
    target.species = "deer";
    
    bool target_valid = (target.pan_angle == 120.0f &&
                        target.tilt_angle == 80.0f &&
                        target.confidence == 0.85f &&
                        target.active &&
                        target.species == "deer");
    
    // Test target with manager
    PanTiltManager manager;
    
    // Should not be tracking initially
    bool initial_tracking = !manager.isTracking();
    
    bool passed = target_valid && initial_tracking;
    Serial.printf("Tracking target test: %s\n", passed ? "PASS" : "FAIL");
    
    return passed;
}

/**
 * Run all servo system tests
 */
void runServoSystemTests() {
    Serial.println("\n===== ESP32 Wildlife Camera - Servo System Tests =====\n");
    
    // Configuration and utility tests
    test_results.recordTest(testServoConfigCreation());
    test_results.recordTest(testServoControllerCreation());
    test_results.recordTest(testPanTiltPositions());
    test_results.recordTest(testPanTiltManagerCreation());
    test_results.recordTest(testServoIntegrationConfig());
    test_results.recordTest(testScanPatternGeneration());
    test_results.recordTest(testTrackingTargets());
    
    // Hardware tests (only if enabled)
    test_results.recordTest(testHardwareInitialization());
    
    // Print summary
    test_results.printSummary();
    
    if (test_results.tests_failed == 0) {
        Serial.println("\n🎉 All servo system tests PASSED! System ready for integration.");
    } else {
        Serial.printf("\n⚠️  %d test(s) FAILED. Review implementation before deployment.\n", 
                     test_results.tests_failed);
    }
}

/**
 * Main test function (can be called from main loop or setup)
 */
void setup() {
    Serial.begin(115200);
    delay(2000);  // Allow serial monitor to connect
    
    runServoSystemTests();
}

void loop() {
    // Test complete - do nothing in loop
    delay(1000);
}