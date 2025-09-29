/**
 * @file servo_integration.h
 * @brief Integration Layer for Servo System with Existing Motion Detection
 * 
 * Provides seamless integration between the new servo control system and
 * the existing motion detection framework with minimal code changes.
 */

#ifndef SERVO_INTEGRATION_H
#define SERVO_INTEGRATION_H

#include "pan_tilt_manager.h"
#include "tracking_algorithms.h"
#include "../detection/motion_detection_manager.h"
#include "../../include/config.h"
#include <Arduino.h>

/**
 * Servo system integration configuration
 */
struct ServoIntegrationConfig {
    bool servo_enabled;             // Enable servo system
    bool auto_tracking_enabled;     // Enable automatic tracking
    bool auto_scanning_enabled;     // Enable automatic scanning when idle
    float tracking_threshold;       // Minimum confidence to start tracking
    uint32_t idle_scan_delay;       // Delay before starting scan when idle (ms)
    ScanPattern default_scan_pattern; // Default scanning pattern
    
    ServoIntegrationConfig() 
        : servo_enabled(PAN_TILT_ENABLED)
        , auto_tracking_enabled(WILDLIFE_TRACKING_ENABLED)
        , auto_scanning_enabled(AUTO_SCANNING_ENABLED)
        , tracking_threshold(TRACKING_MIN_CONFIDENCE)
        , idle_scan_delay(SERVO_SLEEP_MODE_DELAY)
        , default_scan_pattern(static_cast<ScanPattern>(DEFAULT_SCAN_PATTERN)) {
    }
};

/**
 * Servo Integration Manager Class
 * Coordinates servo system with existing motion detection and AI systems
 */
class ServoIntegrationManager {
public:
    /**
     * Constructor
     */
    ServoIntegrationManager();
    
    /**
     * Destructor
     */
    ~ServoIntegrationManager();
    
    /**
     * Initialize servo integration system
     * @param motion_detector Existing motion detection manager
     * @param config Integration configuration
     * @return true if initialization successful
     */
    bool initialize(MotionDetectionManager* motion_detector, 
                   const ServoIntegrationConfig& config = ServoIntegrationConfig());
    
    /**
     * Update servo integration (call regularly in main loop)
     * This is the main integration point - call after motion detection
     */
    void update();
    
    /**
     * Process motion detection result with servo response
     * @param motion_result Motion detection result from existing system
     * @return true if servo system responded to motion
     */
    bool processMotionResult(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Enable/disable servo system
     * @param enable Enable state
     */
    void setServoEnabled(bool enable);
    
    /**
     * Check if servo system is enabled
     * @return true if servo system is enabled
     */
    bool isServoEnabled() const { return config.servo_enabled; }
    
    /**
     * Enable/disable automatic tracking
     * @param enable Enable state
     */
    void setAutoTrackingEnabled(bool enable);
    
    /**
     * Enable/disable automatic scanning
     * @param enable Enable state
     */
    void setAutoScanningEnabled(bool enable);
    
    /**
     * Manual pan/tilt control (for testing or manual operation)
     * @param pan_angle Pan angle in degrees
     * @param tilt_angle Tilt angle in degrees
     * @return true if movement initiated
     */
    bool manualControl(float pan_angle, float tilt_angle);
    
    /**
     * Return to home position
     * @return true if movement initiated
     */
    bool returnHome();
    
    /**
     * Start specific scan pattern
     * @param pattern Scan pattern to start
     * @return true if scan started
     */
    bool startScanPattern(ScanPattern pattern);
    
    /**
     * Stop all servo activities
     */
    void stopAllActivities();
    
    /**
     * Get current servo status
     * @return Status string
     */
    String getServoStatus() const;
    
    /**
     * Get tracking statistics
     * @return Tracking statistics string
     */
    String getTrackingStatistics() const;
    
    /**
     * Enable/disable power saving mode
     * @param enable Enable power saving
     */
    void setPowerSavingMode(bool enable);
    
    /**
     * Get pan/tilt manager instance (for advanced usage)
     * @return Pan/tilt manager pointer
     */
    PanTiltManager* getPanTiltManager() { return pan_tilt_manager.get(); }
    
    /**
     * Get wildlife tracker instance (for advanced usage)
     * @return Wildlife tracker pointer
     */
    WildlifeTracker* getWildlifeTracker() { return wildlife_tracker.get(); }
    
    /**
     * Check if system is currently active (moving, tracking, or scanning)
     * @return true if any servo activity is active
     */
    bool isActive() const;
    
    /**
     * Cleanup servo integration resources
     */
    void cleanup();

private:
    // Configuration
    ServoIntegrationConfig config;
    
    // System components
    std::unique_ptr<PanTiltManager> pan_tilt_manager;
    std::unique_ptr<WildlifeTracker> wildlife_tracker;
    MotionDetectionManager* motion_detector = nullptr;
    
    // State tracking
    bool initialized = false;
    bool power_saving_mode = false;
    unsigned long last_activity_time = 0;
    unsigned long last_motion_time = 0;
    
    // Activity state
    bool currently_tracking = false;
    bool currently_scanning = false;
    unsigned long idle_start_time = 0;
    
    /**
     * Check if system should enter idle mode
     * @return true if system should start scanning or enter power save
     */
    bool shouldEnterIdleMode();
    
    /**
     * Start idle mode activities (scanning)
     */
    void startIdleActivities();
    
    /**
     * Update activity timestamps
     */
    void updateActivityTime();
    
    /**
     * Log integration events
     * @param event Event description
     */
    void logEvent(const String& event);
};

/**
 * Global servo integration instance (optional singleton pattern)
 * Can be used for easier integration with existing code
 */
extern ServoIntegrationManager* g_servo_integration;

/**
 * Convenience functions for easy integration with existing motion detection code
 */
namespace ServoIntegration {
    /**
     * Initialize global servo integration
     * @param motion_detector Motion detection manager
     * @return true if initialization successful
     */
    bool initializeGlobal(MotionDetectionManager* motion_detector);
    
    /**
     * Process motion result with global integration
     * @param motion_result Motion detection result
     * @return true if processed successfully
     */
    bool processMotion(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Update global servo integration (call in main loop)
     */
    void updateGlobal();
    
    /**
     * Cleanup global servo integration
     */
    void cleanupGlobal();
    
    /**
     * Check if global integration is available
     * @return true if global integration is initialized
     */
    bool isGlobalAvailable();
    
    /**
     * Get global integration status
     * @return Status string
     */
    String getGlobalStatus();
}

#endif // SERVO_INTEGRATION_H