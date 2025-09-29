/**
 * @file analog_microphone.h
 * @brief Analog Microphone Interface via ADC
 * 
 * Provides analog microphone support using ESP32 ADC with configurable
 * sampling rates, gain control, and signal conditioning.
 */

#ifndef ANALOG_MICROPHONE_H
#define ANALOG_MICROPHONE_H

#include "../config.h"
#include <Arduino.h>
#include <driver/adc.h>

/**
 * Analog Microphone Configuration
 */
struct AnalogMicrophoneConfig {
    uint8_t adc_pin;            // ADC pin for microphone
    adc1_channel_t adc_channel; // ADC channel
    adc_atten_t attenuation;    // ADC attenuation
    adc_bits_width_t resolution; // ADC resolution
    uint32_t sample_rate;       // Sampling rate in Hz
    float bias_voltage;         // Bias voltage for electret mics
    float gain_factor;          // Software gain multiplier
    bool enable_filtering;      // Enable digital filtering
    uint16_t filter_cutoff_hz;  // High-pass filter cutoff
    
    AnalogMicrophoneConfig() 
        : adc_pin(ANALOG_MIC_PIN)
        , adc_channel(ADC1_CHANNEL_0)  // GPIO36
        , attenuation(ADC_ATTEN_DB_11) // 0-3.3V range
        , resolution(ADC_WIDTH_BIT_12) // 12-bit resolution
        , sample_rate(ADC_SAMPLE_RATE)
        , bias_voltage(ANALOG_MIC_BIAS_VOLTAGE)
        , gain_factor(ANALOG_MIC_GAIN)
        , enable_filtering(true)
        , filter_cutoff_hz(80) {
    }
};

/**
 * Analog Microphone Status
 */
struct AnalogMicrophoneStatus {
    bool initialized;
    bool recording;
    uint32_t samples_read;
    uint32_t adc_errors;
    float signal_level_db;
    float dc_offset;
    unsigned long last_read_time;
    uint16_t raw_adc_value;
    
    AnalogMicrophoneStatus() 
        : initialized(false)
        , recording(false)
        , samples_read(0)
        , adc_errors(0)
        , signal_level_db(-96.0f)
        , dc_offset(0.0f)
        , last_read_time(0)
        , raw_adc_value(0) {
    }
};

/**
 * Digital Filter for analog signal processing
 */
class DigitalFilter {
public:
    DigitalFilter(float cutoff_freq, float sample_rate);
    float process(float input);
    void reset();
    
private:
    float a0, a1, b1;  // Filter coefficients
    float x1, y1;      // Previous samples
};

/**
 * Analog Microphone Handler
 */
class AnalogMicrophone {
public:
    /**
     * Constructor
     */
    AnalogMicrophone();
    
    /**
     * Destructor
     */
    ~AnalogMicrophone();
    
    /**
     * Initialize analog microphone
     * @param config Analog microphone configuration
     * @return true if initialization successful
     */
    bool init(const AnalogMicrophoneConfig& config = AnalogMicrophoneConfig());
    
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
     * @return true if read successful
     */
    bool readSamples(int16_t* buffer, size_t buffer_size, size_t& samples_read);
    
    /**
     * Check if microphone is currently recording
     */
    bool isRecording() const { return status.recording; }
    
    /**
     * Get microphone status
     */
    AnalogMicrophoneStatus getStatus() const { return status; }
    
    /**
     * Get current signal level in dB
     */
    float getSignalLevel() const { return status.signal_level_db; }
    
    /**
     * Calibrate DC offset and bias
     * @param duration_ms Calibration duration in milliseconds
     * @return Measured DC offset
     */
    float calibrateDCOffset(uint32_t duration_ms = 3000);
    
    /**
     * Set gain factor
     * @param gain Software gain multiplier
     */
    void setGain(float gain);
    
    /**
     * Enable/disable digital filtering
     * @param enable true to enable filtering
     */
    void setFiltering(bool enable);
    
    /**
     * Set filter cutoff frequency
     * @param cutoff_hz Cutoff frequency in Hz
     */
    void setFilterCutoff(uint16_t cutoff_hz);
    
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
    
    /**
     * Test microphone functionality
     * @return Test results and quality metrics
     */
    struct TestResult {
        bool functional;
        float snr_db;
        float dc_offset;
        float signal_range_v;
        String quality_assessment;
    };
    
    TestResult testMicrophone();

private:
    AnalogMicrophoneConfig config;
    AnalogMicrophoneStatus status;
    bool initialized;
    float dc_offset_calibrated;
    std::unique_ptr<DigitalFilter> highpass_filter;
    
    // Timing for precise sampling
    unsigned long last_sample_micros;
    uint32_t sample_interval_micros;
    
    // Private methods
    bool configureADC();
    uint16_t readADCValue();
    int16_t processRawSample(uint16_t raw_value);
    float calculateSignalLevel(const int16_t* samples, size_t count);
    void updateStatistics(size_t samples_read);
    bool waitForNextSample();
};

/**
 * Analog Microphone Utilities
 */
namespace AnalogMicrophoneUtils {
    /**
     * Convert ADC value to voltage
     */
    float adcToVoltage(uint16_t adc_value, adc_atten_t attenuation);
    
    /**
     * Convert voltage to 16-bit signed sample
     */
    int16_t voltageToSample(float voltage, float bias_voltage, float gain);
    
    /**
     * Detect electret microphone type
     */
    String detectMicrophoneType(AnalogMicrophone& microphone);
    
    /**
     * Calculate SNR of analog microphone
     */
    float calculateSNR(AnalogMicrophone& microphone, uint32_t test_duration_ms = 5000);
    
    /**
     * Optimize ADC settings for microphone
     */
    AnalogMicrophoneConfig optimizeADCSettings(AnalogMicrophone& microphone);
    
    /**
     * Generate test tone analysis
     */
    struct FrequencyResponse {
        std::vector<float> frequencies;
        std::vector<float> amplitudes;
        float flatness_score;
    };
    
    FrequencyResponse analyzeFrequencyResponse(AnalogMicrophone& microphone);
}

#endif // ANALOG_MICROPHONE_H