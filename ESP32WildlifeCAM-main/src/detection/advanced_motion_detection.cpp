#include "advanced_motion_detection.h"
#include <algorithm>
#include <cmath>

bool AdvancedMotionDetection::initialize() {
    if (!MotionDetection::initialize()) {
        return false;
    }

    // Initialize advanced features
    trainingStartTime = millis();
    adaptiveThreshold = 0.5f;
    backgroundSimilarity = 0.0f;
    objectFirstSeen = 0;
    objectTracked = false;

    currentVectors.clear();
    previousVectors.clear();
    falsePositivePatterns.clear();
    truePositivePatterns.clear();

    Serial.println("Advanced motion detection initialized");
    return true;
}

AdvancedMotionDetection::AdvancedMotionResult 
AdvancedMotionDetection::analyzeFrameAdvanced(camera_fb_t* currentFrame) {
    AdvancedMotionResult result = {};
    uint32_t startTime = millis();

    if (!initialized || !enabled || currentFrame == nullptr) {
        return result;
    }

    // First perform basic motion detection
    MotionResult basicResult = analyzeFrame(currentFrame);
    
    // Copy basic results to advanced result
    result.motionDetected = basicResult.motionDetected;
    result.motionLevel = basicResult.motionLevel;
    result.motionBlocks = basicResult.motionBlocks;
    result.boundingBoxX = basicResult.boundingBoxX;
    result.boundingBoxY = basicResult.boundingBoxY;
    result.boundingBoxW = basicResult.boundingBoxW;
    result.boundingBoxH = basicResult.boundingBoxH;

    // Extract frame data for advanced analysis
    const uint8_t* frameData = currentFrame->buf;
    size_t frameSize = currentFrame->len;
    uint16_t width = frameWidth;
    uint16_t height = frameHeight;

    // Perform advanced analysis if motion detected or always if configured
    if (result.motionDetected || backgroundEnabled) {
        
        // Background subtraction analysis
        if (backgroundEnabled) {
            result.backgroundSimilarity = performBackgroundSubtraction(frameData, width, height);
            
            // Update background model periodically
            uint32_t timeSinceUpdate = millis() - lastBackgroundUpdate;
            if (timeSinceUpdate > bgConfig.updateInterval || 
                backgroundFrameCount > bgConfig.maxFramesWithoutUpdate) {
                updateBackground(frameData, width, height);
                result.backgroundUpdated = true;
                lastBackgroundUpdate = millis();
                backgroundFrameCount = 0;
            } else {
                backgroundFrameCount++;
            }
        }

        // Motion vector analysis
        if (vectorAnalysisEnabled && referenceFrame != nullptr) {
            result.motionVectors = analyzeMotionVectors(frameData, referenceFrame, width, height);
            currentVectors = result.motionVectors;
            
            if (!result.motionVectors.empty()) {
                result.averageSpeed = calculateAverageSpeed(result.motionVectors);
                result.dominantDirection = calculateDominantDirection(result.motionVectors);
            }
        }

        // Object size filtering
        if (sizeFilteringEnabled && !result.motionVectors.empty()) {
            auto filteredVectors = filterObjectsBySize(result.motionVectors);
            result.filteredObjects = result.motionVectors.size() - filteredVectors.size();
            result.motionVectors = filteredVectors;
            
            // Update confidence based on size filtering
            if (result.filteredObjects > 0) {
                result.motionLevel *= (1.0f - sizeConfig.sizeConfidenceWeight);
            }
        }

        // Machine learning false positive filtering
        if (mlFilteringEnabled) {
            auto mlResult = applyMLFiltering(result);
            result.mlConfidence = mlResult.first;
            result.falsePositivePrediction = mlResult.second;
            result.adaptiveThreshold = adaptiveThreshold;
            
            // Adjust final motion detection based on ML confidence
            if (result.falsePositivePrediction && result.mlConfidence < mlConfig.confidenceThreshold) {
                result.motionDetected = false;
            }
        }

        // Analytics - dwell time and object tracking
        if (result.motionDetected) {
            if (!objectTracked) {
                objectFirstSeen = millis();
                objectTracked = true;
                result.isNewObject = true;
            }
            result.dwellTime = millis() - objectFirstSeen;
        } else {
            if (objectTracked) {
                objectTracked = false;
                result.dwellTime = millis() - objectFirstSeen;
            }
        }
    }

    // Store previous vectors for next analysis
    previousVectors = currentVectors;
    
    result.processTime = millis() - startTime;
    updateStatistics(result);

    return result;
}

void AdvancedMotionDetection::configureBackground(const BackgroundConfig& config) {
    bgConfig = config;
    Serial.printf("Background config: learning=%.3f, threshold=%.3f, interval=%d\n",
                  config.learningRate, config.updateThreshold, config.updateInterval);
}

void AdvancedMotionDetection::configureSizeFilter(const SizeFilterConfig& config) {
    sizeConfig = config;
    Serial.printf("Size filter config: min=%d, max=%d, enabled=%s\n",
                  config.minObjectSize, config.maxObjectSize, 
                  config.enableFiltering ? "true" : "false");
}

void AdvancedMotionDetection::configureML(const MLConfig& config) {
    mlConfig = config;
    Serial.printf("ML config: enabled=%s, learning=%.3f, threshold=%.3f\n",
                  config.enableLearning ? "true" : "false",
                  config.learningRate, config.confidenceThreshold);
}

void AdvancedMotionDetection::updateBackgroundModel() {
    lastBackgroundUpdate = 0; // Force update on next frame
    backgroundFrameCount = bgConfig.maxFramesWithoutUpdate;
    Serial.println("Background model update forced");
}

void AdvancedMotionDetection::resetMLAdaptation() {
    adaptiveThreshold = 0.5f;
    falsePositivePatterns.clear();
    truePositivePatterns.clear();
    trainingStartTime = millis();
    Serial.println("ML adaptation reset");
}

void AdvancedMotionDetection::cleanup() {
    MotionDetection::cleanup();
    
    backgroundModel.reset();
    backgroundVariance.reset();
    currentVectors.clear();
    previousVectors.clear();
    falsePositivePatterns.clear();
    truePositivePatterns.clear();
    
    Serial.println("Advanced motion detection cleaned up");
}

// Private method implementations

float AdvancedMotionDetection::performBackgroundSubtraction(const uint8_t* currentFrame, 
                                                           uint16_t width, uint16_t height) {
    if (!backgroundModel) {
        // Initialize background model if not present
        if (!allocateBackgroundModel(width, height)) {
            return 0.0f;
        }
        
        // Initialize with current frame
        for (size_t i = 0; i < width * height; i++) {
            backgroundModel[i] = 128; // Initialize to middle gray
            backgroundVariance[i] = 25.0f; // Initial variance
        }
    }

    // Calculate similarity to background model
    float totalDifference = 0.0f;
    size_t pixelCount = 0;
    
    // Simplified background subtraction (would normally process decoded frame)
    for (size_t i = 0; i < std::min((size_t)(width * height), size_t(1000)); i += 10) {
        uint8_t currentPixel = (i < width * height) ? 128 : 128; // Simplified
        uint8_t backgroundPixel = backgroundModel[i];
        
        float difference = abs(currentPixel - backgroundPixel);
        totalDifference += difference;
        pixelCount++;
    }

    float averageDifference = pixelCount > 0 ? totalDifference / pixelCount : 0.0f;
    return 1.0f - (averageDifference / 255.0f); // Convert to similarity (0-1)
}

void AdvancedMotionDetection::updateBackground(const uint8_t* currentFrame, 
                                             uint16_t width, uint16_t height) {
    if (!backgroundModel) {
        return;
    }

    // Update background model using learning rate
    for (size_t i = 0; i < std::min((size_t)(width * height), size_t(1000)); i += 10) {
        uint8_t currentPixel = 128; // Simplified - would decode actual pixel
        uint8_t& backgroundPixel = backgroundModel[i];
        
        // Adaptive background update
        backgroundPixel = (uint8_t)(backgroundPixel * (1.0f - bgConfig.learningRate) + 
                                   currentPixel * bgConfig.learningRate);
    }
}

std::vector<AdvancedMotionDetection::MotionVector> 
AdvancedMotionDetection::analyzeMotionVectors(const uint8_t* currentFrame,
                                             const uint8_t* previousFrame,
                                             uint16_t width, uint16_t height) {
    std::vector<MotionVector> vectors;
    
    // Simplified motion vector analysis
    // In a real implementation, this would use optical flow or block matching
    
    // Create sample motion vectors for demonstration
    if (width > 0 && height > 0) {
        // Check for motion in grid blocks
        uint16_t blockSize = 16;
        for (uint16_t y = 0; y < height - blockSize; y += blockSize) {
            for (uint16_t x = 0; x < width - blockSize; x += blockSize) {
                
                // Simplified motion detection in block
                bool blockHasMotion = (rand() % 100) < 10; // 10% chance for demo
                
                if (blockHasMotion) {
                    MotionVector vector;
                    vector.x = x + blockSize / 2;
                    vector.y = y + blockSize / 2;
                    vector.dx = (rand() % 20) - 10; // Random motion -10 to +10
                    vector.dy = (rand() % 20) - 10;
                    vector.magnitude = sqrt(vector.dx * vector.dx + vector.dy * vector.dy);
                    vector.confidence = calculateVectorConfidence(vector);
                    
                    if (vector.confidence > 0.3f) { // Only include confident vectors
                        vectors.push_back(vector);
                    }
                }
            }
        }
    }
    
    return vectors;
}

std::vector<AdvancedMotionDetection::MotionVector> 
AdvancedMotionDetection::filterObjectsBySize(const std::vector<MotionVector>& motionRegions) {
    std::vector<MotionVector> filtered;
    
    for (const auto& vector : motionRegions) {
        // Estimate object size from motion vector magnitude and area
        float estimatedSize = vector.magnitude * 2.0f; // Simplified estimation
        
        if (estimatedSize >= sizeConfig.minObjectSize && 
            estimatedSize <= sizeConfig.maxObjectSize) {
            filtered.push_back(vector);
        }
    }
    
    return filtered;
}

std::pair<float, bool> AdvancedMotionDetection::applyMLFiltering(const AdvancedMotionResult& result) {
    // Extract features for ML analysis
    std::vector<float> features = extractMLFeatures(result);
    
    // Simplified ML prediction based on historical patterns
    float mlConfidence = 0.5f;
    bool falsePositivePrediction = false;
    
    if (!falsePositivePatterns.empty()) {
        // Calculate similarity to known false positive patterns
        float avgFalsePositive = 0.0f;
        for (float pattern : falsePositivePatterns) {
            avgFalsePositive += pattern;
        }
        avgFalsePositive /= falsePositivePatterns.size();
        
        // Simple pattern matching
        float currentPattern = result.motionLevel * result.backgroundSimilarity;
        float similarity = 1.0f - abs(currentPattern - avgFalsePositive);
        
        if (similarity > 0.7f) {
            falsePositivePrediction = true;
            mlConfidence = 1.0f - similarity;
        } else {
            mlConfidence = 0.8f;
        }
    }
    
    // Update adaptive threshold
    adaptiveThreshold = adaptiveThreshold * 0.95f + mlConfidence * 0.05f;
    
    return std::make_pair(mlConfidence, falsePositivePrediction);
}

void AdvancedMotionDetection::updateMLModel(const std::vector<float>& features, bool isMotion) {
    if (!mlConfig.enableLearning || features.empty()) {
        return;
    }
    
    // Store pattern based on motion classification
    float pattern = features[0]; // Simplified - use first feature
    
    if (isMotion) {
        truePositivePatterns.push_back(pattern);
        // Limit memory usage
        if (truePositivePatterns.size() > mlConfig.patternMemorySize) {
            truePositivePatterns.erase(truePositivePatterns.begin());
        }
    } else {
        falsePositivePatterns.push_back(pattern);
        if (falsePositivePatterns.size() > mlConfig.patternMemorySize) {
            falsePositivePatterns.erase(falsePositivePatterns.begin());
        }
    }
}

std::vector<float> AdvancedMotionDetection::extractMLFeatures(const AdvancedMotionResult& result) {
    std::vector<float> features;
    
    // Basic features for ML classification
    features.push_back(result.motionLevel);
    features.push_back(result.backgroundSimilarity);
    features.push_back(result.averageSpeed);
    features.push_back(static_cast<float>(result.motionVectors.size()));
    features.push_back(static_cast<float>(result.motionBlocks));
    
    return features;
}

float AdvancedMotionDetection::calculateVectorConfidence(const MotionVector& vector) {
    // Confidence based on magnitude and consistency
    float magnitudeConfidence = std::min(vector.magnitude / 10.0f, 1.0f);
    float consistencyConfidence = 0.8f; // Would compare with previous vectors
    
    return (magnitudeConfidence + consistencyConfidence) / 2.0f;
}

float AdvancedMotionDetection::calculateDominantDirection(const std::vector<MotionVector>& vectors) {
    if (vectors.empty()) {
        return 0.0f;
    }
    
    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& vector : vectors) {
        sumX += vector.dx * vector.confidence;
        sumY += vector.dy * vector.confidence;
    }
    
    return atan2(sumY, sumX); // Return angle in radians
}

float AdvancedMotionDetection::calculateAverageSpeed(const std::vector<MotionVector>& vectors) {
    if (vectors.empty()) {
        return 0.0f;
    }
    
    float totalSpeed = 0.0f;
    for (const auto& vector : vectors) {
        totalSpeed += vector.magnitude * vector.confidence;
    }
    
    return totalSpeed / vectors.size();
}

bool AdvancedMotionDetection::allocateBackgroundModel(uint16_t width, uint16_t height) {
    size_t size = width * height;
    
    try {
        backgroundModel = std::make_unique<uint8_t[]>(size);
        backgroundVariance = std::make_unique<float[]>(size);
        return true;
    } catch (const std::bad_alloc&) {
        Serial.println("Failed to allocate background model");
        return false;
    }
}