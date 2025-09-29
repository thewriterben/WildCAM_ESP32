#include "multi_zone_pir_sensor.h"
#include <algorithm>

bool MultiZonePIRSensor::initialize() {
    if (initialized) {
        return true;
    }

    // Initialize with no zones - they need to be added explicitly
    zones.clear();
    pirSensors.clear();
    
    initialized = true;
    Serial.println("Multi-zone PIR sensor system initialized");
    
    return true;
}

bool MultiZonePIRSensor::addZone(uint8_t zoneId, uint8_t pin, const String& name, 
                                 float sensitivity, uint8_t priority) {
    if (!initialized) {
        Serial.println("Multi-zone PIR not initialized");
        return false;
    }

    // Check if zone already exists
    if (findZone(zoneId) != zones.end()) {
        Serial.printf("Zone %d already exists\n", zoneId);
        return false;
    }

    // Check pin availability (basic validation)
    if (pin == 0 || pin == PIR_PIN) {
        Serial.printf("Invalid or reserved pin %d for zone %d\n", pin, zoneId);
        return false;
    }

    // Create new zone
    PIRZone zone;
    zone.zoneId = zoneId;
    zone.pin = pin;
    zone.name = name;
    zone.sensitivity = constrain(sensitivity, 0.0f, 1.0f);
    zone.enabled = true;
    zone.priority = priority;
    zone.stats = {};

    // Add zone to list
    zones.push_back(zone);

    // Initialize PIR sensor for this zone
    if (!initializePIRForZone(zone)) {
        zones.pop_back();  // Remove zone if PIR initialization failed
        return false;
    }

    Serial.printf("Added PIR zone %d ('%s') on pin %d\n", zoneId, name.c_str(), pin);
    return true;
}

bool MultiZonePIRSensor::removeZone(uint8_t zoneId) {
    auto it = findZone(zoneId);
    if (it == zones.end()) {
        return false;
    }

    // Find and remove corresponding PIR sensor
    auto pirIt = std::find_if(pirSensors.begin(), pirSensors.end(), 
                              [zoneId](const PIRSensor& pir) {
                                  // This is a simplification - in a real implementation,
                                  // we'd need to track the association between zones and sensors
                                  return false; // Placeholder
                              });
    
    if (pirIt != pirSensors.end()) {
        pirIt->cleanup();
        pirSensors.erase(pirIt);
    }

    zones.erase(it);
    Serial.printf("Removed PIR zone %d\n", zoneId);
    return true;
}

MultiZonePIRSensor::MultiZoneResult MultiZonePIRSensor::detectMotion() {
    MultiZoneResult result = {};
    uint32_t startTime = millis();

    if (!initialized || !enabled || zones.empty()) {
        return result;
    }

    std::vector<uint8_t> activeZones;
    uint8_t highestPriorityZone = 255;
    uint8_t highestPriority = 255;

    // Check motion in each zone
    for (size_t i = 0; i < zones.size() && i < pirSensors.size(); i++) {
        const auto& zone = zones[i];
        
        if (!zone.enabled) {
            continue;
        }

        // Check motion on this PIR sensor
        bool zoneMotion = pirSensors[i].hasMotion();
        
        if (zoneMotion) {
            activeZones.push_back(zone.zoneId);
            
            // Track highest priority zone
            if (zone.priority < highestPriority) {
                highestPriority = zone.priority;
                highestPriorityZone = zone.zoneId;
            }

            // Update zone statistics
            updateZoneStatistics(zone.zoneId, zone.sensitivity);
        }
    }

    // Calculate results
    result.motionDetected = !activeZones.empty();
    result.activeZoneCount = activeZones.size();
    result.highestPriorityZone = highestPriorityZone;
    result.activeZones = activeZones;
    result.overallConfidence = calculateOverallConfidence(activeZones);
    result.processTime = millis() - startTime;
    result.description = generateResultDescription(result);

    return result;
}

bool MultiZonePIRSensor::setZoneEnabled(uint8_t zoneId, bool enabled) {
    auto it = findZone(zoneId);
    if (it == zones.end()) {
        return false;
    }

    it->enabled = enabled;
    
    // Enable/disable corresponding PIR sensor
    // Note: This is simplified - real implementation would need proper sensor mapping
    Serial.printf("Zone %d %s\n", zoneId, enabled ? "enabled" : "disabled");
    return true;
}

bool MultiZonePIRSensor::setZoneSensitivity(uint8_t zoneId, float sensitivity) {
    auto it = findZone(zoneId);
    if (it == zones.end()) {
        return false;
    }

    it->sensitivity = constrain(sensitivity, 0.0f, 1.0f);
    
    // Update corresponding PIR sensor sensitivity
    // Note: This is simplified - real implementation would need proper sensor mapping
    Serial.printf("Zone %d sensitivity set to %.2f\n", zoneId, it->sensitivity);
    return true;
}

bool MultiZonePIRSensor::setZonePriority(uint8_t zoneId, uint8_t priority) {
    auto it = findZone(zoneId);
    if (it == zones.end()) {
        return false;
    }

    it->priority = priority;
    Serial.printf("Zone %d priority set to %d\n", zoneId, priority);
    return true;
}

const MultiZonePIRSensor::PIRZone* MultiZonePIRSensor::getZone(uint8_t zoneId) const {
    auto it = findZone(zoneId);
    return (it != zones.end()) ? &(*it) : nullptr;
}

void MultiZonePIRSensor::resetStatistics() {
    for (auto& zone : zones) {
        zone.stats = {};
    }
    Serial.println("Multi-zone PIR statistics reset");
}

void MultiZonePIRSensor::cleanup() {
    if (initialized) {
        // Cleanup all PIR sensors
        for (auto& pir : pirSensors) {
            pir.cleanup();
        }
        
        pirSensors.clear();
        zones.clear();
        initialized = false;
        
        Serial.println("Multi-zone PIR sensor system cleaned up");
    }
}

bool MultiZonePIRSensor::configureDefaultZones() {
    if (!initialized) {
        return false;
    }

    // Clear existing zones
    cleanup();
    initialize();

    // Configure default zones for wildlife monitoring
    // Zone 1: Main trail/path (highest priority)
    addZone(1, SPARE_GPIO_1, "Main Trail", 0.7f, 0);
    
    // Zone 2: Secondary area (medium priority)  
    addZone(2, SPARE_GPIO_2, "Secondary Area", 0.5f, 128);
    
    // Zone 3: Background area (lowest priority)
    // Note: Using PIR_PIN for backwards compatibility
    addZone(3, PIR_PIN, "Background", 0.3f, 255);

    Serial.println("Default multi-zone PIR configuration applied");
    return true;
}

// Private methods

std::vector<MultiZonePIRSensor::PIRZone>::iterator 
MultiZonePIRSensor::findZone(uint8_t zoneId) {
    return std::find_if(zones.begin(), zones.end(), 
                        [zoneId](const PIRZone& zone) {
                            return zone.zoneId == zoneId;
                        });
}

std::vector<MultiZonePIRSensor::PIRZone>::const_iterator 
MultiZonePIRSensor::findZone(uint8_t zoneId) const {
    return std::find_if(zones.begin(), zones.end(), 
                        [zoneId](const PIRZone& zone) {
                            return zone.zoneId == zoneId;
                        });
}

bool MultiZonePIRSensor::initializePIRForZone(const PIRZone& zone) {
    // Create new PIR sensor instance
    PIRSensor pirSensor;
    
    // Note: In a real implementation, we'd need to modify PIRSensor 
    // to support configurable pins. For now, this is a placeholder.
    if (pirSensor.initialize()) {
        pirSensor.setSensitivity(zone.sensitivity);
        pirSensors.push_back(pirSensor);
        return true;
    }
    
    return false;
}

float MultiZonePIRSensor::calculateOverallConfidence(const std::vector<uint8_t>& activeZones) {
    if (activeZones.empty()) {
        return 0.0f;
    }

    float totalConfidence = 0.0f;
    float totalWeight = 0.0f;

    for (uint8_t zoneId : activeZones) {
        const auto* zone = getZone(zoneId);
        if (zone) {
            // Higher priority zones (lower priority number) get more weight
            float weight = 1.0f / (zone->priority + 1);
            totalConfidence += zone->sensitivity * weight;
            totalWeight += weight;
        }
    }

    return totalWeight > 0 ? totalConfidence / totalWeight : 0.0f;
}

void MultiZonePIRSensor::updateZoneStatistics(uint8_t zoneId, float confidence) {
    auto it = findZone(zoneId);
    if (it != zones.end()) {
        auto& stats = it->stats;
        stats.detectionCount++;
        stats.lastDetection = millis();
        stats.currentlyActive = true;
        
        // Update rolling average confidence
        if (stats.detectionCount == 1) {
            stats.averageConfidence = confidence;
        } else {
            stats.averageConfidence = (stats.averageConfidence * 0.9f) + (confidence * 0.1f);
        }
    }
}

String MultiZonePIRSensor::generateResultDescription(const MultiZoneResult& result) {
    if (!result.motionDetected) {
        return "No motion detected";
    }

    String desc = "Motion in " + String(result.activeZoneCount) + " zone(s): ";
    
    for (size_t i = 0; i < result.activeZones.size(); i++) {
        if (i > 0) desc += ", ";
        
        uint8_t zoneId = result.activeZones[i];
        const auto* zone = getZone(zoneId);
        desc += zone ? zone->name : ("Zone " + String(zoneId));
    }

    if (result.highestPriorityZone != 255) {
        const auto* priorityZone = getZone(result.highestPriorityZone);
        desc += " (Priority: " + (priorityZone ? priorityZone->name : String(result.highestPriorityZone)) + ")";
    }

    return desc;
}