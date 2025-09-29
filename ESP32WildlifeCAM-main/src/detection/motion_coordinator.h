/**
 * @file motion_coordinator.h
 * @brief Enhanced Motion Detection Coordinator
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#ifndef MOTION_COORDINATOR_H
#define MOTION_COORDINATOR_H

#include "motion_detection_manager.h"
#include "adaptive_processor.h"
#include "wildlife_analyzer.h"
#include "config_manager.h"
#include "../camera/camera_manager.h"
#include <functional>
#include <memory>

/**
 * @brief Enhanced Motion Detection Coordinator
 * 
 * Central coordinator that intelligently combines AI, Advanced, and PIR detection methods
 * with performance optimization and wildlife pattern recognition.
 */
class MotionCoordinator {
public:
    /**
     * @brief Detection method priorities
     */
    enum class DetectionMethod {
        PIR_ONLY,           // PIR sensor only
        FRAME_ONLY,         // Frame analysis only  
        AI_ONLY,            // AI detection only
        HYBRID_BASIC,       // PIR + Frame analysis
        HYBRID_AI,          // PIR + AI
        FULL_FUSION,        // All methods combined
        ADAPTIVE            // Automatically selects best method
    };

    /**
     * @brief Environmental conditions for adaptive detection
     */
    struct EnvironmentalConditions {
        float batteryVoltage = 3.7f;
        float temperature = 20.0f;
        float lightLevel = 0.5f;
        float windSpeed = 0.0f;
        float humidity = 50.0f;
        uint8_t currentHour = 12;
        bool isNight = false;
        bool isWeatherActive = false;
    };

    /**
     * @brief Comprehensive motion detection result
     */
    struct CoordinatorResult {
        // Basic detection results
        bool motionDetected = false;
        float overallConfidence = 0.0f;
        uint32_t processingTime = 0;
        
        // Method-specific results
        bool pirTriggered = false;
        bool frameMotionDetected = false;
        bool aiDetectionTriggered = false;
        
        // Enhanced analysis
        WildlifeAnalyzer::WildlifeAnalysisResult wildlifeAnalysis;
        AdaptiveProcessor::ProcessingDecision processingDecision;
        
        // Fusion information
        DetectionMethod methodUsed;
        std::vector<String> activeMethods;
        float fusionConfidence = 0.0f;
        
        // Decision outcomes
        bool shouldCapture = false;
        bool shouldSave = false;
        bool shouldTransmit = false;
        bool shouldAlert = false;
        
        // Detailed information
        String description;
        String reasoning;
        uint32_t timestamp = 0;
    };

    /**
     * @brief Configuration for motion coordination
     */
    struct CoordinatorConfig {
        bool enabled = true;
        DetectionMethod defaultMethod = DetectionMethod::ADAPTIVE;
        
        // Fusion weights
        float pirWeight = 0.3f;
        float frameWeight = 0.4f;
        float aiWeight = 0.3f;
        
        // Thresholds
        float motionConfidenceThreshold = 0.6f;
        float captureThreshold = 0.7f;
        float transmitThreshold = 0.8f;
        float alertThreshold = 0.85f;
        
        // Environmental adaptation
        bool useEnvironmentalAdaptation = true;
        bool useTimeOfDayAdaptation = true;
        bool useWeatherCompensation = true;
        
        // Performance settings
        uint32_t maxProcessingTimeMs = 500;
        bool enablePerformanceOptimization = true;
        bool enableWildlifeAnalysis = true;
        
        // Error handling
        uint32_t maxConsecutiveFailures = 5;
        uint32_t recoveryDelayMs = 1000;
        bool enableFallbackMethods = true;
    };

    /**
     * @brief Statistics and performance metrics
     */
    struct CoordinatorStats {
        uint32_t totalDetections = 0;
        uint32_t pirDetections = 0;
        uint32_t frameDetections = 0;
        uint32_t aiDetections = 0;
        uint32_t wildlifeDetections = 0;
        uint32_t captures = 0;
        uint32_t transmissions = 0;
        uint32_t falsePositives = 0;
        uint32_t averageProcessingTime = 0;
        uint32_t failureCount = 0;
        uint32_t lastResetTime = 0;
        float overallAccuracy = 0.0f;
        float powerSavings = 0.0f;
    };

    /**
     * @brief Motion event callback function type
     */
    typedef std::function<void(const CoordinatorResult&)> MotionCallback;

    /**
     * Constructor
     */
    MotionCoordinator();

    /**
     * Destructor
     */
    ~MotionCoordinator();

    /**
     * @brief Initialize motion coordinator
     * @param cameraManager Camera manager instance
     * @param config Coordinator configuration
     * @return true if initialization successful
     */
    bool initialize(CameraManager* cameraManager, const CoordinatorConfig& config = CoordinatorConfig());

    /**
     * @brief Perform comprehensive motion detection
     * @param frameBuffer Camera frame buffer (optional)
     * @param conditions Current environmental conditions
     * @return Comprehensive motion detection result
     */
    CoordinatorResult detectMotion(const camera_fb_t* frameBuffer = nullptr, 
                                  const EnvironmentalConditions& conditions = EnvironmentalConditions());

    /**
     * @brief Update environmental conditions for adaptive behavior
     * @param conditions New environmental conditions
     */
    void updateEnvironmentalConditions(const EnvironmentalConditions& conditions);

    /**
     * @brief Set motion detection callback
     * @param callback Function to call when motion is detected
     */
    void setMotionCallback(MotionCallback callback) { motionCallback_ = callback; }

    /**
     * @brief Configure detection method
     * @param method Detection method to use
     */
    void setDetectionMethod(DetectionMethod method) { config_.defaultMethod = method; }

    /**
     * @brief Get current detection method
     * @return Currently active detection method
     */
    DetectionMethod getCurrentMethod() const { return currentMethod_; }

    /**
     * @brief Configure coordinator settings
     * @param config New configuration
     */
    void configure(const CoordinatorConfig& config);

    /**
     * @brief Get performance statistics
     * @return Current performance stats
     */
    CoordinatorStats getStatistics() const { return stats_; }

    /**
     * @brief Reset statistics and counters
     */
    void resetStatistics();

    /**
     * @brief Get configuration as JSON string
     * @return JSON configuration
     */
    String getConfigJSON() const;

    /**
     * @brief Load configuration from JSON
     * @param jsonConfig JSON configuration string
     * @return true if loaded successfully
     */
    bool loadConfigJSON(const String& jsonConfig);

    /**
     * @brief Get detailed status report
     * @return Status report as JSON string
     */
    String getStatusReport() const;

    /**
     * @brief Enable/disable wildlife analysis
     * @param enabled True to enable wildlife analysis
     */
    void setWildlifeAnalysisEnabled(bool enabled);

    /**
     * @brief Enable/disable performance optimization
     * @param enabled True to enable performance optimization
     */
    void setPerformanceOptimizationEnabled(bool enabled);

    /**
     * @brief Manually train wildlife analyzer
     * @param pattern Confirmed pattern type
     * @param characteristics Movement characteristics
     * @param confidence Confidence in classification
     */
    void trainWildlifeAnalyzer(WildlifeAnalyzer::MovementPattern pattern,
                              const WildlifeAnalyzer::MovementCharacteristics& characteristics,
                              float confidence = 1.0f);

private:
    // Configuration and state
    CoordinatorConfig config_;
    DetectionMethod currentMethod_;
    EnvironmentalConditions lastConditions_;
    bool initialized_;
    
    // Core components
    std::unique_ptr<MotionDetectionManager> motionManager_;
    std::unique_ptr<AdaptiveProcessor> adaptiveProcessor_;
    std::unique_ptr<WildlifeAnalyzer> wildlifeAnalyzer_;
    std::shared_ptr<ConfigManager> configManager_;
    
    // Callback
    MotionCallback motionCallback_;
    
    // Statistics and performance
    CoordinatorStats stats_;
    std::vector<uint32_t> processingTimes_;
    uint32_t consecutiveFailures_;
    uint32_t lastDetectionTime_;
    
    // Helper methods
    DetectionMethod determineOptimalMethod(const EnvironmentalConditions& conditions,
                                         const AdaptiveProcessor::ProcessingDecision& decision);
    
    CoordinatorResult fuseDetectionResults(const MotionDetectionManager::UnifiedMotionResult& baseResult,
                                         const WildlifeAnalyzer::WildlifeAnalysisResult& wildlifeResult,
                                         const AdaptiveProcessor::ProcessingDecision& processingDecision,
                                         const EnvironmentalConditions& conditions);
    
    float calculateFusionConfidence(const MotionDetectionManager::UnifiedMotionResult& baseResult,
                                   const WildlifeAnalyzer::WildlifeAnalysisResult& wildlifeResult);
    
    void updateStatistics(const CoordinatorResult& result);
    void handleDetectionFailure();
    void handleDetectionSuccess();
    bool shouldUseMethod(DetectionMethod method, const EnvironmentalConditions& conditions);
    String getMethodString(DetectionMethod method) const;
    void logDetectionEvent(const CoordinatorResult& result);
    void updateWildlifeAnalysis(const MotionDetectionManager::UnifiedMotionResult& result);
    
    // Environmental adaptation helpers
    float getEnvironmentalConfidenceAdjustment(const EnvironmentalConditions& conditions);
    float getTimeOfDayAdjustment(uint8_t hour);
    float getWeatherAdjustment(const EnvironmentalConditions& conditions);
    
    // Decision making
    bool shouldCapture(const CoordinatorResult& result);
    bool shouldSave(const CoordinatorResult& result);  
    bool shouldTransmit(const CoordinatorResult& result);
    bool shouldAlert(const CoordinatorResult& result);
};

#endif // MOTION_COORDINATOR_H