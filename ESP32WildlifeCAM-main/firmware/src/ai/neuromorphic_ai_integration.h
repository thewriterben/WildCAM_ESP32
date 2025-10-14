/**
 * @file neuromorphic_ai_integration.h
 * @brief Integration layer between neuromorphic computing and existing AI system
 * 
 * Provides seamless integration of neuromorphic computing capabilities
 * with the existing AIWildlifeSystem for ultra-low-power operation.
 */

#ifndef NEUROMORPHIC_AI_INTEGRATION_H
#define NEUROMORPHIC_AI_INTEGRATION_H

#include "ai_common.h"
#include "neuromorphic/neuromorphic_system.h"
#include <memory>

/**
 * Neuromorphic Integration Configuration
 */
struct NeuromorphicIntegrationConfig {
    bool enabled;                      // Enable neuromorphic processing
    bool auto_switching;               // Automatically switch based on power/performance
    float power_threshold_mw;          // Switch to neuromorphic below this power level
    float battery_threshold_percent;   // Switch to neuromorphic below this battery level
    bool hybrid_mode;                  // Use both traditional and neuromorphic
    bool prefer_neuromorphic;          // Prefer neuromorphic when available
    
    NeuromorphicIntegrationConfig()
        : enabled(false)
        , auto_switching(true)
        , power_threshold_mw(50.0f)
        , battery_threshold_percent(20.0f)
        , hybrid_mode(false)
        , prefer_neuromorphic(false) {}
};

/**
 * Neuromorphic AI Integration Manager
 * 
 * Manages integration between neuromorphic computing and traditional AI,
 * providing automatic switching based on power constraints and performance.
 */
class NeuromorphicAIIntegration {
public:
    NeuromorphicAIIntegration();
    ~NeuromorphicAIIntegration();
    
    /**
     * Initialize neuromorphic integration
     */
    bool init(const NeuromorphicIntegrationConfig& config);
    
    /**
     * Check if neuromorphic processing is available
     */
    bool isAvailable() const { return initialized_ && neuromorphic_system_ != nullptr; }
    
    /**
     * Check if currently using neuromorphic processing
     */
    bool isUsingNeuromorphic() const { return use_neuromorphic_; }
    
    /**
     * Process wildlife detection with automatic mode selection
     * @param frame Camera frame
     * @param battery_level Current battery level (0-100%)
     * @return AI result
     */
    AIResult processWildlifeDetection(const CameraFrame& frame, float battery_level);
    
    /**
     * Convert neuromorphic result to standard AIResult
     */
    AIResult convertNeuromorphicToAIResult(const NeuromorphicWildlifeResult& neuro_result);
    
    /**
     * Enable/disable neuromorphic processing
     */
    void enableNeuromorphic(bool enable);
    
    /**
     * Set power budget for neuromorphic processing
     */
    void setPowerBudget(float power_mw);
    
    /**
     * Get current processing mode
     */
    enum ProcessingMode {
        TRADITIONAL_AI,
        NEUROMORPHIC,
        HYBRID
    };
    
    ProcessingMode getCurrentMode() const { return current_mode_; }
    
    /**
     * Force specific processing mode
     */
    void setProcessingMode(ProcessingMode mode);
    
    /**
     * Get neuromorphic system statistics
     */
    struct IntegrationStats {
        uint32_t total_detections;
        uint32_t neuromorphic_detections;
        uint32_t traditional_detections;
        uint32_t hybrid_detections;
        float average_power_traditional;
        float average_power_neuromorphic;
        float power_savings_percent;
        uint32_t mode_switches;
        
        IntegrationStats()
            : total_detections(0)
            , neuromorphic_detections(0)
            , traditional_detections(0)
            , hybrid_detections(0)
            , average_power_traditional(0.0f)
            , average_power_neuromorphic(0.0f)
            , power_savings_percent(0.0f)
            , mode_switches(0) {}
    };
    
    const IntegrationStats& getStatistics() const { return stats_; }
    
    /**
     * Reset statistics
     */
    void resetStatistics();
    
private:
    NeuromorphicIntegrationConfig config_;
    bool initialized_;
    bool use_neuromorphic_;
    ProcessingMode current_mode_;
    
    std::unique_ptr<NeuromorphicSystem> neuromorphic_system_;
    
    IntegrationStats stats_;
    
    // Decision logic
    bool shouldUseNeuromorphic(float battery_level, float available_power) const;
    bool shouldSwitchMode(float battery_level, float power_consumption);
    
    // Mode switching
    void switchToNeuromorphic();
    void switchToTraditional();
    void switchToHybrid();
    
    // Statistics tracking
    void updateStatistics(ProcessingMode mode, float power_consumption);
};

/**
 * Helper function to add neuromorphic capabilities to existing AI system
 * 
 * @param ai_config Existing AI configuration
 * @return Neuromorphic integration configuration
 */
NeuromorphicIntegrationConfig createNeuromorphicConfig(const AIProcessingConfig& ai_config);

/**
 * Helper to determine if neuromorphic processing is beneficial
 * 
 * @param battery_level Current battery level (0-100%)
 * @param power_budget Available power budget (mW)
 * @param performance_requirement Required detection accuracy (0-1)
 * @return True if neuromorphic is recommended
 */
bool shouldEnableNeuromorphic(float battery_level, float power_budget, float performance_requirement);

#endif // NEUROMORPHIC_AI_INTEGRATION_H
