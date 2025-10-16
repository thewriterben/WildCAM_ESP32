/**
 * @file message_protocol.h
 * @brief Enhanced message protocol for multi-board communication
 * 
 * Extends the existing LoRa mesh protocol with multi-board coordination,
 * discovery, and role management capabilities.
 */

#ifndef MESSAGE_PROTOCOL_H
#define MESSAGE_PROTOCOL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../hal/camera_board.h"
#include "../configs/sensor_configs.h"

// Board roles in the multi-board system
enum BoardRole {
    ROLE_UNKNOWN = 0,
    ROLE_COORDINATOR = 1,     // Master board managing the network
    ROLE_NODE = 2,            // Camera board reporting to coordinator
    ROLE_RELAY = 3,           // Network relay for extended range
    ROLE_EDGE_SENSOR = 4,     // Edge sensor node with minimal processing
    ROLE_AI_PROCESSOR = 5,    // AI processing center
    ROLE_HUB = 6,             // High-resolution hub with storage
    ROLE_STEALTH = 7,         // Ultra-low power stealth sensor
    ROLE_PORTABLE = 8         // Portable monitor with cellular
};

// Message types for multi-board coordination
enum MessageType {
    MSG_UNKNOWN = 0,
    MSG_DISCOVERY = 1,        // Board discovery and capability advertisement
    MSG_HEARTBEAT = 2,        // Regular status update (existing)
    MSG_DATA = 3,             // Data transmission (existing)
    MSG_STATUS = 4,           // Status update (existing)
    MSG_IMAGE_META = 5,       // Image metadata (existing)
    MSG_ROLE_ASSIGNMENT = 6,  // Role assignment from coordinator
    MSG_TASK_ASSIGNMENT = 7,  // Task assignment for load balancing
    MSG_ELECTION = 8,         // Coordinator election
    MSG_TOPOLOGY = 9,         // Network topology update
    MSG_CONFIG_UPDATE = 10,   // Configuration synchronization
    MSG_EMERGENCY = 11,       // Emergency communication
    MSG_DETECTION_EVENT = 12, // Wildlife detection event broadcast
    MSG_WILDLIFE_DETECTION = 13 // Wildlife detection report to coordinator
};

// Board capabilities structure
struct BoardCapabilities {
    BoardType boardType;
    SensorType sensorType;
    uint32_t maxResolution;
    bool hasCamera;           // Camera sensor available
    bool hasLoRa;             // LoRa radio available
    bool hasAI;
    bool hasPSRAM;
    bool hasSD;
    bool hasCellular;
    bool hasSatellite;
    uint32_t batteryLevel;    // Percentage (0-100)
    uint32_t powerProfile;    // Power consumption class
    float solarVoltage;       // Solar panel voltage if available
    uint32_t availableStorage; // Available storage in bytes
};

// Network node information
struct NetworkNode {
    int nodeId;
    BoardRole role;
    BoardCapabilities capabilities;
    int signalStrength;
    int hopCount;
    unsigned long lastSeen;
    bool isActive;
    float coordinatorScore;   // Score for coordinator election
};

// Message structure for multi-board communication
struct MultiboardMessage {
    MessageType type;
    int sourceNode;
    int targetNode;           // 0 for broadcast
    BoardRole sourceRole;
    unsigned long timestamp;
    int hopCount;
    JsonObject data;
};

// Discovery message structure
struct DiscoveryMessage {
    int nodeId;
    BoardRole currentRole;
    BoardRole preferredRole;
    BoardCapabilities capabilities;
    bool seekingCoordinator;
    float coordinatorScore;
};

// Task assignment structure
struct TaskAssignment {
    int taskId;
    int assignedNode;
    String taskType;
    JsonObject parameters;
    unsigned long deadline;
    int priority;
};

namespace MessageProtocol {
    /**
     * Initialize the enhanced message protocol
     */
    bool init(int nodeId, BoardType boardType, SensorType sensorType);
    
    /**
     * Create discovery message with board capabilities
     */
    String createDiscoveryMessage(const BoardCapabilities& capabilities, BoardRole preferredRole);
    
    /**
     * Create role assignment message
     */
    String createRoleAssignmentMessage(int targetNode, BoardRole assignedRole);
    
    /**
     * Create task assignment message
     */
    String createTaskAssignmentMessage(const TaskAssignment& task);
    
    /**
     * Create topology update message
     */
    String createTopologyMessage(const std::vector<NetworkNode>& nodes);
    
    /**
     * Create coordinator election message
     */
    String createElectionMessage(float coordinatorScore, bool claimCoordinator);
    
    /**
     * Parse received message into MultiboardMessage structure
     */
    bool parseMessage(const String& message, MultiboardMessage& parsedMsg);
    
    /**
     * Parse discovery message
     */
    bool parseDiscoveryMessage(const JsonObject& data, DiscoveryMessage& discovery);
    
    /**
     * Get board capabilities of current system
     */
    BoardCapabilities getCurrentCapabilities();
    
    /**
     * Calculate coordinator score based on capabilities
     */
    float calculateCoordinatorScore(const BoardCapabilities& caps);
    
    /**
     * Convert board role to string
     */
    const char* roleToString(BoardRole role);
    
    /**
     * Convert message type to string
     */
    const char* messageTypeToString(MessageType type);
    
    /**
     * Validate message integrity
     */
    bool validateMessage(const MultiboardMessage& msg);
}

#endif // MESSAGE_PROTOCOL_H