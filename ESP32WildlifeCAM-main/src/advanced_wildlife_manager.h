/**
 * @file advanced_wildlife_manager.h
 * @brief Advanced Wildlife Camera Integration Manager
 * 
 * Integrates all advanced features: motion detection, servo control, audio classification,
 * and AI/ML systems into a unified wildlife monitoring system.
 */

#ifndef ADVANCED_WILDLIFE_MANAGER_H
#define ADVANCED_WILDLIFE_MANAGER_H

#include "servo/servo_integration.h"
#include "audio/wildlife_audio_classifier.h"
#include "detection/motion_detection_manager.h"
#include "../include/config.h"
#include <Arduino.h>
#include <memory>

/**
 * System operation modes
 */
enum class OperationMode {
    IDLE,                   // System idle, minimal power consumption
    MONITORING,             // Active monitoring with motion detection
    TRACKING,               // Active tracking of detected wildlife
    SCANNING,               // Automated scanning for wildlife
    MULTIMODAL_ANALYSIS     // Combined audio+visual analysis
};

/**
 * Wildlife detection event
 */
struct WildlifeDetectionEvent {
    uint32_t timestamp;
    String species;
    float confidence;
    bool has_visual_detection;
    bool has_audio_detection;
    bool has_servo_tracking;
    float pan_angle;
    float tilt_angle;
    String event_description;
    
    WildlifeDetectionEvent() 
        : timestamp(0)
        , species("unknown")
        , confidence(0.0f)
        , has_visual_detection(false)
        , has_audio_detection(false)
        , has_servo_tracking(false)
        , pan_angle(90.0f)
        , tilt_angle(90.0f)
        , event_description("") {
    }
};

/**
 * System performance metrics
 */
struct SystemMetrics {
    uint32_t total_detections;
    uint32_t visual_only_detections;
    uint32_t audio_only_detections;
    uint32_t multimodal_detections;
    uint32_t successful_tracks;
    uint32_t false_positives;
    float average_detection_confidence;
    uint32_t power_consumption_mah;
    uint32_t uptime_seconds;
    
    SystemMetrics() 
        : total_detections(0)
        , visual_only_detections(0)
        , audio_only_detections(0)
        , multimodal_detections(0)
        , successful_tracks(0)
        , false_positives(0)
        , average_detection_confidence(0.0f)
        , power_consumption_mah(0)
        , uptime_seconds(0) {
    }
};

/**
 * Advanced Wildlife Manager Class
 * Main integration point for all wildlife camera features
 */
class AdvancedWildlifeManager {
public:
    /**
     * Constructor
     */
    AdvancedWildlifeManager();
    
    /**
     * Destructor
     */
    ~AdvancedWildlifeManager();
    
    /**
     * Initialize all wildlife camera systems
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * Main update loop - call regularly from main loop
     * This is the primary integration point for the entire system
     */
    void update();
    
    /**
     * Process motion detection result
     * @param motion_result Motion detection result
     * @return true if wildlife detected and processed
     */
    bool processMotionDetection(const MotionDetectionManager::UnifiedMotionResult& motion_result);
    
    /**
     * Process audio classification result
     * @param audio_result Audio classification result
     * @return true if wildlife detected and processed
     */
    bool processAudioClassification(const AudioClassificationResult& audio_result);
    
    /**
     * Get current operation mode
     * @return Current operation mode
     */
    OperationMode getCurrentMode() const { return current_mode; }
    
    /**
     * Set operation mode
     * @param mode New operation mode
     */
    void setOperationMode(OperationMode mode);
    
    /**
     * Enable/disable specific subsystems
     */
    void setMotionDetectionEnabled(bool enable);
    void setAudioClassificationEnabled(bool enable);
    void setServoControlEnabled(bool enable);
    void setMultiModalDetectionEnabled(bool enable);
    
    /**
     * Get system status
     * @return System status string
     */
    String getSystemStatus() const;
    
    /**
     * Get performance metrics
     * @return System performance metrics
     */
    SystemMetrics getMetrics() const { return metrics; }
    
    /**
     * Get recent detection events
     * @param count Number of recent events to return
     * @return Vector of recent detection events
     */
    std::vector<WildlifeDetectionEvent> getRecentEvents(size_t count = 10) const;
    
    /**
     * Manual control functions
     */
    bool manualServoControl(float pan_angle, float tilt_angle);
    bool startManualScanning(ScanPattern pattern = ScanPattern::HORIZONTAL_SWEEP);
    bool stopAllActivities();
    bool returnToHome();
    
    /**
     * Power management
     */
    void enablePowerSavingMode(bool enable);
    void enterLowPowerMode();
    void exitLowPowerMode();
    
    /**
     * Configuration
     */
    void setDetectionSensitivity(float sensitivity);
    void setTrackingEnabled(bool enable);
    void setScanningEnabled(bool enable);
    
    /**
     * Get subsystem references for advanced usage
     */
    MotionDetectionManager* getMotionDetectionManager() { return motion_detection_manager.get(); }
    ServoIntegrationManager* getServoIntegrationManager() { return servo_integration_manager.get(); }
    WildlifeAudioClassifier* getAudioClassifier() { return audio_classifier.get(); }
    
    /**
     * Cleanup all resources
     */
    void cleanup();

private:
    // Core subsystems
    std::unique_ptr<MotionDetectionManager> motion_detection_manager;
    std::unique_ptr<ServoIntegrationManager> servo_integration_manager;
    std::unique_ptr<WildlifeAudioClassifier> audio_classifier;
    
    // System state
    bool initialized = false;
    OperationMode current_mode = OperationMode::IDLE;
    bool power_saving_mode = false;
    
    // Subsystem enable flags
    bool motion_detection_enabled = true;
    bool audio_classification_enabled = true;
    bool servo_control_enabled = true;
    bool multimodal_detection_enabled = true;
    
    // Detection event history
    std::vector<WildlifeDetectionEvent> detection_events;
    size_t max_event_history = 100;
    
    // Performance metrics
    SystemMetrics metrics;
    unsigned long system_start_time = 0;
    unsigned long last_activity_time = 0;
    
    // Integration state
    bool currently_tracking = false;
    bool currently_scanning = false;
    String last_detected_species = "";
    float last_detection_confidence = 0.0f;
    
    /**
     * Process multimodal detection (audio + visual)
     * @param motion_result Visual motion detection result
     * @param audio_result Audio classification result
     * @return Combined detection event
     */
    WildlifeDetectionEvent processMultiModalDetection(
        const MotionDetectionManager::UnifiedMotionResult& motion_result,
        const AudioClassificationResult& audio_result);
    
    /**
     * Determine best operation mode based on current conditions
     * @return Recommended operation mode
     */
    OperationMode determineOptimalMode();
    
    /**
     * Execute mode-specific logic
     */
    void executeIdleMode();
    void executeMonitoringMode();
    void executeTrackingMode();
    void executeScanningMode();
    void executeMultiModalAnalysisMode();
    
    /**
     * Update system metrics
     */
    void updateMetrics();
    
    /**
     * Log detection event
     * @param event Detection event to log
     */
    void logDetectionEvent(const WildlifeDetectionEvent& event);
    
    /**
     * Check if species correlation exists between audio and visual
     * @param visual_species Visual detection species
     * @param audio_species Audio detection species
     * @return true if species correlation found
     */
    bool checkSpeciesCorrelation(const String& visual_species, const String& audio_species);
    
    /**
     * Calculate combined confidence score
     * @param visual_confidence Visual detection confidence
     * @param audio_confidence Audio detection confidence
     * @param correlation_factor Species correlation factor
     * @return Combined confidence score
     */
    float calculateCombinedConfidence(float visual_confidence, 
                                     float audio_confidence,
                                     float correlation_factor);
    
    /**
     * Handle power management based on activity
     */
    void managePowerConsumption();
    
    /**
     * Initialize individual subsystems
     */
    bool initializeMotionDetection();
    bool initializeServoControl();
    bool initializeAudioClassification();
    
    /**
     * Log system event
     * @param event Event description
     */
    void logSystemEvent(const String& event);
};

/**
 * Global advanced wildlife manager instance (optional)
 */
extern AdvancedWildlifeManager* g_wildlife_manager;

/**
 * Convenience functions for easy integration
 */
namespace WildlifeManager {
    /**
     * Initialize global wildlife manager
     * @return true if initialization successful
     */
    bool initializeGlobal();
    
    /**
     * Update global wildlife manager (call in main loop)
     */
    void updateGlobal();
    
    /**
     * Get global system status
     * @return System status string
     */
    String getGlobalStatus();
    
    /**
     * Cleanup global wildlife manager
     */
    void cleanupGlobal();
    
    /**
     * Check if global manager is available
     * @return true if global manager is initialized
     */
    bool isGlobalAvailable();
}

#endif // ADVANCED_WILDLIFE_MANAGER_H