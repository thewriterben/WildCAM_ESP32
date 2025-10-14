/**
 * @file snn_core.h
 * @brief Core Spiking Neural Network Implementation
 * 
 * Implements brain-inspired spiking neural networks for ultra-efficient
 * wildlife detection with temporal pattern recognition.
 */

#ifndef SNN_CORE_H
#define SNN_CORE_H

#include "../neuromorphic_common.h"
#include <vector>
#include <map>
#include <queue>

/**
 * Spiking Neural Network Layer
 */
class SNNLayer {
public:
    SNNLayer(uint16_t num_neurons, NeuronType type);
    ~SNNLayer();
    
    /**
     * Initialize layer neurons
     */
    bool init();
    
    /**
     * Update all neurons in layer for one time step
     * @param current_time Current simulation time (ms)
     * @param input_spikes Input spike events
     * @return Output spike events generated
     */
    std::vector<SpikeEvent> update(float current_time, 
                                   const std::vector<SpikeEvent>& input_spikes);
    
    /**
     * Reset all neurons to resting state
     */
    void reset();
    
    /**
     * Get neuron state
     */
    const NeuronState& getNeuronState(uint16_t neuron_id) const;
    
    /**
     * Set neuron threshold (for adaptive mechanisms)
     */
    void setNeuronThreshold(uint16_t neuron_id, float threshold);
    
    /**
     * Get layer statistics
     */
    uint32_t getTotalSpikes() const { return total_spikes_; }
    float getMeanFiringRate() const;
    uint16_t getNumNeurons() const { return num_neurons_; }
    
private:
    uint16_t num_neurons_;
    NeuronType neuron_type_;
    std::vector<NeuronState> neurons_;
    uint32_t total_spikes_;
    float time_step_;
    
    // Neuron model implementations
    bool updateLIFNeuron(uint16_t neuron_id, float current_time, float input_current);
    bool updateAdaptiveLIFNeuron(uint16_t neuron_id, float current_time, float input_current);
    bool updateIzhikevichNeuron(uint16_t neuron_id, float current_time, float input_current);
};

/**
 * Core Spiking Neural Network
 */
class SpikingNeuralNetwork {
public:
    SpikingNeuralNetwork();
    ~SpikingNeuralNetwork();
    
    /**
     * Initialize network with configuration
     */
    bool init(const NeuromorphicConfig& config);
    
    /**
     * Cleanup and deallocate resources
     */
    void cleanup();
    
    /**
     * Process input data and generate output spikes
     * @param input_data Input data (e.g., pixel values)
     * @param input_length Length of input data
     * @return Neuromorphic detection result
     */
    NeuromorphicDetection process(const float* input_data, size_t input_length);
    
    /**
     * Process event-based sensor data
     * @param events Vector of event sensor data
     * @return Neuromorphic detection result
     */
    NeuromorphicDetection processEvents(const std::vector<EventSensorData>& events);
    
    /**
     * Train network with STDP
     * @param input_data Training input
     * @param input_length Input length
     * @param target_label Target classification label
     * @return True if training successful
     */
    bool trainSTDP(const float* input_data, size_t input_length, uint8_t target_label);
    
    /**
     * Update synaptic weights using learning rule
     */
    void updateWeights();
    
    /**
     * Enable/disable online learning
     */
    void enableOnlineLearning(bool enable) { online_learning_enabled_ = enable; }
    
    /**
     * Set learning rule
     */
    void setLearningRule(LearningRule rule) { learning_rule_ = rule; }
    
    /**
     * Get network metrics
     */
    const NeuromorphicMetrics& getMetrics() const { return metrics_; }
    
    /**
     * Reset network state
     */
    void reset();
    
    /**
     * Enable adaptive threshold mechanisms
     */
    void enableAdaptiveThresholds(bool enable);
    
    /**
     * Set sparsity target for power optimization
     */
    void setSparsityTarget(float target);
    
private:
    NeuromorphicConfig config_;
    bool initialized_;
    bool online_learning_enabled_;
    LearningRule learning_rule_;
    
    // Network layers
    std::vector<std::unique_ptr<SNNLayer>> layers_;
    
    // Synaptic connections
    std::vector<Synapse> synapses_;
    std::map<uint32_t, std::vector<uint32_t>> connectivity_map_;
    
    // Spike queue for temporal processing
    std::priority_queue<SpikeEvent> spike_queue_;
    
    // Metrics
    NeuromorphicMetrics metrics_;
    float current_time_;
    
    // Internal processing
    std::vector<SpikeEvent> encodeRateCode(const float* input_data, size_t length);
    std::vector<SpikeEvent> encodeTemporalCode(const float* input_data, size_t length);
    uint8_t decodeOutput(const std::vector<SpikeEvent>& output_spikes);
    
    // Learning algorithms
    void applySTDP();
    void applyRewardModulatedSTDP(float reward);
    void applyHomeostaticPlasticity();
    
    // Synaptic operations
    void propagateSpikes(const std::vector<SpikeEvent>& spikes);
    float computeSynapticCurrent(uint32_t neuron_id, float current_time);
    
    // Power optimization
    void enforceSparsity();
    void updateAdaptiveThresholds();
    float estimatePowerConsumption();
    
    // Helper functions
    void buildConnectivityMap();
    void initializeSynapses();
    void updateMetrics();
};

/**
 * Multi-layer Spiking Convolutional Network
 */
class SpikingConvNetwork : public SpikingNeuralNetwork {
public:
    SpikingConvNetwork();
    ~SpikingConvNetwork();
    
    /**
     * Initialize with image dimensions
     */
    bool initConvolutional(uint16_t input_width, uint16_t input_height, 
                          uint8_t num_filters, uint8_t kernel_size);
    
    /**
     * Process image data with spiking convolutions
     */
    NeuromorphicDetection processImage(const uint8_t* image_data, 
                                      uint16_t width, uint16_t height);
    
private:
    uint16_t input_width_;
    uint16_t input_height_;
    uint8_t num_filters_;
    uint8_t kernel_size_;
    
    std::vector<std::vector<float>> conv_kernels_;
    
    void performSpikingConvolution(const std::vector<SpikeEvent>& input_spikes,
                                   std::vector<SpikeEvent>& output_spikes);
};

/**
 * Recurrent Spiking Network for Temporal Analysis
 */
class RecurrentSpikingNetwork : public SpikingNeuralNetwork {
public:
    RecurrentSpikingNetwork();
    ~RecurrentSpikingNetwork();
    
    /**
     * Initialize recurrent connections
     */
    bool initRecurrent(uint16_t num_neurons, float recurrence_strength);
    
    /**
     * Process temporal sequence
     */
    std::vector<NeuromorphicDetection> processSequence(
        const std::vector<std::vector<float>>& sequence);
    
    /**
     * Predict next behavior based on history
     */
    uint8_t predictBehavior(const std::vector<SpikeEvent>& history);
    
private:
    float recurrence_strength_;
    std::vector<Synapse> recurrent_synapses_;
    std::vector<std::vector<SpikeEvent>> spike_history_;
    
    void updateRecurrentConnections();
    void maintainMemoryTrace();
};

#endif // SNN_CORE_H
