#include "hybrid_motion_detector.h"

bool HybridMotionDetector::initialize(CameraManager* cameraManager) {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing hybrid motion detector...");

    camera = cameraManager;
    if (camera == nullptr) {
        Serial.println("Camera manager not provided");
        return false;
    }

    // Initialize PIR sensor
    if (!pirSensor.initialize()) {
        Serial.println("Failed to initialize PIR sensor");
        return false;
    }

    // Initialize motion detection
    if (!motionDetection.initialize()) {
        Serial.println("Failed to initialize motion detection");
        return false;
    }

    // Configure default settings
    configureWeights();
    resetStatistics();

    initialized = true;
    enabled = true;

    Serial.println("Hybrid motion detector initialized successfully");
    return true;
}

HybridMotionDetector::HybridResult HybridMotionDetector::detectMotion() {
    HybridResult result = {};
    uint32_t startTime = millis();

    if (!initialized || !enabled) {
        return result;
    }

    // Detect motion using different modes
    if (pirOnlyMode) {
        result.pirTriggered = detectPIRMotion();
        result.motionDetected = result.pirTriggered;
        result.confidenceScore = result.pirTriggered ? 1.0f : 0.0f;
    } else if (frameOnlyMode) {
        result.frameResult = detectFrameMotion();
        result.frameMotionDetected = result.frameResult.motionDetected;
        result.motionDetected = result.frameMotionDetected;
        result.confidenceScore = result.frameResult.motionLevel;
    } else if (hybridMode) {
        // Hybrid detection - PIR first for efficiency
        result.pirTriggered = detectPIRMotion();
        
        if (result.pirTriggered || !lowPowerMode) {
            // Perform frame analysis if PIR triggered or not in low power mode
            result.frameResult = detectFrameMotion();
            result.frameMotionDetected = result.frameResult.motionDetected;
            
            // Calculate combined confidence
            result.confidenceScore = calculateConfidenceScore(result.pirTriggered, result.frameResult);
            result.motionDetected = result.confidenceScore > 0.5f;
        } else {
            // Low power mode - rely on PIR only
            result.motionDetected = result.pirTriggered;
            result.confidenceScore = result.pirTriggered ? 0.6f : 0.0f;
        }
    }

    result.confidence = scoreToConfidenceLevel(result.confidenceScore);
    result.totalProcessTime = millis() - startTime;
    result.description = generateDescription(result);

    updateStatistics(result);

    if (result.motionDetected) {
        Serial.printf("Hybrid motion detected: %s (confidence: %.2f)\n", 
                     result.description.c_str(), result.confidenceScore);
    }

    return result;
}

void HybridMotionDetector::setDetectionMode(bool useHybrid, bool pirOnly, bool frameOnly) {
    hybridMode = useHybrid && !pirOnly && !frameOnly;
    pirOnlyMode = pirOnly;
    frameOnlyMode = frameOnly;

    if (!hybridMode && !pirOnlyMode && !frameOnlyMode) {
        hybridMode = true; // Default to hybrid mode
    }

    String mode = hybridMode ? "Hybrid" : (pirOnlyMode ? "PIR Only" : "Frame Only");
    Serial.printf("Detection mode set to: %s\n", mode.c_str());
}

void HybridMotionDetector::configureWeights(float pirWeight, float frameWeight, uint32_t confirmationTime) {
    this->pirWeight = constrain(pirWeight, 0.0f, 1.0f);
    this->frameWeight = constrain(frameWeight, 0.0f, 1.0f);
    this->confirmationTime = confirmationTime;

    // Normalize weights
    float totalWeight = this->pirWeight + this->frameWeight;
    if (totalWeight > 0) {
        this->pirWeight /= totalWeight;
        this->frameWeight /= totalWeight;
    }

    Serial.printf("Weights configured - PIR: %.2f, Frame: %.2f, Confirmation: %dms\n",
                 this->pirWeight, this->frameWeight, confirmationTime);
}

void HybridMotionDetector::setEnabled(bool enable) {
    enabled = enable;
    pirSensor.setEnabled(enable);
    motionDetection.setEnabled(enable);
    
    Serial.printf("Hybrid motion detector %s\n", enable ? "enabled" : "disabled");
}

void HybridMotionDetector::resetStatistics() {
    stats = {};
    pirSensor.resetStatistics();
    motionDetection.resetStatistics();
}

void HybridMotionDetector::setLowPowerMode(bool lowPower) {
    lowPowerMode = lowPower;
    
    if (lowPower) {
        // In low power mode, rely more on PIR
        configureWeights(0.8f, 0.2f, confirmationTime);
    } else {
        // Normal mode - balanced weights
        configureWeights(0.6f, 0.4f, confirmationTime);
    }
    
    Serial.printf("Low power mode %s\n", lowPower ? "enabled" : "disabled");
}

String HybridMotionDetector::getConfidenceDescription(ConfidenceLevel level) {
    switch (level) {
        case ConfidenceLevel::NONE: return "None";
        case ConfidenceLevel::LOW: return "Low";
        case ConfidenceLevel::MEDIUM: return "Medium";
        case ConfidenceLevel::HIGH: return "High";
        case ConfidenceLevel::VERY_HIGH: return "Very High";
        default: return "Unknown";
    }
}

void HybridMotionDetector::cleanup() {
    if (initialized) {
        pirSensor.cleanup();
        motionDetection.cleanup();
        initialized = false;
        Serial.println("Hybrid motion detector cleaned up");
    }
}

bool HybridMotionDetector::detectPIRMotion() {
    return pirSensor.hasMotion();
}

MotionDetection::MotionResult HybridMotionDetector::detectFrameMotion() {
    MotionDetection::MotionResult result = {};
    
    if (camera == nullptr || !camera->isReady()) {
        return result;
    }

    // Capture frame for analysis
    camera_fb_t* fb = camera->captureToBuffer();
    if (fb != nullptr) {
        result = motionDetection.analyzeFrame(fb);
        camera->returnFrameBuffer(fb);
    }

    return result;
}

float HybridMotionDetector::calculateConfidenceScore(bool pirDetected, 
                                                    const MotionDetection::MotionResult& frameResult) {
    float score = 0.0f;

    // PIR contribution
    if (pirDetected) {
        score += pirWeight;
    }

    // Frame analysis contribution
    if (frameResult.motionDetected) {
        score += frameWeight * frameResult.motionLevel;
    }

    // Bonus for both detecting motion
    if (pirDetected && frameResult.motionDetected) {
        score += 0.2f; // 20% bonus for correlation
    }

    return constrain(score, 0.0f, 1.0f);
}

HybridMotionDetector::ConfidenceLevel HybridMotionDetector::scoreToConfidenceLevel(float score) {
    if (score >= 0.9f) return ConfidenceLevel::VERY_HIGH;
    if (score >= 0.7f) return ConfidenceLevel::HIGH;
    if (score >= 0.5f) return ConfidenceLevel::MEDIUM;
    if (score >= 0.2f) return ConfidenceLevel::LOW;
    return ConfidenceLevel::NONE;
}

void HybridMotionDetector::updateStatistics(const HybridResult& result) {
    if (result.motionDetected) {
        stats.totalDetections++;
        
        if (result.pirTriggered && !result.frameMotionDetected) {
            stats.pirOnlyDetections++;
        } else if (!result.pirTriggered && result.frameMotionDetected) {
            stats.frameOnlyDetections++;
        } else if (result.pirTriggered && result.frameMotionDetected) {
            stats.combinedDetections++;
        }
        
        // Update average confidence
        stats.averageConfidence = ((stats.averageConfidence * (stats.totalDetections - 1)) + 
                                  result.confidenceScore) / stats.totalDetections;
    }
    
    // Update average processing time
    static uint32_t totalProcessCalls = 0;
    totalProcessCalls++;
    stats.averageProcessTime = ((stats.averageProcessTime * (totalProcessCalls - 1)) + 
                               result.totalProcessTime) / totalProcessCalls;
}

String HybridMotionDetector::generateDescription(const HybridResult& result) {
    String desc = "";
    
    if (result.motionDetected) {
        desc += "Motion detected (";
        
        if (result.pirTriggered && result.frameMotionDetected) {
            desc += "PIR+Frame";
        } else if (result.pirTriggered) {
            desc += "PIR only";
        } else if (result.frameMotionDetected) {
            desc += "Frame only";
        }
        
        desc += ", " + getConfidenceDescription(result.confidence) + ")";
    } else {
        desc = "No motion detected";
    }
    
    return desc;
}