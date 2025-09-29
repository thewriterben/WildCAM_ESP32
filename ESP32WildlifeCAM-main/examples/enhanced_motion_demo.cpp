/**
 * @file enhanced_motion_demo.cpp
 * @brief Enhanced Motion Detection System Demo
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include <Arduino.h>
#include <WiFi.h>
#include "src/detection/motion_coordinator.h"
#include "src/detection/adaptive_processor.h"
#include "src/detection/wildlife_analyzer.h"
#include "src/detection/memory_pool_manager.h"
#include "src/detection/config_manager.h"

// Pin definitions for AI-Thinker ESP32-CAM
#define PIR_PIN 13
#define LED_PIN 4
#define FLASH_PIN 4

// Global components
MotionCoordinator motionCoordinator;
std::shared_ptr<MemoryPoolManager> memoryManager;
std::shared_ptr<ConfigManager> configManager;

// Statistics
uint32_t totalDetections = 0;
uint32_t wildlifeDetections = 0;
uint32_t captures = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=======================================");
    Serial.println("ESP32 Wildlife Camera - Enhanced Motion Detection Demo");
    Serial.println("=======================================");
    
    // Initialize GPIO
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(FLASH_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(FLASH_PIN, LOW);
    
    // Initialize memory manager
    Serial.println("Initializing memory manager...");
    memoryManager = std::make_shared<MemoryPoolManager>();
    if (!memoryManager->initialize(true)) {
        Serial.println("ERROR: Memory manager initialization failed");
        return;
    }
    Serial.println("✓ Memory manager initialized");
    
    // Initialize configuration manager
    Serial.println("Initializing configuration manager...");
    configManager = std::make_shared<ConfigManager>();
    if (!configManager->initialize()) {
        Serial.println("ERROR: Configuration manager initialization failed");
        return;
    }
    Serial.println("✓ Configuration manager initialized");
    
    // Initialize motion coordinator
    Serial.println("Initializing motion coordinator...");
    MotionCoordinator::CoordinatorConfig coordinatorConfig;
    coordinatorConfig.enabled = true;
    coordinatorConfig.defaultMethod = MotionCoordinator::DetectionMethod::ADAPTIVE;
    coordinatorConfig.enablePerformanceOptimization = true;
    coordinatorConfig.enableWildlifeAnalysis = true;
    coordinatorConfig.useEnvironmentalAdaptation = true;
    coordinatorConfig.captureThreshold = 0.7f;
    coordinatorConfig.transmitThreshold = 0.8f;
    
    if (!motionCoordinator.initialize(nullptr, coordinatorConfig)) {
        Serial.println("ERROR: Motion coordinator initialization failed");
        return;
    }
    
    // Set motion detection callback
    motionCoordinator.setMotionCallback([](const MotionCoordinator::CoordinatorResult& result) {
        handleMotionDetected(result);
    });
    
    Serial.println("✓ Motion coordinator initialized");
    Serial.println("=======================================");
    Serial.println("System ready - monitoring for motion...");
    Serial.println("=======================================");
    
    // Print initial memory status
    printMemoryStatus();
    printSystemConfiguration();
}

void loop() {
    static uint32_t lastDetectionCheck = 0;
    static uint32_t lastStatusPrint = 0;
    static uint32_t lastMemoryCheck = 0;
    
    uint32_t currentTime = millis();
    
    // Perform motion detection every second
    if (currentTime - lastDetectionCheck >= 1000) {
        performMotionDetection();
        lastDetectionCheck = currentTime;
    }
    
    // Print status every 30 seconds
    if (currentTime - lastStatusPrint >= 30000) {
        printSystemStatus();
        lastStatusPrint = currentTime;
    }
    
    // Check memory health every 60 seconds
    if (currentTime - lastMemoryCheck >= 60000) {
        checkMemoryHealth();
        lastMemoryCheck = currentTime;
    }
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}

void performMotionDetection() {
    // Create environmental conditions
    MotionCoordinator::EnvironmentalConditions conditions;
    conditions.batteryVoltage = readBatteryVoltage();
    conditions.temperature = 20.0f; // Simulated temperature
    conditions.lightLevel = readLightLevel();
    conditions.currentHour = (millis() / 3600000) % 24; // Simulated hour
    conditions.isNight = (conditions.currentHour < 6 || conditions.currentHour > 20);
    
    // Perform motion detection
    auto result = motionCoordinator.detectMotion(nullptr, conditions);
    
    // Update statistics
    if (result.motionDetected) {
        totalDetections++;
        
        if (result.wildlifeAnalysis.isWildlife) {
            wildlifeDetections++;
        }
        
        if (result.shouldCapture) {
            captures++;
        }
    }
}

void handleMotionDetected(const MotionCoordinator::CoordinatorResult& result) {
    Serial.println("\n🔍 MOTION DETECTED!");
    Serial.printf("├─ Method: %s\n", getMethodName(result.methodUsed).c_str());
    Serial.printf("├─ Confidence: %.2f\n", result.fusionConfidence);
    Serial.printf("├─ Processing Time: %dms\n", result.processingTime);
    
    if (result.wildlifeAnalysis.isWildlife) {
        Serial.printf("├─ 🐾 Wildlife: %s (%.2f confidence)\n", 
                     result.wildlifeAnalysis.description.c_str(),
                     result.wildlifeAnalysis.confidence);
    }
    
    Serial.printf("├─ Actions: ");
    if (result.shouldCapture) Serial.print("CAPTURE ");
    if (result.shouldSave) Serial.print("SAVE ");
    if (result.shouldTransmit) Serial.print("TRANSMIT ");
    if (result.shouldAlert) Serial.print("ALERT ");
    Serial.println();
    
    // Simulate camera actions
    if (result.shouldCapture) {
        simulateCameraCapture(result);
    }
    
    // Flash LED to indicate detection
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("└─ Motion processing complete\n");
}

void simulateCameraCapture(const MotionCoordinator::CoordinatorResult& result) {
    Serial.println("📸 Simulating camera capture...");
    
    // Flash the LED to simulate camera flash
    digitalWrite(FLASH_PIN, HIGH);
    delay(50);
    digitalWrite(FLASH_PIN, LOW);
    
    // Simulate image processing delay
    delay(200);
    
    Serial.printf("✓ Image captured and processed\n");
    
    if (result.shouldSave) {
        Serial.println("💾 Saving image to storage...");
        delay(100);
        Serial.println("✓ Image saved");
    }
    
    if (result.shouldTransmit) {
        Serial.println("📡 Transmitting image via LoRa...");
        delay(500);
        Serial.println("✓ Image transmitted");
    }
    
    if (result.shouldAlert) {
        Serial.println("🚨 Triggering wildlife alert...");
        delay(100);
        Serial.println("✓ Alert sent");
    }
}

void printSystemStatus() {
    Serial.println("\n📊 SYSTEM STATUS");
    Serial.println("================");
    
    // Motion detection statistics
    Serial.printf("Total Detections: %d\n", totalDetections);
    Serial.printf("Wildlife Detections: %d\n", wildlifeDetections);
    Serial.printf("Captures: %d\n", captures);
    
    if (totalDetections > 0) {
        float wildlifeRate = (float)wildlifeDetections / totalDetections * 100.0f;
        float captureRate = (float)captures / totalDetections * 100.0f;
        Serial.printf("Wildlife Rate: %.1f%%\n", wildlifeRate);
        Serial.printf("Capture Rate: %.1f%%\n", captureRate);
    }
    
    // Get coordinator statistics
    auto coordinatorStats = motionCoordinator.getStatistics();
    Serial.printf("Average Processing Time: %dms\n", coordinatorStats.averageProcessingTime);
    Serial.printf("Power Savings: %.1f%%\n", coordinatorStats.powerSavings);
    
    // Environmental conditions
    Serial.printf("Battery Voltage: %.2fV\n", readBatteryVoltage());
    Serial.printf("Light Level: %.2f\n", readLightLevel());
    
    Serial.println("================\n");
}

void printMemoryStatus() {
    if (!memoryManager) return;
    
    Serial.println("\n💾 MEMORY STATUS");
    Serial.println("================");
    
    auto overallStats = memoryManager->getOverallStats();
    Serial.printf("Total Memory: %d bytes\n", overallStats.totalMemory);
    Serial.printf("Used Memory: %d bytes\n", overallStats.usedMemory);
    Serial.printf("Memory Utilization: %.1f%%\n", 
                 (float)overallStats.usedMemory / overallStats.totalMemory * 100.0f);
    
    Serial.printf("Total Allocations: %d\n", overallStats.allocations);
    Serial.printf("Total Deallocations: %d\n", overallStats.deallocations);
    Serial.printf("Allocation Failures: %d\n", overallStats.failures);
    
    // Free heap and PSRAM
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    if (esp_psram_is_initialized()) {
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    }
    
    Serial.println("================\n");
}

void printSystemConfiguration() {
    if (!configManager) return;
    
    Serial.println("\n⚙️  SYSTEM CONFIGURATION");
    Serial.println("========================");
    
    // Motion coordinator configuration
    Serial.println("Motion Coordinator:");
    Serial.printf("├─ Default Method: %s\n", 
                 configManager->getConfigValue(ConfigManager::ConfigSection::MOTION_COORDINATOR, 
                                              "defaultMethod", "adaptive").c_str());
    Serial.printf("├─ Capture Threshold: %s\n",
                 configManager->getConfigValue(ConfigManager::ConfigSection::MOTION_COORDINATOR,
                                              "captureThreshold", "0.7").c_str());
    
    // Adaptive processor configuration
    Serial.println("Adaptive Processor:");
    Serial.printf("├─ Max Processing Time: %sms\n",
                 configManager->getConfigValue(ConfigManager::ConfigSection::ADAPTIVE_PROCESSOR,
                                              "maxProcessingTimeMs", "500").c_str());
    Serial.printf("├─ Battery Low Threshold: %sV\n",
                 configManager->getConfigValue(ConfigManager::ConfigSection::ADAPTIVE_PROCESSOR,
                                              "batteryLowThreshold", "3.2").c_str());
    
    // Wildlife analyzer configuration
    Serial.println("Wildlife Analyzer:");
    Serial.printf("├─ Wildlife Confidence Threshold: %s\n",
                 configManager->getConfigValue(ConfigManager::ConfigSection::WILDLIFE_ANALYZER,
                                              "wildlifeConfidenceThreshold", "0.6").c_str());
    Serial.printf("└─ Time of Day Adaptation: %s\n",
                 configManager->getConfigValue(ConfigManager::ConfigSection::WILDLIFE_ANALYZER,
                                              "useTimeOfDayAdaptation", "true").c_str());
    
    Serial.println("========================\n");
}

void checkMemoryHealth() {
    if (!memoryManager) return;
    
    if (!memoryManager->checkHealth()) {
        Serial.println("⚠️  Memory health check failed - performing garbage collection");
        uint32_t cleaned = memoryManager->garbageCollect();
        Serial.printf("✓ Cleaned up %d buffers\n", cleaned);
    }
    
    auto overallStats = memoryManager->getOverallStats();
    float utilization = (float)overallStats.usedMemory / overallStats.totalMemory * 100.0f;
    
    if (utilization > 85.0f) {
        Serial.printf("⚠️  High memory utilization: %.1f%%\n", utilization);
    }
}

float readBatteryVoltage() {
    // Simulate battery voltage reading
    // In real implementation, this would read from ADC pin connected to voltage divider
    static float voltage = 3.7f;
    voltage += (random(-10, 11) / 1000.0f); // Add some random variation
    voltage = constrain(voltage, 3.0f, 4.2f);
    return voltage;
}

float readLightLevel() {
    // Simulate light level reading
    // In real implementation, this would read from light sensor
    uint32_t hour = (millis() / 3600000) % 24;
    
    if (hour >= 6 && hour <= 18) {
        return 0.5f + (random(0, 50) / 100.0f); // Daytime
    } else {
        return 0.1f + (random(0, 20) / 100.0f); // Nighttime
    }
}

String getMethodName(MotionCoordinator::DetectionMethod method) {
    switch (method) {
        case MotionCoordinator::DetectionMethod::PIR_ONLY: return "PIR Only";
        case MotionCoordinator::DetectionMethod::FRAME_ONLY: return "Frame Only";
        case MotionCoordinator::DetectionMethod::AI_ONLY: return "AI Only";
        case MotionCoordinator::DetectionMethod::HYBRID_BASIC: return "Hybrid Basic";
        case MotionCoordinator::DetectionMethod::HYBRID_AI: return "Hybrid AI";
        case MotionCoordinator::DetectionMethod::FULL_FUSION: return "Full Fusion";
        case MotionCoordinator::DetectionMethod::ADAPTIVE: return "Adaptive";
        default: return "Unknown";
    }
}