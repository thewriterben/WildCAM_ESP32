/**
 * Cloud Analytics Engine - ML Processing and Analytics
 * 
 * Provides cloud-based machine learning processing for wildlife
 * monitoring data with real-time analytics and insights.
 */

#ifndef CLOUD_ANALYTICS_ENGINE_H
#define CLOUD_ANALYTICS_ENGINE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../cloud_integrator.h"

// Analytics Types
enum AnalyticsType {
    ANALYTICS_SPECIES_DETECTION,
    ANALYTICS_BEHAVIOR_ANALYSIS,
    ANALYTICS_POPULATION_COUNT,
    ANALYTICS_MIGRATION_PATTERN,
    ANALYTICS_HABITAT_ASSESSMENT,
    ANALYTICS_CONSERVATION_ALERT
};

// ML Model Types
enum MLModelType {
    MODEL_YOLO_DETECTION,
    MODEL_CLASSIFICATION,
    MODEL_BEHAVIOR_RECOGNITION,
    MODEL_POPULATION_ESTIMATION,
    MODEL_CUSTOM
};

// Analytics Result
struct AnalyticsResult {
    String resultId;
    AnalyticsType type;
    float confidence;
    JsonDocument data;
    uint32_t processingTime;
    String modelVersion;
    
    AnalyticsResult() : 
        resultId(""), type(ANALYTICS_SPECIES_DETECTION), confidence(0.0),
        processingTime(0), modelVersion("") {}
};

/**
 * Cloud Analytics Engine Class
 * 
 * Cloud-based ML processing and analytics for wildlife data
 */
class CloudAnalyticsEngine {
public:
    CloudAnalyticsEngine();
    ~CloudAnalyticsEngine();
    
    // Initialization
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Analytics processing
    bool processWildlifeImage(const String& imagePath, AnalyticsResult& result);
    bool processVideoClip(const String& videoPath, AnalyticsResult& result);
    bool analyzeEnvironmentalData(const JsonDocument& sensorData, AnalyticsResult& result);
    
    // Batch processing
    bool processBatch(const std::vector<String>& imagePaths, std::vector<AnalyticsResult>& results);
    
    // Real-time analytics
    bool enableRealtimeAnalytics(bool enable);
    bool isRealtimeEnabled() const { return realtimeEnabled_; }
    
    // Model management
    bool loadModel(MLModelType type, const String& modelPath);
    bool updateModel(MLModelType type, const String& newModelPath);
    std::vector<MLModelType> getLoadedModels() const;

private:
    bool initialized_;
    bool realtimeEnabled_;
    std::map<MLModelType, String> loadedModels_;
};

// Global analytics engine instance
extern CloudAnalyticsEngine* g_cloudAnalyticsEngine;

#endif // CLOUD_ANALYTICS_ENGINE_H