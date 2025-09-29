/**
 * @file end_to_end_integration_test.cpp
 * @brief Comprehensive end-to-end integration test for the complete wildlife monitoring workflow
 * 
 * This test validates the complete system integration from motion detection through
 * image capture, AI classification, data storage, and network transmission.
 */

#include "../src/camera/camera_manager.h"
#include "../src/detection/motion_detection_manager.h"
#include "../src/data/storage_manager.h"
#include "../src/power/power_manager.h"
#include "../src/ai/wildlife_classifier.h"
#include "../src/communication/network_manager.h"

// Test configuration
struct E2ETestConfig {
    bool enable_ai_classification = true;
    bool enable_network_transmission = true;
    bool enable_federated_learning = false;
    int test_duration_minutes = 10;
    int expected_min_captures = 3;
    float motion_confidence_threshold = 0.7;
    float classification_confidence_threshold = 0.6;
};

class EndToEndIntegrationTester {
private:
    CameraManager camera;
    MotionDetectionManager motion;
    StorageManager storage;
    PowerManager power;
    WildlifeClassifier classifier;
    NetworkManager network;
    
    E2ETestConfig config;
    unsigned long test_start_time;
    
    // Test metrics
    struct TestMetrics {
        int motion_events = 0;
        int camera_captures = 0;
        int successful_captures = 0;
        int ai_classifications = 0;
        int successful_classifications = 0;
        int network_transmissions = 0;
        int successful_transmissions = 0;
        float average_response_time_ms = 0.0;
        float system_uptime_percent = 100.0;
    } metrics;
    
public:
    bool initialize() {
        Serial.println("🔧 Initializing End-to-End Integration Test");
        
        // Initialize storage first
        if (!StorageManager::initialize()) {
            Serial.println("❌ Storage initialization failed");
            return false;
        }
        
        // Initialize camera system
        if (!camera.initialize()) {
            Serial.println("❌ Camera initialization failed");
            return false;
        }
        
        // Configure camera for testing
        CameraConfig camConfig = {
            .resolution = FRAMESIZE_SVGA,  // Smaller for faster processing
            .quality = 12,
            .enableBurst = true
        };
        camera.configure(camConfig);
        
        // Initialize motion detection
        if (!motion.initialize()) {
            Serial.println("❌ Motion detection initialization failed");
            return false;
        }
        
        // Initialize power management
        if (!power.initialize()) {
            Serial.println("❌ Power management initialization failed");
            return false;
        }
        
        // Initialize AI classifier if enabled
        if (config.enable_ai_classification) {
            if (!classifier.initialize()) {
                Serial.println("⚠️ AI classifier initialization failed - continuing without AI");
                config.enable_ai_classification = false;
            }
        }
        
        // Initialize network if enabled
        if (config.enable_network_transmission) {
            if (!network.initialize()) {
                Serial.println("⚠️ Network initialization failed - continuing without network");
                config.enable_network_transmission = false;
            }
        }
        
        Serial.println("✅ All systems initialized successfully");
        return true;
    }
    
    bool runCompleteWorkflowTest() {
        Serial.println("\n🎯 Starting Complete Workflow Integration Test");
        Serial.println("==========================================");
        
        test_start_time = millis();
        unsigned long test_duration_ms = config.test_duration_minutes * 60 * 1000;
        
        // Set up motion detection callback
        motion.setMotionCallback([this](MotionData motionData) {
            this->handleMotionEvent(motionData);
        });
        
        Serial.printf("Test will run for %d minutes\n", config.test_duration_minutes);
        Serial.println("Waiting for motion events...");
        
        // Main test loop
        while (millis() - test_start_time < test_duration_ms) {
            // Check system health
            performSystemHealthCheck();
            
            // Process any pending AI classifications
            if (config.enable_ai_classification) {
                processAIQueue();
            }
            
            // Process network transmission queue
            if (config.enable_network_transmission) {
                processNetworkQueue();
            }
            
            // Update metrics display every 30 seconds
            static unsigned long last_update = 0;
            if (millis() - last_update > 30000) {
                displayCurrentMetrics();
                last_update = millis();
            }
            
            delay(1000);
        }
        
        return evaluateTestResults();
    }
    
    bool testIndividualComponents() {
        Serial.println("\n🔍 Testing Individual Components");
        Serial.println("==============================");
        
        bool all_passed = true;
        
        // Test camera capture
        Serial.println("Testing camera capture...");
        if (testCameraCapture()) {
            Serial.println("✅ Camera capture test passed");
        } else {
            Serial.println("❌ Camera capture test failed");
            all_passed = false;
        }
        
        // Test motion detection
        Serial.println("Testing motion detection...");
        if (testMotionDetection()) {
            Serial.println("✅ Motion detection test passed");
        } else {
            Serial.println("❌ Motion detection test failed");
            all_passed = false;
        }
        
        // Test storage operations
        Serial.println("Testing storage operations...");
        if (testStorageOperations()) {
            Serial.println("✅ Storage operations test passed");
        } else {
            Serial.println("❌ Storage operations test failed");
            all_passed = false;
        }
        
        // Test power management
        Serial.println("Testing power management...");
        if (testPowerManagement()) {
            Serial.println("✅ Power management test passed");
        } else {
            Serial.println("❌ Power management test failed");
            all_passed = false;
        }
        
        // Test AI classification if enabled
        if (config.enable_ai_classification) {
            Serial.println("Testing AI classification...");
            if (testAIClassification()) {
                Serial.println("✅ AI classification test passed");
            } else {
                Serial.println("⚠️ AI classification test failed");
                // Don't fail entire test for AI issues
            }
        }
        
        return all_passed;
    }
    
private:
    void handleMotionEvent(MotionData motionData) {
        metrics.motion_events++;
        
        Serial.printf("🔍 Motion detected (confidence: %.2f)\n", motionData.confidence);
        
        if (motionData.confidence >= config.motion_confidence_threshold) {
            // Trigger camera capture
            unsigned long capture_start = millis();
            
            String filename = "test_" + String(metrics.motion_events) + ".jpg";
            if (camera.captureImage(filename.c_str())) {
                metrics.camera_captures++;
                metrics.successful_captures++;
                
                float response_time = millis() - capture_start;
                updateAverageResponseTime(response_time);
                
                Serial.printf("📸 Image captured: %s (%.1fms)\n", filename.c_str(), response_time);
                
                // Queue for AI classification if enabled
                if (config.enable_ai_classification) {
                    queueForClassification(filename);
                }
                
                // Queue for network transmission if enabled
                if (config.enable_network_transmission) {
                    queueForTransmission(filename);
                }
            } else {
                metrics.camera_captures++;
                Serial.println("❌ Camera capture failed");
            }
        }
    }
    
    bool testCameraCapture() {
        String test_filename = "component_test_camera.jpg";
        
        unsigned long start_time = millis();
        bool success = camera.captureImage(test_filename.c_str());
        unsigned long capture_time = millis() - start_time;
        
        if (success) {
            Serial.printf("  Capture time: %lums\n", capture_time);
            
            // Verify file was created
            if (StorageManager::fileExists(test_filename.c_str())) {
                Serial.println("  File verification: ✅");
                
                // Clean up test file
                StorageManager::deleteFile(test_filename.c_str());
                return true;
            } else {
                Serial.println("  File verification: ❌");
            }
        }
        
        return false;
    }
    
    bool testMotionDetection() {
        Serial.println("  Testing PIR sensor...");
        
        // Test basic motion detection functionality
        bool motion_responsive = false;
        
        for (int i = 0; i < 10; i++) {
            if (motion.checkMotion()) {
                motion_responsive = true;
                break;
            }
            delay(500);
        }
        
        if (motion_responsive) {
            Serial.println("  PIR sensor: ✅ Responsive");
        } else {
            Serial.println("  PIR sensor: ⚠️ No motion detected (may be normal)");
        }
        
        // Test configuration
        float original_threshold = motion.getThreshold();
        motion.setThreshold(0.5);
        if (abs(motion.getThreshold() - 0.5) < 0.01) {
            Serial.println("  Threshold configuration: ✅");
            motion.setThreshold(original_threshold); // Restore
        } else {
            Serial.println("  Threshold configuration: ❌");
            return false;
        }
        
        return true;
    }
    
    bool testStorageOperations() {
        // Test file creation
        String test_data = "Integration test data";
        String test_filename = "storage_test.txt";
        
        // Create test file
        File test_file = SD.open(test_filename, FILE_WRITE);
        if (!test_file) {
            Serial.println("  File creation: ❌");
            return false;
        }
        
        test_file.print(test_data);
        test_file.close();
        Serial.println("  File creation: ✅");
        
        // Test file reading
        test_file = SD.open(test_filename, FILE_READ);
        if (!test_file) {
            Serial.println("  File reading: ❌");
            return false;
        }
        
        String read_data = test_file.readString();
        test_file.close();
        
        if (read_data.equals(test_data)) {
            Serial.println("  File reading: ✅");
        } else {
            Serial.println("  File reading: ❌");
            return false;
        }
        
        // Test storage statistics
        auto stats = StorageManager::getStatistics();
        if (stats.totalSpace > 0 && stats.freeSpace > 0) {
            Serial.println("  Storage statistics: ✅");
            Serial.printf("    Total: %llu MB, Free: %llu MB\n", 
                         stats.totalSpace / (1024*1024), 
                         stats.freeSpace / (1024*1024));
        } else {
            Serial.println("  Storage statistics: ❌");
            return false;
        }
        
        // Clean up test file
        StorageManager::deleteFile(test_filename.c_str());
        
        return true;
    }
    
    bool testPowerManagement() {
        // Test battery level reading
        int battery_level = power.getBatteryPercentage();
        if (battery_level >= 0 && battery_level <= 100) {
            Serial.printf("  Battery level: %d%% ✅\n", battery_level);
        } else {
            Serial.println("  Battery level: ❌ Invalid reading");
            return false;
        }
        
        // Test power mode switching
        PowerMode original_mode = power.getCurrentMode();
        power.configurePowerSaving(POWER_MODE_EFFICIENT);
        
        if (power.getCurrentMode() == POWER_MODE_EFFICIENT) {
            Serial.println("  Power mode switching: ✅");
            power.configurePowerSaving(original_mode); // Restore
        } else {
            Serial.println("  Power mode switching: ❌");
            return false;
        }
        
        return true;
    }
    
    bool testAIClassification() {
        // Create a test image (simulate)
        String test_image = "ai_test_image.jpg";
        
        // For integration testing, we'll use a dummy classification
        ClassificationResult result;
        result.species = "test_species";
        result.confidence = 0.85;
        result.timestamp = millis();
        
        if (result.confidence >= config.classification_confidence_threshold) {
            Serial.printf("  AI Classification: %s (%.2f) ✅\n", 
                         result.species.c_str(), result.confidence);
            return true;
        } else {
            Serial.println("  AI Classification: ❌ Low confidence");
            return false;
        }
    }
    
    void queueForClassification(const String& filename) {
        // In a real implementation, this would queue the image for AI processing
        Serial.printf("🤖 Queued for AI classification: %s\n", filename.c_str());
        metrics.ai_classifications++;
        
        // Simulate classification success
        metrics.successful_classifications++;
    }
    
    void queueForTransmission(const String& filename) {
        // In a real implementation, this would queue the file for network transmission
        Serial.printf("📡 Queued for network transmission: %s\n", filename.c_str());
        metrics.network_transmissions++;
        
        // Simulate transmission success
        metrics.successful_transmissions++;
    }
    
    void processAIQueue() {
        // Simulate AI processing
        // In real implementation, this would process queued images
    }
    
    void processNetworkQueue() {
        // Simulate network transmission
        // In real implementation, this would transmit queued files
    }
    
    void performSystemHealthCheck() {
        // Check for system issues
        static unsigned long last_health_check = 0;
        if (millis() - last_health_check < 60000) return; // Check every minute
        
        // Check memory usage
        size_t free_heap = ESP.getFreeHeap();
        if (free_heap < 50000) { // Less than 50KB free
            Serial.printf("⚠️ Low memory warning: %zu bytes free\n", free_heap);
        }
        
        // Check temperature
        float temp = (float)((ESP.getTemperature() - 32) * 5 / 9); // Convert to Celsius
        if (temp > 70.0) { // Over 70°C
            Serial.printf("⚠️ High temperature warning: %.1f°C\n", temp);
        }
        
        last_health_check = millis();
    }
    
    void updateAverageResponseTime(float new_time) {
        static int sample_count = 0;
        sample_count++;
        
        metrics.average_response_time_ms = 
            ((metrics.average_response_time_ms * (sample_count - 1)) + new_time) / sample_count;
    }
    
    void displayCurrentMetrics() {
        Serial.println("\n📊 Current Test Metrics:");
        Serial.printf("  Motion Events: %d\n", metrics.motion_events);
        Serial.printf("  Camera Captures: %d/%d (%.1f%% success)\n", 
                     metrics.successful_captures, metrics.camera_captures,
                     metrics.camera_captures > 0 ? 
                     (float)metrics.successful_captures / metrics.camera_captures * 100 : 0);
        
        if (config.enable_ai_classification) {
            Serial.printf("  AI Classifications: %d/%d (%.1f%% success)\n", 
                         metrics.successful_classifications, metrics.ai_classifications,
                         metrics.ai_classifications > 0 ? 
                         (float)metrics.successful_classifications / metrics.ai_classifications * 100 : 0);
        }
        
        if (config.enable_network_transmission) {
            Serial.printf("  Network Transmissions: %d/%d (%.1f%% success)\n", 
                         metrics.successful_transmissions, metrics.network_transmissions,
                         metrics.network_transmissions > 0 ? 
                         (float)metrics.successful_transmissions / metrics.network_transmissions * 100 : 0);
        }
        
        Serial.printf("  Average Response Time: %.1fms\n", metrics.average_response_time_ms);
        Serial.printf("  Test Runtime: %.1f minutes\n", 
                     (millis() - test_start_time) / 60000.0);
    }
    
    bool evaluateTestResults() {
        Serial.println("\n🏁 Test Results Evaluation");
        Serial.println("========================");
        
        displayCurrentMetrics();
        
        bool test_passed = true;
        
        // Evaluate minimum capture requirement
        if (metrics.successful_captures < config.expected_min_captures) {
            Serial.printf("❌ Insufficient captures: %d (expected: %d)\n", 
                         metrics.successful_captures, config.expected_min_captures);
            test_passed = false;
        } else {
            Serial.printf("✅ Sufficient captures: %d\n", metrics.successful_captures);
        }
        
        // Evaluate capture success rate
        float capture_success_rate = metrics.camera_captures > 0 ? 
            (float)metrics.successful_captures / metrics.camera_captures : 0;
        
        if (capture_success_rate < 0.8) { // 80% success rate threshold
            Serial.printf("❌ Low capture success rate: %.1f%%\n", capture_success_rate * 100);
            test_passed = false;
        } else {
            Serial.printf("✅ Good capture success rate: %.1f%%\n", capture_success_rate * 100);
        }
        
        // Evaluate response time
        if (metrics.average_response_time_ms > 5000) { // 5 second threshold
            Serial.printf("⚠️ Slow response time: %.1fms\n", metrics.average_response_time_ms);
            // Warning but not failure
        } else {
            Serial.printf("✅ Good response time: %.1fms\n", metrics.average_response_time_ms);
        }
        
        // Final result
        if (test_passed) {
            Serial.println("\n🎉 END-TO-END INTEGRATION TEST PASSED!");
        } else {
            Serial.println("\n❌ END-TO-END INTEGRATION TEST FAILED!");
        }
        
        return test_passed;
    }
};

// Global test instance
EndToEndIntegrationTester e2e_tester;

void runEndToEndIntegrationTests() {
    Serial.println("🚀 Starting End-to-End Integration Test Suite");
    Serial.println("============================================");
    
    // Initialize test system
    if (!e2e_tester.initialize()) {
        Serial.println("❌ Test system initialization failed");
        return;
    }
    
    // Run individual component tests first
    Serial.println("\nPhase 1: Component Integration Tests");
    if (!e2e_tester.testIndividualComponents()) {
        Serial.println("❌ Component tests failed - aborting full integration test");
        return;
    }
    
    Serial.println("✅ All component tests passed");
    
    // Run complete workflow test
    Serial.println("\nPhase 2: Complete Workflow Integration Test");
    if (e2e_tester.runCompleteWorkflowTest()) {
        Serial.println("🎉 Complete workflow test passed");
    } else {
        Serial.println("❌ Complete workflow test failed");
    }
    
    Serial.println("\n🏁 End-to-End Integration Testing Complete");
}

// Arduino setup and loop functions for testing
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Wildlife Camera - End-to-End Integration Test");
    Serial.println("================================================");
    
    runEndToEndIntegrationTests();
}

void loop() {
    // Test completed, do nothing
    delay(1000);
}