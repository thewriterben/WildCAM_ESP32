/**
 * @file raptor_detection.h
 * @brief AI Models for Raptor-Specific Detection and Classification
 * 
 * Implements TensorFlow Lite models specialized for bird of prey identification,
 * species classification, and behavior recognition optimized for ESP32 hardware.
 */

#ifndef RAPTOR_DETECTION_H
#define RAPTOR_DETECTION_H

#include "../ai/ai_common.h"
#include "../ai/tinyml/inference_engine.h"
#include "../scenarios/raptor_configs.h"
#include <vector>
#include <memory>

/**
 * Raptor Detection Model Types
 */
enum class RaptorModelType {
    RAPTOR_CLASSIFIER,          // Basic raptor vs non-raptor classification
    SPECIES_IDENTIFIER,         // Species-specific identification
    BEHAVIOR_ANALYZER,          // Behavior pattern recognition
    SIZE_ESTIMATOR,             // Size and biometric estimation
    FLIGHT_PATTERN_ANALYZER,    // Flight characteristics analysis
    ACOUSTIC_CLASSIFIER         // Audio-based raptor call classification
};

/**
 * Raptor Model Configuration
 */
struct RaptorModelConfig {
    RaptorModelType modelType;
    String modelPath;
    uint32_t modelSize_KB;
    uint32_t inputDimensions[3];        // [width, height, channels]
    uint32_t outputClasses;
    float quantizationScale;
    int8_t quantizationZeroPoint;
    uint32_t inferenceTime_ms;
    float accuracy_percent;
    bool requiresPSRAM;
    
    RaptorModelConfig(RaptorModelType type, const String& path) 
        : modelType(type), modelPath(path), modelSize_KB(0), outputClasses(0),
          quantizationScale(1.0f), quantizationZeroPoint(0), 
          inferenceTime_ms(0), accuracy_percent(0.0f), requiresPSRAM(false) {
        inputDimensions[0] = inputDimensions[1] = inputDimensions[2] = 0;
    }
};

/**
 * Raptor Species Detection Results
 */
struct RaptorSpeciesResult {
    RaptorSpecies species;
    float confidence;
    String scientificName;
    String commonName;
    
    // Morphological characteristics
    struct {
        float estimatedWingspan_cm;
        float estimatedLength_cm;
        float estimatedWeight_g;
        String billShape;
        String tailShape;
        String wingShape;
    } morphology;
    
    // Conservation status
    String conservationStatus;      // "LC", "NT", "VU", "EN", "CR"
    bool isProtectedSpecies;
    bool requiresSpecialHandling;
    
    RaptorSpeciesResult() : species(RaptorSpecies::UNKNOWN_RAPTOR), confidence(0.0f),
                           isProtectedSpecies(false), requiresSpecialHandling(false) {}
};

/**
 * Flight Pattern Analysis Result
 */
struct FlightPatternResult {
    String patternType;             // "soaring", "gliding", "diving", "hovering", "flapping"
    float wingbeatFrequency_Hz;
    float glideRatio;
    float thermalUtilization;
    bool migratoryBehavior;
    
    // Movement characteristics
    struct {
        float velocity_mps;
        float acceleration_mps2;
        float turnRadius_m;
        float climbRate_mps;
        float altitude_m;
    } kinematics;
    
    // Environmental interaction
    bool usingThermals;
    bool ridgeSoaring;
    bool windUtilization;
    float energyEfficiency;
    
    FlightPatternResult() : wingbeatFrequency_Hz(0.0f), glideRatio(0.0f),
                           thermalUtilization(0.0f), migratoryBehavior(false),
                           usingThermals(false), ridgeSoaring(false),
                           windUtilization(false), energyEfficiency(0.0f) {}
};

/**
 * Acoustic Detection Result for Raptor Calls
 */
struct RaptorAcousticResult {
    bool callDetected;
    RaptorSpecies likelySpecies;
    String callType;                // "alarm", "territorial", "mating", "hunting", "distress"
    
    // Audio characteristics
    struct {
        float frequency_Hz;
        float duration_ms;
        float amplitude_dB;
        float harmonics[5];         // First 5 harmonic frequencies
        String syllablePattern;
    } acousticFeatures;
    
    // Behavioral context
    bool aggressionIndicator;
    bool territorialDisplay;
    bool matingCall;
    bool distressCall;
    float urgencyLevel;
    
    // Quality metrics
    float signalToNoise_dB;
    float confidence;
    bool backgroundNoise;
    
    RaptorAcousticResult() : callDetected(false), likelySpecies(RaptorSpecies::UNKNOWN_RAPTOR),
                            aggressionIndicator(false), territorialDisplay(false),
                            matingCall(false), distressCall(false), urgencyLevel(0.0f),
                            signalToNoise_dB(0.0f), confidence(0.0f), backgroundNoise(false) {}
};

/**
 * Main Raptor Detection Class
 */
class RaptorDetection {
public:
    /**
     * Constructor
     */
    RaptorDetection();
    
    /**
     * Destructor
     */
    ~RaptorDetection();
    
    /**
     * Initialize raptor detection system
     * @param scenario Target monitoring scenario
     * @return true if initialization successful
     */
    bool init(RaptorScenario scenario);
    
    /**
     * Load specific model for detection
     * @param modelType Type of model to load
     * @return true if model loaded successfully
     */
    bool loadModel(RaptorModelType modelType);
    
    /**
     * Detect raptors in image
     * @param imageData Input image data
     * @param width Image width
     * @param height Image height
     * @param channels Image channels (1 or 3)
     * @return true if raptor detected
     */
    bool detectRaptor(const uint8_t* imageData, uint32_t width, uint32_t height, uint8_t channels);
    
    /**
     * Classify raptor species
     * @param imageData Input image data (cropped raptor region)
     * @param width Image width
     * @param height Image height
     * @return Species classification result
     */
    RaptorSpeciesResult classifySpecies(const uint8_t* imageData, uint32_t width, uint32_t height);
    
    /**
     * Analyze flight pattern from sequence
     * @param detectionSequence Vector of recent detections
     * @return Flight pattern analysis
     */
    FlightPatternResult analyzeFlightPattern(const std::vector<struct RaptorDetectionResult>& detectionSequence);
    
    /**
     * Process audio for raptor calls
     * @param audioData Audio sample data
     * @param sampleCount Number of audio samples
     * @param sampleRate Sample rate in Hz
     * @return Acoustic analysis result
     */
    RaptorAcousticResult processAudio(const int16_t* audioData, uint32_t sampleCount, uint32_t sampleRate);
    
    /**
     * Get current detection confidence threshold
     */
    float getConfidenceThreshold() const { return confidenceThreshold; }
    
    /**
     * Set detection confidence threshold
     * @param threshold Minimum confidence (0.0-1.0)
     */
    void setConfidenceThreshold(float threshold);
    
    /**
     * Get model performance statistics
     */
    struct ModelPerformance {
        uint32_t inferenceCount;
        float averageInferenceTime_ms;
        float averageConfidence;
        uint32_t cacheHitRate_percent;
        float memoryUsage_KB;
        float powerConsumption_mW;
    } getPerformanceStats() const;
    
    /**
     * Enable/disable model caching for performance
     */
    void enableModelCaching(bool enable);
    
    /**
     * Calibrate detection for specific environment
     * @param calibrationImages Vector of background images
     * @return true if calibration successful
     */
    bool calibrateEnvironment(const std::vector<uint8_t*>& calibrationImages);
    
    /**
     * Get supported raptor species list
     */
    std::vector<RaptorSpecies> getSupportedSpecies() const;
    
    /**
     * Check if model is loaded and ready
     */
    bool isReady() const { return initialized && modelLoaded; }

private:
    // Core system state
    bool initialized;
    bool modelLoaded;
    RaptorScenario currentScenario;
    float confidenceThreshold;
    
    // Model management
    std::unique_ptr<InferenceEngine> inferenceEngine;
    std::vector<RaptorModelConfig> availableModels;
    RaptorModelType currentModelType;
    
    // Performance optimization
    bool cachingEnabled;
    uint32_t inferenceCount;
    float totalInferenceTime;
    float totalConfidence;
    uint32_t cacheHits;
    uint32_t cacheTotal;
    
    // Model configurations
    static const RaptorModelConfig RAPTOR_MODELS[];
    static const size_t RAPTOR_MODEL_COUNT;
    
    // Image preprocessing
    bool preprocessImage(const uint8_t* inputImage, uint32_t width, uint32_t height, 
                        uint8_t channels, uint8_t* outputBuffer, uint32_t targetWidth, 
                        uint32_t targetHeight);
    
    // Feature extraction methods
    std::vector<float> extractVisualFeatures(const uint8_t* imageData, uint32_t width, uint32_t height);
    std::vector<float> extractAudioFeatures(const int16_t* audioData, uint32_t sampleCount, uint32_t sampleRate);
    std::vector<float> extractMotionFeatures(const std::vector<struct RaptorDetectionResult>& sequence);
    
    // Post-processing methods
    RaptorSpeciesResult processSpeciesOutput(const float* modelOutput, uint32_t outputSize);
    FlightPatternResult processFlightOutput(const float* modelOutput, uint32_t outputSize);
    RaptorAcousticResult processAudioOutput(const float* modelOutput, uint32_t outputSize);
    
    // Model loading helpers
    bool initializeModelFromFile(const String& modelPath);
    bool validateModelCompatibility(const RaptorModelConfig& config);
    void optimizeModelForHardware();
    
    // Performance optimization
    void updatePerformanceMetrics(float inferenceTime, float confidence);
    bool shouldUseCache(const uint8_t* imageData) const;
    void cacheResult(const uint8_t* imageData, const float* result);
    bool getCachedResult(const uint8_t* imageData, float* result);
    
    // Environmental adaptation
    void adaptToLightingConditions(uint8_t* imageData, uint32_t size);
    void adaptToWeatherConditions(uint8_t* imageData, uint32_t size);
    float calculateEnvironmentalFactor() const;
    
    // Species-specific processing
    bool isEagleCandidate(const float* features) const;
    bool isHawkCandidate(const float* features) const;
    bool isFalconCandidate(const float* features) const;
    
    // Audio processing helpers
    void computeSpectogram(const int16_t* audioData, uint32_t sampleCount, float* spectrogram);
    void extractMFCC(const float* spectrogram, float* mfccCoeffs);
    bool detectCallPattern(const float* audioFeatures) const;
    
    // Utility methods
    uint32_t calculateImageHash(const uint8_t* imageData, uint32_t size) const;
    float calculateImageSimilarity(const uint8_t* img1, const uint8_t* img2, uint32_t size) const;
    void logDetectionResult(const RaptorSpeciesResult& result) const;
    void cleanup();
};

/**
 * Raptor Model Factory for creating optimized model instances
 */
class RaptorModelFactory {
public:
    /**
     * Create optimized model configuration for scenario
     */
    static std::vector<RaptorModelConfig> createOptimizedModels(RaptorScenario scenario);
    
    /**
     * Get recommended models for hardware configuration
     */
    static std::vector<RaptorModelType> getRecommendedModels(bool hasPSRAM, uint32_t flashSize_MB);
    
    /**
     * Load pre-trained model from assets
     */
    static bool loadPretrainedModel(RaptorModelType modelType, const String& assetPath);
    
    /**
     * Validate model performance requirements
     */
    static bool validatePerformanceRequirements(const RaptorModelConfig& config, 
                                               uint32_t availableRAM_KB, 
                                               uint32_t targetInferenceTime_ms);
};

#endif // RAPTOR_DETECTION_H