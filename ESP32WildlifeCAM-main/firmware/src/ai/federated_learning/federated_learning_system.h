/**
 * @file federated_learning_system.h
 * @brief Main Federated Learning System Integration
 * 
 * High-level interface for federated learning system that integrates
 * all components and provides easy-to-use API for the main application.
 */

#ifndef FEDERATED_LEARNING_SYSTEM_H
#define FEDERATED_LEARNING_SYSTEM_H

#include "federated_common.h"
#include "local_training_module.h"
#include "privacy_preserving_aggregation.h"
#include "federated_learning_coordinator.h"
#include "model_update_protocol.h"
#include "network_topology_manager.h"
#include "../power/power_manager.h"

/**
 * Federated Learning System Status
 */
enum class FLSystemStatus {
    UNINITIALIZED,
    INITIALIZING,
    READY,
    ACTIVE,
    PAUSED,
    ERROR,
    SHUTDOWN
};

/**
 * System Configuration
 */
struct FLSystemConfig {
    FederatedLearningConfig federatedConfig;
    TopologyConfig topologyConfig;
    NetworkConfig networkConfig;
    AggregationConfig aggregationConfig;
    
    // System-level settings
    bool enablePowerManagement;
    bool enableAutomaticMode;
    uint32_t systemUpdateInterval;
    bool enableWebInterface;
    bool enableLogging;
    String logLevel;
    
    FLSystemConfig() : enablePowerManagement(true), enableAutomaticMode(true),
                      systemUpdateInterval(60000), enableWebInterface(true),
                      enableLogging(true), logLevel("INFO") {}
};

/**
 * System Statistics
 */
struct FLSystemStats {
    FLSystemStatus status;
    uint32_t uptime;
    uint32_t totalRounds;
    uint32_t successfulRounds;
    float averageAccuracyImprovement;
    uint32_t modelsContributed;
    float totalPrivacyBudgetUsed;
    uint32_t networkNodes;
    float batteryLevel;
    uint32_t memoryUsage;
    
    FLSystemStats() : status(FLSystemStatus::UNINITIALIZED), uptime(0),
                     totalRounds(0), successfulRounds(0),
                     averageAccuracyImprovement(0.0f), modelsContributed(0),
                     totalPrivacyBudgetUsed(0.0f), networkNodes(0),
                     batteryLevel(1.0f), memoryUsage(0) {}
};

/**
 * Federated Learning System
 * 
 * Main system class that coordinates all federated learning components:
 * - Provides high-level API for federated learning operations
 * - Manages component lifecycle and coordination
 * - Handles power management integration
 * - Provides web interface for monitoring and control
 * - Handles automatic mode operations
 */
class FederatedLearningSystem {
public:
    FederatedLearningSystem();
    ~FederatedLearningSystem();
    
    // System lifecycle
    bool init(const FLSystemConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // System control
    bool start();
    bool stop();
    bool pause();
    bool resume();
    FLSystemStatus getStatus() const { return status_; }
    
    // Configuration management
    void setSystemConfig(const FLSystemConfig& config);
    FLSystemConfig getSystemConfig() const { return config_; }
    bool updateConfig(const String& configJson);
    String getConfigAsJson() const;
    
    // Federated learning operations
    bool enableFederatedLearning(bool enable);
    bool participateInRound(const String& roundId = "");
    bool initiateTraining(WildlifeModelType modelType, const std::vector<TrainingSample>& samples);
    bool stopCurrentOperation();
    
    // Model management
    bool loadModel(WildlifeModelType modelType, const String& modelPath);
    bool saveModel(WildlifeModelType modelType, const String& modelPath);
    ModelInfo getModelInfo(WildlifeModelType modelType) const;
    std::vector<ModelInfo> getAllModelInfo() const;
    
    // Data management
    bool addTrainingData(const TrainingSample& sample);
    bool addTrainingData(const std::vector<TrainingSample>& samples);
    void clearTrainingData();
    size_t getTrainingDataCount() const;
    
    // Network management
    bool discoverNetwork();
    bool joinNetwork(const String& networkId);
    bool leaveNetwork();
    std::vector<String> getAvailableNetworks() const;
    NetworkMetrics getNetworkStatus() const;
    
    // Privacy management
    void setPrivacyLevel(PrivacyLevel level);
    PrivacyLevel getPrivacyLevel() const;
    float getRemainingPrivacyBudget() const;
    bool validatePrivacySettings() const;
    
    // Power management integration
    void setPowerConstraints(float maxPower, uint32_t maxMemory);
    bool isPowerOptimized() const;
    void handlePowerEvent(const String& event, float batteryLevel);
    
    // Monitoring and statistics
    FLSystemStats getSystemStats() const;
    FederatedLearningMetrics getFederatedMetrics() const;
    NetworkMetrics getNetworkMetrics() const;
    void resetStatistics();
    
    // Web interface support
    bool startWebInterface(uint16_t port = 80);
    void stopWebInterface();
    String getWebInterfaceURL() const;
    bool handleWebRequest(const String& endpoint, const String& method, 
                         const String& body, String& response);
    
    // Event callbacks
    typedef std::function<void(FLSystemStatus, FLSystemStatus)> StatusChangeCallback;
    typedef std::function<void(const String&, const TrainingResult&)> TrainingCompleteCallback;
    typedef std::function<void(const String&, float)> AccuracyImprovementCallback;
    typedef std::function<void(const String&)> ErrorCallback;
    typedef std::function<void(const String&, const String&)> EventCallback;
    
    void setStatusChangeCallback(StatusChangeCallback callback) { statusChangeCallback_ = callback; }
    void setTrainingCompleteCallback(TrainingCompleteCallback callback) { trainingCompleteCallback_ = callback; }
    void setAccuracyImprovementCallback(AccuracyImprovementCallback callback) { accuracyImprovementCallback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    void setEventCallback(EventCallback callback) { eventCallback_ = callback; }
    
    // Automatic mode operations
    bool enableAutomaticMode(bool enable);
    bool isAutomaticModeEnabled() const { return automaticMode_; }
    void updateAutomaticMode();
    
    // Utility methods
    bool exportSystemLogs(const String& filepath);
    bool importConfiguration(const String& filepath);
    bool exportConfiguration(const String& filepath);
    String generateSystemReport() const;
    bool validateSystemIntegrity() const;

private:
    // Configuration
    FLSystemConfig config_;
    bool initialized_;
    FLSystemStatus status_;
    bool automaticMode_;
    
    // Component instances
    std::unique_ptr<LocalTrainingModule> localTraining_;
    std::unique_ptr<PrivacyPreservingAggregation> privacyAggregation_;
    std::unique_ptr<FederatedLearningCoordinator> coordinator_;
    std::unique_ptr<ModelUpdateProtocol> updateProtocol_;
    std::unique_ptr<NetworkTopologyManager> topologyManager_;
    
    // Power management integration
    PowerManager* powerManager_;
    bool powerOptimized_;
    float maxPowerConsumption_;
    uint32_t maxMemoryUsage_;
    
    // System state
    uint32_t systemStartTime_;
    String currentRoundId_;
    String currentNetworkId_;
    std::vector<TrainingSample> pendingTrainingData_;
    
    // Statistics
    FLSystemStats systemStats_;
    
    // Web interface
    bool webInterfaceEnabled_;
    uint16_t webInterfacePort_;
    
    // Callbacks
    StatusChangeCallback statusChangeCallback_;
    TrainingCompleteCallback trainingCompleteCallback_;
    AccuracyImprovementCallback accuracyImprovementCallback_;
    ErrorCallback errorCallback_;
    EventCallback eventCallback_;
    
    // Private methods - System management
    bool initializeComponents();
    void cleanupComponents();
    bool validateConfiguration() const;
    void updateSystemStatus(FLSystemStatus newStatus);
    
    // Private methods - Component coordination
    void setupComponentCallbacks();
    void handleTrainingComplete(const TrainingResult& result);
    void handleRoundComplete(const String& roundId, const RoundStatistics& stats);
    void handleNetworkChange();
    void handlePrivacyBudgetExhausted();
    
    // Private methods - Automatic mode
    void checkForNewRounds();
    void evaluateTrainingOpportunity();
    void optimizeSystemPerformance();
    void handlePowerConstraints();
    
    // Private methods - Web interface
    String generateDashboardHTML() const;
    String generateConfigHTML() const;
    String generateStatsJSON() const;
    bool handleConfigUpdate(const String& configJson);
    
    // Private methods - Utilities
    void logEvent(const String& event, const String& level = "INFO");
    void notifyError(const String& error);
    void updateStatistics();
    uint32_t getCurrentTimestamp() const;
    String generateSystemId() const;
};

// Global instance
extern FederatedLearningSystem* g_federatedLearningSystem;

// High-level utility functions
bool initializeFederatedLearning(const FLSystemConfig& config);
void cleanupFederatedLearning();
bool startFederatedLearning();
bool stopFederatedLearning();
FLSystemStatus getFederatedLearningStatus();
bool addWildlifeObservation(const CameraFrame& frame, const AIResult& result);
bool enableFederatedMode(bool enable);
String getFederatedLearningReport();

#endif // FEDERATED_LEARNING_SYSTEM_H