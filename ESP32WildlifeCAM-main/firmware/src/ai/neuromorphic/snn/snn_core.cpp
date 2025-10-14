/**
 * @file snn_core.cpp
 * @brief Core Spiking Neural Network Implementation
 */

#include "snn_core.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// SNNLayer Implementation
// ============================================================================

SNNLayer::SNNLayer(uint16_t num_neurons, NeuronType type)
    : num_neurons_(num_neurons)
    , neuron_type_(type)
    , total_spikes_(0)
    , time_step_(1.0f) {
}

SNNLayer::~SNNLayer() {
    neurons_.clear();
}

bool SNNLayer::init() {
    neurons_.resize(num_neurons_);
    total_spikes_ = 0;
    return true;
}

std::vector<SpikeEvent> SNNLayer::update(float current_time, 
                                         const std::vector<SpikeEvent>& input_spikes) {
    std::vector<SpikeEvent> output_spikes;
    
    // Accumulate input currents for each neuron
    std::vector<float> input_currents(num_neurons_, 0.0f);
    
    for (const auto& spike : input_spikes) {
        if (spike.neuronId < num_neurons_) {
            input_currents[spike.neuronId] += spike.amplitude;
        }
    }
    
    // Update each neuron
    for (uint16_t i = 0; i < num_neurons_; i++) {
        bool spiked = false;
        
        switch (neuron_type_) {
            case NeuronType::LEAKY_INTEGRATE_FIRE:
                spiked = updateLIFNeuron(i, current_time, input_currents[i]);
                break;
            case NeuronType::ADAPTIVE_LIF:
                spiked = updateAdaptiveLIFNeuron(i, current_time, input_currents[i]);
                break;
            case NeuronType::IZHIKEVICH:
                spiked = updateIzhikevichNeuron(i, current_time, input_currents[i]);
                break;
            default:
                spiked = updateLIFNeuron(i, current_time, input_currents[i]);
                break;
        }
        
        if (spiked) {
            output_spikes.emplace_back(i, static_cast<uint32_t>(current_time * 1000), 1.0f, 0);
            total_spikes_++;
        }
    }
    
    return output_spikes;
}

void SNNLayer::reset() {
    for (auto& neuron : neurons_) {
        neuron.membrane_potential = 0.0f;
        neuron.is_refractory = false;
        neuron.last_spike_time = 0.0f;
        neuron.adaptation_variable = 0.0f;
    }
    total_spikes_ = 0;
}

const NeuronState& SNNLayer::getNeuronState(uint16_t neuron_id) const {
    return neurons_[neuron_id];
}

void SNNLayer::setNeuronThreshold(uint16_t neuron_id, float threshold) {
    if (neuron_id < num_neurons_) {
        neurons_[neuron_id].threshold = threshold;
    }
}

float SNNLayer::getMeanFiringRate() const {
    if (num_neurons_ == 0) return 0.0f;
    return static_cast<float>(total_spikes_) / num_neurons_;
}

bool SNNLayer::updateLIFNeuron(uint16_t neuron_id, float current_time, float input_current) {
    NeuronState& neuron = neurons_[neuron_id];
    
    // Check refractory period
    if (neuron.is_refractory) {
        if (current_time - neuron.last_spike_time >= neuron.refractory_period) {
            neuron.is_refractory = false;
        } else {
            return false;
        }
    }
    
    // LIF dynamics: dV/dt = -(V - V_rest)/tau + I
    float tau_membrane = 20.0f; // ms
    float v_rest = -70.0f;      // mV
    
    float leak_current = -(neuron.membrane_potential - v_rest) / tau_membrane;
    neuron.membrane_potential += (leak_current + input_current) * time_step_;
    
    // Check for spike
    if (neuron.membrane_potential >= neuron.threshold) {
        neuron.membrane_potential = -65.0f; // Reset potential
        neuron.last_spike_time = current_time;
        neuron.is_refractory = true;
        return true;
    }
    
    return false;
}

bool SNNLayer::updateAdaptiveLIFNeuron(uint16_t neuron_id, float current_time, float input_current) {
    NeuronState& neuron = neurons_[neuron_id];
    
    // Check refractory period
    if (neuron.is_refractory) {
        if (current_time - neuron.last_spike_time >= neuron.refractory_period) {
            neuron.is_refractory = false;
        } else {
            return false;
        }
    }
    
    // Adaptive LIF dynamics
    float tau_membrane = 20.0f;
    float tau_adaptation = 100.0f;
    float v_rest = -70.0f;
    
    float leak_current = -(neuron.membrane_potential - v_rest) / tau_membrane;
    neuron.membrane_potential += (leak_current + input_current - neuron.adaptation_variable) * time_step_;
    
    // Adaptation variable decay
    neuron.adaptation_variable -= neuron.adaptation_variable / tau_adaptation * time_step_;
    
    // Check for spike
    if (neuron.membrane_potential >= neuron.threshold) {
        neuron.membrane_potential = -65.0f;
        neuron.adaptation_variable += 0.02f; // Increase adaptation
        neuron.last_spike_time = current_time;
        neuron.is_refractory = true;
        
        // Adaptive threshold increase
        neuron.threshold += 0.1f;
        if (neuron.threshold > 2.0f) {
            neuron.threshold = 2.0f;
        }
        
        return true;
    } else {
        // Gradual threshold decay
        neuron.threshold -= 0.001f * time_step_;
        if (neuron.threshold < 1.0f) {
            neuron.threshold = 1.0f;
        }
    }
    
    return false;
}

bool SNNLayer::updateIzhikevichNeuron(uint16_t neuron_id, float current_time, float input_current) {
    NeuronState& neuron = neurons_[neuron_id];
    
    // Izhikevich model parameters (regular spiking)
    float a = 0.02f;
    float b = 0.2f;
    float c = -65.0f;
    float d = 8.0f;
    
    float v = neuron.membrane_potential;
    float u = neuron.adaptation_variable;
    
    // Izhikevich dynamics
    float dv = (0.04f * v * v + 5.0f * v + 140.0f - u + input_current) * time_step_;
    float du = a * (b * v - u) * time_step_;
    
    v += dv;
    u += du;
    
    // Check for spike
    if (v >= 30.0f) {
        neuron.membrane_potential = c;
        neuron.adaptation_variable = u + d;
        neuron.last_spike_time = current_time;
        return true;
    }
    
    neuron.membrane_potential = v;
    neuron.adaptation_variable = u;
    
    return false;
}

// ============================================================================
// SpikingNeuralNetwork Implementation
// ============================================================================

SpikingNeuralNetwork::SpikingNeuralNetwork()
    : initialized_(false)
    , online_learning_enabled_(false)
    , learning_rule_(LearningRule::STDP)
    , current_time_(0.0f) {
}

SpikingNeuralNetwork::~SpikingNeuralNetwork() {
    cleanup();
}

bool SpikingNeuralNetwork::init(const NeuromorphicConfig& config) {
    config_ = config;
    
    // Create layers
    layers_.clear();
    
    // Input layer
    auto input_layer = std::make_unique<SNNLayer>(config_.input_neurons, config_.neuron_type);
    if (!input_layer->init()) {
        return false;
    }
    layers_.push_back(std::move(input_layer));
    
    // Hidden layers
    for (uint8_t i = 0; i < config_.num_layers - 2; i++) {
        auto hidden_layer = std::make_unique<SNNLayer>(config_.hidden_neurons, config_.neuron_type);
        if (!hidden_layer->init()) {
            return false;
        }
        layers_.push_back(std::move(hidden_layer));
    }
    
    // Output layer
    auto output_layer = std::make_unique<SNNLayer>(config_.output_neurons, config_.neuron_type);
    if (!output_layer->init()) {
        return false;
    }
    layers_.push_back(std::move(output_layer));
    
    // Initialize synapses
    initializeSynapses();
    buildConnectivityMap();
    
    initialized_ = true;
    return true;
}

void SpikingNeuralNetwork::cleanup() {
    layers_.clear();
    synapses_.clear();
    connectivity_map_.clear();
    initialized_ = false;
}

NeuromorphicDetection SpikingNeuralNetwork::process(const float* input_data, size_t input_length) {
    NeuromorphicDetection result;
    
    if (!initialized_ || input_data == nullptr) {
        result.is_valid = false;
        return result;
    }
    
    uint32_t start_time = micros();
    reset();
    
    // Encode input as spike train (rate coding)
    std::vector<SpikeEvent> input_spikes = encodeRateCode(input_data, input_length);
    metrics_.input_spikes = input_spikes.size();
    
    // Simulate network for configured time
    std::vector<SpikeEvent> current_spikes = input_spikes;
    std::vector<SpikeEvent> all_output_spikes;
    
    for (float t = 0; t < config_.simulation_time; t += config_.time_step) {
        current_time_ = t;
        
        // Process each layer
        for (size_t layer_idx = 0; layer_idx < layers_.size(); layer_idx++) {
            current_spikes = layers_[layer_idx]->update(t, current_spikes);
            
            // Apply synaptic connections for next layer
            if (layer_idx < layers_.size() - 1) {
                propagateSpikes(current_spikes);
            }
        }
        
        // Collect output spikes
        all_output_spikes.insert(all_output_spikes.end(), 
                                current_spikes.begin(), current_spikes.end());
        
        // Online learning
        if (online_learning_enabled_) {
            updateWeights();
        }
        
        // Power optimization
        if (config_.sparse_coding_enabled) {
            enforceSparsity();
        }
    }
    
    // Decode output
    result.species_id = decodeOutput(all_output_spikes);
    result.output_spikes = all_output_spikes;
    result.detection_time = micros() - start_time;
    
    // Calculate confidence based on output spike rate
    float max_spikes = config_.simulation_time / 10.0f; // Expected max
    float output_spike_count = static_cast<float>(all_output_spikes.size());
    result.confidence = std::min(1.0f, output_spike_count / max_spikes);
    
    // Update metrics
    updateMetrics();
    result.metrics = metrics_;
    result.power_used = estimatePowerConsumption();
    result.is_valid = true;
    
    return result;
}

NeuromorphicDetection SpikingNeuralNetwork::processEvents(const std::vector<EventSensorData>& events) {
    NeuromorphicDetection result;
    
    if (!initialized_ || events.empty()) {
        result.is_valid = false;
        return result;
    }
    
    // Convert events to spike trains
    std::vector<float> event_image(config_.input_neurons, 0.0f);
    
    for (const auto& event : events) {
        // Map event to neuron (simplified spatial mapping)
        uint32_t neuron_idx = (event.y * 28 + event.x) % config_.input_neurons;
        event_image[neuron_idx] += event.polarity > 0 ? 1.0f : -0.5f;
    }
    
    return process(event_image.data(), event_image.size());
}

bool SpikingNeuralNetwork::trainSTDP(const float* input_data, size_t input_length, uint8_t target_label) {
    if (!initialized_) return false;
    
    // Process input
    NeuromorphicDetection detection = process(input_data, input_length);
    
    // Apply STDP based on prediction error
    float reward = (detection.species_id == target_label) ? 1.0f : -0.5f;
    
    switch (learning_rule_) {
        case LearningRule::STDP:
            applySTDP();
            break;
        case LearningRule::REWARD_MODULATED_STDP:
            applyRewardModulatedSTDP(reward);
            break;
        case LearningRule::HOMEOSTATIC:
            applyHomeostaticPlasticity();
            break;
        default:
            applySTDP();
            break;
    }
    
    return true;
}

void SpikingNeuralNetwork::updateWeights() {
    applySTDP();
}

void SpikingNeuralNetwork::reset() {
    for (auto& layer : layers_) {
        layer->reset();
    }
    current_time_ = 0.0f;
    metrics_ = NeuromorphicMetrics();
}

void SpikingNeuralNetwork::enableAdaptiveThresholds(bool enable) {
    config_.adaptive_thresholds = enable;
}

void SpikingNeuralNetwork::setSparsityTarget(float target) {
    config_.sparsity_target = std::max(0.01f, std::min(1.0f, target));
}

// ============================================================================
// Private Methods
// ============================================================================

std::vector<SpikeEvent> SpikingNeuralNetwork::encodeRateCode(const float* input_data, size_t length) {
    std::vector<SpikeEvent> spikes;
    
    size_t max_neurons = std::min(length, static_cast<size_t>(config_.input_neurons));
    
    for (size_t i = 0; i < max_neurons; i++) {
        // Rate coding: higher value = more spikes
        float value = input_data[i];
        float firing_rate = value * 100.0f; // Scale to Hz
        
        // Generate Poisson spike train
        float num_spikes = firing_rate * config_.simulation_time / 1000.0f;
        
        for (int spike = 0; spike < static_cast<int>(num_spikes); spike++) {
            uint32_t spike_time = random(0, static_cast<uint32_t>(config_.simulation_time * 1000));
            spikes.emplace_back(i, spike_time, value, 0);
        }
    }
    
    return spikes;
}

std::vector<SpikeEvent> SpikingNeuralNetwork::encodeTemporalCode(const float* input_data, size_t length) {
    std::vector<SpikeEvent> spikes;
    
    // Temporal coding: higher value = earlier spike
    for (size_t i = 0; i < std::min(length, static_cast<size_t>(config_.input_neurons)); i++) {
        if (input_data[i] > 0.1f) {
            uint32_t spike_time = static_cast<uint32_t>((1.0f - input_data[i]) * config_.simulation_time * 1000);
            spikes.emplace_back(i, spike_time, 1.0f, 0);
        }
    }
    
    return spikes;
}

uint8_t SpikingNeuralNetwork::decodeOutput(const std::vector<SpikeEvent>& output_spikes) {
    // Count spikes per output neuron
    std::vector<uint32_t> spike_counts(config_.output_neurons, 0);
    
    for (const auto& spike : output_spikes) {
        if (spike.neuronId < config_.output_neurons) {
            spike_counts[spike.neuronId]++;
        }
    }
    
    // Return neuron with highest spike count
    auto max_it = std::max_element(spike_counts.begin(), spike_counts.end());
    return static_cast<uint8_t>(std::distance(spike_counts.begin(), max_it));
}

void SpikingNeuralNetwork::applySTDP() {
    for (auto& synapse : synapses_) {
        // STDP rule: LTP if pre before post, LTD if post before pre
        float dt = static_cast<float>(synapse.last_post_spike - synapse.last_pre_spike);
        
        if (dt > 0 && dt < config_.stdp_tau_plus * 5) {
            // Long-term potentiation
            float weight_change = config_.a_plus * exp(-dt / config_.stdp_tau_plus);
            synapse.weight += weight_change * synapse.learning_rate;
        } else if (dt < 0 && dt > -config_.stdp_tau_minus * 5) {
            // Long-term depression
            float weight_change = config_.a_minus * exp(dt / config_.stdp_tau_minus);
            synapse.weight -= weight_change * synapse.learning_rate;
        }
        
        // Bound weights
        synapse.weight = std::max(config_.weight_min, 
                                 std::min(config_.weight_max, synapse.weight));
    }
}

void SpikingNeuralNetwork::applyRewardModulatedSTDP(float reward) {
    for (auto& synapse : synapses_) {
        float dt = static_cast<float>(synapse.last_post_spike - synapse.last_pre_spike);
        
        if (abs(dt) < config_.stdp_tau_plus * 5) {
            float stdp_window = exp(-abs(dt) / config_.stdp_tau_plus);
            float weight_change = reward * stdp_window * synapse.learning_rate;
            synapse.weight += weight_change;
            
            synapse.weight = std::max(config_.weight_min, 
                                     std::min(config_.weight_max, synapse.weight));
        }
    }
}

void SpikingNeuralNetwork::applyHomeostaticPlasticity() {
    // Maintain target firing rate across network
    float target_rate = 10.0f; // Hz
    
    for (size_t layer_idx = 0; layer_idx < layers_.size(); layer_idx++) {
        float mean_rate = layers_[layer_idx]->getMeanFiringRate();
        float rate_error = target_rate - mean_rate;
        
        // Adjust thresholds to regulate firing
        for (uint16_t neuron_id = 0; neuron_id < layers_[layer_idx]->getNumNeurons(); neuron_id++) {
            const auto& state = layers_[layer_idx]->getNeuronState(neuron_id);
            float new_threshold = state.threshold - 0.001f * rate_error;
            layers_[layer_idx]->setNeuronThreshold(neuron_id, new_threshold);
        }
    }
}

void SpikingNeuralNetwork::propagateSpikes(const std::vector<SpikeEvent>& spikes) {
    for (const auto& spike : spikes) {
        // Update synapse spike times
        auto it = connectivity_map_.find(spike.neuronId);
        if (it != connectivity_map_.end()) {
            for (uint32_t syn_id : it->second) {
                if (syn_id < synapses_.size()) {
                    synapses_[syn_id].last_pre_spike = spike.timestamp;
                }
            }
        }
    }
}

float SpikingNeuralNetwork::computeSynapticCurrent(uint32_t neuron_id, float current_time) {
    float total_current = 0.0f;
    
    // Sum weighted inputs from all presynaptic neurons
    for (const auto& synapse : synapses_) {
        if (synapse.post_neuron_id == neuron_id) {
            float time_since_spike = current_time - synapse.last_pre_spike / 1000.0f;
            if (time_since_spike < 20.0f) {
                // Exponential synaptic current
                float synaptic_current = synapse.weight * exp(-time_since_spike / config_.tau_synapse);
                total_current += synaptic_current;
            }
        }
    }
    
    return total_current;
}

void SpikingNeuralNetwork::enforceSparsity() {
    // Simple sparsity enforcement by increasing thresholds
    if (config_.adaptive_thresholds) {
        updateAdaptiveThresholds();
    }
}

void SpikingNeuralNetwork::updateAdaptiveThresholds() {
    for (auto& layer : layers_) {
        float current_sparsity = 1.0f - (layer->getMeanFiringRate() / 100.0f);
        float sparsity_error = config_.sparsity_target - current_sparsity;
        
        // Adjust thresholds to achieve target sparsity
        for (uint16_t i = 0; i < layer->getNumNeurons(); i++) {
            const auto& state = layer->getNeuronState(i);
            float threshold_adjustment = sparsity_error * 0.1f;
            layer->setNeuronThreshold(i, state.threshold + threshold_adjustment);
        }
    }
}

float SpikingNeuralNetwork::estimatePowerConsumption() {
    // Simplified power model: power scales with spike count
    // Real neuromorphic chips: ~50pJ per spike per synapse
    float energy_per_spike = 50e-12f; // 50 pJ
    float total_energy = metrics_.total_spikes * energy_per_spike;
    
    // Convert to average power (mW) over simulation time
    float simulation_time_s = config_.simulation_time / 1000.0f;
    float power_mw = (total_energy / simulation_time_s) * 1000.0f;
    
    return power_mw;
}

void SpikingNeuralNetwork::buildConnectivityMap() {
    connectivity_map_.clear();
    
    for (size_t i = 0; i < synapses_.size(); i++) {
        connectivity_map_[synapses_[i].pre_neuron_id].push_back(i);
    }
}

void SpikingNeuralNetwork::initializeSynapses() {
    synapses_.clear();
    
    // Fully connected layers with random weights
    uint32_t synapse_id = 0;
    
    for (size_t layer = 0; layer < layers_.size() - 1; layer++) {
        uint16_t pre_neurons = layers_[layer]->getNumNeurons();
        uint16_t post_neurons = layers_[layer + 1]->getNumNeurons();
        
        // Sparse connectivity (10% connection probability)
        for (uint16_t pre = 0; pre < pre_neurons; pre++) {
            for (uint16_t post = 0; post < post_neurons; post++) {
                if (random(100) < 10) { // 10% connectivity
                    Synapse syn;
                    syn.pre_neuron_id = pre;
                    syn.post_neuron_id = post;
                    syn.weight = random(50, 150) / 100.0f; // Random 0.5-1.5
                    syn.delay = random(1, 5) * 1.0f;
                    syn.learning_rate = config_.a_plus;
                    synapses_.push_back(syn);
                }
            }
        }
    }
}

void SpikingNeuralNetwork::updateMetrics() {
    metrics_.total_spikes = 0;
    metrics_.active_neurons = 0;
    
    for (const auto& layer : layers_) {
        metrics_.total_spikes += layer->getTotalSpikes();
        
        for (uint16_t i = 0; i < layer->getNumNeurons(); i++) {
            if (layer->getNeuronState(i).last_spike_time > 0) {
                metrics_.active_neurons++;
            }
        }
    }
    
    uint32_t total_neurons = 0;
    for (const auto& layer : layers_) {
        total_neurons += layer->getNumNeurons();
    }
    
    metrics_.network_sparsity = total_neurons > 0 ? 
        1.0f - (static_cast<float>(metrics_.active_neurons) / total_neurons) : 0.0f;
    
    metrics_.synaptic_operations = synapses_.size();
    metrics_.mean_firing_rate = metrics_.total_spikes / 
        (config_.simulation_time / 1000.0f) / total_neurons;
    metrics_.power_consumption = estimatePowerConsumption();
}

// ============================================================================
// SpikingConvNetwork Implementation
// ============================================================================

SpikingConvNetwork::SpikingConvNetwork()
    : input_width_(0), input_height_(0), num_filters_(0), kernel_size_(0) {
}

SpikingConvNetwork::~SpikingConvNetwork() {
}

bool SpikingConvNetwork::initConvolutional(uint16_t input_width, uint16_t input_height,
                                          uint8_t num_filters, uint8_t kernel_size) {
    input_width_ = input_width;
    input_height_ = input_height;
    num_filters_ = num_filters;
    kernel_size_ = kernel_size;
    
    // Initialize base SNN
    NeuromorphicConfig config;
    config.input_neurons = input_width * input_height;
    config.output_neurons = 20; // Wildlife species count
    
    return init(config);
}

NeuromorphicDetection SpikingConvNetwork::processImage(const uint8_t* image_data,
                                                       uint16_t width, uint16_t height) {
    // Convert image to normalized float array
    std::vector<float> normalized_image(width * height);
    for (size_t i = 0; i < width * height; i++) {
        normalized_image[i] = image_data[i] / 255.0f;
    }
    
    return process(normalized_image.data(), normalized_image.size());
}

void SpikingConvNetwork::performSpikingConvolution(const std::vector<SpikeEvent>& input_spikes,
                                                   std::vector<SpikeEvent>& output_spikes) {
    // Simplified spiking convolution - groups spikes spatially
    // Real implementation would use learned convolutional kernels
    output_spikes = input_spikes; // Placeholder
}

// ============================================================================
// RecurrentSpikingNetwork Implementation
// ============================================================================

RecurrentSpikingNetwork::RecurrentSpikingNetwork()
    : recurrence_strength_(0.5f) {
}

RecurrentSpikingNetwork::~RecurrentSpikingNetwork() {
}

bool RecurrentSpikingNetwork::initRecurrent(uint16_t num_neurons, float recurrence_strength) {
    recurrence_strength_ = recurrence_strength;
    
    NeuromorphicConfig config;
    config.hidden_neurons = num_neurons;
    
    if (!init(config)) {
        return false;
    }
    
    // Add recurrent connections
    for (uint16_t i = 0; i < num_neurons; i++) {
        for (uint16_t j = 0; j < num_neurons; j++) {
            if (i != j && random(100) < 5) { // 5% recurrent connectivity
                Synapse syn;
                syn.pre_neuron_id = i;
                syn.post_neuron_id = j;
                syn.weight = recurrence_strength_ * (random(50, 150) / 100.0f);
                syn.delay = random(2, 10) * 1.0f;
                recurrent_synapses_.push_back(syn);
            }
        }
    }
    
    return true;
}

std::vector<NeuromorphicDetection> RecurrentSpikingNetwork::processSequence(
    const std::vector<std::vector<float>>& sequence) {
    
    std::vector<NeuromorphicDetection> results;
    
    for (const auto& frame : sequence) {
        NeuromorphicDetection detection = process(frame.data(), frame.size());
        results.push_back(detection);
        
        // Maintain spike history for temporal processing
        spike_history_.push_back(detection.output_spikes);
        if (spike_history_.size() > 10) {
            spike_history_.erase(spike_history_.begin());
        }
    }
    
    return results;
}

uint8_t RecurrentSpikingNetwork::predictBehavior(const std::vector<SpikeEvent>& history) {
    // Simplified behavior prediction from spike patterns
    // Real implementation would analyze temporal spike patterns
    
    if (history.empty()) return 0;
    
    // Count spike rates in different time windows
    uint32_t early_spikes = 0;
    uint32_t late_spikes = 0;
    uint32_t midpoint = history.back().timestamp / 2;
    
    for (const auto& spike : history) {
        if (spike.timestamp < midpoint) {
            early_spikes++;
        } else {
            late_spikes++;
        }
    }
    
    // Simple heuristic: increasing activity suggests active behavior
    return (late_spikes > early_spikes) ? 1 : 0;
}

void RecurrentSpikingNetwork::updateRecurrentConnections() {
    // Apply STDP to recurrent connections
    for (auto& synapse : recurrent_synapses_) {
        float dt = static_cast<float>(synapse.last_post_spike - synapse.last_pre_spike);
        
        if (dt > 0 && dt < 100) {
            synapse.weight += 0.01f * exp(-dt / 20.0f);
        } else if (dt < 0 && dt > -100) {
            synapse.weight -= 0.012f * exp(dt / 20.0f);
        }
        
        synapse.weight = std::max(0.0f, std::min(1.0f, synapse.weight));
    }
}

void RecurrentSpikingNetwork::maintainMemoryTrace() {
    // Gradually decay old spike history
    if (spike_history_.size() > 20) {
        spike_history_.erase(spike_history_.begin());
    }
}
