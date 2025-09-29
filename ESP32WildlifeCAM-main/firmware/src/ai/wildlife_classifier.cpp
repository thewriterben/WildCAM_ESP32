/**
 * @file wildlife_classifier.cpp
 * @brief Implementation of Wildlife Species Classification System
 * 
 * Advanced wildlife classification with federated learning integration.
 */

#include "wildlife_classifier.h"
#include "../utils/logger.h"
#include <cmath>
#include <algorithm>

// Wildlife species database
const std::vector<SpeciesInfo> WILDLIFE_SPECIES_DATABASE = {
    {WildlifeSpecies::DEER, "White-tailed Deer", "Odocoileus virginianus", false, 0b1111, 1.2f, {AnimalBehavior::FEEDING, AnimalBehavior::ALERT, AnimalBehavior::MOVING}},
    {WildlifeSpecies::BEAR, "Black Bear", "Ursus americanus", false, 0b1110, 1.8f, {AnimalBehavior::FEEDING, AnimalBehavior::MOVING, AnimalBehavior::AGGRESSIVE}},
    {WildlifeSpecies::WOLF, "Gray Wolf", "Canis lupus", true, 0b1111, 1.5f, {AnimalBehavior::MOVING, AnimalBehavior::TERRITORIAL, AnimalBehavior::SOCIAL}},
    {WildlifeSpecies::FOX, "Red Fox", "Vulpes vulpes", true, 0b1111, 0.6f, {AnimalBehavior::MOVING, AnimalBehavior::ALERT, AnimalBehavior::FEEDING}},
    {WildlifeSpecies::RABBIT, "Cottontail Rabbit", "Sylvilagus floridanus", false, 0b1111, 0.3f, {AnimalBehavior::FEEDING, AnimalBehavior::ALERT, AnimalBehavior::MOVING}},
    {WildlifeSpecies::RACCOON, "Raccoon", "Procyon lotor", true, 0b1111, 0.7f, {AnimalBehavior::FEEDING, AnimalBehavior::MOVING, AnimalBehavior::TERRITORIAL}},
    {WildlifeSpecies::EAGLE, "Bald Eagle", "Haliaeetus leucocephalus", false, 0b1111, 0.9f, {AnimalBehavior::MOVING, AnimalBehavior::TERRITORIAL, AnimalBehavior::NESTING}},
    {WildlifeSpecies::OWL, "Great Horned Owl", "Bubo virginianus", true, 0b1111, 0.5f, {AnimalBehavior::ALERT, AnimalBehavior::TERRITORIAL, AnimalBehavior::NESTING}}
};

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

WildlifeClassifier::WildlifeClassifier()
    : initialized_(false)
    , speciesModel_(nullptr)
    , behaviorModel_(nullptr)
    , environmentModel_(nullptr)
    , inputBuffer_(nullptr)
    , outputBuffer_(nullptr)
    , bufferSize_(0)
    , maxHistorySize_(100)
    , contributionsThisHour_(0)
    , lastContributionReset_(0)
{
    config_ = WildlifeClassifierConfig();
    stats_ = WildlifeClassifierStats();
    currentEnvironment_ = EnvironmentalContext();
}

WildlifeClassifier::~WildlifeClassifier() {
    cleanup();
}

bool WildlifeClassifier::init(const WildlifeClassifierConfig& config) {
    if (initialized_) {
        Logger::warn("[WildlifeClassifier] Already initialized");
        return true;
    }

    Logger::info("[WildlifeClassifier] Initializing wildlife classifier");

    config_ = config;

    // Initialize processing buffers
    bufferSize_ = 224 * 224 * 3; // Typical input size for wildlife models
    inputBuffer_ = (uint8_t*)malloc(bufferSize_);
    outputBuffer_ = (float*)malloc(1000 * sizeof(float)); // Max output classes

    if (!inputBuffer_ || !outputBuffer_) {
        Logger::error("[WildlifeClassifier] Failed to allocate processing buffers");
        cleanup();
        return false;
    }

    // Initialize models
    if (!initializeModels()) {
        Logger::error("[WildlifeClassifier] Failed to initialize models");
        cleanup();
        return false;
    }

    // Initialize statistics
    stats_ = WildlifeClassifierStats();
    recentDetections_.clear();
    recentDetections_.reserve(maxHistorySize_);

    // Initialize environmental weights
    environmentalWeights_.clear();

    initialized_ = true;
    Logger::info("[WildlifeClassifier] Wildlife classifier initialized successfully");
    return true;
}

void WildlifeClassifier::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[WildlifeClassifier] Cleaning up wildlife classifier");

    // Cleanup models
    cleanupModels();

    // Free buffers
    if (inputBuffer_) {
        free(inputBuffer_);
        inputBuffer_ = nullptr;
    }

    if (outputBuffer_) {
        free(outputBuffer_);
        outputBuffer_ = nullptr;
    }

    // Clear data
    recentDetections_.clear();
    environmentalWeights_.clear();

    initialized_ = false;
}

// ===========================
// MAIN CLASSIFICATION INTERFACE
// ===========================

std::vector<WildlifeDetection> WildlifeClassifier::classifyImage(const CameraFrame& frame,
                                                                const EnvironmentalContext& environment) {
    if (!initialized_) {
        Logger::error("[WildlifeClassifier] Classifier not initialized");
        return {};
    }

    uint32_t startTime = getCurrentTimeMs();
    Logger::debug("[WildlifeClassifier] Starting image classification");

    // Update environmental context
    currentEnvironment_ = environment;

    // Preprocess image
    if (!preprocessImage(frame, inputBuffer_)) {
        Logger::error("[WildlifeClassifier] Image preprocessing failed");
        return {};
    }

    // Run species inference
    std::vector<float> speciesOutputs = runSpeciesInference(inputBuffer_);
    if (speciesOutputs.empty()) {
        Logger::error("[WildlifeClassifier] Species inference failed");
        return {};
    }

    // Apply environmental adaptation
    updateSpeciesLikelihood(speciesOutputs, environment);

    // Post-process detections
    std::vector<WildlifeDetection> detections = postProcessDetections(speciesOutputs, environment);

    // Filter by confidence threshold
    auto it = std::remove_if(detections.begin(), detections.end(),
        [this](const WildlifeDetection& det) {
            return det.overallConfidence < config_.confidenceThreshold;
        });
    detections.erase(it, detections.end());

    // Apply non-maximum suppression
    if (detections.size() > 1) {
        applyNonMaxSuppression(detections);
    }

    // Limit to max detections
    if (detections.size() > (size_t)config_.maxDetections) {
        detections.resize(config_.maxDetections);
    }

    // Analyze behavior for each detection
    if (config_.enableBehaviorAnalysis) {
        for (auto& detection : detections) {
            detection.behavior = analyzeBehavior({detection}, environment);
            
            // Run behavior-specific inference if needed
            if (behaviorModel_ && detection.behavior == AnimalBehavior::UNKNOWN) {
                std::vector<float> behaviorOutputs = runBehaviorInference(inputBuffer_, detection.species);
                // Process behavior outputs (simplified)
                if (!behaviorOutputs.empty()) {
                    auto maxIt = std::max_element(behaviorOutputs.begin(), behaviorOutputs.end());
                    int behaviorIndex = std::distance(behaviorOutputs.begin(), maxIt);
                    detection.behavior = (AnimalBehavior)(behaviorIndex + 1);
                    detection.behaviorConfidence = *maxIt;
                }
            }
        }
    }

    // Update statistics
    for (const auto& detection : detections) {
        updateStatistics(detection);
        
        // Add to recent detections
        if (recentDetections_.size() >= maxHistorySize_) {
            recentDetections_.erase(recentDetections_.begin());
        }
        recentDetections_.push_back(detection);

        // Trigger callback
        if (detectionCallback_) {
            detectionCallback_(detection);
        }

        // Consider contributing to federated learning
        if (config_.contributeToFederatedLearning && shouldContributeToFL(detection)) {
            contributeDetection(detection, frame);
        }

        // Log detection
        logDetection(detection);
    }

    uint32_t processingTime = getCurrentTimeMs() - startTime;
    stats_.averageProcessingTimeMs = (stats_.averageProcessingTimeMs + processingTime) / 2;

    Logger::info("[WildlifeClassifier] Classified image: %d detections in %dms", 
                detections.size(), processingTime);

    return detections;
}

WildlifeDetection WildlifeClassifier::classifyRegion(const CameraFrame& frame, 
                                                   float x, float y, float width, float height,
                                                   const EnvironmentalContext& environment) {
    // Create a cropped frame for the specific region
    // In a real implementation, this would extract the region from the frame
    
    std::vector<WildlifeDetection> detections = classifyImage(frame, environment);
    
    // Find detection that best matches the region
    WildlifeDetection bestMatch;
    float bestOverlap = 0.0f;
    
    for (const auto& detection : detections) {
        // Calculate overlap with specified region
        float overlapX = std::max(0.0f, std::min(x + width, detection.x + detection.width) - std::max(x, detection.x));
        float overlapY = std::max(0.0f, std::min(y + height, detection.y + detection.height) - std::max(y, detection.y));
        float overlapArea = overlapX * overlapY;
        float regionArea = width * height;
        float overlap = (regionArea > 0) ? overlapArea / regionArea : 0.0f;
        
        if (overlap > bestOverlap) {
            bestOverlap = overlap;
            bestMatch = detection;
        }
    }
    
    return bestMatch;
}

// ===========================
// BEHAVIOR ANALYSIS
// ===========================

AnimalBehavior WildlifeClassifier::analyzeBehavior(const std::vector<WildlifeDetection>& history,
                                                  const EnvironmentalContext& environment) {
    if (history.empty()) {
        return AnimalBehavior::UNKNOWN;
    }

    const WildlifeDetection& current = history.back();

    // Infer from environment
    AnimalBehavior envBehavior = inferBehaviorFromEnvironment(current.species, environment);
    if (envBehavior != AnimalBehavior::UNKNOWN) {
        return envBehavior;
    }

    // Infer from movement if we have history
    if (history.size() > 1) {
        AnimalBehavior movementBehavior = inferBehaviorFromMovement(history);
        if (movementBehavior != AnimalBehavior::UNKNOWN) {
            return movementBehavior;
        }
    }

    // Infer from pose
    AnimalBehavior poseBehavior = inferBehaviorFromPose(current);
    if (poseBehavior != AnimalBehavior::UNKNOWN) {
        return poseBehavior;
    }

    return AnimalBehavior::UNKNOWN;
}

std::vector<AnimalBehavior> WildlifeClassifier::getPossibleBehaviors(WildlifeSpecies species,
                                                                   const EnvironmentalContext& environment) {
    std::vector<AnimalBehavior> behaviors;

    // Find species in database
    for (const auto& speciesInfo : WILDLIFE_SPECIES_DATABASE) {
        if (speciesInfo.species == species) {
            behaviors = speciesInfo.commonBehaviors;
            break;
        }
    }

    // Filter by environment
    auto it = std::remove_if(behaviors.begin(), behaviors.end(),
        [&](AnimalBehavior behavior) {
            // Example filtering logic
            if (behavior == AnimalBehavior::FEEDING && environment.timeOfDay > 22) {
                return !isNocturnalSpecies(species);
            }
            return false;
        });
    behaviors.erase(it, behaviors.end());

    return behaviors;
}

// ===========================
// ENVIRONMENTAL ADAPTATION
// ===========================

void WildlifeClassifier::updateEnvironmentalContext(const EnvironmentalContext& environment) {
    currentEnvironment_ = environment;
    
    // Adapt weights based on environment
    if (config_.enableEnvironmentalAdaptation) {
        adaptToEnvironment(environment);
    }
}

void WildlifeClassifier::adaptToEnvironment(const EnvironmentalContext& environment) {
    // Update environmental weights for different species
    for (const auto& speciesInfo : WILDLIFE_SPECIES_DATABASE) {
        String key = getSpeciesName(speciesInfo.species);
        float weight = getEnvironmentalWeight(speciesInfo.species, environment);
        environmentalWeights_[key] = weight;
    }

    Logger::debug("[WildlifeClassifier] Adapted to environment: temp=%.1f, time=%d", 
                 environment.temperature, environment.timeOfDay);
}

bool WildlifeClassifier::calibrateForLocation(float latitude, float longitude) {
    Logger::info("[WildlifeClassifier] Calibrating for location: %.6f, %.6f", latitude, longitude);

    // Adjust species probabilities based on geographic location
    // This would typically involve loading regional species databases
    
    return true;
}

// ===========================
// FEDERATED LEARNING INTEGRATION
// ===========================

bool WildlifeClassifier::contributeDetection(const WildlifeDetection& detection, const CameraFrame& frame) {
    if (!config_.contributeToFederatedLearning) {
        return false;
    }

    // Check rate limiting
    uint32_t currentHour = getCurrentTimeMs() / 3600000;
    if (currentHour != lastContributionReset_) {
        contributionsThisHour_ = 0;
        lastContributionReset_ = currentHour;
    }

    if (contributionsThisHour_ >= config_.maxContributionsPerHour) {
        Logger::debug("[WildlifeClassifier] Contribution rate limit reached");
        return false;
    }

    // Check quality threshold
    if (detection.overallConfidence < config_.contributionThreshold) {
        return false;
    }

    Logger::info("[WildlifeClassifier] Contributing detection to federated learning: %s (%.3f)", 
                getSpeciesName(detection.species).c_str(), detection.overallConfidence);

    // Create training sample
    TrainingSample sample;
    sample.confidence = detection.overallConfidence;
    sample.timestamp = detection.timestamp;
    sample.metadata = getSpeciesName(detection.species);
    sample.environmentalContext = detection.environment;

    // Add to federated learning system
    // In a real implementation, this would interface with the federated learning system
    
    contributionsThisHour_++;
    stats_.contributionsToFL++;

    return true;
}

void WildlifeClassifier::updateFromFederatedModel(const ModelUpdate& update) {
    if (!initialized_) {
        return;
    }

    Logger::info("[WildlifeClassifier] Updating from federated model");

    // Apply model update
    // In a real implementation, this would update the actual model weights
    
    // Reset environmental adaptation to account for new model
    if (config_.enableEnvironmentalAdaptation) {
        adaptToEnvironment(currentEnvironment_);
    }

    Logger::info("[WildlifeClassifier] Model updated successfully");
}

bool WildlifeClassifier::validateDetection(const WildlifeDetection& detection, bool isCorrect) {
    if (isCorrect) {
        Logger::debug("[WildlifeClassifier] Detection validated as correct");
    } else {
        stats_.falsePositives++;
        Logger::debug("[WildlifeClassifier] Detection marked as false positive");
    }

    stats_.expertValidations++;
    return true;
}

// ===========================
// PRIVATE METHODS - MODEL OPERATIONS
// ===========================

bool WildlifeClassifier::initializeModels() {
    Logger::info("[WildlifeClassifier] Initializing AI models");

    // Load species classification model
    if (!loadSpeciesModel("/models/wildlife_classifier_v1.tflite")) {
        Logger::error("[WildlifeClassifier] Failed to load species model");
        return false;
    }

    // Load behavior model if enabled
    if (config_.enableBehaviorAnalysis) {
        if (!loadBehaviorModel("/models/behavior_model_v1.tflite")) {
            Logger::warn("[WildlifeClassifier] Failed to load behavior model");
            // Continue without behavior analysis
            config_.enableBehaviorAnalysis = false;
        }
    }

    Logger::info("[WildlifeClassifier] AI models initialized successfully");
    return true;
}

void WildlifeClassifier::cleanupModels() {
    // Cleanup model resources
    if (speciesModel_) {
        // Free species model
        speciesModel_ = nullptr;
    }

    if (behaviorModel_) {
        // Free behavior model
        behaviorModel_ = nullptr;
    }

    if (environmentModel_) {
        // Free environment model
        environmentModel_ = nullptr;
    }
}

bool WildlifeClassifier::preprocessImage(const CameraFrame& frame, uint8_t* buffer) {
    // In a real implementation, this would:
    // 1. Resize image to model input size (224x224)
    // 2. Normalize pixel values
    // 3. Convert color space if needed
    // 4. Apply any preprocessing specific to the model

    // Simulate preprocessing
    if (!buffer || bufferSize_ == 0) {
        return false;
    }

    // Fill with simulated preprocessed data
    for (size_t i = 0; i < bufferSize_; i++) {
        buffer[i] = (uint8_t)(i % 256);
    }

    return true;
}

std::vector<float> WildlifeClassifier::runSpeciesInference(const uint8_t* input) {
    std::vector<float> outputs;

    if (!input || !speciesModel_) {
        return outputs;
    }

    // In a real implementation, this would:
    // 1. Set input tensor data
    // 2. Run inference
    // 3. Extract output tensor data

    // Simulate species inference outputs (simplified)
    outputs.resize(static_cast<int>(WildlifeSpecies::VEHICLE) + 1);
    
    // Generate realistic-looking probabilities
    float sum = 0.0f;
    for (size_t i = 0; i < outputs.size(); i++) {
        outputs[i] = (float)random(0, 1000) / 10000.0f;
        sum += outputs[i];
    }

    // Normalize to probabilities
    for (auto& output : outputs) {
        output /= sum;
    }

    // Make one species more likely (simulate actual detection)
    int dominantSpecies = random(1, outputs.size());
    outputs[dominantSpecies] = 0.6f + (float)random(0, 300) / 1000.0f;

    // Renormalize
    sum = 0.0f;
    for (auto output : outputs) {
        sum += output;
    }
    for (auto& output : outputs) {
        output /= sum;
    }

    return outputs;
}

std::vector<float> WildlifeClassifier::runBehaviorInference(const uint8_t* input, WildlifeSpecies species) {
    std::vector<float> outputs;

    if (!input || !behaviorModel_) {
        return outputs;
    }

    // Simulate behavior inference
    outputs.resize(static_cast<int>(AnimalBehavior::SOCIAL) + 1);
    
    float sum = 0.0f;
    for (size_t i = 0; i < outputs.size(); i++) {
        outputs[i] = (float)random(0, 1000) / 10000.0f;
        sum += outputs[i];
    }

    // Normalize
    for (auto& output : outputs) {
        output /= sum;
    }

    return outputs;
}

// ===========================
// PRIVATE METHODS - POST-PROCESSING
// ===========================

std::vector<WildlifeDetection> WildlifeClassifier::postProcessDetections(const std::vector<float>& outputs,
                                                                        const EnvironmentalContext& environment) {
    std::vector<WildlifeDetection> detections;

    if (outputs.empty()) {
        return detections;
    }

    // Find species with highest confidence
    auto maxIt = std::max_element(outputs.begin(), outputs.end());
    int speciesIndex = std::distance(outputs.begin(), maxIt);
    float confidence = *maxIt;

    // Create detection if confidence is above threshold
    if (confidence > config_.confidenceThreshold) {
        WildlifeDetection detection;
        detection.species = (WildlifeSpecies)speciesIndex;
        detection.speciesConfidence = confidence;
        detection.overallConfidence = confidence;
        detection.timestamp = getCurrentTimeMs();
        detection.environment = environment;
        
        // Set bounding box (simplified - would come from object detection)
        detection.x = 0.3f;
        detection.y = 0.3f;
        detection.width = 0.4f;
        detection.height = 0.4f;

        detections.push_back(detection);
    }

    return detections;
}

void WildlifeClassifier::applyNonMaxSuppression(std::vector<WildlifeDetection>& detections) {
    if (detections.size() <= 1) {
        return;
    }

    // Sort by confidence
    std::sort(detections.begin(), detections.end(),
        [](const WildlifeDetection& a, const WildlifeDetection& b) {
            return a.overallConfidence > b.overallConfidence;
        });

    // Remove overlapping detections
    for (size_t i = 0; i < detections.size(); i++) {
        if (detections[i].overallConfidence == 0.0f) continue;

        for (size_t j = i + 1; j < detections.size(); j++) {
            if (detections[j].overallConfidence == 0.0f) continue;

            // Calculate IoU
            float intersectionArea = std::max(0.0f, 
                std::min(detections[i].x + detections[i].width, detections[j].x + detections[j].width) -
                std::max(detections[i].x, detections[j].x)) *
                std::max(0.0f,
                std::min(detections[i].y + detections[i].height, detections[j].y + detections[j].height) -
                std::max(detections[i].y, detections[j].y));

            float unionArea = detections[i].width * detections[i].height +
                            detections[j].width * detections[j].height - intersectionArea;

            float iou = (unionArea > 0) ? intersectionArea / unionArea : 0.0f;

            if (iou > config_.nmsThreshold) {
                detections[j].overallConfidence = 0.0f; // Mark for removal
            }
        }
    }

    // Remove marked detections
    auto it = std::remove_if(detections.begin(), detections.end(),
        [](const WildlifeDetection& det) {
            return det.overallConfidence == 0.0f;
        });
    detections.erase(it, detections.end());
}

// ===========================
// PRIVATE METHODS - ENVIRONMENTAL ADAPTATION
// ===========================

float WildlifeClassifier::getEnvironmentalWeight(WildlifeSpecies species, const EnvironmentalContext& environment) {
    float weight = 1.0f;

    // Time-based weighting
    bool isNocturnal = isNocturnalSpecies(species);
    bool isNight = (environment.timeOfDay < 6 || environment.timeOfDay > 20);
    
    if (isNocturnal && isNight) {
        weight *= 2.0f;
    } else if (!isNocturnal && !isNight) {
        weight *= 1.5f;
    } else {
        weight *= 0.5f;
    }

    // Seasonal weighting
    if (!isSeasonalSpecies(species, environment.season)) {
        weight *= 0.3f;
    }

    // Temperature weighting (simplified)
    if (environment.temperature < 0 && species == WildlifeSpecies::BEAR) {
        weight *= 0.1f; // Bears hibernate in winter
    }

    return weight;
}

void WildlifeClassifier::updateSpeciesLikelihood(std::vector<float>& probabilities,
                                               const EnvironmentalContext& environment) {
    if (!config_.enableEnvironmentalAdaptation) {
        return;
    }

    for (size_t i = 0; i < probabilities.size(); i++) {
        WildlifeSpecies species = (WildlifeSpecies)i;
        float weight = getEnvironmentalWeight(species, environment);
        probabilities[i] *= weight;
    }

    // Renormalize
    float sum = 0.0f;
    for (auto prob : probabilities) {
        sum += prob;
    }
    
    if (sum > 0.0f) {
        for (auto& prob : probabilities) {
            prob /= sum;
        }
    }
}

// ===========================
// PRIVATE METHODS - BEHAVIOR ANALYSIS
// ===========================

AnimalBehavior WildlifeClassifier::inferBehaviorFromMovement(const std::vector<WildlifeDetection>& history) {
    if (history.size() < 2) {
        return AnimalBehavior::UNKNOWN;
    }

    // Calculate movement between recent detections
    const auto& current = history.back();
    const auto& previous = history[history.size() - 2];

    float deltaX = current.x - previous.x;
    float deltaY = current.y - previous.y;
    float movement = sqrt(deltaX * deltaX + deltaY * deltaY);

    uint32_t deltaTime = current.timestamp - previous.timestamp;

    if (movement < 0.01f && deltaTime > 5000) { // Very little movement over 5 seconds
        return AnimalBehavior::RESTING;
    } else if (movement > 0.1f && deltaTime < 2000) { // Significant movement in short time
        return AnimalBehavior::MOVING;
    }

    return AnimalBehavior::UNKNOWN;
}

AnimalBehavior WildlifeClassifier::inferBehaviorFromPose(const WildlifeDetection& detection) {
    // In a real implementation, this would analyze pose keypoints
    // For now, return unknown
    return AnimalBehavior::UNKNOWN;
}

AnimalBehavior WildlifeClassifier::inferBehaviorFromEnvironment(WildlifeSpecies species,
                                                              const EnvironmentalContext& environment) {
    // Simple heuristics based on environment
    if (environment.timeOfDay >= 6 && environment.timeOfDay <= 8) {
        return AnimalBehavior::FEEDING; // Morning feeding time
    }
    
    if (environment.timeOfDay >= 18 && environment.timeOfDay <= 20) {
        return AnimalBehavior::FEEDING; // Evening feeding time
    }

    if (environment.isRaining) {
        return AnimalBehavior::RESTING; // Seeking shelter
    }

    return AnimalBehavior::UNKNOWN;
}

// ===========================
// UTILITY METHODS
// ===========================

String WildlifeClassifier::getSpeciesName(WildlifeSpecies species) const {
    return String(wildlifeSpeciesToString(species));
}

String WildlifeClassifier::getBehaviorName(AnimalBehavior behavior) const {
    return String(animalBehaviorToString(behavior));
}

bool WildlifeClassifier::isNocturnalSpecies(WildlifeSpecies species) const {
    for (const auto& speciesInfo : WILDLIFE_SPECIES_DATABASE) {
        if (speciesInfo.species == species) {
            return speciesInfo.isNocturnal;
        }
    }
    return false;
}

bool WildlifeClassifier::isSeasonalSpecies(WildlifeSpecies species, uint8_t season) const {
    for (const auto& speciesInfo : WILDLIFE_SPECIES_DATABASE) {
        if (speciesInfo.species == species) {
            return (speciesInfo.seasonalPresence & (1 << season)) != 0;
        }
    }
    return true; // Default to present if not in database
}

void WildlifeClassifier::updateStatistics(const WildlifeDetection& detection) {
    stats_.totalDetections++;
    stats_.averageConfidence = (stats_.averageConfidence + detection.overallConfidence) / 2.0f;
    
    // Update species frequency
    stats_.speciesFrequency[detection.species]++;
    
    // Update behavior frequency
    if (detection.behavior != AnimalBehavior::UNKNOWN) {
        stats_.behaviorFrequency[detection.behavior]++;
    }
}

bool WildlifeClassifier::shouldContributeToFL(const WildlifeDetection& detection) {
    return isDetectionHighQuality(detection) && 
           detection.overallConfidence >= config_.contributionThreshold;
}

bool WildlifeClassifier::isDetectionHighQuality(const WildlifeDetection& detection) {
    return detection.overallConfidence > 0.8f && 
           detection.species != WildlifeSpecies::UNKNOWN;
}

void WildlifeClassifier::logDetection(const WildlifeDetection& detection, const String& level) {
    String logMessage = "[WildlifeClassifier] Detected: " + getSpeciesName(detection.species) + 
                       " (" + getBehaviorName(detection.behavior) + ") " +
                       "Confidence: " + String(detection.overallConfidence, 3);

    if (level == "ERROR") {
        Logger::error(logMessage.c_str());
    } else if (level == "WARNING") {
        Logger::warn(logMessage.c_str());
    } else if (level == "INFO") {
        Logger::info(logMessage.c_str());
    } else {
        Logger::debug(logMessage.c_str());
    }
}

uint32_t WildlifeClassifier::getCurrentTimeMs() const {
    return millis();
}

String WildlifeClassifier::generateDetectionId() const {
    return "DET_" + String(getCurrentTimeMs()) + "_" + String(random(1000, 9999));
}

// ===========================
// GLOBAL UTILITY FUNCTIONS
// ===========================

const char* wildlifeSpeciesToString(WildlifeSpecies species) {
    switch (species) {
        case WildlifeSpecies::DEER: return "Deer";
        case WildlifeSpecies::BEAR: return "Bear";
        case WildlifeSpecies::WOLF: return "Wolf";
        case WildlifeSpecies::FOX: return "Fox";
        case WildlifeSpecies::RABBIT: return "Rabbit";
        case WildlifeSpecies::SQUIRREL: return "Squirrel";
        case WildlifeSpecies::RACCOON: return "Raccoon";
        case WildlifeSpecies::COYOTE: return "Coyote";
        case WildlifeSpecies::MOOSE: return "Moose";
        case WildlifeSpecies::ELK: return "Elk";
        case WildlifeSpecies::EAGLE: return "Eagle";
        case WildlifeSpecies::HAWK: return "Hawk";
        case WildlifeSpecies::OWL: return "Owl";
        case WildlifeSpecies::CROW: return "Crow";
        case WildlifeSpecies::DUCK: return "Duck";
        case WildlifeSpecies::GOOSE: return "Goose";
        case WildlifeSpecies::TURKEY: return "Turkey";
        case WildlifeSpecies::WOODPECKER: return "Woodpecker";
        case WildlifeSpecies::CARDINAL: return "Cardinal";
        case WildlifeSpecies::SPARROW: return "Sparrow";
        case WildlifeSpecies::DOMESTIC_CAT: return "Domestic Cat";
        case WildlifeSpecies::DOMESTIC_DOG: return "Domestic Dog";
        case WildlifeSpecies::HUMAN: return "Human";
        case WildlifeSpecies::VEHICLE: return "Vehicle";
        default: return "Unknown";
    }
}

const char* animalBehaviorToString(AnimalBehavior behavior) {
    switch (behavior) {
        case AnimalBehavior::FEEDING: return "Feeding";
        case AnimalBehavior::RESTING: return "Resting";
        case AnimalBehavior::MOVING: return "Moving";
        case AnimalBehavior::GROOMING: return "Grooming";
        case AnimalBehavior::ALERT: return "Alert";
        case AnimalBehavior::AGGRESSIVE: return "Aggressive";
        case AnimalBehavior::MATING: return "Mating";
        case AnimalBehavior::NESTING: return "Nesting";
        case AnimalBehavior::TERRITORIAL: return "Territorial";
        case AnimalBehavior::SOCIAL: return "Social";
        default: return "Unknown";
    }
}