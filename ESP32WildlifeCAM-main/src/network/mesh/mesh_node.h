/**
 * @file mesh_node.h
 * @brief Individual Mesh Node Management Header
 * 
 * Defines the node class for individual mesh network participants
 * including discovery, routing, and wildlife-specific operations.
 */

#ifndef MESH_NODE_H
#define MESH_NODE_H

#include "../../include/network/mesh_config.h"
#include <vector>
#include <map>
#include <Arduino.h>

// ===========================
// FORWARD DECLARATIONS
// ===========================

struct NeighborInfo;
struct MeshPacket;
struct BeaconPacket;
struct WildlifeDetection;
struct WildlifePacket;
struct ImageChunkPacket;
struct NodeStatusPacket;
struct RouteInfo;
struct MeshNodeStatistics;

// ===========================
// DATA STRUCTURES
// ===========================

/**
 * @brief Information about a neighboring node
 */
struct NeighborInfo {
    uint32_t nodeId;                // Neighbor node ID
    MeshRole role;                  // Neighbor role
    int16_t signalStrength;         // RSSI value
    unsigned long lastSeen;         // Last communication timestamp
    float batteryLevel;             // Battery level (0.0-1.0)
    bool wildlifeActive;            // Wildlife detection status
    bool isActive;                  // Neighbor activity status
    uint32_t packetsReceived;       // Packets received from this neighbor
    uint32_t packetsLost;           // Packets lost from this neighbor
};

/**
 * @brief Mesh packet structure
 */
struct MeshPacket {
    uint32_t source;                // Source node ID
    uint32_t destination;           // Destination node ID
    uint32_t lastHop;               // Last hop node ID
    MeshPacketType type;            // Packet type
    MeshPriority priority;          // Packet priority
    uint8_t hopCount;               // Current hop count
    uint16_t sequence;              // Sequence number
    unsigned long timestamp;        // Packet timestamp
    size_t dataSize;                // Data payload size
    uint8_t data[MESH_MAX_PACKET_SIZE]; // Packet payload
};

/**
 * @brief Beacon packet for network discovery
 */
struct BeaconPacket {
    uint32_t nodeId;                // Node ID
    MeshRole role;                  // Node role
    float batteryLevel;             // Battery level
    bool wildlifeActive;            // Wildlife detection status
    unsigned long timestamp;        // Beacon timestamp
    int16_t signalStrength;         // Signal strength
    uint8_t capabilities;           // Node capabilities flags
};

/**
 * @brief Wildlife detection data
 */
struct WildlifeDetection {
    uint16_t speciesId;             // Species identifier
    float confidence;               // Detection confidence (0.0-1.0)
    uint16_t x, y, width, height;   // Bounding box coordinates
    unsigned long timestamp;        // Detection timestamp
    uint8_t behavior;               // Behavior classification
    float environmentalScore;       // Environmental context score
};

/**
 * @brief Wildlife detection packet
 */
struct WildlifePacket {
    uint32_t nodeId;                // Reporting node ID
    unsigned long timestamp;        // Report timestamp
    WildlifeDetection detection;    // Detection data
    MeshPriority priority;          // Packet priority
    uint8_t imageAvailable;         // Image data availability flag
};

/**
 * @brief Image chunk packet for transmission
 */
struct ImageChunkPacket {
    uint32_t nodeId;                // Source node ID
    uint16_t chunkId;               // Chunk sequence number
    uint16_t totalChunks;           // Total number of chunks
    uint16_t chunkSize;             // Size of this chunk
    unsigned long timestamp;        // Chunk timestamp
    uint8_t data[MESH_WILDLIFE_IMAGE_CHUNK_SIZE]; // Chunk data
};

/**
 * @brief Node status report packet
 */
struct NodeStatusPacket {
    uint32_t nodeId;                // Node ID
    float batteryLevel;             // Current battery level
    uint8_t neighborCount;          // Number of active neighbors
    bool wildlifeActive;            // Wildlife detection status
    unsigned long timestamp;        // Status timestamp
    uint32_t memoryUsage;           // Memory usage percentage
    int16_t signalStrength;         // Current signal strength
};

/**
 * @brief Routing information entry
 */
struct RouteInfo {
    uint32_t destination;           // Destination node ID
    uint32_t nextHop;               // Next hop node ID
    uint8_t hopCount;               // Number of hops
    unsigned long lastUsed;         // Last usage timestamp
    float reliability;              // Route reliability score
    uint32_t packetsTransmitted;    // Packets sent via this route
    uint32_t packetsLost;           // Packets lost via this route
};

/**
 * @brief Node statistics structure
 */
struct MeshNodeStatistics {
    unsigned long startTime;        // Node start time
    unsigned long uptime;           // Total uptime
    uint32_t packetsTransmitted;    // Total packets transmitted
    uint32_t packetsReceived;       // Total packets received
    uint32_t packetsForwarded;      // Total packets forwarded
    uint32_t packetsDropped;        // Total packets dropped
    uint32_t beaconsSent;           // Beacons transmitted
    uint32_t beaconsReceived;       // Beacons received
    uint32_t wildlifeDetections;    // Wildlife detections reported
    uint32_t imagesTransmitted;     // Images transmitted
    uint32_t routingFailures;       // Failed routing attempts
    uint32_t transmissionFailures;  // Failed transmissions
    uint8_t neighborCount;          // Current neighbor count
    uint8_t routingTableSize;       // Routing table size
    bool isConnected;               // Network connection status
};

// ===========================
// MESH NODE CLASS
// ===========================

/**
 * @brief Individual mesh network node for wildlife monitoring
 * 
 * Implements node-level functionality including network discovery,
 * packet routing, wildlife detection reporting, and power management.
 */
class MeshNode {
public:
    // Constructor and destructor
    MeshNode();
    ~MeshNode();
    
    // Initialization and cleanup
    bool initialize(uint32_t nodeId, const MeshConfig& config);
    void cleanup();
    
    // Main processing
    void processNode();
    void performMaintenance();
    
    // Network discovery
    void sendBeacon();
    void handleBeaconReceived(const BeaconPacket& beacon, int16_t rssi);
    void addOrUpdateNeighbor(const NeighborInfo& neighbor);
    
    // Packet routing
    bool routePacket(const MeshPacket& packet);
    uint32_t findNextHop(uint32_t destination);
    bool forwardPacket(MeshPacket packet, uint32_t nextHop);
    
    // Wildlife-specific operations
    void reportWildlifeDetection(const WildlifeDetection& detection);
    void transmitWildlifeImage(const uint8_t* imageData, size_t imageSize);
    void processWildlifeDetection();
    
    // Power management
    void updateBatteryLevel(float level);
    void enterLowPowerMode();
    void exitLowPowerMode();
    
    // Status and statistics
    bool isInitialized() const;
    MeshNodeStatistics getStatistics() const;
    std::vector<NeighborInfo> getNeighbors() const;
    bool isConnectedToNetwork() const;
    float getBatteryLevel() const;
    uint32_t getNodeId() const { return nodeId_; }
    MeshRole getRole() const { return config_.role; }
    
    // Configuration
    void setRole(MeshRole role) { config_.role = role; }
    void setPriorityThreshold(MeshPriority threshold) { priorityThreshold_ = threshold; }
    void setWildlifeDetectionActive(bool active) { wildlifeDetectionActive_ = active; }

private:
    // Node configuration and state
    uint32_t nodeId_;
    MeshRole role_;
    MeshConfig config_;
    uint32_t coordinatorId_;
    bool isConnected_;
    MeshPriority priorityThreshold_;
    
    // Timing
    unsigned long lastBeacon_;
    unsigned long lastMaintenance_;
    
    // Wildlife monitoring
    bool wildlifeDetectionActive_;
    
    // Power management
    float batteryLevel_;
    
    // Network topology
    std::vector<NeighborInfo> neighbors_;
    std::map<uint32_t, RouteInfo> routingTable_;
    std::vector<MeshPacket> pendingPackets_;
    
    // Statistics
    MeshNodeStatistics statistics_;
    
    // Private helper methods
    void updateStatistics();
    void cleanupExpiredNeighbors();
    void cleanupRoutingTable();
    void cleanupPendingPackets();
    void updateConnectivityStatus();
    void updateRoutingTable();
    bool shouldReportStatus();
    void reportNodeStatus();
    int16_t getCurrentSignalStrength();
    
    // Packet handling
    void processPendingPackets();
    void handleLocalPacket(const MeshPacket& packet);
    void updateRouteStatistics(uint32_t nextHop, bool success);
    
    // Communication helpers
    bool transmitToNeighbor(const MeshPacket& packet, const NeighborInfo& neighbor);
    bool broadcastPacket(MeshPacketType type, const uint8_t* data, size_t size);
    bool sendToCoordinator(MeshPacketType type, const uint8_t* data, size_t size, MeshPriority priority);
    
    // Power management helpers
    void reducePowerOutput();
    void restoreNormalPowerOutput();
};

// ===========================
// INLINE IMPLEMENTATIONS
// ===========================

inline uint32_t MeshNode::getNodeId() const {
    return nodeId_;
}

inline MeshRole MeshNode::getRole() const {
    return config_.role;
}

inline void MeshNode::setRole(MeshRole role) {
    config_.role = role;
}

inline void MeshNode::setPriorityThreshold(MeshPriority threshold) {
    priorityThreshold_ = threshold;
}

inline void MeshNode::setWildlifeDetectionActive(bool active) {
    wildlifeDetectionActive_ = active;
}

#endif // MESH_NODE_H