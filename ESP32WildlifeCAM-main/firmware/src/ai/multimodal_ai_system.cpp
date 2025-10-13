/**
 * @file multimodal_ai_system.cpp
 * @brief Multi-Modal AI System Implementation
 * 
 * Implements comprehensive multi-modal wildlife detection combining
 * visual, audio, and environmental sensor data with advanced fusion.
 */

#include "multimodal_ai_system.h"
#include "../debug_utils.h"
#include <Arduino.h>
#include <math.h>
#include <algorithm>

// Static constant initialization
const float MultiModalAISystem::MIN_FUSION_CONFIDENCE = 0.5f;

MultiModalAISystem::MultiModalAISystem() 
    : inferenceEngine_(nullptr)
    , initialized_(false)
    , audioProcessingEnabled_(false)
    , environmentalAdaptationEnabled_(false)
    , temporalAnalysisEnabled_(false)
    , debugModeEnabled_(false) {
    
    // Initialize default weights
    fusionConfig_.visualWeight = 0.4f;
    fusionConfig_.audioWeight = 0.3f;
    fusionConfig_.motionWeight = 0.2f;
    fusionConfig_.environmentalWeight = 0.1f;
    
    // Reserve memory for buffers
    audioBuffer_.reserve(AUDIO_BUFFER_SIZE);
    spectrogramBuffer_.reserve(audioConfig_.fftSize);
    mfccBuffer_.reserve(audioConfig_.numMfccCoefs);
    temporalHistory_.reserve(MAX_TEMPORAL_HISTORY);
}

MultiModalAISystem::~MultiModalAISystem() {
    cleanup();
}

bool MultiModalAISystem::init(InferenceEngine* inferenceEngine) {
    if (initialized_) {
        DEBUG_PRINTLN("MultiModalAISystem already initialized");
        return true;
    }
    
    if (!inferenceEngine) {
        DEBUG_PRINTLN("ERROR: InferenceEngine pointer is null");
        return false;
    }
    
    inferenceEngine_ = inferenceEngine;
    
    // Initialize audio processing config
    audioConfig_ = AudioProcessingConfig();
    
    // Initialize fusion config
    fusionConfig_ = SensorFusionConfig();
    
    // Reset metrics
    resetMetrics();
    
    initialized_ = true;
    DEBUG_PRINTLN("MultiModalAISystem initialized successfully");
    
    return true;
}

void MultiModalAISystem::configure(const SensorFusionConfig& fusionConfig,
                                   const AudioProcessingConfig& audioConfig) {
    fusionConfig_ = fusionConfig;
    audioConfig_ = audioConfig;
    
    DEBUG_PRINTLN("MultiModalAISystem configuration updated");
}

void MultiModalAISystem::cleanup() {
    if (!initialized_) {
        return;
    }
    
    // Clear buffers
    audioBuffer_.clear();
    spectrogramBuffer_.clear();
    mfccBuffer_.clear();
    temporalHistory_.clear();
    
    initialized_ = false;
    audioProcessingEnabled_ = false;
    
    DEBUG_PRINTLN("MultiModalAISystem cleanup complete");
}

bool MultiModalAISystem::isInitialized() const {
    return initialized_;
}

// Multi-modal analysis
MultiModalResult MultiModalAISystem::analyzeMultiModal(
    const CameraFrame& visualData,
    const float* audioData,
    size_t audioLength,
    const EnvironmentalContext& environment) {
    
    if (!initialized_) {
        DEBUG_PRINTLN("ERROR: MultiModalAISystem not initialized");
        return MultiModalResult();
    }
    
    unsigned long startTime = millis();
    MultiModalResult result;
    result.environment = environment;
    result.timestamp = millis();
    
    // Analyze visual data
    result.visualResult = analyzeVisual(visualData, environment);
    result.visualConfidence = result.visualResult.confidence;
    
    // Analyze audio data if enabled and available
    if (audioProcessingEnabled_ && audioData && audioLength > 0) {
        result.audioFeatures = analyzeAudio(audioData, audioLength);
        result.audioSpeciesDetection = classifyAudioSpecies(result.audioFeatures);
        result.audioConfidence = result.audioFeatures.vocalizationConfidence;
        result.detectedSounds = detectWildlifeSounds(result.audioFeatures);
    }
    
    // Analyze motion (simplified - would integrate with PIR sensor)
    result.motionDetected = result.visualResult.confidence > 0.3f;
    result.motionIntensity = result.visualResult.confidence;
    
    // Perform sensor fusion
    result = fuseSensorData(result.visualResult, result.audioFeatures, 
                           result.motionDetected, environment);
    
    // Temporal analysis if enabled
    if (temporalAnalysisEnabled_) {
        updateTemporalHistory(result);
    }
    
    // Update metrics
    float processingTime = (millis() - startTime);
    updateMetrics(result, processingTime);
    
    if (debugModeEnabled_) {
        printFusionReport(result);
    }
    
    return result;
}

AIResult MultiModalAISystem::analyzeVisual(const CameraFrame& frame, 
                                          const EnvironmentalContext& context) {
    AIResult result;
    
    if (!inferenceEngine_) {
        DEBUG_PRINTLN("ERROR: InferenceEngine not available");
        return result;
    }
    
    // Use inference engine for visual analysis
    // This is a placeholder - actual implementation would use the inference engine
    result.detected = false;
    result.confidence = 0.0f;
    result.detectedSpecies = SpeciesType::UNKNOWN;
    result.detectedBehavior = BehaviorType::UNKNOWN;
    
    return result;
}

AudioFeatures MultiModalAISystem::analyzeAudio(const float* audioData, size_t length) {
    AudioFeatures features;
    
    if (!audioData || length == 0) {
        return features;
    }
    
    // Extract audio features
    features = extractAudioFeatures(audioData, length);
    
    // Detect vocalization
    features.hasVocalization = detectVocalization(features);
    
    return features;
}

bool MultiModalAISystem::analyzeMotion(float motionLevel, 
                                      const EnvironmentalContext& context) {
    // Simple motion analysis
    float threshold = 0.3f;
    
    // Adjust threshold based on environmental conditions
    if (environmentalAdaptationEnabled_) {
        if (context.windSpeed_mps > 5.0f) {
            threshold += 0.2f; // Increase threshold in windy conditions
        }
    }
    
    return motionLevel > threshold;
}

float MultiModalAISystem::analyzeEnvironmentalContext(const EnvironmentalContext& context) {
    float suitability = 1.0f;
    
    // Adjust based on light levels
    if (context.lightLevel_lux < 10.0f) {
        suitability *= 0.7f; // Low light reduces visual reliability
    }
    
    // Adjust based on weather
    if (context.precipitation_mm > 0.5f) {
        suitability *= 0.8f; // Rain reduces detection quality
    }
    
    // Adjust based on wind
    if (context.windSpeed_mps > 10.0f) {
        suitability *= 0.9f; // Wind can cause false motion detections
    }
    
    return suitability;
}

// Sensor fusion
MultiModalResult MultiModalAISystem::fuseSensorData(
    const AIResult& visual,
    const AudioFeatures& audio,
    bool motionDetected,
    const EnvironmentalContext& environment) {
    
    MultiModalResult result;
    result.visualResult = visual;
    result.audioFeatures = audio;
    result.motionDetected = motionDetected;
    result.environment = environment;
    result.timestamp = millis();
    
    // Collect confidence scores from each modality
    std::vector<float> confidences;
    std::vector<float> weights;
    
    // Visual confidence
    if (visual.detected) {
        confidences.push_back(visual.confidence);
        weights.push_back(fusionConfig_.visualWeight);
        result.activeSensors.push_back(SensorModality::VISUAL);
    }
    
    // Audio confidence
    if (audio.hasVocalization && audioProcessingEnabled_) {
        confidences.push_back(audio.vocalizationConfidence);
        weights.push_back(fusionConfig_.audioWeight);
        result.activeSensors.push_back(SensorModality::AUDIO);
    }
    
    // Motion confidence
    if (motionDetected) {
        float motionConfidence = result.motionIntensity;
        confidences.push_back(motionConfidence);
        weights.push_back(fusionConfig_.motionWeight);
        result.activeSensors.push_back(SensorModality::MOTION);
    }
    
    // Calculate fused confidence using weighted average
    if (!confidences.empty()) {
        result.fusedConfidence = fuseBayesian(confidences, weights);
        result.overallConfidence = result.fusedConfidence;
    }
    
    // Fuse species detection
    std::vector<std::pair<SpeciesType, float>> speciesDetections;
    if (visual.detected) {
        speciesDetections.push_back({visual.detectedSpecies, visual.confidence});
    }
    if (audio.hasVocalization && result.audioSpeciesDetection != SpeciesType::UNKNOWN) {
        speciesDetections.push_back({result.audioSpeciesDetection, audio.vocalizationConfidence});
    }
    
    if (!speciesDetections.empty()) {
        result.fusedSpeciesDetection = fuseSpeciesClassifications(speciesDetections);
    }
    
    // Behavior analysis
    result.fusedBehaviorDetection = analyzeBehaviorMultiModal(result);
    
    // Threat detection
    result.threatDetected = detectThreatSituation(result);
    
    // Apply environmental context
    if (environmentalAdaptationEnabled_) {
        float envFactor = analyzeEnvironmentalContext(environment);
        result.overallConfidence *= envFactor;
    }
    
    return result;
}

// Advanced audio processing
bool MultiModalAISystem::enableAudioProcessing(bool enable) {
    audioProcessingEnabled_ = enable;
    DEBUG_PRINTLN(String("Audio processing ") + (enable ? "enabled" : "disabled"));
    return true;
}

AudioFeatures MultiModalAISystem::extractAudioFeatures(const float* audioData, size_t length) {
    AudioFeatures features;
    
    if (!audioData || length == 0) {
        return features;
    }
    
    // Calculate volume (RMS)
    float sum = 0.0f;
    for (size_t i = 0; i < length; i++) {
        sum += audioData[i] * audioData[i];
    }
    float rms = sqrt(sum / length);
    features.volume_dB = 20.0f * log10(rms + 1e-10f);
    
    // Compute spectrogram
    features.spectrogram = computeSpectrogram(audioData, length);
    
    // Compute MFCC
    if (!features.spectrogram.empty()) {
        features.mfccCoefficients = computeMFCC(features.spectrogram);
    }
    
    // Compute spectral features
    if (!features.spectrogram.empty()) {
        features.spectralCentroid = computeSpectralCentroid(features.spectrogram);
        features.spectralRolloff = computeSpectralRolloff(features.spectrogram);
    }
    
    // Zero crossing rate
    features.zeroCrossingRate = computeZeroCrossingRate(audioData, length);
    
    // Find dominant frequency (simplified)
    if (!features.spectrogram.empty()) {
        auto maxIt = std::max_element(features.spectrogram.begin(), features.spectrogram.end());
        size_t maxIdx = std::distance(features.spectrogram.begin(), maxIt);
        features.dominantFrequency_Hz = (maxIdx * audioConfig_.sampleRate_Hz) / 
                                       (2.0f * features.spectrogram.size());
    }
    
    return features;
}

std::vector<String> MultiModalAISystem::detectWildlifeSounds(const AudioFeatures& features) {
    std::vector<String> detectedSounds;
    
    // Simple heuristic-based detection
    // In production, this would use trained models
    
    // Bird calls (typically 2-8 kHz)
    if (features.dominantFrequency_Hz > 2000.0f && 
        features.dominantFrequency_Hz < 8000.0f &&
        features.volume_dB > -30.0f) {
        detectedSounds.push_back("bird_call");
    }
    
    // Mammal sounds (typically 200-2000 Hz)
    if (features.dominantFrequency_Hz > 200.0f && 
        features.dominantFrequency_Hz < 2000.0f &&
        features.volume_dB > -20.0f) {
        detectedSounds.push_back("mammal_vocalization");
    }
    
    // Environmental sounds
    if (features.spectralFlux > 0.5f) {
        detectedSounds.push_back("movement");
    }
    
    return detectedSounds;
}

SpeciesType MultiModalAISystem::classifyAudioSpecies(const AudioFeatures& features) {
    // Simplified species classification based on audio features
    // In production, this would use a trained CNN model
    
    if (!features.hasVocalization) {
        return SpeciesType::UNKNOWN;
    }
    
    // Example heuristics (would be replaced with ML model)
    if (features.dominantFrequency_Hz > 3000.0f && features.dominantFrequency_Hz < 5000.0f) {
        return SpeciesType::RED_TAILED_HAWK;
    }
    
    if (features.dominantFrequency_Hz > 1000.0f && features.dominantFrequency_Hz < 2000.0f) {
        return SpeciesType::WHITE_TAILED_DEER;
    }
    
    return SpeciesType::UNKNOWN;
}

bool MultiModalAISystem::detectVocalization(const AudioFeatures& features) {
    // Simple VAD (Voice Activity Detection)
    bool hasEnergy = features.volume_dB > -40.0f;
    bool hasVariation = features.spectralFlux > 0.3f;
    bool inFrequencyRange = features.dominantFrequency_Hz > 100.0f && 
                           features.dominantFrequency_Hz < 12000.0f;
    
    if (hasEnergy && hasVariation && inFrequencyRange) {
        // Calculate confidence based on features
        float confidence = 0.0f;
        confidence += (features.volume_dB + 40.0f) / 60.0f * 0.4f; // Volume contribution
        confidence += features.spectralFlux * 0.3f; // Spectral variation
        confidence += std::min(features.zeroCrossingRate / 0.5f, 1.0f) * 0.3f;
        
        return confidence > 0.5f;
    }
    
    return false;
}

// Environmental intelligence
void MultiModalAISystem::enableEnvironmentalAdaptation(bool enable) {
    environmentalAdaptationEnabled_ = enable;
    DEBUG_PRINTLN(String("Environmental adaptation ") + (enable ? "enabled" : "disabled"));
}

float MultiModalAISystem::calculateEnvironmentalSuitability(
    SpeciesType species,
    const EnvironmentalContext& context) {
    
    float suitability = 1.0f;
    
    // Calculate seasonal factor
    suitability *= calculateSeasonalFactor(species, context.dayOfYear);
    
    // Calculate time of day factor
    suitability *= calculateTimeOfDayFactor(species, context.timeOfDay);
    
    // Calculate weather factor
    suitability *= calculateWeatherFactor(species, context);
    
    return suitability;
}

std::vector<SpeciesType> MultiModalAISystem::predictActiveSpecies(
    const EnvironmentalContext& context) {
    
    std::vector<SpeciesType> activeSpecies;
    
    // Check each known species
    for (const auto& species : knownSpeciesProfiles_) {
        float suitability = calculateEnvironmentalSuitability(species, context);
        
        if (suitability > 0.5f) {
            activeSpecies.push_back(species);
        }
    }
    
    return activeSpecies;
}

float MultiModalAISystem::calculateBiodiversityIndex(
    const std::vector<SpeciesType>& detectedSpecies) {
    
    // Simple Shannon diversity index
    if (detectedSpecies.empty()) {
        return 0.0f;
    }
    
    // Count unique species
    std::map<SpeciesType, int> speciesCounts;
    for (const auto& species : detectedSpecies) {
        speciesCounts[species]++;
    }
    
    float index = 0.0f;
    int total = detectedSpecies.size();
    
    for (const auto& pair : speciesCounts) {
        float proportion = float(pair.second) / total;
        if (proportion > 0) {
            index -= proportion * log(proportion);
        }
    }
    
    return index;
}

// Behavioral analysis
BehaviorType MultiModalAISystem::analyzeBehaviorMultiModal(const MultiModalResult& result) {
    // Combine visual and audio cues for behavior detection
    
    if (result.visualResult.detected) {
        BehaviorType visualBehavior = result.visualResult.detectedBehavior;
        if (visualBehavior != BehaviorType::UNKNOWN) {
            return visualBehavior;
        }
    }
    
    // Infer from audio
    if (result.audioFeatures.hasVocalization) {
        return inferBehaviorFromAudio(result.audioFeatures);
    }
    
    // Infer from motion
    if (result.motionDetected) {
        return inferBehaviorFromMotion(result.motionIntensity, result.motionPattern);
    }
    
    return BehaviorType::UNKNOWN;
}

float MultiModalAISystem::calculateStressLevel(const MultiModalResult& result) {
    float stressLevel = 0.0f;
    
    // High frequency vocalizations indicate stress
    if (result.audioFeatures.dominantFrequency_Hz > 5000.0f) {
        stressLevel += 0.3f;
    }
    
    // Rapid movements indicate stress
    if (result.motionIntensity > 0.8f) {
        stressLevel += 0.3f;
    }
    
    // High vocalization rate
    if (result.audioFeatures.volume_dB > -10.0f) {
        stressLevel += 0.2f;
    }
    
    // Environmental stressors
    if (result.environment.temperature_C > 35.0f || result.environment.temperature_C < -10.0f) {
        stressLevel += 0.2f;
    }
    
    return std::min(stressLevel, 1.0f);
}

bool MultiModalAISystem::detectThreatSituation(const MultiModalResult& result) {
    // Detect potential threats
    
    // High stress level
    if (result.stressLevel > 0.7f) {
        return true;
    }
    
    // Alarm calls (high frequency, loud)
    if (result.audioFeatures.dominantFrequency_Hz > 6000.0f && 
        result.audioFeatures.volume_dB > -15.0f) {
        return true;
    }
    
    // Rapid escape behavior
    if (result.fusedBehaviorDetection == BehaviorType::FLEEING) {
        return true;
    }
    
    return false;
}

std::vector<String> MultiModalAISystem::identifyBehaviorIndicators(
    const MultiModalResult& result) {
    
    std::vector<String> indicators;
    
    if (result.audioFeatures.hasVocalization) {
        indicators.push_back("vocalization_present");
    }
    
    if (result.motionDetected) {
        indicators.push_back("motion_detected");
    }
    
    if (result.audioFeatures.volume_dB > -20.0f) {
        indicators.push_back("high_audio_activity");
    }
    
    if (result.visualConfidence > 0.7f && result.audioConfidence > 0.7f) {
        indicators.push_back("multi_modal_confirmation");
    }
    
    return indicators;
}

// Temporal analysis
void MultiModalAISystem::enableTemporalAnalysis(bool enable) {
    temporalAnalysisEnabled_ = enable;
    DEBUG_PRINTLN(String("Temporal analysis ") + (enable ? "enabled" : "disabled"));
}

void MultiModalAISystem::updateTemporalHistory(const MultiModalResult& result) {
    temporalHistory_.push_back(result);
    
    // Limit history size
    if (temporalHistory_.size() > MAX_TEMPORAL_HISTORY) {
        temporalHistory_.erase(temporalHistory_.begin());
    }
}

std::vector<MultiModalResult> MultiModalAISystem::getTemporalPattern(uint32_t timeWindow_ms) {
    std::vector<MultiModalResult> pattern;
    unsigned long currentTime = millis();
    
    for (const auto& result : temporalHistory_) {
        if (currentTime - result.timestamp <= timeWindow_ms) {
            pattern.push_back(result);
        }
    }
    
    return pattern;
}

BehaviorType MultiModalAISystem::predictNextBehavior(
    const std::vector<MultiModalResult>& history) {
    
    if (history.empty()) {
        return BehaviorType::UNKNOWN;
    }
    
    // Simple Markov chain prediction based on most recent behavior
    return history.back().fusedBehaviorDetection;
}

// Configuration and optimization
void MultiModalAISystem::optimizeFusionWeights() {
    // Placeholder for adaptive weight optimization
    // Would analyze historical performance and adjust weights
    DEBUG_PRINTLN("Optimizing fusion weights...");
}

void MultiModalAISystem::adaptToEnvironmentalConditions() {
    // Placeholder for environmental adaptation
    DEBUG_PRINTLN("Adapting to environmental conditions...");
}

void MultiModalAISystem::updateModalityWeights(const SensorFusionConfig& newWeights) {
    fusionConfig_ = newWeights;
    DEBUG_PRINTLN("Modality weights updated");
}

SensorFusionConfig MultiModalAISystem::getOptimalFusionConfig() {
    return fusionConfig_;
}

// Performance monitoring
MultiModalAISystem::MultiModalMetrics MultiModalAISystem::getPerformanceMetrics() const {
    return metrics_;
}

void MultiModalAISystem::resetMetrics() {
    metrics_ = MultiModalMetrics();
    DEBUG_PRINTLN("Metrics reset");
}

void MultiModalAISystem::printSystemStatus() {
    DEBUG_PRINTLN("=== MultiModal AI System Status ===");
    DEBUG_PRINTLN(String("Initialized: ") + (initialized_ ? "YES" : "NO"));
    DEBUG_PRINTLN(String("Audio Processing: ") + (audioProcessingEnabled_ ? "ON" : "OFF"));
    DEBUG_PRINTLN(String("Total Analyses: ") + metrics_.totalAnalyses);
    DEBUG_PRINTLN(String("Successful Fusions: ") + metrics_.successfulFusions);
    DEBUG_PRINTLN(String("Avg Processing Time: ") + metrics_.averageProcessingTime_ms + " ms");
    DEBUG_PRINTLN(String("Fusion Accuracy: ") + metrics_.fusionAccuracy_percent + "%");
    DEBUG_PRINTLN("====================================");
}

// Debug and diagnostics
void MultiModalAISystem::enableDebugMode(bool enable) {
    debugModeEnabled_ = enable;
    DEBUG_PRINTLN(String("Debug mode ") + (enable ? "enabled" : "disabled"));
}

void MultiModalAISystem::exportAnalysisData(const char* filename) {
    // Placeholder for data export
    DEBUG_PRINTLN(String("Exporting analysis data to: ") + filename);
}

void MultiModalAISystem::printFusionReport(const MultiModalResult& result) {
    DEBUG_PRINTLN("=== Fusion Report ===");
    DEBUG_PRINTLN(String("Visual Confidence: ") + result.visualConfidence);
    DEBUG_PRINTLN(String("Audio Confidence: ") + result.audioConfidence);
    DEBUG_PRINTLN(String("Fused Confidence: ") + result.fusedConfidence);
    DEBUG_PRINTLN(String("Overall Confidence: ") + result.overallConfidence);
    DEBUG_PRINTLN(String("Motion Detected: ") + (result.motionDetected ? "YES" : "NO"));
    DEBUG_PRINTLN(String("Threat Detected: ") + (result.threatDetected ? "YES" : "NO"));
    DEBUG_PRINTLN("====================");
}

bool MultiModalAISystem::validateSensorCalibration() {
    // Placeholder for sensor calibration validation
    DEBUG_PRINTLN("Validating sensor calibration...");
    return true;
}

// Private methods - Audio processing
bool MultiModalAISystem::preprocessAudioData(const float* audioData, size_t length) {
    if (!audioData || length == 0) {
        return false;
    }
    
    // Simple preprocessing: normalization
    float maxVal = 0.0f;
    for (size_t i = 0; i < length; i++) {
        maxVal = std::max(maxVal, fabs(audioData[i]));
    }
    
    if (maxVal > 0.0f) {
        audioBuffer_.resize(length);
        for (size_t i = 0; i < length; i++) {
            audioBuffer_[i] = audioData[i] / maxVal;
        }
    }
    
    return true;
}

std::vector<float> MultiModalAISystem::computeSpectrogram(const float* audioData, size_t length) {
    std::vector<float> spectrogram;
    
    // Simplified spectrogram computation
    // In production, would use proper FFT
    size_t numFrames = length / audioConfig_.hopSize;
    spectrogram.reserve(numFrames * audioConfig_.fftSize / 2);
    
    for (size_t frame = 0; frame < numFrames; frame++) {
        size_t offset = frame * audioConfig_.hopSize;
        
        // Compute magnitude spectrum for this frame
        for (size_t i = 0; i < audioConfig_.fftSize / 2 && (offset + i) < length; i++) {
            float val = audioData[offset + i];
            spectrogram.push_back(fabs(val));
        }
    }
    
    return spectrogram;
}

std::vector<float> MultiModalAISystem::computeMFCC(const std::vector<float>& spectrogram) {
    std::vector<float> mfcc;
    
    // Simplified MFCC computation
    // In production, would use proper mel filterbank and DCT
    mfcc.reserve(audioConfig_.numMfccCoefs);
    
    for (size_t i = 0; i < audioConfig_.numMfccCoefs && i < spectrogram.size(); i++) {
        float sum = 0.0f;
        for (size_t j = 0; j < std::min(size_t(10), spectrogram.size()); j++) {
            sum += spectrogram[j] * cos(M_PI * i * (j + 0.5f) / 10.0f);
        }
        mfcc.push_back(sum);
    }
    
    return mfcc;
}

float MultiModalAISystem::computeSpectralCentroid(const std::vector<float>& spectrogram) {
    if (spectrogram.empty()) {
        return 0.0f;
    }
    
    float weightedSum = 0.0f;
    float totalPower = 0.0f;
    
    for (size_t i = 0; i < spectrogram.size(); i++) {
        float power = spectrogram[i] * spectrogram[i];
        weightedSum += i * power;
        totalPower += power;
    }
    
    return totalPower > 0.0f ? weightedSum / totalPower : 0.0f;
}

float MultiModalAISystem::computeSpectralRolloff(const std::vector<float>& spectrogram) {
    if (spectrogram.empty()) {
        return 0.0f;
    }
    
    float totalEnergy = 0.0f;
    for (float val : spectrogram) {
        totalEnergy += val * val;
    }
    
    float threshold = 0.85f * totalEnergy;
    float cumulativeEnergy = 0.0f;
    
    for (size_t i = 0; i < spectrogram.size(); i++) {
        cumulativeEnergy += spectrogram[i] * spectrogram[i];
        if (cumulativeEnergy >= threshold) {
            return float(i) / spectrogram.size();
        }
    }
    
    return 1.0f;
}

float MultiModalAISystem::computeZeroCrossingRate(const float* audioData, size_t length) {
    if (!audioData || length < 2) {
        return 0.0f;
    }
    
    int crossings = 0;
    for (size_t i = 1; i < length; i++) {
        if ((audioData[i] >= 0.0f && audioData[i-1] < 0.0f) ||
            (audioData[i] < 0.0f && audioData[i-1] >= 0.0f)) {
            crossings++;
        }
    }
    
    return float(crossings) / length;
}

// Sensor fusion algorithms
float MultiModalAISystem::calculateAdaptiveWeight(SensorModality modality,
                                                  const EnvironmentalContext& context) {
    float weight = 1.0f;
    
    switch (modality) {
        case SensorModality::VISUAL:
            // Reduce visual weight in low light
            if (context.lightLevel_lux < 10.0f) {
                weight *= 0.5f;
            }
            break;
            
        case SensorModality::AUDIO:
            // Reduce audio weight in high wind
            if (context.windSpeed_mps > 10.0f) {
                weight *= 0.7f;
            }
            break;
            
        case SensorModality::MOTION:
            // Reduce motion weight in high wind or rain
            if (context.windSpeed_mps > 5.0f || context.precipitation_mm > 0.5f) {
                weight *= 0.6f;
            }
            break;
            
        default:
            break;
    }
    
    return weight;
}

float MultiModalAISystem::fuseBayesian(const std::vector<float>& modalityConfidences,
                                      const std::vector<float>& modalityWeights) {
    if (modalityConfidences.empty() || modalityWeights.empty()) {
        return 0.0f;
    }
    
    // Weighted average fusion
    float weightedSum = 0.0f;
    float totalWeight = 0.0f;
    
    size_t count = std::min(modalityConfidences.size(), modalityWeights.size());
    for (size_t i = 0; i < count; i++) {
        weightedSum += modalityConfidences[i] * modalityWeights[i];
        totalWeight += modalityWeights[i];
    }
    
    return totalWeight > 0.0f ? weightedSum / totalWeight : 0.0f;
}

float MultiModalAISystem::fuseDempsterShafer(const std::vector<float>& modalityEvidence) {
    // Simplified Dempster-Shafer fusion
    if (modalityEvidence.empty()) {
        return 0.0f;
    }
    
    float combinedBelief = modalityEvidence[0];
    
    for (size_t i = 1; i < modalityEvidence.size(); i++) {
        float belief = modalityEvidence[i];
        float conflict = combinedBelief * (1.0f - belief) + belief * (1.0f - combinedBelief);
        
        if (conflict < 1.0f) {
            combinedBelief = (combinedBelief * belief) / (1.0f - conflict);
        }
    }
    
    return combinedBelief;
}

SpeciesType MultiModalAISystem::fuseSpeciesClassifications(
    const std::vector<std::pair<SpeciesType, float>>& detections) {
    
    if (detections.empty()) {
        return SpeciesType::UNKNOWN;
    }
    
    // Find detection with highest confidence
    auto maxIt = std::max_element(detections.begin(), detections.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return maxIt->first;
}

// Environmental analysis helpers
float MultiModalAISystem::calculateSeasonalFactor(SpeciesType species, uint8_t dayOfYear) {
    // Simplified seasonal activity
    // Would use species-specific migration and activity patterns
    
    // Spring/Summer more active (day 80-260)
    if (dayOfYear >= 80 && dayOfYear <= 260) {
        return 1.0f;
    }
    
    // Fall/Winter less active
    return 0.6f;
}

float MultiModalAISystem::calculateTimeOfDayFactor(SpeciesType species, uint8_t hour) {
    // Many wildlife species are crepuscular (dawn/dusk active)
    
    // Dawn (5-8) and Dusk (17-20) hours
    if ((hour >= 5 && hour <= 8) || (hour >= 17 && hour <= 20)) {
        return 1.0f;
    }
    
    // Daytime
    if (hour >= 9 && hour <= 16) {
        return 0.7f;
    }
    
    // Nighttime
    return 0.5f;
}

float MultiModalAISystem::calculateWeatherFactor(SpeciesType species,
                                                 const EnvironmentalContext& context) {
    float factor = 1.0f;
    
    // Reduce activity in extreme weather
    if (context.precipitation_mm > 5.0f) {
        factor *= 0.5f; // Heavy rain
    } else if (context.precipitation_mm > 0.5f) {
        factor *= 0.8f; // Light rain
    }
    
    if (context.windSpeed_mps > 15.0f) {
        factor *= 0.6f; // High wind
    }
    
    if (context.temperature_C < -10.0f || context.temperature_C > 40.0f) {
        factor *= 0.7f; // Extreme temperatures
    }
    
    return factor;
}

// Behavior analysis helpers
BehaviorType MultiModalAISystem::inferBehaviorFromMotion(float motionIntensity,
                                                         const String& motionPattern) {
    if (motionIntensity < 0.3f) {
        return BehaviorType::RESTING;
    }
    
    if (motionIntensity > 0.8f) {
        return BehaviorType::FLEEING;
    }
    
    if (motionPattern == "repetitive") {
        return BehaviorType::FORAGING;
    }
    
    return BehaviorType::MOVING;
}

BehaviorType MultiModalAISystem::inferBehaviorFromAudio(const AudioFeatures& features) {
    // High frequency, repetitive calls suggest alarm
    if (features.dominantFrequency_Hz > 5000.0f && features.volume_dB > -15.0f) {
        return BehaviorType::ALERT;
    }
    
    // Low frequency suggests feeding or socializing
    if (features.dominantFrequency_Hz < 1000.0f) {
        return BehaviorType::FORAGING;
    }
    
    return BehaviorType::UNKNOWN;
}

float MultiModalAISystem::calculateActivityLevel(const MultiModalResult& result) {
    float activity = 0.0f;
    
    activity += result.motionIntensity * 0.4f;
    activity += (result.audioFeatures.volume_dB + 60.0f) / 60.0f * 0.3f;
    activity += result.visualConfidence * 0.3f;
    
    return std::min(activity, 1.0f);
}

bool MultiModalAISystem::detectSocialBehavior(const MultiModalResult& result) {
    // Multiple vocalizations suggest social interaction
    return result.detectedSounds.size() > 2;
}

// Utility functions
void MultiModalAISystem::updateMetrics(const MultiModalResult& result, float processingTime) {
    metrics_.totalAnalyses++;
    
    if (result.fusedConfidence > MIN_FUSION_CONFIDENCE) {
        metrics_.successfulFusions++;
    }
    
    // Update average processing time
    float alpha = 0.1f; // Smoothing factor
    metrics_.averageProcessingTime_ms = 
        alpha * processingTime + (1.0f - alpha) * metrics_.averageProcessingTime_ms;
    
    // Update fusion accuracy
    if (metrics_.totalAnalyses > 0) {
        metrics_.fusionAccuracy_percent = 
            100.0f * metrics_.successfulFusions / metrics_.totalAnalyses;
    }
}

void MultiModalAISystem::cleanupTemporalHistory() {
    temporalHistory_.clear();
}

bool MultiModalAISystem::isValidSensorData(SensorModality modality, const void* data) {
    return data != nullptr;
}

void MultiModalAISystem::logAnalysisEvent(const String& event, const MultiModalResult& result) {
    if (debugModeEnabled_) {
        DEBUG_PRINTLN(String("Event: ") + event + 
                     " | Confidence: " + result.fusedConfidence);
    }
}

// Utility functions (global)
const char* sensorModalityToString(SensorModality modality) {
    switch (modality) {
        case SensorModality::VISUAL: return "Visual";
        case SensorModality::AUDIO: return "Audio";
        case SensorModality::MOTION: return "Motion";
        case SensorModality::ENVIRONMENTAL: return "Environmental";
        case SensorModality::VIBRATION: return "Vibration";
        case SensorModality::MAGNETIC: return "Magnetic";
        case SensorModality::THERMAL: return "Thermal";
        default: return "Unknown";
    }
}

float calculateModalityReliability(SensorModality modality, const EnvironmentalContext& context) {
    float reliability = 1.0f;
    
    switch (modality) {
        case SensorModality::VISUAL:
            if (context.lightLevel_lux < 10.0f) reliability *= 0.5f;
            break;
        case SensorModality::AUDIO:
            if (context.windSpeed_mps > 10.0f) reliability *= 0.6f;
            break;
        case SensorModality::MOTION:
            if (context.windSpeed_mps > 5.0f) reliability *= 0.7f;
            break;
        default:
            break;
    }
    
    return reliability;
}

bool isCompatibleSpeciesDetection(SpeciesType visual, SpeciesType audio) {
    // Check if visual and audio detections are compatible
    if (visual == audio) {
        return true;
    }
    
    // Both unknown
    if (visual == SpeciesType::UNKNOWN || audio == SpeciesType::UNKNOWN) {
        return true;
    }
    
    // Same family (simplified - would use taxonomy)
    return false;
}

String generateMultiModalSummary(const MultiModalResult& result) {
    String summary = "MultiModal Detection Summary:\n";
    summary += "  Visual: " + String(result.visualConfidence, 2) + "\n";
    summary += "  Audio: " + String(result.audioConfidence, 2) + "\n";
    summary += "  Fused: " + String(result.fusedConfidence, 2) + "\n";
    summary += "  Species: " + String(int(result.fusedSpeciesDetection)) + "\n";
    summary += "  Behavior: " + String(int(result.fusedBehaviorDetection)) + "\n";
    
    return summary;
}
