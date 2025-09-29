/**
 * Wildlife Cloud Pipeline - Real-time Detection Processing
 * 
 * Provides real-time processing pipeline for wildlife detection
 * events with cloud-based analysis and alert systems.
 */

#ifndef WILDLIFE_CLOUD_PIPELINE_H
#define WILDLIFE_CLOUD_PIPELINE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <queue>
#include "../cloud_integrator.h"

// Detection Event
struct WildlifeDetectionEvent {
    String eventId;
    String species;
    float confidence;
    String imagePath;
    uint32_t timestamp;
    float latitude;
    float longitude;
    JsonDocument metadata;
    
    WildlifeDetectionEvent() : 
        eventId(""), species(""), confidence(0.0), imagePath(""),
        timestamp(0), latitude(0.0), longitude(0.0) {}
};

// Conservation Alert
struct ConservationAlert {
    String alertId;
    String alertType;
    String species;
    String location;
    String severity;
    String description;
    uint32_t timestamp;
    bool isProcessed;
    
    ConservationAlert() : 
        alertId(""), alertType(""), species(""), location(""),
        severity(""), description(""), timestamp(0), isProcessed(false) {}
};

/**
 * Wildlife Cloud Pipeline Class
 * 
 * Real-time wildlife detection processing and alert system
 */
class WildlifeCloudPipeline {
public:
    WildlifeCloudPipeline();
    ~WildlifeCloudPipeline();
    
    // Initialization
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Event processing
    bool processDetectionEvent(const WildlifeDetectionEvent& event);
    bool processDetectionBatch(const std::vector<WildlifeDetectionEvent>& events);
    
    // Real-time processing
    bool enableRealtimeProcessing(bool enable);
    bool isRealtimeEnabled() const { return realtimeEnabled_; }
    
    // Detection thresholds
    bool setDetectionThreshold(float threshold);
    float getDetectionThreshold() const { return detectionThreshold_; }
    
    // Conservation alerts
    bool checkConservationAlerts(const WildlifeDetectionEvent& event);
    std::vector<ConservationAlert> getPendingAlerts() const;
    bool processAlert(const String& alertId);

private:
    bool initialized_;
    bool realtimeEnabled_;
    float detectionThreshold_;
    std::queue<WildlifeDetectionEvent> eventQueue_;
    std::vector<ConservationAlert> pendingAlerts_;
};

// Global wildlife pipeline instance
extern WildlifeCloudPipeline* g_wildlifeCloudPipeline;

#endif // WILDLIFE_CLOUD_PIPELINE_H