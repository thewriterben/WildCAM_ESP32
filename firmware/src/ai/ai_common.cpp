/**
 * @file ai_common.cpp
 * @brief Implementation of common AI/ML utilities
 */

#include "ai_common.h"
#include <esp_camera.h>

/**
 * Convert SpeciesType enum to string representation
 */
const char* speciesTypeToString(SpeciesType species) {
    switch (species) {
        case SpeciesType::UNKNOWN: return "Unknown";
        case SpeciesType::MAMMAL_SMALL: return "Small Mammal";
        case SpeciesType::MAMMAL_MEDIUM: return "Medium Mammal";
        case SpeciesType::MAMMAL_LARGE: return "Large Mammal";
        case SpeciesType::BIRD_SMALL: return "Small Bird";
        case SpeciesType::BIRD_MEDIUM: return "Medium Bird";
        case SpeciesType::BIRD_LARGE: return "Large Bird";
        case SpeciesType::REPTILE: return "Reptile";
        case SpeciesType::HUMAN: return "Human";
        case SpeciesType::VEHICLE: return "Vehicle";
        default: return "Invalid";
    }
}

/**
 * Convert BehaviorType enum to string representation
 */
const char* behaviorTypeToString(BehaviorType behavior) {
    switch (behavior) {
        case BehaviorType::UNKNOWN: return "Unknown";
        case BehaviorType::FEEDING: return "Feeding";
        case BehaviorType::DRINKING: return "Drinking";
        case BehaviorType::RESTING: return "Resting";
        case BehaviorType::MOVING: return "Moving";
        case BehaviorType::ALERT: return "Alert";
        case BehaviorType::GROOMING: return "Grooming";
        case BehaviorType::MATING: return "Mating";
        case BehaviorType::AGGRESSIVE: return "Aggressive";
        case BehaviorType::TERRITORIAL: return "Territorial";
        case BehaviorType::SOCIAL: return "Social";
        default: return "Invalid";
    }
}

/**
 * Convert ConfidenceLevel enum to string representation
 */
const char* confidenceLevelToString(ConfidenceLevel level) {
    switch (level) {
        case ConfidenceLevel::VERY_LOW: return "Very Low";
        case ConfidenceLevel::LOW: return "Low";
        case ConfidenceLevel::MEDIUM: return "Medium";
        case ConfidenceLevel::HIGH: return "High";
        case ConfidenceLevel::VERY_HIGH: return "Very High";
        default: return "Invalid";
    }
}

/**
 * Convert float confidence to ConfidenceLevel enum
 */
ConfidenceLevel floatToConfidenceLevel(float confidence) {
    if (confidence < 0.2f) return ConfidenceLevel::VERY_LOW;
    if (confidence < 0.4f) return ConfidenceLevel::LOW;
    if (confidence < 0.6f) return ConfidenceLevel::MEDIUM;
    if (confidence < 0.8f) return ConfidenceLevel::HIGH;
    return ConfidenceLevel::VERY_HIGH;
}

/**
 * Constructor for CameraFrame from ESP32 camera frame buffer
 */
CameraFrame::CameraFrame(camera_fb_t* fb) {
    if (fb) {
        data = fb->buf;
        length = fb->len;
        width = fb->width;
        height = fb->height;
        format = fb->format;
        timestamp = millis();
    } else {
        data = nullptr;
        length = 0;
        width = 0;
        height = 0;
        format = 0;
        timestamp = 0;
    }
}