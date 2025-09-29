/**
 * @file phase2_integration_test.cpp
 * @brief Integration test for Phase 2 Core Functionality
 * 
 * This test validates that the Phase 2 integration of motion detection,
 * storage management, and power management works correctly with the
 * existing camera system.
 */

#include "../src/camera/camera_manager.h"
#include "../src/detection/motion_detection_manager.h"
#include "../src/data/storage_manager.h"
#include "../firmware/src/power_manager.h"

void testPowerManagerIntegration() {
    Serial.println("=== Testing Power Manager Integration ===");
    
    PowerManager powerManager;
    
    // Test initialization
    bool initialized = powerManager.init();
    if (initialized) {
        Serial.println("✓ Power manager initialization successful");
    } else {
        Serial.println("✗ Power manager initialization failed");
        return;
    }
    
    // Test power readings
    powerManager.update();
    float batteryVoltage = powerManager.getBatteryVoltage();
    int batteryPercentage = powerManager.getBatteryPercentage();
    float solarVoltage = powerManager.getSolarVoltage();
    
    Serial.printf("✓ Battery voltage: %.2fV (%.0f%%)\n", batteryVoltage, (float)batteryPercentage);
    Serial.printf("✓ Solar voltage: %.2fV\n", solarVoltage);
    
    if (batteryVoltage > 0.0 && batteryPercentage >= 0) {
        Serial.println("✓ Power readings appear valid");
    } else {
        Serial.println("⚠ Power readings may need calibration");
    }
}

void testStorageManagerIntegration() {
    Serial.println("=== Testing Storage Manager Integration ===");
    
    // Test SD card detection and initialization
    bool sdInitialized = StorageManager::initialize();
    if (sdInitialized) {
        Serial.println("✓ SD card detected and initialized");
    } else {
        Serial.println("✗ SD card not detected or initialization failed");
        return;
    }
    
    // Test storage statistics
    auto stats = StorageManager::getStatistics();
    Serial.printf("✓ Storage stats - Total: %llu MB, Free: %llu MB (%.1f%% used)\n", 
                  stats.totalSpace / (1024*1024), 
                  stats.freeSpace / (1024*1024), 
                  stats.usagePercentage);
    
    // Test adequate space check
    bool hasSpace = StorageManager::hasAdequateSpace(10 * 1024 * 1024); // 10MB
    if (hasSpace) {
        Serial.println("✓ Adequate space available for wildlife monitoring");
    } else {
        Serial.println("⚠ Low storage space - cleanup may be needed");
    }
}

void testMotionDetectionIntegration() {
    Serial.println("=== Testing Motion Detection Integration ===");
    
    CameraManager camera;
    MotionDetectionManager motionManager;
    
    // Test camera initialization
    bool cameraInitialized = camera.initialize();
    if (!cameraInitialized) {
        Serial.println("✗ Camera initialization failed - skipping motion detection test");
        return;
    }
    Serial.println("✓ Camera manager initialized for motion detection");
    
    // Test motion detection manager initialization
    bool motionInitialized = motionManager.initialize(&camera, MotionDetectionManager::DetectionSystem::ENHANCED_HYBRID);
    if (!motionInitialized) {
        Serial.println("✗ Motion detection manager initialization failed");
        return;
    }
    Serial.println("✓ Motion detection manager initialized");
    
    // Test enhanced features configuration
    bool featuresConfigured = motionManager.configureEnhancedFeatures(true, true, true);
    if (featuresConfigured) {
        Serial.println("✓ Enhanced motion detection features configured (multi-zone PIR, analytics)");
    } else {
        Serial.println("⚠ Enhanced features configuration had issues");
    }
    
    // Test basic motion detection capability
    auto motionResult = motionManager.detectMotion();
    Serial.printf("✓ Motion detection test completed - detected: %s, confidence: %.2f\n",
                  motionResult.motionDetected ? "YES" : "NO",
                  motionResult.confidenceScore);
    
    if (motionResult.hasEnhancedData) {
        Serial.printf("✓ Enhanced data available - zones: %d, speed: %.1f\n",
                      motionResult.activeZoneCount,
                      motionResult.motionSpeed);
    }
}

void testPhase2Integration() {
    Serial.println("=== Testing Phase 2 Complete Integration ===");
    
    PowerManager powerManager;
    CameraManager camera;
    MotionDetectionManager motionManager;
    
    // Initialize all systems
    bool allInitialized = true;
    
    allInitialized &= powerManager.init();
    allInitialized &= StorageManager::initialize();
    allInitialized &= camera.initialize();
    allInitialized &= motionManager.initialize(&camera, MotionDetectionManager::DetectionSystem::ENHANCED_HYBRID);
    allInitialized &= motionManager.configureEnhancedFeatures(true, true, true);
    
    if (!allInitialized) {
        Serial.println("✗ Phase 2 integration test failed - not all systems initialized");
        return;
    }
    
    Serial.println("✓ All Phase 2 systems initialized successfully");
    
    // Test the complete workflow: Motion Detection → Camera Capture → Storage
    powerManager.update();
    auto motionResult = motionManager.detectMotion();
    
    if (motionResult.motionDetected) {
        Serial.println("✓ Motion detected - testing capture workflow");
        
        // Test wildlife image capture and storage
        auto captureResult = camera.captureImage("/wildlife_motion");
        
        if (captureResult.success) {
            Serial.printf("✓ Motion-triggered capture successful: %s (%.2f KB)\n",
                          captureResult.filename.c_str(),
                          captureResult.imageSize / 1024.0);
        } else {
            Serial.println("✗ Motion-triggered capture failed");
        }
    } else {
        Serial.println("✓ No motion detected - system ready for monitoring");
    }
    
    // Test power awareness
    int batteryLevel = powerManager.getBatteryPercentage();
    if (batteryLevel < 20) {
        Serial.printf("⚠ Low battery (%d%%) - power management active\n", batteryLevel);
    } else {
        Serial.printf("✓ Battery level good (%d%%) - normal operation\n", batteryLevel);
    }
    
    Serial.println("✓ Phase 2 Core Functionality integration test completed");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32WildlifeCAM Phase 2 Integration Test");
    Serial.println("=========================================");
    
    testPowerManagerIntegration();
    delay(1000);
    
    testStorageManagerIntegration();
    delay(1000);
    
    testMotionDetectionIntegration();
    delay(1000);
    
    testPhase2Integration();
    
    Serial.println("\nPhase 2 Integration Test Complete");
}

void loop() {
    delay(10000); // Run test once
}