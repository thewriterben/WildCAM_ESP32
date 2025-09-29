/**
 * @file mesh_protocol.cpp
 * @brief Implementation of foundational ESP32 mesh networking protocol
 * @version 1.0.0
 */

#include "mesh_protocol.h"
#include <cstring>
#include <algorithm>
#include <random>

namespace MeshNetworking {

MeshProtocol::MeshProtocol() 
    : network_status_(NetworkStatus::DISCONNECTED), 
      initialized_(false), 
      mesh_started_(false),
      next_message_id_(1),
      start_time_(0) {
    // Initialize statistics
    memset(&stats_, 0, sizeof(NetworkStats));
}

MeshProtocol::~MeshProtocol() {
    stopMesh();
}

bool MeshProtocol::initialize(const MeshConfig& config) {
    config_ = config;
    
    // Validate configuration
    if (config_.channel < 1 || config_.channel > 13) {
        if (error_callback_) {
            error_callback_("Invalid WiFi channel. Must be 1-13.");
        }
        return false;
    }
    
    if (config_.max_connections == 0 || config_.max_connections > 20) {
        if (error_callback_) {
            error_callback_("Invalid max connections. Must be 1-20.");
        }
        return false;
    }
    
    // Generate node ID if not provided
    if (config_.node_id == 0) {
        config_.node_id = generateNodeId();
    }
    
    // Initialize network status
    updateNetworkStatus(NetworkStatus::DISCONNECTED);
    
    // Reset statistics
    resetNetworkStats();
    
    initialized_ = true;
    return true;
}

bool MeshProtocol::startMesh() {
    if (!initialized_) {
        if (error_callback_) {
            error_callback_("Mesh not initialized. Call initialize() first.");
        }
        return false;
    }
    
    if (mesh_started_) {
        return true;  // Already started
    }
    
    updateNetworkStatus(NetworkStatus::CONNECTING);
    
    // In a full implementation, this would:
    // 1. Initialize ESP32 WiFi mesh
    // 2. Set up event handlers
    // 3. Start mesh networking stack
    // 4. Begin node discovery
    
    // For foundational framework, simulate successful start
    start_time_ = getTimestamp();
    startHeartbeatTimer();
    
    mesh_started_ = true;
    updateNetworkStatus(NetworkStatus::CONNECTED);
    
    if (error_callback_) {
        error_callback_("Mesh networking started successfully");
    }
    
    return true;
}

void MeshProtocol::stopMesh() {
    if (!mesh_started_) {
        return;
    }
    
    stopHeartbeatTimer();
    
    // In a full implementation, this would:
    // 1. Send disconnect messages to neighbors
    // 2. Clean up WiFi mesh resources
    // 3. Stop all timers and tasks
    
    mesh_started_ = false;
    updateNetworkStatus(NetworkStatus::DISCONNECTED);
    discovered_nodes_.clear();
}

bool MeshProtocol::sendMessage(const MeshMessage& message) {
    if (!mesh_started_) {
        return false;
    }
    
    // Validate message
    if (!validateMessage(message)) {
        return false;
    }
    
    // Serialize message for transmission
    auto serialized = serializeMessage(message);
    
    // In a full implementation, this would:
    // 1. Route the message through the mesh network
    // 2. Handle retransmissions if needed
    // 3. Update routing tables
    
    // For foundational framework, simulate successful send
    stats_.messages_sent++;
    
    return true;
}

bool MeshProtocol::sendTextMessage(uint32_t receiver_id, const char* message, uint8_t priority) {
    if (!message || strlen(message) == 0) {
        return false;
    }
    
    MeshMessage mesh_msg;
    mesh_msg.message_id = next_message_id_++;
    mesh_msg.sender_id = config_.node_id;
    mesh_msg.receiver_id = receiver_id;
    mesh_msg.type = MessageType::STATUS_UPDATE;
    mesh_msg.timestamp = getTimestamp();
    mesh_msg.priority = std::min(priority, static_cast<uint8_t>(7));
    mesh_msg.hop_count = 0;
    mesh_msg.max_hops = config_.max_hop_count;
    mesh_msg.requires_ack = (receiver_id != 0);  // Require ACK for unicast
    
    // Copy message text to payload
    size_t msg_len = strlen(message);
    mesh_msg.payload.resize(msg_len);
    memcpy(mesh_msg.payload.data(), message, msg_len);
    mesh_msg.payload_size = msg_len;
    
    return sendMessage(mesh_msg);
}

bool MeshProtocol::sendDetectionAlert(const char* species_detected, float confidence,
                                     const uint8_t* image_data, size_t image_size) {
    if (!species_detected) {
        return false;
    }
    
    MeshMessage alert_msg;
    alert_msg.message_id = next_message_id_++;
    alert_msg.sender_id = config_.node_id;
    alert_msg.receiver_id = 0;  // Broadcast to all nodes
    alert_msg.type = MessageType::DETECTION_ALERT;
    alert_msg.timestamp = getTimestamp();
    alert_msg.priority = 6;  // High priority for detection alerts
    alert_msg.hop_count = 0;
    alert_msg.max_hops = config_.max_hop_count;
    alert_msg.requires_ack = false;  // Broadcast doesn't require ACK
    
    // Create payload: species_name|confidence|image_data
    std::string payload_str = std::string(species_detected) + "|" + std::to_string(confidence);
    
    alert_msg.payload.resize(payload_str.length() + (image_data ? image_size : 0));
    memcpy(alert_msg.payload.data(), payload_str.c_str(), payload_str.length());
    
    if (image_data && image_size > 0) {
        memcpy(alert_msg.payload.data() + payload_str.length(), image_data, image_size);
    }
    
    alert_msg.payload_size = alert_msg.payload.size();
    
    return sendMessage(alert_msg);
}

bool MeshProtocol::sendWildlifeDetection(const WildlifeDetectionData& detection_data, uint8_t priority) {
    MeshMessage detection_msg;
    detection_msg.message_id = next_message_id_++;
    detection_msg.sender_id = config_.node_id;
    detection_msg.receiver_id = 0;  // Broadcast to all nodes
    detection_msg.type = MessageType::WILDLIFE_DETECTION;
    detection_msg.timestamp = getTimestamp();
    detection_msg.priority = priority;
    detection_msg.hop_count = 0;
    detection_msg.max_hops = config_.max_hop_count;
    detection_msg.requires_ack = false;  // Broadcast doesn't require ACK
    
    // Serialize wildlife detection data as binary payload
    detection_msg.payload.resize(sizeof(WildlifeDetectionData));
    memcpy(detection_msg.payload.data(), &detection_data, sizeof(WildlifeDetectionData));
    detection_msg.payload_size = detection_msg.payload.size();
    
    bool success = sendMessage(detection_msg);
    
    if (success) {
        // Update statistics for wildlife detection messages
        stats_.messages_sent++;
        stats_.bytes_sent += detection_msg.payload_size;
    }
    
    return success;
}

std::vector<NodeInfo> MeshProtocol::getDiscoveredNodes() const {
    return discovered_nodes_;
}

NodeInfo MeshProtocol::getNodeInfo() const {
    NodeInfo info;
    info.node_id = config_.node_id;
    info.type = config_.node_type;
    info.rssi = 0;  // Self
    info.hop_distance = 0;  // Self
    info.last_seen = getTimestamp();
    info.is_active = mesh_started_;
    strncpy(info.node_name, config_.node_name, sizeof(info.node_name) - 1);
    info.node_name[sizeof(info.node_name) - 1] = '\0';
    info.battery_level = 0.75f;  // Placeholder
    info.uptime_seconds = start_time_ > 0 ? (getTimestamp() - start_time_) / 1000 : 0;
    
    return info;
}

void MeshProtocol::updateConfig(const MeshConfig& config) {
    bool was_running = mesh_started_;
    
    if (was_running) {
        stopMesh();
    }
    
    config_ = config;
    
    if (was_running) {
        startMesh();
    }
}

void MeshProtocol::resetNetworkStats() {
    memset(&stats_, 0, sizeof(NetworkStats));
    stats_.uptime_seconds = 0;
}

void MeshProtocol::updateNetworkStatus(NetworkStatus new_status) {
    NetworkStatus old_status = network_status_;
    network_status_ = new_status;
    
    if (network_status_callback_ && old_status != new_status) {
        network_status_callback_(old_status, new_status);
    }
}

bool MeshProtocol::processIncomingMessage(const uint8_t* data, size_t data_size, int8_t rssi) {
    if (!data || data_size == 0) {
        return false;
    }
    
    // Deserialize message
    MeshMessage message = deserializeMessage(data, data_size);
    
    // Update statistics
    stats_.messages_received++;
    stats_.average_rssi = (stats_.average_rssi + rssi) / 2.0f;
    
    // Process message based on type
    if (message_callback_) {
        message_callback_(message);
    }
    
    return true;
}

void MeshProtocol::handleHeartbeat() {
    if (!mesh_started_) {
        return;
    }
    
    // Send heartbeat message
    NodeInfo self_info = getNodeInfo();
    
    MeshMessage heartbeat;
    heartbeat.message_id = next_message_id_++;
    heartbeat.sender_id = config_.node_id;
    heartbeat.receiver_id = 0;  // Broadcast
    heartbeat.type = MessageType::HEARTBEAT;
    heartbeat.timestamp = getTimestamp();
    heartbeat.priority = 1;  // Low priority
    heartbeat.hop_count = 0;
    heartbeat.max_hops = 2;  // Limit heartbeat propagation
    heartbeat.requires_ack = false;
    
    // Serialize node info as payload
    heartbeat.payload.resize(sizeof(NodeInfo));
    memcpy(heartbeat.payload.data(), &self_info, sizeof(NodeInfo));
    heartbeat.payload_size = sizeof(NodeInfo);
    
    sendMessage(heartbeat);
}

void MeshProtocol::handleNodeDiscovery() {
    // Clean up stale nodes
    uint32_t current_time = getTimestamp();
    auto it = discovered_nodes_.begin();
    while (it != discovered_nodes_.end()) {
        if (current_time - it->last_seen > config_.node_timeout_ms) {
            it = discovered_nodes_.erase(it);
        } else {
            ++it;
        }
    }
}

void MeshProtocol::updateNodeList(const NodeInfo& node) {
    // Find existing node or add new one
    auto it = std::find_if(discovered_nodes_.begin(), discovered_nodes_.end(),
                          [&node](const NodeInfo& n) { return n.node_id == node.node_id; });
    
    if (it != discovered_nodes_.end()) {
        *it = node;  // Update existing
    } else {
        discovered_nodes_.push_back(node);  // Add new
        stats_.nodes_discovered++;
        
        if (node_discovered_callback_) {
            node_discovered_callback_(node);
        }
    }
}

uint32_t MeshProtocol::generateNodeId() {
    // Generate pseudo-random node ID
    // In a real implementation, this might use MAC address or other unique identifier
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0x10000000, 0xFFFFFFFE);
    return dis(gen);
}

void MeshProtocol::startHeartbeatTimer() {
    // In a full implementation, this would start a hardware timer
    // For now, this is a placeholder
}

void MeshProtocol::stopHeartbeatTimer() {
    // In a full implementation, this would stop the hardware timer
    // For now, this is a placeholder
}

bool MeshProtocol::validateMessage(const MeshMessage& message) {
    if (message.sender_id == 0) return false;
    if (message.payload_size > 1024) return false;  // Reasonable size limit
    if (message.priority > 7) return false;
    if (message.hop_count >= message.max_hops) return false;
    
    return true;
}

std::vector<uint8_t> MeshProtocol::serializeMessage(const MeshMessage& message) {
    // Simple serialization - in production would use more efficient format
    std::vector<uint8_t> data;
    
    // Header
    data.resize(sizeof(message.message_id) + sizeof(message.sender_id) + 
                sizeof(message.receiver_id) + sizeof(message.type) + 
                sizeof(message.timestamp) + sizeof(message.payload_size) + 
                sizeof(message.priority) + sizeof(message.hop_count) + 
                sizeof(message.max_hops) + sizeof(message.requires_ack));
    
    size_t offset = 0;
    memcpy(data.data() + offset, &message.message_id, sizeof(message.message_id));
    offset += sizeof(message.message_id);
    memcpy(data.data() + offset, &message.sender_id, sizeof(message.sender_id));
    offset += sizeof(message.sender_id);
    // ... continue for all fields
    
    // Payload
    data.insert(data.end(), message.payload.begin(), message.payload.end());
    
    return data;
}

MeshMessage MeshProtocol::deserializeMessage(const uint8_t* data, size_t data_size) {
    MeshMessage message;
    
    if (data_size < sizeof(uint32_t) * 3) {
        return message;  // Invalid message
    }
    
    // Simple deserialization - placeholder implementation
    size_t offset = 0;
    memcpy(&message.message_id, data + offset, sizeof(message.message_id));
    offset += sizeof(message.message_id);
    memcpy(&message.sender_id, data + offset, sizeof(message.sender_id));
    offset += sizeof(message.sender_id);
    // ... continue for all fields
    
    return message;
}

// Utility function for timestamp
uint32_t getTimestamp() {
    #ifdef ARDUINO
        return millis();
    #else
        // For testing/simulation
        static uint32_t counter = 0;
        return ++counter * 100;
    #endif
}

} // namespace MeshNetworking