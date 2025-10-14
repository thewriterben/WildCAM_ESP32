/**
 * @file automatic_network_discovery.ino
 * @brief Example demonstrating automatic network discovery and device coordination
 * 
 * This example shows how ESP32 wildlife cameras automatically:
 * - Discover and join mesh networks
 * - Coordinate with other devices in real-time
 * - Assign roles dynamically based on capabilities
 * - Monitor and maintain network health
 * 
 * Hardware Requirements:
 * - ESP32-CAM or compatible board
 * - LoRa module (optional but recommended)
 * - SD card (optional)
 * 
 * Usage:
 * 1. Flash this sketch to multiple ESP32 boards
 * 2. Configure unique NODE_ID for each board
 * 3. Power on boards - they will automatically discover each other
 * 4. Monitor serial output to see network formation
 */

#include <Arduino.h>
#include "../../firmware/src/multi_board/multi_board_system.h"
#include "../../firmware/src/multi_board/discovery_protocol.h"
#include "../../firmware/src/multi_board/message_protocol.h"

// ===========================
// CONFIGURATION
// ===========================

// Unique ID for this node (change for each device!)
#define NODE_ID 1

// Preferred role for this device
#define PREFERRED_ROLE ROLE_NODE  // Options: ROLE_COORDINATOR, ROLE_NODE, ROLE_RELAY, etc.

// Discovery timing
#define DISCOVERY_INTERVAL 30000      // 30 seconds
#define ADVERTISEMENT_INTERVAL 60000  // 1 minute
#define STATUS_REPORT_INTERVAL 120000 // 2 minutes

// ===========================
// GLOBAL VARIABLES
// ===========================

MultiboardSystem* multiboardSystem = nullptr;
unsigned long lastStatusReport = 0;
unsigned long lastNetworkCheck = 10000;
unsigned long systemStartTime = 0;

// ===========================
// SETUP
// ===========================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=================================");
    Serial.println("ESP32 Wildlife Camera");
    Serial.println("Automatic Network Discovery Demo");
    Serial.println("=================================\n");
    
    systemStartTime = millis();
    
    // Initialize multi-board system
    Serial.printf("Initializing Node %d with preferred role: %s\n", 
                  NODE_ID, MessageProtocol::roleToString(PREFERRED_ROLE));
    
    SystemConfig config;
    config.nodeId = NODE_ID;
    config.preferredRole = PREFERRED_ROLE;
    config.enableAutomaticRoleSelection = true;
    config.enableStandaloneFallback = true;
    config.discoveryTimeout = 120000;  // 2 minutes
    config.roleChangeTimeout = 60000;  // 1 minute
    
    multiboardSystem = new MultiboardSystem();
    if (!multiboardSystem->init(config)) {
        Serial.println("❌ Failed to initialize multi-board system!");
        Serial.println(multiboardSystem->getLastError());
        return;
    }
    
    Serial.println("✓ Multi-board system initialized");
    
    // Enable LoRa mesh integration
    multiboardSystem->integrateWithLoraMesh(true);
    Serial.println("✓ LoRa mesh integration enabled");
    
    // Start the system
    if (!multiboardSystem->start()) {
        Serial.println("❌ Failed to start multi-board system!");
        Serial.println(multiboardSystem->getLastError());
        return;
    }
    
    Serial.println("✓ Multi-board system started");
    Serial.println("\n🔍 Starting automatic network discovery...\n");
}

// ===========================
// MAIN LOOP
// ===========================

void loop() {
    // Process multi-board system
    if (multiboardSystem) {
        multiboardSystem->process();
    }
    
    unsigned long now = millis();
    
    // Periodic status reporting
    if (now - lastStatusReport >= STATUS_REPORT_INTERVAL) {
        printSystemStatus();
        lastStatusReport = now;
    }
    
    // Periodic network check
    if (now - lastNetworkCheck >= 10000) {  // Every 10 seconds
        checkNetworkStatus();
        lastNetworkCheck = now;
    }
    
    delay(100);  // Small delay to prevent watchdog issues
}

// ===========================
// STATUS REPORTING
// ===========================

void printSystemStatus() {
    if (!multiboardSystem) return;
    
    Serial.println("\n=================================");
    Serial.println("SYSTEM STATUS REPORT");
    Serial.println("=================================");
    
    SystemStats stats = multiboardSystem->getSystemStats();
    
    // System information
    Serial.printf("Node ID: %d\n", NODE_ID);
    Serial.printf("System State: %d\n", stats.currentState);
    Serial.printf("Current Role: %s\n", MessageProtocol::roleToString(stats.currentRole));
    Serial.printf("Uptime: %lu seconds\n", (millis() - systemStartTime) / 1000);
    
    // Network information
    Serial.printf("\nNetwork Status:\n");
    Serial.printf("  Networked Nodes: %d\n", stats.networkedNodes);
    Serial.printf("  Active Tasks: %d\n", stats.activeTasks);
    Serial.printf("  Messages Processed: %d\n", stats.messagesProcessed);
    Serial.printf("  Network Efficiency: %.1f%%\n", stats.networkEfficiency * 100);
    
    // Topology information
    MultiboardSystem::TopologyInfo topology = multiboardSystem->getNetworkTopology();
    Serial.printf("\nTopology:\n");
    Serial.printf("  Total Nodes: %d\n", topology.totalNodes);
    Serial.printf("  Active Nodes: %d\n", topology.activeNodes);
    Serial.printf("  Coordinator: Node %d\n", topology.coordinatorNode);
    Serial.printf("  Network Stable: %s\n", topology.isStable ? "Yes" : "No");
    Serial.printf("  Last Update: %lu ms ago\n", millis() - topology.lastUpdate);
    
    // Node details
    if (topology.nodes.size() > 0) {
        Serial.printf("\nDiscovered Nodes:\n");
        for (const auto& node : topology.nodes) {
            Serial.printf("  Node %d: %s, RSSI=%d, Hops=%d, Battery=%d%%\n",
                         node.nodeId,
                         MessageProtocol::roleToString(node.role),
                         node.signalStrength,
                         node.hopCount,
                         node.capabilities.batteryLevel);
        }
    }
    
    Serial.println("=================================\n");
}

void checkNetworkStatus() {
    if (!multiboardSystem) return;
    
    SystemState state = multiboardSystem->getSystemState();
    
    switch (state) {
        case SYSTEM_INACTIVE:
            Serial.println("⚠ System inactive");
            break;
            
        case SYSTEM_INITIALIZING:
            Serial.println("⏳ System initializing...");
            break;
            
        case SYSTEM_DISCOVERING:
            Serial.println("🔍 Discovering network...");
            printDiscoveryProgress();
            break;
            
        case SYSTEM_COORDINATOR:
            Serial.println("👑 Operating as Coordinator");
            printCoordinatorInfo();
            break;
            
        case SYSTEM_NODE:
            Serial.println("📡 Operating as Node");
            printNodeInfo();
            break;
            
        case SYSTEM_STANDALONE:
            Serial.println("🏝 Operating in Standalone mode");
            break;
            
        case SYSTEM_ERROR:
            Serial.println("❌ System error");
            Serial.println(multiboardSystem->getLastError());
            break;
    }
}

void printDiscoveryProgress() {
    MultiboardSystem::TopologyInfo topology = multiboardSystem->getNetworkTopology();
    Serial.printf("  Nodes discovered: %d\n", topology.totalNodes);
    
    if (topology.coordinatorNode > 0) {
        Serial.printf("  ✓ Coordinator found: Node %d\n", topology.coordinatorNode);
    } else {
        Serial.println("  ⏳ Searching for coordinator...");
    }
}

void printCoordinatorInfo() {
    MultiboardSystem::TopologyInfo topology = multiboardSystem->getNetworkTopology();
    Serial.printf("  Managing %d nodes\n", topology.activeNodes);
    
    SystemStats stats = multiboardSystem->getSystemStats();
    Serial.printf("  Active tasks: %d\n", stats.activeTasks);
    Serial.printf("  Network efficiency: %.1f%%\n", stats.networkEfficiency * 100);
}

void printNodeInfo() {
    int coordinator = multiboardSystem->getCoordinatorNode();
    if (coordinator > 0) {
        Serial.printf("  Connected to Coordinator: Node %d\n", coordinator);
    } else {
        Serial.println("  ⚠ No coordinator connection");
    }
    
    SystemStats stats = multiboardSystem->getSystemStats();
    Serial.printf("  Active tasks: %d\n", stats.activeTasks);
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

void printWelcomeBanner() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32 Wildlife Camera Network       ║");
    Serial.println("║   Automatic Discovery & Coordination   ║");
    Serial.println("╚════════════════════════════════════════╝\n");
}

// ===========================
// COMMAND INTERFACE (Optional)
// ===========================

void processSerialCommand() {
    if (!Serial.available()) return;
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "status") {
        printSystemStatus();
    } else if (command == "discover") {
        Serial.println("Triggering network discovery...");
        multiboardSystem->triggerDiscovery();
    } else if (command == "help") {
        Serial.println("\nAvailable commands:");
        Serial.println("  status    - Print detailed system status");
        Serial.println("  discover  - Trigger network discovery");
        Serial.println("  help      - Show this help message");
    } else {
        Serial.printf("Unknown command: %s\n", command.c_str());
        Serial.println("Type 'help' for available commands");
    }
}
