/**
 * @file model_aggregator.h
 * @brief Model Aggregator for Wildlife Federated Learning
 * 
 * Handles secure aggregation of wildlife model updates from multiple
 * ESP32 devices with privacy preservation and outlier detection.
 */

#ifndef MODEL_AGGREGATOR_H
#define MODEL_AGGREGATOR_H

#include "federated_learning/federated_common.h"
#include <vector>
#include <map>

/**
 * Aggregation Algorithm Types
 */
enum class AggregationAlgorithm {
    FEDERATED_AVERAGING,    // Standard FedAvg
    FEDERATED_PROX,        // FedProx with proximal term
    ROBUST_AGGREGATION,    // Trimmed mean with outlier detection
    SECURE_AGGREGATION     // Multi-party computation based
};

/**
 * Aggregation Configuration
 */
struct AggregationConfig {
    AggregationAlgorithm aggregationAlgorithm;
    uint32_t minParticipants;
    uint32_t maxParticipants;
    float outlierThreshold;
    float trimRatio;                // For trimmed mean (0.0-0.5)
    float proximalWeight;           // For FedProx
    float maxWeightValue;           // Maximum allowed weight value
    float maxMagnitudeRatio;        // Maximum ratio for model validation
    bool enableSecureAggregation;
    bool enableOutlierDetection;
    
    AggregationConfig() : aggregationAlgorithm(AggregationAlgorithm::FEDERATED_AVERAGING),
                         minParticipants(2), maxParticipants(50),
                         outlierThreshold(2.0f), trimRatio(0.2f),
                         proximalWeight(0.1f), maxWeightValue(10.0f),
                         maxMagnitudeRatio(3.0f), enableSecureAggregation(false),
                         enableOutlierDetection(true) {}
};

/**
 * Aggregation Result
 */
struct AggregationResult {
    bool success;
    String errorMessage;
    ModelUpdate aggregatedUpdate;
    uint32_t participantCount;
    uint32_t aggregationTimeMs;
    float qualityScore;
    std::vector<String> participantIds;
    
    AggregationResult() : success(false), participantCount(0),
                         aggregationTimeMs(0), qualityScore(0.0f) {}
};

/**
 * Aggregation Statistics
 */
struct AggregationStats {
    uint32_t totalAggregations;
    uint32_t successfulAggregations;
    float successRate;
    uint32_t totalModelUpdates;
    uint32_t outliersDetected;
    float averageParticipants;
    uint32_t averageAggregationTimeMs;
    uint32_t pendingUpdates;
    
    AggregationStats() : totalAggregations(0), successfulAggregations(0),
                        successRate(0.0f), totalModelUpdates(0),
                        outliersDetected(0), averageParticipants(0.0f),
                        averageAggregationTimeMs(0), pendingUpdates(0) {}
};

/**
 * Model Aggregator for Wildlife Federated Learning
 * 
 * Handles aggregation of model updates from multiple wildlife cameras:
 * - Multiple aggregation algorithms (FedAvg, FedProx, Robust)
 * - Outlier detection and filtering
 * - Privacy-preserving aggregation
 * - Wildlife-specific model validation
 * - Quality assessment and monitoring
 */
class ModelAggregator {
public:
    ModelAggregator();
    ~ModelAggregator();
    
    // Initialization
    bool init(const AggregationConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setConfig(const AggregationConfig& config) { config_ = config; }
    AggregationConfig getConfig() const { return config_; }
    
    // Model update management
    bool addModelUpdate(const ModelUpdate& update);
    AggregationResult aggregateUpdates(WildlifeModelType modelType);
    
    // Update queue management
    size_t getPendingUpdateCount() const;
    size_t getPendingUpdateCount(WildlifeModelType modelType) const;
    void clearPendingUpdates();
    void clearPendingUpdates(WildlifeModelType modelType);
    
    // Statistics and monitoring
    AggregationStats getStatistics() const;
    void resetStatistics();
    
    // Quality assessment
    float assessModelQuality(const ModelUpdate& update);
    bool isUpdateTrusted(const ModelUpdate& update, const String& deviceId);
    
    // Callbacks
    typedef std::function<void(const AggregationResult&)> AggregationCompleteCallback;
    typedef std::function<void(const ModelUpdate&, const String&)> OutlierDetectedCallback;
    typedef std::function<void(const String&)> ErrorCallback;
    
    void setAggregationCompleteCallback(AggregationCompleteCallback callback) { 
        aggregationCompleteCallback_ = callback; 
    }
    void setOutlierDetectedCallback(OutlierDetectedCallback callback) { 
        outlierDetectedCallback_ = callback; 
    }
    void setErrorCallback(ErrorCallback callback) { 
        errorCallback_ = callback; 
    }

private:
    // Configuration and state
    AggregationConfig config_;
    bool initialized_;
    bool aggregationInProgress_;
    
    // Update storage
    std::vector<ModelUpdate> pendingUpdates_;
    std::map<String, std::vector<ModelUpdate>> validationHistory_;
    
    // Statistics
    AggregationStats stats_;
    uint32_t totalAggregations_;
    uint32_t successfulAggregations_;
    
    // Callbacks
    AggregationCompleteCallback aggregationCompleteCallback_;
    OutlierDetectedCallback outlierDetectedCallback_;
    ErrorCallback errorCallback_;
    
    // Private methods - Aggregation algorithms
    bool performFederatedAveraging(const std::vector<ModelUpdate>& updates,
                                  ModelUpdate& result);
    bool performFederatedProx(const std::vector<ModelUpdate>& updates,
                             ModelUpdate& result);
    bool performRobustAggregation(const std::vector<ModelUpdate>& updates,
                                 ModelUpdate& result);
    bool performSecureAggregation(const std::vector<ModelUpdate>& updates,
                                 ModelUpdate& result);
    
    // Private methods - Validation
    bool validateModelUpdate(const ModelUpdate& update);
    bool validateAggregatedModel(const ModelUpdate& aggregated,
                                const std::vector<ModelUpdate>& sources);
    
    // Private methods - Outlier detection
    bool isOutlierUpdate(const ModelUpdate& update, 
                        const std::vector<ModelUpdate>& allUpdates);
    float calculateUpdateDistance(const ModelUpdate& update1, 
                                 const ModelUpdate& update2);
    
    // Private methods - Utilities
    float calculateTrimmedMean(std::vector<float> values, float trimRatio);
    void updateStatistics(const AggregationResult& result);
    void logAggregationEvent(const String& event, const String& level = "INFO");
};

#endif // MODEL_AGGREGATOR_H