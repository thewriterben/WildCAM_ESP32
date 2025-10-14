/**
 * @file neuromorphic_system.cpp
 * @brief Main Neuromorphic Computing System Implementation
 */

#include "neuromorphic_system.h"
#include <algorithm>

// ============================================================================
// NeuromorphicSystem Implementation
// ============================================================================

NeuromorphicSystem::NeuromorphicSystem()
    : initialized_(false) {
}

NeuromorphicSystem::~NeuromorphicSystem() {
    cleanup();
}

bool NeuromorphicSystem::init(const NeuromorphicSystemConfig& config) {
    config_ = config;
    
    Serial.println("[Neuromorphic] Initializing neuromorphic computing system");
    
    // Initialize hardware manager
    hardware_manager_ = std::make_unique<NeuromorphicHardwareManager>();
    if (!hardware_manager_->init()) {
        Serial.println("[Neuromorphic] Failed to initialize hardware manager");
        return false;
    }
    
    // Configure network based on mode
    if (config_.processing_mode == NeuromorphicMode::SPIKE_BASED ||
        config_.processing_mode == NeuromorphicMode::HYBRID_ANN_SNN) {
        
        // Initialize standard SNN
        snn_ = std::make_unique<SpikingNeuralNetwork>();
        if (!snn_->init(config_.network_config)) {
            Serial.println("[Neuromorphic] Failed to initialize SNN");
            return false;
        }
        
        // Initialize convolutional SNN for image processing
        conv_snn_ = std::make_unique<SpikingConvNetwork>();
        if (!conv_snn_->initConvolutional(config_.image_width, config_.image_height, 16, 3)) {
            Serial.println("[Neuromorphic] Failed to initialize convolutional SNN");
            return false;
        }
        
        // Initialize recurrent SNN for behavior analysis
        recurrent_snn_ = std::make_unique<RecurrentSpikingNetwork>();
        if (!recurrent_snn_->initRecurrent(config_.network_config.hidden_neurons, 0.5f)) {
            Serial.println("[Neuromorphic] Failed to initialize recurrent SNN");
            return false;
        }
    }
    
    // Enable online learning if configured
    if (config_.online_learning_enabled && snn_) {
        snn_->enableOnlineLearning(true);
        Serial.println("[Neuromorphic] Online learning enabled");
    }
    
    // Select hardware platform
    if (config_.hardware_acceleration) {
        if (hardware_manager_->selectPlatform(config_.hardware_platform)) {
            Serial.print("[Neuromorphic] Using hardware platform: ");
            Serial.println(static_cast<int>(config_.hardware_platform));
        } else {
            Serial.println("[Neuromorphic] Hardware acceleration not available, using simulation");
        }
    }
    
    initialized_ = true;
    Serial.println("[Neuromorphic] Initialization complete");
    
    return true;
}

void NeuromorphicSystem::cleanup() {
    snn_.reset();
    conv_snn_.reset();
    recurrent_snn_.reset();
    hardware_manager_.reset();
    initialized_ = false;
}

NeuromorphicWildlifeResult NeuromorphicSystem::processFrame(const CameraFrame& frame) {
    NeuromorphicWildlifeResult result;
    
    if (!initialized_) {
        result.error_message = "System not initialized";
        return result;
    }
    
    // Extract image data
    if (frame.data == nullptr || frame.length == 0) {
        result.error_message = "Invalid frame data";
        return result;
    }
    
    // Process using neuromorphic computing
    return classifyWildlife(frame.data, frame.width, frame.height);
}

NeuromorphicWildlifeResult NeuromorphicSystem::processEvents(const std::vector<EventSensorData>& events) {
    NeuromorphicWildlifeResult result;
    
    if (!initialized_) {
        result.error_message = "System not initialized";
        return result;
    }
    
    if (events.empty()) {
        result.error_message = "No events to process";
        return result;
    }
    
    uint32_t start_time = micros();
    
    // Process events directly with SNN (highly efficient for event data)
    NeuromorphicDetection detection;
    
    if (config_.hardware_acceleration && shouldUseHardwareAcceleration()) {
        detection = hardware_manager_->executeEvents(events);
    } else if (snn_) {
        detection = snn_->processEvents(events);
    } else {
        result.error_message = "No processing engine available";
        return result;
    }
    
    result = convertToWildlifeResult(detection);
    result.detection_time_us = micros() - start_time;
    
    updateStatistics(result);
    updateDetectionHistory(result);
    
    return result;
}

NeuromorphicWildlifeResult NeuromorphicSystem::classifyWildlife(const uint8_t* image_data,
                                                               uint16_t width,
                                                               uint16_t height) {
    NeuromorphicWildlifeResult result;
    
    if (!initialized_ || image_data == nullptr) {
        result.error_message = "Invalid input";
        return result;
    }
    
    uint32_t start_time = micros();
    
    // Use hybrid processing if enabled
    if (config_.hybrid_ann_snn_enabled) {
        result = processHybrid(image_data, width, height);
    } else {
        // Preprocess image
        std::vector<float> preprocessed = preprocessImage(image_data, width, height);
        
        // Process with neuromorphic system
        NeuromorphicDetection detection;
        
        if (config_.hardware_acceleration && shouldUseHardwareAcceleration()) {
            detection = hardware_manager_->execute(preprocessed.data(), preprocessed.size());
        } else if (conv_snn_) {
            detection = conv_snn_->processImage(image_data, width, height);
        } else if (snn_) {
            detection = snn_->process(preprocessed.data(), preprocessed.size());
        } else {
            result.error_message = "No processing engine available";
            return result;
        }
        
        result = convertToWildlifeResult(detection);
    }
    
    result.detection_time_us = micros() - start_time;
    
    updateStatistics(result);
    updateDetectionHistory(result);
    
    return result;
}

BehaviorType NeuromorphicSystem::analyzeBehavior(
    const std::vector<NeuromorphicWildlifeResult>& detection_history) {
    
    if (!initialized_ || detection_history.empty() || !recurrent_snn_) {
        return BehaviorType::UNKNOWN;
    }
    
    // Extract spike patterns from detection history
    std::vector<SpikeEvent> combined_spikes;
    for (const auto& detection : detection_history) {
        combined_spikes.insert(combined_spikes.end(),
                             detection.output_spikes.begin(),
                             detection.output_spikes.end());
    }
    
    // Predict behavior using recurrent SNN
    uint8_t behavior_id = recurrent_snn_->predictBehavior(combined_spikes);
    
    // Map behavior ID to BehaviorType
    switch (behavior_id) {
        case 0: return BehaviorType::RESTING;
        case 1: return BehaviorType::MOVING;
        case 2: return BehaviorType::FEEDING;
        case 3: return BehaviorType::ALERT;
        case 4: return BehaviorType::SOCIAL;
        default: return BehaviorType::UNKNOWN;
    }
}

bool NeuromorphicSystem::train(const uint8_t* image_data, uint16_t width, uint16_t height,
                              SpeciesType species_label) {
    if (!initialized_ || !config_.online_learning_enabled) {
        return false;
    }
    
    // Preprocess image
    std::vector<float> preprocessed = preprocessImage(image_data, width, height);
    
    // Convert species to neuron ID
    uint8_t label = mapSpeciesToNeuronId(species_label);
    
    // Train network
    bool success = false;
    
    if (config_.hardware_acceleration && shouldUseHardwareAcceleration()) {
        success = hardware_manager_->train(preprocessed.data(), preprocessed.size(), label);
    } else if (snn_) {
        success = snn_->trainSTDP(preprocessed.data(), preprocessed.size(), label);
    }
    
    if (success) {
        stats_.training_iterations++;
    }
    
    return success;
}

void NeuromorphicSystem::enableOnlineLearning(bool enable) {
    config_.online_learning_enabled = enable;
    
    if (snn_) {
        snn_->enableOnlineLearning(enable);
    }
    
    Serial.print("[Neuromorphic] Online learning ");
    Serial.println(enable ? "enabled" : "disabled");
}

void NeuromorphicSystem::setPowerBudget(float power_mw) {
    config_.power_budget_mw = power_mw;
    
    if (hardware_manager_) {
        hardware_manager_->setPowerBudget(power_mw);
    }
    
    adjustProcessingForPower();
    
    Serial.print("[Neuromorphic] Power budget set to ");
    Serial.print(power_mw);
    Serial.println(" mW");
}

float NeuromorphicSystem::getPowerConsumption() const {
    if (config_.hardware_acceleration && hardware_manager_) {
        return hardware_manager_->getPowerConsumption();
    }
    
    return current_metrics_.power_consumption;
}

const NeuromorphicMetrics& NeuromorphicSystem::getMetrics() const {
    return current_metrics_;
}

void NeuromorphicSystem::enableHybridProcessing(bool enable) {
    config_.hybrid_ann_snn_enabled = enable;
    
    Serial.print("[Neuromorphic] Hybrid ANN-SNN processing ");
    Serial.println(enable ? "enabled" : "disabled");
}

bool NeuromorphicSystem::switchHardwarePlatform(NeuromorphicPlatform platform) {
    if (!hardware_manager_) {
        return false;
    }
    
    return hardware_manager_->selectPlatform(platform);
}

std::vector<NeuromorphicPlatform> NeuromorphicSystem::getAvailablePlatforms() const {
    if (hardware_manager_) {
        return hardware_manager_->getAvailablePlatforms();
    }
    
    return {NeuromorphicPlatform::SIMULATION};
}

void NeuromorphicSystem::reset() {
    if (snn_) snn_->reset();
    if (conv_snn_) conv_snn_->reset();
    if (recurrent_snn_) recurrent_snn_->reset();
    
    detection_history_.clear();
    current_metrics_ = NeuromorphicMetrics();
    
    Serial.println("[Neuromorphic] System reset");
}

// ============================================================================
// Private Methods
// ============================================================================

std::vector<float> NeuromorphicSystem::preprocessImage(const uint8_t* image_data,
                                                      uint16_t width, uint16_t height) {
    size_t target_size = config_.network_config.input_neurons;
    std::vector<float> normalized(target_size, 0.0f);
    
    // Resize and normalize image
    size_t pixels = width * height;
    float scale = static_cast<float>(target_size) / pixels;
    
    for (size_t i = 0; i < std::min(pixels, target_size); i++) {
        normalized[i] = image_data[i] / 255.0f;
    }
    
    return normalized;
}

NeuromorphicWildlifeResult NeuromorphicSystem::convertToWildlifeResult(
    const NeuromorphicDetection& detection) {
    
    NeuromorphicWildlifeResult result;
    
    result.species = mapNeuronIdToSpecies(detection.species_id);
    result.confidence = detection.confidence;
    result.neuro_metrics = detection.metrics;
    result.power_consumption_mw = detection.power_used;
    result.output_spikes = detection.output_spikes;
    result.is_valid = detection.is_valid;
    
    // Infer behavior from spike patterns (simplified)
    if (!detection.output_spikes.empty()) {
        float spike_rate = detection.output_spikes.size() / 
                          (config_.network_config.simulation_time / 1000.0f);
        
        if (spike_rate > 50.0f) {
            result.behavior = BehaviorType::ALERT;
        } else if (spike_rate > 20.0f) {
            result.behavior = BehaviorType::MOVING;
        } else {
            result.behavior = BehaviorType::RESTING;
        }
    }
    
    current_metrics_ = detection.metrics;
    
    return result;
}

SpeciesType NeuromorphicSystem::mapNeuronIdToSpecies(uint8_t neuron_id) {
    // Simple mapping - in production, use learned mapping
    switch (neuron_id % 9) {
        case 0: return SpeciesType::MAMMAL_SMALL;
        case 1: return SpeciesType::MAMMAL_MEDIUM;
        case 2: return SpeciesType::MAMMAL_LARGE;
        case 3: return SpeciesType::BIRD_SMALL;
        case 4: return SpeciesType::BIRD_MEDIUM;
        case 5: return SpeciesType::BIRD_LARGE;
        case 6: return SpeciesType::REPTILE;
        case 7: return SpeciesType::HUMAN;
        case 8: return SpeciesType::VEHICLE;
        default: return SpeciesType::UNKNOWN;
    }
}

uint8_t NeuromorphicSystem::mapSpeciesToNeuronId(SpeciesType species) {
    // Reverse mapping
    switch (species) {
        case SpeciesType::MAMMAL_SMALL: return 0;
        case SpeciesType::MAMMAL_MEDIUM: return 1;
        case SpeciesType::MAMMAL_LARGE: return 2;
        case SpeciesType::BIRD_SMALL: return 3;
        case SpeciesType::BIRD_MEDIUM: return 4;
        case SpeciesType::BIRD_LARGE: return 5;
        case SpeciesType::REPTILE: return 6;
        case SpeciesType::HUMAN: return 7;
        case SpeciesType::VEHICLE: return 8;
        default: return 0;
    }
}

void NeuromorphicSystem::updateStatistics(const NeuromorphicWildlifeResult& result) {
    stats_.total_inferences++;
    
    if (result.is_valid && result.confidence > 0.5f) {
        stats_.successful_detections++;
    }
    
    // Running average of metrics
    float alpha = 0.1f; // Exponential moving average factor
    
    stats_.average_inference_time_us = 
        alpha * result.detection_time_us + (1.0f - alpha) * stats_.average_inference_time_us;
    
    stats_.average_power_consumption_mw = 
        alpha * result.power_consumption_mw + (1.0f - alpha) * stats_.average_power_consumption_mw;
    
    stats_.average_confidence = 
        alpha * result.confidence + (1.0f - alpha) * stats_.average_confidence;
    
    stats_.total_spikes += result.neuro_metrics.total_spikes;
    
    stats_.average_sparsity = 
        alpha * result.neuro_metrics.network_sparsity + (1.0f - alpha) * stats_.average_sparsity;
}

void NeuromorphicSystem::updateDetectionHistory(const NeuromorphicWildlifeResult& result) {
    detection_history_.push_back(result);
    
    // Limit history size
    if (detection_history_.size() > MAX_HISTORY_SIZE) {
        detection_history_.erase(detection_history_.begin());
    }
}

NeuromorphicWildlifeResult NeuromorphicSystem::processHybrid(const uint8_t* image_data,
                                                            uint16_t width, uint16_t height) {
    NeuromorphicWildlifeResult result;
    
    // In hybrid mode, use traditional CNN for feature extraction
    // and SNN for temporal processing and classification
    // This is a simplified placeholder
    
    // Step 1: Traditional feature extraction (would use existing CNN)
    std::vector<float> features = preprocessImage(image_data, width, height);
    
    // Step 2: SNN classification
    NeuromorphicDetection detection;
    if (snn_) {
        detection = snn_->process(features.data(), features.size());
    }
    
    result = convertToWildlifeResult(detection);
    
    // Hybrid mode typically provides better accuracy at slightly higher power
    result.power_consumption_mw *= 1.2f; // 20% overhead for hybrid processing
    
    return result;
}

void NeuromorphicSystem::adjustProcessingForPower() {
    if (!initialized_) return;
    
    float current_power = getPowerConsumption();
    
    if (current_power > config_.power_budget_mw) {
        // Reduce power consumption
        if (snn_) {
            // Increase sparsity target
            float new_sparsity = std::min(0.95f, config_.network_config.sparsity_target * 1.1f);
            snn_->setSparsityTarget(new_sparsity);
            
            Serial.println("[Neuromorphic] Increasing sparsity to reduce power");
        }
        
        // Consider switching to lower-power hardware platform
        if (hardware_manager_ && config_.hardware_acceleration) {
            auto platforms = hardware_manager_->getAvailablePlatforms();
            for (auto platform : platforms) {
                if (hardware_manager_->selectPlatform(platform)) {
                    if (hardware_manager_->getPowerConsumption() < config_.power_budget_mw) {
                        Serial.println("[Neuromorphic] Switched to lower-power platform");
                        break;
                    }
                }
            }
        }
    }
}

bool NeuromorphicSystem::shouldUseHardwareAcceleration() const {
    if (!config_.hardware_acceleration || !hardware_manager_) {
        return false;
    }
    
    // Use hardware if power consumption is within budget
    float hw_power = hardware_manager_->getPowerConsumption();
    return hw_power <= config_.power_budget_mw;
}

// ============================================================================
// NeuromorphicSystemBuilder Implementation
// ============================================================================

NeuromorphicSystemBuilder::NeuromorphicSystemBuilder() {
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::setProcessingMode(NeuromorphicMode mode) {
    config_.processing_mode = mode;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::setHardwarePlatform(NeuromorphicPlatform platform) {
    config_.hardware_platform = platform;
    config_.hardware_acceleration = true;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::enableHybridProcessing() {
    config_.hybrid_ann_snn_enabled = true;
    config_.processing_mode = NeuromorphicMode::HYBRID_ANN_SNN;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::enableOnlineLearning() {
    config_.online_learning_enabled = true;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::setPowerBudget(float power_mw) {
    config_.power_budget_mw = power_mw;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::setImageSize(uint16_t width, uint16_t height) {
    config_.image_width = width;
    config_.image_height = height;
    config_.network_config.input_neurons = width * height;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::setNumSpecies(uint8_t num_species) {
    config_.num_species = num_species;
    config_.network_config.output_neurons = num_species;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::enableDVSSensor() {
    config_.dvs_sensor_enabled = true;
    return *this;
}

NeuromorphicSystemBuilder& NeuromorphicSystemBuilder::enableEventBasedProcessing() {
    config_.event_based_processing = true;
    config_.processing_mode = NeuromorphicMode::EVENT_DRIVEN;
    return *this;
}

std::unique_ptr<NeuromorphicSystem> NeuromorphicSystemBuilder::build() {
    auto system = std::make_unique<NeuromorphicSystem>();
    
    if (!system->init(config_)) {
        Serial.println("[Builder] Failed to initialize neuromorphic system");
        return nullptr;
    }
    
    return system;
}
