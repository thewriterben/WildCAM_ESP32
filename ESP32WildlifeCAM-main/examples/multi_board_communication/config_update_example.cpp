/**
 * @file config_update_example.cpp
 * @brief Example demonstrating configuration update handling in multi-board system
 * 
 * This example shows how a coordinator can send configuration updates to board nodes,
 * and how nodes validate and apply those updates safely.
 */

#include <Arduino.h>
#include "../firmware/src/multi_board/board_coordinator.h"
#include "../firmware/src/multi_board/board_node.h"
#include "../firmware/src/multi_board/message_protocol.h"

// Global instances
BoardCoordinator coordinator;
BoardNode node1;
BoardNode node2;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== Configuration Update Example ===\n");
    
    // Initialize coordinator
    Serial.println("Initializing coordinator (Node 0)...");
    if (!coordinator.init(0)) {
        Serial.println("Failed to initialize coordinator");
        return;
    }
    coordinator.startCoordinator();
    
    // Initialize nodes
    Serial.println("Initializing node 1...");
    if (!node1.init(1, ROLE_NODE)) {
        Serial.println("Failed to initialize node 1");
        return;
    }
    node1.startNode();
    
    Serial.println("Initializing node 2...");
    if (!node2.init(2, ROLE_NODE)) {
        Serial.println("Failed to initialize node 2");
        return;
    }
    node2.startNode();
    
    delay(2000);
    
    // Example 1: Valid configuration update
    Serial.println("\n--- Example 1: Valid Configuration Update ---");
    demonstrateValidConfigUpdate();
    
    delay(3000);
    
    // Example 2: Invalid configuration update (out of range)
    Serial.println("\n--- Example 2: Invalid Configuration Update ---");
    demonstrateInvalidConfigUpdate();
    
    delay(3000);
    
    // Example 3: Partial configuration update
    Serial.println("\n--- Example 3: Partial Configuration Update ---");
    demonstratePartialConfigUpdate();
    
    delay(3000);
    
    // Example 4: Show current node configurations
    Serial.println("\n--- Example 4: Current Node Configurations ---");
    showNodeConfigurations();
}

void loop() {
    // Process all components
    coordinator.process();
    node1.process();
    node2.process();
    
    delay(100);
}

void demonstrateValidConfigUpdate() {
    Serial.println("Coordinator sending valid configuration update to all nodes...");
    
    // Create configuration update
    DynamicJsonDocument doc(512);
    JsonObject config = doc.to<JsonObject>();
    
    config["heartbeatInterval"] = 45000UL;     // 45 seconds (valid)
    config["coordinatorTimeout"] = 300000UL;   // 5 minutes (valid)
    config["taskTimeout"] = 180000UL;          // 3 minutes (valid)
    config["maxRetries"] = 5;                  // (valid)
    config["enableAutonomousMode"] = true;
    config["enableTaskExecution"] = true;
    
    // Broadcast configuration update
    bool sent = coordinator.broadcastConfigUpdate(config);
    
    if (sent) {
        Serial.println("✓ Configuration update broadcast successfully");
        Serial.println("Expected result: Nodes should accept and apply configuration");
    } else {
        Serial.println("✗ Failed to broadcast configuration update");
    }
}

void demonstrateInvalidConfigUpdate() {
    Serial.println("Coordinator sending invalid configuration update...");
    
    // Create configuration update with invalid values
    DynamicJsonDocument doc(512);
    JsonObject config = doc.to<JsonObject>();
    
    config["heartbeatInterval"] = 5000UL;      // TOO LOW - below 10 seconds
    config["maxRetries"] = 15;                 // TOO HIGH - above 10
    
    // Broadcast configuration update
    bool sent = coordinator.broadcastConfigUpdate(config);
    
    if (sent) {
        Serial.println("✓ Configuration update broadcast (but should be rejected by nodes)");
        Serial.println("Expected result: Nodes should reject and send error acknowledgment");
    } else {
        Serial.println("✗ Failed to broadcast configuration update");
    }
}

void demonstratePartialConfigUpdate() {
    Serial.println("Coordinator sending partial configuration update...");
    
    // Create configuration update with only some parameters
    DynamicJsonDocument doc(512);
    JsonObject config = doc.to<JsonObject>();
    
    config["heartbeatInterval"] = 30000UL;     // 30 seconds
    config["enableAutonomousMode"] = false;    // Disable autonomous mode
    
    // Broadcast configuration update
    bool sent = coordinator.broadcastConfigUpdate(config);
    
    if (sent) {
        Serial.println("✓ Partial configuration update broadcast successfully");
        Serial.println("Expected result: Only specified parameters should be updated");
    } else {
        Serial.println("✗ Failed to broadcast configuration update");
    }
}

void showNodeConfigurations() {
    Serial.println("Current configurations:");
    
    // Show Node 1 configuration
    const NodeConfig& config1 = node1.getNodeConfig();
    Serial.println("\nNode 1:");
    Serial.printf("  - heartbeatInterval: %lu ms\n", config1.heartbeatInterval);
    Serial.printf("  - coordinatorTimeout: %lu ms\n", config1.coordinatorTimeout);
    Serial.printf("  - taskTimeout: %lu ms\n", config1.taskTimeout);
    Serial.printf("  - maxRetries: %d\n", config1.maxRetries);
    Serial.printf("  - enableAutonomousMode: %s\n", config1.enableAutonomousMode ? "true" : "false");
    Serial.printf("  - enableTaskExecution: %s\n", config1.enableTaskExecution ? "true" : "false");
    
    // Show Node 2 configuration
    const NodeConfig& config2 = node2.getNodeConfig();
    Serial.println("\nNode 2:");
    Serial.printf("  - heartbeatInterval: %lu ms\n", config2.heartbeatInterval);
    Serial.printf("  - coordinatorTimeout: %lu ms\n", config2.coordinatorTimeout);
    Serial.printf("  - taskTimeout: %lu ms\n", config2.taskTimeout);
    Serial.printf("  - maxRetries: %d\n", config2.maxRetries);
    Serial.printf("  - enableAutonomousMode: %s\n", config2.enableAutonomousMode ? "true" : "false");
    Serial.printf("  - enableTaskExecution: %s\n", config2.enableTaskExecution ? "true" : "false");
}

/**
 * EXPECTED OUTPUT:
 * 
 * === Configuration Update Example ===
 * 
 * Initializing coordinator (Node 0)...
 * Board coordinator initialized: Node 0
 * Starting coordinator role...
 * 
 * Initializing node 1...
 * Board node initialized: Node 1, Preferred role: Node
 * Starting node operation...
 * 
 * Initializing node 2...
 * Board node initialized: Node 2, Preferred role: Node
 * Starting node operation...
 * 
 * --- Example 1: Valid Configuration Update ---
 * Coordinator sending valid configuration update to all nodes...
 * ✓ Configuration update broadcast successfully
 * Expected result: Nodes should accept and apply configuration
 * 
 * [Node 1] Received configuration update from coordinator
 * [Node 1]   ✓ heartbeatInterval: 45000 ms
 * [Node 1]   ✓ coordinatorTimeout: 300000 ms
 * [Node 1]   ✓ taskTimeout: 180000 ms
 * [Node 1]   ✓ maxRetries: 5
 * [Node 1]   ✓ enableAutonomousMode: true
 * [Node 1]   ✓ enableTaskExecution: true
 * [Node 1] ✓ Configuration update applied successfully
 * [Node 1] ✓ Configuration acknowledgment sent to coordinator
 * 
 * --- Example 2: Invalid Configuration Update ---
 * Coordinator sending invalid configuration update...
 * ✓ Configuration update broadcast (but should be rejected by nodes)
 * Expected result: Nodes should reject and send error acknowledgment
 * 
 * [Node 1] Received configuration update from coordinator
 * [Node 1]   ✗ Invalid heartbeatInterval: 5000 ms
 * [Node 1]   ✗ Invalid maxRetries: 15
 * [Node 1] ✗ Configuration update rejected due to validation errors:
 * [Node 1]   heartbeatInterval out of range (10s-10min); maxRetries out of range (0-10);
 * [Node 1] ✓ Configuration acknowledgment sent to coordinator
 * 
 * --- Example 3: Partial Configuration Update ---
 * Coordinator sending partial configuration update...
 * ✓ Partial configuration update broadcast successfully
 * Expected result: Only specified parameters should be updated
 * 
 * [Node 1] Received configuration update from coordinator
 * [Node 1]   ✓ heartbeatInterval: 30000 ms
 * [Node 1]   ✓ enableAutonomousMode: false
 * [Node 1] ✓ Configuration update applied successfully
 * [Node 1] ✓ Configuration acknowledgment sent to coordinator
 */
