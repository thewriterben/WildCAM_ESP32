/**
 * @file neuromorphic_wildlife_detection.ino
 * @brief Example: Neuromorphic Wildlife Detection
 * 
 * Demonstrates ultra-low-power wildlife detection using neuromorphic
 * computing with spiking neural networks.
 * 
 * Features:
 * - Brain-inspired spiking neural networks
 * - Ultra-low power consumption (<10mW)
 * - Online learning and adaptation
 * - Event-driven processing
 * - Real-time behavior analysis
 */

#include <Arduino.h>
#include "../../firmware/src/ai/neuromorphic/neuromorphic_system.h"

// Configuration
#define IMAGE_WIDTH 28
#define IMAGE_HEIGHT 28
#define POWER_BUDGET_MW 10.0f  // Ultra-low power budget

// Function prototypes
NeuromorphicSystem* buildNeuromorphicSystem();
void displaySystemInfo();
void exampleBasicDetection();
void exampleEventBasedProcessing();
void exampleOnlineLearning();
void exampleBehaviorAnalysis();
void examplePowerOptimization();
void displayStatistics();
uint8_t* generateTestImage();

// Global instances
NeuromorphicSystem* neuromorphicSystem = nullptr;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║  Neuromorphic Wildlife Detection Example                  ║");
    Serial.println("║  Ultra-Low-Power Brain-Inspired AI                        ║");
    Serial.println("╚════════════════════════════════════════════════════════════╝\n");
    
    // Initialize neuromorphic system
    Serial.println("Initializing neuromorphic computing system...");
    
    neuromorphicSystem = buildNeuromorphicSystem();
    
    if (!neuromorphicSystem) {
        Serial.println("❌ Failed to initialize neuromorphic system!");
        return;
    }
    
    Serial.println("✅ Neuromorphic system initialized successfully!");
    Serial.println();
    
    // Display system information
    displaySystemInfo();
    
    // Run examples
    Serial.println("\n=== Running Examples ===\n");
    
    exampleBasicDetection();
    exampleEventBasedProcessing();
    exampleOnlineLearning();
    exampleBehaviorAnalysis();
    examplePowerOptimization();
    
    displayStatistics();
}

void loop() {
    // Continuous wildlife monitoring mode
    delay(5000);
    
    Serial.println("\n--- Continuous Monitoring ---");
    
    uint8_t* simulatedImage = generateTestImage();
    
    if (neuromorphicSystem && simulatedImage) {
        uint32_t start = micros();
        
        auto result = neuromorphicSystem->classifyWildlife(
            simulatedImage, IMAGE_WIDTH, IMAGE_HEIGHT
        );
        
        uint32_t duration = micros() - start;
        
        if (result.is_valid) {
            Serial.printf("Species: %d | Confidence: %.2f | ", 
                         result.species, result.confidence);
            Serial.printf("Time: %d μs | Power: %.2f mW\n",
                         duration, result.power_consumption_mw);
        }
        
        free(simulatedImage);
    }
}

NeuromorphicSystem* buildNeuromorphicSystem() {
    NeuromorphicSystemBuilder builder;
    
    auto system = builder
        .setProcessingMode(NeuromorphicMode::HYBRID_ANN_SNN)
        .enableHybridProcessing()
        .enableOnlineLearning()
        .setPowerBudget(POWER_BUDGET_MW)
        .setImageSize(IMAGE_WIDTH, IMAGE_HEIGHT)
        .setNumSpecies(20)
        .build();
    
    if (!system) {
        return nullptr;
    }
    
    return system.release();
}

void displaySystemInfo() {
    if (!neuromorphicSystem) return;
    
    Serial.println("=== Neuromorphic System Configuration ===");
    
    auto platforms = neuromorphicSystem->getAvailablePlatforms();
    Serial.printf("Available platforms: %d\n", platforms.size());
    
    Serial.printf("Power budget: %.2f mW\n", POWER_BUDGET_MW);
    Serial.println("Processing mode: Hybrid ANN-SNN");
    Serial.println("Online learning: Enabled");
    Serial.println();
}

void exampleBasicDetection() {
    Serial.println("--- Example 1: Basic Wildlife Detection ---");
    
    uint8_t* testImage = generateTestImage();
    
    if (!testImage || !neuromorphicSystem) {
        Serial.println("Failed to generate test image");
        return;
    }
    
    Serial.println("Processing image with neuromorphic system...");
    
    uint32_t start = micros();
    auto result = neuromorphicSystem->classifyWildlife(
        testImage, IMAGE_WIDTH, IMAGE_HEIGHT
    );
    uint32_t duration = micros() - start;
    
    if (result.is_valid) {
        Serial.println("✅ Detection successful!");
        Serial.printf("  Species: %d\n", result.species);
        Serial.printf("  Confidence: %.2f%%\n", result.confidence * 100);
        Serial.printf("  Detection time: %d μs\n", duration);
        Serial.printf("  Power: %.2f mW\n", result.power_consumption_mw);
    } else {
        Serial.println("❌ Detection failed");
    }
    
    free(testImage);
    Serial.println();
}

void exampleEventBasedProcessing() {
    Serial.println("--- Example 2: Event-Based Processing ---");
    
    if (!neuromorphicSystem) return;
    
    std::vector<EventSensorData> events;
    
    for (int i = 0; i < 100; i++) {
        EventSensorData event;
        event.x = random(0, IMAGE_WIDTH);
        event.y = random(0, IMAGE_HEIGHT);
        event.timestamp = i * 100;
        event.polarity = (i % 2 == 0) ? 1 : -1;
        events.push_back(event);
    }
    
    Serial.printf("Generated %d events\n", events.size());
    
    auto result = neuromorphicSystem->processEvents(events);
    
    if (result.is_valid) {
        Serial.println("✅ Event processing successful!");
        Serial.printf("  Power: %.2f mW\n", result.power_consumption_mw);
    }
    
    Serial.println();
}

void exampleOnlineLearning() {
    Serial.println("--- Example 3: Online Learning ---");
    
    if (!neuromorphicSystem) return;
    
    neuromorphicSystem->enableOnlineLearning(true);
    
    Serial.println("Training with new observations...");
    
    uint8_t* trainingImage = generateTestImage();
    if (trainingImage) {
        bool success = neuromorphicSystem->train(
            trainingImage, IMAGE_WIDTH, IMAGE_HEIGHT, SpeciesType::MAMMAL_MEDIUM
        );
        
        Serial.println(success ? "✅ Training successful" : "❌ Training failed");
        free(trainingImage);
    }
    
    Serial.println();
}

void exampleBehaviorAnalysis() {
    Serial.println("--- Example 4: Behavior Analysis ---");
    
    if (!neuromorphicSystem) return;
    
    std::vector<NeuromorphicWildlifeResult> history;
    
    for (int i = 0; i < 5; i++) {
        NeuromorphicWildlifeResult result;
        result.species = SpeciesType::MAMMAL_MEDIUM;
        result.confidence = 0.8f;
        result.is_valid = true;
        history.push_back(result);
    }
    
    BehaviorType behavior = neuromorphicSystem->analyzeBehavior(history);
    Serial.printf("Detected behavior: %d\n", behavior);
    
    Serial.println();
}

void examplePowerOptimization() {
    Serial.println("--- Example 5: Power Optimization ---");
    
    if (!neuromorphicSystem) return;
    
    float budgets[] = {5.0f, 10.0f, 25.0f};
    
    for (float budget : budgets) {
        neuromorphicSystem->setPowerBudget(budget);
        
        uint8_t* testImage = generateTestImage();
        if (testImage) {
            auto result = neuromorphicSystem->classifyWildlife(
                testImage, IMAGE_WIDTH, IMAGE_HEIGHT
            );
            
            if (result.is_valid) {
                Serial.printf("  Budget: %.1f mW | Actual: %.2f mW\n",
                             budget, result.power_consumption_mw);
            }
            
            free(testImage);
        }
    }
    
    Serial.println();
}

void displayStatistics() {
    if (!neuromorphicSystem) return;
    
    Serial.println("\n=== System Statistics ===");
    
    auto stats = neuromorphicSystem->getStatistics();
    
    Serial.printf("Total inferences: %d\n", stats.total_inferences);
    Serial.printf("Successful detections: %d\n", stats.successful_detections);
    Serial.printf("Average inference time: %.1f μs\n", stats.average_inference_time_us);
    Serial.printf("Average power: %.2f mW\n", stats.average_power_consumption_mw);
    
    Serial.println("\n=== End of Examples ===");
}

uint8_t* generateTestImage() {
    uint8_t* image = (uint8_t*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
    
    if (!image) {
        return nullptr;
    }
    
    for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
        image[i] = random(0, 255);
    }
    
    return image;
}
