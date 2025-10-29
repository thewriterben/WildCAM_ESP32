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

void MotionDetector::init(int pin, int debounceMs) {
    pirPin = pin;
    this->debounceMs = debounceMs;
    motionDetected = false;
    lastTriggerTime = 0;
    
    // Set static instance for ISR access
    instance = this;
    
    // Configure GPIO pin as input
    pinMode(pirPin, INPUT);
    
    // Attach interrupt on rising edge (motion detected)
    attachInterrupt(digitalPinToInterrupt(pirPin), motionISR, RISING);
    
    Serial.printf("MotionDetector initialized on pin %d with %dms debounce\n", pirPin, debounceMs);
}

void IRAM_ATTR MotionDetector::motionISR() {
    if (instance == nullptr) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Check debounce period
    if (currentTime - instance->lastTriggerTime >= instance->debounceMs) {
        instance->motionDetected = true;
        instance->lastTriggerTime = currentTime;
    }
}

bool MotionDetector::isMotionDetected() {
    bool detected = motionDetected;
    if (detected) {
        motionDetected = false;  // Reset flag after reading
    }
    return detected;
}

void MotionDetector::setDebounceTime(int ms) {
    debounceMs = ms;
    Serial.printf("Motion debounce time set to: %d ms\n", debounceMs);
}
