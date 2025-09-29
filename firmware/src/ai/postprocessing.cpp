/**
 * @file postprocessing.cpp
 * @brief Post-processing pipeline implementation for wildlife detection results
 */

#include "postprocessing.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <math.h>
#include <string.h>
#include <algorithm>

static const char* TAG = "PostProcessor";

// Default post-processing configurations
const PostProcessingConfig SPECIES_DETECTION_CONFIG = {
    .confidenceThreshold = 0.5f,
    .classSpecificThresholds = {0.5f}, // Initialize all to 0.5f
    .nmsThreshold = 0.5f,
    .enableNMS = true,
    .enableTemporalFilter = true,
    .temporalWindowSize = 5,
    .temporalThreshold = 0.7f,
    .enableTracking = true,
    .trackingThreshold = 0.3f,
    .maxTracks = 10,
    .minBboxArea = 0.01f,
    .maxBboxArea = 0.8f,
    .edgeMargin = 0.05f,
    .filterEdgeDetections = true
};

const PostProcessingConfig MOTION_DETECTION_CONFIG = {
    .confidenceThreshold = 0.3f,
    .classSpecificThresholds = {0.3f}, // Initialize all to 0.3f
    .nmsThreshold = 0.4f,
    .enableNMS = true,
    .enableTemporalFilter = true,
    .temporalWindowSize = 3,
    .temporalThreshold = 0.6f,
    .enableTracking = false,
    .trackingThreshold = 0.4f,
    .maxTracks = 5,
    .minBboxArea = 0.005f,
    .maxBboxArea = 0.9f,
    .edgeMargin = 0.02f,
    .filterEdgeDetections = false
};

const PostProcessingConfig HIGH_PRECISION_CONFIG = {
    .confidenceThreshold = 0.8f,
    .classSpecificThresholds = {0.8f}, // Initialize all to 0.8f
    .nmsThreshold = 0.3f,
    .enableNMS = true,
    .enableTemporalFilter = true,
    .temporalWindowSize = 7,
    .temporalThreshold = 0.8f,
    .enableTracking = true,
    .trackingThreshold = 0.2f,
    .maxTracks = 15,
    .minBboxArea = 0.02f,
    .maxBboxArea = 0.7f,
    .edgeMargin = 0.1f,
    .filterEdgeDetections = true
};

const PostProcessingConfig REAL_TIME_CONFIG = {
    .confidenceThreshold = 0.4f,
    .classSpecificThresholds = {0.4f}, // Initialize all to 0.4f
    .nmsThreshold = 0.6f,
    .enableNMS = false,
    .enableTemporalFilter = false,
    .temporalWindowSize = 2,
    .temporalThreshold = 0.5f,
    .enableTracking = false,
    .trackingThreshold = 0.5f,
    .maxTracks = 5,
    .minBboxArea = 0.01f,
    .maxBboxArea = 0.9f,
    .edgeMargin = 0.02f,
    .filterEdgeDetections = false
};

PostProcessor::PostProcessor()
    : initialized_(false)
    , historyIndex_(0)
    , historyCount_(0)
    , trackCount_(0)
    , nextTrackId_(1)
    , totalProcessed_(0)
    , totalFiltered_(0)
    , totalProcessingTime_(0)
    , frameCount_(0) {
    
    memset(&config_, 0, sizeof(config_));
    memset(history_, 0, sizeof(history_));
    memset(tracks_, 0, sizeof(tracks_));
    
    // Initialize class-specific thresholds to default
    for (int i = 0; i < MAX_CLASSES; i++) {
        config_.classSpecificThresholds[i] = 0.5f;
    }
}

PostProcessor::~PostProcessor() {
    // Cleanup if needed
}

bool PostProcessor::initialize(const PostProcessingConfig& config) {
    config_ = config;
    
    // Reset all state
    historyIndex_ = 0;
    historyCount_ = 0;
    trackCount_ = 0;
    nextTrackId_ = 1;
    
    memset(history_, 0, sizeof(history_));
    memset(tracks_, 0, sizeof(tracks_));
    
    initialized_ = true;
    
    ESP_LOGI(TAG, "Post-processor initialized successfully");
    ESP_LOGI(TAG, "Confidence threshold: %.3f, NMS threshold: %.3f", 
             config_.confidenceThreshold, config_.nmsThreshold);
    ESP_LOGI(TAG, "Temporal filtering: %s, Tracking: %s", 
             config_.enableTemporalFilter ? "enabled" : "disabled",
             config_.enableTracking ? "enabled" : "disabled");
    
    return true;
}

uint8_t PostProcessor::processDetections(const RawDetection* rawDetections,
                                        uint8_t numRawDetections,
                                        ProcessedDetection* processedDetections,
                                        uint8_t maxProcessedDetections) {
    if (!initialized_ || !rawDetections || !processedDetections) {
        ESP_LOGE(TAG, "Invalid parameters for detection processing");
        return 0;
    }
    
    uint32_t startTime = esp_timer_get_time() / 1000; // Convert to milliseconds
    uint32_t currentTimestamp = startTime;
    
    ESP_LOGD(TAG, "Processing %d raw detections", numRawDetections);
    
    // Copy raw detections to working buffer
    RawDetection workingDetections[MAX_DETECTIONS];
    uint8_t workingCount = std::min((int)numRawDetections, MAX_DETECTIONS);
    memcpy(workingDetections, rawDetections, workingCount * sizeof(RawDetection));
    
    // Stage 1: Apply confidence filtering
    workingCount = applyConfidenceFiltering(workingDetections, workingCount);
    ESP_LOGD(TAG, "After confidence filtering: %d detections", workingCount);
    
    // Stage 2: Apply size and edge filtering
    workingCount = applySizeAndEdgeFiltering(workingDetections, workingCount);
    ESP_LOGD(TAG, "After size/edge filtering: %d detections", workingCount);
    
    // Stage 3: Apply non-maximum suppression
    if (config_.enableNMS) {
        workingCount = applyNonMaximumSuppression(workingDetections, workingCount);
        ESP_LOGD(TAG, "After NMS: %d detections", workingCount);
    }
    
    // Stage 4: Apply temporal filtering
    if (config_.enableTemporalFilter) {
        workingCount = applyTemporalFiltering(workingDetections, workingCount);
        ESP_LOGD(TAG, "After temporal filtering: %d detections", workingCount);
    }
    
    // Stage 5: Update object tracking
    uint8_t finalCount = 0;
    if (config_.enableTracking) {
        finalCount = updateObjectTracking(workingDetections, workingCount, processedDetections);
    } else {
        // Direct conversion without tracking
        finalCount = std::min((int)workingCount, (int)maxProcessedDetections);
        for (uint8_t i = 0; i < finalCount; i++) {
            processedDetections[i].classId = workingDetections[i].classId;
            processedDetections[i].confidence = workingDetections[i].confidence;
            processedDetections[i].bbox = workingDetections[i].bbox;
            processedDetections[i].timestamp = currentTimestamp;
            processedDetections[i].stability = 1.0f;
            processedDetections[i].trackId = 0;
            processedDetections[i].isNew = true;
        }
    }
    
    // Add to history for temporal filtering
    if (config_.enableTemporalFilter) {
        addToHistory(workingDetections, workingCount, currentTimestamp);
    }
    
    // Clean up old tracks
    if (config_.enableTracking) {
        cleanupOldTracks(currentTimestamp);
    }
    
    uint32_t endTime = esp_timer_get_time() / 1000;
    uint32_t processingTime = endTime - startTime;
    
    // Update statistics
    updateStatistics(processingTime, numRawDetections, finalCount);
    
    ESP_LOGD(TAG, "Post-processing completed in %lu ms: %d -> %d detections", 
             processingTime, numRawDetections, finalCount);
    
    return finalCount;
}

void PostProcessor::updateConfig(const PostProcessingConfig& config) {
    config_ = config;
    ESP_LOGI(TAG, "Post-processing configuration updated");
}

void PostProcessor::reset() {
    historyIndex_ = 0;
    historyCount_ = 0;
    trackCount_ = 0;
    nextTrackId_ = 1;
    
    memset(history_, 0, sizeof(history_));
    memset(tracks_, 0, sizeof(tracks_));
    
    ESP_LOGI(TAG, "Post-processor state reset");
}

void PostProcessor::getStatistics(uint32_t* totalProcessed, uint32_t* totalFiltered,
                                 uint32_t* avgProcessingTime) const {
    if (totalProcessed) *totalProcessed = totalProcessed_;
    if (totalFiltered) *totalFiltered = totalFiltered_;
    if (avgProcessingTime) {
        *avgProcessingTime = frameCount_ > 0 ? totalProcessingTime_ / frameCount_ : 0;
    }
}

uint8_t PostProcessor::applyConfidenceFiltering(RawDetection* detections, uint8_t count) {
    uint8_t validCount = 0;
    
    for (uint8_t i = 0; i < count; i++) {
        float threshold = config_.confidenceThreshold;
        
        // Use class-specific threshold if available
        if (detections[i].classId < MAX_CLASSES) {
            threshold = config_.classSpecificThresholds[detections[i].classId];
        }
        
        if (detections[i].confidence >= threshold) {
            if (validCount != i) {
                detections[validCount] = detections[i];
            }
            validCount++;
        }
    }
    
    totalFiltered_ += (count - validCount);
    return validCount;
}

uint8_t PostProcessor::applyNonMaximumSuppression(RawDetection* detections, uint8_t count) {
    if (count <= 1) return count;
    
    // Sort detections by confidence (descending)
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = i + 1; j < count; j++) {
            if (detections[j].confidence > detections[i].confidence) {
                RawDetection temp = detections[i];
                detections[i] = detections[j];
                detections[j] = temp;
            }
        }
    }
    
    // Apply NMS
    bool suppressed[MAX_DETECTIONS] = {false};
    uint8_t validCount = 0;
    
    for (uint8_t i = 0; i < count; i++) {
        if (suppressed[i]) continue;
        
        // Keep this detection
        if (validCount != i) {
            detections[validCount] = detections[i];
        }
        validCount++;
        
        // Suppress overlapping detections of the same class
        for (uint8_t j = i + 1; j < count; j++) {
            if (suppressed[j]) continue;
            
            if (detections[i].classId == detections[j].classId) {
                float iou = calculateIoU(detections[i].bbox, detections[j].bbox);
                if (iou > config_.nmsThreshold) {
                    suppressed[j] = true;
                }
            }
        }
    }
    
    return validCount;
}

uint8_t PostProcessor::applyTemporalFiltering(RawDetection* currentDetections, uint8_t count) {
    if (historyCount_ == 0) {
        // No history yet, accept all detections
        return count;
    }
    
    uint8_t validCount = 0;
    
    for (uint8_t i = 0; i < count; i++) {
        float stability = calculateTemporalStability(currentDetections[i]);
        
        if (stability >= config_.temporalThreshold) {
            if (validCount != i) {
                currentDetections[validCount] = currentDetections[i];
            }
            validCount++;
        }
    }
    
    return validCount;
}

uint8_t PostProcessor::applySizeAndEdgeFiltering(RawDetection* detections, uint8_t count) {
    uint8_t validCount = 0;
    
    for (uint8_t i = 0; i < count; i++) {
        const BoundingBox& bbox = detections[i].bbox;
        
        // Calculate bounding box area
        float area = bbox.width * bbox.height;
        
        // Check size constraints
        if (area < config_.minBboxArea || area > config_.maxBboxArea) {
            continue;
        }
        
        // Check edge constraints
        if (config_.filterEdgeDetections) {
            float margin = config_.edgeMargin;
            if (bbox.x - bbox.width/2 < margin ||
                bbox.x + bbox.width/2 > 1.0f - margin ||
                bbox.y - bbox.height/2 < margin ||
                bbox.y + bbox.height/2 > 1.0f - margin) {
                continue;
            }
        }
        
        // Detection passed all filters
        if (validCount != i) {
            detections[validCount] = detections[i];
        }
        validCount++;
    }
    
    return validCount;
}

uint8_t PostProcessor::updateObjectTracking(const RawDetection* detections, uint8_t count,
                                           ProcessedDetection* processedDetections) {
    uint8_t processedCount = 0;
    bool matched[MAX_DETECTIONS] = {false};
    
    // Try to match detections with existing tracks
    for (uint8_t i = 0; i < count && processedCount < MAX_DETECTIONS; i++) {
        int8_t matchedTrack = findMatchingTrack(detections[i]);
        
        if (matchedTrack >= 0) {
            // Update existing track
            updateExistingTrack(matchedTrack, detections[i], &processedDetections[processedCount]);
            matched[i] = true;
        } else {
            // Create new track
            createNewTrack(detections[i], &processedDetections[processedCount]);
        }
        processedCount++;
    }
    
    return processedCount;
}

float PostProcessor::calculateIoU(const BoundingBox& box1, const BoundingBox& box2) {
    // Convert from center-width-height to corner coordinates
    float x1_min = box1.x - box1.width / 2;
    float y1_min = box1.y - box1.height / 2;
    float x1_max = box1.x + box1.width / 2;
    float y1_max = box1.y + box1.height / 2;
    
    float x2_min = box2.x - box2.width / 2;
    float y2_min = box2.y - box2.height / 2;
    float x2_max = box2.x + box2.width / 2;
    float y2_max = box2.y + box2.height / 2;
    
    // Calculate intersection
    float inter_x_min = std::max(x1_min, x2_min);
    float inter_y_min = std::max(y1_min, y2_min);
    float inter_x_max = std::min(x1_max, x2_max);
    float inter_y_max = std::min(y1_max, y2_max);
    
    if (inter_x_max <= inter_x_min || inter_y_max <= inter_y_min) {
        return 0.0f;
    }
    
    float intersection = (inter_x_max - inter_x_min) * (inter_y_max - inter_y_min);
    float area1 = box1.width * box1.height;
    float area2 = box2.width * box2.height;
    float union_area = area1 + area2 - intersection;
    
    return union_area > 0 ? intersection / union_area : 0.0f;
}

float PostProcessor::calculateDistance(const BoundingBox& box1, const BoundingBox& box2) {
    float dx = box1.x - box2.x;
    float dy = box1.y - box2.y;
    return sqrtf(dx * dx + dy * dy);
}

float PostProcessor::calculateTemporalStability(const RawDetection& detection) {
    if (historyCount_ == 0) return 0.0f;
    
    int matches = 0;
    int totalFrames = std::min((int)historyCount_, (int)config_.temporalWindowSize);
    
    // Check recent history frames
    for (int i = 0; i < totalFrames; i++) {
        int historyIdx = (historyIndex_ - 1 - i + 10) % 10;
        const DetectionFrame& frame = history_[historyIdx];
        
        // Look for similar detections in this frame
        for (uint8_t j = 0; j < frame.count; j++) {
            if (frame.detections[j].classId == detection.classId) {
                float distance = calculateDistance(detection.bbox, frame.detections[j].bbox);
                if (distance < config_.trackingThreshold) {
                    matches++;
                    break;
                }
            }
        }
    }
    
    return (float)matches / totalFrames;
}

void PostProcessor::addToHistory(const RawDetection* detections, uint8_t count, uint32_t timestamp) {
    DetectionFrame& frame = history_[historyIndex_];
    
    frame.count = std::min((int)count, MAX_DETECTIONS);
    memcpy(frame.detections, detections, frame.count * sizeof(RawDetection));
    frame.timestamp = timestamp;
    
    historyIndex_ = (historyIndex_ + 1) % 10;
    if (historyCount_ < 10) {
        historyCount_++;
    }
}

int8_t PostProcessor::findMatchingTrack(const RawDetection& detection) {
    float bestDistance = config_.trackingThreshold;
    int8_t bestTrack = -1;
    
    for (uint8_t i = 0; i < trackCount_; i++) {
        if (tracks_[i].classId == detection.classId) {
            float distance = calculateDistance(detection.bbox, tracks_[i].bbox);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTrack = i;
            }
        }
    }
    
    return bestTrack;
}

void PostProcessor::createNewTrack(const RawDetection& detection, ProcessedDetection* processed) {
    if (trackCount_ < config_.maxTracks) {
        ProcessedDetection& track = tracks_[trackCount_];
        
        track.classId = detection.classId;
        track.confidence = detection.confidence;
        track.bbox = detection.bbox;
        track.timestamp = detection.timestamp;
        track.stability = 0.5f; // Initial stability
        track.trackId = nextTrackId_++;
        track.isNew = true;
        
        *processed = track;
        trackCount_++;
    } else {
        // No space for new tracks, convert to processed detection without tracking
        processed->classId = detection.classId;
        processed->confidence = detection.confidence;
        processed->bbox = detection.bbox;
        processed->timestamp = detection.timestamp;
        processed->stability = 0.0f;
        processed->trackId = 0;
        processed->isNew = true;
    }
}

void PostProcessor::updateExistingTrack(uint8_t trackIndex, const RawDetection& detection,
                                       ProcessedDetection* processed) {
    ProcessedDetection& track = tracks_[trackIndex];
    
    // Update track with new detection
    track.confidence = 0.7f * track.confidence + 0.3f * detection.confidence; // Weighted average
    track.bbox = detection.bbox; // Use latest position
    track.timestamp = detection.timestamp;
    track.stability = std::min(1.0f, track.stability + 0.1f); // Increase stability
    track.isNew = false;
    
    *processed = track;
}

void PostProcessor::cleanupOldTracks(uint32_t currentTimestamp) {
    const uint32_t MAX_TRACK_AGE = 5000; // 5 seconds
    
    uint8_t validTracks = 0;
    for (uint8_t i = 0; i < trackCount_; i++) {
        if (currentTimestamp - tracks_[i].timestamp < MAX_TRACK_AGE) {
            if (validTracks != i) {
                tracks_[validTracks] = tracks_[i];
            }
            validTracks++;
        }
    }
    
    trackCount_ = validTracks;
}

void PostProcessor::updateStatistics(uint32_t processingTime, uint8_t inputCount, uint8_t outputCount) {
    totalProcessed_ += inputCount;
    totalProcessingTime_ += processingTime;
    frameCount_++;
    
    if (inputCount > outputCount) {
        totalFiltered_ += (inputCount - outputCount);
    }
}