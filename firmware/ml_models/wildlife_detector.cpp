/**
 * @file wildlife_detector.cpp
 * @brief Implementation of foundational AI wildlife detection system
 * @version 1.1.0
 * 
 * Enhanced with basic AI features:
 * - On-device motion detection with false positive reduction
 * - Simple animal vs. non-animal classification
 * - Size estimation for detected objects
 */

#include "wildlife_detector.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace WildlifeDetection {

WildlifeDetector::WildlifeDetector() 
    : initialized_(false), frame_width_(0), frame_height_(0),
      last_fp_score_(0.0f), last_was_animal_(false), 
      last_size_category_(BasicAI::SizeCategory::MEDIUM) {
    // Initialize statistics
    memset(&stats_, 0, sizeof(Statistics));
}

WildlifeDetector::~WildlifeDetector() {
    // Cleanup will be handled by smart pointers
}

bool WildlifeDetector::initialize(const DetectorConfig& config) {
    config_ = config;
    
    // Validate configuration
    if (config_.confidence_threshold < 0.0f || config_.confidence_threshold > 1.0f) {
        return false;
    }
    
    if (config_.detection_interval_ms == 0) {
        return false;
    }
    
    // Initialize basic AI processor for enhanced detection
    if (!initializeBasicAI()) {
        // Continue without basic AI if it fails to initialize
        // The detector will still work with basic motion detection
    }
    
    // Reset statistics
    resetStatistics();
    
    initialized_ = true;
    return true;
}

bool WildlifeDetector::initializeBasicAI() {
    ai_processor_ = std::make_unique<BasicAI::BasicAIProcessor>();
    
    BasicAI::BasicAIConfig ai_config = BasicAI::getDefaultConfig();
    ai_config.motion_threshold = 25;
    ai_config.enable_temporal_filtering = config_.enable_false_positive_reduction;
    ai_config.min_motion_frames = config_.motion_history_frames > 0 ? config_.motion_history_frames : 2;
    ai_config.animal_confidence_threshold = config_.confidence_threshold;
    ai_config.enable_size_estimation = config_.enable_size_estimation;
    
    return ai_processor_->initialize(ai_config);
}

std::vector<DetectionResult> WildlifeDetector::detectWildlife(const uint8_t* frame_data, 
                                                            size_t frame_size,
                                                            uint16_t width, 
                                                            uint16_t height) {
    std::vector<DetectionResult> results;
    
    if (!initialized_ || !frame_data || frame_size == 0) {
        return results;
    }
    
    // Update frame dimensions if changed
    if (frame_width_ != width || frame_height_ != height) {
        frame_width_ = width;
        frame_height_ = height;
        previous_frame_.reset(new uint8_t[frame_size]);
    }
    
    uint32_t start_time = millis();
    
    // Use basic AI processor for enhanced detection if available
    if (ai_processor_ && ai_processor_->isInitialized()) {
        BasicAI::AIAnalysisResult ai_result;
        
        if (runBasicAIAnalysis(frame_data, frame_size, width, height, ai_result)) {
            // Store last detection info
            last_fp_score_ = ai_result.motion.false_positive_score;
            last_was_animal_ = (ai_result.classification.classification == BasicAI::ClassificationType::ANIMAL);
            last_size_category_ = ai_result.size.category;
            
            // Check if this is a valid detection (motion detected, not a false positive)
            bool valid_motion = ai_result.motion.motion_detected && 
                              ai_result.motion.confidence >= BasicAI::MotionConfidence::MEDIUM;
            
            // Apply false positive filter
            bool passes_fp_filter = true;
            if (config_.enable_false_positive_reduction) {
                passes_fp_filter = ai_result.motion.false_positive_score < config_.false_positive_threshold;
                if (!passes_fp_filter) {
                    stats_.false_positives_filtered++;
                }
            }
            
            // Apply animal classification filter
            bool passes_animal_filter = true;
            if (config_.enable_animal_classification) {
                passes_animal_filter = ai_result.classification.classification == BasicAI::ClassificationType::ANIMAL ||
                                      ai_result.classification.classification == BasicAI::ClassificationType::UNKNOWN;
                
                if (ai_result.classification.classification == BasicAI::ClassificationType::ANIMAL) {
                    stats_.animal_classifications++;
                } else if (ai_result.classification.classification == BasicAI::ClassificationType::NON_ANIMAL) {
                    stats_.non_animal_classifications++;
                }
            }
            
            // Proceed with detection if motion is valid and passes filters
            if ((valid_motion || !config_.enable_motion_trigger) && passes_fp_filter && passes_animal_filter) {
                // Create detection result from the largest motion region
                if (ai_result.motion.region_count > 0) {
                    // Find largest region
                    uint8_t largest_idx = 0;
                    uint32_t largest_area = 0;
                    for (uint8_t i = 0; i < ai_result.motion.region_count; i++) {
                        uint32_t area = ai_result.motion.regions[i].bbox.width * 
                                       ai_result.motion.regions[i].bbox.height;
                        if (area > largest_area) {
                            largest_area = area;
                            largest_idx = i;
                        }
                    }
                    
                    const BasicAI::MotionRegion& region = ai_result.motion.regions[largest_idx];
                    
                    DetectionResult detection;
                    detection.x = region.bbox.x;
                    detection.y = region.bbox.y;
                    detection.width = region.bbox.width;
                    detection.height = region.bbox.height;
                    detection.motion_detected = true;
                    detection.detection_time = millis();
                    
                    // Set confidence based on AI analysis
                    detection.confidence_score = ai_result.overall_confidence;
                    detection.confidence = calculateConfidence(detection.confidence_score);
                    
                    // Size estimation from basic AI
                    if (config_.enable_size_estimation) {
                        detection.size_estimate = ai_result.size.relative_size;
                    }
                    
                    // Species classification
                    if (config_.enable_species_classification) {
                        detection.species = classifyObject(frame_data, detection.x, detection.y, 
                                                         detection.width, detection.height);
                    } else {
                        detection.species = SpeciesType::UNKNOWN;
                    }
                    
                    // Apply confidence threshold
                    if (detection.confidence_score >= config_.confidence_threshold) {
                        results.push_back(detection);
                        updateStatistics(detection);
                    }
                }
            }
            
            // Update processing time statistics
            stats_.processing_time_ms = millis() - start_time;
            
            return results;
        }
    }
    
    // Fallback to basic motion detection if AI processor not available
    bool motion_detected = false;
    if (config_.enable_motion_trigger) {
        motion_detected = detectMotion(frame_data, frame_size);
    }
    
    // If motion detected or continuous detection enabled, proceed with classification
    if (motion_detected || !config_.enable_motion_trigger) {
        DetectionResult detection;
        detection.species = SpeciesType::UNKNOWN;
        detection.confidence = ConfidenceLevel::MEDIUM;
        detection.confidence_score = 0.5f;
        detection.detection_time = millis();
        detection.x = width / 4;
        detection.y = height / 4;
        detection.width = width / 2;
        detection.height = height / 2;
        detection.motion_detected = motion_detected;
        
        // Basic species classification
        if (config_.enable_species_classification) {
            detection.species = classifyObject(frame_data, detection.x, detection.y, 
                                             detection.width, detection.height);
        }
        
        // Size estimation
        if (config_.enable_size_estimation) {
            detection.size_estimate = estimateSize(detection.width, detection.height, 
                                                  frame_width_, frame_height_);
        }
        
        // Apply confidence threshold
        if (detection.confidence_score >= config_.confidence_threshold) {
            results.push_back(detection);
            updateStatistics(detection);
        }
    }
    
    // Update processing time statistics
    stats_.processing_time_ms = millis() - start_time;
    
    // Store current frame for next motion detection
    if (previous_frame_) {
        memcpy(previous_frame_.get(), frame_data, frame_size);
    }
    
    return results;
}

bool WildlifeDetector::runBasicAIAnalysis(const uint8_t* frame_data, size_t frame_size,
                                         uint16_t width, uint16_t height,
                                         BasicAI::AIAnalysisResult& result) {
    if (!ai_processor_ || !ai_processor_->isInitialized()) {
        return false;
    }
    
    // Assume grayscale input for basic processing
    // In a real implementation, channel count would be determined from frame format
    result = ai_processor_->analyzeFrame(frame_data, frame_size, width, height, 1);
    
    return true;
}

bool WildlifeDetector::detectMotion(const uint8_t* frame_data, size_t frame_size) {
    if (!previous_frame_ || !frame_data) {
        return false;
    }
    
    // Simple motion detection using frame differencing
    uint32_t diff_sum = 0;
    uint32_t pixel_count = frame_size;
    
    for (size_t i = 0; i < frame_size; i++) {
        int32_t diff = abs(static_cast<int32_t>(frame_data[i]) - static_cast<int32_t>(previous_frame_[i]));
        diff_sum += diff;
    }
    
    // Calculate average difference
    float avg_diff = static_cast<float>(diff_sum) / pixel_count;
    
    // Motion threshold - this could be made configurable
    const float MOTION_THRESHOLD = 10.0f;
    
    return avg_diff > MOTION_THRESHOLD;
}

void WildlifeDetector::updateConfig(const DetectorConfig& config) {
    config_ = config;
}

void WildlifeDetector::resetStatistics() {
    memset(&stats_, 0, sizeof(Statistics));
}

SpeciesType WildlifeDetector::classifyObject(const uint8_t* region_data, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // Foundational classification logic - simplified for framework
    // In a full implementation, this would use TensorFlow Lite models
    
    // Basic heuristics based on size and position
    float size_ratio = static_cast<float>(w * h) / (frame_width_ * frame_height_);
    
    if (size_ratio > 0.3f) {
        return SpeciesType::DEER;  // Large object
    } else if (size_ratio > 0.1f) {
        return SpeciesType::RACCOON;  // Medium object
    } else if (size_ratio > 0.02f) {
        return SpeciesType::RABBIT;   // Small object
    } else {
        return SpeciesType::BIRD;     // Very small object
    }
}

ConfidenceLevel WildlifeDetector::calculateConfidence(float score) {
    if (score >= 0.8f) return ConfidenceLevel::VERY_HIGH;
    if (score >= 0.6f) return ConfidenceLevel::HIGH;
    if (score >= 0.4f) return ConfidenceLevel::MEDIUM;
    if (score >= 0.2f) return ConfidenceLevel::LOW;
    return ConfidenceLevel::VERY_LOW;
}

float WildlifeDetector::estimateSize(uint16_t width, uint16_t height, uint16_t frame_width, uint16_t frame_height) {
    float area_ratio = static_cast<float>(width * height) / (frame_width * frame_height);
    return std::min(1.0f, area_ratio * 10.0f);  // Scale and clamp to 0-1
}

void WildlifeDetector::updateStatistics(const DetectionResult& result) {
    stats_.total_detections++;
    
    if (static_cast<int>(result.species) <= static_cast<int>(SpeciesType::HUMAN)) {
        stats_.species_counts[static_cast<int>(result.species)]++;
    }
    
    // Update running average confidence
    stats_.average_confidence = (stats_.average_confidence * (stats_.total_detections - 1) + 
                                result.confidence_score) / stats_.total_detections;
}

// Arduino compatibility function
uint32_t millis() {
    // This would be implemented differently depending on the platform
    // For Arduino/ESP32, this function is provided by the framework
    #ifdef ARDUINO
        return ::millis();
    #else
        // For testing/simulation, return a simple counter
        static uint32_t counter = 0;
        return ++counter;
    #endif
}

} // namespace WildlifeDetection