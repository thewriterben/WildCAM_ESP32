#include "enhanced_hybrid_motion_detector.h"
#include <cmath>

bool EnhancedHybridMotionDetector::initialize(CameraManager* cameraManager) {
    // Initialize base hybrid detector
    if (!HybridMotionDetector::initialize(cameraManager)) {
        return false;
    }

    // Initialize enhanced components
    if (!multiZonePIR.initialize()) {
        Serial.println("Failed to initialize multi-zone PIR");
        return false;
    }

    if (!advancedMotion.initialize()) {
        Serial.println("Failed to initialize advanced motion detection");
        return false;
    }

    // Initialize analytics
    motionTrajectory.clear();
    heatmapData.clear();
    lastHeatmapUpdate = 0;
    totalObjectCount = 0;
    averageDwellTime = 0.0f;

    Serial.println("Enhanced hybrid motion detector initialized");
    return true;
}

EnhancedHybridMotionDetector::EnhancedHybridResult 
EnhancedHybridMotionDetector::detectMotionEnhanced() {
    uint32_t startTime = millis();

    switch (detectionMode) {
        case EnhancedDetectionMode::LEGACY_HYBRID:
            return performLegacyDetection();
            
        case EnhancedDetectionMode::MULTI_ZONE_PIR:
            return performMultiZoneDetection();
            
        case EnhancedDetectionMode::ADVANCED_ANALYSIS:
            return performAdvancedAnalysis();
            
        case EnhancedDetectionMode::FULL_ENHANCED:
            return performFullEnhancedDetection();
            
        case EnhancedDetectionMode::ADAPTIVE_MODE:
            detectionMode = selectAdaptiveMode();
            return detectMotionEnhanced(); // Recurse with selected mode
            
        default:
            return performLegacyDetection();
    }
}

void EnhancedHybridMotionDetector::setEnhancedDetectionMode(EnhancedDetectionMode mode) {
    detectionMode = mode;
    
    const char* modeNames[] = {
        "Legacy Hybrid", "Multi-Zone PIR", "Advanced Analysis", 
        "Full Enhanced", "Adaptive Mode"
    };
    
    Serial.printf("Enhanced detection mode set to: %s\n", 
                  modeNames[static_cast<int>(mode)]);
}

void EnhancedHybridMotionDetector::configureAnalytics(const AnalyticsConfig& config) {
    analyticsConfig = config;
    analyticsEnabled = true;
    
    Serial.printf("Analytics configured: heatmap=%s, direction=%s, speed=%s, dwell=%s\n",
                  config.enableMotionHeatmap ? "on" : "off",
                  config.enableDirectionTracking ? "on" : "off", 
                  config.enableSpeedEstimation ? "on" : "off",
                  config.enableDwellTimeAnalysis ? "on" : "off");
}

bool EnhancedHybridMotionDetector::configureMultiZonePIR(bool useDefaultZones) {
    if (!multiZonePIR.isEnabled()) {
        return false;
    }

    if (useDefaultZones) {
        return multiZonePIR.configureDefaultZones();
    }

    // Manual configuration will be done via addPIRZone calls
    multiZoneEnabled = true;
    return true;
}

bool EnhancedHybridMotionDetector::addPIRZone(uint8_t zoneId, uint8_t pin, const String& name, 
                                              float sensitivity, uint8_t priority) {
    bool success = multiZonePIR.addZone(zoneId, pin, name, sensitivity, priority);
    if (success) {
        multiZoneEnabled = true;
    }
    return success;
}

void EnhancedHybridMotionDetector::configureAdvancedAnalysis(bool enableBackground,
                                                            bool enableVectors,
                                                            bool enableSizeFilter, 
                                                            bool enableML) {
    advancedMotion.setBackgroundSubtractionEnabled(enableBackground);
    advancedMotion.setVectorAnalysisEnabled(enableVectors);
    advancedMotion.setSizeFilteringEnabled(enableSizeFilter);
    advancedMotion.setMLFilteringEnabled(enableML);
    
    advancedAnalysisEnabled = true;
    
    Serial.printf("Advanced analysis configured: bg=%s, vectors=%s, size=%s, ml=%s\n",
                  enableBackground ? "on" : "off", enableVectors ? "on" : "off",
                  enableSizeFilter ? "on" : "off", enableML ? "on" : "off");
}

String EnhancedHybridMotionDetector::getAnalyticsSummary() const {
    String summary = "Analytics Summary:\\n";
    summary += "- Total objects detected: " + String(totalObjectCount) + "\\n";
    summary += "- Average dwell time: " + String(averageDwellTime, 1) + "s\\n";
    summary += "- Trajectory points: " + String(motionTrajectory.size()) + "\\n";
    summary += "- Heatmap points: " + String(heatmapData.size()) + "\\n";
    
    if (advancedAnalysisEnabled) {
        summary += "- Current adaptive threshold: " + String(advancedMotion.getAdaptiveThreshold(), 3) + "\\n";
        summary += "- Background similarity: " + String(advancedMotion.getBackgroundSimilarity(), 3) + "\\n";
    }
    
    return summary;
}

String EnhancedHybridMotionDetector::getMotionHeatmap() const {
    if (!analyticsConfig.enableMotionHeatmap || heatmapData.empty()) {
        return "{}";
    }

    String json = "{\"heatmap\":[";
    for (size_t i = 0; i < heatmapData.size(); i++) {
        if (i > 0) json += ",";
        json += "{\"x\":" + String(heatmapData[i].first) + 
                ",\"y\":" + String(heatmapData[i].second) + "}";
    }
    json += "]}";
    
    return json;
}

void EnhancedHybridMotionDetector::resetAnalytics() {
    motionTrajectory.clear();
    heatmapData.clear();
    totalObjectCount = 0;
    averageDwellTime = 0.0f;
    lastHeatmapUpdate = 0;
    
    if (multiZoneEnabled) {
        multiZonePIR.resetStatistics();
    }
    
    if (advancedAnalysisEnabled) {
        advancedMotion.resetMLAdaptation();
    }
    
    Serial.println("Enhanced analytics reset");
}

void EnhancedHybridMotionDetector::cleanup() {
    HybridMotionDetector::cleanup();
    
    multiZonePIR.cleanup();
    advancedMotion.cleanup();
    
    motionTrajectory.clear();
    heatmapData.clear();
    
    Serial.println("Enhanced hybrid motion detector cleaned up");
}

// Private method implementations

EnhancedHybridMotionDetector::EnhancedHybridResult 
EnhancedHybridMotionDetector::performLegacyDetection() {
    EnhancedHybridResult result = {};
    
    // Perform original hybrid detection
    HybridResult legacyResult = HybridMotionDetector::detectMotion();
    
    // Copy legacy results to enhanced result
    result.motionDetected = legacyResult.motionDetected;
    result.confidence = legacyResult.confidence;
    result.confidenceScore = legacyResult.confidenceScore;
    result.pirTriggered = legacyResult.pirTriggered;
    result.frameMotionDetected = legacyResult.frameMotionDetected;
    result.frameResult = legacyResult.frameResult;
    result.totalProcessTime = legacyResult.totalProcessTime;
    result.description = legacyResult.description;
    
    // Set enhanced fields
    result.multiZoneEnabled = false;
    result.advancedAnalysisEnabled = false;
    result.multiZoneProcessTime = 0;
    result.advancedProcessTime = 0;
    result.totalEnhancedProcessTime = result.totalProcessTime;
    
    return result;
}

EnhancedHybridMotionDetector::EnhancedHybridResult 
EnhancedHybridMotionDetector::performMultiZoneDetection() {
    EnhancedHybridResult result = {};
    uint32_t startTime = millis();
    
    // Multi-zone PIR detection
    uint32_t pirStart = millis();
    result.multiZoneResult = multiZonePIR.detectMotion();
    result.multiZoneProcessTime = millis() - pirStart;
    
    // Basic frame analysis if PIR triggered
    if (result.multiZoneResult.motionDetected) {
        result.frameResult = motionDetection.analyzeFrame(camera->captureFrame());
        result.frameMotionDetected = result.frameResult.motionDetected;
    }
    
    // Calculate combined confidence
    result.confidenceScore = calculateEnhancedConfidence(result.multiZoneResult, 
                                                        AdvancedMotionDetection::AdvancedMotionResult{});
    result.motionDetected = result.confidenceScore > 0.5f;
    result.confidence = scoreToConfidenceLevel(result.confidenceScore);
    
    result.multiZoneEnabled = true;
    result.advancedAnalysisEnabled = false;
    result.totalEnhancedProcessTime = millis() - startTime;
    result.description = generateEnhancedDescription(result);
    
    if (analyticsEnabled) {
        updateAnalytics(result);
    }
    
    return result;
}

EnhancedHybridMotionDetector::EnhancedHybridResult 
EnhancedHybridMotionDetector::performAdvancedAnalysis() {
    EnhancedHybridResult result = {};
    uint32_t startTime = millis();
    
    // Basic PIR detection
    result.pirTriggered = detectPIRMotion();
    
    // Advanced frame analysis
    uint32_t advancedStart = millis();
    camera_fb_t* frame = camera->captureFrame();
    if (frame) {
        result.advancedResult = advancedMotion.analyzeFrameAdvanced(frame);
        result.frameMotionDetected = result.advancedResult.motionDetected;
        
        // Extract analytics data
        result.motionDirection = result.advancedResult.dominantDirection;
        result.motionSpeed = result.advancedResult.averageSpeed;
        result.dwellTime = result.advancedResult.dwellTime;
        result.isNewObject = result.advancedResult.isNewObject;
        result.mlConfidence = result.advancedResult.mlConfidence;
        result.falsePositivePrediction = result.advancedResult.falsePositivePrediction;
        result.adaptiveThreshold = result.advancedResult.adaptiveThreshold;
        
        camera->releaseFrame(frame);
    }
    result.advancedProcessTime = millis() - advancedStart;
    
    // Calculate combined confidence
    result.confidenceScore = calculateEnhancedConfidence(MultiZonePIRSensor::MultiZoneResult{}, 
                                                        result.advancedResult);
    result.motionDetected = result.confidenceScore > 0.5f && !result.falsePositivePrediction;
    result.confidence = scoreToConfidenceLevel(result.confidenceScore);
    
    result.multiZoneEnabled = false;
    result.advancedAnalysisEnabled = true;
    result.totalEnhancedProcessTime = millis() - startTime;
    result.description = generateEnhancedDescription(result);
    
    if (analyticsEnabled) {
        updateAnalytics(result);
    }
    
    return result;
}

EnhancedHybridMotionDetector::EnhancedHybridResult 
EnhancedHybridMotionDetector::performFullEnhancedDetection() {
    EnhancedHybridResult result = {};
    uint32_t startTime = millis();
    
    // Multi-zone PIR detection
    uint32_t pirStart = millis();
    result.multiZoneResult = multiZonePIR.detectMotion();
    result.multiZoneProcessTime = millis() - pirStart;
    
    // Advanced frame analysis (always performed for learning)
    uint32_t advancedStart = millis();
    camera_fb_t* frame = camera->captureFrame();
    if (frame) {
        result.advancedResult = advancedMotion.analyzeFrameAdvanced(frame);
        result.frameMotionDetected = result.advancedResult.motionDetected;
        
        // Extract analytics data
        result.motionDirection = result.advancedResult.dominantDirection;
        result.motionSpeed = result.advancedResult.averageSpeed;
        result.dwellTime = result.advancedResult.dwellTime;
        result.isNewObject = result.advancedResult.isNewObject;
        result.mlConfidence = result.advancedResult.mlConfidence;
        result.falsePositivePrediction = result.advancedResult.falsePositivePrediction;
        result.adaptiveThreshold = result.advancedResult.adaptiveThreshold;
        
        camera->releaseFrame(frame);
    }
    result.advancedProcessTime = millis() - advancedStart;
    
    // Calculate combined confidence from all sources
    result.confidenceScore = calculateEnhancedConfidence(result.multiZoneResult, result.advancedResult);
    result.motionDetected = result.confidenceScore > 0.5f && !result.falsePositivePrediction;
    result.confidence = scoreToConfidenceLevel(result.confidenceScore);
    
    result.multiZoneEnabled = true;
    result.advancedAnalysisEnabled = true;
    result.totalEnhancedProcessTime = millis() - startTime;
    result.description = generateEnhancedDescription(result);
    
    if (analyticsEnabled) {
        updateAnalytics(result);
    }
    
    updateEnhancedStatistics(result);
    
    return result;
}

EnhancedHybridMotionDetector::EnhancedDetectionMode 
EnhancedHybridMotionDetector::selectAdaptiveMode() {
    // Adaptive mode selection based on system state and performance
    
    // Check system load and power state
    bool lowPowerMode = false; // Would check actual power state
    bool highSystemLoad = false; // Would check CPU usage
    
    // Check detection accuracy from recent history
    float recentAccuracy = 0.95f; // Would calculate from recent detections
    
    if (lowPowerMode) {
        // Prefer less CPU-intensive modes in low power
        return EnhancedDetectionMode::MULTI_ZONE_PIR;
    } else if (highSystemLoad) {
        // Use legacy mode if system is busy
        return EnhancedDetectionMode::LEGACY_HYBRID;
    } else if (recentAccuracy < 0.9f) {
        // Use full enhanced mode if accuracy is low
        return EnhancedDetectionMode::FULL_ENHANCED;
    } else {
        // Default to advanced analysis for good balance
        return EnhancedDetectionMode::ADVANCED_ANALYSIS;
    }
}

void EnhancedHybridMotionDetector::updateAnalytics(const EnhancedHybridResult& result) {
    if (!analyticsEnabled) {
        return;
    }

    // Update motion trajectory
    if (analyticsConfig.enableDirectionTracking && result.motionDetected) {
        motionTrajectory.push_back(std::make_pair(result.motionDirection, result.motionSpeed));
        
        // Limit trajectory history
        if (motionTrajectory.size() > analyticsConfig.trackingHistorySize) {
            motionTrajectory.erase(motionTrajectory.begin());
        }
    }

    // Update heatmap
    if (analyticsConfig.enableMotionHeatmap && result.motionDetected) {
        uint32_t now = millis();
        if (now - lastHeatmapUpdate > analyticsConfig.heatmapUpdateInterval) {
            // Use center of bounding box for heatmap
            uint16_t centerX = result.frameResult.boundingBoxX + result.frameResult.boundingBoxW / 2;
            uint16_t centerY = result.frameResult.boundingBoxY + result.frameResult.boundingBoxH / 2;
            updateHeatmap(centerX, centerY);
            lastHeatmapUpdate = now;
        }
    }

    // Update object counting and dwell time
    if (analyticsConfig.enableDwellTimeAnalysis && result.isNewObject) {
        totalObjectCount++;
        
        if (result.dwellTime > 0) {
            // Update rolling average dwell time
            if (totalObjectCount == 1) {
                averageDwellTime = result.dwellTime / 1000.0f; // Convert to seconds
            } else {
                averageDwellTime = (averageDwellTime * 0.9f) + (result.dwellTime / 1000.0f * 0.1f);
            }
        }
    }
}

void EnhancedHybridMotionDetector::updateHeatmap(uint16_t x, uint16_t y) {
    heatmapData.push_back(std::make_pair(x, y));
    
    // Limit heatmap data size to prevent memory issues
    if (heatmapData.size() > 1000) {
        heatmapData.erase(heatmapData.begin());
    }
}

float EnhancedHybridMotionDetector::calculateEnhancedConfidence(
    const MultiZonePIRSensor::MultiZoneResult& pirResult,
    const AdvancedMotionDetection::AdvancedMotionResult& frameResult) {
    
    float confidence = 0.0f;
    float totalWeight = 0.0f;
    
    // Multi-zone PIR contribution
    if (multiZoneEnabled && pirResult.motionDetected) {
        float pirWeight = 0.4f;
        confidence += pirResult.overallConfidence * pirWeight;
        totalWeight += pirWeight;
    }
    
    // Advanced frame analysis contribution
    if (advancedAnalysisEnabled && frameResult.motionDetected) {
        float frameWeight = 0.6f;
        float adjustedFrameConfidence = frameResult.motionLevel * frameResult.mlConfidence;
        confidence += adjustedFrameConfidence * frameWeight;
        totalWeight += frameWeight;
    }
    
    // Legacy fallback
    if (totalWeight == 0.0f) {
        confidence = 0.5f; // Default confidence
        totalWeight = 1.0f;
    }
    
    return confidence / totalWeight;
}

String EnhancedHybridMotionDetector::generateEnhancedDescription(const EnhancedHybridResult& result) {
    String desc = "";
    
    if (!result.motionDetected) {
        return "No motion detected";
    }
    
    // Multi-zone information
    if (result.multiZoneEnabled && result.multiZoneResult.motionDetected) {
        desc += result.multiZoneResult.description;
    }
    
    // Advanced analysis information
    if (result.advancedAnalysisEnabled) {
        if (!desc.isEmpty()) desc += " | ";
        desc += "Advanced: ";
        if (result.motionSpeed > 0) {
            desc += "Speed=" + String(result.motionSpeed, 1) + " ";
        }
        if (result.mlConfidence > 0) {
            desc += "ML=" + String(result.mlConfidence, 2) + " ";
        }
        if (result.falsePositivePrediction) {
            desc += "FP-Risk ";
        }
    }
    
    // Analytics information
    if (analyticsEnabled && result.dwellTime > 0) {
        if (!desc.isEmpty()) desc += " | ";
        desc += "Dwell=" + String(result.dwellTime / 1000.0f, 1) + "s";
    }
    
    return desc.isEmpty() ? "Enhanced motion detected" : desc;
}

void EnhancedHybridMotionDetector::updateEnhancedStatistics(const EnhancedHybridResult& result) {
    // Update base statistics
    updateStatistics(result);
    
    // Add enhanced-specific statistics tracking here
    // This would extend the existing stats structure with enhanced metrics
}