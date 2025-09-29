/**
 * @file wildlife_analyzer.h
 * @brief Wildlife Pattern Analyzer for Animal Movement Recognition
 * @author ESP32WildlifeCAM Project
 * @date 2025-01-01
 */

#ifndef WILDLIFE_ANALYZER_H
#define WILDLIFE_ANALYZER_H

#include <Arduino.h>
#include <vector>
#include <map>

/**
 * @brief Wildlife Pattern Analyzer for recognizing animal movement patterns
 * 
 * Analyzes motion patterns to distinguish between different types of wildlife
 * movement and improve detection accuracy while reducing false positives.
 */
class WildlifeAnalyzer {
public:
    /**
     * @brief Types of wildlife movement patterns
     */
    enum class MovementPattern {
        UNKNOWN,            // Unclassified movement
        SMALL_BIRD,         // Quick, erratic movements
        LARGE_BIRD,         // Smooth, directional flight
        SMALL_MAMMAL,       // Quick ground movements, stopping/starting
        MEDIUM_MAMMAL,      // Steady ground movement, some pausing
        LARGE_MAMMAL,       // Slow, steady, heavy movement
        INSECT,             // Very small, rapid movements
        VEGETATION,         // Swaying, repetitive movements
        WEATHER,            // Wind/rain induced motion
        VEHICLE,            // Linear, steady movement
        HUMAN              // Bipedal, vertical movement
    };

    /**
     * @brief Movement characteristics
     */
    struct MovementCharacteristics {
        float speed = 0.0f;              // Movement speed (pixels/second)
        float direction = 0.0f;          // Direction in radians
        float directionStability = 0.0f; // How consistent direction is (0-1)
        float size = 0.0f;               // Object size (relative to frame)
        float periodicity = 0.0f;        // Repetitive pattern strength (0-1)
        float verticality = 0.0f;        // Vertical vs horizontal movement bias
        uint32_t dwellTime = 0;          // Time object remains in frame (ms)
        uint32_t activeTime = 0;         // Time object is moving (ms)
        float intensity = 0.0f;          // Motion intensity (0-1)
    };

    /**
     * @brief Wildlife analysis result
     */
    struct WildlifeAnalysisResult {
        MovementPattern primaryPattern = MovementPattern::UNKNOWN;
        MovementPattern secondaryPattern = MovementPattern::UNKNOWN;
        float confidence = 0.0f;         // Confidence in primary pattern (0-1)
        float wildlifeScore = 0.0f;      // Likelihood it's wildlife (0-1)
        float interestScore = 0.0f;      // How interesting/worth capturing (0-1)
        MovementCharacteristics characteristics;
        String description;
        bool isWildlife = false;
        bool shouldCapture = false;
        bool shouldTriggerAlert = false;
    };

    /**
     * @brief Motion data point for analysis
     */
    struct MotionDataPoint {
        uint32_t timestamp;
        uint16_t x, y;                   // Center coordinates
        uint16_t width, height;          // Bounding box
        float intensity;                 // Motion intensity
        float confidence;                // Detection confidence
    };

    /**
     * @brief Time-of-day activity patterns
     */
    struct TimePattern {
        uint8_t hour;                    // Hour (0-23)
        float activityLevel;             // Expected activity (0-1)
        std::vector<MovementPattern> commonPatterns;
    };

    /**
     * @brief Configuration for wildlife analysis
     */
    struct AnalyzerConfig {
        bool enabled = true;
        uint32_t analysisWindowMs = 10000;       // Analysis window (10 seconds)
        uint32_t minTrackingTimeMs = 500;        // Minimum tracking time
        float minMovementThreshold = 2.0f;       // Minimum movement in pixels
        float maxInsectSize = 0.05f;             // Max size for insect classification
        float minLargeMammalSize = 0.3f;         // Min size for large mammal
        float vegetationPeriodicityThreshold = 0.7f;
        bool useTimeOfDayAdaptation = true;
        bool useSizeFiltering = true;
        bool useMovementFiltering = true;
        float wildlifeConfidenceThreshold = 0.6f;
        float captureThreshold = 0.7f;
    };

    /**
     * Constructor
     */
    WildlifeAnalyzer();

    /**
     * Destructor
     */
    ~WildlifeAnalyzer();

    /**
     * @brief Initialize wildlife analyzer
     * @param config Analyzer configuration
     * @return true if initialization successful
     */
    bool initialize(const AnalyzerConfig& config = AnalyzerConfig());

    /**
     * @brief Add motion data point for analysis
     * @param dataPoint Motion data from detection system
     */
    void addMotionData(const MotionDataPoint& dataPoint);

    /**
     * @brief Analyze accumulated motion data
     * @param currentTimeHour Current hour (0-23) for time adaptation
     * @param temperature Current temperature for behavioral adaptation
     * @param lightLevel Current light level (0-1)
     * @return Analysis result with pattern classification
     */
    WildlifeAnalysisResult analyzePattern(uint8_t currentTimeHour = 12,
                                         float temperature = 20.0f,
                                         float lightLevel = 0.5f);

    /**
     * @brief Get quick analysis for real-time decisions
     * @param recentDataPoints Recent motion data (last 2-3 seconds)
     * @return Quick analysis result
     */
    WildlifeAnalysisResult quickAnalysis(const std::vector<MotionDataPoint>& recentDataPoints);

    /**
     * @brief Learn from user feedback to improve pattern recognition
     * @param pattern User-confirmed pattern type
     * @param characteristics Movement characteristics
     * @param confidence User confidence in classification (0-1)
     */
    void learnPattern(MovementPattern pattern, 
                     const MovementCharacteristics& characteristics,
                     float confidence = 1.0f);

    /**
     * @brief Get statistics about pattern recognition accuracy
     * @return JSON string with statistics
     */
    String getAnalysisStats() const;

    /**
     * @brief Reset accumulated data and start fresh
     */
    void reset();

    /**
     * @brief Configure analyzer settings
     * @param config New configuration
     */
    void configure(const AnalyzerConfig& config) { config_ = config; }

    /**
     * @brief Get configuration as JSON string
     * @return JSON configuration
     */
    String getConfigJSON() const;

    /**
     * @brief Load configuration from JSON
     * @param jsonConfig JSON configuration string
     * @return true if loaded successfully
     */
    bool loadConfigJSON(const String& jsonConfig);

    /**
     * @brief Get current time-of-day activity pattern
     * @param hour Current hour (0-23)
     * @return Expected activity level and common patterns
     */
    TimePattern getTimePattern(uint8_t hour) const;

    /**
     * @brief Update time-of-day patterns based on observations
     * @param hour Hour when pattern was observed
     * @param pattern Pattern that was observed
     * @param confidence Confidence in observation
     */
    void updateTimePattern(uint8_t hour, MovementPattern pattern, float confidence);

private:
    // Configuration
    AnalyzerConfig config_;
    bool initialized_;
    
    // Motion data storage
    std::vector<MotionDataPoint> motionData_;
    uint32_t lastAnalysisTime_;
    uint32_t totalAnalyses_;
    
    // Pattern learning storage
    std::map<MovementPattern, std::vector<MovementCharacteristics>> learnedPatterns_;
    std::map<MovementPattern, uint32_t> patternCounts_;
    
    // Time-of-day patterns
    std::map<uint8_t, TimePattern> timePatterns_;
    
    // Statistics
    mutable uint32_t totalClassifications_;
    mutable uint32_t wildlifeDetections_;
    mutable uint32_t falsePositives_;
    mutable uint32_t captures_;
    
    // Helper methods
    MovementCharacteristics calculateCharacteristics(const std::vector<MotionDataPoint>& data);
    MovementPattern classifyPattern(const MovementCharacteristics& characteristics, 
                                   uint8_t timeHour, float temperature, float lightLevel);
    float calculateWildlifeScore(const MovementCharacteristics& characteristics, 
                                MovementPattern pattern);
    float calculateInterestScore(const MovementCharacteristics& characteristics, 
                                MovementPattern pattern, uint8_t timeHour);
    float calculatePatternConfidence(MovementPattern pattern, 
                                   const MovementCharacteristics& characteristics);
    bool isLikelyVegetation(const MovementCharacteristics& characteristics);
    bool isLikelyWeather(const MovementCharacteristics& characteristics);
    bool isLikelyInsect(const MovementCharacteristics& characteristics);
    bool isLikelyVehicle(const MovementCharacteristics& characteristics);
    void cleanupOldData();
    void initializeTimePatterns();
    String getPatternString(MovementPattern pattern) const;
    
    // Analysis algorithms
    float calculateSpeed(const std::vector<MotionDataPoint>& data);
    float calculateDirection(const std::vector<MotionDataPoint>& data);
    float calculateDirectionStability(const std::vector<MotionDataPoint>& data);
    float calculatePeriodicity(const std::vector<MotionDataPoint>& data);
    float calculateVerticality(const std::vector<MotionDataPoint>& data);
    float calculateAverageSize(const std::vector<MotionDataPoint>& data);
    uint32_t calculateDwellTime(const std::vector<MotionDataPoint>& data);
    uint32_t calculateActiveTime(const std::vector<MotionDataPoint>& data);
    float calculateAverageIntensity(const std::vector<MotionDataPoint>& data);
};

#endif // WILDLIFE_ANALYZER_H