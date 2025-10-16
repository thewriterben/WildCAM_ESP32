/**
 * @file time_manager.h
 * @brief Time management utilities for WildCAM ESP32
 * 
 * Provides time synchronization and formatting functions for the
 * wildlife camera system.
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

/**
 * @brief Initialize the time manager
 * @return true if initialization successful, false otherwise
 */
bool initializeTimeManager();

/**
 * @brief Get the current system time
 * @return Current time as Unix timestamp (seconds since epoch)
 */
time_t getCurrentTime();

/**
 * @brief Synchronize time with RTC (Real-Time Clock)
 * @return true if synchronization successful, false otherwise
 */
bool syncWithRTC();

/**
 * @brief Synchronize time with NTP (Network Time Protocol) server
 * @return true if synchronization successful, false otherwise
 */
bool syncWithNTP();

/**
 * @brief Get formatted time string
 * @return Formatted time string (e.g., "YYYY-MM-DD HH:MM:SS")
 */
String getFormattedTime();

/**
 * @brief Set timezone offset
 * @param offset Timezone offset in hours from UTC (e.g., -5 for EST, +1 for CET)
 */
void setTimezone(int offset);

#endif // TIME_MANAGER_H
