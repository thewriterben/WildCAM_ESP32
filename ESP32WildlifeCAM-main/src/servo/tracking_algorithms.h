/**
 * @file tracking_algorithms.h
 * @brief Wildlife Tracking Algorithms for Pan/Tilt Camera System
 * 
 * Implements intelligent tracking algorithms to follow wildlife movements
 * using motion detection input and predictive tracking capabilities.
 */

#ifndef TRACKING_ALGORITHMS_H
#define TRACKING_ALGORITHMS_H

#include "pan_tilt_manager.h"
#include "../detection/motion_detection_manager.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <vector>

/**
 * Motion detection region structure
 */
struct MotionRegion {
    float x, y;                 // Center coordinates (0.0-1.0)
    float width, height;        // Size (0.0-1.0)
    float confidence;           // Detection confidence (0.0-1.0)
    uint32_t timestamp;         // When this region was detected
    float motion_velocity_x;    // Estimated X velocity
    float motion_velocity_y;    // Estimated Y velocity
    
    MotionRegion() 
        : x(0.5f), y(0.5f), width(0.1f), height(0.1f)
        , confidence(0.0f), timestamp(0)
        , motion_velocity_x(0.0f), motion_velocity_y(0.0f) {
    }
};

/**
 * Tracking state information
 */
enum class TrackingState {
    IDLE,               // Not tracking
    SEARCHING,          // Searching for initial target
    LOCKED,             // Locked onto target
    PREDICTING,         // Predicting target movement
    LOST                // Target lost, attempting recovery
};

/**
 * Tracking performance metrics
 */
struct TrackingMetrics {
    uint32_t total_tracks;          // Total number of tracks initiated
    uint32_t successful_tracks;     // Tracks that completed successfully
    uint32_t lost_tracks;           // Tracks that were lost
    uint32_t average_track_duration;// Average tracking duration (ms)
    float average_accuracy;         // Average tracking accuracy
    uint32_t false_positives;       // False positive detections
    
    TrackingMetrics() 
        : total_tracks(0), successful_tracks(0), lost_tracks(0)
        , average_track_duration(0), average_accuracy(0.0f)
        , false_positives(0) {
    }
};

/**
 * Wildlife Tracking Algorithm Class
 */
class WildlifeTracker {
public:
    /**
     * Constructor
     * @param pan_tilt_manager Pan/tilt manager instance
     * @param motion_detector Motion detection manager instance
     */
    WildlifeTracker(PanTiltManager* pan_tilt_manager, MotionDetectionManager* motion_detector);
    
    /**
     * Destructor
     */
    ~WildlifeTracker();
    
    /**
     * Initialize tracking system
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * Update tracking system (call regularly in main loop)
     */
    void update();
    
    /**
     * Start tracking based on motion detection
     * @param motion_result Motion detection result
     * @return true if tracking started successfully
     */
    bool startTracking(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Stop current tracking
     */
    void stopTracking();
    
    /**
     * Update tracking with new motion detection data
     * @param motion_result Latest motion detection result
     * @return true if tracking updated successfully
     */
    bool updateTracking(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Get current tracking state
     * @return Current tracking state
     */
    TrackingState getCurrentState() const { return current_state; }
    
    /**
     * Check if currently tracking
     * @return true if actively tracking
     */
    bool isTracking() const { return current_state != TrackingState::IDLE; }
    
    /**
     * Get tracking confidence
     * @return Current tracking confidence (0.0-1.0)
     */
    float getTrackingConfidence() const { return current_confidence; }
    
    /**
     * Get tracking duration
     * @return Current tracking duration in milliseconds
     */
    uint32_t getTrackingDuration() const;
    
    /**
     * Set tracking sensitivity
     * @param sensitivity Tracking sensitivity (0.0-1.0)
     */
    void setTrackingSensitivity(float sensitivity);
    
    /**
     * Set prediction parameters
     * @param prediction_time_ms Time to predict ahead (ms)
     * @param max_velocity Maximum expected velocity
     */
    void setPredictionParameters(uint32_t prediction_time_ms, float max_velocity);
    
    /**
     * Enable/disable predictive tracking
     * @param enable Enable predictive tracking
     */
    void setPredictiveTracking(bool enable);
    
    /**
     * Get tracking metrics
     * @return Tracking performance metrics
     */
    TrackingMetrics getMetrics() const { return metrics; }
    
    /**
     * Reset tracking statistics
     */
    void resetMetrics();
    
    /**
     * Get tracking status string
     * @return Human-readable tracking status
     */
    String getStatusString() const;

private:
    // System references
    PanTiltManager* pan_tilt;
    MotionDetectionManager* motion_detector;
    
    // Tracking state
    TrackingState current_state = TrackingState::IDLE;
    float current_confidence = 0.0f;
    uint32_t tracking_start_time = 0;
    uint32_t last_update_time = 0;
    uint32_t lost_count = 0;
    
    // Target information
    MotionRegion current_target;
    std::vector<MotionRegion> target_history;
    size_t max_history_size = 10;
    
    // Tracking parameters
    float tracking_sensitivity = 0.7f;
    uint32_t prediction_time_ms = 1000;
    float max_velocity = 0.5f;          // Screen units per second
    bool predictive_tracking_enabled = true;
    uint32_t loss_timeout_ms = 5000;
    uint32_t search_timeout_ms = 10000;
    
    // Performance metrics
    TrackingMetrics metrics;
    
    /**
     * Convert motion coordinates to pan/tilt position
     * @param motion_x Motion X coordinate (0.0-1.0)
     * @param motion_y Motion Y coordinate (0.0-1.0)
     * @return Pan/tilt position
     */
    PanTiltPosition motionToPanTilt(float motion_x, float motion_y);
    
    /**
     * Extract motion regions from detection result
     * @param motion_result Motion detection result
     * @return Vector of detected motion regions
     */
    std::vector<MotionRegion> extractMotionRegions(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Find best matching region for current target
     * @param regions Available motion regions
     * @return Index of best matching region, or -1 if none found
     */
    int findBestMatchingRegion(const std::vector<MotionRegion>& regions);
    
    /**
     * Calculate region similarity
     * @param region1 First region
     * @param region2 Second region
     * @return Similarity score (0.0-1.0)
     */
    float calculateRegionSimilarity(const MotionRegion& region1, const MotionRegion& region2);
    
    /**
     * Predict target position based on history
     * @param prediction_time Time to predict ahead (ms)
     * @return Predicted motion region
     */
    MotionRegion predictTargetPosition(uint32_t prediction_time);
    
    /**
     * Calculate target velocity from history
     * @return Velocity components (x, y) in screen units per second
     */
    std::pair<float, float> calculateTargetVelocity();
    
    /**
     * Update target history
     * @param region New target region to add
     */
    void updateTargetHistory(const MotionRegion& region);
    
    /**
     * Check if target is lost
     * @return true if target should be considered lost
     */
    bool isTargetLost();
    
    /**
     * Process idle state
     */
    void processIdleState();
    
    /**
     * Process searching state
     */
    void processSearchingState();
    
    /**
     * Process locked state
     */
    void processLockedState();
    
    /**
     * Process predicting state
     */
    void processPredictingState();
    
    /**
     * Process lost state
     */
    void processLostState();
    
    /**
     * Update tracking confidence based on recent performance
     */
    void updateTrackingConfidence();
    
    /**
     * Log tracking event
     * @param event Event description
     */
    void logTrackingEvent(const String& event);
};

/**
 * Tracking algorithm utilities
 */
namespace TrackingUtils {
    /**
     * Calculate distance between two points
     * @param x1, y1 First point coordinates
     * @param x2, y2 Second point coordinates
     * @return Distance in screen units
     */
    float calculateDistance(float x1, float y1, float x2, float y2);
    
    /**
     * Calculate overlap between two regions
     * @param region1 First region
     * @param region2 Second region
     * @return Overlap ratio (0.0-1.0)
     */
    float calculateOverlap(const MotionRegion& region1, const MotionRegion& region2);
    
    /**
     * Smooth position using exponential moving average
     * @param current_pos Current position
     * @param new_pos New position
     * @param smoothing_factor Smoothing factor (0.0-1.0)
     * @return Smoothed position
     */
    MotionRegion smoothPosition(const MotionRegion& current_pos, const MotionRegion& new_pos, float smoothing_factor);
    
    /**
     * Convert screen coordinates to camera field of view angles
     * @param screen_x Screen X coordinate (0.0-1.0)
     * @param screen_y Screen Y coordinate (0.0-1.0)
     * @param fov_horizontal Horizontal field of view (degrees)
     * @param fov_vertical Vertical field of view (degrees)
     * @return Pan/tilt angles relative to center
     */
    std::pair<float, float> screenToFOV(float screen_x, float screen_y, float fov_horizontal, float fov_vertical);
    
    /**
     * Validate motion region parameters
     * @param region Region to validate
     * @return true if region is valid
     */
    bool isValidRegion(const MotionRegion& region);
}

#endif // TRACKING_ALGORITHMS_H