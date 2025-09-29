/**
 * @file mesh_routing.h
 * @brief Intelligent Packet Routing Header for Wildlife Mesh Network
 * 
 * Defines routing algorithms and data structures for optimized wildlife
 * monitoring mesh network with multi-hop communication and load balancing.
 */

#ifndef MESH_ROUTING_H
#define MESH_ROUTING_H

#include "../../include/network/mesh_config.h"
#include <vector>
#include <map>
#include <functional>
#include <Arduino.h>

// ===========================
// ROUTING CONSTANTS
// ===========================

#define MESH_ROUTING_UPDATE_INTERVAL    60000   // Routing table update interval (1 minute)
#define MESH_OPTIMIZATION_INTERVAL      120000  // Route optimization interval (2 minutes)
#define MESH_ROUTE_DISCOVERY_TIMEOUT    10000   // Route discovery timeout (10 seconds)
#define MESH_ROUTE_CACHE_TIMEOUT        30000   // Route cache timeout (30 seconds)
#define MESH_LINK_QUALITY_TIMEOUT       180000  // Link quality timeout (3 minutes)
#define MESH_IMAGE_TRANSMISSION_TIME    30      // Expected image transmission time (seconds)
#define MESH_RECENT_PACKET_TIMEOUT      60000   // Recent packet tracking timeout (1 minute)
#define MESH_MIN_COMPRESSION_SIZE       64      // Minimum payload size for compression

// ===========================
// FORWARD DECLARATIONS
// ===========================

struct RouteEntry;
struct LinkQuality;
struct RouteDiscoveryPacket;
struct RouteDiscoveryResponse;
struct RouteCacheEntry;
struct MeshRoutingStatistics;

// ===========================
// CALLBACK TYPES
// ===========================

typedef std::function<void(uint32_t destination, uint32_t nextHop, uint8_t hopCount)> RouteChangeCallback;

// ===========================
// DATA STRUCTURES
// ===========================

/**
 * @brief Routing table entry with quality metrics
 */
struct RouteEntry {
    uint32_t destination;           // Destination node ID
    uint32_t nextHop;               // Next hop node ID
    uint8_t hopCount;               // Number of hops to destination
    float metric;                   // Route metric (lower is better)
    unsigned long lastUsed;         // Last time route was used
    float reliability;              // Route reliability (0.0-1.0)
    float utilization;              // Link utilization (0.0-1.0)
    bool wildlifePriority;          // Wildlife detection priority flag
    uint32_t loadBalancePartner;    // Load balancing partner next hop
    float trafficRatio;             // Traffic splitting ratio
    uint32_t packetsTransmitted;    // Total packets sent via this route
    uint32_t packetsLost;           // Total packets lost via this route
};

/**
 * @brief Link quality information for routing decisions
 */
struct LinkQuality {
    uint32_t neighborId;            // Neighbor node ID
    float reliability;              // Link reliability (0.0-1.0)
    float packetLoss;               // Packet loss rate (0.0-1.0)
    int16_t signalStrength;         // Signal strength (RSSI)
    float metric;                   // Calculated link metric
    unsigned long lastUpdate;       // Last quality update timestamp
    bool wildlifePriority;          // Wildlife priority flag
    uint32_t bytesTransmitted;      // Total bytes transmitted
    uint32_t transmissionErrors;    // Total transmission errors
};

/**
 * @brief Route discovery packet structure
 */
struct RouteDiscoveryPacket {
    uint32_t originId;              // Route discovery originator
    uint32_t destinationId;         // Target destination
    uint32_t requestId;             // Unique request identifier
    uint8_t hopCount;               // Current hop count
    unsigned long timestamp;        // Request timestamp
    float pathMetric;               // Accumulated path metric
    uint32_t pathIds[MESH_MAX_HOPS]; // Path node IDs
};

/**
 * @brief Route discovery response structure
 */
struct RouteDiscoveryResponse {
    uint32_t originId;              // Original requester
    uint32_t destinationId;         // Responding destination
    uint32_t nextHopId;             // Next hop for route
    uint32_t requestId;             // Original request ID
    uint8_t hopCount;               // Hop count to destination
    float pathMetric;               // Path metric
    unsigned long timestamp;        // Response timestamp
};

/**
 * @brief Route cache entry for performance optimization
 */
struct RouteCacheEntry {
    uint32_t destination;           // Destination node ID
    uint32_t nextHop;               // Cached next hop
    unsigned long timestamp;        // Cache entry timestamp
    uint32_t accessCount;           // Number of cache accesses
};

/**
 * @brief Routing statistics for monitoring and optimization
 */
struct MeshRoutingStatistics {
    unsigned long startTime;        // Routing system start time
    unsigned long uptime;           // Total uptime
    uint32_t routesCalculated;      // Total route calculations
    uint32_t routeDiscoveries;      // Route discovery attempts
    uint32_t loadBalanceOperations; // Load balancing operations
    uint32_t routingTableSize;      // Current routing table size
    uint32_t linkQualityEntries;    // Link quality table entries
    uint32_t cacheHits;             // Route cache hits
    uint32_t cacheMisses;           // Route cache misses
    float cacheHitRate;             // Cache hit rate percentage
    uint32_t pathOptimizations;     // Path optimization operations
    uint32_t wildlifePriorityRoutes; // Routes with wildlife priority
};

// ===========================
// MESH ROUTING CLASS
// ===========================

/**
 * @brief Intelligent mesh routing system for wildlife monitoring
 * 
 * Implements advanced routing algorithms including Dijkstra's shortest path,
 * load balancing, wildlife-specific optimizations, and adaptive quality metrics.
 */
class MeshRouting {
public:
    // Constructor and destructor
    MeshRouting();
    ~MeshRouting();
    
    // Initialization and cleanup
    bool initialize(uint32_t nodeId, const MeshConfig& config);
    void cleanup();
    
    // Main routing processing
    void processRouting();
    void updateRoutingTable();
    void optimizeRoutes();
    
    // Route discovery
    bool discoverRoute(uint32_t destination);
    void handleRouteDiscoveryRequest(const RouteDiscoveryPacket& request);
    void handleRouteDiscoveryResponse(const RouteDiscoveryResponse& response);
    
    // Route management
    RouteEntry* findRoute(uint32_t destination);
    bool addRoute(const RouteEntry& route);
    bool removeRoute(uint32_t destination);
    uint32_t getNextHop(uint32_t destination);
    
    // Load balancing
    std::vector<uint32_t> identifyCongestedRoutes();
    RouteEntry findAlternativeRoute(uint32_t destination, uint32_t excludeNextHop);
    void implementLoadBalancing(uint32_t destination, RouteEntry& primaryRoute, 
                               const RouteEntry& alternativeRoute);
    
    // Link quality management
    void updateLinkQuality();
    void reportLinkQuality(uint32_t neighborId, float reliability, 
                          float packetLoss, int16_t signalStrength);
    
    // Wildlife-specific optimizations
    void applyWildlifeOptimizations();
    void prioritizeWildlifeRoute(uint32_t destination);
    bool optimizeForImageTransmission(uint32_t destination, size_t imageSize);
    
    // Statistics and monitoring
    MeshRoutingStatistics getStatistics() const;
    size_t getRoutingTableSize() const;
    std::vector<RouteEntry> getAllRoutes() const;
    
    // Configuration
    void setRouteChangeCallback(RouteChangeCallback callback);
    void setWildlifePriorityEnabled(bool enabled) { wildlifePriorityEnabled_ = enabled; }
    void setLoadBalancingEnabled(bool enabled) { config_.loadBalancingEnabled = enabled; }

private:
    // Configuration and state
    bool initialized_;
    uint32_t nodeId_;
    MeshConfig config_;
    bool wildlifePriorityEnabled_;
    
    // Timing
    unsigned long lastTableUpdate_;
    unsigned long lastOptimization_;
    bool routeDiscoveryActive_;
    
    // Routing tables
    std::map<uint32_t, RouteEntry> routingTable_;
    std::map<uint32_t, LinkQuality> linkQualityTable_;
    std::map<uint32_t, RouteCacheEntry> routeCache_;
    
    // Statistics
    MeshRoutingStatistics statistics_;
    
    // Callbacks
    RouteChangeCallback routeChangeCallback_;
    
    // Route discovery tracking
    std::vector<std::pair<uint32_t, uint32_t>> seenDiscoveryRequests_;
    
    // Private helper methods - Algorithm implementations
    void calculateShortestPaths();
    uint32_t findMinimumDistanceNode(const std::vector<uint32_t>& unvisited,
                                    const std::map<uint32_t, float>& distances);
    void updateNeighborDistances(uint32_t currentNode,
                               std::map<uint32_t, float>& distances,
                               std::map<uint32_t, uint32_t>& previous);
    void buildRoutingTableFromPaths(const std::map<uint32_t, float>& distances,
                                  const std::map<uint32_t, uint32_t>& previous);
    
    // Link quality and metrics
    void updateLinkMetric(LinkQuality& quality);
    bool isSignificantQualityChange(const LinkQuality& quality);
    float calculatePathMetric(uint32_t nextHop, uint32_t destination);
    uint8_t estimateHopCount(uint32_t nextHop, uint32_t destination);
    
    // Cache management
    void updateRouteCache();
    void updateRouteCacheEntry(uint32_t destination);
    void removeRouteCacheEntry(uint32_t destination);
    void cleanupExpiredRoutes();
    
    // Route discovery helpers
    uint32_t generateRequestId();
    bool waitForRouteDiscoveryResponse(uint32_t requestId, uint32_t timeout);
    void broadcastRouteDiscovery(const RouteDiscoveryPacket& discovery);
    void sendRouteDiscoveryResponse(const RouteDiscoveryPacket& request);
    void forwardRouteDiscoveryRequest(const RouteDiscoveryPacket& request);
    bool isDiscoveryRequestSeen(uint32_t requestId, uint32_t originId);
    void addSeenDiscoveryRequest(uint32_t requestId, uint32_t originId);
    
    // Wildlife-specific helpers
    bool hasWildlifeDetection(uint32_t nodeId);
    void findHighReliabilityPath(uint32_t destination);
    float calculateRequiredBandwidth(size_t imageSize);
    float estimateAvailableBandwidth(uint32_t nextHop);
    RouteEntry findHighBandwidthRoute(uint32_t destination, float requiredBandwidth);
    
    // Utility methods
    std::vector<uint32_t> getNeighbors(uint32_t nodeId);
    float getLinkMetric(uint32_t from, uint32_t to);
    uint8_t calculateHopCount(uint32_t destination, const std::map<uint32_t, uint32_t>& previous);
    void updateStatistics();
    float calculateCacheHitRate();
};

// ===========================
// INLINE IMPLEMENTATIONS
// ===========================

inline void MeshRouting::setWildlifePriorityEnabled(bool enabled) {
    wildlifePriorityEnabled_ = enabled;
}

inline void MeshRouting::setLoadBalancingEnabled(bool enabled) {
    config_.loadBalancingEnabled = enabled;
}

inline size_t MeshRouting::getRoutingTableSize() const {
    return routingTable_.size();
}

#endif // MESH_ROUTING_H