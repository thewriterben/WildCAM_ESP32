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

PowerManager::PowerManager()
    : batteryPin(35),
      batteryVoltage(0.0),
      lowPowerMode(false) {
}

bool PowerManager::init(int batteryMonitorPin) {
    batteryPin = batteryMonitorPin;
    
    // Configure ADC for battery voltage reading
    pinMode(batteryPin, INPUT);
    
    // Configure ADC attenuation for full range
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
    
    // Read initial battery voltage
    getBatteryVoltage();
    
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
    Serial.println(FPSTR(TAG_SEPARATOR));
}
