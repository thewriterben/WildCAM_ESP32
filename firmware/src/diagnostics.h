/**
 * @file diagnostics.h
 * @brief System diagnostics for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides comprehensive hardware diagnostics at boot time to verify
 * all critical systems are operational before field deployment.
 */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <Arduino.h>

/**
 * @brief Run comprehensive system diagnostics
 * 
 * Tests all critical hardware components and logs results:
 * - Camera initialization
 * - SD card read/write operations
 * - PIR sensor
 * - BME280 sensor (if enabled)
 * - RTC (if enabled)
 * - GPS (if enabled)
 * - Power management system
 * - Available RAM and flash memory
 * 
 * @return true if all critical systems pass, false otherwise
 */
bool runSystemDiagnostics();

/**
 * @brief Test camera initialization
 * @return true if camera is detected and can capture a frame
 */
bool testCameraInit();

/**
 * @brief Test SD card read/write operations
 * @return true if SD card can be mounted, written to, and read from
 */
bool testSDCardReadWrite();

/**
 * @brief Test PIR sensor reading
 * @return true if PIR sensor responds logically
 */
bool testPIRSensor();

/**
 * @brief Test BME280 sensor (if enabled)
 * @return true if sensor reads valid temperature, humidity, pressure
 */
bool testBME280Sensor();

/**
 * @brief Test RTC (if enabled)
 * @return true if RTC reads valid time value
 */
bool testRTC();

/**
 * @brief Test GPS (if enabled)
 * @return true if GPS reads fix status and coordinates
 */
bool testGPS();

/**
 * @brief Test power management system
 * @return true if battery voltage and charge state can be read
 */
bool testPowerManagement();

/**
 * @brief Check available RAM and flash memory
 * @return true if memory check completes successfully
 */
bool testMemory();

#endif // DIAGNOSTICS_H
