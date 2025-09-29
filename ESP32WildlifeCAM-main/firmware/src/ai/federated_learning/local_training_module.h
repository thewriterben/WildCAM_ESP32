/**
 * @file local_training_module.h
 * @brief Local Training Module for On-Device Federated Learning
 * 
 * Implements local training capabilities for federated learning, including
 * incremental learning, data management, and training optimization for
 * ESP32 hardware constraints.
 */

#ifndef LOCAL_TRAINING_MODULE_H
#define LOCAL_TRAINING_MODULE_H

#include "federated_common.h"
#include "../tensorflow_lite_implementation.h"
#include "../tinyml/inference_engine.h"
#include <vector>
#include <memory>

/**
 * Training Mode
 */
enum class TrainingMode {
    INCREMENTAL,                // Incremental learning from new samples
    FINE_TUNING,               // Fine-tuning existing model
    TRANSFER_LEARNING,          // Transfer learning from global model
    REINFORCEMENT              // Reinforcement learning from feedback
};

/**
 * Training Progress
 */
struct TrainingProgress {
    uint32_t epoch;
    uint32_t totalEpochs;
    float loss;
    float accuracy;
    uint32_t samplesProcessed;
    uint32_t totalSamples;
    uint32_t elapsedTimeMs;
    bool completed;
    
    TrainingProgress() : epoch(0), totalEpochs(0), loss(0.0f), accuracy(0.0f),
                        samplesProcessed(0), totalSamples(0), elapsedTimeMs(0),
                        completed(false) {}
};

/**
 * Training Result
 */
struct TrainingResult {
    bool success;
    String errorMessage;
    TrainingProgress finalProgress;
    ModelUpdate modelUpdate;
    uint32_t totalTrainingTimeMs;
    uint32_t memoryUsed;
    float powerConsumed;
    
    TrainingResult() : success(false), totalTrainingTimeMs(0), memoryUsed(0),
                      powerConsumed(0.0f) {}
};

/**
 * Data Augmentation Settings
 */
struct DataAugmentationConfig {
    bool enableRotation;
    bool enableScaling;
    bool enableBrightness;
    bool enableContrast;
    bool enableNoise;
    float rotationRange;        // degrees
    float scalingRange;         // 0.0 to 1.0
    float brightnessRange;      // -1.0 to 1.0
    float contrastRange;        // 0.0 to 2.0
    float noiseLevel;           // 0.0 to 1.0
    
    DataAugmentationConfig() : enableRotation(true), enableScaling(true),
                              enableBrightness(true), enableContrast(true),
                              enableNoise(false), rotationRange(15.0f),
                              scalingRange(0.1f), brightnessRange(0.2f),
                              contrastRange(0.2f), noiseLevel(0.05f) {}
};

/**
 * Local Training Module
 * 
 * Handles on-device training for federated learning with:
 * - Incremental learning capabilities
 * - Data augmentation and preprocessing
 * - Memory-efficient training algorithms
 * - Power-aware training scheduling
 * - Training progress monitoring
 */
class LocalTrainingModule {
public:
    LocalTrainingModule();
    ~LocalTrainingModule();
    
    // Initialization
    bool init(const FederatedLearningConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Training sample management
    bool addTrainingSample(const TrainingSample& sample);
    bool addTrainingSamples(const std::vector<TrainingSample>& samples);
    void clearTrainingSamples();
    size_t getTrainingSampleCount() const { return trainingSamples_.size(); }
    
    // Model training
    TrainingResult trainModel(WildlifeModelType modelType, TrainingMode mode = TrainingMode::INCREMENTAL);
    TrainingResult continueTraining(const String& checkpointId);
    bool pauseTraining();
    bool resumeTraining();
    bool stopTraining();
    
    // Training progress and monitoring
    TrainingProgress getTrainingProgress() const { return currentProgress_; }
    bool isTraining() const { return trainingActive_; }
    String getCurrentCheckpointId() const { return currentCheckpointId_; }
    
    // Model management
    bool loadGlobalModel(const ModelUpdate& globalModel);
    ModelUpdate exportModelUpdate();
    bool validateModelUpdate(const ModelUpdate& update);
    
    // Data augmentation
    void setDataAugmentationConfig(const DataAugmentationConfig& config);
    DataAugmentationConfig getDataAugmentationConfig() const { return augmentationConfig_; }
    
    // Configuration
    void setLearningRate(float learningRate);
    void setBatchSize(uint32_t batchSize);
    void setMaxEpochs(uint32_t maxEpochs);
    void setMaxTrainingTime(uint32_t maxTimeMs);
    
    // Power management integration
    void setPowerConstraints(float maxPowerConsumption, uint32_t maxMemoryUsage);
    bool isPowerOptimized() const { return powerOptimized_; }
    
    // Metrics and monitoring
    FederatedLearningMetrics getMetrics() const { return metrics_; }
    float getModelAccuracy(WildlifeModelType modelType) const;
    uint32_t getMemoryUsage() const;
    float getCurrentPowerConsumption() const;
    
    // Callbacks for training events
    typedef std::function<void(const TrainingProgress&)> ProgressCallback;
    typedef std::function<void(const TrainingResult&)> CompletionCallback;
    typedef std::function<bool()> PowerCheckCallback;
    
    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }
    void setCompletionCallback(CompletionCallback callback) { completionCallback_ = callback; }
    void setPowerCheckCallback(PowerCheckCallback callback) { powerCheckCallback_ = callback; }

private:
    // Configuration
    FederatedLearningConfig config_;
    DataAugmentationConfig augmentationConfig_;
    bool initialized_;
    bool powerOptimized_;
    
    // Training state
    bool trainingActive_;
    TrainingMode currentMode_;
    WildlifeModelType currentModelType_;
    TrainingProgress currentProgress_;
    String currentCheckpointId_;
    uint32_t trainingStartTime_;
    
    // Training data
    std::vector<TrainingSample> trainingSamples_;
    std::vector<TrainingSample> validationSamples_;
    size_t currentBatchIndex_;
    
    // Model components
    std::unique_ptr<TensorFlowLiteImplementation> tfLiteImpl_;
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    
    // Power management
    float maxPowerConsumption_;
    uint32_t maxMemoryUsage_;
    uint32_t currentMemoryUsage_;
    
    // Metrics
    FederatedLearningMetrics metrics_;
    
    // Callbacks
    ProgressCallback progressCallback_;
    CompletionCallback completionCallback_;
    PowerCheckCallback powerCheckCallback_;
    
    // Private methods
    bool prepareTrainingData();
    bool preprocessSample(TrainingSample& sample);
    TrainingSample augmentSample(const TrainingSample& sample);
    
    bool initializeModelForTraining(WildlifeModelType modelType);
    bool performTrainingEpoch();
    bool performIncrementalLearning();
    bool performFineTuning();
    bool performTransferLearning();
    
    bool validateTrainingData() const;
    bool checkPowerConstraints() const;
    bool checkMemoryConstraints() const;
    
    void updateProgress(uint32_t epoch, float loss, float accuracy);
    void saveCheckpoint();
    bool loadCheckpoint(const String& checkpointId);
    
    // Data augmentation methods
    CameraFrame rotateImage(const CameraFrame& frame, float angle);
    CameraFrame scaleImage(const CameraFrame& frame, float scale);
    CameraFrame adjustBrightness(const CameraFrame& frame, float brightness);
    CameraFrame adjustContrast(const CameraFrame& frame, float contrast);
    CameraFrame addNoise(const CameraFrame& frame, float noiseLevel);
    
    // Utility methods
    String generateCheckpointId() const;
    uint32_t calculateRequiredMemory(WildlifeModelType modelType) const;
    float estimateTrainingTime(uint32_t samples, uint32_t epochs) const;
};

// Global instance
extern LocalTrainingModule* g_localTrainingModule;

// Utility functions
bool initializeLocalTraining(const FederatedLearningConfig& config);
void cleanupLocalTraining();
bool startTraining(WildlifeModelType modelType, const std::vector<TrainingSample>& samples);
TrainingResult getLastTrainingResult();
bool isLocalTrainingActive();

#endif // LOCAL_TRAINING_MODULE_H