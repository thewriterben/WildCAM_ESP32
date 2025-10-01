/**
 * @file advanced_wildlife_monitoring_example.cpp
 * @brief Complete example of advanced wildlife monitoring features
 * 
 * Demonstrates:
 * - AI species classification with 20+ species
 * - Dangerous species detection and alerts
 * - Motion pattern analysis with temporal analysis
 * - LoRa mesh network integration for alerts
 * - Real-time wildlife behavior monitoring
 */

#include <Arduino.h>
#include "../src/ai/wildlife_classifier.h"
#include "../src/ai/dangerous_species_alert.h"
#include "../src/detection/motion_pattern_analyzer.h"
#include "../src/detection/hybrid_motion_detector.h"
#include "../firmware/src/lora_wildlife_alerts.h"
#include "../src/camera/camera_manager.h"

// System components
WildlifeClassifier classifier;
DangerousSpeciesAlertSystem alertSystem;
MotionPatternAnalyzer patternAnalyzer;
HybridMotionDetector motionDetector;
CameraManager cameraManager;

// Configuration
const float CLASSIFICATION_CONFIDENCE_THRESHOLD = 0.70f;
const float ALERT_CONFIDENCE_THRESHOLD = 0.70f;
const uint32_t MIN_CONSECUTIVE_DETECTIONS = 2;

// Statistics
struct SystemStats {
    uint32_t totalDetections = 0;
    uint32_t animalDetections = 0;
    uint32_t dangerousSpeciesDetections = 0;
    uint32_t alertsSent = 0;
    uint32_t falsePositives = 0;
    uint32_t systemUptime = 0;
};

SystemStats stats;

/**
 * Alert callback for received LoRa alerts
 */
void onWildlifeAlertReceived(const WildlifeAlertMessage& alert) {
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║     WILDLIFE ALERT RECEIVED              ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.printf("  Species: %s\n", alert.speciesName);
    Serial.printf("  Confidence: %.1f%%\n", alert.confidence * 100.0f);
    Serial.printf("  Priority: %d\n", alert.alertPriority);
    Serial.printf("  Source Node: %d\n", alert.sourceNodeId);
    Serial.printf("  Location: %.6f, %.6f\n", alert.latitude, alert.longitude);
    Serial.println("══════════════════════════════════════════\n");
    
    // Trigger local actions based on alert priority
    if (alert.alertPriority >= 2) { // HIGH or CRITICAL
        Serial.println("⚠️  HIGH PRIORITY ALERT - Taking precautions!");
        // Could trigger local alarm, LED indicators, etc.
    }
}

/**
 * Initialize all system components
 */
bool initializeSystem() {
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║  WildCAM ESP32 - Advanced Monitoring    ║");
    Serial.println("╚══════════════════════════════════════════╝\n");
    
    // Initialize camera
    Serial.println("Initializing camera system...");
    if (!cameraManager.initialize()) {
        Serial.println("❌ Camera initialization failed!");
        return false;
    }
    Serial.println("✅ Camera system ready");
    
    // Initialize AI classifier
    Serial.println("\nInitializing AI wildlife classifier...");
    if (!classifier.initialize()) {
        Serial.println("❌ Classifier initialization failed!");
        return false;
    }
    classifier.setConfidenceThreshold(CLASSIFICATION_CONFIDENCE_THRESHOLD);
    Serial.println("✅ AI classifier ready");
    Serial.printf("   Supported species: 20+\n");
    Serial.printf("   Confidence threshold: %.0f%%\n", CLASSIFICATION_CONFIDENCE_THRESHOLD * 100);
    
    // Initialize alert system
    Serial.println("\nInitializing dangerous species alert system...");
    if (!alertSystem.initialize()) {
        Serial.println("❌ Alert system initialization failed!");
        return false;
    }
    alertSystem.setConfidenceThreshold(ALERT_CONFIDENCE_THRESHOLD);
    alertSystem.setMinConsecutiveDetections(MIN_CONSECUTIVE_DETECTIONS);
    Serial.println("✅ Alert system ready");
    Serial.printf("   Alert threshold: %.0f%%\n", ALERT_CONFIDENCE_THRESHOLD * 100);
    Serial.printf("   Min consecutive detections: %d\n", MIN_CONSECUTIVE_DETECTIONS);
    
    // Initialize motion pattern analyzer
    Serial.println("\nInitializing motion pattern analyzer...");
    if (!patternAnalyzer.initialize()) {
        Serial.println("❌ Pattern analyzer initialization failed!");
        return false;
    }
    Serial.println("✅ Motion pattern analyzer ready");
    Serial.println("   Temporal analysis enabled");
    Serial.println("   False positive filtering active");
    
    // Initialize motion detector
    Serial.println("\nInitializing hybrid motion detector...");
    if (!motionDetector.initialize(&cameraManager)) {
        Serial.println("❌ Motion detector initialization failed!");
        return false;
    }
    motionDetector.setDetectionMode(true, false, false); // Hybrid mode
    Serial.println("✅ Motion detector ready");
    
    // Initialize LoRa wildlife alerts
    Serial.println("\nInitializing LoRa wildlife alert system...");
    if (!LoRaWildlifeAlerts::init()) {
        Serial.println("⚠️  LoRa alerts initialization failed (continuing without LoRa)");
    } else {
        LoRaWildlifeAlerts::setAlertCallback(onWildlifeAlertReceived);
        LoRaWildlifeAlerts::setLocation(45.5231, -122.6765); // Example coordinates
        Serial.println("✅ LoRa alert system ready");
        Serial.println("   Mesh networking active");
        Serial.println("   Remote alerts enabled");
    }
    
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║  System Ready - Monitoring Wildlife      ║");
    Serial.println("╚══════════════════════════════════════════╝\n");
    
    return true;
}

/**
 * Process wildlife detection
 */
void processWildlifeDetection() {
    // Step 1: Detect motion
    HybridMotionDetector::HybridResult motionResult = motionDetector.detectMotion();
    
    if (!motionResult.motionDetected) {
        return; // No motion, nothing to process
    }
    
    stats.totalDetections++;
    
    Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("Motion Detected! (Confidence: %.2f)\n", motionResult.confidenceScore);
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Step 2: Analyze motion pattern
    MotionDetection::MotionResult frameMotion = motionResult.frameResult;
    MotionPatternResult patternResult = patternAnalyzer.analyzePattern(frameMotion);
    
    Serial.printf("📊 Pattern Analysis: %s\n", patternResult.description.c_str());
    Serial.printf("   Confidence: %.1f%%\n", patternResult.confidence * 100);
    Serial.printf("   Animal Motion: %s\n", patternResult.isAnimalMotion ? "Yes" : "No");
    
    // Check for false positives
    if (patternResult.pattern == MotionPattern::FALSE_POSITIVE || 
        patternResult.pattern == MotionPattern::ENVIRONMENTAL) {
        Serial.println("⚠️  False positive detected - Skipping classification");
        stats.falsePositives++;
        return;
    }
    
    if (!patternResult.isAnimalMotion) {
        Serial.println("ℹ️  Non-animal motion - Skipping classification");
        return;
    }
    
    stats.animalDetections++;
    
    // Step 3: Capture image and classify wildlife
    Serial.println("\n📸 Capturing image for AI classification...");
    camera_fb_t* frame = cameraManager.captureToBuffer();
    
    if (frame == nullptr) {
        Serial.println("❌ Failed to capture frame");
        return;
    }
    
    WildlifeClassifier::ClassificationResult classResult = classifier.classifyFrame(frame);
    cameraManager.returnFrameBuffer(frame);
    
    if (!classResult.isValid) {
        Serial.println("⚠️  Classification confidence too low");
        return;
    }
    
    // Step 4: Display classification results
    Serial.println("\n🦌 Wildlife Classification Results:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("   Species: %s\n", classResult.speciesName.c_str());
    Serial.printf("   Confidence: %.1f%%\n", classResult.confidence * 100);
    Serial.printf("   Confidence Level: %s\n", 
                 WildlifeClassifier::getConfidenceLevelDescription(classResult.confidenceLevel).c_str());
    Serial.printf("   Inference Time: %d ms\n", classResult.inferenceTime);
    Serial.printf("   Animals Detected: %d\n", classResult.animalCount);
    
    // Step 5: Check for dangerous species
    bool isDangerous = WildlifeClassifier::isDangerousSpecies(classResult.species);
    
    if (isDangerous) {
        Serial.println("   ⚠️  DANGEROUS SPECIES DETECTED!");
        stats.dangerousSpeciesDetections++;
        
        // Process through alert system
        DangerousSpeciesAlert* alert = alertSystem.processClassification(classResult);
        
        if (alert != nullptr) {
            Serial.println("\n🚨 ALERT GENERATED:");
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Serial.printf("   %s\n", alert->alertMessage.c_str());
            Serial.printf("   Priority: %s\n", 
                         alert->priority == AlertPriority::CRITICAL ? "CRITICAL" :
                         alert->priority == AlertPriority::HIGH ? "HIGH" :
                         alert->priority == AlertPriority::MEDIUM ? "MEDIUM" : "LOW");
            Serial.printf("   Immediate Action Required: %s\n", 
                         alert->requiresImmediate ? "YES" : "NO");
            Serial.printf("   Detection Count: %d\n", alert->detectionCount);
            
            // Send alert via LoRa mesh
            if (LoRaWildlifeAlerts::isEnabled()) {
                if (LoRaWildlifeAlerts::sendAlert(*alert)) {
                    Serial.println("   ✅ Alert transmitted via LoRa mesh");
                    stats.alertsSent++;
                } else {
                    Serial.println("   ⚠️  Alert queued for transmission");
                }
            }
            
            Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        } else {
            Serial.println("   ℹ️  Alert criteria not met (cooldown/threshold)");
        }
    }
    
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

/**
 * Print system statistics
 */
void printStatistics() {
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║         System Statistics                ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.printf("System Uptime: %d seconds\n", stats.systemUptime);
    Serial.printf("Total Detections: %d\n", stats.totalDetections);
    Serial.printf("Animal Detections: %d\n", stats.animalDetections);
    Serial.printf("Dangerous Species: %d\n", stats.dangerousSpeciesDetections);
    Serial.printf("Alerts Sent: %d\n", stats.alertsSent);
    Serial.printf("False Positives Filtered: %d\n", stats.falsePositives);
    
    // Classifier statistics
    WildlifeClassifier::ClassifierStats classStats = classifier.getStatistics();
    Serial.printf("\nAI Classifier:\n");
    Serial.printf("  Total Inferences: %d\n", classStats.totalInferences);
    Serial.printf("  Successful: %d\n", classStats.successfulInferences);
    Serial.printf("  Avg Confidence: %.1f%%\n", classStats.averageConfidence * 100);
    Serial.printf("  Avg Inference Time: %d ms\n", classStats.averageInferenceTime);
    
    // Alert system statistics
    Serial.printf("\nAlert System:\n");
    Serial.printf("  Total Alerts: %d\n", alertSystem.getTotalAlerts());
    Serial.printf("  Critical Alerts: %d\n", alertSystem.getCriticalAlerts());
    
    // Motion pattern analyzer statistics
    Serial.printf("\nPattern Analyzer:\n");
    Serial.printf("  Patterns Analyzed: %d\n", patternAnalyzer.getTotalAnalyzed());
    Serial.printf("  False Positives: %d\n", patternAnalyzer.getFalsePositivesDetected());
    
    // LoRa statistics
    if (LoRaWildlifeAlerts::isEnabled()) {
        WildlifeAlertStats loraStats = LoRaWildlifeAlerts::getStatistics();
        Serial.printf("\nLoRa Alert System:\n");
        Serial.printf("  Total Sent: %d\n", loraStats.totalAlertsSent);
        Serial.printf("  Critical Sent: %d\n", loraStats.criticalAlertsSent);
        Serial.printf("  Success Rate: %.1f%%\n", 
                     (loraStats.successfulTransmissions * 100.0f) / 
                     max(1u, loraStats.totalAlertsSent));
    }
    
    Serial.println("══════════════════════════════════════════\n");
}

/**
 * Setup function
 */
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    delay(2000); // Wait for serial monitor
    
    if (!initializeSystem()) {
        Serial.println("\n❌ SYSTEM INITIALIZATION FAILED!");
        Serial.println("Please check connections and try again.");
        while (1) {
            delay(1000);
        }
    }
    
    stats.systemUptime = 0;
}

/**
 * Main loop
 */
void loop() {
    // Update uptime
    stats.systemUptime = millis() / 1000;
    
    // Process wildlife detection
    processWildlifeDetection();
    
    // Print statistics every 5 minutes
    static uint32_t lastStatsTime = 0;
    if (millis() - lastStatsTime > 300000) {
        printStatistics();
        lastStatsTime = millis();
    }
    
    // Small delay to prevent overwhelming the system
    delay(100);
}
