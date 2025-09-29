/**
 * @file acoustic_detection.h
 * @brief Sound-based Detection System for Raptor Calls and Vocalizations
 * 
 * Implements acoustic detection and analysis capabilities for bird of prey
 * monitoring including call classification, territorial vocalization detection,
 * and environmental sound filtering.
 */

#ifndef ACOUSTIC_DETECTION_H
#define ACOUSTIC_DETECTION_H

#include "../scenarios/raptor_configs.h"
#include "../ai/raptor_detection.h"
#include <vector>
#include <memory>
#include <deque>

/**
 * Microphone Types
 */
enum class MicrophoneType {
    I2S_DIGITAL,                // I2S digital microphone (INMP441, etc.)
    ANALOG_ADC,                 // Analog microphone via ADC
    UNKNOWN
};

/**
 * Audio Processing Configuration
 */
struct AudioConfig {
    uint32_t sampleRate_Hz;
    uint8_t channels;
    uint8_t bitsPerSample;
    uint32_t bufferSize_samples;
    uint32_t fftSize;
    uint32_t hopSize;
    uint32_t windowSize_ms;
    bool enableVAD;                     // Voice Activity Detection
    float vadThreshold;
    bool enableNoiseReduction;
    float noiseReductionLevel;
    
    AudioConfig() : sampleRate_Hz(44100), channels(1), bitsPerSample(16),
                   bufferSize_samples(4096), fftSize(1024), hopSize(512),
                   windowSize_ms(50), enableVAD(true), vadThreshold(0.3f),
                   enableNoiseReduction(true), noiseReductionLevel(0.7f) {}
};

/**
 * Raptor Call Types
 */
enum class RaptorCallType {
    ALARM_CALL,                 // Danger/threat warning
    TERRITORIAL_CALL,           // Territory establishment/defense
    MATING_CALL,                // Courtship vocalization
    BEGGING_CALL,               // Juvenile begging for food
    CONTACT_CALL,               // Pair communication
    HUNTING_CALL,               // Hunting coordination
    DISTRESS_CALL,              // Pain/injury vocalization
    AGGRESSIVE_CALL,            // Aggressive display
    NEST_CALL,                  // Nest-related communication
    UNKNOWN_CALL                // Unclassified raptor call
};

/**
 * Environmental Sound Types
 */
enum class EnvironmentalSound {
    WIND,                       // Wind through vegetation
    RAIN,                       // Precipitation
    THUNDER,                    // Thunder/storm sounds
    HUMAN_VOICE,                // Human vocalizations
    VEHICLE,                    // Vehicle noise
    AIRCRAFT,                   // Aircraft overhead
    OTHER_BIRDS,                // Non-raptor bird calls
    MAMMALS,                    // Mammalian vocalizations
    INSECTS,                    // Insect sounds
    MECHANICAL,                 // Mechanical/artificial sounds
    SILENCE,                    // Background/ambient silence
    UNKNOWN_SOUND               // Unclassified sound
};

/**
 * Acoustic Detection Result
 */
struct AcousticDetectionResult {
    bool soundDetected;
    bool raptorCallDetected;
    RaptorCallType callType;
    RaptorSpecies likelySpecies;
    float confidence;
    
    // Audio characteristics
    struct {
        float dominantFrequency_Hz;
        float fundamentalFrequency_Hz;
        float bandwidth_Hz;
        float duration_ms;
        float amplitude_dB;
        float signalToNoise_dB;
        std::vector<float> harmonics;       // Harmonic frequencies
        std::vector<float> formants;        // Formant frequencies
        String spectralShape;
    } acousticFeatures;
    
    // Temporal characteristics
    struct {
        unsigned long startTime_ms;
        unsigned long endTime_ms;
        bool isRepeating;
        uint16_t repetitionRate_bpm;        // Beats per minute
        float syllablePattern[10];          // Pattern of syllables
        uint8_t syllableCount;
    } temporal;
    
    // Environmental context
    struct {
        std::vector<EnvironmentalSound> backgroundSounds;
        float windNoise_dB;
        float ambientNoise_dB;
        bool precipitationDetected;
        float overallNoiseLevel_dB;
        String acousticEnvironment;         // "forest", "open", "urban", etc.
    } environment;
    
    // Behavioral implications
    struct {
        bool territorialBehavior;
        bool aggressiveIntent;
        bool distressIndicator;
        bool matingBehavior;
        bool parentalBehavior;
        float urgencyLevel;                 // 0-1 scale
        String behavioralContext;
    } behavior;
    
    // Quality metrics
    float audioQuality;
    bool clipped;
    bool hasArtifacts;
    float processingTime_ms;
    
    AcousticDetectionResult() : soundDetected(false), raptorCallDetected(false),
                               callType(RaptorCallType::UNKNOWN_CALL),
                               likelySpecies(RaptorSpecies::UNKNOWN_RAPTOR),
                               confidence(0.0f), audioQuality(0.0f),
                               clipped(false), hasArtifacts(false),
                               processingTime_ms(0.0f) {}
};

/**
 * Audio Spectrogram Analysis
 */
struct SpectrogramData {
    std::vector<std::vector<float>> magnitude;  // [frequency][time]
    std::vector<float> frequencies;
    std::vector<float> timeStamps;
    uint32_t frequencyBins;
    uint32_t timeBins;
    float frequencyResolution_Hz;
    float timeResolution_ms;
    
    SpectrogramData() : frequencyBins(0), timeBins(0),
                       frequencyResolution_Hz(0.0f), timeResolution_ms(0.0f) {}
};

/**
 * Call Pattern Database Entry
 */
struct CallPattern {
    RaptorSpecies species;
    RaptorCallType callType;
    
    // Frequency characteristics
    struct {
        float minFrequency_Hz;
        float maxFrequency_Hz;
        float peakFrequency_Hz;
        std::vector<float> harmonicRatios;
        float bandwidthTypical_Hz;
    } frequency;
    
    // Temporal characteristics
    struct {
        float minDuration_ms;
        float maxDuration_ms;
        float typicalDuration_ms;
        float repetitionRate_Hz;
        bool isRepeated;
        String syllablePattern;
    } temporal;
    
    // Recognition parameters
    float matchingThreshold;
    uint32_t observationCount;
    float averageConfidence;
    String description;
    String conservationNotes;
    
    CallPattern() : species(RaptorSpecies::UNKNOWN_RAPTOR),
                   callType(RaptorCallType::UNKNOWN_CALL),
                   matchingThreshold(0.7f), observationCount(0),
                   averageConfidence(0.0f) {}
};

/**
 * Main Acoustic Detection System
 */
class AcousticDetection {
public:
    /**
     * Constructor
     */
    AcousticDetection();
    
    /**
     * Destructor
     */
    ~AcousticDetection();
    
    /**
     * Initialize acoustic detection system
     * @param scenario Target monitoring scenario
     * @param config Audio configuration parameters
     * @return true if initialization successful
     */
    bool init(RaptorScenario scenario, const AudioConfig& config = AudioConfig());
    
    /**
     * Process audio buffer for raptor calls
     * @param audioData Raw audio sample data
     * @param sampleCount Number of audio samples
     * @return Acoustic detection result
     */
    AcousticDetectionResult processAudio(const int16_t* audioData, uint32_t sampleCount);
    
    /**
     * Continuous audio monitoring (non-blocking)
     * Call this regularly to process incoming audio
     */
    void updateAudioMonitoring();
    
    /**
     * Start continuous audio recording
     * @param duration_minutes Recording duration (0 = continuous)
     * @return true if recording started successfully
     */
    bool startRecording(uint32_t duration_minutes = 0);
    
    /**
     * Stop audio recording
     * @return Recording statistics
     */
    struct RecordingStats {
        uint32_t duration_s;
        uint32_t callsDetected;
        uint32_t speciesIdentified;
        float averageConfidence;
        uint32_t dataSize_KB;
    } stopRecording();
    
    /**
     * Analyze spectrogram for call patterns
     * @param spectrogram Spectrogram data
     * @return Vector of detected call patterns
     */
    std::vector<AcousticDetectionResult> analyzeSpectrogram(const SpectrogramData& spectrogram);
    
    /**
     * Classify environmental sounds
     * @param audioData Raw audio data
     * @param sampleCount Number of samples
     * @return Vector of environmental sounds detected
     */
    std::vector<EnvironmentalSound> classifyEnvironmentalSounds(const int16_t* audioData, 
                                                              uint32_t sampleCount);
    
    /**
     * Enable/disable specific call type detection
     * @param callType Type of call to monitor
     * @param enable true to enable detection
     */
    void enableCallTypeDetection(RaptorCallType callType, bool enable);
    
    /**
     * Set detection sensitivity for call types
     * @param callType Target call type
     * @param sensitivity 0-100 sensitivity level
     */
    void setCallSensitivity(RaptorCallType callType, uint8_t sensitivity);
    
    /**
     * Add custom call pattern to database
     * @param pattern Call pattern definition
     * @return true if pattern added successfully
     */
    bool addCallPattern(const CallPattern& pattern);
    
    /**
     * Calibrate for current acoustic environment
     * @param calibrationDuration_s Calibration time in seconds
     * @return true if calibration successful
     */
    bool calibrateAcousticEnvironment(uint32_t calibrationDuration_s = 60);
    
    /**
     * Get current noise levels
     */
    struct NoiseAnalysis {
        float backgroundNoise_dB;
        float peakNoise_dB;
        float averageNoise_dB;
        std::vector<float> frequencyProfile;    // Noise profile by frequency
        String dominantNoiseSource;
    } getCurrentNoiseAnalysis() const;
    
    /**
     * Export audio detection data
     * @param format Export format ("csv", "json", "xml")
     * @param includeAudio true to include audio samples
     * @return Exported data string
     */
    String exportDetectionData(const String& format = "csv", bool includeAudio = false) const;
    
    /**
     * Get system performance statistics
     */
    struct AcousticPerformance {
        uint32_t samplesProcessed;
        float averageProcessingTime_ms;
        uint32_t callsDetected;
        uint32_t falsePositives;
        float accuracyRate_percent;
        float cpuUsage_percent;
        float memoryUsage_KB;
    } getPerformanceStats() const;
    
    /**
     * Check if acoustic system is operational
     */
    bool isOperational() const { return initialized && recording; }
    
    /**
     * Get detected call history
     */
    std::vector<AcousticDetectionResult> getCallHistory(uint32_t maxResults = 100) const;
    
    /**
     * Get species name for display
     */
    const char* getSpeciesName(RaptorSpecies species) const;

private:
    // Core system state
    bool initialized;
    bool recording;
    RaptorScenario currentScenario;
    AudioConfig audioConfig;
    
    // Hardware state
    MicrophoneType microphoneType;
    uint32_t sampleRate;
    uint32_t bufferSize;
    
    // Audio processing components
    std::unique_ptr<uint8_t[]> audioBuffer;
    std::unique_ptr<float[]> fftBuffer;
    std::unique_ptr<float[]> spectrogramBuffer;
    std::deque<int16_t> audioHistory;
    static const size_t MAX_AUDIO_HISTORY = 44100 * 30; // 30 seconds at 44.1kHz
    
    // Task management
    static bool g_audioTaskRunning;
    static TaskHandle_t g_audioTaskHandle;
    
    // Performance tracking
    unsigned long lastProcessingTime;
    uint64_t totalSamplesProcessed;
    
    // Call pattern database
    std::vector<CallPattern> callDatabase;
    std::vector<bool> enabledCallTypes;
    std::vector<uint8_t> callSensitivities;
    
    // Detection history
    std::deque<AcousticDetectionResult> detectionHistory;
    static const size_t MAX_DETECTION_HISTORY = 1000;
    
    // Environmental calibration
    struct {
        float backgroundNoiseLevel_dB;
        float windNoiseLevel_dB;
        unsigned long lastCalibrationTime;
        float adaptiveThreshold;
        std::vector<float> noiseProfile;        // Background noise by frequency
    } environmentalState;
    
    // Performance tracking
    struct {
        uint32_t samplesProcessed;
        float averageProcessingTime_ms;
        uint32_t callsDetected;
        uint32_t falsePositives;
        float accuracyRate_percent;
        float cpuUsage_percent;
        float memoryUsage_KB;
    } performanceMetrics;
    
    // Hardware initialization methods
    bool initI2SMicrophone();
    bool initAnalogMicrophone();
    bool allocateBuffers();
    void initializeCallDatabase();
    bool startAudioTask();
    static void audioTaskWrapper(void* parameter);
    void audioProcessingTask();
    void cleanup();
    
    // Audio data acquisition
    bool readAudioData();
    bool readI2SAudioData();
    bool readAnalogAudioData();
    void processAudioBuffer();
    
    // Utility methods
    const char* getSpeciesName(RaptorSpecies species);
    void triggerCameraCapture(const AcousticDetectionResult& result);
    void updatePerformanceMetrics();
    
    // Audio processing methods
    void preprocessAudio(int16_t* audioData, uint32_t sampleCount);
    void computeSpectrogram(const int16_t* audioData, uint32_t sampleCount, SpectrogramData& spectrogram);
    void applyNoiseReduction(float* spectrogramData, uint32_t size);
    bool detectVoiceActivity(const int16_t* audioData, uint32_t sampleCount);
    
    // Feature extraction
    std::vector<float> extractSpectralFeatures(const SpectrogramData& spectrogram);
    std::vector<float> extractTemporalFeatures(const int16_t* audioData, uint32_t sampleCount);
    std::vector<float> extractCepstralFeatures(const SpectrogramData& spectrogram);
    
    // Call detection algorithms
    bool detectRaptorCall(const SpectrogramData& spectrogram, AcousticDetectionResult& result);
    RaptorCallType classifyCallType(const std::vector<float>& features);
    RaptorSpecies identifySpecies(const std::vector<float>& features, RaptorCallType callType);
    float calculateCallConfidence(const std::vector<float>& features, const CallPattern& pattern);
    
    // Pattern matching
    float matchCallPattern(const std::vector<float>& features, const CallPattern& pattern);
    bool validateCallDetection(const AcousticDetectionResult& result);
    void updateCallStatistics(const AcousticDetectionResult& result);
    
    // Environmental analysis
    std::vector<EnvironmentalSound> analyzeBackground(const SpectrogramData& spectrogram);
    float calculateNoiseLevel(const int16_t* audioData, uint32_t sampleCount);
    void updateEnvironmentalModel(const SpectrogramData& spectrogram);
    bool isEnvironmentalNoise(const std::vector<float>& features);
    
    // Filtering and validation
    bool passesFrequencyFilter(float frequency, RaptorSpecies species);
    bool passesDurationFilter(float duration, RaptorCallType callType);
    bool passesAmplitudeFilter(float amplitude);
    bool passesQualityFilter(const AcousticDetectionResult& result);
    
    // Database management
    void loadDefaultCallPatterns();
    bool loadCallPatternFromFile(const String& filename);
    void optimizeCallDatabase();
    
    // Audio utilities
    void applyWindowFunction(float* data, uint32_t size);
    void computeFFT(const float* input, float* output, uint32_t size);
    float* computeMelFilterBank(const float* spectrum, uint32_t spectrumSize);
    std::vector<float> computeMFCC(const float* melSpectrum, uint32_t size);
    
    // Performance optimization
    void optimizeProcessingParameters();
    bool shouldSkipProcessing() const;
    void manageMemoryUsage();
    
    // Scenario-specific configuration
    void configureForNestingMonitoring();
    void configureForMigrationTracking();
    void configureForHuntingAnalysis();
    void configureForTerritorialMonitoring();
    
    // Utility methods
    void logAcousticEvent(const AcousticDetectionResult& result);
    String formatAcousticData(const AcousticDetectionResult& result, const String& format) const;
    void cleanupOldData();
    float calculateOverallConfidence(const std::vector<float>& confidences) const;
    void triggerAcousticAlert(const AcousticDetectionResult& result);
};

/**
 * Acoustic Utilities
 */
namespace AcousticUtils {
    /**
     * Convert call type to string
     */
    const char* callTypeToString(RaptorCallType callType);
    
    /**
     * Convert environmental sound to string
     */
    const char* environmentalSoundToString(EnvironmentalSound sound);
    
    /**
     * Calculate frequency from MIDI note
     */
    float midiToFrequency(uint8_t midiNote);
    
    /**
     * Calculate MIDI note from frequency
     */
    uint8_t frequencyToMidi(float frequency);
    
    /**
     * Convert dB to linear scale
     */
    float dbToLinear(float db);
    
    /**
     * Convert linear scale to dB
     */
    float linearToDb(float linear);
    
    /**
     * Generate acoustic environment summary
     */
    String generateAcousticSummary(const std::vector<AcousticDetectionResult>& detections);
    
    /**
     * Calculate acoustic diversity index
     */
    float calculateAcousticDiversityIndex(const std::vector<AcousticDetectionResult>& detections);
}

#endif // ACOUSTIC_DETECTION_H