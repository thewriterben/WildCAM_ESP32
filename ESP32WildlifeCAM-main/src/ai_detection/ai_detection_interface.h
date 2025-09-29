/**
 * @file ai_detection_interface.h
 * @brief Interface bridge to existing AI system
 * @version 1.0.0
 * 
 * This interface provides a bridge between the foundational AI detection framework
 * and the comprehensive AI system in firmware/src/ai/. It allows seamless integration
 * while maintaining modularity and backward compatibility.
 */

#ifndef AI_DETECTION_INTERFACE_H
#define AI_DETECTION_INTERFACE_H

#include "wildlife_detector.h"

namespace WildlifeDetection {

/**
 * @brief Interface to integrate with existing AI system
 * 
 * This class provides integration points with the comprehensive AI system
 * located in firmware/src/ai/, allowing the foundational framework to
 * leverage advanced features when available.
 */
class AIDetectionInterface {
public:
    AIDetectionInterface();
    ~AIDetectionInterface();

    /**
     * @brief Initialize interface with existing AI system
     * @return true if advanced AI system is available and connected
     */
    bool initializeAdvancedAI();

    /**
     * @brief Check if advanced AI features are available
     * @return true if connected to advanced AI system
     */
    bool hasAdvancedAI() const { return advanced_ai_available_; }

    /**
     * @brief Enhanced detection using advanced AI if available
     * @param frame_data Raw image data
     * @param frame_size Size of image data
     * @param width Image width
     * @param height Image height
     * @return Enhanced detection results
     */
    std::vector<DetectionResult> enhancedDetection(const uint8_t* frame_data,
                                                   size_t frame_size,
                                                   uint16_t width,
                                                   uint16_t height);

    /**
     * @brief Get available AI models
     * @return List of available AI model names
     */
    std::vector<const char*> getAvailableModels() const;

    /**
     * @brief Load specific AI model
     * @param model_name Name of the model to load
     * @return true if model loaded successfully
     */
    bool loadModel(const char* model_name);

    /**
     * @brief Configuration for advanced AI features
     */
    struct AdvancedConfig {
        bool use_tensorflow_lite = true;
        bool enable_behavior_analysis = false;
        bool enable_species_confidence_boost = true;
        float ai_confidence_threshold = 0.7f;
        uint32_t ai_processing_timeout_ms = 2000;
    };

    /**
     * @brief Configure advanced AI features
     * @param config Advanced configuration
     */
    void configureAdvancedAI(const AdvancedConfig& config);

private:
    bool advanced_ai_available_;
    AdvancedConfig advanced_config_;
    
    // Forward declarations for integration with existing AI system
    void* ai_system_ptr_;          // Points to AIWildlifeSystem if available
    void* species_detector_ptr_;   // Points to SpeciesDetector if available
    void* behavior_analyzer_ptr_;  // Points to BehaviorAnalyzer if available
    
    // Helper methods
    bool detectAdvancedAISystem();
    DetectionResult convertAdvancedResult(const void* advanced_result);
};

/**
 * @brief Utility functions for AI detection framework
 */
namespace Utils {
    /**
     * @brief Convert species type to human-readable string
     */
    const char* speciesToString(SpeciesType species);
    
    /**
     * @brief Convert confidence level to string
     */
    const char* confidenceToString(ConfidenceLevel confidence);
    
    /**
     * @brief Calculate detection area in pixels
     */
    uint32_t calculateDetectionArea(const DetectionResult& result);
    
    /**
     * @brief Validate detection result integrity
     */
    bool validateDetectionResult(const DetectionResult& result, uint16_t frame_width, uint16_t frame_height);
}

} // namespace WildlifeDetection

#endif // AI_DETECTION_INTERFACE_H