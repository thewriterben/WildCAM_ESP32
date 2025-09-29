#ifndef MOTION_DETECTION_MANAGER_H
#define MOTION_DETECTION_MANAGER_H

#include "hybrid_motion_detector.h"
#include "enhanced_hybrid_motion_detector.h"
#include "../include/config.h"

/**
 * @brief Motion Detection Manager - Integration Layer
 * 
 * Provides a unified interface for both legacy and enhanced motion detection.
 * Allows seamless upgrading from existing HybridMotionDetector to 
 * EnhancedHybridMotionDetector without breaking existing code.
 */
class MotionDetectionManager {
public:
    /**
     * @brief Detection system type
     */
    enum class DetectionSystem {
        LEGACY_HYBRID,      // Original HybridMotionDetector
        ENHANCED_HYBRID     // New EnhancedHybridMotionDetector
    };

    /**
     * @brief Unified motion detection result
     */
    struct UnifiedMotionResult {
        // Common fields (compatible with both systems)
        bool motionDetected;
        float confidenceScore;
        HybridMotionDetector::ConfidenceLevel confidence;
        bool pirTriggered;
        bool frameMotionDetected;
        uint32_t processTime;
        String description;
        
        // Enhanced fields (only populated if using enhanced system)
        bool hasEnhancedData = false;
        uint8_t activeZoneCount = 0;
        float motionDirection = 0.0f;
        float motionSpeed = 0.0f;
        uint32_t dwellTime = 0;
        bool isNewObject = false;
        float mlConfidence = 0.0f;
        bool falsePositivePrediction = false;
    };

    /**
     * @brief Initialize motion detection manager
     * @param cameraManager Camera manager instance
     * @param system Detection system to use
     * @return true if initialization successful
     */
    bool initialize(CameraManager* cameraManager, DetectionSystem system = DetectionSystem::ENHANCED_HYBRID);

    /**
     * @brief Detect motion using configured system
     * @return Unified motion detection result
     */
    UnifiedMotionResult detectMotion();

    /**
     * @brief Upgrade from legacy to enhanced system
     * @return true if upgrade successful
     */
    bool upgradeToEnhanced();

    /**
     * @brief Configure enhanced features (only if using enhanced system)
     * @param enableMultiZone Enable multi-zone PIR
     * @param enableAdvancedAnalysis Enable advanced frame analysis
     * @param enableAnalytics Enable motion analytics
     * @return true if configuration successful
     */
    bool configureEnhancedFeatures(bool enableMultiZone = true, 
                                  bool enableAdvancedAnalysis = true,
                                  bool enableAnalytics = true);

    /**
     * @brief Get current detection system type
     * @return Active detection system
     */
    DetectionSystem getCurrentSystem() const { return currentSystem; }

    /**
     * @brief Check if enhanced features are available
     * @return true if enhanced system is active
     */
    bool hasEnhancedFeatures() const { return currentSystem == DetectionSystem::ENHANCED_HYBRID; }

    /**
     * @brief Get analytics summary (enhanced system only)
     * @return Analytics summary or empty string
     */
    String getAnalyticsSummary() const;

    /**
     * @brief Reset all statistics and analytics
     */
    void resetStatistics();

    /**
     * @brief Set detection sensitivity
     * @param sensitivity Detection sensitivity (0.0-1.0)
     */
    void setSensitivity(float sensitivity);

    /**
     * @brief Enable/disable low power mode
     * @param lowPower Low power mode state
     */
    void setLowPowerMode(bool lowPower);

    /**
     * @brief Cleanup motion detection resources
     */
    void cleanup();

private:
    DetectionSystem currentSystem = DetectionSystem::LEGACY_HYBRID;
    bool initialized = false;
    CameraManager* camera = nullptr;
    
    // Detection system instances
    std::unique_ptr<HybridMotionDetector> legacyDetector;
    std::unique_ptr<EnhancedHybridMotionDetector> enhancedDetector;

    /**
     * @brief Convert enhanced result to unified format
     * @param enhancedResult Enhanced detection result
     * @return Unified result
     */
    UnifiedMotionResult convertEnhancedResult(const EnhancedHybridMotionDetector::EnhancedHybridResult& enhancedResult);

    /**
     * @brief Convert legacy result to unified format
     * @param legacyResult Legacy detection result
     * @return Unified result
     */
    UnifiedMotionResult convertLegacyResult(const HybridMotionDetector::HybridResult& legacyResult);
    
    // Servo integration (optional)
    #if PAN_TILT_ENABLED
    friend class ServoIntegrationManager;
    std::unique_ptr<class ServoIntegrationManager> servo_integration;
    bool servo_integration_enabled = false;
    
    /**
     * Initialize servo integration
     * @return true if servo integration initialized successfully
     */
    bool initializeServoIntegration();
    
    /**
     * Process motion result with servo response
     * @param result Motion detection result
     */
    void processServoResponse(const UnifiedMotionResult& result);
    #endif
};

#endif // MOTION_DETECTION_MANAGER_H