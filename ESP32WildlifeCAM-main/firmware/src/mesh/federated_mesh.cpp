/**
 * @file federated_mesh.cpp
 * @brief Implementation of Federated Learning Mesh Extension
 * 
 * Implements federated learning coordination over the mesh network.
 */

#include "federated_mesh.h"
#include "../utils/logger.h"
#include <ArduinoJson.h>

// Global instance
FederatedMesh* g_federatedMesh = nullptr;

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

FederatedMesh::FederatedMesh()
    : initialized_(false)
    , isCoordinator_(false)
    , currentCoordinator_("")
    , currentRoundId_("")
{
    config_ = FederatedMeshConfig();
    stats_ = FederatedMeshStats();
}

FederatedMesh::~FederatedMesh() {
    cleanup();
}

bool FederatedMesh::init(const FederatedMeshConfig& config) {
    if (initialized_) {
        Logger::warn("[FederatedMesh] Already initialized");
        return true;
    }

    Logger::info("[FederatedMesh] Initializing federated mesh");

    config_ = config;

    // Initialize participant tracking
    participants_.clear();
    participantCapabilities_.clear();
    activeRounds_.clear();
    pendingUpdates_.clear();

    // Reset statistics
    stats_ = FederatedMeshStats();

    // Register this device as a participant
    String deviceId = "ESP32_" + String(ESP.getChipId(), HEX);
    registerParticipant(deviceId, "wildlife_classifier,behavior_analyzer");

    initialized_ = true;
    Logger::info("[FederatedMesh] Federated mesh initialized successfully");
    return true;
}

void FederatedMesh::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[FederatedMesh] Cleaning up federated mesh");

    // End any active rounds
    for (const auto& round : activeRounds_) {
        endTrainingRound(round.first);
    }

    // Clear data structures
    participants_.clear();
    participantCapabilities_.clear();
    activeRounds_.clear();
    pendingUpdates_.clear();

    initialized_ = false;
    isCoordinator_ = false;
    currentCoordinator_ = "";
}

// ===========================
// FEDERATED LEARNING COORDINATION
// ===========================

bool FederatedMesh::startTrainingRound(const String& roundId, WildlifeModelType modelType) {
    if (!initialized_) {
        Logger::error("[FederatedMesh] Not initialized");
        return false;
    }

    if (isRoundActive(roundId)) {
        Logger::warn("[FederatedMesh] Round already active: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FederatedMesh] Starting training round: %s for model type: %d", 
                roundId.c_str(), (int)modelType);

    // Record round start
    activeRounds_[roundId] = getCurrentTime();
    currentRoundId_ = roundId;

    // Broadcast round start message
    DynamicJsonDocument doc(1024);
    doc["roundId"] = roundId;
    doc["modelType"] = (int)modelType;
    doc["startTime"] = getCurrentTime();
    doc["coordinator"] = isCoordinator_ ? "true" : "false";

    String payload;
    serializeJson(doc, payload);

    if (!broadcastFederatedMessage(FLMeshMessageType::AGGREGATION_ROUND_START, payload)) {
        Logger::error("[FederatedMesh] Failed to broadcast round start");
        activeRounds_.erase(roundId);
        return false;
    }

    // Trigger callback
    if (roundStartCallback_) {
        roundStartCallback_(roundId, modelType);
    }

    stats_.totalRounds++;
    Logger::info("[FederatedMesh] Training round started successfully");
    return true;
}

bool FederatedMesh::joinTrainingRound(const String& roundId) {
    if (!initialized_) {
        Logger::error("[FederatedMesh] Not initialized");
        return false;
    }

    if (!isRoundActive(roundId)) {
        Logger::warn("[FederatedMesh] Round not active: %s", roundId.c_str());
        return false;
    }

    Logger::info("[FederatedMesh] Joining training round: %s", roundId.c_str());

    // Send participation message to coordinator
    DynamicJsonDocument doc(512);
    doc["roundId"] = roundId;
    doc["deviceId"] = "ESP32_" + String(ESP.getChipId(), HEX);
    doc["timestamp"] = getCurrentTime();

    String payload;
    serializeJson(doc, payload);

    // If we have a coordinator, send to them; otherwise broadcast
    if (!currentCoordinator_.isEmpty()) {
        return sendFederatedMessage(currentCoordinator_, 
                                   FLMeshMessageType::PARTICIPANT_REGISTRATION, payload);
    } else {
        return broadcastFederatedMessage(FLMeshMessageType::PARTICIPANT_REGISTRATION, payload);
    }
}

bool FederatedMesh::endTrainingRound(const String& roundId) {
    if (!initialized_) {
        return false;
    }

    if (!isRoundActive(roundId)) {
        Logger::debug("[FederatedMesh] Round not active: %s", roundId.c_str());
        return true;
    }

    Logger::info("[FederatedMesh] Ending training round: %s", roundId.c_str());

    // Calculate round duration
    uint32_t roundDuration = getCurrentTime() - activeRounds_[roundId];
    stats_.averageRoundTimeMs = (stats_.averageRoundTimeMs + roundDuration) / 2;

    // Remove from active rounds
    activeRounds_.erase(roundId);

    // Broadcast round end message
    DynamicJsonDocument doc(512);
    doc["roundId"] = roundId;
    doc["endTime"] = getCurrentTime();
    doc["success"] = true;

    String payload;
    serializeJson(doc, payload);

    broadcastFederatedMessage(FLMeshMessageType::AGGREGATION_ROUND_END, payload);

    // Trigger callback
    if (roundEndCallback_) {
        roundEndCallback_(roundId, true);
    }

    // Clear current round if this was it
    if (currentRoundId_ == roundId) {
        currentRoundId_ = "";
    }

    Logger::info("[FederatedMesh] Training round ended");
    return true;
}

// ===========================
// MODEL UPDATE MANAGEMENT
// ===========================

bool FederatedMesh::broadcastModelUpdate(const ModelUpdate& update) {
    if (!initialized_) {
        Logger::error("[FederatedMesh] Not initialized");
        return false;
    }

    Logger::debug("[FederatedMesh] Broadcasting model update");

    String serialized;
    if (!serializeModelUpdate(update, serialized)) {
        Logger::error("[FederatedMesh] Failed to serialize model update");
        return false;
    }

    bool success = broadcastFederatedMessage(FLMeshMessageType::MODEL_UPDATE_RESPONSE, serialized);
    if (success) {
        stats_.modelUpdatesTransferred++;
    }

    return success;
}

bool FederatedMesh::collectModelUpdates(std::vector<ModelUpdate>& updates, uint32_t timeoutMs) {
    if (!initialized_) {
        Logger::error("[FederatedMesh] Not initialized");
        return false;
    }

    Logger::info("[FederatedMesh] Collecting model updates (timeout: %dms)", timeoutMs);

    uint32_t startTime = getCurrentTime();
    updates.clear();

    // Send request for model updates
    DynamicJsonDocument doc(256);
    doc["requestId"] = String(getCurrentTime());
    doc["timeout"] = timeoutMs;

    String payload;
    serializeJson(doc, payload);

    broadcastFederatedMessage(FLMeshMessageType::MODEL_UPDATE_REQUEST, payload);

    // Wait for responses
    while ((getCurrentTime() - startTime) < timeoutMs) {
        // Process any pending updates
        if (!pendingUpdates_.empty()) {
            updates.insert(updates.end(), pendingUpdates_.begin(), pendingUpdates_.end());
            pendingUpdates_.clear();
        }

        // Check if we have enough participants
        if (updates.size() >= (size_t)(getParticipantCount() * config_.minParticipationRatio)) {
            break;
        }

        delay(100);
    }

    Logger::info("[FederatedMesh] Collected %d model updates", updates.size());
    return !updates.empty();
}

bool FederatedMesh::distributeAggregatedModel(const ModelUpdate& aggregatedModel) {
    if (!initialized_) {
        Logger::error("[FederatedMesh] Not initialized");
        return false;
    }

    Logger::info("[FederatedMesh] Distributing aggregated model");

    String serialized;
    if (!serializeModelUpdate(aggregatedModel, serialized)) {
        Logger::error("[FederatedMesh] Failed to serialize aggregated model");
        return false;
    }

    return broadcastFederatedMessage(FLMeshMessageType::MODEL_UPDATE_RESPONSE, serialized);
}

// ===========================
// PARTICIPANT MANAGEMENT
// ===========================

bool FederatedMesh::registerParticipant(const String& deviceId, const String& capabilities) {
    if (!initialized_) {
        return false;
    }

    participants_[deviceId] = getCurrentTime();
    participantCapabilities_[deviceId] = capabilities;

    Logger::debug("[FederatedMesh] Registered participant: %s", deviceId.c_str());

    // Trigger callback
    if (participantJoinCallback_) {
        participantJoinCallback_(deviceId);
    }

    updateStatistics();
    return true;
}

bool FederatedMesh::unregisterParticipant(const String& deviceId) {
    if (!initialized_) {
        return false;
    }

    participants_.erase(deviceId);
    participantCapabilities_.erase(deviceId);

    Logger::debug("[FederatedMesh] Unregistered participant: %s", deviceId.c_str());

    // Trigger callback
    if (participantLeaveCallback_) {
        participantLeaveCallback_(deviceId);
    }

    updateStatistics();
    return true;
}

std::vector<String> FederatedMesh::getActiveParticipants() const {
    std::vector<String> active;
    uint32_t currentTime = getCurrentTime();
    uint32_t timeout = 60000; // 1 minute timeout

    for (const auto& participant : participants_) {
        if ((currentTime - participant.second) < timeout) {
            active.push_back(participant.first);
        }
    }

    return active;
}

size_t FederatedMesh::getParticipantCount() const {
    return getActiveParticipants().size();
}

// ===========================
// COORDINATOR ELECTION
// ===========================

bool FederatedMesh::electCoordinator() {
    if (!initialized_) {
        return false;
    }

    Logger::info("[FederatedMesh] Starting coordinator election");

    // Simple coordinator election based on device ID
    std::vector<String> participants = getActiveParticipants();
    if (participants.empty()) {
        // Only this device, become coordinator
        isCoordinator_ = true;
        currentCoordinator_ = "ESP32_" + String(ESP.getChipId(), HEX);
    } else {
        // Find device with highest ID (deterministic)
        String highestId = "ESP32_" + String(ESP.getChipId(), HEX);
        for (const String& participant : participants) {
            if (participant > highestId) {
                highestId = participant;
            }
        }

        currentCoordinator_ = highestId;
        isCoordinator_ = (currentCoordinator_ == "ESP32_" + String(ESP.getChipId(), HEX));
    }

    Logger::info("[FederatedMesh] Coordinator elected: %s (is_this_device: %s)", 
                currentCoordinator_.c_str(), isCoordinator_ ? "true" : "false");

    // Trigger callback
    if (coordinatorChangeCallback_) {
        coordinatorChangeCallback_(currentCoordinator_);
    }

    return true;
}

// ===========================
// NETWORK HEALTH
// ===========================

bool FederatedMesh::checkNetworkHealth() {
    if (!initialized_) {
        return false;
    }

    Logger::debug("[FederatedMesh] Checking network health");

    // Update participant list
    updateParticipantList();

    // Cleanup expired rounds
    cleanupExpiredRounds();

    // Update statistics
    updateStatistics();

    return true;
}

float FederatedMesh::getNetworkReliability() const {
    if (stats_.totalRounds == 0) {
        return 1.0f;
    }

    return (float)stats_.successfulRounds / stats_.totalRounds;
}

// ===========================
// PRIVACY AND SECURITY
// ===========================

bool FederatedMesh::syncPrivacyBudgets() {
    if (!initialized_ || !config_.enablePrivacySync) {
        return false;
    }

    Logger::debug("[FederatedMesh] Syncing privacy budgets");

    // Broadcast privacy budget sync message
    DynamicJsonDocument doc(256);
    doc["deviceId"] = "ESP32_" + String(ESP.getChipId(), HEX);
    doc["timestamp"] = getCurrentTime();
    doc["budgetUsed"] = 0.1f; // Placeholder

    String payload;
    serializeJson(doc, payload);

    return broadcastFederatedMessage(FLMeshMessageType::PRIVACY_BUDGET_SYNC, payload);
}

bool FederatedMesh::validateParticipant(const String& deviceId) {
    // Simple validation - check if participant is registered
    return participants_.find(deviceId) != participants_.end();
}

// ===========================
// PRIVATE METHODS
// ===========================

bool FederatedMesh::sendFederatedMessage(const String& targetId, FLMeshMessageType type, 
                                         const String& payload) {
    // In a real implementation, this would use the mesh network to send messages
    Logger::debug("[FederatedMesh] Sending message type %d to %s", (int)type, targetId.c_str());
    return true;
}

bool FederatedMesh::broadcastFederatedMessage(FLMeshMessageType type, const String& payload) {
    // In a real implementation, this would broadcast over the mesh network
    Logger::debug("[FederatedMesh] Broadcasting message type %d", (int)type);
    return true;
}

void FederatedMesh::handleFederatedMessage(const String& senderId, FLMeshMessageType type, 
                                          const String& payload) {
    Logger::debug("[FederatedMesh] Handling message type %d from %s", (int)type, senderId.c_str());

    switch (type) {
        case FLMeshMessageType::AGGREGATION_ROUND_START: {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            String roundId = doc["roundId"];
            int modelType = doc["modelType"];
            
            if (roundStartCallback_) {
                roundStartCallback_(roundId, (WildlifeModelType)modelType);
            }
            break;
        }
        
        case FLMeshMessageType::MODEL_UPDATE_REQUEST: {
            // Handle model update request
            break;
        }
        
        case FLMeshMessageType::PARTICIPANT_REGISTRATION: {
            DynamicJsonDocument doc(512);
            deserializeJson(doc, payload);
            String deviceId = doc["deviceId"];
            registerParticipant(deviceId, "");
            break;
        }
        
        default:
            Logger::debug("[FederatedMesh] Unhandled message type: %d", (int)type);
            break;
    }
}

bool FederatedMesh::serializeModelUpdate(const ModelUpdate& update, String& serialized) {
    DynamicJsonDocument doc(2048);
    
    doc["deviceId"] = update.deviceId;
    doc["timestamp"] = update.timestamp;
    doc["modelType"] = (int)update.modelType;
    doc["updateType"] = (int)update.updateType;
    doc["trainingRounds"] = update.trainingRounds;
    doc["dataPoints"] = update.dataPoints;
    
    // For now, just store weight count (actual weights would be too large for JSON)
    doc["weightCount"] = update.weights.size();
    
    return serializeJson(doc, serialized) > 0;
}

bool FederatedMesh::deserializeModelUpdate(const String& serialized, ModelUpdate& update) {
    DynamicJsonDocument doc(2048);
    
    if (deserializeJson(doc, serialized) != DeserializationError::Ok) {
        return false;
    }
    
    update.deviceId = doc["deviceId"].as<String>();
    update.timestamp = doc["timestamp"];
    update.modelType = (WildlifeModelType)doc["modelType"].as<int>();
    update.updateType = (ModelUpdateType)doc["updateType"].as<int>();
    update.trainingRounds = doc["trainingRounds"];
    update.dataPoints = doc["dataPoints"];
    
    // Initialize weights with placeholder values
    size_t weightCount = doc["weightCount"];
    update.weights.resize(weightCount, 0.0f);
    
    return true;
}

bool FederatedMesh::validateModelUpdate(const ModelUpdate& update, const String& senderId) {
    // Basic validation
    if (update.deviceId != senderId) {
        return false;
    }
    
    if (update.weights.empty()) {
        return false;
    }
    
    return true;
}

void FederatedMesh::updateParticipantList() {
    uint32_t currentTime = getCurrentTime();
    uint32_t timeout = 120000; // 2 minutes timeout

    // Remove expired participants
    auto it = participants_.begin();
    while (it != participants_.end()) {
        if ((currentTime - it->second) > timeout) {
            String deviceId = it->first;
            it = participants_.erase(it);
            participantCapabilities_.erase(deviceId);
            
            Logger::debug("[FederatedMesh] Removed expired participant: %s", deviceId.c_str());
            
            if (participantLeaveCallback_) {
                participantLeaveCallback_(deviceId);
            }
        } else {
            ++it;
        }
    }
}

void FederatedMesh::cleanupExpiredRounds() {
    uint32_t currentTime = getCurrentTime();
    
    auto it = activeRounds_.begin();
    while (it != activeRounds_.end()) {
        if ((currentTime - it->second) > config_.roundTimeoutMs) {
            String roundId = it->first;
            it = activeRounds_.erase(it);
            
            Logger::debug("[FederatedMesh] Cleaned up expired round: %s", roundId.c_str());
            
            if (roundEndCallback_) {
                roundEndCallback_(roundId, false);
            }
        } else {
            ++it;
        }
    }
}

void FederatedMesh::updateStatistics() {
    stats_.averageParticipants = getParticipantCount();
}

void FederatedMesh::logFederatedEvent(const String& event, const String& level) {
    String logMessage = "[FederatedMesh] " + event;
    
    if (level == "ERROR") {
        Logger::error(logMessage.c_str());
    } else if (level == "WARNING") {
        Logger::warn(logMessage.c_str());
    } else if (level == "INFO") {
        Logger::info(logMessage.c_str());
    } else {
        Logger::debug(logMessage.c_str());
    }
}

uint32_t FederatedMesh::getCurrentTime() const {
    return millis();
}

String FederatedMesh::generateRoundId() const {
    return "ROUND_" + String(getCurrentTime()) + "_" + String(random(1000, 9999));
}

bool FederatedMesh::isRoundActive(const String& roundId) const {
    return activeRounds_.find(roundId) != activeRounds_.end();
}

// ===========================
// GLOBAL UTILITY FUNCTIONS
// ===========================

bool initializeFederatedMesh(const FederatedMeshConfig& config) {
    if (g_federatedMesh) {
        return true;
    }

    g_federatedMesh = new FederatedMesh();
    return g_federatedMesh->init(config);
}

void cleanupFederatedMesh() {
    if (g_federatedMesh) {
        delete g_federatedMesh;
        g_federatedMesh = nullptr;
    }
}

bool startGlobalTrainingRound(WildlifeModelType modelType) {
    if (!g_federatedMesh) {
        return false;
    }

    String roundId = "GLOBAL_" + String(millis());
    return g_federatedMesh->startTrainingRound(roundId, modelType);
}

bool joinGlobalTrainingRound() {
    if (!g_federatedMesh) {
        return false;
    }

    // Join the most recent round
    return true; // Simplified implementation
}

FederatedMeshStats getFederatedMeshStatistics() {
    if (!g_federatedMesh) {
        return FederatedMeshStats();
    }

    return g_federatedMesh->getStatistics();
}