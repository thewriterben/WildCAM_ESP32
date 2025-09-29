/**
 * @file multimodal_ai_system.h
 * @brief Multi-Modal AI System for Enhanced Wildlife Detection
 * 
 * Integrates visual, audio, and environmental sensor data for comprehensive
 * wildlife detection and analysis with sensor fusion capabilities.
 */

#ifndef MULTIMODAL_AI_SYSTEM_H
#define MULTIMODAL_AI_SYSTEM_H

#include "ai_common.h"
#include "tinyml/inference_engine.h"
#include <vector>
#include <memory>

/**
 * Sensor Modalities
 */
enum class SensorModality {
    VISUAL,         // Camera image data
    AUDIO,          // Microphone audio data
    MOTION,         // PIR motion sensor
    ENVIRONMENTAL,  // Temperature, humidity, pressure
    VIBRATION,      // Seismic/vibration sensors
    MAGNETIC,       // Magnetometer for metal detection
    THERMAL         // Thermal imaging (if available)
};

/**
 * Audio Features for Wildlife Detection
 */
struct AudioFeatures {
    float volume_dB;
    float dominantFrequency_Hz;
    std::vector<float> spectrogram;
    std::vector<float> mfccCoefficients;
    float spectralCentroid;
    float spectralRolloff;
    float zeroCrossingRate;
    float spectralFlux;
    bool hasVocalization;
    float vocalizationConfidence;
    
    AudioFeatures() : volume_dB(0.0f), dominantFrequency_Hz(0.0f),
                     spectralCentroid(0.0f), spectralRolloff(0.0f),
                     zeroCrossingRate(0.0f), spectralFlux(0.0f),
                     hasVocalization(false), vocalizationConfidence(0.0f) {}
};

/**
 * Environmental Context
 */
struct EnvironmentalContext {
    float temperature_C;
    float humidity_percent;
    float pressure_hPa;
    float lightLevel_lux;
    float windSpeed_mps;
    float precipitation_mm;
    float vibrationLevel;
    float magneticField_uT;
    uint8_t timeOfDay; // 0-23 hours
    uint8_t dayOfYear; // 1-365
    String weatherCondition;
    
    EnvironmentalContext() : temperature_C(25.0f), humidity_percent(50.0f),
                           pressure_hPa(1013.25f), lightLevel_lux(1000.0f),
                           windSpeed_mps(0.0f), precipitation_mm(0.0f),
                           vibrationLevel(0.0f), magneticField_uT(50.0f),
                           timeOfDay(12), dayOfYear(180) {}
};

/**
 * Multi-Modal Detection Result
 */
struct MultiModalResult {
    // Visual analysis
    AIResult visualResult;
    float visualConfidence;
    
    // Audio analysis
    AudioFeatures audioFeatures;
    SpeciesType audioSpeciesDetection;
    float audioConfidence;
    std::vector<String> detectedSounds;
    
    // Motion analysis
    bool motionDetected;
    float motionIntensity;
    String motionPattern;
    
    // Environmental context
    EnvironmentalContext environment;
    
    // Fused results
    SpeciesType fusedSpeciesDetection;
    BehaviorType fusedBehaviorDetection;
    float fusedConfidence;
    float overallConfidence;
    
    // Additional insights
    std::vector<String> behaviorIndicators;
    float stressLevel;
    bool threatDetected;
    bool humanPresenceDetected;
    float biodiversityIndex;
    
    // Metadata
    unsigned long timestamp;
    std::vector<SensorModality> activeSensors;
    String analysisNotes;
    
    MultiModalResult() : visualConfidence(0.0f),
                        audioSpeciesDetection(SpeciesType::UNKNOWN),
                        audioConfidence(0.0f),
                        motionDetected(false),
                        motionIntensity(0.0f),
                        fusedSpeciesDetection(SpeciesType::UNKNOWN),
                        fusedBehaviorDetection(BehaviorType::UNKNOWN),
                        fusedConfidence(0.0f),
                        overallConfidence(0.0f),
                        stressLevel(0.0f),
                        threatDetected(false),
                        humanPresenceDetected(false),
                        biodiversityIndex(0.0f),
                        timestamp(0) {}
};

/**
 * Sensor Fusion Configuration
 */
struct SensorFusionConfig {
    std::map<SensorModality, float> modalityWeights;
    float visualWeight;
    float audioWeight;
    float motionWeight;
    float environmentalWeight;
    bool enableAdaptiveWeighting;
    bool enableContextualFusion;
    float fusionConfidenceThreshold;
    uint32_t temporalWindow_ms;
    
    SensorFusionConfig() : visualWeight(0.4f), audioWeight(0.3f),
                          motionWeight(0.2f), environmentalWeight(0.1f),
                          enableAdaptiveWeighting(true),
                          enableContextualFusion(true),
                          fusionConfidenceThreshold(0.6f),
                          temporalWindow_ms(5000) {}
};

/**
 * Audio Processing Configuration
 */
struct AudioProcessingConfig {
    uint32_t sampleRate_Hz;
    uint32_t fftSize;
    uint32_t hopSize;
    uint32_t numMelFilters;
    uint32_t numMfccCoefs;
    float preEmphasisCoef;
    float minFrequency_Hz;
    float maxFrequency_Hz;
    bool enableVAD; // Voice Activity Detection
    float vadThreshold;
    
    AudioProcessingConfig() : sampleRate_Hz(22050), fftSize(1024),
                             hopSize(512), numMelFilters(40),
                             numMfccCoefs(13), preEmphasisCoef(0.97f),
                             minFrequency_Hz(50.0f), maxFrequency_Hz(11000.0f),
                             enableVAD(true), vadThreshold(0.3f) {}
};

/**
 * Multi-Modal AI System
 * 
 * Integrates multiple sensor modalities for comprehensive wildlife
 * detection and analysis with advanced sensor fusion capabilities.
 */
class MultiModalAISystem {
public:
    MultiModalAISystem();
    ~MultiModalAISystem();

    // Core initialization
    bool init(InferenceEngine* inferenceEngine);
    void configure(const SensorFusionConfig& fusionConfig,
                  const AudioProcessingConfig& audioConfig);
    void cleanup();
    bool isInitialized() const;
    
    // Multi-modal analysis
    MultiModalResult analyzeMultiModal(const CameraFrame& visualData,
                                      const float* audioData,
                                      size_t audioLength,
                                      const EnvironmentalContext& environment);
    
    // Individual modality analysis
    AIResult analyzeVisual(const CameraFrame& frame, const EnvironmentalContext& context);
    AudioFeatures analyzeAudio(const float* audioData, size_t length);
    bool analyzeMotion(float motionLevel, const EnvironmentalContext& context);
    float analyzeEnvironmentalContext(const EnvironmentalContext& context);
    
    // Sensor fusion
    MultiModalResult fuseSensorData(const AIResult& visual,
                                   const AudioFeatures& audio,
                                   bool motionDetected,
                                   const EnvironmentalContext& environment);
    
    // Advanced audio processing
    bool enableAudioProcessing(bool enable = true);
    AudioFeatures extractAudioFeatures(const float* audioData, size_t length);
    std::vector<String> detectWildlifeSounds(const AudioFeatures& features);
    SpeciesType classifyAudioSpecies(const AudioFeatures& features);
    bool detectVocalization(const AudioFeatures& features);
    
    // Environmental intelligence
    void enableEnvironmentalAdaptation(bool enable = true);
    float calculateEnvironmentalSuitability(SpeciesType species,
                                           const EnvironmentalContext& context);
    std::vector<SpeciesType> predictActiveSpecies(const EnvironmentalContext& context);
    float calculateBiodiversityIndex(const std::vector<SpeciesType>& detectedSpecies);
    
    // Behavioral analysis
    BehaviorType analyzeBehaviorMultiModal(const MultiModalResult& result);
    float calculateStressLevel(const MultiModalResult& result);
    bool detectThreatSituation(const MultiModalResult& result);
    std::vector<String> identifyBehaviorIndicators(const MultiModalResult& result);
    
    // Temporal analysis
    void enableTemporalAnalysis(bool enable = true);
    void updateTemporalHistory(const MultiModalResult& result);
    std::vector<MultiModalResult> getTemporalPattern(uint32_t timeWindow_ms);
    BehaviorType predictNextBehavior(const std::vector<MultiModalResult>& history);
    
    // Species-specific optimization
    void optimizeForSpecies(const std::vector<SpeciesType>& targetSpecies);
    void setSpeciesDetectionPriorities(const std::map<SpeciesType, float>& priorities);
    void enableSpeciesAdaptation(bool enable = true);
    
    // Advanced features
    bool enableEcosystemMonitoring(bool enable = true);
    void detectEcosystemChanges(const std::vector<MultiModalResult>& timeSeriesData);
    void generateEcosystemReport(const char* filename);
    bool detectMigrationPatterns(const std::vector<MultiModalResult>& seasonalData);
    
    // Real-time processing
    void enableRealTimeProcessing(bool enable = true);
    void setProcessingPriority(SensorModality modality, int priority);
    bool processStreamingData(const float* audioStream, size_t streamLength);
    
    // Configuration and optimization
    void optimizeFusionWeights();
    void adaptToEnvironmentalConditions();
    void updateModalityWeights(const SensorFusionConfig& newWeights);
    SensorFusionConfig getOptimalFusionConfig();
    
    // Performance monitoring
    struct MultiModalMetrics {
        float averageProcessingTime_ms;
        float fusionAccuracy_percent;
        uint32_t totalAnalyses;
        uint32_t successfulFusions;
        std::map<SensorModality, float> modalityContributions;
        float overallSystemEfficiency;
        
        MultiModalMetrics() : averageProcessingTime_ms(0.0f),
                             fusionAccuracy_percent(0.0f),
                             totalAnalyses(0),
                             successfulFusions(0),
                             overallSystemEfficiency(0.0f) {}
    };
    
    MultiModalMetrics getPerformanceMetrics() const;
    void resetMetrics();
    void printSystemStatus();
    
    // Debug and diagnostics
    void enableDebugMode(bool enable = true);
    void exportAnalysisData(const char* filename);
    void printFusionReport(const MultiModalResult& result);
    bool validateSensorCalibration();

private:
    // Core components
    InferenceEngine* inferenceEngine_;
    bool initialized_;
    
    // Configuration
    SensorFusionConfig fusionConfig_;
    AudioProcessingConfig audioConfig_;
    bool audioProcessingEnabled_;
    bool environmentalAdaptationEnabled_;
    bool temporalAnalysisEnabled_;
    bool debugModeEnabled_;
    
    // Processing state
    std::vector<MultiModalResult> temporalHistory_;
    std::map<SpeciesType, float> speciesDetectionPriorities_;
    std::map<SensorModality, int> processingPriorities_;
    
    // Performance tracking
    mutable MultiModalMetrics metrics_;
    
    // Audio processing components
    std::vector<float> audioBuffer_;
    std::vector<float> spectrogramBuffer_;
    std::vector<float> mfccBuffer_;
    
    // Environmental analysis
    std::vector<SpeciesType> knownSpeciesProfiles_;
    std::map<SpeciesType, EnvironmentalContext> speciesPreferences_;
    
    // Internal processing methods
    bool preprocessAudioData(const float* audioData, size_t length);
    std::vector<float> computeSpectrogram(const float* audioData, size_t length);
    std::vector<float> computeMFCC(const std::vector<float>& spectrogram);
    float computeSpectralCentroid(const std::vector<float>& spectrogram);
    float computeSpectralRolloff(const std::vector<float>& spectrogram);
    float computeZeroCrossingRate(const float* audioData, size_t length);
    
    // Sensor fusion algorithms
    float calculateAdaptiveWeight(SensorModality modality, const EnvironmentalContext& context);
    float fuseBayesian(const std::vector<float>& modalityConfidences,
                      const std::vector<float>& modalityWeights);
    float fuseDempsterShafer(const std::vector<float>& modalityEvidence);
    SpeciesType fuseSpeciesClassifications(const std::vector<std::pair<SpeciesType, float>>& detections);
    
    // Environmental analysis helpers
    float calculateSeasonalFactor(SpeciesType species, uint8_t dayOfYear);
    float calculateTimeOfDayFactor(SpeciesType species, uint8_t hour);
    float calculateWeatherFactor(SpeciesType species, const EnvironmentalContext& context);
    
    // Behavior analysis helpers
    BehaviorType inferBehaviorFromMotion(float motionIntensity, const String& motionPattern);
    BehaviorType inferBehaviorFromAudio(const AudioFeatures& features);
    float calculateActivityLevel(const MultiModalResult& result);
    bool detectSocialBehavior(const MultiModalResult& result);
    
    // Utility functions
    void updateMetrics(const MultiModalResult& result, float processingTime);
    void cleanupTemporalHistory();
    bool isValidSensorData(SensorModality modality, const void* data);
    void logAnalysisEvent(const String& event, const MultiModalResult& result);
    
    // Constants
    static const size_t MAX_TEMPORAL_HISTORY = 1000;
    static const size_t AUDIO_BUFFER_SIZE = 44100; // 1 second at 44.1kHz
    static const uint32_t DEFAULT_TEMPORAL_WINDOW = 60000; // 1 minute
    static const float MIN_FUSION_CONFIDENCE;
};

// Utility functions
const char* sensorModalityToString(SensorModality modality);
float calculateModalityReliability(SensorModality modality, const EnvironmentalContext& context);
bool isCompatibleSpeciesDetection(SpeciesType visual, SpeciesType audio);
String generateMultiModalSummary(const MultiModalResult& result);

#endif // MULTIMODAL_AI_SYSTEM_H