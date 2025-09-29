/**
 * @file mesh_interface.cpp
 * @brief Meshtastic Protocol Interface Implementation for ESP32WildlifeCAM
 * 
 * Core implementation of the Meshtastic protocol stack with wildlife-specific
 * optimizations and mesh networking capabilities.
 */

#include "mesh_interface.h"
#include "../debug_utils.h"
#include <WiFi.h>
#include <esp_system.h>
#include <mbedtls/aes.h>

// Static instance for callback access
MeshInterface* MeshInterface::instance_ = nullptr;

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

MeshInterface::MeshInterface() 
    : initialized_(false)
    , radioDriver_(nullptr)
    , nodeId_(0)
    , activeChannel_(0)
    , nextPacketId_(1)
    , messageCallback_(nullptr)
    , nodeDiscoveredCallback_(nullptr)
    , routeChangedCallback_(nullptr)
    , networkStatusCallback_(nullptr)
    , lastBeacon_(0)
    , lastMaintenance_(0)
    , lastNodeDiscovery_(0)
{
    instance_ = this;
    
    // Initialize statistics
    memset(&statistics_, 0, sizeof(statistics_));
    
    // Generate node ID from MAC address
    generateNodeId();
}

MeshInterface::~MeshInterface() {
    cleanup();
    instance_ = nullptr;
}

// ===========================
// INITIALIZATION
// ===========================

bool MeshInterface::init(const LoRaConfig& radioConfig) {
    DEBUG_PRINTLN("MeshInterface: Initializing...");
    
    // Create and initialize radio driver
    radioDriver_ = createLoRaDriver(radioConfig);
    if (!radioDriver_) {
        DEBUG_PRINTLN("MeshInterface: Failed to create radio driver");
        return false;
    }
    
    // Set radio receive callback
    radioDriver_->setReceiveCallback(onRadioReceive);
    
    // Initialize default channel
    ChannelSettings defaultChannel;
    defaultChannel.index = 0;
    defaultChannel.name = "default";
    defaultChannel.psk = MESH_CHANNEL_KEY;
    defaultChannel.frequency = radioConfig.frequency;
    defaultChannel.bandwidth = radioConfig.bandwidth;
    defaultChannel.spreadingFactor = radioConfig.spreadingFactor;
    defaultChannel.codingRate = radioConfig.codingRate;
    defaultChannel.uplinkEnabled = true;
    defaultChannel.downlinkEnabled = true;
    defaultChannel.dutyCycle = 100;
    
    channels_.push_back(defaultChannel);
    
    // Initialize node info
    nodeInfo_.nodeId = nodeId_;
    nodeInfo_.shortName = "WildlifeCam";
    nodeInfo_.longName = "ESP32 Wildlife Camera";
    nodeInfo_.macAddress = WiFi.macAddress();
    nodeInfo_.status = NODE_STATUS_ONLINE;
    nodeInfo_.lastSeen = millis();
    nodeInfo_.batteryLevel = 100;
    nodeInfo_.voltage = 4.2;
    nodeInfo_.uptimeSeconds = millis() / 1000;
    nodeInfo_.hasPosition = false;
    
    initialized_ = true;
    
    DEBUG_PRINTF("MeshInterface: Initialized successfully (Node ID: %08X)\n", nodeId_);
    
    return true;
}

bool MeshInterface::configure(const ChannelSettings& channelConfig) {
    if (!initialized_) {
        return false;
    }
    
    // Update channel configuration
    if (channelConfig.index < channels_.size()) {
        channels_[channelConfig.index] = channelConfig;
        
        // Reconfigure radio if this is the active channel
        if (channelConfig.index == activeChannel_) {
            // Update radio parameters
            radioDriver_->setFrequency(channelConfig.frequency);
            radioDriver_->setBandwidth(channelConfig.bandwidth);
            radioDriver_->setSpreadingFactor(channelConfig.spreadingFactor);
            radioDriver_->setCodingRate(channelConfig.codingRate);
        }
        
        return true;
    }
    
    return false;
}

void MeshInterface::cleanup() {
    if (radioDriver_) {
        delete radioDriver_;
        radioDriver_ = nullptr;
    }
    
    channels_.clear();
    nodes_.clear();
    routingTable_.clear();
    messageQueue_.clear();
    outgoingQueue_.clear();
    
    initialized_ = false;
}

// ===========================
// NODE MANAGEMENT
// ===========================

bool MeshInterface::setNodeInfo(const NodeInfo& nodeInfo) {
    if (!initialized_) {
        return false;
    }
    
    nodeInfo_ = nodeInfo;
    nodeInfo_.nodeId = nodeId_;  // Don't allow changing node ID
    nodeInfo_.lastSeen = millis();
    
    return true;
}

NodeInfo MeshInterface::getNodeInfo() const {
    return nodeInfo_;
}

uint32_t MeshInterface::getNodeId() const {
    return nodeId_;
}

bool MeshInterface::generateNodeId() {
    // Generate node ID from MAC address
    nodeId_ = generateNodeIdFromMac();
    return (nodeId_ != 0);
}

// ===========================
// CHANNEL MANAGEMENT
// ===========================

bool MeshInterface::addChannel(const ChannelSettings& channel) {
    if (channels_.size() >= 8) {  // Meshtastic limit
        return false;
    }
    
    channels_.push_back(channel);
    return true;
}

bool MeshInterface::removeChannel(uint8_t channelIndex) {
    if (channelIndex >= channels_.size() || channelIndex == 0) {
        return false;  // Can't remove default channel
    }
    
    channels_.erase(channels_.begin() + channelIndex);
    
    // Adjust active channel if necessary
    if (activeChannel_ >= channels_.size()) {
        activeChannel_ = 0;
    }
    
    return true;
}

bool MeshInterface::setActiveChannel(uint8_t channelIndex) {
    if (channelIndex >= channels_.size()) {
        return false;
    }
    
    if (channelIndex != activeChannel_) {
        activeChannel_ = channelIndex;
        
        // Reconfigure radio for new channel
        const ChannelSettings& channel = channels_[channelIndex];
        radioDriver_->setFrequency(channel.frequency);
        radioDriver_->setBandwidth(channel.bandwidth);
        radioDriver_->setSpreadingFactor(channel.spreadingFactor);
        radioDriver_->setCodingRate(channel.codingRate);
    }
    
    return true;
}

ChannelSettings MeshInterface::getChannel(uint8_t channelIndex) const {
    if (channelIndex < channels_.size()) {
        return channels_[channelIndex];
    }
    return ChannelSettings();
}

std::vector<ChannelSettings> MeshInterface::getChannels() const {
    return channels_;
}

// ===========================
// MESSAGE TRANSMISSION
// ===========================

bool MeshInterface::sendMessage(uint32_t destination, MeshPacketType type, 
                                const std::vector<uint8_t>& payload, 
                                RoutingPriority priority, bool requestAck) {
    if (!initialized_ || payload.size() > MESH_MAX_PACKET_SIZE) {
        return false;
    }
    
    // Create packet
    MeshPacket packet = createPacket(destination, type, payload, priority, requestAck);
    
    // Add to outgoing queue
    outgoingQueue_.push_back(packet);
    
    DEBUG_PRINTF("MeshInterface: Queued message to %08X (type: %d, size: %d)\n", 
                 destination, type, payload.size());
    
    return true;
}

bool MeshInterface::broadcastMessage(MeshPacketType type, const std::vector<uint8_t>& payload,
                                   RoutingPriority priority) {
    return sendMessage(0, type, payload, priority, false);  // 0 = broadcast
}

bool MeshInterface::sendText(uint32_t destination, const String& text) {
    std::vector<uint8_t> payload(text.begin(), text.end());
    return sendMessage(destination, PACKET_TYPE_TEXT, payload);
}

bool MeshInterface::sendPosition(uint32_t destination, float lat, float lon, uint32_t alt) {
    DynamicJsonDocument doc(256);
    doc["lat"] = lat;
    doc["lon"] = lon;
    doc["alt"] = alt;
    doc["time"] = millis();
    
    std::vector<uint8_t> payload = serializeJson(doc.as<JsonObject>());
    return sendMessage(destination, PACKET_TYPE_POSITION, payload);
}

bool MeshInterface::sendTelemetry(uint32_t destination, const JsonObject& telemetryData) {
    std::vector<uint8_t> payload = serializeJson(telemetryData);
    return sendMessage(destination, PACKET_TYPE_TELEMETRY, payload);
}

// ===========================
// MESSAGE RECEPTION
// ===========================

bool MeshInterface::receiveMessage(MeshPacket& packet) {
    if (messageQueue_.empty()) {
        return false;
    }
    
    packet = messageQueue_.front();
    messageQueue_.erase(messageQueue_.begin());
    
    return true;
}

bool MeshInterface::hasMessages() const {
    return !messageQueue_.empty();
}

size_t MeshInterface::getMessageQueueSize() const {
    return messageQueue_.size();
}

// ===========================
// ROUTING MANAGEMENT
// ===========================

bool MeshInterface::addRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount) {
    RouteInfo route;
    route.destination = destination;
    route.nextHop = nextHop;
    route.hopCount = hopCount;
    route.lastUsed = millis();
    route.reliability = 1.0;
    route.signalStrength = -100;
    
    routingTable_[destination] = route;
    
    if (routeChangedCallback_) {
        routeChangedCallback_(destination, nextHop);
    }
    
    return true;
}

bool MeshInterface::removeRoute(uint32_t destination) {
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end()) {
        routingTable_.erase(it);
        return true;
    }
    return false;
}

RouteInfo MeshInterface::getRoute(uint32_t destination) const {
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end()) {
        return it->second;
    }
    return RouteInfo();
}

std::vector<RouteInfo> MeshInterface::getRoutingTable() const {
    std::vector<RouteInfo> routes;
    for (const auto& pair : routingTable_) {
        routes.push_back(pair.second);
    }
    return routes;
}

// ===========================
// NETWORK DISCOVERY
// ===========================

bool MeshInterface::startNodeDiscovery() {
    // Broadcast node info request
    DynamicJsonDocument doc(256);
    doc["type"] = "node_discovery";
    doc["requester"] = nodeId_;
    doc["timestamp"] = millis();
    
    std::vector<uint8_t> payload = serializeJson(doc.as<JsonObject>());
    return broadcastMessage(PACKET_TYPE_ADMIN, payload);
}

bool MeshInterface::sendBeacon() {
    // Update node info
    nodeInfo_.lastSeen = millis();
    nodeInfo_.uptimeSeconds = millis() / 1000;
    
    // Serialize node info
    std::vector<uint8_t> payload = serializeNodeInfo(nodeInfo_);
    
    // Broadcast beacon
    bool success = broadcastMessage(PACKET_TYPE_ROUTING, payload, PRIORITY_BACKGROUND);
    
    if (success) {
        lastBeacon_ = millis();
    }
    
    return success;
}

bool MeshInterface::requestNodeInfo(uint32_t nodeId) {
    DynamicJsonDocument doc(256);
    doc["type"] = "node_info_request";
    doc["requester"] = nodeId_;
    doc["target"] = nodeId;
    doc["timestamp"] = millis();
    
    std::vector<uint8_t> payload = serializeJson(doc.as<JsonObject>());
    return sendMessage(nodeId, PACKET_TYPE_ADMIN, payload);
}

// ===========================
// PROCESSING METHODS
// ===========================

void MeshInterface::process() {
    if (!initialized_) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Process outgoing packets
    processOutgoingPackets();
    
    // Send periodic beacon
    if (currentTime - lastBeacon_ > MESH_BEACON_INTERVAL) {
        sendBeacon();
    }
    
    // Perform maintenance
    if (currentTime - lastMaintenance_ > 60000) {  // Every minute
        performMaintenance();
        lastMaintenance_ = currentTime;
    }
    
    // Update node info uptime
    nodeInfo_.uptimeSeconds = currentTime / 1000;
}

void MeshInterface::performMaintenance() {
    // Clean up expired routes
    cleanupRoutingTable();
    
    // Remove expired nodes
    removeExpiredNodes();
    
    // Clean up recent packets list
    cleanupRecentPackets();
    
    // Update statistics
    statistics_.networkNodes = nodes_.size();
    statistics_.routingTableSize = routingTable_.size();
}

// ===========================
// PACKET PROCESSING
// ===========================

bool MeshInterface::processIncomingPacket(const LoRaPacket& radioPacket) {
    if (radioPacket.length < sizeof(MeshPacketHeader)) {
        statistics_.packetsDropped++;
        return false;
    }
    
    // Parse mesh packet
    MeshPacket packet;
    memcpy(&packet.header, radioPacket.data, sizeof(MeshPacketHeader));
    
    // Extract payload
    size_t payloadSize = radioPacket.length - sizeof(MeshPacketHeader);
    packet.payload.resize(payloadSize);
    memcpy(packet.payload.data(), radioPacket.data + sizeof(MeshPacketHeader), payloadSize);
    
    packet.rxRssi = radioPacket.rssi;
    packet.rxSnr = radioPacket.snr;
    packet.rxTime = radioPacket.timestamp;
    
    statistics_.packetsReceived++;
    statistics_.averageRssi = (statistics_.averageRssi * 0.9) + (radioPacket.rssi * 0.1);
    statistics_.averageSnr = (statistics_.averageSnr * 0.9) + (radioPacket.snr * 0.1);
    
    // Check for duplicate packets
    if (isDuplicatePacket(packet.header.id)) {
        statistics_.duplicatesDropped++;
        return false;
    }
    
    addToRecentPackets(packet.header.id);
    
    // Decrypt if necessary
    if (packet.encrypted) {
        if (!decryptPacket(packet)) {
            statistics_.encryptionErrors++;
            return false;
        }
    }
    
    // Update routing table based on packet source
    updateRoutingTable(packet);
    
    // Check if packet is for us or broadcast
    if (packet.header.to == nodeId_ || packet.header.to == 0) {
        // Process packet based on type
        switch (packet.header.portNum) {
            case PACKET_TYPE_DATA:
                handleDataPacket(packet);
                break;
            case PACKET_TYPE_ROUTING:
                handleRoutingPacket(packet);
                break;
            case PACKET_TYPE_ADMIN:
                handleAdminPacket(packet);
                break;
            case PACKET_TYPE_TELEMETRY:
                handleTelemetryPacket(packet);
                break;
            case PACKET_TYPE_POSITION:
                handlePositionPacket(packet);
                break;
            case PACKET_TYPE_TEXT:
                handleTextPacket(packet);
                break;
            case PACKET_TYPE_WILDLIFE:
                handleWildlifePacket(packet);
                break;
            default:
                DEBUG_PRINTF("MeshInterface: Unknown packet type: %d\n", packet.header.portNum);
                break;
        }
        
        // Add to message queue for application processing
        messageQueue_.push_back(packet);
        
        // Limit queue size
        if (messageQueue_.size() > MESH_MESSAGE_QUEUE_SIZE) {
            messageQueue_.erase(messageQueue_.begin());
        }
        
        // Call message callback
        if (messageCallback_) {
            messageCallback_(packet);
        }
    }
    
    // Forward packet if necessary
    if (shouldForwardPacket(packet)) {
        forwardPacket(packet);
    }
    
    return true;
}

bool MeshInterface::processOutgoingPackets() {
    if (outgoingQueue_.empty()) {
        return false;
    }
    
    // Process one packet per call to avoid blocking
    MeshPacket packet = outgoingQueue_.front();
    outgoingQueue_.erase(outgoingQueue_.begin());
    
    // Encrypt if necessary
    if (packet.encrypted) {
        if (!encryptPacket(packet)) {
            statistics_.encryptionErrors++;
            return false;
        }
    }
    
    // Create radio packet
    std::vector<uint8_t> radioData;
    radioData.resize(sizeof(MeshPacketHeader) + packet.payload.size());
    
    memcpy(radioData.data(), &packet.header, sizeof(MeshPacketHeader));
    memcpy(radioData.data() + sizeof(MeshPacketHeader), packet.payload.data(), packet.payload.size());
    
    // Transmit via radio
    bool success = radioDriver_->transmit(radioData.data(), radioData.size());
    
    if (success) {
        statistics_.packetsSent++;
        
        // Track pending ACKs if requested
        if (packet.header.wantAck) {
            pendingAcks_[packet.header.id] = millis();
        }
    } else {
        statistics_.routingErrors++;
        DEBUG_PRINTF("MeshInterface: Failed to transmit packet %08X\n", packet.header.id);
    }
    
    return success;
}

// ===========================
// PACKET CREATION
// ===========================

MeshPacket MeshInterface::createPacket(uint32_t destination, MeshPacketType type,
                                      const std::vector<uint8_t>& payload,
                                      RoutingPriority priority, bool requestAck) {
    MeshPacket packet;
    
    // Fill header
    packet.header.id = nextPacketId_++;
    packet.header.from = nodeId_;
    packet.header.to = destination;
    packet.header.hopLimit = MESH_HOP_LIMIT;
    packet.header.hopStart = MESH_HOP_LIMIT;
    packet.header.channel = activeChannel_;
    packet.header.priority = priority;
    packet.header.rxTime = millis();
    packet.header.wantAck = requestAck;
    packet.header.portNum = type;
    
    // Set payload
    packet.payload = payload;
    packet.encrypted = MESH_ENCRYPTION_ENABLED;
    
    return packet;
}

// ===========================
// ROUTING METHODS
// ===========================

uint32_t MeshInterface::findNextHop(uint32_t destination) {
    // Direct communication if destination is a neighbor
    auto nodeIt = nodes_.find(destination);
    if (nodeIt != nodes_.end() && nodeIt->second.hopCount == 1) {
        return destination;
    }
    
    // Look up in routing table
    auto routeIt = routingTable_.find(destination);
    if (routeIt != routingTable_.end()) {
        return routeIt->second.nextHop;
    }
    
    // No route found
    return 0;
}

bool MeshInterface::forwardPacket(MeshPacket& packet) {
    if (packet.header.hopLimit <= 1) {
        return false;  // Hop limit exceeded
    }
    
    // Decrement hop limit
    packet.header.hopLimit--;
    
    // Find next hop
    uint32_t nextHop = findNextHop(packet.header.to);
    if (nextHop == 0) {
        statistics_.routingErrors++;
        return false;
    }
    
    // Add to outgoing queue
    outgoingQueue_.push_back(packet);
    statistics_.packetsForwarded++;
    
    return true;
}

bool MeshInterface::shouldForwardPacket(const MeshPacket& packet) {
    // Don't forward if:
    // - Packet is for us
    // - Hop limit exceeded
    // - We are the original sender
    return (packet.header.to != nodeId_ && 
            packet.header.to != 0 &&  // Don't forward broadcasts
            packet.header.hopLimit > 1 &&
            packet.header.from != nodeId_);
}

void MeshInterface::updateRoutingTable(const MeshPacket& packet) {
    uint32_t sourceNode = packet.header.from;
    uint8_t hopCount = packet.header.hopStart - packet.header.hopLimit + 1;
    
    // Update or add route to source
    RouteInfo route;
    route.destination = sourceNode;
    route.nextHop = sourceNode;  // Direct route for now
    route.hopCount = hopCount;
    route.lastUsed = millis();
    route.reliability = 1.0;
    route.signalStrength = packet.rxRssi;
    
    routingTable_[sourceNode] = route;
}

void MeshInterface::cleanupRoutingTable() {
    unsigned long currentTime = millis();
    
    for (auto it = routingTable_.begin(); it != routingTable_.end();) {
        if (currentTime - it->second.lastUsed > MESH_ROUTE_TIMEOUT) {
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
}

// ===========================
// PACKET HANDLERS
// ===========================

bool MeshInterface::handleDataPacket(const MeshPacket& packet) {
    DEBUG_PRINTF("MeshInterface: Received data packet from %08X\n", packet.header.from);
    return true;
}

bool MeshInterface::handleRoutingPacket(const MeshPacket& packet) {
    // Parse node info from routing packet (beacon)
    NodeInfo node = deserializeNodeInfo(packet.payload);
    
    if (node.nodeId != 0) {
        addOrUpdateNode(node);
        
        if (nodeDiscoveredCallback_) {
            nodeDiscoveredCallback_(node);
        }
    }
    
    return true;
}

bool MeshInterface::handleAdminPacket(const MeshPacket& packet) {
    JsonObject adminData = deserializeJson(packet.payload);
    String type = adminData["type"];
    
    if (type == "node_discovery") {
        // Respond with our node info
        std::vector<uint8_t> response = serializeNodeInfo(nodeInfo_);
        sendMessage(packet.header.from, PACKET_TYPE_ROUTING, response);
    } else if (type == "node_info_request") {
        uint32_t target = adminData["target"];
        if (target == nodeId_) {
            std::vector<uint8_t> response = serializeNodeInfo(nodeInfo_);
            sendMessage(packet.header.from, PACKET_TYPE_ROUTING, response);
        }
    }
    
    return true;
}

bool MeshInterface::handleTelemetryPacket(const MeshPacket& packet) {
    DEBUG_PRINTF("MeshInterface: Received telemetry from %08X\n", packet.header.from);
    return true;
}

bool MeshInterface::handlePositionPacket(const MeshPacket& packet) {
    JsonObject posData = deserializeJson(packet.payload);
    
    // Update node position
    auto nodeIt = nodes_.find(packet.header.from);
    if (nodeIt != nodes_.end()) {
        nodeIt->second.latitude = posData["lat"];
        nodeIt->second.longitude = posData["lon"];
        nodeIt->second.altitude = posData["alt"];
        nodeIt->second.hasPosition = true;
    }
    
    return true;
}

bool MeshInterface::handleTextPacket(const MeshPacket& packet) {
    DEBUG_PRINTF("MeshInterface: Received text from %08X\n", packet.header.from);
    return true;
}

bool MeshInterface::handleWildlifePacket(const MeshPacket& packet) {
    DEBUG_PRINTF("MeshInterface: Received wildlife data from %08X\n", packet.header.from);
    return true;
}

// ===========================
// NODE MANAGEMENT
// ===========================

void MeshInterface::addOrUpdateNode(const NodeInfo& node) {
    nodes_[node.nodeId] = node;
    lastSeen_[node.nodeId] = millis();
}

void MeshInterface::removeExpiredNodes() {
    unsigned long currentTime = millis();
    
    for (auto it = nodes_.begin(); it != nodes_.end();) {
        if (isNodeExpired(it->second)) {
            uint32_t nodeId = it->first;
            it = nodes_.erase(it);
            
            // Also remove from lastSeen and routing table
            lastSeen_.erase(nodeId);
            routingTable_.erase(nodeId);
        } else {
            ++it;
        }
    }
}

bool MeshInterface::isNodeExpired(const NodeInfo& node) const {
    unsigned long currentTime = millis();
    auto lastSeenIt = lastSeen_.find(node.nodeId);
    
    if (lastSeenIt != lastSeen_.end()) {
        return (currentTime - lastSeenIt->second) > MESH_NEIGHBOR_TIMEOUT;
    }
    
    return true;  // No last seen record, consider expired
}

// ===========================
// UTILITY METHODS
// ===========================

bool MeshInterface::isDuplicatePacket(uint32_t packetId) {
    return recentPackets_.find(packetId) != recentPackets_.end();
}

void MeshInterface::addToRecentPackets(uint32_t packetId) {
    recentPackets_[packetId] = millis();
}

void MeshInterface::cleanupRecentPackets() {
    unsigned long currentTime = millis();
    
    for (auto it = recentPackets_.begin(); it != recentPackets_.end();) {
        if (currentTime - it->second > 300000) {  // 5 minutes
            it = recentPackets_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<uint8_t> MeshInterface::serializeNodeInfo(const NodeInfo& node) {
    DynamicJsonDocument doc(512);
    doc["nodeId"] = node.nodeId;
    doc["shortName"] = node.shortName;
    doc["longName"] = node.longName;
    doc["macAddress"] = node.macAddress;
    doc["status"] = node.status;
    doc["batteryLevel"] = node.batteryLevel;
    doc["voltage"] = node.voltage;
    doc["uptimeSeconds"] = node.uptimeSeconds;
    
    if (node.hasPosition) {
        doc["latitude"] = node.latitude;
        doc["longitude"] = node.longitude;
        doc["altitude"] = node.altitude;
    }
    
    return serializeJson(doc.as<JsonObject>());
}

NodeInfo MeshInterface::deserializeNodeInfo(const std::vector<uint8_t>& data) {
    JsonObject nodeData = deserializeJson(data);
    
    NodeInfo node;
    node.nodeId = nodeData["nodeId"];
    node.shortName = nodeData["shortName"].as<String>();
    node.longName = nodeData["longName"].as<String>();
    node.macAddress = nodeData["macAddress"].as<String>();
    node.status = (NodeStatus)nodeData["status"].as<int>();
    node.batteryLevel = nodeData["batteryLevel"];
    node.voltage = nodeData["voltage"];
    node.uptimeSeconds = nodeData["uptimeSeconds"];
    node.lastSeen = millis();
    
    if (nodeData.containsKey("latitude")) {
        node.latitude = nodeData["latitude"];
        node.longitude = nodeData["longitude"];
        node.altitude = nodeData["altitude"];
        node.hasPosition = true;
    }
    
    return node;
}

// ===========================
// ENCRYPTION (SIMPLIFIED)
// ===========================

bool MeshInterface::encryptPacket(MeshPacket& packet) {
    // Simplified encryption - real implementation would use AES
    // For now, just mark as encrypted
    packet.encrypted = true;
    return true;
}

bool MeshInterface::decryptPacket(MeshPacket& packet) {
    // Simplified decryption - real implementation would use AES
    // For now, just mark as decrypted
    packet.encrypted = false;
    return true;
}

// ===========================
// CALLBACK MANAGEMENT
// ===========================

void MeshInterface::setMessageCallback(MessageCallback callback) {
    messageCallback_ = callback;
}

void MeshInterface::setNodeDiscoveredCallback(NodeDiscoveredCallback callback) {
    nodeDiscoveredCallback_ = callback;
}

void MeshInterface::setRouteChangedCallback(RouteChangedCallback callback) {
    routeChangedCallback_ = callback;
}

void MeshInterface::setNetworkStatusCallback(NetworkStatusCallback callback) {
    networkStatusCallback_ = callback;
}

// ===========================
// STATISTICS
// ===========================

MeshStatistics MeshInterface::getStatistics() const {
    return statistics_;
}

void MeshInterface::resetStatistics() {
    memset(&statistics_, 0, sizeof(statistics_));
}

// ===========================
// RADIO CALLBACK
// ===========================

void MeshInterface::onRadioReceive(LoRaPacket packet) {
    if (instance_) {
        instance_->processIncomingPacket(packet);
    }
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

std::vector<uint8_t> serializeJson(const JsonObject& json) {
    String jsonString;
    serializeJson(json, jsonString);
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

JsonObject deserializeJson(const std::vector<uint8_t>& data) {
    static DynamicJsonDocument doc(1024);
    doc.clear();
    
    String jsonString(data.begin(), data.end());
    deserializeJson(doc, jsonString);
    
    return doc.as<JsonObject>();
}

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

String formatNodeId(uint32_t nodeId) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%08X", nodeId);
    return String(buffer);
}

MeshInterface* createMeshInterface(const LoRaConfig& radioConfig) {
    MeshInterface* mesh = new MeshInterface();
    if (mesh->init(radioConfig)) {
        return mesh;
    } else {
        delete mesh;
        return nullptr;
    }
}