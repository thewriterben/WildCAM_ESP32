/**
 * @file MeshManager.cpp
 * @brief LoRa Mesh Network Manager Implementation for WildCAM ESP32
 * @version 1.0.0
 * 
 * Implements Meshtastic/MeshCore compatible mesh networking for long-range
 * wildlife camera communication.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include "MeshManager.h"
#include "config.h"
#include "Logger.h"
#include <WiFi.h>

#if LORA_ENABLED
#include <SPI.h>
#include <LoRa.h>
#endif

// Global instance
MeshManager meshManager;

//==============================================================================
// CONSTRUCTOR / DESTRUCTOR
//==============================================================================

MeshManager::MeshManager() :
    _initialized(false),
    _nodeId(0),
    _nodeName("WildCAM"),
    _role(MESH_ROLE_NODE),
    _latitude(0.0f),
    _longitude(0.0f),
    _lowPowerMode(false),
    _coordinatorId(0),
    _lastBeaconTime(0),
    _lastMaintenanceTime(0),
    _packetsReceived(0),
    _packetsSent(0),
    _packetsForwarded(0),
    _lastRssi(0),
    _lastSnr(0.0f),
    _messageCallback(nullptr),
    _wildlifeCallback(nullptr),
    _nodeCallback(nullptr)
{
    _nodes.reserve(MESH_MAX_NODES);
    _routes.reserve(MESH_MAX_NODES);
}

MeshManager::~MeshManager() {
    cleanup();
}

//==============================================================================
// INITIALIZATION
//==============================================================================

bool MeshManager::init(uint32_t nodeId) {
#if !LORA_ENABLED
    LOG_WARN("LoRa mesh networking is disabled in config");
    Serial.println("LoRa mesh networking is disabled in config.h");
    Serial.println("Set LORA_ENABLED to true to enable");
    return false;
#else
    if (_initialized) {
        LOG_WARN("MeshManager already initialized");
        return true;
    }
    
    LOG_INFO("Initializing LoRa mesh network...");
    Serial.println("Initializing LoRa mesh network...");
    
    // Set or generate node ID
    if (nodeId != 0) {
        _nodeId = nodeId;
    } else if (MESH_NODE_ID != 0) {
        _nodeId = MESH_NODE_ID;
    } else {
        _nodeId = generateNodeId();
    }
    
    LOG_INFO("Node ID: 0x%08X", _nodeId);
    Serial.printf("   Node ID: 0x%08X\n", _nodeId);
    
    // Initialize SPI for LoRa module
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN);
    
    // Configure LoRa pins
    LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
    
    // Initialize LoRa radio
    if (!LoRa.begin(LORA_FREQUENCY)) {
        LOG_ERROR("LoRa initialization failed!");
        Serial.println("   ✗ LoRa radio initialization failed!");
        return false;
    }
    
    // Configure radio parameters
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.enableCrc();
    
    LOG_INFO("LoRa radio configured:");
    LOG_INFO("  Frequency: %lu Hz", LORA_FREQUENCY);
    LOG_INFO("  Bandwidth: %lu Hz", LORA_BANDWIDTH);
    LOG_INFO("  Spreading Factor: %d", LORA_SPREADING_FACTOR);
    LOG_INFO("  TX Power: %d dBm", LORA_TX_POWER);
    
    Serial.printf("   Frequency: %.2f MHz\n", LORA_FREQUENCY / 1000000.0f);
    Serial.printf("   Bandwidth: %lu kHz\n", LORA_BANDWIDTH / 1000);
    Serial.printf("   Spreading Factor: SF%d\n", LORA_SPREADING_FACTOR);
    Serial.printf("   TX Power: %d dBm\n", LORA_TX_POWER);
    
    // Start receiving
    LoRa.receive();
    
    _initialized = true;
    _lastBeaconTime = millis();
    _lastMaintenanceTime = millis();
    
    LOG_INFO("LoRa mesh network initialized successfully");
    Serial.println("   ✓ LoRa mesh network initialized");
    
    // Send initial beacon
    sendBeacon();
    
    return true;
#endif
}

bool MeshManager::isInitialized() const {
    return _initialized;
}

void MeshManager::cleanup() {
#if LORA_ENABLED
    if (_initialized) {
        LoRa.end();
        _initialized = false;
        _nodes.clear();
        _routes.clear();
        LOG_INFO("MeshManager cleaned up");
    }
#endif
}

//==============================================================================
// NODE CONFIGURATION
//==============================================================================

void MeshManager::setNodeId(uint32_t nodeId) {
    _nodeId = nodeId;
    LOG_DEBUG("Node ID set to: 0x%08X", _nodeId);
}

uint32_t MeshManager::getNodeId() const {
    return _nodeId;
}

void MeshManager::setNodeName(const String& name) {
    _nodeName = name.substring(0, 8);  // Max 8 chars
    LOG_DEBUG("Node name set to: %s", _nodeName.c_str());
}

String MeshManager::getNodeName() const {
    return _nodeName;
}

void MeshManager::setPosition(float latitude, float longitude) {
    _latitude = latitude;
    _longitude = longitude;
    LOG_DEBUG("Position set to: %.6f, %.6f", _latitude, _longitude);
}

//==============================================================================
// COORDINATOR FUNCTIONS
//==============================================================================

bool MeshManager::becomeCoordinator() {
    if (!_initialized) {
        LOG_WARN("Cannot become coordinator: mesh not initialized");
        return false;
    }
    
    _role = MESH_ROLE_COORDINATOR;
    _coordinatorId = _nodeId;
    
    LOG_INFO("Node became coordinator: 0x%08X", _nodeId);
    Serial.printf("Node became mesh coordinator: 0x%08X\n", _nodeId);
    
    // Broadcast coordinator announcement
    sendBeacon();
    
    return true;
}

bool MeshManager::isCoordinator() const {
    return _role == MESH_ROLE_COORDINATOR;
}

uint32_t MeshManager::getCoordinatorId() const {
    return _coordinatorId;
}

//==============================================================================
// MESSAGE TRANSMISSION
//==============================================================================

bool MeshManager::sendMessage(uint32_t destination, const uint8_t* data, size_t length,
                             MeshPriority priority) {
#if !LORA_ENABLED
    return false;
#else
    if (!_initialized || data == nullptr || length == 0) {
        return false;
    }
    
    // Build packet header
    uint8_t packet[256];
    size_t packetLen = 0;
    
    // Packet structure: [type:1][priority:1][from:4][to:4][hopCount:1][length:2][data:N]
    packet[packetLen++] = PACKET_DATA;
    packet[packetLen++] = (uint8_t)priority;
    
    // Source node ID
    packet[packetLen++] = (_nodeId >> 24) & 0xFF;
    packet[packetLen++] = (_nodeId >> 16) & 0xFF;
    packet[packetLen++] = (_nodeId >> 8) & 0xFF;
    packet[packetLen++] = _nodeId & 0xFF;
    
    // Destination node ID
    packet[packetLen++] = (destination >> 24) & 0xFF;
    packet[packetLen++] = (destination >> 16) & 0xFF;
    packet[packetLen++] = (destination >> 8) & 0xFF;
    packet[packetLen++] = destination & 0xFF;
    
    // Hop count
    packet[packetLen++] = MESH_MAX_HOPS;
    
    // Data length
    packet[packetLen++] = (length >> 8) & 0xFF;
    packet[packetLen++] = length & 0xFF;
    
    // Copy data
    if (packetLen + length > sizeof(packet)) {
        LOG_WARN("Message too large: %zu bytes", length);
        return false;
    }
    memcpy(&packet[packetLen], data, length);
    packetLen += length;
    
    // Transmit
    LoRa.beginPacket();
    LoRa.write(packet, packetLen);
    LoRa.endPacket();
    
    // Return to receive mode
    LoRa.receive();
    
    _packetsSent++;
    LOG_DEBUG("Sent message to 0x%08X, %d bytes", destination, length);
    
    return true;
#endif
}

bool MeshManager::broadcastMessage(const uint8_t* data, size_t length, MeshPriority priority) {
    return sendMessage(0xFFFFFFFF, data, length, priority);  // Broadcast address
}

bool MeshManager::sendText(uint32_t destination, const String& message) {
    return sendMessage(destination, (const uint8_t*)message.c_str(), message.length());
}

bool MeshManager::sendWildlifeEvent(const WildlifeEvent& event) {
#if !LORA_ENABLED
    return false;
#else
    if (!_initialized) {
        return false;
    }
    
    // Serialize wildlife event
    uint8_t packet[128];
    size_t packetLen = 0;
    
    packet[packetLen++] = PACKET_WILDLIFE;
    packet[packetLen++] = (uint8_t)PRIORITY_HIGH;
    
    // Source node ID
    packet[packetLen++] = (_nodeId >> 24) & 0xFF;
    packet[packetLen++] = (_nodeId >> 16) & 0xFF;
    packet[packetLen++] = (_nodeId >> 8) & 0xFF;
    packet[packetLen++] = _nodeId & 0xFF;
    
    // Broadcast destination
    packet[packetLen++] = 0xFF;
    packet[packetLen++] = 0xFF;
    packet[packetLen++] = 0xFF;
    packet[packetLen++] = 0xFF;
    
    // Hop count
    packet[packetLen++] = MESH_MAX_HOPS;
    
    // Timestamp
    packet[packetLen++] = (event.timestamp >> 24) & 0xFF;
    packet[packetLen++] = (event.timestamp >> 16) & 0xFF;
    packet[packetLen++] = (event.timestamp >> 8) & 0xFF;
    packet[packetLen++] = event.timestamp & 0xFF;
    
    // Confidence (scaled to 0-255)
    packet[packetLen++] = (uint8_t)(event.confidence * 255);
    
    // Has image flag
    packet[packetLen++] = event.hasImage ? 1 : 0;
    
    // Species name (truncated to 16 chars)
    String species = event.species.substring(0, 16);
    packet[packetLen++] = species.length();
    memcpy(&packet[packetLen], species.c_str(), species.length());
    packetLen += species.length();
    
    // Transmit
    LoRa.beginPacket();
    LoRa.write(packet, packetLen);
    LoRa.endPacket();
    
    // Return to receive mode
    LoRa.receive();
    
    _packetsSent++;
    LOG_INFO("Broadcast wildlife event: %s (%.1f%% confidence)", 
             event.species.c_str(), event.confidence * 100);
    
    return true;
#endif
}

bool MeshManager::sendImage(const uint8_t* imageData, size_t imageSize, const String& filename) {
#if !LORA_ENABLED
    return false;
#else
    if (!_initialized || imageData == nullptr || imageSize == 0) {
        return false;
    }
    
    // Image transmission requires chunking due to LoRa packet size limits
    // Maximum payload per packet is ~240 bytes
    const size_t CHUNK_SIZE = 200;
    size_t totalChunks = (imageSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
    
    LOG_INFO("Starting image transmission: %s (%zu bytes, %zu chunks)", 
             filename.c_str(), imageSize, totalChunks);
    
    for (size_t chunk = 0; chunk < totalChunks; chunk++) {
        size_t offset = chunk * CHUNK_SIZE;
        size_t chunkLen = min(CHUNK_SIZE, imageSize - offset);
        
        uint8_t packet[256];
        size_t packetLen = 0;
        
        packet[packetLen++] = PACKET_IMAGE;
        packet[packetLen++] = (uint8_t)PRIORITY_NORMAL;
        
        // Source node ID
        packet[packetLen++] = (_nodeId >> 24) & 0xFF;
        packet[packetLen++] = (_nodeId >> 16) & 0xFF;
        packet[packetLen++] = (_nodeId >> 8) & 0xFF;
        packet[packetLen++] = _nodeId & 0xFF;
        
        // Broadcast destination
        packet[packetLen++] = 0xFF;
        packet[packetLen++] = 0xFF;
        packet[packetLen++] = 0xFF;
        packet[packetLen++] = 0xFF;
        
        // Hop count
        packet[packetLen++] = MESH_MAX_HOPS;
        
        // Chunk info
        packet[packetLen++] = (chunk >> 8) & 0xFF;
        packet[packetLen++] = chunk & 0xFF;
        packet[packetLen++] = (totalChunks >> 8) & 0xFF;
        packet[packetLen++] = totalChunks & 0xFF;
        
        // Chunk data
        memcpy(&packet[packetLen], &imageData[offset], chunkLen);
        packetLen += chunkLen;
        
        // Transmit chunk
        LoRa.beginPacket();
        LoRa.write(packet, packetLen);
        LoRa.endPacket();
        
        _packetsSent++;
        
        // Short delay between chunks to avoid radio congestion
        // Use yield() to prevent watchdog timeout during long transfers
        delay(50);
        yield();
    }
    
    // Return to receive mode
    LoRa.receive();
    
    LOG_INFO("Image transmission complete: %zu packets sent", totalChunks);
    return true;
#endif
}

//==============================================================================
// MESSAGE PROCESSING
//==============================================================================

void MeshManager::process() {
#if LORA_ENABLED
    if (!_initialized) {
        return;
    }
    
    unsigned long now = millis();
    
    // Check for incoming packets
    int packetSize = LoRa.parsePacket();
    if (packetSize > 0) {
        uint8_t buffer[256];
        size_t len = 0;
        
        while (LoRa.available() && len < sizeof(buffer)) {
            buffer[len++] = LoRa.read();
        }
        
        _lastRssi = LoRa.packetRssi();
        _lastSnr = LoRa.packetSnr();
        _packetsReceived++;
        
        if (len > 0) {
            // Process packet based on type
            uint8_t packetType = buffer[0];
            
            switch (packetType) {
                case PACKET_BEACON:
                    processBeacon(buffer, len, _lastRssi, _lastSnr);
                    break;
                    
                case PACKET_DATA:
                case PACKET_WILDLIFE:
                case PACKET_TELEMETRY:
                    // Extract destination and check if for us or needs forwarding
                    if (len >= 10) {
                        uint32_t destination = ((uint32_t)buffer[6] << 24) | 
                                              ((uint32_t)buffer[7] << 16) |
                                              ((uint32_t)buffer[8] << 8) | 
                                              buffer[9];
                        
                        if (destination == _nodeId || destination == 0xFFFFFFFF) {
                            // Message for us
                            uint32_t source = ((uint32_t)buffer[2] << 24) | 
                                             ((uint32_t)buffer[3] << 16) |
                                             ((uint32_t)buffer[4] << 8) | 
                                             buffer[5];
                            
                            if (packetType == PACKET_WILDLIFE && _wildlifeCallback) {
                                WildlifeEvent event;
                                event.nodeId = source;
                                event.timestamp = ((uint32_t)buffer[12] << 24) | 
                                                 ((uint32_t)buffer[13] << 16) |
                                                 ((uint32_t)buffer[14] << 8) | 
                                                 buffer[15];
                                event.confidence = buffer[16] / 255.0f;
                                event.hasImage = buffer[17] != 0;
                                
                                uint8_t speciesLen = buffer[18];
                                if (speciesLen > 0 && len >= 19 + speciesLen) {
                                    event.species = String((char*)&buffer[19], speciesLen);
                                }
                                
                                _wildlifeCallback(event);
                            }
                            
                            if (_messageCallback && len > 13) {
                                size_t dataLen = ((size_t)buffer[11] << 8) | buffer[12];
                                if (dataLen > 0 && len >= 13 + dataLen) {
                                    _messageCallback(source, &buffer[13], dataLen);
                                }
                            }
                        } else if (destination != 0xFFFFFFFF) {
                            // Forward packet if hop count > 0
                            forwardPacket(buffer, len);
                        }
                    }
                    break;
                    
                default:
                    LOG_DEBUG("Unknown packet type: 0x%02X", packetType);
                    break;
            }
        }
    }
    
    // Send periodic beacon
    unsigned long beaconInterval = _lowPowerMode ? 
                                   MESH_BEACON_INTERVAL_MS * 4 : 
                                   MESH_BEACON_INTERVAL_MS;
    if (now - _lastBeaconTime > beaconInterval) {
        sendBeacon();
        _lastBeaconTime = now;
    }
    
    // Periodic maintenance (every 60 seconds)
    if (now - _lastMaintenanceTime > 60000) {
        cleanupExpiredNodes();
        updateRouting();
        _lastMaintenanceTime = now;
    }
#endif
}

bool MeshManager::hasMessages() const {
#if LORA_ENABLED
    return LoRa.parsePacket() > 0;
#else
    return false;
#endif
}

//==============================================================================
// NETWORK STATUS
//==============================================================================

MeshNetworkStatus MeshManager::getNetworkStatus() const {
    MeshNetworkStatus status;
    status.initialized = _initialized;
    status.nodeId = _nodeId;
    status.role = _role;
    status.connectedNodes = _nodes.size();
    status.coordinatorId = _coordinatorId;
    status.isCoordinator = (_role == MESH_ROLE_COORDINATOR);
    status.lastRssi = _lastRssi;
    status.lastSnr = _lastSnr;
    status.packetsReceived = _packetsReceived;
    status.packetsSent = _packetsSent;
    status.packetsForwarded = _packetsForwarded;
    status.lastBeaconTime = _lastBeaconTime;
    status.lastMessageTime = millis();  // Placeholder
    return status;
}

SignalQuality MeshManager::getSignalQuality() const {
    SignalQuality quality;
    quality.rssi = _lastRssi;
    quality.snr = _lastSnr;
    quality.strength = classifySignal(_lastRssi);
    return quality;
}

std::vector<MeshNode> MeshManager::getNodes() const {
    return _nodes;
}

MeshNode MeshManager::getNode(uint32_t nodeId) const {
    for (const auto& node : _nodes) {
        if (node.nodeId == nodeId) {
            return node;
        }
    }
    return MeshNode();  // Empty node
}

uint16_t MeshManager::getNodeCount() const {
    uint16_t count = 0;
    for (const auto& node : _nodes) {
        if (node.isOnline) {
            count++;
        }
    }
    return count;
}

//==============================================================================
// ROUTING
//==============================================================================

std::vector<MeshRoute> MeshManager::getRoutingTable() const {
    return _routes;
}

uint32_t MeshManager::getNextHop(uint32_t destination) const {
    for (const auto& route : _routes) {
        if (route.destination == destination) {
            return route.nextHop;
        }
    }
    return 0;  // No route
}

//==============================================================================
// CALLBACKS
//==============================================================================

void MeshManager::setMessageCallback(MeshMessageCallback callback) {
    _messageCallback = callback;
}

void MeshManager::setWildlifeCallback(MeshWildlifeCallback callback) {
    _wildlifeCallback = callback;
}

void MeshManager::setNodeCallback(MeshNodeCallback callback) {
    _nodeCallback = callback;
}

//==============================================================================
// POWER MANAGEMENT
//==============================================================================

void MeshManager::enterLowPowerMode() {
    _lowPowerMode = true;
#if LORA_ENABLED
    LoRa.setTxPower(LORA_TX_POWER - 5);  // Reduce TX power
#endif
    LOG_INFO("Entered low power mode");
}

void MeshManager::exitLowPowerMode() {
    _lowPowerMode = false;
#if LORA_ENABLED
    LoRa.setTxPower(LORA_TX_POWER);
#endif
    LOG_INFO("Exited low power mode");
}

bool MeshManager::isLowPowerMode() const {
    return _lowPowerMode;
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

void MeshManager::sendBeacon() {
#if LORA_ENABLED
    uint8_t beacon[64];
    size_t len = 0;
    
    beacon[len++] = PACKET_BEACON;
    beacon[len++] = (uint8_t)PRIORITY_LOW;
    
    // Node ID
    beacon[len++] = (_nodeId >> 24) & 0xFF;
    beacon[len++] = (_nodeId >> 16) & 0xFF;
    beacon[len++] = (_nodeId >> 8) & 0xFF;
    beacon[len++] = _nodeId & 0xFF;
    
    // Role
    beacon[len++] = (uint8_t)_role;
    
    // Node name
    beacon[len++] = _nodeName.length();
    memcpy(&beacon[len], _nodeName.c_str(), _nodeName.length());
    len += _nodeName.length();
    
    // Coordinator ID
    beacon[len++] = (_coordinatorId >> 24) & 0xFF;
    beacon[len++] = (_coordinatorId >> 16) & 0xFF;
    beacon[len++] = (_coordinatorId >> 8) & 0xFF;
    beacon[len++] = _coordinatorId & 0xFF;
    
    LoRa.beginPacket();
    LoRa.write(beacon, len);
    LoRa.endPacket();
    
    // Return to receive mode
    LoRa.receive();
    
    _packetsSent++;
    LOG_DEBUG("Sent beacon");
#endif
}

void MeshManager::processBeacon(const uint8_t* data, size_t length, int16_t rssi, float snr) {
    if (length < 8) {
        return;
    }
    
    MeshNode node;
    node.nodeId = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                  ((uint32_t)data[4] << 8) | data[5];
    node.role = (MeshRole)data[6];
    
    uint8_t nameLen = data[7];
    if (nameLen > 0 && length >= 8 + nameLen) {
        node.shortName = String((char*)&data[8], nameLen);
    }
    
    size_t offset = 8 + nameLen;
    if (length >= offset + 4) {
        uint32_t coordId = ((uint32_t)data[offset] << 24) | 
                           ((uint32_t)data[offset+1] << 16) |
                           ((uint32_t)data[offset+2] << 8) | 
                           data[offset+3];
        
        // Update coordinator if this beacon has one
        if (coordId != 0 && (_coordinatorId == 0 || node.role == MESH_ROLE_COORDINATOR)) {
            _coordinatorId = coordId;
        }
    }
    
    node.lastRssi = rssi;
    node.lastSnr = snr;
    node.lastSeen = millis();
    node.hopCount = 1;  // Direct reception = 1 hop
    node.isOnline = true;
    
    addOrUpdateNode(node);
    
    LOG_DEBUG("Received beacon from 0x%08X (%s), RSSI: %d", 
              node.nodeId, node.shortName.c_str(), rssi);
}

void MeshManager::updateRouting() {
    // Simple routing update: direct routes for all known nodes
    _routes.clear();
    
    for (const auto& node : _nodes) {
        if (node.isOnline && node.nodeId != _nodeId) {
            MeshRoute route;
            route.destination = node.nodeId;
            route.nextHop = node.nodeId;  // Direct for now
            route.hopCount = node.hopCount;
            route.reliability = 1.0f - (abs(node.lastRssi) / 137.0f);  // Approximate
            route.lastUsed = millis();
            _routes.push_back(route);
        }
    }
}

void MeshManager::cleanupExpiredNodes() {
    unsigned long now = millis();
    
    for (auto& node : _nodes) {
        if (now - node.lastSeen > MESH_NODE_TIMEOUT_MS) {
            node.isOnline = false;
            LOG_DEBUG("Node 0x%08X marked offline", node.nodeId);
        }
    }
}

uint32_t MeshManager::generateNodeId() {
    // Generate from ESP32 MAC address using all 6 bytes
    // XOR upper bytes with lower to ensure all bits contribute to uniqueness
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    // Create a 32-bit ID by XORing pairs of bytes and combining
    uint32_t upper = ((uint32_t)mac[0] << 8) | mac[1];
    uint32_t middle = ((uint32_t)mac[2] << 8) | mac[3];
    uint32_t lower = ((uint32_t)mac[4] << 8) | mac[5];
    
    // Combine with XOR for better distribution
    return (upper << 16) ^ (middle << 8) ^ lower ^ (middle << 16);
}

SignalStrength MeshManager::classifySignal(int16_t rssi) const {
    if (rssi > -70) return SIGNAL_EXCELLENT;
    if (rssi > -85) return SIGNAL_GOOD;
    if (rssi > -100) return SIGNAL_FAIR;
    return SIGNAL_POOR;
}

void MeshManager::addOrUpdateNode(const MeshNode& node) {
    for (auto& existingNode : _nodes) {
        if (existingNode.nodeId == node.nodeId) {
            // Update existing node
            existingNode.shortName = node.shortName;
            existingNode.lastRssi = node.lastRssi;
            existingNode.lastSnr = node.lastSnr;
            existingNode.lastSeen = node.lastSeen;
            existingNode.role = node.role;
            existingNode.isOnline = true;
            
            if (_nodeCallback) {
                _nodeCallback(existingNode, false);
            }
            return;
        }
    }
    
    // Add new node
    if (_nodes.size() < MESH_MAX_NODES) {
        _nodes.push_back(node);
        LOG_INFO("New node discovered: 0x%08X (%s)", node.nodeId, node.shortName.c_str());
        
        if (_nodeCallback) {
            _nodeCallback(node, true);
        }
    }
}

bool MeshManager::forwardPacket(const uint8_t* data, size_t length) {
#if LORA_ENABLED
    if (length < 11) {
        return false;
    }
    
    // Check hop count
    uint8_t hopCount = data[10];
    if (hopCount <= 1) {
        LOG_DEBUG("Packet TTL expired, not forwarding");
        return false;
    }
    
    // Create forwarded packet with decremented hop count
    uint8_t forwardPacket[256];
    memcpy(forwardPacket, data, length);
    forwardPacket[10] = hopCount - 1;
    
    // Transmit
    LoRa.beginPacket();
    LoRa.write(forwardPacket, length);
    LoRa.endPacket();
    
    // Return to receive mode
    LoRa.receive();
    
    _packetsForwarded++;
    LOG_DEBUG("Forwarded packet, remaining hops: %d", hopCount - 1);
    
    return true;
#else
    return false;
#endif
}
