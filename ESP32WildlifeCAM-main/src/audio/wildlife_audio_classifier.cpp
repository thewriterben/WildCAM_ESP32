/**
 * @file wildlife_audio_classifier.cpp
 * @brief Wildlife Audio Classification System Implementation
 * 
 * Implements comprehensive wildlife audio classification with
 * species identification and audio-visual fusion capabilities.
 */

#include "wildlife_audio_classifier.h"
#include <Arduino.h>
#include <math.h>
#include <algorithm>

// Constructor
WildlifeAudioClassifier::WildlifeAudioClassifier() {
    initialized = false;
    monitoring_active = false;
    sensitivity = 0.7f;
    noise_filtering_enabled = true;
    
    // Initialize audio buffer
    audio_buffer.reserve(4096);
    frequency_spectrum.reserve(2048);
    amplitude_history.reserve(max_history_size);
}

// Destructor
WildlifeAudioClassifier::~WildlifeAudioClassifier() {
    cleanup();
}

// Initialize audio classifier
bool WildlifeAudioClassifier::initialize(MicrophoneType microphone_type) {
    if (initialized) {
        Serial.println("WildlifeAudioClassifier already initialized");
        return true;
    }
    
    Serial.println("Initializing Wildlife Audio Classifier...");
    
    this->microphone_type = microphone_type;
    
    // Initialize I2S microphone if selected
    if (microphone_type == MicrophoneType::I2S_DIGITAL) {
        i2s_microphone = std::unique_ptr<I2SMicrophone>(new I2SMicrophone());
        
        I2SMicrophoneConfig mic_config;
        mic_config.sample_rate = config.sampleRate_Hz;
        
        if (!i2s_microphone->init(mic_config)) {
            Serial.println("ERROR: Failed to initialize I2S microphone");
            return false;
        }
        
        Serial.println("I2S microphone initialized successfully");
    }
    
    // Load predefined wildlife signatures
    loadWildlifeSignatures();
    
    initialized = true;
    Serial.println("Wildlife Audio Classifier initialized successfully");
    
    return true;
}

// Process audio buffer for wildlife classification
AudioClassificationResult WildlifeAudioClassifier::processAudioBuffer(
    const int16_t* audio_buffer, size_t buffer_size) {
    
    AudioClassificationResult result;
    
    if (!audio_buffer || buffer_size == 0) {
        return result;
    }
    
    total_classifications++;
    
    // Analyze frequency spectrum
    std::vector<float> spectrum = analyzeFrequencySpectrum(audio_buffer, buffer_size);
    
    // Apply noise filtering if enabled
    if (noise_filtering_enabled) {
        spectrum = applyNoiseFiltering(spectrum);
        updateNoiseFloor(spectrum);
    }
    
    // Extract audio features
    std::vector<float> features = extractAudioFeatures(spectrum);
    
    // Match against species signatures
    auto [detected_species, confidence] = matchSpeciesSignature(features);
    
    // Fill result
    result.detected_species = detected_species;
    result.confidence = confidence * sensitivity;
    result.confidence_level = getConfidenceLevel(result.confidence);
    result.detection_timestamp = millis();
    result.noise_level = current_noise_floor;
    
    // Determine wildlife type
    result.is_wildlife = confidence > 0.3f;
    result.is_bird_call = false;
    result.is_mammal_call = false;
    
    if (result.is_wildlife) {
        // Simple heuristic classification
        float peak_freq = 0.0f;
        if (!spectrum.empty()) {
            auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
            size_t maxIdx = std::distance(spectrum.begin(), maxIt);
            peak_freq = (maxIdx * config.sampleRate_Hz) / (2.0f * spectrum.size());
        }
        
        // Bird calls typically 2-8 kHz
        result.is_bird_call = (peak_freq > 2000.0f && peak_freq < 8000.0f);
        
        // Mammal calls typically 200-2000 Hz
        result.is_mammal_call = (peak_freq > 200.0f && peak_freq < 2000.0f);
        
        successful_detections++;
        last_detection_time = millis();
        
        // Update detection counts
        detection_counts[detected_species]++;
    }
    
    // Log result
    logClassificationResult(result);
    
    return result;
}

// Continuous audio monitoring
AudioClassificationResult WildlifeAudioClassifier::monitorAudio() {
    AudioClassificationResult result;
    
    if (!initialized || !monitoring_active) {
        return result;
    }
    
    if (!i2s_microphone || !i2s_microphone->isRecording()) {
        return result;
    }
    
    // Read audio samples
    std::vector<int16_t> buffer(1024);
    size_t samples_read = 0;
    
    if (i2s_microphone->readSamples(buffer.data(), buffer.size(), samples_read, 100)) {
        // Process the audio buffer
        result = processAudioBuffer(buffer.data(), samples_read);
    }
    
    return result;
}

// Combine audio and visual detection results
MultiModalResult WildlifeAudioClassifier::combineWithVisual(
    const AudioClassificationResult& audio_result,
    const String& visual_species,
    float visual_confidence) {
    
    MultiModalResult result;
    result.audio_result = audio_result;
    result.has_visual_detection = (visual_confidence > 0.0f);
    result.visual_species = visual_species;
    result.visual_confidence = visual_confidence;
    
    // Calculate correlation
    float time_correlation = 1.0f; // Assume synchronized
    float correlation_score = AudioVisualFusion::calculateSpeciesCorrelation(
        audio_result.detected_species, visual_species, time_correlation);
    
    result.correlation_found = (correlation_score > 0.5f);
    
    // Combine confidence scores
    result.combined_confidence = AudioVisualFusion::combineConfidenceScores(
        audio_result.confidence, visual_confidence, correlation_score);
    
    // Validate consistency
    bool consistent = AudioVisualFusion::validateDetectionConsistency(result);
    
    if (!consistent) {
        // Reduce combined confidence if detections are inconsistent
        result.combined_confidence *= 0.8f;
    }
    
    return result;
}

// Add species audio signature
void WildlifeAudioClassifier::addSpeciesSignature(const SpeciesAudioSignature& signature) {
    species_signatures.push_back(signature);
    Serial.println("Added signature for: " + signature.species_name);
}

// Load predefined wildlife signatures
void WildlifeAudioClassifier::loadWildlifeSignatures() {
    Serial.println("Loading wildlife audio signatures...");
    
    // Load bird signatures
    auto bird_sigs = WildlifeAudioSignatures::getBirdSignatures();
    for (const auto& sig : bird_sigs) {
        addSpeciesSignature(sig);
    }
    
    // Load mammal signatures
    auto mammal_sigs = WildlifeAudioSignatures::getMammalSignatures();
    for (const auto& sig : mammal_sigs) {
        addSpeciesSignature(sig);
    }
    
    // Load raptor signatures
    auto raptor_sigs = WildlifeAudioSignatures::getRaptorSignatures();
    for (const auto& sig : raptor_sigs) {
        addSpeciesSignature(sig);
    }
    
    Serial.println("Loaded " + String(species_signatures.size()) + " species signatures");
}

// Set classification sensitivity
void WildlifeAudioClassifier::setSensitivity(float sensitivity) {
    this->sensitivity = constrain(sensitivity, 0.0f, 1.0f);
    Serial.println("Sensitivity set to: " + String(this->sensitivity, 2));
}

// Enable/disable noise filtering
void WildlifeAudioClassifier::setNoiseFiltering(bool enable) {
    noise_filtering_enabled = enable;
    Serial.println(String("Noise filtering ") + (enable ? "enabled" : "disabled"));
}

// Get audio classification statistics
String WildlifeAudioClassifier::getStatistics() const {
    String stats = "=== Audio Classification Statistics ===\n";
    stats += "Total Classifications: " + String(total_classifications) + "\n";
    stats += "Successful Detections: " + String(successful_detections) + "\n";
    stats += "False Positives: " + String(false_positives) + "\n";
    
    float success_rate = total_classifications > 0 ? 
        (100.0f * successful_detections / total_classifications) : 0.0f;
    stats += "Success Rate: " + String(success_rate, 1) + "%\n";
    
    stats += "\nTop Detected Species:\n";
    for (const auto& pair : detection_counts) {
        stats += "  " + pair.first + ": " + String(pair.second) + "\n";
    }
    
    return stats;
}

// Get current audio level
float WildlifeAudioClassifier::getCurrentAudioLevel() const {
    if (i2s_microphone) {
        return i2s_microphone->getSignalLevel();
    }
    return current_noise_floor;
}

// Enable/disable audio monitoring
void WildlifeAudioClassifier::setMonitoringEnabled(bool enable) {
    monitoring_active = enable;
    
    if (enable && i2s_microphone && !i2s_microphone->isRecording()) {
        i2s_microphone->startRecording();
        Serial.println("Audio monitoring started");
    } else if (!enable && i2s_microphone && i2s_microphone->isRecording()) {
        i2s_microphone->stopRecording();
        Serial.println("Audio monitoring stopped");
    }
}

// Cleanup
void WildlifeAudioClassifier::cleanup() {
    if (i2s_microphone) {
        if (i2s_microphone->isRecording()) {
            i2s_microphone->stopRecording();
        }
        i2s_microphone.reset();
    }
    
    audio_buffer.clear();
    frequency_spectrum.clear();
    amplitude_history.clear();
    species_signatures.clear();
    detection_counts.clear();
    
    initialized = false;
    monitoring_active = false;
    
    Serial.println("Wildlife Audio Classifier cleanup complete");
}

// Private methods

// Analyze audio frequency spectrum
std::vector<float> WildlifeAudioClassifier::analyzeFrequencySpectrum(
    const int16_t* audio_buffer, size_t buffer_size) {
    
    std::vector<float> spectrum;
    spectrum.reserve(buffer_size / 2);
    
    // Simple magnitude spectrum computation
    // In production, would use proper FFT library
    for (size_t i = 0; i < buffer_size / 2; i++) {
        float real = float(audio_buffer[i * 2]) / 32768.0f;
        float imag = (i * 2 + 1 < buffer_size) ? float(audio_buffer[i * 2 + 1]) / 32768.0f : 0.0f;
        float magnitude = sqrt(real * real + imag * imag);
        spectrum.push_back(magnitude);
    }
    
    return spectrum;
}

// Extract audio features
std::vector<float> WildlifeAudioClassifier::extractAudioFeatures(
    const std::vector<float>& spectrum) {
    
    std::vector<float> features;
    
    if (spectrum.empty()) {
        return features;
    }
    
    // Feature 1: Peak frequency
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    size_t peakIdx = std::distance(spectrum.begin(), maxIt);
    float peakFreq = (peakIdx * config.sampleRate_Hz) / (2.0f * spectrum.size());
    features.push_back(peakFreq);
    
    // Feature 2: Spectral centroid
    float weightedSum = 0.0f;
    float totalPower = 0.0f;
    for (size_t i = 0; i < spectrum.size(); i++) {
        float power = spectrum[i] * spectrum[i];
        weightedSum += i * power;
        totalPower += power;
    }
    float centroid = totalPower > 0.0f ? weightedSum / totalPower : 0.0f;
    features.push_back(centroid);
    
    // Feature 3: Spectral bandwidth
    float bandwidth = 0.0f;
    if (totalPower > 0.0f) {
        for (size_t i = 0; i < spectrum.size(); i++) {
            float power = spectrum[i] * spectrum[i];
            bandwidth += (i - centroid) * (i - centroid) * power;
        }
        bandwidth = sqrt(bandwidth / totalPower);
    }
    features.push_back(bandwidth);
    
    // Feature 4: Total energy
    features.push_back(totalPower);
    
    // Feature 5: Spectral flatness
    float geometricMean = 0.0f;
    float arithmeticMean = 0.0f;
    for (float val : spectrum) {
        if (val > 0.0f) {
            geometricMean += log(val);
        }
        arithmeticMean += val;
    }
    geometricMean = exp(geometricMean / spectrum.size());
    arithmeticMean /= spectrum.size();
    float flatness = arithmeticMean > 0.0f ? geometricMean / arithmeticMean : 0.0f;
    features.push_back(flatness);
    
    return features;
}

// Match audio features against species signatures
std::pair<String, float> WildlifeAudioClassifier::matchSpeciesSignature(
    const std::vector<float>& features) {
    
    if (features.empty() || species_signatures.empty()) {
        return {"unknown", 0.0f};
    }
    
    String bestMatch = "unknown";
    float bestScore = 0.0f;
    
    // Extract peak frequency from features
    float peakFreq = features[0];
    float totalEnergy = features.size() > 3 ? features[3] : 0.0f;
    
    // Match against each species signature
    for (const auto& signature : species_signatures) {
        float score = 0.0f;
        
        // Frequency match
        if (peakFreq >= signature.frequency_range_min && 
            peakFreq <= signature.frequency_range_max) {
            
            float freqDiff = fabs(peakFreq - signature.peak_frequency);
            float freqRange = signature.frequency_range_max - signature.frequency_range_min;
            float freqScore = 1.0f - (freqDiff / freqRange);
            score += freqScore * 0.6f;
            
            // Energy match (simplified amplitude check)
            float energyDb = 10.0f * log10(totalEnergy + 1e-10f);
            if (energyDb > signature.amplitude_threshold) {
                score += 0.4f;
            }
            
            if (score > bestScore) {
                bestScore = score;
                bestMatch = signature.species_name;
            }
        }
    }
    
    return {bestMatch, bestScore};
}

// Calculate frequency match score
float WildlifeAudioClassifier::calculateFrequencyMatch(
    const std::vector<float>& spectrum,
    const SpeciesAudioSignature& signature) {
    
    if (spectrum.empty()) {
        return 0.0f;
    }
    
    // Find peak frequency in spectrum
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    size_t peakIdx = std::distance(spectrum.begin(), maxIt);
    float peakFreq = (peakIdx * config.sampleRate_Hz) / (2.0f * spectrum.size());
    
    // Check if within signature range
    if (peakFreq < signature.frequency_range_min || peakFreq > signature.frequency_range_max) {
        return 0.0f;
    }
    
    // Calculate match score based on distance from ideal peak
    float freqDiff = fabs(peakFreq - signature.peak_frequency);
    float freqRange = signature.frequency_range_max - signature.frequency_range_min;
    
    return 1.0f - (freqDiff / freqRange);
}

// Calculate duration match score
float WildlifeAudioClassifier::calculateDurationMatch(
    float call_duration,
    const SpeciesAudioSignature& signature) {
    
    if (call_duration < signature.call_duration_min || 
        call_duration > signature.call_duration_max) {
        return 0.0f;
    }
    
    // Duration within range
    return 1.0f;
}

// Update noise floor estimation
void WildlifeAudioClassifier::updateNoiseFloor(const std::vector<float>& spectrum) {
    if (spectrum.empty()) {
        return;
    }
    
    // Calculate median energy as noise floor estimate
    std::vector<float> sortedSpectrum = spectrum;
    std::sort(sortedSpectrum.begin(), sortedSpectrum.end());
    
    float median = sortedSpectrum[sortedSpectrum.size() / 2];
    float noiseDb = 10.0f * log10(median + 1e-10f);
    
    // Exponential smoothing
    float alpha = 0.1f;
    current_noise_floor = alpha * noiseDb + (1.0f - alpha) * current_noise_floor;
}

// Apply noise filtering to spectrum
std::vector<float> WildlifeAudioClassifier::applyNoiseFiltering(
    const std::vector<float>& spectrum) {
    
    std::vector<float> filtered = spectrum;
    
    // Simple spectral subtraction
    float noiseLevel = pow(10.0f, current_noise_floor / 10.0f);
    
    for (float& val : filtered) {
        float subtracted = val * val - noiseLevel;
        val = subtracted > 0.0f ? sqrt(subtracted) : 0.0f;
    }
    
    return filtered;
}

// Detect call boundaries
std::vector<std::pair<uint32_t, uint32_t>> WildlifeAudioClassifier::detectCallBoundaries(
    const std::vector<float>& amplitude_data) {
    
    std::vector<std::pair<uint32_t, uint32_t>> boundaries;
    
    if (amplitude_data.empty()) {
        return boundaries;
    }
    
    // Simple energy-based boundary detection
    float threshold = 0.1f;
    bool inCall = false;
    uint32_t callStart = 0;
    
    for (size_t i = 0; i < amplitude_data.size(); i++) {
        if (!inCall && amplitude_data[i] > threshold) {
            inCall = true;
            callStart = i;
        } else if (inCall && amplitude_data[i] <= threshold) {
            inCall = false;
            boundaries.push_back({callStart, i});
        }
    }
    
    return boundaries;
}

// Convert confidence score to level
AudioConfidenceLevel WildlifeAudioClassifier::getConfidenceLevel(float confidence) {
    if (confidence >= 0.8f) return AudioConfidenceLevel::VERY_HIGH;
    if (confidence >= 0.6f) return AudioConfidenceLevel::HIGH;
    if (confidence >= 0.4f) return AudioConfidenceLevel::MEDIUM;
    if (confidence >= 0.2f) return AudioConfidenceLevel::LOW;
    return AudioConfidenceLevel::VERY_LOW;
}

// Log classification result
void WildlifeAudioClassifier::logClassificationResult(
    const AudioClassificationResult& result) {
    
    if (result.confidence > 0.5f) {
        Serial.print("Audio Detection: ");
        Serial.print(result.detected_species);
        Serial.print(" | Confidence: ");
        Serial.print(result.confidence, 2);
        Serial.print(" | Type: ");
        if (result.is_bird_call) Serial.print("Bird");
        else if (result.is_mammal_call) Serial.print("Mammal");
        else Serial.print("Unknown");
        Serial.println();
    }
}

// Wildlife Audio Signatures Implementation
namespace WildlifeAudioSignatures {

std::vector<SpeciesAudioSignature> getBirdSignatures() {
    std::vector<SpeciesAudioSignature> signatures;
    
    // Red-tailed Hawk
    signatures.push_back(createRedTailedHawkSignature());
    
    // Owl
    signatures.push_back(createOwlSignature());
    
    // Wild Turkey
    signatures.push_back(createWildTurkeySignature());
    
    return signatures;
}

std::vector<SpeciesAudioSignature> getMammalSignatures() {
    std::vector<SpeciesAudioSignature> signatures;
    
    // White-tailed Deer
    signatures.push_back(createWhiteTailedDeerSignature());
    
    // Raccoon
    signatures.push_back(createRaccoonSignature());
    
    // Coyote
    signatures.push_back(createCoyoteSignature());
    
    return signatures;
}

std::vector<SpeciesAudioSignature> getRaptorSignatures() {
    std::vector<SpeciesAudioSignature> signatures;
    
    // Red-tailed Hawk (raptor specific)
    signatures.push_back(createRedTailedHawkSignature());
    
    return signatures;
}

SpeciesAudioSignature createWhiteTailedDeerSignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "white_tailed_deer";
    sig.frequency_range_min = 500.0f;
    sig.frequency_range_max = 3000.0f;
    sig.peak_frequency = 1500.0f;
    sig.call_duration_min = 200.0f;
    sig.call_duration_max = 1000.0f;
    sig.repetition_interval = 2000.0f;
    sig.amplitude_threshold = -35.0f;
    return sig;
}

SpeciesAudioSignature createRaccoonSignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "raccoon";
    sig.frequency_range_min = 400.0f;
    sig.frequency_range_max = 2500.0f;
    sig.peak_frequency = 1200.0f;
    sig.call_duration_min = 150.0f;
    sig.call_duration_max = 800.0f;
    sig.repetition_interval = 1500.0f;
    sig.amplitude_threshold = -30.0f;
    return sig;
}

SpeciesAudioSignature createCoyoteSignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "coyote";
    sig.frequency_range_min = 300.0f;
    sig.frequency_range_max = 2000.0f;
    sig.peak_frequency = 800.0f;
    sig.call_duration_min = 500.0f;
    sig.call_duration_max = 3000.0f;
    sig.repetition_interval = 3000.0f;
    sig.amplitude_threshold = -25.0f;
    return sig;
}

SpeciesAudioSignature createOwlSignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "owl";
    sig.frequency_range_min = 200.0f;
    sig.frequency_range_max = 1500.0f;
    sig.peak_frequency = 600.0f;
    sig.call_duration_min = 300.0f;
    sig.call_duration_max = 1500.0f;
    sig.repetition_interval = 2000.0f;
    sig.amplitude_threshold = -35.0f;
    return sig;
}

SpeciesAudioSignature createRedTailedHawkSignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "red_tailed_hawk";
    sig.frequency_range_min = 2000.0f;
    sig.frequency_range_max = 6000.0f;
    sig.peak_frequency = 4000.0f;
    sig.call_duration_min = 500.0f;
    sig.call_duration_max = 2000.0f;
    sig.repetition_interval = 3000.0f;
    sig.amplitude_threshold = -30.0f;
    return sig;
}

SpeciesAudioSignature createWildTurkeySignature() {
    SpeciesAudioSignature sig;
    sig.species_name = "wild_turkey";
    sig.frequency_range_min = 300.0f;
    sig.frequency_range_max = 1500.0f;
    sig.peak_frequency = 800.0f;
    sig.call_duration_min = 200.0f;
    sig.call_duration_max = 1000.0f;
    sig.repetition_interval = 1000.0f;
    sig.amplitude_threshold = -28.0f;
    return sig;
}

} // namespace WildlifeAudioSignatures

// Audio-Visual Fusion Implementation
namespace AudioVisualFusion {

float calculateSpeciesCorrelation(const String& audio_species,
                                 const String& visual_species,
                                 float time_correlation) {
    // Exact match
    if (audio_species == visual_species) {
        return 1.0f * time_correlation;
    }
    
    // Both unknown
    if (audio_species == "unknown" || visual_species == "unknown" ||
        audio_species == "none" || visual_species == "none") {
        return 0.3f * time_correlation;
    }
    
    // Check for related species (simplified taxonomy)
    // In production, would use proper species taxonomy database
    
    // Bird family correlations
    if ((audio_species.indexOf("hawk") >= 0 && visual_species.indexOf("hawk") >= 0) ||
        (audio_species.indexOf("owl") >= 0 && visual_species.indexOf("owl") >= 0)) {
        return 0.7f * time_correlation;
    }
    
    // Mammal family correlations
    if ((audio_species.indexOf("deer") >= 0 && visual_species.indexOf("deer") >= 0) ||
        (audio_species.indexOf("coyote") >= 0 && visual_species.indexOf("coyote") >= 0)) {
        return 0.7f * time_correlation;
    }
    
    // Generic bird-bird or mammal-mammal correlation
    bool audio_is_bird = (audio_species.indexOf("hawk") >= 0 || 
                         audio_species.indexOf("owl") >= 0 ||
                         audio_species.indexOf("turkey") >= 0);
    bool visual_is_bird = (visual_species.indexOf("hawk") >= 0 || 
                          visual_species.indexOf("owl") >= 0 ||
                          visual_species.indexOf("turkey") >= 0);
    
    if (audio_is_bird && visual_is_bird) {
        return 0.5f * time_correlation;
    }
    
    // No correlation
    return 0.2f * time_correlation;
}

float combineConfidenceScores(float audio_confidence,
                              float visual_confidence,
                              float correlation_score) {
    // Weighted fusion based on correlation
    float audio_weight = 0.4f;
    float visual_weight = 0.4f;
    float correlation_weight = 0.2f;
    
    // If highly correlated, increase both weights
    if (correlation_score > 0.7f) {
        audio_weight = 0.45f;
        visual_weight = 0.45f;
        correlation_weight = 0.1f;
    }
    
    float combined = audio_weight * audio_confidence +
                    visual_weight * visual_confidence +
                    correlation_weight * correlation_score;
    
    // Bonus for multi-modal confirmation
    if (audio_confidence > 0.5f && visual_confidence > 0.5f && correlation_score > 0.5f) {
        combined = std::min(combined * 1.2f, 1.0f);
    }
    
    return combined;
}

bool validateDetectionConsistency(const MultiModalResult& result) {
    // Check if audio and visual detections are consistent
    
    // If no audio detection, visual only is consistent
    if (!result.audio_result.is_wildlife) {
        return true;
    }
    
    // If no visual detection, audio only is consistent
    if (!result.has_visual_detection) {
        return true;
    }
    
    // Both detected - check correlation
    if (result.correlation_found) {
        return true;
    }
    
    // If both have high confidence but no correlation, inconsistent
    if (result.audio_result.confidence > 0.6f && result.visual_confidence > 0.6f) {
        return false;
    }
    
    // Otherwise consider consistent (one might be weak)
    return true;
}

} // namespace AudioVisualFusion
