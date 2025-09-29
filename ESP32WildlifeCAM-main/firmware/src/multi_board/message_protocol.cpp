/**
 * @file message_protocol.cpp
 * @brief Implementation of enhanced message protocol for multi-board communication
 */

#include "message_protocol.h"
#include "../config.h"
#include "../hal/board_detector.h"
#include "../power_manager.h"
#include "../../src/data/storage_manager.h"
#include <esp_system.h>

namespace MessageProtocol {

// Global variables
static int g_nodeId = 0;
static BoardType g_boardType = BOARD_UNKNOWN;
static SensorType g_sensorType = SENSOR_UNKNOWN;
static bool g_initialized = false;
static PowerManager g_powerManager;

/**
 * Detect AI capabilities based on hardware features
 */
bool detectAICapabilities() {
    // Check for ESP32-S3 which has AI acceleration features
    const char* chip_model = BoardDetector::getChipModel();
    if (strstr(chip_model, "ESP32-S3") != nullptr) {
        Serial.println("AI capabilities detected: ESP32-S3 with AI acceleration");
        return true;
    }
    
    // Check for PSRAM which is often required for AI workloads
    if (BoardDetector::hasPSRAM()) {
        size_t psram_size = ESP.getPsramSize();
        if (psram_size >= 2 * 1024 * 1024) { // At least 2MB PSRAM
            Serial.printf("AI capabilities detected: Sufficient PSRAM (%d bytes) for AI processing\n", psram_size);
            return true;
        }
    }
    
    // Check for specific board types that are AI-capable
    BoardType board_type = BoardDetector::detectBoardType();
    switch (board_type) {
        case BOARD_ESP32_S3_CAM:
        case BOARD_ESP32_S3_EYE:
        case BOARD_XIAO_ESP32S3_SENSE:
            Serial.println("AI capabilities detected: AI-capable board type");
            return true;
        case BOARD_AI_THINKER_ESP32_CAM:
            // AI-Thinker ESP32-CAM can support basic AI with PSRAM
            if (BoardDetector::hasPSRAM()) {
                Serial.println("AI capabilities detected: AI-Thinker with PSRAM for AI workloads");
                return true;
            }
            Serial.println("AI capabilities: Limited on AI-Thinker without PSRAM");
            return false;
        default:
            break;
    }
    
    // Additional check for ESP32 variants with sufficient resources
    if (ESP.getChipCores() >= 2 && ESP.getFlashChipSize() >= 4 * 1024 * 1024) {
        Serial.println("AI capabilities detected: Multi-core ESP32 with sufficient flash");
        return true;
    }
    
    Serial.println("No AI capabilities detected");
    return false;
}

bool init(int nodeId, BoardType boardType, SensorType sensorType) {
    g_nodeId = nodeId;
    g_boardType = boardType;
    g_sensorType = sensorType;
    g_initialized = true;
    
    Serial.printf("Message protocol initialized: Node %d, Board %d, Sensor %d\n", 
                  nodeId, boardType, sensorType);
    return true;
}

String createDiscoveryMessage(const BoardCapabilities& capabilities, BoardRole preferredRole) {
    DynamicJsonDocument doc(1024);
    
    doc["type"] = MSG_DISCOVERY;
    doc["source_node"] = g_nodeId;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    // Discovery-specific data
    JsonObject data = doc.createNestedObject("data");
    data["node_id"] = g_nodeId;
    data["current_role"] = ROLE_UNKNOWN;
    data["preferred_role"] = preferredRole;
    data["seeking_coordinator"] = true;
    data["coordinator_score"] = calculateCoordinatorScore(capabilities);
    
    // Board capabilities
    JsonObject caps = data.createNestedObject("capabilities");
    caps["board_type"] = capabilities.boardType;
    caps["sensor_type"] = capabilities.sensorType;
    caps["max_resolution"] = capabilities.maxResolution;
    caps["has_ai"] = capabilities.hasAI;
    caps["has_psram"] = capabilities.hasPSRAM;
    caps["has_sd"] = capabilities.hasSD;
    caps["has_cellular"] = capabilities.hasCellular;
    caps["has_satellite"] = capabilities.hasSatellite;
    caps["battery_level"] = capabilities.batteryLevel;
    caps["power_profile"] = capabilities.powerProfile;
    caps["solar_voltage"] = capabilities.solarVoltage;
    caps["available_storage"] = capabilities.availableStorage;
    
    String message;
    serializeJson(doc, message);
    return message;
}

String createRoleAssignmentMessage(int targetNode, BoardRole assignedRole) {
    DynamicJsonDocument doc(512);
    
    doc["type"] = MSG_ROLE_ASSIGNMENT;
    doc["source_node"] = g_nodeId;
    doc["target_node"] = targetNode;
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["assigned_role"] = assignedRole;
    data["role_name"] = roleToString(assignedRole);
    data["assignment_time"] = millis();
    
    String message;
    serializeJson(doc, message);
    return message;
}

String createTaskAssignmentMessage(const TaskAssignment& task) {
    DynamicJsonDocument doc(1024);
    
    doc["type"] = MSG_TASK_ASSIGNMENT;
    doc["source_node"] = g_nodeId;
    doc["target_node"] = task.assignedNode;
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["task_id"] = task.taskId;
    data["task_type"] = task.taskType;
    data["deadline"] = task.deadline;
    data["priority"] = task.priority;
    
    // Copy task parameters
    JsonObject params = data.createNestedObject("parameters");
    for (JsonPair kv : task.parameters) {
        params[kv.key()] = kv.value();
    }
    
    String message;
    serializeJson(doc, message);
    return message;
}

String createTopologyMessage(const std::vector<NetworkNode>& nodes) {
    DynamicJsonDocument doc(2048);
    
    doc["type"] = MSG_TOPOLOGY;
    doc["source_node"] = g_nodeId;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["node_count"] = nodes.size();
    
    JsonArray nodeArray = data.createNestedArray("nodes");
    for (const auto& node : nodes) {
        JsonObject nodeObj = nodeArray.createNestedObject();
        nodeObj["node_id"] = node.nodeId;
        nodeObj["role"] = node.role;
        nodeObj["signal_strength"] = node.signalStrength;
        nodeObj["hop_count"] = node.hopCount;
        nodeObj["last_seen"] = node.lastSeen;
        nodeObj["is_active"] = node.isActive;
        nodeObj["coordinator_score"] = node.coordinatorScore;
    }
    
    String message;
    serializeJson(doc, message);
    return message;
}

String createElectionMessage(float coordinatorScore, bool claimCoordinator) {
    DynamicJsonDocument doc(512);
    
    doc["type"] = MSG_ELECTION;
    doc["source_node"] = g_nodeId;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["coordinator_score"] = coordinatorScore;
    data["claim_coordinator"] = claimCoordinator;
    data["election_time"] = millis();
    
    String message;
    serializeJson(doc, message);
    return message;
}

bool parseMessage(const String& message, MultiboardMessage& parsedMsg) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.printf("Message parse error: %s\n", error.c_str());
        return false;
    }
    
    parsedMsg.type = static_cast<MessageType>(doc["type"].as<int>());
    parsedMsg.sourceNode = doc["source_node"];
    parsedMsg.targetNode = doc["target_node"] | 0;
    parsedMsg.sourceRole = static_cast<BoardRole>(doc["source_role"].as<int>());
    parsedMsg.timestamp = doc["timestamp"];
    parsedMsg.hopCount = doc["hop_count"] | 0;
    parsedMsg.data = doc["data"];
    
    return true;
}

bool parseDiscoveryMessage(const JsonObject& data, DiscoveryMessage& discovery) {
    if (!data.containsKey("capabilities")) {
        return false;
    }
    
    discovery.nodeId = data["node_id"];
    discovery.currentRole = static_cast<BoardRole>(data["current_role"].as<int>());
    discovery.preferredRole = static_cast<BoardRole>(data["preferred_role"].as<int>());
    discovery.seekingCoordinator = data["seeking_coordinator"];
    discovery.coordinatorScore = data["coordinator_score"];
    
    // Parse capabilities
    JsonObject caps = data["capabilities"];
    discovery.capabilities.boardType = static_cast<BoardType>(caps["board_type"].as<int>());
    discovery.capabilities.sensorType = static_cast<SensorType>(caps["sensor_type"].as<int>());
    discovery.capabilities.maxResolution = caps["max_resolution"];
    discovery.capabilities.hasAI = caps["has_ai"];
    discovery.capabilities.hasPSRAM = caps["has_psram"];
    discovery.capabilities.hasSD = caps["has_sd"];
    discovery.capabilities.hasCellular = caps["has_cellular"];
    discovery.capabilities.hasSatellite = caps["has_satellite"];
    discovery.capabilities.batteryLevel = caps["battery_level"];
    discovery.capabilities.powerProfile = caps["power_profile"];
    discovery.capabilities.solarVoltage = caps["solar_voltage"];
    discovery.capabilities.availableStorage = caps["available_storage"];
    
    return true;
}

BoardCapabilities getCurrentCapabilities() {
    BoardCapabilities caps;
    
    caps.boardType = g_boardType;
    caps.sensorType = g_sensorType;
    
    // Get sensor capabilities
    const SensorCapabilities* sensorCaps = getSensorCapabilities(g_sensorType);
    if (sensorCaps) {
        caps.maxResolution = sensorCaps->max_width * sensorCaps->max_height;
    } else {
        caps.maxResolution = 640 * 480; // Default VGA
    }
    
    // Detect hardware capabilities
    caps.hasPSRAM = BoardDetector::hasPSRAM();
    
    // Detect AI capabilities based on hardware features
    caps.hasAI = detectAICapabilities();
    
    caps.hasSD = StorageManager::initialize(); // Detect SD card presence
    caps.hasCellular = false; // TODO: Detect cellular modem
    caps.hasSatellite = false; // TODO: Detect satellite modem
    
    // Power status - read from actual power management system
    // Initialize power manager if not already done
    if (!g_powerManager.isInitialized()) {
        g_powerManager.init();
    }
    g_powerManager.update();
    
    caps.batteryLevel = (int)g_powerManager.getBatteryPercentage();
    caps.powerProfile = 2; // Medium power
    caps.solarVoltage = g_powerManager.getSolarVoltage();
    caps.availableStorage = caps.hasSD ? StorageManager::getStatistics().freeSpace : 0;
    
    return caps;
}

float calculateCoordinatorScore(const BoardCapabilities& caps) {
    float score = 0.0;
    
    // Base score from board type
    switch (caps.boardType) {
        case BOARD_ESP_EYE:
        case BOARD_ESP32_S3_CAM:
            score += 30.0; // High-end boards
            break;
        case BOARD_AI_THINKER_ESP32_CAM:
            score += 20.0; // Standard boards
            break;
        case BOARD_XIAO_ESP32S3_SENSE:
        case BOARD_M5STACK_TIMER_CAM:
            score += 10.0; // Lower-end boards
            break;
        default:
            score += 15.0; // Unknown boards
            break;
    }
    
    // Add points for capabilities
    if (caps.hasPSRAM) score += 15.0;
    if (caps.hasAI) score += 20.0;
    if (caps.hasSD) score += 10.0;
    if (caps.hasCellular) score += 15.0;
    if (caps.hasSatellite) score += 10.0;
    
    // Power considerations
    score += (caps.batteryLevel / 100.0) * 10.0; // Up to 10 points for battery
    if (caps.solarVoltage > 3.5) score += 5.0; // Solar power bonus
    
    // Storage capacity
    score += (caps.availableStorage / (1024.0 * 1024.0)) * 2.0; // 2 points per MB
    
    return score;
}

const char* roleToString(BoardRole role) {
    switch (role) {
        case ROLE_COORDINATOR: return "Coordinator";
        case ROLE_NODE: return "Node";
        case ROLE_RELAY: return "Relay";
        case ROLE_EDGE_SENSOR: return "Edge Sensor";
        case ROLE_AI_PROCESSOR: return "AI Processor";
        case ROLE_HUB: return "Hub";
        case ROLE_STEALTH: return "Stealth";
        case ROLE_PORTABLE: return "Portable";
        default: return "Unknown";
    }
}

const char* messageTypeToString(MessageType type) {
    switch (type) {
        case MSG_DISCOVERY: return "Discovery";
        case MSG_HEARTBEAT: return "Heartbeat";
        case MSG_DATA: return "Data";
        case MSG_STATUS: return "Status";
        case MSG_IMAGE_META: return "Image Meta";
        case MSG_ROLE_ASSIGNMENT: return "Role Assignment";
        case MSG_TASK_ASSIGNMENT: return "Task Assignment";
        case MSG_ELECTION: return "Election";
        case MSG_TOPOLOGY: return "Topology";
        case MSG_CONFIG_UPDATE: return "Config Update";
        case MSG_EMERGENCY: return "Emergency";
        default: return "Unknown";
    }
}

bool validateMessage(const MultiboardMessage& msg) {
    // Basic validation
    if (msg.type == MSG_UNKNOWN) return false;
    if (msg.sourceNode <= 0) return false;
    if (msg.timestamp == 0) return false;
    
    // Additional validation based on message type
    switch (msg.type) {
        case MSG_DISCOVERY:
            return msg.data.containsKey("capabilities");
        case MSG_ROLE_ASSIGNMENT:
            return msg.data.containsKey("assigned_role");
        case MSG_TASK_ASSIGNMENT:
            return msg.data.containsKey("task_id") && msg.data.containsKey("task_type");
        default:
            return true; // Basic validation passed
    }
}

} // namespace MessageProtocol