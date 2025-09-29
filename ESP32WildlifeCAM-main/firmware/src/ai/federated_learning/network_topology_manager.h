/**
 * @file network_topology_manager.h
 * @brief Network Topology Manager for Federated Learning
 * 
 * Manages network topology for federated learning, supporting star,
 * mesh, and hybrid configurations with dynamic adaptation.
 */

#ifndef NETWORK_TOPOLOGY_MANAGER_H
#define NETWORK_TOPOLOGY_MANAGER_H

#include "federated_common.h"
#include "model_update_protocol.h"
#include "../../network_selector.h"
#include <vector>
#include <map>
#include <set>
#include <memory>

/**
 * Node Types in Network Topology
 */
enum class NodeType {
    CLIENT,                    // Regular client node
    COORDINATOR,              // Central coordinator
    RELAY,                    // Relay/forwarder node
    EDGE_GATEWAY,             // Edge gateway node
    BACKUP_COORDINATOR        // Backup coordinator
};

/**
 * Connection Status
 */
enum class ConnectionStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING,
    ERROR,
    TIMEOUT
};

/**
 * Network Node Information
 */
struct NetworkNode {
    String nodeId;
    String ipAddress;
    NodeType type;
    ConnectionStatus status;
    std::vector<String> protocols;  // Supported protocols
    uint32_t lastSeen;
    float reliability;              // 0.0 to 1.0
    uint32_t hopDistance;          // Hops from coordinator
    float signalStrength;          // Signal quality
    uint32_t bandwidth;            // Available bandwidth
    float batteryLevel;            // Battery level (0.0 to 1.0)
    bool isReachable;
    String location;               // Geographic coordinates
    
    NetworkNode() : type(NodeType::CLIENT), status(ConnectionStatus::DISCONNECTED),
                   lastSeen(0), reliability(1.0f), hopDistance(0),
                   signalStrength(0.0f), bandwidth(0), batteryLevel(1.0f),
                   isReachable(false) {}
};

/**
 * Network Route Information
 */
struct NetworkRoute {
    String sourceId;
    String destinationId;
    std::vector<String> path;      // Node IDs in route
    uint32_t cost;                 // Route cost metric
    uint32_t latency;              // Estimated latency
    float reliability;             // Route reliability
    String protocol;               // Primary protocol
    bool active;
    
    NetworkRoute() : cost(0), latency(0), reliability(1.0f), active(false) {}
};

/**
 * Topology Configuration
 */
struct TopologyConfig {
    NetworkTopology preferredTopology;
    uint32_t maxNodes;
    uint32_t maxHops;
    uint32_t heartbeatInterval;
    uint32_t discoveryInterval;
    uint32_t routeUpdateInterval;
    float minReliability;
    bool enableAutoReconfiguration;
    bool enableLoadBalancing;
    bool enableFaultTolerance;
    
    TopologyConfig() : preferredTopology(NetworkTopology::STAR),
                      maxNodes(50), maxHops(3), heartbeatInterval(30000),
                      discoveryInterval(60000), routeUpdateInterval(120000),
                      minReliability(0.8f), enableAutoReconfiguration(true),
                      enableLoadBalancing(true), enableFaultTolerance(true) {}
};

/**
 * Network Metrics
 */
struct NetworkMetrics {
    uint32_t totalNodes;
    uint32_t connectedNodes;
    uint32_t activeRoutes;
    float averageReliability;
    uint32_t averageLatency;
    uint32_t totalBandwidth;
    uint32_t networkPartitions;
    uint32_t failedConnections;
    float networkEfficiency;
    
    NetworkMetrics() : totalNodes(0), connectedNodes(0), activeRoutes(0),
                      averageReliability(0.0f), averageLatency(0),
                      totalBandwidth(0), networkPartitions(0),
                      failedConnections(0), networkEfficiency(0.0f) {}
};

/**
 * Network Topology Manager
 * 
 * Manages federated learning network topology:
 * - Node discovery and management
 * - Route calculation and optimization
 * - Topology adaptation and reconfiguration
 * - Fault tolerance and recovery
 * - Load balancing and optimization
 */
class NetworkTopologyManager {
public:
    NetworkTopologyManager();
    ~NetworkTopologyManager();
    
    // Initialization
    bool init(const TopologyConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setTopologyConfig(const TopologyConfig& config);
    TopologyConfig getTopologyConfig() const { return config_; }
    void setPreferredTopology(NetworkTopology topology);
    
    // Node management
    bool addNode(const NetworkNode& node);
    bool removeNode(const String& nodeId);
    bool updateNode(const NetworkNode& node);
    NetworkNode getNode(const String& nodeId) const;
    std::vector<NetworkNode> getAllNodes() const;
    std::vector<NetworkNode> getNodesOfType(NodeType type) const;
    
    // Node discovery
    bool startNodeDiscovery();
    bool stopNodeDiscovery();
    bool discoverNearbyNodes();
    void handleNodeAnnouncement(const String& nodeId, const NetworkNode& node);
    
    // Topology management
    NetworkTopology getCurrentTopology() const { return currentTopology_; }
    bool reconfigureTopology(NetworkTopology newTopology);
    bool optimizeTopology();
    bool validateTopology() const;
    
    // Coordinator selection and management
    String selectCoordinator();
    bool promoteToCoordinator(const String& nodeId);
    bool electNewCoordinator();
    String getCurrentCoordinator() const { return currentCoordinator_; }
    
    // Route management
    bool calculateRoutes();
    NetworkRoute findBestRoute(const String& sourceId, const String& destinationId);
    std::vector<NetworkRoute> findAlternativeRoutes(const String& sourceId, const String& destinationId);
    bool updateRoute(const NetworkRoute& route);
    void invalidateRoute(const String& sourceId, const String& destinationId);
    
    // Connection management
    bool establishConnection(const String& nodeId);
    bool closeConnection(const String& nodeId);
    ConnectionStatus getConnectionStatus(const String& nodeId) const;
    std::vector<String> getConnectedNodes() const;
    
    // Message routing
    bool routeMessage(const String& sourceId, const String& destinationId, 
                     const std::vector<uint8_t>& message);
    bool broadcastMessage(const std::vector<uint8_t>& message, 
                         const std::set<String>& excludeNodes = {});
    bool forwardMessage(const String& nextHop, const std::vector<uint8_t>& message);
    
    // Network monitoring
    bool checkNodeHealth(const String& nodeId);
    void updateNodeReliability(const String& nodeId, bool success);
    bool detectNetworkPartition();
    void handleNodeFailure(const String& nodeId);
    
    // Load balancing
    bool enableLoadBalancing(bool enable);
    String selectLeastLoadedRoute(const String& destinationId);
    void distributeLoad();
    float getNodeLoad(const String& nodeId) const;
    
    // Fault tolerance
    bool enableFaultTolerance(bool enable);
    void handleNetworkPartition();
    bool recoverFromPartition();
    bool createBackupRoutes();
    
    // Network optimization
    bool optimizeForBandwidth();
    bool optimizeForLatency();
    bool optimizeForReliability();
    bool optimizeForPowerConsumption();
    
    // Metrics and monitoring
    NetworkMetrics getNetworkMetrics() const;
    void resetMetrics();
    float calculateNetworkEfficiency() const;
    uint32_t estimateMessageDeliveryTime(const String& destinationId) const;
    
    // Event callbacks
    typedef std::function<void(const NetworkNode&)> NodeJoinCallback;
    typedef std::function<void(const String&)> NodeLeaveCallback;
    typedef std::function<void(const String&)> NodeFailureCallback;
    typedef std::function<void(NetworkTopology, NetworkTopology)> TopologyChangeCallback;
    typedef std::function<void(const String&)> CoordinatorChangeCallback;
    
    void setNodeJoinCallback(NodeJoinCallback callback) { nodeJoinCallback_ = callback; }
    void setNodeLeaveCallback(NodeLeaveCallback callback) { nodeLeaveCallback_ = callback; }
    void setNodeFailureCallback(NodeFailureCallback callback) { nodeFailureCallback_ = callback; }
    void setTopologyChangeCallback(TopologyChangeCallback callback) { topologyChangeCallback_ = callback; }
    void setCoordinatorChangeCallback(CoordinatorChangeCallback callback) { coordinatorChangeCallback_ = callback; }
    
    // Utility methods
    bool isNodeReachable(const String& nodeId) const;
    uint32_t getHopDistance(const String& nodeId) const;
    std::vector<String> getNeighborNodes(const String& nodeId) const;
    String getNextHop(const String& destinationId) const;
    
    // Protocol-specific methods
    bool configureWiFiMesh();
    bool configureLoRaMesh();
    bool configureCellularNetwork();
    bool configureHybridNetwork();

private:
    // Configuration
    TopologyConfig config_;
    bool initialized_;
    
    // Current topology state
    NetworkTopology currentTopology_;
    String currentCoordinator_;
    String localNodeId_;
    
    // Node management
    std::map<String, NetworkNode> nodes_;
    std::set<String> connectedNodes_;
    std::map<String, uint32_t> nodeLoads_;
    
    // Route management
    std::map<std::pair<String, String>, NetworkRoute> routes_;
    std::map<String, std::vector<String>> routingTable_;
    
    // Network state
    bool discoveryActive_;
    uint32_t lastDiscovery_;
    uint32_t lastRouteUpdate_;
    NetworkMetrics metrics_;
    
    // Protocol management
    std::unique_ptr<ModelUpdateProtocol> updateProtocol_;
    std::unique_ptr<NetworkSelector> networkSelector_;
    
    // Callbacks
    NodeJoinCallback nodeJoinCallback_;
    NodeLeaveCallback nodeLeaveCallback_;
    NodeFailureCallback nodeFailureCallback_;
    TopologyChangeCallback topologyChangeCallback_;
    CoordinatorChangeCallback coordinatorChangeCallback_;
    
    // Private methods - Topology algorithms
    bool buildStarTopology();
    bool buildMeshTopology();
    bool buildHierarchicalTopology();
    bool buildHybridTopology();
    
    // Private methods - Route calculation
    std::vector<String> dijkstraShortestPath(const String& source, const String& destination);
    uint32_t calculateRouteCost(const std::vector<String>& path);
    float calculateRouteReliability(const std::vector<String>& path);
    bool isRouteValid(const NetworkRoute& route) const;
    
    // Private methods - Coordinator election
    String electCoordinatorByReliability();
    String electCoordinatorByBattery();
    String electCoordinatorByConnectivity();
    bool validateCoordinatorEligibility(const String& nodeId) const;
    
    // Private methods - Network optimization
    void rebalanceNetwork();
    void pruneUnreliableRoutes();
    void updateNetworkMetrics();
    void detectAndHandleFailures();
    
    // Private methods - Protocol integration
    bool sendTopologyUpdate(const String& nodeId);
    bool handleTopologyUpdate(const String& nodeId, const std::vector<uint8_t>& data);
    void broadcastTopologyChange();
    
    // Private methods - Utilities
    uint32_t getCurrentTimestamp() const;
    String generateRouteKey(const String& source, const String& destination) const;
    void logTopologyEvent(const String& event, const String& details = "");
    bool validateNodeId(const String& nodeId) const;
};

// Global instance
extern NetworkTopologyManager* g_networkTopologyManager;

// Utility functions
bool initializeNetworkTopology(const TopologyConfig& config);
void cleanupNetworkTopology();
bool discoverFederatedNodes();
String getCurrentFederatedCoordinator();
bool isNetworkTopologyReady();
NetworkMetrics getCurrentNetworkMetrics();

#endif // NETWORK_TOPOLOGY_MANAGER_H