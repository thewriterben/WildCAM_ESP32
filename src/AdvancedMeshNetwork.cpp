/**
 * @file AdvancedMeshNetwork.cpp
 * @brief Advanced LoRa Mesh Networking Implementation for WildCAM ESP32
 * @version 1.0.0
 * 
 * Implements advanced mesh networking features including multi-camera
 * coordination, extended range communication, and shared detection alerts.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include "AdvancedMeshNetwork.h"
#include "config.h"
#include "Logger.h"

// Global instance pointer
AdvancedMeshNetwork* advancedMesh = nullptr;

//==============================================================================
// PACKET TYPE DEFINITIONS FOR ADVANCED MESH
//==============================================================================

// Extended packet types for advanced features
#define ADV_PACKET_COORD_SYNC      0x10  // Coordination sync
#define ADV_PACKET_COORD_STATUS    0x11  // Camera status update
#define ADV_PACKET_COORD_CAPTURE   0x12  // Sync capture request
#define ADV_PACKET_COORD_JOIN      0x13  // Group join request
#define ADV_PACKET_COORD_LEAVE     0x14  // Group leave notification
#define ADV_PACKET_RANGE_PROBE     0x20  // Range probe packet
#define ADV_PACKET_RANGE_REPORT    0x21  // Range quality report
#define ADV_PACKET_RELAY_REG       0x22  // Relay registration
#define ADV_PACKET_ALERT           0x30  // Shared alert
#define ADV_PACKET_ALERT_ACK       0x31  // Alert acknowledgment
#define ADV_PACKET_ALERT_FORWARD   0x32  // Alert forwarding

// Federated learning packet types
#define ADV_PACKET_FL_ROUND_ANNOUNCE  0x40  // FL round announcement
#define ADV_PACKET_FL_JOIN_REQUEST    0x41  // FL join request
#define ADV_PACKET_FL_JOIN_RESPONSE   0x42  // FL join response
#define ADV_PACKET_FL_MODEL_CHUNK     0x43  // Model gradient chunk
#define ADV_PACKET_FL_MODEL_ACK       0x44  // Model chunk acknowledgment
#define ADV_PACKET_FL_AGGREGATION     0x45  // Aggregated model distribution
#define ADV_PACKET_FL_ROUND_COMPLETE  0x46  // Round completion notification
#define ADV_PACKET_FL_LEAVE           0x47  // Leave FL round

//==============================================================================
// CONSTRUCTOR / DESTRUCTOR
//==============================================================================

AdvancedMeshNetwork::AdvancedMeshNetwork(MeshManager& meshManager) :
    _meshManager(meshManager),
    _initialized(false),
    _cameraRole(CAMERA_ROLE_STANDALONE),
    _syncState(SYNC_STATE_UNSYNC),
    _lastSyncTime(0),
    _lastStatusBroadcast(0),
    _syncCaptureRequested(false),
    _rangeMode(RANGE_MODE_ADAPTIVE),
    _lastRangeCheck(0),
    _alertIdCounter(0),
    _lastAlertCleanup(0),
    _flRole(FL_ROLE_PARTICIPANT),
    _flCompression(FL_COMPRESS_QUANTIZE_8BIT),
    _flRoundIdCounter(0),
    _lastFLProcess(0),
    _flUpdatePending(false),
    _coordCallback(nullptr),
    _alertCallback(nullptr),
    _rangeCallback(nullptr),
    _flRoundCallback(nullptr),
    _flModelUpdateCallback(nullptr),
    _flAggregationCallback(nullptr)
{
    // Initialize current group
    _currentGroup.groupId = 0;
    _currentGroup.primaryNodeId = 0;
    _currentGroup.memberCount = 0;
    _currentGroup.lastSync = 0;
    _currentGroup.isActive = false;
    
    // Initialize federated learning round
    _currentFLRound.roundId = 0;
    _currentFLRound.coordinatorId = 0;
    _currentFLRound.startTime = 0;
    _currentFLRound.deadline = 0;
    _currentFLRound.state = FL_STATE_IDLE;
    _currentFLRound.participantCount = 0;
    _currentFLRound.updatesReceived = 0;
    _currentFLRound.targetParticipants = 0;
    _currentFLRound.minParticipationRatio = 0.6f;
    
    // Initialize aggregation stats
    _lastAggregationStats.roundId = 0;
    _lastAggregationStats.contributorCount = 0;
    _lastAggregationStats.totalSamples = 0;
    _lastAggregationStats.averageLoss = 0.0f;
    _lastAggregationStats.accuracyImprovement = 0.0f;
    _lastAggregationStats.aggregationTime = 0;
    _lastAggregationStats.distributionTime = 0;
    
    // Reserve space for vectors
    _groupCameras.reserve(MESH_MAX_CAMERAS);
    _relayNodes.reserve(MESH_MAX_NODES);
    _recentAlerts.reserve(MESH_MAX_RECENT_ALERTS);
    _alertDedup.reserve(MESH_MAX_RECENT_ALERTS * 2);
    _flParticipants.reserve(MESH_FL_MAX_PARTICIPANTS);
    _pendingChunks.reserve(32);
    
    // Initialize range settings to normal mode
    _currentSettings = getSettingsForMode(RANGE_MODE_NORMAL);
}

AdvancedMeshNetwork::~AdvancedMeshNetwork() {
    _groupCameras.clear();
    _relayNodes.clear();
    _recentAlerts.clear();
    _alertDedup.clear();
    _flParticipants.clear();
    _pendingChunks.clear();
}

//==============================================================================
// INITIALIZATION
//==============================================================================

bool AdvancedMeshNetwork::init() {
    if (_initialized) {
        LOG_WARN("AdvancedMeshNetwork already initialized");
        return true;
    }
    
    // Verify base mesh manager is initialized
    if (!_meshManager.isInitialized()) {
        LOG_ERROR("Cannot initialize AdvancedMeshNetwork: MeshManager not initialized");
        return false;
    }
    
    LOG_INFO("Initializing Advanced Mesh Network...");
    Serial.println("Initializing Advanced Mesh Network...");
    
    // Initialize alert ID counter from node ID for uniqueness
    _alertIdCounter = (_meshManager.getNodeId() << 16) | (millis() & 0xFFFF);
    
    // Initialize FL round ID counter
    _flRoundIdCounter = (_meshManager.getNodeId() << 8) | 1;
    
    // Set initial range settings
    updateRangeSettings();
    
    _initialized = true;
    _lastSyncTime = millis();
    _lastStatusBroadcast = millis();
    _lastRangeCheck = millis();
    _lastAlertCleanup = millis();
    _lastFLProcess = millis();
    
    LOG_INFO("Advanced Mesh Network initialized");
    Serial.println("   ✓ Advanced Mesh Network initialized");
    Serial.println("   ✓ Multi-camera coordination enabled");
    Serial.println("   ✓ Extended range communication enabled");
    Serial.println("   ✓ Shared detection alerts enabled");
    Serial.println("   ✓ Federated learning mesh support enabled");
    
    return true;
}

bool AdvancedMeshNetwork::isInitialized() const {
    return _initialized;
}

void AdvancedMeshNetwork::process() {
    if (!_initialized) {
        return;
    }
    
    // Process multi-camera coordination
    #if MESH_MULTI_CAMERA_ENABLED
    processCoordination();
    #endif
    
    // Process extended range
    #if MESH_EXTENDED_RANGE_ENABLED
    processExtendedRange();
    #endif
    
    // Process shared alerts
    #if MESH_SHARED_ALERTS_ENABLED
    processAlerts();
    #endif
    
    // Process federated learning
    #if MESH_FEDERATED_LEARNING_ENABLED
    processFederatedLearning();
    #endif
}

//==============================================================================
// MULTI-CAMERA COORDINATION
//==============================================================================

bool AdvancedMeshNetwork::createCameraGroup(uint8_t groupId) {
    if (!_initialized) {
        return false;
    }
    
    // Leave current group if active
    if (_currentGroup.isActive) {
        leaveCameraGroup();
    }
    
    // Create new group with this node as primary
    _currentGroup.groupId = groupId;
    _currentGroup.primaryNodeId = _meshManager.getNodeId();
    _currentGroup.memberNodes[0] = _meshManager.getNodeId();
    _currentGroup.memberCount = 1;
    _currentGroup.lastSync = millis();
    _currentGroup.isActive = true;
    
    _cameraRole = CAMERA_ROLE_PRIMARY;
    _syncState = SYNC_STATE_LEADER;
    
    LOG_INFO("Created camera group %d as primary node", groupId);
    Serial.printf("Created camera group %d as primary\n", groupId);
    
    // Broadcast group creation
    sendCoordinationSync();
    
    return true;
}

bool AdvancedMeshNetwork::joinCameraGroup(uint8_t groupId) {
    if (!_initialized) {
        return false;
    }
    
    // Leave current group if active
    if (_currentGroup.isActive) {
        leaveCameraGroup();
    }
    
    _currentGroup.groupId = groupId;
    _currentGroup.primaryNodeId = 0;  // Will be set when we receive sync
    _currentGroup.memberNodes[0] = _meshManager.getNodeId();
    _currentGroup.memberCount = 1;
    _currentGroup.lastSync = millis();
    _currentGroup.isActive = true;
    
    _cameraRole = CAMERA_ROLE_SECONDARY;
    _syncState = SYNC_STATE_SYNCING;
    
    LOG_INFO("Joining camera group %d as secondary node", groupId);
    Serial.printf("Joining camera group %d as secondary\n", groupId);
    
    // Send join request
    uint8_t packet[8];
    packet[0] = ADV_PACKET_COORD_JOIN;
    packet[1] = groupId;
    uint32_t nodeId = _meshManager.getNodeId();
    packet[2] = (nodeId >> 24) & 0xFF;
    packet[3] = (nodeId >> 16) & 0xFF;
    packet[4] = (nodeId >> 8) & 0xFF;
    packet[5] = nodeId & 0xFF;
    packet[6] = (uint8_t)_cameraRole;
    packet[7] = 0;  // Reserved
    
    _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_HIGH);
    
    return true;
}

void AdvancedMeshNetwork::leaveCameraGroup() {
    if (!_currentGroup.isActive) {
        return;
    }
    
    LOG_INFO("Leaving camera group %d", _currentGroup.groupId);
    
    // Send leave notification
    uint8_t packet[6];
    packet[0] = ADV_PACKET_COORD_LEAVE;
    packet[1] = _currentGroup.groupId;
    uint32_t nodeId = _meshManager.getNodeId();
    packet[2] = (nodeId >> 24) & 0xFF;
    packet[3] = (nodeId >> 16) & 0xFF;
    packet[4] = (nodeId >> 8) & 0xFF;
    packet[5] = nodeId & 0xFF;
    
    _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_NORMAL);
    
    // Reset group state
    _currentGroup.groupId = 0;
    _currentGroup.primaryNodeId = 0;
    _currentGroup.memberCount = 0;
    _currentGroup.isActive = false;
    
    _cameraRole = CAMERA_ROLE_STANDALONE;
    _syncState = SYNC_STATE_UNSYNC;
    
    _groupCameras.clear();
}

CameraGroup AdvancedMeshNetwork::getCameraGroup() const {
    return _currentGroup;
}

void AdvancedMeshNetwork::setCameraRole(CameraRole role) {
    _cameraRole = role;
    LOG_DEBUG("Camera role set to: %d", (int)role);
}

CameraRole AdvancedMeshNetwork::getCameraRole() const {
    return _cameraRole;
}

CameraSyncState AdvancedMeshNetwork::getSyncState() const {
    return _syncState;
}

bool AdvancedMeshNetwork::requestSyncCapture() {
    if (!_initialized || !_currentGroup.isActive) {
        return false;
    }
    
    LOG_INFO("Requesting synchronized capture for group %d", _currentGroup.groupId);
    
    // Create sync capture request packet
    uint8_t packet[16];
    packet[0] = ADV_PACKET_COORD_CAPTURE;
    packet[1] = _currentGroup.groupId;
    
    uint32_t nodeId = _meshManager.getNodeId();
    packet[2] = (nodeId >> 24) & 0xFF;
    packet[3] = (nodeId >> 16) & 0xFF;
    packet[4] = (nodeId >> 8) & 0xFF;
    packet[5] = nodeId & 0xFF;
    
    // Include capture timestamp (2 seconds from now for sync)
    uint32_t captureTime = millis() + 2000;
    packet[6] = (captureTime >> 24) & 0xFF;
    packet[7] = (captureTime >> 16) & 0xFF;
    packet[8] = (captureTime >> 8) & 0xFF;
    packet[9] = captureTime & 0xFF;
    
    // Padding
    for (int i = 10; i < 16; i++) {
        packet[i] = 0;
    }
    
    _syncCaptureRequested = true;
    
    return _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_HIGH);
}

void AdvancedMeshNetwork::updateCameraStatus(const CameraStatus& status) {
    if (!_initialized) {
        return;
    }
    
    // Update local camera list
    bool found = false;
    for (auto& camera : _groupCameras) {
        if (camera.nodeId == status.nodeId) {
            camera = status;
            found = true;
            break;
        }
    }
    
    if (!found && _groupCameras.size() < MESH_MAX_CAMERAS) {
        _groupCameras.push_back(status);
    }
    
    // Broadcast status if in a group
    if (_currentGroup.isActive) {
        // Serialize and broadcast status
        uint8_t packet[32];
        size_t len = 0;
        
        packet[len++] = ADV_PACKET_COORD_STATUS;
        packet[len++] = _currentGroup.groupId;
        
        // Node ID
        packet[len++] = (status.nodeId >> 24) & 0xFF;
        packet[len++] = (status.nodeId >> 16) & 0xFF;
        packet[len++] = (status.nodeId >> 8) & 0xFF;
        packet[len++] = status.nodeId & 0xFF;
        
        // Status fields
        packet[len++] = (uint8_t)status.role;
        packet[len++] = (uint8_t)status.syncState;
        packet[len++] = status.batteryLevel;
        packet[len++] = (status.isCapturing ? 0x01 : 0) | (status.hasSDSpace ? 0x02 : 0);
        
        // Signal strength
        packet[len++] = (status.signalStrength >> 8) & 0xFF;
        packet[len++] = status.signalStrength & 0xFF;
        
        // Last capture time
        packet[len++] = (status.lastCapture >> 24) & 0xFF;
        packet[len++] = (status.lastCapture >> 16) & 0xFF;
        packet[len++] = (status.lastCapture >> 8) & 0xFF;
        packet[len++] = status.lastCapture & 0xFF;
        
        // Image count
        packet[len++] = (status.imageCount >> 24) & 0xFF;
        packet[len++] = (status.imageCount >> 16) & 0xFF;
        packet[len++] = (status.imageCount >> 8) & 0xFF;
        packet[len++] = status.imageCount & 0xFF;
        
        _meshManager.broadcastMessage(packet, len, PRIORITY_LOW);
    }
}

std::vector<CameraStatus> AdvancedMeshNetwork::getGroupCameras() const {
    return _groupCameras;
}

bool AdvancedMeshNetwork::isReadyForCoordinatedCapture() const {
    if (!_currentGroup.isActive || _syncState != SYNC_STATE_SYNCED) {
        return false;
    }
    
    // Check if all cameras in group are ready
    for (const auto& camera : _groupCameras) {
        if (camera.isCapturing || !camera.hasSDSpace) {
            return false;
        }
    }
    
    return true;
}

//==============================================================================
// EXTENDED RANGE COMMUNICATION
//==============================================================================

void AdvancedMeshNetwork::setRangeMode(ExtendedRangeMode mode) {
    if (_rangeMode == mode) {
        return;
    }
    
    ExtendedRangeMode oldMode = _rangeMode;
    _rangeMode = mode;
    
    // Apply new settings
    updateRangeSettings();
    
    LOG_INFO("Range mode changed from %d to %d", (int)oldMode, (int)mode);
    
    // Notify callback
    if (_rangeCallback) {
        _rangeCallback(mode);
    }
}

ExtendedRangeMode AdvancedMeshNetwork::getRangeMode() const {
    return _rangeMode;
}

RangeSettings AdvancedMeshNetwork::getRangeSettings() const {
    return _currentSettings;
}

bool AdvancedMeshNetwork::optimizeRelayPath(uint32_t destNodeId) {
    if (!_initialized) {
        return false;
    }
    
    // Get current route to destination
    uint32_t currentNextHop = _meshManager.getNextHop(destNodeId);
    if (currentNextHop == 0) {
        LOG_DEBUG("No route to node 0x%08X for optimization", destNodeId);
        return false;
    }
    
    // Find best relay based on signal quality
    uint32_t bestRelay = getBestRelay(destNodeId);
    if (bestRelay == 0 || bestRelay == currentNextHop) {
        return false;  // No better relay found
    }
    
    LOG_INFO("Optimized relay path to 0x%08X: via 0x%08X", destNodeId, bestRelay);
    
    return true;
}

bool AdvancedMeshNetwork::registerRelay(uint32_t nodeId) {
    if (!_initialized) {
        return false;
    }
    
    // Check if already registered
    for (const auto& relay : _relayNodes) {
        if (relay.nodeId == nodeId) {
            return true;  // Already registered
        }
    }
    
    if (_relayNodes.size() >= MESH_MAX_NODES) {
        LOG_WARN("Maximum relay nodes reached");
        return false;
    }
    
    // Add new relay node
    RelayNode newRelay;
    newRelay.nodeId = nodeId;
    newRelay.rssiToRelay = 0;
    newRelay.rssiFromRelay = 0;
    newRelay.hopCount = 1;
    newRelay.reliability = 1.0f;
    newRelay.lastSeen = millis();
    newRelay.isActive = true;
    
    _relayNodes.push_back(newRelay);
    
    LOG_INFO("Registered relay node: 0x%08X", nodeId);
    
    // Send relay registration packet
    uint8_t packet[8];
    packet[0] = ADV_PACKET_RELAY_REG;
    packet[1] = 0x01;  // Registration
    uint32_t myNodeId = _meshManager.getNodeId();
    packet[2] = (myNodeId >> 24) & 0xFF;
    packet[3] = (myNodeId >> 16) & 0xFF;
    packet[4] = (myNodeId >> 8) & 0xFF;
    packet[5] = myNodeId & 0xFF;
    packet[6] = 0;
    packet[7] = 0;
    
    _meshManager.sendMessage(nodeId, packet, sizeof(packet), PRIORITY_NORMAL);
    
    return true;
}

std::vector<RelayNode> AdvancedMeshNetwork::getRelayNodes() const {
    return _relayNodes;
}

uint32_t AdvancedMeshNetwork::getBestRelay(uint32_t destNodeId) const {
    uint32_t bestRelay = 0;
    float bestScore = 0.0f;
    
    for (const auto& relay : _relayNodes) {
        if (!relay.isActive) {
            continue;
        }
        
        // Calculate relay score based on RSSI and reliability
        float rssiScore = (137.0f + relay.rssiToRelay) / 137.0f;  // Normalize RSSI
        float score = rssiScore * relay.reliability;
        
        if (score > bestScore) {
            bestScore = score;
            bestRelay = relay.nodeId;
        }
    }
    
    return bestRelay;
}

bool AdvancedMeshNetwork::needsExtendedRange(uint32_t destNodeId) const {
    // Get node info from mesh manager
    MeshNode node = _meshManager.getNode(destNodeId);
    
    // Need extended range if RSSI is below threshold or hop count is high
    if (node.lastRssi < MESH_EXTENDED_RANGE_RSSI_THRESHOLD) {
        return true;
    }
    
    if (node.hopCount >= MESH_RELAY_OPTIMIZATION_THRESHOLD) {
        return true;
    }
    
    return false;
}

//==============================================================================
// SHARED DETECTION ALERTS
//==============================================================================

bool AdvancedMeshNetwork::broadcastAlert(const SharedAlert& alert) {
    if (!_initialized) {
        return false;
    }
    
    // Check for duplicate
    if (isDuplicateAlert(alert.alertId, alert.originNodeId)) {
        LOG_DEBUG("Alert %08X is duplicate, not broadcasting", alert.alertId);
        return false;
    }
    
    // Add to dedup list
    addAlertDedup(alert.alertId, alert.originNodeId);
    
    // Store alert
    if (_recentAlerts.size() >= MESH_MAX_RECENT_ALERTS) {
        _recentAlerts.erase(_recentAlerts.begin());
    }
    _recentAlerts.push_back(alert);
    
    LOG_INFO("Broadcasting alert %08X: %s (priority: %d)", 
             alert.alertId, alert.species.c_str(), (int)alert.priority);
    
    // Serialize alert
    uint8_t packet[128];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_ALERT;
    packet[len++] = (uint8_t)alert.priority;
    packet[len++] = (uint8_t)alert.type;
    
    // Alert ID
    packet[len++] = (alert.alertId >> 24) & 0xFF;
    packet[len++] = (alert.alertId >> 16) & 0xFF;
    packet[len++] = (alert.alertId >> 8) & 0xFF;
    packet[len++] = alert.alertId & 0xFF;
    
    // Origin node
    packet[len++] = (alert.originNodeId >> 24) & 0xFF;
    packet[len++] = (alert.originNodeId >> 16) & 0xFF;
    packet[len++] = (alert.originNodeId >> 8) & 0xFF;
    packet[len++] = alert.originNodeId & 0xFF;
    
    // Timestamp
    packet[len++] = (alert.timestamp >> 24) & 0xFF;
    packet[len++] = (alert.timestamp >> 16) & 0xFF;
    packet[len++] = (alert.timestamp >> 8) & 0xFF;
    packet[len++] = alert.timestamp & 0xFF;
    
    // Confidence (scaled to 0-255)
    packet[len++] = (uint8_t)(alert.confidence * 255);
    
    // Hop count
    packet[len++] = alert.hopCount;
    
    // Has image flag
    packet[len++] = alert.hasImage ? 1 : 0;
    
    // Species name (max 20 chars)
    String species = alert.species.substring(0, 20);
    packet[len++] = species.length();
    for (size_t i = 0; i < species.length(); i++) {
        packet[len++] = species[i];
    }
    
    // Determine message priority based on alert priority
    MeshPriority meshPriority;
    switch (alert.priority) {
        case ALERT_PRIORITY_EMERGENCY:
            meshPriority = PRIORITY_EMERGENCY;
            break;
        case ALERT_PRIORITY_URGENT:
        case ALERT_PRIORITY_HIGH:
            meshPriority = PRIORITY_HIGH;
            break;
        default:
            meshPriority = PRIORITY_NORMAL;
            break;
    }
    
    return _meshManager.broadcastMessage(packet, len, meshPriority);
}

bool AdvancedMeshNetwork::acknowledgeAlert(uint32_t alertId) {
    if (!_initialized) {
        return false;
    }
    
    // Find and mark alert as acknowledged
    for (auto& alert : _recentAlerts) {
        if (alert.alertId == alertId) {
            alert.acknowledged = true;
            break;
        }
    }
    
    // Send acknowledgment
    uint8_t packet[8];
    packet[0] = ADV_PACKET_ALERT_ACK;
    packet[1] = 0;
    uint32_t nodeId = _meshManager.getNodeId();
    packet[2] = (nodeId >> 24) & 0xFF;
    packet[3] = (nodeId >> 16) & 0xFF;
    packet[4] = (nodeId >> 8) & 0xFF;
    packet[5] = nodeId & 0xFF;
    packet[6] = (alertId >> 8) & 0xFF;
    packet[7] = alertId & 0xFF;
    
    return _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_NORMAL);
}

std::vector<SharedAlert> AdvancedMeshNetwork::getRecentAlerts() const {
    return _recentAlerts;
}

SharedAlert AdvancedMeshNetwork::getAlert(uint32_t alertId) const {
    for (const auto& alert : _recentAlerts) {
        if (alert.alertId == alertId) {
            return alert;
        }
    }
    return SharedAlert();  // Empty alert
}

void AdvancedMeshNetwork::clearAlerts() {
    _recentAlerts.clear();
    _alertDedup.clear();
    LOG_INFO("Cleared all alerts");
}

uint16_t AdvancedMeshNetwork::getPendingAlertCount() const {
    uint16_t count = 0;
    for (const auto& alert : _recentAlerts) {
        if (!alert.acknowledged) {
            count++;
        }
    }
    return count;
}

bool AdvancedMeshNetwork::isDuplicateAlert(uint32_t alertId, uint32_t originNodeId) const {
    unsigned long now = millis();
    
    for (const auto& dedup : _alertDedup) {
        if (dedup.alertId == alertId && dedup.originNodeId == originNodeId) {
            // Check if within dedup window
            if (now - dedup.receivedTime < MESH_ALERT_DEDUP_WINDOW_MS) {
                return true;
            }
        }
    }
    
    return false;
}

//==============================================================================
// CALLBACKS
//==============================================================================

void AdvancedMeshNetwork::setCameraCoordCallback(CameraCoordCallback callback) {
    _coordCallback = callback;
}

void AdvancedMeshNetwork::setSharedAlertCallback(SharedAlertCallback callback) {
    _alertCallback = callback;
}

void AdvancedMeshNetwork::setRangeChangeCallback(RangeChangeCallback callback) {
    _rangeCallback = callback;
}

//==============================================================================
// UTILITY
//==============================================================================

uint32_t AdvancedMeshNetwork::generateAlertId() {
    return ++_alertIdCounter;
}

String AdvancedMeshNetwork::getNetworkStatistics() const {
    String stats = "{";
    stats += "\"initialized\":" + String(_initialized ? "true" : "false") + ",";
    stats += "\"cameraRole\":" + String((int)_cameraRole) + ",";
    stats += "\"syncState\":" + String((int)_syncState) + ",";
    stats += "\"groupActive\":" + String(_currentGroup.isActive ? "true" : "false") + ",";
    stats += "\"groupId\":" + String(_currentGroup.groupId) + ",";
    stats += "\"groupMembers\":" + String(_currentGroup.memberCount) + ",";
    stats += "\"rangeMode\":" + String((int)_rangeMode) + ",";
    stats += "\"relayNodes\":" + String(_relayNodes.size()) + ",";
    stats += "\"recentAlerts\":" + String(_recentAlerts.size()) + ",";
    stats += "\"pendingAlerts\":" + String(getPendingAlertCount());
    stats += "}";
    return stats;
}

//==============================================================================
// PRIVATE METHODS
//==============================================================================

void AdvancedMeshNetwork::processCoordination() {
    unsigned long now = millis();
    
    // Send periodic coordination sync
    if (_currentGroup.isActive && now - _lastSyncTime > MESH_COORD_SYNC_INTERVAL_MS) {
        sendCoordinationSync();
        _lastSyncTime = now;
    }
    
    // Check for stale cameras in group
    for (auto it = _groupCameras.begin(); it != _groupCameras.end();) {
        if (now - it->lastCapture > MESH_NODE_TIMEOUT_MS && 
            it->nodeId != _meshManager.getNodeId()) {
            LOG_DEBUG("Removing stale camera 0x%08X from group", it->nodeId);
            it = _groupCameras.erase(it);
        } else {
            ++it;
        }
    }
}

void AdvancedMeshNetwork::processExtendedRange() {
    unsigned long now = millis();
    
    // Check range settings periodically (every 30 seconds)
    if (now - _lastRangeCheck > 30000) {
        if (_rangeMode == RANGE_MODE_ADAPTIVE) {
            // Get average RSSI from known nodes
            int totalRssi = 0;
            int nodeCount = 0;
            
            auto nodes = _meshManager.getNodes();
            for (const auto& node : nodes) {
                if (node.isOnline) {
                    totalRssi += node.lastRssi;
                    nodeCount++;
                }
            }
            
            if (nodeCount > 0) {
                int avgRssi = totalRssi / nodeCount;
                
                // Adapt range based on average signal strength
                RangeSettings newSettings;
                if (avgRssi < -100) {
                    newSettings = getSettingsForMode(RANGE_MODE_MAXIMUM);
                } else if (avgRssi < MESH_EXTENDED_RANGE_RSSI_THRESHOLD) {
                    newSettings = getSettingsForMode(RANGE_MODE_EXTENDED);
                } else {
                    newSettings = getSettingsForMode(RANGE_MODE_NORMAL);
                }
                
                // Apply if different
                if (newSettings.spreadingFactor != _currentSettings.spreadingFactor) {
                    applyRangeSettings(newSettings);
                }
            }
        }
        
        // Clean up stale relay nodes
        for (auto it = _relayNodes.begin(); it != _relayNodes.end();) {
            if (now - it->lastSeen > MESH_NODE_TIMEOUT_MS) {
                LOG_DEBUG("Removing stale relay 0x%08X", it->nodeId);
                it = _relayNodes.erase(it);
            } else {
                ++it;
            }
        }
        
        _lastRangeCheck = now;
    }
}

void AdvancedMeshNetwork::processAlerts() {
    unsigned long now = millis();
    
    // Clean up old dedup entries periodically
    if (now - _lastAlertCleanup > 60000) {
        cleanupOldAlerts();
        _lastAlertCleanup = now;
    }
}

void AdvancedMeshNetwork::sendCoordinationSync() {
    if (!_currentGroup.isActive) {
        return;
    }
    
    uint8_t packet[32];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_COORD_SYNC;
    packet[len++] = _currentGroup.groupId;
    
    // Primary node ID
    packet[len++] = (_currentGroup.primaryNodeId >> 24) & 0xFF;
    packet[len++] = (_currentGroup.primaryNodeId >> 16) & 0xFF;
    packet[len++] = (_currentGroup.primaryNodeId >> 8) & 0xFF;
    packet[len++] = _currentGroup.primaryNodeId & 0xFF;
    
    // This node ID
    uint32_t myNodeId = _meshManager.getNodeId();
    packet[len++] = (myNodeId >> 24) & 0xFF;
    packet[len++] = (myNodeId >> 16) & 0xFF;
    packet[len++] = (myNodeId >> 8) & 0xFF;
    packet[len++] = myNodeId & 0xFF;
    
    // Member count
    packet[len++] = _currentGroup.memberCount;
    
    // Role and sync state
    packet[len++] = (uint8_t)_cameraRole;
    packet[len++] = (uint8_t)_syncState;
    
    _meshManager.broadcastMessage(packet, len, PRIORITY_LOW);
}

void AdvancedMeshNetwork::handleCoordinationMessage(uint32_t fromNode, const uint8_t* data, size_t length) {
    if (length < 2) {
        return;
    }
    
    uint8_t packetType = data[0];
    uint8_t groupId = data[1];
    
    // Only process if for our group
    if (_currentGroup.isActive && groupId != _currentGroup.groupId) {
        return;
    }
    
    switch (packetType) {
        case ADV_PACKET_COORD_SYNC:
            // Handle sync message
            if (length >= 13) {
                uint32_t primaryNode = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                                       ((uint32_t)data[4] << 8) | data[5];
                
                if (_currentGroup.primaryNodeId == 0) {
                    _currentGroup.primaryNodeId = primaryNode;
                }
                
                if (_syncState == SYNC_STATE_SYNCING) {
                    _syncState = SYNC_STATE_SYNCED;
                    LOG_INFO("Synchronized with group %d", groupId);
                }
            }
            break;
            
        case ADV_PACKET_COORD_STATUS:
            // Handle status update
            if (length >= 20) {
                CameraStatus status;
                status.nodeId = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                               ((uint32_t)data[4] << 8) | data[5];
                status.role = (CameraRole)data[6];
                status.syncState = (CameraSyncState)data[7];
                status.batteryLevel = data[8];
                status.isCapturing = (data[9] & 0x01) != 0;
                status.hasSDSpace = (data[9] & 0x02) != 0;
                status.signalStrength = ((int16_t)data[10] << 8) | data[11];
                status.lastCapture = ((uint32_t)data[12] << 24) | ((uint32_t)data[13] << 16) |
                                    ((uint32_t)data[14] << 8) | data[15];
                status.imageCount = ((uint32_t)data[16] << 24) | ((uint32_t)data[17] << 16) |
                                   ((uint32_t)data[18] << 8) | data[19];
                
                // Update or add camera
                bool found = false;
                for (auto& cam : _groupCameras) {
                    if (cam.nodeId == status.nodeId) {
                        cam = status;
                        found = true;
                        break;
                    }
                }
                
                if (!found && _groupCameras.size() < MESH_MAX_CAMERAS) {
                    _groupCameras.push_back(status);
                    
                    if (_coordCallback) {
                        _coordCallback(status, true);
                    }
                } else if (_coordCallback) {
                    _coordCallback(status, false);
                }
            }
            break;
            
        case ADV_PACKET_COORD_CAPTURE:
            // Handle capture request
            if (length >= 10) {
                _syncCaptureRequested = true;
                LOG_INFO("Received sync capture request for group %d", groupId);
            }
            break;
            
        case ADV_PACKET_COORD_JOIN:
            // Handle join request
            if (length >= 6 && _cameraRole == CAMERA_ROLE_PRIMARY) {
                uint32_t joiningNode = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                                       ((uint32_t)data[4] << 8) | data[5];
                
                // Add to members if space available
                if (_currentGroup.memberCount < MESH_MAX_CAMERAS) {
                    _currentGroup.memberNodes[_currentGroup.memberCount] = joiningNode;
                    _currentGroup.memberCount++;
                    LOG_INFO("Node 0x%08X joined group %d", joiningNode, groupId);
                    
                    // Send updated sync
                    sendCoordinationSync();
                }
            }
            break;
            
        case ADV_PACKET_COORD_LEAVE:
            // Handle leave notification
            if (length >= 6) {
                uint32_t leavingNode = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                                       ((uint32_t)data[4] << 8) | data[5];
                
                // Remove from members
                for (uint8_t i = 0; i < _currentGroup.memberCount; i++) {
                    if (_currentGroup.memberNodes[i] == leavingNode) {
                        // Shift remaining
                        for (uint8_t j = i; j < _currentGroup.memberCount - 1; j++) {
                            _currentGroup.memberNodes[j] = _currentGroup.memberNodes[j + 1];
                        }
                        _currentGroup.memberCount--;
                        LOG_INFO("Node 0x%08X left group %d", leavingNode, groupId);
                        break;
                    }
                }
                
                // Remove from camera list
                for (auto it = _groupCameras.begin(); it != _groupCameras.end(); ++it) {
                    if (it->nodeId == leavingNode) {
                        _groupCameras.erase(it);
                        break;
                    }
                }
            }
            break;
    }
}

void AdvancedMeshNetwork::handleAlertMessage(uint32_t fromNode, const uint8_t* data, size_t length) {
    if (length < 3) {
        return;
    }
    
    uint8_t packetType = data[0];
    
    switch (packetType) {
        case ADV_PACKET_ALERT:
            // Parse alert
            if (length >= 18) {
                SharedAlert alert;
                alert.priority = (AlertPriority)data[1];
                alert.type = (AlertType)data[2];
                
                alert.alertId = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) |
                               ((uint32_t)data[5] << 8) | data[6];
                alert.originNodeId = ((uint32_t)data[7] << 24) | ((uint32_t)data[8] << 16) |
                                    ((uint32_t)data[9] << 8) | data[10];
                alert.timestamp = ((uint32_t)data[11] << 24) | ((uint32_t)data[12] << 16) |
                                 ((uint32_t)data[13] << 8) | data[14];
                alert.confidence = data[15] / 255.0f;
                alert.hopCount = data[16] + 1;  // Increment hop count
                alert.hasImage = data[17] != 0;
                
                // Parse species name
                if (length > 18) {
                    uint8_t speciesLen = data[18];
                    if (speciesLen > 0 && length >= 19 + speciesLen) {
                        alert.species = String((char*)&data[19], speciesLen);
                    }
                }
                
                alert.acknowledged = false;
                
                // Check for duplicate
                if (!isDuplicateAlert(alert.alertId, alert.originNodeId)) {
                    // Add to dedup
                    addAlertDedup(alert.alertId, alert.originNodeId);
                    
                    // Store alert
                    if (_recentAlerts.size() >= MESH_MAX_RECENT_ALERTS) {
                        _recentAlerts.erase(_recentAlerts.begin());
                    }
                    _recentAlerts.push_back(alert);
                    
                    LOG_INFO("Received alert %08X from 0x%08X: %s", 
                             alert.alertId, alert.originNodeId, alert.species.c_str());
                    
                    // Notify callback
                    if (_alertCallback) {
                        _alertCallback(alert);
                    }
                    
                    // Forward alert (rebroadcast) if hop count allows
                    if (alert.hopCount < MESH_MAX_HOPS) {
                        broadcastAlert(alert);
                    }
                }
            }
            break;
            
        case ADV_PACKET_ALERT_ACK:
            // Handle acknowledgment
            if (length >= 8) {
                uint32_t alertId = ((uint32_t)data[6] << 8) | data[7];
                
                for (auto& alert : _recentAlerts) {
                    if (alert.alertId == alertId) {
                        alert.acknowledged = true;
                        LOG_DEBUG("Alert %08X acknowledged", alertId);
                        break;
                    }
                }
            }
            break;
    }
}

void AdvancedMeshNetwork::updateRangeSettings() {
    RangeSettings settings = getSettingsForMode(_rangeMode);
    applyRangeSettings(settings);
}

void AdvancedMeshNetwork::cleanupOldAlerts() {
    unsigned long now = millis();
    
    // Remove old dedup entries
    for (auto it = _alertDedup.begin(); it != _alertDedup.end();) {
        if (now - it->receivedTime > MESH_ALERT_DEDUP_WINDOW_MS * 10) {
            it = _alertDedup.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remove acknowledged alerts older than 5 minutes
    for (auto it = _recentAlerts.begin(); it != _recentAlerts.end();) {
        if (it->acknowledged && now - it->timestamp > 300000) {
            it = _recentAlerts.erase(it);
        } else {
            ++it;
        }
    }
}

void AdvancedMeshNetwork::addAlertDedup(uint32_t alertId, uint32_t originNode) {
    // Remove old entry if exists
    for (auto it = _alertDedup.begin(); it != _alertDedup.end(); ++it) {
        if (it->alertId == alertId && it->originNodeId == originNode) {
            it->receivedTime = millis();
            return;
        }
    }
    
    // Add new entry
    if (_alertDedup.size() >= MESH_MAX_RECENT_ALERTS * 2) {
        _alertDedup.erase(_alertDedup.begin());
    }
    
    AlertDedup dedup;
    dedup.alertId = alertId;
    dedup.originNodeId = originNode;
    dedup.receivedTime = millis();
    _alertDedup.push_back(dedup);
}

bool AdvancedMeshNetwork::applyRangeSettings(const RangeSettings& settings) {
    _currentSettings = settings;
    
    LOG_INFO("Applied range settings: SF%d, BW=%lukHz, CR=4/%d, TX=%ddBm",
             settings.spreadingFactor, settings.bandwidth / 1000,
             settings.codingRate, settings.txPower);
    
    // Note: Actual LoRa radio settings are applied by the MeshManager
    // This just stores the desired settings for reference
    
    return true;
}

RangeSettings AdvancedMeshNetwork::getSettingsForMode(ExtendedRangeMode mode) const {
    RangeSettings settings;
    
    switch (mode) {
        case RANGE_MODE_MAXIMUM:
            // Maximum range: SF12, lowest BW, highest coding rate
            settings.spreadingFactor = 12;
            settings.bandwidth = 125000;
            settings.codingRate = 8;
            settings.txPower = 20;
            break;
            
        case RANGE_MODE_EXTENDED:
            // Extended range: SF10, medium settings
            settings.spreadingFactor = 10;
            settings.bandwidth = 125000;
            settings.codingRate = 6;
            settings.txPower = 17;
            break;
            
        case RANGE_MODE_NORMAL:
        case RANGE_MODE_ADAPTIVE:
        default:
            // Normal range: balanced settings
            settings.spreadingFactor = 9;
            settings.bandwidth = 125000;
            settings.codingRate = 5;
            settings.txPower = 14;
            break;
    }
    
    return settings;
}

//==============================================================================
// FEDERATED LEARNING OVER MESH IMPLEMENTATION
//==============================================================================

uint32_t AdvancedMeshNetwork::startFLRound(const String& modelType, uint8_t targetParticipants, 
                                           uint32_t deadlineMs) {
    if (!_initialized) {
        return 0;
    }
    
    // Cannot start new round while one is active
    if (_currentFLRound.state != FL_STATE_IDLE && _currentFLRound.state != FL_STATE_COMPLETE) {
        LOG_WARN("Cannot start FL round: round already active");
        return 0;
    }
    
    // Generate new round ID
    uint32_t roundId = generateFLRoundId();
    
    // Initialize round
    _currentFLRound.roundId = roundId;
    _currentFLRound.coordinatorId = _meshManager.getNodeId();
    _currentFLRound.startTime = millis();
    _currentFLRound.deadline = millis() + deadlineMs;
    _currentFLRound.state = FL_STATE_ANNOUNCING;
    _currentFLRound.participantCount = 1;  // Include ourselves
    _currentFLRound.updatesReceived = 0;
    _currentFLRound.targetParticipants = targetParticipants;
    _currentFLRound.modelType = modelType;
    
    // Set ourselves as coordinator
    _flRole = FL_ROLE_AGGREGATOR;
    
    // Clear old participants
    _flParticipants.clear();
    _pendingChunks.clear();
    
    // Add ourselves as first participant
    FLParticipant self;
    self.nodeId = _meshManager.getNodeId();
    self.role = FL_ROLE_AGGREGATOR;
    self.batteryLevel = 100;  // Placeholder
    self.availableMemory = ESP.getFreeHeap();
    self.reliability = 1.0f;
    self.lastSeen = millis();
    self.updateSubmitted = false;
    self.signalStrength = 0;
    _flParticipants.push_back(self);
    
    LOG_INFO("Started FL round %08X for model '%s', target: %d participants",
             roundId, modelType.c_str(), targetParticipants);
    Serial.printf("Started federated learning round %08X\n", roundId);
    
    // Broadcast round announcement
    broadcastFLRoundAnnouncement();
    
    // Notify callback
    if (_flRoundCallback) {
        _flRoundCallback(_currentFLRound);
    }
    
    return roundId;
}

bool AdvancedMeshNetwork::joinFLRound(uint32_t roundId) {
    if (!_initialized) {
        return false;
    }
    
    if (_flRole == FL_ROLE_AGGREGATOR) {
        LOG_WARN("Cannot join FL round: this node is coordinator");
        return false;
    }
    
    // Find the coordinator for this round
    // For now, we assume we received an announcement with the coordinator info
    if (_currentFLRound.roundId != roundId) {
        LOG_WARN("Unknown FL round: %08X", roundId);
        return false;
    }
    
    _flRole = FL_ROLE_PARTICIPANT;
    
    // Send join request to coordinator
    sendFLJoinRequest(_currentFLRound.coordinatorId);
    
    LOG_INFO("Requested to join FL round %08X", roundId);
    
    return true;
}

void AdvancedMeshNetwork::leaveFLRound() {
    if (_currentFLRound.state == FL_STATE_IDLE) {
        return;
    }
    
    LOG_INFO("Leaving FL round %08X", _currentFLRound.roundId);
    
    // Send leave notification
    uint8_t packet[12];
    packet[0] = ADV_PACKET_FL_LEAVE;
    uint32_t roundId = _currentFLRound.roundId;
    packet[1] = (roundId >> 24) & 0xFF;
    packet[2] = (roundId >> 16) & 0xFF;
    packet[3] = (roundId >> 8) & 0xFF;
    packet[4] = roundId & 0xFF;
    uint32_t nodeId = _meshManager.getNodeId();
    packet[5] = (nodeId >> 24) & 0xFF;
    packet[6] = (nodeId >> 16) & 0xFF;
    packet[7] = (nodeId >> 8) & 0xFF;
    packet[8] = nodeId & 0xFF;
    packet[9] = 0;  // Reserved
    packet[10] = 0;
    packet[11] = 0;
    
    _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_NORMAL);
    
    // Reset state
    _currentFLRound.state = FL_STATE_IDLE;
    _currentFLRound.roundId = 0;
    _flRole = FL_ROLE_PARTICIPANT;
    _flParticipants.clear();
    _pendingChunks.clear();
}

bool AdvancedMeshNetwork::submitModelUpdate(const uint8_t* gradients, size_t gradientSize,
                                            uint32_t localSamples, float localLoss) {
    if (!_initialized || gradients == nullptr || gradientSize == 0) {
        return false;
    }
    
    if (_currentFLRound.state != FL_STATE_TRAINING) {
        LOG_WARN("Cannot submit update: FL round not in training state");
        return false;
    }
    
    LOG_INFO("Submitting model update: %zu bytes, %u samples, loss=%.4f",
             gradientSize, localSamples, localLoss);
    
    // Compress gradients if needed
    size_t compressedSize = gradientSize;
    const uint8_t* dataToSend = gradients;
    uint8_t* compressedData = nullptr;
    
    if (_flCompression != FL_COMPRESS_NONE) {
        compressedData = compressGradients(gradients, gradientSize, &compressedSize);
        if (compressedData != nullptr) {
            dataToSend = compressedData;
            LOG_DEBUG("Compressed gradients: %zu -> %zu bytes (%.1f%%)",
                     gradientSize, compressedSize, (100.0f * compressedSize / gradientSize));
        }
    }
    
    // Split into chunks and send
    uint16_t totalChunks = (compressedSize + MESH_FL_MAX_CHUNK_SIZE - 1) / MESH_FL_MAX_CHUNK_SIZE;
    
    for (uint16_t i = 0; i < totalChunks; i++) {
        FLModelChunk chunk;
        chunk.roundId = _currentFLRound.roundId;
        chunk.originNodeId = _meshManager.getNodeId();
        chunk.chunkIndex = i;
        chunk.totalChunks = totalChunks;
        chunk.modelVersion = 1;  // Placeholder
        chunk.compression = _flCompression;
        
        size_t offset = i * MESH_FL_MAX_CHUNK_SIZE;
        size_t remaining = compressedSize - offset;
        chunk.dataSize = (remaining < MESH_FL_MAX_CHUNK_SIZE) ? remaining : MESH_FL_MAX_CHUNK_SIZE;
        memcpy(chunk.data, dataToSend + offset, chunk.dataSize);
        
        if (!sendModelChunk(chunk)) {
            LOG_ERROR("Failed to send model chunk %d/%d", i + 1, totalChunks);
            if (compressedData != nullptr) {
                free(compressedData);
            }
            return false;
        }
        
        // Small delay between chunks
        delay(50);
        yield();
    }
    
    if (compressedData != nullptr) {
        free(compressedData);
    }
    
    _flUpdatePending = false;
    
    // Notify callback
    if (_flModelUpdateCallback) {
        _flModelUpdateCallback(_meshManager.getNodeId(), _currentFLRound.roundId, true);
    }
    
    LOG_INFO("Model update submitted: %d chunks sent", totalChunks);
    
    return true;
}

FLRound AdvancedMeshNetwork::getCurrentFLRound() const {
    return _currentFLRound;
}

std::vector<FLParticipant> AdvancedMeshNetwork::getFLParticipants() const {
    return _flParticipants;
}

bool AdvancedMeshNetwork::isFLCoordinator() const {
    return _flRole == FL_ROLE_AGGREGATOR;
}

void AdvancedMeshNetwork::setFLRole(FLNodeRole role) {
    _flRole = role;
}

FLNodeRole AdvancedMeshNetwork::getFLRole() const {
    return _flRole;
}

FLAggregationStats AdvancedMeshNetwork::getFLAggregationStats() const {
    return _lastAggregationStats;
}

bool AdvancedMeshNetwork::isFLActive() const {
    return _currentFLRound.state != FL_STATE_IDLE && 
           _currentFLRound.state != FL_STATE_COMPLETE;
}

void AdvancedMeshNetwork::setFLCompression(FLCompressionType compression) {
    _flCompression = compression;
}

bool AdvancedMeshNetwork::requestGlobalModel() {
    if (!_initialized || _currentFLRound.state == FL_STATE_IDLE) {
        return false;
    }
    
    // Request from coordinator
    uint8_t packet[8];
    packet[0] = ADV_PACKET_FL_MODEL_CHUNK;  // Reuse as request
    packet[1] = 0xFF;  // Request flag
    uint32_t roundId = _currentFLRound.roundId;
    packet[2] = (roundId >> 24) & 0xFF;
    packet[3] = (roundId >> 16) & 0xFF;
    packet[4] = (roundId >> 8) & 0xFF;
    packet[5] = roundId & 0xFF;
    packet[6] = 0;
    packet[7] = 0;
    
    return _meshManager.sendMessage(_currentFLRound.coordinatorId, packet, sizeof(packet), PRIORITY_HIGH);
}

void AdvancedMeshNetwork::setFLRoundCallback(FLRoundCallback callback) {
    _flRoundCallback = callback;
}

void AdvancedMeshNetwork::setFLModelUpdateCallback(FLModelUpdateCallback callback) {
    _flModelUpdateCallback = callback;
}

void AdvancedMeshNetwork::setFLAggregationCallback(FLAggregationCallback callback) {
    _flAggregationCallback = callback;
}

//==============================================================================
// FEDERATED LEARNING INTERNAL METHODS
//==============================================================================

void AdvancedMeshNetwork::processFederatedLearning() {
    unsigned long now = millis();
    
    // Process every 100ms
    if (now - _lastFLProcess < 100) {
        return;
    }
    _lastFLProcess = now;
    
    // Skip if no active round
    if (_currentFLRound.state == FL_STATE_IDLE) {
        return;
    }
    
    // Check for round timeout
    if (now > _currentFLRound.deadline) {
        if (_flRole == FL_ROLE_AGGREGATOR) {
            // Coordinator: check if we have enough updates to aggregate
            float participationRatio = (float)_currentFLRound.updatesReceived / 
                                       _currentFLRound.targetParticipants;
            
            if (participationRatio >= _currentFLRound.minParticipationRatio) {
                LOG_INFO("FL round deadline reached with %.1f%% participation, aggregating",
                        participationRatio * 100);
                _currentFLRound.state = FL_STATE_AGGREGATING;
                aggregateModelUpdates();
            } else {
                LOG_WARN("FL round failed: insufficient participation (%.1f%%)",
                        participationRatio * 100);
                _currentFLRound.state = FL_STATE_COMPLETE;
            }
        } else {
            // Participant: round expired
            LOG_WARN("FL round %08X timed out", _currentFLRound.roundId);
            _currentFLRound.state = FL_STATE_COMPLETE;
        }
    }
    
    // Process state-specific logic
    switch (_currentFLRound.state) {
        case FL_STATE_ANNOUNCING:
            // Coordinator: wait for participants
            if (_flRole == FL_ROLE_AGGREGATOR) {
                if (_currentFLRound.participantCount >= _currentFLRound.targetParticipants) {
                    LOG_INFO("FL round has enough participants, starting training");
                    _currentFLRound.state = FL_STATE_TRAINING;
                    
                    // Broadcast training start
                    uint8_t packet[8];
                    packet[0] = ADV_PACKET_FL_ROUND_ANNOUNCE;
                    packet[1] = 0x02;  // Training start signal
                    uint32_t roundId = _currentFLRound.roundId;
                    packet[2] = (roundId >> 24) & 0xFF;
                    packet[3] = (roundId >> 16) & 0xFF;
                    packet[4] = (roundId >> 8) & 0xFF;
                    packet[5] = roundId & 0xFF;
                    packet[6] = 0;
                    packet[7] = 0;
                    _meshManager.broadcastMessage(packet, sizeof(packet), PRIORITY_HIGH);
                }
            }
            break;
            
        case FL_STATE_TRAINING:
            // Participants perform local training (handled externally)
            break;
            
        case FL_STATE_COLLECTING:
            // Coordinator: collecting updates
            processReceivedChunks();
            break;
            
        case FL_STATE_AGGREGATING:
            // Aggregation happens in aggregateModelUpdates()
            break;
            
        case FL_STATE_DISTRIBUTING:
            // Distribution happens in distributeAggregatedModel()
            break;
            
        case FL_STATE_COMPLETE:
            // Cleanup old round data after 30 seconds
            if (now - _currentFLRound.startTime > 30000) {
                _currentFLRound.state = FL_STATE_IDLE;
                _pendingChunks.clear();
            }
            break;
            
        default:
            break;
    }
    
    // Clean up stale participants
    for (auto it = _flParticipants.begin(); it != _flParticipants.end();) {
        if (now - it->lastSeen > MESH_FL_UPDATE_TIMEOUT_MS && 
            it->nodeId != _meshManager.getNodeId()) {
            LOG_DEBUG("Removing stale FL participant 0x%08X", it->nodeId);
            it = _flParticipants.erase(it);
            _currentFLRound.participantCount = _flParticipants.size();
        } else {
            ++it;
        }
    }
}

void AdvancedMeshNetwork::broadcastFLRoundAnnouncement() {
    uint8_t packet[48];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_FL_ROUND_ANNOUNCE;
    packet[len++] = 0x01;  // Announcement type
    
    // Round ID
    packet[len++] = (_currentFLRound.roundId >> 24) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 16) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 8) & 0xFF;
    packet[len++] = _currentFLRound.roundId & 0xFF;
    
    // Coordinator ID
    packet[len++] = (_currentFLRound.coordinatorId >> 24) & 0xFF;
    packet[len++] = (_currentFLRound.coordinatorId >> 16) & 0xFF;
    packet[len++] = (_currentFLRound.coordinatorId >> 8) & 0xFF;
    packet[len++] = _currentFLRound.coordinatorId & 0xFF;
    
    // Deadline (ms from now)
    uint32_t deadlineMs = _currentFLRound.deadline - millis();
    packet[len++] = (deadlineMs >> 24) & 0xFF;
    packet[len++] = (deadlineMs >> 16) & 0xFF;
    packet[len++] = (deadlineMs >> 8) & 0xFF;
    packet[len++] = deadlineMs & 0xFF;
    
    // Target participants
    packet[len++] = _currentFLRound.targetParticipants;
    
    // Model type (max 16 chars)
    String modelType = _currentFLRound.modelType.substring(0, 16);
    packet[len++] = modelType.length();
    for (size_t i = 0; i < modelType.length(); i++) {
        packet[len++] = modelType[i];
    }
    
    _meshManager.broadcastMessage(packet, len, PRIORITY_HIGH);
}

void AdvancedMeshNetwork::sendFLJoinRequest(uint32_t coordinatorId) {
    uint8_t packet[16];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_FL_JOIN_REQUEST;
    
    // Round ID
    packet[len++] = (_currentFLRound.roundId >> 24) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 16) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 8) & 0xFF;
    packet[len++] = _currentFLRound.roundId & 0xFF;
    
    // Our node ID
    uint32_t nodeId = _meshManager.getNodeId();
    packet[len++] = (nodeId >> 24) & 0xFF;
    packet[len++] = (nodeId >> 16) & 0xFF;
    packet[len++] = (nodeId >> 8) & 0xFF;
    packet[len++] = nodeId & 0xFF;
    
    // Battery level (placeholder)
    packet[len++] = 100;
    
    // Available memory (KB)
    uint32_t freeHeap = ESP.getFreeHeap() / 1024;
    packet[len++] = (freeHeap >> 8) & 0xFF;
    packet[len++] = freeHeap & 0xFF;
    
    _meshManager.sendMessage(coordinatorId, packet, len, PRIORITY_HIGH);
}

void AdvancedMeshNetwork::sendFLJoinResponse(uint32_t nodeId, bool accepted) {
    uint8_t packet[12];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_FL_JOIN_RESPONSE;
    
    // Round ID
    packet[len++] = (_currentFLRound.roundId >> 24) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 16) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 8) & 0xFF;
    packet[len++] = _currentFLRound.roundId & 0xFF;
    
    // Accepted flag
    packet[len++] = accepted ? 0x01 : 0x00;
    
    // Current participant count
    packet[len++] = _currentFLRound.participantCount;
    
    _meshManager.sendMessage(nodeId, packet, len, PRIORITY_HIGH);
}

bool AdvancedMeshNetwork::sendModelChunk(const FLModelChunk& chunk) {
    // Calculate packet size
    size_t packetSize = 16 + chunk.dataSize;
    uint8_t* packet = (uint8_t*)malloc(packetSize);
    if (packet == nullptr) {
        return false;
    }
    
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_FL_MODEL_CHUNK;
    
    // Round ID
    packet[len++] = (chunk.roundId >> 24) & 0xFF;
    packet[len++] = (chunk.roundId >> 16) & 0xFF;
    packet[len++] = (chunk.roundId >> 8) & 0xFF;
    packet[len++] = chunk.roundId & 0xFF;
    
    // Origin node ID
    packet[len++] = (chunk.originNodeId >> 24) & 0xFF;
    packet[len++] = (chunk.originNodeId >> 16) & 0xFF;
    packet[len++] = (chunk.originNodeId >> 8) & 0xFF;
    packet[len++] = chunk.originNodeId & 0xFF;
    
    // Chunk index and total
    packet[len++] = (chunk.chunkIndex >> 8) & 0xFF;
    packet[len++] = chunk.chunkIndex & 0xFF;
    packet[len++] = (chunk.totalChunks >> 8) & 0xFF;
    packet[len++] = chunk.totalChunks & 0xFF;
    
    // Compression type and data size
    packet[len++] = (uint8_t)chunk.compression;
    packet[len++] = (chunk.dataSize >> 8) & 0xFF;
    packet[len++] = chunk.dataSize & 0xFF;
    
    // Copy data
    memcpy(&packet[len], chunk.data, chunk.dataSize);
    len += chunk.dataSize;
    
    // Send to coordinator or broadcast
    bool success;
    if (_flRole == FL_ROLE_AGGREGATOR) {
        success = _meshManager.broadcastMessage(packet, len, PRIORITY_HIGH);
    } else {
        success = _meshManager.sendMessage(_currentFLRound.coordinatorId, packet, len, PRIORITY_HIGH);
    }
    
    free(packet);
    return success;
}

void AdvancedMeshNetwork::processReceivedChunks() {
    // Group chunks by origin node and check for complete updates
    // This is a simplified implementation
    
    if (_pendingChunks.empty()) {
        return;
    }
    
    // Track which nodes have complete updates
    std::vector<uint32_t> completedNodes;
    
    for (const auto& chunk : _pendingChunks) {
        // Count chunks from this node
        uint16_t receivedCount = 0;
        uint16_t expectedCount = chunk.totalChunks;
        
        for (const auto& c : _pendingChunks) {
            if (c.originNodeId == chunk.originNodeId && c.roundId == chunk.roundId) {
                receivedCount++;
            }
        }
        
        if (receivedCount >= expectedCount) {
            // Check if already counted
            bool alreadyCounted = false;
            for (uint32_t nodeId : completedNodes) {
                if (nodeId == chunk.originNodeId) {
                    alreadyCounted = true;
                    break;
                }
            }
            
            if (!alreadyCounted) {
                completedNodes.push_back(chunk.originNodeId);
                _currentFLRound.updatesReceived++;
                
                LOG_INFO("Received complete model update from node 0x%08X", chunk.originNodeId);
                
                // Mark participant as submitted
                for (auto& p : _flParticipants) {
                    if (p.nodeId == chunk.originNodeId) {
                        p.updateSubmitted = true;
                        break;
                    }
                }
                
                // Notify callback
                if (_flModelUpdateCallback) {
                    _flModelUpdateCallback(chunk.originNodeId, chunk.roundId, true);
                }
            }
        }
    }
}

void AdvancedMeshNetwork::aggregateModelUpdates() {
    LOG_INFO("Aggregating model updates from %d participants", _currentFLRound.updatesReceived);
    
    uint32_t startTime = millis();
    
    // In a real implementation, this would:
    // 1. Reassemble gradient chunks from each participant
    // 2. Decompress if necessary
    // 3. Apply FedAvg or other aggregation algorithm
    // 4. Update global model
    
    // For now, we just track statistics
    _lastAggregationStats.roundId = _currentFLRound.roundId;
    _lastAggregationStats.contributorCount = _currentFLRound.updatesReceived;
    _lastAggregationStats.totalSamples = _currentFLRound.updatesReceived * 100;  // Placeholder
    _lastAggregationStats.averageLoss = 0.5f;  // Placeholder
    _lastAggregationStats.accuracyImprovement = 0.02f;  // Placeholder
    _lastAggregationStats.aggregationTime = millis() - startTime;
    
    LOG_INFO("Aggregation complete in %lu ms", _lastAggregationStats.aggregationTime);
    
    _currentFLRound.state = FL_STATE_DISTRIBUTING;
    
    // Distribute aggregated model
    distributeAggregatedModel();
}

void AdvancedMeshNetwork::distributeAggregatedModel() {
    LOG_INFO("Distributing aggregated model to %d participants", _currentFLRound.participantCount);
    
    uint32_t startTime = millis();
    
    // Broadcast round completion
    uint8_t packet[24];
    size_t len = 0;
    
    packet[len++] = ADV_PACKET_FL_ROUND_COMPLETE;
    
    // Round ID
    packet[len++] = (_currentFLRound.roundId >> 24) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 16) & 0xFF;
    packet[len++] = (_currentFLRound.roundId >> 8) & 0xFF;
    packet[len++] = _currentFLRound.roundId & 0xFF;
    
    // Stats
    packet[len++] = _lastAggregationStats.contributorCount;
    
    // Accuracy improvement (scaled)
    uint16_t improvement = (uint16_t)(_lastAggregationStats.accuracyImprovement * 10000);
    packet[len++] = (improvement >> 8) & 0xFF;
    packet[len++] = improvement & 0xFF;
    
    _meshManager.broadcastMessage(packet, len, PRIORITY_HIGH);
    
    _lastAggregationStats.distributionTime = millis() - startTime;
    
    LOG_INFO("Distribution complete in %lu ms", _lastAggregationStats.distributionTime);
    
    // Mark round complete
    _currentFLRound.state = FL_STATE_COMPLETE;
    
    // Notify callback
    if (_flAggregationCallback) {
        _flAggregationCallback(_lastAggregationStats);
    }
    
    // Clear chunks
    _pendingChunks.clear();
}

uint8_t* AdvancedMeshNetwork::compressGradients(const uint8_t* data, size_t size, size_t* compressedSize) {
    if (data == nullptr || size == 0) {
        *compressedSize = 0;
        return nullptr;
    }
    
    switch (_flCompression) {
        case FL_COMPRESS_QUANTIZE_8BIT:
            // Already 8-bit, just copy
            {
                uint8_t* result = (uint8_t*)malloc(size);
                if (result != nullptr) {
                    memcpy(result, data, size);
                    *compressedSize = size;
                }
                return result;
            }
            
        case FL_COMPRESS_QUANTIZE_4BIT:
            // Pack two 8-bit values into one byte
            {
                size_t newSize = (size + 1) / 2;
                uint8_t* result = (uint8_t*)malloc(newSize);
                if (result != nullptr) {
                    for (size_t i = 0; i < newSize; i++) {
                        uint8_t high = (i * 2 < size) ? (data[i * 2] >> 4) : 0;
                        uint8_t low = (i * 2 + 1 < size) ? (data[i * 2 + 1] >> 4) : 0;
                        result[i] = (high << 4) | low;
                    }
                    *compressedSize = newSize;
                }
                return result;
            }
            
        case FL_COMPRESS_SPARSE:
            // Simple sparse encoding: only send non-zero values
            // Format: [index:2][value:1]...
            {
                // Count non-zero values
                size_t nonZeroCount = 0;
                for (size_t i = 0; i < size; i++) {
                    if (data[i] != 0) {
                        nonZeroCount++;
                    }
                }
                
                // Check if sparse is actually smaller
                size_t sparseSize = nonZeroCount * 3 + 2;  // 2 bytes for count header
                if (sparseSize >= size) {
                    // Not worth it, just copy
                    uint8_t* result = (uint8_t*)malloc(size);
                    if (result != nullptr) {
                        memcpy(result, data, size);
                        *compressedSize = size;
                    }
                    return result;
                }
                
                uint8_t* result = (uint8_t*)malloc(sparseSize);
                if (result != nullptr) {
                    // Write count
                    result[0] = (nonZeroCount >> 8) & 0xFF;
                    result[1] = nonZeroCount & 0xFF;
                    
                    size_t offset = 2;
                    for (size_t i = 0; i < size && offset < sparseSize; i++) {
                        if (data[i] != 0) {
                            result[offset++] = (i >> 8) & 0xFF;
                            result[offset++] = i & 0xFF;
                            result[offset++] = data[i];
                        }
                    }
                    *compressedSize = sparseSize;
                }
                return result;
            }
            
        default:
            // No compression
            {
                uint8_t* result = (uint8_t*)malloc(size);
                if (result != nullptr) {
                    memcpy(result, data, size);
                    *compressedSize = size;
                }
                return result;
            }
    }
}

uint8_t* AdvancedMeshNetwork::decompressGradients(const uint8_t* data, size_t size, size_t* decompressedSize) {
    // Reverse of compressGradients
    // For now, just return a copy for QUANTIZE_8BIT
    uint8_t* result = (uint8_t*)malloc(size);
    if (result != nullptr) {
        memcpy(result, data, size);
        *decompressedSize = size;
    }
    return result;
}

uint32_t AdvancedMeshNetwork::generateFLRoundId() {
    // Generate unique round ID: high bits from node ID, low bits incremental
    _flRoundIdCounter++;
    return (_meshManager.getNodeId() & 0xFFFF0000) | (_flRoundIdCounter & 0xFFFF);
}

void AdvancedMeshNetwork::handleFLMessage(uint32_t fromNode, const uint8_t* data, size_t length) {
    if (length < 2) {
        return;
    }
    
    uint8_t packetType = data[0];
    
    switch (packetType) {
        case ADV_PACKET_FL_ROUND_ANNOUNCE:
            // Round announcement received
            if (length >= 16 && data[1] == 0x01) {
                uint32_t roundId = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) |
                                  ((uint32_t)data[4] << 8) | data[5];
                uint32_t coordinatorId = ((uint32_t)data[6] << 24) | ((uint32_t)data[7] << 16) |
                                        ((uint32_t)data[8] << 8) | data[9];
                uint32_t deadlineMs = ((uint32_t)data[10] << 24) | ((uint32_t)data[11] << 16) |
                                     ((uint32_t)data[12] << 8) | data[13];
                uint8_t targetParticipants = data[14];
                
                // Store round info
                _currentFLRound.roundId = roundId;
                _currentFLRound.coordinatorId = coordinatorId;
                _currentFLRound.deadline = millis() + deadlineMs;
                _currentFLRound.targetParticipants = targetParticipants;
                _currentFLRound.state = FL_STATE_ANNOUNCING;
                
                // Parse model type
                if (length > 15) {
                    uint8_t modelTypeLen = data[15];
                    if (modelTypeLen > 0 && length >= 16 + modelTypeLen) {
                        _currentFLRound.modelType = String((char*)&data[16], modelTypeLen);
                    }
                }
                
                LOG_INFO("Received FL round announcement: %08X from coordinator 0x%08X",
                        roundId, coordinatorId);
                
                // Notify callback
                if (_flRoundCallback) {
                    _flRoundCallback(_currentFLRound);
                }
            } else if (data[1] == 0x02) {
                // Training start signal
                _currentFLRound.state = FL_STATE_TRAINING;
                LOG_INFO("FL training started for round %08X", _currentFLRound.roundId);
            }
            break;
            
        case ADV_PACKET_FL_JOIN_REQUEST:
            // Join request (coordinator only)
            if (_flRole == FL_ROLE_AGGREGATOR && length >= 12) {
                uint32_t nodeId = ((uint32_t)data[5] << 24) | ((uint32_t)data[6] << 16) |
                                 ((uint32_t)data[7] << 8) | data[8];
                uint8_t batteryLevel = data[9];
                uint16_t freeMemKB = ((uint16_t)data[10] << 8) | data[11];
                
                // Check if we can accept
                bool accept = (_flParticipants.size() < MESH_FL_MAX_PARTICIPANTS);
                
                if (accept) {
                    FLParticipant participant;
                    participant.nodeId = nodeId;
                    participant.role = FL_ROLE_PARTICIPANT;
                    participant.batteryLevel = batteryLevel;
                    participant.availableMemory = freeMemKB * 1024;
                    participant.reliability = 1.0f;
                    participant.lastSeen = millis();
                    participant.updateSubmitted = false;
                    participant.signalStrength = 0;
                    
                    _flParticipants.push_back(participant);
                    _currentFLRound.participantCount = _flParticipants.size();
                    
                    LOG_INFO("Accepted FL participant 0x%08X (battery: %d%%, memory: %dKB)",
                            nodeId, batteryLevel, freeMemKB);
                }
                
                sendFLJoinResponse(nodeId, accept);
            }
            break;
            
        case ADV_PACKET_FL_JOIN_RESPONSE:
            // Join response (participant only)
            if (length >= 7) {
                bool accepted = data[5] != 0;
                uint8_t participantCount = data[6];
                
                if (accepted) {
                    _currentFLRound.participantCount = participantCount;
                    LOG_INFO("Joined FL round %08X (%d participants)",
                            _currentFLRound.roundId, participantCount);
                } else {
                    LOG_WARN("FL join request rejected");
                    _currentFLRound.state = FL_STATE_IDLE;
                }
            }
            break;
            
        case ADV_PACKET_FL_MODEL_CHUNK:
            // Model chunk received
            if (length >= 17) {
                FLModelChunk chunk;
                chunk.roundId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) |
                               ((uint32_t)data[3] << 8) | data[4];
                chunk.originNodeId = ((uint32_t)data[5] << 24) | ((uint32_t)data[6] << 16) |
                                    ((uint32_t)data[7] << 8) | data[8];
                chunk.chunkIndex = ((uint16_t)data[9] << 8) | data[10];
                chunk.totalChunks = ((uint16_t)data[11] << 8) | data[12];
                chunk.compression = (FLCompressionType)data[13];
                chunk.dataSize = ((uint16_t)data[14] << 8) | data[15];
                
                // Validate
                if (chunk.roundId != _currentFLRound.roundId) {
                    break;
                }
                
                // Copy data
                size_t dataOffset = 16;
                if (chunk.dataSize > 0 && length >= dataOffset + chunk.dataSize &&
                    chunk.dataSize <= MESH_FL_MAX_CHUNK_SIZE) {
                    memcpy(chunk.data, &data[dataOffset], chunk.dataSize);
                    _pendingChunks.push_back(chunk);
                    
                    LOG_DEBUG("Received FL chunk %d/%d from 0x%08X",
                             chunk.chunkIndex + 1, chunk.totalChunks, chunk.originNodeId);
                    
                    // Update participant last seen
                    for (auto& p : _flParticipants) {
                        if (p.nodeId == chunk.originNodeId) {
                            p.lastSeen = millis();
                            break;
                        }
                    }
                }
            }
            break;
            
        case ADV_PACKET_FL_ROUND_COMPLETE:
            // Round complete notification
            if (length >= 8) {
                uint32_t roundId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) |
                                  ((uint32_t)data[3] << 8) | data[4];
                uint8_t contributorCount = data[5];
                uint16_t improvementScaled = ((uint16_t)data[6] << 8) | data[7];
                float improvement = improvementScaled / 10000.0f;
                
                LOG_INFO("FL round %08X complete: %d contributors, %.2f%% improvement",
                        roundId, contributorCount, improvement * 100);
                
                _currentFLRound.state = FL_STATE_COMPLETE;
                
                // Update stats
                _lastAggregationStats.roundId = roundId;
                _lastAggregationStats.contributorCount = contributorCount;
                _lastAggregationStats.accuracyImprovement = improvement;
                
                if (_flAggregationCallback) {
                    _flAggregationCallback(_lastAggregationStats);
                }
            }
            break;
            
        case ADV_PACKET_FL_LEAVE:
            // Participant left
            if (length >= 9) {
                uint32_t nodeId = ((uint32_t)data[5] << 24) | ((uint32_t)data[6] << 16) |
                                 ((uint32_t)data[7] << 8) | data[8];
                
                for (auto it = _flParticipants.begin(); it != _flParticipants.end(); ++it) {
                    if (it->nodeId == nodeId) {
                        _flParticipants.erase(it);
                        _currentFLRound.participantCount = _flParticipants.size();
                        LOG_INFO("FL participant 0x%08X left round", nodeId);
                        break;
                    }
                }
            }
            break;
    }
}
