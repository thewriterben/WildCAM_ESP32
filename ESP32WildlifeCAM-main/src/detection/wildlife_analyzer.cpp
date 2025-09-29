/**
 * @file wildlife_analyzer.cpp
 * @brief Wildlife Pattern Analyzer Implementation
 * @author ESP32WildlifeCAM Project  
 * @date 2025-01-01
 */

#include "wildlife_analyzer.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <cmath>

WildlifeAnalyzer::WildlifeAnalyzer()
    : initialized_(false)
    , lastAnalysisTime_(0)
    , totalAnalyses_(0)
    , totalClassifications_(0)
    , wildlifeDetections_(0)
    , falsePositives_(0)
    , captures_(0) {
}

WildlifeAnalyzer::~WildlifeAnalyzer() {
    // Cleanup if needed
}

bool WildlifeAnalyzer::initialize(const AnalyzerConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    
    // Reserve space for motion data
    motionData_.reserve(200);
    
    // Initialize time patterns
    initializeTimePatterns();
    
    // Initialize pattern counts
    for (int i = (int)MovementPattern::UNKNOWN; i <= (int)MovementPattern::HUMAN; i++) {
        patternCounts_[(MovementPattern)i] = 0;
    }
    
    initialized_ = true;
    Serial.println("WildlifeAnalyzer: Initialized successfully");
    
    return true;
}

void WildlifeAnalyzer::addMotionData(const MotionDataPoint& dataPoint) {
    if (!initialized_) return;
    
    motionData_.push_back(dataPoint);
    
    // Limit data size
    if (motionData_.size() > 500) {
        motionData_.erase(motionData_.begin());
    }
    
    // Cleanup old data periodically
    if (millis() % 30000 == 0) { // Every 30 seconds
        cleanupOldData();
    }
}

WildlifeAnalyzer::WildlifeAnalysisResult WildlifeAnalyzer::analyzePattern(uint8_t currentTimeHour,
                                                                         float temperature,
                                                                         float lightLevel) {
    if (!initialized_ || motionData_.empty()) {
        return WildlifeAnalysisResult{};
    }
    
    uint32_t currentTime = millis();
    
    // Get recent data for analysis
    std::vector<MotionDataPoint> recentData;
    for (const auto& point : motionData_) {
        if (currentTime - point.timestamp <= config_.analysisWindowMs) {
            recentData.push_back(point);
        }
    }
    
    if (recentData.size() < 3) {
        return WildlifeAnalysisResult{};
    }
    
    WildlifeAnalysisResult result;
    
    // Calculate movement characteristics
    result.characteristics = calculateCharacteristics(recentData);
    
    // Classify pattern
    result.primaryPattern = classifyPattern(result.characteristics, currentTimeHour, temperature, lightLevel);
    
    // Calculate confidence
    result.confidence = calculatePatternConfidence(result.primaryPattern, result.characteristics);
    
    // Calculate wildlife score
    result.wildlifeScore = calculateWildlifeScore(result.characteristics, result.primaryPattern);
    
    // Calculate interest score
    result.interestScore = calculateInterestScore(result.characteristics, result.primaryPattern, currentTimeHour);
    
    // Make decisions
    result.isWildlife = result.wildlifeScore >= config_.wildlifeConfidenceThreshold;
    result.shouldCapture = result.interestScore >= config_.captureThreshold;
    result.shouldTriggerAlert = result.isWildlife && result.confidence > 0.8f;
    
    // Build description
    result.description = getPatternString(result.primaryPattern) + 
                        " (conf: " + String(result.confidence, 2) + 
                        ", wildlife: " + String(result.wildlifeScore, 2) + 
                        ", interest: " + String(result.interestScore, 2) + ")";
    
    // Update statistics
    totalClassifications_++;
    if (result.isWildlife) wildlifeDetections_++;
    if (result.shouldCapture) captures_++;
    
    patternCounts_[result.primaryPattern]++;
    lastAnalysisTime_ = currentTime;
    
    return result;
}

WildlifeAnalyzer::WildlifeAnalysisResult WildlifeAnalyzer::quickAnalysis(const std::vector<MotionDataPoint>& recentDataPoints) {
    if (!initialized_ || recentDataPoints.size() < 2) {
        return WildlifeAnalysisResult{};
    }
    
    WildlifeAnalysisResult result;
    
    // Quick characteristic calculation
    result.characteristics = calculateCharacteristics(recentDataPoints);
    
    // Simple pattern classification for real-time decisions
    if (result.characteristics.size < config_.maxInsectSize) {
        result.primaryPattern = MovementPattern::INSECT;
        result.wildlifeScore = 0.3f;
    } else if (result.characteristics.periodicity > config_.vegetationPeriodicityThreshold) {
        result.primaryPattern = MovementPattern::VEGETATION;
        result.wildlifeScore = 0.1f;
    } else if (result.characteristics.speed > 100.0f && result.characteristics.directionStability > 0.8f) {
        result.primaryPattern = MovementPattern::VEHICLE;
        result.wildlifeScore = 0.0f;
    } else if (result.characteristics.size > config_.minLargeMammalSize) {
        result.primaryPattern = MovementPattern::LARGE_MAMMAL;
        result.wildlifeScore = 0.9f;
    } else if (result.characteristics.speed > 20.0f && result.characteristics.size < 0.2f) {
        result.primaryPattern = MovementPattern::SMALL_BIRD;
        result.wildlifeScore = 0.8f;
    } else {
        result.primaryPattern = MovementPattern::MEDIUM_MAMMAL;
        result.wildlifeScore = 0.7f;
    }
    
    result.confidence = 0.6f; // Lower confidence for quick analysis
    result.interestScore = result.wildlifeScore * 0.8f;
    result.isWildlife = result.wildlifeScore >= 0.5f;
    result.shouldCapture = result.interestScore >= 0.6f;
    
    return result;
}

void WildlifeAnalyzer::learnPattern(MovementPattern pattern, 
                                   const MovementCharacteristics& characteristics,
                                   float confidence) {
    if (!initialized_ || confidence < 0.5f) return;
    
    // Store learned pattern
    learnedPatterns_[pattern].push_back(characteristics);
    
    // Limit learned examples per pattern
    if (learnedPatterns_[pattern].size() > 20) {
        learnedPatterns_[pattern].erase(learnedPatterns_[pattern].begin());
    }
    
    Serial.printf("WildlifeAnalyzer: Learned %s pattern with confidence %.2f\n",
                  getPatternString(pattern).c_str(), confidence);
}

String WildlifeAnalyzer::getAnalysisStats() const {
    DynamicJsonDocument doc(1024);
    
    doc["totalClassifications"] = totalClassifications_;
    doc["wildlifeDetections"] = wildlifeDetections_;
    doc["falsePositives"] = falsePositives_;
    doc["captures"] = captures_;
    
    if (totalClassifications_ > 0) {
        doc["wildlifeRate"] = (float)wildlifeDetections_ / totalClassifications_;
        doc["captureRate"] = (float)captures_ / totalClassifications_;
    }
    
    JsonObject patterns = doc.createNestedObject("patternCounts");
    for (const auto& pair : patternCounts_) {
        patterns[getPatternString(pair.first)] = pair.second;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

void WildlifeAnalyzer::reset() {
    motionData_.clear();
    lastAnalysisTime_ = 0;
    totalAnalyses_ = 0;
    
    // Reset statistics
    totalClassifications_ = 0;
    wildlifeDetections_ = 0;
    falsePositives_ = 0;
    captures_ = 0;
    
    // Clear pattern counts
    for (auto& pair : patternCounts_) {
        pair.second = 0;
    }
    
    Serial.println("WildlifeAnalyzer: Reset completed");
}

String WildlifeAnalyzer::getConfigJSON() const {
    DynamicJsonDocument doc(1024);
    
    doc["enabled"] = config_.enabled;
    doc["analysisWindowMs"] = config_.analysisWindowMs;
    doc["minTrackingTimeMs"] = config_.minTrackingTimeMs;
    doc["minMovementThreshold"] = config_.minMovementThreshold;
    doc["maxInsectSize"] = config_.maxInsectSize;
    doc["minLargeMammalSize"] = config_.minLargeMammalSize;
    doc["vegetationPeriodicityThreshold"] = config_.vegetationPeriodicityThreshold;
    doc["useTimeOfDayAdaptation"] = config_.useTimeOfDayAdaptation;
    doc["useSizeFiltering"] = config_.useSizeFiltering;
    doc["useMovementFiltering"] = config_.useMovementFiltering;
    doc["wildlifeConfidenceThreshold"] = config_.wildlifeConfidenceThreshold;
    doc["captureThreshold"] = config_.captureThreshold;
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool WildlifeAnalyzer::loadConfigJSON(const String& jsonConfig) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonConfig);
    
    if (error) {
        Serial.println("WildlifeAnalyzer: Failed to parse JSON config");
        return false;
    }
    
    if (doc.containsKey("enabled")) config_.enabled = doc["enabled"];
    if (doc.containsKey("analysisWindowMs")) config_.analysisWindowMs = doc["analysisWindowMs"];
    if (doc.containsKey("minTrackingTimeMs")) config_.minTrackingTimeMs = doc["minTrackingTimeMs"];
    if (doc.containsKey("minMovementThreshold")) config_.minMovementThreshold = doc["minMovementThreshold"];
    if (doc.containsKey("maxInsectSize")) config_.maxInsectSize = doc["maxInsectSize"];
    if (doc.containsKey("minLargeMammalSize")) config_.minLargeMammalSize = doc["minLargeMammalSize"];
    if (doc.containsKey("vegetationPeriodicityThreshold")) config_.vegetationPeriodicityThreshold = doc["vegetationPeriodicityThreshold"];
    if (doc.containsKey("useTimeOfDayAdaptation")) config_.useTimeOfDayAdaptation = doc["useTimeOfDayAdaptation"];
    if (doc.containsKey("useSizeFiltering")) config_.useSizeFiltering = doc["useSizeFiltering"];
    if (doc.containsKey("useMovementFiltering")) config_.useMovementFiltering = doc["useMovementFiltering"];
    if (doc.containsKey("wildlifeConfidenceThreshold")) config_.wildlifeConfidenceThreshold = doc["wildlifeConfidenceThreshold"];
    if (doc.containsKey("captureThreshold")) config_.captureThreshold = doc["captureThreshold"];
    
    Serial.println("WildlifeAnalyzer: Configuration loaded from JSON");
    return true;
}

WildlifeAnalyzer::TimePattern WildlifeAnalyzer::getTimePattern(uint8_t hour) const {
    auto it = timePatterns_.find(hour);
    if (it != timePatterns_.end()) {
        return it->second;
    }
    
    // Return default pattern
    TimePattern defaultPattern;
    defaultPattern.hour = hour;
    defaultPattern.activityLevel = 0.5f;
    defaultPattern.commonPatterns.push_back(MovementPattern::UNKNOWN);
    return defaultPattern;
}

void WildlifeAnalyzer::updateTimePattern(uint8_t hour, MovementPattern pattern, float confidence) {
    if (confidence < 0.3f) return;
    
    TimePattern& timePattern = timePatterns_[hour];
    timePattern.hour = hour;
    
    // Adjust activity level based on pattern
    float activityBoost = 0.0f;
    switch (pattern) {
        case MovementPattern::SMALL_BIRD:
        case MovementPattern::LARGE_BIRD:
        case MovementPattern::SMALL_MAMMAL:
        case MovementPattern::MEDIUM_MAMMAL:
        case MovementPattern::LARGE_MAMMAL:
            activityBoost = confidence * 0.1f;
            break;
        default:
            activityBoost = 0.0f;
            break;
    }
    
    timePattern.activityLevel = std::min(1.0f, timePattern.activityLevel + activityBoost);
    
    // Add pattern to common patterns if not already present
    if (std::find(timePattern.commonPatterns.begin(), timePattern.commonPatterns.end(), pattern) 
        == timePattern.commonPatterns.end()) {
        timePattern.commonPatterns.push_back(pattern);
    }
}

// Private method implementations

WildlifeAnalyzer::MovementCharacteristics WildlifeAnalyzer::calculateCharacteristics(const std::vector<MotionDataPoint>& data) {
    MovementCharacteristics characteristics;
    
    if (data.empty()) return characteristics;
    
    characteristics.speed = calculateSpeed(data);
    characteristics.direction = calculateDirection(data);
    characteristics.directionStability = calculateDirectionStability(data);
    characteristics.size = calculateAverageSize(data);
    characteristics.periodicity = calculatePeriodicity(data);
    characteristics.verticality = calculateVerticality(data);
    characteristics.dwellTime = calculateDwellTime(data);
    characteristics.activeTime = calculateActiveTime(data);
    characteristics.intensity = calculateAverageIntensity(data);
    
    return characteristics;
}

WildlifeAnalyzer::MovementPattern WildlifeAnalyzer::classifyPattern(const MovementCharacteristics& characteristics,
                                                                   uint8_t timeHour, 
                                                                   float temperature, 
                                                                   float lightLevel) {
    // Check for non-wildlife patterns first
    if (isLikelyVegetation(characteristics)) {
        return MovementPattern::VEGETATION;
    }
    
    if (isLikelyWeather(characteristics)) {
        return MovementPattern::WEATHER;
    }
    
    if (isLikelyInsect(characteristics)) {
        return MovementPattern::INSECT;
    }
    
    if (isLikelyVehicle(characteristics)) {
        return MovementPattern::VEHICLE;
    }
    
    // Classify wildlife patterns
    if (characteristics.size > config_.minLargeMammalSize) {
        if (characteristics.speed < 5.0f) {
            return MovementPattern::LARGE_MAMMAL;
        } else if (characteristics.verticality > 0.7f) {
            return MovementPattern::HUMAN;
        }
    }
    
    if (characteristics.speed > 30.0f && characteristics.size < 0.2f) {
        if (characteristics.verticality > 0.3f) {
            return MovementPattern::LARGE_BIRD;
        } else {
            return MovementPattern::SMALL_BIRD;
        }
    }
    
    if (characteristics.size > 0.1f && characteristics.size < 0.3f) {
        if (characteristics.speed > 10.0f) {
            return MovementPattern::SMALL_MAMMAL;
        } else {
            return MovementPattern::MEDIUM_MAMMAL;
        }
    }
    
    // Default to medium mammal for unclassified wildlife
    return MovementPattern::MEDIUM_MAMMAL;
}

float WildlifeAnalyzer::calculateWildlifeScore(const MovementCharacteristics& characteristics, MovementPattern pattern) {
    float score = 0.0f;
    
    switch (pattern) {
        case MovementPattern::SMALL_BIRD:
        case MovementPattern::LARGE_BIRD:
        case MovementPattern::SMALL_MAMMAL:
        case MovementPattern::MEDIUM_MAMMAL:
        case MovementPattern::LARGE_MAMMAL:
            score = 0.9f;
            break;
        case MovementPattern::INSECT:
            score = 0.3f;
            break;
        case MovementPattern::VEGETATION:
        case MovementPattern::WEATHER:
        case MovementPattern::VEHICLE:
            score = 0.0f;
            break;
        case MovementPattern::HUMAN:
            score = 0.1f; // Humans are interesting but not wildlife
            break;
        default:
            score = 0.5f;
            break;
    }
    
    // Adjust based on characteristics
    if (characteristics.intensity > 0.7f) score += 0.1f;
    if (characteristics.dwellTime > 2000) score += 0.05f;
    if (characteristics.size > 0.05f && characteristics.size < 0.8f) score += 0.05f;
    
    return std::min(1.0f, score);
}

float WildlifeAnalyzer::calculateInterestScore(const MovementCharacteristics& characteristics, 
                                              MovementPattern pattern, uint8_t timeHour) {
    float baseScore = calculateWildlifeScore(characteristics, pattern);
    
    // Adjust for time of day
    if (config_.useTimeOfDayAdaptation) {
        TimePattern timePattern = getTimePattern(timeHour);
        baseScore *= (0.5f + timePattern.activityLevel * 0.5f);
    }
    
    // Bonus for larger animals
    if (characteristics.size > 0.2f) {
        baseScore += 0.1f;
    }
    
    // Bonus for longer observation
    if (characteristics.dwellTime > 5000) {
        baseScore += 0.1f;
    }
    
    return std::min(1.0f, baseScore);
}

float WildlifeAnalyzer::calculatePatternConfidence(MovementPattern pattern, 
                                                  const MovementCharacteristics& characteristics) {
    float confidence = 0.5f; // Base confidence
    
    // Higher confidence for clear patterns
    switch (pattern) {
        case MovementPattern::VEGETATION:
            if (characteristics.periodicity > 0.7f) confidence = 0.9f;
            break;
        case MovementPattern::VEHICLE:
            if (characteristics.directionStability > 0.8f && characteristics.speed > 50.0f) confidence = 0.9f;
            break;
        case MovementPattern::INSECT:
            if (characteristics.size < 0.03f && characteristics.speed > 20.0f) confidence = 0.8f;
            break;
        case MovementPattern::LARGE_MAMMAL:
            if (characteristics.size > 0.4f && characteristics.speed < 10.0f) confidence = 0.85f;
            break;
        default:
            confidence = 0.6f;
            break;
    }
    
    // Adjust based on data quality
    if (characteristics.dwellTime > 3000) confidence += 0.1f;
    if (characteristics.intensity > 0.8f) confidence += 0.05f;
    
    return std::min(1.0f, confidence);
}

bool WildlifeAnalyzer::isLikelyVegetation(const MovementCharacteristics& characteristics) {
    return characteristics.periodicity > config_.vegetationPeriodicityThreshold &&
           characteristics.speed < 5.0f &&
           characteristics.directionStability < 0.3f;
}

bool WildlifeAnalyzer::isLikelyWeather(const MovementCharacteristics& characteristics) {
    return characteristics.size > 0.5f && // Large area affected
           characteristics.periodicity > 0.4f &&
           characteristics.intensity < 0.4f; // Low intensity, widespread movement
}

bool WildlifeAnalyzer::isLikelyInsect(const MovementCharacteristics& characteristics) {
    return characteristics.size < config_.maxInsectSize &&
           characteristics.speed > 15.0f &&
           characteristics.directionStability < 0.5f;
}

bool WildlifeAnalyzer::isLikelyVehicle(const MovementCharacteristics& characteristics) {
    return characteristics.speed > 30.0f &&
           characteristics.directionStability > 0.7f &&
           characteristics.periodicity < 0.2f;
}

void WildlifeAnalyzer::cleanupOldData() {
    uint32_t currentTime = millis();
    uint32_t maxAge = config_.analysisWindowMs * 3; // Keep 3x analysis window
    
    auto it = std::remove_if(motionData_.begin(), motionData_.end(),
        [currentTime, maxAge](const MotionDataPoint& point) {
            return currentTime - point.timestamp > maxAge;
        });
    
    motionData_.erase(it, motionData_.end());
}

void WildlifeAnalyzer::initializeTimePatterns() {
    // Initialize basic time patterns for wildlife activity
    // Early morning (5-8): High bird activity
    for (uint8_t h = 5; h <= 8; h++) {
        TimePattern& pattern = timePatterns_[h];
        pattern.hour = h;
        pattern.activityLevel = 0.8f;
        pattern.commonPatterns = {MovementPattern::SMALL_BIRD, MovementPattern::LARGE_BIRD};
    }
    
    // Morning (9-11): Moderate mammal activity
    for (uint8_t h = 9; h <= 11; h++) {
        TimePattern& pattern = timePatterns_[h];
        pattern.hour = h;
        pattern.activityLevel = 0.6f;
        pattern.commonPatterns = {MovementPattern::SMALL_MAMMAL, MovementPattern::MEDIUM_MAMMAL};
    }
    
    // Midday (12-15): Lower activity
    for (uint8_t h = 12; h <= 15; h++) {
        TimePattern& pattern = timePatterns_[h];
        pattern.hour = h;
        pattern.activityLevel = 0.3f;
        pattern.commonPatterns = {MovementPattern::INSECT};
    }
    
    // Evening (16-19): High mammal activity
    for (uint8_t h = 16; h <= 19; h++) {
        TimePattern& pattern = timePatterns_[h];
        pattern.hour = h;
        pattern.activityLevel = 0.9f;
        pattern.commonPatterns = {MovementPattern::MEDIUM_MAMMAL, MovementPattern::LARGE_MAMMAL};
    }
    
    // Night (20-4): Lower activity, nocturnal animals
    std::vector<uint8_t> nightHours = {20, 21, 22, 23, 0, 1, 2, 3, 4};
    for (uint8_t h : nightHours) {
        TimePattern& pattern = timePatterns_[h];
        pattern.hour = h;
        pattern.activityLevel = 0.4f;
        pattern.commonPatterns = {MovementPattern::SMALL_MAMMAL};
    }
}

String WildlifeAnalyzer::getPatternString(MovementPattern pattern) const {
    switch (pattern) {
        case MovementPattern::UNKNOWN: return "Unknown";
        case MovementPattern::SMALL_BIRD: return "Small Bird";
        case MovementPattern::LARGE_BIRD: return "Large Bird";
        case MovementPattern::SMALL_MAMMAL: return "Small Mammal";
        case MovementPattern::MEDIUM_MAMMAL: return "Medium Mammal";
        case MovementPattern::LARGE_MAMMAL: return "Large Mammal";
        case MovementPattern::INSECT: return "Insect";
        case MovementPattern::VEGETATION: return "Vegetation";
        case MovementPattern::WEATHER: return "Weather";
        case MovementPattern::VEHICLE: return "Vehicle";
        case MovementPattern::HUMAN: return "Human";
        default: return "Unknown";
    }
}

// Analysis algorithm implementations
float WildlifeAnalyzer::calculateSpeed(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 2) return 0.0f;
    
    float totalDistance = 0.0f;
    uint32_t totalTime = 0;
    
    for (size_t i = 1; i < data.size(); i++) {
        float dx = data[i].x - data[i-1].x;
        float dy = data[i].y - data[i-1].y;
        float distance = sqrt(dx*dx + dy*dy);
        uint32_t dt = data[i].timestamp - data[i-1].timestamp;
        
        totalDistance += distance;
        totalTime += dt;
    }
    
    return totalTime > 0 ? (totalDistance * 1000.0f) / totalTime : 0.0f; // pixels per second
}

float WildlifeAnalyzer::calculateDirection(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 2) return 0.0f;
    
    float dx = data.back().x - data.front().x;
    float dy = data.back().y - data.front().y;
    
    return atan2(dy, dx);
}

float WildlifeAnalyzer::calculateDirectionStability(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 3) return 0.0f;
    
    std::vector<float> directions;
    for (size_t i = 1; i < data.size(); i++) {
        float dx = data[i].x - data[i-1].x;
        float dy = data[i].y - data[i-1].y;
        if (dx != 0 || dy != 0) {
            directions.push_back(atan2(dy, dx));
        }
    }
    
    if (directions.empty()) return 0.0f;
    
    // Calculate variance in directions
    float meanDir = 0.0f;
    for (float dir : directions) {
        meanDir += dir;
    }
    meanDir /= directions.size();
    
    float variance = 0.0f;
    for (float dir : directions) {
        float diff = dir - meanDir;
        variance += diff * diff;
    }
    variance /= directions.size();
    
    // Convert variance to stability score (0-1)
    return std::max(0.0f, 1.0f - variance);
}

float WildlifeAnalyzer::calculatePeriodicity(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 10) return 0.0f;
    
    // Simple periodicity detection based on position oscillation
    std::vector<float> xPositions, yPositions;
    for (const auto& point : data) {
        xPositions.push_back(point.x);
        yPositions.push_back(point.y);
    }
    
    // Calculate autocorrelation for different lags
    float maxCorrelation = 0.0f;
    for (int lag = 2; lag < (int)data.size() / 3; lag++) {
        float correlation = 0.0f;
        int count = 0;
        
        for (int i = lag; i < (int)data.size(); i++) {
            correlation += (xPositions[i] - xPositions[i-lag]) * (xPositions[i] - xPositions[i-lag]);
            count++;
        }
        
        if (count > 0) {
            correlation = 1.0f / (1.0f + correlation / count);
            maxCorrelation = std::max(maxCorrelation, correlation);
        }
    }
    
    return maxCorrelation;
}

float WildlifeAnalyzer::calculateVerticality(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 2) return 0.0f;
    
    float totalVertical = 0.0f;
    float totalHorizontal = 0.0f;
    
    for (size_t i = 1; i < data.size(); i++) {
        float dx = abs((int)data[i].x - (int)data[i-1].x);
        float dy = abs((int)data[i].y - (int)data[i-1].y);
        
        totalHorizontal += dx;
        totalVertical += dy;
    }
    
    float totalMovement = totalHorizontal + totalVertical;
    return totalMovement > 0 ? totalVertical / totalMovement : 0.0f;
}

float WildlifeAnalyzer::calculateAverageSize(const std::vector<MotionDataPoint>& data) {
    if (data.empty()) return 0.0f;
    
    float totalSize = 0.0f;
    for (const auto& point : data) {
        // Normalize size relative to frame size (assuming 320x240)
        float size = (point.width * point.height) / (320.0f * 240.0f);
        totalSize += size;
    }
    
    return totalSize / data.size();
}

uint32_t WildlifeAnalyzer::calculateDwellTime(const std::vector<MotionDataPoint>& data) {
    if (data.empty()) return 0;
    return data.back().timestamp - data.front().timestamp;
}

uint32_t WildlifeAnalyzer::calculateActiveTime(const std::vector<MotionDataPoint>& data) {
    if (data.size() < 2) return 0;
    
    uint32_t activeTime = 0;
    for (size_t i = 1; i < data.size(); i++) {
        float dx = data[i].x - data[i-1].x;
        float dy = data[i].y - data[i-1].y;
        float movement = sqrt(dx*dx + dy*dy);
        
        if (movement > config_.minMovementThreshold) {
            activeTime += data[i].timestamp - data[i-1].timestamp;
        }
    }
    
    return activeTime;
}

float WildlifeAnalyzer::calculateAverageIntensity(const std::vector<MotionDataPoint>& data) {
    if (data.empty()) return 0.0f;
    
    float totalIntensity = 0.0f;
    for (const auto& point : data) {
        totalIntensity += point.intensity;
    }
    
    return totalIntensity / data.size();
}