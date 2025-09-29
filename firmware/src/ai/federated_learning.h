/**
 * @file federated_learning.h
 * @brief Federated learning integration for ESP32 wildlife classification
 * 
 * Enables on-device learning and model updates through federated learning
 * while preserving privacy and reducing bandwidth requirements.
 */

#ifndef FEDERATED_LEARNING_H
#define FEDERATED_LEARNING_H

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

// Federated learning configuration
#define MAX_LOCAL_SAMPLES 100
#define MAX_GRADIENT_SIZE 1024
#define MAX_MODEL_UPDATES 50
#define CONTRIBUTION_BUFFER_SIZE 2048

/**
 * Privacy levels for federated learning
 */
enum PrivacyLevel {
    PRIVACY_NONE = 0,       // No privacy protection
    PRIVACY_LOW = 1,        // Basic anonymization
    PRIVACY_MEDIUM = 2,     // Differential privacy with moderate noise
    PRIVACY_HIGH = 3        // Strong differential privacy
};

/**
 * Federated learning round status
 */
enum FLRoundStatus {
    FL_IDLE = 0,
    FL_COLLECTING = 1,      // Collecting local data
    FL_TRAINING = 2,        // Local training in progress
    FL_UPLOADING = 3,       // Uploading updates to server
    FL_DOWNLOADING = 4,     // Downloading global model
    FL_COMPLETE = 5,        // Round completed successfully
    FL_ERROR = 6            // Error in federated learning
};

/**
 * Local training sample
 */
typedef struct {
    uint8_t* imageData;         // Image data
    uint16_t width;             // Image width
    uint16_t height;            // Image height
    uint8_t channels;           // Number of channels
    uint8_t trueLabel;          // Ground truth label
    float confidence;           // Confidence in ground truth
    uint32_t timestamp;         // Sample timestamp
    bool validated;             // Whether sample has been expert-validated
} TrainingSample;

/**
 * Model update gradients
 */
typedef struct {
    float* gradients;           // Gradient values
    size_t gradientSize;        // Size of gradient array
    uint32_t sampleCount;       // Number of samples used
    float loss;                 // Training loss
    float accuracy;             // Local accuracy
    uint32_t roundId;           // Federated learning round ID
} ModelUpdate;

/**
 * Federated learning configuration
 */
typedef struct {
    bool enabled;                       // Whether FL is enabled
    PrivacyLevel privacyLevel;         // Privacy protection level
    float contributionThreshold;        // Minimum confidence to contribute
    uint8_t localEpochs;               // Number of local training epochs
    float learningRate;                // Local learning rate
    uint8_t batchSize;                 // Local batch size
    uint32_t roundInterval;            // Time between FL rounds (ms)
    bool requireExpertValidation;      // Require expert validation
    float differentialPrivacyEpsilon;  // Differential privacy parameter
    char serverEndpoint[128];          // FL server endpoint
    char deviceId[32];                 // Unique device identifier
} FederatedLearningConfig;

/**
 * Federated learning statistics
 */
typedef struct {
    uint32_t totalRounds;              // Total FL rounds participated
    uint32_t successfulRounds;         // Successful rounds
    uint32_t samplesContributed;       // Total samples contributed
    uint32_t modelUpdatesReceived;     // Model updates received
    float avgLocalAccuracy;            // Average local accuracy
    float avgGlobalAccuracy;           // Average global accuracy improvement
    uint32_t totalTrainingTime;        // Total training time (ms)
    uint32_t lastRoundTimestamp;       // Last round timestamp
} FederatedLearningStats;

/**
 * Federated Learning Manager Class
 * 
 * Manages on-device federated learning including data collection,
 * local training, privacy protection, and communication with FL server.
 */
class FederatedLearningManager {
public:
    FederatedLearningManager();
    ~FederatedLearningManager();

    /**
     * Initialize federated learning system
     * @param config FL configuration
     * @return true if initialization successful
     */
    bool initialize(const FederatedLearningConfig& config);

    /**
     * Add training sample for potential contribution
     * @param imageData Image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @param trueLabel Ground truth label
     * @param confidence Confidence in ground truth
     * @param validated Whether sample is expert-validated
     * @return true if sample added successfully
     */
    bool addTrainingSample(const uint8_t* imageData, uint16_t width,
                          uint16_t height, uint8_t channels,
                          uint8_t trueLabel, float confidence, bool validated = false);

    /**
     * Start federated learning round
     * @param roundId Round identifier from server
     * @return true if round started successfully
     */
    bool startFederatedRound(uint32_t roundId);

    /**
     * Check for new federated learning rounds
     * @return true if new round available
     */
    bool checkForNewRound();

    /**
     * Get current FL status
     * @return Current federated learning status
     */
    FLRoundStatus getStatus() const { return status_; }

    /**
     * Get federated learning statistics
     * @return Current FL statistics
     */
    FederatedLearningStats getStatistics() const { return stats_; }

    /**
     * Update configuration
     * @param config New configuration
     */
    void updateConfig(const FederatedLearningConfig& config);

    /**
     * Enable or disable federated learning
     * @param enabled Whether to enable FL
     */
    void setEnabled(bool enabled);

    /**
     * Get number of collected samples
     * @return Number of samples ready for training
     */
    uint8_t getSampleCount() const { return sampleCount_; }

    /**
     * Clear all collected samples
     */
    void clearSamples();

    /**
     * Process federated learning tasks (call periodically)
     */
    void processFederatedLearning();

private:
    bool initialized_;                          // Initialization status
    FederatedLearningConfig config_;           // Configuration
    FLRoundStatus status_;                     // Current status
    FederatedLearningStats stats_;             // Statistics
    
    // Training samples
    TrainingSample samples_[MAX_LOCAL_SAMPLES]; // Local training samples
    uint8_t sampleCount_;                       // Number of collected samples
    
    // Model updates
    ModelUpdate currentUpdate_;                 // Current model update
    uint32_t currentRoundId_;                   // Current round ID
    
    // Timing
    uint32_t lastRoundCheck_;                   // Last time checked for new round
    uint32_t roundStartTime_;                   // Round start timestamp

    /**
     * Perform local training on collected samples
     * @return true if training successful
     */
    bool performLocalTraining();

    /**
     * Apply differential privacy to gradients
     * @param gradients Gradient array to modify
     * @param size Size of gradient array
     * @param epsilon Privacy parameter
     */
    void applyDifferentialPrivacy(float* gradients, size_t size, float epsilon);

    /**
     * Upload model updates to federated server
     * @param update Model update to upload
     * @return true if upload successful
     */
    bool uploadModelUpdate(const ModelUpdate& update);

    /**
     * Download global model from federated server
     * @param roundId Round ID
     * @return true if download successful
     */
    bool downloadGlobalModel(uint32_t roundId);

    /**
     * Validate sample quality
     * @param sample Sample to validate
     * @return true if sample meets quality criteria
     */
    bool validateSampleQuality(const TrainingSample& sample);

    /**
     * Select samples for training based on contribution criteria
     * @param selected Output array for selected samples
     * @param maxSamples Maximum number of samples to select
     * @return Number of samples selected
     */
    uint8_t selectSamplesForTraining(TrainingSample* selected, uint8_t maxSamples);

    /**
     * Calculate gradient updates from training samples
     * @param samples Training samples
     * @param sampleCount Number of samples
     * @param gradients Output gradient array
     * @param maxGradients Maximum gradient array size
     * @return Size of calculated gradients
     */
    size_t calculateGradients(const TrainingSample* samples, uint8_t sampleCount,
                             float* gradients, size_t maxGradients);

    /**
     * Communicate with federated learning server
     * @param endpoint Server endpoint
     * @param request Request data
     * @param requestSize Size of request data
     * @param response Response buffer
     * @param maxResponseSize Maximum response size
     * @return Response size (0 if failed)
     */
    size_t communicateWithServer(const char* endpoint, const uint8_t* request,
                                size_t requestSize, uint8_t* response,
                                size_t maxResponseSize);

    /**
     * Update federated learning statistics
     * @param roundSuccess Whether round was successful
     * @param samplesUsed Number of samples used in training
     * @param trainingTime Time spent training
     */
    void updateStatistics(bool roundSuccess, uint8_t samplesUsed, uint32_t trainingTime);

    /**
     * Generate device fingerprint for privacy
     * @param fingerprint Output buffer for fingerprint
     * @param maxSize Maximum fingerprint size
     * @return Fingerprint size
     */
    size_t generateDeviceFingerprint(uint8_t* fingerprint, size_t maxSize);
};

/**
 * Expert validation callback function type
 * @param sample Training sample requiring validation
 * @param userContext User context
 * @return true if sample is validated as correct
 */
typedef bool (*ExpertValidationCallback)(const TrainingSample* sample, void* userContext);

/**
 * Federated learning event callback function type
 * @param event Event type
 * @param roundId Round ID
 * @param userContext User context
 */
typedef void (*FLEventCallback)(FLRoundStatus event, uint32_t roundId, void* userContext);

/**
 * Global federated learning manager instance
 */
extern FederatedLearningManager* g_fl_manager;

/**
 * Initialize global federated learning manager
 * @param config FL configuration
 * @return true if initialization successful
 */
bool initializeFederatedLearning(const FederatedLearningConfig& config);

/**
 * Cleanup global federated learning manager
 */
void cleanupFederatedLearning();

/**
 * Set expert validation callback
 * @param callback Validation callback function
 * @param userContext User context for callback
 */
void setExpertValidationCallback(ExpertValidationCallback callback, void* userContext);

/**
 * Set federated learning event callback
 * @param callback Event callback function
 * @param userContext User context for callback
 */
void setFLEventCallback(FLEventCallback callback, void* userContext);

// Default federated learning configurations
extern const FederatedLearningConfig FL_CONFIG_CONSERVATIVE;
extern const FederatedLearningConfig FL_CONFIG_BALANCED;
extern const FederatedLearningConfig FL_CONFIG_AGGRESSIVE;

#endif // FEDERATED_LEARNING_H