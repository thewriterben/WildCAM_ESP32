#include "PowerManager.h"
#include "config.h"

// Store constant strings in PROGMEM to save RAM
static const char TAG_INIT[] PROGMEM = "Power Manager initialized";
static const char TAG_BATTERY_ADC[] PROGMEM = "Battery ADC: %d, Voltage: %.2fV\n";
static const char TAG_LOW_BATTERY[] PROGMEM = "WARNING: Low battery detected! Voltage: %.2fV (minimum: %.2fV)\n";
static const char TAG_DEEP_SLEEP[] PROGMEM = "Entering deep sleep for %llu seconds...\n";
static const char TAG_WAKEUP_MOTION[] PROGMEM = "Configured wakeup on motion (pin %d)\n";
static const char TAG_WAKEUP_TIMER[] PROGMEM = "Configured wakeup on timer (%llu seconds)\n";
static const char TAG_POWER_STATUS[] PROGMEM = "========== Power Status ==========";
static const char TAG_VOLTAGE[] PROGMEM = "Battery Voltage: %.2fV\n";
static const char TAG_PERCENTAGE[] PROGMEM = "Battery Percentage: %d%%\n";
static const char TAG_LOW_BAT[] PROGMEM = "Low Battery: %s\n";
static const char TAG_LOW_POWER_MODE[] PROGMEM = "Low Power Mode: %s\n";
static const char TAG_BAT_MIN[] PROGMEM = "Battery Min: %.2fV\n";
static const char TAG_BAT_MAX[] PROGMEM = "Battery Max: %.2fV\n";
static const char TAG_SEPARATOR[] PROGMEM = "==================================";

// Energy accounting PROGMEM strings
static const char TAG_ENERGY_HEADER[] PROGMEM = "======= Energy Accounting =======";
static const char TAG_ENERGY_ACTIVE[] PROGMEM = "Active Time: %lu ms\n";
static const char TAG_ENERGY_SLEEP[] PROGMEM = "Sleep Time: %lu ms\n";
static const char TAG_ENERGY_CAPTURES[] PROGMEM = "Image Captures: %lu\n";
static const char TAG_ENERGY_TRANSMITS[] PROGMEM = "Transmissions: %lu\n";
static const char TAG_ENERGY_CONSUMED[] PROGMEM = "Energy Consumed: %.2f mAh\n";
static const char TAG_ENERGY_AVG_CURRENT[] PROGMEM = "Average Current: %.2f mA\n";
static const char TAG_ENERGY_REMAINING[] PROGMEM = "Est. Remaining: %.1f hours\n";
static const char TAG_ENERGY_TRACKING[] PROGMEM = "Energy Tracking: %s\n";

// Default power profile values for ESP32-CAM
static const float DEFAULT_IDLE_CURRENT_MA = 80.0f;
static const float DEFAULT_CAPTURE_CURRENT_MA = 310.0f;
static const float DEFAULT_TRANSMIT_CURRENT_MA = 250.0f;
static const float DEFAULT_DEEP_SLEEP_CURRENT_UA = 10.0f;
static const unsigned long DEFAULT_CAPTURE_DURATION_MS = 2000;
static const unsigned long DEFAULT_TRANSMIT_DURATION_MS = 5000;
static const float DEFAULT_BATTERY_CAPACITY_MAH = 2000.0f;

PowerManager::PowerManager()
    : batteryPin(35),
      batteryVoltage(0.0),
      lowPowerMode(false),
      sessionStartTime(0),
      lastVoltageReadTime(0),
      initialVoltage(0.0),
      energyTrackingEnabled(true) {
    
    // Initialize energy stats
    memset(&energyStats, 0, sizeof(energyStats));
    energyStats.batteryCapacityMah = DEFAULT_BATTERY_CAPACITY_MAH;
    energyStats.isValid = false;
    
    // Initialize power profile with default values
    powerProfile.idleCurrentMa = DEFAULT_IDLE_CURRENT_MA;
    powerProfile.captureCurrentMa = DEFAULT_CAPTURE_CURRENT_MA;
    powerProfile.transmitCurrentMa = DEFAULT_TRANSMIT_CURRENT_MA;
    powerProfile.deepSleepCurrentUa = DEFAULT_DEEP_SLEEP_CURRENT_UA;
    powerProfile.captureDurationMs = DEFAULT_CAPTURE_DURATION_MS;
    powerProfile.transmitDurationMs = DEFAULT_TRANSMIT_DURATION_MS;
}

bool PowerManager::init(int batteryMonitorPin) {
    batteryPin = batteryMonitorPin;
    
    // Configure ADC for battery voltage reading
    pinMode(batteryPin, INPUT);
    
    // Configure ADC attenuation for full range
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
    
    // Read initial battery voltage
    getBatteryVoltage();
    
    // Initialize energy tracking
    sessionStartTime = millis();
    initialVoltage = batteryVoltage;
    lastVoltageReadTime = sessionStartTime;
    energyStats.lastUpdateTime = sessionStartTime;
    energyStats.isValid = true;
    
    Serial.println(FPSTR(TAG_INIT));
    printPowerStatus();
    
    return true;
}

float PowerManager::getBatteryVoltage() {
    int adcValue = analogRead(batteryPin);
    
    // ESP32 ADC: 0-4095 maps to 0-3.3V (with 11dB attenuation)
    // Adjust for voltage divider if present
    const float ADC_MAX = 4095.0;
    const float ADC_VREF = 3.3;
    const float VOLTAGE_DIVIDER_RATIO = 2.0;  // Adjust based on your hardware
    
    batteryVoltage = (adcValue / ADC_MAX) * ADC_VREF * VOLTAGE_DIVIDER_RATIO;
    
    Serial.printf_P(TAG_BATTERY_ADC, adcValue, batteryVoltage);
    return batteryVoltage;
}

int PowerManager::getBatteryPercentage() {
    float voltage = getBatteryVoltage();
    
    // Clamp voltage to min/max range
    if (voltage < BATTERY_MIN) {
        return 0;
    }
    if (voltage > BATTERY_MAX) {
        return 100;
    }
    
    // Linear interpolation
    float percentage = ((voltage - BATTERY_MIN) / 
                       (BATTERY_MAX - BATTERY_MIN)) * 100.0;
    
    return (int)percentage;
}

bool PowerManager::isLowBattery() {
    float voltage = getBatteryVoltage();
    bool isLow = voltage < BATTERY_MIN;
    
    if (isLow) {
        Serial.printf_P(TAG_LOW_BATTERY, voltage, BATTERY_MIN);
    }
    
    return isLow;
}

void PowerManager::enterDeepSleep(uint64_t sleepTimeSeconds) {
    Serial.printf_P(TAG_DEEP_SLEEP, sleepTimeSeconds);
    Serial.flush();
    
    // Enter deep sleep
    esp_deep_sleep(sleepTimeSeconds * 1000000ULL);
}

void PowerManager::configureWakeOnMotion(int pirPin) {
    // Configure external wakeup from PIR sensor
    esp_sleep_enable_ext0_wakeup((gpio_num_t)pirPin, HIGH);
    
    Serial.printf_P(TAG_WAKEUP_MOTION, pirPin);
}

void PowerManager::configureWakeOnTimer(uint64_t seconds) {
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Convert to microseconds
    
    Serial.printf_P(TAG_WAKEUP_TIMER, seconds);
}

void PowerManager::printPowerStatus() {
    Serial.println(FPSTR(TAG_POWER_STATUS));
    Serial.printf_P(TAG_VOLTAGE, getBatteryVoltage());
    Serial.printf_P(TAG_PERCENTAGE, getBatteryPercentage());
    Serial.printf_P(TAG_LOW_BAT, isLowBattery() ? "YES" : "NO");
    Serial.printf_P(TAG_LOW_POWER_MODE, lowPowerMode ? "ENABLED" : "DISABLED");
    Serial.printf_P(TAG_BAT_MIN, BATTERY_MIN);
    Serial.printf_P(TAG_BAT_MAX, BATTERY_MAX);
    Serial.printf_P(TAG_ENERGY_TRACKING, energyTrackingEnabled ? "ENABLED" : "DISABLED");
    Serial.println(FPSTR(TAG_SEPARATOR));
}

// ============================================================================
// Energy Accounting Implementation
// ============================================================================

void PowerManager::updateEnergyEstimate() {
    if (!energyTrackingEnabled) return;
    
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - energyStats.lastUpdateTime;
    
    // Update active time
    energyStats.activeTimeMs += elapsed;
    
    // Calculate idle energy consumption (mAh)
    float idleEnergy = (powerProfile.idleCurrentMa * elapsed) / (1000.0f * 3600.0f);
    energyStats.energyConsumedMah += idleEnergy;
    
    // Update average current
    unsigned long totalTime = energyStats.activeTimeMs + energyStats.sleepTimeMs;
    if (totalTime > 0) {
        energyStats.averageCurrentMa = (energyStats.energyConsumedMah * 3600.0f * 1000.0f) / totalTime;
    }
    
    // Update estimated remaining hours
    float remainingPercent = (float)getBatteryPercentage() / 100.0f;
    float remainingCapacity = energyStats.batteryCapacityMah * remainingPercent;
    if (energyStats.averageCurrentMa > 0) {
        energyStats.estimatedRemainingHours = remainingCapacity / energyStats.averageCurrentMa;
    }
    
    energyStats.lastUpdateTime = currentTime;
}

void PowerManager::setEnergyTrackingEnabled(bool enable) {
    energyTrackingEnabled = enable;
    if (enable) {
        energyStats.lastUpdateTime = millis();
    }
}

bool PowerManager::isEnergyTrackingEnabled() const {
    return energyTrackingEnabled;
}

void PowerManager::recordCapture() {
    if (!energyTrackingEnabled) return;
    
    energyStats.captureCount++;
    
    // Add energy consumption for capture operation
    float captureEnergy = (powerProfile.captureCurrentMa * powerProfile.captureDurationMs) / (1000.0f * 3600.0f);
    energyStats.energyConsumedMah += captureEnergy;
    
    // Update peak current if this is higher
    if (powerProfile.captureCurrentMa > energyStats.peakCurrentMa) {
        energyStats.peakCurrentMa = powerProfile.captureCurrentMa;
    }
    
    updateEnergyEstimate();
}

void PowerManager::recordTransmission() {
    if (!energyTrackingEnabled) return;
    
    energyStats.transmitCount++;
    
    // Add energy consumption for transmission operation
    float transmitEnergy = (powerProfile.transmitCurrentMa * powerProfile.transmitDurationMs) / (1000.0f * 3600.0f);
    energyStats.energyConsumedMah += transmitEnergy;
    
    // Update peak current if this is higher
    if (powerProfile.transmitCurrentMa > energyStats.peakCurrentMa) {
        energyStats.peakCurrentMa = powerProfile.transmitCurrentMa;
    }
    
    updateEnergyEstimate();
}

void PowerManager::recordSleepPeriod(unsigned long sleepDurationMs) {
    if (!energyTrackingEnabled) return;
    
    energyStats.sleepTimeMs += sleepDurationMs;
    
    // Add energy consumption for deep sleep (convert uA to mA)
    float sleepEnergy = ((powerProfile.deepSleepCurrentUa / 1000.0f) * sleepDurationMs) / (1000.0f * 3600.0f);
    energyStats.energyConsumedMah += sleepEnergy;
    
    updateEnergyEstimate();
}

EnergyStats PowerManager::getEnergyStats() const {
    return energyStats;
}

void PowerManager::resetEnergyStats() {
    memset(&energyStats, 0, sizeof(energyStats));
    energyStats.batteryCapacityMah = DEFAULT_BATTERY_CAPACITY_MAH;
    energyStats.lastUpdateTime = millis();
    energyStats.isValid = true;
    sessionStartTime = millis();
    initialVoltage = batteryVoltage;
}

void PowerManager::setBatteryCapacity(float capacityMah) {
    if (capacityMah > 0) {
        energyStats.batteryCapacityMah = capacityMah;
    }
}

float PowerManager::getEstimatedRemainingHours() {
    updateEnergyEstimate();
    return energyStats.estimatedRemainingHours;
}

void PowerManager::setPowerProfile(const PowerProfile& profile) {
    powerProfile = profile;
}

PowerProfile PowerManager::getPowerProfile() const {
    return powerProfile;
}

void PowerManager::printEnergyReport() {
    updateEnergyEstimate();
    
    Serial.println(FPSTR(TAG_ENERGY_HEADER));
    Serial.printf_P(TAG_ENERGY_TRACKING, energyTrackingEnabled ? "ENABLED" : "DISABLED");
    Serial.printf_P(TAG_ENERGY_ACTIVE, energyStats.activeTimeMs);
    Serial.printf_P(TAG_ENERGY_SLEEP, energyStats.sleepTimeMs);
    Serial.printf_P(TAG_ENERGY_CAPTURES, energyStats.captureCount);
    Serial.printf_P(TAG_ENERGY_TRANSMITS, energyStats.transmitCount);
    Serial.printf_P(TAG_ENERGY_CONSUMED, energyStats.energyConsumedMah);
    Serial.printf_P(TAG_ENERGY_AVG_CURRENT, energyStats.averageCurrentMa);
    Serial.printf_P(TAG_ENERGY_REMAINING, energyStats.estimatedRemainingHours);
    Serial.println(FPSTR(TAG_SEPARATOR));
}
