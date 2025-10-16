/**
 * @file node_failure_detection_example.cpp
 * @brief Example demonstrating node failure detection and task reassignment
 * 
 * This example shows how the coordinator automatically detects failed nodes
 * and reassigns their tasks to healthy nodes in the network.
 */

#include <Arduino.h>
#include "../firmware/src/multi_board/board_coordinator.h"
#include "../firmware/src/multi_board/message_protocol.h"

BoardCoordinator coordinator;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("========================================");
    Serial.println("Node Failure Detection Example");
    Serial.println("========================================");
    
    // Initialize coordinator
    if (!coordinator.init(1)) {
        Serial.println("Failed to initialize coordinator!");
        return;
    }
    
    // Start coordinator role
    if (!coordinator.startCoordinator()) {
        Serial.println("Failed to start coordinator!");
        return;
    }
    
    Serial.println("Coordinator started successfully");
    Serial.println("Node failure detection is active");
    Serial.println("Monitoring for node heartbeats...");
    Serial.println();
}

void loop() {
    // Process coordinator (includes automatic node health checks)
    coordinator.process();
    
    // Display statistics every 10 seconds
    static unsigned long lastStats = 0;
    if (millis() - lastStats >= 10000) {
        lastStats = millis();
        displayNetworkStatus();
    }
    
    delay(100);
}

void displayNetworkStatus() {
    BoardCoordinator::CoordinatorStats stats = coordinator.getStats();
    const std::vector<NetworkNode>& nodes = coordinator.getManagedNodes();
    
    Serial.println("\n========================================");
    Serial.println("NETWORK STATUS");
    Serial.println("========================================");
    Serial.printf("Managed Nodes: %d\n", stats.managedNodes);
    Serial.printf("Active Tasks: %d\n", stats.activeTasks);
    Serial.printf("Completed Tasks: %d\n", stats.completedTasks);
    Serial.printf("Network Uptime: %.2f minutes\n", stats.uptimeMs / 60000.0);
    Serial.printf("Network Efficiency: %.1f%%\n", stats.networkEfficiency * 100);
    Serial.println();
    
    if (nodes.size() > 0) {
        Serial.println("Node Health Status:");
        Serial.println("ID | Role | Status | Last Seen | Battery | Signal");
        Serial.println("---|------|--------|-----------|---------|-------");
        
        unsigned long now = millis();
        for (const auto& node : nodes) {
            // Skip coordinator itself
            if (node.nodeId == 1) continue;
            
            unsigned long timeSinceLastSeen = now - node.lastSeen;
            const char* status = node.isActive ? "ACTIVE" : "FAILED";
            
            Serial.printf("%2d | %-4s | %-6s | %5lu s | %3d%%    | %4d dBm\n",
                         node.nodeId,
                         MessageProtocol::roleToString(node.role),
                         status,
                         timeSinceLastSeen / 1000,
                         node.capabilities.batteryLevel,
                         node.signalStrength);
        }
    } else {
        Serial.println("No managed nodes detected yet");
    }
    
    Serial.println("========================================\n");
}

/**
 * Example Output:
 * 
 * ========================================
 * Node Failure Detection Example
 * ========================================
 * Coordinator started successfully
 * Node failure detection is active
 * Monitoring for node heartbeats...
 * 
 * ========================================
 * NETWORK STATUS
 * ========================================
 * Managed Nodes: 4
 * Active Tasks: 3
 * Completed Tasks: 12
 * Network Uptime: 5.25 minutes
 * Network Efficiency: 92.3%
 * 
 * Node Health Status:
 * ID | Role | Status | Last Seen | Battery | Signal
 * ---|------|--------|-----------|---------|-------
 *  2 | NODE | ACTIVE |    15 s   | 85%     |  -55 dBm
 *  3 | AI   | ACTIVE |    12 s   | 92%     |  -48 dBm
 *  4 | NODE | ACTIVE |    18 s   | 78%     |  -62 dBm
 *  5 | RELAY| FAILED |    67 s   | 45%     |  -85 dBm
 * ========================================
 * 
 * ⚠ Node 5 failure detected: no heartbeat for 67000 ms
 * ✗ Node 5 marked as FAILED
 * ========================================
 * NODE FAILURE EVENT
 * ========================================
 * Timestamp: 315000 ms
 * Node ID: 5
 * Reason: Heartbeat timeout
 * Uptime at failure: 315000 ms
 * Active nodes remaining: 3
 * Tasks to reassign: 2
 * ========================================
 * ↻ Reassigning task 7 from node 5 to node 3
 * ↻ Reassigning task 9 from node 5 to node 2
 * ✓ Reassigned 2 tasks from failed node 5
 */
