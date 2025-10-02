/**
 * @file dangerous_species_alert.cpp
 * @brief Implementation of dangerous species alert system
 */

#include "dangerous_species_alert.h"
#include "../utils/logger.h"

bool DangerousSpeciesAlertSystem::initialize() {
    if (initialized) {
        return true;
    }
    
    LOG_INFO("Initializing dangerous species alert system...");
    
    // Reset statistics
    totalAlerts = 0;
    criticalAlerts = 0;
    consecutiveDetections = 0;
    lastAlertTime = 0;
    lastDetectedSpecies = WildlifeClassifier::SpeciesType::UNKNOWN;
    
    initialized = true;
    enabled = true;
    
    LOG_INFO("Dangerous species alert system initialized");
    LOG_INFO("Confidence threshold: " + String(confidenceThreshold, 2));
    LOG_INFO("Min consecutive detections: " + String(minConsecutiveDetections));
    
    return true;
}

DangerousSpeciesAlert* DangerousSpeciesAlertSystem::processClassification(
    const WildlifeClassifier::ClassificationResult& result) {
    
    if (!initialized || !enabled || !result.isValid) {
        return nullptr;
    }
    
    // Check if this is a dangerous species
    if (!WildlifeClassifier::isDangerousSpecies(result.species)) {
        // Reset consecutive count if not dangerous
        if (lastDetectedSpecies != result.species) {
            consecutiveDetections = 0;
            lastDetectedSpecies = WildlifeClassifier::SpeciesType::UNKNOWN;
        }
        return nullptr;
    }
    
    // Check confidence threshold
    if (result.confidence < confidenceThreshold) {
        LOG_DEBUG("Dangerous species detected but below confidence threshold: " + 
                 String(result.confidence, 2));
        return nullptr;
    }
    
    // Update consecutive detection count
    uint32_t count = updateConsecutiveCount(result.species);
    
    // Check if we have enough consecutive detections
    if (count < minConsecutiveDetections) {
        LOG_DEBUG("Dangerous species consecutive detection: " + String(count) + 
                 "/" + String(minConsecutiveDetections));
        return nullptr;
    }
    
    // Check cooldown period
    if (isInCooldown()) {
        LOG_DEBUG("Alert in cooldown period, skipping");
        return nullptr;
    }
    
    // Generate alert
    currentAlert = generateAlert(result);
    currentAlert.detectionCount = count;
    
    // Update statistics
    totalAlerts++;
    if (currentAlert.priority == AlertPriority::CRITICAL) {
        criticalAlerts++;
    }
    
    lastAlertTime = millis();
    
    LOG_WARNING("DANGEROUS SPECIES ALERT: " + currentAlert.alertMessage);
    LOG_WARNING("Priority: " + String(static_cast<int>(currentAlert.priority)) + 
               " | Confidence: " + String(currentAlert.confidence, 2));
    
    return &currentAlert;
}

AlertPriority DangerousSpeciesAlertSystem::getAlertPriority(WildlifeClassifier::SpeciesType species) {
    switch (species) {
        case WildlifeClassifier::SpeciesType::BLACK_BEAR:
            return AlertPriority::CRITICAL;  // Large, potentially dangerous
        
        case WildlifeClassifier::SpeciesType::GRAY_WOLF:
        case WildlifeClassifier::SpeciesType::MOUNTAIN_LION:
            return AlertPriority::HIGH;  // Apex predators
        
        case WildlifeClassifier::SpeciesType::COYOTE:
        case WildlifeClassifier::SpeciesType::BOBCAT:
            return AlertPriority::LOW;  // Generally avoid humans
        
        default:
            return AlertPriority::LOW;
    }
}

bool DangerousSpeciesAlertSystem::requiresImmediateAlert(
    WildlifeClassifier::SpeciesType species, float confidence) {
    
    // Critical priority species with high confidence require immediate alert
    if (getAlertPriority(species) == AlertPriority::CRITICAL && confidence >= 0.85f) {
        return true;
    }
    
    // High priority with very high confidence
    if (getAlertPriority(species) == AlertPriority::HIGH && confidence >= 0.90f) {
        return true;
    }
    
    return false;
}

String DangerousSpeciesAlertSystem::getAlertMessage(
    WildlifeClassifier::SpeciesType species, float confidence) {
    
    String speciesName = WildlifeClassifier::getSpeciesName(species);
    String confidenceStr = String(confidence * 100.0f, 1);
    
    String message = "ALERT: " + speciesName + " detected";
    message += " (Confidence: " + confidenceStr + "%)";
    
    AlertPriority priority = getAlertPriority(species);
    
    switch (priority) {
        case AlertPriority::CRITICAL:
            message += " - CRITICAL: Large predator detected! Exercise extreme caution.";
            break;
        case AlertPriority::HIGH:
            message += " - HIGH: Dangerous predator in area. Maintain safe distance.";
            break;
        case AlertPriority::LOW:
            message += " - LOW: Wildlife activity detected. Stay alert.";
            break;
        default:
            break;
    }
    
    return message;
}

void DangerousSpeciesAlertSystem::setConfidenceThreshold(float threshold) {
    confidenceThreshold = constrain(threshold, 0.0f, 1.0f);
    LOG_INFO("Alert confidence threshold set to: " + String(confidenceThreshold, 2));
}

void DangerousSpeciesAlertSystem::setMinConsecutiveDetections(uint32_t count) {
    minConsecutiveDetections = max(1u, count);
    LOG_INFO("Min consecutive detections set to: " + String(minConsecutiveDetections));
}

void DangerousSpeciesAlertSystem::clearAlertHistory() {
    totalAlerts = 0;
    criticalAlerts = 0;
    consecutiveDetections = 0;
    lastDetectedSpecies = WildlifeClassifier::SpeciesType::UNKNOWN;
    lastAlertTime = 0;
    LOG_INFO("Alert history cleared");
}

bool DangerousSpeciesAlertSystem::isInCooldown() {
    if (lastAlertTime == 0) {
        return false;
    }
    
    uint32_t elapsed = millis() - lastAlertTime;
    return (elapsed < ALERT_COOLDOWN_MS);
}

uint32_t DangerousSpeciesAlertSystem::updateConsecutiveCount(WildlifeClassifier::SpeciesType species) {
    if (species == lastDetectedSpecies) {
        consecutiveDetections++;
    } else {
        consecutiveDetections = 1;
        lastDetectedSpecies = species;
    }
    
    return consecutiveDetections;
}

DangerousSpeciesAlert DangerousSpeciesAlertSystem::generateAlert(
    const WildlifeClassifier::ClassificationResult& result) {
    
    DangerousSpeciesAlert alert;
    
    alert.species = result.species;
    alert.speciesName = result.speciesName;
    alert.confidence = result.confidence;
    alert.priority = getAlertPriority(result.species);
    alert.timestamp = millis();
    alert.requiresImmediate = requiresImmediateAlert(result.species, result.confidence);
    alert.alertMessage = getAlertMessage(result.species, result.confidence);
    alert.detectionCount = consecutiveDetections;
    alert.alertSent = false;
    
    return alert;
}
