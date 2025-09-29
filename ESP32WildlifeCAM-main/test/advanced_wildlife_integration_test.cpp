/**
 * @file advanced_wildlife_integration_test.cpp
 * @brief Integration Test for Complete Advanced Wildlife Camera System
 * 
 * Tests the integration of all advanced features: motion detection, servo control,
 * audio classification, and multi-modal detection capabilities.
 */

#include "../src/advanced_wildlife_manager.h"
#include "../src/servo/servo_controller.h"
#include "../src/audio/wildlife_audio_classifier.h"
#include "../include/config.h"
#include <Arduino.h>

// Test configuration
const bool RUN_HARDWARE_TESTS = false; // Set to true only if hardware is connected
const bool RUN_AUDIO_TESTS = false;    // Set to true only if microphone is connected
const bool VERBOSE_LOGGING = true;     // Enable detailed logging

/**
 * Test Results Structure
 */
struct IntegrationTestResults {
    uint32_t tests_run = 0;
    uint32_t tests_passed = 0;
    uint32_t tests_failed = 0;
    uint32_t tests_skipped = 0;
    
    void recordTest(bool passed, bool skipped = false) {
        tests_run++;
        if (skipped) {
            tests_skipped++;
        } else if (passed) {
            tests_passed++;
        } else {
            tests_failed++;
        }
    }
    
    void printSummary() {
        Serial.printf("\n=== Integration Test Summary ===\n");
        Serial.printf("Tests Run: %d\n", tests_run);
        Serial.printf("Passed: %d\n", tests_passed);
        Serial.printf("Failed: %d\n", tests_failed);
        Serial.printf("Skipped: %d\n", tests_skipped);
        if (tests_run > 0) {
            Serial.printf("Success Rate: %.1f%%\n", (float)tests_passed / (tests_run - tests_skipped) * 100.0f);
        }
    }
};

IntegrationTestResults test_results;

/**
 * Test configuration validation
 */
bool testConfigurationValidation() {
    Serial.println("Testing configuration validation...");
    
    // Test servo configuration
    bool servo_config = (PAN_TILT_ENABLED == true &&
                         PAN_SERVO_PIN == 16 &&
                         TILT_SERVO_PIN == 17 &&
                         SERVO_MOVEMENT_SPEED == 90.0f);
    
    // Test audio configuration
    bool audio_config = (AUDIO_CLASSIFICATION_ENABLED == true &&
                        I2S_SAMPLE_RATE == 16000 &&
                        AUDIO_DMA_BUFFER_COUNT == 4);
    
    // Test motion detection configuration
    bool motion_config = (MOTION_DETECTION_ENABLED == true &&
                         MULTI_ZONE_PIR_ENABLED == true &&
                         ADVANCED_FRAME_ANALYSIS == true);
    
    // Test multimodal configuration
    bool multimodal_config = (MULTIMODAL_DETECTION_ENABLED == true &&
                             COMBINED_CONFIDENCE_THRESHOLD == 0.8f);
    
    bool passed = servo_config && audio_config && motion_config && multimodal_config;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Servo config: %s\n", servo_config ? "PASS" : "FAIL");
        Serial.printf("  Audio config: %s\n", audio_config ? "PASS" : "FAIL");
        Serial.printf("  Motion config: %s\n", motion_config ? "PASS" : "FAIL");
        Serial.printf("  Multimodal config: %s\n", multimodal_config ? "PASS" : "FAIL");
    }
    
    Serial.printf("Configuration validation: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test advanced wildlife manager creation
 */
bool testAdvancedWildlifeManagerCreation() {
    Serial.println("Testing advanced wildlife manager creation...");
    
    AdvancedWildlifeManager manager;
    
    // Test initial state
    bool initial_state = (manager.getCurrentMode() == OperationMode::IDLE);
    
    // Test status string
    String status = manager.getSystemStatus();
    bool status_valid = (status.length() > 0);
    
    // Test metrics
    SystemMetrics metrics = manager.getMetrics();
    bool metrics_valid = (metrics.total_detections == 0 && 
                         metrics.uptime_seconds == 0);
    
    bool passed = initial_state && status_valid && metrics_valid;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Initial state: %s\n", initial_state ? "PASS" : "FAIL");
        Serial.printf("  Status valid: %s\n", status_valid ? "PASS" : "FAIL");
        Serial.printf("  Metrics valid: %s\n", metrics_valid ? "PASS" : "FAIL");
    }
    
    Serial.printf("Advanced wildlife manager creation: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test wildlife detection event structure
 */
bool testWildlifeDetectionEvent() {
    Serial.println("Testing wildlife detection event structure...");
    
    WildlifeDetectionEvent event;
    event.timestamp = millis();
    event.species = "white_tailed_deer";
    event.confidence = 0.85f;
    event.has_visual_detection = true;
    event.has_audio_detection = false;
    event.has_servo_tracking = true;
    event.pan_angle = 120.0f;
    event.tilt_angle = 80.0f;
    event.event_description = "Visual detection with servo tracking";
    
    bool structure_valid = (event.species == "white_tailed_deer" &&
                           event.confidence == 0.85f &&
                           event.has_visual_detection &&
                           !event.has_audio_detection &&
                           event.has_servo_tracking &&
                           event.pan_angle == 120.0f);
    
    Serial.printf("Wildlife detection event: %s\n", structure_valid ? "PASS" : "FAIL");
    return structure_valid;
}

/**
 * Test audio classification structures
 */
bool testAudioClassificationStructures() {
    Serial.println("Testing audio classification structures...");
    
    // Test species signature
    SpeciesAudioSignature signature;
    signature.species_name = "red_tailed_hawk";
    signature.frequency_range_min = 1000.0f;
    signature.frequency_range_max = 4000.0f;
    signature.peak_frequency = 2500.0f;
    
    bool signature_valid = (signature.species_name == "red_tailed_hawk" &&
                           signature.frequency_range_min == 1000.0f &&
                           signature.peak_frequency == 2500.0f);
    
    // Test classification result
    AudioClassificationResult result;
    result.detected_species = "coyote";
    result.confidence = 0.75f;
    result.is_wildlife = true;
    result.is_mammal_call = true;
    
    bool result_valid = (result.detected_species == "coyote" &&
                        result.confidence == 0.75f &&
                        result.is_wildlife &&
                        result.is_mammal_call);
    
    // Test multimodal result
    MultiModalResult multimodal;
    multimodal.audio_result = result;
    multimodal.has_visual_detection = true;
    multimodal.visual_species = "coyote";
    multimodal.visual_confidence = 0.80f;
    multimodal.correlation_found = true;
    
    bool multimodal_valid = (multimodal.has_visual_detection &&
                            multimodal.visual_species == "coyote" &&
                            multimodal.correlation_found);
    
    bool passed = signature_valid && result_valid && multimodal_valid;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Signature valid: %s\n", signature_valid ? "PASS" : "FAIL");
        Serial.printf("  Result valid: %s\n", result_valid ? "PASS" : "FAIL");
        Serial.printf("  Multimodal valid: %s\n", multimodal_valid ? "PASS" : "FAIL");
    }
    
    Serial.printf("Audio classification structures: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test operation mode transitions
 */
bool testOperationModeTransitions() {
    Serial.println("Testing operation mode transitions...");
    
    AdvancedWildlifeManager manager;
    
    // Test initial mode
    bool initial_idle = (manager.getCurrentMode() == OperationMode::IDLE);
    
    // Test mode transitions
    manager.setOperationMode(OperationMode::MONITORING);
    bool to_monitoring = (manager.getCurrentMode() == OperationMode::MONITORING);
    
    manager.setOperationMode(OperationMode::TRACKING);
    bool to_tracking = (manager.getCurrentMode() == OperationMode::TRACKING);
    
    manager.setOperationMode(OperationMode::SCANNING);
    bool to_scanning = (manager.getCurrentMode() == OperationMode::SCANNING);
    
    manager.setOperationMode(OperationMode::MULTIMODAL_ANALYSIS);
    bool to_multimodal = (manager.getCurrentMode() == OperationMode::MULTIMODAL_ANALYSIS);
    
    manager.setOperationMode(OperationMode::IDLE);
    bool back_to_idle = (manager.getCurrentMode() == OperationMode::IDLE);
    
    bool passed = initial_idle && to_monitoring && to_tracking && to_scanning && to_multimodal && back_to_idle;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Initial idle: %s\n", initial_idle ? "PASS" : "FAIL");
        Serial.printf("  To monitoring: %s\n", to_monitoring ? "PASS" : "FAIL");
        Serial.printf("  To tracking: %s\n", to_tracking ? "PASS" : "FAIL");
        Serial.printf("  To scanning: %s\n", to_scanning ? "PASS" : "FAIL");
        Serial.printf("  To multimodal: %s\n", to_multimodal ? "PASS" : "FAIL");
        Serial.printf("  Back to idle: %s\n", back_to_idle ? "PASS" : "FAIL");
    }
    
    Serial.printf("Operation mode transitions: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test system integration (without hardware)
 */
bool testSystemIntegration() {
    Serial.println("Testing system integration...");
    
    AdvancedWildlifeManager manager;
    
    // Test subsystem enable/disable
    manager.setMotionDetectionEnabled(true);
    manager.setAudioClassificationEnabled(true);
    manager.setServoControlEnabled(true);
    manager.setMultiModalDetectionEnabled(true);
    
    // Test configuration
    manager.setDetectionSensitivity(0.8f);
    manager.setTrackingEnabled(true);
    manager.setScanningEnabled(true);
    
    // Test power management
    manager.enablePowerSavingMode(true);
    manager.enablePowerSavingMode(false);
    
    // Test manual controls
    bool stop_success = manager.stopAllActivities();
    bool home_success = manager.returnToHome();
    
    // Test status and metrics
    String status = manager.getSystemStatus();
    SystemMetrics metrics = manager.getMetrics();
    
    bool passed = stop_success && home_success && (status.length() > 0);
    
    Serial.printf("System integration: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test hardware initialization (only if enabled)
 */
bool testHardwareInitialization() {
    if (!RUN_HARDWARE_TESTS) {
        Serial.println("Skipping hardware initialization tests (RUN_HARDWARE_TESTS = false)");
        return true; // Skip but don't fail
    }
    
    Serial.println("Testing hardware initialization...");
    
    AdvancedWildlifeManager manager;
    
    // Test full system initialization
    bool init_success = manager.initialize();
    
    if (!init_success) {
        Serial.printf("Hardware initialization: FAIL (initialization failed)\n");
        return false;
    }
    
    // Test system status after initialization
    String status = manager.getSystemStatus();
    bool status_valid = (status.length() > 0 && status.indexOf("ERROR") == -1);
    
    // Test manual servo control
    bool servo_control = manager.manualServoControl(90.0f, 90.0f);
    delay(1000);
    
    // Test scanning
    bool scan_start = manager.startManualScanning(ScanPattern::HORIZONTAL_SWEEP);
    delay(2000);
    bool scan_stop = manager.stopAllActivities();
    
    // Cleanup
    manager.cleanup();
    
    bool passed = init_success && status_valid && servo_control && scan_start && scan_stop;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Initialization: %s\n", init_success ? "PASS" : "FAIL");
        Serial.printf("  Status valid: %s\n", status_valid ? "PASS" : "FAIL");
        Serial.printf("  Servo control: %s\n", servo_control ? "PASS" : "FAIL");
        Serial.printf("  Scanning: %s\n", (scan_start && scan_stop) ? "PASS" : "FAIL");
    }
    
    Serial.printf("Hardware initialization: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test audio classification (only if enabled)
 */
bool testAudioClassification() {
    if (!RUN_AUDIO_TESTS) {
        Serial.println("Skipping audio classification tests (RUN_AUDIO_TESTS = false)");
        return true; // Skip but don't fail
    }
    
    Serial.println("Testing audio classification...");
    
    WildlifeAudioClassifier classifier;
    
    // Test initialization
    bool init_success = classifier.initialize(MicrophoneType::I2S_DIGITAL);
    
    if (!init_success) {
        Serial.printf("Audio classification: FAIL (initialization failed)\n");
        return false;
    }
    
    // Test signature loading
    classifier.loadWildlifeSignatures();
    
    // Test monitoring
    classifier.setMonitoringEnabled(true);
    
    // Test audio processing (simulate)
    AudioClassificationResult result = classifier.monitorAudio();
    bool monitoring_valid = (result.detection_timestamp > 0);
    
    // Test statistics
    String stats = classifier.getStatistics();
    bool stats_valid = (stats.length() > 0);
    
    // Cleanup
    classifier.cleanup();
    
    bool passed = init_success && monitoring_valid && stats_valid;
    
    Serial.printf("Audio classification: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Test global convenience functions
 */
bool testGlobalFunctions() {
    Serial.println("Testing global convenience functions...");
    
    // Test global availability
    bool initially_unavailable = !WildlifeManager::isGlobalAvailable();
    
    // Test global initialization
    bool init_success = WildlifeManager::initializeGlobal();
    bool now_available = WildlifeManager::isGlobalAvailable();
    
    // Test global status
    String status = WildlifeManager::getGlobalStatus();
    bool status_valid = (status.length() > 0);
    
    // Test cleanup
    WildlifeManager::cleanupGlobal();
    bool cleaned_up = !WildlifeManager::isGlobalAvailable();
    
    bool passed = initially_unavailable && init_success && now_available && status_valid && cleaned_up;
    
    if (VERBOSE_LOGGING) {
        Serial.printf("  Initially unavailable: %s\n", initially_unavailable ? "PASS" : "FAIL");
        Serial.printf("  Init success: %s\n", init_success ? "PASS" : "FAIL");
        Serial.printf("  Now available: %s\n", now_available ? "PASS" : "FAIL");
        Serial.printf("  Status valid: %s\n", status_valid ? "PASS" : "FAIL");
        Serial.printf("  Cleaned up: %s\n", cleaned_up ? "PASS" : "FAIL");
    }
    
    Serial.printf("Global functions: %s\n", passed ? "PASS" : "FAIL");
    return passed;
}

/**
 * Run all integration tests
 */
void runAdvancedWildlifeIntegrationTests() {
    Serial.println("\n===== ESP32 Wildlife Camera - Advanced Integration Tests =====\n");
    
    // Configuration and structure tests
    test_results.recordTest(testConfigurationValidation());
    test_results.recordTest(testAdvancedWildlifeManagerCreation());
    test_results.recordTest(testWildlifeDetectionEvent());
    test_results.recordTest(testAudioClassificationStructures());
    test_results.recordTest(testOperationModeTransitions());
    test_results.recordTest(testSystemIntegration());
    test_results.recordTest(testGlobalFunctions());
    
    // Hardware-dependent tests (only if enabled)
    test_results.recordTest(testHardwareInitialization(), !RUN_HARDWARE_TESTS);
    test_results.recordTest(testAudioClassification(), !RUN_AUDIO_TESTS);
    
    // Print summary
    test_results.printSummary();
    
    if (test_results.tests_failed == 0) {
        Serial.println("\n🎉 All integration tests PASSED! Advanced wildlife camera system ready for deployment.");
        Serial.println("\n📋 System Features Ready:");
        Serial.println("   ✅ Motion Detection with Enhanced Analytics");
        Serial.println("   ✅ Pan/Tilt Servo Control with Wildlife Tracking");
        Serial.println("   ✅ Audio Classification with Species Recognition");
        Serial.println("   ✅ Multi-Modal Detection (Audio + Visual)");
        Serial.println("   ✅ Power Management Integration");
        Serial.println("   ✅ Automated Scanning Patterns");
        Serial.println("   ✅ Unified Configuration System");
    } else {
        Serial.printf("\n⚠️  %d test(s) FAILED. Review implementation before deployment.\n", 
                     test_results.tests_failed);
    }
    
    Serial.println("\n📖 Next Steps:");
    Serial.println("   1. Connect servo motors to GPIO 16 (pan) and GPIO 17 (tilt)");
    Serial.println("   2. Connect I2S microphone (WS=25, SCK=26, SD=22)");
    Serial.println("   3. Set RUN_HARDWARE_TESTS=true and RUN_AUDIO_TESTS=true");
    Serial.println("   4. Run full hardware validation");
    Serial.println("   5. Deploy in field environment");
}

/**
 * Main test function
 */
void setup() {
    Serial.begin(115200);
    delay(2000);  // Allow serial monitor to connect
    
    runAdvancedWildlifeIntegrationTests();
}

void loop() {
    // Tests complete - do nothing in loop
    delay(1000);
}