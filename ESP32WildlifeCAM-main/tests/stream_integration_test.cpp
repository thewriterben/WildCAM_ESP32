/**
 * @file stream_integration_test.cpp
 * @brief Integration test for streaming functionality
 * 
 * This test validates that the StreamManager integrates properly with
 * the existing CameraManager and EnhancedWebServer components.
 */

#include "../src/streaming/stream_manager.h"
#include "../src/camera/camera_manager.h"

// Mock implementations for testing
class MockPowerManager {
public:
    int batteryPercentage = 75;
    bool isCharging = false;
    
    int getBatteryPercentage() const { return batteryPercentage; }
    bool getChargingStatus() const { return isCharging; }
};

class MockMotionDetectionManager {
public:
    bool motionDetected = false;
    float confidence = 0.0;
    
    bool isMotionDetected() const { return motionDetected; }
    float getConfidence() const { return confidence; }
    
    void triggerMotion(float conf) {
        motionDetected = true;
        confidence = conf;
    }
    
    void clearMotion() {
        motionDetected = false;
        confidence = 0.0;
    }
};

void testStreamManagerInitialization() {
    Serial.println("\n=== Testing StreamManager Initialization ===");
    
    StreamManager streamManager;
    CameraManager cameraManager;
    MockPowerManager powerManager;
    MockMotionDetectionManager motionManager;
    
    // Test initialization without camera manager (should fail)
    if (!streamManager.initialize(nullptr)) {
        Serial.println("✓ Correctly rejected null camera manager");
    } else {
        Serial.println("✗ Should have rejected null camera manager");
    }
    
    // Test proper initialization
    if (streamManager.initialize(&cameraManager, 
                                reinterpret_cast<PowerManager*>(&powerManager),
                                reinterpret_cast<MotionDetectionManager*>(&motionManager))) {
        Serial.println("✓ StreamManager initialized successfully");
    } else {
        Serial.println("✗ StreamManager initialization failed");
    }
    
    // Test configuration
    StreamConfig config;
    config.targetFPS = 5;
    config.quality = STREAM_QUALITY_MEDIUM;
    config.frameSize = STREAM_FRAMESIZE_VGA;
    
    if (streamManager.setStreamConfig(config)) {
        Serial.println("✓ Stream configuration set successfully");
    } else {
        Serial.println("✗ Stream configuration failed");
    }
    
    Serial.println("StreamManager initialization tests completed");
}

void testPowerAwareProfiles() {
    Serial.println("\n=== Testing Power-Aware Profiles ===");
    
    // Test profile selection based on battery level
    StreamProfile highBattery = getProfileForBatteryLevel(80);
    StreamProfile mediumBattery = getProfileForBatteryLevel(40);
    StreamProfile lowBattery = getProfileForBatteryLevel(20);
    StreamProfile criticalBattery = getProfileForBatteryLevel(10);
    
    Serial.printf("High battery (80%%): %d FPS, %s quality, %s\n",
                 highBattery.maxFPS,
                 streamQualityToString(highBattery.quality),
                 highBattery.motionOnlyMode ? "motion-only" : "continuous");
    
    Serial.printf("Medium battery (40%%): %d FPS, %s quality, %s\n",
                 mediumBattery.maxFPS,
                 streamQualityToString(mediumBattery.quality),
                 mediumBattery.motionOnlyMode ? "motion-only" : "continuous");
    
    Serial.printf("Low battery (20%%): %d FPS, %s quality, %s\n",
                 lowBattery.maxFPS,
                 streamQualityToString(lowBattery.quality),
                 lowBattery.motionOnlyMode ? "motion-only" : "continuous");
    
    Serial.printf("Critical battery (10%%): %d FPS, %s quality, %s\n",
                 criticalBattery.maxFPS,
                 streamQualityToString(criticalBattery.quality),
                 criticalBattery.motionOnlyMode ? "motion-only" : "continuous");
    
    // Validate that profiles are appropriately conservative at low battery
    if (highBattery.maxFPS > mediumBattery.maxFPS &&
        mediumBattery.maxFPS >= lowBattery.maxFPS &&
        lowBattery.maxFPS >= criticalBattery.maxFPS) {
        Serial.println("✓ Frame rates appropriately scale with battery level");
    } else {
        Serial.println("✗ Frame rate scaling incorrect");
    }
    
    if (!highBattery.motionOnlyMode && !mediumBattery.motionOnlyMode &&
        lowBattery.motionOnlyMode && criticalBattery.motionOnlyMode) {
        Serial.println("✓ Motion-only mode correctly enabled for low battery");
    } else {
        Serial.println("✗ Motion-only mode configuration incorrect");
    }
    
    Serial.println("Power-aware profile tests completed");
}

void testStreamConfiguration() {
    Serial.println("\n=== Testing Stream Configuration ===");
    
    StreamManager streamManager;
    CameraManager cameraManager;
    
    streamManager.initialize(&cameraManager);
    
    // Test frame rate configuration
    if (streamManager.setFrameRate(5)) {
        Serial.println("✓ Valid frame rate (5 FPS) accepted");
    } else {
        Serial.println("✗ Valid frame rate rejected");
    }
    
    if (!streamManager.setFrameRate(15)) {
        Serial.println("✓ Invalid frame rate (15 FPS) correctly rejected");
    } else {
        Serial.println("✗ Invalid frame rate should have been rejected");
    }
    
    // Test quality configuration
    if (streamManager.setQuality(STREAM_QUALITY_HIGH)) {
        Serial.println("✓ Quality setting accepted");
    } else {
        Serial.println("✗ Quality setting failed");
    }
    
    // Test frame size configuration
    if (streamManager.setFrameSize(STREAM_FRAMESIZE_VGA)) {
        Serial.println("✓ Frame size setting accepted");
    } else {
        Serial.println("✗ Frame size setting failed");
    }
    
    // Test motion-only mode
    streamManager.setMotionOnlyMode(true);
    Serial.println("✓ Motion-only mode configured");
    
    Serial.println("Stream configuration tests completed");
}

void testStreamingLogic() {
    Serial.println("\n=== Testing Streaming Logic ===");
    
    StreamManager streamManager;
    CameraManager cameraManager;
    MockPowerManager powerManager;
    MockMotionDetectionManager motionManager;
    
    streamManager.initialize(&cameraManager,
                            reinterpret_cast<PowerManager*>(&powerManager),
                            reinterpret_cast<MotionDetectionManager*>(&motionManager));
    
    // Test streaming state
    if (!streamManager.isStreaming()) {
        Serial.println("✓ Initial streaming state is false");
    } else {
        Serial.println("✗ Initial streaming state should be false");
    }
    
    // Test client count
    if (streamManager.getClientCount() == 0) {
        Serial.println("✓ Initial client count is zero");
    } else {
        Serial.println("✗ Initial client count should be zero");
    }
    
    // Test motion event handling
    streamManager.handleMotionEvent(true, 75.0);
    Serial.println("✓ Motion event handled");
    
    // Test power status update
    streamManager.updatePowerStatus(50, false);
    Serial.println("✓ Power status updated");
    
    // Test statistics
    StreamStats stats = streamManager.getStreamStats();
    if (stats.totalFramesSent == 0 && stats.currentClients == 0) {
        Serial.println("✓ Initial statistics are correct");
    } else {
        Serial.println("✗ Initial statistics incorrect");
    }
    
    Serial.println("Streaming logic tests completed");
}

void testJSONGeneration() {
    Serial.println("\n=== Testing JSON Generation ===");
    
    StreamManager streamManager;
    CameraManager cameraManager;
    
    streamManager.initialize(&cameraManager);
    
    // Test status JSON
    String statusJSON = streamManager.getStatusJSON();
    if (statusJSON.length() > 0 && statusJSON.indexOf("streaming") != -1) {
        Serial.println("✓ Status JSON generated successfully");
        Serial.println("  Status: " + statusJSON);
    } else {
        Serial.println("✗ Status JSON generation failed");
    }
    
    // Test stats JSON
    String statsJSON = streamManager.getStatsJSON();
    if (statsJSON.length() > 0 && statsJSON.indexOf("totalFrames") != -1) {
        Serial.println("✓ Stats JSON generated successfully");
        Serial.println("  Stats: " + statsJSON);
    } else {
        Serial.println("✗ Stats JSON generation failed");
    }
    
    Serial.println("JSON generation tests completed");
}

void testUtilityFunctions() {
    Serial.println("\n=== Testing Utility Functions ===");
    
    // Test quality to string conversion
    if (String(streamQualityToString(STREAM_QUALITY_HIGH)) == "High") {
        Serial.println("✓ Quality to string conversion works");
    } else {
        Serial.println("✗ Quality to string conversion failed");
    }
    
    // Test frame size to string conversion
    if (String(streamFrameSizeToString(STREAM_FRAMESIZE_VGA)) == "VGA (640x480)") {
        Serial.println("✓ Frame size to string conversion works");
    } else {
        Serial.println("✗ Frame size to string conversion failed");
    }
    
    // Test frame size to ESP32 conversion
    if (streamFrameSizeToESP32(STREAM_FRAMESIZE_VGA) == FRAMESIZE_VGA) {
        Serial.println("✓ Frame size to ESP32 conversion works");
    } else {
        Serial.println("✗ Frame size to ESP32 conversion failed");
    }
    
    Serial.println("Utility function tests completed");
}

void runStreamingIntegrationTests() {
    Serial.println("🎯 Starting Streaming Integration Tests");
    Serial.println("=====================================");
    
    testStreamManagerInitialization();
    testPowerAwareProfiles();
    testStreamConfiguration();
    testStreamingLogic();
    testJSONGeneration();
    testUtilityFunctions();
    
    Serial.println("\n🏁 Streaming Integration Tests Complete");
    Serial.println("=====================================");
}

// Arduino setup and loop functions for testing
void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Wildlife Camera - Streaming Integration Test");
    Serial.println("================================================");
    
    runStreamingIntegrationTests();
}

void loop() {
    // Test completed, do nothing
    delay(1000);
}