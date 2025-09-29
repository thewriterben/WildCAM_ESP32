#ifndef ENHANCED_HYBRID_MOTION_DETECTOR_H
#define ENHANCED_HYBRID_MOTION_DETECTOR_H

#include "hybrid_motion_detector.h"
#include "multi_zone_pir_sensor.h"
#include "advanced_motion_detection.h"
#include <vector>

/**
 * @brief Enhanced hybrid motion detector with advanced features
 * 
 * Extends the basic hybrid detector with:
 * - Multi-zone PIR sensor support
 * - Advanced frame analysis algorithms
 * - Machine learning false positive reduction
 * - Analytics and motion pattern analysis
 */
class EnhancedHybridMotionDetector : public HybridMotionDetector {
public:
    /**
     * @brief Enhanced hybrid detection result
     */
    struct EnhancedHybridResult : public HybridResult {
        // Multi-zone PIR results
        MultiZonePIRSensor::MultiZoneResult multiZoneResult;
        bool multiZoneEnabled;
        
        // Advanced motion analysis
        AdvancedMotionDetection::AdvancedMotionResult advancedResult;
        bool advancedAnalysisEnabled;
        
        // Analytics
        float motionDirection;      // Dominant direction in radians
        float motionSpeed;          // Average speed
        uint32_t dwellTime;         // Time object present
        bool isNewObject;           // First detection of object
        
        // Machine learning
        float mlConfidence;
        bool falsePositivePrediction;
        float adaptiveThreshold;
        
        // Performance metrics
        uint32_t multiZoneProcessTime;
        uint32_t advancedProcessTime;
        uint32_t totalEnhancedProcessTime;
    };

    /**
     * @brief Enhanced detection mode configuration
     */
    enum class EnhancedDetectionMode {
        LEGACY_HYBRID,          // Original hybrid mode (backwards compatible)
        MULTI_ZONE_PIR,         // Multi-zone PIR + basic frame analysis
        ADVANCED_ANALYSIS,      // Single PIR + advanced frame analysis  
        FULL_ENHANCED,          // Multi-zone PIR + advanced analysis
        ADAPTIVE_MODE           // Automatically adjust based on conditions
    };

    /**
     * @brief Analytics configuration
     */
    struct AnalyticsConfig {
        bool enableMotionHeatmap = false;      // Generate motion heatmaps
        bool enableDirectionTracking = true;   // Track movement direction
        bool enableSpeedEstimation = true;     // Estimate movement speed
        bool enableDwellTimeAnalysis = true;   // Calculate dwell times
        uint32_t heatmapUpdateInterval = 10000; // Heatmap update interval (ms)
        uint32_t trackingHistorySize = 100;    // Number of tracking points to keep
    };

    /**
     * @brief Initialize enhanced hybrid motion detector
     * @param cameraManager Camera manager instance
     * @return true if initialization successful
     */
    bool initialize(CameraManager* cameraManager) override;

    /**
     * @brief Check for motion using enhanced hybrid approach
     * @return Enhanced hybrid detection result
     */
    EnhancedHybridResult detectMotionEnhanced();

    /**
     * @brief Set enhanced detection mode
     * @param mode Detection mode to use
     */
    void setEnhancedDetectionMode(EnhancedDetectionMode mode);

    /**
     * @brief Configure analytics features
     * @param config Analytics configuration
     */
    void configureAnalytics(const AnalyticsConfig& config);

    /**
     * @brief Configure multi-zone PIR system
     * @param useDefaultZones Use default zone configuration
     * @return true if configuration successful
     */
    bool configureMultiZonePIR(bool useDefaultZones = true);

    /**
     * @brief Add custom PIR zone
     * @param zoneId Zone identifier
     * @param pin GPIO pin for PIR sensor
     * @param name Zone name
     * @param sensitivity Zone sensitivity (0.0-1.0)
     * @param priority Zone priority (0=highest)
     * @return true if zone added successfully
     */
    bool addPIRZone(uint8_t zoneId, uint8_t pin, const String& name, 
                    float sensitivity = 0.5f, uint8_t priority = 128);

    /**
     * @brief Configure advanced motion analysis
     * @param enableBackground Enable background subtraction
     * @param enableVectors Enable motion vector analysis
     * @param enableSizeFilter Enable object size filtering
     * @param enableML Enable machine learning filtering
     */
    void configureAdvancedAnalysis(bool enableBackground = true,
                                  bool enableVectors = true,
                                  bool enableSizeFilter = true, 
                                  bool enableML = true);

    /**
     * @brief Force machine learning retraining
     */
    void retrain() { advancedMotion.resetMLAdaptation(); }

    /**
     * @brief Update background model
     */
    void updateBackground() { advancedMotion.updateBackgroundModel(); }

    /**
     * @brief Get current analytics data
     * @return Analytics summary
     */
    String getAnalyticsSummary() const;

    /**
     * @brief Get motion heatmap data (if enabled)
     * @return Heatmap data as JSON string
     */
    String getMotionHeatmap() const;

    /**
     * @brief Reset all statistics and analytics
     */
    void resetAnalytics();

    /**
     * @brief Enable/disable specific enhanced features
     */
    void setMultiZoneEnabled(bool enabled) { multiZoneEnabled = enabled; }
    void setAdvancedAnalysisEnabled(bool enabled) { advancedAnalysisEnabled = enabled; }
    void setAnalyticsEnabled(bool enabled) { analyticsEnabled = enabled; }

    /**
     * @brief Get current detection mode
     * @return Current enhanced detection mode
     */
    EnhancedDetectionMode getDetectionMode() const { return detectionMode; }

    /**
     * @brief Cleanup enhanced detector resources
     */
    void cleanup() override;

private:
    // Enhanced components
    MultiZonePIRSensor multiZonePIR;
    AdvancedMotionDetection advancedMotion;
    
    // Configuration
    EnhancedDetectionMode detectionMode = EnhancedDetectionMode::LEGACY_HYBRID;
    AnalyticsConfig analyticsConfig;
    
    // Feature enable flags
    bool multiZoneEnabled = false;
    bool advancedAnalysisEnabled = false;
    bool analyticsEnabled = false;
    
    // Analytics data
    std::vector<std::pair<float, float>> motionTrajectory;  // Direction, speed pairs
    std::vector<std::pair<uint16_t, uint16_t>> heatmapData; // X, Y coordinates
    uint32_t lastHeatmapUpdate = 0;
    uint32_t totalObjectCount = 0;
    float averageDwellTime = 0.0f;

    /**
     * @brief Perform legacy hybrid detection (backwards compatible)
     * @return Enhanced result with legacy data
     */
    EnhancedHybridResult performLegacyDetection();

    /**
     * @brief Perform multi-zone PIR detection
     * @return Enhanced result with multi-zone data
     */
    EnhancedHybridResult performMultiZoneDetection();

    /**
     * @brief Perform advanced frame analysis
     * @return Enhanced result with advanced analysis data
     */
    EnhancedHybridResult performAdvancedAnalysis();

    /**
     * @brief Perform full enhanced detection
     * @return Complete enhanced result
     */
    EnhancedHybridResult performFullEnhancedDetection();

    /**
     * @brief Adaptive mode selection based on conditions
     * @return Automatically selected detection mode
     */
    EnhancedDetectionMode selectAdaptiveMode();

    /**
     * @brief Update motion analytics
     * @param result Detection result to analyze
     */
    void updateAnalytics(const EnhancedHybridResult& result);

    /**
     * @brief Update motion heatmap
     * @param x X coordinate of motion
     * @param y Y coordinate of motion
     */
    void updateHeatmap(uint16_t x, uint16_t y);

    /**
     * @brief Calculate combined confidence from all sources
     * @param pirResult Multi-zone PIR result
     * @param frameResult Advanced frame analysis result
     * @return Combined confidence score
     */
    float calculateEnhancedConfidence(const MultiZonePIRSensor::MultiZoneResult& pirResult,
                                     const AdvancedMotionDetection::AdvancedMotionResult& frameResult);

    /**
     * @brief Generate enhanced result description
     * @param result Enhanced detection result
     * @return Human-readable description
     */
    String generateEnhancedDescription(const EnhancedHybridResult& result);

    /**
     * @brief Update enhanced statistics
     * @param result Enhanced detection result
     */
    void updateEnhancedStatistics(const EnhancedHybridResult& result);
};

#endif // ENHANCED_HYBRID_MOTION_DETECTOR_H