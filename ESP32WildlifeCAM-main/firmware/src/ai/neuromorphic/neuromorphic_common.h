/**
 * @file neuromorphic_common.h
 * @brief Common definitions for neuromorphic computing system
 * 
 * Provides common data structures, enums, and constants used across
 * the neuromorphic computing modules for wildlife monitoring.
 */

#ifndef NEUROMORPHIC_COMMON_H
#define NEUROMORPHIC_COMMON_H

#include <Arduino.h>
#include <vector>
#include <memory>

/**
 * Neuromorphic Processing Modes
 */
enum class NeuromorphicMode {
    SPIKE_BASED,           // Pure spiking neural network processing
    EVENT_DRIVEN,          // Event-driven asynchronous processing
    HYBRID_ANN_SNN,        // Hybrid artificial and spiking networks
    POWER_OPTIMIZED        // Ultra-low power mode with sparse updates
};

/**
 * Neuron Types
 */
enum class NeuronType {
    LEAKY_INTEGRATE_FIRE,  // LIF neuron model
    IZHIKEVICH,            // Izhikevich neuron model
    HODGKIN_HUXLEY,        // Hodgkin-Huxley neuron model (computationally expensive)
    ADAPTIVE_LIF           // Adaptive LIF with dynamic thresholds
};

/**
 * Learning Rule Types
 */
enum class LearningRule {
    STDP,                  // Spike-timing-dependent plasticity
    REWARD_MODULATED_STDP, // Reward-modulated STDP
    HOMEOSTATIC,           // Homeostatic plasticity
    HEBBIAN,               // Hebbian learning
    ANTI_HEBBIAN,          // Anti-Hebbian learning
    TRIPLET_STDP           // Triplet-based STDP
};

/**
 * Spike Event Structure
 */
struct SpikeEvent {
    uint32_t neuronId;
    uint32_t timestamp;    // Microsecond precision
    float amplitude;       // Normalized 0.0-1.0
    uint8_t layerId;
    
    SpikeEvent() : neuronId(0), timestamp(0), amplitude(1.0f), layerId(0) {}
    SpikeEvent(uint32_t nid, uint32_t ts, float amp = 1.0f, uint8_t lid = 0)
        : neuronId(nid), timestamp(ts), amplitude(amp), layerId(lid) {}
};

/**
 * Neuron State
 */
struct NeuronState {
    float membrane_potential;      // Current membrane potential (mV)
    float threshold;               // Spike threshold (mV)
    float refractory_period;       // Refractory period (ms)
    float last_spike_time;         // Time of last spike (ms)
    bool is_refractory;           // Currently in refractory period
    float adaptation_variable;     // For adaptive neurons
    
    NeuronState() 
        : membrane_potential(0.0f)
        , threshold(1.0f)
        , refractory_period(2.0f)
        , last_spike_time(0.0f)
        , is_refractory(false)
        , adaptation_variable(0.0f) {}
};

/**
 * Synapse Structure
 */
struct Synapse {
    uint32_t pre_neuron_id;        // Pre-synaptic neuron ID
    uint32_t post_neuron_id;       // Post-synaptic neuron ID
    float weight;                  // Synaptic weight
    float delay;                   // Transmission delay (ms)
    float learning_rate;           // Plasticity learning rate
    uint32_t last_pre_spike;       // Last pre-synaptic spike time
    uint32_t last_post_spike;      // Last post-synaptic spike time
    
    Synapse() 
        : pre_neuron_id(0)
        , post_neuron_id(0)
        , weight(0.5f)
        , delay(1.0f)
        , learning_rate(0.01f)
        , last_pre_spike(0)
        , last_post_spike(0) {}
};

/**
 * Neuromorphic Network Configuration
 */
struct NeuromorphicConfig {
    NeuromorphicMode mode;
    NeuronType neuron_type;
    LearningRule learning_rule;
    
    // Network topology
    uint16_t input_neurons;
    uint16_t hidden_neurons;
    uint16_t output_neurons;
    uint8_t num_layers;
    
    // Temporal parameters
    float time_step;               // Simulation time step (ms)
    float simulation_time;         // Total simulation time (ms)
    
    // Neuron parameters
    float tau_membrane;            // Membrane time constant (ms)
    float tau_synapse;             // Synaptic time constant (ms)
    float resting_potential;       // Resting membrane potential (mV)
    float reset_potential;         // Post-spike reset potential (mV)
    float spike_threshold;         // Default spike threshold (mV)
    
    // Learning parameters
    float stdp_tau_plus;           // STDP time constant for potentiation (ms)
    float stdp_tau_minus;          // STDP time constant for depression (ms)
    float a_plus;                  // STDP potentiation amplitude
    float a_minus;                 // STDP depression amplitude
    float weight_min;              // Minimum synaptic weight
    float weight_max;              // Maximum synaptic weight
    
    // Power optimization
    bool sparse_coding_enabled;
    float sparsity_target;         // Target network sparsity (0.0-1.0)
    bool adaptive_thresholds;
    
    NeuromorphicConfig()
        : mode(NeuromorphicMode::HYBRID_ANN_SNN)
        , neuron_type(NeuronType::LEAKY_INTEGRATE_FIRE)
        , learning_rule(LearningRule::STDP)
        , input_neurons(784)
        , hidden_neurons(256)
        , output_neurons(20)
        , num_layers(3)
        , time_step(1.0f)
        , simulation_time(100.0f)
        , tau_membrane(20.0f)
        , tau_synapse(5.0f)
        , resting_potential(-70.0f)
        , reset_potential(-65.0f)
        , spike_threshold(-55.0f)
        , stdp_tau_plus(20.0f)
        , stdp_tau_minus(20.0f)
        , a_plus(0.01f)
        , a_minus(0.012f)
        , weight_min(0.0f)
        , weight_max(1.0f)
        , sparse_coding_enabled(true)
        , sparsity_target(0.05f)
        , adaptive_thresholds(true) {}
};

/**
 * Neuromorphic Processing Metrics
 */
struct NeuromorphicMetrics {
    uint32_t total_spikes;
    uint32_t input_spikes;
    uint32_t output_spikes;
    float network_sparsity;        // Percentage of neurons that spiked
    float power_consumption;       // Estimated power (mW)
    float inference_time;          // Processing time (ms)
    float synaptic_operations;     // Number of synaptic operations
    uint32_t active_neurons;
    float mean_firing_rate;        // Spikes per second
    
    NeuromorphicMetrics()
        : total_spikes(0)
        , input_spikes(0)
        , output_spikes(0)
        , network_sparsity(0.0f)
        , power_consumption(0.0f)
        , inference_time(0.0f)
        , synaptic_operations(0.0f)
        , active_neurons(0)
        , mean_firing_rate(0.0f) {}
};

/**
 * Wildlife Detection Result from Neuromorphic Processing
 */
struct NeuromorphicDetection {
    uint8_t species_id;
    float confidence;
    uint32_t detection_time;       // Time to detection (us)
    float power_used;              // Power consumed (mW)
    std::vector<SpikeEvent> output_spikes;
    NeuromorphicMetrics metrics;
    bool is_valid;
    
    NeuromorphicDetection()
        : species_id(0)
        , confidence(0.0f)
        , detection_time(0)
        , power_used(0.0f)
        , is_valid(false) {}
};

/**
 * Event-based sensor data
 */
struct EventSensorData {
    uint16_t x;
    uint16_t y;
    uint32_t timestamp;
    int8_t polarity;               // -1 for OFF event, +1 for ON event
    uint8_t sensor_id;
    
    EventSensorData()
        : x(0), y(0), timestamp(0), polarity(0), sensor_id(0) {}
};

#endif // NEUROMORPHIC_COMMON_H
