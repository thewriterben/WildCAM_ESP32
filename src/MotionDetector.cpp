#include "MotionDetector.h"
#include "config.h"

MotionDetector::MotionDetector(int pin, int sens, unsigned long cooldown)
    : pirPin(pin), sensitivity(sens), cooldownPeriod(cooldown),
      lastTriggerTime(0), calibrationComplete(false) {
}

bool MotionDetector::begin() {
    pinMode(pirPin, INPUT);
    
    // Wait for PIR sensor to stabilize
    Serial.println("Calibrating PIR sensor...");
    calibrate();
    
    return true;
}

bool MotionDetector::detectMotion() {
    if (!calibrationComplete) {
        return false;
    }
    
    // Check cooldown period
    unsigned long currentTime = millis();
    if (currentTime - lastTriggerTime < cooldownPeriod) {
        return false;
    }
    
    // Read PIR sensor
    int sensorValue = digitalRead(pirPin);
    
    if (sensorValue == HIGH) {
        lastTriggerTime = currentTime;
        Serial.println("Motion detected!");
        return true;
    }
    
    return false;
}

void MotionDetector::calibrate() {
    Serial.println("Starting PIR calibration (waiting 30 seconds)...");
    
    // PIR sensors typically need 30-60 seconds to stabilize
    unsigned long calibrationTime = 30000;  // 30 seconds
    unsigned long startTime = millis();
    
    while (millis() - startTime < calibrationTime) {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.println("\nPIR calibration complete!");
    calibrationComplete = true;
}

void MotionDetector::setSensitivity(int level) {
    if (level >= 0 && level <= 100) {
        sensitivity = level;
        Serial.printf("Motion sensitivity set to: %d\n", sensitivity);
    }
}

void MotionDetector::setCooldown(unsigned long ms) {
    cooldownPeriod = ms;
    Serial.printf("Motion cooldown set to: %lu ms\n", cooldownPeriod);
}

unsigned long MotionDetector::getTimeSinceLastTrigger() {
    return millis() - lastTriggerTime;
}

bool MotionDetector::isReady() {
    return calibrationComplete;
}
