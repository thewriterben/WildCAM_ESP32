/**
 * @file meshtastic_basic.ino
 * @brief Basic Meshtastic Example for ESP32WildlifeCAM
 * 
 * This example demonstrates basic Meshtastic mesh networking functionality:
 * - Node initialization and configuration
 * - Text message transmission and reception
 * - Network discovery and routing
 * - Basic telemetry sharing
 * 
 * Hardware Requirements:
 * - ESP32-CAM or ESP32-S3-CAM
 * - LoRa module (SX1276/SX1262/SX1280)
 * - Antenna suitable for your frequency band
 * 
 * Wiring (for AI-Thinker ESP32-CAM):
 * - LoRa SCLK -> GPIO 12
 * - LoRa MISO -> GPIO 13
 * - LoRa MOSI -> GPIO 15
 * - LoRa CS   -> GPIO 14
 * - LoRa RST  -> GPIO 2
 * - LoRa DIO0 -> GPIO 4
 * - LoRa VCC  -> 3.3V
 * - LoRa GND  -> GND
 */

#include <Arduino.h>
#include <WiFi.h>

// Include Meshtastic headers
#include "../../firmware/src/meshtastic/mesh_config.h"
#include "../../firmware/src/meshtastic/lora_driver.h"
#include "../../firmware/src/meshtastic/mesh_interface.h"

// Global objects
LoRaDriver* radioDriver = nullptr;
MeshInterface* meshInterface = nullptr;

// Node configuration
String nodeName = "BasicNode";
uint32_t nodeId = 0;

// Timing variables
unsigned long lastHeartbeat = 0;
unsigned long lastStatusPrint = 0;
const unsigned long HEARTBEAT_INTERVAL = 60000;  // 1 minute
const unsigned long STATUS_INTERVAL = 30000;     // 30 seconds

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ESP32WildlifeCAM Meshtastic Basic Example ===");
    Serial.println("Initializing mesh network...");
    
    // Initialize WiFi for MAC address generation (will be disabled after)
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    delay(1000);
    
    // Generate node ID from MAC address
    nodeId = generateNodeIdFromMac();
    Serial.printf("Generated Node ID: %08X\n", nodeId);
    
    // Disable WiFi to save power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    // Create LoRa configuration
    LoRaConfig radioConfig = createDefaultConfig(LORA_REGION);
    
    // Override GPIO pins for AI-Thinker ESP32-CAM
    radioConfig.csPin = 14;
    radioConfig.resetPin = 2;
    radioConfig.dio0Pin = 4;
    radioConfig.dio1Pin = 16;
    radioConfig.dio2Pin = -1;  // Not used
    
    // Create and initialize radio driver
    radioDriver = createLoRaDriver(radioConfig);
    if (!radioDriver) {
        Serial.println("ERROR: Failed to create LoRa driver!");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("LoRa driver initialized successfully");
    radioDriver->printConfiguration();
    
    // Create and initialize mesh interface
    meshInterface = createMeshInterface(radioConfig);
    if (!meshInterface) {
        Serial.println("ERROR: Failed to create mesh interface!");
        while (1) {
            delay(1000);
        }
    }
    
    // Configure node information
    NodeInfo nodeInfo;
    nodeInfo.nodeId = nodeId;
    nodeInfo.shortName = nodeName;
    nodeInfo.longName = "ESP32WildlifeCAM Basic Mesh Node";
    nodeInfo.macAddress = WiFi.macAddress();
    nodeInfo.status = NODE_STATUS_ONLINE;
    nodeInfo.batteryLevel = 100;
    nodeInfo.voltage = 3.8;
    nodeInfo.hasPosition = false;
    
    meshInterface->setNodeInfo(nodeInfo);
    
    // Set up callbacks
    meshInterface->setMessageCallback(onMessageReceived);
    meshInterface->setNodeDiscoveredCallback(onNodeDiscovered);
    meshInterface->setNetworkStatusCallback(onNetworkStatusChanged);
    
    Serial.println("Mesh interface initialized successfully");
    
    // Start node discovery
    meshInterface->startNodeDiscovery();
    
    Serial.println("=== Mesh Network Ready ===");
    Serial.println("Commands:");
    Serial.println("  help - Show this help");
    Serial.println("  status - Show network status");
    Serial.println("  nodes - List discovered nodes");
    Serial.println("  send <nodeId> <message> - Send text message");
    Serial.println("  broadcast <message> - Broadcast message");
    Serial.println("  discovery - Start node discovery");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Process mesh networking
    meshInterface->process();
    
    // Send periodic heartbeat
    if (currentTime - lastHeartbeat > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // Print status periodically
    if (currentTime - lastStatusPrint > STATUS_INTERVAL) {
        printNetworkStatus();
        lastStatusPrint = currentTime;
    }
    
    // Handle serial commands
    handleSerialCommands();
    
    // Small delay to prevent watchdog issues
    delay(10);
}

void sendHeartbeat() {
    // Create telemetry data
    DynamicJsonDocument doc(512);
    doc["type"] = "heartbeat";
    doc["nodeId"] = nodeId;
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["rssi"] = radioDriver->getRssi();
    doc["snr"] = radioDriver->getSnr();
    doc["timestamp"] = millis();
    
    // Serialize and send
    String jsonString;
    serializeJson(doc, jsonString);
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    
    meshInterface->broadcastMessage(PACKET_TYPE_TELEMETRY, payload, PRIORITY_BACKGROUND);
    
    Serial.printf("Heartbeat sent (uptime: %lu s, heap: %u bytes)\n", 
                  millis() / 1000, ESP.getFreeHeap());
}

void printNetworkStatus() {
    MeshStatistics stats = meshInterface->getStatistics();
    
    Serial.println("\n=== Network Status ===");
    Serial.printf("Node ID: %08X\n", nodeId);
    Serial.printf("Packets RX: %u, TX: %u\n", stats.packetsReceived, stats.packetsSent);
    Serial.printf("Network Nodes: %u\n", stats.networkNodes);
    Serial.printf("Routing Table: %u entries\n", stats.routingTableSize);
    Serial.printf("Signal: RSSI=%d dBm, SNR=%.1f dB\n", 
                  radioDriver->getRssi(), radioDriver->getSnr());
    Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
    Serial.println("========================\n");
}

void handleSerialCommands() {
    if (!Serial.available()) {
        return;
    }
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.length() == 0) {
        return;
    }
    
    Serial.printf("> %s\n", command.c_str());
    
    if (command == "help") {
        printHelp();
    } else if (command == "status") {
        printDetailedStatus();
    } else if (command == "nodes") {
        printDiscoveredNodes();
    } else if (command == "discovery") {
        meshInterface->startNodeDiscovery();
        Serial.println("Node discovery started");
    } else if (command.startsWith("send ")) {
        handleSendCommand(command);
    } else if (command.startsWith("broadcast ")) {
        handleBroadcastCommand(command);
    } else {
        Serial.println("Unknown command. Type 'help' for available commands.");
    }
}

void printHelp() {
    Serial.println("\n=== Available Commands ===");
    Serial.println("help                     - Show this help");
    Serial.println("status                   - Show detailed network status");
    Serial.println("nodes                    - List all discovered nodes");
    Serial.println("send <nodeId> <message>  - Send text message to specific node");
    Serial.println("broadcast <message>      - Broadcast message to all nodes");
    Serial.println("discovery                - Start node discovery process");
    Serial.println("===========================\n");
}

void printDetailedStatus() {
    Serial.println("\n=== Detailed Status ===");
    
    // Node information
    NodeInfo nodeInfo = meshInterface->getNodeInfo();
    Serial.printf("Node Name: %s\n", nodeInfo.shortName.c_str());
    Serial.printf("Node ID: %08X\n", nodeInfo.nodeId);
    Serial.printf("MAC Address: %s\n", nodeInfo.macAddress.c_str());
    Serial.printf("Status: %s\n", nodeStatusToString(nodeInfo.status).c_str());
    Serial.printf("Battery: %u%%, Voltage: %.2fV\n", nodeInfo.batteryLevel, nodeInfo.voltage);
    
    // Radio status
    LoRaStatus radioStatus = radioDriver->getStatus();
    Serial.printf("Radio Initialized: %s\n", radioStatus.initialized ? "Yes" : "No");
    Serial.printf("Radio TX: %u, RX: %u, Errors: %u\n", 
                  radioStatus.packetsTransmitted, radioStatus.packetsReceived, 
                  radioStatus.transmissionErrors);
    
    // Mesh statistics
    MeshStatistics meshStats = meshInterface->getStatistics();
    Serial.printf("Mesh TX: %u, RX: %u, Forwarded: %u\n", 
                  meshStats.packetsSent, meshStats.packetsReceived, meshStats.packetsForwarded);
    Serial.printf("Duplicates Dropped: %u, Encryption Errors: %u\n", 
                  meshStats.duplicatesDropped, meshStats.encryptionErrors);
    Serial.printf("Average RSSI: %.1f dBm, SNR: %.1f dB\n", 
                  meshStats.averageRssi, meshStats.averageSnr);
    
    // System information
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap: %u bytes\n", ESP.getMinFreeHeap());
    
    Serial.println("========================\n");
}

void printDiscoveredNodes() {
    std::vector<NodeInfo> nodes = meshInterface->getAllNodes();
    
    Serial.printf("\n=== Discovered Nodes (%d) ===\n", nodes.size());
    
    if (nodes.empty()) {
        Serial.println("No nodes discovered yet. Try 'discovery' command.");
    } else {
        for (const auto& node : nodes) {
            Serial.printf("Node %08X: %s (%s)\n", 
                          node.nodeId, node.shortName.c_str(), node.longName.c_str());
            Serial.printf("  Last seen: %lu ms ago\n", millis() - node.lastSeen);
            Serial.printf("  Signal: RSSI=%d dBm, SNR=%.1f dB\n", node.lastRssi, node.lastSnr);
            Serial.printf("  Battery: %u%%, Status: %s\n", 
                          node.batteryLevel, nodeStatusToString(node.status).c_str());
            if (node.hasPosition) {
                Serial.printf("  Position: %.6f, %.6f (alt: %um)\n", 
                              node.latitude, node.longitude, node.altitude);
            }
            Serial.println();
        }
    }
    
    Serial.println("================================\n");
}

void handleSendCommand(const String& command) {
    // Parse: send <nodeId> <message>
    int firstSpace = command.indexOf(' ', 5);  // Skip "send "
    if (firstSpace == -1) {
        Serial.println("Usage: send <nodeId> <message>");
        return;
    }
    
    String nodeIdStr = command.substring(5, firstSpace);
    String message = command.substring(firstSpace + 1);
    
    // Convert nodeId from hex string
    uint32_t targetNodeId = strtoul(nodeIdStr.c_str(), nullptr, 16);
    
    if (targetNodeId == 0) {
        Serial.println("Invalid node ID. Use hex format (e.g., A1B2C3D4)");
        return;
    }
    
    // Send message
    bool success = meshInterface->sendText(targetNodeId, message);
    
    if (success) {
        Serial.printf("Message sent to %08X: %s\n", targetNodeId, message.c_str());
    } else {
        Serial.printf("Failed to send message to %08X\n", targetNodeId);
    }
}

void handleBroadcastCommand(const String& command) {
    // Parse: broadcast <message>
    String message = command.substring(10);  // Skip "broadcast "
    
    if (message.length() == 0) {
        Serial.println("Usage: broadcast <message>");
        return;
    }
    
    // Broadcast message
    bool success = meshInterface->sendText(0, message);  // 0 = broadcast
    
    if (success) {
        Serial.printf("Broadcast sent: %s\n", message.c_str());
    } else {
        Serial.println("Failed to send broadcast");
    }
}

// Callback functions
void onMessageReceived(const MeshPacket& packet) {
    Serial.printf("\n>>> Message from %08X (RSSI: %d dBm, SNR: %.1f dB)\n", 
                  packet.header.from, packet.rxRssi, packet.rxSnr);
    
    switch (packet.header.portNum) {
        case PACKET_TYPE_TEXT: {
            String text((char*)packet.payload.data(), packet.payload.size());
            Serial.printf("Text: %s\n", text.c_str());
            break;
        }
        
        case PACKET_TYPE_TELEMETRY: {
            String jsonStr((char*)packet.payload.data(), packet.payload.size());
            DynamicJsonDocument doc(512);
            deserializeJson(doc, jsonStr);
            
            String type = doc["type"];
            Serial.printf("Telemetry (%s): ", type.c_str());
            
            if (type == "heartbeat") {
                Serial.printf("uptime=%lu, heap=%u, rssi=%d, snr=%.1f\n", 
                              doc["uptime"].as<unsigned long>(), 
                              doc["freeHeap"].as<uint32_t>(),
                              doc["rssi"].as<int>(), 
                              doc["snr"].as<float>());
            } else {
                serializeJson(doc, Serial);
                Serial.println();
            }
            break;
        }
        
        case PACKET_TYPE_POSITION: {
            String jsonStr((char*)packet.payload.data(), packet.payload.size());
            DynamicJsonDocument doc(256);
            deserializeJson(doc, jsonStr);
            
            Serial.printf("Position: lat=%.6f, lon=%.6f, alt=%um\n", 
                          doc["lat"].as<float>(), doc["lon"].as<float>(), 
                          doc["alt"].as<uint32_t>());
            break;
        }
        
        default: {
            Serial.printf("Data (%d bytes, type %d)\n", 
                          packet.payload.size(), packet.header.portNum);
            break;
        }
    }
    
    Serial.println();
}

void onNodeDiscovered(const NodeInfo& node) {
    Serial.printf("\n+++ Node discovered: %08X (%s)\n", 
                  node.nodeId, node.shortName.c_str());
    Serial.printf("    Last seen: %lu ms ago, RSSI: %d dBm\n", 
                  millis() - node.lastSeen, node.lastRssi);
    Serial.println();
}

void onNetworkStatusChanged(bool connected) {
    Serial.printf("\n*** Network status changed: %s\n", 
                  connected ? "Connected" : "Disconnected");
    Serial.println();
}

// Utility function implementations
uint32_t generateNodeIdFromMac() {
    String macStr = WiFi.macAddress();
    macStr.replace(":", "");
    
    // Use last 4 bytes of MAC address as node ID
    uint32_t nodeId = 0;
    for (int i = 8; i < 16; i += 2) {
        nodeId = (nodeId << 8) | strtol(macStr.substring(i, i + 2).c_str(), nullptr, 16);
    }
    
    return nodeId;
}

String nodeStatusToString(NodeStatus status) {
    switch (status) {
        case NODE_STATUS_UNKNOWN: return "Unknown";
        case NODE_STATUS_ONLINE: return "Online";
        case NODE_STATUS_OFFLINE: return "Offline";
        case NODE_STATUS_LOW_BATTERY: return "Low Battery";
        case NODE_STATUS_MAINTENANCE: return "Maintenance";
        default: return "Unknown";
    }
}