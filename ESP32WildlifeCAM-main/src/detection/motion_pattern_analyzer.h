/**
 * @file motion_pattern_analyzer.h
 * @brief Advanced motion pattern analysis for wildlife behavior recognition
 * 
 * Implements temporal analysis and pattern recognition to distinguish between
 * different types of motion (animals vs environmental changes) and reduce
 * false positives in wildlife detection.
 */

#ifndef MOTION_PATTERN_ANALYZER_H
#define MOTION_PATTERN_ANALYZER_H

#include <Arduino.h>
#include "motion_detection.h"

/**
 * @brief Motion pattern types
 */
enum class MotionPattern {
    UNKNOWN = 0,
    ANIMAL_WALKING = 1,     // Steady, rhythmic movement
    ANIMAL_RUNNING = 2,     // Fast, continuous movement
    ANIMAL_FEEDING = 3,     // Slow, stationary with small movements
    ANIMAL_GRAZING = 4,     // Slow movement with pauses
    ENVIRONMENTAL = 5,      // Wind, vegetation, shadows
    BIRD_FLIGHT = 6,        // Fast, directional movement
    FALSE_POSITIVE = 7      // Likely not actual motion
};

/**
 * @brief Motion pattern analysis result
 */
struct MotionPatternResult {
    MotionPattern pattern;
    float confidence;           // 0.0 to 1.0
    bool isAnimalMotion;        // True if likely animal
    bool isEnvironmental;       // True if likely environmental
    uint32_t analysisTime;      // Time taken for analysis
    String description;         // Human-readable description
    
    // Movement characteristics
    float averageSpeed;         // Pixels per frame
    float directionConsistency; // 0.0 to 1.0
    float rhythmScore;          // Movement rhythm 0.0 to 1.0
    uint32_t movementDuration;  // Duration in milliseconds
};

/**
 * @brief Temporal motion data for analysis
 */
struct TemporalMotionData {
    uint32_t timestamp;
    float motionLevel;
    int16_t centerX;            // Motion center X coordinate
    int16_t centerY;            // Motion center Y coordinate
    uint16_t motionArea;        // Area of motion in pixels
};

/**
 * @brief Motion pattern analyzer
 * 
 * Analyzes sequences of motion detections to identify patterns and
 * distinguish between animal motion and environmental changes.
 */
class MotionPatternAnalyzer {
public:
    /**
     * @brief Initialize pattern analyzer
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Analyze motion pattern from temporal data
     * @param motionResult Current motion detection result
     * @return Pattern analysis result
     */
    MotionPatternResult analyzePattern(const MotionDetection::MotionResult& motionResult);
    
    /**
     * @brief Add motion data to temporal history
     * @param timestamp Time of detection
     * @param motionLevel Motion level (0.0 to 1.0)
     * @param centerX Motion center X coordinate
     * @param centerY Motion center Y coordinate
     * @param motionArea Area of motion in pixels
     */
    void addMotionData(uint32_t timestamp, float motionLevel, int16_t centerX, 
                      int16_t centerY, uint16_t motionArea);
    
    /**
     * @brief Check if current motion is likely a false positive
     * @return true if likely false positive
     */
    bool isFalsePositive();
    
    /**
     * @brief Get motion pattern description
     * @param pattern Motion pattern type
     * @return Human-readable description
     */
    static String getPatternDescription(MotionPattern pattern);
    
    /**
     * @brief Clear temporal history
     */
    void clearHistory();
    
    /**
     * @brief Set sensitivity threshold for pattern detection
     * @param threshold Sensitivity (0.0 to 1.0)
     */
    void setSensitivity(float threshold);
    
    /**
     * @brief Enable or disable temporal analysis
     * @param enable Enable state
     */
    void setEnabled(bool enable) { enabled = enable; }
    
    /**
     * @brief Check if analyzer is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Get total patterns analyzed
     * @return Number of patterns analyzed
     */
    uint32_t getTotalAnalyzed() const { return totalAnalyzed; }
    
    /**
     * @brief Get false positives detected
     * @return Number of false positives
     */
    uint32_t getFalsePositivesDetected() const { return falsePositivesDetected; }

private:
    bool initialized = false;
    bool enabled = true;
    float sensitivityThreshold = 0.6f;
    
    // Temporal history (circular buffer)
    static const uint8_t HISTORY_SIZE = 20;
    TemporalMotionData history[HISTORY_SIZE];
    uint8_t historyIndex = 0;
    uint8_t historyCount = 0;
    
    // Statistics
    uint32_t totalAnalyzed = 0;
    uint32_t falsePositivesDetected = 0;
    
    /**
     * @brief Calculate average speed from history
     * @return Average speed in pixels per frame
     */
    float calculateAverageSpeed();
    
    /**
     * @brief Calculate direction consistency from history
     * @return Direction consistency (0.0 to 1.0)
     */
    float calculateDirectionConsistency();
    
    /**
     * @brief Calculate movement rhythm score
     * @return Rhythm score (0.0 to 1.0)
     */
    float calculateRhythmScore();
    
    /**
     * @brief Detect if motion is environmental (wind, shadows)
     * @return true if environmental motion detected
     */
    bool detectEnvironmentalMotion();
    
    /**
     * @brief Classify motion pattern based on characteristics
     * @param speed Average speed
     * @param directionConsistency Direction consistency
     * @param rhythmScore Rhythm score
     * @return Classified motion pattern
     */
    MotionPattern classifyPattern(float speed, float directionConsistency, float rhythmScore);
    
    /**
     * @brief Calculate confidence for pattern classification
     * @param pattern Classified pattern
     * @param speed Average speed
     * @param directionConsistency Direction consistency
     * @param rhythmScore Rhythm score
     * @return Confidence score (0.0 to 1.0)
     */
    float calculatePatternConfidence(MotionPattern pattern, float speed, 
                                    float directionConsistency, float rhythmScore);
    
    /**
     * @brief Get motion data from history at offset
     * @param offset Offset from current position
     * @return Motion data pointer or nullptr if invalid
     */
    TemporalMotionData* getHistoryAt(int8_t offset);
};

#endif // MOTION_PATTERN_ANALYZER_H
