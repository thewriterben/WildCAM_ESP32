/**
 * Wildlife Cloud Pipeline Implementation
 * 
 * Provides real-time processing pipeline for wildlife detection
 * events with cloud-based analysis and alert systems.
 */

#include "wildlife_cloud_pipeline.h"

// Global instance
WildlifeCloudPipeline* g_wildlifeCloudPipeline = nullptr;

WildlifeCloudPipeline::WildlifeCloudPipeline() :
    initialized_(false),
    realtimeEnabled_(false),
    detectionThreshold_(0.7f) {
}

WildlifeCloudPipeline::~WildlifeCloudPipeline() {
    cleanup();
}

bool WildlifeCloudPipeline::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Clear queues
    while (!eventQueue_.empty()) {
        eventQueue_.pop();
    }
    pendingAlerts_.clear();
    
    initialized_ = true;
    return true;
}

void WildlifeCloudPipeline::cleanup() {
    while (!eventQueue_.empty()) {
        eventQueue_.pop();
    }
    pendingAlerts_.clear();
    
    realtimeEnabled_ = false;
    initialized_ = false;
}

bool WildlifeCloudPipeline::processDetectionEvent(const WildlifeDetectionEvent& event) {
    if (!initialized_) {
        return false;
    }
    
    // Check if confidence meets threshold
    if (event.confidence < detectionThreshold_) {
        return false;
    }
    
    // Add to processing queue
    eventQueue_.push(event);
    
    // Check for conservation alerts
    checkConservationAlerts(event);
    
    // If real-time processing is enabled, process immediately
    if (realtimeEnabled_) {
        // Process event in real-time
        // Send to cloud for analysis
        // Generate notifications if needed
    }
    
    return true;
}

bool WildlifeCloudPipeline::processDetectionBatch(const std::vector<WildlifeDetectionEvent>& events) {
    if (!initialized_) {
        return false;
    }
    
    bool allSuccess = true;
    
    for (const auto& event : events) {
        if (!processDetectionEvent(event)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool WildlifeCloudPipeline::enableRealtimeProcessing(bool enable) {
    realtimeEnabled_ = enable;
    return true;
}

bool WildlifeCloudPipeline::setDetectionThreshold(float threshold) {
    if (threshold < 0.0f || threshold > 1.0f) {
        return false;
    }
    
    detectionThreshold_ = threshold;
    return true;
}

bool WildlifeCloudPipeline::checkConservationAlerts(const WildlifeDetectionEvent& event) {
    // Check for endangered species
    // Check for unusual behavior patterns
    // Check for protected area violations
    // Generate alerts as needed
    
    // Example: Check for high confidence endangered species detection
    if (event.confidence > 0.9f) {
        ConservationAlert alert;
        alert.alertId = String(millis()) + "-alert";
        alert.alertType = "ENDANGERED_SPECIES_DETECTION";
        alert.species = event.species;
        alert.location = String(event.latitude) + "," + String(event.longitude);
        alert.severity = "HIGH";
        alert.description = "High confidence detection of " + event.species;
        alert.timestamp = event.timestamp;
        alert.isProcessed = false;
        
        pendingAlerts_.push_back(alert);
    }
    
    return true;
}

std::vector<ConservationAlert> WildlifeCloudPipeline::getPendingAlerts() const {
    return pendingAlerts_;
}

bool WildlifeCloudPipeline::processAlert(const String& alertId) {
    for (auto& alert : pendingAlerts_) {
        if (alert.alertId == alertId) {
            alert.isProcessed = true;
            return true;
        }
    }
    return false;
}
