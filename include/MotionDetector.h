/**
 * @file MotionDetector.h
 * @brief ESP32-based PIR motion detection system with interrupt support
 * @version 1.0
 * 
 * Thread-safe motion detection using hardware interrupts with configurable
 * debouncing to prevent false triggers.
 */

#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <Arduino.h>

/**
 * @class MotionDetector
 * @brief Thread-safe PIR motion detector with interrupt-based detection
 * 
 * This class provides reliable motion detection using ESP32 hardware interrupts
 * and built-in debouncing to prevent false triggers from electrical noise or
 * sensor instability.
 */
class MotionDetector {
private:
    volatile bool motionDetected;        ///< Flag set by ISR when motion is detected
    int pirPin;                          ///< GPIO pin number for PIR sensor
    unsigned long lastTriggerTime;       ///< Last trigger time for debouncing
    int debounceMs;                      ///< Debounce time in milliseconds
    static MotionDetector* instance;     ///< Static instance for ISR access

    /**
     * @brief Interrupt service routine for motion detection
     * 
     * This method is called when the PIR sensor triggers an interrupt.
     * It implements debouncing logic to prevent false triggers.
     * Must be declared as IRAM_ATTR to run from IRAM for faster execution.
     */
    static void IRAM_ATTR motionISR();

public:
    /**
     * @brief Default constructor
     * 
     * Creates a MotionDetector instance. Call init() to configure the sensor.
     */
    MotionDetector();

    /**
     * @brief Initialize the motion detector
     * @param pin GPIO pin number connected to the PIR sensor
     * @param debounceMs Debounce time in milliseconds (default: 2000ms)
     * 
     * Configures the GPIO pin, sets up the interrupt handler, and initializes
     * the debouncing mechanism. The PIR sensor should be connected to the
     * specified pin with proper pull-down resistor configuration.
     */
    void init(int pin, int debounceMs = 2000);

    /**
     * @brief Check if motion has been detected
     * @return true if motion was detected since last check, false otherwise
     * 
     * This method returns the current state of the motion detection flag
     * and then resets it to false. Thread-safe for use with ISR.
     */
    bool isMotionDetected();

    /**
     * @brief Set the debounce time
     * @param ms Debounce time in milliseconds
     * 
     * Updates the debounce period to prevent multiple triggers from a single
     * motion event. Typical values range from 1000ms to 5000ms depending on
     * the application requirements.
     */
    void setDebounceTime(int ms);
};

#endif // MOTION_DETECTOR_H
