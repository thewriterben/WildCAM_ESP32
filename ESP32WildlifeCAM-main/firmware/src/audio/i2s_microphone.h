/**
 * @file i2s_microphone.h
 * @brief I2S Digital Microphone Interface
 * 
 * Provides dedicated I2S microphone support for INMP441 and similar
 * digital microphones with hardware-specific optimizations.
 */

#ifndef I2S_MICROPHONE_H
#define I2S_MICROPHONE_H

#include "../config.h"
#include <Arduino.h>
#include <driver/i2s.h>

/**
 * I2S Microphone Configuration
 */
struct I2SMicrophoneConfig {
    i2s_port_t port;
    int ws_pin;              // Word Select (LR Clock) pin
    int sck_pin;             // Serial Clock pin
    int sd_pin;              // Serial Data pin
    uint32_t sample_rate;    // Sample rate in Hz
    i2s_bits_per_sample_t bits_per_sample;
    i2s_channel_fmt_t channel_format;
    uint32_t dma_buf_count;  // Number of DMA buffers
    uint32_t dma_buf_len;    // Length of each DMA buffer
    bool use_apll;           // Use APLL for better accuracy
    
    I2SMicrophoneConfig() 
        : port(I2S_NUM_0)
        , ws_pin(I2S_WS_PIN)
        , sck_pin(I2S_SCK_PIN)
        , sd_pin(I2S_SD_PIN)
        , sample_rate(I2S_SAMPLE_RATE)
        , bits_per_sample(I2S_BITS_PER_SAMPLE_16BIT)
        , channel_format(I2S_CHANNEL_FMT_ONLY_LEFT)
        , dma_buf_count(AUDIO_DMA_BUFFER_COUNT)
        , dma_buf_len(AUDIO_DMA_BUFFER_SIZE)
        , use_apll(false) {
    }
};

/**
 * I2S Microphone Status
 */
struct I2SMicrophoneStatus {
    bool initialized;
    bool recording;
    uint32_t samples_read;
    uint32_t buffer_overruns;
    uint32_t read_errors;
    float signal_level_db;
    unsigned long last_read_time;
    
    I2SMicrophoneStatus() 
        : initialized(false)
        , recording(false)
        , samples_read(0)
        , buffer_overruns(0)
        , read_errors(0)
        , signal_level_db(-96.0f)
        , last_read_time(0) {
    }
};

/**
 * I2S Digital Microphone Handler
 */
class I2SMicrophone {
public:
    /**
     * Constructor
     */
    I2SMicrophone();
    
    /**
     * Destructor
     */
    ~I2SMicrophone();
    
    /**
     * Initialize I2S microphone
     * @param config I2S configuration parameters
     * @return true if initialization successful
     */
    bool init(const I2SMicrophoneConfig& config = I2SMicrophoneConfig());
    
    /**
     * Start recording from microphone
     * @return true if recording started successfully
     */
    bool startRecording();
    
    /**
     * Stop recording from microphone
     * @return true if recording stopped successfully
     */
    bool stopRecording();
    
    /**
     * Read audio samples from microphone
     * @param buffer Buffer to store audio samples
     * @param buffer_size Size of buffer in samples
     * @param samples_read Number of samples actually read
     * @param timeout_ms Timeout in milliseconds
     * @return true if read successful
     */
    bool readSamples(int16_t* buffer, size_t buffer_size, 
                    size_t& samples_read, uint32_t timeout_ms = 100);
    
    /**
     * Check if microphone is currently recording
     */
    bool isRecording() const { return status.recording; }
    
    /**
     * Get microphone status
     */
    I2SMicrophoneStatus getStatus() const { return status; }
    
    /**
     * Get current signal level in dB
     */
    float getSignalLevel() const { return status.signal_level_db; }
    
    /**
     * Calibrate microphone noise floor
     * @param duration_ms Calibration duration in milliseconds
     * @return Average noise floor in dB
     */
    float calibrateNoiseFloor(uint32_t duration_ms = 5000);
    
    /**
     * Set gain adjustment
     * @param gain_db Gain adjustment in dB
     */
    void setGain(float gain_db);
    
    /**
     * Enable/disable automatic gain control
     * @param enable true to enable AGC
     */
    void setAutoGainControl(bool enable);
    
    /**
     * Get recommended buffer size for given duration
     * @param duration_ms Desired buffer duration in milliseconds
     * @return Recommended buffer size in samples
     */
    size_t getRecommendedBufferSize(uint32_t duration_ms) const;
    
    /**
     * Reset microphone statistics
     */
    void resetStatistics();

private:
    I2SMicrophoneConfig config;
    I2SMicrophoneStatus status;
    bool initialized;
    float gain_db;
    bool auto_gain_enabled;
    float noise_floor_db;
    
    // Private methods
    bool configureI2S();
    bool installI2SDriver();
    void uninstallI2SDriver();
    float calculateSignalLevel(const int16_t* samples, size_t count);
    void updateStatistics(size_t samples_read, bool read_success);
    void applyGainControl(int16_t* samples, size_t count);
};

/**
 * I2S Microphone Utilities
 */
namespace I2SMicrophoneUtils {
    /**
     * Convert dB to linear gain factor
     */
    float dbToLinear(float db);
    
    /**
     * Convert linear amplitude to dB
     */
    float linearToDb(float linear);
    
    /**
     * Calculate RMS value of audio samples
     */
    float calculateRMS(const int16_t* samples, size_t count);
    
    /**
     * Detect microphone type based on response
     * @param microphone Reference to initialized microphone
     * @return Detected microphone model string
     */
    String detectMicrophoneModel(I2SMicrophone& microphone);
    
    /**
     * Test microphone functionality
     * @param microphone Reference to microphone to test
     * @return Test results and recommendations
     */
    struct TestResult {
        bool functional;
        float noise_floor_db;
        float max_signal_db;
        float dynamic_range_db;
        String recommendations;
    };
    
    TestResult testMicrophone(I2SMicrophone& microphone);
}

#endif // I2S_MICROPHONE_H