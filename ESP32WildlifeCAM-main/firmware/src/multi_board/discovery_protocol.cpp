/**
 * @file discovery_protocol.cpp
 * @brief Implementation of board discovery and network topology management
 */

#include "discovery_protocol.h"
#include "../config.h"

DiscoveryProtocol::DiscoveryProtocol() :
    nodeId_(0),
    preferredRole_(ROLE_NODE),
    state_(DISCOVERY_IDLE),
    initialized_(false),
    discoveryInterval_(DEFAULT_DISCOVERY_INTERVAL),
    advertisementInterval_(DEFAULT_ADVERTISEMENT_INTERVAL),
    nodeTimeout_(DEFAULT_NODE_TIMEOUT),
    lastDiscovery_(0),
    lastAdvertisement_(0),
    lastCleanup_(0) {
    
    topology_.coordinatorNode = -1;
    topology_.lastUpdate = 0;
    topology_.isStable = false;
}

DiscoveryProtocol::~DiscoveryProtocol() {
    stopDiscovery();
}

bool DiscoveryProtocol::init(int nodeId, BoardRole preferredRole) {
    nodeId_ = nodeId;
    preferredRole_ = preferredRole;
    capabilities_ = MessageProtocol::getCurrentCapabilities();
    
    // Initialize message protocol if not already done
    if (!MessageProtocol::init(nodeId, capabilities_.boardType, capabilities_.sensorType)) {
        Serial.println("Failed to initialize message protocol");
        return false;
    }
    
    state_ = DISCOVERY_IDLE;
    initialized_ = true;
    
    Serial.printf("Discovery protocol initialized: Node %d, Preferred role: %s\n",
                  nodeId, MessageProtocol::roleToString(preferredRole));
    
    return true;
}

bool DiscoveryProtocol::startDiscovery() {
    if (!initialized_) {
        Serial.println("Discovery protocol not initialized");
        return false;
    }
    
    state_ = DISCOVERY_SCANNING;
    lastDiscovery_ = millis();
    lastAdvertisement_ = millis();
    
    Serial.println("Starting network discovery...");
    
    // Send initial discovery advertisement
    sendDiscoveryAdvertisement();
    
    return true;
}

void DiscoveryProtocol::stopDiscovery() {
    state_ = DISCOVERY_IDLE;
    Serial.println("Discovery protocol stopped");
}

void DiscoveryProtocol::processMessages() {
    if (!initialized_ || state_ == DISCOVERY_IDLE) {
        return;
    }
    
    unsigned long now = millis();
    
    // Send periodic discovery advertisements
    if (now - lastAdvertisement_ >= advertisementInterval_) {
        sendDiscoveryAdvertisement();
        lastAdvertisement_ = now;
    }
    
    // Cleanup inactive nodes
    if (now - lastCleanup_ >= CLEANUP_INTERVAL) {
        cleanupInactiveNodes();
        lastCleanup_ = now;
    }
    
    // Check if discovery is complete
    if (state_ == DISCOVERY_SCANNING) {
        if (topology_.nodes.size() > 0 && 
            (now - lastDiscovery_) > discoveryInterval_ * 2) {
            state_ = DISCOVERY_COMPLETE;
            topology_.isStable = true;
            Serial.printf("Discovery complete: Found %d nodes\n", topology_.nodes.size());
            
            // Send topology update to network
            sendTopologyUpdate();
        }
    }
}

void DiscoveryProtocol::handleDiscoveryMessage(const MultiboardMessage& msg) {
    DiscoveryMessage discovery;
    if (!MessageProtocol::parseDiscoveryMessage(msg.data, discovery)) {
        Serial.println("Failed to parse discovery message");
        return;
    }
    
    Serial.printf("Received discovery from node %d (role: %s)\n",
                  discovery.nodeId, MessageProtocol::roleToString(discovery.preferredRole));
    
    updateTopology(discovery);
    lastDiscovery_ = millis();
}

void DiscoveryProtocol::handleTopologyMessage(const MultiboardMessage& msg) {
    JsonObject data = msg.data;
    int nodeCount = data["node_count"];
    
    Serial.printf("Received topology update from node %d: %d nodes\n",
                  msg.sourceNode, nodeCount);
    
    // Update our topology with received information
    JsonArray nodeArray = data["nodes"];
    for (JsonVariant nodeVar : nodeArray) {
        JsonObject nodeObj = nodeVar.as<JsonObject>();
        
        NetworkNode node;
        node.nodeId = nodeObj["node_id"];
        node.role = static_cast<BoardRole>(nodeObj["role"].as<int>());
        node.signalStrength = nodeObj["signal_strength"];
        node.hopCount = nodeObj["hop_count"];
        node.lastSeen = nodeObj["last_seen"];
        node.isActive = nodeObj["is_active"];
        node.coordinatorScore = nodeObj["coordinator_score"];
        
        updateNode(node);
    }
    
    topology_.lastUpdate = millis();
}

bool DiscoveryProtocol::sendDiscoveryAdvertisement() {
    String message = MessageProtocol::createDiscoveryMessage(capabilities_, preferredRole_);
    
    // Send via LoRa mesh
    if (LoraMesh::queueMessage(message)) {
        Serial.printf("Sent discovery advertisement (role: %s)\n",
                      MessageProtocol::roleToString(preferredRole_));
        return true;
    }
    
    return false;
}

bool DiscoveryProtocol::sendTopologyUpdate() {
    String message = MessageProtocol::createTopologyMessage(topology_.nodes);
    
    if (LoraMesh::queueMessage(message)) {
        Serial.printf("Sent topology update (%d nodes)\n", topology_.nodes.size());
        return true;
    }
    
    return false;
}

const NetworkTopology& DiscoveryProtocol::getNetworkTopology() const {
    return topology_;
}

const std::vector<NetworkNode>& DiscoveryProtocol::getDiscoveredNodes() const {
    return topology_.nodes;
}

int DiscoveryProtocol::getCoordinatorNode() const {
    return topology_.coordinatorNode;
}

bool DiscoveryProtocol::isDiscoveryComplete() const {
    return state_ == DISCOVERY_COMPLETE;
}

DiscoveryState DiscoveryProtocol::getDiscoveryState() const {
    return state_;
}

void DiscoveryProtocol::updateNode(const NetworkNode& node) {
    // Find existing node or add new one
    NetworkNode* existingNode = findNode(node.nodeId);
    
    if (existingNode) {
        // Update existing node
        existingNode->role = node.role;
        existingNode->signalStrength = node.signalStrength;
        existingNode->hopCount = node.hopCount;
        existingNode->lastSeen = millis();
        existingNode->isActive = true;
        existingNode->coordinatorScore = node.coordinatorScore;
    } else {
        // Add new node
        NetworkNode newNode = node;
        newNode.lastSeen = millis();
        newNode.isActive = true;
        topology_.nodes.push_back(newNode);
        
        Serial.printf("Added new node %d to topology\n", node.nodeId);
    }
    
    // Update coordinator selection
    selectCoordinator();
}

void DiscoveryProtocol::cleanupInactiveNodes() {
    unsigned long now = millis();
    auto it = topology_.nodes.begin();
    
    while (it != topology_.nodes.end()) {
        if (!isNodeActive(*it)) {
            Serial.printf("Removing inactive node %d from topology\n", it->nodeId);
            it = topology_.nodes.erase(it);
        } else {
            ++it;
        }
    }
    
    // Update coordinator selection after cleanup
    selectCoordinator();
}

NetworkNode* DiscoveryProtocol::findNode(int nodeId) {
    for (auto& node : topology_.nodes) {
        if (node.nodeId == nodeId) {
            return &node;
        }
    }
    return nullptr;
}

bool DiscoveryProtocol::nodeExists(int nodeId) const {
    return const_cast<DiscoveryProtocol*>(this)->findNode(nodeId) != nullptr;
}

DiscoveryProtocol::NetworkStats DiscoveryProtocol::getNetworkStats() const {
    NetworkStats stats = {};
    
    stats.totalNodes = topology_.nodes.size();
    stats.networkAge = millis() - topology_.lastUpdate;
    
    int totalSignal = 0;
    int maxHops = 0;
    
    for (const auto& node : topology_.nodes) {
        if (node.isActive) {
            stats.activeNodes++;
            totalSignal += node.signalStrength;
            if (node.hopCount > maxHops) {
                maxHops = node.hopCount;
            }
        }
    }
    
    stats.averageSignalStrength = stats.activeNodes > 0 ? totalSignal / stats.activeNodes : 0;
    stats.maxHopCount = maxHops;
    
    return stats;
}

void DiscoveryProtocol::setDiscoveryInterval(unsigned long interval) {
    discoveryInterval_ = interval;
}

void DiscoveryProtocol::setAdvertisementInterval(unsigned long interval) {
    advertisementInterval_ = interval;
}

void DiscoveryProtocol::setNodeTimeout(unsigned long timeout) {
    nodeTimeout_ = timeout;
}

void DiscoveryProtocol::updateTopology(const DiscoveryMessage& discovery) {
    NetworkNode node;
    node.nodeId = discovery.nodeId;
    node.role = discovery.currentRole;
    node.capabilities = discovery.capabilities;
    node.signalStrength = 0; // TODO: Get actual signal strength
    node.hopCount = 0; // TODO: Calculate hop count
    node.coordinatorScore = discovery.coordinatorScore;
    
    updateNode(node);
}

bool DiscoveryProtocol::isNodeActive(const NetworkNode& node) const {
    return (millis() - node.lastSeen) < nodeTimeout_;
}

void DiscoveryProtocol::selectCoordinator() {
    if (topology_.nodes.empty()) {
        topology_.coordinatorNode = -1;
        return;
    }
    
    // Find node with highest coordinator score
    float bestScore = -1.0;
    int bestNode = -1;
    
    for (const auto& node : topology_.nodes) {
        if (node.isActive && node.coordinatorScore > bestScore) {
            bestScore = node.coordinatorScore;
            bestNode = node.nodeId;
        }
    }
    
    // Include our own node in coordinator selection
    float ourScore = MessageProtocol::calculateCoordinatorScore(capabilities_);
    if (ourScore > bestScore) {
        bestNode = nodeId_;
    }
    
    if (bestNode != topology_.coordinatorNode) {
        topology_.coordinatorNode = bestNode;
        Serial.printf("Coordinator selected: Node %d (score: %.1f)\n", 
                      bestNode, bestScore);
    }
}

float DiscoveryProtocol::calculateNodeScore(const NetworkNode& node) const {
    return node.coordinatorScore;
}