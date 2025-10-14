/**
 * @file mesh_interface.h
 * @brief Meshtastic Protocol Interface Header for ESP32WildlifeCAM
 * 
 * Implements the core Meshtastic protocol stack including:
 * - Node discovery and routing
 * - Packet encryption and security
 * - Channel management
 * - MQTT bridging capability
 */

#ifndef MESH_INTERFACE_H
#define MESH_INTERFACE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "mesh_config.h"
#include "lora_driver.h"

// ===========================
// PROTOCOL STRUCTURES
// ===========================

// Meshtastic packet types
enum MeshPacketType {
    PACKET_TYPE_DATA = 0,
    PACKET_TYPE_ROUTING = 1,
    PACKET_TYPE_ADMIN = 2,
    PACKET_TYPE_TELEMETRY = 3,
    PACKET_TYPE_POSITION = 4,
    PACKET_TYPE_TEXT = 5,
    PACKET_TYPE_WILDLIFE = 6,   // Custom wildlife packet type
    PACKET_TYPE_IMAGE = 7       // Image transmission packet type
};

// Routing priorities
enum RoutingPriority {
    PRIORITY_BACKGROUND = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_EMERGENCY = 3
};

// Node status
enum NodeStatus {
    NODE_STATUS_UNKNOWN = 0,
    NODE_STATUS_ONLINE = 1,
    NODE_STATUS_OFFLINE = 2,
    NODE_STATUS_LOW_BATTERY = 3,
    NODE_STATUS_MAINTENANCE = 4
};

// Mesh packet header structure
struct MeshPacketHeader {
    uint32_t id;                    // Packet ID
    uint32_t from;                  // Source node ID
    uint32_t to;                    // Destination node ID (0 = broadcast)
    uint8_t hopLimit;               // Remaining hops
    uint8_t hopStart;               // Original hop limit
    uint8_t channel;                // Channel number
    uint8_t priority;               // Routing priority
    uint32_t rxTime;                // Reception timestamp
    bool wantAck;                   // Request acknowledgment
    uint8_t portNum;                // Application port number
};

// Mesh packet structure
struct MeshPacket {
    MeshPacketHeader header;
    std::vector<uint8_t> payload;
    bool encrypted;
    uint32_t rxRssi;
    float rxSnr;
    unsigned long rxTime;
};

// Node information
struct NodeInfo {
    uint32_t nodeId;
    String shortName;
    String longName;
    String macAddress;
    float latitude;
    float longitude;
    uint32_t altitude;
    NodeStatus status;
    uint32_t lastSeen;
    int16_t lastRssi;
    float lastSnr;
    uint8_t hopCount;
    bool hasPosition;
    uint32_t batteryLevel;
    float voltage;
    uint32_t uptimeSeconds;
};

// Routing table entry
struct RouteInfo {
    uint32_t destination;
    uint32_t nextHop;
    uint8_t hopCount;
    uint32_t lastUsed;
    float reliability;
    int16_t signalStrength;
};

// Channel configuration
struct ChannelSettings {
    uint8_t index;
    String name;
    String psk;                     // Pre-shared key
    uint32_t frequency;
    uint32_t bandwidth;
    uint8_t spreadingFactor;
    uint8_t codingRate;
    bool uplinkEnabled;
    bool downlinkEnabled;
    uint32_t dutyCycle;
};

// Message statistics
struct MeshStatistics {
    uint32_t packetsReceived;
    uint32_t packetsSent;
    uint32_t packetsForwarded;
    uint32_t packetsDropped;
    uint32_t duplicatesDropped;
    uint32_t encryptionErrors;
    uint32_t routingErrors;
    uint32_t ackTimeouts;
    float averageRssi;
    float averageSnr;
    uint32_t networkNodes;
    uint32_t routingTableSize;
};

// ===========================
// MESH INTERFACE CLASS
// ===========================

class MeshInterface {
public:
    MeshInterface();
    ~MeshInterface();
    
    // Initialization and configuration
    bool init(const LoRaConfig& radioConfig);
    bool configure(const ChannelSettings& channelConfig);
    void cleanup();
    
    // Node management
    bool setNodeInfo(const NodeInfo& nodeInfo);
    NodeInfo getNodeInfo() const;
    uint32_t getNodeId() const;
    bool generateNodeId();
    
    // Channel management
    bool addChannel(const ChannelSettings& channel);
    bool removeChannel(uint8_t channelIndex);
    bool setActiveChannel(uint8_t channelIndex);
    ChannelSettings getChannel(uint8_t channelIndex) const;
    std::vector<ChannelSettings> getChannels() const;
    
    // Message transmission
    bool sendMessage(uint32_t destination, MeshPacketType type, 
                    const std::vector<uint8_t>& payload, 
                    RoutingPriority priority = PRIORITY_NORMAL,
                    bool requestAck = false);
    bool broadcastMessage(MeshPacketType type, const std::vector<uint8_t>& payload,
                         RoutingPriority priority = PRIORITY_NORMAL);
    bool sendText(uint32_t destination, const String& text);
    bool sendPosition(uint32_t destination, float lat, float lon, uint32_t alt = 0);
    bool sendTelemetry(uint32_t destination, const JsonObject& telemetryData);
    
    // Message reception
    bool receiveMessage(MeshPacket& packet);
    bool hasMessages() const;
    size_t getMessageQueueSize() const;
    
    // Routing and network topology
    bool addRoute(uint32_t destination, uint32_t nextHop, uint8_t hopCount);
    bool removeRoute(uint32_t destination);
    RouteInfo getRoute(uint32_t destination) const;
    std::vector<RouteInfo> getRoutingTable() const;
    bool updateNeighbors();
    std::vector<NodeInfo> getNeighbors() const;
    std::vector<NodeInfo> getAllNodes() const;
    
    // Network discovery
    bool startNodeDiscovery();
    bool sendBeacon();
    bool requestNodeInfo(uint32_t nodeId);
    
    // Encryption and security
    bool setChannelKey(uint8_t channelIndex, const String& key);
    bool encryptPacket(MeshPacket& packet);
    bool decryptPacket(MeshPacket& packet);
    
    // Power management
    bool enterLowPowerMode();
    bool exitLowPowerMode();
    bool scheduleWakeup(uint32_t intervalMs);
    
    // MQTT bridge functionality
    bool enableMqttBridge(const String& brokerHost, uint16_t port);
    bool disableMqttBridge();
    bool publishToMqtt(const String& topic, const String& payload);
    
    // Statistics and diagnostics
    MeshStatistics getStatistics() const;
    void resetStatistics();
    bool runDiagnostics();
    String getNetworkTopology() const;
    
    // Event callbacks
    typedef void (*MessageCallback)(const MeshPacket& packet);
    typedef void (*NodeDiscoveredCallback)(const NodeInfo& node);
    typedef void (*RouteChangedCallback)(uint32_t destination, uint32_t nextHop);
    typedef void (*NetworkStatusCallback)(bool connected);
    
    void setMessageCallback(MessageCallback callback);
    void setNodeDiscoveredCallback(NodeDiscoveredCallback callback);
    void setRouteChangedCallback(RouteChangedCallback callback);
    void setNetworkStatusCallback(NetworkStatusCallback callback);
    
    // Processing and maintenance
    void process();                 // Call regularly in main loop
    void performMaintenance();      // Periodic maintenance tasks
    
private:
    // Internal state
    bool initialized_;
    LoRaDriver* radioDriver_;
    NodeInfo nodeInfo_;
    uint32_t nodeId_;
    std::vector<ChannelSettings> channels_;
    uint8_t activeChannel_;
    
    // Networking state
    std::map<uint32_t, NodeInfo> nodes_;
    std::map<uint32_t, RouteInfo> routingTable_;
    std::map<uint32_t, unsigned long> lastSeen_;
    std::vector<MeshPacket> messageQueue_;
    std::vector<MeshPacket> outgoingQueue_;
    
    // Packet tracking
    std::map<uint32_t, unsigned long> recentPackets_;
    uint32_t nextPacketId_;
    std::map<uint32_t, unsigned long> pendingAcks_;
    
    // Statistics
    MeshStatistics statistics_;
    
    // Callbacks
    MessageCallback messageCallback_;
    NodeDiscoveredCallback nodeDiscoveredCallback_;
    RouteChangedCallback routeChangedCallback_;
    NetworkStatusCallback networkStatusCallback_;
    
    // Timing
    unsigned long lastBeacon_;
    unsigned long lastMaintenance_;
    unsigned long lastNodeDiscovery_;
    
    // Internal methods
    bool processIncomingPacket(const LoRaPacket& radioPacket);
    bool processOutgoingPackets();
    MeshPacket createPacket(uint32_t destination, MeshPacketType type,
                           const std::vector<uint8_t>& payload,
                           RoutingPriority priority, bool requestAck);
    
    // Routing methods
    uint32_t findNextHop(uint32_t destination);
    bool forwardPacket(MeshPacket& packet);
    bool shouldForwardPacket(const MeshPacket& packet);
    void updateRoutingTable(const MeshPacket& packet);
    void cleanupRoutingTable();
    
    // Packet processing
    bool handleDataPacket(const MeshPacket& packet);
    bool handleRoutingPacket(const MeshPacket& packet);
    bool handleAdminPacket(const MeshPacket& packet);
    bool handleTelemetryPacket(const MeshPacket& packet);
    bool handlePositionPacket(const MeshPacket& packet);
    bool handleTextPacket(const MeshPacket& packet);
    bool handleWildlifePacket(const MeshPacket& packet);
    
    // Node management
    void addOrUpdateNode(const NodeInfo& node);
    void removeExpiredNodes();
    bool isNodeExpired(const NodeInfo& node) const;
    
    // Utility methods
    bool isDuplicatePacket(uint32_t packetId);
    void addToRecentPackets(uint32_t packetId);
    void cleanupRecentPackets();
    std::vector<uint8_t> serializeNodeInfo(const NodeInfo& node);
    NodeInfo deserializeNodeInfo(const std::vector<uint8_t>& data);
    
    // Encryption helpers
    std::vector<uint8_t> generateIV();
    bool deriveKey(const String& psk, std::vector<uint8_t>& key);
    
    // Radio callback
    static void onRadioReceive(LoRaPacket packet);
    static MeshInterface* instance_;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function
MeshInterface* createMeshInterface(const LoRaConfig& radioConfig);

// Helper functions for packet manipulation
std::vector<uint8_t> serializeJson(const JsonObject& json);
JsonObject deserializeJson(const std::vector<uint8_t>& data);
String packetTypeToString(MeshPacketType type);
String priorityToString(RoutingPriority priority);
String nodeStatusToString(NodeStatus status);

// Network utilities
uint32_t generateNodeIdFromMac();
String formatNodeId(uint32_t nodeId);
String formatDuration(uint32_t seconds);
String formatDataSize(uint32_t bytes);

// Validation functions
bool isValidNodeId(uint32_t nodeId);
bool isValidChannelIndex(uint8_t channelIndex);
bool isValidPacketType(MeshPacketType type);

#endif // MESH_INTERFACE_H