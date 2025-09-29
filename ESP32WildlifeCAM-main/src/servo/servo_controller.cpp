/**
 * @file servo_controller.cpp
 * @brief Servo Controller Implementation
 * 
 * Implementation of servo control system for pan/tilt wildlife camera operations.
 */

#include "servo_controller.h"

// Constructor
ServoController::ServoController(const ServoConfig& config) 
    : config(config), status(), servo() {
}

// Destructor
ServoController::~ServoController() {
    cleanup();
}

// Initialize servo controller
bool ServoController::initialize() {
    if (status.initialized) {
        return true; // Already initialized
    }
    
    Serial.printf("[ServoController] Initializing servo on GPIO %d\n", config.pin);
    
    // Validate pin
    if (config.pin == 0 || config.pin > 39) {
        Serial.printf("[ServoController] ERROR: Invalid GPIO pin %d\n", config.pin);
        return false;
    }
    
    // Initialize servo library (ESP32Servo uses timer allocation automatically)
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Set servo configuration
    servo.setPeriodHertz(50); // Standard 50 Hz servo frequency
    
    // Attach servo
    if (!attachServo()) {
        Serial.printf("[ServoController] ERROR: Failed to attach servo\n");
        return false;
    }
    
    // Move to center position
    status.current_angle = config.center_angle;
    status.target_angle = config.center_angle;
    servo.write(status.current_angle);
    delay(500); // Allow time for servo to reach position
    
    status.initialized = true;
    status.power_enabled = true;
    status.last_move_time = millis();
    
    Serial.printf("[ServoController] Servo initialized successfully at %.1f degrees\n", 
                  status.current_angle);
    
    return true;
}

// Cleanup servo resources
void ServoController::cleanup() {
    if (status.attached) {
        detachServo();
    }
    status.initialized = false;
    status.power_enabled = false;
    Serial.printf("[ServoController] Servo cleaned up\n");
}

// Move servo to specific angle
bool ServoController::moveTo(float angle, bool smooth) {
    if (!status.initialized || !status.power_enabled) {
        Serial.printf("[ServoController] ERROR: Servo not initialized or powered\n");
        return false;
    }
    
    // Constrain angle to valid range
    angle = constrainAngle(angle);
    
    if (smooth && abs(angle - status.current_angle) > step_size) {
        // Start smooth movement
        status.target_angle = angle;
        status.moving = true;
        Serial.printf("[ServoController] Starting smooth movement to %.1f degrees\n", angle);
    } else {
        // Direct movement
        status.current_angle = angle;
        status.target_angle = angle;
        status.moving = false;
        
        if (status.attached) {
            servo.write(status.current_angle);
        }
        
        Serial.printf("[ServoController] Direct movement to %.1f degrees\n", angle);
    }
    
    status.last_move_time = millis();
    status.total_movements++;
    
    return true;
}

// Move servo relative to current position
bool ServoController::moveRelative(float delta_angle, bool smooth) {
    float new_angle = status.current_angle + delta_angle;
    return moveTo(new_angle, smooth);
}

// Move servo to center position
bool ServoController::moveToCenter(bool smooth) {
    return moveTo(config.center_angle, smooth);
}

// Update servo position (call regularly)
void ServoController::update() {
    if (!status.initialized) {
        return;
    }
    
    // Handle smooth movement
    if (status.moving) {
        smoothMovementStep();
    }
    
    // Auto-detach servo after idle time (power saving)
    if (status.attached && status.power_enabled) {
        unsigned long idle_time = millis() - status.last_move_time;
        if (idle_time > 5000 && !status.moving) { // 5 second idle timeout
            Serial.printf("[ServoController] Auto-detaching servo for power saving\n");
            detachServo();
        }
    }
}

// Enable/disable servo power
void ServoController::setPowerEnabled(bool enable) {
    if (enable == status.power_enabled) {
        return; // No change needed
    }
    
    status.power_enabled = enable;
    
    if (enable) {
        if (!status.attached) {
            attachServo();
            // Re-position servo
            servo.write(status.current_angle);
        }
        Serial.printf("[ServoController] Servo power enabled\n");
    } else {
        if (status.attached) {
            detachServo();
        }
        status.moving = false;
        Serial.printf("[ServoController] Servo power disabled\n");
    }
}

// Set movement speed
void ServoController::setSpeed(float degrees_per_second) {
    if (degrees_per_second > 0 && degrees_per_second <= 360) {
        movement_speed = degrees_per_second;
        Serial.printf("[ServoController] Movement speed set to %.1f deg/s\n", movement_speed);
    }
}

// Calibrate servo positions
bool ServoController::calibrate() {
    if (!status.initialized) {
        Serial.printf("[ServoController] ERROR: Cannot calibrate uninitialized servo\n");
        return false;
    }
    
    Serial.printf("[ServoController] Starting servo calibration...\n");
    
    // Ensure servo is attached and powered
    setPowerEnabled(true);
    
    // Test full range movement
    moveTo(config.min_angle, false);
    delay(1000);
    
    moveTo(config.max_angle, false);
    delay(1000);
    
    moveTo(config.center_angle, false);
    delay(1000);
    
    Serial.printf("[ServoController] Calibration complete\n");
    return true;
}

// Update servo configuration
bool ServoController::updateConfig(const ServoConfig& new_config) {
    bool was_initialized = status.initialized;
    
    if (was_initialized) {
        cleanup();
    }
    
    config = new_config;
    
    if (was_initialized) {
        return initialize();
    }
    
    return true;
}

// Private methods

// Constrain angle to valid range
float ServoController::constrainAngle(float angle) {
    if (config.invert_direction) {
        angle = config.max_angle - angle + config.min_angle;
    }
    
    return constrain(angle, config.min_angle, config.max_angle);
}

// Convert angle to pulse width
uint16_t ServoController::angleToPulseWidth(float angle) {
    float range = config.max_angle - config.min_angle;
    float pulse_range = config.max_pulse_width - config.min_pulse_width;
    float normalized = (angle - config.min_angle) / range;
    return config.min_pulse_width + (normalized * pulse_range);
}

// Perform smooth movement step
void ServoController::smoothMovementStep() {
    unsigned long current_time = millis();
    unsigned long time_diff = current_time - last_step_time;
    
    if (time_diff < config.move_delay_ms) {
        return; // Not time for next step
    }
    
    last_step_time = current_time;
    
    float angle_diff = status.target_angle - status.current_angle;
    
    if (abs(angle_diff) <= step_size) {
        // Close enough, finish movement
        status.current_angle = status.target_angle;
        status.moving = false;
        
        if (status.attached) {
            servo.write(status.current_angle);
        }
        
        Serial.printf("[ServoController] Smooth movement completed at %.1f degrees\n", 
                      status.current_angle);
    } else {
        // Take a step toward target
        float step = (angle_diff > 0) ? step_size : -step_size;
        status.current_angle += step;
        
        if (status.attached) {
            servo.write(status.current_angle);
        }
    }
    
    status.last_move_time = current_time;
}

// Attach servo to pin
bool ServoController::attachServo() {
    if (status.attached) {
        return true;
    }
    
    bool attached = servo.attach(config.pin, config.min_pulse_width, config.max_pulse_width);
    
    if (attached) {
        status.attached = true;
        Serial.printf("[ServoController] Servo attached to GPIO %d\n", config.pin);
    } else {
        Serial.printf("[ServoController] ERROR: Failed to attach servo to GPIO %d\n", config.pin);
    }
    
    return attached;
}

// Detach servo from pin
void ServoController::detachServo() {
    if (status.attached) {
        servo.detach();
        status.attached = false;
        Serial.printf("[ServoController] Servo detached from GPIO %d\n", config.pin);
    }
}

// Servo utilities namespace implementation
namespace ServoUtils {
    
    ServoConfig createPanConfig(uint8_t pin) {
        ServoConfig config;
        config.pin = pin;
        config.min_angle = 0.0f;        // 0 degrees (full left)
        config.max_angle = 180.0f;      // 180 degrees (full right)
        config.center_angle = 90.0f;    // 90 degrees (center)
        config.invert_direction = false;
        return config;
    }
    
    ServoConfig createTiltConfig(uint8_t pin) {
        ServoConfig config;
        config.pin = pin;
        config.min_angle = 45.0f;       // 45 degrees (looking up)
        config.max_angle = 135.0f;      // 135 degrees (looking down)
        config.center_angle = 90.0f;    // 90 degrees (horizontal)
        config.invert_direction = false;
        return config;
    }
    
    bool isValidAngle(float angle, const ServoConfig& config) {
        return (angle >= config.min_angle && angle <= config.max_angle);
    }
    
    uint32_t calculateMovementDuration(float start_angle, float end_angle, float speed) {
        if (speed <= 0) return 0;
        
        float angle_diff = abs(end_angle - start_angle);
        return (uint32_t)(angle_diff / speed * 1000.0f); // Convert to milliseconds
    }
}