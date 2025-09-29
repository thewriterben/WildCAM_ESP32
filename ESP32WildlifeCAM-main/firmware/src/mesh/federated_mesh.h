/**
 * @file federated_mesh.h
 * @brief Federated Learning Extension for Mesh Network
 * 
 * Extends the existing mesh network to support federated learning coordination,
 * model distribution, and secure aggregation across wildlife camera devices.
 */

#ifndef FEDERATED_MESH_H
#define FEDERATED_MESH_H

#include "../network/mesh/mesh_coordinator.h"
#include "../ai/federated_learning/federated_common.h"
#include <vector>
#include <map>

/**
 * Federated Mesh Message Types
 */
enum class FLMeshMessageType {
    MODEL_UPDATE_REQUEST,
    MODEL_UPDATE_RESPONSE,
    AGGREGATION_ROUND_START,
    AGGREGATION_ROUND_END,
    PRIVACY_BUDGET_SYNC,
    TRAINING_INVITATION,
    PARTICIPANT_REGISTRATION,
    COORDINATOR_ELECTION,
    MODEL_VALIDATION,
    EMERGENCY_STOP
};

/**
 * Federated Learning Mesh Configuration
 */
struct FederatedMeshConfig {
    bool enableFederatedLearning;
    uint32_t maxParticipants;
    uint32_t roundTimeoutMs;
    uint32_t aggregationTimeoutMs;
    bool enableAutoCoordination;
    bool enablePrivacySync;
    float minParticipationRatio;
    
    FederatedMeshConfig() : enableFederatedLearning(true), maxParticipants(20),
                           roundTimeoutMs(300000), aggregationTimeoutMs(120000),
                           enableAutoCoordination(true), enablePrivacySync(true),
                           minParticipationRatio(0.6f) {}
};

/**
 * Federated Mesh Statistics
 */
struct FederatedMeshStats {
    uint32_t totalRounds;
    uint32_t successfulRounds;
    uint32_t averageParticipants;
    uint32_t averageRoundTimeMs;
    uint32_t modelUpdatesTransferred;
    uint32_t networkPartitions;
    float averagePacketLoss;
    
    FederatedMeshStats() : totalRounds(0), successfulRounds(0),
                          averageParticipants(0), averageRoundTimeMs(0),
                          modelUpdatesTransferred(0), networkPartitions(0),
                          averagePacketLoss(0.0f) {}
};

/**
 * Federated Learning Mesh Extension
 * 
 * Extends the mesh network coordinator to support federated learning:
 * - Distributed training round coordination
 * - Model update broadcasting and collection
 * - Participant discovery and registration
 * - Privacy-preserving communication
 * - Fault-tolerant coordination
 */
class FederatedMesh {
public:
    FederatedMesh();
    ~FederatedMesh();
    
    // Initialization
    bool init(const FederatedMeshConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setConfig(const FederatedMeshConfig& config) { config_ = config; }
    FederatedMeshConfig getConfig() const { return config_; }
    
    // Federated learning coordination
    bool startTrainingRound(const String& roundId, WildlifeModelType modelType);
    bool joinTrainingRound(const String& roundId);
    bool endTrainingRound(const String& roundId);
    
    // Model update management
    bool broadcastModelUpdate(const ModelUpdate& update);
    bool collectModelUpdates(std::vector<ModelUpdate>& updates, uint32_t timeoutMs);
    bool distributeAggregatedModel(const ModelUpdate& aggregatedModel);
    
    // Participant management
    bool registerParticipant(const String& deviceId, const String& capabilities);
    bool unregisterParticipant(const String& deviceId);
    std::vector<String> getActiveParticipants() const;
    size_t getParticipantCount() const;
    
    // Coordinator election
    bool electCoordinator();
    bool isCoordinator() const { return isCoordinator_; }
    String getCurrentCoordinator() const { return currentCoordinator_; }
    
    // Network health and monitoring
    bool checkNetworkHealth();
    FederatedMeshStats getStatistics() const;
    float getNetworkReliability() const;
    
    // Privacy and security
    bool syncPrivacyBudgets();
    bool validateParticipant(const String& deviceId);
    bool encryptMessage(const String& message, String& encrypted);
    bool decryptMessage(const String& encrypted, String& message);
    
    // Callbacks
    typedef std::function<void(const String&, WildlifeModelType)> RoundStartCallback;
    typedef std::function<void(const String&, bool)> RoundEndCallback;
    typedef std::function<void(const ModelUpdate&)> ModelUpdateCallback;
    typedef std::function<void(const String&)> ParticipantJoinCallback;
    typedef std::function<void(const String&)> ParticipantLeaveCallback;
    typedef std::function<void(const String&)> CoordinatorChangeCallback;
    
    void setRoundStartCallback(RoundStartCallback callback) { roundStartCallback_ = callback; }
    void setRoundEndCallback(RoundEndCallback callback) { roundEndCallback_ = callback; }
    void setModelUpdateCallback(ModelUpdateCallback callback) { modelUpdateCallback_ = callback; }
    void setParticipantJoinCallback(ParticipantJoinCallback callback) { participantJoinCallback_ = callback; }
    void setParticipantLeaveCallback(ParticipantLeaveCallback callback) { participantLeaveCallback_ = callback; }
    void setCoordinatorChangeCallback(CoordinatorChangeCallback callback) { coordinatorChangeCallback_ = callback; }

private:
    // Configuration and state
    FederatedMeshConfig config_;
    bool initialized_;
    bool isCoordinator_;
    String currentCoordinator_;
    String currentRoundId_;
    
    // Participant tracking
    std::map<String, uint32_t> participants_;  // deviceId -> lastSeen
    std::map<String, String> participantCapabilities_;
    
    // Training round management
    std::map<String, uint32_t> activeRounds_;  // roundId -> startTime
    std::vector<ModelUpdate> pendingUpdates_;
    
    // Statistics
    FederatedMeshStats stats_;
    
    // Callbacks
    RoundStartCallback roundStartCallback_;
    RoundEndCallback roundEndCallback_;
    ModelUpdateCallback modelUpdateCallback_;
    ParticipantJoinCallback participantJoinCallback_;
    ParticipantLeaveCallback participantLeaveCallback_;
    CoordinatorChangeCallback coordinatorChangeCallback_;
    
    // Private methods - Message handling
    bool sendFederatedMessage(const String& targetId, FLMeshMessageType type, 
                             const String& payload);
    bool broadcastFederatedMessage(FLMeshMessageType type, const String& payload);
    void handleFederatedMessage(const String& senderId, FLMeshMessageType type, 
                               const String& payload);
    
    // Private methods - Coordination
    bool initiateCoordinatorElection();
    bool handleCoordinatorElection(const String& candidateId);
    void updateParticipantList();
    void cleanupExpiredRounds();
    
    // Private methods - Model update handling
    bool serializeModelUpdate(const ModelUpdate& update, String& serialized);
    bool deserializeModelUpdate(const String& serialized, ModelUpdate& update);
    bool validateModelUpdate(const ModelUpdate& update, const String& senderId);
    
    // Private methods - Utilities
    void updateStatistics();
    void logFederatedEvent(const String& event, const String& level = "INFO");
    uint32_t getCurrentTime() const;
    String generateRoundId() const;
    bool isRoundActive(const String& roundId) const;
};

// Global utility functions
bool initializeFederatedMesh(const FederatedMeshConfig& config);
void cleanupFederatedMesh();
bool startGlobalTrainingRound(WildlifeModelType modelType);
bool joinGlobalTrainingRound();
FederatedMeshStats getFederatedMeshStatistics();

#endif // FEDERATED_MESH_H