/**
 * @file privacy_preserving_aggregation.h
 * @brief Privacy-Preserving Aggregation for Federated Learning
 * 
 * Implements differential privacy, secure aggregation, and other
 * privacy-preserving techniques for federated learning model updates.
 */

#ifndef PRIVACY_PRESERVING_AGGREGATION_H
#define PRIVACY_PRESERVING_AGGREGATION_H

#include "federated_common.h"
#include "../../production/security/data_protector.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Aggregation Method
 */
enum class AggregationMethod {
    FEDERATED_AVERAGING,        // FedAvg algorithm
    FEDERATED_PROX,            // FedProx algorithm  
    SECURE_AGGREGATION,        // Secure multi-party computation
    DIFFERENTIAL_PRIVATE,      // Differential privacy aggregation
    ROBUST_AGGREGATION         // Robust aggregation with outlier detection
};

/**
 * Noise Generation Method
 */
enum class NoiseMethod {
    GAUSSIAN,                  // Gaussian noise
    LAPLACIAN,                 // Laplacian noise
    EXPONENTIAL,               // Exponential mechanism
    COMPOSITION               // Composition of noise methods
};

/**
 * Aggregation Configuration
 */
struct AggregationConfig {
    AggregationMethod method;
    PrivacyLevel privacyLevel;
    float privacyEpsilon;
    float privacyDelta;
    NoiseMethod noiseMethod;
    float clippingNorm;        // For gradient clipping
    bool enableSecureAggregation;
    bool enableRobustness;
    uint32_t minParticipants;
    uint32_t maxParticipants;
    
    AggregationConfig() : method(AggregationMethod::FEDERATED_AVERAGING),
                         privacyLevel(PrivacyLevel::STANDARD),
                         privacyEpsilon(1.0f), privacyDelta(1e-5f),
                         noiseMethod(NoiseMethod::GAUSSIAN),
                         clippingNorm(1.0f), enableSecureAggregation(true),
                         enableRobustness(true), minParticipants(3),
                         maxParticipants(100) {}
};

/**
 * Aggregation Result
 */
struct AggregationResult {
    bool success;
    String errorMessage;
    ModelUpdate aggregatedUpdate;
    uint32_t participantCount;
    float privacyBudgetUsed;
    float noiseVariance;
    std::vector<String> participantIds;
    uint32_t aggregationTimeMs;
    
    AggregationResult() : success(false), participantCount(0),
                         privacyBudgetUsed(0.0f), noiseVariance(0.0f),
                         aggregationTimeMs(0) {}
};

/**
 * Secure Share for Secure Aggregation
 */
struct SecureShare {
    String participantId;
    std::vector<float> encryptedWeights;
    String proof;              // Zero-knowledge proof
    uint32_t timestamp;
    
    SecureShare() : timestamp(0) {}
};

/**
 * Privacy Budget Manager
 */
class PrivacyBudgetManager {
public:
    PrivacyBudgetManager(float totalBudget = 1.0f);
    
    bool canSpend(float epsilon) const;
    bool spendBudget(float epsilon);
    float getRemainingBudget() const;
    float getUsedBudget() const;
    void resetBudget(float newBudget = 1.0f);
    
    // Advanced composition
    float computeComposedEpsilon(const std::vector<float>& epsilons) const;
    bool trackComposition(float epsilon, uint32_t count = 1);

private:
    float totalBudget_;
    float usedBudget_;
    std::vector<float> spentEpsilons_;
};

/**
 * Privacy-Preserving Aggregation Engine
 * 
 * Implements privacy-preserving aggregation methods for federated learning:
 * - Differential privacy mechanisms
 * - Secure multi-party computation
 * - Robust aggregation algorithms
 * - Privacy budget management
 */
class PrivacyPreservingAggregation {
public:
    PrivacyPreservingAggregation();
    ~PrivacyPreservingAggregation();
    
    // Initialization
    bool init(const AggregationConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setAggregationConfig(const AggregationConfig& config);
    AggregationConfig getAggregationConfig() const { return config_; }
    
    // Model update aggregation
    AggregationResult aggregateUpdates(const std::vector<ModelUpdate>& updates);
    AggregationResult aggregateSecureShares(const std::vector<SecureShare>& shares);
    
    // Differential privacy
    ModelUpdate applyDifferentialPrivacy(const ModelUpdate& update, float epsilon, float delta = 1e-5f);
    std::vector<float> addGaussianNoise(const std::vector<float>& weights, float sigma);
    std::vector<float> addLaplacianNoise(const std::vector<float>& weights, float scale);
    
    // Secure aggregation
    SecureShare createSecureShare(const ModelUpdate& update, const String& participantId);
    bool verifySecureShare(const SecureShare& share);
    ModelUpdate reconstructFromShares(const std::vector<SecureShare>& shares);
    
    // Robustness mechanisms
    std::vector<ModelUpdate> detectOutliers(const std::vector<ModelUpdate>& updates);
    ModelUpdate robustAverage(const std::vector<ModelUpdate>& updates);
    bool validateUpdateIntegrity(const ModelUpdate& update);
    
    // Privacy budget management
    PrivacyBudgetManager& getPrivacyBudgetManager() { return privacyBudgetManager_; }
    float getRemainingPrivacyBudget() const;
    bool trackPrivacyUsage(float epsilon);
    
    // Utility methods
    float calculateOptimalNoise(size_t participantCount, float epsilon, float delta) const;
    float estimatePrivacyLoss(const AggregationConfig& config, size_t rounds) const;
    bool validatePrivacyGuarantees(const AggregationResult& result) const;
    
    // Anonymous credential management
    bool generateAnonymousCredential(const String& deviceId, String& credential);
    bool verifyAnonymousCredential(const String& credential);
    void revokeAnonymousCredential(const String& credential);
    
    // Metrics and monitoring
    struct PrivacyMetrics {
        uint32_t totalAggregations;
        float totalPrivacyBudgetUsed;
        uint32_t successfulAggregations;
        uint32_t failedAggregations;
        float averageNoiseLevel;
        uint32_t outliersDetected;
        uint32_t securityViolations;
        
        PrivacyMetrics() : totalAggregations(0), totalPrivacyBudgetUsed(0.0f),
                          successfulAggregations(0), failedAggregations(0),
                          averageNoiseLevel(0.0f), outliersDetected(0),
                          securityViolations(0) {}
    };
    
    PrivacyMetrics getPrivacyMetrics() const { return metrics_; }
    void resetMetrics();

private:
    // Configuration
    AggregationConfig config_;
    bool initialized_;
    
    // Privacy budget management
    PrivacyBudgetManager privacyBudgetManager_;
    
    // Security components
    std::unique_ptr<DataProtector> dataProtector_;
    
    // Metrics
    PrivacyMetrics metrics_;
    
    // Anonymous credentials store
    std::map<String, bool> anonymousCredentials_;
    
    // Private methods - Differential Privacy
    float computeL2Sensitivity(const std::vector<ModelUpdate>& updates) const;
    float computeGaussianSigma(float epsilon, float delta, float sensitivity) const;
    float computeLaplacianScale(float epsilon, float sensitivity) const;
    std::vector<float> clipGradients(const std::vector<float>& gradients, float norm) const;
    
    // Private methods - Secure Aggregation
    bool initializeSecureAggregation();
    String generateSecretShare(const std::vector<float>& weights, const String& participantId);
    std::vector<float> combineSecretShares(const std::vector<String>& shares);
    bool verifyZeroKnowledgeProof(const String& proof, const SecureShare& share);
    
    // Private methods - Robustness
    float computeMedian(std::vector<float> values) const;
    float computeTrimmedMean(std::vector<float> values, float trimRatio) const;
    bool isOutlier(const ModelUpdate& update, const std::vector<ModelUpdate>& updates) const;
    float computeUpdateDistance(const ModelUpdate& update1, const ModelUpdate& update2) const;
    
    // Private methods - Federated Averaging
    ModelUpdate federatedAveraging(const std::vector<ModelUpdate>& updates);
    ModelUpdate federatedProx(const std::vector<ModelUpdate>& updates, float mu = 0.01f);
    std::vector<float> weightedAverage(const std::vector<std::vector<float>>& weights, 
                                      const std::vector<float>& sampleCounts);
    
    // Utility methods
    void updateMetrics(const AggregationResult& result);
    String generateAnonymousId();
    bool validateModelUpdateFormat(const ModelUpdate& update) const;
    void logPrivacyEvent(const String& event, float epsilonUsed = 0.0f);
};

// Global instance
extern PrivacyPreservingAggregation* g_privacyAggregation;

// Utility functions
bool initializePrivacyAggregation(const AggregationConfig& config);
void cleanupPrivacyAggregation();
AggregationResult aggregateModelUpdates(const std::vector<ModelUpdate>& updates);
bool validatePrivacyGuarantees(float epsilon, float delta, size_t participants);
PrivacyProtectionResult applyPrivacyProtection(ModelUpdate& update, PrivacyLevel level);

#endif // PRIVACY_PRESERVING_AGGREGATION_H