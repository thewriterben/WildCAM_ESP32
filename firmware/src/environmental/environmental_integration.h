/**
 * @file environmental_integration.h
 * @brief Environmental Integration for Wildlife Photography and Motion Detection
 * 
 * Integrates advanced environmental sensors with camera and motion systems
 * for intelligent wildlife monitoring with environmental awareness.
 * 
 * Features:
 * - Environmental data processing and management
 * - Motion detection filtering based on environmental conditions
 * - Photography condition assessment
 * - Sensor health monitoring and diagnostics
 * - Wildlife activity prediction
 */

#ifndef ENVIRONMENTAL_INTEGRATION_H
#define ENVIRONMENTAL_INTEGRATION_H

#include <Arduino.h>
#include <stdint.h>

// Forward declaration of AdvancedEnvironmentalData structure
struct AdvancedEnvironmentalData;

// ===========================
// INITIALIZATION AND MANAGEMENT
// ===========================

/**
 * @brief Initialize the environmental integration subsystem
 * 
 * Initializes the environmental sensors and integration logic for wildlife monitoring.
 * Sets up sensor configurations, calibration, and data collection infrastructure.
 * 
 * @return true if initialization successful, false otherwise
 */
bool initializeEnvironmentalIntegration();

/**
 * @brief Process environmental data for regular updates
 * 
 * Performs periodic environmental data collection, processing, and optimization.
 * Should be called regularly in the main loop to keep environmental data fresh.
 * Includes power optimization checks and adaptive monitoring.
 */
void processEnvironmentalData();

/**
 * @brief Perform comprehensive environmental system diagnostics
 * 
 * Executes a full diagnostic check of all environmental sensors and subsystems.
 * Prints detailed status information for debugging and system health monitoring.
 * Includes sensor validation, error checking, and performance metrics.
 */
void performEnvironmentalDiagnostics();

// ===========================
// DATA ACCESS
// ===========================

/**
 * @brief Get the latest environmental data readings
 * 
 * Retrieves the most recent environmental sensor data including temperature,
 * humidity, pressure, light levels, air quality, and derived metrics.
 * Ensures fresh data by triggering a sensor read if needed.
 * 
 * @return AdvancedEnvironmentalData structure containing all environmental readings
 */
AdvancedEnvironmentalData getLatestEnvironmentalData();

/**
 * @brief Check if environmental sensors are healthy
 * 
 * Validates the health status of all environmental sensors.
 * Checks for communication errors, out-of-range values, and sensor failures.
 * 
 * @return true if all sensors are functioning properly, false if any errors detected
 */
bool areEnvironmentalSensorsHealthy();

// ===========================
// MOTION DETECTION INTEGRATION
// ===========================

/**
 * @brief Determine if motion should be filtered based on environmental conditions
 * 
 * Analyzes current environmental conditions to determine if motion detection
 * should be filtered or suppressed. Filters motion during extreme weather,
 * high humidity (condensation), or poor air quality conditions.
 * 
 * @param data Current environmental data for analysis
 * @return true if motion should be filtered, false if motion detection is reliable
 */
bool shouldFilterMotionByEnvironment(const AdvancedEnvironmentalData& data);

/**
 * @brief Get environmental motion detection threshold adjustment
 * 
 * Calculates a dynamic motion detection threshold based on environmental factors
 * such as temperature, light levels, humidity, and wildlife activity prediction.
 * Returns a multiplier to adjust the base motion threshold.
 * 
 * @param data Current environmental data for threshold calculation
 * @return Threshold adjustment multiplier (1.0 = no adjustment, >1.0 = higher threshold)
 */
float getEnvironmentalMotionThreshold(const AdvancedEnvironmentalData& data);

/**
 * @brief Validate motion detection with environmental context
 * 
 * Enhanced motion validation that incorporates environmental factors and conditions.
 * Filters false positives caused by environmental effects and adjusts confidence
 * thresholds based on wildlife activity predictions and environmental stability.
 * 
 * @param motion_detected Raw motion detection result from motion sensor
 * @param motion_confidence Confidence level of motion detection (0-100)
 * @return true if motion is validated considering environmental context, false otherwise
 */
bool validateMotionWithEnvironment(bool motion_detected, uint16_t motion_confidence);

#endif // ENVIRONMENTAL_INTEGRATION_H
