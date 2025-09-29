/**
 * @file motion_coordinator.cpp
 * @brief Enhanced Motion Detection Coordinator Implementation
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include "motion_coordinator.h"
#include <ArduinoJson.h>
#include <algorithm>

MotionCoordinator::MotionCoordinator()
    : currentMethod_(DetectionMethod::ADAPTIVE)
    , initialized_(false)
    , consecutiveFailures_(0)
    , lastDetectionTime_(0) {
}

MotionCoordinator::~MotionCoordinator() {
    // Cleanup managed by smart pointers
}

bool MotionCoordinator::initialize(CameraManager* cameraManager, const CoordinatorConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    
    // Initialize motion detection manager
    motionManager_ = std::make_unique<MotionDetectionManager>();
    if (!motionManager_->initialize(cameraManager, MotionDetectionManager::DetectionSystem::ENHANCED_HYBRID)) {
        Serial.println("MotionCoordinator: Failed to initialize motion detection manager");
        return false;
    }
    
    // Initialize adaptive processor if performance optimization is enabled
    if (config_.enablePerformanceOptimization) {
        adaptiveProcessor_ = std::make_unique<AdaptiveProcessor>();
        if (!adaptiveProcessor_->initialize()) {
            Serial.println("MotionCoordinator: Failed to initialize adaptive processor");
            return false;
        }
    }
    
    // Initialize wildlife analyzer if enabled
    if (config_.enableWildlifeAnalysis) {
        wildlifeAnalyzer_ = std::make_unique<WildlifeAnalyzer>();
        if (!wildlifeAnalyzer_->initialize()) {
            Serial.println("MotionCoordinator: Failed to initialize wildlife analyzer");
            return false;
        }
    }
    
    // Initialize statistics
    resetStatistics();
    
    // Reserve space for processing times
    processingTimes_.reserve(100);
    
    currentMethod_ = config_.defaultMethod;
    initialized_ = true;
    
    Serial.println("MotionCoordinator: Initialized successfully");
    return true;
}

MotionCoordinator::CoordinatorResult MotionCoordinator::detectMotion(const camera_fb_t* frameBuffer,
                                                                    const EnvironmentalConditions& conditions) {
    if (!initialized_) {
        return CoordinatorResult{};
    }
    
    uint32_t startTime = millis();
    CoordinatorResult result;
    result.timestamp = startTime;
    lastConditions_ = conditions;
    
    try {
        // Get processing decision from adaptive processor
        AdaptiveProcessor::ProcessingDecision processingDecision;
        if (adaptiveProcessor_) {
            processingDecision = adaptiveProcessor_->getProcessingDecision(
                conditions.batteryVoltage, conditions.temperature, conditions.lightLevel);
            result.processingDecision = processingDecision;
            
            // Skip frame if recommended
            if (processingDecision.skipFrame) {
                result.description = "Frame skipped for performance optimization";
                return result;
            }
        }
        
        // Determine optimal detection method
        if (config_.defaultMethod == DetectionMethod::ADAPTIVE) {
            currentMethod_ = determineOptimalMethod(conditions, processingDecision);
        } else {
            currentMethod_ = config_.defaultMethod;
        }
        result.methodUsed = currentMethod_;
        
        // Perform base motion detection
        MotionDetectionManager::UnifiedMotionResult baseResult = motionManager_->detectMotion();
        
        // Extract basic results
        result.motionDetected = baseResult.motionDetected;
        result.pirTriggered = baseResult.pirTriggered;
        result.frameMotionDetected = baseResult.frameMotionDetected;
        result.overallConfidence = baseResult.confidenceScore;
        
        // Update adaptive processor with detection data
        if (adaptiveProcessor_) {
            adaptiveProcessor_->updateActivity(result.motionDetected, result.overallConfidence, 
                                             millis() - startTime);
        }
        
        // Perform wildlife analysis if enabled and motion detected
        WildlifeAnalyzer::WildlifeAnalysisResult wildlifeResult;
        if (wildlifeAnalyzer_ && result.motionDetected) {
            // Update wildlife analyzer with motion data
            updateWildlifeAnalysis(baseResult);
            
            // Analyze pattern
            wildlifeResult = wildlifeAnalyzer_->analyzePattern(conditions.currentHour,
                                                              conditions.temperature,
                                                              conditions.lightLevel);
            result.wildlifeAnalysis = wildlifeResult;
        }
        
        // Fuse all detection results
        result = fuseDetectionResults(baseResult, wildlifeResult, processingDecision, conditions);
        
        // Make final decisions
        result.shouldCapture = shouldCapture(result);
        result.shouldSave = shouldSave(result);
        result.shouldTransmit = shouldTransmit(result);
        result.shouldAlert = shouldAlert(result);
        
        // Calculate processing time
        result.processingTime = millis() - startTime;
        
        // Update statistics
        updateStatistics(result);
        
        // Handle success
        if (result.motionDetected) {
            handleDetectionSuccess();
            
            // Trigger callback if set
            if (motionCallback_) {
                motionCallback_(result);
            }
        }
        
        // Log detection event
        logDetectionEvent(result);
        
    } catch (const std::exception& e) {
        Serial.printf("MotionCoordinator: Detection failed - %s\n", e.what());
        handleDetectionFailure();
        result.description = "Detection failed: " + String(e.what());
    }
    
    return result;
}

void MotionCoordinator::updateEnvironmentalConditions(const EnvironmentalConditions& conditions) {
    lastConditions_ = conditions;
    
    // Update components with new conditions
    if (wildlifeAnalyzer_ && conditions.currentHour != lastConditions_.currentHour) {
        // Hour changed, update time patterns
        wildlifeAnalyzer_->updateTimePattern(conditions.currentHour, 
                                           WildlifeAnalyzer::MovementPattern::UNKNOWN, 0.1f);
    }
}

void MotionCoordinator::configure(const CoordinatorConfig& config) {
    config_ = config;
    currentMethod_ = config.defaultMethod;
    
    // Update component configurations
    if (adaptiveProcessor_) {
        AdaptiveProcessor::AdaptiveConfig adaptiveConfig;
        adaptiveConfig.enabled = config.enablePerformanceOptimization;
        adaptiveConfig.maxProcessingTimeMs = config.maxProcessingTimeMs;
        adaptiveConfig.useEnvironmentalAdaptation = config.useEnvironmentalAdaptation;
        adaptiveConfig.useTimeOfDayAdaptation = config.useTimeOfDayAdaptation;
        adaptiveProcessor_->configure(adaptiveConfig);
    }
    
    if (wildlifeAnalyzer_) {
        WildlifeAnalyzer::AnalyzerConfig wildlifeConfig;
        wildlifeConfig.enabled = config.enableWildlifeAnalysis;
        wildlifeConfig.useTimeOfDayAdaptation = config.useTimeOfDayAdaptation;
        wildlifeConfig.captureThreshold = config.captureThreshold;
        wildlifeAnalyzer_->configure(wildlifeConfig);
    }
    
    Serial.println("MotionCoordinator: Configuration updated");
}

void MotionCoordinator::resetStatistics() {
    stats_ = CoordinatorStats{};
    stats_.lastResetTime = millis();
    processingTimes_.clear();
    consecutiveFailures_ = 0;
}

String MotionCoordinator::getConfigJSON() const {
    DynamicJsonDocument doc(2048);
    
    doc["enabled"] = config_.enabled;
    doc["defaultMethod"] = (int)config_.defaultMethod;
    doc["pirWeight"] = config_.pirWeight;
    doc["frameWeight"] = config_.frameWeight;
    doc["aiWeight"] = config_.aiWeight;
    doc["motionConfidenceThreshold"] = config_.motionConfidenceThreshold;
    doc["captureThreshold"] = config_.captureThreshold;
    doc["transmitThreshold"] = config_.transmitThreshold;
    doc["alertThreshold"] = config_.alertThreshold;
    doc["useEnvironmentalAdaptation"] = config_.useEnvironmentalAdaptation;
    doc["useTimeOfDayAdaptation"] = config_.useTimeOfDayAdaptation;
    doc["useWeatherCompensation"] = config_.useWeatherCompensation;
    doc["maxProcessingTimeMs"] = config_.maxProcessingTimeMs;
    doc["enablePerformanceOptimization"] = config_.enablePerformanceOptimization;
    doc["enableWildlifeAnalysis"] = config_.enableWildlifeAnalysis;
    doc["maxConsecutiveFailures"] = config_.maxConsecutiveFailures;
    doc["recoveryDelayMs"] = config_.recoveryDelayMs;
    doc["enableFallbackMethods"] = config_.enableFallbackMethods;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool MotionCoordinator::loadConfigJSON(const String& jsonConfig) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonConfig);
    
    if (error) {
        Serial.println("MotionCoordinator: Failed to parse JSON config");
        return false;
    }
    
    if (doc.containsKey("enabled")) config_.enabled = doc["enabled"];
    if (doc.containsKey("defaultMethod")) config_.defaultMethod = (DetectionMethod)doc["defaultMethod"].as<int>();
    if (doc.containsKey("pirWeight")) config_.pirWeight = doc["pirWeight"];
    if (doc.containsKey("frameWeight")) config_.frameWeight = doc["frameWeight"];
    if (doc.containsKey("aiWeight")) config_.aiWeight = doc["aiWeight"];
    if (doc.containsKey("motionConfidenceThreshold")) config_.motionConfidenceThreshold = doc["motionConfidenceThreshold"];
    if (doc.containsKey("captureThreshold")) config_.captureThreshold = doc["captureThreshold"];
    if (doc.containsKey("transmitThreshold")) config_.transmitThreshold = doc["transmitThreshold"];
    if (doc.containsKey("alertThreshold")) config_.alertThreshold = doc["alertThreshold"];
    if (doc.containsKey("useEnvironmentalAdaptation")) config_.useEnvironmentalAdaptation = doc["useEnvironmentalAdaptation"];
    if (doc.containsKey("useTimeOfDayAdaptation")) config_.useTimeOfDayAdaptation = doc["useTimeOfDayAdaptation"];
    if (doc.containsKey("useWeatherCompensation")) config_.useWeatherCompensation = doc["useWeatherCompensation"];
    if (doc.containsKey("maxProcessingTimeMs")) config_.maxProcessingTimeMs = doc["maxProcessingTimeMs"];
    if (doc.containsKey("enablePerformanceOptimization")) config_.enablePerformanceOptimization = doc["enablePerformanceOptimization"];
    if (doc.containsKey("enableWildlifeAnalysis")) config_.enableWildlifeAnalysis = doc["enableWildlifeAnalysis"];
    if (doc.containsKey("maxConsecutiveFailures")) config_.maxConsecutiveFailures = doc["maxConsecutiveFailures"];
    if (doc.containsKey("recoveryDelayMs")) config_.recoveryDelayMs = doc["recoveryDelayMs"];
    if (doc.containsKey("enableFallbackMethods")) config_.enableFallbackMethods = doc["enableFallbackMethods"];
    
    Serial.println("MotionCoordinator: Configuration loaded from JSON");
    return true;
}

String MotionCoordinator::getStatusReport() const {
    DynamicJsonDocument doc(2048);
    
    doc["initialized"] = initialized_;
    doc["currentMethod"] = getMethodString(currentMethod_);
    doc["consecutiveFailures"] = consecutiveFailures_;
    doc["lastDetectionTime"] = lastDetectionTime_;
    
    JsonObject stats = doc.createNestedObject("statistics");
    stats["totalDetections"] = stats_.totalDetections;
    stats["pirDetections"] = stats_.pirDetections;
    stats["frameDetections"] = stats_.frameDetections;
    stats["aiDetections"] = stats_.aiDetections;
    stats["wildlifeDetections"] = stats_.wildlifeDetections;
    stats["captures"] = stats_.captures;
    stats["transmissions"] = stats_.transmissions;
    stats["falsePositives"] = stats_.falsePositives;
    stats["averageProcessingTime"] = stats_.averageProcessingTime;
    stats["overallAccuracy"] = stats_.overallAccuracy;
    stats["powerSavings"] = stats_.powerSavings;
    
    if (adaptiveProcessor_) {
        JsonObject adaptive = doc.createNestedObject("adaptiveProcessor");
        auto metrics = adaptiveProcessor_->getPerformanceMetrics();
        adaptive["currentLevel"] = (int)adaptiveProcessor_->getCurrentProcessingLevel();
        adaptive["currentActivity"] = (int)adaptiveProcessor_->getCurrentActivityLevel();
        adaptive["averageProcessingTime"] = metrics.averageProcessingTime;
        adaptive["powerSavings"] = metrics.powerSavings;
    }
    
    if (wildlifeAnalyzer_) {
        doc["wildlifeStats"] = wildlifeAnalyzer_->getAnalysisStats();
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

void MotionCoordinator::setWildlifeAnalysisEnabled(bool enabled) {
    config_.enableWildlifeAnalysis = enabled;
    if (!enabled && wildlifeAnalyzer_) {
        wildlifeAnalyzer_.reset();
    } else if (enabled && !wildlifeAnalyzer_) {
        wildlifeAnalyzer_ = std::make_unique<WildlifeAnalyzer>();
        wildlifeAnalyzer_->initialize();
    }
}

void MotionCoordinator::setPerformanceOptimizationEnabled(bool enabled) {
    config_.enablePerformanceOptimization = enabled;
    if (!enabled && adaptiveProcessor_) {
        adaptiveProcessor_.reset();
    } else if (enabled && !adaptiveProcessor_) {
        adaptiveProcessor_ = std::make_unique<AdaptiveProcessor>();
        adaptiveProcessor_->initialize();
    }
}

void MotionCoordinator::trainWildlifeAnalyzer(WildlifeAnalyzer::MovementPattern pattern,
                                             const WildlifeAnalyzer::MovementCharacteristics& characteristics,
                                             float confidence) {
    if (wildlifeAnalyzer_) {
        wildlifeAnalyzer_->learnPattern(pattern, characteristics, confidence);
    }
}

// Private method implementations

MotionCoordinator::DetectionMethod MotionCoordinator::determineOptimalMethod(const EnvironmentalConditions& conditions,
                                                                            const AdaptiveProcessor::ProcessingDecision& decision) {
    // Use adaptive processor recommendation if available
    if (adaptiveProcessor_) {
        switch (decision.level) {
            case AdaptiveProcessor::ProcessingLevel::MINIMAL:
                return DetectionMethod::PIR_ONLY;
            case AdaptiveProcessor::ProcessingLevel::REDUCED:
                return DetectionMethod::HYBRID_BASIC;
            case AdaptiveProcessor::ProcessingLevel::NORMAL:
                return DetectionMethod::HYBRID_AI;
            case AdaptiveProcessor::ProcessingLevel::ENHANCED:
            case AdaptiveProcessor::ProcessingLevel::MAXIMUM:
                return DetectionMethod::FULL_FUSION;
        }
    }
    
    // Fallback method selection based on conditions
    if (conditions.batteryVoltage < 3.2f) {
        return DetectionMethod::PIR_ONLY;
    }
    
    if (conditions.isNight && conditions.lightLevel < 0.1f) {
        return DetectionMethod::HYBRID_BASIC; // Frame analysis less reliable in dark
    }
    
    if (conditions.isWeatherActive) {
        return DetectionMethod::HYBRID_AI; // AI helps filter weather-induced motion
    }
    
    return DetectionMethod::FULL_FUSION;
}

MotionCoordinator::CoordinatorResult MotionCoordinator::fuseDetectionResults(
    const MotionDetectionManager::UnifiedMotionResult& baseResult,
    const WildlifeAnalyzer::WildlifeAnalysisResult& wildlifeResult,
    const AdaptiveProcessor::ProcessingDecision& processingDecision,
    const EnvironmentalConditions& conditions) {
    
    CoordinatorResult result;
    
    // Copy basic detection results
    result.motionDetected = baseResult.motionDetected;
    result.pirTriggered = baseResult.pirTriggered;
    result.frameMotionDetected = baseResult.frameMotionDetected;
    result.overallConfidence = baseResult.confidenceScore;
    result.wildlifeAnalysis = wildlifeResult;
    result.processingDecision = processingDecision;
    
    // Calculate fusion confidence
    result.fusionConfidence = calculateFusionConfidence(baseResult, wildlifeResult);
    
    // Apply environmental adjustments
    float environmentalAdjustment = getEnvironmentalConfidenceAdjustment(conditions);
    result.overallConfidence *= environmentalAdjustment;
    result.fusionConfidence *= environmentalAdjustment;
    
    // Build active methods list
    result.activeMethods.clear();
    if (baseResult.pirTriggered) result.activeMethods.push_back("PIR");
    if (baseResult.frameMotionDetected) result.activeMethods.push_back("Frame");
    if (baseResult.hasEnhancedData) result.activeMethods.push_back("Enhanced");
    if (wildlifeResult.isWildlife) result.activeMethods.push_back("Wildlife");
    
    // Build description
    result.description = "Methods: ";
    for (size_t i = 0; i < result.activeMethods.size(); i++) {
        result.description += result.activeMethods[i];
        if (i < result.activeMethods.size() - 1) result.description += ", ";
    }
    
    if (wildlifeResult.isWildlife) {
        result.description += " | Wildlife: " + wildlifeResult.description;
    }
    
    // Build reasoning
    result.reasoning = "Fusion confidence: " + String(result.fusionConfidence, 2);
    if (environmentalAdjustment != 1.0f) {
        result.reasoning += ", Environmental adj: " + String(environmentalAdjustment, 2);
    }
    
    return result;
}

float MotionCoordinator::calculateFusionConfidence(const MotionDetectionManager::UnifiedMotionResult& baseResult,
                                                  const WildlifeAnalyzer::WildlifeAnalysisResult& wildlifeResult) {
    float fusedConfidence = 0.0f;
    float totalWeight = 0.0f;
    
    // PIR contribution
    if (baseResult.pirTriggered) {
        fusedConfidence += config_.pirWeight * baseResult.confidenceScore;
        totalWeight += config_.pirWeight;
    }
    
    // Frame analysis contribution
    if (baseResult.frameMotionDetected) {
        fusedConfidence += config_.frameWeight * baseResult.confidenceScore;
        totalWeight += config_.frameWeight;
    }
    
    // AI/Enhanced contribution
    if (baseResult.hasEnhancedData && baseResult.mlConfidence > 0.0f) {
        fusedConfidence += config_.aiWeight * baseResult.mlConfidence;
        totalWeight += config_.aiWeight;
    }
    
    // Wildlife analysis contribution
    if (wildlifeResult.isWildlife) {
        float wildlifeWeight = 0.2f; // Additional weight for wildlife confirmation
        fusedConfidence += wildlifeWeight * wildlifeResult.confidence;
        totalWeight += wildlifeWeight;
    }
    
    return totalWeight > 0.0f ? fusedConfidence / totalWeight : baseResult.confidenceScore;
}

void MotionCoordinator::updateStatistics(const CoordinatorResult& result) {
    stats_.totalDetections++;
    
    if (result.pirTriggered) stats_.pirDetections++;
    if (result.frameMotionDetected) stats_.frameDetections++;
    if (result.wildlifeAnalysis.isWildlife) stats_.wildlifeDetections++;
    if (result.shouldCapture) stats_.captures++;
    if (result.shouldTransmit) stats_.transmissions++;
    
    // Update processing time statistics
    processingTimes_.push_back(result.processingTime);
    if (processingTimes_.size() > 100) {
        processingTimes_.erase(processingTimes_.begin());
    }
    
    // Calculate average processing time
    uint32_t sum = 0;
    for (uint32_t time : processingTimes_) {
        sum += time;
    }
    stats_.averageProcessingTime = processingTimes_.empty() ? 0 : sum / processingTimes_.size();
    
    // Calculate accuracy (simplified)
    if (stats_.totalDetections > 0) {
        stats_.overallAccuracy = (float)(stats_.totalDetections - stats_.falsePositives) / stats_.totalDetections;
    }
    
    // Update power savings from adaptive processor
    if (adaptiveProcessor_) {
        stats_.powerSavings = adaptiveProcessor_->getPerformanceMetrics().powerSavings;
    }
}

void MotionCoordinator::handleDetectionFailure() {
    consecutiveFailures_++;
    stats_.failureCount++;
    
    if (consecutiveFailures_ >= config_.maxConsecutiveFailures) {
        Serial.printf("MotionCoordinator: %d consecutive failures, entering recovery mode\n", consecutiveFailures_);
        
        if (config_.enableFallbackMethods) {
            // Switch to most reliable method
            currentMethod_ = DetectionMethod::PIR_ONLY;
            Serial.println("MotionCoordinator: Switched to PIR-only fallback mode");
        }
        
        delay(config_.recoveryDelayMs);
    }
}

void MotionCoordinator::handleDetectionSuccess() {
    consecutiveFailures_ = 0;
    lastDetectionTime_ = millis();
}

bool MotionCoordinator::shouldCapture(const CoordinatorResult& result) {
    return result.fusionConfidence >= config_.captureThreshold &&
           result.motionDetected &&
           (!result.wildlifeAnalysis.isWildlife || result.wildlifeAnalysis.shouldCapture);
}

bool MotionCoordinator::shouldSave(const CoordinatorResult& result) {
    return result.shouldCapture && result.fusionConfidence >= config_.captureThreshold;
}

bool MotionCoordinator::shouldTransmit(const CoordinatorResult& result) {
    return result.shouldSave && result.fusionConfidence >= config_.transmitThreshold;
}

bool MotionCoordinator::shouldAlert(const CoordinatorResult& result) {
    return result.fusionConfidence >= config_.alertThreshold ||
           (result.wildlifeAnalysis.isWildlife && result.wildlifeAnalysis.shouldTriggerAlert);
}

float MotionCoordinator::getEnvironmentalConfidenceAdjustment(const EnvironmentalConditions& conditions) {
    if (!config_.useEnvironmentalAdaptation) {
        return 1.0f;
    }
    
    float adjustment = 1.0f;
    
    // Battery level adjustment
    if (conditions.batteryVoltage < 3.3f) {
        adjustment *= 0.9f; // Slightly reduce confidence to save power
    }
    
    // Temperature adjustment
    if (conditions.temperature < 0.0f || conditions.temperature > 40.0f) {
        adjustment *= 0.95f; // Extreme temperatures may affect sensors
    }
    
    // Light level adjustment
    if (conditions.lightLevel < 0.1f) {
        adjustment *= 0.9f; // Low light reduces frame analysis reliability
    }
    
    // Weather adjustment
    if (config_.useWeatherCompensation && conditions.isWeatherActive) {
        adjustment *= 0.85f; // Weather increases false positives
    }
    
    // Time of day adjustment
    if (config_.useTimeOfDayAdaptation) {
        adjustment *= getTimeOfDayAdjustment(conditions.currentHour);
    }
    
    return std::max(0.5f, std::min(1.2f, adjustment));
}

float MotionCoordinator::getTimeOfDayAdjustment(uint8_t hour) {
    // Higher confidence during peak wildlife activity times
    if (hour >= 6 && hour <= 8) return 1.1f;   // Dawn
    if (hour >= 17 && hour <= 19) return 1.15f; // Dusk
    if (hour >= 0 && hour <= 5) return 0.9f;    // Deep night
    if (hour >= 12 && hour <= 15) return 0.95f; // Midday
    return 1.0f; // Normal hours
}

String MotionCoordinator::getMethodString(DetectionMethod method) const {
    switch (method) {
        case DetectionMethod::PIR_ONLY: return "PIR Only";
        case DetectionMethod::FRAME_ONLY: return "Frame Only";
        case DetectionMethod::AI_ONLY: return "AI Only";
        case DetectionMethod::HYBRID_BASIC: return "Hybrid Basic";
        case DetectionMethod::HYBRID_AI: return "Hybrid AI";
        case DetectionMethod::FULL_FUSION: return "Full Fusion";
        case DetectionMethod::ADAPTIVE: return "Adaptive";
        default: return "Unknown";
    }
}

void MotionCoordinator::logDetectionEvent(const CoordinatorResult& result) {
    if (result.motionDetected) {
        Serial.printf("MotionCoordinator: Detection - Method: %s, Confidence: %.2f, Wildlife: %s, Capture: %s\n",
                     getMethodString(result.methodUsed).c_str(),
                     result.fusionConfidence,
                     result.wildlifeAnalysis.isWildlife ? "Yes" : "No",
                     result.shouldCapture ? "Yes" : "No");
    }
}

void MotionCoordinator::updateWildlifeAnalysis(const MotionDetectionManager::UnifiedMotionResult& result) {
    if (!wildlifeAnalyzer_ || !result.motionDetected) return;
    
    // Create motion data point from detection result
    WildlifeAnalyzer::MotionDataPoint dataPoint;
    dataPoint.timestamp = millis();
    dataPoint.x = 160; // Default to frame center if no specific coordinates
    dataPoint.y = 120;
    dataPoint.width = 80;  // Default size
    dataPoint.height = 60;
    dataPoint.intensity = result.confidenceScore;
    dataPoint.confidence = result.confidenceScore;
    
    // Use enhanced data if available
    if (result.hasEnhancedData) {
        // Enhanced result may have specific location data
        // This would need to be extracted from the enhanced detection system
    }
    
    wildlifeAnalyzer_->addMotionData(dataPoint);
}