/**
 * @file multi_board_communication_example.cpp
 * @brief Example demonstrating the new multi-board communication system
 * 
 * This example shows how to use the new multi-board coordination features
 * to create a networked wildlife monitoring system with coordinated roles.
 */

#include <Arduino.h>
#include "../firmware/src/multi_board/multi_board_system.h"
#include "../firmware/src/multi_board/message_protocol.h"
#include "../firmware/src/hal/board_detector.h"

// Example node ID (in practice, this would be unique per device)
const int NODE_ID = 1;

// System instance
MultiboardSystem multiboardSystem;

void printSystemStatus() {
    SystemStats stats = multiboardSystem.getSystemStats();
    
    Serial.println("=== Multi-Board System Status ===");
    Serial.printf("State: %d\n", stats.currentState);
    Serial.printf("Role: %s\n", MessageProtocol::roleToString(stats.currentRole));
    Serial.printf("Uptime: %lu ms\n", stats.systemUptime);
    Serial.printf("Networked Nodes: %d\n", stats.networkedNodes);
    Serial.printf("Active Tasks: %d\n", stats.activeTasks);
    Serial.printf("Messages Processed: %d\n", stats.messagesProcessed);
    Serial.printf("Network Efficiency: %.2f\n", stats.networkEfficiency);
    Serial.println();
}

void printNetworkTopology() {
    auto topology = multiboardSystem.getNetworkTopology();
    
    Serial.println("=== Network Topology ===");
    Serial.printf("Total Nodes: %d\n", topology.totalNodes);
    Serial.printf("Active Nodes: %d\n", topology.activeNodes);
    Serial.printf("Coordinator: Node %d\n", topology.coordinatorNode);
    Serial.printf("Network Stable: %s\n", topology.isStable ? "Yes" : "No");
    
    for (const auto& node : topology.nodes) {
        Serial.printf("  Node %d: %s (Signal: %d, Active: %s)\n",
                     node.nodeId,
                     MessageProtocol::roleToString(node.role),
                     node.signalStrength,
                     node.isActive ? "Yes" : "No");
    }
    Serial.println();
}

void demonstrateTaskAssignment() {
    // Only coordinators can assign tasks
    if (multiboardSystem.getCurrentRole() == ROLE_COORDINATOR) {
        Serial.println("=== Demonstrating Task Assignment ===");
        
        // Create a sample task
        DynamicJsonDocument doc(512);
        JsonObject params = doc.to<JsonObject>();
        params["resolution"] = "UXGA";
        params["quality"] = 10;
        params["timeout"] = 30000;
        
        // Assign image capture task to discovered nodes
        auto nodes = multiboardSystem.getDiscoveredNodes();
        for (const auto& node : nodes) {
            if (node.nodeId != NODE_ID && node.isActive) {
                bool success = multiboardSystem.sendTaskToNode(
                    node.nodeId, "image_capture", params, 1);
                
                Serial.printf("Assigned image capture task to Node %d: %s\n",
                             node.nodeId, success ? "Success" : "Failed");
            }
        }
        Serial.println();
    }
}

void demonstrateConfigurationBroadcast() {
    // Only coordinators can broadcast configuration
    if (multiboardSystem.getCurrentRole() == ROLE_COORDINATOR) {
        Serial.println("=== Broadcasting Configuration Update ===");
        
        DynamicJsonDocument doc(512);
        JsonObject config = doc.to<JsonObject>();
        config["capture_interval"] = 300000; // 5 minutes
        config["jpeg_quality"] = 12;
        config["motion_threshold"] = 50;
        config["sleep_duration"] = 60000; // 1 minute
        
        bool success = multiboardSystem.broadcastConfiguration(config);
        Serial.printf("Configuration broadcast: %s\n", success ? "Success" : "Failed");
        Serial.println();
    }
}

void handleNodeTasks() {
    // Show active tasks for nodes
    if (multiboardSystem.getCurrentRole() != ROLE_COORDINATOR) {
        auto tasks = multiboardSystem.getActiveTasks();
        
        if (!tasks.empty()) {
            Serial.println("=== Active Node Tasks ===");
            for (const auto& task : tasks) {
                Serial.printf("Task %d: %s (Priority: %d, Status: %d)\n",
                             task.taskId, task.taskType.c_str(), 
                             task.priority, task.status);
            }
            Serial.println();
        }
    }
}

void simulateNetworkActivity() {
    static unsigned long lastActivity = 0;
    unsigned long now = millis();
    
    // Simulate network activity every 30 seconds
    if (now - lastActivity > 30000) {
        Serial.println("=== Simulating Network Activity ===");
        
        // Trigger discovery to find new nodes
        multiboardSystem.triggerDiscovery();
        
        // Demonstrate coordinator features
        demonstrateTaskAssignment();
        demonstrateConfigurationBroadcast();
        
        // Show node tasks
        handleNodeTasks();
        
        lastActivity = now;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Wildlife Camera - Multi-Board Communication Example");
    Serial.println("=========================================================");
    
    // Print system information
    Serial.printf("Chip Model: %s\n", BoardDetector::getChipModel());
    Serial.printf("Node ID: %d\n", NODE_ID);
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println();
    
    // Initialize multi-board system
    SystemConfig config;
    config.nodeId = NODE_ID;
    config.preferredRole = ROLE_NODE; // Start as node, coordinator will be elected
    config.enableAutomaticRoleSelection = true;
    config.enableStandaloneFallback = true;
    config.discoveryTimeout = 60000; // 1 minute discovery
    config.roleChangeTimeout = 30000; // 30 seconds for role changes
    
    Serial.println("Initializing multi-board communication system...");
    
    if (!multiboardSystem.init(config)) {
        Serial.println("Failed to initialize multi-board system!");
        Serial.printf("Error: %s\n", multiboardSystem.getLastError().c_str());
        return;
    }
    
    if (!multiboardSystem.start()) {
        Serial.println("Failed to start multi-board system!");
        Serial.printf("Error: %s\n", multiboardSystem.getLastError().c_str());
        return;
    }
    
    // Enable LoRa mesh integration
    multiboardSystem.integrateWithLoraMesh(true);
    
    Serial.println("Multi-board system initialized successfully!");
    Serial.println("Starting network discovery and coordination...");
    Serial.println();
}

void loop() {
    // Process multi-board system
    multiboardSystem.process();
    
    // Print status every 15 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 15000) {
        printSystemStatus();
        printNetworkTopology();
        lastStatus = millis();
    }
    
    // Simulate network activity
    simulateNetworkActivity();
    
    // Handle any error conditions
    if (!multiboardSystem.isOperational()) {
        Serial.printf("System error: %s\n", multiboardSystem.getLastError().c_str());
        delay(5000);
    }
    
    delay(100);
}