/**
 * @file ai_common.h
 * @brief Common AI/ML definitions and data structures
 * 
 * Defines common data structures, enums, and constants used across
 * the AI/ML modules for the ESP32 Wildlife Camera system.
 */

#ifndef AI_COMMON_H
#define AI_COMMON_H

#include <Arduino.h>
#include <vector>
#include <memory>

// Forward declarations
struct camera_fb_t;

/**
 * AI Processing Confidence Levels
 */
enum class ConfidenceLevel {
    VERY_LOW = 0,    // 0-20%
    LOW = 1,         // 20-40%
    MEDIUM = 2,      // 40-60%
    HIGH = 3,        // 60-80%
    VERY_HIGH = 4    // 80-100%
};

/**
 * Wildlife Species Categories
 */
enum class SpeciesType {
    UNKNOWN = 0,
    MAMMAL_SMALL,    // Rabbits, squirrels, etc.
    MAMMAL_MEDIUM,   // Foxes, raccoons, etc.
    MAMMAL_LARGE,    // Deer, bears, etc.
    BIRD_SMALL,      // Songbirds, finches, etc.
    BIRD_MEDIUM,     // Cardinals, jays, etc.
    BIRD_LARGE,      // Hawks, eagles, etc.
    REPTILE,         // Snakes, lizards, etc.
    HUMAN,           // Human activity detection
    VEHICLE          // Vehicle detection
};

/**
 * Animal Behavior Classifications
 */
enum class BehaviorType {
    UNKNOWN = 0,
    FEEDING,
    DRINKING,
    RESTING,
    MOVING,
    ALERT,
    GROOMING,
    MATING,
    AGGRESSIVE,
    TERRITORIAL,
    SOCIAL
};

/**
 * AI Processing Results Structure
 */
struct AIResult {
    SpeciesType species;
    BehaviorType behavior;
    float confidence;
    ConfidenceLevel confidenceLevel;
    uint16_t boundingBoxX;
    uint16_t boundingBoxY;
    uint16_t boundingBoxWidth;
    uint16_t boundingBoxHeight;
    unsigned long timestamp;
    bool isValidDetection;
    
    AIResult() : 
        species(SpeciesType::UNKNOWN),
        behavior(BehaviorType::UNKNOWN),
        confidence(0.0f),
        confidenceLevel(ConfidenceLevel::VERY_LOW),
        boundingBoxX(0), boundingBoxY(0),
        boundingBoxWidth(0), boundingBoxHeight(0),
        timestamp(0),
        isValidDetection(false) {}
};

/**
 * Camera Frame Data Structure for AI Processing
 */
struct CameraFrame {
    uint8_t* data;
    size_t length;
    uint16_t width;
    uint16_t height;
    uint8_t format;  // JPEG, RGB565, etc.
    unsigned long timestamp;
    
    CameraFrame() : 
        data(nullptr), length(0), width(0), height(0), 
        format(0), timestamp(0) {}
        
    CameraFrame(camera_fb_t* fb);  // Constructor from ESP32 camera frame
};

/**
 * Wildlife AI Model Types
 */
enum WildlifeModelType {
    MODEL_SPECIES_CLASSIFIER,
    MODEL_BEHAVIOR_ANALYZER,
    MODEL_MOTION_DETECTOR,
    MODEL_HUMAN_DETECTOR,
    MODEL_COUNT
};

/**
 * AI Model Information
 */
struct ModelInfo {
    String name;
    String version;
    String description;
    size_t modelSize;
    float accuracy;
    uint32_t checksum;
    bool isLoaded;
    
    ModelInfo() : 
        modelSize(0), accuracy(0.0f), checksum(0), isLoaded(false) {}
};

/**
 * AI Processing Performance Metrics
 */
struct AIMetrics {
    float inferenceTime;        // ms
    float preprocessingTime;    // ms
    float postprocessingTime;   // ms
    uint32_t totalInferences;
    uint32_t successfulInferences;
    uint32_t memoryUsage;       // bytes
    float powerConsumption;     // mA
    
    AIMetrics() : 
        inferenceTime(0.0f), preprocessingTime(0.0f), 
        postprocessingTime(0.0f), totalInferences(0),
        successfulInferences(0), memoryUsage(0), 
        powerConsumption(0.0f) {}
};

/**
 * AI Configuration Structure
 */
struct AIConfig {
    bool enableSpeciesClassification;
    bool enableBehaviorAnalysis;
    bool enableMotionDetection;
    bool enableObjectTracking;
    float confidenceThreshold;
    int maxDetectionsPerFrame;
    bool enablePowerOptimization;
    bool enableCloudSync;
    
    AIConfig() : 
        enableSpeciesClassification(true),
        enableBehaviorAnalysis(true),
        enableMotionDetection(true),
        enableObjectTracking(false),
        confidenceThreshold(0.6f),
        maxDetectionsPerFrame(5),
        enablePowerOptimization(true),
        enableCloudSync(false) {}
};

// Utility functions
const char* speciesTypeToString(SpeciesType species);
const char* behaviorTypeToString(BehaviorType behavior);
const char* confidenceLevelToString(ConfidenceLevel level);
ConfidenceLevel floatToConfidenceLevel(float confidence);

// Constants
static const float AI_CONFIDENCE_THRESHOLD_DEFAULT = 0.6f;
static const int AI_MAX_DETECTIONS_PER_FRAME = 5;
static const int AI_MODEL_CACHE_SIZE = 3;
static const unsigned long AI_INFERENCE_TIMEOUT_MS = 5000;

#endif // AI_COMMON_H