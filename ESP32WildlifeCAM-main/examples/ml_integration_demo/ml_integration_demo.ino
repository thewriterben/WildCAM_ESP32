/**
 * ML Integration Demo for ESP32 Wildlife Camera
 * 
 * Demonstrates the complete machine learning integration including:
 * - Model loading
 * - Image capture
 * - On-device inference
 * - Classification results
 * 
 * This example shows how to use the Wildlife Classifier for
 * automated species identification.
 */

#include <Arduino.h>
#include "esp_camera.h"

// NOTE: This is a demonstration sketch showing the ML integration API
// The actual implementation requires the full firmware libraries
// See the main project for complete integration

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
    
    // Use VGA for faster processing in demo
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    Serial.println("✓ Camera initialized successfully");
    return true;
}

/**
 * Demonstrate ML integration workflow
 */
void demonstrateMLWorkflow() {
    Serial.println("\n=== ML Integration Workflow Demo ===");
    Serial.println();
    
    Serial.println("1. Camera Capture");
    Serial.println("   ↓");
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("   ✗ Camera capture failed");
        return;
    }
    Serial.printf("   ✓ Image captured: %dx%d (%d bytes)\n", 
                 fb->width, fb->height, fb->len);
    
    Serial.println();
    Serial.println("2. Preprocessing");
    Serial.println("   - Resize to 224x224");
    Serial.println("   - Normalize pixel values");
    Serial.println("   - Convert to RGB format");
    Serial.println("   ↓");
    
    Serial.println();
    Serial.println("3. AI Inference (Simulated)");
    Serial.println("   - Load TFLite model");
    Serial.println("   - Run inference on preprocessed image");
    Serial.println("   - Get classification probabilities");
    
    // Simulate inference time
    delay(500);
    Serial.println("   ✓ Inference completed in ~850ms");
    Serial.println("   ↓");
    
    Serial.println();
    Serial.println("4. Classification Results");
    Serial.println("   Species: White-tailed Deer");
    Serial.println("   Confidence: 92.3%");
    Serial.println("   Dangerous: No");
    Serial.println("   Estimated size: 68.5 kg");
    Serial.println("   ↓");
    
    Serial.println();
    Serial.println("5. Post-Processing");
    Serial.println("   - Log detection");
    Serial.println("   - Update statistics");
    Serial.println("   - Save metadata");
    Serial.println("   - Trigger alerts if needed");
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    Serial.println();
    Serial.println("✓ Complete workflow demonstrated");
    Serial.println();
}

/**
 * Print ML integration architecture
 */
void printArchitecture() {
    Serial.println("\n=== ML Integration Architecture ===");
    Serial.println();
    Serial.println("┌─────────────────────────────────────┐");
    Serial.println("│     Motion Detection (PIR)          │");
    Serial.println("└──────────────┬──────────────────────┘");
    Serial.println("               ↓");
    Serial.println("┌─────────────────────────────────────┐");
    Serial.println("│     Camera Manager                  │");
    Serial.println("│     - Capture image                 │");
    Serial.println("│     - Frame buffer management       │");
    Serial.println("└──────────────┬──────────────────────┘");
    Serial.println("               ↓");
    Serial.println("┌─────────────────────────────────────┐");
    Serial.println("│     Inference Engine                │");
    Serial.println("│     - TensorFlow Lite Micro         │");
    Serial.println("│     - Model loading                 │");
    Serial.println("│     - Memory management             │");
    Serial.println("└──────────────┬──────────────────────┘");
    Serial.println("               ↓");
    Serial.println("┌─────────────────────────────────────┐");
    Serial.println("│     Wildlife Classifier             │");
    Serial.println("│     - Species identification        │");
    Serial.println("│     - Confidence scoring            │");
    Serial.println("│     - Environmental adaptation      │");
    Serial.println("└──────────────┬──────────────────────┘");
    Serial.println("               ↓");
    Serial.println("┌─────────────────────────────────────┐");
    Serial.println("│     Result Processing               │");
    Serial.println("│     - Logging                       │");
    Serial.println("│     - Data collection               │");
    Serial.println("│     - Alert triggering              │");
    Serial.println("└─────────────────────────────────────┘");
    Serial.println();
}

/**
 * Print API usage examples
 */
void printAPIExamples() {
    Serial.println("\n=== ML Integration API Examples ===");
    Serial.println();
    
    Serial.println("// 1. Initialize Classifier");
    Serial.println("WildlifeClassifier classifier;");
    Serial.println("WildlifeClassifierConfig config;");
    Serial.println("config.confidenceThreshold = 0.7;");
    Serial.println("classifier.init(config);");
    Serial.println();
    
    Serial.println("// 2. Capture and Classify");
    Serial.println("camera_fb_t* fb = esp_camera_fb_get();");
    Serial.println("ClassificationResult result = classifier.classifyFrame(fb);");
    Serial.println();
    
    Serial.println("// 3. Process Results");
    Serial.println("if (result.isValid) {");
    Serial.println("  Serial.printf(\"Detected: %s\\n\", result.speciesName.c_str());");
    Serial.println("  Serial.printf(\"Confidence: %.1f%%\\n\", result.confidence * 100);");
    Serial.println("}");
    Serial.println();
    
    Serial.println("// 4. Check for Dangerous Species");
    Serial.println("if (WildlifeClassifier::isDangerousSpecies(result.species)) {");
    Serial.println("  triggerAlert(result);");
    Serial.println("}");
    Serial.println();
}

/**
 * Setup
 */
void setup() {
    // Initialize serial
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.println("║  ESP32 Wildlife Camera - ML Integration   ║");
    Serial.println("║           Demo & Documentation             ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.println();
    
    // Check PSRAM
    if (psramFound()) {
        Serial.printf("✓ PSRAM found: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("⚠️  PSRAM not found - ML features require PSRAM");
    }
    Serial.println();
    
    // Initialize camera for demonstration
    if (!initCamera()) {
        Serial.println("✗ Camera initialization failed!");
        while (true) delay(1000);
    }
    
    Serial.println();
    Serial.println("╔════════════════════════════════════════════╗");
    Serial.println("║           DEMONSTRATION MODE               ║");
    Serial.println("╚════════════════════════════════════════════╝");
    Serial.println();
    Serial.println("This is a demonstration showing the ML");
    Serial.println("integration workflow and API usage.");
    Serial.println();
    Serial.println("For full ML functionality, please:");
    Serial.println("1. Train a model using backend/ml/ tools");
    Serial.println("2. Deploy model to firmware/models/");
    Serial.println("3. Use the complete firmware in src/main.cpp");
    Serial.println();
    Serial.println("See: docs/ml_integration_guide.md");
    Serial.println();
    
    Serial.println("\nCommands:");
    Serial.println("  w - Show workflow");
    Serial.println("  a - Show architecture");
    Serial.println("  x - Show API examples");
    Serial.println("  h - Help");
    Serial.println();
}

/**
 * Main loop
 */
void loop() {
    // Check for serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'w':
            case 'W':
                demonstrateMLWorkflow();
                break;
                
            case 'a':
            case 'A':
                printArchitecture();
                break;
                
            case 'x':
            case 'X':
                printAPIExamples();
                break;
                
            case 'h':
            case 'H':
                Serial.println("\nCommands:");
                Serial.println("  w - Show workflow");
                Serial.println("  a - Show architecture");
                Serial.println("  x - Show API examples");
                Serial.println("  h - Help");
                break;
        }
    }
    
    delay(100);
}
