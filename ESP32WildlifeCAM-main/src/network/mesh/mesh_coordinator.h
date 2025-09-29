/**
 * @file mesh_coordinator.h
 * @brief Central Mesh Network Coordinator Header
 * 
 * Defines the coordinator class for managing mesh network topology,
 * load balancing, and wildlife-specific optimizations.
 */

#ifndef MESH_COORDINATOR_H
#define MESH_COORDINATOR_H

#include "../../include/network/mesh_config.h"
#include <vector>
#include <map>
#include <Arduino.h>

// ===========================
// FORWARD DECLARATIONS
// ===========================

struct NodeInfo;
struct RouteEntry;
struct TopologyUpdatePacket;
struct MeshStatistics;

// ===========================
// DATA STRUCTURES
// ===========================

/**
 * @brief Information about a mesh network node
 */
struct NodeInfo {
    uint32_t nodeId;                // Unique node identifier
    MeshRole role;                  // Node role in mesh
    int16_t signalStrength;         // RSSI value
    uint8_t hopCount;               // Hops from coordinator
    unsigned long lastSeen;         // Last communication timestamp
    bool isActive;                  // Node activity status
    bool hasWildlifeDetection;      // Current wildlife detection status
    float batteryLevel;             // Battery percentage (0.0-1.0)
    uint32_t dataTransmitted;       // Total data transmitted (bytes)
    uint32_t packetsLost;           // Number of lost packets
};

/**
 * @brief Routing table entry
 */
struct RouteEntry {
    uint32_t destination;           // Destination node ID
    uint32_t nextHop;               // Next hop node ID
    uint8_t hopCount;               // Number of hops to destination
    float utilization;              // Link utilization (0.0-1.0)
    unsigned long lastUsed;         // Last time route was used
    float reliability;              // Route reliability score (0.0-1.0)
};

/**
 * @brief Topology update packet structure
 */
struct TopologyUpdatePacket {
    uint32_t coordinatorId;         // Coordinator node ID
    unsigned long timestamp;        // Update timestamp
    float networkHealth;            // Current network health score
    uint16_t nodeCount;             // Number of active nodes
    uint8_t emergencyFlags;         // Emergency status flags
};

/**
 * @brief Mesh network statistics
 */
struct MeshStatistics {
    unsigned long startTime;        // System start time
    unsigned long uptime;           // Total uptime in milliseconds
    uint32_t packetsRouted;         // Total packets routed
    uint32_t nodesManaged;          // Total nodes managed
    uint32_t emergencyEvents;       // Number of emergency events
    uint32_t routeOptimizations;    // Number of route optimizations
    float networkHealth;            // Current network health
    uint32_t activeNodes;           // Currently active nodes
    uint32_t wildlifeDetections;    // Total wildlife detections processed
};

// ===========================
// MESH COORDINATOR CLASS
// ===========================

/**
 * @brief Central mesh network coordinator for wildlife monitoring
 * 
 * Manages network topology, implements load balancing, handles emergency
 * situations, and optimizes data flow for wildlife monitoring applications.
 */
class MeshCoordinator {
public:
    // Constructor and destructor
    MeshCoordinator();
    ~MeshCoordinator();
    
    // Initialization and cleanup
    bool initialize(uint32_t nodeId, const MeshConfig& config);
    void cleanup();
    
    // Main coordination processing
    void processCoordination();
    
    // Network topology management
    void updateNetworkTopology();
    void calculateNetworkHealth();
    bool addNode(const NodeInfo& node);
    bool removeNode(uint32_t nodeId);
    std::vector<NodeInfo> getActiveNodes() const;
    
    // Load balancing
    void performLoadBalancing();
    uint32_t findAlternativePath(uint32_t source, uint32_t destination);
    void redistributeTraffic(uint32_t overloadedPath, uint32_t alternativePath);
    
    // Emergency handling
    void enterEmergencyMode();
    void exitEmergencyMode();
    void handleEmergencyCoordination();
    
    // Routing management
    void recalculateRoutes();
    void removeRoutesForNode(uint32_t nodeId);
    void broadcastTopologyUpdate();
    
    // Status and statistics
    bool isInitialized() const;
    MeshStatistics getStatistics() const;
    float getNetworkHealth() const;
    uint32_t getTotalNodes() const;
    bool isEmergencyMode() const { return emergencyMode_; }
    
    // Wildlife-specific methods
    void reportWildlifeDetection(uint32_t nodeId, uint8_t priority);
    void prioritizeWildlifeData(uint32_t nodeId);
    void handleImageTransmission(uint32_t sourceNode, size_t imageSize);

private:
    // Configuration and state
    uint32_t nodeId_;
    MeshRole role_;
    MeshConfig config_;
    bool emergencyMode_;
    
    // Network topology
    std::vector<NodeInfo> topology_;
    std::map<uint32_t, RouteEntry> routingTable_;
    std::vector<uint32_t> activeConnections_;
    
    // Network metrics
    float networkHealth_;
    uint32_t totalNodes_;
    unsigned long lastTopologyUpdate_;
    
    // Statistics
    MeshStatistics statistics_;
    
    // Private helper methods
    void updateStatistics();
    void increasePowerOutput();
    void restoreNormalPowerOutput();
    void adjustPriorityScheduling(bool emergencyMode);
    void attemptNodeReconnection(uint32_t nodeId);
    void sendTopologyUpdate(uint32_t nodeId, const TopologyUpdatePacket& update);
    
    // Wildlife-specific helpers
    void optimizeForWildlifeData();
    void handleHighPriorityDetection(uint32_t nodeId);
    void manageImageTransmissionBandwidth();
};

// ===========================
// INLINE IMPLEMENTATIONS
// ===========================

inline void MeshCoordinator::reportWildlifeDetection(uint32_t nodeId, uint8_t priority) {
    statistics_.wildlifeDetections++;
    
    // Find node and update status
    for (auto& node : topology_) {
        if (node.nodeId == nodeId) {
            node.hasWildlifeDetection = true;
            break;
        }
    }
    
    // Handle high-priority detections
    if (priority >= MESH_PRIORITY_HIGH) {
        handleHighPriorityDetection(nodeId);
    }
}

inline void MeshCoordinator::prioritizeWildlifeData(uint32_t nodeId) {
    // Increase route priority for wildlife data from this node
    auto it = routingTable_.find(nodeId);
    if (it != routingTable_.end()) {
        it->second.reliability = 1.0;  // Maximum reliability
    }
}

inline void MeshCoordinator::handleImageTransmission(uint32_t sourceNode, size_t imageSize) {
    // Optimize network for large image transmission
    manageImageTransmissionBandwidth();
    
    // Update statistics
    auto it = std::find_if(topology_.begin(), topology_.end(),
                          [sourceNode](const NodeInfo& node) { return node.nodeId == sourceNode; });
    if (it != topology_.end()) {
        it->dataTransmitted += imageSize;
    }
}

#endif // MESH_COORDINATOR_H