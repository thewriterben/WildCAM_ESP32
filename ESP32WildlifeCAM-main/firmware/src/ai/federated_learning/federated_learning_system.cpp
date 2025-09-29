/**
 * @file federated_learning_system.cpp
 * @brief Implementation of the main Federated Learning System
 * 
 * Core federated learning system that coordinates all components and provides
 * high-level API for federated learning operations.
 */

#include "federated_learning_system.h"
#include "../../utils/logger.h"
#include <ArduinoJson.h>

// Global instance
FederatedLearningSystem* g_federatedLearningSystem = nullptr;

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

FederatedLearningSystem::FederatedLearningSystem()
    : initialized_(false)
    , status_(FLSystemStatus::UNINITIALIZED)
    , automaticMode_(false)
    , powerManager_(nullptr)
    , powerOptimized_(false)
    , maxPowerConsumption_(1.0f)
    , maxMemoryUsage_(50000)
    , systemStartTime_(0)
    , webInterfaceEnabled_(false)
    , webInterfacePort_(80)
{
    systemStats_ = FLSystemStats();
}

FederatedLearningSystem::~FederatedLearningSystem() {
    cleanup();
}

bool FederatedLearningSystem::init(const FLSystemConfig& config) {
    if (initialized_) {
        logEvent("System already initialized", "WARNING");
        return true;
    }

    logEvent("Initializing Federated Learning System", "INFO");
    
    // Validate configuration
    if (!validateConfiguration()) {
        logEvent("Configuration validation failed", "ERROR");
        return false;
    }

    config_ = config;
    systemStartTime_ = getCurrentTimestamp();
    
    // Initialize components
    if (!initializeComponents()) {
        logEvent("Component initialization failed", "ERROR");
        cleanup();
        return false;
    }

    // Setup component callbacks
    setupComponentCallbacks();

    // Initialize power management if enabled
    if (config_.enablePowerManagement && powerManager_) {
        powerOptimized_ = true;
        logEvent("Power management enabled", "INFO");
    }

    // Update status
    updateSystemStatus(FLSystemStatus::READY);
    initialized_ = true;

    logEvent("Federated Learning System initialized successfully", "INFO");
    return true;
}

void FederatedLearningSystem::cleanup() {
    if (!initialized_) {
        return;
    }

    logEvent("Cleaning up Federated Learning System", "INFO");
    
    // Stop any active operations
    stop();
    
    // Stop web interface
    if (webInterfaceEnabled_) {
        stopWebInterface();
    }

    // Cleanup components
    cleanupComponents();

    // Reset state
    initialized_ = false;
    status_ = FLSystemStatus::UNINITIALIZED;
    automaticMode_ = false;
    powerManager_ = nullptr;
    
    logEvent("Federated Learning System cleanup complete", "INFO");
}

// ===========================
// SYSTEM CONTROL
// ===========================

bool FederatedLearningSystem::start() {
    if (!initialized_) {
        logEvent("Cannot start - system not initialized", "ERROR");
        return false;
    }

    if (status_ == FLSystemStatus::ACTIVE) {
        logEvent("System already active", "WARNING");
        return true;
    }

    logEvent("Starting Federated Learning System", "INFO");

    // Check power constraints
    if (powerOptimized_ && !isPowerAvailableForTraining()) {
        logEvent("Insufficient power for federated learning", "WARNING");
        return false;
    }

    // Start network topology manager
    if (topologyManager_ && !topologyManager_->start()) {
        logEvent("Failed to start network topology manager", "ERROR");
        return false;
    }

    // Start coordinator
    if (coordinator_ && !coordinator_->start()) {
        logEvent("Failed to start federated learning coordinator", "ERROR");
        return false;
    }

    // Enable automatic mode if configured
    if (config_.enableAutomaticMode) {
        automaticMode_ = true;
        logEvent("Automatic mode enabled", "INFO");
    }

    updateSystemStatus(FLSystemStatus::ACTIVE);
    logEvent("Federated Learning System started successfully", "INFO");
    return true;
}

bool FederatedLearningSystem::stop() {
    if (status_ == FLSystemStatus::READY || status_ == FLSystemStatus::UNINITIALIZED) {
        return true;
    }

    logEvent("Stopping Federated Learning System", "INFO");

    // Disable automatic mode
    automaticMode_ = false;

    // Stop coordinator
    if (coordinator_) {
        coordinator_->stop();
    }

    // Stop topology manager
    if (topologyManager_) {
        topologyManager_->stop();
    }

    // Stop any active training
    if (localTraining_) {
        localTraining_->stopTraining();
    }

    updateSystemStatus(FLSystemStatus::READY);
    logEvent("Federated Learning System stopped", "INFO");
    return true;
}

bool FederatedLearningSystem::pause() {
    if (status_ != FLSystemStatus::ACTIVE) {
        return false;
    }

    logEvent("Pausing Federated Learning System", "INFO");
    updateSystemStatus(FLSystemStatus::PAUSED);
    return true;
}

bool FederatedLearningSystem::resume() {
    if (status_ != FLSystemStatus::PAUSED) {
        return false;
    }

    logEvent("Resuming Federated Learning System", "INFO");
    updateSystemStatus(FLSystemStatus::ACTIVE);
    return true;
}

// ===========================
// FEDERATED LEARNING OPERATIONS
// ===========================

bool FederatedLearningSystem::enableFederatedLearning(bool enable) {
    config_.federatedConfig.enableFederatedLearning = enable;
    
    if (enable) {
        logEvent("Federated learning enabled", "INFO");
        if (status_ == FLSystemStatus::READY) {
            return start();
        }
    } else {
        logEvent("Federated learning disabled", "INFO");
        if (status_ == FLSystemStatus::ACTIVE) {
            return stop();
        }
    }
    
    return true;
}

bool FederatedLearningSystem::participateInRound(const String& roundId) {
    if (!initialized_ || status_ != FLSystemStatus::ACTIVE) {
        logEvent("Cannot participate - system not active", "ERROR");
        return false;
    }

    if (!coordinator_) {
        logEvent("No coordinator available", "ERROR");
        return false;
    }

    // Check power constraints
    if (powerOptimized_ && !isPowerAvailableForTraining()) {
        logEvent("Insufficient power for round participation", "WARNING");
        return false;
    }

    logEvent("Participating in federated learning round: " + roundId, "INFO");
    currentRoundId_ = roundId;
    
    // Participate through coordinator
    return coordinator_->participateInRound(roundId);
}

bool FederatedLearningSystem::initiateTraining(WildlifeModelType modelType, 
                                               const std::vector<TrainingSample>& samples) {
    if (!initialized_ || !localTraining_) {
        logEvent("Cannot initiate training - system not ready", "ERROR");
        return false;
    }

    if (samples.empty()) {
        logEvent("No training samples provided", "WARNING");
        return false;
    }

    logEvent("Initiating local training with " + String(samples.size()) + " samples", "INFO");
    
    // Start local training
    return localTraining_->startTraining(modelType, samples);
}

// ===========================
// DATA MANAGEMENT
// ===========================

bool FederatedLearningSystem::addTrainingData(const TrainingSample& sample) {
    if (!initialized_) {
        return false;
    }

    // Validate sample quality
    if (sample.confidence < config_.federatedConfig.contributionThreshold) {
        logEvent("Sample quality below threshold", "DEBUG");
        return false;
    }

    pendingTrainingData_.push_back(sample);
    
    // Trigger training if we have enough samples
    if (pendingTrainingData_.size() >= config_.federatedConfig.minSamplesBeforeContribution) {
        if (automaticMode_ && localTraining_) {
            localTraining_->addTrainingSamples(pendingTrainingData_);
            pendingTrainingData_.clear();
        }
    }

    return true;
}

bool FederatedLearningSystem::addTrainingData(const std::vector<TrainingSample>& samples) {
    bool allSuccess = true;
    for (const auto& sample : samples) {
        if (!addTrainingData(sample)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

void FederatedLearningSystem::clearTrainingData() {
    pendingTrainingData_.clear();
    logEvent("Training data cleared", "INFO");
}

size_t FederatedLearningSystem::getTrainingDataCount() const {
    return pendingTrainingData_.size();
}

// ===========================
// NETWORK MANAGEMENT
// ===========================

bool FederatedLearningSystem::discoverNetwork() {
    if (!topologyManager_) {
        return false;
    }

    logEvent("Discovering federated learning networks", "INFO");
    return topologyManager_->discoverNetworks();
}

bool FederatedLearningSystem::joinNetwork(const String& networkId) {
    if (!topologyManager_) {
        return false;
    }

    logEvent("Joining network: " + networkId, "INFO");
    currentNetworkId_ = networkId;
    return topologyManager_->joinNetwork(networkId);
}

// ===========================
// PRIVACY MANAGEMENT
// ===========================

void FederatedLearningSystem::setPrivacyLevel(PrivacyLevel level) {
    config_.federatedConfig.privacyLevel = level;
    
    if (privacyAggregation_) {
        AggregationConfig aggConfig = privacyAggregation_->getAggregationConfig();
        aggConfig.privacyLevel = level;
        privacyAggregation_->setAggregationConfig(aggConfig);
    }

    logEvent("Privacy level set to: " + String(privacyLevelToString(level)), "INFO");
}

float FederatedLearningSystem::getRemainingPrivacyBudget() const {
    if (!privacyAggregation_) {
        return 0.0f;
    }
    return privacyAggregation_->getRemainingPrivacyBudget();
}

// ===========================
// MONITORING AND STATISTICS
// ===========================

FLSystemStats FederatedLearningSystem::getSystemStats() const {
    FLSystemStats stats = systemStats_;
    stats.status = status_;
    stats.uptime = getCurrentTimestamp() - systemStartTime_;
    stats.memoryUsage = ESP.getFreeHeap();
    
    if (powerManager_) {
        stats.batteryLevel = powerManager_->getBatteryLevel();
    }

    return stats;
}

// ===========================
// PRIVATE METHODS
// ===========================

bool FederatedLearningSystem::initializeComponents() {
    logEvent("Initializing federated learning components", "INFO");

    // Initialize local training module
    localTraining_ = std::unique_ptr<LocalTrainingModule>(new LocalTrainingModule());
    if (!localTraining_->init(config_.federatedConfig)) {
        logEvent("Failed to initialize local training module", "ERROR");
        return false;
    }

    // Initialize privacy aggregation
    privacyAggregation_ = std::unique_ptr<PrivacyPreservingAggregation>(new PrivacyPreservingAggregation());
    if (!privacyAggregation_->init(config_.aggregationConfig)) {
        logEvent("Failed to initialize privacy aggregation", "ERROR");
        return false;
    }

    // Initialize coordinator
    coordinator_ = std::unique_ptr<FederatedLearningCoordinator>(new FederatedLearningCoordinator());
    if (!coordinator_->init(config_.federatedConfig)) {
        logEvent("Failed to initialize coordinator", "ERROR");
        return false;
    }

    // Initialize topology manager
    topologyManager_ = std::unique_ptr<NetworkTopologyManager>(new NetworkTopologyManager());
    if (!topologyManager_->init(config_.topologyConfig)) {
        logEvent("Failed to initialize topology manager", "ERROR");
        return false;
    }

    // Initialize update protocol
    updateProtocol_ = std::unique_ptr<ModelUpdateProtocol>(new ModelUpdateProtocol());
    if (!updateProtocol_->init(config_.networkConfig)) {
        logEvent("Failed to initialize update protocol", "ERROR");
        return false;
    }

    logEvent("All components initialized successfully", "INFO");
    return true;
}

void FederatedLearningSystem::cleanupComponents() {
    if (updateProtocol_) {
        updateProtocol_->cleanup();
        updateProtocol_.reset();
    }

    if (topologyManager_) {
        topologyManager_->cleanup();
        topologyManager_.reset();
    }

    if (coordinator_) {
        coordinator_->cleanup();
        coordinator_.reset();
    }

    if (privacyAggregation_) {
        privacyAggregation_->cleanup();
        privacyAggregation_.reset();
    }

    if (localTraining_) {
        localTraining_->cleanup();
        localTraining_.reset();
    }
}

bool FederatedLearningSystem::validateConfiguration() const {
    // Basic validation
    if (config_.federatedConfig.learningRate <= 0.0f || config_.federatedConfig.learningRate > 1.0f) {
        return false;
    }

    if (config_.federatedConfig.contributionThreshold < 0.0f || config_.federatedConfig.contributionThreshold > 1.0f) {
        return false;
    }

    if (config_.federatedConfig.privacyEpsilon <= 0.0f) {
        return false;
    }

    return true;
}

void FederatedLearningSystem::updateSystemStatus(FLSystemStatus newStatus) {
    FLSystemStatus oldStatus = status_;
    status_ = newStatus;
    
    if (statusChangeCallback_) {
        statusChangeCallback_(oldStatus, newStatus);
    }
    
    updateStatistics();
}

void FederatedLearningSystem::setupComponentCallbacks() {
    // Setup callbacks for component coordination
    if (localTraining_) {
        localTraining_->setTrainingCompleteCallback(
            [this](const TrainingResult& result) {
                handleTrainingComplete(result);
            });
    }

    if (coordinator_) {
        coordinator_->setRoundCompleteCallback(
            [this](const String& roundId, const RoundStatistics& stats) {
                handleRoundComplete(roundId, stats);
            });
    }
}

void FederatedLearningSystem::handleTrainingComplete(const TrainingResult& result) {
    systemStats_.modelsContributed++;
    
    if (result.success && result.accuracyImprovement > 0.0f) {
        systemStats_.averageAccuracyImprovement = 
            (systemStats_.averageAccuracyImprovement + result.accuracyImprovement) / 2.0f;
    }

    if (trainingCompleteCallback_) {
        trainingCompleteCallback_(currentRoundId_, result);
    }

    logEvent("Training completed - Accuracy improvement: " + String(result.accuracyImprovement), "INFO");
}

void FederatedLearningSystem::handleRoundComplete(const String& roundId, const RoundStatistics& stats) {
    systemStats_.totalRounds++;
    if (stats.success) {
        systemStats_.successfulRounds++;
    }

    logEvent("Federated learning round completed: " + roundId, "INFO");
}

void FederatedLearningSystem::updateAutomaticMode() {
    if (!automaticMode_ || status_ != FLSystemStatus::ACTIVE) {
        return;
    }

    // Check for new training opportunities
    evaluateTrainingOpportunity();
    
    // Check for new rounds
    checkForNewRounds();
    
    // Optimize system performance
    optimizeSystemPerformance();
    
    // Handle power constraints
    handlePowerConstraints();
}

void FederatedLearningSystem::evaluateTrainingOpportunity() {
    // Check if we have enough data for training
    if (pendingTrainingData_.size() >= config_.federatedConfig.minSamplesBeforeContribution) {
        if (isPowerAvailableForTraining() && localTraining_) {
            localTraining_->addTrainingSamples(pendingTrainingData_);
            pendingTrainingData_.clear();
        }
    }
}

void FederatedLearningSystem::checkForNewRounds() {
    if (coordinator_) {
        coordinator_->checkForNewRounds();
    }
}

void FederatedLearningSystem::optimizeSystemPerformance() {
    updateStatistics();
}

void FederatedLearningSystem::handlePowerConstraints() {
    if (!powerOptimized_ || !powerManager_) {
        return;
    }

    float batteryLevel = powerManager_->getBatteryLevel();
    if (batteryLevel < 0.2f) { // Below 20%
        logEvent("Low battery - pausing federated learning", "WARNING");
        pause();
    }
}

bool FederatedLearningSystem::isPowerAvailableForTraining() const {
    if (!powerOptimized_ || !powerManager_) {
        return true;
    }

    return powerManager_->getBatteryLevel() > 0.3f; // Above 30%
}

void FederatedLearningSystem::logEvent(const String& event, const String& level) {
    if (!config_.enableLogging) {
        return;
    }

    String logMessage = "[FL] " + event;
    
    if (level == "ERROR") {
        Logger::error(logMessage.c_str());
    } else if (level == "WARNING") {
        Logger::warn(logMessage.c_str());
    } else if (level == "INFO") {
        Logger::info(logMessage.c_str());
    } else if (level == "DEBUG") {
        Logger::debug(logMessage.c_str());
    }
}

void FederatedLearningSystem::notifyError(const String& error) {
    updateSystemStatus(FLSystemStatus::ERROR);
    
    if (errorCallback_) {
        errorCallback_(error);
    }
    
    logEvent("Error: " + error, "ERROR");
}

void FederatedLearningSystem::updateStatistics() {
    systemStats_.uptime = getCurrentTimestamp() - systemStartTime_;
    systemStats_.memoryUsage = ESP.getFreeHeap();
    
    if (topologyManager_) {
        NetworkMetrics metrics = topologyManager_->getNetworkMetrics();
        systemStats_.networkNodes = metrics.nodeCount;
    }
}

uint32_t FederatedLearningSystem::getCurrentTimestamp() const {
    return millis();
}

String FederatedLearningSystem::generateSystemId() const {
    return "FL_" + String(ESP.getChipId(), HEX);
}

// ===========================
// GLOBAL UTILITY FUNCTIONS
// ===========================

bool initializeFederatedLearning(const FLSystemConfig& config) {
    if (g_federatedLearningSystem) {
        return true;
    }

    g_federatedLearningSystem = new FederatedLearningSystem();
    return g_federatedLearningSystem->init(config);
}

void cleanupFederatedLearning() {
    if (g_federatedLearningSystem) {
        delete g_federatedLearningSystem;
        g_federatedLearningSystem = nullptr;
    }
}

bool startFederatedLearning() {
    if (!g_federatedLearningSystem) {
        return false;
    }
    return g_federatedLearningSystem->start();
}

bool stopFederatedLearning() {
    if (!g_federatedLearningSystem) {
        return false;
    }
    return g_federatedLearningSystem->stop();
}

FLSystemStatus getFederatedLearningStatus() {
    if (!g_federatedLearningSystem) {
        return FLSystemStatus::UNINITIALIZED;
    }
    return g_federatedLearningSystem->getStatus();
}

bool addWildlifeObservation(const CameraFrame& frame, const AIResult& result) {
    if (!g_federatedLearningSystem) {
        return false;
    }

    // Convert AI result to training sample
    TrainingSample sample;
    sample.confidence = result.confidence;
    sample.timestamp = millis();
    sample.metadata = result.species;
    
    return g_federatedLearningSystem->addTrainingData(sample);
}

bool enableFederatedMode(bool enable) {
    if (!g_federatedLearningSystem) {
        return false;
    }
    return g_federatedLearningSystem->enableFederatedLearning(enable);
}

String getFederatedLearningReport() {
    if (!g_federatedLearningSystem) {
        return "Federated learning not initialized";
    }
    
    return g_federatedLearningSystem->generateSystemReport();
}