/**
 * @file ai_federated_integration.h
 * @brief Federated Learning Integration for AI Wildlife System
 * 
 * Extends the existing AI Wildlife System to support federated learning
 * capabilities while maintaining backward compatibility.
 */

#ifndef AI_FEDERATED_INTEGRATION_H
#define AI_FEDERATED_INTEGRATION_H

#include "ai_wildlife_system.h"
#include "federated_learning/federated_learning_system.h"
#include "federated_learning/local_training_module.h"

/**
 * Enhanced Wildlife Analysis Result with Federated Learning Support
 */
struct FederatedWildlifeResult : public WildlifeAnalysisResult {
    // Federated learning specific fields
    bool contributedToFederation;    // Whether result was contributed to FL
    float federationConfidence;      // Confidence for federation contribution
    String federationRoundId;        // Current federation round ID
    bool privacyProtected;          // Whether privacy protection was applied
    PrivacyLevel privacyLevel;      // Level of privacy protection used
    
    FederatedWildlifeResult() : WildlifeAnalysisResult(),
                               contributedToFederation(false),
                               federationConfidence(0.0f),
                               privacyProtected(false),
                               privacyLevel(PrivacyLevel::STANDARD) {}
};

/**
 * AI Federated Configuration
 */
struct AIFederatedConfig {
    // Enable/disable federated learning
    bool enableFederatedLearning;
    
    // Contribution thresholds
    float minConfidenceForContribution;
    float minAccuracyForContribution;
    uint32_t maxContributionsPerDay;
    
    // Privacy settings
    PrivacyLevel defaultPrivacyLevel;
    bool autoApplyPrivacy;
    bool requireExpertValidation;
    
    // Model update settings
    bool enableAutomaticModelUpdates;
    uint32_t modelUpdateCheckInterval;
    bool requireManualApproval;
    
    // Training settings
    TrainingMode defaultTrainingMode;
    uint32_t maxTrainingTimeMs;
    bool enableContinuousLearning;
    
    AIFederatedConfig() : enableFederatedLearning(false),
                         minConfidenceForContribution(0.9f),
                         minAccuracyForContribution(0.85f),
                         maxContributionsPerDay(50),
                         defaultPrivacyLevel(PrivacyLevel::STANDARD),
                         autoApplyPrivacy(true),
                         requireExpertValidation(false),
                         enableAutomaticModelUpdates(true),
                         modelUpdateCheckInterval(3600000), // 1 hour
                         requireManualApproval(false),
                         defaultTrainingMode(TrainingMode::INCREMENTAL),
                         maxTrainingTimeMs(300000), // 5 minutes
                         enableContinuousLearning(true) {}
};

/**
 * AI Federated Integration Manager
 * 
 * Integrates federated learning capabilities with the existing AI Wildlife System:
 * - Seamless integration with existing AI processing pipeline
 * - Automatic contribution of high-confidence results to federated learning
 * - Privacy-preserving data sharing
 * - Continuous model improvement
 * - Backward compatibility with existing system
 */
class AIFederatedIntegration {
public:
    AIFederatedIntegration();
    ~AIFederatedIntegration();
    
    // Initialization and lifecycle
    bool init(const AIFederatedConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Integration with AI Wildlife System
    bool attachToAISystem(AIWildlifeSystem* aiSystem);
    void detachFromAISystem();
    bool isAttached() const { return aiSystemAttached_; }
    
    // Enhanced analysis with federated learning
    FederatedWildlifeResult processFrameWithFederation(const CameraFrame& frame);
    FederatedWildlifeResult enhanceAnalysisResult(const WildlifeAnalysisResult& result, 
                                                 const CameraFrame& frame);
    
    // Federated learning operations
    bool contributeToContinuousLearning(const FederatedWildlifeResult& result, 
                                       const CameraFrame& frame);
    bool participateInFederatedTraining();
    bool checkForModelUpdates();
    bool applyModelUpdate(const ModelUpdate& update);
    
    // Configuration management
    void setFederatedConfig(const AIFederatedConfig& config);
    AIFederatedConfig getFederatedConfig() const { return config_; }
    void enableFederatedLearning(bool enable);
    void setPrivacyLevel(PrivacyLevel level);
    
    // Expert validation interface
    bool submitForExpertValidation(const FederatedWildlifeResult& result, 
                                  const CameraFrame& frame);
    bool processExpertFeedback(const String& resultId, bool validated, 
                              const AIResult& correctedResult);
    
    // Model management
    bool downloadLatestModels();
    bool validateModelCompatibility(const ModelUpdate& update);
    std::vector<ModelInfo> getAvailableModelUpdates();
    bool rollbackToLastKnownGood();
    
    // Privacy and security
    PrivacyProtectionResult applyPrivacyProtection(FederatedWildlifeResult& result);
    bool validateDataPrivacy(const TrainingSample& sample);
    void anonymizeResult(FederatedWildlifeResult& result);
    
    // Monitoring and statistics
    struct FederatedAIStats {
        uint32_t totalAnalyses;
        uint32_t contributedToFederation;
        uint32_t federatedRoundsParticipated;
        float averageConfidenceImprovement;
        uint32_t modelUpdatesReceived;
        uint32_t expertValidationsRequested;
        float currentModelAccuracy;
        uint32_t privacyProtectedSamples;
        
        FederatedAIStats() : totalAnalyses(0), contributedToFederation(0),
                            federatedRoundsParticipated(0), averageConfidenceImprovement(0.0f),
                            modelUpdatesReceived(0), expertValidationsRequested(0),
                            currentModelAccuracy(0.0f), privacyProtectedSamples(0) {}
    };
    
    FederatedAIStats getFederatedStats() const { return federatedStats_; }
    void resetFederatedStats();
    
    // Event callbacks
    typedef std::function<void(const FederatedWildlifeResult&)> ResultContributedCallback;
    typedef std::function<void(const ModelUpdate&)> ModelUpdateCallback;
    typedef std::function<void(float)> AccuracyImprovementCallback;
    typedef std::function<void(const String&)> ExpertValidationRequestCallback;
    
    void setResultContributedCallback(ResultContributedCallback callback) { 
        resultContributedCallback_ = callback; 
    }
    void setModelUpdateCallback(ModelUpdateCallback callback) { 
        modelUpdateCallback_ = callback; 
    }
    void setAccuracyImprovementCallback(AccuracyImprovementCallback callback) { 
        accuracyImprovementCallback_ = callback; 
    }
    void setExpertValidationRequestCallback(ExpertValidationRequestCallback callback) { 
        expertValidationRequestCallback_ = callback; 
    }
    
    // Utility methods
    bool isResultSuitableForFederation(const WildlifeAnalysisResult& result) const;
    float calculateContributionValue(const WildlifeAnalysisResult& result) const;
    bool shouldRequestExpertValidation(const WildlifeAnalysisResult& result) const;
    String generateResultFingerprint(const WildlifeAnalysisResult& result) const;

private:
    // Configuration
    AIFederatedConfig config_;
    bool initialized_;
    bool aiSystemAttached_;
    
    // Component integration
    AIWildlifeSystem* aiSystem_;
    std::unique_ptr<FederatedLearningSystem> federatedSystem_;
    
    // Statistics and monitoring
    FederatedAIStats federatedStats_;
    uint32_t lastModelUpdateCheck_;
    
    // Expert validation queue
    std::map<String, std::pair<FederatedWildlifeResult, CameraFrame>> pendingValidations_;
    
    // Callbacks
    ResultContributedCallback resultContributedCallback_;
    ModelUpdateCallback modelUpdateCallback_;
    AccuracyImprovementCallback accuracyImprovementCallback_;
    ExpertValidationRequestCallback expertValidationRequestCallback_;
    
    // Private methods - Integration
    void setupAISystemCallbacks();
    void handleAIResult(const WildlifeAnalysisResult& result, const CameraFrame& frame);
    bool shouldContributeResult(const WildlifeAnalysisResult& result) const;
    
    // Private methods - Federated learning
    TrainingSample createTrainingSample(const FederatedWildlifeResult& result, 
                                       const CameraFrame& frame);
    bool evaluateModelUpdate(const ModelUpdate& update);
    void notifyModelUpdateAvailable(const ModelUpdate& update);
    
    // Private methods - Privacy
    bool isDataSensitive(const FederatedWildlifeResult& result) const;
    void applyLocationPrivacy(FederatedWildlifeResult& result);
    void applyTemporalPrivacy(FederatedWildlifeResult& result);
    
    // Private methods - Utilities
    void updateFederatedStats(const FederatedWildlifeResult& result);
    void logFederatedEvent(const String& event, const String& details = "");
    String generateValidationId() const;
    bool validateResultConsistency(const WildlifeAnalysisResult& original, 
                                  const AIResult& corrected) const;
};

// Global instance
extern AIFederatedIntegration* g_aiFederatedIntegration;

// Integration utility functions
bool initializeAIFederatedIntegration(const AIFederatedConfig& config);
void cleanupAIFederatedIntegration();
bool enableAIFederatedLearning(bool enable);
FederatedWildlifeResult processWildlifeFrameWithFederation(const CameraFrame& frame);
bool contributeWildlifeObservation(const WildlifeAnalysisResult& result, const CameraFrame& frame);
FederatedAIStats getCurrentFederatedAIStats();

#endif // AI_FEDERATED_INTEGRATION_H