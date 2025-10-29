#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <Arduino.h>

class MotionDetector {
private:
    int pirPin;
    int sensitivity;
    unsigned long lastTriggerTime;
    unsigned long cooldownPeriod;
    bool calibrationComplete;

public:
    MotionDetector(int pin, int sens = 50, unsigned long cooldown = 5000);
    
    // Initialize the motion detector
    bool begin();
    
    // Check if motion is detected
    bool detectMotion();
    
    // Calibrate the PIR sensor
    void calibrate();
    
    // Set sensitivity level (0-100)
    void setSensitivity(int level);
    
    // Set cooldown period between detections
    void setCooldown(unsigned long ms);
    
    // Get time since last detection
    unsigned long getTimeSinceLastTrigger();
    
    // Check if sensor is ready
    bool isReady();
};

#endif // MOTION_DETECTOR_H
