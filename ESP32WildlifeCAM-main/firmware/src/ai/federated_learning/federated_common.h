/**
 * @file federated_common.h
 * @brief Common definitions and data structures for federated learning
 * 
 * Defines common data structures, enums, and constants used across
 * the federated learning modules for collaborative AI model improvement.
 */

#ifndef FEDERATED_COMMON_H
#define FEDERATED_COMMON_H

#include "../ai_common.h"
#include <Arduino.h>
#include <vector>
#include <map>

/**
 * Federated Learning Protocol Version
 */
#define FL_PROTOCOL_VERSION "1.0.0"

/**
 * Federated Learning States
 */
enum class FederatedLearningState {
    FL_IDLE,                    // Not participating in any round
    FL_WAITING_FOR_ROUND,       // Waiting for round invitation
    FL_DOWNLOADING_MODEL,       // Downloading global model
    FL_TRAINING,                // Performing local training
    FL_UPLOADING_UPDATES,       // Uploading model updates
    FL_AGGREGATING,             // Coordinator aggregating updates
    FL_VALIDATING,              // Validating new model
    FL_ERROR,                   // Error state
    FL_DISABLED                 // Federated learning disabled
};

/**
 * Privacy Protection Levels
 */
enum class PrivacyLevel {
    NONE = 0,                   // No privacy protection
    BASIC = 1,                  // Basic anonymization
    STANDARD = 2,               // Standard differential privacy
    HIGH = 3,                   // High privacy protection
    MAXIMUM = 4                 // Maximum privacy protection
};

/**
 * Network Topology Types
 */
enum class NetworkTopology {
    STAR,                       // Central coordinator
    MESH,                       // Peer-to-peer mesh
    HYBRID,                     // Mixed topology
    HIERARCHICAL               // Tree-like structure
};

/**
 * Model Update Type
 */
enum class ModelUpdateType {
    FULL_MODEL,                 // Complete model weights
    GRADIENT_UPDATE,            // Gradient updates only
    COMPRESSED_UPDATE,          // Compressed updates
    DIFFERENTIAL_UPDATE         // Differential updates
};

/**
 * Federated Learning Configuration
 */
struct FederatedLearningConfig {
    // Participation settings
    bool enableFederatedLearning;
    float contributionThreshold;        // Minimum confidence to contribute
    uint32_t minSamplesBeforeContribution;
    uint32_t maxContributionsPerDay;
    
    // Privacy settings
    PrivacyLevel privacyLevel;
    float privacyEpsilon;              // Differential privacy parameter
    bool anonymizeLocation;
    bool anonymizeTimestamps;
    
    // Training settings
    float learningRate;
    uint32_t localEpochs;
    uint32_t batchSize;
    uint32_t maxTrainingTimeMs;
    
    // Network settings
    NetworkTopology topology;
    String federationNetwork;
    std::vector<String> trustedPartners;
    uint32_t communicationTimeoutMs;
    
    // Update settings
    ModelUpdateType updateType;
    uint32_t modelUpdateIntervalDays;
    bool requireManualApproval;
    float minAccuracyImprovement;
    
    FederatedLearningConfig() :
        enableFederatedLearning(false),
        contributionThreshold(0.95f),
        minSamplesBeforeContribution(100),
        maxContributionsPerDay(10),
        privacyLevel(PrivacyLevel::STANDARD),
        privacyEpsilon(1.0f),
        anonymizeLocation(true),
        anonymizeTimestamps(true),
        learningRate(0.001f),
        localEpochs(5),
        batchSize(8),
        maxTrainingTimeMs(300000), // 5 minutes
        topology(NetworkTopology::STAR),
        federationNetwork("wildlife_global"),
        communicationTimeoutMs(60000),
        updateType(ModelUpdateType::GRADIENT_UPDATE),
        modelUpdateIntervalDays(7),
        requireManualApproval(false),
        minAccuracyImprovement(0.02f) {}
};

/**
 * Training Sample for Federated Learning
 */
struct TrainingSample {
    CameraFrame frame;
    AIResult label;
    float confidence;
    uint32_t timestamp;
    bool validated;             // Expert or automated validation
    String deviceId;            // Source device (anonymized)
    
    TrainingSample() : confidence(0.0f), timestamp(0), validated(false) {}
};

/**
 * Model Update Package
 */
struct ModelUpdate {
    String modelId;
    String roundId;
    ModelUpdateType type;
    std::vector<float> weights;
    std::vector<float> gradients;
    size_t sampleCount;
    float accuracy;
    uint32_t timestamp;
    String checksum;
    
    // Privacy metadata
    bool privacyPreserved;
    float noiseLevel;
    
    ModelUpdate() : type(ModelUpdateType::GRADIENT_UPDATE), sampleCount(0), 
                   accuracy(0.0f), timestamp(0), privacyPreserved(false), 
                   noiseLevel(0.0f) {}
};

/**
 * Federated Round Information
 */
struct FederatedRound {
    String roundId;
    String modelId;
    uint32_t minParticipants;
    uint32_t maxParticipants;
    uint32_t currentParticipants;
    uint32_t roundTimeoutMs;
    uint32_t startTimestamp;
    uint32_t endTimestamp;
    bool active;
    
    FederatedRound() : minParticipants(3), maxParticipants(100), 
                      currentParticipants(0), roundTimeoutMs(3600000), // 1 hour
                      startTimestamp(0), endTimestamp(0), active(false) {}
};

/**
 * Device Participation Status
 */
struct ParticipationStatus {
    String deviceId;
    FederatedLearningState state;
    String currentRoundId;
    uint32_t samplesContributed;
    uint32_t roundsParticipated;
    float lastAccuracyImprovement;
    uint32_t lastParticipationTime;
    bool eligible;
    
    ParticipationStatus() : state(FederatedLearningState::FL_IDLE),
                           samplesContributed(0), roundsParticipated(0),
                           lastAccuracyImprovement(0.0f), lastParticipationTime(0),
                           eligible(true) {}
};

/**
 * Privacy Protection Result
 */
struct PrivacyProtectionResult {
    bool dataAnonymized;
    bool locationFuzzed;
    bool timestampNoisified;
    bool differentialPrivacyApplied;
    float privacyBudget;
    float actualEpsilon;
    
    PrivacyProtectionResult() : dataAnonymized(false), locationFuzzed(false),
                               timestampNoisified(false), differentialPrivacyApplied(false),
                               privacyBudget(1.0f), actualEpsilon(0.0f) {}
};

/**
 * Federated Learning Metrics
 */
struct FederatedLearningMetrics {
    uint32_t totalRoundsParticipated;
    uint32_t successfulRounds;
    uint32_t failedRounds;
    float averageAccuracyImprovement;
    uint32_t totalSamplesContributed;
    uint32_t totalTrainingTimeMs;
    float averagePrivacyBudget;
    uint32_t networkErrors;
    uint32_t powerOptimizationEvents;
    
    FederatedLearningMetrics() : totalRoundsParticipated(0), successfulRounds(0),
                                failedRounds(0), averageAccuracyImprovement(0.0f),
                                totalSamplesContributed(0), totalTrainingTimeMs(0),
                                averagePrivacyBudget(0.0f), networkErrors(0),
                                powerOptimizationEvents(0) {}
};

// Utility functions
const char* federatedLearningStateToString(FederatedLearningState state);
const char* privacyLevelToString(PrivacyLevel level);
const char* networkTopologyToString(NetworkTopology topology);
const char* modelUpdateTypeToString(ModelUpdateType type);

// Constants
static const uint32_t FL_DEFAULT_TIMEOUT_MS = 60000;
static const uint32_t FL_MAX_TRAINING_TIME_MS = 300000;  // 5 minutes
static const uint32_t FL_MIN_SAMPLES_FOR_TRAINING = 10;
static const float FL_DEFAULT_PRIVACY_EPSILON = 1.0f;
static const float FL_MIN_ACCURACY_IMPROVEMENT = 0.01f;

#endif // FEDERATED_COMMON_H