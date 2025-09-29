#include "motion_detection.h"
#include <esp_heap_caps.h>

bool MotionDetection::initialize() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing motion detection...");

    // Reset statistics
    resetStatistics();

    initialized = true;
    enabled = true;

    Serial.println("Motion detection initialized successfully");
    return true;
}

MotionDetection::MotionResult MotionDetection::analyzeFrame(camera_fb_t* currentFrame) {
    MotionResult result = {};
    uint32_t startTime = millis();

    if (!initialized || !enabled || currentFrame == nullptr) {
        return result;
    }

    // For JPEG frames, we need to work with downsampled data for performance
    const uint8_t* frameData = currentFrame->buf;
    size_t frameSize = currentFrame->len;

    // Estimate frame dimensions (this is simplified - real implementation would decode JPEG header)
    uint16_t width = 80;   // Downsampled for performance
    uint16_t height = 60;
    size_t estimatedGraySize = width * height;

    // Allocate reference frame if needed
    if (referenceFrame == nullptr) {
        if (!allocateReferenceFrame(estimatedGraySize)) {
            Serial.println("Failed to allocate reference frame");
            return result;
        }
        frameWidth = width;
        frameHeight = height;
        
        // Initialize reference frame with current frame (simplified)
        // In real implementation, would convert JPEG to grayscale
        memset(referenceFrame, 128, estimatedGraySize); // Gray initialization
        return result; // First frame - no motion possible
    }

    // For demonstration, we'll use a simplified motion detection
    // Real implementation would decode JPEG to grayscale and perform full analysis
    result = simplifiedMotionAnalysis(frameData, frameSize);
    
    result.processTime = millis() - startTime;
    updateStatistics(result);

    // Update reference frame if auto-update is enabled
    if (autoRefUpdate && result.motionLevel < 0.3f) {
        // Update reference frame when motion is low (simplified)
        // Real implementation would update grayscale reference
    }

    return result;
}

MotionDetection::MotionResult MotionDetection::simplifiedMotionAnalysis(const uint8_t* frameData, size_t frameSize) {
    MotionResult result = {};
    
    // Simplified motion detection using frame size changes and data variance
    static size_t lastFrameSize = 0;
    static uint32_t lastDataSum = 0;
    
    if (lastFrameSize == 0) {
        lastFrameSize = frameSize;
        lastDataSum = calculateDataSum(frameData, min(frameSize, (size_t)1000));
        return result;
    }
    
    // Calculate current frame characteristics
    uint32_t currentDataSum = calculateDataSum(frameData, min(frameSize, (size_t)1000));
    
    // Detect motion based on data changes
    float sizeChange = abs((int)frameSize - (int)lastFrameSize) / (float)lastFrameSize;
    float dataChange = abs((int)currentDataSum - (int)lastDataSum) / (float)max(lastDataSum, 1U);
    
    result.motionLevel = (sizeChange * 0.3f + dataChange * 0.7f);
    result.motionDetected = result.motionLevel > (motionThreshold / 255.0f);
    
    if (result.motionDetected) {
        result.motionBlocks = (uint16_t)(result.motionLevel * MOTION_DETECTION_BLOCKS);
        result.boundingBoxW = frameWidth / 2;
        result.boundingBoxH = frameHeight / 2;
        result.boundingBoxX = frameWidth / 4;
        result.boundingBoxY = frameHeight / 4;
    }
    
    lastFrameSize = frameSize;
    lastDataSum = currentDataSum;
    
    return result;
}

uint32_t MotionDetection::calculateDataSum(const uint8_t* data, size_t size) {
    uint32_t sum = 0;
    for (size_t i = 0; i < size; i += 10) { // Sample every 10th byte for performance
        sum += data[i];
    }
    return sum;
}

void MotionDetection::setSensitivity(uint8_t threshold, uint16_t minBlocks) {
    motionThreshold = threshold;
    minMotionBlocks = minBlocks;
    
    Serial.printf("Motion sensitivity set: threshold=%d, minBlocks=%d\n", threshold, minBlocks);
}

void MotionDetection::resetStatistics() {
    stats = {};
    frameCounter = 0;
}

void MotionDetection::setReferenceFrame(camera_fb_t* frame) {
    if (frame == nullptr) {
        return;
    }

    // Estimate frame size for reference
    uint16_t width = 80;
    uint16_t height = 60;
    size_t graySize = width * height;

    if (!allocateReferenceFrame(graySize)) {
        return;
    }

    frameWidth = width;
    frameHeight = height;

    // Initialize reference frame (simplified)
    memset(referenceFrame, 128, graySize);
    
    Serial.println("Reference frame set");
}

void MotionDetection::cleanup() {
    if (referenceFrame != nullptr) {
        heap_caps_free(referenceFrame);
        referenceFrame = nullptr;
        referenceFrameSize = 0;
    }
    
    initialized = false;
    Serial.println("Motion detection cleaned up");
}

bool MotionDetection::allocateReferenceFrame(size_t size) {
    if (referenceFrame != nullptr && referenceFrameSize >= size) {
        return true; // Already allocated with sufficient size
    }

    // Free existing frame if too small
    if (referenceFrame != nullptr) {
        heap_caps_free(referenceFrame);
    }

    // Allocate new reference frame
    referenceFrame = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_8BIT);
    if (referenceFrame == nullptr) {
        Serial.printf("Failed to allocate reference frame: %d bytes\n", size);
        referenceFrameSize = 0;
        return false;
    }

    referenceFrameSize = size;
    Serial.printf("Reference frame allocated: %d bytes\n", size);
    return true;
}

void MotionDetection::updateStatistics(const MotionResult& result) {
    stats.totalFrames++;
    
    if (result.motionDetected) {
        stats.motionFrames++;
        
        // Update running average motion level
        stats.averageMotionLevel = ((stats.averageMotionLevel * (stats.motionFrames - 1)) + result.motionLevel) / stats.motionFrames;
    }
    
    // Update average processing time
    stats.averageProcessTime = ((stats.averageProcessTime * (stats.totalFrames - 1)) + result.processTime) / stats.totalFrames;
    
    // Calculate detection rate
    stats.detectionRate = (float)stats.motionFrames / stats.totalFrames;
}