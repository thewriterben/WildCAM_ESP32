/**
 * @file acoustic_detection.cpp
 * @brief Sound-based Detection System Implementation
 * 
 * Implements comprehensive acoustic detection and analysis capabilities for
 * wildlife monitoring including call classification, environmental sound
 * filtering, and camera triggering.
 */

#include "acoustic_detection.h"
#include "../config.h"
#include "../debug_utils.h"
#include <driver/i2s.h>
#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Static members initialization
bool AcousticDetection::g_audioTaskRunning = false;
TaskHandle_t AcousticDetection::g_audioTaskHandle = nullptr;

AcousticDetection::AcousticDetection() 
    : initialized(false)
    , recording(false)
    , currentScenario(RaptorScenario::GENERAL_MONITORING)
    , audioBuffer(nullptr)
    , fftBuffer(nullptr)
    , spectrogramBuffer(nullptr)
    , sampleRate(I2S_SAMPLE_RATE)
    , bufferSize(0)
    , lastProcessingTime(0)
    , totalSamplesProcessed(0) {
    
    // Initialize performance metrics
    performanceMetrics.samplesProcessed = 0;
    performanceMetrics.averageProcessingTime_ms = 0.0f;
    performanceMetrics.callsDetected = 0;
    performanceMetrics.falsePositives = 0;
    performanceMetrics.accuracyRate_percent = 0.0f;
    performanceMetrics.cpuUsage_percent = 0.0f;
    performanceMetrics.memoryUsage_KB = 0.0f;
    
    // Initialize environmental state
    environmentalState.backgroundNoiseLevel_dB = 0.0f;
    environmentalState.windNoiseLevel_dB = 0.0f;
    environmentalState.lastCalibrationTime = 0;
    environmentalState.adaptiveThreshold = SOUND_DETECTION_THRESHOLD;
    environmentalState.noiseProfile.clear();
}

AcousticDetection::~AcousticDetection() {
    if (initialized) {
        cleanup();
    }
}

bool AcousticDetection::init(RaptorScenario scenario, const AudioConfig& config) {
    if (initialized) {
        DEBUG_PRINTLN("Acoustic detection already initialized");
        return true;
    }
    
    DEBUG_PRINTLN("Initializing acoustic detection system...");
    
    // Store configuration
    currentScenario = scenario;
    audioConfig = config;
    sampleRate = config.sampleRate_Hz;
    
    // Calculate buffer sizes
    bufferSize = (config.sampleRate_Hz * AUDIO_BUFFER_SIZE_MS) / 1000;
    
    // Initialize audio system based on configuration
    bool audioInitialized = false;
    
#if I2S_MICROPHONE_ENABLED
    if (!audioInitialized) {
        audioInitialized = initI2SMicrophone();
        if (audioInitialized) {
            DEBUG_PRINTLN("I2S microphone initialized successfully");
        }
    }
#endif

#if ANALOG_MICROPHONE_ENABLED
    if (!audioInitialized) {
        audioInitialized = initAnalogMicrophone();
        if (audioInitialized) {
            DEBUG_PRINTLN("Analog microphone initialized successfully");
        }
    }
#endif

    if (!audioInitialized) {
        DEBUG_PRINTLN("Failed to initialize any microphone");
        return false;
    }
    
    // Allocate audio processing buffers
    if (!allocateBuffers()) {
        DEBUG_PRINTLN("Failed to allocate audio buffers");
        return false;
    }
    
    // Initialize call pattern database
    initializeCallDatabase();
    
    // Start audio processing task
    if (!startAudioTask()) {
        DEBUG_PRINTLN("Failed to start audio processing task");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Acoustic detection system initialized successfully");
    return true;
}

bool AcousticDetection::initI2SMicrophone() {
#if I2S_MICROPHONE_ENABLED
    DEBUG_PRINTLN("Configuring I2S microphone...");
    
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = audioConfig.sampleRate_Hz,
        .bits_per_sample = (i2s_bits_per_sample_t)audioConfig.bitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = AUDIO_DMA_BUFFER_COUNT,
        .dma_buf_len = AUDIO_DMA_BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to install I2S driver: %d\n", result);
        return false;
    }
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    result = i2s_set_pin(I2S_PORT, &pin_config);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to set I2S pins: %d\n", result);
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }
    
    // Clear I2S DMA buffer
    i2s_zero_dma_buffer(I2S_PORT);
    
    microphoneType = MicrophoneType::I2S_DIGITAL;
    return true;
#else
    return false;
#endif
}

bool AcousticDetection::initAnalogMicrophone() {
#if ANALOG_MICROPHONE_ENABLED
    DEBUG_PRINTLN("Configuring analog microphone...");
    
    // Configure ADC for analog microphone
    analogReadResolution(12); // 12-bit resolution
    analogSetAttenuation(ADC_11db); // Set attenuation for 3.3V range
    
    // Configure microphone pin
    pinMode(ANALOG_MIC_PIN, INPUT);
    
    microphoneType = MicrophoneType::ANALOG_ADC;
    return true;
#else
    return false;
#endif
}

bool AcousticDetection::allocateBuffers() {
    try {
        // Allocate main audio buffer
        audioBuffer = std::make_unique<uint8_t[]>(bufferSize * sizeof(int16_t));
        if (!audioBuffer) {
            DEBUG_PRINTLN("Failed to allocate audio buffer");
            return false;
        }
        
        // Allocate FFT buffer
        fftBuffer = std::make_unique<float[]>(audioConfig.fftSize);
        if (!fftBuffer) {
            DEBUG_PRINTLN("Failed to allocate FFT buffer");
            return false;
        }
        
        // Allocate spectrogram buffer
        spectrogramBuffer = std::make_unique<float[]>(audioConfig.fftSize / 2);
        if (!spectrogramBuffer) {
            DEBUG_PRINTLN("Failed to allocate spectrogram buffer");
            return false;
        }
        
        // Initialize audio history buffer
        audioHistory.clear();
        audioHistory.reserve(MAX_AUDIO_HISTORY);
        
        DEBUG_PRINTF("Audio buffers allocated: %d bytes\n", 
                    bufferSize * sizeof(int16_t) + audioConfig.fftSize * sizeof(float) * 2);
        
        return true;
    } catch (const std::bad_alloc& e) {
        DEBUG_PRINTLN("Memory allocation failed for audio buffers");
        return false;
    }
}

void AcousticDetection::initializeCallDatabase() {
    DEBUG_PRINTLN("Initializing call pattern database...");
    
    callDatabase.clear();
    
    // Initialize with basic raptor call patterns
    // Red-tailed Hawk patterns
    CallPattern redTailedHawk;
    redTailedHawk.species = RaptorSpecies::HAWK;  // Using HAWK category
    redTailedHawk.callType = RaptorCallType::TERRITORIAL_CALL;
    redTailedHawk.frequency.minFrequency_Hz = 800;
    redTailedHawk.frequency.maxFrequency_Hz = 3000;
    redTailedHawk.frequency.peakFrequency_Hz = 1800;
    redTailedHawk.temporal.typicalDuration_ms = 1500;
    redTailedHawk.matchingThreshold = 0.7f;
    redTailedHawk.description = "Harsh descending scream";
    callDatabase.push_back(redTailedHawk);
    
    // Cooper's Hawk patterns
    CallPattern coopersHawk;
    coopersHawk.species = RaptorSpecies::HAWK;  // Using HAWK category
    coopersHawk.callType = RaptorCallType::ALARM_CALL;
    coopersHawk.frequency.minFrequency_Hz = 1000;
    coopersHawk.frequency.maxFrequency_Hz = 4000;
    coopersHawk.frequency.peakFrequency_Hz = 2200;
    coopersHawk.temporal.typicalDuration_ms = 800;
    coopersHawk.matchingThreshold = 0.6f;
    coopersHawk.description = "Sharp kek-kek-kek call";
    callDatabase.push_back(coopersHawk);
    
    // Initialize enabled call types (all enabled by default)
    enabledCallTypes.resize(static_cast<size_t>(RaptorCallType::UNKNOWN_CALL) + 1, true);
    
    // Initialize call sensitivities (medium sensitivity by default)
    callSensitivities.resize(static_cast<size_t>(RaptorCallType::UNKNOWN_CALL) + 1, 5);
    
    DEBUG_PRINTF("Call database initialized with %d patterns\n", callDatabase.size());
}

bool AcousticDetection::startAudioTask() {
    if (g_audioTaskRunning) {
        DEBUG_PRINTLN("Audio task already running");
        return true;
    }
    
    BaseType_t result = xTaskCreate(
        audioTaskWrapper,           // Task function
        "AudioProcessing",          // Task name
        AUDIO_TASK_STACK_SIZE,     // Stack size
        this,                      // Task parameter
        AUDIO_TASK_PRIORITY,       // Priority
        &g_audioTaskHandle         // Task handle
    );
    
    if (result != pdPASS) {
        DEBUG_PRINTLN("Failed to create audio processing task");
        return false;
    }
    
    g_audioTaskRunning = true;
    DEBUG_PRINTLN("Audio processing task started");
    return true;
}

void AcousticDetection::audioTaskWrapper(void* parameter) {
    AcousticDetection* detector = static_cast<AcousticDetection*>(parameter);
    detector->audioProcessingTask();
}

void AcousticDetection::audioProcessingTask() {
    DEBUG_PRINTLN("Audio processing task started");
    
    const TickType_t processingInterval = pdMS_TO_TICKS(AUDIO_PROCESSING_INTERVAL);
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while (g_audioTaskRunning && initialized) {
        // Read audio data
        if (readAudioData()) {
            // Process audio for wildlife detection
            processAudioBuffer();
        }
        
        // Update performance metrics
        updatePerformanceMetrics();
        
        // Yield to other tasks
        vTaskDelayUntil(&lastWakeTime, processingInterval);
    }
    
    DEBUG_PRINTLN("Audio processing task ended");
    g_audioTaskHandle = nullptr;
}

bool AcousticDetection::readAudioData() {
    if (microphoneType == MicrophoneType::I2S_DIGITAL) {
        return readI2SAudioData();
    } else if (microphoneType == MicrophoneType::ANALOG_ADC) {
        return readAnalogAudioData();
    }
    return false;
}

bool AcousticDetection::readI2SAudioData() {
#if I2S_MICROPHONE_ENABLED
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, audioBuffer.get(), 
                               bufferSize * sizeof(int16_t), 
                               &bytesRead, portMAX_DELAY);
    
    if (result != ESP_OK) {
        DEBUG_PRINTF("I2S read error: %d\n", result);
        return false;
    }
    
    if (bytesRead == 0) {
        return false;
    }
    
    // Convert bytes to sample count
    size_t samplesRead = bytesRead / sizeof(int16_t);
    
    // Update audio history
    int16_t* samples = reinterpret_cast<int16_t*>(audioBuffer.get());
    for (size_t i = 0; i < samplesRead; i++) {
        audioHistory.push_back(samples[i]);
        if (audioHistory.size() > MAX_AUDIO_HISTORY) {
            audioHistory.pop_front();
        }
    }
    
    totalSamplesProcessed += samplesRead;
    return true;
#else
    return false;
#endif
}

bool AcousticDetection::readAnalogAudioData() {
#if ANALOG_MICROPHONE_ENABLED
    size_t samplesPerBuffer = bufferSize;
    int16_t* samples = reinterpret_cast<int16_t*>(audioBuffer.get());
    
    // Read analog samples at configured rate
    unsigned long targetInterval = 1000000 / ADC_SAMPLE_RATE; // microseconds
    unsigned long lastSampleTime = micros();
    
    for (size_t i = 0; i < samplesPerBuffer; i++) {
        // Wait for proper sample timing
        while (micros() - lastSampleTime < targetInterval) {
            delayMicroseconds(10);
        }
        lastSampleTime = micros();
        
        // Read ADC value and convert to signed 16-bit
        int adcValue = analogRead(ANALOG_MIC_PIN);
        // Convert 12-bit ADC (0-4095) to 16-bit signed (-32768 to 32767)
        samples[i] = (int16_t)((adcValue - 2048) * 16); // Center around 0 and amplify
        
        // Apply gain
        samples[i] = (int16_t)(samples[i] * ANALOG_MIC_GAIN);
        
        // Update audio history
        audioHistory.push_back(samples[i]);
        if (audioHistory.size() > MAX_AUDIO_HISTORY) {
            audioHistory.pop_front();
        }
    }
    
    totalSamplesProcessed += samplesPerBuffer;
    return true;
#else
    return false;
#endif
}

void AcousticDetection::processAudioBuffer() {
    if (!audioBuffer || audioHistory.empty()) {
        return;
    }
    
    unsigned long processingStart = millis();
    
    // Get current audio data
    int16_t* samples = reinterpret_cast<int16_t*>(audioBuffer.get());
    size_t sampleCount = bufferSize;
    
    // Preprocess audio (gain control, noise gating)
    preprocessAudio(samples, sampleCount);
    
    // Detect voice activity
    bool voiceActivity = detectVoiceActivity(samples, sampleCount);
    
    if (voiceActivity) {
        // Compute spectrogram for frequency analysis
        SpectrogramData spectrogram;
        computeSpectrogram(samples, sampleCount, spectrogram);
        
        // Apply noise reduction
        if (audioConfig.enableNoiseReduction) {
            applyNoiseReduction(spectrogramBuffer.get(), audioConfig.fftSize / 2);
        }
        
        // Detect raptor calls
        AcousticDetectionResult result;
        if (detectRaptorCall(spectrogram, result)) {
            // Valid detection found
            result.temporal.startTime_ms = millis();
            result.processingTime_ms = millis() - processingStart;
            
            // Add to detection history
            detectionHistory.push_back(result);
            if (detectionHistory.size() > MAX_DETECTION_HISTORY) {
                detectionHistory.pop_front();
            }
            
            // Trigger camera if enabled
            if (AUDIO_TRIGGERED_CAPTURE && result.confidence > WILDLIFE_DETECTION_THRESHOLD) {
                triggerCameraCapture(result);
            }
            
            DEBUG_PRINTF("Wildlife sound detected: %s (confidence: %.2f)\n", 
                        getSpeciesName(result.likelySpecies), result.confidence);
        }
    }
    
    lastProcessingTime = millis() - processingStart;
}

// Additional methods would continue here...
// This is the core implementation structure

void AcousticDetection::cleanup() {
    DEBUG_PRINTLN("Cleaning up acoustic detection system...");
    
    // Stop audio task
    if (g_audioTaskRunning) {
        g_audioTaskRunning = false;
        if (g_audioTaskHandle) {
            vTaskDelete(g_audioTaskHandle);
            g_audioTaskHandle = nullptr;
        }
    }
    
    // Cleanup I2S
#if I2S_MICROPHONE_ENABLED
    if (microphoneType == MicrophoneType::I2S_DIGITAL) {
        i2s_driver_uninstall(I2S_PORT);
    }
#endif
    
    // Free buffers
    audioBuffer.reset();
    fftBuffer.reset();
    spectrogramBuffer.reset();
    
    // Clear containers
    audioHistory.clear();
    detectionHistory.clear();
    callDatabase.clear();
    
    initialized = false;
    DEBUG_PRINTLN("Acoustic detection system cleaned up");
}

// Placeholder implementations for additional methods
const char* AcousticDetection::getSpeciesName(RaptorSpecies species) {
    switch (species) {
        case RaptorSpecies::EAGLE: return "Eagle";
        case RaptorSpecies::HAWK: return "Hawk";
        case RaptorSpecies::FALCON: return "Falcon";
        case RaptorSpecies::HARRIER: return "Harrier";
        case RaptorSpecies::KITE: return "Kite";
        case RaptorSpecies::BUZZARD: return "Buzzard";
        default: return "Unknown Raptor";
    }
}

void AcousticDetection::triggerCameraCapture(const AcousticDetectionResult& result) {
    // This will be implemented to interface with the camera system
    DEBUG_PRINTF("Triggering camera capture for %s detection\n", 
                getSpeciesName(result.likelySpecies));
}

// Minimal implementations for core methods to allow compilation
void AcousticDetection::preprocessAudio(int16_t* audioData, uint32_t sampleCount) {
    // Apply basic gain control and noise gating
    for (uint32_t i = 0; i < sampleCount; i++) {
        // Simple noise gate
        if (abs(audioData[i]) < (int16_t)(NOISE_GATE_THRESHOLD * 32767)) {
            audioData[i] = 0;
        }
    }
}

void AcousticDetection::computeSpectrogram(const int16_t* audioData, uint32_t sampleCount, SpectrogramData& spectrogram) {
    // Simplified spectrogram computation
    spectrogram.frequencyBins = audioConfig.fftSize / 2;
    spectrogram.timeBins = 1;
    spectrogram.frequencyResolution_Hz = sampleRate / audioConfig.fftSize;
    spectrogram.timeResolution_ms = (audioConfig.fftSize * 1000.0f) / sampleRate;
}

void AcousticDetection::applyNoiseReduction(float* spectrogramData, uint32_t size) {
    // Basic spectral subtraction noise reduction
    for (uint32_t i = 0; i < size; i++) {
        spectrogramData[i] *= audioConfig.noiseReductionLevel;
    }
}

bool AcousticDetection::detectVoiceActivity(const int16_t* audioData, uint32_t sampleCount) {
    // Simple energy-based voice activity detection
    float energy = 0.0f;
    for (uint32_t i = 0; i < sampleCount; i++) {
        energy += (audioData[i] * audioData[i]);
    }
    energy /= sampleCount;
    energy = sqrt(energy) / 32767.0f; // Normalize to 0-1
    
    return energy > audioConfig.vadThreshold;
}

bool AcousticDetection::detectRaptorCall(const SpectrogramData& spectrogram, AcousticDetectionResult& result) {
    // Simplified raptor call detection
    result.soundDetected = true;
    result.raptorCallDetected = true;
    result.callType = RaptorCallType::TERRITORIAL_CALL;
    result.likelySpecies = RaptorSpecies::HAWK;
    result.confidence = 0.8f; // Placeholder confidence
    
    return result.confidence > WILDLIFE_DETECTION_THRESHOLD;
}

void AcousticDetection::updatePerformanceMetrics() {
    performanceMetrics.samplesProcessed = totalSamplesProcessed;
    performanceMetrics.averageProcessingTime_ms = lastProcessingTime;
    performanceMetrics.callsDetected = detectionHistory.size();
    
    // Calculate memory usage
    size_t audioBufferSize = bufferSize * sizeof(int16_t);
    size_t fftBufferSize = audioConfig.fftSize * sizeof(float);
    size_t spectrogramBufferSize = (audioConfig.fftSize / 2) * sizeof(float);
    size_t historySize = audioHistory.size() * sizeof(int16_t);
    
    performanceMetrics.memoryUsage_KB = 
        (audioBufferSize + fftBufferSize + spectrogramBufferSize + historySize) / 1024.0f;
}

bool AcousticDetection::isOperational() const {
    return initialized && g_audioTaskRunning;
}

std::vector<AcousticDetectionResult> AcousticDetection::getCallHistory(uint32_t maxResults) const {
    std::vector<AcousticDetectionResult> history;
    
    // Get most recent detections up to maxResults
    size_t count = std::min(static_cast<size_t>(maxResults), detectionHistory.size());
    if (count > 0) {
        auto it = detectionHistory.end() - count;
        history.assign(it, detectionHistory.end());
    }
    
    return history;
}