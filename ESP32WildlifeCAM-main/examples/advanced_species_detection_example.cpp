/**
 * @file advanced_species_detection_example.cpp
 * @brief Example application demonstrating Advanced Species Detection
 * 
 * This example shows how to use the advanced AI species detection system
 * with 150+ species support, edge computing optimization, and hierarchical
 * taxonomic classification on ESP32-S3 hardware.
 */

#include <Arduino.h>
#include "esp_camera.h"
#include "../firmware/src/ai/advanced_species_detection.h"
#include "../firmware/ml_models/ai_detection_interface.h"

// Camera pins for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Global objects
AdvancedSpeciesDetector* detector = nullptr;
WildlifeDetection::AIDetectionInterface* aiInterface = nullptr;

/**
 * Initialize camera
 */
bool initCamera() {
    Serial.println("Initializing camera...");
    
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
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Use higher resolution for better detection
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;  // 800x600
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    Serial.println("Camera initialized successfully");
    return true;
}

/**
 * Initialize advanced species detection
 */
bool initAdvancedDetection() {
    Serial.println("\n=== Initializing Advanced Species Detection ===");
    
    // Create detector instance
    detector = new AdvancedSpeciesDetector();
    if (!detector) {
        Serial.println("ERROR: Failed to create detector instance");
        return false;
    }
    
    // Configure for optimal performance
    AdvancedDetectionConfig config;
    config.modelPath = "wildlife_research_v3.tflite";
    config.confidenceThreshold = 0.80f;  // 80% confidence threshold
    config.enableEdgeOptimization = true;
    config.enableHierarchicalClassification = true;
    config.enableSubspeciesDetection = true;
    config.enableSizeEstimation = true;
    config.enableAgeEstimation = true;
    config.region = "north_america";
    
    // Check hardware
    Serial.println("\nHardware Check:");
    Serial.printf("  CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    if (psramFound()) {
        Serial.printf("  PSRAM: %d bytes\n", ESP.getPsramSize());
        Serial.printf("  Free PSRAM: %d bytes\n", ESP.getFreePsram());
    } else {
        Serial.println("  WARNING: PSRAM not found - detection may fail");
    }
    
    // Initialize detector
    if (!detector->initialize(config)) {
        Serial.println("ERROR: Failed to initialize detector");
        delete detector;
        detector = nullptr;
        return false;
    }
    
    Serial.println("\n✓ Advanced Species Detector initialized successfully");
    
    // Run self-test
    if (detector->runSelfTest()) {
        Serial.println("✓ Self-test passed");
    } else {
        Serial.println("✗ Self-test failed");
    }
    
    // Print model info
    ModelInfo modelInfo = detector->getModelInfo();
    Serial.println("\nModel Information:");
    Serial.printf("  Name: %s\n", modelInfo.name.c_str());
    Serial.printf("  Version: %s\n", modelInfo.version.c_str());
    Serial.printf("  Accuracy: %.1f%%\n", modelInfo.accuracy * 100.0f);
    Serial.printf("  Size: %d KB\n", modelInfo.modelSize / 1024);
    
    return true;
}

/**
 * Initialize AI Detection Interface
 */
bool initAIInterface() {
    Serial.println("\n=== Initializing AI Detection Interface ===");
    
    aiInterface = new WildlifeDetection::AIDetectionInterface();
    if (!aiInterface) {
        Serial.println("ERROR: Failed to create AI interface");
        return false;
    }
    
    // Configure interface
    WildlifeDetection::AIDetectionInterface::AdvancedConfig config;
    config.use_tensorflow_lite = true;
    config.enable_advanced_species_detection = true;
    config.enable_behavior_analysis = false;
    config.ai_confidence_threshold = 0.80f;
    config.ai_processing_timeout_ms = 6000;
    
    aiInterface->configureAdvancedAI(config);
    
    // Initialize
    if (!aiInterface->initializeAdvancedAI()) {
        Serial.println("WARNING: Advanced AI not fully available");
    }
    
    // Check capabilities
    if (aiInterface->hasAdvancedSpeciesDetection()) {
        Serial.println("✓ Advanced species detection (150+ species) enabled");
    }
    
    // List available models
    std::vector<const char*> models = aiInterface->getAvailableModels();
    Serial.println("\nAvailable AI Models:");
    for (const char* model : models) {
        Serial.printf("  - %s\n", model);
    }
    
    return true;
}

/**
 * Capture and analyze frame
 */
void captureAndAnalyze() {
    Serial.println("\n========================================");
    Serial.println("Capturing frame for analysis...");
    
    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("ERROR: Camera capture failed");
        return;
    }
    
    Serial.printf("Captured: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
    
    // Create camera frame structure
    CameraFrame frame;
    frame.data = fb->buf;
    frame.length = fb->len;
    frame.width = fb->width;
    frame.height = fb->height;
    frame.format = fb->format;
    frame.timestamp = millis();
    
    // Run detection
    unsigned long startTime = millis();
    AdvancedSpeciesResult result = detector->detectSpecies(frame);
    unsigned long detectionTime = millis() - startTime;
    
    // Display results
    Serial.println("\n--- Detection Results ---");
    Serial.printf("Processing Time: %lu ms\n", detectionTime);
    Serial.printf("Inference Time: %u ms\n", result.inferenceTime_ms);
    
    if (result.isValidDetection) {
        Serial.println("\n✓ SPECIES DETECTED");
        Serial.printf("Primary Species: %s\n", 
                     advancedSpeciesToString(result.primarySpecies));
        Serial.printf("Confidence: %.1f%%\n", result.primaryConfidence * 100.0f);
        
        if (result.secondaryConfidence > 0.5f) {
            Serial.printf("Alternative: %s (%.1f%%)\n",
                         advancedSpeciesToString(result.secondarySpecies),
                         result.secondaryConfidence * 100.0f);
        }
        
        // Taxonomic information
        Serial.println("\n--- Taxonomic Classification ---");
        Serial.printf("Common Name: %s\n", result.taxonomy.commonName.c_str());
        Serial.printf("Class: %s\n", result.taxonomy.className.c_str());
        Serial.printf("Order: %s\n", result.taxonomy.order.c_str());
        Serial.printf("Family: %s\n", result.taxonomy.family.c_str());
        Serial.printf("Genus: %s\n", result.taxonomy.genus.c_str());
        Serial.printf("Species: %s\n", result.taxonomy.species.c_str());
        
        if (result.taxonomy.subspecies.length() > 0) {
            Serial.printf("Subspecies: %s\n", result.taxonomy.subspecies.c_str());
        }
        
        // Physical characteristics
        Serial.println("\n--- Physical Characteristics ---");
        if (result.sizeEstimate_cm > 0) {
            Serial.printf("Estimated Size: %.1f cm\n", result.sizeEstimate_cm);
        }
        Serial.printf("Age: %s\n", result.isJuvenile ? "Juvenile" : "Adult");
        Serial.printf("Count: %d individual(s)\n", result.animalCount);
        
        // Bounding box
        Serial.printf("Location: [%d, %d, %dx%d]\n",
                     result.boundingBox[0], result.boundingBox[1],
                     result.boundingBox[2], result.boundingBox[3]);
        
        // Conservation status
        if (isEndangeredSpecies(result.primarySpecies)) {
            Serial.println("\n⚠️  ENDANGERED SPECIES ALERT");
            Serial.printf("Conservation Status: %s\n",
                         getConservationStatus(result.primarySpecies).c_str());
        }
        
        // Typical species information
        float typicalSize = getTypicalSpeciesSize(result.primarySpecies);
        Serial.printf("\nTypical Size: %.1f cm\n", typicalSize);
        
    } else {
        Serial.println("\n✗ NO VALID DETECTION");
        Serial.printf("Confidence too low: %.1f%%\n", result.primaryConfidence * 100.0f);
    }
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    // Display performance metrics
    AIMetrics metrics = detector->getPerformanceMetrics();
    Serial.println("\n--- Performance Metrics ---");
    Serial.printf("Total Detections: %u\n", metrics.totalInferences);
    Serial.printf("Successful: %u\n", metrics.successfulInferences);
    Serial.printf("Success Rate: %.1f%%\n", 
                 detector->getOverallAccuracy() * 100.0f);
    Serial.printf("Avg Inference Time: %.1f ms\n", metrics.inferenceTime);
    
    Serial.println("========================================\n");
}

/**
 * Demonstrate different inference modes
 */
void demonstrateInferenceModes() {
    Serial.println("\n=== Testing Different Inference Modes ===\n");
    
    const char* modes[] = {"fast", "balanced", "accurate"};
    
    for (const char* mode : modes) {
        Serial.printf("--- %s mode ---\n", mode);
        detector->setInferenceMode(mode);
        
        // Capture and analyze
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            CameraFrame frame;
            frame.data = fb->buf;
            frame.length = fb->len;
            frame.width = fb->width;
            frame.height = fb->height;
            
            unsigned long startTime = millis();
            AdvancedSpeciesResult result = detector->detectSpecies(frame);
            unsigned long time = millis() - startTime;
            
            Serial.printf("Processing time: %lu ms\n", time);
            Serial.printf("Confidence: %.1f%%\n", result.primaryConfidence * 100.0f);
            
            esp_camera_fb_return(fb);
        }
        
        delay(1000);
    }
}

/**
 * Demonstrate power optimization
 */
void demonstratePowerOptimization() {
    Serial.println("\n=== Testing Power Optimization ===\n");
    
    float batteryLevels[] = {1.0f, 0.7f, 0.4f, 0.15f};
    
    for (float level : batteryLevels) {
        Serial.printf("--- Battery Level: %.0f%% ---\n", level * 100.0f);
        detector->optimizeForPower(level);
        delay(500);
    }
}

/**
 * Display system information
 */
void displaySystemInfo() {
    Serial.println("\n========================================");
    Serial.println("System Information");
    Serial.println("========================================");
    
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Cores: %d\n", ESP.getChipCores());
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    if (psramFound()) {
        Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    } else {
        Serial.println("PSRAM: Not available");
    }
    
    Serial.println("========================================\n");
}

// ===========================================
// ARDUINO SETUP AND LOOP
// ===========================================

void setup() {
    // Initialize serial
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    delay(2000);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("Advanced Species Detection Example");
    Serial.println("ESP32 Wildlife Camera System");
    Serial.println("========================================\n");
    
    // Display system info
    displaySystemInfo();
    
    // Set CPU to maximum frequency
    setCpuFrequencyMhz(240);
    Serial.println("CPU set to 240 MHz for optimal performance\n");
    
    // Initialize camera
    if (!initCamera()) {
        Serial.println("FATAL: Camera initialization failed");
        while (1) {
            delay(1000);
        }
    }
    
    // Initialize AI detection interface
    if (!initAIInterface()) {
        Serial.println("WARNING: AI interface initialization failed");
    }
    
    // Initialize advanced detection
    if (!initAdvancedDetection()) {
        Serial.println("FATAL: Advanced detection initialization failed");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("\n✓ All systems initialized successfully!");
    Serial.println("\nStarting automated detection cycle...\n");
    
    // Demonstrate features
    demonstrateInferenceModes();
    demonstratePowerOptimization();
    
    delay(2000);
}

void loop() {
    // Capture and analyze every 10 seconds
    captureAndAnalyze();
    
    // Wait before next capture
    delay(10000);
}
