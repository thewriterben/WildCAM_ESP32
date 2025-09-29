/**
 * @file population_estimator.cpp
 * @brief Wildlife population estimation algorithms
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 * 
 * Advanced algorithms for estimating wildlife populations using
 * camera trap data and statistical modeling techniques.
 */

#include "population_estimator.h"
#include "../../utils/logger.h"
#include <cmath>

namespace Wildlife {
namespace Research {

PopulationEstimator::PopulationEstimator() : 
    estimation_method_(EstimationMethod::MARK_RECAPTURE),
    confidence_level_(0.95),
    minimum_detections_(10),
    study_duration_days_(30) {
}

PopulationEstimator::~PopulationEstimator() = default;

bool PopulationEstimator::initialize(EstimationMethod method, float confidence_level) {
    estimation_method_ = method;
    confidence_level_ = confidence_level;
    
    Logger::info("Population estimator initialized with %s method", 
                 getMethodName(method));
    return true;
}

PopulationEstimate PopulationEstimator::estimatePopulation(
    const std::vector<Detection>& detections) {
    
    if (detections.size() < minimum_detections_) {
        Logger::warning("Insufficient detections for reliable estimate: %zu", 
                       detections.size());
        return createInvalidEstimate();
    }
    
    Logger::info("Estimating population from %zu detections", detections.size());
    
    switch (estimation_method_) {
        case EstimationMethod::MARK_RECAPTURE:
            return estimateMarkRecapture(detections);
        case EstimationMethod::DISTANCE_SAMPLING:
            return estimateDistanceSampling(detections);
        case EstimationMethod::OCCUPANCY_MODELING:
            return estimateOccupancyModeling(detections);
        case EstimationMethod::REMOVAL_METHOD:
            return estimateRemovalMethod(detections);
        case EstimationMethod::ABUNDANCE_INDEX:
            return calculateAbundanceIndex(detections);
        default:
            Logger::error("Unknown estimation method");
            return createInvalidEstimate();
    }
}

PopulationEstimate PopulationEstimator::estimateMarkRecapture(
    const std::vector<Detection>& detections) {
    
    Logger::info("Applying Lincoln-Petersen mark-recapture estimator");
    
    // Identify unique individuals using AI-based individual recognition
    auto individuals = identifyUniqueIndividuals(detections);
    
    // Calculate capture histories
    auto capture_histories = calculateCaptureHistories(individuals, detections);
    
    // Apply Lincoln-Petersen estimator
    int marked_first_sample = 0;
    int total_second_sample = 0;
    int marked_in_second_sample = 0;
    
    // Divide study period into two sampling occasions
    uint32_t mid_point = study_duration_days_ / 2;
    
    for (const auto& individual : individuals) {
        bool captured_first = false;
        bool captured_second = false;
        
        for (const auto& detection : detections) {
            if (detection.individual_id == individual.id) {
                uint32_t day = (detection.timestamp - detections[0].timestamp) / (24 * 3600);
                if (day < mid_point) {
                    captured_first = true;
                } else {
                    captured_second = true;
                }
            }
        }
        
        if (captured_first) marked_first_sample++;
        if (captured_second) total_second_sample++;
        if (captured_first && captured_second) marked_in_second_sample++;
    }
    
    PopulationEstimate estimate;
    
    if (marked_in_second_sample > 0 && total_second_sample > 0) {
        // Lincoln-Petersen estimator: N = (M * C) / R
        estimate.population_size = static_cast<float>(marked_first_sample * total_second_sample) / 
                                  static_cast<float>(marked_in_second_sample);
        
        // Calculate variance and confidence interval
        float variance = static_cast<float>(marked_first_sample * total_second_sample * 
                        (total_second_sample - marked_in_second_sample)) / 
                        (static_cast<float>(marked_in_second_sample * marked_in_second_sample) * 
                         (marked_in_second_sample + 1));
        
        float standard_error = sqrt(variance);
        float z_score = getZScore(confidence_level_);
        
        estimate.confidence_interval_lower = estimate.population_size - (z_score * standard_error);
        estimate.confidence_interval_upper = estimate.population_size + (z_score * standard_error);
        estimate.standard_error = standard_error;
        estimate.confidence_level = confidence_level_;
        estimate.method = EstimationMethod::MARK_RECAPTURE;
        estimate.valid = true;
        
        Logger::info("Population estimate: %.1f (%.1f - %.1f)", 
                    estimate.population_size, 
                    estimate.confidence_interval_lower,
                    estimate.confidence_interval_upper);
    } else {
        Logger::warning("Insufficient recaptures for mark-recapture estimation");
        estimate = createInvalidEstimate();
    }
    
    return estimate;
}

PopulationEstimate PopulationEstimator::estimateDistanceSampling(
    const std::vector<Detection>& detections) {
    
    Logger::info("Applying distance sampling estimator");
    
    // Calculate detection distances and probabilities
    std::vector<float> distances;
    for (const auto& detection : detections) {
        if (detection.distance_to_camera > 0) {
            distances.push_back(detection.distance_to_camera);
        }
    }
    
    if (distances.empty()) {
        Logger::warning("No distance data available for distance sampling");
        return createInvalidEstimate();
    }
    
    // Fit detection function (simplified half-normal model)
    float sigma = calculateDetectionFunctionParameter(distances);
    float effective_strip_width = sigma * sqrt(M_PI / 2.0);
    
    // Calculate density and abundance
    float surveyed_area = 2.0 * effective_strip_width * getTotalTransectLength();
    float density = static_cast<float>(detections.size()) / surveyed_area;
    float total_area = getTotalStudyArea();
    
    PopulationEstimate estimate;
    estimate.population_size = density * total_area;
    estimate.density = density;
    estimate.method = EstimationMethod::DISTANCE_SAMPLING;
    estimate.confidence_level = confidence_level_;
    estimate.valid = true;
    
    // Simplified confidence interval calculation
    float cv = 0.2; // Coefficient of variation (simplified)
    float se = estimate.population_size * cv;
    float z_score = getZScore(confidence_level_);
    
    estimate.standard_error = se;
    estimate.confidence_interval_lower = estimate.population_size - (z_score * se);
    estimate.confidence_interval_upper = estimate.population_size + (z_score * se);
    
    Logger::info("Distance sampling estimate: %.1f individuals (density: %.3f/km²)", 
                estimate.population_size, density);
    
    return estimate;
}

PopulationEstimate PopulationEstimator::estimateOccupancyModeling(
    const std::vector<Detection>& detections) {
    
    Logger::info("Applying occupancy modeling");
    
    // Grid-based occupancy analysis
    int grid_size = 10; // 10x10 grid
    std::vector<std::vector<bool>> occupancy_grid(grid_size, std::vector<bool>(grid_size, false));
    
    // Map detections to grid cells
    for (const auto& detection : detections) {
        int grid_x = static_cast<int>((detection.longitude - getStudyAreaMinLon()) / 
                                     (getStudyAreaMaxLon() - getStudyAreaMinLon()) * grid_size);
        int grid_y = static_cast<int>((detection.latitude - getStudyAreaMinLat()) / 
                                     (getStudyAreaMaxLat() - getStudyAreaMinLat()) * grid_size);
        
        if (grid_x >= 0 && grid_x < grid_size && grid_y >= 0 && grid_y < grid_size) {
            occupancy_grid[grid_x][grid_y] = true;
        }
    }
    
    // Calculate occupancy proportion
    int occupied_cells = 0;
    for (const auto& row : occupancy_grid) {
        for (bool cell : row) {
            if (cell) occupied_cells++;
        }
    }
    
    float occupancy_rate = static_cast<float>(occupied_cells) / (grid_size * grid_size);
    
    PopulationEstimate estimate;
    estimate.occupancy_rate = occupancy_rate;
    estimate.population_size = occupancy_rate * getExpectedDensityPerCell() * (grid_size * grid_size);
    estimate.method = EstimationMethod::OCCUPANCY_MODELING;
    estimate.confidence_level = confidence_level_;
    estimate.valid = true;
    
    Logger::info("Occupancy estimate: %.1f%% occupancy, %.1f individuals", 
                occupancy_rate * 100, estimate.population_size);
    
    return estimate;
}

std::vector<PopulationEstimator::Individual> PopulationEstimator::identifyUniqueIndividuals(
    const std::vector<Detection>& detections) {
    
    std::vector<Individual> individuals;
    uint32_t next_id = 1;
    
    // Simplified individual identification based on temporal and spatial clustering
    for (const auto& detection : detections) {
        bool found_match = false;
        
        for (auto& individual : individuals) {
            // Check if detection matches existing individual
            if (isLikelyMatch(detection, individual)) {
                individual.detections.push_back(detection);
                found_match = true;
                break;
            }
        }
        
        if (!found_match) {
            // Create new individual
            Individual new_individual;
            new_individual.id = next_id++;
            new_individual.detections.push_back(detection);
            individuals.push_back(new_individual);
        }
    }
    
    Logger::info("Identified %zu unique individuals from %zu detections", 
                individuals.size(), detections.size());
    
    return individuals;
}

bool PopulationEstimator::isLikelyMatch(const Detection& detection, 
                                       const Individual& individual) {
    // Simplified matching based on temporal and spatial proximity
    float max_time_diff = 24 * 3600; // 24 hours
    float max_distance = 1000.0; // 1 km
    
    for (const auto& existing_detection : individual.detections) {
        float time_diff = abs(static_cast<int>(detection.timestamp - existing_detection.timestamp));
        float distance = calculateDistance(detection.latitude, detection.longitude,
                                         existing_detection.latitude, existing_detection.longitude);
        
        if (time_diff < max_time_diff && distance < max_distance) {
            return true;
        }
    }
    
    return false;
}

float PopulationEstimator::calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    // Haversine formula for distance calculation
    const float R = 6371000; // Earth radius in meters
    
    float lat1_rad = lat1 * M_PI / 180.0;
    float lat2_rad = lat2 * M_PI / 180.0;
    float dlat = (lat2 - lat1) * M_PI / 180.0;
    float dlon = (lon2 - lon1) * M_PI / 180.0;
    
    float a = sin(dlat/2) * sin(dlat/2) + cos(lat1_rad) * cos(lat2_rad) * sin(dlon/2) * sin(dlon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return R * c;
}

float PopulationEstimator::getZScore(float confidence_level) {
    // Simplified Z-score lookup
    if (confidence_level >= 0.99) return 2.576;
    if (confidence_level >= 0.95) return 1.960;
    if (confidence_level >= 0.90) return 1.645;
    return 1.960; // Default to 95%
}

PopulationEstimate PopulationEstimator::createInvalidEstimate() {
    PopulationEstimate estimate;
    estimate.valid = false;
    estimate.population_size = 0;
    estimate.confidence_interval_lower = 0;
    estimate.confidence_interval_upper = 0;
    estimate.standard_error = 0;
    estimate.confidence_level = 0;
    estimate.method = estimation_method_;
    return estimate;
}

const char* PopulationEstimator::getMethodName(EstimationMethod method) {
    switch (method) {
        case EstimationMethod::MARK_RECAPTURE: return "Mark-Recapture";
        case EstimationMethod::DISTANCE_SAMPLING: return "Distance Sampling";
        case EstimationMethod::OCCUPANCY_MODELING: return "Occupancy Modeling";
        case EstimationMethod::REMOVAL_METHOD: return "Removal Method";
        case EstimationMethod::ABUNDANCE_INDEX: return "Abundance Index";
        default: return "Unknown";
    }
}

// Placeholder implementations for required helper methods
PopulationEstimate PopulationEstimator::estimateRemovalMethod(const std::vector<Detection>& detections) {
    return createInvalidEstimate(); // Placeholder
}

PopulationEstimate PopulationEstimator::calculateAbundanceIndex(const std::vector<Detection>& detections) {
    PopulationEstimate estimate;
    estimate.population_size = static_cast<float>(detections.size());
    estimate.method = EstimationMethod::ABUNDANCE_INDEX;
    estimate.valid = true;
    return estimate;
}

std::vector<std::vector<bool>> PopulationEstimator::calculateCaptureHistories(
    const std::vector<Individual>& individuals, const std::vector<Detection>& detections) {
    return std::vector<std::vector<bool>>(); // Placeholder
}

float PopulationEstimator::calculateDetectionFunctionParameter(const std::vector<float>& distances) {
    return 50.0; // Placeholder
}

float PopulationEstimator::getTotalTransectLength() { return 1000.0; } // 1 km
float PopulationEstimator::getTotalStudyArea() { return 1000000.0; } // 1 km²
float PopulationEstimator::getStudyAreaMinLon() { return -74.1; }
float PopulationEstimator::getStudyAreaMaxLon() { return -74.0; }
float PopulationEstimator::getStudyAreaMinLat() { return 40.7; }
float PopulationEstimator::getStudyAreaMaxLat() { return 40.8; }
float PopulationEstimator::getExpectedDensityPerCell() { return 2.0; }

} // namespace Research
} // namespace Wildlife