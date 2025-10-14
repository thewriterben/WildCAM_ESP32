/**
 * @file network_topology_manager.cpp
 * @brief Implementation of Network Topology Manager
 * 
 * Manages network topology for federated learning, supporting star,
 * mesh, and hybrid configurations with dynamic adaptation.
 */

#include "network_topology_manager.h"
#include "../../utils/logger.h"
#include <algorithm>
#include <queue>
#include <limits>

// Global instance
NetworkTopologyManager* g_networkTopologyManager = nullptr;

// ===========================
// CONSTRUCTOR / DESTRUCTOR
// ===========================

NetworkTopologyManager::NetworkTopologyManager()
    : initialized_(false)
    , currentTopology_(NetworkTopology::STAR)
    , discoveryActive_(false)
    , lastDiscovery_(0)
    , lastRouteUpdate_(0)
{
    localNodeId_ = "node_" + String(ESP.getEfuseMac(), HEX);
}

NetworkTopologyManager::~NetworkTopologyManager() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool NetworkTopologyManager::init(const TopologyConfig& config) {
    if (initialized_) {
        Logger::warn("[NetworkTopology] Already initialized");
        return true;
    }

    Logger::info("[NetworkTopology] Initializing topology manager...");

    config_ = config;
    currentTopology_ = config.preferredTopology;

    // Initialize local node
    NetworkNode localNode;
    localNode.nodeId = localNodeId_;
    localNode.type = NodeType::CLIENT;
    localNode.status = ConnectionStatus::CONNECTED;
    localNode.reliability = 1.0f;
    localNode.isReachable = true;
    
    nodes_[localNodeId_] = localNode;
    connectedNodes_.insert(localNodeId_);

    // Initialize protocol
    updateProtocol_ = std::make_unique<ModelUpdateProtocol>();
    NetworkConfig netConfig;
    netConfig.enableEncryption = true;
    if (!updateProtocol_->init(netConfig)) {
        Logger::error("[NetworkTopology] Failed to initialize protocol");
        return false;
    }

    // Initialize metrics
    metrics_ = NetworkMetrics();
    metrics_.totalNodes = 1;
    metrics_.connectedNodes = 1;

    initialized_ = true;

    Logger::info("[NetworkTopology] Initialization complete, topology: %d", 
                static_cast<int>(currentTopology_));
    return true;
}

void NetworkTopologyManager::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[NetworkTopology] Cleaning up topology manager...");

    // Stop discovery
    stopNodeDiscovery();

    // Clear all data structures
    nodes_.clear();
    connectedNodes_.clear();
    nodeLoads_.clear();
    routes_.clear();
    routingTable_.clear();

    // Cleanup protocol
    if (updateProtocol_) {
        updateProtocol_->cleanup();
        updateProtocol_.reset();
    }

    initialized_ = false;

    Logger::info("[NetworkTopology] Cleanup complete");
}

// ===========================
// CONFIGURATION
// ===========================

void NetworkTopologyManager::setTopologyConfig(const TopologyConfig& config) {
    config_ = config;
    Logger::info("[NetworkTopology] Configuration updated");
}

void NetworkTopologyManager::setPreferredTopology(NetworkTopology topology) {
    if (topology == currentTopology_) {
        return;
    }

    Logger::info("[NetworkTopology] Changing topology from %d to %d",
                static_cast<int>(currentTopology_), static_cast<int>(topology));

    NetworkTopology oldTopology = currentTopology_;
    currentTopology_ = topology;

    // Reconfigure network
    reconfigureTopology(topology);

    // Notify callback
    if (topologyChangeCallback_) {
        topologyChangeCallback_(oldTopology, topology);
    }
}

// ===========================
// NODE MANAGEMENT
// ===========================

bool NetworkTopologyManager::addNode(const NetworkNode& node) {
    if (!validateNodeId(node.nodeId)) {
        Logger::error("[NetworkTopology] Invalid node ID");
        return false;
    }

    Logger::info("[NetworkTopology] Adding node: %s", node.nodeId.c_str());

    nodes_[node.nodeId] = node;
    metrics_.totalNodes++;

    if (node.isReachable) {
        connectedNodes_.insert(node.nodeId);
        metrics_.connectedNodes++;
    }

    // Notify callback
    if (nodeJoinCallback_) {
        nodeJoinCallback_(node);
    }

    // Recalculate routes
    calculateRoutes();

    return true;
}

bool NetworkTopologyManager::removeNode(const String& nodeId) {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        Logger::warn("[NetworkTopology] Node not found: %s", nodeId.c_str());
        return false;
    }

    Logger::info("[NetworkTopology] Removing node: %s", nodeId.c_str());

    // Notify callback
    if (nodeLeaveCallback_) {
        nodeLeaveCallback_(nodeId);
    }

    // Remove from connected nodes
    connectedNodes_.erase(nodeId);
    if (metrics_.connectedNodes > 0) {
        metrics_.connectedNodes--;
    }

    // Remove from nodes
    nodes_.erase(it);
    if (metrics_.totalNodes > 0) {
        metrics_.totalNodes--;
    }

    // Clear node load
    nodeLoads_.erase(nodeId);

    // Invalidate routes involving this node
    for (auto& pair : nodes_) {
        invalidateRoute(nodeId, pair.first);
        invalidateRoute(pair.first, nodeId);
    }

    // If this was the coordinator, elect a new one
    if (nodeId == currentCoordinator_) {
        electNewCoordinator();
    }

    return true;
}

bool NetworkTopologyManager::updateNode(const NetworkNode& node) {
    auto it = nodes_.find(node.nodeId);
    if (it == nodes_.end()) {
        return addNode(node);
    }

    Logger::debug("[NetworkTopology] Updating node: %s", node.nodeId.c_str());

    bool wasReachable = it->second.isReachable;
    it->second = node;

    // Update connectivity status
    if (node.isReachable && !wasReachable) {
        connectedNodes_.insert(node.nodeId);
        metrics_.connectedNodes++;
    } else if (!node.isReachable && wasReachable) {
        connectedNodes_.erase(node.nodeId);
        if (metrics_.connectedNodes > 0) {
            metrics_.connectedNodes--;
        }
    }

    return true;
}

NetworkNode NetworkTopologyManager::getNode(const String& nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        return it->second;
    }
    return NetworkNode();
}

std::vector<NetworkNode> NetworkTopologyManager::getAllNodes() const {
    std::vector<NetworkNode> result;
    for (const auto& pair : nodes_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<NetworkNode> NetworkTopologyManager::getNodesOfType(NodeType type) const {
    std::vector<NetworkNode> result;
    for (const auto& pair : nodes_) {
        if (pair.second.type == type) {
            result.push_back(pair.second);
        }
    }
    return result;
}

// ===========================
// NODE DISCOVERY
// ===========================

bool NetworkTopologyManager::startNodeDiscovery() {
    if (discoveryActive_) {
        Logger::warn("[NetworkTopology] Discovery already active");
        return true;
    }

    Logger::info("[NetworkTopology] Starting node discovery...");

    discoveryActive_ = true;
    lastDiscovery_ = getCurrentTimestamp();

    // Broadcast discovery message
    if (updateProtocol_) {
        updateProtocol_->broadcastPeerDiscovery();
    }

    return true;
}

bool NetworkTopologyManager::stopNodeDiscovery() {
    if (!discoveryActive_) {
        return true;
    }

    Logger::info("[NetworkTopology] Stopping node discovery...");
    discoveryActive_ = false;

    return true;
}

bool NetworkTopologyManager::discoverNearbyNodes() {
    Logger::info("[NetworkTopology] Discovering nearby nodes...");

    lastDiscovery_ = getCurrentTimestamp();

    // Broadcast discovery message
    if (updateProtocol_) {
        updateProtocol_->broadcastPeerDiscovery();
    }

    return true;
}

void NetworkTopologyManager::handleNodeAnnouncement(const String& nodeId, const NetworkNode& node) {
    Logger::info("[NetworkTopology] Received node announcement from: %s", nodeId.c_str());

    if (nodes_.find(nodeId) == nodes_.end()) {
        addNode(node);
    } else {
        updateNode(node);
    }
}

// ===========================
// TOPOLOGY MANAGEMENT
// ===========================

bool NetworkTopologyManager::reconfigureTopology(NetworkTopology newTopology) {
    Logger::info("[NetworkTopology] Reconfiguring to topology: %d", static_cast<int>(newTopology));

    bool success = false;

    switch (newTopology) {
        case NetworkTopology::STAR:
            success = buildStarTopology();
            break;

        case NetworkTopology::MESH:
            success = buildMeshTopology();
            break;

        case NetworkTopology::HIERARCHICAL:
            success = buildHierarchicalTopology();
            break;

        case NetworkTopology::HYBRID:
            success = buildHybridTopology();
            break;

        default:
            Logger::error("[NetworkTopology] Unknown topology type");
            return false;
    }

    if (success) {
        currentTopology_ = newTopology;
        broadcastTopologyChange();
    }

    return success;
}

bool NetworkTopologyManager::optimizeTopology() {
    Logger::info("[NetworkTopology] Optimizing topology...");

    // Recalculate routes
    calculateRoutes();

    // Rebalance network
    rebalanceNetwork();

    // Prune unreliable routes
    pruneUnreliableRoutes();

    // Update metrics
    updateNetworkMetrics();

    return true;
}

bool NetworkTopologyManager::validateTopology() const {
    // Check if all nodes are reachable
    for (const auto& pair : nodes_) {
        if (!pair.second.isReachable && pair.first != localNodeId_) {
            Logger::warn("[NetworkTopology] Unreachable node: %s", pair.first.c_str());
        }
    }

    // Check if coordinator exists
    if (currentTopology_ == NetworkTopology::STAR && currentCoordinator_.isEmpty()) {
        Logger::warn("[NetworkTopology] No coordinator in star topology");
        return false;
    }

    return true;
}

// ===========================
// COORDINATOR SELECTION
// ===========================

String NetworkTopologyManager::selectCoordinator() {
    Logger::info("[NetworkTopology] Selecting coordinator...");

    // Use reliability-based election
    String coordinator = electCoordinatorByReliability();

    if (!coordinator.isEmpty()) {
        currentCoordinator_ = coordinator;

        // Update node type
        auto it = nodes_.find(coordinator);
        if (it != nodes_.end()) {
            it->second.type = NodeType::COORDINATOR;
        }

        Logger::info("[NetworkTopology] Selected coordinator: %s", coordinator.c_str());

        // Notify callback
        if (coordinatorChangeCallback_) {
            coordinatorChangeCallback_(coordinator);
        }
    }

    return coordinator;
}

bool NetworkTopologyManager::promoteToCoordinator(const String& nodeId) {
    if (!validateCoordinatorEligibility(nodeId)) {
        Logger::error("[NetworkTopology] Node not eligible for coordinator: %s", nodeId.c_str());
        return false;
    }

    Logger::info("[NetworkTopology] Promoting node to coordinator: %s", nodeId.c_str());

    // Demote current coordinator if exists
    if (!currentCoordinator_.isEmpty() && currentCoordinator_ != nodeId) {
        auto it = nodes_.find(currentCoordinator_);
        if (it != nodes_.end()) {
            it->second.type = NodeType::CLIENT;
        }
    }

    // Promote new coordinator
    currentCoordinator_ = nodeId;
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        it->second.type = NodeType::COORDINATOR;
    }

    // Notify callback
    if (coordinatorChangeCallback_) {
        coordinatorChangeCallback_(nodeId);
    }

    return true;
}

bool NetworkTopologyManager::electNewCoordinator() {
    Logger::info("[NetworkTopology] Electing new coordinator...");

    String newCoordinator = selectCoordinator();
    return !newCoordinator.isEmpty();
}

// ===========================
// ROUTE MANAGEMENT
// ===========================

bool NetworkTopologyManager::calculateRoutes() {
    Logger::info("[NetworkTopology] Calculating routes...");

    lastRouteUpdate_ = getCurrentTimestamp();

    // Clear existing routes
    routes_.clear();
    routingTable_.clear();

    // Calculate routes between all pairs of nodes
    for (const auto& source : nodes_) {
        for (const auto& dest : nodes_) {
            if (source.first == dest.first) {
                continue;
            }

            NetworkRoute route = findBestRoute(source.first, dest.first);
            if (route.active) {
                auto key = std::make_pair(source.first, dest.first);
                routes_[key] = route;

                // Update routing table
                if (!route.path.empty() && route.path.size() > 1) {
                    routingTable_[source.first].push_back(route.path[1]);
                }
            }
        }
    }

    metrics_.activeRoutes = routes_.size();

    Logger::info("[NetworkTopology] Calculated %u routes", metrics_.activeRoutes);

    return true;
}

NetworkRoute NetworkTopologyManager::findBestRoute(const String& sourceId, const String& destinationId) {
    NetworkRoute route;
    route.sourceId = sourceId;
    route.destinationId = destinationId;

    // Use Dijkstra's algorithm
    std::vector<String> path = dijkstraShortestPath(sourceId, destinationId);

    if (path.empty()) {
        route.active = false;
        return route;
    }

    route.path = path;
    route.cost = calculateRouteCost(path);
    route.reliability = calculateRouteReliability(path);
    route.latency = route.cost * 10; // Simple latency estimation
    route.protocol = "WiFi"; // Default protocol
    route.active = true;

    return route;
}

std::vector<NetworkRoute> NetworkTopologyManager::findAlternativeRoutes(const String& sourceId, 
                                                                        const String& destinationId) {
    std::vector<NetworkRoute> alternatives;

    // Find primary route
    NetworkRoute primary = findBestRoute(sourceId, destinationId);
    if (primary.active) {
        alternatives.push_back(primary);
    }

    // Would find additional alternative routes
    // For now, just return the primary route

    return alternatives;
}

bool NetworkTopologyManager::updateRoute(const NetworkRoute& route) {
    if (!isRouteValid(route)) {
        Logger::error("[NetworkTopology] Invalid route");
        return false;
    }

    auto key = std::make_pair(route.sourceId, route.destinationId);
    routes_[key] = route;

    Logger::debug("[NetworkTopology] Updated route: %s -> %s", 
                 route.sourceId.c_str(), route.destinationId.c_str());

    return true;
}

void NetworkTopologyManager::invalidateRoute(const String& sourceId, const String& destinationId) {
    auto key = std::make_pair(sourceId, destinationId);
    auto it = routes_.find(key);

    if (it != routes_.end()) {
        it->second.active = false;
        Logger::debug("[NetworkTopology] Invalidated route: %s -> %s", 
                     sourceId.c_str(), destinationId.c_str());
    }
}

// ===========================
// CONNECTION MANAGEMENT
// ===========================

bool NetworkTopologyManager::establishConnection(const String& nodeId) {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        Logger::error("[NetworkTopology] Node not found: %s", nodeId.c_str());
        return false;
    }

    Logger::info("[NetworkTopology] Establishing connection to: %s", nodeId.c_str());

    it->second.status = ConnectionStatus::CONNECTING;

    // Would establish actual connection
    // For now, just mark as connected
    it->second.status = ConnectionStatus::CONNECTED;
    it->second.isReachable = true;
    connectedNodes_.insert(nodeId);
    metrics_.connectedNodes++;

    return true;
}

bool NetworkTopologyManager::closeConnection(const String& nodeId) {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return false;
    }

    Logger::info("[NetworkTopology] Closing connection to: %s", nodeId.c_str());

    it->second.status = ConnectionStatus::DISCONNECTED;
    it->second.isReachable = false;
    connectedNodes_.erase(nodeId);
    
    if (metrics_.connectedNodes > 0) {
        metrics_.connectedNodes--;
    }

    return true;
}

ConnectionStatus NetworkTopologyManager::getConnectionStatus(const String& nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        return it->second.status;
    }
    return ConnectionStatus::DISCONNECTED;
}

std::vector<String> NetworkTopologyManager::getConnectedNodes() const {
    return std::vector<String>(connectedNodes_.begin(), connectedNodes_.end());
}

// ===========================
// MESSAGE ROUTING
// ===========================

bool NetworkTopologyManager::routeMessage(const String& sourceId, const String& destinationId,
                                         const std::vector<uint8_t>& message) {
    Logger::debug("[NetworkTopology] Routing message: %s -> %s", 
                 sourceId.c_str(), destinationId.c_str());

    // Find route
    auto key = std::make_pair(sourceId, destinationId);
    auto it = routes_.find(key);

    if (it == routes_.end() || !it->second.active) {
        Logger::error("[NetworkTopology] No active route found");
        return false;
    }

    // Get next hop
    String nextHop = getNextHop(destinationId);
    if (nextHop.isEmpty()) {
        Logger::error("[NetworkTopology] No next hop found");
        return false;
    }

    // Forward message
    return forwardMessage(nextHop, message);
}

bool NetworkTopologyManager::broadcastMessage(const std::vector<uint8_t>& message,
                                             const std::set<String>& excludeNodes) {
    Logger::info("[NetworkTopology] Broadcasting message to all nodes");

    for (const auto& nodeId : connectedNodes_) {
        if (nodeId == localNodeId_ || excludeNodes.count(nodeId) > 0) {
            continue;
        }

        forwardMessage(nodeId, message);
    }

    return true;
}

bool NetworkTopologyManager::forwardMessage(const String& nextHop, 
                                           const std::vector<uint8_t>& message) {
    Logger::debug("[NetworkTopology] Forwarding message to: %s", nextHop.c_str());

    // Would actually send the message
    // For now, just return success
    return true;
}

// ===========================
// NETWORK MONITORING
// ===========================

bool NetworkTopologyManager::checkNodeHealth(const String& nodeId) {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return false;
    }

    // Simple health check based on last seen time
    uint32_t timeSinceLastSeen = getCurrentTimestamp() - it->second.lastSeen;
    bool healthy = timeSinceLastSeen < config_.heartbeatInterval * 3;

    if (!healthy) {
        Logger::warn("[NetworkTopology] Node unhealthy: %s", nodeId.c_str());
        handleNodeFailure(nodeId);
    }

    return healthy;
}

void NetworkTopologyManager::updateNodeReliability(const String& nodeId, bool success) {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return;
    }

    // Update reliability with exponential moving average
    float alpha = 0.1f;
    float newReliability = success ? 1.0f : 0.0f;
    it->second.reliability = alpha * newReliability + (1.0f - alpha) * it->second.reliability;

    Logger::debug("[NetworkTopology] Updated reliability for %s: %.3f", 
                 nodeId.c_str(), it->second.reliability);
}

bool NetworkTopologyManager::detectNetworkPartition() {
    // Check if network is partitioned
    // Simple check: if less than half of nodes are reachable
    if (metrics_.totalNodes > 1) {
        float reachabilityRatio = static_cast<float>(metrics_.connectedNodes) / metrics_.totalNodes;
        if (reachabilityRatio < 0.5f) {
            Logger::warn("[NetworkTopology] Network partition detected");
            metrics_.networkPartitions++;
            handleNetworkPartition();
            return true;
        }
    }

    return false;
}

void NetworkTopologyManager::handleNodeFailure(const String& nodeId) {
    Logger::warn("[NetworkTopology] Handling node failure: %s", nodeId.c_str());

    // Notify callback
    if (nodeFailureCallback_) {
        nodeFailureCallback_(nodeId);
    }

    // Mark node as unreachable
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        it->second.status = ConnectionStatus::ERROR;
        it->second.isReachable = false;
    }

    // Invalidate routes
    for (const auto& pair : nodes_) {
        invalidateRoute(nodeId, pair.first);
        invalidateRoute(pair.first, nodeId);
    }

    // If this was the coordinator, elect new one
    if (nodeId == currentCoordinator_) {
        electNewCoordinator();
    }

    metrics_.failedConnections++;
}

// ===========================
// LOAD BALANCING
// ===========================

bool NetworkTopologyManager::enableLoadBalancing(bool enable) {
    config_.enableLoadBalancing = enable;
    Logger::info("[NetworkTopology] Load balancing %s", enable ? "enabled" : "disabled");
    return true;
}

String NetworkTopologyManager::selectLeastLoadedRoute(const String& destinationId) {
    String bestNext = getNextHop(destinationId);
    float minLoad = getNodeLoad(bestNext);

    // Find alternative routes and select least loaded
    auto alternatives = findAlternativeRoutes(localNodeId_, destinationId);
    for (const auto& route : alternatives) {
        if (route.path.size() > 1) {
            String nextHop = route.path[1];
            float load = getNodeLoad(nextHop);
            if (load < minLoad) {
                minLoad = load;
                bestNext = nextHop;
            }
        }
    }

    return bestNext;
}

void NetworkTopologyManager::distributeLoad() {
    Logger::info("[NetworkTopology] Distributing load across network...");
    rebalanceNetwork();
}

float NetworkTopologyManager::getNodeLoad(const String& nodeId) const {
    auto it = nodeLoads_.find(nodeId);
    if (it != nodeLoads_.end()) {
        return static_cast<float>(it->second) / 1000.0f;
    }
    return 0.0f;
}

// ===========================
// FAULT TOLERANCE
// ===========================

bool NetworkTopologyManager::enableFaultTolerance(bool enable) {
    config_.enableFaultTolerance = enable;
    Logger::info("[NetworkTopology] Fault tolerance %s", enable ? "enabled" : "disabled");
    return true;
}

void NetworkTopologyManager::handleNetworkPartition() {
    Logger::warn("[NetworkTopology] Handling network partition...");

    // Try to reconfigure topology
    if (config_.enableAutoReconfiguration) {
        optimizeTopology();
    }

    // Create backup routes if possible
    if (config_.enableFaultTolerance) {
        createBackupRoutes();
    }
}

bool NetworkTopologyManager::recoverFromPartition() {
    Logger::info("[NetworkTopology] Recovering from network partition...");

    // Rediscover nodes
    discoverNearbyNodes();

    // Recalculate routes
    calculateRoutes();

    // Validate topology
    return validateTopology();
}

bool NetworkTopologyManager::createBackupRoutes() {
    Logger::info("[NetworkTopology] Creating backup routes...");

    // Find alternative routes for all node pairs
    for (const auto& source : nodes_) {
        for (const auto& dest : nodes_) {
            if (source.first == dest.first) {
                continue;
            }

            auto alternatives = findAlternativeRoutes(source.first, dest.first);
            // Would store backup routes
        }
    }

    return true;
}

// ===========================
// NETWORK OPTIMIZATION
// ===========================

bool NetworkTopologyManager::optimizeForBandwidth() {
    Logger::info("[NetworkTopology] Optimizing for bandwidth...");
    // Would implement bandwidth-optimized routing
    return calculateRoutes();
}

bool NetworkTopologyManager::optimizeForLatency() {
    Logger::info("[NetworkTopology] Optimizing for latency...");
    // Routes already optimized for shortest path (lowest latency)
    return true;
}

bool NetworkTopologyManager::optimizeForReliability() {
    Logger::info("[NetworkTopology] Optimizing for reliability...");
    pruneUnreliableRoutes();
    return true;
}

bool NetworkTopologyManager::optimizeForPowerConsumption() {
    Logger::info("[NetworkTopology] Optimizing for power consumption...");
    // Would prefer routes through nodes with higher battery levels
    return calculateRoutes();
}

// ===========================
// METRICS AND MONITORING
// ===========================

NetworkMetrics NetworkTopologyManager::getNetworkMetrics() const {
    return metrics_;
}

void NetworkTopologyManager::resetMetrics() {
    metrics_ = NetworkMetrics();
    metrics_.totalNodes = nodes_.size();
    metrics_.connectedNodes = connectedNodes_.size();
    Logger::info("[NetworkTopology] Metrics reset");
}

float NetworkTopologyManager::calculateNetworkEfficiency() const {
    if (metrics_.totalNodes <= 1) {
        return 1.0f;
    }

    // Simple efficiency calculation
    float connectivityRatio = static_cast<float>(metrics_.connectedNodes) / metrics_.totalNodes;
    float reliabilityAvg = metrics_.averageReliability;

    return (connectivityRatio + reliabilityAvg) / 2.0f;
}

uint32_t NetworkTopologyManager::estimateMessageDeliveryTime(const String& destinationId) const {
    auto key = std::make_pair(localNodeId_, destinationId);
    auto it = routes_.find(key);

    if (it != routes_.end()) {
        return it->second.latency;
    }

    return 1000; // Default 1 second
}

// ===========================
// UTILITY METHODS
// ===========================

bool NetworkTopologyManager::isNodeReachable(const String& nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        return it->second.isReachable;
    }
    return false;
}

uint32_t NetworkTopologyManager::getHopDistance(const String& nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it != nodes_.end()) {
        return it->second.hopDistance;
    }
    return 0;
}

std::vector<String> NetworkTopologyManager::getNeighborNodes(const String& nodeId) const {
    std::vector<String> neighbors;

    // Find all nodes directly connected to this node
    for (const auto& pair : routes_) {
        if (pair.first.first == nodeId && pair.second.path.size() == 2) {
            neighbors.push_back(pair.first.second);
        }
    }

    return neighbors;
}

String NetworkTopologyManager::getNextHop(const String& destinationId) const {
    auto it = routingTable_.find(localNodeId_);
    if (it != routingTable_.end() && !it->second.empty()) {
        // Would select appropriate next hop based on destination
        // For now, return first entry
        return it->second[0];
    }

    return "";
}

// ===========================
// PROTOCOL-SPECIFIC METHODS
// ===========================

bool NetworkTopologyManager::configureWiFiMesh() {
    Logger::info("[NetworkTopology] Configuring WiFi mesh...");
    // Would configure WiFi mesh network
    return true;
}

bool NetworkTopologyManager::configureLoRaMesh() {
    Logger::info("[NetworkTopology] Configuring LoRa mesh...");
    // Would configure LoRa mesh network
    return true;
}

bool NetworkTopologyManager::configureCellularNetwork() {
    Logger::info("[NetworkTopology] Configuring cellular network...");
    // Would configure cellular network
    return true;
}

bool NetworkTopologyManager::configureHybridNetwork() {
    Logger::info("[NetworkTopology] Configuring hybrid network...");
    // Would configure hybrid multi-protocol network
    return true;
}

// ===========================
// PRIVATE METHODS - TOPOLOGY ALGORITHMS
// ===========================

bool NetworkTopologyManager::buildStarTopology() {
    Logger::info("[NetworkTopology] Building star topology...");

    // Select coordinator if not already selected
    if (currentCoordinator_.isEmpty()) {
        selectCoordinator();
    }

    // All nodes connect to coordinator
    for (auto& pair : nodes_) {
        if (pair.first != currentCoordinator_) {
            pair.second.hopDistance = 1;
        } else {
            pair.second.hopDistance = 0;
        }
    }

    return calculateRoutes();
}

bool NetworkTopologyManager::buildMeshTopology() {
    Logger::info("[NetworkTopology] Building mesh topology...");

    // All nodes can potentially connect to all other nodes
    // Calculate optimal connections based on signal strength and distance
    
    return calculateRoutes();
}

bool NetworkTopologyManager::buildHierarchicalTopology() {
    Logger::info("[NetworkTopology] Building hierarchical topology...");

    // Build tree structure with coordinator at root
    if (currentCoordinator_.isEmpty()) {
        selectCoordinator();
    }

    // Assign hop distances in tree structure
    std::queue<String> queue;
    std::set<String> visited;

    queue.push(currentCoordinator_);
    visited.insert(currentCoordinator_);

    auto it = nodes_.find(currentCoordinator_);
    if (it != nodes_.end()) {
        it->second.hopDistance = 0;
    }

    while (!queue.empty()) {
        String current = queue.front();
        queue.pop();

        uint32_t currentHops = nodes_[current].hopDistance;

        // Add children (simplified - would use actual connectivity)
        for (auto& pair : nodes_) {
            if (visited.find(pair.first) == visited.end()) {
                pair.second.hopDistance = currentHops + 1;
                queue.push(pair.first);
                visited.insert(pair.first);
            }
        }
    }

    return calculateRoutes();
}

bool NetworkTopologyManager::buildHybridTopology() {
    Logger::info("[NetworkTopology] Building hybrid topology...");

    // Combine star and mesh characteristics
    // Critical nodes use star, others use mesh

    buildStarTopology();
    
    // Add mesh connections between nearby nodes
    // Would implement actual hybrid logic

    return true;
}

// ===========================
// PRIVATE METHODS - ROUTE CALCULATION
// ===========================

std::vector<String> NetworkTopologyManager::dijkstraShortestPath(const String& source, 
                                                                 const String& destination) {
    // Dijkstra's shortest path algorithm
    std::map<String, uint32_t> distances;
    std::map<String, String> previous;
    std::set<String> unvisited;

    // Initialize
    for (const auto& pair : nodes_) {
        distances[pair.first] = std::numeric_limits<uint32_t>::max();
        unvisited.insert(pair.first);
    }
    distances[source] = 0;

    while (!unvisited.empty()) {
        // Find node with minimum distance
        String current;
        uint32_t minDist = std::numeric_limits<uint32_t>::max();
        for (const auto& nodeId : unvisited) {
            if (distances[nodeId] < minDist) {
                minDist = distances[nodeId];
                current = nodeId;
            }
        }

        if (current.isEmpty() || minDist == std::numeric_limits<uint32_t>::max()) {
            break;
        }

        unvisited.erase(current);

        if (current == destination) {
            break;
        }

        // Update neighbors
        auto neighbors = getNeighborNodes(current);
        for (const auto& neighbor : neighbors) {
            if (unvisited.find(neighbor) == unvisited.end()) {
                continue;
            }

            uint32_t alt = distances[current] + 1; // Unit cost per hop
            if (alt < distances[neighbor]) {
                distances[neighbor] = alt;
                previous[neighbor] = current;
            }
        }
    }

    // Reconstruct path
    std::vector<String> path;
    String current = destination;
    
    if (previous.find(current) == previous.end() && current != source) {
        return path; // No path found
    }

    while (!current.isEmpty()) {
        path.insert(path.begin(), current);
        if (current == source) {
            break;
        }
        auto it = previous.find(current);
        if (it != previous.end()) {
            current = it->second;
        } else {
            break;
        }
    }

    return path;
}

uint32_t NetworkTopologyManager::calculateRouteCost(const std::vector<String>& path) {
    // Simple cost: number of hops
    return path.size() > 0 ? path.size() - 1 : 0;
}

float NetworkTopologyManager::calculateRouteReliability(const std::vector<String>& path) {
    if (path.empty()) {
        return 0.0f;
    }

    float reliability = 1.0f;
    for (const auto& nodeId : path) {
        auto it = nodes_.find(nodeId);
        if (it != nodes_.end()) {
            reliability *= it->second.reliability;
        }
    }

    return reliability;
}

bool NetworkTopologyManager::isRouteValid(const NetworkRoute& route) const {
    if (route.path.empty()) {
        return false;
    }

    if (route.path.front() != route.sourceId || route.path.back() != route.destinationId) {
        return false;
    }

    // Check all nodes in path exist and are reachable
    for (const auto& nodeId : route.path) {
        if (!isNodeReachable(nodeId)) {
            return false;
        }
    }

    return true;
}

// ===========================
// PRIVATE METHODS - COORDINATOR ELECTION
// ===========================

String NetworkTopologyManager::electCoordinatorByReliability() {
    String best;
    float maxReliability = 0.0f;

    for (const auto& pair : nodes_) {
        if (validateCoordinatorEligibility(pair.first) && 
            pair.second.reliability > maxReliability) {
            maxReliability = pair.second.reliability;
            best = pair.first;
        }
    }

    return best;
}

String NetworkTopologyManager::electCoordinatorByBattery() {
    String best;
    float maxBattery = 0.0f;

    for (const auto& pair : nodes_) {
        if (validateCoordinatorEligibility(pair.first) && 
            pair.second.batteryLevel > maxBattery) {
            maxBattery = pair.second.batteryLevel;
            best = pair.first;
        }
    }

    return best;
}

String NetworkTopologyManager::electCoordinatorByConnectivity() {
    String best;
    size_t maxConnections = 0;

    for (const auto& pair : nodes_) {
        if (validateCoordinatorEligibility(pair.first)) {
            auto neighbors = getNeighborNodes(pair.first);
            if (neighbors.size() > maxConnections) {
                maxConnections = neighbors.size();
                best = pair.first;
            }
        }
    }

    return best;
}

bool NetworkTopologyManager::validateCoordinatorEligibility(const String& nodeId) const {
    auto it = nodes_.find(nodeId);
    if (it == nodes_.end()) {
        return false;
    }

    const auto& node = it->second;

    // Check basic requirements
    if (!node.isReachable) {
        return false;
    }

    if (node.reliability < config_.minReliability) {
        return false;
    }

    if (node.batteryLevel < 0.3f) {
        return false;
    }

    return true;
}

// ===========================
// PRIVATE METHODS - NETWORK OPTIMIZATION
// ===========================

void NetworkTopologyManager::rebalanceNetwork() {
    Logger::debug("[NetworkTopology] Rebalancing network...");

    // Redistribute load across nodes
    for (auto& pair : nodeLoads_) {
        pair.second = 0;
    }

    // Recalculate optimal routes
    calculateRoutes();
}

void NetworkTopologyManager::pruneUnreliableRoutes() {
    Logger::debug("[NetworkTopology] Pruning unreliable routes...");

    for (auto& pair : routes_) {
        if (pair.second.reliability < config_.minReliability) {
            pair.second.active = false;
            metrics_.activeRoutes--;
        }
    }
}

void NetworkTopologyManager::updateNetworkMetrics() {
    // Calculate average reliability
    float totalReliability = 0.0f;
    uint32_t totalLatency = 0;

    for (const auto& pair : nodes_) {
        totalReliability += pair.second.reliability;
    }

    for (const auto& pair : routes_) {
        if (pair.second.active) {
            totalLatency += pair.second.latency;
        }
    }

    if (!nodes_.empty()) {
        metrics_.averageReliability = totalReliability / nodes_.size();
    }

    if (metrics_.activeRoutes > 0) {
        metrics_.averageLatency = totalLatency / metrics_.activeRoutes;
    }

    metrics_.networkEfficiency = calculateNetworkEfficiency();
}

void NetworkTopologyManager::detectAndHandleFailures() {
    for (const auto& pair : nodes_) {
        if (pair.first != localNodeId_) {
            checkNodeHealth(pair.first);
        }
    }
}

// ===========================
// PRIVATE METHODS - PROTOCOL INTEGRATION
// ===========================

bool NetworkTopologyManager::sendTopologyUpdate(const String& nodeId) {
    Logger::debug("[NetworkTopology] Sending topology update to: %s", nodeId.c_str());
    // Would send actual topology update
    return true;
}

bool NetworkTopologyManager::handleTopologyUpdate(const String& nodeId, 
                                                 const std::vector<uint8_t>& data) {
    Logger::debug("[NetworkTopology] Handling topology update from: %s", nodeId.c_str());
    // Would process topology update
    return true;
}

void NetworkTopologyManager::broadcastTopologyChange() {
    Logger::info("[NetworkTopology] Broadcasting topology change");
    
    if (updateProtocol_) {
        // Would broadcast topology change notification
    }
}

// ===========================
// PRIVATE METHODS - UTILITIES
// ===========================

uint32_t NetworkTopologyManager::getCurrentTimestamp() const {
    return millis();
}

String NetworkTopologyManager::generateRouteKey(const String& source, const String& destination) const {
    return source + "->" + destination;
}

void NetworkTopologyManager::logTopologyEvent(const String& event, const String& details) {
    Logger::debug("[NetworkTopology] %s: %s", event.c_str(), details.c_str());
}

bool NetworkTopologyManager::validateNodeId(const String& nodeId) const {
    return !nodeId.isEmpty() && nodeId.length() < 64;
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

bool initializeNetworkTopology(const TopologyConfig& config) {
    if (g_networkTopologyManager) {
        Logger::warn("[NetworkTopology] Already initialized");
        return true;
    }

    g_networkTopologyManager = new NetworkTopologyManager();
    return g_networkTopologyManager->init(config);
}

void cleanupNetworkTopology() {
    if (g_networkTopologyManager) {
        delete g_networkTopologyManager;
        g_networkTopologyManager = nullptr;
    }
}

bool discoverFederatedNodes() {
    if (!g_networkTopologyManager || !g_networkTopologyManager->isInitialized()) {
        return false;
    }

    return g_networkTopologyManager->discoverNearbyNodes();
}

String getCurrentFederatedCoordinator() {
    if (!g_networkTopologyManager) {
        return "";
    }

    return g_networkTopologyManager->getCurrentCoordinator();
}

bool isNetworkTopologyReady() {
    if (!g_networkTopologyManager) {
        return false;
    }

    return g_networkTopologyManager->isInitialized() && 
           g_networkTopologyManager->validateTopology();
}

NetworkMetrics getCurrentNetworkMetrics() {
    if (!g_networkTopologyManager) {
        return NetworkMetrics();
    }

    return g_networkTopologyManager->getNetworkMetrics();
}
