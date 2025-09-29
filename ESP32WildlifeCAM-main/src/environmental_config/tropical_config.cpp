/**
 * @file tropical_config.cpp
 * @brief Tropical Climate Configuration Implementation
 * 
 * Implementation of tropical climate management for ESP32 Wildlife Camera
 */

#include "tropical_config.h"
#include "../debug_utils.h"
#include <math.h>

// Global tropical manager instance
TropicalClimateManager g_tropicalManager;

bool TropicalClimateManager::initialize() {
    DEBUG_PRINTLN("Initializing tropical climate manager...");
    
    // Initialize GPIO pins for tropical hardware
    pinMode(TROPICAL_VENTILATION_PIN, OUTPUT);
    pinMode(TROPICAL_LENS_HEATER_PIN, OUTPUT);
    pinMode(TROPICAL_HUMIDITY_SENSOR_PIN, INPUT);
    pinMode(TROPICAL_DRAINAGE_VALVE_PIN, OUTPUT);
    
    // Initial states
    digitalWrite(TROPICAL_VENTILATION_PIN, LOW);
    digitalWrite(TROPICAL_LENS_HEATER_PIN, LOW);
    digitalWrite(TROPICAL_DRAINAGE_VALVE_PIN, LOW);
    
    // Initialize sensors and systems
    // Note: This would typically initialize actual hardware sensors
    
    initialized_ = true;
    last_mold_check_ = millis();
    last_corrosion_check_ = millis();
    
    DEBUG_PRINTLN("Tropical climate manager initialized successfully");
    return true;
}

void TropicalClimateManager::update() {
    if (!initialized_) {
        return;
    }
    
    uint32_t current_time = millis();
    
    // Check for mold prevention needs
    if (current_time - last_mold_check_ > config_.mold_check_interval) {
        if (shouldActivateMoldPrevention()) {
            performMoldPrevention();
        }
        last_mold_check_ = current_time;
    }
    
    // Check for corrosion monitoring
    if (current_time - last_corrosion_check_ > config_.corrosion_check_interval) {
        if (detectCorrosionRisk()) {
            performCorrosionPrevention();
        }
        last_corrosion_check_ = current_time;
    }
    
    // Handle active ventilation timing
    if (ventilation_active_ && 
        current_time - ventilation_start_time_ > config_.ventilation_duration) {
        controlVentilationFan(false);
        ventilation_active_ = false;
        DEBUG_PRINTLN("Ventilation cycle completed");
    }
    
    // Monitor lens heating needs
    float temp_differential = getCurrentTemperature() - getCurrentHumidity() / 4.0; // Estimate dew point
    if (config_.lens_heating_enabled && temp_differential < config_.lens_heating_threshold) {
        if (!lens_heating_active_) {
            activateLensHeating(true);
        }
    } else if (lens_heating_active_) {
        activateLensHeating(false);
    }
    
    // Check for emergency conditions
    if (detectFloodConditions()) {
        handleEmergency("flood_detected");
    }
    
    float current_temp = getCurrentTemperature();
    if (current_temp > config_.thermal_reduction_threshold) {
        handleEmergency("thermal_stress");
    }
    
    // Log environmental data periodically
    static uint32_t last_log_time = 0;
    if (current_time - last_log_time > 300000) { // Log every 5 minutes
        logEnvironmentalData();
        last_log_time = current_time;
    }
}

void TropicalClimateManager::setConfig(const TropicalConfig& config) {
    config_ = config;
    DEBUG_PRINTLN("Tropical configuration updated");
}

bool TropicalClimateManager::shouldActivateMoldPrevention() const {
    float humidity = getCurrentHumidity();
    float temperature = getCurrentTemperature();
    
    // High humidity and warm temperature create mold risk
    bool high_humidity_risk = humidity > config_.humidity_threshold;
    bool temperature_risk = temperature > 25.0 && temperature < 35.0; // Optimal mold range
    
    return high_humidity_risk && temperature_risk;
}

void TropicalClimateManager::activateVentilation(uint32_t duration_ms) {
    uint32_t duration = duration_ms > 0 ? duration_ms : config_.ventilation_duration;
    
    controlVentilationFan(true);
    ventilation_active_ = true;
    ventilation_start_time_ = millis();
    
    DEBUG_PRINTF("Ventilation activated for %d seconds\n", duration / 1000);
}

bool TropicalClimateManager::checkDrainageSystem() {
    // This would check actual drainage sensors/flow meters
    // For now, return true indicating drainage is functional
    
    // Simulate drainage check
    bool drainage_clear = true;
    bool valve_operational = true;
    bool pressure_normal = true;
    
    if (!drainage_clear || !valve_operational || !pressure_normal) {
        DEBUG_PRINTLN("Drainage system issue detected");
        return false;
    }
    
    return true;
}

void TropicalClimateManager::activateLensHeating(bool enable) {
    digitalWrite(TROPICAL_LENS_HEATER_PIN, enable ? HIGH : LOW);
    lens_heating_active_ = enable;
    
    if (enable) {
        DEBUG_PRINTLN("Lens heating activated");
    } else {
        DEBUG_PRINTLN("Lens heating deactivated");
    }
}

uint8_t TropicalClimateManager::getEnvironmentalHealth() const {
    uint8_t health_score = 100;
    
    // Reduce score based on various risk factors
    float humidity = getCurrentHumidity();
    if (humidity > 95.0) health_score -= 20;
    else if (humidity > 90.0) health_score -= 10;
    
    float temperature = getCurrentTemperature();
    if (temperature > 40.0) health_score -= 15;
    else if (temperature > 38.0) health_score -= 8;
    
    if (detectMoldRisk()) health_score -= 25;
    if (detectCorrosionRisk()) health_score -= 15;
    if (!checkDrainageSystem()) health_score -= 20;
    
    return max(0, (int)health_score);
}

void TropicalClimateManager::handleEmergency(const String& emergency_type) {
    DEBUG_PRINTF("Tropical emergency detected: %s\n", emergency_type.c_str());
    
    if (emergency_type == "flood_detected") {
        // Open all drainage valves
        controlDrainageValve(true);
        // Activate emergency ventilation
        activateVentilation(1800000); // 30 minutes
        // Send alert
        sendEnvironmentalAlert("FLOOD", "Water ingress detected - emergency drainage activated");
        
    } else if (emergency_type == "thermal_stress") {
        // Reduce power consumption
        // Activate cooling measures
        activateVentilation(600000); // 10 minutes
        sendEnvironmentalAlert("THERMAL", "High temperature detected - thermal protection activated");
        
    } else if (emergency_type == "mold_outbreak") {
        // Aggressive mold prevention
        activateVentilation(3600000); // 1 hour
        if (config_.uv_sterilization_enabled) {
            // Activate UV-C sterilization
        }
        sendEnvironmentalAlert("MOLD", "Mold risk detected - prevention measures activated");
        
    } else if (emergency_type == "corrosion_critical") {
        // Alert for maintenance needed
        sendEnvironmentalAlert("CORROSION", "Critical corrosion detected - immediate maintenance required");
    }
}

// Private helper functions

float TropicalClimateManager::getCurrentHumidity() const {
    // In a real implementation, this would read from actual humidity sensor
    // For now, simulate typical tropical humidity
    return 85.0 + (random(150) / 10.0); // 85-100% range
}

float TropicalClimateManager::getCurrentTemperature() const {
    // In a real implementation, this would read from actual temperature sensor
    // Simulate tropical temperature range
    return 22.0 + (random(200) / 10.0); // 22-42°C range
}

float TropicalClimateManager::getPrecipitationRate() const {
    // Simulate precipitation sensor reading
    return random(500) / 10.0; // 0-50 mm/hr range
}

bool TropicalClimateManager::detectMoldRisk() const {
    float humidity = getCurrentHumidity();
    float temperature = getCurrentTemperature();
    
    // Mold growth risk assessment
    bool high_humidity = humidity > 85.0;
    bool optimal_temp = temperature > 20.0 && temperature < 35.0;
    bool poor_ventilation = !ventilation_active_;
    
    return high_humidity && optimal_temp && poor_ventilation;
}

bool TropicalClimateManager::detectCorrosionRisk() const {
    float humidity = getCurrentHumidity();
    float temperature = getCurrentTemperature();
    
    // Simplified corrosion risk model
    bool high_moisture = humidity > 80.0;
    bool warm_conditions = temperature > 25.0;
    
    return high_moisture && warm_conditions;
}

bool TropicalClimateManager::detectFloodConditions() const {
    // This would read from actual water level sensors
    // For simulation, randomly detect flood conditions
    return random(1000) < 5; // 0.5% chance per check
}

void TropicalClimateManager::controlVentilationFan(bool enable) {
    digitalWrite(TROPICAL_VENTILATION_PIN, enable ? HIGH : LOW);
    
    if (enable) {
        DEBUG_PRINTLN("Ventilation fan activated");
    } else {
        DEBUG_PRINTLN("Ventilation fan deactivated");
    }
}

void TropicalClimateManager::controlLensHeater(bool enable) {
    digitalWrite(TROPICAL_LENS_HEATER_PIN, enable ? HIGH : LOW);
}

void TropicalClimateManager::controlDrainageValve(bool open) {
    digitalWrite(TROPICAL_DRAINAGE_VALVE_PIN, open ? HIGH : LOW);
    
    if (open) {
        DEBUG_PRINTLN("Emergency drainage valve opened");
    } else {
        DEBUG_PRINTLN("Emergency drainage valve closed");
    }
}

void TropicalClimateManager::performMoldPrevention() {
    DEBUG_PRINTLN("Performing mold prevention cycle");
    
    // Activate ventilation
    activateVentilation();
    
    // If available, activate UV-C sterilization
    if (config_.uv_sterilization_enabled) {
        // UV-C activation code would go here
        DEBUG_PRINTLN("UV-C sterilization activated");
    }
    
    // Log the prevention action
    logEnvironmentalData();
}

void TropicalClimateManager::performCorrosionPrevention() {
    DEBUG_PRINTLN("Performing corrosion prevention check");
    
    // This would typically involve checking electrical resistance of components
    // or other corrosion detection methods
    
    // For now, just log the check
    logEnvironmentalData();
}

void TropicalClimateManager::logEnvironmentalData() {
    float temp = getCurrentTemperature();
    float humidity = getCurrentHumidity();
    float precipitation = getPrecipitationRate();
    uint8_t health = getEnvironmentalHealth();
    
    DEBUG_PRINTF("Environmental Status - Temp: %.1f°C, Humidity: %.1f%%, Precipitation: %.1fmm/hr, Health: %d%%\n",
                 temp, humidity, precipitation, health);
}

void TropicalClimateManager::sendEnvironmentalAlert(const String& alert_type, const String& message) {
    // This would send alerts via configured communication method
    // For now, just log the alert
    DEBUG_PRINTF("ALERT [%s]: %s\n", alert_type.c_str(), message.c_str());
}

// Global function implementations

bool initializeTropicalConfig() {
    return g_tropicalManager.initialize();
}

void updateTropicalManagement() {
    g_tropicalManager.update();
}

String getTropicalDeploymentRecommendations(const String& location) {
    String recommendations = "=== TROPICAL DEPLOYMENT RECOMMENDATIONS ===\n\n";
    
    recommendations += "Location: " + location + "\n\n";
    
    recommendations += "HARDWARE REQUIREMENTS:\n";
    recommendations += "- Enhanced IP68+ waterproof enclosure\n";
    recommendations += "- Active ventilation system with filters\n";
    recommendations += "- Marine-grade stainless steel hardware\n";
    recommendations += "- Hydrophobic lens coating\n";
    recommendations += "- Anti-fungal treated materials\n\n";
    
    recommendations += "ENVIRONMENTAL PREPARATIONS:\n";
    recommendations += "- Install drainage channels around mounting area\n";
    recommendations += "- Apply corrosion-resistant coatings\n";
    recommendations += "- Position for partial canopy protection\n";
    recommendations += "- Ensure access during dry season\n\n";
    
    recommendations += "MAINTENANCE SCHEDULE:\n";
    recommendations += "- Weekly: Remote monitoring check\n";
    recommendations += "- Monthly: Physical inspection and cleaning\n";
    recommendations += "- Quarterly: Filter replacement and deep clean\n";
    recommendations += "- Annually: Complete system overhaul\n\n";
    
    recommendations += "SPECIES OPTIMIZATION:\n";
    recommendations += "- Enable nocturnal mode for night-active species\n";
    recommendations += "- Adjust motion sensitivity for vegetation movement\n";
    recommendations += "- Configure crepuscular enhancement\n";
    recommendations += "- Set up seasonal migration tracking\n\n";
    
    return recommendations;
}

String generateTropicalMaintenanceSchedule(uint32_t deployment_duration) {
    String schedule = "=== TROPICAL MAINTENANCE SCHEDULE ===\n\n";
    
    schedule += "Deployment Duration: " + String(deployment_duration) + " days\n\n";
    
    // Calculate maintenance intervals
    uint32_t weekly_checks = deployment_duration / 7;
    uint32_t monthly_visits = deployment_duration / 30;
    uint32_t quarterly_maintenance = deployment_duration / 90;
    
    schedule += "SCHEDULED MAINTENANCE:\n";
    schedule += "- Weekly Checks: " + String(weekly_checks) + " remote monitoring sessions\n";
    schedule += "- Monthly Visits: " + String(monthly_visits) + " field maintenance visits\n";
    schedule += "- Quarterly Service: " + String(quarterly_maintenance) + " comprehensive service\n\n";
    
    schedule += "WEEKLY CHECKLIST (Remote):\n";
    schedule += "□ System health status\n";
    schedule += "□ Battery level and charging\n";
    schedule += "□ Image quality assessment\n";
    schedule += "□ Environmental sensor readings\n";
    schedule += "□ Communication link status\n\n";
    
    schedule += "MONTHLY CHECKLIST (Field Visit):\n";
    schedule += "□ Visual inspection of enclosure\n";
    schedule += "□ Clear drainage channels\n";
    schedule += "□ Clean lens and protective surfaces\n";
    schedule += "□ Check mounting system integrity\n";
    schedule += "□ Replace ventilation filters\n";
    schedule += "□ Apply anti-fungal treatment\n\n";
    
    schedule += "QUARTERLY CHECKLIST (Comprehensive):\n";
    schedule += "□ Complete disassembly and inspection\n";
    schedule += "□ Replace all gaskets and seals\n";
    schedule += "□ Hardware corrosion assessment\n";
    schedule += "□ Software updates and calibration\n";
    schedule += "□ Pressure testing verification\n";
    schedule += "□ Documentation and reporting\n\n";
    
    return schedule;
}