/**
 * @file node_failure_detection_demo.cpp
 * @brief Demonstration of node failure detection and task reassignment
 * 
 * This example shows how the coordinator detects failed nodes and 
 * automatically reassigns their tasks to healthy nodes in the network.
 */

#include <Arduino.h>
#include "../src/multi_board/board_coordinator.h"
#include "../src/multi_board/message_protocol.h"
#include "../src/lora_mesh.h"

// Coordinator instance
BoardCoordinator coordinator;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=================================================");
    Serial.println("Node Failure Detection & Task Reassignment Demo");
    Serial.println("=================================================\n");
    
    // Initialize LoRa mesh
    if (!LoraMesh::init()) {
        Serial.println("Failed to initialize LoRa mesh");
        return;
    }
    
    // Initialize coordinator with node ID 1
    if (!coordinator.init(1)) {
        Serial.println("Failed to initialize coordinator");
        return;
    }
    
    // Configure network settings
    NetworkConfig config = coordinator.getNetworkConfig();
    config.heartbeatInterval = 30000;  // 30 seconds
    config.taskTimeout = 300000;       // 5 minutes
    config.enableLoadBalancing = true;
    config.enableAutomaticRoleAssignment = true;
    coordinator.setNetworkConfig(config);
    
    // Start coordinator role
    if (coordinator.startCoordinator()) {
        Serial.println("✓ Coordinator started successfully");
    } else {
        Serial.println("✗ Failed to start coordinator");
        return;
    }
    
    Serial.println("\nWaiting for nodes to join the network...");
    Serial.println("The coordinator will automatically:");
    Serial.println("  1. Detect nodes that don't send heartbeats for 60 seconds");
    Serial.println("  2. Mark those nodes as failed");
    Serial.println("  3. Reassign their tasks to healthy nodes");
    Serial.println("  4. Log all failure and reassignment events");
    Serial.println();
}

void loop() {
    // Process coordinator operations
    coordinator.process();
    
    // Display status every 10 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 10000) {
        lastStatus = millis();
        displayStatus();
    }
    
    delay(100);
}

void displayStatus() {
    Serial.println("\n--- Network Status ---");
    
    // Get coordinator stats
    auto stats = coordinator.getStats();
    Serial.printf("Managed Nodes: %d\n", stats.managedNodes);
    Serial.printf("Active Tasks: %d\n", stats.activeTasks);
    Serial.printf("Completed Tasks: %d\n", stats.completedTasks);
    Serial.printf("Failed Tasks: %d\n", stats.failedTasks);
    Serial.printf("Network Efficiency: %.1f%%\n", stats.networkEfficiency * 100);
    Serial.printf("Uptime: %lu seconds\n", stats.uptimeMs / 1000);
    
    // Show node details
    const auto& nodes = coordinator.getManagedNodes();
    if (nodes.empty()) {
        Serial.println("\nNo nodes in network yet");
    } else {
        Serial.println("\nActive Nodes:");
        for (const auto& node : nodes) {
            if (node.isActive) {
                unsigned long timeSinceHeartbeat = (millis() - node.lastSeen) / 1000;
                Serial.printf("  • Node %d (%s) - Last seen: %lu sec ago, Signal: %d dBm\n",
                             node.nodeId,
                             MessageProtocol::roleToString(node.role),
                             timeSinceHeartbeat,
                             node.signalStrength);
            }
        }
        
        // Show failed nodes
        bool hasFailedNodes = false;
        for (const auto& node : nodes) {
            if (!node.isActive) {
                if (!hasFailedNodes) {
                    Serial.println("\nFailed Nodes:");
                    hasFailedNodes = true;
                }
                unsigned long timeSinceHeartbeat = (millis() - node.lastSeen) / 1000;
                Serial.printf("  ✗ Node %d (%s) - Failed %lu sec ago\n",
                             node.nodeId,
                             MessageProtocol::roleToString(node.role),
                             timeSinceHeartbeat);
            }
        }
    }
    
    // Show active tasks
    const auto& tasks = coordinator.getActiveTasks();
    if (!tasks.empty()) {
        Serial.println("\nActive Tasks:");
        for (const auto& task : tasks) {
            unsigned long timeRemaining = (task.deadline - millis()) / 1000;
            Serial.printf("  • Task %d (%s) -> Node %d, Priority: %d, Deadline: %lu sec\n",
                         task.taskId,
                         task.taskType.c_str(),
                         task.assignedNode,
                         task.priority,
                         timeRemaining);
        }
    }
    
    Serial.println("----------------------\n");
}

/**
 * Example: Manually trigger a task assignment
 * This can be called from loop() to test task reassignment
 */
void assignTestTask() {
    static int taskCounter = 0;
    const auto& nodes = coordinator.getManagedNodes();
    
    if (nodes.empty()) {
        Serial.println("No nodes available for task assignment");
        return;
    }
    
    // Find an active node
    for (const auto& node : nodes) {
        if (node.isActive && node.nodeId != 1) {  // Skip coordinator
            // Create task parameters
            DynamicJsonDocument doc(512);
            JsonObject params = doc.to<JsonObject>();
            params["test_param"] = taskCounter++;
            
            // Assign task
            bool success = coordinator.assignTask(
                "TEST_TASK",        // Task type
                node.nodeId,        // Target node
                params,             // Parameters
                1,                  // Priority
                millis() + 60000    // Deadline (60 seconds)
            );
            
            if (success) {
                Serial.printf("✓ Assigned test task to node %d\n", node.nodeId);
            } else {
                Serial.printf("✗ Failed to assign task to node %d\n", node.nodeId);
            }
            break;
        }
    }
}

/**
 * Example: Monitor for node failures
 * This shows what the console output looks like when a node fails
 * 
 * Expected Console Output:
 * ------------------------
 * ❌ NODE FAILURE DETECTED: Node 3 - Last seen 62345 ms ago (timeout: 60000 ms)
 *    Node details: Role=AI_PROCESSOR, Signal=-75 dBm, HopCount=2
 *    Reassigning 2 task(s) from failed node 3
 *    ✓ Task 101 (AI_INFERENCE) reassigned: 3 -> 5 (Priority: 2)
 *    ✓ Task 102 (IMAGE_STORAGE) reassigned: 3 -> 4 (Priority: 1)
 * ⚠️  Network degradation: 1 node(s) failed, 4 active nodes remaining
 * 📡 Network topology broadcast: 5 nodes
 */
