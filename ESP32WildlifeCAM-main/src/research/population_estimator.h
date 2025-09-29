/**
 * @file population_estimator.h
 * @brief Wildlife population estimation algorithms
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef POPULATION_ESTIMATOR_H
#define POPULATION_ESTIMATOR_H

#include <Arduino.h>
#include <vector>

namespace Wildlife {
namespace Research {

/**
 * Population estimation methods
 */
enum class EstimationMethod {
    MARK_RECAPTURE,      // Lincoln-Petersen and related methods
    DISTANCE_SAMPLING,   // Line transect distance sampling
    OCCUPANCY_MODELING,  // Site occupancy models
    REMOVAL_METHOD,      // Removal sampling
    ABUNDANCE_INDEX      // Relative abundance indices
};

/**
 * Wildlife detection data structure
 */
struct Detection {
    uint32_t timestamp = 0;
    float latitude = 0.0;
    float longitude = 0.0;
    String species = "";
    uint32_t individual_id = 0;
    float confidence = 0.0;
    float distance_to_camera = 0.0;
    String camera_id = "";
    String image_path = "";
};

/**
 * Individual animal tracking
 */
struct Individual {
    uint32_t id = 0;
    String species = "";
    std::vector<Detection> detections;
    float estimated_body_size = 0.0;
    String distinctive_features = "";
};

/**
 * Population estimate results
 */
struct PopulationEstimate {
    float population_size = 0.0;
    float confidence_interval_lower = 0.0;
    float confidence_interval_upper = 0.0;
    float standard_error = 0.0;
    float confidence_level = 0.95;
    float density = 0.0;           // individuals per unit area
    float occupancy_rate = 0.0;    // proportion of sites occupied
    EstimationMethod method = EstimationMethod::MARK_RECAPTURE;
    bool valid = false;
    String notes = "";
};

/**
 * Wildlife Population Estimator
 * 
 * Advanced statistical methods for estimating wildlife populations
 * from camera trap data using various ecological models.
 */
class PopulationEstimator {
public:
    PopulationEstimator();
    ~PopulationEstimator();
    
    // Configuration
    bool initialize(EstimationMethod method, float confidence_level = 0.95);
    void setMinimumDetections(uint32_t min_detections) { minimum_detections_ = min_detections; }
    void setStudyDuration(uint32_t days) { study_duration_days_ = days; }
    
    // Population estimation
    PopulationEstimate estimatePopulation(const std::vector<Detection>& detections);
    
    // Method-specific estimators
    PopulationEstimate estimateMarkRecapture(const std::vector<Detection>& detections);
    PopulationEstimate estimateDistanceSampling(const std::vector<Detection>& detections);
    PopulationEstimate estimateOccupancyModeling(const std::vector<Detection>& detections);
    PopulationEstimate estimateRemovalMethod(const std::vector<Detection>& detections);
    PopulationEstimate calculateAbundanceIndex(const std::vector<Detection>& detections);
    
    // Individual identification
    std::vector<Individual> identifyUniqueIndividuals(const std::vector<Detection>& detections);
    
    // Utility functions
    static const char* getMethodName(EstimationMethod method);
    static float calculateDistance(float lat1, float lon1, float lat2, float lon2);
    
private:
    EstimationMethod estimation_method_;
    float confidence_level_;
    uint32_t minimum_detections_;
    uint32_t study_duration_days_;
    
    // Helper methods
    bool isLikelyMatch(const Detection& detection, const Individual& individual);
    std::vector<std::vector<bool>> calculateCaptureHistories(
        const std::vector<Individual>& individuals, 
        const std::vector<Detection>& detections);
    float calculateDetectionFunctionParameter(const std::vector<float>& distances);
    float getZScore(float confidence_level);
    PopulationEstimate createInvalidEstimate();
    
    // Study area parameters (would be configurable)
    float getTotalTransectLength();
    float getTotalStudyArea();
    float getStudyAreaMinLon();
    float getStudyAreaMaxLon();
    float getStudyAreaMinLat();
    float getStudyAreaMaxLat();
    float getExpectedDensityPerCell();
};

} // namespace Research
} // namespace Wildlife

#endif // POPULATION_ESTIMATOR_H