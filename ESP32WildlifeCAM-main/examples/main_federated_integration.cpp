/**
 * @file main_federated_integration.cpp
 * @brief Example integration of federated learning with main ESP32WildlifeCAM system
 * 
 * This example shows how to integrate the complete federated learning system
 * with the existing wildlife camera firmware.
 */

#include <Arduino.h>
#include "firmware/src/ai/federated_learning/federated_learning_system.h"
#include "firmware/src/ai/wildlife_classifier.h"
#include "firmware/src/ai/ai_federated_integration.h"
#include "firmware/src/mesh/federated_mesh.h"
#include "firmware/src/power/federated_power_manager.h"
#include "utils/logger.h"

// Global components
FederatedLearningSystem* g_flSystem = nullptr;
WildlifeClassifier* g_wildlifeClassifier = nullptr;
AIFederatedIntegration* g_aiIntegration = nullptr;
FederatedMesh* g_federatedMesh = nullptr;
FederatedPowerManager* g_powerManager = nullptr;

/**
 * Initialize the complete federated learning system
 */
bool initializeFederatedSystem() {
    Logger::info("Initializing federated learning system...");

    // 1. Initialize federated learning system
    FLSystemConfig flConfig;
    flConfig.federatedConfig.enableFederatedLearning = true;
    flConfig.federatedConfig.contributionThreshold = 0.85f;
    flConfig.federatedConfig.privacyLevel = PrivacyLevel::HIGH;
    flConfig.federatedConfig.privacyEpsilon = 0.5f;
    flConfig.topologyConfig.preferredTopology = NetworkTopology::MESH;
    flConfig.networkConfig.protocolPriority = {"WiFi", "LoRa", "Cellular"};
    flConfig.enablePowerManagement = true;
    flConfig.enableAutomaticMode = true;

    g_flSystem = new FederatedLearningSystem();
    if (!g_flSystem->init(flConfig)) {
        Logger::error("Failed to initialize federated learning system");
        return false;
    }

    // 2. Initialize wildlife classifier
    WildlifeClassifierConfig classifierConfig;
    classifierConfig.enableBehaviorAnalysis = true;
    classifierConfig.enableEnvironmentalAdaptation = true;
    classifierConfig.contributeToFederatedLearning = true;
    classifierConfig.contributionThreshold = 0.85f;

    g_wildlifeClassifier = new WildlifeClassifier();
    if (!g_wildlifeClassifier->init(classifierConfig)) {
        Logger::error("Failed to initialize wildlife classifier");
        return false;
    }

    // 3. Initialize AI federated integration
    AIFederatedConfig aiConfig;
    aiConfig.enableFederatedLearning = true;
    aiConfig.contributionThreshold = 0.85f;
    aiConfig.privacyLevel = PrivacyLevel::HIGH;

    g_aiIntegration = new AIFederatedIntegration();
    if (!g_aiIntegration->init(aiConfig)) {
        Logger::error("Failed to initialize AI federated integration");
        return false;
    }

    // 4. Initialize federated mesh
    FederatedMeshConfig meshConfig;
    meshConfig.enableFederatedLearning = true;
    meshConfig.maxParticipants = 15;
    meshConfig.enableAutoCoordination = true;

    g_federatedMesh = new FederatedMesh();
    if (!g_federatedMesh->init(meshConfig)) {
        Logger::error("Failed to initialize federated mesh");
        return false;
    }

    // 5. Initialize federated power manager
    FLPowerConfig powerConfig;
    powerConfig.enablePowerOptimization = true;
    powerConfig.solarIntegration = true;
    powerConfig.batteryThresholds.criticalLevel = 0.2f;
    powerConfig.batteryThresholds.lowLevel = 0.3f;

    g_powerManager = new FederatedPowerManager();
    if (!g_powerManager->init(powerConfig)) {
        Logger::error("Failed to initialize federated power manager");
        return false;
    }

    // 6. Setup callbacks and integration
    setupCallbacks();

    Logger::info("Federated learning system initialized successfully");
    return true;
}

/**
 * Setup callbacks between components
 */
void setupCallbacks() {
    // Wildlife classifier callbacks
    g_wildlifeClassifier->setDetectionCallback([](const WildlifeDetection& detection) {
        Logger::info("Wildlife detected: %s (%.3f confidence)", 
                    wildlifeSpeciesToString(detection.species), detection.overallConfidence);
        
        // Process with federated integration
        if (g_aiIntegration) {
            // Convert to federated result format
            FederatedWildlifeResult result;
            result.species = detection.species;
            result.behavior = detection.behavior;
            result.confidence = detection.overallConfidence;
            result.timestamp = detection.timestamp;
            result.environment = detection.environment;
            
            // Create dummy camera frame (in real implementation, pass actual frame)
            CameraFrame frame;
            
            g_aiIntegration->processWildlifeResult(result, frame);
        }
    });

    // Federated mesh callbacks
    g_federatedMesh->setRoundStartCallback([](const String& roundId, WildlifeModelType modelType) {
        Logger::info("Training round started: %s for model type: %d", roundId.c_str(), (int)modelType);
        
        if (g_flSystem) {
            g_flSystem->participateInRound(roundId);
        }
    });

    g_federatedMesh->setRoundEndCallback([](const String& roundId, bool success) {
        Logger::info("Training round ended: %s (success: %s)", roundId.c_str(), success ? "true" : "false");
    });

    // Power manager callbacks
    g_powerManager->setPowerEventCallback([](const String& event, float batteryLevel) {
        Logger::info("Power event: %s (battery: %.1f%%)", event.c_str(), batteryLevel * 100);
        
        if (event == "LOW_BATTERY" && g_flSystem) {
            g_flSystem->pause();
        } else if (event == "POWER_RESTORED" && g_flSystem) {
            g_flSystem->resume();
        }
    });

    // Federated learning system callbacks
    g_flSystem->setStatusChangeCallback([](FLSystemStatus oldStatus, FLSystemStatus newStatus) {
        Logger::info("FL status change: %d -> %d", (int)oldStatus, (int)newStatus);
    });

    g_flSystem->setTrainingCompleteCallback([](const String& roundId, const TrainingResult& result) {
        Logger::info("Training completed for round %s: accuracy improvement %.3f", 
                    roundId.c_str(), result.accuracyImprovement);
    });
}

/**
 * Process wildlife observation with federated learning
 */
void processWildlifeObservation(const CameraFrame& frame) {
    if (!g_wildlifeClassifier) {
        return;
    }

    // Get environmental context
    EnvironmentalContext environment;
    environment.temperature = 20.0f;  // From temperature sensor
    environment.humidity = 60.0f;     // From humidity sensor
    environment.lightLevel = 500.0f;  // From light sensor
    environment.timeOfDay = 14;       // From RTC
    environment.season = 1;           // Summer

    // Classify wildlife
    std::vector<WildlifeDetection> detections = g_wildlifeClassifier->classifyImage(frame, environment);

    // Process each detection
    for (const auto& detection : detections) {
        Logger::info("Detected: %s (%s) - Confidence: %.3f", 
                    wildlifeSpeciesToString(detection.species),
                    animalBehaviorToString(detection.behavior),
                    detection.overallConfidence);

        // High-confidence detections are automatically contributed to federated learning
        // via the callback system
    }
}

/**
 * Main setup function
 */
void setup() {
    Serial.begin(115200);
    delay(2000);

    Logger::info("=== ESP32WildlifeCAM with Federated Learning ===");
    Logger::info("Initializing system...");

    // Initialize federated learning system
    if (!initializeFederatedSystem()) {
        Logger::error("Failed to initialize federated learning system");
        Logger::error("Entering safe mode...");
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
        }
    }

    // Start federated learning
    if (g_flSystem && !g_flSystem->start()) {
        Logger::error("Failed to start federated learning");
    }

    // Elect mesh coordinator
    if (g_federatedMesh) {
        g_federatedMesh->electCoordinator();
    }

    Logger::info("System initialization complete!");
    Logger::info("Federated learning active: %s", 
                g_flSystem && g_flSystem->getStatus() == FLSystemStatus::ACTIVE ? "true" : "false");
}

/**
 * Main loop function
 */
void loop() {
    static uint32_t lastUpdate = 0;
    static uint32_t lastWildlifeCheck = 0;
    uint32_t currentTime = millis();

    // Update federated learning system (every 100ms)
    if (currentTime - lastUpdate > 100) {
        if (g_flSystem) {
            g_flSystem->updateAutomaticMode();
        }
        
        if (g_federatedMesh) {
            g_federatedMesh->checkNetworkHealth();
        }
        
        if (g_powerManager) {
            g_powerManager->updatePowerStatus();
        }
        
        lastUpdate = currentTime;
    }

    // Simulate wildlife detection (every 5 seconds)
    if (currentTime - lastWildlifeCheck > 5000) {
        // Create dummy camera frame
        CameraFrame frame;
        frame.width = 640;
        frame.height = 480;
        frame.timestamp = currentTime;
        
        // Process wildlife observation
        processWildlifeObservation(frame);
        
        lastWildlifeCheck = currentTime;
    }

    // Yield to other tasks
    yield();
}

/**
 * Cleanup function
 */
void cleanup() {
    Logger::info("Cleaning up federated learning system...");

    if (g_flSystem) {
        g_flSystem->stop();
        delete g_flSystem;
        g_flSystem = nullptr;
    }

    if (g_wildlifeClassifier) {
        g_wildlifeClassifier->cleanup();
        delete g_wildlifeClassifier;
        g_wildlifeClassifier = nullptr;
    }

    if (g_aiIntegration) {
        g_aiIntegration->cleanup();
        delete g_aiIntegration;
        g_aiIntegration = nullptr;
    }

    if (g_federatedMesh) {
        g_federatedMesh->cleanup();
        delete g_federatedMesh;
        g_federatedMesh = nullptr;
    }

    if (g_powerManager) {
        g_powerManager->cleanup();
        delete g_powerManager;
        g_powerManager = nullptr;
    }

    Logger::info("Cleanup complete");
}

/**
 * Emergency stop for federated learning
 */
void emergencyStop() {
    Logger::warn("Emergency stop triggered!");
    
    if (g_flSystem) {
        g_flSystem->stop();
    }
    
    if (g_federatedMesh) {
        // Broadcast emergency stop to all participants
        // This would be implemented in the mesh layer
    }
}

/**
 * Status report function
 */
void printStatusReport() {
    Logger::info("=== Federated Learning Status Report ===");
    
    if (g_flSystem) {
        FLSystemStats stats = g_flSystem->getSystemStats();
        Logger::info("FL Status: %d", (int)stats.status);
        Logger::info("Uptime: %d ms", stats.uptime);
        Logger::info("Total Rounds: %d", stats.totalRounds);
        Logger::info("Successful Rounds: %d", stats.successfulRounds);
        Logger::info("Models Contributed: %d", stats.modelsContributed);
        Logger::info("Privacy Budget Used: %.3f", stats.totalPrivacyBudgetUsed);
        Logger::info("Network Nodes: %d", stats.networkNodes);
        Logger::info("Battery Level: %.1f%%", stats.batteryLevel * 100);
    }
    
    if (g_wildlifeClassifier) {
        WildlifeClassifierStats classifierStats = g_wildlifeClassifier->getStatistics();
        Logger::info("Total Detections: %d", classifierStats.totalDetections);
        Logger::info("Species Detected: %d", classifierStats.speciesDetected);
        Logger::info("Average Confidence: %.3f", classifierStats.averageConfidence);
        Logger::info("FL Contributions: %d", classifierStats.contributionsToFL);
    }
    
    if (g_federatedMesh) {
        FederatedMeshStats meshStats = g_federatedMesh->getStatistics();
        Logger::info("Mesh Rounds: %d", meshStats.totalRounds);
        Logger::info("Average Participants: %d", meshStats.averageParticipants);
        Logger::info("Network Reliability: %.1f%%", g_federatedMesh->getNetworkReliability() * 100);
    }
    
    Logger::info("========================================");
}

// Example function to trigger manual training round
void startManualTrainingRound() {
    if (g_federatedMesh && g_federatedMesh->isCoordinator()) {
        String roundId = "MANUAL_" + String(millis());
        g_federatedMesh->startTrainingRound(roundId, WildlifeModelType::SPECIES_CLASSIFIER);
        Logger::info("Started manual training round: %s", roundId.c_str());
    } else {
        Logger::warn("Not coordinator, cannot start training round");
    }
}

// Example function to get federated learning report
String getFederatedLearningReport() {
    String report = "Federated Learning System Report\n";
    report += "==================================\n";
    
    if (g_flSystem) {
        FLSystemStats stats = g_flSystem->getSystemStats();
        report += "Status: " + String((int)stats.status) + "\n";
        report += "Rounds: " + String(stats.totalRounds) + " total, " + String(stats.successfulRounds) + " successful\n";
        report += "Contributions: " + String(stats.modelsContributed) + "\n";
        report += "Privacy Budget: " + String(stats.totalPrivacyBudgetUsed) + " used\n";
        report += "Network: " + String(stats.networkNodes) + " nodes\n";
    }
    
    return report;
}