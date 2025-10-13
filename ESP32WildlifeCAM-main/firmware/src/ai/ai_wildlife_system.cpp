/**
 * @file ai_wildlife_system.cpp
 * @brief Enhanced AI Wildlife Monitoring System Implementation
 * 
 * Integrates all AI/ML components including multi-modal processing
 * for comprehensive wildlife monitoring.
 */

#include "ai_wildlife_system.h"
#include "../debug_utils.h"
#include <Arduino.h>

// ===========================
// AIWildlifeSystem Implementation
// ===========================

AIWildlifeSystem::AIWildlifeSystem() {
    // Constructor implementation
}

AIWildlifeSystem::~AIWildlifeSystem() {
    cleanup();
}

bool AIWildlifeSystem::init() {
    DEBUG_PRINTLN("Initializing AI Wildlife System...");
    
    // Initialize multi-modal AI system
    if (!multiModalSystem_.init(&inferenceEngine_)) {
        DEBUG_PRINTLN("ERROR: Failed to initialize multi-modal system");
        return false;
    }
    
    DEBUG_PRINTLN("AI Wildlife System initialized successfully");
    return true;
}

bool AIWildlifeSystem::loadModels(const char* modelsDirectory) {
    DEBUG_PRINTLN(String("Loading models from: ") + modelsDirectory);
    
    // Load TensorFlow Lite models for inference engine
    // This would be implemented to load actual model files
    
    return true;
}

void AIWildlifeSystem::configure(const AIProcessingConfig& config) {
    config_ = config;
    
    // Configure multi-modal processing
    if (config.enableMultiModalProcessing) {
        SensorFusionConfig fusionConfig;
        AudioProcessingConfig audioConfig;
        
        multiModalSystem_.configure(fusionConfig, audioConfig);
        multiModalSystem_.enableAudioProcessing(true);
    }
    
    // Configure environmental adaptation
    if (config.enableEnvironmentalAdaptation) {
        multiModalSystem_.enableEnvironmentalAdaptation(true);
    }
    
    DEBUG_PRINTLN("AI Wildlife System configured");
}

void AIWildlifeSystem::cleanup() {
    multiModalSystem_.cleanup();
    DEBUG_PRINTLN("AI Wildlife System cleanup complete");
}

bool AIWildlifeSystem::isSystemReady() const {
    return multiModalSystem_.isInitialized();
}

// Enhanced processing functions
WildlifeAnalysisResult AIWildlifeSystem::analyzeFrame(const CameraFrame& frame) {
    WildlifeAnalysisResult result;
    
    // Simple visual-only analysis
    EnvironmentalContext context;
    AIResult visualResult = multiModalSystem_.analyzeVisual(frame, context);
    
    // Map to WildlifeAnalysisResult
    result.species.detected = visualResult.detected;
    result.species.species = visualResult.detectedSpecies;
    result.species.confidence = visualResult.confidence;
    
    result.behavior.behavior = visualResult.detectedBehavior;
    result.behavior.confidence = visualResult.confidence;
    
    result.overallConfidence = visualResult.confidence;
    result.timestamp = millis();
    
    return result;
}

WildlifeAnalysisResult AIWildlifeSystem::analyzeFrameWithAudio(
    const CameraFrame& frame,
    const float* audioData,
    size_t audioLength) {
    
    WildlifeAnalysisResult result;
    
    if (!config_.enableMultiModalProcessing) {
        // Fall back to visual-only
        return analyzeFrame(frame);
    }
    
    // Multi-modal analysis
    EnvironmentalContext context;
    MultiModalResult mmResult = multiModalSystem_.analyzeMultiModal(
        frame, audioData, audioLength, context);
    
    // Map MultiModalResult to WildlifeAnalysisResult
    result.multiModalResult = mmResult;
    result.audioAnalysis = mmResult.audioFeatures;
    result.environmentalContext = mmResult.environment;
    
    // Species detection
    result.species.detected = (mmResult.fusedConfidence > config_.confidenceThreshold);
    result.species.species = mmResult.fusedSpeciesDetection;
    result.species.confidence = mmResult.fusedConfidence;
    
    // Behavior detection
    result.behavior.behavior = mmResult.fusedBehaviorDetection;
    result.behavior.confidence = mmResult.fusedConfidence;
    
    // Overall metrics
    result.overallConfidence = mmResult.overallConfidence;
    result.motionDetected = mmResult.motionDetected;
    result.threatDetected = mmResult.threatDetected;
    result.humanPresenceDetected = mmResult.humanPresenceDetected;
    result.timestamp = millis();
    
    // Advanced analysis
    result.stressLevel = mmResult.stressLevel;
    result.biodiversityIndex = mmResult.biodiversityIndex;
    result.behaviorIndicators = mmResult.behaviorIndicators;
    
    return result;
}

WildlifeAnalysisResult AIWildlifeSystem::analyzeFrameSequence(
    const std::vector<CameraFrame>& frames) {
    
    WildlifeAnalysisResult result;
    
    if (frames.empty()) {
        return result;
    }
    
    // Analyze most recent frame (simplified)
    result = analyzeFrame(frames.back());
    
    // Could implement temporal analysis across frames here
    
    return result;
}

WildlifeAnalysisResult AIWildlifeSystem::analyzeMultiModal(
    const CameraFrame& frame,
    const float* audioData,
    size_t audioLength,
    const EnvironmentalContext& environment) {
    
    WildlifeAnalysisResult result;
    
    // Full multi-modal analysis with environmental context
    MultiModalResult mmResult = multiModalSystem_.analyzeMultiModal(
        frame, audioData, audioLength, environment);
    
    // Map results
    result.multiModalResult = mmResult;
    result.audioAnalysis = mmResult.audioFeatures;
    result.environmentalContext = environment;
    
    result.species.detected = (mmResult.fusedConfidence > config_.confidenceThreshold);
    result.species.species = mmResult.fusedSpeciesDetection;
    result.species.confidence = mmResult.fusedConfidence;
    
    result.behavior.behavior = mmResult.fusedBehaviorDetection;
    result.behavior.confidence = mmResult.fusedConfidence;
    
    result.overallConfidence = mmResult.overallConfidence;
    result.motionDetected = mmResult.motionDetected;
    result.threatDetected = mmResult.threatDetected;
    result.humanPresenceDetected = mmResult.humanPresenceDetected;
    result.timestamp = millis();
    
    result.stressLevel = mmResult.stressLevel;
    result.biodiversityIndex = mmResult.biodiversityIndex;
    result.behaviorIndicators = mmResult.behaviorIndicators;
    
    return result;
}

// Multi-modal processing
void AIWildlifeSystem::enableMultiModalProcessing(bool enable) {
    config_.enableMultiModalProcessing = enable;
    DEBUG_PRINTLN(String("Multi-modal processing ") + (enable ? "enabled" : "disabled"));
}

void AIWildlifeSystem::enableAudioProcessing(bool enable) {
    multiModalSystem_.enableAudioProcessing(enable);
    DEBUG_PRINTLN(String("Audio processing ") + (enable ? "enabled" : "disabled"));
}

void AIWildlifeSystem::configureSensorFusion(const SensorFusionConfig& config) {
    AudioProcessingConfig audioConfig;
    multiModalSystem_.configure(config, audioConfig);
    DEBUG_PRINTLN("Sensor fusion configured");
}

MultiModalResult AIWildlifeSystem::getLastMultiModalResult() const {
    // Return last stored multi-modal result
    // In a full implementation, this would be cached
    return MultiModalResult();
}

// Advanced features
void AIWildlifeSystem::enableEcosystemMonitoring(bool enable) {
    multiModalSystem_.enableEcosystemMonitoring(enable);
}

void AIWildlifeSystem::enableThreatDetection(bool enable) {
    config_.enableThreatDetection = enable;
}

void AIWildlifeSystem::setConservationTargets(const std::vector<SpeciesType>& targets) {
    multiModalSystem_.optimizeForSpecies(targets);
}

std::vector<SpeciesType> AIWildlifeSystem::getPredictedActiveSpecies() const {
    EnvironmentalContext context;
    return multiModalSystem_.predictActiveSpecies(context);
}

float AIWildlifeSystem::calculateBiodiversityIndex() const {
    std::vector<SpeciesType> detectedSpecies;
    // In full implementation, would track detected species over time
    return multiModalSystem_.calculateBiodiversityIndex(detectedSpecies);
}

// Research and data export
void AIWildlifeSystem::enableResearchMode(bool enable) {
    multiModalSystem_.enableDebugMode(enable);
}

void AIWildlifeSystem::exportAnalysisData(const char* filename) {
    multiModalSystem_.exportAnalysisData(filename);
}

// ===========================
// IntelligentTrigger Implementation
// ===========================

IntelligentTrigger::IntelligentTrigger() 
    : timeBasedTriggersEnabled_(true)
    , behaviorBasedTriggersEnabled_(true)
    , powerOptimizationEnabled_(false) {
}

bool IntelligentTrigger::init() {
    DEBUG_PRINTLN("Initializing Intelligent Trigger System...");
    return true;
}

bool IntelligentTrigger::shouldCapture(const WildlifeAnalysisResult& analysis) {
    // Determine if we should capture based on analysis
    
    // Always capture if high confidence
    if (analysis.overallConfidence > 0.7f) {
        return true;
    }
    
    // Capture if target species detected
    if (analysis.species.detected) {
        return true;
    }
    
    // Capture if threat detected
    if (analysis.threatDetected) {
        return true;
    }
    
    // Calculate trigger probability based on learned patterns
    float probability = calculateTriggerProbability(analysis);
    
    return probability > 0.5f;
}

void IntelligentTrigger::learnFromCapture(const WildlifeAnalysisResult& analysis, 
                                         bool wasUseful) {
    updatePatterns(analysis, wasUseful);
}

float IntelligentTrigger::predictAnimalProbability() {
    // Predict probability of animal presence based on time/patterns
    // Simplified implementation
    return 0.5f;
}

void IntelligentTrigger::optimizeForSpecies(SpeciesType target) {
    // Optimize trigger for specific species
    DEBUG_PRINTLN(String("Optimizing trigger for species: ") + int(target));
}

void IntelligentTrigger::setTimeBasedTriggers(bool enable) {
    timeBasedTriggersEnabled_ = enable;
}

void IntelligentTrigger::setBehaviorBasedTriggers(bool enable) {
    behaviorBasedTriggersEnabled_ = enable;
}

void IntelligentTrigger::enablePowerOptimization(bool enable) {
    powerOptimizationEnabled_ = enable;
}

unsigned long IntelligentTrigger::getPredictedSleepTime() {
    // Predict optimal sleep time based on patterns
    // Simplified: 10 minutes default
    return 10 * 60 * 1000;
}

bool IntelligentTrigger::shouldWakeUp() {
    // Determine if system should wake up
    // Would check environmental conditions, time patterns, etc.
    return true;
}

// Private methods
void IntelligentTrigger::updatePatterns(const WildlifeAnalysisResult& analysis, 
                                       bool success) {
    // Update learned patterns based on capture success
    
    TriggerPattern pattern;
    pattern.species = analysis.species.species;
    pattern.behavior = analysis.behavior.behavior;
    pattern.hourOfDay = hour();  // Would get actual time
    pattern.totalCount = 1;
    pattern.successCount = success ? 1 : 0;
    pattern.probability = success ? 1.0f : 0.0f;
    
    // Find existing pattern or add new
    bool found = false;
    for (auto& p : learnedPatterns_) {
        if (p.species == pattern.species && 
            p.behavior == pattern.behavior &&
            p.hourOfDay == pattern.hourOfDay) {
            
            p.totalCount++;
            if (success) p.successCount++;
            p.probability = float(p.successCount) / p.totalCount;
            found = true;
            break;
        }
    }
    
    if (!found) {
        learnedPatterns_.push_back(pattern);
    }
}

float IntelligentTrigger::calculateTriggerProbability(
    const WildlifeAnalysisResult& analysis) {
    
    float probability = 0.5f; // Base probability
    
    // Look for matching patterns
    for (const auto& pattern : learnedPatterns_) {
        if (pattern.species == analysis.species.species &&
            pattern.behavior == analysis.behavior.behavior) {
            probability = pattern.probability;
            break;
        }
    }
    
    return probability;
}
