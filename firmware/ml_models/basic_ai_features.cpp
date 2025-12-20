/**
 * @file basic_ai_features.cpp
 * @brief Implementation of basic AI features for ESP32 Wildlife Camera
 * @version 1.0.0
 * 
 * Implements on-device AI capabilities optimized for ESP32:
 * - Motion detection with false positive reduction
 * - Simple animal vs. non-animal classification  
 * - Size estimation for detected objects
 */

#include "basic_ai_features.h"
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef ARDUINO
#include <Arduino.h>
#define GET_TIME_US() micros()
#define GET_TIME_MS() millis()
#else
// For testing without Arduino
#include <chrono>
static uint32_t GET_TIME_US() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}
static uint32_t GET_TIME_MS() {
    return GET_TIME_US() / 1000;
}
#endif

namespace BasicAI {

//==============================================================================
// Default Configurations
//==============================================================================

BasicAIConfig getDefaultConfig() {
    BasicAIConfig config;
    config.motion_threshold = DEFAULT_MOTION_THRESHOLD;
    config.min_motion_frames = 2;
    config.enable_temporal_filtering = true;
    config.noise_reduction_level = 0.3f;
    config.animal_confidence_threshold = CLASSIFICATION_THRESHOLD;
    config.enable_human_detection = true;
    config.enable_vehicle_detection = true;
    config.enable_size_estimation = true;
    config.min_object_size = 0.01f;
    config.max_object_size = 0.9f;
    config.fast_mode = false;
    config.processing_width = 320;
    config.processing_height = 240;
    return config;
}

BasicAIConfig getLowPowerConfig() {
    BasicAIConfig config = getDefaultConfig();
    config.fast_mode = true;
    config.processing_width = 160;
    config.processing_height = 120;
    config.enable_temporal_filtering = false;
    config.min_motion_frames = 1;
    return config;
}

BasicAIConfig getHighAccuracyConfig() {
    BasicAIConfig config = getDefaultConfig();
    config.motion_threshold = 20;
    config.min_motion_frames = 3;
    config.noise_reduction_level = 0.5f;
    config.animal_confidence_threshold = 0.6f;
    config.fast_mode = false;
    config.processing_width = 640;
    config.processing_height = 480;
    return config;
}

//==============================================================================
// Utility Functions
//==============================================================================

const char* classificationTypeToString(ClassificationType type) {
    switch (type) {
        case ClassificationType::UNKNOWN:    return "Unknown";
        case ClassificationType::ANIMAL:     return "Animal";
        case ClassificationType::NON_ANIMAL: return "Non-Animal";
        case ClassificationType::HUMAN:      return "Human";
        case ClassificationType::VEHICLE:    return "Vehicle";
        default:                             return "Invalid";
    }
}

const char* sizeCategoryToString(SizeCategory category) {
    switch (category) {
        case SizeCategory::TINY:       return "Tiny";
        case SizeCategory::SMALL:      return "Small";
        case SizeCategory::MEDIUM:     return "Medium";
        case SizeCategory::LARGE:      return "Large";
        case SizeCategory::VERY_LARGE: return "Very Large";
        default:                       return "Unknown";
    }
}

const char* motionConfidenceToString(MotionConfidence confidence) {
    switch (confidence) {
        case MotionConfidence::NONE:      return "None";
        case MotionConfidence::LOW:       return "Low";
        case MotionConfidence::MEDIUM:    return "Medium";
        case MotionConfidence::HIGH:      return "High";
        case MotionConfidence::VERY_HIGH: return "Very High";
        default:                          return "Unknown";
    }
}

//==============================================================================
// BasicAIProcessor Implementation
//==============================================================================

BasicAIProcessor::BasicAIProcessor()
    : initialized_(false)
    , previous_frame_(nullptr)
    , diff_frame_(nullptr)
    , frame_buffer_size_(0)
    , current_width_(0)
    , current_height_(0)
    , history_index_(0) {
    
    // Initialize statistics
    memset(&stats_, 0, sizeof(Statistics));
    
    // Initialize motion history
    memset(motion_history_, 0, sizeof(motion_history_));
}

BasicAIProcessor::~BasicAIProcessor() {
    freeBuffers();
}

bool BasicAIProcessor::initialize(const BasicAIConfig& config) {
    config_ = config;
    
    // Validate configuration
    if (config_.processing_width == 0 || config_.processing_height == 0) {
        return false;
    }
    
    if (config_.motion_threshold == 0 || config_.motion_threshold > 255) {
        config_.motion_threshold = DEFAULT_MOTION_THRESHOLD;
    }
    
    // Allocate initial buffers
    size_t buffer_size = config_.processing_width * config_.processing_height;
    if (!allocateBuffers(buffer_size)) {
        return false;
    }
    
    current_width_ = config_.processing_width;
    current_height_ = config_.processing_height;
    
    // Reset state
    reset();
    
    initialized_ = true;
    return true;
}

void BasicAIProcessor::updateConfig(const BasicAIConfig& config) {
    config_ = config;
    
    // Reallocate buffers if size changed
    size_t new_size = config_.processing_width * config_.processing_height;
    if (new_size != frame_buffer_size_) {
        freeBuffers();
        allocateBuffers(new_size);
        current_width_ = config_.processing_width;
        current_height_ = config_.processing_height;
    }
}

bool BasicAIProcessor::allocateBuffers(size_t size) {
    freeBuffers();
    
    previous_frame_ = new (std::nothrow) uint8_t[size];
    diff_frame_ = new (std::nothrow) uint8_t[size];
    
    if (!previous_frame_ || !diff_frame_) {
        freeBuffers();
        return false;
    }
    
    memset(previous_frame_, 0, size);
    memset(diff_frame_, 0, size);
    frame_buffer_size_ = size;
    
    return true;
}

void BasicAIProcessor::freeBuffers() {
    if (previous_frame_) {
        delete[] previous_frame_;
        previous_frame_ = nullptr;
    }
    if (diff_frame_) {
        delete[] diff_frame_;
        diff_frame_ = nullptr;
    }
    frame_buffer_size_ = 0;
}

void BasicAIProcessor::reset() {
    if (previous_frame_ && frame_buffer_size_ > 0) {
        memset(previous_frame_, 0, frame_buffer_size_);
    }
    if (diff_frame_ && frame_buffer_size_ > 0) {
        memset(diff_frame_, 0, frame_buffer_size_);
    }
    
    memset(motion_history_, 0, sizeof(motion_history_));
    history_index_ = 0;
}

void BasicAIProcessor::resetStatistics() {
    memset(&stats_, 0, sizeof(Statistics));
}

//==============================================================================
// Motion Detection with False Positive Reduction
//==============================================================================

MotionDetectionResult BasicAIProcessor::detectMotion(
    const uint8_t* current_frame,
    size_t frame_size,
    uint16_t width,
    uint16_t height,
    uint8_t channels) {
    
    MotionDetectionResult result;
    memset(&result, 0, sizeof(result));
    
    if (!initialized_ || !current_frame || frame_size == 0) {
        return result;
    }
    
    uint32_t start_time = GET_TIME_US();
    
    // Convert to grayscale if needed
    uint8_t* grayscale_frame = nullptr;
    size_t grayscale_size = width * height;
    
    if (channels > 1) {
        grayscale_frame = new (std::nothrow) uint8_t[grayscale_size];
        if (!grayscale_frame) {
            return result;
        }
        convertToGrayscale(current_frame, grayscale_frame, width * height, channels);
    } else {
        // Use frame directly (cast away const for internal use only)
        grayscale_frame = const_cast<uint8_t*>(current_frame);
    }
    
    // Resize buffers if frame size changed
    if (grayscale_size != frame_buffer_size_) {
        allocateBuffers(grayscale_size);
        current_width_ = width;
        current_height_ = height;
    }
    
    // Calculate frame difference
    calculateFrameDifference(
        grayscale_frame, 
        previous_frame_, 
        diff_frame_, 
        grayscale_size,
        config_.motion_threshold
    );
    
    // Find motion regions
    result.region_count = findMotionRegions(
        diff_frame_,
        width,
        height,
        result.regions,
        MAX_MOTION_REGIONS
    );
    
    // Determine if motion is detected
    result.motion_detected = result.region_count > 0;
    
    // Calculate overall motion intensity
    if (result.motion_detected) {
        float total_intensity = 0.0f;
        for (uint8_t i = 0; i < result.region_count; i++) {
            total_intensity += result.regions[i].intensity;
        }
        float avg_intensity = total_intensity / result.region_count;
        
        // Determine confidence level based on intensity and region count
        if (avg_intensity > 0.7f && result.region_count >= 1) {
            result.confidence = MotionConfidence::VERY_HIGH;
        } else if (avg_intensity > 0.5f && result.region_count >= 1) {
            result.confidence = MotionConfidence::HIGH;
        } else if (avg_intensity > 0.3f) {
            result.confidence = MotionConfidence::MEDIUM;
        } else {
            result.confidence = MotionConfidence::LOW;
        }
    } else {
        result.confidence = MotionConfidence::NONE;
    }
    
    // Analyze false positive likelihood
    if (config_.enable_temporal_filtering) {
        updateMotionHistory(result);
        result.false_positive_score = analyzeFalsePositive(result);
        
        // Filter out likely false positives
        if (result.false_positive_score > 0.7f) {
            // High likelihood of false positive - reduce confidence
            if (result.confidence > MotionConfidence::LOW) {
                result.confidence = MotionConfidence::LOW;
            }
        }
    }
    
    // Store current frame for next comparison
    memcpy(previous_frame_, grayscale_frame, grayscale_size);
    
    // Clean up temporary buffer if we allocated it
    if (channels > 1 && grayscale_frame != current_frame) {
        delete[] grayscale_frame;
    }
    
    result.processing_time_us = GET_TIME_US() - start_time;
    
    // Update statistics
    updateStatistics(result.processing_time_us, result.motion_detected, false, 
                    result.false_positive_score > 0.5f);
    
    return result;
}

void BasicAIProcessor::calculateFrameDifference(
    const uint8_t* current,
    const uint8_t* previous,
    uint8_t* diff,
    size_t size,
    uint8_t threshold) {
    
    for (size_t i = 0; i < size; i++) {
        int16_t difference = static_cast<int16_t>(current[i]) - static_cast<int16_t>(previous[i]);
        uint8_t abs_diff = static_cast<uint8_t>(abs(difference));
        
        // Apply threshold
        diff[i] = (abs_diff > threshold) ? abs_diff : 0;
    }
}

uint8_t BasicAIProcessor::findMotionRegions(
    const uint8_t* diff_frame,
    uint16_t width,
    uint16_t height,
    MotionRegion* regions,
    uint8_t max_regions) {
    
    uint8_t region_count = 0;
    
    // Simple grid-based region detection
    // Divide frame into grid cells and find cells with significant motion
    const uint8_t grid_cols = 8;
    const uint8_t grid_rows = 6;
    uint16_t cell_width = width / grid_cols;
    uint16_t cell_height = height / grid_rows;
    
    // Track which cells have motion
    bool cell_motion[grid_rows][grid_cols];
    float cell_intensity[grid_rows][grid_cols];
    memset(cell_motion, 0, sizeof(cell_motion));
    memset(cell_intensity, 0, sizeof(cell_intensity));
    
    // Analyze each cell
    for (uint8_t row = 0; row < grid_rows; row++) {
        for (uint8_t col = 0; col < grid_cols; col++) {
            uint32_t motion_pixels = 0;
            uint32_t total_diff = 0;
            
            uint16_t start_x = col * cell_width;
            uint16_t start_y = row * cell_height;
            
            for (uint16_t y = start_y; y < start_y + cell_height && y < height; y++) {
                for (uint16_t x = start_x; x < start_x + cell_width && x < width; x++) {
                    uint8_t diff = diff_frame[y * width + x];
                    if (diff > 0) {
                        motion_pixels++;
                        total_diff += diff;
                    }
                }
            }
            
            // Check if cell has significant motion
            uint32_t cell_pixels = cell_width * cell_height;
            float motion_ratio = static_cast<float>(motion_pixels) / cell_pixels;
            
            if (motion_ratio > 0.1f && motion_pixels >= MIN_MOTION_PIXELS / (grid_cols * grid_rows)) {
                cell_motion[row][col] = true;
                cell_intensity[row][col] = motion_ratio;
            }
        }
    }
    
    // Merge adjacent cells into regions
    bool visited[grid_rows][grid_cols];
    memset(visited, 0, sizeof(visited));
    
    for (uint8_t row = 0; row < grid_rows && region_count < max_regions; row++) {
        for (uint8_t col = 0; col < grid_cols && region_count < max_regions; col++) {
            if (cell_motion[row][col] && !visited[row][col]) {
                // Start a new region
                MotionRegion& region = regions[region_count];
                
                // Find bounds of connected cells
                uint8_t min_row = row, max_row = row;
                uint8_t min_col = col, max_col = col;
                float total_intensity = 0.0f;
                uint8_t intensity_count = 0;
                
                // Simple flood fill to find connected region
                for (uint8_t r = row; r < grid_rows; r++) {
                    bool found_in_row = false;
                    for (uint8_t c = 0; c < grid_cols; c++) {
                        if (cell_motion[r][c] && !visited[r][c]) {
                            // Check if adjacent to current region
                            bool adjacent = false;
                            if (r == row || 
                                (r > 0 && visited[r-1][c]) ||
                                (c > 0 && visited[r][c-1])) {
                                adjacent = true;
                            }
                            
                            if (adjacent || (r == row && c == col)) {
                                visited[r][c] = true;
                                found_in_row = true;
                                
                                if (r < min_row) min_row = r;
                                if (r > max_row) max_row = r;
                                if (c < min_col) min_col = c;
                                if (c > max_col) max_col = c;
                                
                                total_intensity += cell_intensity[r][c];
                                intensity_count++;
                            }
                        }
                    }
                    if (!found_in_row && r > row) break;
                }
                
                // Calculate region bounds in pixels
                region.bbox.x = min_col * cell_width;
                region.bbox.y = min_row * cell_height;
                region.bbox.width = (max_col - min_col + 1) * cell_width;
                region.bbox.height = (max_row - min_row + 1) * cell_height;
                
                // Clamp to frame bounds
                if (region.bbox.x + region.bbox.width > width) {
                    region.bbox.width = width - region.bbox.x;
                }
                if (region.bbox.y + region.bbox.height > height) {
                    region.bbox.height = height - region.bbox.y;
                }
                
                region.intensity = intensity_count > 0 ? total_intensity / intensity_count : 0.0f;
                region.pixel_count = intensity_count * cell_width * cell_height;
                region.timestamp = GET_TIME_MS();
                region.is_valid = true;
                
                region_count++;
            }
        }
    }
    
    return region_count;
}

float BasicAIProcessor::analyzeFalsePositive(const MotionDetectionResult& result) {
    if (!result.motion_detected) {
        return 0.0f;
    }
    
    float fp_score = 0.0f;
    
    // Factor 1: Motion consistency over time
    float consistency = analyzeMotionConsistency();
    if (consistency < 0.3f) {
        // Sporadic motion is more likely a false positive
        fp_score += 0.3f;
    }
    
    // Factor 2: Motion intensity
    float avg_intensity = 0.0f;
    for (uint8_t i = 0; i < result.region_count; i++) {
        avg_intensity += result.regions[i].intensity;
    }
    if (result.region_count > 0) {
        avg_intensity /= result.region_count;
    }
    
    // Very low intensity motion is often noise
    if (avg_intensity < 0.2f) {
        fp_score += 0.3f;
    }
    
    // Factor 3: Region characteristics
    // Too many small regions often indicate noise (wind, lighting changes)
    if (result.region_count > 5) {
        fp_score += 0.2f;
    }
    
    // Factor 4: Region size
    // Very large regions covering most of the frame are often lighting changes
    for (uint8_t i = 0; i < result.region_count; i++) {
        float region_ratio = static_cast<float>(result.regions[i].bbox.width * result.regions[i].bbox.height) /
                            (current_width_ * current_height_);
        if (region_ratio > 0.6f) {
            fp_score += 0.2f;
            break;
        }
    }
    
    return std::min(1.0f, fp_score);
}

void BasicAIProcessor::updateMotionHistory(const MotionDetectionResult& result) {
    motion_history_[history_index_].timestamp = GET_TIME_MS();
    motion_history_[history_index_].intensity = 0.0f;
    motion_history_[history_index_].region_count = result.region_count;
    motion_history_[history_index_].valid = result.motion_detected;
    
    if (result.motion_detected && result.region_count > 0) {
        float total_intensity = 0.0f;
        for (uint8_t i = 0; i < result.region_count; i++) {
            total_intensity += result.regions[i].intensity;
        }
        motion_history_[history_index_].intensity = total_intensity / result.region_count;
    }
    
    history_index_ = (history_index_ + 1) % MOTION_HISTORY_SIZE;
}

float BasicAIProcessor::analyzeMotionConsistency() {
    uint8_t valid_count = 0;
    uint8_t motion_count = 0;
    
    for (uint8_t i = 0; i < MOTION_HISTORY_SIZE; i++) {
        if (motion_history_[i].timestamp > 0) {
            valid_count++;
            if (motion_history_[i].valid) {
                motion_count++;
            }
        }
    }
    
    if (valid_count < 2) {
        return 0.5f; // Not enough history
    }
    
    return static_cast<float>(motion_count) / valid_count;
}

//==============================================================================
// Animal vs Non-Animal Classification
//==============================================================================

ClassificationResult BasicAIProcessor::classifyRegion(
    const uint8_t* frame_data,
    size_t frame_size,
    uint16_t width,
    uint16_t height,
    const BoundingBox& roi,
    uint8_t channels) {
    
    ClassificationResult result;
    memset(&result, 0, sizeof(result));
    result.classification = ClassificationType::UNKNOWN;
    
    if (!initialized_ || !frame_data || frame_size == 0) {
        return result;
    }
    
    uint32_t start_time = GET_TIME_US();
    
    // Validate ROI bounds
    if (roi.x >= width || roi.y >= height || 
        roi.width == 0 || roi.height == 0) {
        return result;
    }
    
    uint16_t roi_w = std::min(static_cast<uint16_t>(roi.x + roi.width), width) - roi.x;
    uint16_t roi_h = std::min(static_cast<uint16_t>(roi.y + roi.height), height) - roi.y;
    
    // Extract ROI data
    size_t roi_size = roi_w * roi_h;
    uint8_t* roi_data = new (std::nothrow) uint8_t[roi_size];
    if (!roi_data) {
        return result;
    }
    
    // Copy ROI and convert to grayscale if needed
    size_t roi_idx = 0;
    for (uint16_t y = roi.y; y < roi.y + roi_h; y++) {
        for (uint16_t x = roi.x; x < roi.x + roi_w; x++) {
            size_t src_idx = (y * width + x) * channels;
            if (channels == 1) {
                roi_data[roi_idx] = frame_data[src_idx];
            } else {
                // RGB to grayscale: 0.299*R + 0.587*G + 0.114*B
                roi_data[roi_idx] = static_cast<uint8_t>(
                    0.299f * frame_data[src_idx] +
                    0.587f * frame_data[src_idx + 1] +
                    0.114f * frame_data[src_idx + 2]
                );
            }
            roi_idx++;
        }
    }
    
    // Calculate texture and edge features
    float texture_score = calculateTextureScore(roi_data, roi_w, roi_h);
    float edge_density = calculateEdgeDensity(roi_data, roi_w, roi_h);
    
    // Simple classification heuristics based on texture and edges
    // Animals tend to have moderate texture and clear edges
    // Non-animals (vegetation, shadows) tend to have high texture or very smooth areas
    
    // Animal characteristics:
    // - Moderate to high texture (fur, feathers)
    // - Clear edges (body outline)
    // - Not too uniform (unlike flat surfaces)
    
    float animal_features = 0.0f;
    float non_animal_features = 0.0f;
    
    // Texture analysis
    if (texture_score > 0.2f && texture_score < 0.7f) {
        animal_features += 0.3f; // Moderate texture suggests animal fur/feathers
    } else if (texture_score > 0.8f) {
        non_animal_features += 0.3f; // Very high texture often indicates vegetation
    } else if (texture_score < 0.1f) {
        non_animal_features += 0.2f; // Very smooth suggests shadow or solid surface
    }
    
    // Edge density analysis
    if (edge_density > 0.15f && edge_density < 0.5f) {
        animal_features += 0.3f; // Clear body edges suggest animal
    } else if (edge_density > 0.6f) {
        non_animal_features += 0.3f; // Too many edges suggest complex background
    } else if (edge_density < 0.05f) {
        non_animal_features += 0.2f; // No edges suggest uniform area
    }
    
    // Size-based adjustment (ROI relative to frame)
    float roi_ratio = static_cast<float>(roi_w * roi_h) / (width * height);
    if (roi_ratio > 0.02f && roi_ratio < 0.5f) {
        animal_features += 0.2f; // Reasonable size for wildlife
    } else if (roi_ratio < 0.01f) {
        non_animal_features += 0.1f; // Too small, likely noise
    } else if (roi_ratio > 0.7f) {
        non_animal_features += 0.2f; // Too large, likely background change
    }
    
    // Aspect ratio analysis (most animals have certain aspect ratios)
    float aspect_ratio = static_cast<float>(roi_w) / roi_h;
    if (aspect_ratio > 0.3f && aspect_ratio < 3.0f) {
        animal_features += 0.1f; // Reasonable aspect ratio for animals
    }
    
    // Normalize scores
    float total = animal_features + non_animal_features;
    if (total > 0) {
        result.animal_score = animal_features / (total + 0.1f);
        result.non_animal_score = non_animal_features / (total + 0.1f);
    } else {
        result.animal_score = 0.5f;
        result.non_animal_score = 0.5f;
    }
    
    // Make classification decision
    if (result.animal_score > config_.animal_confidence_threshold && 
        result.animal_score > result.non_animal_score) {
        result.classification = ClassificationType::ANIMAL;
        result.confidence = result.animal_score;
    } else if (result.non_animal_score > result.animal_score) {
        result.classification = ClassificationType::NON_ANIMAL;
        result.confidence = result.non_animal_score;
    } else {
        result.classification = ClassificationType::UNKNOWN;
        result.confidence = std::max(result.animal_score, result.non_animal_score);
    }
    
    delete[] roi_data;
    
    result.processing_time_us = GET_TIME_US() - start_time;
    
    return result;
}

float BasicAIProcessor::calculateTextureScore(
    const uint8_t* region_data,
    uint16_t width,
    uint16_t height) {
    
    if (!region_data || width < 3 || height < 3) {
        return 0.0f;
    }
    
    // Calculate local variance as texture measure
    uint64_t variance_sum = 0;
    uint32_t sample_count = 0;
    
    // Sample pixels in a grid pattern for efficiency
    uint16_t step = config_.fast_mode ? 4 : 2;
    
    for (uint16_t y = 1; y < height - 1; y += step) {
        for (uint16_t x = 1; x < width - 1; x += step) {
            // Calculate local variance using 3x3 neighborhood
            int32_t sum = 0;
            int32_t sum_sq = 0;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    uint8_t pixel = region_data[(y + dy) * width + (x + dx)];
                    sum += pixel;
                    sum_sq += pixel * pixel;
                }
            }
            
            // Variance = E[X^2] - E[X]^2
            float mean = sum / 9.0f;
            float variance = (sum_sq / 9.0f) - (mean * mean);
            variance_sum += static_cast<uint64_t>(variance);
            sample_count++;
        }
    }
    
    if (sample_count == 0) {
        return 0.0f;
    }
    
    // Normalize variance to 0-1 range (max variance for 8-bit is ~16000)
    float avg_variance = static_cast<float>(variance_sum) / sample_count;
    float texture_score = std::min(1.0f, avg_variance / 2000.0f);
    
    return texture_score;
}

float BasicAIProcessor::calculateEdgeDensity(
    const uint8_t* region_data,
    uint16_t width,
    uint16_t height) {
    
    if (!region_data || width < 3 || height < 3) {
        return 0.0f;
    }
    
    // Simple Sobel-like edge detection
    uint32_t edge_count = 0;
    uint32_t total_pixels = 0;
    const int edge_threshold = 30;
    
    uint16_t step = config_.fast_mode ? 3 : 1;
    
    for (uint16_t y = 1; y < height - 1; y += step) {
        for (uint16_t x = 1; x < width - 1; x += step) {
            // Calculate gradient using simple differences
            int gx = region_data[y * width + (x + 1)] - region_data[y * width + (x - 1)];
            int gy = region_data[(y + 1) * width + x] - region_data[(y - 1) * width + x];
            
            // Magnitude approximation
            int magnitude = abs(gx) + abs(gy);
            
            if (magnitude > edge_threshold) {
                edge_count++;
            }
            total_pixels++;
        }
    }
    
    if (total_pixels == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(edge_count) / total_pixels;
}

bool BasicAIProcessor::hasAnimalCharacteristics(
    const uint8_t* frame_data,
    uint16_t width,
    uint16_t height,
    uint8_t channels) {
    
    BoundingBox full_frame;
    full_frame.x = 0;
    full_frame.y = 0;
    full_frame.width = width;
    full_frame.height = height;
    
    ClassificationResult result = classifyRegion(
        frame_data, width * height * channels,
        width, height, full_frame, channels
    );
    
    return result.classification == ClassificationType::ANIMAL &&
           result.confidence >= config_.animal_confidence_threshold;
}

//==============================================================================
// Size Estimation
//==============================================================================

SizeEstimationResult BasicAIProcessor::estimateSize(
    const BoundingBox& bbox,
    uint16_t frame_width,
    uint16_t frame_height) {
    
    SizeEstimationResult result;
    memset(&result, 0, sizeof(result));
    
    if (frame_width == 0 || frame_height == 0) {
        return result;
    }
    
    // Calculate relative size
    uint32_t bbox_area = bbox.width * bbox.height;
    uint32_t frame_area = frame_width * frame_height;
    
    result.relative_size = static_cast<float>(bbox_area) / frame_area;
    result.estimated_pixels = bbox_area;
    result.width_ratio = static_cast<float>(bbox.width) / frame_width;
    result.height_ratio = static_cast<float>(bbox.height) / frame_height;
    
    // Determine size category
    result.category = getSizeCategory(result.relative_size);
    
    return result;
}

SizeCategory BasicAIProcessor::getSizeCategory(float relative_size) {
    if (relative_size < 0.05f) {
        return SizeCategory::TINY;
    } else if (relative_size < 0.15f) {
        return SizeCategory::SMALL;
    } else if (relative_size < 0.30f) {
        return SizeCategory::MEDIUM;
    } else if (relative_size < 0.50f) {
        return SizeCategory::LARGE;
    } else {
        return SizeCategory::VERY_LARGE;
    }
}

//==============================================================================
// Combined Analysis
//==============================================================================

AIAnalysisResult BasicAIProcessor::analyzeFrame(
    const uint8_t* current_frame,
    size_t frame_size,
    uint16_t width,
    uint16_t height,
    uint8_t channels) {
    
    AIAnalysisResult result;
    memset(&result, 0, sizeof(result));
    
    if (!initialized_ || !current_frame || frame_size == 0) {
        return result;
    }
    
    uint32_t start_time = GET_TIME_US();
    
    // Step 1: Motion detection with false positive reduction
    result.motion = detectMotion(current_frame, frame_size, width, height, channels);
    
    // Step 2: If motion detected, classify and estimate size
    if (result.motion.motion_detected && result.motion.region_count > 0) {
        // Find the largest/most significant motion region
        uint8_t best_region = 0;
        float best_intensity = 0.0f;
        
        for (uint8_t i = 0; i < result.motion.region_count; i++) {
            if (result.motion.regions[i].intensity > best_intensity) {
                best_intensity = result.motion.regions[i].intensity;
                best_region = i;
            }
        }
        
        const MotionRegion& main_region = result.motion.regions[best_region];
        
        // Classify the main region
        result.classification = classifyRegion(
            current_frame, frame_size,
            width, height,
            main_region.bbox,
            channels
        );
        
        // Estimate size
        if (config_.enable_size_estimation) {
            result.size = estimateSize(main_region.bbox, width, height);
        }
        
        // Determine if this is a valid wildlife event
        result.is_valid_wildlife_event = 
            result.motion.confidence >= MotionConfidence::MEDIUM &&
            result.motion.false_positive_score < 0.5f &&
            result.classification.classification == ClassificationType::ANIMAL;
        
        // Calculate overall confidence
        float motion_conf = static_cast<float>(result.motion.confidence) / 
                          static_cast<float>(MotionConfidence::VERY_HIGH);
        float class_conf = result.classification.confidence;
        float fp_factor = 1.0f - result.motion.false_positive_score;
        
        result.overall_confidence = (motion_conf * 0.3f + class_conf * 0.5f + fp_factor * 0.2f);
        
        // Update statistics
        if (result.classification.classification == ClassificationType::ANIMAL) {
            stats_.animal_classifications++;
        } else {
            stats_.non_animal_classifications++;
        }
    }
    
    result.total_processing_time_us = GET_TIME_US() - start_time;
    
    return result;
}

//==============================================================================
// Internal Helpers
//==============================================================================

void BasicAIProcessor::convertToGrayscale(
    const uint8_t* input,
    uint8_t* output,
    size_t pixel_count,
    uint8_t channels) {
    
    if (channels == 1) {
        memcpy(output, input, pixel_count);
        return;
    }
    
    for (size_t i = 0; i < pixel_count; i++) {
        size_t src_idx = i * channels;
        // RGB to grayscale: 0.299*R + 0.587*G + 0.114*B
        output[i] = static_cast<uint8_t>(
            0.299f * input[src_idx] +
            0.587f * input[src_idx + 1] +
            0.114f * input[src_idx + 2]
        );
    }
}

void BasicAIProcessor::updateStatistics(
    uint32_t processing_time_us,
    bool motion_detected,
    bool is_animal,
    bool was_false_positive) {
    
    stats_.total_frames_processed++;
    
    if (motion_detected) {
        stats_.motion_detections++;
    }
    
    if (was_false_positive) {
        stats_.false_positives_filtered++;
    }
    
    // Update running average of processing time
    if (stats_.total_frames_processed == 1) {
        stats_.average_processing_time_us = processing_time_us;
    } else {
        stats_.average_processing_time_us = 
            (stats_.average_processing_time_us * (stats_.total_frames_processed - 1) + 
             processing_time_us) / stats_.total_frames_processed;
    }
}

} // namespace BasicAI
