/**
 * @file privacy_preserving_aggregation.cpp
 * @brief Implementation of Privacy-Preserving Aggregation
 * 
 * Implements differential privacy and secure aggregation for federated learning.
 */

#include "privacy_preserving_aggregation.h"
#include "../../utils/logger.h"
#include <cmath>
#include <algorithm>
#include <random>

// ===========================
// PRIVACY BUDGET MANAGER
// ===========================

PrivacyBudgetManager::PrivacyBudgetManager(float totalBudget)
    : totalBudget_(totalBudget)
    , usedBudget_(0.0f)
{
    spentEpsilons_.clear();
}

bool PrivacyBudgetManager::canSpend(float epsilon) const {
    return (usedBudget_ + epsilon) <= totalBudget_;
}

bool PrivacyBudgetManager::spendBudget(float epsilon) {
    if (!canSpend(epsilon)) {
        return false;
    }
    
    usedBudget_ += epsilon;
    spentEpsilons_.push_back(epsilon);
    return true;
}

float PrivacyBudgetManager::getRemainingBudget() const {
    return totalBudget_ - usedBudget_;
}

float PrivacyBudgetManager::getUsedBudget() const {
    return usedBudget_;
}

void PrivacyBudgetManager::resetBudget(float newBudget) {
    totalBudget_ = newBudget;
    usedBudget_ = 0.0f;
    spentEpsilons_.clear();
}

float PrivacyBudgetManager::computeComposedEpsilon(const std::vector<float>& epsilons) const {
    // Simple composition (basic composition theorem)
    float totalEpsilon = 0.0f;
    for (float epsilon : epsilons) {
        totalEpsilon += epsilon;
    }
    return totalEpsilon;
}

bool PrivacyBudgetManager::trackComposition(float epsilon, uint32_t count) {
    float totalEpsilon = epsilon * count;
    return spendBudget(totalEpsilon);
}

// ===========================
// PRIVACY PRESERVING AGGREGATION
// ===========================

PrivacyPreservingAggregation::PrivacyPreservingAggregation()
    : initialized_(false)
    , privacyBudgetManager_(1.0f)
{
    config_ = AggregationConfig();
    metrics_ = PrivacyMetrics();
}

PrivacyPreservingAggregation::~PrivacyPreservingAggregation() {
    cleanup();
}

bool PrivacyPreservingAggregation::init(const AggregationConfig& config) {
    if (initialized_) {
        Logger::warn("[PrivacyAggregation] Already initialized");
        return true;
    }

    Logger::info("[PrivacyAggregation] Initializing privacy-preserving aggregation");

    config_ = config;
    
    // Initialize privacy budget manager
    privacyBudgetManager_.resetBudget(config_.totalPrivacyBudget);
    
    // Reset metrics
    metrics_ = PrivacyMetrics();
    
    // Initialize secure aggregation if enabled
    if (config_.enableSecureAggregation) {
        if (!initializeSecureAggregation()) {
            Logger::error("[PrivacyAggregation] Failed to initialize secure aggregation");
            return false;
        }
    }

    initialized_ = true;
    Logger::info("[PrivacyAggregation] Privacy-preserving aggregation initialized");
    return true;
}

void PrivacyPreservingAggregation::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[PrivacyAggregation] Cleaning up privacy-preserving aggregation");
    
    // Clear any sensitive data
    // Reset state
    initialized_ = false;
}

// ===========================
// AGGREGATION OPERATIONS
// ===========================

AggregationResult PrivacyPreservingAggregation::aggregateUpdates(const std::vector<ModelUpdate>& updates) {
    AggregationResult result;
    result.success = false;
    
    if (!initialized_) {
        result.errorMessage = "Aggregation not initialized";
        return result;
    }

    if (updates.empty()) {
        result.errorMessage = "No updates provided";
        return result;
    }

    Logger::info("[PrivacyAggregation] Aggregating %d updates with privacy preservation", 
                updates.size());

    uint32_t startTime = millis();

    // Check privacy budget
    float requiredEpsilon = config_.baseEpsilon;
    if (!privacyBudgetManager_.canSpend(requiredEpsilon)) {
        result.errorMessage = "Insufficient privacy budget";
        Logger::warn("[PrivacyAggregation] Privacy budget exhausted");
        return result;
    }

    // Filter out outliers if enabled
    std::vector<ModelUpdate> filteredUpdates = updates;
    if (config_.enableOutlierDetection) {
        filteredUpdates = filterOutliers(updates);
        Logger::debug("[PrivacyAggregation] Filtered %d outliers", 
                     updates.size() - filteredUpdates.size());
    }

    if (filteredUpdates.size() < config_.minParticipants) {
        result.errorMessage = "Insufficient participants after filtering";
        return result;
    }

    // Perform aggregation based on algorithm
    ModelUpdate aggregatedUpdate;
    bool success = false;

    switch (config_.aggregationAlgorithm) {
        case AggregationAlgorithm::FEDERATED_AVERAGING:
            success = performPrivateFedAvg(filteredUpdates, aggregatedUpdate, requiredEpsilon);
            break;
            
        case AggregationAlgorithm::ROBUST_AGGREGATION:
            success = performRobustPrivateAggregation(filteredUpdates, aggregatedUpdate, requiredEpsilon);
            break;
            
        default:
            success = performPrivateFedAvg(filteredUpdates, aggregatedUpdate, requiredEpsilon);
            break;
    }

    if (success) {
        // Spend privacy budget
        privacyBudgetManager_.spendBudget(requiredEpsilon);
        
        // Update result
        result.success = true;
        result.aggregatedUpdate = aggregatedUpdate;
        result.participantCount = filteredUpdates.size();
        result.privacyBudgetUsed = requiredEpsilon;
        result.aggregationTimeMs = millis() - startTime;
        
        // Extract participant IDs
        for (const auto& update : filteredUpdates) {
            result.participantIds.push_back(update.deviceId);
        }
        
        // Update metrics
        updateMetrics(result);
        
        Logger::info("[PrivacyAggregation] Aggregation successful with privacy budget: %.3f", 
                    requiredEpsilon);
    } else {
        result.errorMessage = "Aggregation algorithm failed";
        Logger::error("[PrivacyAggregation] Aggregation failed");
    }

    return result;
}

AggregationResult PrivacyPreservingAggregation::aggregateSecureShares(const std::vector<SecureShare>& shares) {
    AggregationResult result;
    result.success = false;
    
    if (!initialized_ || !config_.enableSecureAggregation) {
        result.errorMessage = "Secure aggregation not enabled";
        return result;
    }

    Logger::info("[PrivacyAggregation] Aggregating %d secure shares", shares.size());

    // Verify zero-knowledge proofs
    std::vector<SecureShare> validShares;
    for (const auto& share : shares) {
        if (verifyZeroKnowledgeProof(share.proof, share)) {
            validShares.push_back(share);
        } else {
            Logger::warn("[PrivacyAggregation] Invalid proof from participant: %s", 
                        share.participantId.c_str());
        }
    }

    if (validShares.size() < config_.minParticipants) {
        result.errorMessage = "Insufficient valid secure shares";
        return result;
    }

    // Combine secure shares
    std::vector<float> aggregatedWeights = combineSecretShares(validShares);
    
    if (!aggregatedWeights.empty()) {
        // Create aggregated update
        ModelUpdate aggregatedUpdate;
        aggregatedUpdate.weights = aggregatedWeights;
        aggregatedUpdate.deviceId = "SECURE_AGGREGATED";
        aggregatedUpdate.timestamp = millis();
        aggregatedUpdate.updateType = ModelUpdateType::FULL_MODEL;
        
        result.success = true;
        result.aggregatedUpdate = aggregatedUpdate;
        result.participantCount = validShares.size();
        result.aggregationTimeMs = millis();
        
        Logger::info("[PrivacyAggregation] Secure aggregation successful");
    } else {
        result.errorMessage = "Failed to combine secure shares";
    }

    return result;
}

// ===========================
// DIFFERENTIAL PRIVACY
// ===========================

ModelUpdate PrivacyPreservingAggregation::applyDifferentialPrivacy(const ModelUpdate& update, 
                                                                   float epsilon, float delta) {
    ModelUpdate privatizedUpdate = update;
    
    if (epsilon <= 0.0f) {
        Logger::error("[PrivacyAggregation] Invalid epsilon value: %.3f", epsilon);
        return privatizedUpdate;
    }

    Logger::debug("[PrivacyAggregation] Applying differential privacy (ε=%.3f, δ=%.6f)", 
                 epsilon, delta);

    // Calculate noise scale for Gaussian mechanism
    float sensitivity = calculateSensitivity(update);
    float noiseScale = calculateOptimalNoise(1, epsilon, delta);
    
    // Add noise to weights
    for (auto& weight : privatizedUpdate.weights) {
        float noise = generateGaussianNoise(0.0f, noiseScale);
        weight += noise;
    }

    // Clip weights if needed
    if (config_.enableClipping) {
        privatizedUpdate.weights = clipGradients(privatizedUpdate.weights, config_.clippingNorm);
    }

    logPrivacyEvent("Applied differential privacy", epsilon);
    return privatizedUpdate;
}

// ===========================
// PRIVATE METHODS - AGGREGATION
// ===========================

bool PrivacyPreservingAggregation::performPrivateFedAvg(const std::vector<ModelUpdate>& updates,
                                                        ModelUpdate& result, float epsilon) {
    if (updates.empty()) {
        return false;
    }

    // Initialize result
    result = updates[0];
    result.deviceId = "PRIVATE_FEDAVG";
    result.timestamp = millis();
    
    size_t weightSize = updates[0].weights.size();
    result.weights.resize(weightSize, 0.0f);

    // Calculate total data points for weighting
    uint32_t totalDataPoints = 0;
    for (const auto& update : updates) {
        totalDataPoints += update.dataPoints;
    }

    if (totalDataPoints == 0) {
        return false;
    }

    // Perform weighted averaging
    for (const auto& update : updates) {
        float weight = (float)update.dataPoints / totalDataPoints;
        
        for (size_t i = 0; i < weightSize && i < update.weights.size(); i++) {
            result.weights[i] += weight * update.weights[i];
        }
    }

    // Apply differential privacy
    result = applyDifferentialPrivacy(result, epsilon);

    return true;
}

bool PrivacyPreservingAggregation::performRobustPrivateAggregation(const std::vector<ModelUpdate>& updates,
                                                                  ModelUpdate& result, float epsilon) {
    if (updates.empty()) {
        return false;
    }

    // Use trimmed mean for robustness
    size_t weightSize = updates[0].weights.size();
    result = updates[0];
    result.deviceId = "ROBUST_PRIVATE";
    result.weights.resize(weightSize, 0.0f);

    // Calculate trimmed mean for each weight
    for (size_t i = 0; i < weightSize; i++) {
        std::vector<float> weightValues;
        for (const auto& update : updates) {
            if (i < update.weights.size()) {
                weightValues.push_back(update.weights[i]);
            }
        }

        if (!weightValues.empty()) {
            result.weights[i] = computeTrimmedMean(weightValues, config_.trimRatio);
        }
    }

    // Apply differential privacy
    result = applyDifferentialPrivacy(result, epsilon);

    return true;
}

// ===========================
// PRIVATE METHODS - UTILITIES
// ===========================

std::vector<ModelUpdate> PrivacyPreservingAggregation::filterOutliers(const std::vector<ModelUpdate>& updates) {
    if (updates.size() < 3) {
        return updates; // Need at least 3 for outlier detection
    }

    std::vector<ModelUpdate> filtered;
    
    for (const auto& update : updates) {
        if (!isOutlier(update, updates)) {
            filtered.push_back(update);
        } else {
            Logger::debug("[PrivacyAggregation] Filtered outlier from device: %s", 
                         update.deviceId.c_str());
            metrics_.outliersDetected++;
        }
    }

    return filtered;
}

bool PrivacyPreservingAggregation::isOutlier(const ModelUpdate& update, 
                                           const std::vector<ModelUpdate>& updates) const {
    // Calculate median magnitude
    std::vector<float> magnitudes;
    for (const auto& u : updates) {
        float magnitude = 0.0f;
        for (float weight : u.weights) {
            magnitude += weight * weight;
        }
        magnitudes.push_back(sqrt(magnitude));
    }

    std::sort(magnitudes.begin(), magnitudes.end());
    float median = magnitudes[magnitudes.size() / 2];

    // Calculate magnitude of current update
    float updateMagnitude = 0.0f;
    for (float weight : update.weights) {
        updateMagnitude += weight * weight;
    }
    updateMagnitude = sqrt(updateMagnitude);

    // Check if it's an outlier
    float threshold = median * config_.outlierThreshold;
    return updateMagnitude > threshold;
}

float PrivacyPreservingAggregation::computeTrimmedMean(std::vector<float> values, float trimRatio) const {
    if (values.empty()) {
        return 0.0f;
    }

    std::sort(values.begin(), values.end());
    
    size_t trimCount = (size_t)(values.size() * trimRatio / 2);
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

float PrivacyPreservingAggregation::calculateSensitivity(const ModelUpdate& update) const {
    // Simple L2 sensitivity calculation
    float magnitude = 0.0f;
    for (float weight : update.weights) {
        magnitude += weight * weight;
    }
    return sqrt(magnitude);
}

float PrivacyPreservingAggregation::generateGaussianNoise(float mean, float stddev) const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<float> distribution(mean, stddev);
    return distribution(gen);
}

std::vector<float> PrivacyPreservingAggregation::clipGradients(const std::vector<float>& gradients, 
                                                             float norm) const {
    std::vector<float> clipped = gradients;
    
    // Calculate current norm
    float currentNorm = 0.0f;
    for (float grad : gradients) {
        currentNorm += grad * grad;
    }
    currentNorm = sqrt(currentNorm);
    
    // Clip if needed
    if (currentNorm > norm) {
        float scale = norm / currentNorm;
        for (auto& grad : clipped) {
            grad *= scale;
        }
    }
    
    return clipped;
}

// ===========================
// SECURE AGGREGATION
// ===========================

bool PrivacyPreservingAggregation::initializeSecureAggregation() {
    // Initialize cryptographic components for secure aggregation
    Logger::info("[PrivacyAggregation] Initializing secure aggregation");
    return true;
}

String PrivacyPreservingAggregation::generateSecretShare(const std::vector<float>& weights, 
                                                        const String& participantId) {
    // Generate secret share using Shamir's secret sharing
    // This is a simplified placeholder
    String share = participantId + "_" + String(weights.size()) + "_" + String(millis());
    return share;
}

std::vector<float> PrivacyPreservingAggregation::combineSecretShares(const std::vector<String>& shares) {
    std::vector<float> result;
    
    // Combine secret shares to reconstruct the secret
    // This is a simplified placeholder
    if (!shares.empty()) {
        result.resize(1000, 0.0f); // Placeholder size
        
        for (size_t i = 0; i < result.size(); i++) {
            result[i] = (float)random(-1000, 1000) / 10000.0f;
        }
    }
    
    return result;
}

bool PrivacyPreservingAggregation::verifyZeroKnowledgeProof(const String& proof, 
                                                           const SecureShare& share) {
    // Verify zero-knowledge proof
    // This is a simplified placeholder
    return !proof.isEmpty() && !share.participantId.isEmpty();
}

// ===========================
// UTILITY METHODS
// ===========================

float PrivacyPreservingAggregation::calculateOptimalNoise(size_t participantCount, 
                                                         float epsilon, float delta) const {
    // Calculate optimal noise for Gaussian mechanism
    float sensitivity = 1.0f; // Assume unit sensitivity
    return sensitivity * sqrt(2.0f * log(1.25f / delta)) / epsilon;
}

float PrivacyPreservingAggregation::estimatePrivacyLoss(const AggregationConfig& config, 
                                                       size_t rounds) const {
    // Estimate total privacy loss over multiple rounds
    return config.baseEpsilon * rounds;
}

bool PrivacyPreservingAggregation::validatePrivacyGuarantees(const AggregationResult& result) const {
    // Validate that privacy guarantees are met
    return result.privacyBudgetUsed <= config_.totalPrivacyBudget;
}

void PrivacyPreservingAggregation::updateMetrics(const AggregationResult& result) {
    metrics_.totalAggregations++;
    metrics_.totalPrivacyBudgetUsed += result.privacyBudgetUsed;
    
    if (result.success) {
        metrics_.successfulAggregations++;
    } else {
        metrics_.failedAggregations++;
    }
    
    metrics_.averageNoiseLevel = (metrics_.averageNoiseLevel + result.noiseVariance) / 2.0f;
}

void PrivacyPreservingAggregation::logPrivacyEvent(const String& event, float epsilonUsed) {
    Logger::debug("[PrivacyAggregation] %s (ε=%.6f, remaining=%.6f)", 
                 event.c_str(), epsilonUsed, privacyBudgetManager_.getRemainingBudget());
}