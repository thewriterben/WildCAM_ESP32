/**
 * @file pan_tilt_manager.h
 * @brief Pan/Tilt Manager for Coordinated Servo Control
 * 
 * Manages coordinated pan and tilt servo movements for wildlife tracking
 * and automated scanning patterns with power optimization.
 */

#ifndef PAN_TILT_MANAGER_H
#define PAN_TILT_MANAGER_H

#include "servo_controller.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <vector>

/**
 * Pan/Tilt position structure
 */
struct PanTiltPosition {
    float pan_angle;    // Pan angle in degrees (0-180)
    float tilt_angle;   // Tilt angle in degrees (45-135)
    String name;        // Position name for identification
    
    PanTiltPosition() 
        : pan_angle(90.0f), tilt_angle(90.0f), name("center") {
    }
    
    PanTiltPosition(float pan, float tilt, const String& position_name = "") 
        : pan_angle(pan), tilt_angle(tilt), name(position_name) {
    }
};

/**
 * Scanning pattern types
 */
enum class ScanPattern {
    NONE,               // No scanning
    HORIZONTAL_SWEEP,   // Left-right horizontal sweep
    VERTICAL_SWEEP,     // Up-down vertical sweep
    GRID_SCAN,          // Grid pattern scan
    SPIRAL_SCAN,        // Spiral pattern from center
    RANDOM_SCAN,        // Random position scanning
    CUSTOM              // Custom predefined positions
};

/**
 * Scanning configuration
 */
struct ScanConfig {
    ScanPattern pattern;
    uint32_t dwell_time_ms;     // Time to pause at each position
    uint32_t move_time_ms;      // Time allowed for movement
    float pan_step;             // Pan angle step size
    float tilt_step;            // Tilt angle step size
    bool continuous;            // Continuous scanning or single pass
    uint8_t scan_speed;         // Scanning speed (1-10)
    
    ScanConfig() 
        : pattern(ScanPattern::HORIZONTAL_SWEEP)
        , dwell_time_ms(2000)
        , move_time_ms(1000)
        , pan_step(30.0f)
        , tilt_step(30.0f)
        , continuous(true)
        , scan_speed(5) {
    }
};

/**
 * Tracking target information
 */
struct TrackingTarget {
    float pan_angle;            // Target pan position
    float tilt_angle;           // Target tilt position
    float confidence;           // Tracking confidence (0.0-1.0)
    uint32_t last_seen_time;    // When target was last detected
    bool active;                // Is tracking active
    String species;             // Detected species (if known)
    
    TrackingTarget() 
        : pan_angle(90.0f)
        , tilt_angle(90.0f)
        , confidence(0.0f)
        , last_seen_time(0)
        , active(false)
        , species("unknown") {
    }
};

/**
 * Pan/Tilt Manager Class
 * Coordinates pan and tilt servos for wildlife monitoring
 */
class PanTiltManager {
public:
    /**
     * Constructor
     */
    PanTiltManager();
    
    /**
     * Destructor
     */
    ~PanTiltManager();
    
    /**
     * Initialize pan/tilt system
     * @param pan_pin GPIO pin for pan servo
     * @param tilt_pin GPIO pin for tilt servo
     * @return true if initialization successful
     */
    bool initialize(uint8_t pan_pin = 16, uint8_t tilt_pin = 17);
    
    /**
     * Cleanup pan/tilt resources
     */
    void cleanup();
    
    /**
     * Update pan/tilt system (call regularly in main loop)
     */
    void update();
    
    /**
     * Move to specific pan/tilt position
     * @param position Target position
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveTo(const PanTiltPosition& position, bool smooth = true);
    
    /**
     * Move to specific pan/tilt angles
     * @param pan_angle Pan angle in degrees
     * @param tilt_angle Tilt angle in degrees
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveTo(float pan_angle, float tilt_angle, bool smooth = true);
    
    /**
     * Move to center/home position
     * @param smooth Enable smooth movement
     * @return true if movement initiated successfully
     */
    bool moveToHome(bool smooth = true);
    
    /**
     * Start automated scanning
     * @param config Scanning configuration
     * @return true if scanning started successfully
     */
    bool startScanning(const ScanConfig& config = ScanConfig());
    
    /**
     * Stop automated scanning
     */
    void stopScanning();
    
    /**
     * Check if system is currently scanning
     * @return true if scanning is active
     */
    bool isScanning() const { return scanning_active; }
    
    /**
     * Start tracking a target
     * @param target Target tracking information
     * @return true if tracking started successfully
     */
    bool startTracking(const TrackingTarget& target);
    
    /**
     * Update tracking target position
     * @param target Updated target information
     * @return true if tracking updated successfully
     */
    bool updateTracking(const TrackingTarget& target);
    
    /**
     * Stop tracking
     */
    void stopTracking();
    
    /**
     * Check if system is currently tracking
     * @return true if tracking is active
     */
    bool isTracking() const { return tracking_active; }
    
    /**
     * Get current pan/tilt position
     * @return Current position
     */
    PanTiltPosition getCurrentPosition() const;
    
    /**
     * Check if servos are currently moving
     * @return true if either servo is moving
     */
    bool isMoving() const;
    
    /**
     * Enable/disable pan/tilt system power
     * @param enable Power state
     */
    void setPowerEnabled(bool enable);
    
    /**
     * Check if pan/tilt system is powered
     * @return true if system is powered
     */
    bool isPowerEnabled() const;
    
    /**
     * Set movement speed for both servos
     * @param degrees_per_second Movement speed
     */
    void setSpeed(float degrees_per_second);
    
    /**
     * Calibrate both servos
     * @return true if calibration successful
     */
    bool calibrate();
    
    /**
     * Add a custom scan position
     * @param position Position to add to custom scan
     */
    void addScanPosition(const PanTiltPosition& position);
    
    /**
     * Clear custom scan positions
     */
    void clearScanPositions();
    
    /**
     * Get scan positions count
     * @return Number of custom scan positions
     */
    size_t getScanPositionsCount() const { return custom_positions.size(); }
    
    /**
     * Get scanning statistics
     * @return Scanning statistics string
     */
    String getScanStatistics() const;
    
    /**
     * Get tracking statistics
     * @return Tracking statistics string
     */
    String getTrackingStatistics() const;

private:
    // Servo controllers
    std::unique_ptr<ServoController> pan_servo;
    std::unique_ptr<ServoController> tilt_servo;
    
    // System state
    bool initialized = false;
    bool power_enabled = false;
    
    // Scanning state
    bool scanning_active = false;
    ScanConfig current_scan_config;
    size_t current_scan_position = 0;
    unsigned long last_scan_move_time = 0;
    unsigned long scan_dwell_start_time = 0;
    bool scan_moving = false;
    std::vector<PanTiltPosition> custom_positions;
    
    // Tracking state
    bool tracking_active = false;
    TrackingTarget current_target;
    unsigned long last_tracking_update = 0;
    
    // Statistics
    uint32_t total_scan_cycles = 0;
    uint32_t total_tracking_time = 0;
    uint32_t successful_tracks = 0;
    uint32_t lost_tracks = 0;
    
    /**
     * Generate scan positions for current pattern
     * @return Vector of scan positions
     */
    std::vector<PanTiltPosition> generateScanPositions();
    
    /**
     * Process scanning state machine
     */
    void processScanningStateMachine();
    
    /**
     * Process tracking state machine
     */
    void processTrackingStateMachine();
    
    /**
     * Generate horizontal sweep positions
     * @return Vector of positions for horizontal sweep
     */
    std::vector<PanTiltPosition> generateHorizontalSweep();
    
    /**
     * Generate vertical sweep positions
     * @return Vector of positions for vertical sweep
     */
    std::vector<PanTiltPosition> generateVerticalSweep();
    
    /**
     * Generate grid scan positions
     * @return Vector of positions for grid scan
     */
    std::vector<PanTiltPosition> generateGridScan();
    
    /**
     * Generate spiral scan positions
     * @return Vector of positions for spiral scan
     */
    std::vector<PanTiltPosition> generateSpiralScan();
    
    /**
     * Generate random scan positions
     * @return Vector of positions for random scan
     */
    std::vector<PanTiltPosition> generateRandomScan();
    
    /**
     * Validate pan/tilt position
     * @param position Position to validate
     * @return true if position is valid
     */
    bool isValidPosition(const PanTiltPosition& position);
    
    /**
     * Calculate movement time between positions
     * @param from Source position
     * @param to Target position
     * @return Estimated movement time in milliseconds
     */
    uint32_t calculateMovementTime(const PanTiltPosition& from, const PanTiltPosition& to);
};

/**
 * Pan/Tilt utilities namespace
 */
namespace PanTiltUtils {
    /**
     * Create predefined positions
     */
    PanTiltPosition createCenterPosition();
    PanTiltPosition createHomePosition();
    PanTiltPosition createLookLeftPosition();
    PanTiltPosition createLookRightPosition();
    PanTiltPosition createLookUpPosition();
    PanTiltPosition createLookDownPosition();
    
    /**
     * Convert motion detection coordinates to pan/tilt angles
     * @param motion_x Motion X coordinate (0.0-1.0)
     * @param motion_y Motion Y coordinate (0.0-1.0)
     * @return Pan/tilt position
     */
    PanTiltPosition motionToPosition(float motion_x, float motion_y);
    
    /**
     * Calculate distance between two positions
     * @param pos1 First position
     * @param pos2 Second position
     * @return Distance in degrees
     */
    float calculateDistance(const PanTiltPosition& pos1, const PanTiltPosition& pos2);
    
    /**
     * Interpolate between two positions
     * @param from Start position
     * @param to End position
     * @param t Interpolation factor (0.0-1.0)
     * @return Interpolated position
     */
    PanTiltPosition interpolatePosition(const PanTiltPosition& from, const PanTiltPosition& to, float t);
}

#endif // PAN_TILT_MANAGER_H