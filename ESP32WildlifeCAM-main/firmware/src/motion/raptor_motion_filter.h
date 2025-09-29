/**
 * @file raptor_motion_filter.h
 * @brief Specialized Motion Detection for Raptor Flight Patterns
 * 
 * Implements advanced motion detection algorithms optimized for bird of prey
 * flight characteristics including soaring, gliding, diving, and perching behaviors.
 */

#ifndef RAPTOR_MOTION_FILTER_H
#define RAPTOR_MOTION_FILTER_H

#include "../motion_filter.h"
#include "../scenarios/raptor_configs.h"
#include <vector>
#include <deque>
#include <memory>

/**
 * Motion Detection Modes for Raptors
 */
enum class RaptorMotionMode {
    GENERAL_DETECTION,          // Standard motion detection
    SOARING_OPTIMIZED,          // Optimized for soaring/circling
    HUNTING_OPTIMIZED,          // High-speed diving/hunting detection  
    PERCH_MONITORING,           // Perched bird micro-movements
    FLIGHT_TRACKING,            // Active flight pattern tracking
    NESTING_SENSITIVE,          // Ultra-sensitive for nesting sites
    MIGRATION_WIDE_ANGLE        // Wide-angle migration detection
};

/**
 * Flight Pattern Characteristics
 */
enum class FlightPattern {
    SOARING,                    // Circular soaring motion
    GLIDING,                    // Straight-line gliding
    FLAPPING,                   // Active flapping flight
    DIVING,                     // High-speed diving/stooping
    HOVERING,                   // Stationary hovering (kestrels)
    THERMAL_RIDING,             // Spiral climbing in thermals
    RIDGE_SOARING,              // Following ridges/updrafts
    PERCHED,                    // Stationary on perch
    TAKEOFF,                    // Launch from perch
    LANDING,                    // Approach to perch
    UNKNOWN                     // Unclassified movement
};

/**
 * Motion Vector for Tracking
 */
struct MotionVector {
    float x, y;                         // Position coordinates (normalized 0-1)
    float dx, dy;                       // Velocity components (pixels/frame)
    float magnitude;                    // Speed magnitude
    float direction_degrees;            // Movement direction
    float acceleration;                 // Acceleration magnitude
    unsigned long timestamp;
    bool valid;
    
    MotionVector() : x(0), y(0), dx(0), dy(0), magnitude(0), 
                    direction_degrees(0), acceleration(0), 
                    timestamp(0), valid(false) {}
};

/**
 * Raptor Motion Detection Result
 */
struct RaptorMotionResult {
    bool motionDetected;
    FlightPattern flightPattern;
    float confidence;
    
    // Motion characteristics
    struct {
        MotionVector currentVector;
        std::vector<MotionVector> trajectory;   // Recent motion history
        float averageSpeed_pxps;                // Pixels per second
        float peakSpeed_pxps;
        float averageDirection_degrees;
        float directionVariance;
        bool directionalMovement;
    } motion;
    
    // Flight-specific metrics
    struct {
        bool isCircular;                        // Circular motion (soaring)
        float circularRadius_px;                // Radius of circular motion
        float wingbeatFrequency_Hz;             // Detected wingbeat rate
        bool thermalDetected;                   // Using thermal currents
        float climbRate_pxps;                   // Vertical climb rate
        float glideRatio;                       // Glide efficiency
    } flight;
    
    // Size and shape analysis
    struct {
        uint16_t boundingBox[4];                // [x1, y1, x2, y2]
        float aspectRatio;                      // Width/height ratio
        float areaChange_percent;               // Size variation
        bool wingspanDetected;                  // Wingspan visible
        float estimatedWingspan_px;
        float bodyToWingRatio;
    } morphology;
    
    // Behavioral indicators
    struct {
        bool huntingBehavior;                   // Hunting patterns detected
        bool territorialDisplay;                // Territorial flight patterns
        bool courtshipFlight;                   // Mating display patterns
        bool predatorResponse;                  // Evasive maneuvers
        bool nestingApproach;                   // Approach to nesting area
    } behavior;
    
    // Quality metrics
    float trackingQuality;
    bool occluded;
    float lightingQuality;
    uint32_t framesContinuous;
    
    RaptorMotionResult() : motionDetected(false), flightPattern(FlightPattern::UNKNOWN),
                          confidence(0.0f), trackingQuality(0.0f), occluded(false),
                          lightingQuality(0.0f), framesContinuous(0) {}
};

/**
 * Motion Prediction for Pre-trigger Capture
 */
struct MotionPrediction {
    bool predictionAvailable;
    MotionVector predictedVector;       // Predicted next position/velocity
    float predictionConfidence;
    uint32_t predictionHorizon_ms;      // How far ahead predicted
    bool shouldTriggerCapture;
    String reasoning;                   // Why capture should trigger
    
    MotionPrediction() : predictionAvailable(false), predictionConfidence(0.0f),
                        predictionHorizon_ms(0), shouldTriggerCapture(false) {}
};

/**
 * Multi-zone Motion Detection
 */
struct MotionZone {
    uint8_t zoneId;
    uint16_t bounds[4];                 // [x1, y1, x2, y2] in pixels
    float sensitivity;                  // Zone-specific sensitivity
    bool enabled;
    String name;                        // Zone description
    RaptorMotionMode mode;              // Detection mode for this zone
    
    // Zone statistics
    struct {
        uint32_t detectionCount;
        float averageConfidence;
        unsigned long lastDetection;
        bool currentlyActive;
    } stats;
    
    MotionZone() : zoneId(0), sensitivity(0.5f), enabled(true),
                  mode(RaptorMotionMode::GENERAL_DETECTION) {
        bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
    }
};

/**
 * Advanced Raptor Motion Filter
 */
class RaptorMotionFilter : public MotionFilter {
public:
    /**
     * Constructor
     */
    RaptorMotionFilter();
    
    /**
     * Destructor
     */
    ~RaptorMotionFilter();
    
    /**
     * Initialize raptor motion filter
     * @param scenario Target monitoring scenario
     * @return true if initialization successful
     */
    bool initRaptorFilter(RaptorScenario scenario);
    
    /**
     * Analyze motion in current frame
     * @param currentFrame Current camera frame
     * @param previousFrame Previous camera frame
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     * @return Raptor motion analysis result
     */
    RaptorMotionResult analyzeRaptorMotion(const uint8_t* currentFrame, 
                                          const uint8_t* previousFrame,
                                          uint32_t width, uint32_t height);
    
    /**
     * Track raptor across multiple frames
     * @param frame Current frame data
     * @param width Frame width
     * @param height Frame height
     * @return Updated tracking result
     */
    RaptorMotionResult trackRaptor(const uint8_t* frame, uint32_t width, uint32_t height);
    
    /**
     * Predict next motion for pre-trigger capture
     * @param lookAhead_ms Prediction time horizon
     * @return Motion prediction result
     */
    MotionPrediction predictMotion(uint32_t lookAhead_ms = 500);
    
    /**
     * Configure motion zones for multi-area detection
     * @param zones Vector of motion zone configurations
     * @return true if zones configured successfully
     */
    bool configureMotionZones(const std::vector<MotionZone>& zones);
    
    /**
     * Set motion detection mode
     * @param mode Detection mode to use
     */
    void setMotionMode(RaptorMotionMode mode);
    
    /**
     * Enable/disable flight pattern analysis
     * @param enable true to enable pattern analysis
     */
    void enableFlightPatternAnalysis(bool enable);
    
    /**
     * Set size-based filtering parameters
     * @param minSize_px Minimum object size in pixels
     * @param maxSize_px Maximum object size in pixels
     */
    void setSizeFiltering(uint16_t minSize_px, uint16_t maxSize_px);
    
    /**
     * Enable thermal/updraft detection
     * @param enable true to detect thermal utilization
     */
    void enableThermalDetection(bool enable);
    
    /**
     * Get current tracking statistics
     */
    struct TrackingStats {
        uint32_t totalDetections;
        uint32_t validTracks;
        float averageTrackDuration_s;
        float averageConfidence;
        uint32_t lostTracks;
        uint32_t falsePositives;
        float trackingAccuracy_percent;
    } getTrackingStats() const;
    
    /**
     * Get motion zones status
     */
    std::vector<MotionZone> getMotionZones() const { return motionZones; }
    
    /**
     * Export motion tracking data
     * @param format Export format ("csv", "json", "xml")
     * @param maxRecords Maximum number of records to export
     * @return Formatted motion data
     */
    String exportMotionData(const String& format = "csv", uint32_t maxRecords = 1000) const;
    
    /**
     * Calibrate motion detection for current environment
     * @param calibrationFrames Number of frames for background learning
     * @return true if calibration successful
     */
    bool calibrateBackgroundModel(uint32_t calibrationFrames = 50);
    
    /**
     * Check if motion filter is ready for raptor detection
     */
    bool isRaptorFilterReady() const { return raptorFilterInitialized && backgroundModelReady; }

private:
    // Raptor-specific state
    bool raptorFilterInitialized;
    bool backgroundModelReady;
    RaptorScenario currentScenario;
    RaptorMotionMode currentMode;
    bool flightPatternAnalysisEnabled;
    bool thermalDetectionEnabled;
    
    // Motion tracking
    std::deque<RaptorMotionResult> motionHistory;
    std::deque<MotionVector> trajectoryBuffer;
    static const size_t MAX_MOTION_HISTORY = 100;
    static const size_t MAX_TRAJECTORY_POINTS = 50;
    
    // Background model
    std::unique_ptr<uint8_t[]> backgroundModel;
    std::unique_ptr<float[]> backgroundVariance;
    uint32_t backgroundFrameCount;
    bool adaptiveBackgroundEnabled;
    
    // Motion zones
    std::vector<MotionZone> motionZones;
    bool multiZoneEnabled;
    
    // Size filtering
    uint16_t minObjectSize_px;
    uint16_t maxObjectSize_px;
    bool sizeFilteringEnabled;
    
    // Performance tracking
    struct {
        uint32_t totalFramesProcessed;
        float totalProcessingTime_ms;
        uint32_t successfulTracks;
        uint32_t lostTracks;
        uint32_t falseDetections;
        unsigned long lastStatsUpdate;
    } performanceStats;
    
    // Motion analysis methods
    std::vector<MotionVector> detectMotionVectors(const uint8_t* currentFrame,
                                                  const uint8_t* previousFrame,
                                                  uint32_t width, uint32_t height);
    FlightPattern classifyFlightPattern(const std::vector<MotionVector>& trajectory);
    bool detectCircularMotion(const std::vector<MotionVector>& trajectory, float& radius);
    bool detectDivingBehavior(const std::vector<MotionVector>& trajectory);
    bool detectHoveringBehavior(const std::vector<MotionVector>& trajectory);
    bool detectThermalUtilization(const std::vector<MotionVector>& trajectory);
    
    // Tracking algorithms
    bool initializeTrack(const MotionVector& initialVector);
    bool updateTrack(const MotionVector& newVector);
    bool validateTrack(const std::vector<MotionVector>& trajectory);
    void predictNextPosition(const std::vector<MotionVector>& trajectory, MotionVector& predicted);
    
    // Background subtraction
    void updateBackgroundModel(const uint8_t* frame, uint32_t width, uint32_t height);
    void performBackgroundSubtraction(const uint8_t* frame, uint8_t* foreground,
                                     uint32_t width, uint32_t height);
    bool isBackgroundPixel(uint8_t pixel, uint8_t background, float variance);
    
    // Blob analysis
    struct Blob {
        uint16_t x, y, width, height;
        uint32_t area;
        float aspectRatio;
        MotionVector centerOfMass;
    };
    std::vector<Blob> extractBlobs(const uint8_t* binaryImage, uint32_t width, uint32_t height);
    bool isBlobRaptorCandidate(const Blob& blob);
    MotionVector calculateBlobMotion(const Blob& currentBlob, const Blob& previousBlob);
    
    // Feature extraction
    std::vector<float> extractMotionFeatures(const std::vector<MotionVector>& trajectory);
    std::vector<float> extractShapeFeatures(const Blob& blob);
    std::vector<float> extractTemporalFeatures(const std::deque<RaptorMotionResult>& history);
    
    // Pattern recognition
    bool matchFlightPattern(const std::vector<float>& features, FlightPattern pattern);
    float calculatePatternConfidence(const std::vector<float>& features, FlightPattern pattern);
    bool detectWingbeatFrequency(const std::vector<MotionVector>& trajectory, float& frequency);
    
    // Multi-zone processing
    void processMotionZones(const uint8_t* frame, uint32_t width, uint32_t height);
    bool isMotionInZone(const MotionVector& motion, const MotionZone& zone);
    void updateZoneStatistics(MotionZone& zone, bool motionDetected, float confidence);
    
    // Filtering and validation
    bool passesRaptorSizeFilter(const Blob& blob);
    bool passesRaptorShapeFilter(const Blob& blob);
    bool passesRaptorMotionFilter(const MotionVector& motion);
    bool passesTrajectoryFilter(const std::vector<MotionVector>& trajectory);
    
    // Prediction algorithms
    MotionVector extrapolateMotion(const std::vector<MotionVector>& trajectory, uint32_t timeHorizon_ms);
    float calculatePredictionConfidence(const std::vector<MotionVector>& trajectory);
    bool shouldTriggerPreCapture(const MotionPrediction& prediction);
    
    // Scenario-specific optimization
    void configureForNestingMonitoring();
    void configureForMigrationTracking();
    void configureForHuntingAnalysis();
    void configureForTerritorialMonitoring();
    
    // Utility methods
    void updatePerformanceStats(float processingTime, bool successfulDetection);
    void cleanupOldTrackingData();
    float calculateMotionSimilarity(const MotionVector& a, const MotionVector& b);
    String formatMotionResult(const RaptorMotionResult& result, const String& format) const;
    void logMotionEvent(const RaptorMotionResult& result);
    
    // Image processing utilities
    void applyGaussianBlur(uint8_t* image, uint32_t width, uint32_t height, float sigma);
    void computeOpticalFlow(const uint8_t* frame1, const uint8_t* frame2,
                           float* flowX, float* flowY, uint32_t width, uint32_t height);
    void morphologicalOperations(uint8_t* binaryImage, uint32_t width, uint32_t height);
    void connectedComponentLabeling(const uint8_t* binaryImage, uint16_t* labelImage,
                                   uint32_t width, uint32_t height);
};

/**
 * Raptor Motion Utilities
 */
namespace RaptorMotionUtils {
    /**
     * Convert flight pattern to string
     */
    const char* flightPatternToString(FlightPattern pattern);
    
    /**
     * Convert motion mode to string
     */
    const char* motionModeToString(RaptorMotionMode mode);
    
    /**
     * Calculate flight energy efficiency
     */
    float calculateFlightEfficiency(const std::vector<MotionVector>& trajectory);
    
    /**
     * Estimate flight altitude from motion patterns
     */
    float estimateFlightAltitude(const RaptorMotionResult& result, float focalLength_mm);
    
    /**
     * Generate motion analysis summary
     */
    String generateMotionSummary(const std::vector<RaptorMotionResult>& results);
    
    /**
     * Calculate territory boundaries from motion data
     */
    std::vector<std::pair<float, float>> calculateTerritoryFromMotion(
        const std::vector<RaptorMotionResult>& results);
    
    /**
     * Detect hunting success from motion patterns
     */
    bool detectHuntingSuccess(const std::vector<RaptorMotionResult>& sequence);
}

#endif // RAPTOR_MOTION_FILTER_H