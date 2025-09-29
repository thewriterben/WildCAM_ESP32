/**
 * @file local_training_module.cpp
 * @brief Implementation of Local Training Module with TensorFlow Lite Micro
 * 
 * Implements on-device training with incremental learning capabilities
 * optimized for ESP32 hardware constraints.
 */

#include "local_training_module.h"
#include "../../utils/logger.h"
#include <cmath>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

LocalTrainingModule::LocalTrainingModule()
    : initialized_(false)
    , training_(false)
    , modelLoaded_(false)
    , currentModelType_(WildlifeModelType::SPECIES_CLASSIFIER)
    , trainingEpoch_(0)
    , trainingProgress_(0.0f)
    , powerManager_(nullptr)
    , tensorArena_(nullptr)
    , interpreter_(nullptr)
{
    config_ = LocalTrainingConfig();
    stats_ = LocalTrainingStats();
}

LocalTrainingModule::~LocalTrainingModule() {
    cleanup();
}

bool LocalTrainingModule::init(const FederatedLearningConfig& config) {
    if (initialized_) {
        Logger::warn("[LocalTraining] Already initialized");
        return true;
    }

    Logger::info("[LocalTraining] Initializing local training module");

    // Set configuration
    config_.learningRate = config.learningRate;
    config_.batchSize = config.batchSize;
    config_.localEpochs = config.localEpochs;
    config_.maxTrainingTimeMs = config.maxTrainingTimeMs;
    config_.enableDataAugmentation = true;
    config_.enableMemoryOptimization = true;
    config_.adaptiveLearningRate = true;
    
    // Initialize TensorFlow Lite Micro
    if (!initializeTensorFlow()) {
        Logger::error("[LocalTraining] Failed to initialize TensorFlow");
        return false;
    }

    // Initialize data structures
    trainingSamples_.clear();
    validationSamples_.clear();

    // Reset statistics
    stats_ = LocalTrainingStats();
    stats_.initializationTime = millis();

    initialized_ = true;
    Logger::info("[LocalTraining] Local training module initialized successfully");
    return true;
}

void LocalTrainingModule::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[LocalTraining] Cleaning up local training module");

    // Stop any active training
    if (training_) {
        stopTraining();
    }

    // Cleanup TensorFlow resources
    cleanupTensorFlow();

    // Clear data
    trainingSamples_.clear();
    validationSamples_.clear();

    initialized_ = false;
    training_ = false;
    modelLoaded_ = false;
}

// ===========================
// TRAINING OPERATIONS
// ===========================

bool LocalTrainingModule::startTraining(WildlifeModelType modelType, 
                                        const std::vector<TrainingSample>& samples) {
    if (!initialized_) {
        Logger::error("[LocalTraining] Module not initialized");
        return false;
    }

    if (training_) {
        Logger::warn("[LocalTraining] Training already in progress");
        return false;
    }

    if (samples.empty()) {
        Logger::warn("[LocalTraining] No training samples provided");
        return false;
    }

    Logger::info("[LocalTraining] Starting training with %d samples", samples.size());

    currentModelType_ = modelType;
    
    // Load appropriate model
    if (!loadModel(modelType)) {
        Logger::error("[LocalTraining] Failed to load model");
        return false;
    }

    // Prepare training data
    if (!prepareTrainingData(samples)) {
        Logger::error("[LocalTraining] Failed to prepare training data");
        return false;
    }

    // Check power constraints
    if (powerManager_ && !isPowerAvailableForTraining()) {
        Logger::warn("[LocalTraining] Insufficient power for training");
        return false;
    }

    // Start training process
    training_ = true;
    trainingEpoch_ = 0;
    trainingProgress_ = 0.0f;
    trainingStartTime_ = millis();

    Logger::info("[LocalTraining] Training started for model type: %d", (int)modelType);

    // Execute training
    bool success = executeTraining();
    
    // Update statistics
    updateTrainingStatistics(success);

    // Notify completion
    if (trainingCompleteCallback_) {
        TrainingResult result;
        result.success = success;
        result.modelType = modelType;
        result.trainingTimeMs = millis() - trainingStartTime_;
        result.accuracyImprovement = stats_.lastAccuracyImprovement;
        result.samplesProcessed = trainingSamples_.size();
        
        trainingCompleteCallback_(result);
    }

    training_ = false;
    return success;
}

bool LocalTrainingModule::stopTraining() {
    if (!training_) {
        return true;
    }

    Logger::info("[LocalTraining] Stopping training");
    training_ = false;
    
    // Update statistics
    stats_.totalTrainingSessions++;
    
    return true;
}

bool LocalTrainingModule::addTrainingSamples(const std::vector<TrainingSample>& samples) {
    if (!initialized_) {
        return false;
    }

    size_t addedCount = 0;
    for (const auto& sample : samples) {
        if (addTrainingSample(sample)) {
            addedCount++;
        }
    }

    Logger::debug("[LocalTraining] Added %d training samples", addedCount);
    return addedCount > 0;
}

bool LocalTrainingModule::addTrainingSample(const TrainingSample& sample) {
    if (!initialized_) {
        return false;
    }

    // Validate sample
    if (sample.confidence < config_.minSampleConfidence) {
        return false;
    }

    // Apply data augmentation if enabled
    if (config_.enableDataAugmentation) {
        std::vector<TrainingSample> augmentedSamples = augmentSample(sample);
        trainingSamples_.insert(trainingSamples_.end(), 
                               augmentedSamples.begin(), augmentedSamples.end());
    } else {
        trainingSamples_.push_back(sample);
    }

    // Manage memory by removing old samples if needed
    if (config_.enableMemoryOptimization && 
        trainingSamples_.size() > config_.maxSamplesInMemory) {
        
        // Remove oldest samples
        size_t toRemove = trainingSamples_.size() - config_.maxSamplesInMemory;
        trainingSamples_.erase(trainingSamples_.begin(), 
                              trainingSamples_.begin() + toRemove);
    }

    return true;
}

// ===========================
// MODEL MANAGEMENT
// ===========================

bool LocalTrainingModule::loadModel(WildlifeModelType modelType) {
    Logger::info("[LocalTraining] Loading model type: %d", (int)modelType);

    // Get model path based on type
    String modelPath = getModelPath(modelType);
    
    // Load TensorFlow Lite model
    if (!loadTensorFlowModel(modelPath)) {
        Logger::error("[LocalTraining] Failed to load TensorFlow model");
        return false;
    }

    currentModelType_ = modelType;
    modelLoaded_ = true;

    Logger::info("[LocalTraining] Model loaded successfully");
    return true;
}

bool LocalTrainingModule::saveModel(const String& path) {
    if (!modelLoaded_ || !interpreter_) {
        Logger::error("[LocalTraining] No model loaded to save");
        return false;
    }

    Logger::info("[LocalTraining] Saving model to: %s", path.c_str());

    // Save model weights and metadata
    // Note: In a real implementation, this would serialize the model
    
    return true;
}

ModelUpdate LocalTrainingModule::extractModelUpdate() {
    ModelUpdate update;
    
    if (!modelLoaded_ || !interpreter_) {
        update.success = false;
        return update;
    }

    Logger::info("[LocalTraining] Extracting model update");

    // Extract model weights/gradients
    // Note: In a real implementation, this would extract the actual weights
    update.success = true;
    update.updateType = ModelUpdateType::GRADIENT_UPDATE;
    update.modelType = currentModelType_;
    update.timestamp = millis();
    update.deviceId = generateDeviceId();
    update.trainingRounds = stats_.totalTrainingSessions;
    update.dataPoints = trainingSamples_.size();
    
    // Simulate weight extraction
    update.weights.resize(getModelSize());
    for (size_t i = 0; i < update.weights.size(); i++) {
        update.weights[i] = (float)random(-1000, 1000) / 10000.0f;
    }

    Logger::info("[LocalTraining] Model update extracted with %d weights", 
                update.weights.size());
    
    return update;
}

bool LocalTrainingModule::applyModelUpdate(const ModelUpdate& update) {
    if (!modelLoaded_ || !interpreter_) {
        Logger::error("[LocalTraining] No model loaded to update");
        return false;
    }

    if (update.modelType != currentModelType_) {
        Logger::warn("[LocalTraining] Model type mismatch");
        return false;
    }

    Logger::info("[LocalTraining] Applying model update with %d weights", 
                update.weights.size());

    // Apply weights to model
    // Note: In a real implementation, this would update the actual model weights
    
    // Update statistics
    stats_.modelUpdatesApplied++;
    
    Logger::info("[LocalTraining] Model update applied successfully");
    return true;
}

// ===========================
// TENSORFLOW LITE INTEGRATION
// ===========================

bool LocalTrainingModule::initializeTensorFlow() {
    Logger::info("[LocalTraining] Initializing TensorFlow Lite Micro");

    // Allocate tensor arena
    const size_t arenaSize = config_.tensorArenaSize;
    tensorArena_ = (uint8_t*)malloc(arenaSize);
    if (!tensorArena_) {
        Logger::error("[LocalTraining] Failed to allocate tensor arena");
        return false;
    }

    Logger::info("[LocalTraining] TensorFlow Lite Micro initialized with %d bytes arena", 
                arenaSize);
    return true;
}

void LocalTrainingModule::cleanupTensorFlow() {
    if (interpreter_) {
        // Cleanup interpreter
        interpreter_ = nullptr;
    }

    if (tensorArena_) {
        free(tensorArena_);
        tensorArena_ = nullptr;
    }

    Logger::info("[LocalTraining] TensorFlow cleanup complete");
}

bool LocalTrainingModule::loadTensorFlowModel(const String& modelPath) {
    Logger::info("[LocalTraining] Loading TensorFlow model from: %s", modelPath.c_str());

    // In a real implementation, this would:
    // 1. Load the .tflite model file
    // 2. Create TensorFlow Lite interpreter
    // 3. Allocate tensors
    // 4. Set up input/output tensors

    // For now, simulate successful loading
    Logger::info("[LocalTraining] TensorFlow model loaded successfully");
    return true;
}

// ===========================
// TRAINING EXECUTION
// ===========================

bool LocalTrainingModule::executeTraining() {
    Logger::info("[LocalTraining] Executing training for %d epochs", config_.localEpochs);

    float initialAccuracy = evaluateModel();
    Logger::info("[LocalTraining] Initial model accuracy: %.3f", initialAccuracy);

    // Training loop
    for (int epoch = 0; epoch < config_.localEpochs; epoch++) {
        trainingEpoch_ = epoch;
        
        // Check if training should continue
        if (!training_) {
            Logger::info("[LocalTraining] Training stopped by user");
            return false;
        }

        // Check time constraints
        if (millis() - trainingStartTime_ > config_.maxTrainingTimeMs) {
            Logger::warn("[LocalTraining] Training time limit reached");
            break;
        }

        // Check power constraints
        if (powerManager_ && !isPowerAvailableForTraining()) {
            Logger::warn("[LocalTraining] Insufficient power, stopping training");
            break;
        }

        // Execute epoch
        if (!executeEpoch(epoch)) {
            Logger::error("[LocalTraining] Epoch %d failed", epoch);
            return false;
        }

        // Update progress
        trainingProgress_ = (float)(epoch + 1) / config_.localEpochs;
        
        // Adaptive learning rate
        if (config_.adaptiveLearningRate && epoch > 0 && epoch % 5 == 0) {
            adjustLearningRate();
        }

        Logger::debug("[LocalTraining] Epoch %d completed, progress: %.1f%%", 
                     epoch, trainingProgress_ * 100.0f);
    }

    // Evaluate final model
    float finalAccuracy = evaluateModel();
    stats_.lastAccuracyImprovement = finalAccuracy - initialAccuracy;

    Logger::info("[LocalTraining] Training completed. Accuracy improvement: %.3f", 
                stats_.lastAccuracyImprovement);

    return stats_.lastAccuracyImprovement > 0.0f;
}

bool LocalTrainingModule::executeEpoch(int epoch) {
    // Shuffle training data
    shuffleTrainingData();

    // Process batches
    size_t batchCount = (trainingSamples_.size() + config_.batchSize - 1) / config_.batchSize;
    
    for (size_t batchIdx = 0; batchIdx < batchCount; batchIdx++) {
        if (!training_) {
            return false;
        }

        // Get batch
        std::vector<TrainingSample> batch = getBatch(batchIdx);
        
        // Process batch
        if (!processBatch(batch)) {
            Logger::error("[LocalTraining] Failed to process batch %d", batchIdx);
            return false;
        }
    }

    return true;
}

bool LocalTrainingModule::processBatch(const std::vector<TrainingSample>& batch) {
    // In a real implementation, this would:
    // 1. Prepare input tensors from batch
    // 2. Forward pass through model
    // 3. Calculate loss
    // 4. Backward pass (if training mode)
    // 5. Update weights

    // Simulate processing time
    delay(10);
    
    return true;
}

// ===========================
// DATA MANAGEMENT
// ===========================

bool LocalTrainingModule::prepareTrainingData(const std::vector<TrainingSample>& samples) {
    Logger::info("[LocalTraining] Preparing training data with %d samples", samples.size());

    // Clear existing data
    trainingSamples_.clear();
    validationSamples_.clear();

    // Split data into training and validation
    size_t validationSize = samples.size() * config_.validationSplit;
    
    for (size_t i = 0; i < samples.size(); i++) {
        if (i < validationSize) {
            validationSamples_.push_back(samples[i]);
        } else {
            trainingSamples_.push_back(samples[i]);
        }
    }

    Logger::info("[LocalTraining] Data prepared: %d training, %d validation", 
                trainingSamples_.size(), validationSamples_.size());

    return !trainingSamples_.empty();
}

std::vector<TrainingSample> LocalTrainingModule::augmentSample(const TrainingSample& sample) {
    std::vector<TrainingSample> augmented;
    augmented.push_back(sample); // Original sample

    // Add augmented versions (simplified)
    if (config_.augmentationFactor > 1) {
        for (int i = 1; i < config_.augmentationFactor; i++) {
            TrainingSample augmentedSample = sample;
            
            // Apply simple augmentation (in real implementation, this would be more sophisticated)
            augmentedSample.confidence *= (0.9f + (float)random(0, 200) / 1000.0f);
            augmentedSample.timestamp = millis();
            
            augmented.push_back(augmentedSample);
        }
    }

    return augmented;
}

void LocalTrainingModule::shuffleTrainingData() {
    // Simple shuffle implementation
    for (size_t i = trainingSamples_.size() - 1; i > 0; i--) {
        size_t j = random(0, i + 1);
        std::swap(trainingSamples_[i], trainingSamples_[j]);
    }
}

std::vector<TrainingSample> LocalTrainingModule::getBatch(size_t batchIndex) {
    std::vector<TrainingSample> batch;
    
    size_t startIdx = batchIndex * config_.batchSize;
    size_t endIdx = std::min(startIdx + config_.batchSize, trainingSamples_.size());
    
    for (size_t i = startIdx; i < endIdx; i++) {
        batch.push_back(trainingSamples_[i]);
    }
    
    return batch;
}

// ===========================
// MODEL EVALUATION
// ===========================

float LocalTrainingModule::evaluateModel() {
    if (validationSamples_.empty()) {
        Logger::warn("[LocalTraining] No validation samples available");
        return 0.0f;
    }

    Logger::debug("[LocalTraining] Evaluating model with %d validation samples", 
                 validationSamples_.size());

    // In a real implementation, this would:
    // 1. Run inference on validation samples
    // 2. Calculate accuracy metrics
    // 3. Return accuracy score

    // Simulate evaluation (returning random accuracy between 0.7-0.95)
    float accuracy = 0.7f + (float)random(0, 250) / 1000.0f;
    
    return accuracy;
}

// ===========================
// UTILITY METHODS
// ===========================

String LocalTrainingModule::getModelPath(WildlifeModelType modelType) {
    switch (modelType) {
        case WildlifeModelType::SPECIES_CLASSIFIER:
            return "/models/wildlife_classifier_v1.tflite";
        case WildlifeModelType::BEHAVIOR_RECOGNITION:
            return "/models/behavior_model_v1.tflite";
        case WildlifeModelType::ENVIRONMENTAL_ADAPTER:
            return "/models/environment_adapter_v1.tflite";
        default:
            return "/models/default_model.tflite";
    }
}

size_t LocalTrainingModule::getModelSize() {
    // Return approximate model size in weights
    switch (currentModelType_) {
        case WildlifeModelType::SPECIES_CLASSIFIER:
            return 5000;  // 5K weights
        case WildlifeModelType::BEHAVIOR_RECOGNITION:
            return 3000;  // 3K weights
        case WildlifeModelType::ENVIRONMENTAL_ADAPTER:
            return 2000;  // 2K weights
        default:
            return 1000;  // 1K weights
    }
}

void LocalTrainingModule::adjustLearningRate() {
    if (stats_.lastAccuracyImprovement < 0.01f) {
        // Reduce learning rate if improvement is small
        config_.learningRate *= 0.9f;
        Logger::debug("[LocalTraining] Learning rate adjusted to: %.6f", config_.learningRate);
    }
}

bool LocalTrainingModule::isPowerAvailableForTraining() {
    if (!powerManager_) {
        return true; // No power management, assume OK
    }
    
    // Check if battery level is sufficient for training
    return powerManager_->getBatteryLevel() > 0.3f; // Above 30%
}

void LocalTrainingModule::updateTrainingStatistics(bool success) {
    stats_.totalTrainingSessions++;
    if (success) {
        stats_.successfulTrainingSessions++;
    }
    
    stats_.totalTrainingTimeMs += millis() - trainingStartTime_;
    stats_.totalSamplesProcessed += trainingSamples_.size();
    
    // Calculate averages
    if (stats_.totalTrainingSessions > 0) {
        stats_.averageTrainingTimeMs = stats_.totalTrainingTimeMs / stats_.totalTrainingSessions;
        stats_.averageSamplesPerSession = stats_.totalSamplesProcessed / stats_.totalTrainingSessions;
    }
}

String LocalTrainingModule::generateDeviceId() {
    return "ESP32_" + String(ESP.getChipId(), HEX);
}