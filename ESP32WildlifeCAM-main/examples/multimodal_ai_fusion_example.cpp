/**
 * @file multimodal_ai_fusion_example.cpp
 * @brief Multi-Modal AI Fusion System Example
 * 
 * Demonstrates comprehensive multi-modal wildlife detection combining
 * audio, visual, motion, and environmental sensors with advanced fusion.
 * 
 * Features Demonstrated:
 * - I2S microphone audio capture
 * - Real-time audio preprocessing and feature extraction
 * - Wildlife sound classification
 * - Audio-visual fusion with confidence scoring
 * - Temporal correlation analysis
 * - Power-aware processing with audio-first triggering
 * - Environmental adaptation
 * 
 * Hardware Requirements:
 * - ESP32-S3 with PSRAM
 * - ESP32-CAM compatible camera module
 * - I2S MEMS microphone (INMP441 or similar)
 * - PIR motion sensor
 * - Optional: BME280 environmental sensor
 */

#include <Arduino.h>
#include "../firmware/src/ai/multimodal_ai_system.h"
#include "../firmware/src/ai/tinyml/inference_engine.h"
#include "../src/audio/wildlife_audio_classifier.h"
#include "../firmware/src/audio/i2s_microphone.h"
#include "../firmware/src/audio/acoustic_detection.h"
#include "esp_camera.h"

// ===========================
// Configuration
// ===========================

// I2S Microphone Pins (ESP32-S3)
#define I2S_WS_PIN    42    // Word Select
#define I2S_SCK_PIN   41    // Serial Clock
#define I2S_SD_PIN    2     // Serial Data

// Audio Processing Configuration
#define AUDIO_SAMPLE_RATE   22050
#define AUDIO_BUFFER_SIZE   2048
#define AUDIO_TRIGGER_THRESHOLD -35.0f  // dB

// Power Management
#define ENABLE_AUDIO_FIRST_TRIGGER  true
#define VISUAL_PROCESSING_DELAY_MS  100

// Multi-Modal Fusion
#define MIN_FUSION_CONFIDENCE  0.6f
#define ENABLE_TEMPORAL_ANALYSIS true
#define TEMPORAL_WINDOW_MS     5000

// ===========================
// Global Objects
// ===========================

MultiModalAISystem multiModalSystem;
InferenceEngine inferenceEngine;
WildlifeAudioClassifier audioClassifier;
I2SMicrophone i2sMicrophone;

// Audio processing buffers
float audioBuffer[AUDIO_BUFFER_SIZE];
int16_t rawAudioBuffer[AUDIO_BUFFER_SIZE];

// State tracking
bool systemInitialized = false;
bool audioTriggered = false;
unsigned long lastDetectionTime = 0;
unsigned long audioTriggerTime = 0;

// Statistics
struct SystemStats {
    uint32_t totalDetections = 0;
    uint32_t audioOnlyDetections = 0;
    uint32_t visualOnlyDetections = 0;
    uint32_t multiModalDetections = 0;
    uint32_t falsePositives = 0;
    float averageConfidence = 0.0f;
    float powerSavings_percent = 0.0f;
} stats;

// ===========================
// Function Declarations
// ===========================

bool initializeCamera();
bool initializeMultiModalSystem();
void processAudioFirst();
void processMultiModal();
void handleAudioTrigger(const AudioClassificationResult& audioResult);
void handleMultiModalResult(const MultiModalResult& result);
void printSystemStatus();
void printDetectionReport(const MultiModalResult& result);
CameraFrame captureFrame();
EnvironmentalContext readEnvironmentalSensors();

// ===========================
// Setup
// ===========================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===========================================");
    Serial.println("Multi-Modal AI Fusion System");
    Serial.println("ESP32 Wildlife Camera");
    Serial.println("===========================================\n");
    
    // Initialize camera
    Serial.println("[1/5] Initializing camera...");
    if (!initializeCamera()) {
        Serial.println("ERROR: Camera initialization failed!");
        Serial.println("System will continue without camera");
    } else {
        Serial.println("✓ Camera initialized successfully");
    }
    
    // Initialize multi-modal AI system
    Serial.println("\n[2/5] Initializing Multi-Modal AI System...");
    if (!initializeMultiModalSystem()) {
        Serial.println("ERROR: Multi-Modal AI System initialization failed!");
        return;
    }
    Serial.println("✓ Multi-Modal AI System initialized");
    
    // Initialize I2S microphone
    Serial.println("\n[3/5] Initializing I2S microphone...");
    I2SMicrophoneConfig micConfig;
    micConfig.ws_pin = I2S_WS_PIN;
    micConfig.sck_pin = I2S_SCK_PIN;
    micConfig.sd_pin = I2S_SD_PIN;
    micConfig.sample_rate = AUDIO_SAMPLE_RATE;
    
    if (!i2sMicrophone.init(micConfig)) {
        Serial.println("ERROR: I2S microphone initialization failed!");
        return;
    }
    Serial.println("✓ I2S microphone initialized");
    
    // Initialize wildlife audio classifier
    Serial.println("\n[4/5] Initializing Wildlife Audio Classifier...");
    if (!audioClassifier.initialize(MicrophoneType::I2S_DIGITAL)) {
        Serial.println("ERROR: Audio classifier initialization failed!");
        return;
    }
    Serial.println("✓ Audio classifier initialized");
    
    // Start audio recording
    Serial.println("\n[5/5] Starting audio monitoring...");
    if (!i2sMicrophone.startRecording()) {
        Serial.println("ERROR: Failed to start audio recording!");
        return;
    }
    audioClassifier.setMonitoringEnabled(true);
    Serial.println("✓ Audio monitoring started");
    
    // System ready
    systemInitialized = true;
    
    Serial.println("\n===========================================");
    Serial.println("✓ System Initialization Complete");
    Serial.println("===========================================\n");
    
    Serial.println("Operating Mode: Audio-First Triggered Visual Processing");
    Serial.println("This saves power by only activating camera when audio detects wildlife\n");
    
    printSystemStatus();
}

// ===========================
// Main Loop
// ===========================

void loop() {
    if (!systemInitialized) {
        delay(1000);
        return;
    }
    
    if (ENABLE_AUDIO_FIRST_TRIGGER) {
        // Power-efficient mode: Audio triggers visual processing
        processAudioFirst();
    } else {
        // Continuous multi-modal processing
        processMultiModal();
    }
    
    // Print status every 30 seconds
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint > 30000) {
        printSystemStatus();
        lastStatusPrint = millis();
    }
    
    delay(10); // Small delay to prevent watchdog issues
}

// ===========================
// System Initialization
// ===========================

bool initializeCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Use lower resolution for faster processing
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
    
    esp_err_t err = esp_camera_init(&config);
    return (err == ESP_OK);
}

bool initializeMultiModalSystem() {
    // Initialize inference engine
    if (!inferenceEngine.init()) {
        Serial.println("ERROR: Failed to initialize inference engine");
        return false;
    }
    
    // Initialize multi-modal system
    if (!multiModalSystem.init(&inferenceEngine)) {
        Serial.println("ERROR: Failed to initialize multi-modal system");
        return false;
    }
    
    // Configure sensor fusion
    SensorFusionConfig fusionConfig;
    fusionConfig.visualWeight = 0.4f;
    fusionConfig.audioWeight = 0.35f;
    fusionConfig.motionWeight = 0.15f;
    fusionConfig.environmentalWeight = 0.1f;
    fusionConfig.enableAdaptiveWeighting = true;
    fusionConfig.enableContextualFusion = true;
    fusionConfig.fusionConfidenceThreshold = MIN_FUSION_CONFIDENCE;
    fusionConfig.temporalWindow_ms = TEMPORAL_WINDOW_MS;
    
    // Configure audio processing
    AudioProcessingConfig audioConfig;
    audioConfig.sampleRate_Hz = AUDIO_SAMPLE_RATE;
    audioConfig.fftSize = 1024;
    audioConfig.hopSize = 512;
    audioConfig.numMelFilters = 40;
    audioConfig.numMfccCoefs = 13;
    audioConfig.enableVAD = true;
    audioConfig.vadThreshold = 0.3f;
    
    multiModalSystem.configure(fusionConfig, audioConfig);
    
    // Enable features
    multiModalSystem.enableAudioProcessing(true);
    multiModalSystem.enableEnvironmentalAdaptation(true);
    
    if (ENABLE_TEMPORAL_ANALYSIS) {
        multiModalSystem.enableTemporalAnalysis(true);
    }
    
    return true;
}

// ===========================
// Audio-First Processing
// ===========================

void processAudioFirst() {
    // Read audio samples
    size_t samplesRead = 0;
    if (!i2sMicrophone.readSamples(rawAudioBuffer, AUDIO_BUFFER_SIZE, 
                                   samplesRead, 50)) {
        return;
    }
    
    // Convert to float
    for (size_t i = 0; i < samplesRead; i++) {
        audioBuffer[i] = float(rawAudioBuffer[i]) / 32768.0f;
    }
    
    // Classify audio
    AudioClassificationResult audioResult = 
        audioClassifier.processAudioBuffer(rawAudioBuffer, samplesRead);
    
    // Check if audio detected wildlife
    if (audioResult.is_wildlife && audioResult.confidence > 0.4f) {
        Serial.println("\n*** AUDIO TRIGGER DETECTED ***");
        handleAudioTrigger(audioResult);
        
        // Trigger visual processing after short delay
        delay(VISUAL_PROCESSING_DELAY_MS);
        
        // Capture and process visual frame
        CameraFrame frame = captureFrame();
        EnvironmentalContext environment = readEnvironmentalSensors();
        
        // Perform multi-modal analysis
        MultiModalResult result = multiModalSystem.analyzeMultiModal(
            frame, audioBuffer, samplesRead, environment);
        
        handleMultiModalResult(result);
        
        // Calculate power savings
        // Audio processing uses ~20% of camera processing power
        stats.powerSavings_percent = 80.0f * 
            (1.0f - float(stats.multiModalDetections) / 
             (stats.totalDetections + 1));
    }
}

// ===========================
// Continuous Multi-Modal Processing
// ===========================

void processMultiModal() {
    // Capture frame
    CameraFrame frame = captureFrame();
    
    // Read audio
    size_t samplesRead = 0;
    i2sMicrophone.readSamples(rawAudioBuffer, AUDIO_BUFFER_SIZE, 
                             samplesRead, 50);
    
    for (size_t i = 0; i < samplesRead; i++) {
        audioBuffer[i] = float(rawAudioBuffer[i]) / 32768.0f;
    }
    
    // Read environmental sensors
    EnvironmentalContext environment = readEnvironmentalSensors();
    
    // Perform multi-modal analysis
    MultiModalResult result = multiModalSystem.analyzeMultiModal(
        frame, audioBuffer, samplesRead, environment);
    
    handleMultiModalResult(result);
}

// ===========================
// Event Handlers
// ===========================

void handleAudioTrigger(const AudioClassificationResult& audioResult) {
    audioTriggered = true;
    audioTriggerTime = millis();
    
    Serial.print("  Species: ");
    Serial.println(audioResult.detected_species);
    Serial.print("  Confidence: ");
    Serial.print(audioResult.confidence * 100.0f, 1);
    Serial.println("%");
    Serial.print("  Type: ");
    if (audioResult.is_bird_call) Serial.println("Bird Call");
    else if (audioResult.is_mammal_call) Serial.println("Mammal Vocalization");
    else Serial.println("Wildlife Sound");
    
    stats.audioOnlyDetections++;
}

void handleMultiModalResult(const MultiModalResult& result) {
    stats.totalDetections++;
    
    // Check if this is a valid multi-modal detection
    if (result.fusedConfidence >= MIN_FUSION_CONFIDENCE) {
        stats.multiModalDetections++;
        
        // Update average confidence
        stats.averageConfidence = 
            (stats.averageConfidence * (stats.totalDetections - 1) + 
             result.fusedConfidence) / stats.totalDetections;
        
        lastDetectionTime = millis();
        
        // Print detailed report
        printDetectionReport(result);
        
        // Check for threats
        if (result.threatDetected) {
            Serial.println("\n!!! THREAT DETECTED !!!");
            Serial.println("High stress indicators or alarm calls detected");
        }
        
        // Check for human presence
        if (result.humanPresenceDetected) {
            Serial.println("\n*** Human Presence Detected ***");
        }
    } else {
        // Low confidence - possible false positive
        if (result.visualConfidence > 0.3f || result.audioConfidence > 0.3f) {
            stats.falsePositives++;
        }
    }
    
    audioTriggered = false;
}

// ===========================
// Helper Functions
// ===========================

CameraFrame captureFrame() {
    CameraFrame frame;
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
        frame.width = fb->width;
        frame.height = fb->height;
        frame.format = fb->format;
        frame.data = fb->buf;
        frame.size = fb->len;
        frame.timestamp = millis();
        
        // Note: Don't return fb here, caller should manage it
    }
    
    return frame;
}

EnvironmentalContext readEnvironmentalSensors() {
    EnvironmentalContext context;
    
    // Read actual sensors here if available
    // For demo, using placeholder values
    context.temperature_C = 22.0f;
    context.humidity_percent = 60.0f;
    context.pressure_hPa = 1013.25f;
    context.lightLevel_lux = 500.0f;
    context.windSpeed_mps = 2.0f;
    context.precipitation_mm = 0.0f;
    
    // Get time info
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    context.timeOfDay = timeInfo->tm_hour;
    context.dayOfYear = timeInfo->tm_yday;
    
    return context;
}

void printDetectionReport(const MultiModalResult& result) {
    Serial.println("\n========================================");
    Serial.println("  MULTI-MODAL DETECTION REPORT");
    Serial.println("========================================");
    
    // Visual Analysis
    Serial.println("\n[Visual Analysis]");
    Serial.print("  Confidence: ");
    Serial.print(result.visualConfidence * 100.0f, 1);
    Serial.println("%");
    if (result.visualResult.detected) {
        Serial.print("  Species: ");
        Serial.println(int(result.visualResult.detectedSpecies));
        Serial.print("  Behavior: ");
        Serial.println(int(result.visualResult.detectedBehavior));
    } else {
        Serial.println("  No visual detection");
    }
    
    // Audio Analysis
    Serial.println("\n[Audio Analysis]");
    Serial.print("  Confidence: ");
    Serial.print(result.audioConfidence * 100.0f, 1);
    Serial.println("%");
    if (result.audioFeatures.hasVocalization) {
        Serial.print("  Dominant Freq: ");
        Serial.print(result.audioFeatures.dominantFrequency_Hz, 0);
        Serial.println(" Hz");
        Serial.print("  Volume: ");
        Serial.print(result.audioFeatures.volume_dB, 1);
        Serial.println(" dB");
        Serial.print("  Detected Sounds: ");
        for (const auto& sound : result.detectedSounds) {
            Serial.print(sound);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("  No vocalization detected");
    }
    
    // Motion Analysis
    Serial.println("\n[Motion Analysis]");
    Serial.print("  Motion Detected: ");
    Serial.println(result.motionDetected ? "YES" : "NO");
    if (result.motionDetected) {
        Serial.print("  Intensity: ");
        Serial.print(result.motionIntensity * 100.0f, 1);
        Serial.println("%");
    }
    
    // Fusion Results
    Serial.println("\n[Sensor Fusion]");
    Serial.print("  Active Sensors: ");
    for (const auto& sensor : result.activeSensors) {
        Serial.print(sensorModalityToString(sensor));
        Serial.print(" ");
    }
    Serial.println();
    Serial.print("  Fused Confidence: ");
    Serial.print(result.fusedConfidence * 100.0f, 1);
    Serial.println("%");
    Serial.print("  Overall Confidence: ");
    Serial.print(result.overallConfidence * 100.0f, 1);
    Serial.println("%");
    
    // Behavioral Analysis
    Serial.println("\n[Behavioral Analysis]");
    Serial.print("  Fused Species: ");
    Serial.println(int(result.fusedSpeciesDetection));
    Serial.print("  Fused Behavior: ");
    Serial.println(int(result.fusedBehaviorDetection));
    Serial.print("  Stress Level: ");
    Serial.print(result.stressLevel * 100.0f, 1);
    Serial.println("%");
    Serial.print("  Biodiversity Index: ");
    Serial.println(result.biodiversityIndex, 2);
    
    // Environmental Context
    Serial.println("\n[Environmental Context]");
    Serial.print("  Temperature: ");
    Serial.print(result.environment.temperature_C, 1);
    Serial.println(" °C");
    Serial.print("  Light Level: ");
    Serial.print(result.environment.lightLevel_lux, 0);
    Serial.println(" lux");
    Serial.print("  Wind Speed: ");
    Serial.print(result.environment.windSpeed_mps, 1);
    Serial.println(" m/s");
    
    Serial.println("\n========================================\n");
}

void printSystemStatus() {
    Serial.println("\n========================================");
    Serial.println("  SYSTEM STATUS");
    Serial.println("========================================");
    
    // Multi-modal system metrics
    auto metrics = multiModalSystem.getPerformanceMetrics();
    Serial.println("\n[Multi-Modal AI System]");
    Serial.print("  Total Analyses: ");
    Serial.println(metrics.totalAnalyses);
    Serial.print("  Successful Fusions: ");
    Serial.println(metrics.successfulFusions);
    Serial.print("  Avg Processing Time: ");
    Serial.print(metrics.averageProcessingTime_ms, 1);
    Serial.println(" ms");
    Serial.print("  Fusion Accuracy: ");
    Serial.print(metrics.fusionAccuracy_percent, 1);
    Serial.println("%");
    
    // Audio classifier stats
    Serial.println("\n[Audio Classifier]");
    Serial.println(audioClassifier.getStatistics());
    
    // Detection statistics
    Serial.println("\n[Detection Statistics]");
    Serial.print("  Total Detections: ");
    Serial.println(stats.totalDetections);
    Serial.print("  Audio-Only: ");
    Serial.println(stats.audioOnlyDetections);
    Serial.print("  Visual-Only: ");
    Serial.println(stats.visualOnlyDetections);
    Serial.print("  Multi-Modal: ");
    Serial.println(stats.multiModalDetections);
    Serial.print("  False Positives: ");
    Serial.println(stats.falsePositives);
    Serial.print("  Avg Confidence: ");
    Serial.print(stats.averageConfidence * 100.0f, 1);
    Serial.println("%");
    
    if (ENABLE_AUDIO_FIRST_TRIGGER) {
        Serial.print("  Power Savings: ");
        Serial.print(stats.powerSavings_percent, 1);
        Serial.println("%");
    }
    
    // Memory status
    Serial.println("\n[Memory Status]");
    Serial.print("  Free Heap: ");
    Serial.print(ESP.getFreeHeap() / 1024);
    Serial.println(" KB");
    Serial.print("  Min Free Heap: ");
    Serial.print(ESP.getMinFreeHeap() / 1024);
    Serial.println(" KB");
    
    #ifdef BOARD_HAS_PSRAM
    Serial.print("  Free PSRAM: ");
    Serial.print(ESP.getFreePsram() / 1024);
    Serial.println(" KB");
    #endif
    
    Serial.println("\n========================================\n");
}
