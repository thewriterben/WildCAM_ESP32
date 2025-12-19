/**
 * @file MeshManager.h
 * @brief LoRa Mesh Network Manager for WildCAM ESP32
 * @version 1.0.0
 * 
 * Provides Meshtastic/MeshCore compatible mesh networking for long-range
 * wildlife camera communication. Supports multi-hop routing, node discovery,
 * and encrypted data transmission.
 * 
 * Features:
 * - Automatic mesh network formation
 * - Multi-hop routing with Dijkstra's algorithm
 * - Node discovery and heartbeat
 * - AES-256 encrypted communication
 * - Wildlife detection alert broadcasting
 * - Image transmission with chunking
 * - Coordinator election and failover
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include <Arduino.h>
#include <vector>

//==============================================================================
// MESH NETWORK STRUCTURES
//==============================================================================

/**
 * @brief Signal strength classification
 */
enum SignalStrength {
    SIGNAL_POOR = 0,        ///< RSSI < -100 dBm
    SIGNAL_FAIR = 1,        ///< RSSI -100 to -85 dBm
    SIGNAL_GOOD = 2,        ///< RSSI -85 to -70 dBm
    SIGNAL_EXCELLENT = 3    ///< RSSI > -70 dBm
};

/**
 * @brief Mesh node role in the network
 */
enum MeshRole {
    MESH_ROLE_NODE = 0,         ///< Regular mesh node
    MESH_ROLE_COORDINATOR = 1,  ///< Network coordinator
    MESH_ROLE_GATEWAY = 2,      ///< Gateway to external networks (WiFi/Internet)
    MESH_ROLE_REPEATER = 3      ///< Dedicated relay node
};

/**
 * @brief Mesh message priority levels
 */
enum MeshPriority {
    PRIORITY_LOW = 0,           ///< Background telemetry
    PRIORITY_NORMAL = 1,        ///< Regular messages
    PRIORITY_HIGH = 2,          ///< Wildlife detection alerts
    PRIORITY_EMERGENCY = 3      ///< System emergencies
};

/**
 * @brief Mesh packet types
 */
enum MeshPacketType {
    PACKET_BEACON = 0x01,       ///< Network discovery beacon
    PACKET_DATA = 0x02,         ///< Regular data packet
    PACKET_ACK = 0x03,          ///< Acknowledgment
    PACKET_ROUTING = 0x04,      ///< Routing information
    PACKET_WILDLIFE = 0x05,     ///< Wildlife detection alert
    PACKET_IMAGE = 0x06,        ///< Image data chunk
    PACKET_TELEMETRY = 0x07,    ///< Environmental telemetry
    PACKET_EMERGENCY = 0x08     ///< Emergency alert
};

/**
 * @brief Signal quality information
 */
struct SignalQuality {
    int16_t rssi;               ///< Received Signal Strength Indicator (dBm)
    float snr;                  ///< Signal-to-Noise Ratio (dB)
    SignalStrength strength;    ///< Classified signal strength
};

/**
 * @brief Information about a mesh network node
 */
struct MeshNode {
    uint32_t nodeId;            ///< Unique node identifier
    String shortName;           ///< Short display name
    float latitude;             ///< GPS latitude (if available)
    float longitude;            ///< GPS longitude (if available)
    uint8_t batteryLevel;       ///< Battery percentage (0-100)
    int16_t lastRssi;           ///< Last received RSSI
    float lastSnr;              ///< Last received SNR
    uint8_t hopCount;           ///< Hops to reach this node
    uint32_t lastSeen;          ///< Last seen timestamp (millis)
    MeshRole role;              ///< Node's role in network
    bool isOnline;              ///< Currently reachable
};

/**
 * @brief Routing table entry
 */
struct MeshRoute {
    uint32_t destination;       ///< Destination node ID
    uint32_t nextHop;           ///< Next hop node ID
    uint8_t hopCount;           ///< Total hops to destination
    float reliability;          ///< Route reliability (0.0-1.0)
    uint32_t lastUsed;          ///< Last usage timestamp
};

/**
 * @brief Mesh network status information
 */
struct MeshNetworkStatus {
    bool initialized;           ///< Network initialized
    uint32_t nodeId;            ///< This node's ID
    MeshRole role;              ///< This node's role
    uint16_t connectedNodes;    ///< Number of known nodes
    uint32_t coordinatorId;     ///< Current coordinator ID
    bool isCoordinator;         ///< Is this node the coordinator
    int16_t lastRssi;           ///< Last received RSSI
    float lastSnr;              ///< Last received SNR
    uint32_t packetsReceived;   ///< Total packets received
    uint32_t packetsSent;       ///< Total packets sent
    uint32_t packetsForwarded;  ///< Total packets forwarded
    uint32_t lastBeaconTime;    ///< Last beacon timestamp
    uint32_t lastMessageTime;   ///< Last message timestamp
};

/**
 * @brief Wildlife detection event for mesh transmission
 */
struct WildlifeEvent {
    uint32_t timestamp;         ///< Detection timestamp
    uint32_t nodeId;            ///< Originating node
    String species;             ///< Detected species (if known)
    float confidence;           ///< Detection confidence (0.0-1.0)
    float latitude;             ///< GPS latitude
    float longitude;            ///< GPS longitude
    uint32_t imageSize;         ///< Size of associated image
    bool hasImage;              ///< Image available flag
};

/**
 * @brief Callback function types
 */
typedef void (*MeshMessageCallback)(uint32_t fromNode, const uint8_t* data, size_t length);
typedef void (*MeshWildlifeCallback)(const WildlifeEvent& event);
typedef void (*MeshNodeCallback)(const MeshNode& node, bool isNew);

//==============================================================================
// MESH MANAGER CLASS
//==============================================================================

/**
 * @class MeshManager
 * @brief LoRa mesh network manager for wildlife cameras
 * 
 * Provides mesh networking capabilities for distributed wildlife monitoring.
 * Implements Meshtastic-compatible protocol for interoperability.
 */
class MeshManager {
public:
    /**
     * @brief Constructor
     */
    MeshManager();
    
    /**
     * @brief Destructor
     */
    ~MeshManager();
    
    //==========================================================================
    // INITIALIZATION
    //==========================================================================
    
    /**
     * @brief Initialize the mesh network
     * @param nodeId Unique node ID (0 = auto-generate from MAC)
     * @return true if initialization successful
     */
    bool init(uint32_t nodeId = 0);
    
    /**
     * @brief Check if mesh is initialized
     * @return true if mesh is ready
     */
    bool isInitialized() const;
    
    /**
     * @brief Cleanup mesh network resources
     */
    void cleanup();
    
    //==========================================================================
    // NODE CONFIGURATION
    //==========================================================================
    
    /**
     * @brief Set this node's ID
     * @param nodeId Unique node identifier
     */
    void setNodeId(uint32_t nodeId);
    
    /**
     * @brief Get this node's ID
     * @return Node ID
     */
    uint32_t getNodeId() const;
    
    /**
     * @brief Set this node's short name
     * @param name Short display name (max 8 chars)
     */
    void setNodeName(const String& name);
    
    /**
     * @brief Get this node's short name
     * @return Node name
     */
    String getNodeName() const;
    
    /**
     * @brief Set node's GPS position
     * @param latitude GPS latitude
     * @param longitude GPS longitude
     */
    void setPosition(float latitude, float longitude);
    
    //==========================================================================
    // COORDINATOR FUNCTIONS
    //==========================================================================
    
    /**
     * @brief Become network coordinator
     * @return true if successfully became coordinator
     */
    bool becomeCoordinator();
    
    /**
     * @brief Check if this node is coordinator
     * @return true if node is coordinator
     */
    bool isCoordinator() const;
    
    /**
     * @brief Get coordinator node ID
     * @return Coordinator ID, 0 if none
     */
    uint32_t getCoordinatorId() const;
    
    //==========================================================================
    // MESSAGE TRANSMISSION
    //==========================================================================
    
    /**
     * @brief Send data to specific node
     * @param destination Destination node ID
     * @param data Data to send
     * @param length Data length
     * @param priority Message priority
     * @return true if queued successfully
     */
    bool sendMessage(uint32_t destination, const uint8_t* data, size_t length,
                    MeshPriority priority = PRIORITY_NORMAL);
    
    /**
     * @brief Broadcast data to all nodes
     * @param data Data to broadcast
     * @param length Data length
     * @param priority Message priority
     * @return true if queued successfully
     */
    bool broadcastMessage(const uint8_t* data, size_t length,
                         MeshPriority priority = PRIORITY_NORMAL);
    
    /**
     * @brief Send text message
     * @param destination Destination node ID (0 = broadcast)
     * @param message Text message
     * @return true if queued successfully
     */
    bool sendText(uint32_t destination, const String& message);
    
    /**
     * @brief Send wildlife detection event
     * @param event Wildlife detection event
     * @return true if queued successfully
     */
    bool sendWildlifeEvent(const WildlifeEvent& event);
    
    /**
     * @brief Send image data (chunked transmission)
     * @param imageData Pointer to image data
     * @param imageSize Size of image in bytes
     * @param filename Image filename
     * @return true if transmission started
     */
    bool sendImage(const uint8_t* imageData, size_t imageSize, const String& filename);
    
    //==========================================================================
    // MESSAGE PROCESSING
    //==========================================================================
    
    /**
     * @brief Process incoming messages
     * @note Call this regularly in main loop
     */
    void process();
    
    /**
     * @brief Check if messages are available
     * @return true if messages pending
     */
    bool hasMessages() const;
    
    //==========================================================================
    // NETWORK STATUS
    //==========================================================================
    
    /**
     * @brief Get mesh network status
     * @return Network status structure
     */
    MeshNetworkStatus getNetworkStatus() const;
    
    /**
     * @brief Get signal quality
     * @return Signal quality information
     */
    SignalQuality getSignalQuality() const;
    
    /**
     * @brief Get list of known nodes
     * @return Vector of known mesh nodes
     */
    std::vector<MeshNode> getNodes() const;
    
    /**
     * @brief Get specific node info
     * @param nodeId Node ID to query
     * @return Node information (empty if not found)
     */
    MeshNode getNode(uint32_t nodeId) const;
    
    /**
     * @brief Get number of connected nodes
     * @return Number of online nodes
     */
    uint16_t getNodeCount() const;
    
    //==========================================================================
    // ROUTING
    //==========================================================================
    
    /**
     * @brief Get routing table
     * @return Vector of routing entries
     */
    std::vector<MeshRoute> getRoutingTable() const;
    
    /**
     * @brief Get next hop for destination
     * @param destination Destination node ID
     * @return Next hop node ID, 0 if no route
     */
    uint32_t getNextHop(uint32_t destination) const;
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    /**
     * @brief Set callback for received messages
     * @param callback Function to call when message received
     */
    void setMessageCallback(MeshMessageCallback callback);
    
    /**
     * @brief Set callback for wildlife events
     * @param callback Function to call when wildlife event received
     */
    void setWildlifeCallback(MeshWildlifeCallback callback);
    
    /**
     * @brief Set callback for node discovery
     * @param callback Function to call when node discovered/updated
     */
    void setNodeCallback(MeshNodeCallback callback);
    
    //==========================================================================
    // POWER MANAGEMENT
    //==========================================================================
    
    /**
     * @brief Enter low power mode
     * @note Reduces beacon frequency and TX power
     */
    void enterLowPowerMode();
    
    /**
     * @brief Exit low power mode
     */
    void exitLowPowerMode();
    
    /**
     * @brief Check if in low power mode
     * @return true if in low power mode
     */
    bool isLowPowerMode() const;

private:
    // Internal state
    bool _initialized;
    uint32_t _nodeId;
    String _nodeName;
    MeshRole _role;
    float _latitude;
    float _longitude;
    bool _lowPowerMode;
    
    // Network state
    uint32_t _coordinatorId;
    uint32_t _lastBeaconTime;
    uint32_t _lastMaintenanceTime;
    
    // Statistics
    uint32_t _packetsReceived;
    uint32_t _packetsSent;
    uint32_t _packetsForwarded;
    
    // Signal quality
    int16_t _lastRssi;
    float _lastSnr;
    
    // Node tracking
    std::vector<MeshNode> _nodes;
    std::vector<MeshRoute> _routes;
    
    // Callbacks
    MeshMessageCallback _messageCallback;
    MeshWildlifeCallback _wildlifeCallback;
    MeshNodeCallback _nodeCallback;
    
    // Internal methods
    bool initRadio();
    void sendBeacon();
    void processBeacon(const uint8_t* data, size_t length, int16_t rssi, float snr);
    void updateRouting();
    void cleanupExpiredNodes();
    uint32_t generateNodeId();
    SignalStrength classifySignal(int16_t rssi) const;
    void addOrUpdateNode(const MeshNode& node);
    bool forwardPacket(const uint8_t* data, size_t length);
};

// Global mesh manager instance
extern MeshManager meshManager;

#endif // MESH_MANAGER_H
