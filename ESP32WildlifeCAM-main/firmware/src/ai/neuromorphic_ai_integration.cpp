/**
 * @file neuromorphic_ai_integration.cpp
 * @brief Integration layer implementation
 */

#include "neuromorphic_ai_integration.h"

// ============================================================================
// NeuromorphicAIIntegration Implementation
// ============================================================================

NeuromorphicAIIntegration::NeuromorphicAIIntegration()
    : initialized_(false)
    , use_neuromorphic_(false)
    , current_mode_(ProcessingMode::TRADITIONAL_AI) {
}

NeuromorphicAIIntegration::~NeuromorphicAIIntegration() {
    neuromorphic_system_.reset();
}

bool NeuromorphicAIIntegration::init(const NeuromorphicIntegrationConfig& config) {
    config_ = config;
    
    if (!config_.enabled) {
        Serial.println("[Neuro Integration] Neuromorphic processing disabled");
        return false;
    }
    
    Serial.println("[Neuro Integration] Initializing neuromorphic integration...");
    
    // Initialize neuromorphic system
    NeuromorphicSystemConfig neuro_config;
    neuro_config.processing_mode = config_.hybrid_mode ? 
        NeuromorphicMode::HYBRID_ANN_SNN : NeuromorphicMode::SPIKE_BASED;
    neuro_config.online_learning_enabled = true;
    neuro_config.power_budget_mw = config_.power_threshold_mw;
    neuro_config.image_width = 224;
    neuro_config.image_height = 224;
    neuro_config.num_species = 20;
    
    neuromorphic_system_ = std::make_unique<NeuromorphicSystem>();
    if (!neuromorphic_system_->init(neuro_config)) {
        Serial.println("[Neuro Integration] Failed to initialize neuromorphic system");
        neuromorphic_system_.reset();
        return false;
    }
    
    initialized_ = true;
    current_mode_ = ProcessingMode::TRADITIONAL_AI;
    
    Serial.println("[Neuro Integration] Initialization complete");
    return true;
}

AIResult NeuromorphicAIIntegration::processWildlifeDetection(const CameraFrame& frame, 
                                                             float battery_level) {
    AIResult result;
    
    if (!initialized_) {
        result.isValidDetection = false;
        return result;
    }
    
    // Determine processing mode
    float available_power = 100.0f; // Simplified - would get from power manager
    bool should_use_neuro = shouldUseNeuromorphic(battery_level, available_power);
    
    if (config_.auto_switching && should_use_neuro != use_neuromorphic_) {
        if (should_use_neuro) {
            switchToNeuromorphic();
        } else {
            switchToTraditional();
        }
    }
    
    // Process based on current mode
    if (current_mode_ == ProcessingMode::NEUROMORPHIC && neuromorphic_system_) {
        // Use neuromorphic processing
        NeuromorphicWildlifeResult neuro_result = neuromorphic_system_->processFrame(frame);
        result = convertNeuromorphicToAIResult(neuro_result);
        
        updateStatistics(ProcessingMode::NEUROMORPHIC, neuro_result.power_consumption_mw);
        
        Serial.println("[Neuro Integration] Used neuromorphic processing");
        
    } else if (current_mode_ == ProcessingMode::HYBRID && neuromorphic_system_) {
        // Hybrid mode - use both (neuromorphic for speed, traditional for validation)
        NeuromorphicWildlifeResult neuro_result = neuromorphic_system_->processFrame(frame);
        result = convertNeuromorphicToAIResult(neuro_result);
        
        updateStatistics(ProcessingMode::HYBRID, neuro_result.power_consumption_mw);
        
        Serial.println("[Neuro Integration] Used hybrid processing");
        
    } else {
        // Use traditional AI processing
        // In real implementation, would call traditional AI system
        result.species = SpeciesType::UNKNOWN;
        result.behavior = BehaviorType::UNKNOWN;
        result.confidence = 0.0f;
        result.isValidDetection = false;
        
        updateStatistics(ProcessingMode::TRADITIONAL_AI, 500.0f);  // Typical CNN power
        
        Serial.println("[Neuro Integration] Used traditional AI processing");
    }
    
    return result;
}

AIResult NeuromorphicAIIntegration::convertNeuromorphicToAIResult(
    const NeuromorphicWildlifeResult& neuro_result) {
    
    AIResult result;
    
    result.species = neuro_result.species;
    result.behavior = neuro_result.behavior;
    result.confidence = neuro_result.confidence;
    result.timestamp = millis();
    result.isValidDetection = neuro_result.is_valid;
    
    // Map confidence to confidence level
    if (result.confidence >= 0.8f) {
        result.confidenceLevel = ConfidenceLevel::VERY_HIGH;
    } else if (result.confidence >= 0.6f) {
        result.confidenceLevel = ConfidenceLevel::HIGH;
    } else if (result.confidence >= 0.4f) {
        result.confidenceLevel = ConfidenceLevel::MEDIUM;
    } else if (result.confidence >= 0.2f) {
        result.confidenceLevel = ConfidenceLevel::LOW;
    } else {
        result.confidenceLevel = ConfidenceLevel::VERY_LOW;
    }
    
    return result;
}

void NeuromorphicAIIntegration::enableNeuromorphic(bool enable) {
    if (!initialized_) return;
    
    use_neuromorphic_ = enable;
    
    if (enable) {
        switchToNeuromorphic();
    } else {
        switchToTraditional();
    }
    
    Serial.printf("[Neuro Integration] Neuromorphic processing %s\n", 
                 enable ? "enabled" : "disabled");
}

void NeuromorphicAIIntegration::setPowerBudget(float power_mw) {
    if (neuromorphic_system_) {
        neuromorphic_system_->setPowerBudget(power_mw);
    }
    
    config_.power_threshold_mw = power_mw;
}

void NeuromorphicAIIntegration::setProcessingMode(ProcessingMode mode) {
    switch (mode) {
        case ProcessingMode::NEUROMORPHIC:
            switchToNeuromorphic();
            break;
        case ProcessingMode::TRADITIONAL_AI:
            switchToTraditional();
            break;
        case ProcessingMode::HYBRID:
            switchToHybrid();
            break;
    }
}

void NeuromorphicAIIntegration::resetStatistics() {
    stats_ = IntegrationStats();
}

// ============================================================================
// Private Methods
// ============================================================================

bool NeuromorphicAIIntegration::shouldUseNeuromorphic(float battery_level, 
                                                      float available_power) const {
    if (!config_.enabled || !initialized_ || !neuromorphic_system_) {
        return false;
    }
    
    // Always use neuromorphic if preferred
    if (config_.prefer_neuromorphic) {
        return true;
    }
    
    // Switch to neuromorphic on low battery
    if (battery_level < config_.battery_threshold_percent) {
        return true;
    }
    
    // Switch to neuromorphic on low power
    if (available_power < config_.power_threshold_mw) {
        return true;
    }
    
    return false;
}

bool NeuromorphicAIIntegration::shouldSwitchMode(float battery_level, float power_consumption) {
    if (!config_.auto_switching) {
        return false;
    }
    
    // Switch to neuromorphic if battery low
    if (battery_level < config_.battery_threshold_percent && 
        current_mode_ != ProcessingMode::NEUROMORPHIC) {
        return true;
    }
    
    // Switch back to traditional if battery recovered
    if (battery_level > config_.battery_threshold_percent + 10.0f &&
        current_mode_ == ProcessingMode::NEUROMORPHIC) {
        return true;
    }
    
    return false;
}

void NeuromorphicAIIntegration::switchToNeuromorphic() {
    if (current_mode_ != ProcessingMode::NEUROMORPHIC) {
        current_mode_ = ProcessingMode::NEUROMORPHIC;
        use_neuromorphic_ = true;
        stats_.mode_switches++;
        
        Serial.println("[Neuro Integration] Switched to neuromorphic mode");
    }
}

void NeuromorphicAIIntegration::switchToTraditional() {
    if (current_mode_ != ProcessingMode::TRADITIONAL_AI) {
        current_mode_ = ProcessingMode::TRADITIONAL_AI;
        use_neuromorphic_ = false;
        stats_.mode_switches++;
        
        Serial.println("[Neuro Integration] Switched to traditional AI mode");
    }
}

void NeuromorphicAIIntegration::switchToHybrid() {
    if (current_mode_ != ProcessingMode::HYBRID) {
        current_mode_ = ProcessingMode::HYBRID;
        use_neuromorphic_ = true;
        stats_.mode_switches++;
        
        Serial.println("[Neuro Integration] Switched to hybrid mode");
    }
}

void NeuromorphicAIIntegration::updateStatistics(ProcessingMode mode, float power_consumption) {
    stats_.total_detections++;
    
    switch (mode) {
        case ProcessingMode::NEUROMORPHIC:
            stats_.neuromorphic_detections++;
            stats_.average_power_neuromorphic = 
                (stats_.average_power_neuromorphic * (stats_.neuromorphic_detections - 1) + 
                 power_consumption) / stats_.neuromorphic_detections;
            break;
            
        case ProcessingMode::TRADITIONAL_AI:
            stats_.traditional_detections++;
            stats_.average_power_traditional = 
                (stats_.average_power_traditional * (stats_.traditional_detections - 1) + 
                 power_consumption) / stats_.traditional_detections;
            break;
            
        case ProcessingMode::HYBRID:
            stats_.hybrid_detections++;
            break;
    }
    
    // Calculate power savings
    if (stats_.average_power_traditional > 0 && stats_.average_power_neuromorphic > 0) {
        stats_.power_savings_percent = 
            100.0f * (1.0f - stats_.average_power_neuromorphic / stats_.average_power_traditional);
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

NeuromorphicIntegrationConfig createNeuromorphicConfig(const AIProcessingConfig& ai_config) {
    NeuromorphicIntegrationConfig config;
    
    // Enable neuromorphic if any AI processing is enabled
    config.enabled = ai_config.enableSpeciesClassification || 
                    ai_config.enableBehaviorAnalysis;
    
    // Use hybrid mode if both species and behavior analysis are needed
    config.hybrid_mode = ai_config.enableSpeciesClassification && 
                        ai_config.enableBehaviorAnalysis;
    
    // Auto-switching enabled by default
    config.auto_switching = true;
    
    // Conservative thresholds
    config.power_threshold_mw = 50.0f;
    config.battery_threshold_percent = 25.0f;
    
    // Don't prefer neuromorphic by default (let auto-switching decide)
    config.prefer_neuromorphic = false;
    
    return config;
}

bool shouldEnableNeuromorphic(float battery_level, float power_budget, 
                             float performance_requirement) {
    // Enable neuromorphic if:
    // 1. Low battery (<30%)
    if (battery_level < 30.0f) {
        return true;
    }
    
    // 2. Very limited power budget (<50mW)
    if (power_budget < 50.0f) {
        return true;
    }
    
    // 3. Performance requirement not too high (<0.9 accuracy)
    //    and power is a concern
    if (performance_requirement < 0.9f && power_budget < 200.0f) {
        return true;
    }
    
    // 4. Extreme power saving needed
    if (battery_level < 50.0f && power_budget < 100.0f) {
        return true;
    }
    
    return false;
}
