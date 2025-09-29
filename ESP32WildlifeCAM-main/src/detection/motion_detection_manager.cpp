#include "motion_detection_manager.h"
#include <memory>

bool MotionDetectionManager::initialize(CameraManager* cameraManager, DetectionSystem system) {
    if (initialized) {
        return true;
    }

    camera = cameraManager;
    if (camera == nullptr) {
        Serial.println("Camera manager not provided to MotionDetectionManager");
        return false;
    }

    currentSystem = system;

    // Initialize the appropriate detection system
    if (currentSystem == DetectionSystem::LEGACY_HYBRID) {
        Serial.println("Initializing legacy hybrid motion detection...");
        legacyDetector = std::make_unique<HybridMotionDetector>();
        if (!legacyDetector->initialize(cameraManager)) {
            Serial.println("Failed to initialize legacy hybrid detector");
            return false;
        }
    } else {
        Serial.println("Initializing enhanced hybrid motion detection...");
        enhancedDetector = std::make_unique<EnhancedHybridMotionDetector>();
        if (!enhancedDetector->initialize(cameraManager)) {
            Serial.println("Failed to initialize enhanced hybrid detector");
            return false;
        }
        
        // Configure default enhanced features based on config.h
        #if MULTI_ZONE_PIR_ENABLED
        enhancedDetector->configureMultiZonePIR(true);
        #endif
        
        #if ADVANCED_FRAME_ANALYSIS
        enhancedDetector->configureAdvancedAnalysis(
            BACKGROUND_SUBTRACTION_ENABLED,
            MOTION_VECTOR_ANALYSIS,
            OBJECT_SIZE_FILTERING,
            ML_FALSE_POSITIVE_FILTERING
        );
        #endif
        
        #if MOTION_ANALYTICS_ENABLED
        EnhancedHybridMotionDetector::AnalyticsConfig analyticsConfig;
        analyticsConfig.enableMotionHeatmap = MOTION_HEATMAP_ENABLED;
        analyticsConfig.enableDirectionTracking = DIRECTION_TRACKING_ENABLED;
        analyticsConfig.enableSpeedEstimation = SPEED_ESTIMATION_ENABLED;
        analyticsConfig.enableDwellTimeAnalysis = DWELL_TIME_ANALYSIS_ENABLED;
        enhancedDetector->configureAnalytics(analyticsConfig);
        #endif
        
        // Set default enhanced detection mode
        auto mode = static_cast<EnhancedHybridMotionDetector::EnhancedDetectionMode>(DEFAULT_ENHANCED_MODE);
        enhancedDetector->setEnhancedDetectionMode(mode);
    }

    initialized = true;
    Serial.printf("Motion detection manager initialized with %s system\n",
                  (currentSystem == DetectionSystem::LEGACY_HYBRID) ? "legacy" : "enhanced");
    
    return true;
}

MotionDetectionManager::UnifiedMotionResult MotionDetectionManager::detectMotion() {
    if (!initialized) {
        return UnifiedMotionResult{};
    }

    if (currentSystem == DetectionSystem::LEGACY_HYBRID && legacyDetector) {
        auto legacyResult = legacyDetector->detectMotion();
        return convertLegacyResult(legacyResult);
    } else if (currentSystem == DetectionSystem::ENHANCED_HYBRID && enhancedDetector) {
        auto enhancedResult = enhancedDetector->detectMotionEnhanced();
        return convertEnhancedResult(enhancedResult);
    }

    return UnifiedMotionResult{};
}

bool MotionDetectionManager::upgradeToEnhanced() {
    if (currentSystem == DetectionSystem::ENHANCED_HYBRID) {
        Serial.println("Already using enhanced system");
        return true;
    }

    Serial.println("Upgrading to enhanced motion detection system...");

    // Clean up legacy system
    if (legacyDetector) {
        legacyDetector->cleanup();
        legacyDetector.reset();
    }

    // Initialize enhanced system
    enhancedDetector = std::make_unique<EnhancedHybridMotionDetector>();
    if (!enhancedDetector->initialize(camera)) {
        Serial.println("Failed to upgrade to enhanced system");
        
        // Fallback to legacy system
        legacyDetector = std::make_unique<HybridMotionDetector>();
        if (legacyDetector->initialize(camera)) {
            Serial.println("Reverted to legacy system");
            return false;
        }
        
        initialized = false;
        return false;
    }

    currentSystem = DetectionSystem::ENHANCED_HYBRID;
    
    // Configure with default enhanced features
    configureEnhancedFeatures();
    
    Serial.println("Successfully upgraded to enhanced motion detection");
    return true;
}

bool MotionDetectionManager::configureEnhancedFeatures(bool enableMultiZone, 
                                                       bool enableAdvancedAnalysis,
                                                       bool enableAnalytics) {
    if (currentSystem != DetectionSystem::ENHANCED_HYBRID || !enhancedDetector) {
        Serial.println("Enhanced features not available - not using enhanced system");
        return false;
    }

    // Configure multi-zone PIR
    if (enableMultiZone) {
        if (!enhancedDetector->configureMultiZonePIR(true)) {
            Serial.println("Failed to configure multi-zone PIR");
        } else {
            Serial.println("Multi-zone PIR configured");
        }
    }

    // Configure advanced analysis
    if (enableAdvancedAnalysis) {
        enhancedDetector->configureAdvancedAnalysis(true, true, true, true);
        Serial.println("Advanced analysis configured");
    }

    // Configure analytics
    if (enableAnalytics) {
        EnhancedHybridMotionDetector::AnalyticsConfig config;
        config.enableDirectionTracking = true;
        config.enableSpeedEstimation = true;
        config.enableDwellTimeAnalysis = true;
        config.enableMotionHeatmap = false; // Default off to save memory
        
        enhancedDetector->configureAnalytics(config);
        Serial.println("Motion analytics configured");
    }

    // Set to full enhanced mode for best performance
    enhancedDetector->setEnhancedDetectionMode(
        EnhancedHybridMotionDetector::EnhancedDetectionMode::FULL_ENHANCED);

    return true;
}

String MotionDetectionManager::getAnalyticsSummary() const {
    if (currentSystem == DetectionSystem::ENHANCED_HYBRID && enhancedDetector) {
        return enhancedDetector->getAnalyticsSummary();
    }
    return "Analytics not available (legacy system)";
}

void MotionDetectionManager::resetStatistics() {
    if (currentSystem == DetectionSystem::LEGACY_HYBRID && legacyDetector) {
        legacyDetector->resetStatistics();
    } else if (currentSystem == DetectionSystem::ENHANCED_HYBRID && enhancedDetector) {
        enhancedDetector->resetAnalytics();
    }
}

void MotionDetectionManager::setSensitivity(float sensitivity) {
    if (currentSystem == DetectionSystem::LEGACY_HYBRID && legacyDetector) {
        // Legacy system sensitivity would be set here
        Serial.printf("Setting legacy sensitivity to %.2f\n", sensitivity);
    } else if (currentSystem == DetectionSystem::ENHANCED_HYBRID && enhancedDetector) {
        // Enhanced system can set zone-specific sensitivity
        Serial.printf("Setting enhanced sensitivity to %.2f\n", sensitivity);
        // Would iterate through zones and set sensitivity
    }
}

void MotionDetectionManager::setLowPowerMode(bool lowPower) {
    if (currentSystem == DetectionSystem::LEGACY_HYBRID && legacyDetector) {
        legacyDetector->setLowPowerMode(lowPower);
    } else if (currentSystem == DetectionSystem::ENHANCED_HYBRID && enhancedDetector) {
        // Enhanced system can use adaptive mode for power optimization
        if (lowPower) {
            enhancedDetector->setEnhancedDetectionMode(
                EnhancedHybridMotionDetector::EnhancedDetectionMode::MULTI_ZONE_PIR);
        } else {
            enhancedDetector->setEnhancedDetectionMode(
                EnhancedHybridMotionDetector::EnhancedDetectionMode::FULL_ENHANCED);
        }
    }
}

void MotionDetectionManager::cleanup() {
    if (legacyDetector) {
        legacyDetector->cleanup();
        legacyDetector.reset();
    }
    
    if (enhancedDetector) {
        enhancedDetector->cleanup();
        enhancedDetector.reset();
    }
    
    initialized = false;
    Serial.println("Motion detection manager cleaned up");
}

// Private methods

MotionDetectionManager::UnifiedMotionResult 
MotionDetectionManager::convertEnhancedResult(const EnhancedHybridMotionDetector::EnhancedHybridResult& enhancedResult) {
    UnifiedMotionResult result;
    
    // Copy common fields
    result.motionDetected = enhancedResult.motionDetected;
    result.confidenceScore = enhancedResult.confidenceScore;
    result.confidence = enhancedResult.confidence;
    result.pirTriggered = enhancedResult.pirTriggered;
    result.frameMotionDetected = enhancedResult.frameMotionDetected;
    result.processTime = enhancedResult.totalEnhancedProcessTime;
    result.description = enhancedResult.description;
    
    // Copy enhanced fields
    result.hasEnhancedData = true;
    result.activeZoneCount = enhancedResult.multiZoneResult.activeZoneCount;
    result.motionDirection = enhancedResult.motionDirection;
    result.motionSpeed = enhancedResult.motionSpeed;
    result.dwellTime = enhancedResult.dwellTime;
    result.isNewObject = enhancedResult.isNewObject;
    result.mlConfidence = enhancedResult.mlConfidence;
    result.falsePositivePrediction = enhancedResult.falsePositivePrediction;
    
    return result;
}

MotionDetectionManager::UnifiedMotionResult 
MotionDetectionManager::convertLegacyResult(const HybridMotionDetector::HybridResult& legacyResult) {
    UnifiedMotionResult result;
    
    // Copy common fields
    result.motionDetected = legacyResult.motionDetected;
    result.confidenceScore = legacyResult.confidenceScore;
    result.confidence = legacyResult.confidence;
    result.pirTriggered = legacyResult.pirTriggered;
    result.frameMotionDetected = legacyResult.frameMotionDetected;
    result.processTime = legacyResult.totalProcessTime;
    result.description = legacyResult.description;
    
    // Enhanced fields not available in legacy system
    result.hasEnhancedData = false;
    
    return result;
}