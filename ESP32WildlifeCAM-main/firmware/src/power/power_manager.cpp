/**
 * @file power_manager.cpp
 * @brief Core Power Management System Implementation
 * @author thewriterben
 * @date 2025-01-27
 * @version 0.1.0
 */

#include "../../include/power/power_manager.h"
#include "../../utils/logger.h"
#include <esp_sleep.h>
#include <esp_adc_cal.h>
#include <soc/adc_channel.h>

// Global power manager instance
PowerManager* g_powerManager = nullptr;

PowerManager::PowerManager() 
    : m_initialized(false), m_measurementsValid(false), m_powerSavingEnabled(false),
      m_adcCalibrated(false), m_lastMeasurementTime(0), m_targetPowerState(STATE_NORMAL_OPERATION),
      m_stateChangeTime(0), m_inLowPowerMode(false), m_lastCameraActivity(0),
      m_lastNetworkActivity(0), m_lastSensorActivity(0) {
}

PowerManager::~PowerManager() {
    cleanup();
}

bool PowerManager::initialize(const PowerConfig& config) {
    Logger::info("Initializing Power Management System...");
    
    m_config = config;
    
    // Initialize ADC for voltage measurements
    if (!initializeADC()) {
        Logger::error("Failed to initialize ADC for power monitoring");
        return false;
    }
    
    // Initialize GPIO pins
    if (!initializeGPIO()) {
        Logger::error("Failed to initialize GPIO for power management");
        return false;
    }
    
    // Resolve GPIO conflicts for AI-Thinker ESP32-CAM
    if (!resolvePinConflicts()) {
        Logger::warning("GPIO pin conflicts detected - some features may be limited");
    }
    
    // Initialize measurements
    m_lastMeasurementTime = millis();
    updateMeasurements();
    
    m_initialized = true;
    
    Logger::info("Power Management initialized successfully");
    Logger::info("  Battery Pin: GPIO %d", m_config.batteryPin);
    Logger::info("  Solar Pin: GPIO %d %s", m_config.solarPin, 
                isSolarPinShared() ? "(shared with camera PWDN)" : "");
    Logger::info("  Low Battery Threshold: %.2fV", m_config.lowBatteryThreshold);
    Logger::info("  Critical Battery Threshold: %.2fV", m_config.criticalBatteryThreshold);
    
    return true;
}

void PowerManager::cleanup() {
    if (m_initialized) {
        // Clean up resources
        m_initialized = false;
        m_measurementsValid = false;
        
        Logger::info("Power Management cleaned up");
    }
}

bool PowerManager::initializeADC() {
    // Configure ADC1 for battery monitoring (GPIO 33 = ADC1_CH5)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11); // GPIO 33
    
    // Calibrate ADC if possible
    esp_adc_cal_value_t cal_type = esp_adc_cal_characterize(
        ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &m_adcChars);
    
    if (cal_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Logger::info("ADC calibrated using eFuse Vref");
        m_adcCalibrated = true;
    } else if (cal_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Logger::info("ADC calibrated using eFuse Two Point");
        m_adcCalibrated = true;
    } else {
        Logger::warning("ADC calibrated using default Vref");
        m_adcCalibrated = false;
    }
    
    return true;
}

bool PowerManager::initializeGPIO() {
    // Configure battery monitoring pin (ADC input)
    pinMode(m_config.batteryPin, INPUT);
    
    // Configure charging indicator LED
    if (m_config.chargingIndicatorPin >= 0) {
        pinMode(m_config.chargingIndicatorPin, OUTPUT);
        digitalWrite(m_config.chargingIndicatorPin, LOW);
    }
    
    // Note: Solar pin (GPIO 32) is shared with camera PWDN
    // We'll handle this conflict in resolvePinConflicts()
    
    return true;
}

bool PowerManager::resolvePinConflicts() {
    if (isSolarPinShared()) {
        Logger::warning("Solar monitoring pin GPIO 32 is shared with camera PWDN");
        Logger::warning("Solar monitoring will be limited when camera is active");
        // We'll handle this by timing solar measurements when camera is off
        return true;
    }
    
    // If not shared, configure solar pin as input
    pinMode(m_config.solarPin, INPUT);
    return true;
}

bool PowerManager::isSolarPinShared() const {
    // GPIO 32 is shared with camera PWDN on AI-Thinker ESP32-CAM
    return (m_config.solarPin == 32);
}

void PowerManager::handleSharedPinAccess() {
    // When solar pin is shared with camera PWDN, we need to coordinate access
    if (isSolarPinShared()) {
        // Temporarily configure pin as input for solar measurement
        pinMode(m_config.solarPin, INPUT);
        delay(10); // Allow pin to stabilize
    }
}

float PowerManager::readBatteryVoltage() {
    if (!m_initialized) return 0.0f;
    
    uint32_t totalReading = 0;
    
    // Take multiple samples for accuracy
    for (int i = 0; i < VOLTAGE_SAMPLES; i++) {
        uint32_t reading = adc1_get_raw(ADC1_CHANNEL_5); // GPIO 33
        totalReading += reading;
        delay(1);
    }
    
    uint32_t avgReading = totalReading / VOLTAGE_SAMPLES;
    
    // Convert to voltage
    float voltage;
    if (m_adcCalibrated) {
        voltage = esp_adc_cal_raw_to_voltage(avgReading, &m_adcChars) / 1000.0f;
    } else {
        voltage = (avgReading * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
    }
    
    // Apply voltage divider multiplier (for external voltage divider)
    voltage *= m_config.batteryVoltageMultiplier;
    
    return voltage;
}

float PowerManager::readSolarVoltage() {
    if (!m_initialized) return 0.0f;
    
    // Handle shared pin access
    handleSharedPinAccess();
    
    // Use analogRead for GPIO 32 (not part of ADC1 channels we configured)
    uint32_t totalReading = 0;
    
    for (int i = 0; i < VOLTAGE_SAMPLES; i++) {
        uint32_t reading = analogRead(m_config.solarPin);
        totalReading += reading;
        delay(1);
    }
    
    uint32_t avgReading = totalReading / VOLTAGE_SAMPLES;
    
    // Convert to voltage
    float voltage = (avgReading * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
    
    // Apply voltage divider multiplier
    voltage *= m_config.solarVoltageMultiplier;
    
    return voltage;
}

float PowerManager::calculateBatteryPercentage(float voltage) {
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0.0f;
    if (voltage >= BATTERY_MAX_VOLTAGE) return 100.0f;
    
    // Simple linear interpolation - could be improved with actual battery curve
    float percentage = ((voltage - BATTERY_MIN_VOLTAGE) / 
                       (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0f;
    
    return constrain(percentage, 0.0f, 100.0f);
}

BatteryHealth PowerManager::assessBatteryHealth(float voltage, float percentage) {
    if (voltage < BATTERY_MIN_VOLTAGE) {
        return BATTERY_DISCONNECTED;
    } else if (voltage < m_config.criticalBatteryThreshold) {
        return BATTERY_CRITICAL;
    } else if (percentage < 20.0f) {
        return BATTERY_DEGRADED;
    } else {
        return BATTERY_HEALTHY;
    }
}

SolarStatus PowerManager::assessSolarStatus(float voltage) {
    if (voltage < 1.0f) {
        return SOLAR_DISCONNECTED;
    } else if (voltage < m_config.minSolarVoltage) {
        return SOLAR_INSUFFICIENT;
    } else {
        return SOLAR_AVAILABLE;
    }
}

PowerStatus PowerManager::determinePowerStatus() {
    float batteryVoltage = m_currentMetrics.batteryVoltage;
    bool charging = m_currentMetrics.isCharging;
    
    if (batteryVoltage < m_config.criticalBatteryThreshold) {
        return POWER_CRITICAL_BATTERY;
    } else if (batteryVoltage < m_config.lowBatteryThreshold) {
        return POWER_LOW_BATTERY;
    } else if (charging) {
        return POWER_CHARGING;
    } else if (batteryVoltage >= m_config.fullChargeVoltage * 0.95f) {
        return POWER_FULLY_CHARGED;
    } else {
        return POWER_NORMAL;
    }
}

void PowerManager::updateMeasurements() {
    if (!m_initialized) return;
    
    unsigned long now = millis();
    
    // Check if it's time for an update
    uint32_t interval = m_inLowPowerMode ? m_config.lowPowerInterval : m_config.measurementInterval;
    if (now - m_lastMeasurementTime < interval) {
        return;
    }
    
    // Store previous metrics
    m_previousMetrics = m_currentMetrics;
    
    // Read new measurements
    m_currentMetrics.batteryVoltage = readBatteryVoltage();
    m_currentMetrics.solarVoltage = readSolarVoltage();
    m_currentMetrics.lastUpdate = now;
    
    // Calculate derived values
    m_currentMetrics.batteryPercentage = calculateBatteryPercentage(m_currentMetrics.batteryVoltage);
    m_currentMetrics.batteryHealth = assessBatteryHealth(m_currentMetrics.batteryVoltage, 
                                                        m_currentMetrics.batteryPercentage);
    m_currentMetrics.solarStatus = assessSolarStatus(m_currentMetrics.solarVoltage);
    
    // Determine charging status (simple heuristic)
    m_currentMetrics.isCharging = (m_currentMetrics.solarStatus == SOLAR_AVAILABLE) &&
                                  (m_currentMetrics.batteryVoltage < m_config.fullChargeVoltage);
    
    // Update overall power status
    m_currentMetrics.powerStatus = determinePowerStatus();
    m_currentMetrics.powerState = m_targetPowerState;
    
    // Calculate estimated runtime (simplified)
    if (m_currentMetrics.batteryPercentage > 0) {
        m_currentMetrics.estimatedRuntimeHours = m_currentMetrics.batteryPercentage * 0.2f; // Rough estimate
    } else {
        m_currentMetrics.estimatedRuntimeHours = 0.0f;
    }
    
    // Update warnings
    updateWarnings();
    
    // Update charging indicator LED
    if (m_config.chargingIndicatorPin >= 0) {
        digitalWrite(m_config.chargingIndicatorPin, m_currentMetrics.isCharging ? HIGH : LOW);
    }
    
    m_measurementsValid = true;
    m_lastMeasurementTime = now;
    
    // Check power thresholds and adjust system state if needed
    checkPowerThresholds();
}

void PowerManager::updateWarnings() {
    m_currentMetrics.lowPowerWarning = (m_currentMetrics.powerStatus == POWER_LOW_BATTERY);
    m_currentMetrics.criticalPowerWarning = (m_currentMetrics.powerStatus == POWER_CRITICAL_BATTERY);
}

void PowerManager::checkPowerThresholds() {
    PowerState newState = m_targetPowerState;
    
    // State machine with three primary states: NORMAL, CRITICAL, CHARGING
    // Apply appropriate power profiles based on current power status
    
    if (m_currentMetrics.powerStatus == POWER_CHARGING || 
        m_currentMetrics.powerStatus == POWER_FULLY_CHARGED) {
        // CHARGING state - can operate normally or even boost performance
        if (m_targetPowerState != STATE_NORMAL_OPERATION) {
            newState = STATE_NORMAL_OPERATION;
            Logger::info("Power state: CHARGING - restoring normal operation");
        }
    } else if (m_currentMetrics.powerStatus == POWER_CRITICAL_BATTERY) {
        // CRITICAL state - must enter emergency shutdown
        if (m_targetPowerState != STATE_EMERGENCY_SHUTDOWN) {
            newState = STATE_EMERGENCY_SHUTDOWN;
            Logger::warning("Power state: CRITICAL - entering emergency shutdown");
        }
    } else if (m_currentMetrics.powerStatus == POWER_LOW_BATTERY) {
        // LOW state - transition to power saving
        if (m_targetPowerState == STATE_NORMAL_OPERATION) {
            newState = STATE_LOW_POWER;
            Logger::info("Power state: LOW - entering power saving mode");
        }
    } else {
        // NORMAL state - standard operation
        if (m_targetPowerState == STATE_LOW_POWER || m_targetPowerState == STATE_POWER_SAVING) {
            newState = STATE_NORMAL_OPERATION;
            Logger::info("Power state: NORMAL - resuming normal operation");
        }
    }
    
    if (newState != m_targetPowerState) {
        setPowerState(newState);
    }
}

PowerMetrics PowerManager::getPowerMetrics() {
    updateMeasurements();
    return m_currentMetrics;
}

float PowerManager::getBatteryVoltage() {
    updateMeasurements();
    return m_currentMetrics.batteryVoltage;
}

float PowerManager::getBatteryPercentage() {
    updateMeasurements();
    return m_currentMetrics.batteryPercentage;
}

BatteryHealth PowerManager::getBatteryHealth() {
    updateMeasurements();
    return m_currentMetrics.batteryHealth;
}

bool PowerManager::isBatteryLow() {
    return m_currentMetrics.lowPowerWarning;
}

bool PowerManager::isBatteryCritical() {
    return m_currentMetrics.criticalPowerWarning;
}

float PowerManager::getEstimatedRuntimeHours() {
    updateMeasurements();
    return m_currentMetrics.estimatedRuntimeHours;
}

float PowerManager::getSolarVoltage() {
    updateMeasurements();
    return m_currentMetrics.solarVoltage;
}

SolarStatus PowerManager::getSolarStatus() {
    updateMeasurements();
    return m_currentMetrics.solarStatus;
}

bool PowerManager::isCharging() {
    updateMeasurements();
    return m_currentMetrics.isCharging;
}

bool PowerManager::isSolarAvailable() {
    return (getSolarStatus() == SOLAR_AVAILABLE);
}

PowerStatus PowerManager::getPowerStatus() {
    updateMeasurements();
    return m_currentMetrics.powerStatus;
}

PowerState PowerManager::getPowerState() {
    return m_targetPowerState;
}

void PowerManager::setPowerState(PowerState state) {
    if (state == m_targetPowerState) return;
    
    Logger::info("Power state changing from %d to %d", m_targetPowerState, state);
    
    PowerState previousState = m_targetPowerState;
    m_targetPowerState = state;
    m_stateChangeTime = millis();
    
    updatePowerState();
}

void PowerManager::updatePowerState() {
    switch (m_targetPowerState) {
        case STATE_NORMAL_OPERATION:
            restoreNormalOperation();
            break;
        case STATE_POWER_SAVING:
        case STATE_LOW_POWER:
            enterLowPowerMode();
            break;
        case STATE_DEEP_SLEEP:
            // Deep sleep should be initiated by the main system
            break;
        case STATE_EMERGENCY_SHUTDOWN:
            handleEmergencyShutdown();
            break;
    }
}

bool PowerManager::enterLowPowerMode() {
    if (m_inLowPowerMode) return true;
    
    Logger::warning("Entering low power mode - battery level: %.2fV (%.1f%%)", 
                   m_currentMetrics.batteryVoltage, m_currentMetrics.batteryPercentage);
    
    // Reduce measurement frequency
    m_inLowPowerMode = true;
    m_powerSavingEnabled = true;
    
    // Signal other systems to reduce power consumption
    shutdownNonEssentialSystems();
    
    return true;
}

bool PowerManager::exitLowPowerMode() {
    if (!m_inLowPowerMode) return true;
    
    Logger::info("Exiting low power mode - battery level: %.2fV (%.1f%%)", 
                m_currentMetrics.batteryVoltage, m_currentMetrics.batteryPercentage);
    
    m_inLowPowerMode = false;
    m_powerSavingEnabled = false;
    
    restoreNormalOperation();
    
    return true;
}

bool PowerManager::enterDeepSleep(uint32_t sleepTimeMs) {
    // Intelligent deep sleep based on current power state
    uint32_t actualSleepTime = sleepTimeMs;
    
    // Adjust sleep duration based on power state
    if (m_targetPowerState == STATE_EMERGENCY_SHUTDOWN) {
        // Critical battery - sleep indefinitely until external wakeup
        actualSleepTime = 0;
        Logger::error("Entering emergency deep sleep - critical battery %.2fV", 
                     m_currentMetrics.batteryVoltage);
    } else if (m_currentMetrics.powerStatus == POWER_LOW_BATTERY) {
        // Low battery - extend sleep time to conserve power
        if (actualSleepTime > 0 && actualSleepTime < 3600000) { // < 1 hour
            actualSleepTime *= 2; // Double the sleep time
            Logger::warning("Extending deep sleep to %d ms for battery conservation", actualSleepTime);
        }
    } else if (m_currentMetrics.powerStatus == POWER_CHARGING) {
        // Charging - can use shorter sleep times for more frequent monitoring
        if (actualSleepTime > 300000) { // > 5 minutes
            actualSleepTime = 300000; // Cap at 5 minutes while charging
            Logger::info("Reducing deep sleep to %d ms while charging", actualSleepTime);
        }
    }
    
    Logger::warning("Entering deep sleep for %d ms (state: %d)", actualSleepTime, m_targetPowerState);
    
    // Configure wakeup source
    if (actualSleepTime > 0) {
        esp_sleep_enable_timer_wakeup(actualSleepTime * 1000); // Convert to microseconds
    }
    
    // Enter deep sleep
    esp_deep_sleep_start();
    
    // This line should never be reached
    return false;
}

void PowerManager::optimizePowerConsumption() {
    // Implement power optimization strategies based on current state
    if (m_currentMetrics.powerStatus == POWER_LOW_BATTERY) {
        if (!m_inLowPowerMode) {
            enterLowPowerMode();
        }
    }
}

void PowerManager::enablePowerSaving(bool enable) {
    m_powerSavingEnabled = enable;
    Logger::info("Power saving %s", enable ? "enabled" : "disabled");
}

void PowerManager::clearWarnings() {
    m_currentMetrics.lowPowerWarning = false;
    m_currentMetrics.criticalPowerWarning = false;
}

void PowerManager::handleEmergencyShutdown() {
    Logger::error("EMERGENCY SHUTDOWN - Critical battery level: %.2fV", 
                 m_currentMetrics.batteryVoltage);
    
    // Shutdown all non-essential systems immediately
    shutdownNonEssentialSystems();
    
    // Enter deep sleep to preserve remaining battery
    enterDeepSleep(0); // Sleep until external wakeup
}

bool PowerManager::isEmergencyShutdownRequired() {
    return (m_currentMetrics.powerStatus == POWER_CRITICAL_BATTERY);
}

void PowerManager::shutdownNonEssentialSystems() {
    // This would signal other systems to shut down
    // Implementation depends on system architecture
    Logger::warning("Shutting down non-essential systems for power conservation");
}

void PowerManager::restoreNormalOperation() {
    // Restore normal system operation
    Logger::info("Restoring normal power operation");
}

void PowerManager::onCameraActivation() {
    m_lastCameraActivity = millis();
}

void PowerManager::onCameraDeactivation() {
    // Camera deactivated - we can now safely read solar voltage if pin is shared
}

void PowerManager::onNetworkActivity() {
    m_lastNetworkActivity = millis();
}

void PowerManager::onSensorActivity() {
    m_lastSensorActivity = millis();
}

// Utility functions implementation
bool initializePowerManagement(const PowerConfig& config) {
    if (g_powerManager == nullptr) {
        g_powerManager = new PowerManager();
    }
    
    return g_powerManager->initialize(config);
}

void cleanupPowerManagement() {
    if (g_powerManager != nullptr) {
        delete g_powerManager;
        g_powerManager = nullptr;
    }
}

PowerMetrics getSystemPowerMetrics() {
    if (g_powerManager != nullptr) {
        return g_powerManager->getPowerMetrics();
    }
    return PowerMetrics(); // Return default metrics if not initialized
}

bool isSystemPowerHealthy() {
    if (g_powerManager != nullptr) {
        PowerMetrics metrics = g_powerManager->getPowerMetrics();
        return (metrics.powerStatus != POWER_CRITICAL_BATTERY) && 
               (metrics.batteryHealth != BATTERY_CRITICAL);
    }
    return false;
}

void logPowerStatus() {
    if (g_powerManager != nullptr) {
        PowerMetrics metrics = g_powerManager->getPowerMetrics();
        Logger::info("Power Status: Battery %.2fV (%.1f%%), Solar %.2fV, %s", 
                    metrics.batteryVoltage, metrics.batteryPercentage, 
                    metrics.solarVoltage, metrics.isCharging ? "Charging" : "Not Charging");
    }
}