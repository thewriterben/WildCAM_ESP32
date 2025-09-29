/**
 * @file federated_common.cpp
 * @brief Implementation of common utility functions for federated learning
 */

#include "federated_common.h"

/**
 * Convert FederatedLearningState to string
 */
const char* federatedLearningStateToString(FederatedLearningState state) {
    switch (state) {
        case FederatedLearningState::FL_IDLE: return "IDLE";
        case FederatedLearningState::FL_WAITING_FOR_ROUND: return "WAITING_FOR_ROUND";
        case FederatedLearningState::FL_DOWNLOADING_MODEL: return "DOWNLOADING_MODEL";
        case FederatedLearningState::FL_TRAINING: return "TRAINING";
        case FederatedLearningState::FL_UPLOADING_UPDATES: return "UPLOADING_UPDATES";
        case FederatedLearningState::FL_AGGREGATING: return "AGGREGATING";
        case FederatedLearningState::FL_VALIDATING: return "VALIDATING";
        case FederatedLearningState::FL_ERROR: return "ERROR";
        case FederatedLearningState::FL_DISABLED: return "DISABLED";
        default: return "UNKNOWN";
    }
}

/**
 * Convert PrivacyLevel to string
 */
const char* privacyLevelToString(PrivacyLevel level) {
    switch (level) {
        case PrivacyLevel::NONE: return "NONE";
        case PrivacyLevel::BASIC: return "BASIC";
        case PrivacyLevel::STANDARD: return "STANDARD";
        case PrivacyLevel::HIGH: return "HIGH";
        case PrivacyLevel::MAXIMUM: return "MAXIMUM";
        default: return "UNKNOWN";
    }
}

/**
 * Convert NetworkTopology to string
 */
const char* networkTopologyToString(NetworkTopology topology) {
    switch (topology) {
        case NetworkTopology::STAR: return "STAR";
        case NetworkTopology::MESH: return "MESH";
        case NetworkTopology::HYBRID: return "HYBRID";
        case NetworkTopology::HIERARCHICAL: return "HIERARCHICAL";
        default: return "UNKNOWN";
    }
}

/**
 * Convert ModelUpdateType to string
 */
const char* modelUpdateTypeToString(ModelUpdateType type) {
    switch (type) {
        case ModelUpdateType::FULL_MODEL: return "FULL_MODEL";
        case ModelUpdateType::GRADIENT_UPDATE: return "GRADIENT_UPDATE";
        case ModelUpdateType::COMPRESSED_UPDATE: return "COMPRESSED_UPDATE";
        case ModelUpdateType::DIFFERENTIAL_UPDATE: return "DIFFERENTIAL_UPDATE";
        default: return "UNKNOWN";
    }
}