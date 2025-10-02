/**
 * @file motion_pattern_analyzer.cpp
 * @brief Implementation of motion pattern analyzer
 */

#include "motion_pattern_analyzer.h"

bool MotionPatternAnalyzer::initialize() {
    if (initialized) {
        return true;
    }
    
    Serial.println("Initializing motion pattern analyzer...");
    
    // Clear history
    clearHistory();
    
    // Reset statistics
    totalAnalyzed = 0;
    falsePositivesDetected = 0;
    
    initialized = true;
    enabled = true;
    
    Serial.println("Motion pattern analyzer initialized");
    Serial.printf("Sensitivity threshold: %.2f\n", sensitivityThreshold);
    
    return true;
}

MotionPatternResult MotionPatternAnalyzer::analyzePattern(
    const MotionDetection::MotionResult& motionResult) {
    
    MotionPatternResult result = {};
    uint32_t startTime = millis();
    
    if (!initialized || !enabled) {
        result.pattern = MotionPattern::UNKNOWN;
        return result;
    }
    
    // Add current motion data to history
    if (motionResult.motionDetected) {
        addMotionData(millis(), motionResult.motionLevel, 
                     motionResult.motionCenterX, motionResult.motionCenterY,
                     motionResult.changedPixels);
    }
    
    // Need at least 3 data points for pattern analysis
    if (historyCount < 3) {
        result.pattern = MotionPattern::UNKNOWN;
        result.confidence = 0.0f;
        result.description = "Insufficient data for pattern analysis";
        return result;
    }
    
    // Calculate movement characteristics
    float speed = calculateAverageSpeed();
    float directionConsistency = calculateDirectionConsistency();
    float rhythmScore = calculateRhythmScore();
    
    // Detect environmental motion
    bool isEnvironmental = detectEnvironmentalMotion();
    
    // Classify pattern
    MotionPattern pattern;
    if (isEnvironmental) {
        pattern = MotionPattern::ENVIRONMENTAL;
    } else {
        pattern = classifyPattern(speed, directionConsistency, rhythmScore);
    }
    
    // Calculate confidence
    float confidence = calculatePatternConfidence(pattern, speed, 
                                                  directionConsistency, rhythmScore);
    
    // Build result
    result.pattern = pattern;
    result.confidence = confidence;
    result.isAnimalMotion = (pattern != MotionPattern::ENVIRONMENTAL && 
                            pattern != MotionPattern::FALSE_POSITIVE &&
                            pattern != MotionPattern::UNKNOWN);
    result.isEnvironmental = (pattern == MotionPattern::ENVIRONMENTAL);
    result.averageSpeed = speed;
    result.directionConsistency = directionConsistency;
    result.rhythmScore = rhythmScore;
    result.description = getPatternDescription(pattern);
    result.analysisTime = millis() - startTime;
    
    // Update statistics
    totalAnalyzed++;
    if (pattern == MotionPattern::FALSE_POSITIVE || pattern == MotionPattern::ENVIRONMENTAL) {
        falsePositivesDetected++;
    }
    
    return result;
}

void MotionPatternAnalyzer::addMotionData(uint32_t timestamp, float motionLevel, 
                                         int16_t centerX, int16_t centerY, 
                                         uint16_t motionArea) {
    // Add to circular buffer
    history[historyIndex].timestamp = timestamp;
    history[historyIndex].motionLevel = motionLevel;
    history[historyIndex].centerX = centerX;
    history[historyIndex].centerY = centerY;
    history[historyIndex].motionArea = motionArea;
    
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) {
        historyCount++;
    }
}

bool MotionPatternAnalyzer::isFalsePositive() {
    if (historyCount < 3) {
        return false; // Not enough data
    }
    
    // Check for environmental motion patterns
    if (detectEnvironmentalMotion()) {
        return true;
    }
    
    // Check for very low motion consistency
    float directionConsistency = calculateDirectionConsistency();
    if (directionConsistency < 0.2f) {
        return true; // Random motion, likely environmental
    }
    
    return false;
}

String MotionPatternAnalyzer::getPatternDescription(MotionPattern pattern) {
    switch (pattern) {
        case MotionPattern::ANIMAL_WALKING:
            return "Animal Walking - Steady rhythmic movement";
        case MotionPattern::ANIMAL_RUNNING:
            return "Animal Running - Fast continuous movement";
        case MotionPattern::ANIMAL_FEEDING:
            return "Animal Feeding - Slow stationary movement";
        case MotionPattern::ANIMAL_GRAZING:
            return "Animal Grazing - Slow movement with pauses";
        case MotionPattern::ENVIRONMENTAL:
            return "Environmental - Wind/vegetation/shadows";
        case MotionPattern::BIRD_FLIGHT:
            return "Bird Flight - Fast directional movement";
        case MotionPattern::FALSE_POSITIVE:
            return "False Positive - Not actual motion";
        default:
            return "Unknown Pattern";
    }
}

void MotionPatternAnalyzer::clearHistory() {
    historyIndex = 0;
    historyCount = 0;
    memset(history, 0, sizeof(history));
}

void MotionPatternAnalyzer::setSensitivity(float threshold) {
    sensitivityThreshold = constrain(threshold, 0.0f, 1.0f);
    Serial.printf("Pattern analyzer sensitivity set to: %.2f\n", sensitivityThreshold);
}

float MotionPatternAnalyzer::calculateAverageSpeed() {
    if (historyCount < 2) {
        return 0.0f;
    }
    
    float totalSpeed = 0.0f;
    uint8_t speedCount = 0;
    
    for (uint8_t i = 1; i < historyCount; i++) {
        TemporalMotionData* current = getHistoryAt(-i);
        TemporalMotionData* previous = getHistoryAt(-(i+1));
        
        if (current && previous) {
            // Calculate distance moved
            float dx = current->centerX - previous->centerX;
            float dy = current->centerY - previous->centerY;
            float distance = sqrt(dx*dx + dy*dy);
            
            // Calculate time difference
            uint32_t timeDiff = current->timestamp - previous->timestamp;
            if (timeDiff > 0) {
                float speed = distance / (timeDiff / 1000.0f); // pixels per second
                totalSpeed += speed;
                speedCount++;
            }
        }
    }
    
    return (speedCount > 0) ? (totalSpeed / speedCount) : 0.0f;
}

float MotionPatternAnalyzer::calculateDirectionConsistency() {
    if (historyCount < 3) {
        return 0.0f;
    }
    
    float totalConsistency = 0.0f;
    uint8_t consistencyCount = 0;
    
    // Calculate direction vectors and compare
    for (uint8_t i = 1; i < historyCount - 1; i++) {
        TemporalMotionData* current = getHistoryAt(-i);
        TemporalMotionData* next = getHistoryAt(-(i+1));
        TemporalMotionData* prev = getHistoryAt(-(i-1));
        
        if (current && next && prev) {
            // Direction from prev to current
            float dx1 = current->centerX - prev->centerX;
            float dy1 = current->centerY - prev->centerY;
            
            // Direction from current to next
            float dx2 = next->centerX - current->centerX;
            float dy2 = next->centerY - current->centerY;
            
            // Calculate dot product (normalized)
            float len1 = sqrt(dx1*dx1 + dy1*dy1);
            float len2 = sqrt(dx2*dx2 + dy2*dy2);
            
            if (len1 > 0 && len2 > 0) {
                float dotProduct = (dx1*dx2 + dy1*dy2) / (len1 * len2);
                // Convert from [-1,1] to [0,1]
                totalConsistency += (dotProduct + 1.0f) / 2.0f;
                consistencyCount++;
            }
        }
    }
    
    return (consistencyCount > 0) ? (totalConsistency / consistencyCount) : 0.0f;
}

float MotionPatternAnalyzer::calculateRhythmScore() {
    if (historyCount < 4) {
        return 0.0f;
    }
    
    // Calculate motion level variations
    float variations[HISTORY_SIZE];
    uint8_t variationCount = 0;
    
    for (uint8_t i = 1; i < historyCount; i++) {
        TemporalMotionData* current = getHistoryAt(-i);
        TemporalMotionData* previous = getHistoryAt(-(i+1));
        
        if (current && previous) {
            variations[variationCount++] = abs(current->motionLevel - previous->motionLevel);
        }
    }
    
    if (variationCount < 3) {
        return 0.0f;
    }
    
    // Calculate standard deviation of variations
    float mean = 0.0f;
    for (uint8_t i = 0; i < variationCount; i++) {
        mean += variations[i];
    }
    mean /= variationCount;
    
    float variance = 0.0f;
    for (uint8_t i = 0; i < variationCount; i++) {
        float diff = variations[i] - mean;
        variance += diff * diff;
    }
    variance /= variationCount;
    
    // Low variance = rhythmic motion
    // Convert to 0-1 score (lower variance = higher score)
    float stdDev = sqrt(variance);
    float rhythmScore = 1.0f - constrain(stdDev, 0.0f, 1.0f);
    
    return rhythmScore;
}

bool MotionPatternAnalyzer::detectEnvironmentalMotion() {
    if (historyCount < 5) {
        return false;
    }
    
    // Environmental motion characteristics:
    // 1. Low direction consistency
    // 2. High variability in motion area
    // 3. No clear movement direction
    
    float directionConsistency = calculateDirectionConsistency();
    
    // Calculate area variability
    float totalArea = 0.0f;
    for (uint8_t i = 0; i < historyCount; i++) {
        TemporalMotionData* data = getHistoryAt(-i);
        if (data) {
            totalArea += data->motionArea;
        }
    }
    float meanArea = totalArea / historyCount;
    
    float areaVariance = 0.0f;
    for (uint8_t i = 0; i < historyCount; i++) {
        TemporalMotionData* data = getHistoryAt(-i);
        if (data) {
            float diff = data->motionArea - meanArea;
            areaVariance += diff * diff;
        }
    }
    areaVariance /= historyCount;
    float areaStdDev = sqrt(areaVariance);
    float areaVariability = areaStdDev / (meanArea + 1.0f); // Normalize
    
    // Environmental motion: low consistency + high variability
    if (directionConsistency < 0.3f && areaVariability > 0.5f) {
        return true;
    }
    
    return false;
}

MotionPattern MotionPatternAnalyzer::classifyPattern(float speed, float directionConsistency, 
                                                    float rhythmScore) {
    // Fast movement with high direction consistency
    if (speed > 50.0f && directionConsistency > 0.7f) {
        if (speed > 100.0f) {
            return MotionPattern::BIRD_FLIGHT;
        }
        return MotionPattern::ANIMAL_RUNNING;
    }
    
    // Moderate speed with rhythm (walking)
    if (speed > 10.0f && speed <= 50.0f && rhythmScore > 0.6f) {
        return MotionPattern::ANIMAL_WALKING;
    }
    
    // Slow movement with low rhythm (feeding/grazing)
    if (speed <= 10.0f && directionConsistency > 0.5f) {
        if (rhythmScore < 0.4f) {
            return MotionPattern::ANIMAL_FEEDING;
        } else {
            return MotionPattern::ANIMAL_GRAZING;
        }
    }
    
    // Low confidence patterns
    if (directionConsistency < 0.3f || speed < 2.0f) {
        return MotionPattern::FALSE_POSITIVE;
    }
    
    return MotionPattern::UNKNOWN;
}

float MotionPatternAnalyzer::calculatePatternConfidence(MotionPattern pattern, float speed, 
                                                       float directionConsistency, float rhythmScore) {
    float confidence = 0.5f; // Base confidence
    
    switch (pattern) {
        case MotionPattern::ANIMAL_WALKING:
            // High rhythm and moderate speed = high confidence
            confidence = (rhythmScore * 0.5f) + (directionConsistency * 0.3f) + 0.2f;
            break;
            
        case MotionPattern::ANIMAL_RUNNING:
            // High speed and direction consistency = high confidence
            confidence = (directionConsistency * 0.6f) + 0.3f;
            break;
            
        case MotionPattern::ANIMAL_FEEDING:
            // Low speed with some consistency = moderate confidence
            confidence = (directionConsistency * 0.4f) + 0.3f;
            break;
            
        case MotionPattern::ANIMAL_GRAZING:
            // Moderate rhythm = moderate confidence
            confidence = (rhythmScore * 0.4f) + (directionConsistency * 0.3f) + 0.2f;
            break;
            
        case MotionPattern::BIRD_FLIGHT:
            // Very high speed and consistency = high confidence
            confidence = (directionConsistency * 0.7f) + 0.2f;
            break;
            
        case MotionPattern::ENVIRONMENTAL:
            // Low confidence for non-animal motion
            confidence = 0.3f;
            break;
            
        case MotionPattern::FALSE_POSITIVE:
            // Very low confidence
            confidence = 0.1f;
            break;
            
        default:
            confidence = 0.5f;
            break;
    }
    
    return constrain(confidence, 0.0f, 1.0f);
}

TemporalMotionData* MotionPatternAnalyzer::getHistoryAt(int8_t offset) {
    if (historyCount == 0 || abs(offset) >= historyCount) {
        return nullptr;
    }
    
    // Calculate actual index (accounting for circular buffer)
    int16_t index = historyIndex + offset;
    if (index < 0) {
        index += HISTORY_SIZE;
    }
    index = index % HISTORY_SIZE;
    
    return &history[index];
}
