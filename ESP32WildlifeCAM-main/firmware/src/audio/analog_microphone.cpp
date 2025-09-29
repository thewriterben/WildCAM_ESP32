/**
 * @file analog_microphone.cpp
 * @brief Analog Microphone Implementation
 */

#include "analog_microphone.h"
#include "../debug_utils.h"
#include <math.h>

// Digital Filter Implementation
DigitalFilter::DigitalFilter(float cutoff_freq, float sample_rate) 
    : x1(0.0f), y1(0.0f) {
    // Simple high-pass filter coefficients
    float omega = 2.0f * PI * cutoff_freq / sample_rate;
    float alpha = 1.0f / (1.0f + omega);
    
    a0 = alpha;
    a1 = -alpha;
    b1 = alpha - 1.0f;
}

float DigitalFilter::process(float input) {
    float output = a0 * input + a1 * x1 - b1 * y1;
    x1 = input;
    y1 = output;
    return output;
}

void DigitalFilter::reset() {
    x1 = y1 = 0.0f;
}

// Analog Microphone Implementation
AnalogMicrophone::AnalogMicrophone() 
    : initialized(false)
    , dc_offset_calibrated(0.0f)
    , last_sample_micros(0)
    , sample_interval_micros(0) {
}

AnalogMicrophone::~AnalogMicrophone() {
    if (status.recording) {
        stopRecording();
    }
}

bool AnalogMicrophone::init(const AnalogMicrophoneConfig& micConfig) {
    if (initialized) {
        DEBUG_PRINTLN("Analog microphone already initialized");
        return true;
    }
    
    DEBUG_PRINTLN("Initializing analog microphone...");
    
    config = micConfig;
    
    // Configure ADC
    if (!configureADC()) {
        DEBUG_PRINTLN("Failed to configure ADC for analog microphone");
        return false;
    }
    
    // Calculate sampling interval
    sample_interval_micros = 1000000 / config.sample_rate;
    
    // Initialize digital filter if enabled
    if (config.enable_filtering) {
        highpass_filter = std::make_unique<DigitalFilter>(
            config.filter_cutoff_hz, config.sample_rate);
    }
    
    // Calibrate DC offset
    calibrateDCOffset();
    
    // Initialize status
    status = AnalogMicrophoneStatus();
    status.initialized = true;
    
    initialized = true;
    DEBUG_PRINTLN("Analog microphone initialized successfully");
    
    return true;
}

bool AnalogMicrophone::configureADC() {
    // Configure ADC resolution and attenuation
    adc1_config_width(config.resolution);
    adc1_config_channel_atten(config.adc_channel, config.attenuation);
    
    // Set ADC pin as input
    pinMode(config.adc_pin, INPUT);
    
    DEBUG_PRINTF("ADC configured: pin %d, channel %d, %d-bit\n", 
                config.adc_pin, config.adc_channel, 
                (config.resolution == ADC_WIDTH_BIT_12) ? 12 : 
                (config.resolution == ADC_WIDTH_BIT_11) ? 11 : 
                (config.resolution == ADC_WIDTH_BIT_10) ? 10 : 9);
    
    return true;
}

bool AnalogMicrophone::startRecording() {
    if (!initialized) {
        DEBUG_PRINTLN("Analog microphone not initialized");
        return false;
    }
    
    if (status.recording) {
        DEBUG_PRINTLN("Already recording");
        return true;
    }
    
    // Reset filter if enabled
    if (highpass_filter) {
        highpass_filter->reset();
    }
    
    // Initialize timing
    last_sample_micros = micros();
    
    status.recording = true;
    status.last_read_time = millis();
    
    DEBUG_PRINTLN("Analog microphone recording started");
    return true;
}

bool AnalogMicrophone::stopRecording() {
    if (!initialized || !status.recording) {
        return true;
    }
    
    status.recording = false;
    DEBUG_PRINTLN("Analog microphone recording stopped");
    
    return true;
}

bool AnalogMicrophone::readSamples(int16_t* buffer, size_t buffer_size, size_t& samples_read) {
    if (!initialized || !status.recording) {
        samples_read = 0;
        return false;
    }
    
    samples_read = 0;
    
    for (size_t i = 0; i < buffer_size; i++) {
        // Wait for proper sampling interval
        if (!waitForNextSample()) {
            break; // Timeout or error
        }
        
        // Read ADC value
        uint16_t raw_value = readADCValue();
        if (raw_value == 0xFFFF) { // Error value
            status.adc_errors++;
            continue;
        }
        
        // Process raw sample
        buffer[i] = processRawSample(raw_value);
        samples_read++;
        
        status.raw_adc_value = raw_value;
    }
    
    if (samples_read > 0) {
        // Calculate signal level
        status.signal_level_db = calculateSignalLevel(buffer, samples_read);
        status.last_read_time = millis();
    }
    
    // Update statistics
    updateStatistics(samples_read);
    
    return samples_read > 0;
}

bool AnalogMicrophone::waitForNextSample() {
    unsigned long current_micros = micros();
    unsigned long elapsed = current_micros - last_sample_micros;
    
    if (elapsed < sample_interval_micros) {
        // Wait for proper interval
        unsigned long wait_time = sample_interval_micros - elapsed;
        if (wait_time > 10000) { // Don't wait more than 10ms
            return false;
        }
        delayMicroseconds(wait_time);
    }
    
    last_sample_micros = micros();
    return true;
}

uint16_t AnalogMicrophone::readADCValue() {
    int raw_value = adc1_get_raw(config.adc_channel);
    if (raw_value < 0) {
        return 0xFFFF; // Error indicator
    }
    return (uint16_t)raw_value;
}

int16_t AnalogMicrophone::processRawSample(uint16_t raw_value) {
    // Convert ADC value to voltage
    float voltage = AnalogMicrophoneUtils::adcToVoltage(raw_value, config.attenuation);
    
    // Remove DC offset
    voltage -= dc_offset_calibrated;
    
    // Apply gain
    voltage *= config.gain_factor;
    
    // Apply digital filtering if enabled
    if (highpass_filter && config.enable_filtering) {
        voltage = highpass_filter->process(voltage);
    }
    
    // Convert to 16-bit signed sample
    return AnalogMicrophoneUtils::voltageToSample(voltage, 0.0f, 1.0f);
}

float AnalogMicrophone::calculateSignalLevel(const int16_t* samples, size_t count) {
    if (count == 0) return -96.0f;
    
    float sum_squares = 0.0f;
    for (size_t i = 0; i < count; i++) {
        float sample = samples[i] / 32768.0f;
        sum_squares += sample * sample;
    }
    
    float rms = sqrtf(sum_squares / count);
    return (rms > 0.0f) ? 20.0f * log10f(rms) : -96.0f;
}

void AnalogMicrophone::updateStatistics(size_t samples_read) {
    status.samples_read += samples_read;
}

float AnalogMicrophone::calibrateDCOffset(uint32_t duration_ms) {
    DEBUG_PRINTF("Calibrating DC offset for %d ms...\n", duration_ms);
    
    const size_t num_samples = (config.sample_rate * duration_ms) / 1000;
    float voltage_sum = 0.0f;
    size_t valid_samples = 0;
    
    unsigned long start_time = millis();
    unsigned long end_time = start_time + duration_ms;
    
    while (millis() < end_time && valid_samples < num_samples) {
        uint16_t raw_value = readADCValue();
        if (raw_value != 0xFFFF) {
            float voltage = AnalogMicrophoneUtils::adcToVoltage(raw_value, config.attenuation);
            voltage_sum += voltage;
            valid_samples++;
        }
        
        // Wait for next sample
        delayMicroseconds(sample_interval_micros);
    }
    
    if (valid_samples > 0) {
        dc_offset_calibrated = voltage_sum / valid_samples;
    } else {
        dc_offset_calibrated = config.bias_voltage; // Use expected bias
    }
    
    status.dc_offset = dc_offset_calibrated;
    
    DEBUG_PRINTF("DC offset calibrated to %.3f V\n", dc_offset_calibrated);
    return dc_offset_calibrated;
}

void AnalogMicrophone::setGain(float gain) {
    config.gain_factor = gain;
    DEBUG_PRINTF("Analog microphone gain set to %.2f\n", gain);
}

void AnalogMicrophone::setFiltering(bool enable) {
    config.enable_filtering = enable;
    if (enable && !highpass_filter) {
        highpass_filter = std::make_unique<DigitalFilter>(
            config.filter_cutoff_hz, config.sample_rate);
    }
    DEBUG_PRINTF("Analog microphone filtering %s\n", enable ? "enabled" : "disabled");
}

void AnalogMicrophone::setFilterCutoff(uint16_t cutoff_hz) {
    config.filter_cutoff_hz = cutoff_hz;
    if (config.enable_filtering) {
        highpass_filter = std::make_unique<DigitalFilter>(cutoff_hz, config.sample_rate);
    }
    DEBUG_PRINTF("Filter cutoff set to %d Hz\n", cutoff_hz);
}

size_t AnalogMicrophone::getRecommendedBufferSize(uint32_t duration_ms) const {
    return (config.sample_rate * duration_ms) / 1000;
}

void AnalogMicrophone::resetStatistics() {
    status.samples_read = 0;
    status.adc_errors = 0;
    DEBUG_PRINTLN("Analog microphone statistics reset");
}

AnalogMicrophone::TestResult AnalogMicrophone::testMicrophone() {
    TestResult result;
    result.functional = false;
    
    if (!initialized) {
        result.quality_assessment = "Not initialized";
        return result;
    }
    
    // Test basic functionality
    bool was_recording = isRecording();
    if (!was_recording) {
        startRecording();
    }
    
    const size_t buffer_size = 1024;
    int16_t buffer[buffer_size];
    size_t samples_read;
    
    if (readSamples(buffer, buffer_size, samples_read)) {
        result.functional = true;
        result.dc_offset = dc_offset_calibrated;
        
        // Calculate signal range
        int16_t min_val = 32767, max_val = -32768;
        for (size_t i = 0; i < samples_read; i++) {
            if (buffer[i] < min_val) min_val = buffer[i];
            if (buffer[i] > max_val) max_val = buffer[i];
        }
        result.signal_range_v = (max_val - min_val) / 32768.0f * 3.3f;
        
        // Estimate SNR (simplified)
        float signal_level = calculateSignalLevel(buffer, samples_read);
        result.snr_db = signal_level + 60.0f; // Approximate SNR
        
        // Quality assessment
        if (result.snr_db > 50.0f) {
            result.quality_assessment = "Excellent";
        } else if (result.snr_db > 40.0f) {
            result.quality_assessment = "Good";
        } else if (result.snr_db > 30.0f) {
            result.quality_assessment = "Fair";
        } else {
            result.quality_assessment = "Poor - check connections";
        }
    } else {
        result.quality_assessment = "Not responding";
    }
    
    if (!was_recording) {
        stopRecording();
    }
    
    return result;
}

// Utility functions implementation
namespace AnalogMicrophoneUtils {
    float adcToVoltage(uint16_t adc_value, adc_atten_t attenuation) {
        float max_voltage;
        switch (attenuation) {
            case ADC_ATTEN_DB_0:   max_voltage = 1.1f; break;
            case ADC_ATTEN_DB_2_5: max_voltage = 1.5f; break;
            case ADC_ATTEN_DB_6:   max_voltage = 2.2f; break;
            case ADC_ATTEN_DB_11:  max_voltage = 3.3f; break;
            default:               max_voltage = 3.3f; break;
        }
        
        return (adc_value / 4095.0f) * max_voltage;
    }
    
    int16_t voltageToSample(float voltage, float bias_voltage, float gain) {
        // Remove bias and apply gain
        float signal = (voltage - bias_voltage) * gain;
        
        // Convert to 16-bit signed with clipping
        float sample = signal * 32768.0f / 3.3f; // Assuming 3.3V reference
        
        if (sample > 32767.0f) return 32767;
        if (sample < -32768.0f) return -32768;
        
        return (int16_t)sample;
    }
    
    String detectMicrophoneType(AnalogMicrophone& microphone) {
        if (!microphone.getStatus().initialized) {
            return "Not initialized";
        }
        
        AnalogMicrophone::TestResult test = microphone.testMicrophone();
        
        if (!test.functional) {
            return "No microphone detected";
        }
        
        // Simple detection based on characteristics
        if (test.dc_offset > 1.4f && test.dc_offset < 1.8f) {
            return "Electret microphone (standard bias)";
        } else if (test.dc_offset > 0.8f && test.dc_offset < 1.2f) {
            return "MEMS microphone (low bias)";
        } else if (test.dc_offset < 0.2f) {
            return "Dynamic microphone (no bias)";
        } else {
            return "Unknown microphone type";
        }
    }
    
    float calculateSNR(AnalogMicrophone& microphone, uint32_t test_duration_ms) {
        // Simplified SNR calculation
        AnalogMicrophone::TestResult test = microphone.testMicrophone();
        return test.snr_db; // Use the test result
    }
}