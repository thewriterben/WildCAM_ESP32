/**
 * @file discovery_protocol.h
 * @brief Board discovery and network topology management
 * 
 * Implements automatic board detection, capability advertisement,
 * and network topology mapping for multi-board coordination.
 */

#ifndef DISCOVERY_PROTOCOL_H
#define DISCOVERY_PROTOCOL_H

#include <Arduino.h>
#include <vector>
#include "message_protocol.h"
#include "../lora_mesh.h"

// Discovery states
enum DiscoveryState {
    DISCOVERY_IDLE = 0,
    DISCOVERY_SCANNING = 1,
    DISCOVERY_ADVERTISING = 2,
    DISCOVERY_COMPLETE = 3
};

// Network topology information
struct NetworkTopology {
    std::vector<NetworkNode> nodes;
    int coordinatorNode;
    unsigned long lastUpdate;
    bool isStable;
};

class DiscoveryProtocol {
public:
    DiscoveryProtocol();
    ~DiscoveryProtocol();
    
    /**
     * Initialize discovery protocol
     */
    bool init(int nodeId, BoardRole preferredRole = ROLE_NODE);
    
    /**
     * Start network discovery process
     */
    bool startDiscovery();
    
    /**
     * Stop discovery process
     */
    void stopDiscovery();
    
    /**
     * Process discovery messages and update topology
     */
    void processMessages();
    
    /**
     * Handle incoming discovery message
     */
    void handleDiscoveryMessage(const MultiboardMessage& msg);
    
    /**
     * Handle topology update message
     */
    void handleTopologyMessage(const MultiboardMessage& msg);
    
    /**
     * Send discovery advertisement
     */
    bool sendDiscoveryAdvertisement();
    
    /**
     * Send topology update to network
     */
    bool sendTopologyUpdate();
    
    /**
     * Get current network topology
     */
    const NetworkTopology& getNetworkTopology() const;
    
    /**
     * Get discovered nodes
     */
    const std::vector<NetworkNode>& getDiscoveredNodes() const;
    
    /**
     * Find coordinator node
     */
    int getCoordinatorNode() const;
    
    /**
     * Check if discovery is complete
     */
    bool isDiscoveryComplete() const;
    
    /**
     * Get discovery state
     */
    DiscoveryState getDiscoveryState() const;
    
    /**
     * Add or update node in topology
     */
    void updateNode(const NetworkNode& node);
    
    /**
     * Remove inactive nodes from topology
     */
    void cleanupInactiveNodes();
    
    /**
     * Get node by ID
     */
    NetworkNode* findNode(int nodeId);
    
    /**
     * Check if node exists in topology
     */
    bool nodeExists(int nodeId) const;
    
    /**
     * Get network statistics
     */
    struct NetworkStats {
        int totalNodes;
        int activeNodes;
        int averageSignalStrength;
        int maxHopCount;
        unsigned long networkAge;
    };
    NetworkStats getNetworkStats() const;
    
    /**
     * Set discovery parameters
     */
    void setDiscoveryInterval(unsigned long interval);
    void setAdvertisementInterval(unsigned long interval);
    void setNodeTimeout(unsigned long timeout);
    
private:
    // Core state
    int nodeId_;
    BoardRole preferredRole_;
    BoardCapabilities capabilities_;
    DiscoveryState state_;
    bool initialized_;
    
    // Timing
    unsigned long discoveryInterval_;
    unsigned long advertisementInterval_;
    unsigned long nodeTimeout_;
    unsigned long lastDiscovery_;
    unsigned long lastAdvertisement_;
    unsigned long lastCleanup_;
    
    // Network topology
    NetworkTopology topology_;
    
    // Internal methods
    void broadcastDiscovery();
    void updateTopology(const DiscoveryMessage& discovery);
    bool isNodeActive(const NetworkNode& node) const;
    void selectCoordinator();
    float calculateNodeScore(const NetworkNode& node) const;
    
    // Constants
    static const unsigned long DEFAULT_DISCOVERY_INTERVAL = 30000;    // 30 seconds
    static const unsigned long DEFAULT_ADVERTISEMENT_INTERVAL = 60000; // 1 minute
    static const unsigned long DEFAULT_NODE_TIMEOUT = 300000;         // 5 minutes
    static const unsigned long CLEANUP_INTERVAL = 120000;             // 2 minutes
};

#endif // DISCOVERY_PROTOCOL_H