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
    
    // Cleanup inactive nodes and detect topology changes
    if (now - lastCleanup_ >= CLEANUP_INTERVAL) {
        int nodeCountBefore = topology_.nodes.size();
        cleanupInactiveNodes();
        int nodeCountAfter = topology_.nodes.size();
        
        // If topology changed, mark as unstable and broadcast update
        if (nodeCountBefore != nodeCountAfter) {
            topology_.isStable = false;
            topology_.lastUpdate = now;
            Serial.printf("Topology changed: %d -> %d nodes, broadcasting update\n", 
                         nodeCountBefore, nodeCountAfter);
            sendTopologyUpdate();
        }
        
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
    } else if (state_ == DISCOVERY_COMPLETE) {
        // Continue monitoring for new devices even after discovery is complete
        // This enables real-time mesh formation when new devices join
        if ((now - topology_.lastUpdate) > advertisementInterval_ * 3) {
            // Re-check topology stability periodically
            topology_.isStable = true;
            for (const auto& node : topology_.nodes) {
                if (!isNodeActive(node)) {
                    topology_.isStable = false;
                    break;
                }
            }
        }
    }
}

void DiscoveryProtocol::handleDiscoveryMessage(const MultiboardMessage& msg) {
    DiscoveryMessage discovery;
    if (!MessageProtocol::parseDiscoveryMessage(msg.data, discovery)) {
        Serial.println("Failed to parse discovery message");
        return;
    }
    
    Serial.printf("Received discovery from node %d (role: %s, hop count: %d, RSSI: %d)\n",
                  discovery.nodeId, MessageProtocol::roleToString(discovery.preferredRole),
                  msg.hopCount, LoraMesh::getSignalQuality().rssi);
    
    updateTopology(discovery, msg.hopCount);
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
    bool isNewNode = (existingNode == nullptr);
    
    if (existingNode) {
        // Update existing node
        existingNode->role = node.role;
        existingNode->capabilities = node.capabilities;
        existingNode->signalStrength = node.signalStrength;
        existingNode->hopCount = node.hopCount;
        existingNode->lastSeen = millis();
        existingNode->isActive = true;
        existingNode->coordinatorScore = node.coordinatorScore;
    } else {
        // Add new node - automatic device joining
        NetworkNode newNode = node;
        newNode.lastSeen = millis();
        newNode.isActive = true;
        topology_.nodes.push_back(newNode);
        
        // Mark topology as unstable when new device joins
        topology_.isStable = false;
        topology_.lastUpdate = millis();
        
        Serial.printf("✓ New device joined network: Node %d (role: %s, score: %.1f)\n", 
                     node.nodeId, MessageProtocol::roleToString(node.role), 
                     node.coordinatorScore);
        
        // Broadcast topology update to inform all nodes
        sendTopologyUpdate();
    }
    
    // Update coordinator selection
    selectCoordinator();
    
    // If new node joined and we're in SCANNING state, reset discovery timer
    // to allow more time for other nodes to respond
    if (isNewNode && state_ == DISCOVERY_SCANNING) {
        lastDiscovery_ = millis();
    }
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

void DiscoveryProtocol::updateTopology(const DiscoveryMessage& discovery, int hopCount) {
    NetworkNode node;
    node.nodeId = discovery.nodeId;
    node.role = discovery.currentRole;
    node.capabilities = discovery.capabilities;
    
    // Get actual signal strength from LoRa mesh network
    SignalQuality signalQuality = LoraMesh::getSignalQuality();
    node.signalStrength = signalQuality.rssi;
    
    // Calculate hop count from message routing
    // The hop count tells us how many intermediate nodes the message passed through
    node.hopCount = hopCount;
    
    node.coordinatorScore = discovery.coordinatorScore;
    
    updateNode(node);
}

bool DiscoveryProtocol::isNodeActive(const NetworkNode& node) const {
    return (millis() - node.lastSeen) < nodeTimeout_;
}

void DiscoveryProtocol::selectCoordinator() {
    if (topology_.nodes.empty() && nodeId_ == 0) {
        topology_.coordinatorNode = -1;
        return;
    }
    
    // Find node with highest coordinator score
    float bestScore = -1.0;
    int bestNode = -1;
    
    // Check all active nodes in topology
    for (const auto& node : topology_.nodes) {
        if (node.isActive && node.coordinatorScore > bestScore) {
            bestScore = node.coordinatorScore;
            bestNode = node.nodeId;
        }
    }
    
    // Include our own node in coordinator selection
    float ourScore = MessageProtocol::calculateCoordinatorScore(capabilities_);
    if (ourScore > bestScore || (topology_.nodes.empty() && nodeId_ > 0)) {
        bestScore = ourScore;
        bestNode = nodeId_;
    }
    
    // Dynamic coordinator assignment with role transition
    if (bestNode != topology_.coordinatorNode) {
        int previousCoordinator = topology_.coordinatorNode;
        topology_.coordinatorNode = bestNode;
        topology_.lastUpdate = millis();
        
        Serial.printf("⚡ Coordinator change: Node %d -> Node %d (score: %.1f)\n", 
                      previousCoordinator, bestNode, bestScore);
        
        // If we became coordinator, announce to network
        if (bestNode == nodeId_) {
            Serial.println("✓ This node is now coordinator");
        }
        
        // Mark topology as updated to trigger coordination
        topology_.isStable = false;
    }
}

float DiscoveryProtocol::calculateNodeScore(const NetworkNode& node) const {
    return node.coordinatorScore;
}

float DiscoveryProtocol::getNetworkHealth() const {
    if (topology_.nodes.empty()) {
        return 0.0;
    }
    
    int activeNodes = 0;
    int totalSignalStrength = 0;
    int totalHopCount = 0;
    
    for (const auto& node : topology_.nodes) {
        if (isNodeActive(node)) {
            activeNodes++;
            totalSignalStrength += node.signalStrength;
            totalHopCount += node.hopCount;
        }
    }
    
    if (activeNodes == 0) {
        return 0.0;
    }
    
    // Calculate health based on:
    // 1. Percentage of active nodes (0-40%)
    // 2. Average signal strength (0-40%)
    // 3. Network hop efficiency (0-20%)
    
    float activeNodeScore = (float)activeNodes / topology_.nodes.size() * 0.4;
    
    int avgSignalStrength = totalSignalStrength / activeNodes;
    // RSSI typically ranges from -120 (worst) to -30 (best)
    float signalScore = ((avgSignalStrength + 120.0) / 90.0) * 0.4;
    signalScore = constrain(signalScore, 0.0, 0.4);
    
    float avgHopCount = (float)totalHopCount / activeNodes;
    // Lower hop count is better (max reasonable hops = 5)
    float hopScore = (1.0 - (avgHopCount / 5.0)) * 0.2;
    hopScore = constrain(hopScore, 0.0, 0.2);
    
    float totalHealth = activeNodeScore + signalScore + hopScore;
    return constrain(totalHealth, 0.0, 1.0);
}

void DiscoveryProtocol::broadcastTopologyNow() {
    if (!initialized_) {
        return;
    }
    
    Serial.println("⚡ Broadcasting topology update (forced)");
    sendTopologyUpdate();
    topology_.lastUpdate = millis();
}