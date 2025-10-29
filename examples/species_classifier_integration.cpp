/**
 * Species Classifier Models Integration Example
 * 
 * Demonstrates how to use the complete model library with different
 * deployment strategies and use cases.
 * 
 * Model Library:
 * - wildlife_classifier_v2: Main production model (186 KB, 36 species)
 * - deer_classifier: Specialized deer detection (30 KB, 6 species)
 * - bird_classifier: Bird species identification (36 KB, 7 species)
 * - mammal_classifier: Small-medium mammals (36 KB, 7 species)
 * - predator_classifier: Large predator detection (36 KB, 7 species)
 */

#include <Arduino.h>
#include "esp_camera.h"
#include "firmware/src/ai/wildlife_classifier.h"
#include "firmware/src/ai/vision/species_classifier.h"

// Configuration
#define USE_COMPREHENSIVE_MODEL   // Comment out to use specialized models
#define ENABLE_PREDATOR_ALERTS
#define CONFIDENCE_THRESHOLD 0.7

// Model paths
const char* PRODUCTION_MODEL = "/models/production/wildlife_classifier_v2_quantized.tflite";
const char* DEER_MODEL = "/models/species_detection/deer/deer_classifier.tflite";
const char* BIRD_MODEL = "/models/species_detection/bird/bird_classifier.tflite";
const char* MAMMAL_MODEL = "/models/species_detection/mammal/mammal_classifier.tflite";
const char* PREDATOR_MODEL = "/models/species_detection/predator/predator_classifier.tflite";

// Global classifier instances
WildlifeClassifier mainClassifier;
SpeciesClassifier specializedClassifier;

/**
 * STRATEGY 1: Single Comprehensive Model
 * 
 * Use the main production model for all wildlife classification.
 * Best for general-purpose monitoring and research.
 */
void strategy1_comprehensive_model() {
    Serial.println("\n=== Strategy 1: Comprehensive Model ===");
    
    // Configure classifier
    WildlifeClassifier::ClassifierConfig config;
    config.confidenceThreshold = CONFIDENCE_THRESHOLD;
    config.enableEnvironmentalAdaptation = true;
    
    // Initialize with production model
    if (!mainClassifier.initialize(config)) {
        Serial.println("❌ Failed to initialize classifier");
        return;
    }
    
    Serial.println("✅ Classifier initialized with 36-species model");
    Serial.println("   Model: wildlife_classifier_v2");
    Serial.println("   Species coverage: Comprehensive (deer, predators, birds, mammals)");
    Serial.println("   Inference time: ~850ms");
}

/**
 * STRATEGY 2: Cascaded Specialized Models
 * 
 * Use specialized models for higher accuracy on specific species.
 * Best for focused monitoring (e.g., deer hunting season).
 */
void strategy2_cascaded_models() {
    Serial.println("\n=== Strategy 2: Cascaded Specialized Models ===");
    
    // Initialize primary comprehensive model
    WildlifeClassifier::ClassifierConfig config;
    config.confidenceThreshold = 0.6;  // Lower threshold for first pass
    
    if (!mainClassifier.initialize(config)) {
        Serial.println("❌ Failed to initialize primary classifier");
        return;
    }
    
    Serial.println("✅ Primary classifier initialized");
    Serial.println("✅ Specialized models ready for cascading");
    Serial.println("   - Deer model for cervid specialization");
    Serial.println("   - Predator model for safety alerts");
    Serial.println("   - Bird model for avian monitoring");
}

/**
 * STRATEGY 3: Power-Optimized Deployment
 * 
 * Use smallest specialized model with motion triggers.
 * Best for remote, battery-powered deployments.
 */
void strategy3_power_optimized() {
    Serial.println("\n=== Strategy 3: Power-Optimized ===");
    
    // Load only predator model for safety monitoring
    specializedClassifier.init();
    
    if (!specializedClassifier.loadModel(PREDATOR_MODEL)) {
        Serial.println("❌ Failed to load predator model");
        return;
    }
    
    Serial.println("✅ Power-optimized configuration");
    Serial.println("   Model: predator_classifier (36 KB)");
    Serial.println("   Power savings: ~30% vs comprehensive model");
    Serial.println("   Inference time: ~600ms");
    Serial.println("   Use case: Safety alerts only");
}

/**
 * Process wildlife detection with comprehensive model
 */
void processComprehensiveDetection() {
    Serial.println("\n--- Processing with Comprehensive Model ---");
    
    // Capture frame from camera
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("❌ Camera capture failed");
        return;
    }
    
    // Classify
    auto result = mainClassifier.classifyFrame(fb);
    
    if (result.isValid) {
        Serial.printf("✅ Detection: %s\n", result.speciesName.c_str());
        Serial.printf("   Confidence: %.1f%%\n", result.confidence * 100);
        Serial.printf("   Category: %s\n", result.category.c_str());
        
        // Check for dangerous species
        if (result.isDangerous) {
            Serial.println("   ⚠️  DANGEROUS SPECIES - Alert triggered");
            triggerSafetyAlert(result.speciesName, result.confidence);
        }
        
        // Log detection
        logDetection(result);
    } else {
        Serial.println("❌ No valid detection");
    }
    
    // Return frame buffer
    esp_camera_fb_return(fb);
}

/**
 * Process with cascaded models for higher accuracy
 */
void processCascadedDetection() {
    Serial.println("\n--- Processing with Cascaded Models ---");
    
    // Step 1: Primary classification
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("❌ Camera capture failed");
        return;
    }
    
    auto primaryResult = mainClassifier.classifyFrame(fb);
    
    if (!primaryResult.isValid) {
        esp_camera_fb_return(fb);
        Serial.println("❌ No detection in primary pass");
        return;
    }
    
    Serial.printf("Primary: %s (%.1f%%)\n", 
                 primaryResult.speciesName.c_str(),
                 primaryResult.confidence * 100);
    
    // Step 2: Specialized classification based on category
    const char* specializedModel = nullptr;
    
    if (primaryResult.category == "deer" || primaryResult.category == "cervid") {
        specializedModel = DEER_MODEL;
        Serial.println("→ Using deer specialist");
    }
    else if (primaryResult.category == "bird") {
        specializedModel = BIRD_MODEL;
        Serial.println("→ Using bird specialist");
    }
    else if (primaryResult.category == "predator") {
        specializedModel = PREDATOR_MODEL;
        Serial.println("→ Using predator specialist");
    }
    else if (primaryResult.category == "mammal") {
        specializedModel = MAMMAL_MODEL;
        Serial.println("→ Using mammal specialist");
    }
    
    // Run specialized model if applicable
    if (specializedModel) {
        specializedClassifier.loadModel(specializedModel);
        
        CameraFrame frame = {fb->buf, fb->width, fb->height, fb->format};
        auto specializedResult = specializedClassifier.classify(frame);
        
        if (specializedResult.confidence > primaryResult.confidence) {
            Serial.printf("✅ Refined: %s (%.1f%% ↑)\n",
                         specializedResult.species_name.c_str(),
                         specializedResult.confidence * 100);
            
            // Use refined result
            logRefinedDetection(specializedResult);
        } else {
            Serial.println("   Primary result maintained");
        }
    }
    
    esp_camera_fb_return(fb);
}

/**
 * Power-optimized predator monitoring
 */
void processPreatoryMonitoring() {
    Serial.println("\n--- Predator Monitoring (Power-Optimized) ---");
    
    // Only process if motion detected (saves power)
    if (!motionDetected()) {
        return;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return;
    
    CameraFrame frame = {fb->buf, fb->width, fb->height, fb->format};
    auto result = specializedClassifier.classify(frame);
    
    if (result.confidence > 0.8) {
        Serial.printf("⚠️  PREDATOR ALERT: %s\n", result.species_name.c_str());
        Serial.printf("   Confidence: %.1f%%\n", result.confidence * 100);
        
        // Trigger alert
        triggerSafetyAlert(result.species_name.c_str(), result.confidence);
        
        // Log critical detection
        logCriticalDetection(result);
    }
    
    esp_camera_fb_return(fb);
}

/**
 * Helper Functions
 */

void triggerSafetyAlert(String species, float confidence) {
    // Send alert via LoRa, WiFi, or satellite
    Serial.printf("📡 Alert: %s detected (%.1f%% confidence)\n", 
                 species.c_str(), confidence * 100);
    
    // Flash LED, sound alarm, etc.
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
}

void logDetection(WildlifeClassifier::ClassificationResult result) {
    // Log to SD card, send to cloud, etc.
    Serial.println("📝 Logging detection...");
    
    // Format: timestamp, species, confidence, location
    // Save to: /sd/detections/2025-10-29.csv
}

void logRefinedDetection(SpeciesClassifier::ClassificationResult result) {
    Serial.println("📝 Logging refined detection...");
}

void logCriticalDetection(SpeciesClassifier::ClassificationResult result) {
    Serial.println("📝 Logging critical detection...");
}

bool motionDetected() {
    // Check PIR sensor or frame differencing
    return digitalRead(PIR_PIN) == HIGH;
}

/**
 * Setup and Main Loop
 */

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n");
    Serial.println("╔════════════════════════════════════════════════════════╗");
    Serial.println("║    WildCAM ESP32 Species Classifier Integration       ║");
    Serial.println("║    Model Library v2.0.0                                ║");
    Serial.println("╚════════════════════════════════════════════════════════╝");
    
    // Initialize camera
    Serial.println("\nInitializing camera...");
    if (!initCamera()) {
        Serial.println("❌ Camera initialization failed");
        return;
    }
    Serial.println("✅ Camera ready");
    
    // Initialize pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIR_PIN, INPUT);
    
    // Choose deployment strategy
    #ifdef USE_COMPREHENSIVE_MODEL
        strategy1_comprehensive_model();
    #else
        strategy2_cascaded_models();
        // or strategy3_power_optimized();
    #endif
    
    Serial.println("\n🚀 System ready - starting wildlife monitoring\n");
}

void loop() {
    // Choose processing method based on strategy
    #ifdef USE_COMPREHENSIVE_MODEL
        processComprehensiveDetection();
    #else
        processCascadedDetection();
        // or processPredatorMonitoring();
    #endif
    
    // Wait before next detection
    delay(5000);
}

/**
 * Camera Initialization
 */

bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA;  // 320x240
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    return true;
}

/**
 * Example Output:
 * 
 * === Strategy 1: Comprehensive Model ===
 * ✅ Classifier initialized with 36-species model
 *    Model: wildlife_classifier_v2
 *    Species coverage: Comprehensive
 *    Inference time: ~850ms
 * 
 * --- Processing with Comprehensive Model ---
 * ✅ Detection: white_tailed_deer
 *    Confidence: 87.3%
 *    Category: cervid
 * 📝 Logging detection...
 * 
 * --- Processing with Comprehensive Model ---
 * ✅ Detection: black_bear
 *    Confidence: 92.1%
 *    Category: predator
 *    ⚠️  DANGEROUS SPECIES - Alert triggered
 * 📡 Alert: black_bear detected (92.1% confidence)
 * 📝 Logging detection...
 */
