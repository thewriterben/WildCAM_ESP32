/**
 * @file power_safety_impl.h
 * @brief Safe Power Management Implementation Patterns
 * @author WildCAM ESP32 Project
 * 
 * This file provides implementation patterns for safe power management
 * operations with proper error handling and resource cleanup.
 */

#ifndef POWER_SAFETY_IMPL_H
#define POWER_SAFETY_IMPL_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include "../utils/logger.h"

/**
 * @brief Battery voltage reading with filtering and validation
 * 
 * Provides averaged readings with outlier rejection for stable
 * battery voltage measurements.
 */
class SafeBatteryMonitor {
private:
    static constexpr size_t SAMPLE_COUNT = 5;
    static constexpr float MIN_VALID_VOLTAGE = 2.5f;  // LiPo minimum
    static constexpr float MAX_VALID_VOLTAGE = 4.3f;  // LiPo maximum
    
    float m_samples[SAMPLE_COUNT];
    size_t m_sample_index;
    uint8_t m_adc_pin;
    float m_voltage_divider_ratio;
    
public:
    /**
     * @brief Constructor
     * @param adc_pin ADC pin for voltage reading
     * @param voltage_divider_ratio Voltage divider ratio (e.g., 2.0 for 1:1 divider)
     */
    SafeBatteryMonitor(uint8_t adc_pin, float voltage_divider_ratio = 2.0f)
        : m_sample_index(0)
        , m_adc_pin(adc_pin)
        , m_voltage_divider_ratio(voltage_divider_ratio) {
        
        // Initialize samples
        for (size_t i = 0; i < SAMPLE_COUNT; i++) {
            m_samples[i] = 0.0f;
        }
        
        // Configure ADC
        pinMode(m_adc_pin, INPUT);
        analogReadResolution(12);  // 12-bit ADC on ESP32
        analogSetAttenuation(ADC_11db);  // 0-3.3V range
    }
    
    /**
     * @brief Read battery voltage with filtering
     * @return Filtered voltage or -1.0f on error
     */
    float readVoltage() {
        // Read ADC value
        int adc_value = analogRead(m_adc_pin);
        if (adc_value < 0 || adc_value > 4095) {
            Logger::error("Invalid ADC reading: %d", adc_value);
            return -1.0f;
        }
        
        // Convert to voltage
        float voltage = (adc_value / 4095.0f) * 3.3f * m_voltage_divider_ratio;
        
        // Validate range
        if (voltage < MIN_VALID_VOLTAGE || voltage > MAX_VALID_VOLTAGE) {
            Logger::warning("Voltage out of valid range: %.2fV", voltage);
            return -1.0f;
        }
        
        // Store sample
        m_samples[m_sample_index] = voltage;
        m_sample_index = (m_sample_index + 1) % SAMPLE_COUNT;
        
        // Calculate average (skip zero samples on startup)
        float sum = 0.0f;
        size_t valid_samples = 0;
        for (size_t i = 0; i < SAMPLE_COUNT; i++) {
            if (m_samples[i] > 0.0f) {
                sum += m_samples[i];
                valid_samples++;
            }
        }
        
        if (valid_samples == 0) {
            return -1.0f;
        }
        
        return sum / valid_samples;
    }
    
    /**
     * @brief Calculate battery percentage from voltage
     * @param voltage Battery voltage
     * @return Battery percentage (0-100) or -1 on error
     */
    static int8_t voltageToPercentage(float voltage) {
        if (voltage < MIN_VALID_VOLTAGE || voltage > MAX_VALID_VOLTAGE) {
            return -1;
        }
        
        // LiPo voltage curve approximation
        // 4.2V = 100%, 3.7V = 50%, 3.0V = 0%
        const float v_max = 4.2f;
        const float v_min = 3.0f;
        
        if (voltage >= v_max) return 100;
        if (voltage <= v_min) return 0;
        
        // Linear approximation
        float percentage = ((voltage - v_min) / (v_max - v_min)) * 100.0f;
        return constrain((int8_t)percentage, 0, 100);
    }
};

/**
 * @brief Safe deep sleep configuration with wake source validation
 */
class SafeDeepSleep {
public:
    /**
     * @brief Configure and enter deep sleep safely
     * @param sleep_duration_sec Sleep duration in seconds (0 = indefinite)
     * @param enable_ext0_wakeup Enable external GPIO wakeup (EXT0)
     * @param ext0_gpio GPIO for EXT0 wakeup
     * @param ext0_level Wake level (0 or 1)
     * @return true if sleep configured successfully, false otherwise
     */
    static bool enter(uint32_t sleep_duration_sec = 0, 
                     bool enable_ext0_wakeup = false,
                     gpio_num_t ext0_gpio = GPIO_NUM_NC,
                     int ext0_level = 0) {
        
        Logger::info("Preparing for deep sleep...");
        
        // Configure timer wakeup if duration specified
        if (sleep_duration_sec > 0) {
            uint64_t sleep_duration_us = (uint64_t)sleep_duration_sec * 1000000ULL;
            esp_err_t err = esp_sleep_enable_timer_wakeup(sleep_duration_us);
            if (err != ESP_OK) {
                Logger::error("Failed to configure timer wakeup: 0x%x", err);
                return false;
            }
            Logger::info("Timer wakeup configured: %us", sleep_duration_sec);
        }
        
        // Configure external GPIO wakeup if enabled
        if (enable_ext0_wakeup) {
            if (ext0_gpio == GPIO_NUM_NC) {
                Logger::error("Invalid GPIO for EXT0 wakeup");
                return false;
            }
            
            esp_err_t err = esp_sleep_enable_ext0_wakeup(ext0_gpio, ext0_level);
            if (err != ESP_OK) {
                Logger::error("Failed to configure EXT0 wakeup: 0x%x", err);
                return false;
            }
            Logger::info("EXT0 wakeup configured: GPIO %d, level %d", ext0_gpio, ext0_level);
        }
        
        // Ensure all pending operations complete
        Serial.flush();
        delay(100);
        
        Logger::info("Entering deep sleep...");
        
        // Enter deep sleep (does not return)
        esp_deep_sleep_start();
        
        // Never reached
        return true;
    }
    
    /**
     * @brief Get wake up cause after deep sleep
     * @return Human-readable wake up cause string
     */
    static const char* getWakeupCause() {
        esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
        
        switch (cause) {
            case ESP_SLEEP_WAKEUP_TIMER:
                return "Timer";
            case ESP_SLEEP_WAKEUP_EXT0:
                return "External GPIO (EXT0)";
            case ESP_SLEEP_WAKEUP_EXT1:
                return "External GPIO (EXT1)";
            case ESP_SLEEP_WAKEUP_TOUCHPAD:
                return "Touch Pad";
            case ESP_SLEEP_WAKEUP_ULP:
                return "ULP Program";
            case ESP_SLEEP_WAKEUP_GPIO:
                return "GPIO";
            case ESP_SLEEP_WAKEUP_UART:
                return "UART";
            case ESP_SLEEP_WAKEUP_WIFI:
                return "WiFi";
            case ESP_SLEEP_WAKEUP_COCPU:
                return "Co-processor";
            case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
                return "Co-processor Trap Trigger";
            case ESP_SLEEP_WAKEUP_BT:
                return "Bluetooth";
            default:
                return "Unknown/Reset";
        }
    }
    
    /**
     * @brief Validate power down options before sleep
     * @return true if power down options are safe
     */
    static bool validatePowerDownOptions() {
        // Check if WiFi is properly shut down
        if (WiFi.getMode() != WIFI_MODE_NULL) {
            Logger::warning("WiFi not disabled before sleep");
            WiFi.mode(WIFI_OFF);
            delay(100);
        }
        
        // Ensure all tasks are suspended or cleaned up
        // This should be done by the application
        
        return true;
    }
};

/**
 * @brief Safe watchdog timer management
 */
class SafeWatchdog {
private:
    uint32_t m_timeout_sec;
    bool m_enabled;
    
public:
    /**
     * @brief Constructor
     * @param timeout_sec Watchdog timeout in seconds
     */
    SafeWatchdog(uint32_t timeout_sec = 30) 
        : m_timeout_sec(timeout_sec), m_enabled(false) {}
    
    /**
     * @brief Initialize and enable watchdog
     * @return true if successful
     */
    bool enable() {
        if (m_enabled) {
            Logger::warning("Watchdog already enabled");
            return true;
        }
        
        esp_err_t err = esp_task_wdt_init(m_timeout_sec, true);
        if (err != ESP_OK) {
            Logger::error("Failed to initialize watchdog: 0x%x", err);
            return false;
        }
        
        err = esp_task_wdt_add(NULL);  // Add current task
        if (err != ESP_OK) {
            Logger::error("Failed to add task to watchdog: 0x%x", err);
            return false;
        }
        
        m_enabled = true;
        Logger::info("Watchdog enabled: %us timeout", m_timeout_sec);
        return true;
    }
    
    /**
     * @brief Reset watchdog timer
     */
    void reset() {
        if (m_enabled) {
            esp_task_wdt_reset();
        }
    }
    
    /**
     * @brief Disable watchdog
     */
    void disable() {
        if (m_enabled) {
            esp_task_wdt_delete(NULL);
            esp_task_wdt_deinit();
            m_enabled = false;
            Logger::info("Watchdog disabled");
        }
    }
    
    /**
     * @brief Destructor - ensures cleanup
     */
    ~SafeWatchdog() {
        disable();
    }
};

/**
 * @brief Power state validation helper
 */
inline bool validatePowerState(float voltage, float temperature_c) {
    // Validate battery voltage
    if (voltage < 3.0f) {
        Logger::error("Critical battery voltage: %.2fV", voltage);
        return false;
    }
    
    // Validate temperature range
    const float MIN_TEMP = -20.0f;
    const float MAX_TEMP = 70.0f;
    
    if (temperature_c < MIN_TEMP || temperature_c > MAX_TEMP) {
        Logger::error("Temperature out of safe range: %.1f°C", temperature_c);
        return false;
    }
    
    return true;
}

/**
 * @brief Calculate estimated runtime
 * @param battery_capacity_mah Battery capacity in mAh
 * @param current_draw_ma Current draw in mA
 * @param battery_percent Current battery percentage (0-100)
 * @return Estimated runtime in hours
 */
inline float estimateRuntime(uint32_t battery_capacity_mah, 
                             float current_draw_ma,
                             uint8_t battery_percent) {
    if (current_draw_ma <= 0.0f) {
        return 0.0f;
    }
    
    float remaining_capacity = (battery_capacity_mah * battery_percent) / 100.0f;
    float runtime_hours = remaining_capacity / current_draw_ma;
    
    // Add safety margin (80% of calculated runtime)
    return runtime_hours * 0.8f;
}

#endif // POWER_SAFETY_IMPL_H
