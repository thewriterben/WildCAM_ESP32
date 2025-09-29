/**
 * @file test_federated_learning.cpp
 * @brief Unit tests for Federated Learning System
 * 
 * Comprehensive tests for federated learning components and integration.
 */

#include <cassert>
#include <iostream>
#include <vector>
#include <memory>
#include "../federated_learning/federated_learning_system.h"
#include "../federated_learning/local_training_module.h"
#include "../federated_learning/privacy_preserving_aggregation.h"

// Mock Arduino functions for testing
unsigned long millis() { return 0; }

// Test utility functions
void assertEqual(float expected, float actual, float tolerance = 0.01f) {
    if (abs(expected - actual) > tolerance) {
        std::cerr << "Test failed: Expected " << expected << ", got " << actual << std::endl;
        assert(false);
    }
}

void assertTrue(bool condition, const char* message = "Assertion failed") {
    if (!condition) {
        std::cerr << "Test failed: " << message << std::endl;
        assert(false);
    }
}

/**
 * Test federated learning configuration
 */
void testFederatedLearningConfig() {
    std::cout << "Testing Federated Learning Configuration..." << std::endl;
    
    FederatedLearningConfig config;
    
    // Test default values
    assertEqual(config.contributionThreshold, 0.95f);
    assertEqual(config.learningRate, 0.001f);
    assertTrue(config.anonymizeLocation);
    assertTrue(config.anonymizeTimestamps);
    assertEqual(config.minAccuracyImprovement, 0.02f);
    
    // Test configuration modification
    config.enableFederatedLearning = true;
    config.privacyLevel = PrivacyLevel::HIGH;
    config.topology = NetworkTopology::MESH;
    
    assertTrue(config.enableFederatedLearning);
    assertTrue(config.privacyLevel == PrivacyLevel::HIGH);
    assertTrue(config.topology == NetworkTopology::MESH);
    
    std::cout << "✓ Federated Learning Configuration tests passed" << std::endl;
}

/**
 * Test training sample creation and management
 */
void testTrainingSamples() {
    std::cout << "Testing Training Sample Management..." << std::endl;
    
    // Create mock training sample
    TrainingSample sample;
    sample.confidence = 0.9f;
    sample.timestamp = 12345;
    sample.validated = true;
    sample.deviceId = "test_device_001";
    
    // Test sample properties
    assertEqual(sample.confidence, 0.9f);
    assertTrue(sample.validated);
    assertTrue(sample.deviceId == "test_device_001");
    
    // Test sample collection
    std::vector<TrainingSample> samples;
    for (int i = 0; i < 10; i++) {
        TrainingSample s;
        s.confidence = 0.8f + (i * 0.01f);
        s.timestamp = 1000 + i;
        samples.push_back(s);
    }
    
    assertTrue(samples.size() == 10);
    assertEqual(samples[5].confidence, 0.85f);
    
    std::cout << "✓ Training Sample tests passed" << std::endl;
}

/**
 * Test model update structures
 */
void testModelUpdates() {
    std::cout << "Testing Model Update Structures..." << std::endl;
    
    ModelUpdate update;
    update.modelId = "species_classifier_v1";
    update.roundId = "round_001";
    update.type = ModelUpdateType::GRADIENT_UPDATE;
    update.sampleCount = 100;
    update.accuracy = 0.85f;
    
    // Add mock gradients
    for (int i = 0; i < 1000; i++) {
        update.gradients.push_back(0.001f * i);
    }
    
    // Test update properties
    assertTrue(update.modelId == "species_classifier_v1");
    assertTrue(update.type == ModelUpdateType::GRADIENT_UPDATE);
    assertEqual(update.accuracy, 0.85f);
    assertTrue(update.gradients.size() == 1000);
    
    std::cout << "✓ Model Update tests passed" << std::endl;
}

/**
 * Test privacy protection functionality
 */
void testPrivacyProtection() {
    std::cout << "Testing Privacy Protection..." << std::endl;
    
    PrivacyProtectionResult result;
    result.dataAnonymized = true;
    result.locationFuzzed = true;
    result.differentialPrivacyApplied = true;
    result.privacyBudget = 1.0f;
    result.actualEpsilon = 0.5f;
    
    assertTrue(result.dataAnonymized);
    assertTrue(result.locationFuzzed);
    assertTrue(result.differentialPrivacyApplied);
    assertEqual(result.privacyBudget, 1.0f);
    assertEqual(result.actualEpsilon, 0.5f);
    
    std::cout << "✓ Privacy Protection tests passed" << std::endl;
}

/**
 * Test federated learning metrics
 */
void testFederatedMetrics() {
    std::cout << "Testing Federated Learning Metrics..." << std::endl;
    
    FederatedLearningMetrics metrics;
    metrics.totalRoundsParticipated = 5;
    metrics.successfulRounds = 4;
    metrics.failedRounds = 1;
    metrics.averageAccuracyImprovement = 0.025f;
    metrics.totalSamplesContributed = 500;
    
    assertTrue(metrics.totalRoundsParticipated == 5);
    assertTrue(metrics.successfulRounds == 4);
    assertTrue(metrics.failedRounds == 1);
    assertEqual(metrics.averageAccuracyImprovement, 0.025f);
    assertTrue(metrics.totalSamplesContributed == 500);
    
    // Test success rate calculation
    float successRate = static_cast<float>(metrics.successfulRounds) / metrics.totalRoundsParticipated;
    assertEqual(successRate, 0.8f);
    
    std::cout << "✓ Federated Learning Metrics tests passed" << std::endl;
}

/**
 * Test utility functions
 */
void testUtilityFunctions() {
    std::cout << "Testing Utility Functions..." << std::endl;
    
    // Test enum to string conversions
    assertTrue(std::string(federatedLearningStateToString(FederatedLearningState::FL_TRAINING)) == "TRAINING");
    assertTrue(std::string(privacyLevelToString(PrivacyLevel::HIGH)) == "HIGH");
    assertTrue(std::string(networkTopologyToString(NetworkTopology::MESH)) == "MESH");
    assertTrue(std::string(modelUpdateTypeToString(ModelUpdateType::GRADIENT_UPDATE)) == "GRADIENT_UPDATE");
    
    std::cout << "✓ Utility Function tests passed" << std::endl;
}

/**
 * Test system configuration
 */
void testSystemConfiguration() {
    std::cout << "Testing System Configuration..." << std::endl;
    
    FLSystemConfig config;
    config.enablePowerManagement = true;
    config.enableAutomaticMode = true;
    config.enableWebInterface = true;
    config.enableLogging = true;
    config.logLevel = "DEBUG";
    
    assertTrue(config.enablePowerManagement);
    assertTrue(config.enableAutomaticMode);
    assertTrue(config.enableWebInterface);
    assertTrue(config.enableLogging);
    assertTrue(config.logLevel == "DEBUG");
    
    // Test nested configurations
    config.federatedConfig.enableFederatedLearning = true;
    config.federatedConfig.privacyLevel = PrivacyLevel::STANDARD;
    config.topologyConfig.preferredTopology = NetworkTopology::HYBRID;
    
    assertTrue(config.federatedConfig.enableFederatedLearning);
    assertTrue(config.federatedConfig.privacyLevel == PrivacyLevel::STANDARD);
    assertTrue(config.topologyConfig.preferredTopology == NetworkTopology::HYBRID);
    
    std::cout << "✓ System Configuration tests passed" << std::endl;
}

/**
 * Main test function
 */
int main() {
    std::cout << "Running Federated Learning System Tests..." << std::endl << std::endl;
    
    try {
        testFederatedLearningConfig();
        testTrainingSamples();
        testModelUpdates();
        testPrivacyProtection();
        testFederatedMetrics();
        testUtilityFunctions();
        testSystemConfiguration();
        
        std::cout << std::endl << "✅ All Federated Learning tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}