/**
 * @file adaptive_processor.h
 * @brief Adaptive Processing Manager for Motion Detection Performance Optimization
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#ifndef ADAPTIVE_PROCESSOR_H
#define ADAPTIVE_PROCESSOR_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include "memory_pool_manager.h"

/**
 * @brief Adaptive Processing Manager for motion detection performance optimization
 * 
 * Manages processing intensity based on activity levels, environmental conditions,
 * and system resources to optimize battery life and detection accuracy.
 */
class AdaptiveProcessor {
public:
    /**
     * @brief Processing intensity levels
     */
    enum class ProcessingLevel {
        MINIMAL,        // Basic PIR only, ~50ms processing
        REDUCED,        // PIR + basic frame analysis, ~150ms processing  
        NORMAL,         // Standard hybrid detection, ~300ms processing
        ENHANCED,       // Full AI + analytics, ~500ms processing
        MAXIMUM         // All features + high resolution, ~800ms processing
    };

    /**
     * @brief Activity level indicators
     */
    enum class ActivityLevel {
        DORMANT,        // No activity for >30 minutes
        LOW,            // Minimal activity, <1 detection/hour
        MODERATE,       // Regular activity, 1-5 detections/hour
        HIGH,           // Frequent activity, 5-15 detections/hour
        PEAK            // Continuous activity, >15 detections/hour
    };

    /**
     * @brief Configuration for adaptive processing
     */
    struct AdaptiveConfig {
        bool enabled = true;
        uint32_t activityWindowMinutes = 60;        // Activity assessment window
        uint32_t dormantThresholdMinutes = 30;      // Time before entering dormant mode
        float batteryLowThreshold = 3.2f;           // Voltage for power saving mode
        float temperatureOptimalMin = 10.0f;        // Optimal temp range
        float temperatureOptimalMax = 30.0f;
        bool useEnvironmentalAdaptation = true;
        bool useTimeOfDayAdaptation = true;
        uint32_t maxProcessingTimeMs = 500;         // Hard limit for processing time
    };

    /**
     * @brief Region of Interest (ROI) configuration
     */
    struct ROIConfig {
        bool enabled = false;
        uint16_t x = 0;          // ROI top-left X coordinate
        uint16_t y = 0;          // ROI top-left Y coordinate  
        uint16_t width = 320;    // ROI width
        uint16_t height = 240;   // ROI height
        float confidence = 0.0f; // Confidence in ROI placement
        uint32_t lastUpdate = 0; // Last ROI update timestamp
    };

    /**
     * @brief Frame rate adaptive configuration
     */
    struct FrameRateConfig {
        uint32_t normalIntervalMs = 1000;      // Normal processing interval
        uint32_t lowActivityIntervalMs = 5000; // Low activity interval
        uint32_t dormantIntervalMs = 30000;    // Dormant mode interval
        uint32_t maxFrameSkips = 10;           // Max consecutive frame skips
        float qualityReductionFactor = 0.8f;   // Quality reduction during low activity
    };

    /**
     * @brief Performance metrics and statistics
     */
    struct PerformanceMetrics {
        uint32_t totalProcessingTime = 0;
        uint32_t averageProcessingTime = 0;
        uint32_t peakProcessingTime = 0;
        uint32_t frameSkips = 0;
        uint32_t roiOptimizations = 0;
        uint32_t levelChanges = 0;
        float powerSavings = 0.0f;
        uint32_t lastResetTime = 0;
    };

    /**
     * @brief Processing decision result
     */
    struct ProcessingDecision {
        ProcessingLevel level;
        bool useROI;
        bool skipFrame;
        uint32_t maxProcessingTime;
        float qualityFactor;
        String reasoning;
    };

    /**
     * Constructor
     */
    AdaptiveProcessor();

    /**
     * Destructor
     */
    ~AdaptiveProcessor();

    /**
     * @brief Initialize adaptive processor
     * @param config Adaptive processing configuration
     * @return true if initialization successful
     */
    bool initialize(const AdaptiveConfig& config = AdaptiveConfig());

    /**
     * @brief Update activity level based on detection events
     * @param detectionOccurred True if motion was detected
     * @param confidence Detection confidence (0.0-1.0)
     * @param processingTime Time taken for last detection (ms)
     */
    void updateActivity(bool detectionOccurred, float confidence, uint32_t processingTime);

    /**
     * @brief Get processing decision for current conditions
     * @param batteryVoltage Current battery voltage
     * @param temperature Current temperature (°C)
     * @param lightLevel Current light level (0.0-1.0)
     * @return Processing decision with level and parameters
     */
    ProcessingDecision getProcessingDecision(float batteryVoltage = 3.7f, 
                                           float temperature = 20.0f, 
                                           float lightLevel = 0.5f);

    /**
     * @brief Update Region of Interest based on recent detections
     * @param detectionX X coordinate of detection center
     * @param detectionY Y coordinate of detection center
     * @param detectionWidth Width of detected object
     * @param detectionHeight Height of detected object
     * @param confidence Detection confidence
     */
    void updateROI(uint16_t detectionX, uint16_t detectionY, 
                   uint16_t detectionWidth, uint16_t detectionHeight, 
                   float confidence);

    /**
     * @brief Get current ROI configuration
     * @return Current ROI settings
     */
    ROIConfig getCurrentROI() const { return roiConfig_; }

    /**
     * @brief Get current activity level
     * @return Current activity assessment
     */
    ActivityLevel getCurrentActivityLevel() const { return currentActivity_; }

    /**
     * @brief Get current processing level
     * @return Current processing intensity
     */
    ProcessingLevel getCurrentProcessingLevel() const { return currentLevel_; }

    /**
     * @brief Get performance metrics
     * @return Current performance statistics
     */
    PerformanceMetrics getPerformanceMetrics() const { return metrics_; }

    /**
     * @brief Reset performance metrics
     */
    void resetMetrics();

    /**
     * @brief Configure adaptive settings
     * @param config New configuration
     */
    void configure(const AdaptiveConfig& config) { config_ = config; }

    /**
     * @brief Get configuration as JSON string
     * @return JSON configuration string
     */
    String getConfigJSON() const;

    /**
     * @brief Load configuration from JSON string
     * @param jsonConfig JSON configuration string
     * @return true if loaded successfully
     */
    bool loadConfigJSON(const String& jsonConfig);

    /**
     * @brief Get memory pool manager instance
     * @return Shared pointer to memory manager
     */
    std::shared_ptr<MemoryPoolManager> getMemoryManager() const { return memoryManager_; }

private:
    // Configuration
    AdaptiveConfig config_;
    ROIConfig roiConfig_;
    FrameRateConfig frameRateConfig_;
    
    // Current state
    ProcessingLevel currentLevel_;
    ActivityLevel currentActivity_;
    bool initialized_;
    
    // Activity tracking
    std::vector<uint32_t> detectionTimes_;
    std::vector<float> confidenceHistory_;
    std::vector<uint32_t> processingTimes_;
    uint32_t lastDetectionTime_;
    uint32_t lastProcessingTime_;
    
    // Performance metrics
    PerformanceMetrics metrics_;
    
    // Memory management
    std::shared_ptr<MemoryPoolManager> memoryManager_;
    
    // ROI tracking
    std::vector<uint16_t> detectionX_;
    std::vector<uint16_t> detectionY_;
    uint32_t roiUpdateCount_;
    
    // Helper methods
    ActivityLevel assessActivityLevel();
    ProcessingLevel determineOptimalLevel(float batteryVoltage, float temperature, float lightLevel);
    void updateMetrics(uint32_t processingTime);
    void cleanupOldData();
    float calculatePowerSavings();
    bool shouldSkipFrame();
    void updateROIInternal();
    String getActivityLevelString(ActivityLevel level) const;
    String getProcessingLevelString(ProcessingLevel level) const;
};

#endif // ADAPTIVE_PROCESSOR_H