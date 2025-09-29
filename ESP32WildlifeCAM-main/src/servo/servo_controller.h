/**
 * @file servo_controller.h
 * @brief Servo Controller for Pan/Tilt Wildlife Camera System
 * 
 * Provides precise control of SG90 servo motors for automated camera positioning
 * with power-efficient operation and wildlife tracking capabilities.
 */

#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "../../include/config.h"
#include <Arduino.h>
#include <ESP32Servo.h>

/**
 * Servo configuration structure
 */
struct ServoConfig {
    uint8_t pin;                    // GPIO pin for servo control
    uint16_t min_pulse_width;       // Minimum pulse width (microseconds)
    uint16_t max_pulse_width;       // Maximum pulse width (microseconds)
    float min_angle;                // Minimum angle (degrees)
    float max_angle;                // Maximum angle (degrees)
    float center_angle;             // Center/home position (degrees)
    uint16_t move_delay_ms;         // Delay between movements (ms)
    bool invert_direction;          // Invert movement direction
    
    ServoConfig() 
        : pin(16)
        , min_pulse_width(544)      // Standard SG90 min pulse
        , max_pulse_width(2400)     // Standard SG90 max pulse
        , min_angle(0.0f)
        , max_angle(180.0f)
        , center_angle(90.0f)
        , move_delay_ms(20)
        , invert_direction(false) {
    }
};

/**
 * Servo status structure
 */
struct ServoStatus {
    bool initialized;
    bool attached;
    bool moving;
    float current_angle;
    float target_angle;
    unsigned long last_move_time;
    uint32_t total_movements;
    bool power_enabled;
    
    ServoStatus() 
        : initialized(false)
        , attached(false)
        , moving(false)
        , current_angle(90.0f)
        , target_angle(90.0f)
        , last_move_time(0)
        , total_movements(0)
        , power_enabled(false) {
    }
};

/**
 * Servo Controller Class
 * Manages individual servo motors with power optimization
 */
class ServoController {
public:
    /**
     * Constructor
     * @param config Servo configuration
     */
    ServoController(const ServoConfig& config = ServoConfig());
    
    /**
     * Destructor
     */
    ~ServoController();
    
    /**
     * Initialize servo controller
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * Cleanup servo resources
     */
    void cleanup();
    
    /**
     * Move servo to specific angle
     * @param angle Target angle in degrees
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveTo(float angle, bool smooth = true);
    
    /**
     * Move servo relative to current position
     * @param delta_angle Angle change in degrees (positive = clockwise)
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveRelative(float delta_angle, bool smooth = true);
    
    /**
     * Move servo to center/home position
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveToCenter(bool smooth = true);
    
    /**
     * Update servo position (call regularly in main loop)
     * Handles smooth movements and power management
     */
    void update();
    
    /**
     * Check if servo is currently moving
     * @return true if servo is moving
     */
    bool isMoving() const { return status.moving; }
    
    /**
     * Get current servo angle
     * @return Current angle in degrees
     */
    float getCurrentAngle() const { return status.current_angle; }
    
    /**
     * Get target servo angle
     * @return Target angle in degrees
     */
    float getTargetAngle() const { return status.target_angle; }
    
    /**
     * Enable/disable servo power (for power saving)
     * @param enable Power state
     */
    void setPowerEnabled(bool enable);
    
    /**
     * Check if servo power is enabled
     * @return true if power enabled
     */
    bool isPowerEnabled() const { return status.power_enabled; }
    
    /**
     * Set movement speed
     * @param degrees_per_second Movement speed
     */
    void setSpeed(float degrees_per_second);
    
    /**
     * Calibrate servo positions
     * @return true if calibration successful
     */
    bool calibrate();
    
    /**
     * Get servo status
     * @return Servo status structure
     */
    ServoStatus getStatus() const { return status; }
    
    /**
     * Get servo configuration
     * @return Servo configuration structure
     */
    ServoConfig getConfig() const { return config; }
    
    /**
     * Update servo configuration
     * @param new_config New configuration
     * @return true if update successful
     */
    bool updateConfig(const ServoConfig& new_config);

private:
    ServoConfig config;
    ServoStatus status;
    Servo servo;
    
    // Smooth movement parameters
    float movement_speed = 90.0f;   // degrees per second
    float step_size = 1.0f;         // degrees per step
    unsigned long last_step_time = 0;
    
    /**
     * Constrain angle to valid range
     * @param angle Input angle
     * @return Constrained angle
     */
    float constrainAngle(float angle);
    
    /**
     * Convert angle to pulse width
     * @param angle Servo angle in degrees
     * @return Pulse width in microseconds
     */
    uint16_t angleToPulseWidth(float angle);
    
    /**
     * Perform smooth movement step
     */
    void smoothMovementStep();
    
    /**
     * Attach servo to pin
     * @return true if attach successful
     */
    bool attachServo();
    
    /**
     * Detach servo from pin (for power saving)
     */
    void detachServo();
};

/**
 * Servo utilities namespace
 */
namespace ServoUtils {
    /**
     * Create standard pan servo configuration
     * @param pin GPIO pin
     * @return Pan servo configuration
     */
    ServoConfig createPanConfig(uint8_t pin = 16);
    
    /**
     * Create standard tilt servo configuration
     * @param pin GPIO pin
     * @return Tilt servo configuration
     */
    ServoConfig createTiltConfig(uint8_t pin = 17);
    
    /**
     * Validate servo angle range
     * @param angle Angle to validate
     * @param config Servo configuration
     * @return true if angle is valid
     */
    bool isValidAngle(float angle, const ServoConfig& config);
    
    /**
     * Calculate movement duration
     * @param start_angle Starting angle
     * @param end_angle Ending angle
     * @param speed Movement speed (degrees/second)
     * @return Movement duration in milliseconds
     */
    uint32_t calculateMovementDuration(float start_angle, float end_angle, float speed);
}

#endif // SERVO_CONTROLLER_H