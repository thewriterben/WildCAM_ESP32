/**
 * @file environmental_integration.cpp
 * @brief Environmental Integration for Wildlife Photography and Motion Detection
 * 
 * Integrates advanced environmental sensors with camera and motion systems
 * for intelligent wildlife monitoring.
 */

#include "advanced_environmental_sensors.h"
#include "../camera_handler.h"
#include "../motion_filter.h"
#include "../debug_utils.h"
#include <math.h>

// Global environmental sensors instance
static AdvancedEnvironmentalSensors* g_environmentalSensors = nullptr;

// ===========================
// INITIALIZATION
// ===========================

bool initializeEnvironmentalIntegration() {
    DEBUG_PRINTLN("Initializing environmental integration...");
    
    if (g_environmentalSensors != nullptr) {
        delete g_environmentalSensors;
    }
    
    g_environmentalSensors = new AdvancedEnvironmentalSensors();
    
    if (!g_environmentalSensors->init()) {
        DEBUG_PRINTLN("Failed to initialize environmental sensors");
        delete g_environmentalSensors;
        g_environmentalSensors = nullptr;
        return false;
    }
    
    DEBUG_PRINTLN("Environmental integration initialized successfully");
    return true;
}

void cleanupEnvironmentalIntegration() {
    if (g_environmentalSensors != nullptr) {
        g_environmentalSensors->cleanup();
        delete g_environmentalSensors;
        g_environmentalSensors = nullptr;
    }
}

// ===========================
// CAMERA INTEGRATION
// ===========================

/**
 * Environmentally-aware camera capture with conditions assessment
 */
bool environmentallyAwareCameraCapture() {
    if (g_environmentalSensors == nullptr) {
        DEBUG_PRINTLN("Environmental sensors not initialized - using standard capture");
        return CameraHandler::captureImage(); // Fallback to standard capture
    }
    
    // Get latest environmental data
    g_environmentalSensors->readAllSensors();
    AdvancedEnvironmentalData env_data = g_environmentalSensors->getLatestData();
    
    DEBUG_PRINTF("Photography conditions: %d%%, Wildlife activity: %d%%\n", 
                env_data.photography_conditions, env_data.wildlife_activity_index);
    
    if (env_data.photography_conditions > 70) {
        // Optimal conditions - configure for highest quality
        DEBUG_PRINTLN("Optimal photography conditions detected");
        
        // Configure camera settings based on environmental conditions
        camera_config_t camera_config;
        // Get current config (implementation would depend on camera_handler structure)
        
        // Adjust settings based on light conditions
        if (env_data.visible_light < 50.0) {
            // Low light conditions - increase exposure
            DEBUG_PRINTLN("Low light detected - adjusting exposure");
            // camera_config.aec_value = 600; // Longer exposure
        } else if (env_data.visible_light > 1000.0) {
            // Bright conditions - reduce exposure
            DEBUG_PRINTLN("Bright light detected - reducing exposure");
            // camera_config.aec_value = 200; // Shorter exposure
        }
        
        // Adjust for humidity (prevent condensation issues)
        if (env_data.humidity > 90.0) {
            DEBUG_PRINTLN("High humidity detected - using protective settings");
            // Additional processing to prevent condensation artifacts
        }
        
        return CameraHandler::captureImage();
        
    } else if (env_data.photography_conditions > 40) {
        // Acceptable conditions - standard capture with metadata
        DEBUG_PRINTLN("Acceptable photography conditions");
        
        // Capture with environmental metadata
        bool capture_result = CameraHandler::captureImage();
        
        if (capture_result) {
            // Add environmental data to image metadata (if supported)
            // This would require extending the camera handler to include EXIF data
            DEBUG_PRINTF("Image captured with env data: T=%.1f°C, H=%.1f%%, P=%.1fhPa\n",
                        env_data.temperature, env_data.humidity, env_data.pressure);
        }
        
        return capture_result;
        
    } else {
        // Poor conditions - postpone capture
        DEBUG_PRINTF("Poor photography conditions (%d%%) - postponing capture\n", 
                    env_data.photography_conditions);
        
        // Log the reason for postponement
        if (env_data.humidity > 95.0) {
            DEBUG_PRINTLN("Reason: High condensation risk");
        }
        if (env_data.visible_light < 10.0) {
            DEBUG_PRINTLN("Reason: Insufficient light");
        }
        if (env_data.visible_light > 5000.0) {
            DEBUG_PRINTLN("Reason: Excessive brightness");
        }
        
        return false;
    }
}

/**
 * Assess photography conditions based on environmental data
 */
uint8_t assessPhotographyConditions(const AdvancedEnvironmentalData& data) {
    uint8_t conditions = 50; // Base conditions
    
    // Light optimization (100-1000 lux is optimal)
    if (data.visible_light >= 100.0 && data.visible_light <= 1000.0) {
        conditions += 30; // Optimal lighting
    } else if (data.visible_light >= 50.0 && data.visible_light < 100.0) {
        conditions += 15; // Acceptable low light
    } else if (data.visible_light > 1000.0 && data.visible_light <= 2000.0) {
        conditions += 10; // Acceptable bright light
    } else if (data.visible_light < 10.0) {
        conditions = (conditions > 25) ? conditions - 25 : 0; // Too dark
    } else if (data.visible_light > 5000.0) {
        conditions = (conditions > 20) ? conditions - 20 : 0; // Too bright
    }
    
    // Humidity management (condensation risk >95%)
    if (data.humidity <= 80.0) {
        conditions += 15; // Good humidity
    } else if (data.humidity <= 90.0) {
        conditions += 5; // Acceptable humidity
    } else if (data.humidity > 95.0) {
        conditions = (conditions > 30) ? conditions - 30 : 0; // High condensation risk
    }
    
    // Temperature stability (camera operating range)
    if (data.temperature >= 0.0 && data.temperature <= 40.0) {
        conditions += 10; // Good operating range
    } else if (data.temperature < -10.0 || data.temperature > 50.0) {
        conditions = (conditions > 15) ? conditions - 15 : 0; // Outside safe range
    }
    
    // Weather stability
    if (data.pressure > 1005.0 && data.pressure < 1025.0) {
        conditions += 5; // Stable weather
    }
    
    return constrain(conditions, 0, 100);
}

// ===========================
// MOTION DETECTION INTEGRATION
// ===========================

/**
 * Environmental filtering for motion detection
 */
bool shouldFilterMotionByEnvironment(const AdvancedEnvironmentalData& data) {
    // Filter motion in extreme weather conditions
    if (data.temperature > 35.0 || data.temperature < -5.0) {
        DEBUG_PRINTLN("Filtering motion due to extreme temperature");
        return true; // High thermal noise expected
    }
    
    // Filter during high wind conditions (if wind data available)
    // This would require wind sensor integration
    
    // Filter during high humidity (potential for false positives)
    if (data.humidity > 98.0) {
        DEBUG_PRINTLN("Filtering motion due to very high humidity");
        return true; // Potential condensation artifacts
    }
    
    // Filter during very low air quality (dust/particles)
    if (data.tvoc_ppb > 30000 || data.eco2_ppm > 5000) {
        DEBUG_PRINTLN("Filtering motion due to poor air quality");
        return true; // High particulate matter
    }
    
    return false; // Don't filter
}

/**
 * Get environmental motion threshold adjustment
 */
float getEnvironmentalMotionThreshold(const AdvancedEnvironmentalData& data) {
    float base_threshold = 1.0;
    float adjustment = 1.0;
    
    // Adjust threshold based on temperature (thermal noise)
    if (data.temperature > 30.0) {
        adjustment *= 1.2; // Increase threshold for hot weather
    } else if (data.temperature < 5.0) {
        adjustment *= 1.1; // Slightly increase for cold weather
    }
    
    // Adjust for light conditions
    if (data.visible_light < 50.0) {
        adjustment *= 0.9; // Lower threshold for low light (more sensitive)
    } else if (data.visible_light > 2000.0) {
        adjustment *= 1.1; // Higher threshold for bright light
    }
    
    // Adjust for humidity
    if (data.humidity > 85.0) {
        adjustment *= 1.15; // Increase threshold for high humidity
    }
    
    // Adjust for wildlife activity prediction
    if (data.wildlife_activity_index > 70) {
        adjustment *= 0.85; // Lower threshold during high activity periods
    } else if (data.wildlife_activity_index < 30) {
        adjustment *= 1.2; // Higher threshold during low activity periods
    }
    
    return base_threshold * adjustment;
}

/**
 * Enhanced motion validation with environmental context
 */
bool validateMotionWithEnvironment(bool motion_detected, uint16_t motion_confidence) {
    if (!motion_detected || g_environmentalSensors == nullptr) {
        return motion_detected;
    }
    
    // Get current environmental data
    AdvancedEnvironmentalData env_data = g_environmentalSensors->getLatestData();
    
    // Check if motion should be filtered by environment
    if (shouldFilterMotionByEnvironment(env_data)) {
        DEBUG_PRINTLN("Motion filtered by environmental conditions");
        return false;
    }
    
    // Adjust confidence threshold based on environmental factors
    float env_threshold = getEnvironmentalMotionThreshold(env_data);
    uint16_t adjusted_confidence_threshold = (uint16_t)(50 * env_threshold);
    
    if (motion_confidence < adjusted_confidence_threshold) {
        DEBUG_PRINTF("Motion confidence %d below environmental threshold %d\n", 
                    motion_confidence, adjusted_confidence_threshold);
        return false;
    }
    
    // Additional validation during optimal wildlife activity times
    if (env_data.wildlife_activity_index > 80) {
        DEBUG_PRINTLN("High wildlife activity - motion validation enhanced");
        // During high activity periods, be more permissive
        return motion_confidence > 30;
    }
    
    return true;
}

// ===========================
// POWER OPTIMIZATION INTEGRATION
// ===========================

/**
 * Environmental power optimization
 */
bool optimizePowerForEnvironment() {
    if (g_environmentalSensors == nullptr) {
        return false;
    }
    
    AdvancedEnvironmentalData env_data = g_environmentalSensors->getLatestData();
    
    // During low wildlife activity, reduce sensor polling
    if (env_data.wildlife_activity_index < 30) {
        DEBUG_PRINTLN("Low wildlife activity - reducing sensor polling");
        SensorConfig config = g_environmentalSensors->getConfig();
        config.fast_interval *= 2;    // Double the intervals
        config.medium_interval *= 2;
        config.slow_interval *= 2;
        g_environmentalSensors->setConfig(config);
        return true;
    }
    
    // During extreme weather, enter low power mode
    if (env_data.temperature < -10.0 || env_data.temperature > 40.0) {
        DEBUG_PRINTLN("Extreme temperature - entering environmental low power mode");
        g_environmentalSensors->enterLowPowerMode();
        return true;
    }
    
    // During low battery, optimize environmental monitoring
    if (env_data.battery_percentage < 20.0) {
        DEBUG_PRINTLN("Low battery - optimizing environmental monitoring");
        SensorConfig config = g_environmentalSensors->getConfig();
        config.enable_sgp30 = false;     // Disable air quality (high power)
        config.fast_interval = 30000;    // Reduce to 30 seconds
        g_environmentalSensors->setConfig(config);
        return true;
    }
    
    return false;
}

// ===========================
// DATA ACCESS FUNCTIONS
// ===========================

/**
 * Get latest environmental data for telemetry
 */
AdvancedEnvironmentalData getLatestEnvironmentalData() {
    if (g_environmentalSensors == nullptr) {
        // Return default/invalid data
        AdvancedEnvironmentalData empty_data;
        memset(&empty_data, 0, sizeof(AdvancedEnvironmentalData));
        return empty_data;
    }
    
    // Ensure we have fresh data
    g_environmentalSensors->readAllSensors();
    return g_environmentalSensors->getLatestData();
}

/**
 * Check if environmental sensors are healthy
 */
bool areEnvironmentalSensorsHealthy() {
    if (g_environmentalSensors == nullptr) {
        return false;
    }
    
    return g_environmentalSensors->isDataValid() && 
           (g_environmentalSensors->getSensorErrors() == 0);
}

/**
 * Get environmental sensor status string
 */
String getEnvironmentalSensorStatus() {
    if (g_environmentalSensors == nullptr) {
        return "Not initialized";
    }
    
    uint32_t errors = g_environmentalSensors->getSensorErrors();
    if (errors == 0) {
        return "All sensors healthy";
    }
    
    String status = "Errors: ";
    if (errors & SENSOR_ERROR_BME280) status += "BME280 ";
    if (errors & SENSOR_ERROR_TSL2591) status += "TSL2591 ";
    if (errors & SENSOR_ERROR_SGP30) status += "SGP30 ";
    if (errors & SENSOR_ERROR_DS18B20) status += "DS18B20 ";
    if (errors & SENSOR_ERROR_MAX17048) status += "MAX17048 ";
    
    return status;
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

/**
 * Process environmental data for regular updates
 */
void processEnvironmentalData() {
    if (g_environmentalSensors != nullptr) {
        g_environmentalSensors->readAllSensors();
        
        // Perform power optimization check
        static uint32_t last_optimization = 0;
        if (millis() - last_optimization > 60000) { // Check every minute
            optimizePowerForEnvironment();
            last_optimization = millis();
        }
    }
}

/**
 * Environmental system diagnostics
 */
void performEnvironmentalDiagnostics() {
    if (g_environmentalSensors == nullptr) {
        DEBUG_PRINTLN("Environmental sensors not initialized");
        return;
    }
    
    DEBUG_PRINTLN("=== Environmental System Diagnostics ===");
    
    AdvancedEnvironmentalData data = g_environmentalSensors->getLatestData();
    
    DEBUG_PRINTF("Temperature: %.2f°C (BME280: %s)\n", 
                data.temperature, data.bme280_valid ? "OK" : "FAIL");
    DEBUG_PRINTF("Humidity: %.1f%% RH\n", data.humidity);
    DEBUG_PRINTF("Pressure: %.2f hPa\n", data.pressure);
    DEBUG_PRINTF("Light: %.1f lux (visible), %.1f (IR)\n", 
                data.visible_light, data.infrared_light);
    DEBUG_PRINTF("Air Quality: %d ppb TVOC, %d ppm eCO2\n", 
                data.tvoc_ppb, data.eco2_ppm);
    DEBUG_PRINTF("Battery: %.2fV (%.1f%%)\n", 
                data.battery_voltage, data.battery_percentage);
    DEBUG_PRINTF("Wildlife Activity: %d%%\n", data.wildlife_activity_index);
    DEBUG_PRINTF("Photography Conditions: %d%%\n", data.photography_conditions);
    
    DEBUG_PRINTF("Sensor Errors: 0x%08X\n", g_environmentalSensors->getSensorErrors());
    DEBUG_PRINTLN("========================================");
}