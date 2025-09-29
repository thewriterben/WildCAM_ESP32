/**
 * @file power_ai_optimizer.cpp
 * @brief Implementation of Advanced Power-AI Optimization System
 */

#include "power_ai_optimizer.h"
#include "../config.h"
#include <algorithm>
#include <cmath>

// Constants
const float PowerAIOptimizer::EMERGENCY_BATTERY_THRESHOLD = 3.2f;
const float PowerAIOptimizer::LOW_BATTERY_THRESHOLD = 3.4f;

/**
 * Constructor
 */
PowerAIOptimizer::PowerAIOptimizer()
    : inferenceEngine_(nullptr),
      initialized_(false),
      currentMode_(PowerAIMode::BALANCED),
      adaptivePowerManagementEnabled_(true),
      emergencyModeEnabled_(false),
      predictiveOptimizationEnabled_(true),
      debugModeEnabled_(false),
      currentBatteryVoltage_(3.7f),
      isCharging_(false),
      solarVoltage_(0.0f),
      currentPowerConsumption_mW_(0),
      lastPowerUpdate_(0),
      currentTemperature_(25.0f),
      currentHumidity_(50.0f),
      currentLightLevel_(0.5f),
      windSpeed_(0.0f),
      precipitation_(0.0f),
      currentMonth_(6),
      latitude_(45.0f),
      activityPredictionWindow_(24),
      lastOptimizationTime_(0),
      optimizationInterval_(DEFAULT_OPTIMIZATION_INTERVAL),
      metricsStartTime_(0),
      lastSleepTime_(0),
      totalSleepTime_(0) {
}

/**
 * Destructor
 */
PowerAIOptimizer::~PowerAIOptimizer() {
    cleanup();
}

/**
 * Initialize the power-AI optimization system
 */
bool PowerAIOptimizer::init(InferenceEngine* inferenceEngine) {
    if (!inferenceEngine) {
        DEBUG_PRINTLN("Error: Invalid inference engine");
        return false;
    }
    
    inferenceEngine_ = inferenceEngine;
    
    // Initialize default configuration
    powerBudget_ = PowerBudget();
    aiConfig_ = DynamicAIConfig();
    
    // Set initial operating mode based on battery level
    if (currentBatteryVoltage_ < LOW_BATTERY_THRESHOLD) {
        setOperatingMode(PowerAIMode::ECO_MONITORING);
    } else {
        setOperatingMode(PowerAIMode::ADAPTIVE);
    }
    
    // Initialize metrics
    resetMetrics();
    
    // Set up default detection priorities
    detectionPriorities_ = {
        {SpeciesType::HUMAN, AIWorkloadPriority::CRITICAL},
        {SpeciesType::MAMMAL_LARGE, AIWorkloadPriority::HIGH},
        {SpeciesType::BIRD_LARGE, AIWorkloadPriority::HIGH},
        {SpeciesType::MAMMAL_MEDIUM, AIWorkloadPriority::MEDIUM},
        {SpeciesType::BIRD_MEDIUM, AIWorkloadPriority::MEDIUM},
        {SpeciesType::MAMMAL_SMALL, AIWorkloadPriority::LOW},
        {SpeciesType::BIRD_SMALL, AIWorkloadPriority::LOW}
    };
    
    initialized_ = true;
    metricsStartTime_ = millis();
    
    DEBUG_PRINTLN("Power-AI Optimizer initialized successfully");
    
    if (debugModeEnabled_) {
        printSystemStatus();
    }
    
    return true;
}

/**
 * Configure power budget and operating mode
 */
void PowerAIOptimizer::configure(const PowerBudget& budget, PowerAIMode mode) {
    powerBudget_ = budget;
    setOperatingMode(mode);
    
    // Adjust AI configuration based on power budget
    adaptConfigurationToConditions();
    
    DEBUG_PRINTF("Power-AI Optimizer configured: Mode=%s, Budget=%u mW\n",
                powerAIModeToString(mode), budget.totalBudget_mW);
}

/**
 * Process a camera frame with power-optimized AI
 */
bool PowerAIOptimizer::processFrame(const CameraFrame& frame, AIResult& result) {
    if (!initialized_) {
        DEBUG_PRINTLN("Error: Power-AI Optimizer not initialized");
        return false;
    }
    
    unsigned long startTime = millis();
    bool success = false;
    
    // Check if we should process this frame based on power state
    if (currentBatteryVoltage_ < EMERGENCY_BATTERY_THRESHOLD && !isCharging_) {
        DEBUG_PRINTLN("Emergency power mode - skipping AI processing");
        return false;
    }
    
    // Update power consumption before processing
    updatePowerConsumption();
    
    // Select optimal processing strategy
    if (currentMode_ == PowerAIMode::ADAPTIVE) {
        // Use progressive inference for adaptive mode
        success = runProgressiveInference(frame, result);
    } else {
        // Use configured model for fixed modes
        ModelType selectedModel = selectOptimalModel(currentBatteryVoltage_, 1.0f);
        success = runPowerOptimizedInference(frame, selectedModel, result);
    }
    
    // Record activity if detection was successful
    if (success && result.isValidDetection) {
        recordWildlifeActivity(result);
        
        // Handle high-priority detections
        if (isHighPrioritySpecies(result.species)) {
            handleHighPriorityDetection(result);
        }
    }
    
    // Update metrics
    float processingTime = millis() - startTime;
    updateMetrics(result, currentPowerConsumption_mW_ * (processingTime / 1000.0f));
    
    // Trigger optimization if needed
    if (shouldTriggerOptimization()) {
        optimizeBasedOnMetrics();
    }
    
    return success;
}

/**
 * Run progressive inference with early exit capabilities
 */
bool PowerAIOptimizer::runProgressiveInference(const CameraFrame& frame, AIResult& result) {
    // Stage 1: Quick motion detection
    if (tryLightweightDetection(frame, result)) {
        if (result.confidence < 0.2f) {
            // No significant motion, return early
            result.behavior = BehaviorType::RESTING;
            result.confidence = 1.0f - result.confidence;
            result.isValidDetection = true;
            DEBUG_PRINTLN("Progressive inference: Early exit - no motion");
            return true;
        }
    }
    
    // Stage 2: Species classification if motion detected
    if (result.confidence > 0.2f || result.behavior != BehaviorType::RESTING) {
        ModelType classifierModel = selectOptimalModel(currentBatteryVoltage_, 0.8f);
        AIResult classificationResult = inferenceEngine_->runInference(frame, classifierModel);
        
        if (classificationResult.isValidDetection) {
            result.species = classificationResult.species;
            result.confidence = classificationResult.confidence;
            result.isValidDetection = true;
            
            // Stage 3: Behavior analysis for high-confidence species detection
            if (result.confidence > 0.7f && currentBatteryVoltage_ > LOW_BATTERY_THRESHOLD) {
                AIResult behaviorResult = inferenceEngine_->runInference(frame, ModelType::BEHAVIOR_ANALYZER);
                if (behaviorResult.isValidDetection) {
                    result.behavior = behaviorResult.behavior;
                    // Combine confidences
                    result.confidence = (result.confidence + behaviorResult.confidence) / 2.0f;
                }
            }
        }
    }
    
    return result.isValidDetection;
}

/**
 * Try lightweight detection for quick screening
 */
bool PowerAIOptimizer::tryLightweightDetection(const CameraFrame& frame, AIResult& result) {
    // Use motion detector as lightweight screening
    result = inferenceEngine_->runInference(frame, ModelType::MOTION_DETECTOR);
    return result.isValidDetection;
}

/**
 * Select optimal model based on power and urgency
 */
ModelType PowerAIOptimizer::selectOptimalModel(float batteryLevel, float urgency) {
    // Emergency mode - use most efficient model
    if (batteryLevel < EMERGENCY_BATTERY_THRESHOLD) {
        return ModelType::MOTION_DETECTOR;
    }
    
    // Low battery - prefer efficient models
    if (batteryLevel < LOW_BATTERY_THRESHOLD) {
        return urgency > 0.8f ? ModelType::SPECIES_CLASSIFIER : ModelType::MOTION_DETECTOR;
    }
    
    // Normal operation - use model based on urgency and time of day
    uint8_t currentHour = (millis() / 3600000) % 24;
    bool isDayTime = (currentHour >= 6 && currentHour <= 18);
    
    if (urgency > 0.9f || emergencyModeEnabled_) {
        return ModelType::SPECIES_CLASSIFIER;
    } else if (urgency > 0.5f && isDayTime) {
        return ModelType::SPECIES_CLASSIFIER;
    } else {
        return ModelType::MOTION_DETECTOR;
    }
}

/**
 * Calculate optimal sleep duration
 */
uint32_t PowerAIOptimizer::calculateOptimalSleepDuration() {
    if (!predictiveOptimizationEnabled_) {
        // Use fixed intervals based on operating mode
        switch (currentMode_) {
            case PowerAIMode::HIBERNATION:
                return 300000; // 5 minutes
            case PowerAIMode::ECO_MONITORING:
                return 60000;  // 1 minute
            case PowerAIMode::BALANCED:
                return 30000;  // 30 seconds
            case PowerAIMode::PERFORMANCE:
                return 10000;  // 10 seconds
            case PowerAIMode::RESEARCH:
                return 2000;   // 2 seconds
            default:
                return 30000;  // Default 30 seconds
        }
    }
    
    // Predictive sleep duration
    ActivityPrediction nextActivity = predictNextActivity();
    
    if (nextActivity.probabilityScore > 0.7f) {
        // High probability activity predicted
        return std::min(nextActivity.nextWakeTime_ms, (uint32_t)MAX_SLEEP_DURATION);
    } else if (nextActivity.probabilityScore > 0.3f) {
        // Moderate probability - reduce sleep time
        return std::min(nextActivity.nextWakeTime_ms / 2, (uint32_t)MAX_SLEEP_DURATION);
    } else {
        // Low probability - use standard interval
        return aiConfig_.processingInterval_ms;
    }
}

/**
 * Predict next wildlife activity
 */
ActivityPrediction PowerAIOptimizer::predictNextActivity() {
    ActivityPrediction prediction;
    
    uint8_t currentHour = (millis() / 3600000) % 24;
    uint8_t currentDay = (millis() / (24 * 3600000)) % 7;
    
    // Analyze historical activity patterns
    std::map<uint8_t, float> hourlyActivity;
    std::map<SpeciesType, float> speciesActivity;
    
    for (const auto& record : activityHistory_) {
        // Weight recent records more heavily
        float weight = 1.0f / (1.0f + (millis() - record.timestamp) / 86400000.0f); // Daily decay
        
        hourlyActivity[record.hourOfDay] += weight * record.confidence;
        speciesActivity[record.species] += weight * record.confidence;
    }
    
    // Predict next active hours
    float maxActivity = 0.0f;
    uint8_t peakHour = currentHour;
    
    for (uint8_t h = currentHour; h < currentHour + 24; h++) {
        uint8_t hour = h % 24;
        float activity = hourlyActivity[hour];
        
        // Apply environmental factors
        activity *= getSeasonalActivityFactor();
        activity *= getWeatherActivityFactor();
        
        if (activity > maxActivity) {
            maxActivity = activity;
            peakHour = hour;
        }
    }
    
    // Calculate time until peak activity
    uint32_t hoursUntilPeak = (peakHour >= currentHour) ? 
                             (peakHour - currentHour) : 
                             (24 - currentHour + peakHour);
    
    prediction.probabilityScore = std::min(1.0f, maxActivity / 10.0f); // Normalize
    prediction.nextWakeTime_ms = hoursUntilPeak * 3600000; // Convert to milliseconds
    prediction.confidenceLevel = prediction.probabilityScore;
    
    // Populate expected species
    for (const auto& species : speciesActivity) {
        if (species.second > maxActivity * 0.3f) {
            prediction.expectedSpecies.push_back(species.first);
        }
    }
    
    return prediction;
}

/**
 * Record wildlife activity for pattern learning
 */
void PowerAIOptimizer::recordWildlifeActivity(const AIResult& result) {
    if (!result.isValidDetection) return;
    
    ActivityRecord record;
    record.species = result.species;
    record.behavior = result.behavior;
    record.confidence = result.confidence;
    record.hourOfDay = (millis() / 3600000) % 24;
    record.dayOfWeek = (millis() / (24 * 3600000)) % 7;
    record.month = currentMonth_;
    record.temperature = currentTemperature_;
    record.lightLevel = currentLightLevel_;
    record.timestamp = millis();
    
    activityHistory_.push_back(record);
    
    // Limit history size
    if (activityHistory_.size() > MAX_ACTIVITY_RECORDS) {
        // Remove oldest 10% of records
        size_t removeCount = MAX_ACTIVITY_RECORDS / 10;
        activityHistory_.erase(activityHistory_.begin(), activityHistory_.begin() + removeCount);
    }
    
    DEBUG_PRINTF("Recorded activity: %s at %02d:00, confidence=%.2f\n",
                speciesTypeToString(result.species), record.hourOfDay, result.confidence);
}

/**
 * Update power state
 */
void PowerAIOptimizer::updatePowerState(float batteryVoltage, bool isCharging, float solarVoltage) {
    currentBatteryVoltage_ = batteryVoltage;
    isCharging_ = isCharging;
    solarVoltage_ = solarVoltage;
    lastPowerUpdate_ = millis();
    
    // Adapt operating mode based on power state
    if (adaptivePowerManagementEnabled_) {
        if (batteryVoltage < EMERGENCY_BATTERY_THRESHOLD && !isCharging) {
            setOperatingMode(PowerAIMode::HIBERNATION);
            emergencyModeEnabled_ = true;
        } else if (batteryVoltage < LOW_BATTERY_THRESHOLD && !isCharging) {
            setOperatingMode(PowerAIMode::ECO_MONITORING);
            emergencyModeEnabled_ = false;
        } else if (isCharging && solarVoltage > 4.0f) {
            setOperatingMode(PowerAIMode::PERFORMANCE);
            emergencyModeEnabled_ = false;
        } else {
            setOperatingMode(PowerAIMode::BALANCED);
            emergencyModeEnabled_ = false;
        }
    }
    
    // Update power budget allocation
    adjustPowerAllocation();
}

/**
 * Adjust power allocation based on current conditions
 */
void PowerAIOptimizer::adjustPowerAllocation() {
    PowerBudget adjustedBudget = powerBudget_;
    
    if (currentBatteryVoltage_ < LOW_BATTERY_THRESHOLD) {
        // Reduce AI processing allocation in low battery
        adjustedBudget.aiProcessing_mW = powerBudget_.aiProcessing_mW * 0.5f;
        adjustedBudget.networking_mW = powerBudget_.networking_mW * 0.7f;
    } else if (isCharging_ && solarVoltage_ > 4.0f) {
        // Increase AI processing when charging with good solar
        adjustedBudget.aiProcessing_mW = powerBudget_.aiProcessing_mW * 1.5f;
        adjustedBudget.networking_mW = powerBudget_.networking_mW * 1.2f;
    }
    
    // Ensure total doesn't exceed budget
    uint32_t totalAllocated = adjustedBudget.aiProcessing_mW + 
                             adjustedBudget.camera_mW + 
                             adjustedBudget.networking_mW + 
                             adjustedBudget.sensors_mW;
    
    if (totalAllocated > powerBudget_.totalBudget_mW - powerBudget_.reserve_mW) {
        float scaleFactor = (float)(powerBudget_.totalBudget_mW - powerBudget_.reserve_mW) / totalAllocated;
        adjustedBudget.aiProcessing_mW *= scaleFactor;
        adjustedBudget.networking_mW *= scaleFactor;
    }
    
    powerBudget_ = adjustedBudget;
}

/**
 * Get seasonal activity factor
 */
float PowerAIOptimizer::getSeasonalActivityFactor() {
    // Simplified seasonal model (Northern Hemisphere)
    float seasonalFactors[] = {
        0.6f, 0.7f, 0.8f, 1.0f, 1.2f, 1.3f,  // Jan-Jun
        1.4f, 1.3f, 1.1f, 0.9f, 0.7f, 0.6f   // Jul-Dec
    };
    
    return seasonalFactors[currentMonth_ % 12];
}

/**
 * Get weather activity factor
 */
float PowerAIOptimizer::getWeatherActivityFactor() {
    float factor = 1.0f;
    
    // Temperature effects
    if (currentTemperature_ < 0 || currentTemperature_ > 35) {
        factor *= 0.7f; // Extreme temperatures reduce activity
    } else if (currentTemperature_ >= 15 && currentTemperature_ <= 25) {
        factor *= 1.2f; // Optimal temperature range
    }
    
    // Precipitation effects
    if (precipitation_ > 0.1f) {
        factor *= 0.5f; // Rain reduces outdoor activity
    }
    
    // Wind effects
    if (windSpeed_ > 10.0f) {
        factor *= 0.8f; // High wind reduces activity
    }
    
    return std::max(0.1f, std::min(2.0f, factor));
}

/**
 * Check if species is high priority
 */
bool PowerAIOptimizer::isHighPrioritySpecies(SpeciesType species) {
    for (const auto& priority : detectionPriorities_) {
        if (priority.first == species) {
            return priority.second == AIWorkloadPriority::CRITICAL || 
                   priority.second == AIWorkloadPriority::HIGH;
        }
    }
    return false;
}

/**
 * Update metrics
 */
void PowerAIOptimizer::updateMetrics(const AIResult& result, float powerConsumed) {
    metrics_.totalDetections++;
    
    if (result.isValidDetection) {
        // Calculate response time (placeholder - would be actual detection to alert time)
        float responseTime = 100.0f; // ms
        metrics_.averageResponseTime_ms = (metrics_.averageResponseTime_ms * 
                                          (metrics_.totalDetections - 1) + responseTime) / 
                                          metrics_.totalDetections;
    } else {
        metrics_.missedDetections++;
    }
    
    // Update power consumption
    metrics_.averagePowerConsumption_mW = (metrics_.averagePowerConsumption_mW * 
                                          (metrics_.totalDetections - 1) + powerConsumed) / 
                                          metrics_.totalDetections;
    
    // Calculate power efficiency (detections per mW)
    if (metrics_.averagePowerConsumption_mW > 0) {
        metrics_.powerEfficiency = (float)metrics_.totalDetections / 
                                  metrics_.averagePowerConsumption_mW;
    }
    
    // Calculate battery life extension (compared to always-on operation)
    float alwaysOnPower = powerBudget_.totalBudget_mW;
    if (alwaysOnPower > 0) {
        metrics_.batteryLifeExtension_percent = 
            ((alwaysOnPower - metrics_.averagePowerConsumption_mW) / alwaysOnPower) * 100.0f;
    }
}

/**
 * Set operating mode
 */
void PowerAIOptimizer::setOperatingMode(PowerAIMode mode) {
    if (currentMode_ == mode) return;
    
    currentMode_ = mode;
    
    // Update AI configuration based on mode
    switch (mode) {
        case PowerAIMode::HIBERNATION:
            aiConfig_.processingInterval_ms = 300000; // 5 minutes
            aiConfig_.primaryModel = ModelType::MOTION_DETECTOR;
            aiConfig_.confidenceThreshold = 0.8f;
            aiConfig_.maxInferenceTime_ms = 1000;
            break;
            
        case PowerAIMode::ECO_MONITORING:
            aiConfig_.processingInterval_ms = 60000; // 1 minute
            aiConfig_.primaryModel = ModelType::MOTION_DETECTOR;
            aiConfig_.confidenceThreshold = 0.7f;
            aiConfig_.maxInferenceTime_ms = 1500;
            break;
            
        case PowerAIMode::BALANCED:
            aiConfig_.processingInterval_ms = 10000; // 10 seconds
            aiConfig_.primaryModel = ModelType::SPECIES_CLASSIFIER;
            aiConfig_.confidenceThreshold = 0.6f;
            aiConfig_.maxInferenceTime_ms = 2000;
            break;
            
        case PowerAIMode::PERFORMANCE:
            aiConfig_.processingInterval_ms = 2000; // 2 seconds
            aiConfig_.primaryModel = ModelType::SPECIES_CLASSIFIER;
            aiConfig_.confidenceThreshold = 0.5f;
            aiConfig_.maxInferenceTime_ms = 3000;
            break;
            
        case PowerAIMode::RESEARCH:
            aiConfig_.processingInterval_ms = 1000; // 1 second
            aiConfig_.primaryModel = ModelType::SPECIES_CLASSIFIER;
            aiConfig_.confidenceThreshold = 0.4f;
            aiConfig_.maxInferenceTime_ms = 5000;
            break;
            
        case PowerAIMode::ADAPTIVE:
            // Configuration will be dynamically adjusted
            adaptConfigurationToConditions();
            break;
    }
    
    DEBUG_PRINTF("Operating mode changed to: %s\n", powerAIModeToString(mode));
}

/**
 * Get performance metrics
 */
PowerAIMetrics PowerAIOptimizer::getPerformanceMetrics() const {
    return metrics_;
}

/**
 * Print system status
 */
void PowerAIOptimizer::printSystemStatus() {
    DEBUG_PRINTLN("=== Power-AI Optimizer Status ===");
    DEBUG_PRINTF("Operating Mode: %s\n", powerAIModeToString(currentMode_));
    DEBUG_PRINTF("Battery: %.2fV (%s)\n", currentBatteryVoltage_, 
                isCharging_ ? "charging" : "discharging");
    DEBUG_PRINTF("Power Budget: %u mW (AI: %u mW)\n", 
                powerBudget_.totalBudget_mW, powerBudget_.aiProcessing_mW);
    DEBUG_PRINTF("Processing Interval: %u ms\n", aiConfig_.processingInterval_ms);
    DEBUG_PRINTF("Primary Model: %s\n", modelTypeToString(aiConfig_.primaryModel));
    DEBUG_PRINTF("Total Detections: %u (missed: %u)\n", 
                metrics_.totalDetections, metrics_.missedDetections);
    DEBUG_PRINTF("Power Efficiency: %.2f detections/mW\n", metrics_.powerEfficiency);
    DEBUG_PRINTF("Battery Life Extension: %.1f%%\n", metrics_.batteryLifeExtension_percent);
    DEBUG_PRINTLN("================================");
}

/**
 * Utility function implementations
 */
const char* powerAIModeToString(PowerAIMode mode) {
    switch (mode) {
        case PowerAIMode::HIBERNATION: return "Hibernation";
        case PowerAIMode::ECO_MONITORING: return "Eco Monitoring";
        case PowerAIMode::BALANCED: return "Balanced";
        case PowerAIMode::PERFORMANCE: return "Performance";
        case PowerAIMode::RESEARCH: return "Research";
        case PowerAIMode::ADAPTIVE: return "Adaptive";
        default: return "Unknown";
    }
}

const char* aiWorkloadPriorityToString(AIWorkloadPriority priority) {
    switch (priority) {
        case AIWorkloadPriority::CRITICAL: return "Critical";
        case AIWorkloadPriority::HIGH: return "High";
        case AIWorkloadPriority::MEDIUM: return "Medium";
        case AIWorkloadPriority::LOW: return "Low";
        case AIWorkloadPriority::BACKGROUND: return "Background";
        default: return "Unknown";
    }
}