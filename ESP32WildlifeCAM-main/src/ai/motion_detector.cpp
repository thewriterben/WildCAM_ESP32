#include "motion_detector.h"
#include "../detection/motion_detection_manager.h"
#include "../detection/hybrid_motion_detector.h"
#include "../utils/logger.h"

// Simple AI-based motion detector that integrates existing motion detection
// This provides a clean interface for AI motion detection functionality

MotionDetector::MotionDetector() : initialized(false), detectionThreshold(0.7f) {
    motionManager = std::make_unique<MotionDetectionManager>();
    hybridDetector = std::make_unique<HybridMotionDetector>();
}

MotionDetector::~MotionDetector() {
    if (initialized) {
        cleanup();
    }
}

bool MotionDetector::initialize() {
    if (initialized) {
        return true;
    }
    
    Logger::info("Initializing AI Motion Detector...");
    
    // Initialize the underlying motion detection system
    if (!motionManager->initialize()) {
        Logger::error("Failed to initialize motion detection manager");
        return false;
    }
    
    // Configure hybrid motion detector
    if (!hybridDetector->initialize()) {
        Logger::error("Failed to initialize hybrid motion detector");
        return false;
    }
    
    // Set default detection parameters
    setDetectionThreshold(0.7f);
    setSensitivity(MotionSensitivity::MEDIUM);
    
    initialized = true;
    Logger::info("AI Motion Detector initialized successfully");
    return true;
}

bool MotionDetector::detectMotion(const camera_fb_t* frameBuffer) {
    if (!initialized || !frameBuffer) {
        return false;
    }
    
    // Use the hybrid motion detector for actual detection
    bool motionDetected = hybridDetector->processFrame(frameBuffer->buf, frameBuffer->len);
    
    if (motionDetected) {
        // Apply AI filtering to reduce false positives
        float confidence = calculateConfidence(frameBuffer);
        
        if (confidence >= detectionThreshold) {
            Logger::debug("Motion detected with confidence: %.2f", confidence);
            onMotionDetected(confidence);
            return true;
        } else {
            Logger::debug("Motion filtered out, confidence too low: %.2f", confidence);
        }
    }
    
    return false;
}

bool MotionDetector::detectMotion(const uint8_t* imageData, size_t dataSize) {
    if (!initialized || !imageData || dataSize == 0) {
        return false;
    }
    
    // Use the motion detection manager for raw image data
    return motionManager->detectMotion(imageData, dataSize);
}

void MotionDetector::setDetectionThreshold(float threshold) {
    if (threshold >= 0.0f && threshold <= 1.0f) {
        detectionThreshold = threshold;
        Logger::debug("Motion detection threshold set to: %.2f", threshold);
    }
}

void MotionDetector::setSensitivity(MotionSensitivity sensitivity) {
    currentSensitivity = sensitivity;
    
    // Map sensitivity to threshold values
    switch (sensitivity) {
        case MotionSensitivity::LOW:
            setDetectionThreshold(0.9f);
            break;
        case MotionSensitivity::MEDIUM:
            setDetectionThreshold(0.7f);
            break;
        case MotionSensitivity::HIGH:
            setDetectionThreshold(0.5f);
            break;
        case MotionSensitivity::VERY_HIGH:
            setDetectionThreshold(0.3f);
            break;
    }
    
    Logger::info("Motion sensitivity set to: %d", static_cast<int>(sensitivity));
}

float MotionDetector::calculateConfidence(const camera_fb_t* frameBuffer) {
    if (!frameBuffer) {
        return 0.0f;
    }
    
    // Simple confidence calculation based on frame analysis
    // In a full AI implementation, this would use machine learning models
    
    size_t pixelCount = frameBuffer->len;
    const uint8_t* pixels = frameBuffer->buf;
    
    // Calculate mean and variance in a single pass using Welford's online algorithm
    // This is more efficient than two separate loops
    uint32_t sum = 0;
    uint64_t sumSquares = 0;
    
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t pixel = pixels[i];
        sum += pixel;
        sumSquares += (uint64_t)pixel * pixel;
    }
    
    // Calculate variance: Var(X) = E[X²] - E[X]²
    float mean = (float)sum / pixelCount;
    float variance = ((float)sumSquares / pixelCount) - (mean * mean);
    
    // Higher variance indicates more motion/activity
    float confidence = variance / 10000.0f; // Normalize
    
    // Clamp to [0, 1] range
    if (confidence > 1.0f) confidence = 1.0f;
    if (confidence < 0.0f) confidence = 0.0f;
    
    return confidence;
}

void MotionDetector::onMotionDetected(float confidence) {
    MotionEvent event;
    event.timestamp = millis();
    event.confidence = confidence;
    event.x = 0; // Center of frame for now
    event.y = 0;
    event.width = 100; // Full frame
    event.height = 100;
    
    // Store recent events for analysis
    recentEvents.push_back(event);
    
    // Keep only last 10 events
    if (recentEvents.size() > 10) {
        recentEvents.erase(recentEvents.begin());
    }
    
    // Trigger callback if set
    if (motionCallback) {
        motionCallback(event);
    }
}

void MotionDetector::setMotionCallback(std::function<void(const MotionEvent&)> callback) {
    motionCallback = callback;
}

std::vector<MotionEvent> MotionDetector::getRecentEvents() const {
    return recentEvents;
}

MotionDetectorStats MotionDetector::getStatistics() const {
    MotionDetectorStats stats;
    stats.totalDetections = recentEvents.size();
    stats.averageConfidence = 0.0f;
    stats.lastDetectionTime = 0;
    
    if (!recentEvents.empty()) {
        float sumConfidence = 0.0f;
        for (const auto& event : recentEvents) {
            sumConfidence += event.confidence;
            if (event.timestamp > stats.lastDetectionTime) {
                stats.lastDetectionTime = event.timestamp;
            }
        }
        stats.averageConfidence = sumConfidence / recentEvents.size();
    }
    
    return stats;
}

void MotionDetector::cleanup() {
    if (motionManager) {
        motionManager.reset();
    }
    
    if (hybridDetector) {
        hybridDetector.reset();
    }
    
    recentEvents.clear();
    motionCallback = nullptr;
    initialized = false;
    
    Logger::info("AI Motion Detector cleaned up");
}