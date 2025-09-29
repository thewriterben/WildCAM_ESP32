/**
 * @file model_aggregator.cpp
 * @brief Model Aggregator for Wildlife Federated Learning
 * 
 * Handles aggregation of wildlife model updates from multiple devices
 * with wildlife-specific optimization and validation.
 */

#include "model_aggregator.h"
#include "../utils/logger.h"
#include <algorithm>
#include <numeric>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

ModelAggregator::ModelAggregator()
    : initialized_(false)
    , aggregationInProgress_(false)
    , totalAggregations_(0)
    , successfulAggregations_(0)
{
    config_ = AggregationConfig();
    stats_ = AggregationStats();
}

ModelAggregator::~ModelAggregator() {
    cleanup();
}

bool ModelAggregator::init(const AggregationConfig& config) {
    if (initialized_) {
        Logger::warn("[ModelAggregator] Already initialized");
        return true;
    }

    Logger::info("[ModelAggregator] Initializing model aggregator");

    config_ = config;
    
    // Initialize data structures
    pendingUpdates_.clear();
    validationHistory_.clear();
    
    // Reset statistics
    stats_ = AggregationStats();
    
    initialized_ = true;
    Logger::info("[ModelAggregator] Model aggregator initialized successfully");
    return true;
}

void ModelAggregator::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[ModelAggregator] Cleaning up model aggregator");
    
    // Clear pending updates
    pendingUpdates_.clear();
    validationHistory_.clear();
    
    initialized_ = false;
    aggregationInProgress_ = false;
}

// ===========================
// AGGREGATION OPERATIONS
// ===========================

bool ModelAggregator::addModelUpdate(const ModelUpdate& update) {
    if (!initialized_) {
        Logger::error("[ModelAggregator] Not initialized");
        return false;
    }

    if (aggregationInProgress_) {
        Logger::warn("[ModelAggregator] Aggregation in progress, cannot add update");
        return false;
    }

    // Validate model update
    if (!validateModelUpdate(update)) {
        Logger::warn("[ModelAggregator] Invalid model update rejected");
        return false;
    }

    // Check for duplicates
    for (const auto& existing : pendingUpdates_) {
        if (existing.deviceId == update.deviceId && 
            existing.timestamp == update.timestamp) {
            Logger::debug("[ModelAggregator] Duplicate update ignored");
            return false;
        }
    }

    pendingUpdates_.push_back(update);
    Logger::debug("[ModelAggregator] Added model update from device: %s", 
                 update.deviceId.c_str());

    return true;
}

AggregationResult ModelAggregator::aggregateUpdates(WildlifeModelType modelType) {
    AggregationResult result;
    result.success = false;
    
    if (!initialized_) {
        result.errorMessage = "Aggregator not initialized";
        return result;
    }

    if (aggregationInProgress_) {
        result.errorMessage = "Aggregation already in progress";
        return result;
    }

    Logger::info("[ModelAggregator] Starting aggregation for model type: %d", (int)modelType);
    
    aggregationInProgress_ = true;
    uint32_t startTime = millis();

    // Filter updates by model type
    std::vector<ModelUpdate> relevantUpdates;
    for (const auto& update : pendingUpdates_) {
        if (update.modelType == modelType) {
            relevantUpdates.push_back(update);
        }
    }

    if (relevantUpdates.empty()) {
        result.errorMessage = "No updates available for aggregation";
        aggregationInProgress_ = false;
        return result;
    }

    if (relevantUpdates.size() < config_.minParticipants) {
        result.errorMessage = "Insufficient participants for aggregation";
        aggregationInProgress_ = false;
        return result;
    }

    Logger::info("[ModelAggregator] Aggregating %d updates", relevantUpdates.size());

    // Perform aggregation based on configured algorithm
    bool success = false;
    ModelUpdate aggregatedUpdate;

    switch (config_.aggregationAlgorithm) {
        case AggregationAlgorithm::FEDERATED_AVERAGING:
            success = performFederatedAveraging(relevantUpdates, aggregatedUpdate);
            break;
            
        case AggregationAlgorithm::FEDERATED_PROX:
            success = performFederatedProx(relevantUpdates, aggregatedUpdate);
            break;
            
        case AggregationAlgorithm::ROBUST_AGGREGATION:
            success = performRobustAggregation(relevantUpdates, aggregatedUpdate);
            break;
            
        default:
            success = performFederatedAveraging(relevantUpdates, aggregatedUpdate);
            break;
    }

    if (success) {
        // Validate aggregated model
        if (validateAggregatedModel(aggregatedUpdate, relevantUpdates)) {
            result.success = true;
            result.aggregatedUpdate = aggregatedUpdate;
            result.participantCount = relevantUpdates.size();
            result.aggregationTimeMs = millis() - startTime;
            
            // Update statistics
            successfulAggregations_++;
            stats_.totalModelUpdates += relevantUpdates.size();
            stats_.averageParticipants = (stats_.averageParticipants + relevantUpdates.size()) / 2.0f;
            
            Logger::info("[ModelAggregator] Aggregation successful with %d participants", 
                        relevantUpdates.size());
        } else {
            result.errorMessage = "Aggregated model validation failed";
            Logger::error("[ModelAggregator] Aggregated model validation failed");
        }
    } else {
        result.errorMessage = "Aggregation algorithm failed";
        Logger::error("[ModelAggregator] Aggregation algorithm failed");
    }

    // Clear processed updates
    auto newEnd = std::remove_if(pendingUpdates_.begin(), pendingUpdates_.end(),
        [modelType](const ModelUpdate& update) {
            return update.modelType == modelType;
        });
    pendingUpdates_.erase(newEnd, pendingUpdates_.end());

    totalAggregations_++;
    aggregationInProgress_ = false;

    return result;
}

// ===========================
// AGGREGATION ALGORITHMS
// ===========================

bool ModelAggregator::performFederatedAveraging(const std::vector<ModelUpdate>& updates,
                                               ModelUpdate& result) {
    if (updates.empty()) {
        return false;
    }

    Logger::debug("[ModelAggregator] Performing federated averaging");

    // Initialize result with first update structure
    result = updates[0];
    result.deviceId = "AGGREGATED";
    result.timestamp = millis();
    result.updateType = ModelUpdateType::FULL_MODEL;

    // Calculate total data points for weighting
    uint32_t totalDataPoints = 0;
    for (const auto& update : updates) {
        totalDataPoints += update.dataPoints;
    }

    if (totalDataPoints == 0) {
        Logger::error("[ModelAggregator] No data points in updates");
        return false;
    }

    // Ensure all updates have the same weight size
    size_t weightSize = updates[0].weights.size();
    for (const auto& update : updates) {
        if (update.weights.size() != weightSize) {
            Logger::error("[ModelAggregator] Inconsistent weight sizes");
            return false;
        }
    }

    // Initialize aggregated weights
    result.weights.resize(weightSize, 0.0f);

    // Perform weighted averaging
    for (const auto& update : updates) {
        float weight = (float)update.dataPoints / totalDataPoints;
        
        for (size_t i = 0; i < weightSize; i++) {
            result.weights[i] += weight * update.weights[i];
        }
    }

    // Update metadata
    result.trainingRounds = 1;
    result.dataPoints = totalDataPoints;

    Logger::debug("[ModelAggregator] Federated averaging completed");
    return true;
}

bool ModelAggregator::performFederatedProx(const std::vector<ModelUpdate>& updates,
                                          ModelUpdate& result) {
    Logger::debug("[ModelAggregator] Performing FedProx aggregation");

    // FedProx is similar to FedAvg but with proximal term
    // For simplicity, implement as weighted average with stability weighting
    
    if (!performFederatedAveraging(updates, result)) {
        return false;
    }

    // Apply proximal regularization (simplified)
    float proxWeight = config_.proximalWeight;
    for (auto& weight : result.weights) {
        weight *= (1.0f - proxWeight);
    }

    Logger::debug("[ModelAggregator] FedProx aggregation completed");
    return true;
}

bool ModelAggregator::performRobustAggregation(const std::vector<ModelUpdate>& updates,
                                              ModelUpdate& result) {
    Logger::debug("[ModelAggregator] Performing robust aggregation");

    if (updates.empty()) {
        return false;
    }

    // Filter out outliers
    std::vector<ModelUpdate> filteredUpdates;
    for (const auto& update : updates) {
        if (!isOutlierUpdate(update, updates)) {
            filteredUpdates.push_back(update);
        }
    }

    if (filteredUpdates.empty()) {
        Logger::warn("[ModelAggregator] All updates filtered as outliers, using original");
        filteredUpdates = updates;
    }

    // Use trimmed mean for aggregation
    size_t weightSize = filteredUpdates[0].weights.size();
    result = filteredUpdates[0];
    result.deviceId = "ROBUST_AGGREGATED";
    result.timestamp = millis();
    result.weights.resize(weightSize, 0.0f);

    // Calculate trimmed mean for each weight
    for (size_t i = 0; i < weightSize; i++) {
        std::vector<float> weightValues;
        for (const auto& update : filteredUpdates) {
            if (i < update.weights.size()) {
                weightValues.push_back(update.weights[i]);
            }
        }

        if (!weightValues.empty()) {
            result.weights[i] = calculateTrimmedMean(weightValues, config_.trimRatio);
        }
    }

    Logger::debug("[ModelAggregator] Robust aggregation completed with %d/%d updates", 
                 filteredUpdates.size(), updates.size());
    return true;
}

// ===========================
// VALIDATION
// ===========================

bool ModelAggregator::validateModelUpdate(const ModelUpdate& update) {
    // Basic validation checks
    if (update.weights.empty()) {
        Logger::debug("[ModelAggregator] Update has no weights");
        return false;
    }

    if (update.deviceId.isEmpty()) {
        Logger::debug("[ModelAggregator] Update missing device ID");
        return false;
    }

    if (update.dataPoints == 0) {
        Logger::debug("[ModelAggregator] Update has no data points");
        return false;
    }

    // Check for reasonable weight values
    for (float weight : update.weights) {
        if (std::isnan(weight) || std::isinf(weight)) {
            Logger::debug("[ModelAggregator] Update contains invalid weights");
            return false;
        }
        
        if (std::fabs(weight) > config_.maxWeightValue) {
            Logger::debug("[ModelAggregator] Update weight exceeds maximum");
            return false;
        }
    }

    return true;
}

bool ModelAggregator::validateAggregatedModel(const ModelUpdate& aggregated,
                                             const std::vector<ModelUpdate>& sources) {
    // Validate aggregated model
    if (!validateModelUpdate(aggregated)) {
        return false;
    }

    // Check that aggregated model is reasonable compared to sources
    if (sources.empty()) {
        return false;
    }

    // Calculate average magnitude of source updates
    float sourceAvgMagnitude = 0.0f;
    for (const auto& source : sources) {
        float magnitude = 0.0f;
        for (float weight : source.weights) {
            magnitude += weight * weight;
        }
        sourceAvgMagnitude += sqrt(magnitude);
    }
    sourceAvgMagnitude /= sources.size();

    // Calculate magnitude of aggregated model
    float aggregatedMagnitude = 0.0f;
    for (float weight : aggregated.weights) {
        aggregatedMagnitude += weight * weight;
    }
    aggregatedMagnitude = sqrt(aggregatedMagnitude);

    // Check if aggregated magnitude is reasonable
    float magnitudeRatio = aggregatedMagnitude / (sourceAvgMagnitude + 1e-8f);
    if (magnitudeRatio > config_.maxMagnitudeRatio || magnitudeRatio < 1.0f / config_.maxMagnitudeRatio) {
        Logger::warn("[ModelAggregator] Aggregated model magnitude suspicious: %.3f", magnitudeRatio);
        return false;
    }

    return true;
}

// ===========================
// OUTLIER DETECTION
// ===========================

bool ModelAggregator::isOutlierUpdate(const ModelUpdate& update, 
                                     const std::vector<ModelUpdate>& allUpdates) {
    if (allUpdates.size() < 3) {
        return false; // Need at least 3 updates for outlier detection
    }

    // Calculate update's distance from median
    std::vector<float> distances;
    
    for (const auto& other : allUpdates) {
        if (other.deviceId == update.deviceId) {
            continue;
        }
        
        float distance = calculateUpdateDistance(update, other);
        distances.push_back(distance);
    }

    if (distances.empty()) {
        return false;
    }

    // Calculate median distance
    std::sort(distances.begin(), distances.end());
    float medianDistance = distances[distances.size() / 2];

    // Calculate distance to this update
    float avgDistance = std::accumulate(distances.begin(), distances.end(), 0.0f) / distances.size();

    // Consider outlier if distance is significantly higher than average
    float threshold = avgDistance * config_.outlierThreshold;
    
    for (const auto& other : allUpdates) {
        if (other.deviceId != update.deviceId) {
            float distance = calculateUpdateDistance(update, other);
            if (distance > threshold) {
                Logger::debug("[ModelAggregator] Update from %s flagged as outlier (distance: %.3f)", 
                             update.deviceId.c_str(), distance);
                return true;
            }
        }
    }

    return false;
}

float ModelAggregator::calculateUpdateDistance(const ModelUpdate& update1, 
                                             const ModelUpdate& update2) {
    if (update1.weights.size() != update2.weights.size()) {
        return std::numeric_limits<float>::max();
    }

    float distance = 0.0f;
    for (size_t i = 0; i < update1.weights.size(); i++) {
        float diff = update1.weights[i] - update2.weights[i];
        distance += diff * diff;
    }

    return sqrt(distance);
}

// ===========================
// UTILITY METHODS
// ===========================

float ModelAggregator::calculateTrimmedMean(std::vector<float> values, float trimRatio) {
    if (values.empty()) {
        return 0.0f;
    }

    std::sort(values.begin(), values.end());
    
    size_t trimCount = (size_t)(values.size() * trimRatio / 2);
    if (trimCount >= values.size() / 2) {
        trimCount = 0; // Don't trim if it would remove too much
    }

    size_t startIdx = trimCount;
    size_t endIdx = values.size() - trimCount;
    
    if (startIdx >= endIdx) {
        return values[values.size() / 2]; // Return median
    }

    float sum = 0.0f;
    for (size_t i = startIdx; i < endIdx; i++) {
        sum += values[i];
    }

    return sum / (endIdx - startIdx);
}

size_t ModelAggregator::getPendingUpdateCount() const {
    return pendingUpdates_.size();
}

size_t ModelAggregator::getPendingUpdateCount(WildlifeModelType modelType) const {
    size_t count = 0;
    for (const auto& update : pendingUpdates_) {
        if (update.modelType == modelType) {
            count++;
        }
    }
    return count;
}

void ModelAggregator::clearPendingUpdates() {
    pendingUpdates_.clear();
    Logger::debug("[ModelAggregator] Cleared all pending updates");
}

void ModelAggregator::clearPendingUpdates(WildlifeModelType modelType) {
    auto newEnd = std::remove_if(pendingUpdates_.begin(), pendingUpdates_.end(),
        [modelType](const ModelUpdate& update) {
            return update.modelType == modelType;
        });
    
    size_t removedCount = pendingUpdates_.end() - newEnd;
    pendingUpdates_.erase(newEnd, pendingUpdates_.end());
    
    Logger::debug("[ModelAggregator] Cleared %d pending updates for model type %d", 
                 removedCount, (int)modelType);
}

AggregationStats ModelAggregator::getStatistics() const {
    AggregationStats stats = stats_;
    stats.totalAggregations = totalAggregations_;
    stats.successfulAggregations = successfulAggregations_;
    stats.pendingUpdates = pendingUpdates_.size();
    
    if (totalAggregations_ > 0) {
        stats.successRate = (float)successfulAggregations_ / totalAggregations_;
    }
    
    return stats;
}

void ModelAggregator::resetStatistics() {
    stats_ = AggregationStats();
    totalAggregations_ = 0;
    successfulAggregations_ = 0;
    Logger::debug("[ModelAggregator] Statistics reset");
}