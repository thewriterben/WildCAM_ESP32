/**
 * @file power_management.h
 * @brief Power management interface for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides power management functionality including battery monitoring,
 * solar charging, and power saving modes.
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>

/**
 * @brief Initialize power management system
 * @return true if successful, false otherwise
 */
bool initializePowerManagement();

/**
 * @brief Get current battery voltage
 * @return Battery voltage in volts
 */
float getBatteryVoltage();

/**
 * @brief Get current solar panel voltage
 * @return Solar voltage in volts
 */
float getSolarVoltage();

/**
 * @brief Check if battery is currently charging
 * @return true if charging, false otherwise
 */
bool isCharging();

/**
 * @brief Enter deep sleep mode
 * @param time_in_us Time to sleep in microseconds
 */
void enterDeepSleep(uint64_t time_in_us);

/**
 * @brief Enable power saving mode
 */
void enablePowerSaving();

/**
 * @brief Disable power saving mode
 */
void disablePowerSaving();

#endif // POWER_MANAGEMENT_H
