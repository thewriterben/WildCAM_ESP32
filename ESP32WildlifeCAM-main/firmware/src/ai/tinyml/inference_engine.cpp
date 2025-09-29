/**
 * @file inference_engine.cpp
 * @brief Implementation of TinyML Inference Engine with AI Optimizations
 */

#include "inference_engine.h"
#include "tensorflow_lite_micro.h"
#include "../../config.h"
#include <esp_timer.h>
#include <algorithm>
#include <cmath>  // For fabs() function

// TensorFlow Lite Micro includes (these would be from the actual TFLite library)
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/micro_op_resolver.h"
// #include "tensorflow/lite/micro/micro_error_reporter.h"
// #include "tensorflow/lite/schema/schema_generated.h"

// Constants
const float InferenceEngine::DEFAULT_CONFIDENCE_THRESHOLD = 0.6f;
const unsigned long InferenceEngine::DEFAULT_MAX_INFERENCE_TIME = 3000; // 3 seconds

/**
 * Intelligent Model Cache for performance optimization
 */
class ModelCache {
private:
    struct CacheEntry {
        uint32_t imageHash;
        ModelType modelType;
        AIResult result;
        unsigned long timestamp;
        uint32_t accessCount;
        float similarity;
        
        CacheEntry() : imageHash(0), modelType(ModelType::SPECIES_CLASSIFIER),
                      timestamp(0), accessCount(0), similarity(0.0f) {}
    };
    
    static const size_t CACHE_SIZE = 32;
    CacheEntry cache_[CACHE_SIZE];
    size_t nextIndex_;
    uint32_t totalHits_;
    uint32_t totalRequests_;
    
public:
    ModelCache() : nextIndex_(0), totalHits_(0), totalRequests_(0) {}
    
    uint32_t calculateImageHash(const CameraFrame& frame) {
        // Enhanced hash function with spatial sampling
        uint32_t hash = 0;
        const uint8_t* data = frame.data;
        size_t stepSize = std::max(size_t(1), frame.length / 128); // Sample 128 points
        
        // Sample from different regions of the image
        for (size_t i = 0; i < frame.length; i += stepSize) {
            hash = hash * 31 + data[i];
        }
        
        // Add frame dimensions to hash for better discrimination
        hash = hash * 31 + frame.width;
        hash = hash * 31 + frame.height;
        
        return hash;
    }
    
    bool lookup(const CameraFrame& frame, ModelType type, AIResult& result) {
        totalRequests_++;
        uint32_t hash = calculateImageHash(frame);
        unsigned long currentTime = millis();
        
        float bestSimilarity = 0.0f;
        int bestMatch = -1;
        
        for (size_t i = 0; i < CACHE_SIZE; i++) {
            if (cache_[i].modelType == type && cache_[i].imageHash != 0) {
                // Check temporal validity (5 second window)
                if ((currentTime - cache_[i].timestamp) < 5000) {
                    // Calculate similarity based on hash difference
                    uint32_t hashDiff = cache_[i].imageHash ^ hash;
                    float similarity = 1.0f - (__builtin_popcount(hashDiff) / 32.0f);
                    
                    if (similarity > 0.9f && similarity > bestSimilarity) {
                        bestSimilarity = similarity;
                        bestMatch = i;
                    }
                }
            }
        }
        
        if (bestMatch >= 0) {
            result = cache_[bestMatch].result;
            cache_[bestMatch].accessCount++;
            cache_[bestMatch].timestamp = currentTime; // Update timestamp
            cache_[bestMatch].similarity = bestSimilarity;
            totalHits_++;
            
            DEBUG_PRINTF("Cache hit: similarity=%.2f, type=%d\n", bestSimilarity, static_cast<int>(type));
            return true;
        }
        
        return false;
    }
    
    void store(const CameraFrame& frame, ModelType type, const AIResult& result) {
        uint32_t hash = calculateImageHash(frame);
        
        // Find least recently used entry
        size_t lruIndex = nextIndex_;
        unsigned long oldestTime = cache_[nextIndex_].timestamp;
        
        for (size_t i = 0; i < CACHE_SIZE; i++) {
            if (cache_[i].timestamp < oldestTime) {
                oldestTime = cache_[i].timestamp;
                lruIndex = i;
            }
        }
        
        cache_[lruIndex].imageHash = hash;
        cache_[lruIndex].modelType = type;
        cache_[lruIndex].result = result;
        cache_[lruIndex].timestamp = millis();
        cache_[lruIndex].accessCount = 1;
        cache_[lruIndex].similarity = 1.0f;
        
        nextIndex_ = (lruIndex + 1) % CACHE_SIZE;
    }
    
    float getHitRate() const {
        return totalRequests_ > 0 ? (float)totalHits_ / totalRequests_ : 0.0f;
    }
    
    void clear() {
        for (size_t i = 0; i < CACHE_SIZE; i++) {
            cache_[i] = CacheEntry();
        }
        nextIndex_ = 0;
        totalHits_ = 0;
        totalRequests_ = 0;
    }
    
    void printStats() {
        DEBUG_PRINTF("Model Cache Stats: %u/%u hits (%.1f%%), %u entries\n",
                    totalHits_, totalRequests_, getHitRate() * 100.0f, CACHE_SIZE);
    }
};

/**
 * Predictive Model Preloader with Enhanced Pattern Recognition
 */
class ModelPredictor {
private:
    struct UsagePattern {
        ModelType modelType;
        uint8_t hourOfDay;
        uint8_t dayOfWeek;
        float temperature;
        float lightLevel;
        float probability;
        uint32_t usageCount;
        float averageConfidence;
        unsigned long lastUsed;
    };
    
    std::vector<UsagePattern> patterns_;
    ModelType lastUsedModel_;
    unsigned long lastUsageTime_;
    float movingAverageConfidence_;
    
public:
    ModelPredictor() : lastUsedModel_(ModelType::SPECIES_CLASSIFIER), 
                      lastUsageTime_(0), movingAverageConfidence_(0.0f) {}
    
    void recordUsage(ModelType type, float confidence = 0.0f, 
                    float temperature = 25.0f, float lightLevel = 0.5f) {
        uint8_t currentHour = (millis() / 3600000) % 24;
        uint8_t currentDay = (millis() / (24 * 3600000)) % 7;
        
        // Update moving average confidence
        movingAverageConfidence_ = movingAverageConfidence_ * 0.9f + confidence * 0.1f;
        
        // Find existing pattern or create new one
        for (auto& pattern : patterns_) {
            if (pattern.modelType == type && 
                pattern.hourOfDay == currentHour &&
                fabs(pattern.temperature - temperature) < 5.0f) {
                
                pattern.usageCount++;
                pattern.probability = calculateProbability(pattern.usageCount, confidence);
                pattern.averageConfidence = pattern.averageConfidence * 0.8f + confidence * 0.2f;
                pattern.lastUsed = millis();
                return;
            }
        }
        
        // Create new pattern
        UsagePattern newPattern;
        newPattern.modelType = type;
        newPattern.hourOfDay = currentHour;
        newPattern.dayOfWeek = currentDay;
        newPattern.temperature = temperature;
        newPattern.lightLevel = lightLevel;
        newPattern.usageCount = 1;
        newPattern.probability = 0.1f;
        newPattern.averageConfidence = confidence;
        newPattern.lastUsed = millis();
        patterns_.push_back(newPattern);
        
        lastUsedModel_ = type;
        lastUsageTime_ = millis();
        
        // Limit pattern storage to prevent memory overflow
        if (patterns_.size() > 100) {
            // Remove oldest patterns
            std::sort(patterns_.begin(), patterns_.end(),
                     [](const UsagePattern& a, const UsagePattern& b) {
                         return a.lastUsed < b.lastUsed;
                     });
            patterns_.erase(patterns_.begin(), patterns_.begin() + 20);
        }
    }
    
    std::vector<ModelType> predictNextModels(float currentTemp = 25.0f, float currentLight = 0.5f) {
        std::vector<ModelType> predictions;
        uint8_t currentHour = (millis() / 3600000) % 24;
        uint8_t nextHour = (currentHour + 1) % 24;
        
        // Score all patterns based on current conditions
        std::vector<std::pair<float, ModelType>> candidates;
        
        for (const auto& pattern : patterns_) {
            float score = 0.0f;
            
            // Time-based scoring
            if (pattern.hourOfDay == currentHour) {
                score += 0.4f * pattern.probability;
            } else if (pattern.hourOfDay == nextHour) {
                score += 0.2f * pattern.probability;
            }
            
            // Environmental condition scoring
            float tempDiff = fabs(pattern.temperature - currentTemp);
            float lightDiff = fabs(pattern.lightLevel - currentLight);
            
            if (tempDiff < 5.0f) score += 0.2f;
            if (lightDiff < 0.2f) score += 0.2f;
            
            // Confidence-based scoring
            score += 0.2f * pattern.averageConfidence;
            
            if (score > 0.1f) {
                candidates.push_back({score, pattern.modelType});
            }
        }
        
        // Sort by score (highest first)
        std::sort(candidates.begin(), candidates.end(),
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Return top 3 unique predictions
        std::set<ModelType> addedTypes;
        for (const auto& candidate : candidates) {
            if (addedTypes.find(candidate.second) == addedTypes.end()) {
                predictions.push_back(candidate.second);
                addedTypes.insert(candidate.second);
                if (predictions.size() >= 3) break;
            }
        }
        
        // If no patterns match, use default sequence
        if (predictions.empty()) {
            predictions = {ModelType::MOTION_DETECTOR, ModelType::SPECIES_CLASSIFIER, ModelType::BEHAVIOR_ANALYZER};
        }
        
        return predictions;
    }
    
    float getPredictedAccuracy(ModelType type) {
        for (const auto& pattern : patterns_) {
            if (pattern.modelType == type) {
                return pattern.averageConfidence;
            }
        }
        return movingAverageConfidence_;
    }
    
    void optimizePatterns() {
        // Remove low-performing patterns
        patterns_.erase(
            std::remove_if(patterns_.begin(), patterns_.end(),
                          [](const UsagePattern& p) {
                              return p.averageConfidence < 0.3f && p.usageCount < 3;
                          }),
            patterns_.end());
    }
    
private:
    float calculateProbability(uint32_t usageCount, float confidence) {
        // Enhanced probability calculation considering confidence
        float baseProbability = std::min(0.95f, usageCount * 0.05f);
        float confidenceBoost = confidence * 0.3f;
        return std::min(0.95f, baseProbability + confidenceBoost);
    }
};

/**
 * Constructor
 */
InferenceEngine::InferenceEngine() :
    maxMemoryLimit_(MAX_MEMORY_LIMIT),
    currentMemoryUsage_(0),
    quantizationEnabled_(true),
    powerOptimizationEnabled_(true),
    confidenceThreshold_(DEFAULT_CONFIDENCE_THRESHOLD),
    maxInferenceTime_(DEFAULT_MAX_INFERENCE_TIME),
    modelCache_(std::make_unique<ModelCache>()),
    modelPredictor_(std::make_unique<ModelPredictor>()) {
    
    // Initialize model containers
    models_.resize(MAX_MODELS);
    for (size_t i = 0; i < MAX_MODELS; ++i) {
        models_[i] = std::make_unique<ModelContainer>();
    }
}

/**
 * Destructor
 */
InferenceEngine::~InferenceEngine() {
    // Cleanup all models
    for (auto& model : models_) {
        if (model && model->isLoaded) {
            deallocateTensorArena(model.get());
        }
    }
}

/**
 * Initialize the inference engine
 */
bool InferenceEngine::init() {
    DEBUG_PRINTLN("Initializing TinyML Inference Engine with AI optimizations...");
    
    // Initialize TensorFlow Lite Micro
    if (!TensorFlowLiteMicro::init()) {
        DEBUG_PRINTLN("Error: Failed to initialize TensorFlow Lite Micro");
        return false;
    }
    
    // Enable ESP32-specific optimizations
    TensorFlowLiteMicro::enableESP32Optimizations();
    
    // Configure power optimization if enabled
    if (powerOptimizationEnabled_) {
        TensorFlowLiteMicro::enablePowerSaving();
    }
    
    // Initialize performance monitoring
    resetMetrics();
    
    // Clear intelligent cache
    modelCache_->clear();
    
    DEBUG_PRINTLN("TinyML Inference Engine initialized successfully with intelligent caching");
    return true;
}

/**
 * Load a model from memory
 */
bool InferenceEngine::loadModel(const uint8_t* modelData, size_t modelSize, ModelType type) {
    if (!modelData || modelSize == 0) {
        DEBUG_PRINTLN("Error: Invalid model data");
        return false;
    }
    
    // Validate model format
    if (!validateModelFormat(modelData, modelSize)) {
        DEBUG_PRINTLN("Error: Invalid model format");
        return false;
    }
    
    ModelContainer* container = getModelContainer(type);
    if (!container) {
        DEBUG_PRINTLN("Error: Could not get model container");
        return false;
    }
    
    // Setup interpreter
    if (!setupInterpreter(container, modelData, modelSize)) {
        DEBUG_PRINTLN("Error: Failed to setup interpreter");
        return false;
    }
    
    // Update model info
    container->info.name = String(modelTypeToString(type));
    container->info.modelSize = modelSize;
    container->info.isLoaded = true;
    container->isLoaded = true;
    
    DEBUG_PRINTF("Model loaded successfully: %s (Size: %zu bytes)\n", 
                container->info.name.c_str(), modelSize);
    
    return true;
}

/**
 * Load a model from file
 */
bool InferenceEngine::loadModelFromFile(const char* filename, ModelType type) {
    // This would implement file loading from SD card or LittleFS
    DEBUG_PRINTF("Loading model from file: %s\n", filename);
    
    // For now, return false as this needs file system implementation
    DEBUG_PRINTLN("Error: File loading not yet implemented");
    return false;
}

/**
 * Run inference on a camera frame with intelligent caching
 */
AIResult InferenceEngine::runInference(const CameraFrame& frame, ModelType type) {
    AIResult result;
    unsigned long startTime = millis();
    
    // Check intelligent cache first
    if (modelCache_->lookup(frame, type, result)) {
        DEBUG_PRINTLN("Cache hit - returning cached result");
        updateMetrics(1.0f, true); // Cache hits are very fast
        return result;
    }
    
    ModelContainer* container = getModelContainer(type);
    if (!container || !container->isLoaded) {
        DEBUG_PRINTLN("Error: Model not loaded");
        updateMetrics(0, false);
        return result;
    }
    
    // Preprocess image with enhanced preprocessing
    float* inputTensor = nullptr; // This would be allocated properly
    if (!preprocessImageEnhanced(frame, inputTensor, type)) {
        DEBUG_PRINTLN("Error: Enhanced image preprocessing failed");
        updateMetrics(0, false);
        return result;
    }
    
    // Progressive inference with early exit
    bool earlyExit = false;
    float preliminaryConfidence = 0.0f;
    
    if (type == ModelType::MOTION_DETECTOR) {
        // Quick motion check first
        preliminaryConfidence = quickMotionCheck(frame);
        if (preliminaryConfidence < 0.1f) {
            result.behavior = BehaviorType::RESTING;
            result.confidence = 1.0f - preliminaryConfidence;
            result.isValidDetection = true;
            result.timestamp = millis();
            earlyExit = true;
        }
    }
    
    if (!earlyExit) {
        // Run full inference (this would use actual TFLite interpreter)
        // container->interpreter->Invoke();
        
        // Get output tensor (placeholder implementation)
        float* outputTensor = nullptr; // This would get actual output
        
        // Postprocess output with enhanced analysis
        result = postprocessOutputEnhanced(outputTensor, type, frame);
    }
    
    // Store result in intelligent cache
    if (result.isValidDetection && result.confidence > confidenceThreshold_) {
        modelCache_->store(frame, type, result);
    }
    
    // Record usage pattern for prediction
    float environmentalTemp = 25.0f; // Would get from environmental sensors
    float lightLevel = 0.5f; // Would calculate from image brightness
    modelPredictor_->recordUsage(type, result.confidence, environmentalTemp, lightLevel);
    
    // Update metrics
    float inferenceTime = millis() - startTime;
    updateMetrics(inferenceTime, result.isValidDetection);
    
    // Check inference timeout
    if (inferenceTime > maxInferenceTime_) {
        DEBUG_PRINTF("Warning: Inference timeout (%.2fms > %lums)\n", 
                    inferenceTime, maxInferenceTime_);
    }
    
    return result;
}

/**
 * Enhanced image preprocessing with adaptive optimization
 */
bool InferenceEngine::preprocessImageEnhanced(const CameraFrame& frame, float* inputTensor, ModelType type) {
    if (!frame.data || !inputTensor) return false;
    
    // Model-specific preprocessing optimizations
    switch (type) {
        case ModelType::MOTION_DETECTOR:
            // Motion detection benefits from edge enhancement
            return preprocessForMotionDetection(frame, inputTensor);
            
        case ModelType::SPECIES_CLASSIFIER:
            // Species classification needs good contrast and normalization
            return preprocessForSpeciesClassification(frame, inputTensor);
            
        case ModelType::BEHAVIOR_ANALYZER:
            // Behavior analysis needs temporal information
            return preprocessForBehaviorAnalysis(frame, inputTensor);
            
        default:
            // General preprocessing
            return preprocessImage(frame, inputTensor, type);
    }
}

/**
 * Quick motion detection for early exit
 */
float InferenceEngine::quickMotionCheck(const CameraFrame& frame) {
    // Simple motion detection using frame differencing
    static CameraFrame previousFrame = {};
    
    if (previousFrame.data == nullptr) {
        // First frame - no motion
        previousFrame = frame;
        return 0.0f;
    }
    
    // Calculate simple difference metric
    uint32_t totalDiff = 0;
    size_t sampleCount = 0;
    size_t stepSize = std::max(size_t(1), frame.length / 100); // Sample 100 points
    
    for (size_t i = 0; i < std::min(frame.length, previousFrame.length); i += stepSize) {
        totalDiff += abs(frame.data[i] - previousFrame.data[i]);
        sampleCount++;
    }
    
    float motionLevel = sampleCount > 0 ? (float)totalDiff / (sampleCount * 255.0f) : 0.0f;
    
    // Update previous frame (simple approach - in practice would use ring buffer)
    previousFrame = frame;
    
    return motionLevel;
}

/**
 * Enhanced postprocessing with context awareness
 */
AIResult InferenceEngine::postprocessOutputEnhanced(const float* outputTensor, ModelType type, const CameraFrame& frame) {
    AIResult result = postprocessOutput(outputTensor, type, frame);
    
    if (!result.isValidDetection) {
        return result;
    }
    
    // Apply context-aware confidence adjustment
    float contextAdjustment = calculateContextualConfidence(result, frame);
    result.confidence = std::min(1.0f, result.confidence * contextAdjustment);
    
    // Update confidence level
    result.confidenceLevel = floatToConfidenceLevel(result.confidence);
    
    // Re-evaluate detection validity
    result.isValidDetection = (result.confidence >= confidenceThreshold_);
    
    return result;
}

/**
 * Calculate contextual confidence based on environmental factors
 */
float InferenceEngine::calculateContextualConfidence(const AIResult& result, const CameraFrame& frame) {
    float adjustment = 1.0f;
    
    // Time-of-day adjustment
    uint8_t currentHour = (millis() / 3600000) % 24;
    if (currentHour >= 6 && currentHour <= 18) {
        // Daytime - better visibility
        adjustment *= 1.1f;
    } else {
        // Nighttime - reduced confidence
        adjustment *= 0.9f;
    }
    
    // Image quality assessment
    float brightness = calculateImageBrightness(frame);
    if (brightness > 0.2f && brightness < 0.8f) {
        // Good lighting conditions
        adjustment *= 1.05f;
    } else {
        // Poor lighting conditions
        adjustment *= 0.95f;
    }
    
    // Species-specific adjustments
    switch (result.species) {
        case SpeciesType::MAMMAL_LARGE:
            // Large mammals are easier to detect
            adjustment *= 1.1f;
            break;
        case SpeciesType::BIRD_SMALL:
            // Small birds are harder to detect
            adjustment *= 0.9f;
            break;
        default:
            break;
    }
    
    return std::max(0.5f, std::min(1.5f, adjustment));
}

/**
 * Calculate average brightness of image
 */
float InferenceEngine::calculateImageBrightness(const CameraFrame& frame) {
    if (!frame.data || frame.length == 0) return 0.5f;
    
    uint32_t totalBrightness = 0;
    size_t sampleCount = 0;
    size_t stepSize = std::max(size_t(1), frame.length / 64); // Sample 64 points
    
    for (size_t i = 0; i < frame.length; i += stepSize) {
        totalBrightness += frame.data[i];
        sampleCount++;
    }
    
    return sampleCount > 0 ? (float)totalBrightness / (sampleCount * 255.0f) : 0.5f;
}

/**
 * Preprocessing for motion detection
 */
bool InferenceEngine::preprocessForMotionDetection(const CameraFrame& frame, float* inputTensor) {
    // Enhanced preprocessing for motion detection
    // Would implement edge detection, noise reduction, etc.
    DEBUG_PRINTLN("Motion detection preprocessing applied");
    return preprocessImage(frame, inputTensor, ModelType::MOTION_DETECTOR);
}

/**
 * Preprocessing for species classification
 */
bool InferenceEngine::preprocessForSpeciesClassification(const CameraFrame& frame, float* inputTensor) {
    // Enhanced preprocessing for species classification
    // Would implement contrast enhancement, color normalization, etc.
    DEBUG_PRINTLN("Species classification preprocessing applied");
    return preprocessImage(frame, inputTensor, ModelType::SPECIES_CLASSIFIER);
}

/**
 * Preprocessing for behavior analysis
 */
bool InferenceEngine::preprocessForBehaviorAnalysis(const CameraFrame& frame, float* inputTensor) {
    // Enhanced preprocessing for behavior analysis
    // Would implement temporal feature extraction, motion vectors, etc.
    DEBUG_PRINTLN("Behavior analysis preprocessing applied");
    return preprocessImage(frame, inputTensor, ModelType::BEHAVIOR_ANALYZER);
}

/**
 * Get model container for specified type
 */
InferenceEngine::ModelContainer* InferenceEngine::getModelContainer(ModelType type) {
    size_t index = getModelIndex(type);
    if (index >= models_.size()) {
        return nullptr;
    }
    return models_[index].get();
}

/**
 * Setup interpreter for model container
 */
bool InferenceEngine::setupInterpreter(ModelContainer* container, const uint8_t* modelData, size_t modelSize) {
    if (!container) return false;
    
    // Calculate required arena size
    size_t requiredArenaSize = calculateRequiredArenaSize(modelData, modelSize);
    if (requiredArenaSize == 0) {
        requiredArenaSize = DEFAULT_ARENA_SIZE;
    }
    
    // Allocate tensor arena
    if (!allocateTensorArena(container, requiredArenaSize)) {
        DEBUG_PRINTLN("Error: Failed to allocate tensor arena");
        return false;
    }
    
    // This would create the actual TFLite model and interpreter
    // container->model = tflite::GetModel(modelData);
    // container->resolver = std::make_unique<WildlifeOpResolver>();
    // container->interpreter = std::make_unique<tflite::MicroInterpreter>(
    //     container->model.get(), *container->resolver, 
    //     container->tensorArena, container->arenaSize);
    
    DEBUG_PRINTLN("Interpreter setup completed");
    return true;
}

/**
 * Allocate tensor arena for model
 */
bool InferenceEngine::allocateTensorArena(ModelContainer* container, size_t requiredSize) {
    if (!container || requiredSize == 0) return false;
    
    // Check memory limits
    if (currentMemoryUsage_ + requiredSize > maxMemoryLimit_) {
        DEBUG_PRINTF("Error: Memory limit exceeded (%zu + %zu > %zu)\n",
                    currentMemoryUsage_, requiredSize, maxMemoryLimit_);
        return false;
    }
    
    // Allocate memory (prefer PSRAM if available)
    container->tensorArena = static_cast<uint8_t*>(ESP32MemoryAllocator::allocatePSRAM(requiredSize));
    if (!container->tensorArena) {
        container->tensorArena = static_cast<uint8_t*>(ESP32MemoryAllocator::allocate(requiredSize));
    }
    
    if (!container->tensorArena) {
        DEBUG_PRINTF("Error: Failed to allocate %zu bytes for tensor arena\n", requiredSize);
        return false;
    }
    
    container->arenaSize = requiredSize;
    currentMemoryUsage_ += requiredSize;
    
    DEBUG_PRINTF("Allocated tensor arena: %zu bytes\n", requiredSize);
    return true;
}

/**
 * Deallocate tensor arena
 */
void InferenceEngine::deallocateTensorArena(ModelContainer* container) {
    if (container && container->tensorArena) {
        ESP32MemoryAllocator::deallocate(container->tensorArena);
        currentMemoryUsage_ -= container->arenaSize;
        container->tensorArena = nullptr;
        container->arenaSize = 0;
    }
}

/**
 * Update performance metrics
 */
void InferenceEngine::updateMetrics(float inferenceTime, bool success) {
    metrics_.inferenceTime = inferenceTime;
    metrics_.totalInferences++;
    if (success) {
        metrics_.successfulInferences++;
    }
    metrics_.memoryUsage = currentMemoryUsage_;
}

/**
 * Convert ModelType to string
 */
const char* modelTypeToString(ModelType type) {
    switch (type) {
        case ModelType::SPECIES_CLASSIFIER: return "Species Classifier";
        case ModelType::BEHAVIOR_ANALYZER: return "Behavior Analyzer";
        case ModelType::MOTION_DETECTOR: return "Motion Detector";
        case ModelType::OBJECT_TRACKER: return "Object Tracker";
        case ModelType::THREAT_DETECTOR: return "Threat Detector";
        default: return "Unknown";
    }
}

/**
 * Get model index from type
 */
size_t InferenceEngine::getModelIndex(ModelType type) const {
    return static_cast<size_t>(type);
}

/**
 * Validate model format
 */
bool InferenceEngine::validateModelFormat(const uint8_t* modelData, size_t modelSize) {
    if (!modelData || modelSize < 16) return false;
    
    // Basic validation - would check TFLite magic number and structure
    return true;
}

/**
 * Preprocess image for inference
 */
bool InferenceEngine::preprocessImage(const CameraFrame& frame, float* inputTensor, ModelType type) {
    if (!frame.data || !inputTensor) return false;
    
    // Image preprocessing would be implemented here
    // This includes resizing, normalization, format conversion, etc.
    
    DEBUG_PRINTLN("Image preprocessed for inference");
    return true;
}

/**
 * Postprocess inference output
 */
AIResult InferenceEngine::postprocessOutput(const float* outputTensor, ModelType type, const CameraFrame& frame) {
    AIResult result;
    
    if (!outputTensor) {
        return result;
    }
    
    // Route to appropriate postprocessing based on model type
    switch (type) {
        case ModelType::SPECIES_CLASSIFIER:
            return processSpeciesClassification(outputTensor, frame);
        case ModelType::BEHAVIOR_ANALYZER:
            return processBehaviorAnalysis(outputTensor, frame);
        case ModelType::MOTION_DETECTOR:
            return processMotionDetection(outputTensor, frame);
        default:
            DEBUG_PRINTLN("Warning: Unknown model type for postprocessing");
            break;
    }
    
    return result;
}

/**
 * Process species classification output
 */
AIResult InferenceEngine::processSpeciesClassification(const float* output, const CameraFrame& frame) {
    AIResult result;
    
    // Find the class with highest confidence
    float maxConfidence = 0.0f;
    int maxIndex = 0;
    
    // This would iterate through actual output tensor
    // for (int i = 0; i < numClasses; ++i) {
    //     if (output[i] > maxConfidence) {
    //         maxConfidence = output[i];
    //         maxIndex = i;
    //     }
    // }
    
    // Placeholder values
    maxConfidence = 0.85f;
    maxIndex = 2; // Example: MAMMAL_MEDIUM
    
    if (maxConfidence >= confidenceThreshold_) {
        result.species = static_cast<SpeciesType>(maxIndex);
        result.confidence = maxConfidence;
        result.confidenceLevel = floatToConfidenceLevel(maxConfidence);
        result.isValidDetection = true;
        result.timestamp = millis();
    }
    
    return result;
}

/**
 * Process behavior analysis output
 */
AIResult InferenceEngine::processBehaviorAnalysis(const float* output, const CameraFrame& frame) {
    AIResult result;
    
    // Similar to species classification but for behaviors
    // This would process the actual behavior model output
    
    // Placeholder implementation
    result.behavior = BehaviorType::FEEDING;
    result.confidence = 0.75f;
    result.confidenceLevel = floatToConfidenceLevel(0.75f);
    result.isValidDetection = true;
    result.timestamp = millis();
    
    return result;
}

/**
 * Process motion detection output
 */
AIResult InferenceEngine::processMotionDetection(const float* output, const CameraFrame& frame) {
    AIResult result;
    
    // Process motion detection model output
    // This would analyze motion vectors and confidence
    
    // Placeholder implementation
    result.behavior = BehaviorType::MOVING;
    result.confidence = 0.90f;
    result.confidenceLevel = floatToConfidenceLevel(0.90f);
    result.isValidDetection = true;
    result.timestamp = millis();
    
    return result;
}

/**
 * Get performance metrics
 */
AIMetrics InferenceEngine::getPerformanceMetrics() const {
    return metrics_;
}

/**
 * Calculate required arena size (placeholder implementation)
 */
size_t calculateRequiredArenaSize(const uint8_t* modelData, size_t modelSize) {
    // This would analyze the model to determine memory requirements
    // For now, return a reasonable default based on model size
    return std::max(static_cast<size_t>(64 * 1024), modelSize * 2);
}

/**
 * ModelContainer destructor
 */
InferenceEngine::ModelContainer::~ModelContainer() {
    if (tensorArena) {
        ESP32MemoryAllocator::deallocate(tensorArena);
    }
}