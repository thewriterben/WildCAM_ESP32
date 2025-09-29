/**
 * @file multi_board_farm_example.cpp
 * @brief Example demonstrating multi-board deployment scenario
 * 
 * This example shows how different ESP32 camera boards can work together
 * in a wildlife monitoring network, each optimized for its specific role.
 */

#include <Arduino.h>
#include "../firmware/src/camera_handler.h"
#include "../firmware/src/hal/board_detector.h"
#include "../firmware/src/configs/sensor_configs.h"

// Deployment configuration based on detected board
struct DeploymentConfig {
    const char* role;
    uint32_t capture_interval_ms;
    uint8_t jpeg_quality;
    framesize_t frame_size;
    bool enable_ai_processing;
    bool enable_mesh_relay;
    uint32_t sleep_duration_ms;
};

// Get deployment configuration based on board type and capabilities
DeploymentConfig getDeploymentConfig(BoardType board_type, SensorType sensor_type) {
    DeploymentConfig config;
    
    switch (board_type) {
        case BOARD_AI_THINKER_ESP32_CAM:
            config.role = "Edge Sensor Node";
            config.capture_interval_ms = 300000; // 5 minutes
            config.jpeg_quality = 15;
            config.frame_size = FRAMESIZE_UXGA;
            config.enable_ai_processing = false;
            config.enable_mesh_relay = true;
            config.sleep_duration_ms = 60000; // 1 minute sleep
            break;
            
        case BOARD_ESP32_S3_CAM:
            config.role = "High-Resolution Hub";
            config.capture_interval_ms = 600000; // 10 minutes
            config.jpeg_quality = 8;
            config.frame_size = FRAMESIZE_QXGA;
            config.enable_ai_processing = true;
            config.enable_mesh_relay = true;
            config.sleep_duration_ms = 30000; // 30 seconds sleep
            break;
            
        case BOARD_ESP_EYE:
            config.role = "AI Processing Center";
            config.capture_interval_ms = 180000; // 3 minutes
            config.jpeg_quality = 10;
            config.frame_size = FRAMESIZE_UXGA;
            config.enable_ai_processing = true;
            config.enable_mesh_relay = true;
            config.sleep_duration_ms = 15000; // 15 seconds sleep
            break;
            
        case BOARD_M5STACK_TIMER_CAM:
            config.role = "Portable Monitor";
            config.capture_interval_ms = 900000; // 15 minutes
            config.jpeg_quality = 12;
            config.frame_size = FRAMESIZE_SVGA;
            config.enable_ai_processing = false;
            config.enable_mesh_relay = false;
            config.sleep_duration_ms = 120000; // 2 minutes sleep
            break;
            
        case BOARD_XIAO_ESP32S3_SENSE:
            config.role = "Stealth Sensor";
            config.capture_interval_ms = 1800000; // 30 minutes
            config.jpeg_quality = 18;
            config.frame_size = FRAMESIZE_VGA;
            config.enable_ai_processing = false;
            config.enable_mesh_relay = false;
            config.sleep_duration_ms = 300000; // 5 minutes sleep
            break;
            
        default:
            config.role = "Generic Node";
            config.capture_interval_ms = 600000; // 10 minutes
            config.jpeg_quality = 15;
            config.frame_size = FRAMESIZE_SVGA;
            config.enable_ai_processing = false;
            config.enable_mesh_relay = true;
            config.sleep_duration_ms = 60000; // 1 minute sleep
            break;
    }
    
    // Adjust based on sensor capabilities
    const SensorCapabilities* sensor_caps = getSensorCapabilities(sensor_type);
    if (sensor_caps) {
        // Adjust frame size based on sensor maximum
        if (sensor_caps->max_width < 1600) {
            config.frame_size = FRAMESIZE_SVGA;
        }
        
        // Adjust capture interval for low-power sensors
        if (sensor_caps->power_consumption_mw < 100) {
            config.capture_interval_ms *= 2; // Double interval for power savings
            config.sleep_duration_ms *= 2;
        }
        
        // Enable AI processing for high-resolution sensors
        if (sensor_caps->max_width >= 2048 && config.enable_ai_processing) {
            config.jpeg_quality -= 2; // Higher quality for AI processing
        }
    }
    
    return config;
}

void printSystemInformation() {
    Serial.println("ESP32 Wildlife Camera - Multi-Board Farm Example");
    Serial.println("================================================");
    
    // System information
    Serial.printf("Chip Model: %s\n", BoardDetector::getChipModel());
    Serial.printf("Chip ID: 0x%08X\n", BoardDetector::getChipId());
    Serial.printf("PSRAM: %s\n", BoardDetector::hasPSRAM() ? "Available" : "Not Available");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    if (BoardDetector::hasPSRAM()) {
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    }
    Serial.println();
}

void configureForDeployment(const DeploymentConfig& config) {
    Serial.println("Deployment Configuration:");
    Serial.printf("  Role: %s\n", config.role);
    Serial.printf("  Capture Interval: %lu ms (%.1f minutes)\n", 
                  config.capture_interval_ms, config.capture_interval_ms / 60000.0);
    Serial.printf("  JPEG Quality: %d\n", config.jpeg_quality);
    Serial.printf("  Frame Size: %d\n", config.frame_size);
    Serial.printf("  AI Processing: %s\n", config.enable_ai_processing ? "Enabled" : "Disabled");
    Serial.printf("  Mesh Relay: %s\n", config.enable_mesh_relay ? "Enabled" : "Disabled");
    Serial.printf("  Sleep Duration: %lu ms (%.1f minutes)\n", 
                  config.sleep_duration_ms, config.sleep_duration_ms / 60000.0);
    Serial.println();
}

void performRoleSpecificTasks(const DeploymentConfig& config, CameraBoard* board) {
    Serial.printf("Performing %s tasks...\n", config.role);
    
    if (strcmp(config.role, "AI Processing Center") == 0) {
        Serial.println("  - Running AI species classification");
        Serial.println("  - Processing mesh network data");
        Serial.println("  - Coordinating with satellite uplink");
        
        // Simulate AI processing workload
        board->flashLED(true);
        delay(200);
        board->flashLED(false);
        delay(100);
        board->flashLED(true);
        delay(200);
        board->flashLED(false);
        
    } else if (strcmp(config.role, "High-Resolution Hub") == 0) {
        Serial.println("  - Capturing high-resolution imagery");
        Serial.println("  - Managing local data storage");
        Serial.println("  - Relaying mesh network traffic");
        
        // Simulate high-res capture
        board->flashLED(true);
        delay(500);
        board->flashLED(false);
        
    } else if (strcmp(config.role, "Edge Sensor Node") == 0) {
        Serial.println("  - Motion detection and basic capture");
        Serial.println("  - Battery optimization");
        Serial.println("  - Mesh network participation");
        
        // Quick flash for edge node
        board->flashLED(true);
        delay(100);
        board->flashLED(false);
        
    } else if (strcmp(config.role, "Stealth Sensor") == 0) {
        Serial.println("  - Ultra-low power monitoring");
        Serial.println("  - Minimal RF emissions");
        Serial.println("  - Long-term deployment");
        
        // Very brief flash to minimize detection
        board->flashLED(true);
        delay(50);
        board->flashLED(false);
        
    } else if (strcmp(config.role, "Portable Monitor") == 0) {
        Serial.println("  - Rapid deployment monitoring");
        Serial.println("  - Built-in display status");
        Serial.println("  - Cellular connectivity");
        
        // Status pattern
        for (int i = 0; i < 3; i++) {
            board->flashLED(true);
            delay(100);
            board->flashLED(false);
            delay(100);
        }
    }
    
    Serial.println();
}

void simulateNetworkCoordination() {
    Serial.println("Network Coordination Simulation:");
    Serial.println("  [Hub] Broadcasting time sync signal");
    Serial.println("  [Edge Nodes] Adjusting capture schedules");
    Serial.println("  [AI Center] Processing accumulated data");
    Serial.println("  [Stealth Nodes] Minimal status update");
    Serial.println("  [Portable] Cellular data upload");
    Serial.println();
    
    Serial.println("📡 NEW MULTI-BOARD COORDINATION AVAILABLE:");
    Serial.println("  ✅ Automatic board discovery and role assignment");
    Serial.println("  ✅ Coordinator election based on capabilities");
    Serial.println("  ✅ Task distribution and load balancing");
    Serial.println("  ✅ Network topology mapping and monitoring");
    Serial.println("  ✅ Failover and standalone mode support");
    Serial.println("  ✅ Integration with existing LoRa mesh");
    Serial.println();
    Serial.println("  🚀 To use the new multi-board system:");
    Serial.println("     #include \"../firmware/src/multi_board/multi_board_system.h\"");
    Serial.println("     initializeMultiboardSystem(nodeId, preferredRole);");
    Serial.println("     // See examples/multi_board_communication/ for full example");
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    printSystemInformation();
    
    // Initialize camera with automatic board detection
    if (CameraHandler::init()) {
        Serial.println("Camera system initialized successfully!\n");
        
        // Get system status
        CameraStatus status = CameraHandler::getStatus();
        CameraBoard* board = CameraHandler::getBoard();
        
        Serial.println("Detected Hardware:");
        Serial.printf("  Board: %s\n", status.boardName);
        Serial.printf("  Sensor: %s\n", status.sensorName);
        Serial.printf("  Board Type: %d\n", status.boardType);
        Serial.printf("  Sensor Type: %d\n", status.sensorType);
        Serial.println();
        
        if (board) {
            // Get board-specific information
            PowerProfile power_profile = board->getPowerProfile();
            CameraConfig cam_config = board->getCameraConfig();
            
            Serial.println("Hardware Capabilities:");
            Serial.printf("  Max Resolution: %d\n", cam_config.max_framesize);
            Serial.printf("  PSRAM Required: %s\n", cam_config.psram_required ? "Yes" : "No");
            Serial.printf("  Sleep Current: %lu µA\n", power_profile.sleep_current_ua);
            Serial.printf("  Active Current: %lu mA\n", power_profile.active_current_ma);
            Serial.printf("  Deep Sleep Support: %s\n", power_profile.supports_deep_sleep ? "Yes" : "No");
            Serial.println();
            
            // Get deployment configuration for this board
            DeploymentConfig config = getDeploymentConfig(status.boardType, status.sensorType);
            configureForDeployment(config);
            
            // Simulate role-specific tasks
            performRoleSpecificTasks(config, board);
            
            // Simulate network coordination
            simulateNetworkCoordination();
            
            Serial.println("Multi-board farm simulation completed!");
            Serial.println("In a real deployment, this node would now:");
            Serial.printf("  - Enter sleep mode for %lu ms\n", config.sleep_duration_ms);
            Serial.printf("  - Wake up and capture image every %lu ms\n", config.capture_interval_ms);
            Serial.printf("  - Participate in mesh network as: %s\n", config.role);
            
        } else {
            Serial.println("Error: Unable to get board instance");
        }
        
    } else {
        Serial.println("Error: Camera initialization failed!");
        Serial.println("Check hardware connections and power supply.");
    }
}

void loop() {
    // In a real deployment, this would be the main monitoring loop
    static unsigned long last_status = 0;
    
    if (millis() - last_status > 10000) { // Status every 10 seconds
        CameraBoard* board = CameraHandler::getBoard();
        if (board) {
            // Show we're alive
            board->flashLED(true);
            delay(50);
            board->flashLED(false);
            
            Serial.println("Node Status: Active and monitoring...");
        }
        last_status = millis();
    }
    
    delay(100);
}