/**
 * @file federated_learning_example.h
 * @brief Example Integration of Federated Learning with ESP32WildlifeCAM
 * 
 * This example demonstrates how to integrate the federated learning system
 * with an existing ESP32WildlifeCAM application.
 */

#ifndef FEDERATED_LEARNING_EXAMPLE_H
#define FEDERATED_LEARNING_EXAMPLE_H

#include "ai/federated_learning/federated_learning_system.h"
#include "ai/ai_federated_integration.h"
#include "power/federated_power_manager.h"
#include "web/federated_web_interface.h"

/**
 * Example: Basic Federated Learning Setup
 */
class WildlifeCameraWithFederatedLearning {
public:
    bool setupFederatedLearning() {
        // 1. Configure federated learning system
        FLSystemConfig config;
        
        // Basic federated learning settings
        config.federatedConfig.enableFederatedLearning = true;
        config.federatedConfig.contributionThreshold = 0.9f;  // Only contribute high-confidence results
        config.federatedConfig.minSamplesBeforeContribution = 50;
        config.federatedConfig.maxContributionsPerDay = 20;
        
        // Privacy settings - HIGH privacy for sensitive wildlife data
        config.federatedConfig.privacyLevel = PrivacyLevel::HIGH;
        config.federatedConfig.privacyEpsilon = 0.5f;  // Strong privacy protection
        config.federatedConfig.anonymizeLocation = true;
        config.federatedConfig.anonymizeTimestamps = true;
        
        // Training settings optimized for ESP32
        config.federatedConfig.learningRate = 0.001f;
        config.federatedConfig.localEpochs = 3;  // Keep training short
        config.federatedConfig.batchSize = 4;   // Small batch for memory constraints
        config.federatedConfig.maxTrainingTimeMs = 180000; // 3 minutes max
        
        // Network topology - MESH for wildlife monitoring networks
        config.topologyConfig.preferredTopology = NetworkTopology::MESH;
        config.topologyConfig.maxNodes = 15;
        config.topologyConfig.maxHops = 3;
        config.topologyConfig.enableAutoReconfiguration = true;
        config.topologyConfig.enableFaultTolerance = true;
        
        // Network communication settings
        config.networkConfig.protocolPriority = {"WiFi", "LoRa", "Cellular"};
        config.networkConfig.enableCompression = true;
        config.networkConfig.defaultCompression = CompressionMethod::QUANTIZATION;
        config.networkConfig.maxMessageSize = 32 * 1024; // 32KB max message
        
        // Aggregation settings for privacy and robustness
        config.aggregationConfig.method = AggregationMethod::FEDERATED_AVERAGING;
        config.aggregationConfig.privacyLevel = PrivacyLevel::HIGH;
        config.aggregationConfig.enableSecureAggregation = true;
        config.aggregationConfig.enableRobustness = true;
        config.aggregationConfig.minParticipants = 3;
        
        // System-level settings
        config.enablePowerManagement = true;
        config.enableAutomaticMode = true;
        config.enableWebInterface = true;
        config.enableLogging = true;
        
        // 2. Initialize the federated learning system
        if (!initializeFederatedLearning(config)) {
            Serial.println("Failed to initialize federated learning");
            return false;
        }
        
        // 3. Configure AI integration
        return setupAIIntegration() && setupPowerManagement() && setupWebInterface();
    }
    
private:
    bool setupAIIntegration() {
        AIFederatedConfig aiConfig;
        
        // Enable federated learning for AI
        aiConfig.enableFederatedLearning = true;
        aiConfig.minConfidenceForContribution = 0.85f;
        aiConfig.minAccuracyForContribution = 0.8f;
        aiConfig.maxContributionsPerDay = 25;
        
        // Privacy settings
        aiConfig.defaultPrivacyLevel = PrivacyLevel::HIGH;
        aiConfig.autoApplyPrivacy = true;
        aiConfig.requireExpertValidation = false; // Enable for research deployments
        
        // Model management
        aiConfig.enableAutomaticModelUpdates = true;
        aiConfig.modelUpdateCheckInterval = 3600000; // Check hourly
        aiConfig.requireManualApproval = false;
        
        // Training settings
        aiConfig.defaultTrainingMode = TrainingMode::INCREMENTAL;
        aiConfig.maxTrainingTimeMs = 180000; // 3 minutes
        aiConfig.enableContinuousLearning = true;
        
        if (!initializeAIFederatedIntegration(aiConfig)) {
            Serial.println("Failed to initialize AI federated integration");
            return false;
        }
        
        // Setup callbacks
        g_aiFederatedIntegration->setResultContributedCallback([](const FederatedWildlifeResult& result) {
            Serial.printf("Contributed wildlife observation: species=%d, confidence=%.2f\n", 
                         static_cast<int>(result.species), result.confidence);
        });
        
        g_aiFederatedIntegration->setModelUpdateCallback([](const ModelUpdate& update) {
            Serial.printf("Received model update: %s, accuracy=%.2f\n", 
                         update.modelId.c_str(), update.accuracy);
        });
        
        return true;
    }
    
    bool setupPowerManagement() {
        FLPowerConfig powerConfig;
        
        // Power mode settings
        powerConfig.defaultMode = FLPowerMode::SOLAR_OPTIMIZED;
        
        // Battery thresholds for wildlife camera deployment
        powerConfig.criticalBatteryLevel = 0.15f;  // Stop FL at 15%
        powerConfig.lowBatteryLevel = 0.25f;       // Reduce FL at 25%
        powerConfig.optimalBatteryLevel = 0.6f;    // Full FL at 60%
        
        // Power consumption limits (conservative for wildlife deployment)
        powerConfig.maxTrainingPower = 600.0f;       // 600mW max training
        powerConfig.maxCommunicationPower = 150.0f;  // 150mW max communication
        powerConfig.maxIdlePower = 30.0f;            // 30mW max idle
        
        // Solar charging optimization for wildlife sites
        powerConfig.enableSolarOptimization = true;
        powerConfig.solarPeakStartHour = 9;    // 9 AM
        powerConfig.solarPeakEndHour = 15;     // 3 PM
        powerConfig.minSolarPower = 50.0f;     // 50mW minimum for FL
        
        // Scheduling constraints
        powerConfig.maxTrainingDuration = 180000;  // 3 minutes max
        powerConfig.minIdleTime = 900000;          // 15 minutes between training
        powerConfig.enableNightMode = false;       // No FL at night to save power
        
        if (!initializeFederatedPowerManager(powerConfig)) {
            Serial.println("Failed to initialize federated power manager");
            return false;
        }
        
        // Setup power callbacks
        g_federatedPowerManager->setPowerModeChangeCallback([](FLPowerMode oldMode, FLPowerMode newMode) {
            Serial.printf("Power mode changed: %d -> %d\n", static_cast<int>(oldMode), static_cast<int>(newMode));
        });
        
        g_federatedPowerManager->setEmergencyCallback([]() {
            Serial.println("Power emergency detected - suspending federated learning");
        });
        
        return true;
    }
    
    bool setupWebInterface() {
        FederatedWebConfig webConfig;
        
        // Enable all web interface features
        webConfig.enableFederatedDashboard = true;
        webConfig.enableConfigurationUI = true;
        webConfig.enableRealTimeMetrics = true;
        webConfig.enableExpertValidation = true;
        
        // Security settings
        webConfig.requireAuthentication = true;
        webConfig.authUsername = "wildlife_admin";
        webConfig.authPassword = "federated_2024";
        
        // Update frequency
        webConfig.metricsUpdateInterval = 10000; // 10 seconds for wildlife monitoring
        
        if (!initializeFederatedWebInterface(webConfig)) {
            Serial.println("Failed to initialize federated web interface");
            return false;
        }
        
        Serial.println("Federated Learning Web Interface available at: /federated");
        return true;
    }
};

/**
 * Example: Processing wildlife observation with federated learning
 */
void processWildlifeObservation(const CameraFrame& frame) {
    // Process frame with federated learning integration
    FederatedWildlifeResult result = processWildlifeFrameWithFederation(frame);
    
    // Log the analysis
    Serial.printf("Wildlife detected: species=%s, behavior=%s, confidence=%.2f\n",
                  speciesTypeToString(result.species),
                  behaviorTypeToString(result.behavior),
                  result.confidence);
    
    // Check if contributed to federation
    if (result.contributedToFederation) {
        Serial.printf("Contributed to federated learning (Round: %s, Privacy: %s)\n",
                      result.federationRoundId.c_str(),
                      privacyLevelToString(result.privacyLevel));
    }
    
    // Save observation with metadata
    saveWildlifeObservation(result, frame);
}

/**
 * Example: Expert validation workflow for research deployments
 */
void setupExpertValidation() {
    g_aiFederatedIntegration->setExpertValidationRequestCallback([](const String& validationId) {
        Serial.printf("Expert validation requested for observation: %s\n", validationId.c_str());
        
        // In a real implementation, this would:
        // 1. Send notification to researchers
        // 2. Upload observation to validation platform
        // 3. Wait for expert feedback
        
        // For this example, simulate expert validation after 30 seconds
        // (In practice, this would be done through the web interface)
    });
}

/**
 * Example: Monitoring federated learning status
 */
void printFederatedLearningStatus() {
    FLSystemStats stats = g_federatedLearningSystem->getSystemStats();
    
    Serial.println("=== Federated Learning Status ===");
    Serial.printf("Status: %d\n", static_cast<int>(stats.status));
    Serial.printf("Total Rounds: %u\n", stats.totalRounds);
    Serial.printf("Successful Rounds: %u\n", stats.successfulRounds);
    Serial.printf("Success Rate: %.1f%%\n", 
                  stats.totalRounds > 0 ? (stats.successfulRounds * 100.0f / stats.totalRounds) : 0.0f);
    Serial.printf("Accuracy Improvement: %.2f%%\n", stats.averageAccuracyImprovement * 100);
    Serial.printf("Models Contributed: %u\n", stats.modelsContributed);
    Serial.printf("Privacy Budget Used: %.3f\n", stats.totalPrivacyBudgetUsed);
    Serial.printf("Network Nodes: %u\n", stats.networkNodes);
    Serial.printf("Battery Level: %.1f%%\n", stats.batteryLevel * 100);
    Serial.printf("Memory Usage: %u KB\n", stats.memoryUsage / 1024);
    
    NetworkMetrics network = getCurrentNetworkMetrics();
    Serial.printf("Connected Nodes: %u\n", network.connectedNodes);
    Serial.printf("Network Reliability: %.2f\n", network.averageReliability);
    Serial.printf("Average Latency: %u ms\n", network.averageLatency);
    
    FLPowerMetrics power = g_federatedPowerManager->getPowerMetrics();
    Serial.printf("Current Power Mode: %d\n", static_cast<int>(g_federatedPowerManager->getCurrentPowerMode()));
    Serial.printf("Solar Power: %.1f mW\n", power.currentSolarPower);
    Serial.printf("Power Efficiency: %.2f ops/mWh\n", power.powerEfficiency);
}

/**
 * Example: Complete setup function for main.cpp integration
 */
bool setupWildlifeCameraWithFederatedLearning() {
    Serial.println("Initializing Wildlife Camera with Federated Learning...");
    
    WildlifeCameraWithFederatedLearning camera;
    
    if (!camera.setupFederatedLearning()) {
        Serial.println("Failed to setup federated learning");
        return false;
    }
    
    // Setup expert validation for research deployments
    setupExpertValidation();
    
    Serial.println("Federated Learning Wildlife Camera initialized successfully!");
    Serial.println("Web dashboard available at: http://[device-ip]/federated");
    
    return true;
}

#endif // FEDERATED_LEARNING_EXAMPLE_H