/**
 * @file federated_learning_coordinator.h
 * @brief Federated Learning Coordinator for managing federated rounds
 * 
 * Implements the coordination logic for federated learning rounds,
 * participant management, and global model aggregation.
 */

#ifndef FEDERATED_LEARNING_COORDINATOR_H
#define FEDERATED_LEARNING_COORDINATOR_H

#include "federated_common.h"
#include "local_training_module.h"
#include "privacy_preserving_aggregation.h"
#include "model_update_protocol.h"
#include "../tinyml/inference_engine.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Coordinator Role
 */
enum class CoordinatorRole {
    CLIENT,                     // Regular participating device
    COORDINATOR,               // Central coordinator
    BACKUP_COORDINATOR,        // Backup coordinator
    MESH_NODE                  // Mesh network node
};

/**
 * Round Status
 */
enum class RoundStatus {
    IDLE,                      // No active round
    INITIALIZING,              // Setting up new round
    RECRUITING,                // Recruiting participants
    TRAINING,                  // Training phase active
    AGGREGATING,               // Aggregating updates
    VALIDATING,                // Validating aggregated model
    DEPLOYING,                 // Deploying new model
    COMPLETED,                 // Round completed successfully
    FAILED,                    // Round failed
    CANCELLED                  // Round cancelled
};

/**
 * Round Configuration
 */
struct RoundConfig {
    String modelId;
    WildlifeModelType modelType;
    uint32_t minParticipants;
    uint32_t maxParticipants;
    uint32_t roundTimeoutMs;
    uint32_t trainingTimeoutMs;
    float minAccuracyImprovement;
    bool requireValidation;
    AggregationMethod aggregationMethod;
    PrivacyLevel privacyLevel;
    
    RoundConfig() : modelType(MODEL_SPECIES_CLASSIFIER), minParticipants(3),
                   maxParticipants(50), roundTimeoutMs(3600000), // 1 hour
                   trainingTimeoutMs(600000), // 10 minutes
                   minAccuracyImprovement(0.01f), requireValidation(true),
                   aggregationMethod(AggregationMethod::FEDERATED_AVERAGING),
                   privacyLevel(PrivacyLevel::STANDARD) {}
};

/**
 * Participant Information
 */
struct ParticipantInfo {
    String deviceId;
    String ipAddress;
    CoordinatorRole role;
    FederatedLearningState state;
    uint32_t lastSeen;
    float reliability;         // 0.0 to 1.0
    uint32_t roundsParticipated;
    float averageAccuracy;
    bool eligible;
    String capabilities;       // JSON string of device capabilities
    
    ParticipantInfo() : role(CoordinatorRole::CLIENT), 
                       state(FederatedLearningState::FL_IDLE),
                       lastSeen(0), reliability(1.0f), roundsParticipated(0),
                       averageAccuracy(0.0f), eligible(true) {}
};

/**
 * Round Statistics
 */
struct RoundStatistics {
    String roundId;
    uint32_t participantCount;
    uint32_t successfulParticipants;
    uint32_t failedParticipants;
    float aggregatedAccuracy;
    float accuracyImprovement;
    uint32_t totalTrainingTime;
    uint32_t aggregationTime;
    float privacyBudgetUsed;
    RoundStatus finalStatus;
    String errorMessage;
    
    RoundStatistics() : participantCount(0), successfulParticipants(0),
                       failedParticipants(0), aggregatedAccuracy(0.0f),
                       accuracyImprovement(0.0f), totalTrainingTime(0),
                       aggregationTime(0), privacyBudgetUsed(0.0f),
                       finalStatus(RoundStatus::IDLE) {}
};

/**
 * Federated Learning Coordinator
 * 
 * Manages federated learning rounds and coordinates between participants:
 * - Round initialization and management
 * - Participant recruitment and management
 * - Model aggregation coordination
 * - Validation and deployment
 * - Network topology management
 */
class FederatedLearningCoordinator {
public:
    FederatedLearningCoordinator();
    ~FederatedLearningCoordinator();
    
    // Initialization
    bool init(const FederatedLearningConfig& config, CoordinatorRole role = CoordinatorRole::CLIENT);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Role management
    CoordinatorRole getRole() const { return role_; }
    bool setRole(CoordinatorRole role);
    bool promoteToCoordinator();
    bool demoteToClient();
    
    // Round management (Coordinator functions)
    String initiateRound(const RoundConfig& config);
    bool cancelRound(const String& roundId);
    bool completeRound(const String& roundId);
    RoundStatus getRoundStatus(const String& roundId) const;
    RoundStatistics getRoundStatistics(const String& roundId) const;
    
    // Participant management (Coordinator functions)
    bool recruitParticipants(const String& roundId, uint32_t targetCount);
    bool addParticipant(const String& roundId, const ParticipantInfo& participant);
    bool removeParticipant(const String& roundId, const String& deviceId);
    std::vector<ParticipantInfo> getParticipants(const String& roundId) const;
    
    // Client participation
    bool joinRound(const String& roundId);
    bool leaveRound(const String& roundId);
    bool submitModelUpdate(const String& roundId, const ModelUpdate& update);
    bool downloadGlobalModel(const String& roundId, ModelUpdate& globalModel);
    
    // Model management
    bool aggregateModelUpdates(const String& roundId);
    bool validateAggregatedModel(const String& roundId);
    bool deployGlobalModel(const String& roundId);
    ModelUpdate getCurrentGlobalModel(const String& modelId) const;
    
    // Network topology management
    bool discoverPeers();
    bool establishConnections();
    bool selectCoordinator();
    std::vector<String> getConnectedPeers() const;
    
    // Monitoring and status
    bool isRoundActive() const;
    String getCurrentRoundId() const { return currentRoundId_; }
    FederatedLearningState getCurrentState() const { return currentState_; }
    std::vector<String> getActiveRounds() const;
    
    // Configuration
    void setRoundConfig(const RoundConfig& config);
    RoundConfig getRoundConfig() const { return roundConfig_; }
    void setPrivacyConfig(const AggregationConfig& config);
    
    // Event callbacks
    typedef std::function<void(const String&, RoundStatus)> RoundStatusCallback;
    typedef std::function<void(const String&, const ParticipantInfo&)> ParticipantCallback;
    typedef std::function<void(const String&, const ModelUpdate&)> ModelUpdateCallback;
    typedef std::function<void(const String&)> ErrorCallback;
    
    void setRoundStatusCallback(RoundStatusCallback callback) { roundStatusCallback_ = callback; }
    void setParticipantJoinCallback(ParticipantCallback callback) { participantJoinCallback_ = callback; }
    void setParticipantLeaveCallback(ParticipantCallback callback) { participantLeaveCallback_ = callback; }
    void setModelUpdateCallback(ModelUpdateCallback callback) { modelUpdateCallback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    
    // Utility methods
    bool isEligibleForRound(const RoundConfig& config) const;
    float estimateRoundDuration(const RoundConfig& config) const;
    uint32_t estimateMemoryRequirement(const RoundConfig& config) const;
    bool checkNetworkConnectivity() const;
    
    // Metrics and statistics
    struct CoordinatorMetrics {
        uint32_t totalRoundsCoordinated;
        uint32_t totalRoundsParticipated;
        uint32_t successfulRounds;
        uint32_t failedRounds;
        float averageAccuracyImprovement;
        uint32_t totalParticipants;
        float averageRoundDuration;
        uint32_t networkErrors;
        
        CoordinatorMetrics() : totalRoundsCoordinated(0), totalRoundsParticipated(0),
                              successfulRounds(0), failedRounds(0),
                              averageAccuracyImprovement(0.0f), totalParticipants(0),
                              averageRoundDuration(0.0f), networkErrors(0) {}
    };
    
    CoordinatorMetrics getMetrics() const { return metrics_; }
    void resetMetrics();

private:
    // Configuration
    FederatedLearningConfig config_;
    RoundConfig roundConfig_;
    CoordinatorRole role_;
    bool initialized_;
    
    // Current state
    FederatedLearningState currentState_;
    String currentRoundId_;
    String deviceId_;
    
    // Round management
    std::map<String, FederatedRound> activeRounds_;
    std::map<String, std::vector<ParticipantInfo>> roundParticipants_;
    std::map<String, std::vector<ModelUpdate>> roundUpdates_;
    std::map<String, RoundStatistics> roundStatistics_;
    
    // Network management
    std::vector<String> connectedPeers_;
    String coordinatorAddress_;
    uint32_t lastDiscovery_;
    
    // Component instances
    std::unique_ptr<LocalTrainingModule> localTraining_;
    std::unique_ptr<PrivacyPreservingAggregation> privacyAggregation_;
    std::unique_ptr<ModelUpdateProtocol> updateProtocol_;
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    
    // Metrics
    CoordinatorMetrics metrics_;
    
    // Callbacks
    RoundStatusCallback roundStatusCallback_;
    ParticipantCallback participantJoinCallback_;
    ParticipantCallback participantLeaveCallback_;
    ModelUpdateCallback modelUpdateCallback_;
    ErrorCallback errorCallback_;
    
    // Private methods - Round management
    bool createRound(const String& roundId, const RoundConfig& config);
    bool startRound(const String& roundId);
    bool processRoundTimeout(const String& roundId);
    void updateRoundStatus(const String& roundId, RoundStatus status);
    
    // Private methods - Participant management
    bool validateParticipant(const ParticipantInfo& participant) const;
    void updateParticipantState(const String& roundId, const String& deviceId, 
                               FederatedLearningState state);
    float calculateParticipantReliability(const ParticipantInfo& participant) const;
    
    // Private methods - Model aggregation
    bool collectModelUpdates(const String& roundId);
    ModelUpdate performAggregation(const String& roundId);
    bool validateGlobalModel(const ModelUpdate& globalModel);
    void distributeGlobalModel(const String& roundId, const ModelUpdate& globalModel);
    
    // Private methods - Network management
    bool announceCoordinatorRole();
    bool handleCoordinatorElection();
    void pingParticipants();
    void handleNetworkPartition();
    
    // Private methods - Utilities
    String generateRoundId() const;
    String getDeviceId() const;
    uint32_t getCurrentTimestamp() const;
    void logEvent(const String& event, const String& details = "");
    void notifyError(const String& error);
};

// Global instance
extern FederatedLearningCoordinator* g_federatedCoordinator;

// Utility functions
bool initializeFederatedCoordinator(const FederatedLearningConfig& config, 
                                   CoordinatorRole role = CoordinatorRole::CLIENT);
void cleanupFederatedCoordinator();
bool participateInFederatedLearning();
String getCurrentFederatedRound();
bool isFederatedLearningActive();

#endif // FEDERATED_LEARNING_COORDINATOR_H