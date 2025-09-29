/**
 * @file mesh_node.cpp
 * @brief Individual Mesh Node Management for Wildlife Monitoring
 * 
 * Implements individual node functionality for the mesh network including
 * discovery, routing participation, and wildlife-specific operations.
 */

#include "mesh_node.h"
#include "../../include/network/mesh_config.h"
#include <Arduino.h>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

MeshNode::MeshNode() 
    : nodeId_(0)
    , role_(MESH_ROLE_NODE)
    , coordinatorId_(0)
    , isConnected_(false)
    , lastBeacon_(0)
    , lastMaintenance_(0)
    , wildlifeDetectionActive_(false)
    , batteryLevel_(1.0)
{
    memset(&statistics_, 0, sizeof(statistics_));
    memset(&config_, 0, sizeof(config_));
    neighbors_.clear();
    routingTable_.clear();
}

MeshNode::~MeshNode() {
    cleanup();
}

bool MeshNode::initialize(uint32_t nodeId, const MeshConfig& config) {
    nodeId_ = nodeId;
    config_ = config;
    
    // Initialize routing table
    routingTable_.clear();
    neighbors_.clear();
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.packetsTransmitted = 0;
    statistics_.packetsReceived = 0;
    statistics_.wildlifeDetections = 0;
    
    // Set initial timestamps
    lastBeacon_ = millis();
    lastMaintenance_ = millis();
    
    Serial.printf("Mesh Node initialized - Node ID: %08X, Role: %d\n", nodeId_, config_.role);
    return true;
}

void MeshNode::cleanup() {
    neighbors_.clear();
    routingTable_.clear();
    pendingPackets_.clear();
    isConnected_ = false;
}

// ===========================
// MAIN PROCESSING
// ===========================

void MeshNode::processNode() {
    if (!isInitialized()) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Send periodic beacon
    if (currentTime - lastBeacon_ > config_.beaconInterval) {
        sendBeacon();
        lastBeacon_ = currentTime;
    }
    
    // Process pending packets
    processPendingPackets();
    
    // Perform maintenance
    if (currentTime - lastMaintenance_ > 60000) {  // Every minute
        performMaintenance();
        lastMaintenance_ = currentTime;
    }
    
    // Handle wildlife detection if active
    if (wildlifeDetectionActive_) {
        processWildlifeDetection();
    }
    
    // Update statistics
    updateStatistics();
}

void MeshNode::performMaintenance() {
    // Clean up expired neighbors
    cleanupExpiredNeighbors();
    
    // Clean up routing table
    cleanupRoutingTable();
    
    // Clean up pending packets
    cleanupPendingPackets();
    
    // Update network connectivity status
    updateConnectivityStatus();
    
    // Report status if needed
    if (shouldReportStatus()) {
        reportNodeStatus();
    }
}

// ===========================
// NETWORK DISCOVERY
// ===========================

void MeshNode::sendBeacon() {
    BeaconPacket beacon;
    beacon.nodeId = nodeId_;
    beacon.role = config_.role;
    beacon.batteryLevel = batteryLevel_;
    beacon.wildlifeActive = wildlifeDetectionActive_;
    beacon.timestamp = millis();
    beacon.signalStrength = getCurrentSignalStrength();
    
    // Broadcast beacon to discover neighbors
    broadcastPacket(MESH_PACKET_BEACON, (uint8_t*)&beacon, sizeof(beacon));
    
    statistics_.beaconsSent++;
}

void MeshNode::handleBeaconReceived(const BeaconPacket& beacon, int16_t rssi) {
    // Add or update neighbor
    NeighborInfo neighbor;
    neighbor.nodeId = beacon.nodeId;
    neighbor.role = beacon.role;
    neighbor.signalStrength = rssi;
    neighbor.lastSeen = millis();
    neighbor.batteryLevel = beacon.batteryLevel;
    neighbor.wildlifeActive = beacon.wildlifeActive;
    neighbor.isActive = true;
    
    addOrUpdateNeighbor(neighbor);
    
    // Update coordinator information
    if (beacon.role == MESH_ROLE_COORDINATOR) {
        coordinatorId_ = beacon.nodeId;
        isConnected_ = true;
    }
    
    statistics_.beaconsReceived++;
}

void MeshNode::addOrUpdateNeighbor(const NeighborInfo& neighbor) {
    // Check if neighbor already exists
    for (auto& existing : neighbors_) {
        if (existing.nodeId == neighbor.nodeId) {
            // Update existing neighbor
            existing.signalStrength = neighbor.signalStrength;
            existing.lastSeen = neighbor.lastSeen;
            existing.batteryLevel = neighbor.batteryLevel;
            existing.wildlifeActive = neighbor.wildlifeActive;
            existing.isActive = true;
            return;
        }
    }
    
    // Add new neighbor
    neighbors_.push_back(neighbor);
    Serial.printf("Added new neighbor: %08X (RSSI: %d)\n", neighbor.nodeId, neighbor.signalStrength);
    
    // Update routing table
    updateRoutingTable();
}

// ===========================
// PACKET ROUTING
// ===========================

bool MeshNode::routePacket(const MeshPacket& packet) {
    // Check if packet is for this node
    if (packet.destination == nodeId_) {
        handleLocalPacket(packet);
        return true;
    }
    
    // Find next hop for destination
    uint32_t nextHop = findNextHop(packet.destination);
    if (nextHop == 0) {
        Serial.printf("No route found for destination %08X\n", packet.destination);
        statistics_.routingFailures++;
        return false;
    }
    
    // Check hop limit
    if (packet.hopCount >= MESH_MAX_HOPS) {
        Serial.println("Packet exceeded maximum hop count");
        statistics_.packetsDropped++;
        return false;
    }
    
    // Forward packet
    return forwardPacket(packet, nextHop);
}

uint32_t MeshNode::findNextHop(uint32_t destination) {
    // Check if destination is a direct neighbor
    for (const auto& neighbor : neighbors_) {
        if (neighbor.nodeId == destination && neighbor.isActive) {
            return destination;  // Direct route
        }
    }
    
    // Look up in routing table
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end()) {
        return it->second.nextHop;
    }
    
    // No route found
    return 0;
}

bool MeshNode::forwardPacket(MeshPacket packet, uint32_t nextHop) {
    // Update packet hop count
    packet.hopCount++;
    packet.lastHop = nodeId_;
    
    // Find neighbor for next hop
    for (const auto& neighbor : neighbors_) {
        if (neighbor.nodeId == nextHop && neighbor.isActive) {
            // Send packet to next hop
            bool success = transmitToNeighbor(packet, neighbor);
            if (success) {
                statistics_.packetsForwarded++;
                updateRouteStatistics(nextHop, true);
            } else {
                statistics_.transmissionFailures++;
                updateRouteStatistics(nextHop, false);
            }
            return success;
        }
    }
    
    return false;
}

// ===========================
// WILDLIFE-SPECIFIC OPERATIONS
// ===========================

void MeshNode::reportWildlifeDetection(const WildlifeDetection& detection) {
    wildlifeDetectionActive_ = true;
    statistics_.wildlifeDetections++;
    
    // Create high-priority packet for wildlife detection
    WildlifePacket packet;
    packet.nodeId = nodeId_;
    packet.timestamp = millis();
    packet.detection = detection;
    packet.priority = MESH_PRIORITY_HIGH;
    
    // Send to coordinator with high priority
    sendToCoordinator(MESH_PACKET_IMAGE, (uint8_t*)&packet, sizeof(packet), MESH_PRIORITY_HIGH);
    
    Serial.printf("Wildlife detection reported: Species %d, Confidence %.2f\n", 
                 detection.speciesId, detection.confidence);
}

void MeshNode::transmitWildlifeImage(const uint8_t* imageData, size_t imageSize) {
    if (!isConnected_) {
        Serial.println("Cannot transmit image - not connected to mesh network");
        return;
    }
    
    // Split image into chunks for transmission
    size_t chunkSize = MESH_WILDLIFE_IMAGE_CHUNK_SIZE;
    size_t totalChunks = (imageSize + chunkSize - 1) / chunkSize;
    
    for (size_t i = 0; i < totalChunks; i++) {
        ImageChunkPacket chunk;
        chunk.nodeId = nodeId_;
        chunk.chunkId = i;
        chunk.totalChunks = totalChunks;
        chunk.chunkSize = std::min(chunkSize, imageSize - (i * chunkSize));
        chunk.timestamp = millis();
        
        // Copy chunk data
        memcpy(chunk.data, imageData + (i * chunkSize), chunk.chunkSize);
        
        // Send with high priority
        sendToCoordinator(MESH_PACKET_IMAGE, (uint8_t*)&chunk, sizeof(chunk), MESH_PRIORITY_HIGH);
        
        // Small delay between chunks to avoid overwhelming network
        delay(10);
    }
    
    statistics_.imagesTransmitted++;
    Serial.printf("Transmitted wildlife image: %d bytes in %d chunks\n", imageSize, totalChunks);
}

void MeshNode::processWildlifeDetection() {
    // Check if wildlife detection is still active
    static unsigned long lastWildlifeCheck = 0;
    if (millis() - lastWildlifeCheck > 5000) {  // Check every 5 seconds
        // This would interface with actual wildlife detection system
        // For now, simulate detection timeout
        wildlifeDetectionActive_ = false;
        lastWildlifeCheck = millis();
    }
}

// ===========================
// POWER MANAGEMENT
// ===========================

void MeshNode::updateBatteryLevel(float level) {
    batteryLevel_ = level;
    
    // Enter low power mode if battery is low
    if (level < (MESH_LOW_POWER_THRESHOLD / 100.0)) {
        enterLowPowerMode();
    }
}

void MeshNode::enterLowPowerMode() {
    Serial.println("Entering low power mode");
    
    // Reduce beacon frequency
    config_.beaconInterval = config_.beaconInterval * 2;
    
    // Prioritize only critical packets
    priorityThreshold_ = MESH_PRIORITY_HIGH;
    
    // Reduce transmission power if possible
    reducePowerOutput();
}

void MeshNode::exitLowPowerMode() {
    Serial.println("Exiting low power mode");
    
    // Restore normal beacon frequency
    config_.beaconInterval = MESH_BEACON_INTERVAL;
    
    // Accept all priority packets
    priorityThreshold_ = MESH_PRIORITY_LOW;
    
    // Restore normal transmission power
    restoreNormalPowerOutput();
}

// ===========================
// UTILITY METHODS
// ===========================

bool MeshNode::isInitialized() const {
    return nodeId_ != 0;
}

MeshNodeStatistics MeshNode::getStatistics() const {
    return statistics_;
}

std::vector<NeighborInfo> MeshNode::getNeighbors() const {
    return neighbors_;
}

bool MeshNode::isConnectedToNetwork() const {
    return isConnected_ && coordinatorId_ != 0;
}

float MeshNode::getBatteryLevel() const {
    return batteryLevel_;
}

void MeshNode::updateStatistics() {
    statistics_.uptime = millis() - statistics_.startTime;
    statistics_.neighborCount = neighbors_.size();
    statistics_.routingTableSize = routingTable_.size();
    statistics_.isConnected = isConnected_;
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

void MeshNode::cleanupExpiredNeighbors() {
    auto it = neighbors_.begin();
    while (it != neighbors_.end()) {
        if (millis() - it->lastSeen > MESH_NODE_TIMEOUT) {
            Serial.printf("Removing expired neighbor %08X\n", it->nodeId);
            it = neighbors_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshNode::cleanupRoutingTable() {
    auto it = routingTable_.begin();
    while (it != routingTable_.end()) {
        if (millis() - it->second.lastUsed > MESH_ROUTE_TIMEOUT) {
            it = routingTable_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshNode::cleanupPendingPackets() {
    auto it = pendingPackets_.begin();
    while (it != pendingPackets_.end()) {
        if (millis() - it->timestamp > 30000) {  // 30 second timeout
            it = pendingPackets_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshNode::updateConnectivityStatus() {
    // Check if we still have connection to coordinator
    if (coordinatorId_ != 0) {
        bool coordinatorFound = false;
        for (const auto& neighbor : neighbors_) {
            if (neighbor.nodeId == coordinatorId_ && neighbor.isActive) {
                coordinatorFound = true;
                break;
            }
        }
        
        if (!coordinatorFound) {
            // Lost connection to coordinator
            isConnected_ = false;
            coordinatorId_ = 0;
            Serial.println("Lost connection to mesh coordinator");
        }
    }
}

void MeshNode::updateRoutingTable() {
    // Simple routing table update - would implement more sophisticated algorithm
    for (const auto& neighbor : neighbors_) {
        if (neighbor.isActive) {
            RouteInfo route;
            route.destination = neighbor.nodeId;
            route.nextHop = neighbor.nodeId;
            route.hopCount = 1;
            route.lastUsed = millis();
            route.reliability = 1.0;
            
            routingTable_[neighbor.nodeId] = route;
        }
    }
}

bool MeshNode::shouldReportStatus() {
    static unsigned long lastReport = 0;
    return (millis() - lastReport > 300000);  // Report every 5 minutes
}

void MeshNode::reportNodeStatus() {
    if (!isConnected_) return;
    
    NodeStatusPacket status;
    status.nodeId = nodeId_;
    status.batteryLevel = batteryLevel_;
    status.neighborCount = neighbors_.size();
    status.wildlifeActive = wildlifeDetectionActive_;
    status.timestamp = millis();
    
    sendToCoordinator(MESH_PACKET_TELEMETRY, (uint8_t*)&status, sizeof(status), MESH_PRIORITY_NORMAL);
}

int16_t MeshNode::getCurrentSignalStrength() {
    // Implementation would return actual signal strength
    return -50;  // Placeholder
}

void MeshNode::processPendingPackets() {
    // Process queued packets
    for (auto it = pendingPackets_.begin(); it != pendingPackets_.end();) {
        if (routePacket(*it)) {
            it = pendingPackets_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshNode::handleLocalPacket(const MeshPacket& packet) {
    // Handle packet destined for this node
    statistics_.packetsReceived++;
    
    switch (packet.type) {
        case MESH_PACKET_TELEMETRY:
            // Handle telemetry request
            break;
        case MESH_PACKET_ROUTING:
            // Handle routing update
            break;
        default:
            Serial.printf("Received packet type %d\n", packet.type);
            break;
    }
}

void MeshNode::updateRouteStatistics(uint32_t nextHop, bool success) {
    auto it = routingTable_.find(nextHop);
    if (it != routingTable_.end()) {
        if (success) {
            it->second.reliability = std::min(1.0f, it->second.reliability + 0.1f);
        } else {
            it->second.reliability = std::max(0.0f, it->second.reliability - 0.2f);
        }
        it->second.lastUsed = millis();
    }
}

bool MeshNode::transmitToNeighbor(const MeshPacket& packet, const NeighborInfo& neighbor) {
    // Implementation would transmit packet via radio
    Serial.printf("Transmitting packet to neighbor %08X\n", neighbor.nodeId);
    return true;  // Placeholder
}

bool MeshNode::broadcastPacket(MeshPacketType type, const uint8_t* data, size_t size) {
    // Implementation would broadcast packet
    Serial.printf("Broadcasting packet type %d\n", type);
    return true;  // Placeholder
}

bool MeshNode::sendToCoordinator(MeshPacketType type, const uint8_t* data, size_t size, MeshPriority priority) {
    if (!isConnected_) return false;
    
    // Implementation would send packet to coordinator
    Serial.printf("Sending packet type %d to coordinator %08X\n", type, coordinatorId_);
    return true;  // Placeholder
}

void MeshNode::reducePowerOutput() {
    Serial.println("Reducing transmission power for low power mode");
}

void MeshNode::restoreNormalPowerOutput() {
    Serial.println("Restoring normal transmission power");
}