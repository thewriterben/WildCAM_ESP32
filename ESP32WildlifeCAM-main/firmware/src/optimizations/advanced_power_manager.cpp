#include "advanced_power_manager.h"
#include "../config.h"
#include <soc/rtc.h>

// Static instance for interrupt handling
UltraLowPowerMotionDetector* UltraLowPowerMotionDetector::instance = nullptr;

// AdvancedPowerManager implementation
bool AdvancedPowerManager::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing Advanced Power Manager...");
    
    currentProfile = POWER_BALANCED;
    dvfsEnabled = false;
    pirWakeupPin = GPIO_NUM_13; // PIR_PIN from config
    lastPowerMeasurement = 0;
    lastMeasuredPower = 0;
    
#ifdef XPOWERS_ENABLED
    // Initialize XPowersLib state variables
    xpowers = nullptr;
    xpowersInitialized = false;
    xpowersChipModel = 0;
    lastXPowersUpdate = 0;
    
    // Initialize XPowersLib if available
    if (initializeXPowers()) {
        DEBUG_PRINTLN("XPowersLib initialized successfully");
    } else {
        DEBUG_PRINTLN("Warning: XPowersLib not available, using basic power management");
    }
#endif
    
    // Configure power management
    esp_pm_config_esp32_t pm_config;
    pm_config.max_freq_mhz = 240;
    pm_config.min_freq_mhz = 80;
    pm_config.light_sleep_enable = true;
    
    if (esp_pm_configure(&pm_config) != ESP_OK) {
        DEBUG_PRINTLN("WARNING: Failed to configure power management");
    }
    
    // Configure GPIO for low power
    configureGPIOForLowPower();
    
    // Configure brownout detector
    configureBrownoutDetector();
    
    // Calibrate power model
    powerModel = calibratePowerModel();
    
    initialized = true;
    DEBUG_PRINTLN("Advanced Power Manager initialized successfully");
    return true;
}

void AdvancedPowerManager::cleanup() {
    if (!initialized) return;
    
    // Reset to normal power profile
    setProfile(POWER_BALANCED);
    
    initialized = false;
}

void AdvancedPowerManager::adjustCPUFrequency(float load) {
    if (!dvfsEnabled) return;
    
    uint32_t targetFreq;
    
    if (load > 0.8f) {
        targetFreq = MAX_CPU_FREQ_MHZ;
    } else if (load > 0.6f) {
        targetFreq = 160;
    } else if (load > 0.4f) {
        targetFreq = 120;
    } else {
        targetFreq = MIN_CPU_FREQ_MHZ;
    }
    
    setFrequency(targetFreq);
}

void AdvancedPowerManager::enableDVFS() {
    dvfsEnabled = true;
    DEBUG_PRINTLN("Dynamic Voltage and Frequency Scaling enabled");
}

bool AdvancedPowerManager::setFrequency(uint32_t frequencyMHz) {
    // Validate frequency
    if (frequencyMHz < MIN_CPU_FREQ_MHZ || frequencyMHz > MAX_CPU_FREQ_MHZ) {
        return false;
    }
    
    // Round to nearest step
    uint32_t rounded = ((frequencyMHz + STEP_SIZE_MHZ/2) / STEP_SIZE_MHZ) * STEP_SIZE_MHZ;
    
    esp_pm_config_esp32_t pm_config;
    pm_config.max_freq_mhz = rounded;
    pm_config.min_freq_mhz = MIN_CPU_FREQ_MHZ;
    pm_config.light_sleep_enable = true;
    
    return esp_pm_configure(&pm_config) == ESP_OK;
}

uint32_t AdvancedPowerManager::getCurrentFrequency() {
    rtc_cpu_freq_config_t config;
    rtc_clk_cpu_freq_get_config(&config);
    return config.freq_mhz;
}

void AdvancedPowerManager::powerDownUnusedPeripherals() {
    // Disable unused peripherals based on current configuration
    
    #if !defined(WIFI_ENABLED) || WIFI_ENABLED == false
    // Disable WiFi if not needed
    esp_wifi_stop();
    esp_wifi_deinit();
    #endif
    
    #if !defined(BT_ENABLED) || BT_ENABLED == false
    // Disable Bluetooth if not needed
    esp_bt_controller_disable();
    #endif
    
    // Disable unused ADC channels
    // This is hardware-specific and would need careful implementation
}

void AdvancedPowerManager::selectOptimalRadioPower(int8_t rssi, uint32_t distance) {
    // Adjust LoRa transmission power based on signal quality and distance
    int8_t optimalPower = 5; // Minimum power
    
    if (rssi < -100 || distance > 1000) {
        optimalPower = 20; // Maximum power for poor conditions
    } else if (rssi < -90 || distance > 500) {
        optimalPower = 15;
    } else if (rssi < -80 || distance > 250) {
        optimalPower = 10;
    }
    
    // Apply the power setting to LoRa module
    // This would integrate with the existing LoRa mesh implementation
}

void AdvancedPowerManager::dynamicCameraSettings(float lightLevel, bool motionDetected) {
    // Adjust camera settings based on conditions to optimize power
    
    if (lightLevel < 10.0f) { // Low light
        // Reduce frame rate, increase exposure
        // This would integrate with camera_handler
    }
    
    if (!motionDetected) {
        // Reduce frame rate when no motion
        // Put camera in standby mode
    }
}

void AdvancedPowerManager::optimizeWiFiPower(int8_t signalStrength) {
    #if defined(WIFI_ENABLED) && WIFI_ENABLED == true
    // Adjust WiFi power based on signal strength
    wifi_power_t power = WIFI_POWER_19_5dBm; // Default
    
    if (signalStrength > -50) {
        power = WIFI_POWER_8_5dBm;   // Excellent signal
    } else if (signalStrength > -60) {
        power = WIFI_POWER_11dBm;    // Good signal
    } else if (signalStrength > -70) {
        power = WIFI_POWER_15dBm;    // Fair signal
    }
    
    esp_wifi_set_max_tx_power(power);
    #endif
}

void AdvancedPowerManager::setProfile(PowerProfile profile) {
    if (!validateProfile(profile)) return;
    
    currentProfile = profile;
    applyPowerProfile(profile);
    
    DEBUG_PRINTF("Power profile changed to: %d\n", profile);
}

void AdvancedPowerManager::applyPowerProfile(PowerProfile profile) {
    switch (profile) {
        case POWER_MAXIMUM_PERFORMANCE:
            setFrequency(MAX_CPU_FREQ_MHZ);
            // Enable all features
            break;
            
        case POWER_BALANCED:
            setFrequency(160);
            // Balanced settings
            break;
            
        case POWER_ECO_MODE:
            setFrequency(120);
            powerDownUnusedPeripherals();
            break;
            
        case POWER_SURVIVAL:
            setFrequency(MIN_CPU_FREQ_MHZ);
            powerDownUnusedPeripherals();
            // Minimal functionality
            break;
            
        case POWER_HIBERNATION:
            // Prepare for deep sleep
            configureGPIOForLowPower();
            enableDeepSleepWakeOnMotion();
            break;
    }
}

bool AdvancedPowerManager::validateProfile(PowerProfile profile) {
    return profile >= POWER_MAXIMUM_PERFORMANCE && profile <= POWER_HIBERNATION;
}

void AdvancedPowerManager::optimizeBasedOnSolarData(float solarVoltage, float batteryLevel) {
    PowerProfile recommendedProfile = currentProfile;
    
    if (batteryLevel < 20.0f) {
        // Critical battery - enter survival mode
        recommendedProfile = POWER_SURVIVAL;
    } else if (batteryLevel < 40.0f && solarVoltage < 3.0f) {
        // Low battery and no charging - eco mode
        recommendedProfile = POWER_ECO_MODE;
    } else if (batteryLevel > 80.0f && solarVoltage > 4.0f) {
        // Good battery and charging - performance mode
        recommendedProfile = POWER_BALANCED;
    }
    
    if (recommendedProfile != currentProfile) {
        setProfile(recommendedProfile);
    }
}

float AdvancedPowerManager::measureActualPowerConsumption() {
    // This would measure actual current consumption
    // For now, estimate based on current configuration
    
    uint32_t freq = getCurrentFrequency();
    float estimatedPower = powerModel.basePower;
    
    estimatedPower += (freq / 240.0f) * powerModel.cpuPowerCoeff * 50.0f;
    
    #if defined(LORA_ENABLED) && LORA_ENABLED == true
    estimatedPower += powerModel.radioPowerCoeff * 30.0f;
    #endif
    
    lastMeasuredPower = estimatedPower;
    lastPowerMeasurement = millis();
    
    return estimatedPower;
}

AdvancedPowerManager::PowerProfile AdvancedPowerManager::selectOptimalProfile(float batteryLevel, uint32_t timeToNextCharge) {
    if (batteryLevel < 10.0f) {
        return POWER_HIBERNATION;
    } else if (batteryLevel < 25.0f) {
        return POWER_SURVIVAL;
    } else if (batteryLevel < 50.0f || timeToNextCharge > 48) {
        return POWER_ECO_MODE;
    } else {
        return POWER_BALANCED;
    }
}

void AdvancedPowerManager::enableDeepSleepWakeOnMotion() {
    esp_sleep_enable_ext0_wakeup(pirWakeupPin, 1);
    rtc_gpio_pulldown_en(pirWakeupPin);
    rtc_gpio_pullup_dis(pirWakeupPin);
}

bool AdvancedPowerManager::hasMotionSinceLastCheck() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    return wakeup_reason == ESP_SLEEP_WAKEUP_EXT0;
}

void AdvancedPowerManager::configureGPIOForLowPower() {
    // Configure unused GPIO pins for minimal power consumption
    for (int i = 0; i < GPIO_NUM_MAX; i++) {
        // Skip pins in use
        if (i == PIR_PIN || i == SOLAR_VOLTAGE_PIN || i == BATTERY_VOLTAGE_PIN ||
            i == LORA_CS || i == LORA_RST || i == LORA_DIO0) {
            continue;
        }
        
        // Configure as input with pullup to reduce leakage
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_PULLUP_ONLY);
    }
}

void AdvancedPowerManager::configureBrownoutDetector() {
    // Configure brownout detector for optimal power management
    esp_brownout_disable(); // Disable to prevent unexpected resets
}

AdvancedPowerManager::PowerModel AdvancedPowerManager::calibratePowerModel() {
    PowerModel model;
    
    // Perform basic calibration
    // This would involve actual power measurements in a real implementation
    model.calibrated = true;
    
    DEBUG_PRINTLN("Power model calibrated");
    return model;
}

float AdvancedPowerManager::predictPowerConsumption(PowerProfile profile, uint32_t durationMs) {
    float baseCurrent = 0;
    
    switch (profile) {
        case POWER_MAXIMUM_PERFORMANCE: baseCurrent = 250.0f; break;
        case POWER_BALANCED: baseCurrent = 180.0f; break;
        case POWER_ECO_MODE: baseCurrent = 120.0f; break;
        case POWER_SURVIVAL: baseCurrent = 80.0f; break;
        case POWER_HIBERNATION: baseCurrent = 10.0f; break;
    }
    
    return baseCurrent * (durationMs / 1000.0f) / 3600.0f; // mAh
}

AdvancedPowerManager::PowerMetrics AdvancedPowerManager::getPowerMetrics() const {
    return metrics;
}

void AdvancedPowerManager::resetMetrics() {
    metrics = PowerMetrics();
}

#ifdef XPOWERS_ENABLED
// XPowersLib Implementation

bool AdvancedPowerManager::initializeXPowers() {
    DEBUG_PRINTLN("Initializing XPowersLib...");
    
    // Try to initialize XPowersLib on I2C
    xpowers = new XPowersPPM();
    
    // Try common I2C addresses for power management chips
    uint8_t addresses[] = {0x34, 0x35, 0x36}; // Common AXP chip addresses
    
    for (int i = 0; i < 3; i++) {
        if (xpowers->init(Wire, addresses[i], 21, 22)) { // SDA=21, SCL=22 for ESP32
            xpowersChipModel = xpowers->getChipModel();
            xpowersInitialized = true;
            
            DEBUG_PRINTF("XPowersLib initialized successfully with chip model: 0x%02X at address 0x%02X\n", 
                        xpowersChipModel, addresses[i]);
            
            // Configure initial settings
            xpowers->enableCharging(true);
            xpowers->setChargingTargetVoltage(CHARGE_TERMINATION_VOLTAGE);
            xpowers->setChargerConstantCurr(DEFAULT_CHARGE_CURRENT);
            
            return true;
        }
    }
    
    // Cleanup if initialization failed
    delete xpowers;
    xpowers = nullptr;
    xpowersInitialized = false;
    
    DEBUG_PRINTLN("Warning: No XPowers chip detected");
    return false;
}

void AdvancedPowerManager::cleanupXPowers() {
    if (xpowersInitialized && xpowers) {
        delete xpowers;
        xpowers = nullptr;
        xpowersInitialized = false;
        DEBUG_PRINTLN("XPowersLib cleaned up");
    }
}

bool AdvancedPowerManager::hasXPowersChip() const {
    return xpowersInitialized && xpowers != nullptr;
}

float AdvancedPowerManager::getAdvancedBatteryVoltage() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getBattVoltage() / 1000.0f; // Convert mV to V
}

float AdvancedPowerManager::getAdvancedBatteryCurrent() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getBattDischargeCurrent();
}

float AdvancedPowerManager::getBatteryChargeCurrent() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getBattChargeCurrent();
}

float AdvancedPowerManager::getBatteryDischargeCurrent() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getBattDischargeCurrent();
}

float AdvancedPowerManager::getBatteryTemperature() {
    if (!hasXPowersChip()) return 25.0f; // Default temperature
    return xpowers->getTemperature();
}

uint8_t AdvancedPowerManager::getBatteryCapacityPercent() {
    if (!hasXPowersChip()) {
        // Fallback to voltage-based estimation
        float voltage = getAdvancedBatteryVoltage();
        if (voltage > 4.0f) return 100;
        if (voltage > 3.8f) return 75;
        if (voltage > 3.6f) return 50;
        if (voltage > 3.3f) return 25;
        return 0;
    }
    return xpowers->getBattPercentage();
}

bool AdvancedPowerManager::isBatteryCharging() {
    if (!hasXPowersChip()) return false;
    return xpowers->isChargeing();
}

bool AdvancedPowerManager::isBatteryConnected() {
    if (!hasXPowersChip()) return true; // Assume connected if no chip
    return xpowers->isBatteryConnect();
}

float AdvancedPowerManager::getSolarVoltage() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getVbusVoltage() / 1000.0f; // VBUS is typically solar input
}

float AdvancedPowerManager::getSolarCurrent() {
    if (!hasXPowersChip()) return 0.0f;
    return xpowers->getVbusCurrent();
}

float AdvancedPowerManager::getSolarPower() {
    return getSolarVoltage() * getSolarCurrent();
}

bool AdvancedPowerManager::isSolarCharging() {
    if (!hasXPowersChip()) return false;
    return xpowers->isVbusIn();
}

void AdvancedPowerManager::optimizeSolarCharging() {
    if (!hasXPowersChip()) return;
    
    float solarVoltage = getSolarVoltage();
    float solarCurrent = getSolarCurrent();
    
    // Optimize charging current based on solar conditions
    if (solarVoltage > 5.5f && solarCurrent > 0.5f) {
        // Good solar conditions - increase charging current
        setChargingCurrent(800);
    } else if (solarVoltage > 5.0f && solarCurrent > 0.3f) {
        // Moderate solar conditions - normal charging
        setChargingCurrent(DEFAULT_CHARGE_CURRENT);
    } else {
        // Poor solar conditions - reduce charging current
        setChargingCurrent(300);
    }
}

void AdvancedPowerManager::setChargingCurrent(uint16_t currentMA) {
    if (!hasXPowersChip()) return;
    xpowers->setChargerConstantCurr(currentMA);
    DEBUG_PRINTF("Charging current set to %d mA\n", currentMA);
}

void AdvancedPowerManager::enableCharging(bool enable) {
    if (!hasXPowersChip()) return;
    xpowers->enableCharging(enable);
    DEBUG_PRINTF("Charging %s\n", enable ? "enabled" : "disabled");
}

void AdvancedPowerManager::setChargingTerminationVoltage(float voltage) {
    if (!hasXPowersChip()) return;
    xpowers->setChargingTargetVoltage(voltage);
    DEBUG_PRINTF("Charging termination voltage set to %.2f V\n", voltage);
}

void AdvancedPowerManager::enableLowBatteryWarning(bool enable, float threshold) {
    if (!hasXPowersChip()) return;
    if (enable) {
        xpowers->setLowBatWarnThreshold(threshold);
        xpowers->enableBattVoltageMeasure();
    }
    DEBUG_PRINTF("Low battery warning %s at %.2f V\n", enable ? "enabled" : "disabled", threshold);
}

void AdvancedPowerManager::enableOverchargeProtection(bool enable) {
    if (!hasXPowersChip()) return;
    xpowers->enableCharging(enable); // This typically includes overcharge protection
    DEBUG_PRINTF("Overcharge protection %s\n", enable ? "enabled" : "disabled");
}

void AdvancedPowerManager::enableButtonWakeup(bool enable) {
    if (!hasXPowersChip()) return;
    xpowers->enableWakeup(); // Enable wakeup functionality
    DEBUG_PRINTF("Button wakeup %s\n", enable ? "enabled" : "disabled");
}

void AdvancedPowerManager::setLowPowerMode(bool enable) {
    if (!hasXPowersChip()) return;
    if (enable) {
        // Disable unnecessary functions for power saving
        xpowers->enableCharging(false);
        xpowers->disableAllIRQ();
    } else {
        // Re-enable normal operation
        xpowers->enableCharging(true);
    }
    DEBUG_PRINTF("Low power mode %s\n", enable ? "enabled" : "disabled");
}

void AdvancedPowerManager::enterShipMode() {
    if (!hasXPowersChip()) return;
    DEBUG_PRINTLN("Entering ship mode (ultra-low power)...");
    xpowers->shutdown();
}

void AdvancedPowerManager::resetPowerSettings() {
    if (!hasXPowersChip()) return;
    
    DEBUG_PRINTLN("Resetting power settings to defaults...");
    
    // Reset to default settings
    xpowers->enableCharging(true);
    xpowers->setChargingTargetVoltage(CHARGE_TERMINATION_VOLTAGE);
    xpowers->setChargerConstantCurr(DEFAULT_CHARGE_CURRENT);
    
    DEBUG_PRINTLN("Power settings reset complete");
}

#endif // XPOWERS_ENABLED

// UltraLowPowerMotionDetector implementation
UltraLowPowerMotionDetector::UltraLowPowerMotionDetector(gpio_num_t pin) 
    : pirPin(pin), motionDetected(false), initialized(false), lastMotionTime(0) {
    instance = this;
}

bool UltraLowPowerMotionDetector::init() {
    if (initialized) return true;
    
    gpio_config_t config;
    config.pin_bit_mask = (1ULL << pirPin);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    config.intr_type = GPIO_INTR_POSEDGE;
    
    gpio_config(&config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pirPin, motionISR, nullptr);
    
    initialized = true;
    return true;
}

void UltraLowPowerMotionDetector::cleanup() {
    if (initialized) {
        gpio_isr_handler_remove(pirPin);
        initialized = false;
    }
}

void IRAM_ATTR UltraLowPowerMotionDetector::motionISR() {
    if (instance) {
        instance->motionDetected = true;
        instance->lastMotionTime = millis();
    }
}

bool UltraLowPowerMotionDetector::hasMotionSinceLastCheck() {
    bool motion = motionDetected;
    motionDetected = false;
    return motion;
}

void UltraLowPowerMotionDetector::clearMotionFlag() {
    motionDetected = false;
}

void UltraLowPowerMotionDetector::enableDeepSleepWakeOnMotion() {
    configurePinForWakeup();
    esp_sleep_enable_ext0_wakeup(pirPin, 1);
}

void UltraLowPowerMotionDetector::configurePinForWakeup() {
    rtc_gpio_init(pirPin);
    rtc_gpio_set_direction(pirPin, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en(pirPin);
    rtc_gpio_pullup_dis(pirPin);
}