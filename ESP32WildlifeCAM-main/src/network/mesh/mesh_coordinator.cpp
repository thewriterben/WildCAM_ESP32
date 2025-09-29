/**
 * @file mesh_coordinator.cpp
 * @brief Central Mesh Network Coordinator for Wildlife Monitoring
 * 
 * Implements the central coordination system for managing mesh network
 * topology, load balancing, and wildlife-specific optimizations.
 */

#include "mesh_coordinator.h"
#include "../../include/network/mesh_config.h"
#include <Arduino.h>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

MeshCoordinator::MeshCoordinator() 
    : nodeId_(0)
    , role_(MESH_ROLE_COORDINATOR)
    , networkHealth_(0.0)
    , totalNodes_(0)
    , lastTopologyUpdate_(0)
    , emergencyMode_(false)
{
    memset(&statistics_, 0, sizeof(statistics_));
    memset(&config_, 0, sizeof(config_));
}

MeshCoordinator::~MeshCoordinator() {
    cleanup();
}

bool MeshCoordinator::initialize(uint32_t nodeId, const MeshConfig& config) {
    nodeId_ = nodeId;
    config_ = config;
    config_.role = MESH_ROLE_COORDINATOR;
    
    // Initialize network topology
    topology_.clear();
    routingTable_.clear();
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.packetsRouted = 0;
    statistics_.nodesManaged = 0;
    statistics_.emergencyEvents = 0;
    
    // Set up periodic maintenance
    lastTopologyUpdate_ = millis();
    
    Serial.printf("Mesh Coordinator initialized - Node ID: %08X\n", nodeId_);
    return true;
}

void MeshCoordinator::cleanup() {
    topology_.clear();
    routingTable_.clear();
    activeConnections_.clear();
}

// ===========================
// NETWORK COORDINATION
// ===========================

void MeshCoordinator::processCoordination() {
    if (!isInitialized()) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Update network topology
    if (currentTime - lastTopologyUpdate_ > MESH_BEACON_INTERVAL) {
        updateNetworkTopology();
        lastTopologyUpdate_ = currentTime;
    }
    
    // Monitor network health
    calculateNetworkHealth();
    
    // Perform load balancing
    if (config_.loadBalancingEnabled) {
        performLoadBalancing();
    }
    
    // Handle emergency situations
    if (emergencyMode_) {
        handleEmergencyCoordination();
    }
    
    // Update statistics
    updateStatistics();
}

void MeshCoordinator::updateNetworkTopology() {
    // Remove expired nodes
    auto it = topology_.begin();
    while (it != topology_.end()) {
        if (millis() - it->lastSeen > MESH_NODE_TIMEOUT) {
            Serial.printf("Removing expired node %08X from topology\n", it->nodeId);
            it = topology_.erase(it);
        } else {
            ++it;
        }
    }
    
    totalNodes_ = topology_.size();
    
    // Recalculate optimal routes
    recalculateRoutes();
    
    // Broadcast topology updates to nodes
    broadcastTopologyUpdate();
}

void MeshCoordinator::calculateNetworkHealth() {
    if (topology_.empty()) {
        networkHealth_ = 0.0;
        return;
    }
    
    float totalHealth = 0.0;
    int healthyNodes = 0;
    
    for (const auto& node : topology_) {
        if (node.isActive && node.signalStrength > -80) {  // Good signal threshold
            totalHealth += 1.0;
            healthyNodes++;
        } else if (node.isActive) {
            totalHealth += 0.5;  // Marginal connection
        }
    }
    
    networkHealth_ = totalHealth / topology_.size();
    
    // Check for emergency conditions
    if (networkHealth_ < 0.3 && !emergencyMode_) {
        enterEmergencyMode();
    } else if (networkHealth_ > 0.7 && emergencyMode_) {
        exitEmergencyMode();
    }
}

// ===========================
// LOAD BALANCING
// ===========================

void MeshCoordinator::performLoadBalancing() {
    // Analyze current traffic patterns
    for (auto& route : routingTable_) {
        if (route.second.utilization > MESH_CONGESTION_THRESHOLD) {
            // Find alternative path
            uint32_t alternativePath = findAlternativePath(route.first, route.second.destination);
            if (alternativePath != 0) {
                redistributeTraffic(route.first, alternativePath);
            }
        }
    }
}

uint32_t MeshCoordinator::findAlternativePath(uint32_t source, uint32_t destination) {
    // Simplified path finding - would implement Dijkstra's algorithm
    for (const auto& node : topology_) {
        if (node.nodeId != source && node.isActive && 
            node.signalStrength > -70) {  // Good alternative path
            return node.nodeId;
        }
    }
    return 0;  // No alternative found
}

void MeshCoordinator::redistributeTraffic(uint32_t overloadedPath, uint32_t alternativePath) {
    Serial.printf("Redistributing traffic from %08X to %08X\n", overloadedPath, alternativePath);
    
    // Update routing table with new path
    RouteEntry newRoute;
    newRoute.destination = routingTable_[overloadedPath].destination;
    newRoute.nextHop = alternativePath;
    newRoute.hopCount = routingTable_[overloadedPath].hopCount + 1;
    newRoute.utilization = 0.1;  // Start with low utilization
    newRoute.lastUsed = millis();
    
    routingTable_[alternativePath] = newRoute;
    
    statistics_.routeOptimizations++;
}

// ===========================
// EMERGENCY HANDLING
// ===========================

void MeshCoordinator::enterEmergencyMode() {
    emergencyMode_ = true;
    statistics_.emergencyEvents++;
    
    Serial.println("MESH COORDINATOR: Entering emergency mode due to poor network health");
    
    // Reduce beacon intervals for faster recovery
    config_.beaconInterval = MESH_BEACON_INTERVAL / 2;
    
    // Increase transmission power if possible
    increasePowerOutput();
    
    // Prioritize critical wildlife detection packets
    adjustPriorityScheduling(true);
}

void MeshCoordinator::exitEmergencyMode() {
    emergencyMode_ = false;
    
    Serial.println("MESH COORDINATOR: Exiting emergency mode - network health restored");
    
    // Restore normal beacon intervals
    config_.beaconInterval = MESH_BEACON_INTERVAL;
    
    // Restore normal power output
    restoreNormalPowerOutput();
    
    // Restore normal priority scheduling
    adjustPriorityScheduling(false);
}

void MeshCoordinator::handleEmergencyCoordination() {
    // Prioritize critical nodes (those with wildlife detections)
    for (auto& node : topology_) {
        if (node.hasWildlifeDetection && !node.isActive) {
            // Try to re-establish connection
            attemptNodeReconnection(node.nodeId);
        }
    }
}

// ===========================
// NODE MANAGEMENT
// ===========================

bool MeshCoordinator::addNode(const NodeInfo& node) {
    // Check if node already exists
    for (auto& existingNode : topology_) {
        if (existingNode.nodeId == node.nodeId) {
            // Update existing node
            existingNode.signalStrength = node.signalStrength;
            existingNode.lastSeen = millis();
            existingNode.isActive = true;
            return true;
        }
    }
    
    // Add new node
    NodeInfo newNode = node;
    newNode.lastSeen = millis();
    newNode.isActive = true;
    topology_.push_back(newNode);
    
    Serial.printf("Added new node %08X to mesh network\n", node.nodeId);
    statistics_.nodesManaged++;
    
    // Recalculate routes with new node
    recalculateRoutes();
    
    return true;
}

bool MeshCoordinator::removeNode(uint32_t nodeId) {
    auto it = std::find_if(topology_.begin(), topology_.end(),
                          [nodeId](const NodeInfo& node) { return node.nodeId == nodeId; });
    
    if (it != topology_.end()) {
        Serial.printf("Removing node %08X from mesh network\n", nodeId);
        topology_.erase(it);
        
        // Remove associated routes
        removeRoutesForNode(nodeId);
        
        // Recalculate network topology
        recalculateRoutes();
        
        return true;
    }
    
    return false;
}

std::vector<NodeInfo> MeshCoordinator::getActiveNodes() const {
    std::vector<NodeInfo> activeNodes;
    
    for (const auto& node : topology_) {
        if (node.isActive) {
            activeNodes.push_back(node);
        }
    }
    
    return activeNodes;
}

// ===========================
// ROUTING MANAGEMENT
// ===========================

void MeshCoordinator::recalculateRoutes() {
    routingTable_.clear();
    
    // Simplified routing - would implement more sophisticated algorithm
    for (const auto& node : topology_) {
        if (!node.isActive) continue;
        
        RouteEntry route;
        route.destination = node.nodeId;
        route.nextHop = node.nodeId;  // Direct route for now
        route.hopCount = 1;
        route.utilization = 0.0;
        route.lastUsed = millis();
        
        routingTable_[node.nodeId] = route;
    }
}

void MeshCoordinator::removeRoutesForNode(uint32_t nodeId) {
    auto it = routingTable_.begin();
    while (it != routingTable_.end()) {
        if (it->second.nextHop == nodeId || it->second.destination == nodeId) {
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshCoordinator::broadcastTopologyUpdate() {
    // Create topology update packet
    TopologyUpdatePacket update;
    update.coordinatorId = nodeId_;
    update.timestamp = millis();
    update.networkHealth = networkHealth_;
    update.nodeCount = topology_.size();
    
    // Broadcast to all active nodes
    for (const auto& node : topology_) {
        if (node.isActive) {
            sendTopologyUpdate(node.nodeId, update);
        }
    }
}

// ===========================
// UTILITY METHODS
// ===========================

bool MeshCoordinator::isInitialized() const {
    return nodeId_ != 0 && role_ == MESH_ROLE_COORDINATOR;
}

MeshStatistics MeshCoordinator::getStatistics() const {
    return statistics_;
}

float MeshCoordinator::getNetworkHealth() const {
    return networkHealth_;
}

uint32_t MeshCoordinator::getTotalNodes() const {
    return totalNodes_;
}

void MeshCoordinator::updateStatistics() {
    statistics_.uptime = millis() - statistics_.startTime;
    statistics_.networkHealth = networkHealth_;
    statistics_.activeNodes = totalNodes_;
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

void MeshCoordinator::increasePowerOutput() {
    // Implementation would increase radio power output
    Serial.println("Increasing mesh transmission power for emergency mode");
}

void MeshCoordinator::restoreNormalPowerOutput() {
    // Implementation would restore normal radio power output
    Serial.println("Restoring normal mesh transmission power");
}

void MeshCoordinator::adjustPriorityScheduling(bool emergencyMode) {
    // Implementation would adjust packet priority scheduling
    if (emergencyMode) {
        Serial.println("Prioritizing wildlife detection packets in emergency mode");
    } else {
        Serial.println("Restoring normal packet priority scheduling");
    }
}

void MeshCoordinator::attemptNodeReconnection(uint32_t nodeId) {
    Serial.printf("Attempting to reconnect to critical node %08X\n", nodeId);
    // Implementation would send reconnection packets
}

void MeshCoordinator::sendTopologyUpdate(uint32_t nodeId, const TopologyUpdatePacket& update) {
    // Implementation would send topology update packet to specified node
    Serial.printf("Sending topology update to node %08X\n", nodeId);
}