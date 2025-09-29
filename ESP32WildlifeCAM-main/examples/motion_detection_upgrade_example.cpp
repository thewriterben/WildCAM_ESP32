/**
 * @file motion_detection_upgrade_example.cpp
 * @brief Example showing how to upgrade existing motion detection to advanced features
 * 
 * This example demonstrates:
 * 1. Drop-in replacement for existing HybridMotionDetector
 * 2. Gradual feature upgrade path
 * 3. Real-world integration patterns
 */

#include <Arduino.h>
#include "detection/motion_detection_manager.h"
#include "camera/camera_manager.h"

// Global objects
CameraManager cameraManager;
MotionDetectionManager motionManager;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== Motion Detection Upgrade Example ===");
    
    // Initialize camera (existing code unchanged)
    if (!cameraManager.initialize()) {
        Serial.println("Failed to initialize camera");
        return;
    }
    
    // OPTION 1: Drop-in replacement - use legacy mode for existing code
    Serial.println("\\n1. Drop-in replacement (no code changes needed):");
    if (motionManager.initialize(&cameraManager, MotionDetectionManager::DetectionSystem::LEGACY_HYBRID)) {
        Serial.println("Legacy mode initialized - existing code works unchanged");
        
        // Test existing motion detection pattern
        auto result = motionManager.detectMotion();
        Serial.printf("Motion: %s, Confidence: %.2f\\n", 
                     result.motionDetected ? "YES" : "NO", 
                     result.confidenceScore);
    }
    
    // OPTION 2: Easy upgrade to enhanced features
    Serial.println("\\n2. Upgrading to enhanced features:");
    if (motionManager.upgradeToEnhanced()) {
        Serial.println("Successfully upgraded to enhanced motion detection!");
        
        // Configure enhanced features
        motionManager.configureEnhancedFeatures(
            true,  // Enable multi-zone PIR
            true,  // Enable advanced analysis
            true   // Enable analytics
        );
        
        // Test enhanced motion detection
        auto enhancedResult = motionManager.detectMotion();
        Serial.printf("Enhanced Motion: %s, Confidence: %.2f\\n", 
                     enhancedResult.motionDetected ? "YES" : "NO", 
                     enhancedResult.confidenceScore);
        
        if (enhancedResult.hasEnhancedData) {
            Serial.printf("Enhanced Data: Zones=%d, Speed=%.1f, Direction=%.1f°, ML=%.2f\\n",
                         enhancedResult.activeZoneCount,
                         enhancedResult.motionSpeed,
                         enhancedResult.motionDirection * 180 / PI,
                         enhancedResult.mlConfidence);
        }
    }
    
    Serial.println("\\n=== Starting Runtime Example ===");
}

void loop() {
    static uint32_t lastDetection = 0;
    uint32_t now = millis();
    
    // Run motion detection every 500ms
    if (now - lastDetection >= 500) {
        lastDetection = now;
        
        // Detect motion using whatever system is configured
        auto result = motionManager.detectMotion();
        
        if (result.motionDetected) {
            Serial.printf("[%lu] MOTION DETECTED!\\n", now);
            Serial.printf("  Confidence: %.2f\\n", result.confidenceScore);
            Serial.printf("  PIR: %s, Frame: %s\\n",
                         result.pirTriggered ? "YES" : "NO",
                         result.frameMotionDetected ? "YES" : "NO");
            Serial.printf("  Process time: %lums\\n", result.processTime);
            Serial.printf("  Description: %s\\n", result.description.c_str());
            
            // Enhanced data (only available if using enhanced system)
            if (result.hasEnhancedData) {
                Serial.printf("  Enhanced Features:\\n");
                if (result.activeZoneCount > 0) {
                    Serial.printf("    Active zones: %d\\n", result.activeZoneCount);
                }
                if (result.motionSpeed > 0) {
                    Serial.printf("    Speed: %.1f units/sec\\n", result.motionSpeed);
                    Serial.printf("    Direction: %.1f degrees\\n", result.motionDirection * 180 / PI);
                }
                if (result.dwellTime > 0) {
                    Serial.printf("    Dwell time: %.1f seconds\\n", result.dwellTime / 1000.0f);
                }
                if (result.mlConfidence > 0) {
                    Serial.printf("    ML confidence: %.2f\\n", result.mlConfidence);
                    if (result.falsePositivePrediction) {
                        Serial.println("    Warning: Possible false positive");
                    }
                }
                if (result.isNewObject) {
                    Serial.println("    New object detected");
                }
            }
            
            Serial.println();
        }
        
        // Print analytics summary every 30 seconds
        static uint32_t lastAnalytics = 0;
        if (now - lastAnalytics >= 30000) {
            lastAnalytics = now;
            
            if (motionManager.hasEnhancedFeatures()) {
                Serial.println("=== Analytics Summary ===");
                Serial.println(motionManager.getAnalyticsSummary());
                Serial.println("========================");
            }
        }
    }
    
    // Example of dynamic configuration changes
    static uint32_t lastConfigChange = 0;
    if (now - lastConfigChange >= 60000) { // Every minute
        lastConfigChange = now;
        
        // Example: Switch to low power mode during certain conditions
        static bool lowPowerMode = false;
        lowPowerMode = !lowPowerMode;
        
        Serial.printf("Switching to %s power mode\\n", lowPowerMode ? "low" : "high");
        motionManager.setLowPowerMode(lowPowerMode);
    }
    
    delay(10); // Small delay to prevent watchdog issues
}

/**
 * MIGRATION EXAMPLES:
 * 
 * BEFORE (Existing Code):
 * ```cpp
 * HybridMotionDetector detector;
 * detector.initialize(&camera);
 * auto result = detector.detectMotion();
 * if (result.motionDetected) {
 *     // Handle motion
 * }
 * ```
 * 
 * AFTER (Drop-in replacement):
 * ```cpp
 * MotionDetectionManager manager;
 * manager.initialize(&camera, MotionDetectionManager::DetectionSystem::LEGACY_HYBRID);
 * auto result = manager.detectMotion();
 * if (result.motionDetected) {
 *     // Handle motion - same code!
 * }
 * ```
 * 
 * ENHANCED (Gradual upgrade):
 * ```cpp
 * MotionDetectionManager manager;
 * manager.initialize(&camera, MotionDetectionManager::DetectionSystem::ENHANCED_HYBRID);
 * manager.configureEnhancedFeatures();
 * auto result = manager.detectMotion();
 * if (result.motionDetected) {
 *     // Handle motion with enhanced data
 *     if (result.hasEnhancedData) {
 *         // Use new features: zones, speed, direction, ML confidence, etc.
 *     }
 * }
 * ```
 */