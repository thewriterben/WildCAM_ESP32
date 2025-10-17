/**
 * @file test_model_integration_example.cpp
 * @brief Example demonstrating how to load and use the test species classification model
 * @author WildCAM ESP32 Team
 * @date 2025-10-16
 * 
 * This example shows how to:
 * 1. Initialize the species classifier
 * 2. Load the test model
 * 3. Run inference on camera frames
 * 4. Handle classification results
 * 
 * NOTE: This uses the TEST MODEL which is trained on synthetic data.
 *       For production use, train a real model following TRAINING_DEPLOYMENT_GUIDE.md
 */

#include <Arduino.h>
#include "firmware/src/ai/vision/species_classifier.h"
#include "firmware/src/ai/model_manager.h"

// Global instances
WildlifeClassifier classifier;

// Configuration
const char* TEST_MODEL_PATH = "/models/test/wildlife_classifier_test_quantized.tflite";
const float CONFIDENCE_THRESHOLD = 0.5f;  // 50% minimum confidence

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println("\n=== WildCAM Species Classification Test ===\n");
    
    // Step 1: Initialize classifier
    Serial.println("1. Initializing species classifier...");
    if (!classifier.init()) {
        Serial.println("ERROR: Failed to initialize classifier");
        return;
    }
    Serial.println("   ✓ Classifier initialized");
    
    // Step 2: Load test model
    Serial.println("\n2. Loading test model...");
    Serial.printf("   Model path: %s\n", TEST_MODEL_PATH);
    
    if (!classifier.loadSpeciesModel(TEST_MODEL_PATH)) {
        Serial.println("ERROR: Failed to load test model");
        Serial.println("Verify model file exists at specified path");
        return;
    }
    Serial.println("   ✓ Test model loaded successfully");
    
    // Step 3: Configure classifier
    Serial.println("\n3. Configuring classifier...");
    classifier.setConfidenceThreshold(CONFIDENCE_THRESHOLD);
    Serial.printf("   Confidence threshold: %.2f\n", CONFIDENCE_THRESHOLD);
    
    // Optional: Enable additional features
    classifier.enableSizeEstimation(true);
    classifier.enableAgeEstimation(false);
    classifier.enableTemporalTracking(false);
    Serial.println("   ✓ Configuration complete");
    
    // Step 4: Display supported species
    Serial.println("\n4. Supported species (test model):");
    std::vector<String> species = classifier.getSupportedSpecies();
    for (size_t i = 0; i < species.size(); i++) {
        Serial.printf("   %d. %s\n", i + 1, species[i].c_str());
    }
    
    Serial.println("\n=== Setup Complete ===");
    Serial.println("Ready to classify images\n");
}

void loop() {
    // In a real application, you would capture frames from camera
    // For this example, we'll simulate classification
    
    static unsigned long lastClassification = 0;
    const unsigned long CLASSIFICATION_INTERVAL = 5000;  // 5 seconds
    
    if (millis() - lastClassification >= CLASSIFICATION_INTERVAL) {
        lastClassification = millis();
        
        Serial.println("\n--- Running Classification ---");
        
        // Simulate capturing a camera frame
        // In real code: CameraFrame frame = camera.capture();
        CameraFrame simulatedFrame;
        // ... populate frame data ...
        
        // Classify image
        unsigned long startTime = millis();
        SpeciesResult result = classifier.classifyImage(simulatedFrame);
        unsigned long inferenceTime = millis() - startTime;
        
        // Display results
        Serial.println("\nClassification Results:");
        Serial.printf("  Species: %s\n", result.specificName.c_str());
        Serial.printf("  Confidence: %.2f%%\n", result.confidence * 100.0f);
        Serial.printf("  Confidence Level: %d\n", (int)result.confidenceLevel);
        Serial.printf("  Animal Count: %d\n", result.animalCount);
        Serial.printf("  Size Category: %d\n", result.animalSize);
        Serial.printf("  Is Juvenile: %s\n", result.isJuvenile ? "Yes" : "No");
        Serial.printf("  Bounding Box: [%d, %d, %d, %d]\n",
                     result.boundingBox[0], result.boundingBox[1],
                     result.boundingBox[2], result.boundingBox[3]);
        Serial.printf("  Inference Time: %lu ms\n", inferenceTime);
        
        // Check if detection is valid
        if (result.confidence >= CONFIDENCE_THRESHOLD) {
            Serial.println("\n✓ Valid detection");
            
            // Take action based on species
            handleSpeciesDetection(result);
        } else {
            Serial.println("\n✗ Low confidence - ignoring detection");
        }
        
        // Display performance metrics
        AIMetrics metrics = classifier.getClassificationMetrics();
        Serial.printf("\nPerformance Metrics:\n");
        Serial.printf("  Total Classifications: %lu\n", metrics.totalInferences);
        Serial.printf("  Average Inference Time: %.2f ms\n", metrics.avgInferenceTime);
        Serial.printf("  Memory Usage: %lu KB\n", metrics.memoryUsage / 1024);
    }
    
    delay(100);
}

/**
 * Handle species detection based on result
 */
void handleSpeciesDetection(const SpeciesResult& result) {
    // Example actions based on detected species
    
    if (result.species == SpeciesType::DEER) {
        Serial.println("Action: Log deer sighting");
        // Log to SD card, send notification, etc.
        
    } else if (result.species == SpeciesType::BEAR) {
        Serial.println("Action: ALERT - Bear detected!");
        // Send high-priority alert
        // Activate deterrent systems if configured
        
    } else if (result.species == SpeciesType::PREDATOR) {
        Serial.println("Action: Log predator activity");
        // Track predator patterns
        
    } else if (result.species == SpeciesType::HUMAN) {
        Serial.println("Action: Log human presence");
        // Security logging
        
    } else {
        Serial.printf("Action: Log %s sighting\n", result.specificName.c_str());
    }
    
    // Common actions for all detections
    saveDetectionToSD(result);
    updateStatistics(result);
    
    // Optional: Send to cloud
    if (WiFi.status() == WL_CONNECTED) {
        sendDetectionToCloud(result);
    }
}

/**
 * Save detection to SD card
 */
void saveDetectionToSD(const SpeciesResult& result) {
    Serial.println("  → Saving to SD card");
    // Implementation would write to SD card
    // Format: timestamp, species, confidence, location, etc.
}

/**
 * Update detection statistics
 */
void updateStatistics(const SpeciesResult& result) {
    Serial.println("  → Updating statistics");
    // Track detections per species
    // Calculate trends
    // Update dashboard data
}

/**
 * Send detection to cloud backend
 */
void sendDetectionToCloud(const SpeciesResult& result) {
    Serial.println("  → Sending to cloud");
    // POST to API endpoint
    // Include image thumbnail if available
    // Send metadata (GPS, weather, etc.)
}

/**
 * Example: Batch classification of multiple images
 */
void batchClassificationExample() {
    Serial.println("\n=== Batch Classification Example ===");
    
    // Simulate multiple frames
    const int NUM_FRAMES = 5;
    std::vector<CameraFrame> frames;
    
    // ... populate frames ...
    
    Serial.printf("Processing %d frames...\n", NUM_FRAMES);
    
    for (int i = 0; i < NUM_FRAMES; i++) {
        Serial.printf("\nFrame %d:\n", i + 1);
        
        SpeciesResult result = classifier.classifyImage(frames[i]);
        
        if (result.confidence >= CONFIDENCE_THRESHOLD) {
            Serial.printf("  Detected: %s (%.2f%%)\n",
                         result.specificName.c_str(),
                         result.confidence * 100.0f);
        } else {
            Serial.println("  No confident detection");
        }
    }
    
    Serial.println("\n=== Batch Complete ===");
}

/**
 * Example: Multi-animal detection in a single frame
 */
void multiAnimalDetectionExample(const CameraFrame& frame) {
    Serial.println("\n=== Multi-Animal Detection Example ===");
    
    std::vector<SpeciesResult> detections = 
        classifier.classifyMultipleAnimals(frame);
    
    Serial.printf("Detected %d animals:\n", detections.size());
    
    for (size_t i = 0; i < detections.size(); i++) {
        Serial.printf("\nAnimal %d:\n", i + 1);
        Serial.printf("  Species: %s\n", detections[i].specificName.c_str());
        Serial.printf("  Confidence: %.2f%%\n", detections[i].confidence * 100.0f);
        Serial.printf("  Location: [%d, %d]\n",
                     detections[i].boundingBox[0],
                     detections[i].boundingBox[1]);
    }
}

/**
 * Example: Performance benchmarking
 */
void benchmarkPerformance() {
    Serial.println("\n=== Performance Benchmark ===");
    
    const int NUM_ITERATIONS = 100;
    unsigned long totalTime = 0;
    unsigned long minTime = ULONG_MAX;
    unsigned long maxTime = 0;
    
    CameraFrame testFrame;
    // ... populate test frame ...
    
    Serial.printf("Running %d iterations...\n", NUM_ITERATIONS);
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        unsigned long start = millis();
        SpeciesResult result = classifier.classifyImage(testFrame);
        unsigned long elapsed = millis() - start;
        
        totalTime += elapsed;
        if (elapsed < minTime) minTime = elapsed;
        if (elapsed > maxTime) maxTime = elapsed;
        
        if ((i + 1) % 10 == 0) {
            Serial.printf("  Progress: %d/%d\n", i + 1, NUM_ITERATIONS);
        }
    }
    
    Serial.println("\nResults:");
    Serial.printf("  Average: %lu ms\n", totalTime / NUM_ITERATIONS);
    Serial.printf("  Min: %lu ms\n", minTime);
    Serial.printf("  Max: %lu ms\n", maxTime);
    Serial.printf("  Target: <200 ms\n");
    
    if (totalTime / NUM_ITERATIONS <= 200) {
        Serial.println("  ✓ Performance target met!");
    } else {
        Serial.println("  ✗ Performance needs optimization");
    }
}

/**
 * IMPORTANT NOTES:
 * 
 * 1. TEST MODEL LIMITATIONS:
 *    - Trained on synthetic data
 *    - ~10% accuracy (essentially random)
 *    - Only for integration testing
 *    - DO NOT use in production
 * 
 * 2. FOR PRODUCTION USE:
 *    - Train a real model (see TRAINING_DEPLOYMENT_GUIDE.md)
 *    - Collect real wildlife datasets (1000+ images per species)
 *    - Achieve >85% accuracy before deployment
 *    - Test in field conditions
 * 
 * 3. PERFORMANCE OPTIMIZATION:
 *    - Use PSRAM for model storage
 *    - Set CPU frequency to 240MHz during inference
 *    - Consider smaller input size (160x160)
 *    - Batch multiple inferences when possible
 * 
 * 4. MEMORY MANAGEMENT:
 *    - Model uses ~512KB of memory
 *    - Input buffer: 224x224x3 = 150KB
 *    - Ensure sufficient free heap before inference
 *    - Free resources when not in use
 * 
 * 5. ERROR HANDLING:
 *    - Always check return values
 *    - Validate confidence scores
 *    - Implement fallback mechanisms
 *    - Log errors for debugging
 */
