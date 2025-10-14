/**
 * @file federated_learning_coordinator.cpp
 * @brief Implementation of Federated Learning Coordinator
 * 
 * Implements coordination logic for federated learning rounds,
 * participant management, and global model aggregation.
 */

#include "federated_learning_coordinator.h"
#include "../../utils/logger.h"
#include <cmath>
#include <algorithm>
#include <random>

// Global instance
FederatedLearningCoordinator* g_federatedCoordinator = nullptr;

// ===========================
// CONSTRUCTOR / DESTRUCTOR
// ===========================

FederatedLearningCoordinator::FederatedLearningCoordinator()
    : initialized_(false)
    , role_(CoordinatorRole::CLIENT)
    , currentState_(FederatedLearningState::FL_IDLE)
    , lastDiscovery_(0)
{
    deviceId_ = getDeviceId();
}

FederatedLearningCoordinator::~FederatedLearningCoordinator() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool FederatedLearningCoordinator::init(const FederatedLearningConfig& config, CoordinatorRole role) {
    if (initialized_) {
        Logger::warn("[FLCoordinator] Already initialized");
        return true;
    }

    Logger::info("[FLCoordinator] Initializing coordinator...");

    config_ = config;
    role_ = role;

    // Initialize component instances
    localTraining_ = std::make_unique<LocalTrainingModule>();
    if (!localTraining_->init(config)) {
        Logger::error("[FLCoordinator] Failed to initialize local training");
        return false;
    }

    privacyAggregation_ = std::make_unique<PrivacyPreservingAggregation>();
    AggregationConfig aggConfig;
    aggConfig.privacyLevel = config.privacyLevel;
    aggConfig.privacyEpsilon = config.privacyEpsilon;
    if (!privacyAggregation_->init(aggConfig)) {
        Logger::error("[FLCoordinator] Failed to initialize privacy aggregation");
        return false;
    }

    updateProtocol_ = std::make_unique<ModelUpdateProtocol>();
    NetworkConfig netConfig;
    netConfig.enableEncryption = true;
    if (!updateProtocol_->init(netConfig)) {
        Logger::error("[FLCoordinator] Failed to initialize update protocol");
        return false;
    }

    // Setup message handlers
    updateProtocol_->setRoundAnnouncementHandler(
        [this](const String& senderId, const FederatedRound& round, const RoundConfig& config) {
            this->joinRound(round.roundId);
        }
    );

    updateProtocol_->setModelUpdateHandler(
        [this](const String& senderId, const ModelUpdate& update) {
            if (role_ == CoordinatorRole::COORDINATOR) {
                this->submitModelUpdate(currentRoundId_, update);
            }
        }
    );

    initialized_ = true;
    currentState_ = FederatedLearningState::FL_IDLE;

    Logger::info("[FLCoordinator] Initialization complete, role: %d", static_cast<int>(role_));
    return true;
}

void FederatedLearningCoordinator::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[FLCoordinator] Cleaning up coordinator...");

    // Cancel any active rounds
    for (auto& pair : activeRounds_) {
        cancelRound(pair.first);
    }

    // Cleanup components
    if (localTraining_) {
        localTraining_->cleanup();
        localTraining_.reset();
    }

    if (privacyAggregation_) {
        privacyAggregation_->cleanup();
        privacyAggregation_.reset();
    }

    if (updateProtocol_) {
        updateProtocol_->cleanup();
        updateProtocol_.reset();
    }

    initialized_ = false;
    currentState_ = FederatedLearningState::FL_DISABLED;

    Logger::info("[FLCoordinator] Cleanup complete");
}

// ===========================
// ROLE MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::setRole(CoordinatorRole role) {
    if (role_ == role) {
        return true;
    }

    Logger::info("[FLCoordinator] Changing role from %d to %d", 
                static_cast<int>(role_), static_cast<int>(role));

    role_ = role;

    if (role == CoordinatorRole::COORDINATOR) {
        return announceCoordinatorRole();
    }

    return true;
}

bool FederatedLearningCoordinator::promoteToCoordinator() {
    return setRole(CoordinatorRole::COORDINATOR);
}

bool FederatedLearningCoordinator::demoteToClient() {
    return setRole(CoordinatorRole::CLIENT);
}

// ===========================
// ROUND MANAGEMENT (COORDINATOR)
// ===========================

String FederatedLearningCoordinator::initiateRound(const RoundConfig& config) {
    if (role_ != CoordinatorRole::COORDINATOR) {
        Logger::error("[FLCoordinator] Only coordinator can initiate rounds");
        return "";
    }

    String roundId = generateRoundId();
    Logger::info("[FLCoordinator] Initiating round: %s", roundId.c_str());

    if (!createRound(roundId, config)) {
        Logger::error("[FLCoordinator] Failed to create round");
        return "";
    }

    // Announce round to all participants
    FederatedRound round;
    round.roundId = roundId;
    round.modelType = config.modelType;
    round.startTime = getCurrentTimestamp();
    round.state = FederatedLearningState::FL_WAITING_FOR_ROUND;

    updateProtocol_->broadcastRoundAnnouncement(round, config);

    // Start participant recruitment
    if (!recruitParticipants(roundId, config.minParticipants)) {
        Logger::warn("[FLCoordinator] Failed to recruit minimum participants");
    }

    updateRoundStatus(roundId, RoundStatus::RECRUITING);
    metrics_.totalRoundsCoordinated++;

    return roundId;
}

bool FederatedLearningCoordinator::cancelRound(const String& roundId) {
    auto it = activeRounds_.find(roundId);
    if (it == activeRounds_.end()) {
        Logger::warn("[FLCoordinator] Round not found: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FLCoordinator] Cancelling round: %s", roundId.c_str());

    updateRoundStatus(roundId, RoundStatus::CANCELLED);
    
    // Notify participants
    updateProtocol_->broadcastRoundComplete(roundId);

    // Clean up round data
    activeRounds_.erase(it);
    roundParticipants_.erase(roundId);
    roundUpdates_.erase(roundId);

    return true;
}

bool FederatedLearningCoordinator::completeRound(const String& roundId) {
    auto it = activeRounds_.find(roundId);
    if (it == activeRounds_.end()) {
        Logger::warn("[FLCoordinator] Round not found: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FLCoordinator] Completing round: %s", roundId.c_str());

    updateRoundStatus(roundId, RoundStatus::COMPLETED);

    // Update statistics
    auto& stats = roundStatistics_[roundId];
    stats.finalStatus = RoundStatus::COMPLETED;

    metrics_.successfulRounds++;
    metrics_.averageAccuracyImprovement = 
        (metrics_.averageAccuracyImprovement * (metrics_.successfulRounds - 1) + stats.accuracyImprovement) 
        / metrics_.successfulRounds;

    // Notify participants
    updateProtocol_->broadcastRoundComplete(roundId);

    // Clean up
    activeRounds_.erase(it);

    return true;
}

RoundStatus FederatedLearningCoordinator::getRoundStatus(const String& roundId) const {
    auto it = roundStatistics_.find(roundId);
    if (it != roundStatistics_.end()) {
        return it->second.finalStatus;
    }
    return RoundStatus::IDLE;
}

RoundStatistics FederatedLearningCoordinator::getRoundStatistics(const String& roundId) const {
    auto it = roundStatistics_.find(roundId);
    if (it != roundStatistics_.end()) {
        return it->second;
    }
    return RoundStatistics();
}

// ===========================
// PARTICIPANT MANAGEMENT (COORDINATOR)
// ===========================

bool FederatedLearningCoordinator::recruitParticipants(const String& roundId, uint32_t targetCount) {
    Logger::info("[FLCoordinator] Recruiting %u participants for round %s", 
                targetCount, roundId.c_str());

    // In a real implementation, this would use peer discovery
    // For now, we'll wait for participants to join voluntarily
    
    return true;
}

bool FederatedLearningCoordinator::addParticipant(const String& roundId, const ParticipantInfo& participant) {
    if (!validateParticipant(participant)) {
        Logger::warn("[FLCoordinator] Invalid participant: %s", participant.deviceId.c_str());
        return false;
    }

    auto& participants = roundParticipants_[roundId];
    
    // Check if already registered
    for (const auto& p : participants) {
        if (p.deviceId == participant.deviceId) {
            Logger::warn("[FLCoordinator] Participant already registered: %s", participant.deviceId.c_str());
            return false;
        }
    }

    participants.push_back(participant);
    
    Logger::info("[FLCoordinator] Added participant %s to round %s", 
                participant.deviceId.c_str(), roundId.c_str());

    // Update statistics
    auto& stats = roundStatistics_[roundId];
    stats.participantCount++;
    metrics_.totalParticipants++;

    // Notify callback
    if (participantJoinCallback_) {
        participantJoinCallback_(roundId, participant);
    }

    return true;
}

bool FederatedLearningCoordinator::removeParticipant(const String& roundId, const String& deviceId) {
    auto& participants = roundParticipants_[roundId];
    
    auto it = std::remove_if(participants.begin(), participants.end(),
        [&deviceId](const ParticipantInfo& p) { return p.deviceId == deviceId; });
    
    if (it == participants.end()) {
        return false;
    }

    participants.erase(it, participants.end());
    
    Logger::info("[FLCoordinator] Removed participant %s from round %s", 
                deviceId.c_str(), roundId.c_str());

    // Update statistics
    auto& stats = roundStatistics_[roundId];
    if (stats.participantCount > 0) {
        stats.participantCount--;
    }

    return true;
}

std::vector<ParticipantInfo> FederatedLearningCoordinator::getParticipants(const String& roundId) const {
    auto it = roundParticipants_.find(roundId);
    if (it != roundParticipants_.end()) {
        return it->second;
    }
    return std::vector<ParticipantInfo>();
}

// ===========================
// CLIENT PARTICIPATION
// ===========================

bool FederatedLearningCoordinator::joinRound(const String& roundId) {
    if (role_ == CoordinatorRole::COORDINATOR) {
        Logger::warn("[FLCoordinator] Coordinator cannot join as client");
        return false;
    }

    Logger::info("[FLCoordinator] Joining round: %s", roundId.c_str());

    currentRoundId_ = roundId;
    currentState_ = FederatedLearningState::FL_WAITING_FOR_ROUND;

    // Send join request
    ParticipantInfo myInfo;
    myInfo.deviceId = deviceId_;
    myInfo.role = role_;
    myInfo.state = currentState_;
    myInfo.reliability = 1.0f;

    JoinRequestPayload payload;
    payload.deviceId = deviceId_;
    payload.capabilities = "{}"; // Would include device capabilities

    if (!updateProtocol_->sendJoinRequest(coordinatorAddress_, payload)) {
        Logger::error("[FLCoordinator] Failed to send join request");
        return false;
    }

    metrics_.totalRoundsParticipated++;

    return true;
}

bool FederatedLearningCoordinator::leaveRound(const String& roundId) {
    if (currentRoundId_ != roundId) {
        Logger::warn("[FLCoordinator] Not participating in round: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FLCoordinator] Leaving round: %s", roundId.c_str());

    currentRoundId_ = "";
    currentState_ = FederatedLearningState::FL_IDLE;

    return true;
}

bool FederatedLearningCoordinator::submitModelUpdate(const String& roundId, const ModelUpdate& update) {
    if (currentRoundId_ != roundId && role_ != CoordinatorRole::COORDINATOR) {
        Logger::warn("[FLCoordinator] Not participating in round: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FLCoordinator] Submitting model update for round: %s", roundId.c_str());

    if (role_ == CoordinatorRole::COORDINATOR) {
        // Store update for aggregation
        auto& updates = roundUpdates_[roundId];
        updates.push_back(update);
        
        auto& stats = roundStatistics_[roundId];
        stats.successfulParticipants++;
        
        // Check if we have enough updates to aggregate
        auto it = activeRounds_.find(roundId);
        if (it != activeRounds_.end()) {
            uint32_t minParticipants = roundConfig_.minParticipants;
            if (updates.size() >= minParticipants) {
                Logger::info("[FLCoordinator] Enough updates received, starting aggregation");
                aggregateModelUpdates(roundId);
            }
        }
    } else {
        // Send to coordinator
        if (!updateProtocol_->sendModelUpdate(coordinatorAddress_, update)) {
            Logger::error("[FLCoordinator] Failed to send model update");
            return false;
        }
    }

    return true;
}

bool FederatedLearningCoordinator::downloadGlobalModel(const String& roundId, ModelUpdate& globalModel) {
    Logger::info("[FLCoordinator] Downloading global model for round: %s", roundId.c_str());

    // In a real implementation, this would request from coordinator
    // For now, return success if we're the coordinator
    if (role_ == CoordinatorRole::COORDINATOR) {
        globalModel = getCurrentGlobalModel(roundId);
        return true;
    }

    currentState_ = FederatedLearningState::FL_DOWNLOADING_MODEL;

    // Would wait for model from coordinator
    return true;
}

// ===========================
// MODEL MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::aggregateModelUpdates(const String& roundId) {
    if (role_ != CoordinatorRole::COORDINATOR) {
        Logger::error("[FLCoordinator] Only coordinator can aggregate");
        return false;
    }

    Logger::info("[FLCoordinator] Aggregating model updates for round: %s", roundId.c_str());

    auto it = roundUpdates_.find(roundId);
    if (it == roundUpdates_.end() || it->second.empty()) {
        Logger::error("[FLCoordinator] No updates to aggregate");
        return false;
    }

    updateRoundStatus(roundId, RoundStatus::AGGREGATING);

    uint32_t startTime = getCurrentTimestamp();

    // Perform aggregation using privacy-preserving methods
    AggregationResult result = privacyAggregation_->aggregateUpdates(it->second);

    uint32_t aggregationTime = getCurrentTimestamp() - startTime;

    if (!result.success) {
        Logger::error("[FLCoordinator] Aggregation failed: %s", result.errorMessage.c_str());
        updateRoundStatus(roundId, RoundStatus::FAILED);
        return false;
    }

    // Update statistics
    auto& stats = roundStatistics_[roundId];
    stats.aggregationTime = aggregationTime;
    stats.aggregatedAccuracy = result.aggregatedModel.accuracy;
    stats.privacyBudgetUsed = result.privacyBudgetUsed;

    Logger::info("[FLCoordinator] Aggregation complete, accuracy: %.3f", result.aggregatedModel.accuracy);

    // Validate and deploy
    if (validateAggregatedModel(roundId)) {
        deployGlobalModel(roundId);
    }

    return true;
}

bool FederatedLearningCoordinator::validateAggregatedModel(const String& roundId) {
    Logger::info("[FLCoordinator] Validating aggregated model for round: %s", roundId.c_str());

    updateRoundStatus(roundId, RoundStatus::VALIDATING);

    // Basic validation
    auto& updates = roundUpdates_[roundId];
    if (updates.empty()) {
        return false;
    }

    // Would perform actual validation here
    // For now, assume valid if we have updates
    return true;
}

bool FederatedLearningCoordinator::deployGlobalModel(const String& roundId) {
    Logger::info("[FLCoordinator] Deploying global model for round: %s", roundId.c_str());

    updateRoundStatus(roundId, RoundStatus::DEPLOYING);

    auto it = roundUpdates_.find(roundId);
    if (it == roundUpdates_.end() || it->second.empty()) {
        return false;
    }

    // Get aggregated model
    AggregationResult result = privacyAggregation_->aggregateUpdates(it->second);
    if (!result.success) {
        return false;
    }

    // Distribute to all participants
    distributeGlobalModel(roundId, result.aggregatedModel);

    // Mark round as complete
    completeRound(roundId);

    return true;
}

ModelUpdate FederatedLearningCoordinator::getCurrentGlobalModel(const String& modelId) const {
    // Return empty model for now
    ModelUpdate model;
    model.modelId = modelId;
    model.version = "1.0.0";
    return model;
}

// ===========================
// NETWORK TOPOLOGY MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::discoverPeers() {
    Logger::info("[FLCoordinator] Discovering peers...");

    lastDiscovery_ = getCurrentTimestamp();

    // Broadcast peer discovery message
    if (updateProtocol_) {
        updateProtocol_->broadcastPeerDiscovery();
    }

    return true;
}

bool FederatedLearningCoordinator::establishConnections() {
    Logger::info("[FLCoordinator] Establishing connections...");

    // Would establish connections to discovered peers
    return true;
}

bool FederatedLearningCoordinator::selectCoordinator() {
    Logger::info("[FLCoordinator] Selecting coordinator...");

    // Simple election: promote self if no coordinator found
    if (coordinatorAddress_.isEmpty() && connectedPeers_.size() > 0) {
        return promoteToCoordinator();
    }

    return true;
}

std::vector<String> FederatedLearningCoordinator::getConnectedPeers() const {
    return connectedPeers_;
}

// ===========================
// MONITORING AND STATUS
// ===========================

bool FederatedLearningCoordinator::isRoundActive() const {
    return !activeRounds_.empty();
}

std::vector<String> FederatedLearningCoordinator::getActiveRounds() const {
    std::vector<String> rounds;
    for (const auto& pair : activeRounds_) {
        rounds.push_back(pair.first);
    }
    return rounds;
}

// ===========================
// CONFIGURATION
// ===========================

void FederatedLearningCoordinator::setRoundConfig(const RoundConfig& config) {
    roundConfig_ = config;
}

void FederatedLearningCoordinator::setPrivacyConfig(const AggregationConfig& config) {
    if (privacyAggregation_) {
        privacyAggregation_->setAggregationConfig(config);
    }
}

// ===========================
// UTILITY METHODS
// ===========================

bool FederatedLearningCoordinator::isEligibleForRound(const RoundConfig& config) const {
    // Check if device has sufficient resources
    return initialized_ && currentState_ != FederatedLearningState::FL_ERROR;
}

float FederatedLearningCoordinator::estimateRoundDuration(const RoundConfig& config) const {
    // Simple estimation
    return config.trainingTimeoutMs + 60000; // Training + overhead
}

uint32_t FederatedLearningCoordinator::estimateMemoryRequirement(const RoundConfig& config) const {
    // Estimate based on model type
    return 512 * 1024; // 512KB baseline
}

bool FederatedLearningCoordinator::checkNetworkConnectivity() const {
    return updateProtocol_ && updateProtocol_->isConnected();
}

void FederatedLearningCoordinator::resetMetrics() {
    metrics_ = CoordinatorMetrics();
}

// ===========================
// PRIVATE METHODS - ROUND MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::createRound(const String& roundId, const RoundConfig& config) {
    FederatedRound round;
    round.roundId = roundId;
    round.modelType = config.modelType;
    round.startTime = getCurrentTimestamp();
    round.state = FederatedLearningState::FL_WAITING_FOR_ROUND;

    activeRounds_[roundId] = round;
    roundStatistics_[roundId] = RoundStatistics();
    roundStatistics_[roundId].roundId = roundId;

    return true;
}

bool FederatedLearningCoordinator::startRound(const String& roundId) {
    auto it = activeRounds_.find(roundId);
    if (it == activeRounds_.end()) {
        return false;
    }

    it->second.state = FederatedLearningState::FL_TRAINING;
    updateRoundStatus(roundId, RoundStatus::TRAINING);

    return true;
}

bool FederatedLearningCoordinator::processRoundTimeout(const String& roundId) {
    Logger::warn("[FLCoordinator] Round timeout: %s", roundId.c_str());
    
    auto& stats = roundStatistics_[roundId];
    stats.finalStatus = RoundStatus::FAILED;
    stats.errorMessage = "Round timeout";
    
    metrics_.failedRounds++;
    
    return cancelRound(roundId);
}

void FederatedLearningCoordinator::updateRoundStatus(const String& roundId, RoundStatus status) {
    auto& stats = roundStatistics_[roundId];
    stats.finalStatus = status;

    if (roundStatusCallback_) {
        roundStatusCallback_(roundId, status);
    }

    logEvent("Round status change", roundId + " -> " + String(static_cast<int>(status)));
}

// ===========================
// PRIVATE METHODS - PARTICIPANT MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::validateParticipant(const ParticipantInfo& participant) const {
    // Basic validation
    if (participant.deviceId.isEmpty()) {
        return false;
    }

    if (participant.reliability < 0.0f || participant.reliability > 1.0f) {
        return false;
    }

    return true;
}

void FederatedLearningCoordinator::updateParticipantState(const String& roundId, 
                                                          const String& deviceId,
                                                          FederatedLearningState state) {
    auto& participants = roundParticipants_[roundId];
    
    for (auto& p : participants) {
        if (p.deviceId == deviceId) {
            p.state = state;
            p.lastSeen = getCurrentTimestamp();
            break;
        }
    }
}

float FederatedLearningCoordinator::calculateParticipantReliability(const ParticipantInfo& participant) const {
    // Simple reliability calculation
    if (participant.roundsParticipated == 0) {
        return 1.0f;
    }

    return participant.reliability;
}

// ===========================
// PRIVATE METHODS - MODEL AGGREGATION
// ===========================

bool FederatedLearningCoordinator::collectModelUpdates(const String& roundId) {
    // Would wait for all participants to submit updates
    return true;
}

ModelUpdate FederatedLearningCoordinator::performAggregation(const String& roundId) {
    auto it = roundUpdates_.find(roundId);
    if (it == roundUpdates_.end()) {
        return ModelUpdate();
    }

    AggregationResult result = privacyAggregation_->aggregateUpdates(it->second);
    return result.aggregatedModel;
}

bool FederatedLearningCoordinator::validateGlobalModel(const ModelUpdate& globalModel) {
    // Would validate model quality
    return !globalModel.weights.empty();
}

void FederatedLearningCoordinator::distributeGlobalModel(const String& roundId, const ModelUpdate& globalModel) {
    Logger::info("[FLCoordinator] Distributing global model for round: %s", roundId.c_str());

    auto participants = getParticipants(roundId);
    for (const auto& p : participants) {
        updateProtocol_->sendGlobalModel(p.deviceId, globalModel, 0.05f);
    }
}

// ===========================
// PRIVATE METHODS - NETWORK MANAGEMENT
// ===========================

bool FederatedLearningCoordinator::announceCoordinatorRole() {
    Logger::info("[FLCoordinator] Announcing coordinator role");

    // Would broadcast coordinator announcement
    return true;
}

bool FederatedLearningCoordinator::handleCoordinatorElection() {
    Logger::info("[FLCoordinator] Handling coordinator election");

    // Simple election algorithm
    return promoteToCoordinator();
}

void FederatedLearningCoordinator::pingParticipants() {
    auto participants = getParticipants(currentRoundId_);
    for (const auto& p : participants) {
        updateProtocol_->sendHeartbeat(p.deviceId);
    }
}

void FederatedLearningCoordinator::handleNetworkPartition() {
    Logger::warn("[FLCoordinator] Network partition detected");
    
    // Would handle network partition recovery
}

// ===========================
// PRIVATE METHODS - UTILITIES
// ===========================

String FederatedLearningCoordinator::generateRoundId() const {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "round_%u_%u", 
             getCurrentTimestamp(), random(10000, 99999));
    return String(buffer);
}

String FederatedLearningCoordinator::getDeviceId() const {
    // Would return actual device ID
    return "device_" + String(ESP.getEfuseMac(), HEX);
}

uint32_t FederatedLearningCoordinator::getCurrentTimestamp() const {
    return millis();
}

void FederatedLearningCoordinator::logEvent(const String& event, const String& details) {
    Logger::debug("[FLCoordinator] %s: %s", event.c_str(), details.c_str());
}

void FederatedLearningCoordinator::notifyError(const String& error) {
    Logger::error("[FLCoordinator] Error: %s", error.c_str());
    
    if (errorCallback_) {
        errorCallback_(error);
    }
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

bool initializeFederatedCoordinator(const FederatedLearningConfig& config, CoordinatorRole role) {
    if (g_federatedCoordinator) {
        Logger::warn("[FLCoordinator] Already initialized");
        return true;
    }

    g_federatedCoordinator = new FederatedLearningCoordinator();
    return g_federatedCoordinator->init(config, role);
}

void cleanupFederatedCoordinator() {
    if (g_federatedCoordinator) {
        delete g_federatedCoordinator;
        g_federatedCoordinator = nullptr;
    }
}

bool participateInFederatedLearning() {
    if (!g_federatedCoordinator || !g_federatedCoordinator->isInitialized()) {
        return false;
    }

    return g_federatedCoordinator->discoverPeers();
}

String getCurrentFederatedRound() {
    if (!g_federatedCoordinator) {
        return "";
    }

    return g_federatedCoordinator->getCurrentRoundId();
}

bool isFederatedLearningActive() {
    if (!g_federatedCoordinator) {
        return false;
    }

    return g_federatedCoordinator->isRoundActive();
}
