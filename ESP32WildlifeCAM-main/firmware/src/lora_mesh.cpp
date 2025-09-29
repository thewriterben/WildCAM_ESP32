/**
 * LoRa Mesh Network Module
 * 
 * Implements LoRa mesh networking for wildlife camera communication
 * in remote areas without cellular or WiFi coverage.
 */

#include "lora_mesh.h"
#include "config.h"
#include <LoRa.h>
#include <ArduinoJson.h>

namespace LoraMesh {

// Static variables
static bool initialized = false;
static int nodeId = NODE_ID;
static unsigned long lastHeartbeat = 0;
static unsigned long packetCounter = 0;
static int rssiLast = 0;
static float snrLast = 0.0;
static MeshNetworkStatus networkStatus;

// Message queue for outgoing packets
static struct {
    String messages[LORA_MESSAGE_QUEUE_SIZE];
    int count = 0;
} messageQueue;

// Routing table for mesh network
static struct {
    int nodeIds[MAX_MESH_NODES];
    int nextHops[MAX_MESH_NODES];
    int hopCounts[MAX_MESH_NODES];
    unsigned long lastSeen[MAX_MESH_NODES];
    int count = 0;
} routingTable;

// Function prototypes
static void onReceive(int packetSize);
static bool sendPacket(const String& message, int targetNode = 0);
static void processReceivedMessage(const String& message);
static void updateRoutingTable(int sourceNode, int hopCount);
static int findRouteToNode(int targetNode);
static void sendHeartbeat();
static String createMessage(const String& type, const JsonObject& data);
static void handleDataMessage(const JsonObject& message);
static void handleHeartbeat(const JsonObject& message);
static void handleImageTransmission(const JsonObject& message);

/**
 * Initialize LoRa mesh networking
 */
bool init() {
    DEBUG_PRINTLN("Initializing LoRa mesh network...");
    
    // Set LoRa pins
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    
    // Initialize LoRa
    if (!LoRa.begin(LORA_FREQUENCY)) {
        DEBUG_PRINTLN("Error: Failed to initialize LoRa radio");
        return false;
    }
    
    // Configure LoRa parameters
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    
    // Set receive callback
    LoRa.onReceive(onReceive);
    LoRa.receive();  // Start listening
    
    // Initialize network status
    networkStatus.initialized = true;
    networkStatus.nodeId = nodeId;
    networkStatus.connectedNodes = 0;
    networkStatus.lastMessageTime = 0;
    networkStatus.rssi = 0;
    networkStatus.snr = 0.0;
    networkStatus.packetsReceived = 0;
    networkStatus.packetsSent = 0;
    
    initialized = true;
    DEBUG_PRINTF("LoRa mesh network initialized - Node ID: %d\n", nodeId);
    
    return true;
}

/**
 * Process incoming messages and handle mesh routing
 */
void processMessages() {
    if (!initialized) return;
    
    // Send heartbeat at configured interval
    unsigned long now = millis();
    if (now - lastHeartbeat > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = now;
    }
    
    // Process outgoing message queue
    if (messageQueue.count > 0) {
        if (sendPacket(messageQueue.messages[0])) {
            // Remove sent message from queue
            for (int i = 1; i < messageQueue.count; i++) {
                messageQueue.messages[i-1] = messageQueue.messages[i];
            }
            messageQueue.count--;
        }
    }
    
    // Clean up old routing table entries (remove nodes not seen for 5 minutes)
    for (int i = 0; i < routingTable.count; i++) {
        if (now - routingTable.lastSeen[i] > 300000) {  // 5 minutes
            DEBUG_PRINTF("Removing stale route to node %d\n", routingTable.nodeIds[i]);
            // Shift remaining entries
            for (int j = i; j < routingTable.count - 1; j++) {
                routingTable.nodeIds[j] = routingTable.nodeIds[j+1];
                routingTable.nextHops[j] = routingTable.nextHops[j+1];
                routingTable.hopCounts[j] = routingTable.hopCounts[j+1];
                routingTable.lastSeen[j] = routingTable.lastSeen[j+1];
            }
            routingTable.count--;
            networkStatus.connectedNodes = routingTable.count;
            i--; // Adjust index after removal
        }
    }
}

/**
 * Send image data over LoRa mesh (compressed)
 */
bool transmitImage(camera_fb_t* fb, const String& filename) {
    if (!initialized || !fb) return false;
    
    DEBUG_PRINTF("Transmitting image: %s (%d bytes)\n", filename.c_str(), fb->len);
    
    // Create image metadata message
    DynamicJsonDocument doc(JSON_BUFFER_MEDIUM);
    doc["type"] = "image_meta";
    doc["node_id"] = nodeId;
    doc["filename"] = filename;
    doc["size"] = fb->len;
    doc["width"] = fb->width;
    doc["height"] = fb->height;
    doc["timestamp"] = millis();
    
    String metaMessage;
    serializeJson(doc, metaMessage);
    
    // Queue metadata message
    queueMessage(metaMessage);
    
    // For large images, we would typically:
    // 1. Compress the image
    // 2. Split into chunks
    // 3. Send chunks with sequence numbers
    // 4. Handle acknowledgments and retransmissions
    
    // For now, just send a notification that an image was captured
    DEBUG_PRINTLN("Image metadata queued for transmission");
    
    return true;
}

/**
 * Send status update to mesh network
 */
bool sendStatus(const JsonObject& statusData) {
    if (!initialized) return false;
    
    String message = createMessage("status", statusData);
    return queueMessage(message);
}

/**
 * Queue message for transmission
 */
bool queueMessage(const String& message) {
    if (messageQueue.count >= LORA_MESSAGE_QUEUE_SIZE) {
        DEBUG_PRINTLN("Warning: Message queue full, dropping oldest message");
        // Remove oldest message
        for (int i = 1; i < messageQueue.count; i++) {
            messageQueue.messages[i-1] = messageQueue.messages[i];
        }
        messageQueue.count--;
    }
    
    messageQueue.messages[messageQueue.count] = message;
    messageQueue.count++;
    
    DEBUG_PRINTF("Message queued (%d/%d in queue)\n", messageQueue.count, LORA_MESSAGE_QUEUE_SIZE);
    return true;
}

/**
 * Get mesh network status
 */
MeshNetworkStatus getNetworkStatus() {
    networkStatus.connectedNodes = routingTable.count;
    return networkStatus;
}

/**
 * LoRa receive callback
 */
static void onReceive(int packetSize) {
    if (packetSize == 0) return;
    
    String message = "";
    while (LoRa.available()) {
        message += (char)LoRa.read();
    }
    
    // Get signal quality
    rssiLast = LoRa.packetRssi();
    snrLast = LoRa.packetSnr();
    
    networkStatus.rssi = rssiLast;
    networkStatus.snr = snrLast;
    networkStatus.packetsReceived++;
    networkStatus.lastMessageTime = millis();
    
    DEBUG_PRINTF("Received: %s (RSSI: %d, SNR: %.1f)\n", message.c_str(), rssiLast, snrLast);
    
    processReceivedMessage(message);
}

/**
 * Send packet over LoRa
 */
static bool sendPacket(const String& message, int targetNode) {
    if (!initialized) return false;
    
    // Start packet
    LoRa.beginPacket();
    LoRa.print(message);
    bool success = LoRa.endPacket();
    
    if (success) {
        networkStatus.packetsSent++;
        packetCounter++;
        DEBUG_PRINTF("Sent packet #%lu to node %d\n", packetCounter, targetNode);
    } else {
        DEBUG_PRINTLN("Error: Failed to send packet");
    }
    
    // Return to receive mode
    LoRa.receive();
    
    return success;
}

/**
 * Process received message
 */
static void processReceivedMessage(const String& message) {
    DynamicJsonDocument doc(JSON_BUFFER_LARGE);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        DEBUG_PRINTF("JSON parse error: %s\n", error.c_str());
        return;
    }
    
    String messageType = doc["type"];
    int sourceNode = doc["source_node"];
    int hopCount = doc["hop_count"] | 0;
    
    // Update routing table
    updateRoutingTable(sourceNode, hopCount);
    
    // Handle different message types
    if (messageType == "heartbeat") {
        handleHeartbeat(doc.as<JsonObject>());
    } else if (messageType == "data" || messageType == "status") {
        handleDataMessage(doc.as<JsonObject>());
    } else if (messageType == "image_meta") {
        handleImageTransmission(doc.as<JsonObject>());
    }
    
    // Check if message needs to be forwarded
    int targetNode = doc["target_node"] | 0;
    if (targetNode != 0 && targetNode != nodeId) {
        // This message is for another node, try to forward it
        int nextHop = findRouteToNode(targetNode);
        if (nextHop != -1) {
            DEBUG_PRINTF("Forwarding message to node %d via %d\n", targetNode, nextHop);
            // Increment hop count and forward
            doc["hop_count"] = hopCount + 1;
            String forwardMessage;
            serializeJson(doc, forwardMessage);
            queueMessage(forwardMessage);
        }
    }
}

/**
 * Update routing table with node information
 */
static void updateRoutingTable(int sourceNode, int hopCount) {
    if (sourceNode == nodeId) return;  // Don't add ourselves
    
    // Find existing entry or add new one
    int index = -1;
    for (int i = 0; i < routingTable.count; i++) {
        if (routingTable.nodeIds[i] == sourceNode) {
            index = i;
            break;
        }
    }
    
    if (index == -1 && routingTable.count < MAX_MESH_NODES) {
        // Add new entry
        index = routingTable.count;
        routingTable.count++;
        routingTable.nodeIds[index] = sourceNode;
        DEBUG_PRINTF("Added new route to node %d\n", sourceNode);
    }
    
    if (index != -1) {
        // Update route information
        routingTable.nextHops[index] = sourceNode;  // Direct connection for now
        routingTable.hopCounts[index] = hopCount + 1;
        routingTable.lastSeen[index] = millis();
    }
}

/**
 * Find route to target node
 */
static int findRouteToNode(int targetNode) {
    for (int i = 0; i < routingTable.count; i++) {
        if (routingTable.nodeIds[i] == targetNode) {
            return routingTable.nextHops[i];
        }
    }
    return -1;  // No route found
}

/**
 * Send heartbeat message
 */
static void sendHeartbeat() {
    DynamicJsonDocument doc(JSON_BUFFER_SMALL);
    doc["type"] = "heartbeat";
    doc["source_node"] = nodeId;
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    String message;
    serializeJson(doc, message);
    
    sendPacket(message);
}

/**
 * Create formatted message
 */
static String createMessage(const String& type, const JsonObject& data) {
    DynamicJsonDocument doc(JSON_BUFFER_LARGE);
    doc["type"] = type;
    doc["source_node"] = nodeId;
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    // Copy data fields
    for (JsonPair kv : data) {
        doc[kv.key()] = kv.value();
    }
    
    String message;
    serializeJson(doc, message);
    return message;
}

/**
 * Handle data/status messages
 */
static void handleDataMessage(const JsonObject& message) {
    int sourceNode = message["source_node"];
    String messageType = message["type"];
    
    DEBUG_PRINTF("Received %s from node %d\n", messageType.c_str(), sourceNode);
    
    // Process status messages
    if (messageType == "status") {
        // Could log status information, forward to base station, etc.
    }
}

/**
 * Handle heartbeat messages
 */
static void handleHeartbeat(const JsonObject& message) {
    int sourceNode = message["source_node"];
    DEBUG_PRINTF("Heartbeat from node %d\n", sourceNode);
}

/**
 * Handle image transmission messages
 */
static void handleImageTransmission(const JsonObject& message) {
    String filename = message["filename"];
    int imageSize = message["size"];
    int sourceNode = message["source_node"];
    
    DEBUG_PRINTF("Image notification from node %d: %s (%d bytes)\n", 
                sourceNode, filename.c_str(), imageSize);
}

/**
 * Set node ID
 */
void setNodeId(int newNodeId) {
    nodeId = newNodeId;
    networkStatus.nodeId = nodeId;
    DEBUG_PRINTF("Node ID set to %d\n", nodeId);
}

/**
 * Get signal quality information
 */
SignalQuality getSignalQuality() {
    SignalQuality quality;
    quality.rssi = rssiLast;
    quality.snr = snrLast;
    
    // Determine signal strength
    if (rssiLast > -70) quality.strength = SIGNAL_EXCELLENT;
    else if (rssiLast > -85) quality.strength = SIGNAL_GOOD;
    else if (rssiLast > -100) quality.strength = SIGNAL_FAIR;
    else quality.strength = SIGNAL_POOR;
    
    return quality;
}

/**
 * Cleanup LoRa mesh resources
 */
void cleanup() {
    if (initialized) {
        LoRa.end();
        initialized = false;
        DEBUG_PRINTLN("LoRa mesh network cleaned up");
    }
}

} // namespace LoraMesh