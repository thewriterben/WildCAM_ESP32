/**
 * @file mesh_protocol.h
 * @brief Foundational ESP32 mesh networking protocol
 * @version 1.0.0
 * 
 * This file provides the foundational framework for ESP32 mesh communication
 * for multi-device wildlife camera deployments. It integrates with the existing
 * comprehensive mesh system in firmware/src/meshtastic/ while providing a
 * simplified interface for basic mesh operations.
 */

#ifndef MESH_PROTOCOL_H
#define MESH_PROTOCOL_H

#include <stdint.h>
#include <vector>
#include <memory>
#include <functional>

namespace MeshNetworking {

/**
 * @brief Mesh node types for wildlife camera network
 */
enum class NodeType {
    UNKNOWN = 0,
    CAMERA_NODE,      // Standard wildlife camera
    GATEWAY_NODE,     // Data collection gateway
    RELAY_NODE,       // Signal repeater/booster
    SENSOR_NODE,      // Environmental sensors only
    MAINTENANCE_NODE  // Temporary maintenance access
};

/**
 * @brief Message types for mesh communication
 */
enum class MessageType {
    HEARTBEAT = 0,
    IMAGE_DATA,
    DETECTION_ALERT,
    WILDLIFE_DETECTION,   // Wildlife detection results
    STATUS_UPDATE,
    CONFIGURATION,
    MAINTENANCE,
    EMERGENCY
};

/**
 * @brief Mesh network status
 */
enum class NetworkStatus {
    DISCONNECTED = 0,
    CONNECTING,
    CONNECTED,
    ROOT_NODE,
    LEAF_NODE,
    ERROR_STATE
};

/**
 * @brief Mesh message structure
 */
struct MeshMessage {
    uint32_t message_id;
    uint32_t sender_id;
    uint32_t receiver_id;    // 0 = broadcast
    MessageType type;
    uint32_t timestamp;
    uint16_t payload_size;
    uint8_t priority;        // 0-7, higher = more priority
    uint8_t hop_count;       // Number of hops from source
    uint8_t max_hops;        // Maximum allowed hops
    bool requires_ack;
    std::vector<uint8_t> payload;
};

/**
 * @brief Node information structure
 */
struct NodeInfo {
    uint32_t node_id;
    NodeType type;
    int8_t rssi;             // Signal strength
    uint8_t hop_distance;    // Hops to reach this node
    uint32_t last_seen;      // Timestamp of last communication
    bool is_active;
    char node_name[32];
    float battery_level;     // 0.0-1.0
    uint32_t uptime_seconds;
    bool supports_ai;        // Node has AI detection capabilities
};

/**
 * @brief Wildlife detection data for mesh transmission
 */
struct WildlifeDetectionData {
    uint32_t detection_id;
    uint32_t source_node_id;
    uint32_t timestamp;
    uint8_t species_type;    // From SpeciesType enum
    uint8_t confidence_level; // From ConfidenceLevel enum
    uint16_t detection_x, detection_y;
    uint16_t detection_width, detection_height;
    float size_estimate;
    uint8_t priority;        // Detection priority (0-3)
    bool has_image_data;     // Whether associated image is available
};

/**
 * @brief Mesh network configuration
 */
struct MeshConfig {
    uint32_t node_id = 0;              // 0 = auto-generate
    NodeType node_type = NodeType::CAMERA_NODE;
    uint8_t channel = 1;               // WiFi channel 1-13
    uint16_t max_connections = 10;
    uint32_t heartbeat_interval_ms = 30000;
    uint32_t node_timeout_ms = 120000;
    uint8_t max_hop_count = 6;
    bool auto_heal_enabled = true;
    bool encryption_enabled = true;
    char mesh_password[64] = "WildlifeCam2025";
    char node_name[32] = "WildlifeCam";
    
    // Edge processing configuration
    bool prefer_local_processing = true;     // Process locally when possible
    bool enable_offline_operation = true;   // Function without internet
    uint32_t offline_storage_limit_mb = 100; // offline_storage limit in MB
};

/**
 * @brief Callback function types
 */
using MessageCallback = std::function<void(const MeshMessage&)>;
using NodeDiscoveredCallback = std::function<void(const NodeInfo&)>;
using NetworkStatusCallback = std::function<void(NetworkStatus, NetworkStatus)>; // old, new
using ErrorCallback = std::function<void(const char* error_message)>;

/**
 * @brief Main mesh networking class
 * 
 * Provides foundational mesh networking capabilities for wildlife camera deployments.
 * Can be extended with more sophisticated routing and data management features.
 */
class MeshProtocol {
public:
    MeshProtocol();
    ~MeshProtocol();

    /**
     * @brief Initialize mesh networking with configuration
     * @param config Mesh network configuration
     * @return true if initialization successful
     */
    bool initialize(const MeshConfig& config);

    /**
     * @brief Start mesh networking
     * @return true if mesh started successfully
     */
    bool startMesh();

    /**
     * @brief Stop mesh networking
     */
    void stopMesh();

    /**
     * @brief Send message to specific node or broadcast
     * @param message Message to send
     * @return true if message queued successfully
     */
    bool sendMessage(const MeshMessage& message);

    /**
     * @brief Send simple text message
     * @param receiver_id Target node ID (0 = broadcast)
     * @param message Text message
     * @param priority Message priority (0-7)
     * @return true if message sent
     */
    bool sendTextMessage(uint32_t receiver_id, const char* message, uint8_t priority = 3);

    /**
     * @brief Send detection alert to network
     * @param species_detected Species name/type
     * @param confidence Detection confidence 0.0-1.0
     * @param image_data Optional image data
     * @param image_size Size of image data
     * @return true if alert sent
     */
    bool sendDetectionAlert(const char* species_detected, float confidence,
                           const uint8_t* image_data = nullptr, size_t image_size = 0);

    /**
     * @brief Send wildlife detection data to mesh network
     * @param detection_data Wildlife detection information
     * @param priority Message priority (0-7, higher = more urgent)
     * @return true if detection data sent successfully
     */
    bool sendWildlifeDetection(const WildlifeDetectionData& detection_data, uint8_t priority = 5);

    /**
     * @brief Get current network status
     * @return Current network status
     */
    NetworkStatus getNetworkStatus() const { return network_status_; }

    /**
     * @brief Get list of discovered nodes
     * @return Vector of node information
     */
    std::vector<NodeInfo> getDiscoveredNodes() const;

    /**
     * @brief Get our node information
     * @return This node's information
     */
    NodeInfo getNodeInfo() const;

    /**
     * @brief Set callback functions
     */
    void setMessageCallback(MessageCallback callback) { message_callback_ = callback; }
    void setNodeDiscoveredCallback(NodeDiscoveredCallback callback) { node_discovered_callback_ = callback; }
    void setNetworkStatusCallback(NetworkStatusCallback callback) { network_status_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }

    /**
     * @brief Update mesh configuration
     * @param config New configuration
     */
    void updateConfig(const MeshConfig& config);

    /**
     * @brief Get current configuration
     * @return Current mesh configuration
     */
    MeshConfig getConfig() const { return config_; }

    /**
     * @brief Network statistics
     */
    struct NetworkStats {
        uint32_t messages_sent = 0;
        uint32_t messages_received = 0;
        uint32_t messages_forwarded = 0;
        uint32_t connection_failures = 0;
        uint32_t nodes_discovered = 0;
        uint32_t network_resets = 0;
        float average_rssi = 0.0f;
        uint32_t uptime_seconds = 0;
    };

    NetworkStats getNetworkStats() const { return stats_; }
    void resetNetworkStats();

    /**
     * @brief Check if mesh is initialized and running
     */
    bool isRunning() const { return initialized_ && mesh_started_; }

private:
    MeshConfig config_;
    NetworkStatus network_status_;
    NetworkStats stats_;
    bool initialized_;
    bool mesh_started_;
    
    std::vector<NodeInfo> discovered_nodes_;
    uint32_t next_message_id_;
    uint32_t start_time_;
    
    // Callback functions
    MessageCallback message_callback_;
    NodeDiscoveredCallback node_discovered_callback_;
    NetworkStatusCallback network_status_callback_;
    ErrorCallback error_callback_;
    
    // Internal mesh management
    void updateNetworkStatus(NetworkStatus new_status);
    bool processIncomingMessage(const uint8_t* data, size_t data_size, int8_t rssi);
    void handleHeartbeat();
    void handleNodeDiscovery();
    void updateNodeList(const NodeInfo& node);
    uint32_t generateNodeId();
    void startHeartbeatTimer();
    void stopHeartbeatTimer();
    
    // Message handling
    bool validateMessage(const MeshMessage& message);
    std::vector<uint8_t> serializeMessage(const MeshMessage& message);
    MeshMessage deserializeMessage(const uint8_t* data, size_t data_size);
};

} // namespace MeshNetworking

#endif // MESH_PROTOCOL_H