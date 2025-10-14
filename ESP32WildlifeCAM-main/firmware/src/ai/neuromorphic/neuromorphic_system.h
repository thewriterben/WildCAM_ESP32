/**
 * @file neuromorphic_system.h
 * @brief Main Neuromorphic Computing System Integration
 * 
 * Integrates neuromorphic computing capabilities with the existing
 * AI wildlife monitoring system for ultra-low-power inference and
 * adaptive learning.
 */

#ifndef NEUROMORPHIC_SYSTEM_H
#define NEUROMORPHIC_SYSTEM_H

#include "neuromorphic_common.h"
#include "snn/snn_core.h"
#include "hardware/neuromorphic_hardware.h"
#include "../ai_common.h"
#include <memory>

/**
 * Neuromorphic System Configuration
 */
struct NeuromorphicSystemConfig {
    NeuromorphicMode processing_mode;
    NeuromorphicPlatform hardware_platform;
    
    // Network architecture
    NeuromorphicConfig network_config;
    
    // Integration settings
    bool hybrid_ann_snn_enabled;       // Use hybrid ANN-SNN approach
    bool hardware_acceleration;        // Use neuromorphic hardware if available
    bool online_learning_enabled;      // Enable continuous learning
    float power_budget_mw;            // Power budget in milliwatts
    
    // Wildlife detection settings
    uint8_t num_species;              // Number of wildlife species to detect
    uint16_t image_width;             // Input image width
    uint16_t image_height;            // Input image height
    
    // Event-based processing
    bool dvs_sensor_enabled;          // Dynamic vision sensor enabled
    bool event_based_processing;      // Event-driven processing mode
    
    NeuromorphicSystemConfig()
        : processing_mode(NeuromorphicMode::HYBRID_ANN_SNN)
        , hardware_platform(NeuromorphicPlatform::SIMULATION)
        , hybrid_ann_snn_enabled(true)
        , hardware_acceleration(false)
        , online_learning_enabled(true)
        , power_budget_mw(10.0f)
        , num_species(20)
        , image_width(224)
        , image_height(224)
        , dvs_sensor_enabled(false)
        , event_based_processing(false) {}
};

/**
 * Neuromorphic Wildlife Detection Result
 */
struct NeuromorphicWildlifeResult {
    // Detection information
    SpeciesType species;
    BehaviorType behavior;
    float confidence;
    
    // Neuromorphic metrics
    NeuromorphicMetrics neuro_metrics;
    uint32_t detection_time_us;
    float power_consumption_mw;
    
    // Spike patterns (for analysis)
    std::vector<SpikeEvent> output_spikes;
    
    // Validity
    bool is_valid;
    String error_message;
    
    NeuromorphicWildlifeResult()
        : species(SpeciesType::UNKNOWN)
        , behavior(BehaviorType::UNKNOWN)
        , confidence(0.0f)
        , detection_time_us(0)
        , power_consumption_mw(0.0f)
        , is_valid(false) {}
};

/**
 * Main Neuromorphic Computing System
 * 
 * Provides neuromorphic computing capabilities for wildlife monitoring
 * with ultra-low power consumption and adaptive learning.
 */
class NeuromorphicSystem {
public:
    NeuromorphicSystem();
    ~NeuromorphicSystem();
    
    /**
     * Initialize neuromorphic system
     */
    bool init(const NeuromorphicSystemConfig& config);
    
    /**
     * Cleanup and deallocate resources
     */
    void cleanup();
    
    /**
     * Process camera frame with neuromorphic computing
     * @param frame Camera frame data
     * @return Neuromorphic detection result
     */
    NeuromorphicWildlifeResult processFrame(const CameraFrame& frame);
    
    /**
     * Process event-based sensor data
     * @param events Vector of event sensor data
     * @return Neuromorphic detection result
     */
    NeuromorphicWildlifeResult processEvents(const std::vector<EventSensorData>& events);
    
    /**
     * Classify wildlife species using neuromorphic processing
     * @param image_data Raw image data
     * @param width Image width
     * @param height Image height
     * @return Detection result
     */
    NeuromorphicWildlifeResult classifyWildlife(const uint8_t* image_data,
                                               uint16_t width,
                                               uint16_t height);
    
    /**
     * Analyze behavior patterns using recurrent SNN
     * @param detection_history History of recent detections
     * @return Predicted behavior
     */
    BehaviorType analyzeBehavior(const std::vector<NeuromorphicWildlifeResult>& detection_history);
    
    /**
     * Train network with new observation
     * @param image_data Training image
     * @param width Image width
     * @param height Image height
     * @param species_label True species label
     * @return True if training successful
     */
    bool train(const uint8_t* image_data, uint16_t width, uint16_t height,
              SpeciesType species_label);
    
    /**
     * Enable/disable online learning
     */
    void enableOnlineLearning(bool enable);
    
    /**
     * Set power budget for adaptive processing
     */
    void setPowerBudget(float power_mw);
    
    /**
     * Get current power consumption
     */
    float getPowerConsumption() const;
    
    /**
     * Get neuromorphic metrics
     */
    const NeuromorphicMetrics& getMetrics() const;
    
    /**
     * Enable hybrid ANN-SNN processing
     */
    void enableHybridProcessing(bool enable);
    
    /**
     * Switch neuromorphic hardware platform
     */
    bool switchHardwarePlatform(NeuromorphicPlatform platform);
    
    /**
     * Get available hardware platforms
     */
    std::vector<NeuromorphicPlatform> getAvailablePlatforms() const;
    
    /**
     * Reset network state
     */
    void reset();
    
    /**
     * Get system statistics
     */
    struct Statistics {
        uint32_t total_inferences;
        uint32_t successful_detections;
        float average_inference_time_us;
        float average_power_consumption_mw;
        float average_confidence;
        uint32_t total_spikes;
        float average_sparsity;
        uint32_t training_iterations;
        
        Statistics()
            : total_inferences(0)
            , successful_detections(0)
            , average_inference_time_us(0.0f)
            , average_power_consumption_mw(0.0f)
            , average_confidence(0.0f)
            , total_spikes(0)
            , average_sparsity(0.0f)
            , training_iterations(0) {}
    };
    
    const Statistics& getStatistics() const { return stats_; }
    
private:
    NeuromorphicSystemConfig config_;
    bool initialized_;
    
    // Core components
    std::unique_ptr<SpikingNeuralNetwork> snn_;
    std::unique_ptr<SpikingConvNetwork> conv_snn_;
    std::unique_ptr<RecurrentSpikingNetwork> recurrent_snn_;
    std::unique_ptr<NeuromorphicHardwareManager> hardware_manager_;
    
    // Metrics and statistics
    NeuromorphicMetrics current_metrics_;
    Statistics stats_;
    
    // Detection history for behavior analysis
    std::vector<NeuromorphicWildlifeResult> detection_history_;
    static const size_t MAX_HISTORY_SIZE = 100;
    
    // Internal processing methods
    std::vector<float> preprocessImage(const uint8_t* image_data, 
                                      uint16_t width, uint16_t height);
    
    NeuromorphicWildlifeResult convertToWildlifeResult(const NeuromorphicDetection& detection);
    
    SpeciesType mapNeuronIdToSpecies(uint8_t neuron_id);
    uint8_t mapSpeciesToNeuronId(SpeciesType species);
    
    void updateStatistics(const NeuromorphicWildlifeResult& result);
    void updateDetectionHistory(const NeuromorphicWildlifeResult& result);
    
    // Hybrid ANN-SNN processing
    NeuromorphicWildlifeResult processHybrid(const uint8_t* image_data,
                                            uint16_t width, uint16_t height);
    
    // Power management
    void adjustProcessingForPower();
    bool shouldUseHardwareAcceleration() const;
};

/**
 * Neuromorphic System Builder
 * 
 * Provides convenient builder pattern for constructing neuromorphic systems
 */
class NeuromorphicSystemBuilder {
public:
    NeuromorphicSystemBuilder();
    
    NeuromorphicSystemBuilder& setProcessingMode(NeuromorphicMode mode);
    NeuromorphicSystemBuilder& setHardwarePlatform(NeuromorphicPlatform platform);
    NeuromorphicSystemBuilder& enableHybridProcessing();
    NeuromorphicSystemBuilder& enableOnlineLearning();
    NeuromorphicSystemBuilder& setPowerBudget(float power_mw);
    NeuromorphicSystemBuilder& setImageSize(uint16_t width, uint16_t height);
    NeuromorphicSystemBuilder& setNumSpecies(uint8_t num_species);
    NeuromorphicSystemBuilder& enableDVSSensor();
    NeuromorphicSystemBuilder& enableEventBasedProcessing();
    
    std::unique_ptr<NeuromorphicSystem> build();
    
private:
    NeuromorphicSystemConfig config_;
};

#endif // NEUROMORPHIC_SYSTEM_H
