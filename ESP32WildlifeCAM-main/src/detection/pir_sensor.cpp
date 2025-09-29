#include "pir_sensor.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>

// Static instance pointer for interrupt handler
PIRSensor* PIRSensor::instance = nullptr;

bool PIRSensor::initialize() {
    if (status.initialized) {
        return true;
    }

    Serial.println("Initializing PIR sensor...");

    // Configure PIR pin
    pinMode(PIR_PIN, INPUT);
    
    // Configure PIR power pin if available
    #ifdef PIR_POWER_PIN
    pinMode(PIR_POWER_PIN, OUTPUT);
    digitalWrite(PIR_POWER_PIN, HIGH);  // Power on PIR sensor
    delay(1000);  // Allow PIR to stabilize
    #endif

    // Set up interrupt
    instance = this;
    attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionInterruptHandler, PIR_TRIGGER_MODE);

    // Initialize status
    status.initialized = true;
    status.enabled = true;
    status.motionDetected = false;
    status.lastMotionTime = 0;
    status.motionCount = 0;
    status.falsePositiveCount = 0;
    status.sensitivity = 1.0f;

    Serial.println("PIR sensor initialized successfully");
    return true;
}

void PIRSensor::setEnabled(bool enable) {
    status.enabled = enable;
    
    if (enable) {
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionInterruptHandler, PIR_TRIGGER_MODE);
        Serial.println("PIR sensor enabled");
    } else {
        detachInterrupt(digitalPinToInterrupt(PIR_PIN));
        clearMotion();
        Serial.println("PIR sensor disabled");
    }
}

bool PIRSensor::hasMotion() {
    if (!status.initialized || !status.enabled) {
        return false;
    }

    // Check motion flag and apply debounce
    if (motionFlag && debounceMotion()) {
        motionFlag = false;
        status.motionDetected = true;
        status.motionCount++;
        status.lastMotionTime = millis();
        
        Serial.println("PIR motion detected");
        return true;
    }

    return false;
}

void PIRSensor::clearMotion() {
    motionFlag = false;
    status.motionDetected = false;
}

uint32_t PIRSensor::getTimeSinceLastMotion() const {
    if (status.lastMotionTime == 0) {
        return UINT32_MAX;  // No motion detected yet
    }
    return millis() - status.lastMotionTime;
}

void PIRSensor::setSensitivity(float sensitivity) {
    status.sensitivity = constrain(sensitivity, 0.0f, 1.0f);
    
    // Adjust PIR sensitivity if hardware supports it
    // This is a software implementation - hardware sensitivity
    // would need additional circuitry
    Serial.printf("PIR sensitivity set to: %.2f\n", status.sensitivity);
}

void PIRSensor::configureForDeepSleep() {
    if (!status.initialized) {
        return;
    }

    // Configure GPIO for RTC domain operation
    rtc_gpio_init((gpio_num_t)PIR_PIN);
    rtc_gpio_set_direction((gpio_num_t)PIR_PIN, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en((gpio_num_t)PIR_PIN);
    rtc_gpio_pullup_dis((gpio_num_t)PIR_PIN);

    // Enable wake-up on PIR trigger
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1);
    
    Serial.println("PIR configured for deep sleep wake-up");
}

void PIRSensor::resetStatistics() {
    status.motionCount = 0;
    status.falsePositiveCount = 0;
    status.lastMotionTime = 0;
    Serial.println("PIR statistics reset");
}

bool PIRSensor::isCurrentlyDetecting() const {
    if (!status.initialized || !status.enabled) {
        return false;
    }

    // Check if PIR is currently high (within debounce time)
    return (digitalRead(PIR_PIN) == HIGH) && 
           (millis() - status.lastMotionTime < PIR_DEBOUNCE_TIME);
}

void PIRSensor::cleanup() {
    if (status.initialized) {
        detachInterrupt(digitalPinToInterrupt(PIR_PIN));
        
        #ifdef PIR_POWER_PIN
        digitalWrite(PIR_POWER_PIN, LOW);  // Power off PIR sensor
        #endif
        
        status.initialized = false;
        instance = nullptr;
        
        Serial.println("PIR sensor cleaned up");
    }
}

void IRAM_ATTR PIRSensor::motionInterruptHandler() {
    if (instance != nullptr && instance->status.enabled) {
        instance->motionFlag = true;
    }
}

void PIRSensor::handleMotionDetected() {
    uint32_t now = millis();
    
    // Simple false positive filtering based on sensitivity
    if (status.sensitivity < 1.0f) {
        // Reduce sensitivity by requiring multiple triggers
        static uint32_t lastTriggerTime = 0;
        static int triggerCount = 0;
        
        if (now - lastTriggerTime < 1000) {  // Within 1 second
            triggerCount++;
        } else {
            triggerCount = 1;
        }
        
        lastTriggerTime = now;
        
        // Require multiple triggers for low sensitivity
        int requiredTriggers = (int)(1.0f / status.sensitivity);
        if (triggerCount < requiredTriggers) {
            status.falsePositiveCount++;
            return;
        }
    }
    
    // Valid motion detected
    status.motionDetected = true;
    status.motionCount++;
    status.lastMotionTime = now;
}

bool PIRSensor::debounceMotion() {
    uint32_t now = millis();
    
    // Apply debounce time
    if (now - lastDebounceTime < PIR_DEBOUNCE_TIME) {
        return false;
    }
    
    lastDebounceTime = now;
    return true;
}