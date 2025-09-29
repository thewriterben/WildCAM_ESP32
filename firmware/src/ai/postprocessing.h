/**
 * @file postprocessing.h
 * @brief Post-processing pipeline for wildlife detection results
 * 
 * Implements non-maximum suppression, confidence filtering, and result
 * refinement for ESP32 wildlife classification and object detection.
 */

#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H

#include <stdint.h>
#include <stddef.h>

// Maximum number of detections to process
#define MAX_DETECTIONS 20
#define MAX_CLASSES 50

/**
 * Bounding box structure (normalized coordinates)
 */
typedef struct {
    float x;        // Center x coordinate (0-1)
    float y;        // Center y coordinate (0-1)  
    float width;    // Width (0-1)
    float height;   // Height (0-1)
} BoundingBox;

/**
 * Detection result before post-processing
 */
typedef struct {
    uint8_t classId;        // Detected class ID
    float confidence;       // Detection confidence (0-1)
    BoundingBox bbox;       // Bounding box
    uint32_t timestamp;     // Detection timestamp
    bool valid;             // Whether detection is valid
} RawDetection;

/**
 * Processed detection result
 */
typedef struct {
    uint8_t classId;        // Detected class ID
    float confidence;       // Detection confidence (0-1)
    BoundingBox bbox;       // Refined bounding box
    uint32_t timestamp;     // Detection timestamp
    float stability;        // Temporal stability score
    uint8_t trackId;        // Track ID for multi-frame tracking
    bool isNew;             // Whether this is a new detection
} ProcessedDetection;

/**
 * Post-processing configuration
 */
typedef struct {
    // Confidence filtering
    float confidenceThreshold;      // Minimum confidence threshold
    float classSpecificThresholds[MAX_CLASSES]; // Per-class thresholds
    
    // Non-maximum suppression
    float nmsThreshold;            // IoU threshold for NMS
    bool enableNMS;                // Whether to apply NMS
    
    // Temporal filtering
    bool enableTemporalFilter;     // Enable temporal consistency
    uint8_t temporalWindowSize;    // Number of frames to consider
    float temporalThreshold;       // Temporal consistency threshold
    
    // Tracking
    bool enableTracking;           // Enable object tracking
    float trackingThreshold;       // Distance threshold for tracking
    uint8_t maxTracks;             // Maximum number of simultaneous tracks
    
    // Size filtering
    float minBboxArea;             // Minimum bounding box area
    float maxBboxArea;             // Maximum bounding box area
    
    // Edge constraints
    float edgeMargin;              // Margin from image edges
    bool filterEdgeDetections;     // Filter detections near edges
} PostProcessingConfig;

/**
 * Detection history for temporal filtering
 */
typedef struct {
    RawDetection detections[MAX_DETECTIONS];
    uint8_t count;
    uint32_t timestamp;
} DetectionFrame;

/**
 * Post-processing pipeline class
 * 
 * Handles complete post-processing of raw detection results including
 * confidence filtering, NMS, temporal consistency, and object tracking.
 */
class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    /**
     * Initialize post-processor
     * @param config Post-processing configuration
     * @return true if initialization successful
     */
    bool initialize(const PostProcessingConfig& config);

    /**
     * Process raw detections
     * @param rawDetections Array of raw detections
     * @param numRawDetections Number of raw detections
     * @param processedDetections Output array for processed detections  
     * @param maxProcessedDetections Maximum number of processed detections
     * @return Number of processed detections
     */
    uint8_t processDetections(const RawDetection* rawDetections,
                             uint8_t numRawDetections,
                             ProcessedDetection* processedDetections,
                             uint8_t maxProcessedDetections);

    /**
     * Update configuration
     * @param config New configuration
     */
    void updateConfig(const PostProcessingConfig& config);

    /**
     * Get current configuration
     * @return Current configuration
     */
    PostProcessingConfig getConfig() const { return config_; }

    /**
     * Reset temporal history and tracking state
     */
    void reset();

    /**
     * Get processing statistics
     * @param totalProcessed Total detections processed
     * @param totalFiltered Total detections filtered out
     * @param avgProcessingTime Average processing time per frame
     */
    void getStatistics(uint32_t* totalProcessed, uint32_t* totalFiltered,
                      uint32_t* avgProcessingTime) const;

private:
    PostProcessingConfig config_;           // Configuration
    bool initialized_;                      // Initialization status
    
    // Temporal filtering
    DetectionFrame history_[10];            // Detection history buffer
    uint8_t historyIndex_;                  // Current history index
    uint8_t historyCount_;                  // Number of frames in history
    
    // Object tracking
    ProcessedDetection tracks_[MAX_DETECTIONS]; // Active tracks
    uint8_t trackCount_;                    // Number of active tracks
    uint8_t nextTrackId_;                   // Next track ID to assign
    
    // Statistics
    uint32_t totalProcessed_;               // Total detections processed
    uint32_t totalFiltered_;                // Total detections filtered
    uint32_t totalProcessingTime_;          // Total processing time
    uint32_t frameCount_;                   // Number of frames processed

    /**
     * Apply confidence filtering
     * @param detections Array of detections to filter
     * @param count Number of detections
     * @return Number of detections after filtering
     */
    uint8_t applyConfidenceFiltering(RawDetection* detections, uint8_t count);

    /**
     * Apply non-maximum suppression
     * @param detections Array of detections
     * @param count Number of detections
     * @return Number of detections after NMS
     */
    uint8_t applyNonMaximumSuppression(RawDetection* detections, uint8_t count);

    /**
     * Apply temporal filtering
     * @param currentDetections Current frame detections
     * @param count Number of current detections
     * @return Number of detections after temporal filtering
     */
    uint8_t applyTemporalFiltering(RawDetection* currentDetections, uint8_t count);

    /**
     * Apply size and edge filtering
     * @param detections Array of detections
     * @param count Number of detections
     * @return Number of detections after filtering
     */
    uint8_t applySizeAndEdgeFiltering(RawDetection* detections, uint8_t count);

    /**
     * Update object tracking
     * @param detections Current detections
     * @param count Number of detections
     * @param processedDetections Output processed detections
     * @return Number of tracked detections
     */
    uint8_t updateObjectTracking(const RawDetection* detections, uint8_t count,
                                ProcessedDetection* processedDetections);

    /**
     * Calculate Intersection over Union (IoU) between two bounding boxes
     * @param box1 First bounding box
     * @param box2 Second bounding box
     * @return IoU value (0-1)
     */
    float calculateIoU(const BoundingBox& box1, const BoundingBox& box2);

    /**
     * Calculate distance between two bounding boxes
     * @param box1 First bounding box
     * @param box2 Second bounding box
     * @return Distance value
     */
    float calculateDistance(const BoundingBox& box1, const BoundingBox& box2);

    /**
     * Check temporal consistency of detection
     * @param detection Detection to check
     * @return Temporal stability score (0-1)
     */
    float calculateTemporalStability(const RawDetection& detection);

    /**
     * Add detection to history
     * @param detections Array of detections
     * @param count Number of detections
     * @param timestamp Frame timestamp
     */
    void addToHistory(const RawDetection* detections, uint8_t count, uint32_t timestamp);

    /**
     * Find matching track for detection
     * @param detection Detection to match
     * @return Track index or -1 if no match
     */
    int8_t findMatchingTrack(const RawDetection& detection);

    /**
     * Create new track from detection
     * @param detection Detection to create track from
     * @param processed Output processed detection
     */
    void createNewTrack(const RawDetection& detection, ProcessedDetection* processed);

    /**
     * Update existing track
     * @param trackIndex Index of track to update
     * @param detection New detection data
     * @param processed Output processed detection
     */
    void updateExistingTrack(uint8_t trackIndex, const RawDetection& detection,
                           ProcessedDetection* processed);

    /**
     * Clean up old tracks
     * @param currentTimestamp Current timestamp
     */
    void cleanupOldTracks(uint32_t currentTimestamp);

    /**
     * Update processing statistics
     * @param processingTime Time taken for processing
     * @param inputCount Number of input detections
     * @param outputCount Number of output detections
     */
    void updateStatistics(uint32_t processingTime, uint8_t inputCount, uint8_t outputCount);
};

// Default post-processing configurations
extern const PostProcessingConfig SPECIES_DETECTION_CONFIG;
extern const PostProcessingConfig MOTION_DETECTION_CONFIG;
extern const PostProcessingConfig HIGH_PRECISION_CONFIG;
extern const PostProcessingConfig REAL_TIME_CONFIG;

#endif // POSTPROCESSING_H