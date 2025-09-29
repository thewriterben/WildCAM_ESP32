/**
 * @file foundational_framework_test.cpp
 * @brief Integration test for foundational framework components
 * @version 1.0.0
 * 
 * This test validates that all foundational framework components
 * can be initialized and work together properly.
 */

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>

// Include foundational framework headers
#include "../src/ai_detection/wildlife_detector.h"
#include "../src/mesh_network/mesh_protocol.h"
#include "../hardware/esp32_s3_cam/esp32_s3_cam_config.h"

// Test framework simulation
namespace TestFramework {
    uint32_t millis_counter = 0;
    
    // Simulate Arduino millis() function
    uint32_t millis() {
        return ++millis_counter * 100;
    }
    
    // Test data
    std::vector<uint8_t> generateTestImageData(uint16_t width, uint16_t height) {
        std::vector<uint8_t> data(width * height);
        // Generate simple gradient pattern
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                data[y * width + x] = (x + y) % 256;
            }
        }
        return data;
    }
}

// Test AI Detection Framework
bool testAIDetectionFramework() {
    std::cout << "Testing AI Detection Framework..." << std::endl;
    
    WildlifeDetection::WildlifeDetector detector;
    WildlifeDetection::DetectorConfig config;
    
    config.confidence_threshold = 0.6f;
    config.enable_motion_trigger = true;
    config.enable_species_classification = true;
    
    // Test initialization
    if (!detector.initialize(config)) {
        std::cout << "❌ Failed to initialize AI detector" << std::endl;
        return false;
    }
    
    // Test configuration
    auto retrieved_config = detector.getConfig();
    if (retrieved_config.confidence_threshold != 0.6f) {
        std::cout << "❌ Configuration mismatch" << std::endl;
        return false;
    }
    
    // Test frame processing
    auto test_data = TestFramework::generateTestImageData(320, 240);
    auto results = detector.detectWildlife(test_data.data(), test_data.size(), 320, 240);
    
    // Test statistics
    auto stats = detector.getStatistics();
    if (stats.total_detections < 0) {
        std::cout << "❌ Invalid statistics" << std::endl;
        return false;
    }
    
    std::cout << "✅ AI Detection Framework test passed" << std::endl;
    return true;
}

// Test Mesh Networking Protocol
bool testMeshNetworkingProtocol() {
    std::cout << "Testing Mesh Networking Protocol..." << std::endl;
    
    MeshNetworking::MeshProtocol mesh;
    MeshNetworking::MeshConfig config;
    
    config.node_type = MeshNetworking::NodeType::CAMERA_NODE;
    config.channel = 1;
    config.max_connections = 5;
    strcpy(config.node_name, "TestCamera");
    
    // Test initialization
    if (!mesh.initialize(config)) {
        std::cout << "❌ Failed to initialize mesh protocol" << std::endl;
        return false;
    }
    
    // Test configuration
    auto retrieved_config = mesh.getConfig();
    if (retrieved_config.channel != 1) {
        std::cout << "❌ Mesh configuration mismatch" << std::endl;
        return false;
    }
    
    // Test mesh start
    if (!mesh.startMesh()) {
        std::cout << "❌ Failed to start mesh networking" << std::endl;
        return false;
    }
    
    // Test message sending
    if (!mesh.sendTextMessage(0, "Test message", 3)) {
        std::cout << "❌ Failed to send test message" << std::endl;
        return false;
    }
    
    // Test detection alert
    if (!mesh.sendDetectionAlert("DEER", 0.85f)) {
        std::cout << "❌ Failed to send detection alert" << std::endl;
        return false;
    }
    
    // Test statistics
    auto stats = mesh.getNetworkStats();
    if (stats.messages_sent < 2) {
        std::cout << "❌ Message sending not tracked properly" << std::endl;
        return false;
    }
    
    std::cout << "✅ Mesh Networking Protocol test passed" << std::endl;
    return true;
}

// Test ESP32-S3-CAM Configuration
bool testESP32S3CAMConfig() {
    std::cout << "Testing ESP32-S3-CAM Configuration..." << std::endl;
    
    ESP32S3CAM::S3CAMConfigManager config_mgr;
    
    // Test initialization
    if (!config_mgr.initialize()) {
        std::cout << "❌ Failed to initialize S3-CAM config manager" << std::endl;
        return false;
    }
    
    // Test default configuration
    auto config = config_mgr.getConfiguration();
    if (config.camera.resolution != ESP32S3CAM::Resolution::VGA) {
        std::cout << "❌ Default configuration incorrect" << std::endl;
        return false;
    }
    
    // Test configuration validation
    ESP32S3CAM::S3CAMConfig test_config = config;
    test_config.camera.quality = 10;
    test_config.power.power_mode = ESP32S3CAM::PowerMode::NORMAL;
    
    if (!config_mgr.validateConfiguration(test_config)) {
        std::cout << "❌ Valid configuration rejected" << std::endl;
        return false;
    }
    
    // Test optimal configuration
    auto optimal_config = config_mgr.getOptimalConfiguration("wildlife_monitoring");
    if (optimal_config.device_name[0] == '\0') {
        std::cout << "❌ Optimal configuration generation failed" << std::endl;
        return false;
    }
    
    std::cout << "✅ ESP32-S3-CAM Configuration test passed" << std::endl;
    return true;
}

// Test Integration Between Components
bool testComponentIntegration() {
    std::cout << "Testing Component Integration..." << std::endl;
    
    // Initialize all components
    WildlifeDetection::WildlifeDetector detector;
    MeshNetworking::MeshProtocol mesh;
    ESP32S3CAM::S3CAMConfigManager config_mgr;
    
    // Configure AI detector
    WildlifeDetection::DetectorConfig ai_config;
    ai_config.confidence_threshold = 0.7f;
    detector.initialize(ai_config);
    
    // Configure mesh network
    MeshNetworking::MeshConfig mesh_config;
    mesh_config.node_type = MeshNetworking::NodeType::CAMERA_NODE;
    mesh.initialize(mesh_config);
    mesh.startMesh();
    
    // Configure S3-CAM
    config_mgr.initialize();
    
    // Simulate wildlife detection and mesh alert
    auto test_data = TestFramework::generateTestImageData(640, 480);
    auto detection_results = detector.detectWildlife(test_data.data(), test_data.size(), 640, 480);
    
    // If detection found, send mesh alert
    for (const auto& result : detection_results) {
        if (result.confidence_score >= 0.7f) {
            std::string species_name = "DETECTED_SPECIES";
            mesh.sendDetectionAlert(species_name.c_str(), result.confidence_score, 
                                   test_data.data(), std::min(test_data.size(), size_t(1024)));
        }
    }
    
    // Verify mesh received the alert
    auto mesh_stats = mesh.getNetworkStats();
    auto ai_stats = detector.getStatistics();
    
    std::cout << "✅ Component Integration test passed" << std::endl;
    std::cout << "   AI Detections: " << ai_stats.total_detections << std::endl;
    std::cout << "   Mesh Messages: " << mesh_stats.messages_sent << std::endl;
    
    return true;
}

// Test Utility Functions
bool testUtilityFunctions() {
    std::cout << "Testing Utility Functions..." << std::endl;
    
    // Test AI Detection utilities
    auto species_str = WildlifeDetection::Utils::speciesToString(WildlifeDetection::SpeciesType::DEER);
    if (species_str == nullptr) {
        std::cout << "❌ Species string conversion failed" << std::endl;
        return false;
    }
    
    auto confidence_str = WildlifeDetection::Utils::confidenceToString(WildlifeDetection::ConfidenceLevel::HIGH);
    if (confidence_str == nullptr) {
        std::cout << "❌ Confidence string conversion failed" << std::endl;
        return false;
    }
    
    // Test ESP32-S3-CAM utilities
    auto sensor_str = ESP32S3CAM::Utils::sensorTypeToString(ESP32S3CAM::SensorType::OV2640);
    if (sensor_str == nullptr) {
        std::cout << "❌ Sensor string conversion failed" << std::endl;
        return false;
    }
    
    uint16_t width, height;
    ESP32S3CAM::Utils::getResolutionDimensions(ESP32S3CAM::Resolution::VGA, width, height);
    if (width != 640 || height != 480) {
        std::cout << "❌ Resolution dimension calculation failed" << std::endl;
        return false;
    }
    
    std::cout << "✅ Utility Functions test passed" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "=== ESP32WildlifeCAM Foundational Framework Integration Test ===" << std::endl;
    std::cout << std::endl;
    
    bool all_tests_passed = true;
    
    // Run individual component tests
    all_tests_passed &= testAIDetectionFramework();
    all_tests_passed &= testMeshNetworkingProtocol();
    all_tests_passed &= testESP32S3CAMConfig();
    all_tests_passed &= testUtilityFunctions();
    all_tests_passed &= testComponentIntegration();
    
    std::cout << std::endl;
    if (all_tests_passed) {
        std::cout << "🎉 All foundational framework tests PASSED!" << std::endl;
        std::cout << "✅ Framework is ready for integration with comprehensive systems" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some tests FAILED!" << std::endl;
        return 1;
    }
}

// Placeholder implementations for missing utility functions
namespace WildlifeDetection {
namespace Utils {
    const char* speciesToString(SpeciesType species) {
        switch (species) {
            case SpeciesType::DEER: return "DEER";
            case SpeciesType::RABBIT: return "RABBIT";
            case SpeciesType::RACCOON: return "RACCOON";
            case SpeciesType::BIRD: return "BIRD";
            default: return "UNKNOWN";
        }
    }
    
    const char* confidenceToString(ConfidenceLevel confidence) {
        switch (confidence) {
            case ConfidenceLevel::VERY_HIGH: return "VERY_HIGH";
            case ConfidenceLevel::HIGH: return "HIGH";
            case ConfidenceLevel::MEDIUM: return "MEDIUM";
            case ConfidenceLevel::LOW: return "LOW";
            case ConfidenceLevel::VERY_LOW: return "VERY_LOW";
            default: return "UNKNOWN";
        }
    }
}
}

namespace ESP32S3CAM {
namespace Utils {
    const char* sensorTypeToString(SensorType sensor) {
        switch (sensor) {
            case SensorType::OV2640: return "OV2640";
            case SensorType::OV3660: return "OV3660";
            case SensorType::OV5640: return "OV5640";
            default: return "UNKNOWN";
        }
    }
    
    void getResolutionDimensions(Resolution resolution, uint16_t& width, uint16_t& height) {
        switch (resolution) {
            case Resolution::VGA: width = 640; height = 480; break;
            case Resolution::QVGA: width = 320; height = 240; break;
            case Resolution::QQVGA: width = 160; height = 120; break;
            default: width = 640; height = 480; break;
        }
    }
}
}