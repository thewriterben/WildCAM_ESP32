#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>
#include "../include/config.h"
#include "../include/pins.h"

/**
 * @brief PIR (Passive Infrared) sensor management class
 * 
 * Handles PIR sensor initialization, interrupt handling,
 * and motion detection for wildlife monitoring.
 */
class PIRSensor {
public:
    /**
     * @brief PIR sensor status structure
     */
    struct PIRStatus {
        bool initialized;
        bool motionDetected;
        uint32_t lastMotionTime;
        uint32_t motionCount;
        uint32_t falsePositiveCount;
        float sensitivity;
        bool enabled;
    };

    /**
     * @brief Initialize PIR sensor
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Enable or disable PIR sensor
     * @param enable Enable state
     */
    void setEnabled(bool enable);

    /**
     * @brief Check if motion was detected since last check
     * @return true if motion detected
     */
    bool hasMotion();

    /**
     * @brief Clear motion detection flag
     */
    void clearMotion();

    /**
     * @brief Get time since last motion detection
     * @return Time in milliseconds since last motion
     */
    uint32_t getTimeSinceLastMotion() const;

    /**
     * @brief Set PIR sensor sensitivity
     * @param sensitivity Sensitivity level (0.0 to 1.0)
     */
    void setSensitivity(float sensitivity);

    /**
     * @brief Configure PIR for deep sleep wake-up
     */
    void configureForDeepSleep();

    /**
     * @brief Get PIR sensor status
     * @return Current PIR status
     */
    PIRStatus getStatus() const { return status; }

    /**
     * @brief Reset PIR statistics
     */
    void resetStatistics();

    /**
     * @brief Check if PIR is currently detecting motion
     * @return true if currently detecting
     */
    bool isCurrentlyDetecting() const;

    /**
     * @brief Cleanup PIR sensor resources
     */
    void cleanup();

    /**
     * @brief Static interrupt handler
     */
    static void IRAM_ATTR motionInterruptHandler();

private:
    PIRStatus status = {};
    static PIRSensor* instance;
    uint32_t lastDebounceTime = 0;
    bool motionFlag = false;

    /**
     * @brief Handle motion detection
     */
    void handleMotionDetected();

    /**
     * @brief Apply debounce filtering
     * @return true if motion is valid after debounce
     */
    bool debounceMotion();
};

#endif // PIR_SENSOR_H