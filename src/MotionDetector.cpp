/**
 * @file MotionDetector.cpp
 * @brief Implementation of ESP32-based PIR motion detection system
 */

#include "MotionDetector.h"
#include "config.h"

// Initialize static instance pointer
MotionDetector* MotionDetector::instance = nullptr;

MotionDetector::MotionDetector()
    : motionDetected(false), pirPin(-1), lastTriggerTime(0), debounceMs(2000) {
}

bool MotionDetector::init(int pin, int debounceMs) {
    // Validate input parameters
    if (pin < 0 || pin > 39) {
        Serial.println("ERROR: Invalid GPIO pin number");
        return false;
    }
    
    if (debounceMs < 100 || debounceMs > 10000) {
        Serial.println("ERROR: Debounce time must be between 100ms and 10000ms");
        return false;
    }
    
    pirPin = pin;
    this->debounceMs = debounceMs;
    motionDetected = false;
    lastTriggerTime = 0;
    
    // Set static instance for ISR access
    // Note: Only one MotionDetector instance should be used at a time
    // due to the static instance pointer
    instance = this;
    
    // Configure GPIO pin as input with pulldown
    pinMode(pirPin, INPUT_PULLDOWN);
    
    // Attach interrupt on rising edge (motion detected)
    attachInterrupt(digitalPinToInterrupt(pirPin), motionISR, RISING);
    
    Serial.printf("MotionDetector initialized on pin %d with %dms debounce\n", pirPin, debounceMs);
    return true;
}

void IRAM_ATTR MotionDetector::motionISR() {
    if (instance == nullptr) {
        return;
    }
    
    // Note: millis() is ISR-safe on ESP32 as it uses a hardware timer
    // that maintains the count independently
    unsigned long currentTime = millis();
    
    // Check debounce period
    if (currentTime - instance->lastTriggerTime >= instance->debounceMs) {
        instance->motionDetected = true;
        instance->lastTriggerTime = currentTime;
    }
}

bool MotionDetector::isMotionDetected() {
    // Disable interrupts briefly to ensure atomic read-and-clear
    noInterrupts();
    bool detected = motionDetected;
    if (detected) {
        motionDetected = false;  // Reset flag after reading
    }
    interrupts();
    return detected;
}

void MotionDetector::setDebounceTime(int ms) {
    // Validate input (minimum 100ms, maximum 10000ms)
    if (ms < 100 || ms > 10000) {
        Serial.printf("ERROR: Invalid debounce time %d ms. Must be between 100-10000ms\n", ms);
        return;
    }
    
    debounceMs = ms;
    Serial.printf("Motion debounce time set to: %d ms\n", debounceMs);
}
