/**
 * @file AdvancedMeshNetwork.h
 * @brief Advanced LoRa Mesh Networking for WildCAM ESP32
 * @version 1.0.0
 * 
 * Provides advanced mesh networking features for multi-camera federated learning:
 * - Multi-camera coordination with role assignment for distributed ML training
 * - Extended range communication for model gradient/update sharing
 * - Shared detection alerts for collaborative learning feedback
 * - Federated learning protocol support over LoRa mesh
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef ADVANCED_MESH_NETWORK_H
#define ADVANCED_MESH_NETWORK_H

#include <Arduino.h>
#include <vector>
#include "MeshManager.h"

//==============================================================================
// ADVANCED MESH CONFIGURATION
//==============================================================================

/**
 * @brief Enable multi-camera coordination
 * @default true
 */
#ifndef MESH_MULTI_CAMERA_ENABLED
#define MESH_MULTI_CAMERA_ENABLED true
#endif

/**
 * @brief Enable extended range mode
 * @default true
 */
#ifndef MESH_EXTENDED_RANGE_ENABLED
#define MESH_EXTENDED_RANGE_ENABLED true
#endif

/**
 * @brief Enable shared detection alerts
 * @default true
 */
#ifndef MESH_SHARED_ALERTS_ENABLED
#define MESH_SHARED_ALERTS_ENABLED true
#endif

/**
 * @brief Maximum cameras in coordination group
 * @default 8
 */
#ifndef MESH_MAX_CAMERAS
#define MESH_MAX_CAMERAS 8
#endif

/**
 * @brief Coordination sync interval in milliseconds
 * @default 10000 (10 seconds)
 */
#ifndef MESH_COORD_SYNC_INTERVAL_MS
#define MESH_COORD_SYNC_INTERVAL_MS 10000
#endif

/**
 * @brief Alert deduplication window in milliseconds
 * @default 5000 (5 seconds)
 */
#ifndef MESH_ALERT_DEDUP_WINDOW_MS
#define MESH_ALERT_DEDUP_WINDOW_MS 5000
#endif

/**
 * @brief Maximum recent alerts to track for deduplication
 * @default 20
 */
#ifndef MESH_MAX_RECENT_ALERTS
#define MESH_MAX_RECENT_ALERTS 20
#endif

/**
 * @brief Extended range RSSI threshold (dBm)
 * @note Below this threshold, adaptive range settings are applied
 * @default -90
 */
#ifndef MESH_EXTENDED_RANGE_RSSI_THRESHOLD
#define MESH_EXTENDED_RANGE_RSSI_THRESHOLD -90
#endif

/**
 * @brief Relay hop optimization threshold
 * @note Routes with more hops than this trigger relay optimization
 * @default 3
 */
#ifndef MESH_RELAY_OPTIMIZATION_THRESHOLD
#define MESH_RELAY_OPTIMIZATION_THRESHOLD 3
#endif

/**
 * @brief Maximum nodes in mesh network (from config.h if not defined)
 * @default 16
 */
#ifndef MESH_MAX_NODES
#define MESH_MAX_NODES 16
#endif

/**
 * @brief Node timeout in milliseconds (from config.h if not defined)
 * @default 300000 (5 minutes)
 */
#ifndef MESH_NODE_TIMEOUT_MS
#define MESH_NODE_TIMEOUT_MS 300000
#endif

/**
 * @brief Maximum mesh network hops (from config.h if not defined)
 * @default 5
 */
#ifndef MESH_MAX_HOPS
#define MESH_MAX_HOPS 5
#endif

//==============================================================================
// MULTI-CAMERA COORDINATION STRUCTURES
//==============================================================================

/**
 * @brief Camera coordination role
 */
enum CameraRole {
    CAMERA_ROLE_STANDALONE = 0,    ///< Independent operation
    CAMERA_ROLE_PRIMARY = 1,       ///< Primary camera in group
    CAMERA_ROLE_SECONDARY = 2,     ///< Secondary camera in group
    CAMERA_ROLE_RELAY = 3,         ///< Relay-only node (no camera)
    CAMERA_ROLE_GATEWAY = 4        ///< Gateway to external network
};

/**
 * @brief Camera synchronization state
 */
enum CameraSyncState {
    SYNC_STATE_UNSYNC = 0,         ///< Not synchronized
    SYNC_STATE_SYNCING = 1,        ///< Synchronization in progress
    SYNC_STATE_SYNCED = 2,         ///< Fully synchronized
    SYNC_STATE_LEADER = 3          ///< Sync leader (time master)
};

/**
 * @brief Camera coordination group information
 */
struct CameraGroup {
    uint8_t groupId;               ///< Group identifier
    uint32_t primaryNodeId;        ///< Primary camera node ID
    uint32_t memberNodes[MESH_MAX_CAMERAS];  ///< Member node IDs
    uint8_t memberCount;           ///< Number of members
    uint32_t lastSync;             ///< Last sync timestamp
    bool isActive;                 ///< Group is active
};

/**
 * @brief Camera status information for coordination
 */
struct CameraStatus {
    uint32_t nodeId;               ///< Node identifier
    CameraRole role;               ///< Camera role
    CameraSyncState syncState;     ///< Sync state
    uint8_t batteryLevel;          ///< Battery percentage
    bool isCapturing;              ///< Currently capturing
    bool hasSDSpace;               ///< Has SD card space
    uint32_t lastCapture;          ///< Last capture timestamp
    uint32_t imageCount;           ///< Total images captured
    int16_t signalStrength;        ///< Average signal strength
};

//==============================================================================
// EXTENDED RANGE COMMUNICATION STRUCTURES
//==============================================================================

/**
 * @brief Extended range mode settings
 */
enum ExtendedRangeMode {
    RANGE_MODE_NORMAL = 0,         ///< Normal range (faster, shorter)
    RANGE_MODE_EXTENDED = 1,       ///< Extended range (slower, longer)
    RANGE_MODE_MAXIMUM = 2,        ///< Maximum range (slowest, longest)
    RANGE_MODE_ADAPTIVE = 3        ///< Adaptive based on signal quality
};

/**
 * @brief LoRa radio settings for different range modes
 */
struct RangeSettings {
    uint8_t spreadingFactor;       ///< SF7-SF12
    uint32_t bandwidth;            ///< Bandwidth in Hz
    uint8_t codingRate;            ///< Coding rate 5-8
    uint8_t txPower;               ///< TX power in dBm
};

/**
 * @brief Relay node information for extended range
 */
struct RelayNode {
    uint32_t nodeId;               ///< Relay node ID
    int16_t rssiToRelay;           ///< Signal strength to relay
    int16_t rssiFromRelay;         ///< Signal strength from relay
    uint8_t hopCount;              ///< Hops through this relay
    float reliability;             ///< Link reliability (0.0-1.0)
    uint32_t lastSeen;             ///< Last seen timestamp
    bool isActive;                 ///< Relay is active
};

//==============================================================================
// SHARED DETECTION ALERT STRUCTURES
//==============================================================================

/**
 * @brief Alert priority levels
 */
enum AlertPriority {
    ALERT_PRIORITY_LOW = 0,        ///< Low priority (routine)
    ALERT_PRIORITY_NORMAL = 1,     ///< Normal priority
    ALERT_PRIORITY_HIGH = 2,       ///< High priority (target species)
    ALERT_PRIORITY_URGENT = 3,     ///< Urgent (endangered species)
    ALERT_PRIORITY_EMERGENCY = 4   ///< Emergency (dangerous wildlife)
};

/**
 * @brief Alert type classification
 */
enum AlertType {
    ALERT_TYPE_DETECTION = 0,      ///< Wildlife detection
    ALERT_TYPE_MOTION = 1,         ///< Motion only (no ID)
    ALERT_TYPE_SPECIES = 2,        ///< Species identified
    ALERT_TYPE_BEHAVIOR = 3,       ///< Behavior detected
    ALERT_TYPE_INTRUSION = 4,      ///< Human/vehicle intrusion
    ALERT_TYPE_SYSTEM = 5          ///< System alert (low battery, etc.)
};

/**
 * @brief Shared detection alert
 */
struct SharedAlert {
    uint32_t alertId;              ///< Unique alert identifier
    uint32_t originNodeId;         ///< Originating node ID
    uint32_t timestamp;            ///< Alert timestamp
    AlertPriority priority;        ///< Alert priority
    AlertType type;                ///< Alert type
    String species;                ///< Species name (if identified)
    float confidence;              ///< Detection confidence
    float latitude;                ///< GPS latitude
    float longitude;               ///< GPS longitude
    uint8_t hopCount;              ///< Hops from origin
    bool hasImage;                 ///< Image available
    uint32_t imageSize;            ///< Image size if available
    bool acknowledged;             ///< Alert was acknowledged
};

/**
 * @brief Alert deduplication entry
 */
struct AlertDedup {
    uint32_t alertId;              ///< Alert ID
    uint32_t originNodeId;         ///< Origin node
    uint32_t receivedTime;         ///< Time received
};

//==============================================================================
// FEDERATED LEARNING MESH STRUCTURES
//==============================================================================

/**
 * @brief Enable federated learning over mesh
 * @default true
 */
#ifndef MESH_FEDERATED_LEARNING_ENABLED
#define MESH_FEDERATED_LEARNING_ENABLED true
#endif

/**
 * @brief Maximum model chunk size for LoRa transmission (bytes)
 * @default 200
 */
#ifndef MESH_FL_MAX_CHUNK_SIZE
#define MESH_FL_MAX_CHUNK_SIZE 200
#endif

/**
 * @brief Model update timeout in milliseconds
 * @default 30000 (30 seconds)
 */
#ifndef MESH_FL_UPDATE_TIMEOUT_MS
#define MESH_FL_UPDATE_TIMEOUT_MS 30000
#endif

/**
 * @brief Maximum participants in federated learning round
 * @default 16
 */
#ifndef MESH_FL_MAX_PARTICIPANTS
#define MESH_FL_MAX_PARTICIPANTS 16
#endif

/**
 * @brief Federated learning node role
 */
enum FLNodeRole {
    FL_ROLE_PARTICIPANT = 0,       ///< Regular training participant
    FL_ROLE_AGGREGATOR = 1,        ///< Model aggregator (coordinator)
    FL_ROLE_RELAY = 2,             ///< Relay node (forwards updates)
    FL_ROLE_OBSERVER = 3           ///< Observer only (no training)
};

/**
 * @brief Federated learning round state
 */
enum FLRoundState {
    FL_STATE_IDLE = 0,             ///< No active round
    FL_STATE_ANNOUNCING = 1,       ///< Round announced, waiting for participants
    FL_STATE_TRAINING = 2,         ///< Local training in progress
    FL_STATE_COLLECTING = 3,       ///< Collecting model updates
    FL_STATE_AGGREGATING = 4,      ///< Aggregating model updates
    FL_STATE_DISTRIBUTING = 5,     ///< Distributing aggregated model
    FL_STATE_COMPLETE = 6          ///< Round complete
};

/**
 * @brief Model update compression type
 */
enum FLCompressionType {
    FL_COMPRESS_NONE = 0,          ///< No compression
    FL_COMPRESS_QUANTIZE_8BIT = 1, ///< 8-bit quantization
    FL_COMPRESS_QUANTIZE_4BIT = 2, ///< 4-bit quantization
    FL_COMPRESS_SPARSE = 3,        ///< Sparse gradient (top-k)
    FL_COMPRESS_COMBINED = 4       ///< Quantization + sparsification
};

/**
 * @brief Federated learning round information
 */
struct FLRound {
    uint32_t roundId;              ///< Round identifier
    uint32_t coordinatorId;        ///< Coordinator node ID
    uint32_t startTime;            ///< Round start timestamp
    uint32_t deadline;             ///< Training deadline
    FLRoundState state;            ///< Current round state
    uint8_t participantCount;      ///< Number of participants
    uint8_t updatesReceived;       ///< Updates received so far
    uint8_t targetParticipants;    ///< Target participant count
    float minParticipationRatio;   ///< Minimum participation ratio
    String modelType;              ///< Model type identifier
};

/**
 * @brief Federated learning participant info
 */
struct FLParticipant {
    uint32_t nodeId;               ///< Node identifier
    FLNodeRole role;               ///< Node role
    uint8_t batteryLevel;          ///< Battery percentage
    uint32_t availableMemory;      ///< Available memory in bytes
    float reliability;             ///< Historical reliability (0.0-1.0)
    uint32_t lastSeen;             ///< Last seen timestamp
    bool updateSubmitted;          ///< Has submitted update this round
    int16_t signalStrength;        ///< Signal strength to this node
};

/**
 * @brief Model gradient/update chunk for transmission
 */
struct FLModelChunk {
    uint32_t roundId;              ///< Associated round ID
    uint32_t originNodeId;         ///< Origin node ID
    uint16_t chunkIndex;           ///< Chunk index (0-based)
    uint16_t totalChunks;          ///< Total chunks in update
    uint32_t modelVersion;         ///< Model version
    FLCompressionType compression; ///< Compression type used
    uint16_t dataSize;             ///< Size of data in this chunk
    uint8_t data[MESH_FL_MAX_CHUNK_SIZE];  ///< Chunk data
};

/**
 * @brief Aggregated model update statistics
 */
struct FLAggregationStats {
    uint32_t roundId;              ///< Round ID
    uint8_t contributorCount;      ///< Number of contributors
    uint32_t totalSamples;         ///< Total training samples
    float averageLoss;             ///< Average training loss
    float accuracyImprovement;     ///< Accuracy improvement from last round
    uint32_t aggregationTime;      ///< Time spent aggregating (ms)
    uint32_t distributionTime;     ///< Time spent distributing (ms)
};

//==============================================================================
// CALLBACK FUNCTION TYPES
//==============================================================================

typedef void (*CameraCoordCallback)(const CameraStatus& status, bool isNew);
typedef void (*SharedAlertCallback)(const SharedAlert& alert);
typedef void (*RangeChangeCallback)(ExtendedRangeMode mode);
typedef void (*FLRoundCallback)(const FLRound& round);
typedef void (*FLModelUpdateCallback)(uint32_t nodeId, uint32_t roundId, bool success);
typedef void (*FLAggregationCallback)(const FLAggregationStats& stats);

//==============================================================================
// ADVANCED MESH NETWORK CLASS
//==============================================================================

/**
 * @class AdvancedMeshNetwork
 * @brief Advanced mesh networking with multi-camera coordination
 * 
 * Extends basic mesh networking with:
 * - Multi-camera group coordination
 * - Extended range adaptive communication
 * - Shared detection alert broadcasting
 */
class AdvancedMeshNetwork {
public:
    /**
     * @brief Constructor
     * @param meshManager Reference to base MeshManager
     */
    AdvancedMeshNetwork(MeshManager& meshManager);
    
    /**
     * @brief Destructor
     */
    ~AdvancedMeshNetwork();
    
    //==========================================================================
    // INITIALIZATION
    //==========================================================================
    
    /**
     * @brief Initialize advanced mesh networking
     * @return true if initialization successful
     */
    bool init();
    
    /**
     * @brief Check if advanced mesh is initialized
     * @return true if initialized
     */
    bool isInitialized() const;
    
    /**
     * @brief Process advanced mesh networking
     * @note Call this regularly in main loop
     */
    void process();
    
    //==========================================================================
    // MULTI-CAMERA COORDINATION
    //==========================================================================
    
    /**
     * @brief Create a new camera coordination group
     * @param groupId Group identifier
     * @return true if group created
     */
    bool createCameraGroup(uint8_t groupId);
    
    /**
     * @brief Join an existing camera group
     * @param groupId Group to join
     * @return true if joined successfully
     */
    bool joinCameraGroup(uint8_t groupId);
    
    /**
     * @brief Leave current camera group
     */
    void leaveCameraGroup();
    
    /**
     * @brief Get current camera group
     * @return Current group information
     */
    CameraGroup getCameraGroup() const;
    
    /**
     * @brief Set this camera's role
     * @param role Camera role
     */
    void setCameraRole(CameraRole role);
    
    /**
     * @brief Get this camera's role
     * @return Camera role
     */
    CameraRole getCameraRole() const;
    
    /**
     * @brief Get sync state
     * @return Synchronization state
     */
    CameraSyncState getSyncState() const;
    
    /**
     * @brief Request capture synchronization across group
     * @return true if sync request sent
     */
    bool requestSyncCapture();
    
    /**
     * @brief Update camera status for coordination
     * @param status Status to broadcast
     */
    void updateCameraStatus(const CameraStatus& status);
    
    /**
     * @brief Get all cameras in group
     * @return Vector of camera statuses
     */
    std::vector<CameraStatus> getGroupCameras() const;
    
    /**
     * @brief Check if camera is ready for coordinated capture
     * @return true if ready
     */
    bool isReadyForCoordinatedCapture() const;
    
    //==========================================================================
    // EXTENDED RANGE COMMUNICATION
    //==========================================================================
    
    /**
     * @brief Set extended range mode
     * @param mode Range mode to use
     */
    void setRangeMode(ExtendedRangeMode mode);
    
    /**
     * @brief Get current range mode
     * @return Current range mode
     */
    ExtendedRangeMode getRangeMode() const;
    
    /**
     * @brief Get range settings for current mode
     * @return Range settings structure
     */
    RangeSettings getRangeSettings() const;
    
    /**
     * @brief Optimize relay path for destination
     * @param destNodeId Destination node ID
     * @return true if optimization performed
     */
    bool optimizeRelayPath(uint32_t destNodeId);
    
    /**
     * @brief Register node as relay
     * @param nodeId Node to register
     * @return true if registered
     */
    bool registerRelay(uint32_t nodeId);
    
    /**
     * @brief Get available relay nodes
     * @return Vector of relay nodes
     */
    std::vector<RelayNode> getRelayNodes() const;
    
    /**
     * @brief Get best relay for destination
     * @param destNodeId Destination node
     * @return Best relay node ID, 0 if none
     */
    uint32_t getBestRelay(uint32_t destNodeId) const;
    
    /**
     * @brief Check if extended range needed for destination
     * @param destNodeId Destination node ID
     * @return true if extended range recommended
     */
    bool needsExtendedRange(uint32_t destNodeId) const;
    
    //==========================================================================
    // SHARED DETECTION ALERTS
    //==========================================================================
    
    /**
     * @brief Broadcast a shared detection alert
     * @param alert Alert to broadcast
     * @return true if alert broadcast started
     */
    bool broadcastAlert(const SharedAlert& alert);
    
    /**
     * @brief Acknowledge a received alert
     * @param alertId Alert ID to acknowledge
     * @return true if acknowledgment sent
     */
    bool acknowledgeAlert(uint32_t alertId);
    
    /**
     * @brief Get recent alerts
     * @return Vector of recent alerts
     */
    std::vector<SharedAlert> getRecentAlerts() const;
    
    /**
     * @brief Get alert by ID
     * @param alertId Alert ID to find
     * @return Alert structure (empty if not found)
     */
    SharedAlert getAlert(uint32_t alertId) const;
    
    /**
     * @brief Clear all alerts
     */
    void clearAlerts();
    
    /**
     * @brief Get number of pending alerts
     * @return Number of unacknowledged alerts
     */
    uint16_t getPendingAlertCount() const;
    
    /**
     * @brief Check if alert is duplicate
     * @param alertId Alert ID to check
     * @param originNodeId Origin node
     * @return true if duplicate
     */
    bool isDuplicateAlert(uint32_t alertId, uint32_t originNodeId) const;
    
    //==========================================================================
    // FEDERATED LEARNING OVER MESH
    //==========================================================================
    
    /**
     * @brief Start a federated learning round as coordinator
     * @param modelType Model type identifier (e.g., "wildlife_classifier")
     * @param targetParticipants Target number of participants
     * @param deadlineMs Training deadline in milliseconds from now
     * @return Round ID if started, 0 on failure
     */
    uint32_t startFLRound(const String& modelType, uint8_t targetParticipants, 
                          uint32_t deadlineMs = 60000);
    
    /**
     * @brief Join an active federated learning round
     * @param roundId Round ID to join
     * @return true if join request sent
     */
    bool joinFLRound(uint32_t roundId);
    
    /**
     * @brief Leave current federated learning round
     */
    void leaveFLRound();
    
    /**
     * @brief Submit local model update for current round
     * @param gradients Pointer to gradient data
     * @param gradientSize Size of gradient data in bytes
     * @param localSamples Number of local training samples
     * @param localLoss Local training loss
     * @return true if submission started
     */
    bool submitModelUpdate(const uint8_t* gradients, size_t gradientSize,
                          uint32_t localSamples, float localLoss);
    
    /**
     * @brief Get the current federated learning round info
     * @return Current round information
     */
    FLRound getCurrentFLRound() const;
    
    /**
     * @brief Get federated learning participants
     * @return Vector of participants
     */
    std::vector<FLParticipant> getFLParticipants() const;
    
    /**
     * @brief Check if this node is the FL coordinator
     * @return true if coordinator
     */
    bool isFLCoordinator() const;
    
    /**
     * @brief Set federated learning node role
     * @param role Node role
     */
    void setFLRole(FLNodeRole role);
    
    /**
     * @brief Get federated learning node role
     * @return Current FL role
     */
    FLNodeRole getFLRole() const;
    
    /**
     * @brief Get aggregation statistics for last completed round
     * @return Aggregation statistics
     */
    FLAggregationStats getFLAggregationStats() const;
    
    /**
     * @brief Check if federated learning is active
     * @return true if FL round is active
     */
    bool isFLActive() const;
    
    /**
     * @brief Set compression type for model updates
     * @param compression Compression type
     */
    void setFLCompression(FLCompressionType compression);
    
    /**
     * @brief Request global model from coordinator
     * @return true if request sent
     */
    bool requestGlobalModel();
    
    /**
     * @brief Set callbacks for federated learning events
     */
    void setFLRoundCallback(FLRoundCallback callback);
    void setFLModelUpdateCallback(FLModelUpdateCallback callback);
    void setFLAggregationCallback(FLAggregationCallback callback);
    
    //==========================================================================
    // CALLBACKS
    //==========================================================================
    
    /**
     * @brief Set callback for camera coordination events
     * @param callback Function to call on coordination events
     */
    void setCameraCoordCallback(CameraCoordCallback callback);
    
    /**
     * @brief Set callback for shared alerts
     * @param callback Function to call when alert received
     */
    void setSharedAlertCallback(SharedAlertCallback callback);
    
    /**
     * @brief Set callback for range mode changes
     * @param callback Function to call when range mode changes
     */
    void setRangeChangeCallback(RangeChangeCallback callback);
    
    //==========================================================================
    // UTILITY
    //==========================================================================
    
    /**
     * @brief Generate unique alert ID
     * @return Unique alert identifier
     */
    uint32_t generateAlertId();
    
    /**
     * @brief Get network statistics
     * @return JSON-formatted statistics string
     */
    String getNetworkStatistics() const;
    
private:
    // Reference to base mesh manager
    MeshManager& _meshManager;
    
    // Initialization state
    bool _initialized;
    
    // Multi-camera coordination
    CameraGroup _currentGroup;
    CameraRole _cameraRole;
    CameraSyncState _syncState;
    std::vector<CameraStatus> _groupCameras;
    uint32_t _lastSyncTime;
    uint32_t _lastStatusBroadcast;
    bool _syncCaptureRequested;
    
    // Extended range
    ExtendedRangeMode _rangeMode;
    RangeSettings _currentSettings;
    std::vector<RelayNode> _relayNodes;
    uint32_t _lastRangeCheck;
    
    // Shared alerts
    std::vector<SharedAlert> _recentAlerts;
    std::vector<AlertDedup> _alertDedup;
    uint32_t _alertIdCounter;
    uint32_t _lastAlertCleanup;
    
    // Federated learning state
    FLRound _currentFLRound;
    FLNodeRole _flRole;
    FLCompressionType _flCompression;
    std::vector<FLParticipant> _flParticipants;
    std::vector<FLModelChunk> _pendingChunks;
    FLAggregationStats _lastAggregationStats;
    uint32_t _flRoundIdCounter;
    uint32_t _lastFLProcess;
    bool _flUpdatePending;
    
    // Callbacks
    CameraCoordCallback _coordCallback;
    SharedAlertCallback _alertCallback;
    RangeChangeCallback _rangeCallback;
    FLRoundCallback _flRoundCallback;
    FLModelUpdateCallback _flModelUpdateCallback;
    FLAggregationCallback _flAggregationCallback;
    
    // Internal methods
    void processCoordination();
    void processExtendedRange();
    void processAlerts();
    void processFederatedLearning();
    void sendCoordinationSync();
    void handleCoordinationMessage(uint32_t fromNode, const uint8_t* data, size_t length);
    void handleAlertMessage(uint32_t fromNode, const uint8_t* data, size_t length);
    void handleFLMessage(uint32_t fromNode, const uint8_t* data, size_t length);
    void updateRangeSettings();
    void cleanupOldAlerts();
    void addAlertDedup(uint32_t alertId, uint32_t originNode);
    bool applyRangeSettings(const RangeSettings& settings);
    RangeSettings getSettingsForMode(ExtendedRangeMode mode) const;
    
    // Federated learning internal methods
    void broadcastFLRoundAnnouncement();
    void sendFLJoinRequest(uint32_t coordinatorId);
    void sendFLJoinResponse(uint32_t nodeId, bool accepted);
    bool sendModelChunk(const FLModelChunk& chunk);
    void processReceivedChunks();
    void aggregateModelUpdates();
    void distributeAggregatedModel();
    uint8_t* compressGradients(const uint8_t* data, size_t size, size_t* compressedSize);
    uint8_t* decompressGradients(const uint8_t* data, size_t size, size_t* decompressedSize);
    uint32_t generateFLRoundId();
};

// Global advanced mesh instance
extern AdvancedMeshNetwork* advancedMesh;

#endif // ADVANCED_MESH_NETWORK_H
