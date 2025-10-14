/**
 * @file lora_mesh_demo.ino
 * @brief LoRa Mesh Networking Demo for Wildlife Monitoring
 * 
 * Demonstrates:
 * - LoRa mesh initialization
 * - Device discovery and routing
 * - Signal strength monitoring
 * - Coordinator election
 * - Wildlife alert transmission
 * 
 * Compatible with: ESP32-S3-CAM, ESP-EYE, ESP32-XIAO
 * Note: ESP32-CAM has GPIO conflicts - see LORA_MESH_COMPATIBILITY.md
 */

#include <Arduino.h>
#include "../../firmware/src/lora_mesh.h"
#include <ArduinoJson.h>

// Configuration
#define NODE_ID 1  // Change for each node: 1, 2, 3, etc.
#define UPDATE_INTERVAL 5000  // Status update interval (5 seconds)

unsigned long lastUpdate = 0;
int messageCount = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=================================");
    Serial.println("LoRa Mesh Networking Demo");
    Serial.println("Wildlife Monitoring System");
    Serial.println("=================================\n");
    
    // Set unique node ID
    LoraMesh::setNodeId(NODE_ID);
    Serial.printf("Node ID: %d\n", NODE_ID);
    
    // Initialize LoRa mesh
    Serial.println("Initializing LoRa mesh network...");
    if (LoraMesh::init()) {
        Serial.println("✓ LoRa mesh initialized successfully\n");
    } else {
        Serial.println("✗ LoRa mesh initialization failed");
        Serial.println("Check hardware connections and GPIO configuration");
        Serial.println("See LORA_MESH_COMPATIBILITY.md for details\n");
        return;
    }
    
    // For Node 1, become coordinator
    if (NODE_ID == 1) {
        Serial.println("Attempting to become network coordinator...");
        if (LoraMesh::becomeCoordinator()) {
            Serial.println("✓ This node is now the coordinator\n");
        }
    }
    
    Serial.println("Starting mesh operations...\n");
}

void loop() {
    // Process mesh messages (heartbeat, discovery, routing)
    LoraMesh::processMessages();
    
    // Periodic status update
    unsigned long now = millis();
    if (now - lastUpdate > UPDATE_INTERVAL) {
        lastUpdate = now;
        
        printNetworkStatus();
        printSignalQuality();
        
        // Simulate wildlife detection (every 3rd update)
        if (messageCount % 3 == 0) {
            sendWildlifeAlert();
        }
        
        messageCount++;
        Serial.println("-----------------------------------\n");
    }
    
    delay(100);
}

/**
 * Print network status information
 */
void printNetworkStatus() {
    MeshNetworkStatus status = LoraMesh::getNetworkStatus();
    
    Serial.println("Network Status:");
    Serial.printf("  Node ID: %d\n", status.nodeId);
    Serial.printf("  Connected Nodes: %d\n", status.connectedNodes);
    Serial.printf("  Packets RX/TX: %lu / %lu\n", 
                 status.packetsReceived, status.packetsSent);
    
    // Coordinator status
    if (status.isCoordinator) {
        Serial.println("  Role: COORDINATOR");
    } else {
        Serial.println("  Role: NODE");
        if (status.coordinatorNodeId != -1) {
            Serial.printf("  Coordinator: Node %d\n", status.coordinatorNodeId);
            unsigned long timeSinceCoord = millis() - status.lastCoordinatorSeen;
            Serial.printf("  Last seen: %lu ms ago\n", timeSinceCoord);
        } else {
            Serial.println("  Coordinator: Not detected");
        }
    }
    
    // Network health indicator
    if (status.connectedNodes == 0) {
        Serial.println("  Health: ⚠️  ISOLATED (no neighbors)");
    } else if (status.connectedNodes < 3) {
        Serial.println("  Health: ⚠️  LIMITED (few neighbors)");
    } else {
        Serial.println("  Health: ✓ GOOD (mesh formed)");
    }
    
    Serial.println();
}

/**
 * Print signal quality information
 */
void printSignalQuality() {
    SignalQuality quality = LoraMesh::getSignalQuality();
    
    Serial.println("Signal Quality:");
    Serial.printf("  RSSI: %d dBm\n", quality.rssi);
    Serial.printf("  SNR: %.1f dB\n", quality.snr);
    
    Serial.print("  Strength: ");
    switch (quality.strength) {
        case SIGNAL_EXCELLENT:
            Serial.println("✓ EXCELLENT (> -70 dBm)");
            break;
        case SIGNAL_GOOD:
            Serial.println("✓ GOOD (-70 to -85 dBm)");
            break;
        case SIGNAL_FAIR:
            Serial.println("⚠️  FAIR (-85 to -100 dBm)");
            break;
        case SIGNAL_POOR:
            Serial.println("✗ POOR (< -100 dBm)");
            break;
    }
    
    // Provide range estimate based on RSSI
    if (quality.rssi > -70) {
        Serial.println("  Estimated Range: < 1 km");
    } else if (quality.rssi > -85) {
        Serial.println("  Estimated Range: 1-3 km");
    } else if (quality.rssi > -100) {
        Serial.println("  Estimated Range: 3-5 km");
    } else {
        Serial.println("  Estimated Range: > 5 km (weak)");
    }
    
    Serial.println();
}

/**
 * Send simulated wildlife detection alert
 */
void sendWildlifeAlert() {
    // Create wildlife detection message
    DynamicJsonDocument doc(256);
    doc["event"] = "wildlife_detected";
    doc["node_id"] = NODE_ID;
    doc["species"] = "deer";
    doc["confidence"] = 0.87;
    doc["timestamp"] = millis();
    doc["location"]["lat"] = 37.7749 + (random(0, 100) / 10000.0);
    doc["location"]["lon"] = -122.4194 + (random(0, 100) / 10000.0);
    
    Serial.println("Wildlife Detection Event:");
    Serial.println("  Species: Deer (87% confidence)");
    Serial.println("  Sending alert to mesh network...");
    
    // Send status update
    if (LoraMesh::sendStatus(doc.as<JsonObject>())) {
        Serial.println("  ✓ Alert queued for transmission");
    } else {
        Serial.println("  ✗ Failed to queue alert");
    }
    
    Serial.println();
}

/**
 * Additional helper: Send custom message
 */
void sendCustomMessage(const char* message) {
    String msg = String(message);
    if (LoraMesh::queueMessage(msg)) {
        Serial.printf("Custom message queued: %s\n", message);
    }
}

/**
 * Additional helper: Check if coordinator
 */
void checkCoordinatorStatus() {
    if (LoraMesh::isCoordinator()) {
        Serial.println("This node is the active coordinator");
        
        // Coordinators can perform special tasks:
        // - Aggregate data from all nodes
        // - Forward to gateway/cloud
        // - Manage network topology
    } else {
        int coordId = LoraMesh::getCoordinatorId();
        if (coordId != -1) {
            Serial.printf("Network coordinator is Node %d\n", coordId);
        } else {
            Serial.println("No coordinator elected yet - network forming...");
        }
    }
}
