/**
 * @file adaptive_processor.cpp
 * @brief Adaptive Processing Manager Implementation
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#include "adaptive_processor.h"
#include <ArduinoJson.h>
#include <algorithm>

AdaptiveProcessor::AdaptiveProcessor() 
    : currentLevel_(ProcessingLevel::NORMAL)
    , currentActivity_(ActivityLevel::LOW)
    , initialized_(false)
    , lastDetectionTime_(0)
    , lastProcessingTime_(0)
    , roiUpdateCount_(0) {
}

AdaptiveProcessor::~AdaptiveProcessor() {
    // Cleanup if needed
}

bool AdaptiveProcessor::initialize(const AdaptiveConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    
    // Initialize memory manager
    memoryManager_ = std::make_shared<MemoryPoolManager>();
    if (!memoryManager_->initialize(true)) {
        Serial.println("AdaptiveProcessor: Warning - Memory manager initialization failed");
        // Continue without memory management
    }
    
    // Initialize ROI to center region
    roiConfig_.enabled = false;
    roiConfig_.x = 80;  // Center of 320px width
    roiConfig_.y = 60;  // Center of 240px height
    roiConfig_.width = 160;  // Half width
    roiConfig_.height = 120; // Half height
    roiConfig_.confidence = 0.0f;
    roiConfig_.lastUpdate = 0;
    
    // Initialize frame rate config
    frameRateConfig_.normalIntervalMs = 1000;
    frameRateConfig_.lowActivityIntervalMs = 5000;
    frameRateConfig_.dormantIntervalMs = 30000;
    frameRateConfig_.maxFrameSkips = 10;
    frameRateConfig_.qualityReductionFactor = 0.8f;
    
    // Reset metrics
    resetMetrics();
    
    // Reserve space for tracking vectors
    detectionTimes_.reserve(100);
    confidenceHistory_.reserve(100);
    processingTimes_.reserve(100);
    detectionX_.reserve(50);
    detectionY_.reserve(50);
    
    initialized_ = true;
    Serial.println("AdaptiveProcessor: Initialized successfully");
    
    return true;
}

void AdaptiveProcessor::updateActivity(bool detectionOccurred, float confidence, uint32_t processingTime) {
    if (!initialized_) return;
    
    uint32_t currentTime = millis();
    
    if (detectionOccurred) {
        // Record detection event
        detectionTimes_.push_back(currentTime);
        confidenceHistory_.push_back(confidence);
        lastDetectionTime_ = currentTime;
        
        // Limit history size
        if (detectionTimes_.size() > 100) {
            detectionTimes_.erase(detectionTimes_.begin());
            confidenceHistory_.erase(confidenceHistory_.begin());
        }
    }
    
    // Record processing time
    processingTimes_.push_back(processingTime);
    lastProcessingTime_ = processingTime;
    
    if (processingTimes_.size() > 50) {
        processingTimes_.erase(processingTimes_.begin());
    }
    
    // Update metrics
    updateMetrics(processingTime);
    
    // Reassess activity level
    currentActivity_ = assessActivityLevel();
    
    // Clean up old data periodically
    if (currentTime % 60000 == 0) { // Every minute
        cleanupOldData();
    }
}

AdaptiveProcessor::ProcessingDecision AdaptiveProcessor::getProcessingDecision(float batteryVoltage, 
                                                                              float temperature, 
                                                                              float lightLevel) {
    if (!initialized_) {
        return {ProcessingLevel::NORMAL, false, false, 500, 1.0f, "Not initialized"};
    }
    
    ProcessingDecision decision;
    
    // Determine optimal processing level
    decision.level = determineOptimalLevel(batteryVoltage, temperature, lightLevel);
    
    // Determine ROI usage
    decision.useROI = roiConfig_.enabled && roiConfig_.confidence > 0.3f;
    
    // Determine frame skipping
    decision.skipFrame = shouldSkipFrame();
    
    // Set processing time limit
    switch (decision.level) {
        case ProcessingLevel::MINIMAL:
            decision.maxProcessingTime = 100;
            decision.qualityFactor = 0.6f;
            break;
        case ProcessingLevel::REDUCED:
            decision.maxProcessingTime = 200;
            decision.qualityFactor = 0.7f;
            break;
        case ProcessingLevel::NORMAL:
            decision.maxProcessingTime = 350;
            decision.qualityFactor = 0.85f;
            break;
        case ProcessingLevel::ENHANCED:
            decision.maxProcessingTime = 500;
            decision.qualityFactor = 1.0f;
            break;
        case ProcessingLevel::MAXIMUM:
            decision.maxProcessingTime = 800;
            decision.qualityFactor = 1.0f;
            break;
    }
    
    // Apply quality reduction during low activity
    if (currentActivity_ == ActivityLevel::LOW || currentActivity_ == ActivityLevel::DORMANT) {
        decision.qualityFactor *= frameRateConfig_.qualityReductionFactor;
    }
    
    // Build reasoning string
    decision.reasoning = "Level: " + getProcessingLevelString(decision.level) + 
                        ", Activity: " + getActivityLevelString(currentActivity_) +
                        ", Battery: " + String(batteryVoltage, 2) + "V";
    
    if (decision.useROI) {
        decision.reasoning += ", ROI: " + String(roiConfig_.confidence, 2);
    }
    
    currentLevel_ = decision.level;
    
    return decision;
}

void AdaptiveProcessor::updateROI(uint16_t detectionX, uint16_t detectionY, 
                                 uint16_t detectionWidth, uint16_t detectionHeight, 
                                 float confidence) {
    if (!initialized_ || confidence < 0.3f) return;
    
    // Store detection location
    detectionX_.push_back(detectionX);
    detectionY_.push_back(detectionY);
    
    // Limit history
    if (detectionX_.size() > 50) {
        detectionX_.erase(detectionX_.begin());
        detectionY_.erase(detectionY_.begin());
    }
    
    roiUpdateCount_++;
    
    // Update ROI every 5 detections
    if (roiUpdateCount_ % 5 == 0) {
        updateROIInternal();
        metrics_.roiOptimizations++;
    }
}

void AdaptiveProcessor::resetMetrics() {
    metrics_.totalProcessingTime = 0;
    metrics_.averageProcessingTime = 0;
    metrics_.peakProcessingTime = 0;
    metrics_.frameSkips = 0;
    metrics_.roiOptimizations = 0;
    metrics_.levelChanges = 0;
    metrics_.powerSavings = 0.0f;
    metrics_.lastResetTime = millis();
}

String AdaptiveProcessor::getConfigJSON() const {
    DynamicJsonDocument doc(1024);
    
    doc["enabled"] = config_.enabled;
    doc["activityWindowMinutes"] = config_.activityWindowMinutes;
    doc["dormantThresholdMinutes"] = config_.dormantThresholdMinutes;
    doc["batteryLowThreshold"] = config_.batteryLowThreshold;
    doc["temperatureOptimalMin"] = config_.temperatureOptimalMin;
    doc["temperatureOptimalMax"] = config_.temperatureOptimalMax;
    doc["useEnvironmentalAdaptation"] = config_.useEnvironmentalAdaptation;
    doc["useTimeOfDayAdaptation"] = config_.useTimeOfDayAdaptation;
    doc["maxProcessingTimeMs"] = config_.maxProcessingTimeMs;
    
    JsonObject roi = doc.createNestedObject("roi");
    roi["enabled"] = roiConfig_.enabled;
    roi["x"] = roiConfig_.x;
    roi["y"] = roiConfig_.y;
    roi["width"] = roiConfig_.width;
    roi["height"] = roiConfig_.height;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool AdaptiveProcessor::loadConfigJSON(const String& jsonConfig) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonConfig);
    
    if (error) {
        Serial.println("AdaptiveProcessor: Failed to parse JSON config");
        return false;
    }
    
    if (doc.containsKey("enabled")) config_.enabled = doc["enabled"];
    if (doc.containsKey("activityWindowMinutes")) config_.activityWindowMinutes = doc["activityWindowMinutes"];
    if (doc.containsKey("dormantThresholdMinutes")) config_.dormantThresholdMinutes = doc["dormantThresholdMinutes"];
    if (doc.containsKey("batteryLowThreshold")) config_.batteryLowThreshold = doc["batteryLowThreshold"];
    if (doc.containsKey("temperatureOptimalMin")) config_.temperatureOptimalMin = doc["temperatureOptimalMin"];
    if (doc.containsKey("temperatureOptimalMax")) config_.temperatureOptimalMax = doc["temperatureOptimalMax"];
    if (doc.containsKey("useEnvironmentalAdaptation")) config_.useEnvironmentalAdaptation = doc["useEnvironmentalAdaptation"];
    if (doc.containsKey("useTimeOfDayAdaptation")) config_.useTimeOfDayAdaptation = doc["useTimeOfDayAdaptation"];
    if (doc.containsKey("maxProcessingTimeMs")) config_.maxProcessingTimeMs = doc["maxProcessingTimeMs"];
    
    if (doc.containsKey("roi")) {
        JsonObject roi = doc["roi"];
        if (roi.containsKey("enabled")) roiConfig_.enabled = roi["enabled"];
        if (roi.containsKey("x")) roiConfig_.x = roi["x"];
        if (roi.containsKey("y")) roiConfig_.y = roi["y"];
        if (roi.containsKey("width")) roiConfig_.width = roi["width"];
        if (roi.containsKey("height")) roiConfig_.height = roi["height"];
    }
    
    Serial.println("AdaptiveProcessor: Configuration loaded from JSON");
    return true;
}

// Private method implementations

AdaptiveProcessor::ActivityLevel AdaptiveProcessor::assessActivityLevel() {
    uint32_t currentTime = millis();
    uint32_t windowMs = config_.activityWindowMinutes * 60000;
    
    // Count detections in current window
    uint32_t detectionsInWindow = 0;
    for (uint32_t detectionTime : detectionTimes_) {
        if (currentTime - detectionTime <= windowMs) {
            detectionsInWindow++;
        }
    }
    
    // Check for dormant state
    if (lastDetectionTime_ > 0 && 
        currentTime - lastDetectionTime_ > config_.dormantThresholdMinutes * 60000) {
        return ActivityLevel::DORMANT;
    }
    
    // Calculate detections per hour
    float detectionsPerHour = (float)detectionsInWindow * 3600000.0f / windowMs;
    
    if (detectionsPerHour > 15) return ActivityLevel::PEAK;
    if (detectionsPerHour > 5) return ActivityLevel::HIGH;
    if (detectionsPerHour > 1) return ActivityLevel::MODERATE;
    if (detectionsPerHour > 0) return ActivityLevel::LOW;
    
    return ActivityLevel::DORMANT;
}

AdaptiveProcessor::ProcessingLevel AdaptiveProcessor::determineOptimalLevel(float batteryVoltage, 
                                                                           float temperature, 
                                                                           float lightLevel) {
    ProcessingLevel baseLevel = ProcessingLevel::NORMAL;
    
    // Adjust for battery level
    if (batteryVoltage < config_.batteryLowThreshold) {
        baseLevel = ProcessingLevel::MINIMAL;
    } else if (batteryVoltage < config_.batteryLowThreshold + 0.2f) {
        baseLevel = ProcessingLevel::REDUCED;
    }
    
    // Adjust for activity level
    switch (currentActivity_) {
        case ActivityLevel::DORMANT:
            baseLevel = std::min(baseLevel, ProcessingLevel::MINIMAL);
            break;
        case ActivityLevel::LOW:
            baseLevel = std::min(baseLevel, ProcessingLevel::REDUCED);
            break;
        case ActivityLevel::MODERATE:
            // Keep current level
            break;
        case ActivityLevel::HIGH:
            if (batteryVoltage > 3.5f) {
                baseLevel = std::max(baseLevel, ProcessingLevel::ENHANCED);
            }
            break;
        case ActivityLevel::PEAK:
            if (batteryVoltage > 3.6f) {
                baseLevel = ProcessingLevel::MAXIMUM;
            }
            break;
    }
    
    // Environmental adjustments
    if (config_.useEnvironmentalAdaptation) {
        // Reduce processing in extreme temperatures
        if (temperature < config_.temperatureOptimalMin - 10 || 
            temperature > config_.temperatureOptimalMax + 10) {
            baseLevel = std::min(baseLevel, ProcessingLevel::REDUCED);
        }
        
        // Adjust for light conditions
        if (lightLevel < 0.1f) { // Very dark
            baseLevel = std::min(baseLevel, ProcessingLevel::REDUCED);
        }
    }
    
    // Track level changes
    if (baseLevel != currentLevel_) {
        metrics_.levelChanges++;
    }
    
    return baseLevel;
}

void AdaptiveProcessor::updateMetrics(uint32_t processingTime) {
    metrics_.totalProcessingTime += processingTime;
    metrics_.peakProcessingTime = std::max(metrics_.peakProcessingTime, processingTime);
    
    // Calculate rolling average
    if (!processingTimes_.empty()) {
        uint32_t sum = 0;
        for (uint32_t time : processingTimes_) {
            sum += time;
        }
        metrics_.averageProcessingTime = sum / processingTimes_.size();
    }
    
    // Calculate power savings
    metrics_.powerSavings = calculatePowerSavings();
}

void AdaptiveProcessor::cleanupOldData() {
    uint32_t currentTime = millis();
    uint32_t windowMs = config_.activityWindowMinutes * 60000;
    
    // Remove old detection times
    auto it = std::remove_if(detectionTimes_.begin(), detectionTimes_.end(),
        [currentTime, windowMs](uint32_t time) {
            return currentTime - time > windowMs;
        });
    detectionTimes_.erase(it, detectionTimes_.end());
    
    // Clean up confidence history (same size as detection times)
    if (confidenceHistory_.size() > detectionTimes_.size()) {
        confidenceHistory_.resize(detectionTimes_.size());
    }
}

float AdaptiveProcessor::calculatePowerSavings() {
    if (processingTimes_.empty()) return 0.0f;
    
    // Estimate power savings based on reduced processing time
    uint32_t baselineTime = 400; // Normal processing time
    uint32_t actualTime = metrics_.averageProcessingTime;
    
    if (actualTime < baselineTime) {
        return ((float)(baselineTime - actualTime) / baselineTime) * 100.0f;
    }
    
    return 0.0f;
}

bool AdaptiveProcessor::shouldSkipFrame() {
    if (currentActivity_ == ActivityLevel::DORMANT) {
        return (millis() % frameRateConfig_.dormantIntervalMs) != 0;
    }
    
    if (currentActivity_ == ActivityLevel::LOW) {
        return (millis() % frameRateConfig_.lowActivityIntervalMs) != 0;
    }
    
    return false;
}

void AdaptiveProcessor::updateROIInternal() {
    if (detectionX_.size() < 3) return;
    
    // Calculate average detection center
    uint32_t avgX = 0, avgY = 0;
    for (size_t i = 0; i < detectionX_.size(); i++) {
        avgX += detectionX_[i];
        avgY += detectionY_[i];
    }
    avgX /= detectionX_.size();
    avgY /= detectionY_.size();
    
    // Update ROI to center on average detection location
    roiConfig_.x = std::max(0, (int)avgX - (int)roiConfig_.width / 2);
    roiConfig_.y = std::max(0, (int)avgY - (int)roiConfig_.height / 2);
    
    // Ensure ROI stays within bounds (assuming 320x240 frame)
    if (roiConfig_.x + roiConfig_.width > 320) {
        roiConfig_.x = 320 - roiConfig_.width;
    }
    if (roiConfig_.y + roiConfig_.height > 240) {
        roiConfig_.y = 240 - roiConfig_.height;
    }
    
    // Increase confidence
    roiConfig_.confidence = std::min(1.0f, roiConfig_.confidence + 0.1f);
    roiConfig_.lastUpdate = millis();
    roiConfig_.enabled = true;
}

String AdaptiveProcessor::getActivityLevelString(ActivityLevel level) const {
    switch (level) {
        case ActivityLevel::DORMANT: return "Dormant";
        case ActivityLevel::LOW: return "Low";
        case ActivityLevel::MODERATE: return "Moderate";
        case ActivityLevel::HIGH: return "High";
        case ActivityLevel::PEAK: return "Peak";
        default: return "Unknown";
    }
}

String AdaptiveProcessor::getProcessingLevelString(ProcessingLevel level) const {
    switch (level) {
        case ProcessingLevel::MINIMAL: return "Minimal";
        case ProcessingLevel::REDUCED: return "Reduced";
        case ProcessingLevel::NORMAL: return "Normal";
        case ProcessingLevel::ENHANCED: return "Enhanced";
        case ProcessingLevel::MAXIMUM: return "Maximum";
        default: return "Unknown";
    }
}