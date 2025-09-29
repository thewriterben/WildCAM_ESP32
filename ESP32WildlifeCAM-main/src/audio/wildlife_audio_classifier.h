/**
 * @file wildlife_audio_classifier.h
 * @brief Enhanced Wildlife Audio Classification System
 * 
 * Extends the existing acoustic detection system with comprehensive wildlife
 * species identification and audio-visual fusion capabilities.
 */

#ifndef WILDLIFE_AUDIO_CLASSIFIER_H
#define WILDLIFE_AUDIO_CLASSIFIER_H

#include "../../firmware/src/audio/acoustic_detection.h"
#include "../../firmware/src/audio/i2s_microphone.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <vector>
#include <map>

/**
 * Audio classification confidence levels
 */
enum class AudioConfidenceLevel {
    VERY_LOW = 0,
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3,
    VERY_HIGH = 4
};

/**
 * Wildlife species audio signatures
 */
struct SpeciesAudioSignature {
    String species_name;
    float frequency_range_min;      // Hz
    float frequency_range_max;      // Hz
    float peak_frequency;           // Hz
    float call_duration_min;        // ms
    float call_duration_max;        // ms
    float repetition_interval;      // ms
    float amplitude_threshold;      // dB
    std::vector<float> harmonic_ratios;
    
    SpeciesAudioSignature() 
        : species_name("unknown")
        , frequency_range_min(100.0f)
        , frequency_range_max(8000.0f)
        , peak_frequency(2000.0f)
        , call_duration_min(100.0f)
        , call_duration_max(5000.0f)
        , repetition_interval(1000.0f)
        , amplitude_threshold(-40.0f) {
    }
};

/**
 * Audio classification result
 */
struct AudioClassificationResult {
    String detected_species;
    float confidence;
    AudioConfidenceLevel confidence_level;
    float frequency_match;
    float duration_match;
    float amplitude_match;
    uint32_t detection_timestamp;
    bool is_wildlife;
    bool is_bird_call;
    bool is_mammal_call;
    float noise_level;
    
    AudioClassificationResult() 
        : detected_species("none")
        , confidence(0.0f)
        , confidence_level(AudioConfidenceLevel::VERY_LOW)
        , frequency_match(0.0f)
        , duration_match(0.0f)
        , amplitude_match(0.0f)
        , detection_timestamp(0)
        , is_wildlife(false)
        , is_bird_call(false)
        , is_mammal_call(false)
        , noise_level(0.0f) {
    }
};

/**
 * Multi-modal detection result (audio + visual)
 */
struct MultiModalResult {
    AudioClassificationResult audio_result;
    bool has_visual_detection;
    String visual_species;
    float visual_confidence;
    float combined_confidence;
    bool correlation_found;
    
    MultiModalResult() 
        : has_visual_detection(false)
        , visual_species("none")
        , visual_confidence(0.0f)
        , combined_confidence(0.0f)
        , correlation_found(false) {
    }
};

/**
 * Wildlife Audio Classifier Class
 */
class WildlifeAudioClassifier {
public:
    /**
     * Constructor
     */
    WildlifeAudioClassifier();
    
    /**
     * Destructor
     */
    ~WildlifeAudioClassifier();
    
    /**
     * Initialize audio classifier
     * @param microphone_type Type of microphone to use
     * @return true if initialization successful
     */
    bool initialize(MicrophoneType microphone_type = MicrophoneType::I2S_DIGITAL);
    
    /**
     * Process audio buffer for wildlife classification
     * @param audio_buffer Audio sample buffer
     * @param buffer_size Size of audio buffer
     * @return Classification result
     */
    AudioClassificationResult processAudioBuffer(const int16_t* audio_buffer, size_t buffer_size);
    
    /**
     * Continuous audio monitoring (call regularly)
     * @return Latest classification result
     */
    AudioClassificationResult monitorAudio();
    
    /**
     * Combine audio and visual detection results
     * @param audio_result Audio classification result
     * @param visual_species Visual detection species
     * @param visual_confidence Visual detection confidence
     * @return Combined multi-modal result
     */
    MultiModalResult combineWithVisual(const AudioClassificationResult& audio_result,
                                     const String& visual_species,
                                     float visual_confidence);
    
    /**
     * Add species audio signature for classification
     * @param signature Species audio signature
     */
    void addSpeciesSignature(const SpeciesAudioSignature& signature);
    
    /**
     * Load predefined wildlife signatures
     */
    void loadWildlifeSignatures();
    
    /**
     * Set classification sensitivity
     * @param sensitivity Sensitivity level (0.0-1.0)
     */
    void setSensitivity(float sensitivity);
    
    /**
     * Enable/disable noise filtering
     * @param enable Enable noise filtering
     */
    void setNoiseFiltering(bool enable);
    
    /**
     * Get audio classification statistics
     * @return Statistics string
     */
    String getStatistics() const;
    
    /**
     * Get current audio level
     * @return Audio level in dB
     */
    float getCurrentAudioLevel() const;
    
    /**
     * Check if audio classification is active
     * @return true if actively monitoring
     */
    bool isActive() const { return monitoring_active; }
    
    /**
     * Enable/disable audio monitoring
     * @param enable Enable audio monitoring
     */
    void setMonitoringEnabled(bool enable);
    
    /**
     * Cleanup audio classifier resources
     */
    void cleanup();

private:
    // Configuration
    AudioConfig config;
    MicrophoneType microphone_type;
    bool initialized = false;
    bool monitoring_active = false;
    float sensitivity = 0.7f;
    bool noise_filtering_enabled = true;
    
    // Audio processing
    std::unique_ptr<I2SMicrophone> i2s_microphone;
    std::vector<int16_t> audio_buffer;
    size_t buffer_position = 0;
    
    // Species signatures
    std::vector<SpeciesAudioSignature> species_signatures;
    std::map<String, uint32_t> detection_counts;
    
    // Audio analysis
    std::vector<float> frequency_spectrum;
    std::vector<float> amplitude_history;
    size_t max_history_size = 100;
    float current_noise_floor = -60.0f;
    
    // Statistics
    uint32_t total_classifications = 0;
    uint32_t successful_detections = 0;
    uint32_t false_positives = 0;
    unsigned long last_detection_time = 0;
    
    /**
     * Analyze audio frequency spectrum
     * @param audio_buffer Audio samples
     * @param buffer_size Buffer size
     * @return Frequency analysis results
     */
    std::vector<float> analyzeFrequencySpectrum(const int16_t* audio_buffer, size_t buffer_size);
    
    /**
     * Calculate audio features for classification
     * @param spectrum Frequency spectrum
     * @return Feature vector
     */
    std::vector<float> extractAudioFeatures(const std::vector<float>& spectrum);
    
    /**
     * Match audio features against species signatures
     * @param features Audio feature vector
     * @return Best matching species and confidence
     */
    std::pair<String, float> matchSpeciesSignature(const std::vector<float>& features);
    
    /**
     * Calculate frequency match score
     * @param spectrum Frequency spectrum
     * @param signature Species signature
     * @return Match score (0.0-1.0)
     */
    float calculateFrequencyMatch(const std::vector<float>& spectrum, 
                                  const SpeciesAudioSignature& signature);
    
    /**
     * Calculate duration match score
     * @param call_duration Detected call duration
     * @param signature Species signature
     * @return Match score (0.0-1.0)
     */
    float calculateDurationMatch(float call_duration, 
                                const SpeciesAudioSignature& signature);
    
    /**
     * Update noise floor estimation
     * @param spectrum Current frequency spectrum
     */
    void updateNoiseFloor(const std::vector<float>& spectrum);
    
    /**
     * Apply noise filtering to spectrum
     * @param spectrum Input spectrum
     * @return Filtered spectrum
     */
    std::vector<float> applyNoiseFiltering(const std::vector<float>& spectrum);
    
    /**
     * Detect call boundaries in audio
     * @param amplitude_data Amplitude data
     * @return Call start and end times
     */
    std::vector<std::pair<uint32_t, uint32_t>> detectCallBoundaries(const std::vector<float>& amplitude_data);
    
    /**
     * Convert confidence score to confidence level
     * @param confidence Confidence score (0.0-1.0)
     * @return Confidence level
     */
    AudioConfidenceLevel getConfidenceLevel(float confidence);
    
    /**
     * Log classification result
     * @param result Classification result
     */
    void logClassificationResult(const AudioClassificationResult& result);
};

/**
 * Wildlife audio signatures database
 */
namespace WildlifeAudioSignatures {
    /**
     * Get bird species signatures
     */
    std::vector<SpeciesAudioSignature> getBirdSignatures();
    
    /**
     * Get mammal species signatures
     */
    std::vector<SpeciesAudioSignature> getMammalSignatures();
    
    /**
     * Get raptor species signatures (existing)
     */
    std::vector<SpeciesAudioSignature> getRaptorSignatures();
    
    /**
     * Create signature for specific species
     */
    SpeciesAudioSignature createWhiteTailedDeerSignature();
    SpeciesAudioSignature createRaccoonSignature();
    SpeciesAudioSignature createCoyoteSignature();
    SpeciesAudioSignature createOwlSignature();
    SpeciesAudioSignature createRedTailedHawkSignature();
    SpeciesAudioSignature createWildTurkeySignature();
}

/**
 * Audio-visual fusion utilities
 */
namespace AudioVisualFusion {
    /**
     * Calculate correlation between audio and visual detections
     * @param audio_species Audio detected species
     * @param visual_species Visual detected species
     * @param time_correlation Time correlation factor
     * @return Correlation score (0.0-1.0)
     */
    float calculateSpeciesCorrelation(const String& audio_species, 
                                     const String& visual_species,
                                     float time_correlation);
    
    /**
     * Combine confidence scores using weighted fusion
     * @param audio_confidence Audio detection confidence
     * @param visual_confidence Visual detection confidence
     * @param correlation_score Species correlation score
     * @return Combined confidence score
     */
    float combineConfidenceScores(float audio_confidence, 
                                 float visual_confidence,
                                 float correlation_score);
    
    /**
     * Validate multi-modal detection consistency
     * @param result Multi-modal result
     * @return true if detection is consistent
     */
    bool validateDetectionConsistency(const MultiModalResult& result);
}

#endif // WILDLIFE_AUDIO_CLASSIFIER_H