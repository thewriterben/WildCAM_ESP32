/**
 * @file pan_tilt_manager.cpp
 * @brief Pan/Tilt Manager Implementation
 * 
 * Implementation of coordinated pan/tilt servo control system.
 */

#include "pan_tilt_manager.h"

// Constructor
PanTiltManager::PanTiltManager() {
}

// Destructor
PanTiltManager::~PanTiltManager() {
    cleanup();
}

// Initialize pan/tilt system
bool PanTiltManager::initialize(uint8_t pan_pin, uint8_t tilt_pin) {
    if (initialized) {
        return true; // Already initialized
    }
    
    Serial.printf("[PanTiltManager] Initializing pan/tilt system on pins %d, %d\n", pan_pin, tilt_pin);
    
    // Create servo configurations
    ServoConfig pan_config = ServoUtils::createPanConfig(pan_pin);
    ServoConfig tilt_config = ServoUtils::createTiltConfig(tilt_pin);
    
    // Create servo controllers
    pan_servo = std::make_unique<ServoController>(pan_config);
    tilt_servo = std::make_unique<ServoController>(tilt_config);
    
    // Initialize servos
    if (!pan_servo->initialize()) {
        Serial.printf("[PanTiltManager] ERROR: Failed to initialize pan servo\n");
        return false;
    }
    
    if (!tilt_servo->initialize()) {
        Serial.printf("[PanTiltManager] ERROR: Failed to initialize tilt servo\n");
        pan_servo->cleanup();
        return false;
    }
    
    // Move to home position
    moveToHome(false);
    delay(1000); // Allow servos to reach position
    
    initialized = true;
    power_enabled = true;
    
    Serial.printf("[PanTiltManager] Pan/tilt system initialized successfully\n");
    return true;
}

// Cleanup resources
void PanTiltManager::cleanup() {
    if (pan_servo) {
        pan_servo->cleanup();
        pan_servo.reset();
    }
    
    if (tilt_servo) {
        tilt_servo->cleanup();
        tilt_servo.reset();
    }
    
    stopScanning();
    stopTracking();
    custom_positions.clear();
    
    initialized = false;
    power_enabled = false;
    
    Serial.printf("[PanTiltManager] Pan/tilt system cleaned up\n");
}

// Update system (call regularly)
void PanTiltManager::update() {
    if (!initialized) {
        return;
    }
    
    // Update servo controllers
    pan_servo->update();
    tilt_servo->update();
    
    // Process scanning state machine
    if (scanning_active) {
        processScanningStateMachine();
    }
    
    // Process tracking state machine
    if (tracking_active) {
        processTrackingStateMachine();
    }
}

// Move to specific position
bool PanTiltManager::moveTo(const PanTiltPosition& position, bool smooth) {
    return moveTo(position.pan_angle, position.tilt_angle, smooth);
}

// Move to specific angles
bool PanTiltManager::moveTo(float pan_angle, float tilt_angle, bool smooth) {
    if (!initialized || !power_enabled) {
        Serial.printf("[PanTiltManager] ERROR: System not initialized or powered\n");
        return false;
    }
    
    // Validate position
    PanTiltPosition pos(pan_angle, tilt_angle);
    if (!isValidPosition(pos)) {
        Serial.printf("[PanTiltManager] ERROR: Invalid position (%.1f, %.1f)\n", pan_angle, tilt_angle);
        return false;
    }
    
    // Move servos
    bool pan_success = pan_servo->moveTo(pan_angle, smooth);
    bool tilt_success = tilt_servo->moveTo(tilt_angle, smooth);
    
    if (pan_success && tilt_success) {
        Serial.printf("[PanTiltManager] Moving to position (%.1f, %.1f)\n", pan_angle, tilt_angle);
        return true;
    } else {
        Serial.printf("[PanTiltManager] ERROR: Failed to move to position\n");
        return false;
    }
}

// Move to home position
bool PanTiltManager::moveToHome(bool smooth) {
    return moveTo(90.0f, 90.0f, smooth);  // Center position
}

// Start scanning
bool PanTiltManager::startScanning(const ScanConfig& config) {
    if (!initialized) {
        Serial.printf("[PanTiltManager] ERROR: Cannot start scanning - not initialized\n");
        return false;
    }
    
    if (tracking_active) {
        stopTracking();
    }
    
    current_scan_config = config;
    current_scan_position = 0;
    last_scan_move_time = 0;
    scan_dwell_start_time = 0;
    scan_moving = false;
    scanning_active = true;
    
    Serial.printf("[PanTiltManager] Started scanning with pattern %d\n", (int)config.pattern);
    
    // Generate scan positions based on pattern
    custom_positions = generateScanPositions();
    
    if (custom_positions.empty()) {
        Serial.printf("[PanTiltManager] WARNING: No scan positions generated\n");
        scanning_active = false;
        return false;
    }
    
    return true;
}

// Stop scanning
void PanTiltManager::stopScanning() {
    if (scanning_active) {
        scanning_active = false;
        scan_moving = false;
        total_scan_cycles++;
        Serial.printf("[PanTiltManager] Scanning stopped\n");
    }
}

// Start tracking
bool PanTiltManager::startTracking(const TrackingTarget& target) {
    if (!initialized) {
        return false;
    }
    
    if (scanning_active) {
        stopScanning();
    }
    
    current_target = target;
    tracking_active = true;
    last_tracking_update = millis();
    
    // Move to target position
    moveTo(target.pan_angle, target.tilt_angle, true);
    
    Serial.printf("[PanTiltManager] Started tracking target at (%.1f, %.1f)\n", 
                  target.pan_angle, target.tilt_angle);
    
    return true;
}

// Update tracking
bool PanTiltManager::updateTracking(const TrackingTarget& target) {
    if (!tracking_active) {
        return false;
    }
    
    current_target = target;
    last_tracking_update = millis();
    
    // Move to updated target position
    return moveTo(target.pan_angle, target.tilt_angle, true);
}

// Stop tracking
void PanTiltManager::stopTracking() {
    if (tracking_active) {
        tracking_active = false;
        
        // Update statistics
        if (current_target.active) {
            successful_tracks++;
            total_tracking_time += (millis() - last_tracking_update);
        } else {
            lost_tracks++;
        }
        
        Serial.printf("[PanTiltManager] Tracking stopped\n");
    }
}

// Get current position
PanTiltPosition PanTiltManager::getCurrentPosition() const {
    if (!initialized) {
        return PanTiltPosition();
    }
    
    PanTiltPosition pos;
    pos.pan_angle = pan_servo->getCurrentAngle();
    pos.tilt_angle = tilt_servo->getCurrentAngle();
    pos.name = "current";
    
    return pos;
}

// Check if moving
bool PanTiltManager::isMoving() const {
    if (!initialized) {
        return false;
    }
    
    return pan_servo->isMoving() || tilt_servo->isMoving();
}

// Set power enabled
void PanTiltManager::setPowerEnabled(bool enable) {
    power_enabled = enable;
    
    if (initialized) {
        pan_servo->setPowerEnabled(enable);
        tilt_servo->setPowerEnabled(enable);
        
        if (!enable) {
            stopScanning();
            stopTracking();
        }
    }
    
    Serial.printf("[PanTiltManager] Power %s\n", enable ? "enabled" : "disabled");
}

// Set speed
void PanTiltManager::setSpeed(float degrees_per_second) {
    if (initialized) {
        pan_servo->setSpeed(degrees_per_second);
        tilt_servo->setSpeed(degrees_per_second);
        Serial.printf("[PanTiltManager] Speed set to %.1f deg/s\n", degrees_per_second);
    }
}

// Calibrate servos
bool PanTiltManager::calibrate() {
    if (!initialized) {
        return false;
    }
    
    Serial.printf("[PanTiltManager] Starting calibration...\n");
    
    bool pan_calibrated = pan_servo->calibrate();
    bool tilt_calibrated = tilt_servo->calibrate();
    
    if (pan_calibrated && tilt_calibrated) {
        Serial.printf("[PanTiltManager] Calibration completed successfully\n");
        return true;
    } else {
        Serial.printf("[PanTiltManager] Calibration failed\n");
        return false;
    }
}

// Add scan position
void PanTiltManager::addScanPosition(const PanTiltPosition& position) {
    if (isValidPosition(position)) {
        custom_positions.push_back(position);
        Serial.printf("[PanTiltManager] Added scan position: %s (%.1f, %.1f)\n", 
                      position.name.c_str(), position.pan_angle, position.tilt_angle);
    }
}

// Clear scan positions
void PanTiltManager::clearScanPositions() {
    custom_positions.clear();
    Serial.printf("[PanTiltManager] Scan positions cleared\n");
}

// Get scan statistics
String PanTiltManager::getScanStatistics() const {
    String stats = "Scan Stats: ";
    stats += "Cycles: " + String(total_scan_cycles);
    stats += ", Active: " + String(scanning_active ? "Yes" : "No");
    stats += ", Positions: " + String(custom_positions.size());
    return stats;
}

// Get tracking statistics
String PanTiltManager::getTrackingStatistics() const {
    String stats = "Track Stats: ";
    stats += "Success: " + String(successful_tracks);
    stats += ", Lost: " + String(lost_tracks);
    stats += ", Active: " + String(tracking_active ? "Yes" : "No");
    return stats;
}

// Private methods

// Generate scan positions
std::vector<PanTiltPosition> PanTiltManager::generateScanPositions() {
    switch (current_scan_config.pattern) {
        case ScanPattern::HORIZONTAL_SWEEP:
            return generateHorizontalSweep();
        case ScanPattern::VERTICAL_SWEEP:
            return generateVerticalSweep();
        case ScanPattern::GRID_SCAN:
            return generateGridScan();
        case ScanPattern::SPIRAL_SCAN:
            return generateSpiralScan();
        case ScanPattern::RANDOM_SCAN:
            return generateRandomScan();
        case ScanPattern::CUSTOM:
            return custom_positions;
        default:
            return std::vector<PanTiltPosition>();
    }
}

// Process scanning state machine
void PanTiltManager::processScanningStateMachine() {
    if (custom_positions.empty()) {
        stopScanning();
        return;
    }
    
    unsigned long current_time = millis();
    
    if (!scan_moving) {
        // Check if dwell time is complete
        if (current_time - scan_dwell_start_time >= current_scan_config.dwell_time_ms) {
            // Move to next position
            current_scan_position = (current_scan_position + 1) % custom_positions.size();
            
            const PanTiltPosition& next_pos = custom_positions[current_scan_position];
            if (moveTo(next_pos, true)) {
                scan_moving = true;
                last_scan_move_time = current_time;
            }
        }
    } else {
        // Check if movement is complete
        if (!isMoving() && (current_time - last_scan_move_time >= current_scan_config.move_time_ms)) {
            scan_moving = false;
            scan_dwell_start_time = current_time;
        }
    }
}

// Process tracking state machine
void PanTiltManager::processTrackingStateMachine() {
    unsigned long current_time = millis();
    
    // Check for tracking timeout
    if (current_time - last_tracking_update > TRACKING_TIMEOUT_MS) {
        Serial.printf("[PanTiltManager] Tracking timeout\n");
        stopTracking();
    }
}

// Generate horizontal sweep
std::vector<PanTiltPosition> PanTiltManager::generateHorizontalSweep() {
    std::vector<PanTiltPosition> positions;
    
    float start_pan = 30.0f;
    float end_pan = 150.0f;
    float tilt = 90.0f;
    
    for (float pan = start_pan; pan <= end_pan; pan += current_scan_config.pan_step) {
        positions.emplace_back(pan, tilt, "sweep_" + String((int)pan));
    }
    
    return positions;
}

// Generate vertical sweep
std::vector<PanTiltPosition> PanTiltManager::generateVerticalSweep() {
    std::vector<PanTiltPosition> positions;
    
    float pan = 90.0f;
    float start_tilt = 60.0f;
    float end_tilt = 120.0f;
    
    for (float tilt = start_tilt; tilt <= end_tilt; tilt += current_scan_config.tilt_step) {
        positions.emplace_back(pan, tilt, "sweep_" + String((int)tilt));
    }
    
    return positions;
}

// Generate grid scan
std::vector<PanTiltPosition> PanTiltManager::generateGridScan() {
    std::vector<PanTiltPosition> positions;
    
    float start_pan = 45.0f;
    float end_pan = 135.0f;
    float start_tilt = 70.0f;
    float end_tilt = 110.0f;
    
    for (float tilt = start_tilt; tilt <= end_tilt; tilt += current_scan_config.tilt_step) {
        for (float pan = start_pan; pan <= end_pan; pan += current_scan_config.pan_step) {
            positions.emplace_back(pan, tilt, "grid_" + String((int)pan) + "_" + String((int)tilt));
        }
    }
    
    return positions;
}

// Generate spiral scan
std::vector<PanTiltPosition> PanTiltManager::generateSpiralScan() {
    std::vector<PanTiltPosition> positions;
    
    float center_pan = 90.0f;
    float center_tilt = 90.0f;
    float radius = 0.0f;
    float angle = 0.0f;
    float max_radius = 45.0f;
    
    while (radius <= max_radius) {
        float pan = center_pan + radius * cos(angle);
        float tilt = center_tilt + radius * sin(angle);
        
        if (isValidPosition(PanTiltPosition(pan, tilt))) {
            positions.emplace_back(pan, tilt, "spiral_" + String((int)radius));
        }
        
        angle += PI / 6.0f;  // 30 degree increments
        if (angle >= 2 * PI) {
            angle = 0;
            radius += 15.0f;
        }
    }
    
    return positions;
}

// Generate random scan
std::vector<PanTiltPosition> PanTiltManager::generateRandomScan() {
    std::vector<PanTiltPosition> positions;
    
    randomSeed(millis());
    
    for (int i = 0; i < 10; i++) {
        float pan = random(45, 136);    // 45-135 degrees
        float tilt = random(70, 111);   // 70-110 degrees
        
        positions.emplace_back(pan, tilt, "random_" + String(i));
    }
    
    return positions;
}

// Validate position
bool PanTiltManager::isValidPosition(const PanTiltPosition& position) {
    return (position.pan_angle >= 0.0f && position.pan_angle <= 180.0f &&
            position.tilt_angle >= 45.0f && position.tilt_angle <= 135.0f);
}

// Calculate movement time
uint32_t PanTiltManager::calculateMovementTime(const PanTiltPosition& from, const PanTiltPosition& to) {
    float pan_diff = abs(to.pan_angle - from.pan_angle);
    float tilt_diff = abs(to.tilt_angle - from.tilt_angle);
    float max_diff = max(pan_diff, tilt_diff);
    
    // Assume 90 degrees per second movement speed
    return (uint32_t)(max_diff / 90.0f * 1000.0f);
}

// Utility functions implementation
namespace PanTiltUtils {
    
    PanTiltPosition createCenterPosition() {
        return PanTiltPosition(90.0f, 90.0f, "center");
    }
    
    PanTiltPosition createHomePosition() {
        return createCenterPosition();
    }
    
    PanTiltPosition createLookLeftPosition() {
        return PanTiltPosition(135.0f, 90.0f, "left");
    }
    
    PanTiltPosition createLookRightPosition() {
        return PanTiltPosition(45.0f, 90.0f, "right");
    }
    
    PanTiltPosition createLookUpPosition() {
        return PanTiltPosition(90.0f, 70.0f, "up");
    }
    
    PanTiltPosition createLookDownPosition() {
        return PanTiltPosition(90.0f, 110.0f, "down");
    }
    
    PanTiltPosition motionToPosition(float motion_x, float motion_y) {
        // Convert motion coordinates (0.0-1.0) to pan/tilt angles
        float pan = 45.0f + motion_x * 90.0f;   // 45-135 degrees
        float tilt = 70.0f + motion_y * 40.0f;  // 70-110 degrees
        
        return PanTiltPosition(pan, tilt, "motion");
    }
    
    float calculateDistance(const PanTiltPosition& pos1, const PanTiltPosition& pos2) {
        float pan_diff = pos2.pan_angle - pos1.pan_angle;
        float tilt_diff = pos2.tilt_angle - pos1.tilt_angle;
        
        return sqrt(pan_diff * pan_diff + tilt_diff * tilt_diff);
    }
    
    PanTiltPosition interpolatePosition(const PanTiltPosition& from, const PanTiltPosition& to, float t) {
        t = constrain(t, 0.0f, 1.0f);
        
        float pan = from.pan_angle + t * (to.pan_angle - from.pan_angle);
        float tilt = from.tilt_angle + t * (to.tilt_angle - from.tilt_angle);
        
        return PanTiltPosition(pan, tilt, "interpolated");
    }
}