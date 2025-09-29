/**
 * @file i2s_microphone.cpp
 * @brief I2S Digital Microphone Implementation
 */

#include "i2s_microphone.h"
#include "../debug_utils.h"
#include <math.h>

I2SMicrophone::I2SMicrophone() 
    : initialized(false)
    , gain_db(0.0f)
    , auto_gain_enabled(false)
    , noise_floor_db(-60.0f) {
}

I2SMicrophone::~I2SMicrophone() {
    if (initialized) {
        uninstallI2SDriver();
    }
}

bool I2SMicrophone::init(const I2SMicrophoneConfig& micConfig) {
    if (initialized) {
        DEBUG_PRINTLN("I2S microphone already initialized");
        return true;
    }
    
    DEBUG_PRINTLN("Initializing I2S microphone...");
    
    config = micConfig;
    
    // Configure and install I2S driver
    if (!installI2SDriver()) {
        DEBUG_PRINTLN("Failed to install I2S driver");
        return false;
    }
    
    // Configure I2S pins
    if (!configureI2S()) {
        DEBUG_PRINTLN("Failed to configure I2S pins");
        uninstallI2SDriver();
        return false;
    }
    
    // Clear DMA buffers
    i2s_zero_dma_buffer(config.port);
    
    // Initialize status
    status = I2SMicrophoneStatus();
    status.initialized = true;
    
    initialized = true;
    DEBUG_PRINTLN("I2S microphone initialized successfully");
    
    return true;
}

bool I2SMicrophone::installI2SDriver() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = config.sample_rate,
        .bits_per_sample = config.bits_per_sample,
        .channel_format = config.channel_format,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = config.dma_buf_count,
        .dma_buf_len = config.dma_buf_len,
        .use_apll = config.use_apll,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    esp_err_t result = i2s_driver_install(config.port, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to install I2S driver: %d\n", result);
        return false;
    }
    
    return true;
}

bool I2SMicrophone::configureI2S() {
    i2s_pin_config_t pin_config = {
        .bck_io_num = config.sck_pin,
        .ws_io_num = config.ws_pin,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = config.sd_pin
    };
    
    esp_err_t result = i2s_set_pin(config.port, &pin_config);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to set I2S pins: %d\n", result);
        return false;
    }
    
    return true;
}

void I2SMicrophone::uninstallI2SDriver() {
    if (status.recording) {
        stopRecording();
    }
    
    i2s_driver_uninstall(config.port);
    initialized = false;
    status.initialized = false;
}

bool I2SMicrophone::startRecording() {
    if (!initialized) {
        DEBUG_PRINTLN("Microphone not initialized");
        return false;
    }
    
    if (status.recording) {
        DEBUG_PRINTLN("Already recording");
        return true;
    }
    
    // Start I2S
    esp_err_t result = i2s_start(config.port);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to start I2S: %d\n", result);
        return false;
    }
    
    status.recording = true;
    status.last_read_time = millis();
    
    DEBUG_PRINTLN("I2S microphone recording started");
    return true;
}

bool I2SMicrophone::stopRecording() {
    if (!initialized || !status.recording) {
        return true;
    }
    
    // Stop I2S
    esp_err_t result = i2s_stop(config.port);
    if (result != ESP_OK) {
        DEBUG_PRINTF("Failed to stop I2S: %d\n", result);
        return false;
    }
    
    status.recording = false;
    DEBUG_PRINTLN("I2S microphone recording stopped");
    
    return true;
}

bool I2SMicrophone::readSamples(int16_t* buffer, size_t buffer_size, 
                               size_t& samples_read, uint32_t timeout_ms) {
    if (!initialized || !status.recording) {
        samples_read = 0;
        return false;
    }
    
    size_t bytes_to_read = buffer_size * sizeof(int16_t);
    size_t bytes_read = 0;
    
    esp_err_t result = i2s_read(config.port, buffer, bytes_to_read, 
                               &bytes_read, pdMS_TO_TICKS(timeout_ms));
    
    samples_read = bytes_read / sizeof(int16_t);
    bool success = (result == ESP_OK) && (bytes_read > 0);
    
    if (success) {
        // Apply gain control if enabled
        if (gain_db != 0.0f || auto_gain_enabled) {
            applyGainControl(buffer, samples_read);
        }
        
        // Calculate signal level
        status.signal_level_db = calculateSignalLevel(buffer, samples_read);
        status.last_read_time = millis();
    } else {
        status.read_errors++;
    }
    
    // Update statistics
    updateStatistics(samples_read, success);
    
    return success;
}

float I2SMicrophone::calculateSignalLevel(const int16_t* samples, size_t count) {
    if (count == 0) return -96.0f; // Minimum dB level
    
    float rms = I2SMicrophoneUtils::calculateRMS(samples, count);
    return I2SMicrophoneUtils::linearToDb(rms);
}

void I2SMicrophone::updateStatistics(size_t samples_read, bool read_success) {
    status.samples_read += samples_read;
    
    if (!read_success) {
        status.read_errors++;
    }
    
    // Check for buffer overruns (simplified detection)
    if (millis() - status.last_read_time > 200) { // 200ms without read
        status.buffer_overruns++;
    }
}

void I2SMicrophone::applyGainControl(int16_t* samples, size_t count) {
    float gain_factor = I2SMicrophoneUtils::dbToLinear(gain_db);
    
    for (size_t i = 0; i < count; i++) {
        float sample = samples[i] * gain_factor;
        
        // Clip to 16-bit range
        if (sample > 32767.0f) {
            samples[i] = 32767;
        } else if (sample < -32768.0f) {
            samples[i] = -32768;
        } else {
            samples[i] = (int16_t)sample;
        }
    }
}

float I2SMicrophone::calibrateNoiseFloor(uint32_t duration_ms) {
    if (!initialized) return -96.0f;
    
    DEBUG_PRINTF("Calibrating noise floor for %d ms...\n", duration_ms);
    
    if (!startRecording()) {
        return -96.0f;
    }
    
    const size_t buffer_size = 1024;
    int16_t buffer[buffer_size];
    std::vector<float> signal_levels;
    
    unsigned long start_time = millis();
    unsigned long end_time = start_time + duration_ms;
    
    while (millis() < end_time) {
        size_t samples_read;
        if (readSamples(buffer, buffer_size, samples_read, 100)) {
            float level = calculateSignalLevel(buffer, samples_read);
            signal_levels.push_back(level);
        }
        delay(50); // Small delay between reads
    }
    
    // Calculate average noise floor
    if (signal_levels.empty()) {
        noise_floor_db = -60.0f; // Default
    } else {
        float sum = 0.0f;
        for (float level : signal_levels) {
            sum += level;
        }
        noise_floor_db = sum / signal_levels.size();
    }
    
    DEBUG_PRINTF("Noise floor calibrated to %.1f dB\n", noise_floor_db);
    return noise_floor_db;
}

void I2SMicrophone::setGain(float gain_db_value) {
    gain_db = gain_db_value;
    DEBUG_PRINTF("I2S microphone gain set to %.1f dB\n", gain_db);
}

void I2SMicrophone::setAutoGainControl(bool enable) {
    auto_gain_enabled = enable;
    DEBUG_PRINTF("I2S microphone AGC %s\n", enable ? "enabled" : "disabled");
}

size_t I2SMicrophone::getRecommendedBufferSize(uint32_t duration_ms) const {
    return (config.sample_rate * duration_ms) / 1000;
}

void I2SMicrophone::resetStatistics() {
    status.samples_read = 0;
    status.buffer_overruns = 0;
    status.read_errors = 0;
    DEBUG_PRINTLN("I2S microphone statistics reset");
}

// Utility functions implementation
namespace I2SMicrophoneUtils {
    float dbToLinear(float db) {
        return powf(10.0f, db / 20.0f);
    }
    
    float linearToDb(float linear) {
        if (linear <= 0.0f) return -96.0f; // Minimum dB
        return 20.0f * log10f(linear);
    }
    
    float calculateRMS(const int16_t* samples, size_t count) {
        if (count == 0) return 0.0f;
        
        float sum_squares = 0.0f;
        for (size_t i = 0; i < count; i++) {
            float sample = samples[i] / 32768.0f; // Normalize to -1.0 to 1.0
            sum_squares += sample * sample;
        }
        
        return sqrtf(sum_squares / count);
    }
    
    String detectMicrophoneModel(I2SMicrophone& microphone) {
        // Simplified microphone detection based on response characteristics
        if (!microphone.isRecording()) {
            microphone.startRecording();
        }
        
        float noise_floor = microphone.calibrateNoiseFloor(2000);
        
        if (noise_floor < -55.0f && noise_floor > -75.0f) {
            return "INMP441 (High-quality digital)";
        } else if (noise_floor < -45.0f && noise_floor > -65.0f) {
            return "ICS-43434 (Medium-quality digital)";
        } else if (noise_floor < -35.0f) {
            return "Generic I2S microphone (Lower quality)";
        } else {
            return "Unknown or no microphone detected";
        }
    }
    
    TestResult testMicrophone(I2SMicrophone& microphone) {
        TestResult result;
        result.functional = false;
        
        if (!microphone.getStatus().initialized) {
            result.recommendations = "Microphone not initialized";
            return result;
        }
        
        // Start recording for test
        bool was_recording = microphone.isRecording();
        if (!was_recording) {
            microphone.startRecording();
        }
        
        // Test basic functionality
        const size_t buffer_size = 1024;
        int16_t buffer[buffer_size];
        size_t samples_read;
        
        bool read_success = microphone.readSamples(buffer, buffer_size, samples_read, 1000);
        
        if (read_success && samples_read > 0) {
            result.functional = true;
            
            // Measure noise floor
            result.noise_floor_db = microphone.calibrateNoiseFloor(3000);
            
            // Estimate dynamic range (simplified)
            result.max_signal_db = -6.0f; // Typical max for 16-bit
            result.dynamic_range_db = result.max_signal_db - result.noise_floor_db;
            
            // Generate recommendations
            if (result.noise_floor_db < -60.0f) {
                result.recommendations = "Excellent microphone quality";
            } else if (result.noise_floor_db < -50.0f) {
                result.recommendations = "Good microphone quality";
            } else if (result.noise_floor_db < -40.0f) {
                result.recommendations = "Acceptable quality, consider filtering";
            } else {
                result.recommendations = "Poor quality, check connections";
            }
        } else {
            result.recommendations = "Microphone not responding - check wiring";
        }
        
        // Restore recording state
        if (!was_recording) {
            microphone.stopRecording();
        }
        
        return result;
    }
}