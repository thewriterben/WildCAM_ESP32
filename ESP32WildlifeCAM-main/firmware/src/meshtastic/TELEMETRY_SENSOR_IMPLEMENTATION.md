# Wildlife Telemetry Sensor Implementation

## Overview
This document describes the actual sensor implementations in the Wildlife Telemetry system, replacing the previous placeholder values with real hardware and software integrations.

## Implemented Sensors

### 1. Power Management
**Location:** `collectPowerStatus()` function (line ~701)

**Implementation:**
- Reads battery voltage from `SolarManager::getBatteryVoltage()`
- Reads solar panel voltage from `SolarManager::getSolarVoltage()`
- Calculates battery percentage using linear interpolation between voltage thresholds:
  - Full: 4.2V (100%)
  - Critical: 2.8V (0%)
- Determines charging status based on solar voltage threshold (4.0V minimum)

**Error Handling:**
- Null check for SolarManager instance
- Try-catch block for voltage reading failures
- Fallback to safe default values (3.7V battery, 0.0V solar)
- Increments global error counter on failures

**Configuration Constants (from config.h):**
- `BATTERY_FULL_VOLTAGE`: 4.2V
- `BATTERY_CRITICAL_THRESHOLD`: 2.8V
- `SOLAR_CHARGING_VOLTAGE_MIN`: 4.0V

### 2. CPU Temperature
**Location:** `collectDeviceHealth()` function (line ~780)

**Implementation:**
- Uses ESP32's built-in `temperatureRead()` function
- Returns temperature in Celsius
- Available on ESP32-S3 and newer chips
- Falls back to 45.0°C default if function not available

**Error Handling:**
- Platform detection using `#if defined(ESP32)`
- Default fallback value for unsupported platforms

### 3. LoRa Signal Strength
**Location:** `collectDeviceHealth()` function (line ~798)

**Implementation:**
- Checks `LORA_ENABLED` compile-time flag
- Reads RSSI from LoRa driver using `getRssi()`
- Converts RSSI to 0-100 scale:
  - Assumes typical range: -120 to -30 dBm
  - Formula: `(rssi + 120) * 100 / 90`
  - Clamps result to 0-100 range
- Returns 0 if LoRa disabled or driver unavailable

**Error Handling:**
- Compile-time check for LORA_ENABLED
- Null check for global LoRa driver instance
- Try-catch block for RSSI reading failures
- Increments global error counter on failures

### 4. System Error Tracking
**Location:** Global implementation with NVS persistent storage

**Implementation:**
- Global error counter: `g_systemErrorCount`
- Persistent storage using ESP32 Preferences library
- Namespace: "telemetry"
- Key: "errorCount"
- Automatically loaded on telemetry system initialization
- Automatically saved on cleanup
- Periodic saves every 10 errors to reduce NVS wear

**Public API:**
```cpp
// Increment error counter and persist to NVS (every 10 errors)
void incrementSystemErrorCount();

// Get current error count
uint32_t getSystemErrorCount();

// Reset error counter to zero
void resetSystemErrorCount();
```

**Usage Example:**
```cpp
// In any subsystem that detects an error:
if (sensorReadFailed) {
    DEBUG_PRINTLN("Sensor read failed!");
    incrementSystemErrorCount();
}

// To check error count:
uint32_t errors = getSystemErrorCount();
DEBUG_PRINTF("Total system errors: %u\n", errors);

// To reset after maintenance:
resetSystemErrorCount();
```

**Error Handling:**
- Graceful handling of NVS initialization failures
- No error propagation if NVS unavailable (continues operation)
- Debug logging for NVS issues

### 5. Camera Status
**Location:** `collectDeviceHealth()` function (line ~845)

**Implementation:**
- Accesses global camera handler instance
- Calls `getStatus()` to retrieve camera state
- Checks two conditions:
  1. Camera is initialized (`initialized == true`)
  2. Last operation succeeded (`lastError == ESP_OK`)
- Returns true only if both conditions met

**Error Handling:**
- Null check for global camera handler instance
- Try-catch block for status retrieval failures
- Returns false on any error
- Increments global error counter on failures

## External Dependencies

### Required Headers:
```cpp
#include "../power_manager.h"      // For SolarManager namespace
#include "../camera_handler.h"     // For CameraHandler and CameraStatus
#include "lora_driver.h"           // For LoRaDriver
#include <Preferences.h>           // For NVS error tracking
```

### Expected External Instances:
```cpp
extern CameraHandler* g_cameraHandler;
extern LoRaDriver* g_loraDriver;
```

### Expected External Functions:
```cpp
// From SolarManager namespace (power_manager.h)
float SolarManager::getBatteryVoltage();
float SolarManager::getSolarVoltage();
```

## Configuration

All sensor thresholds and configurations are defined in `config.h`:

```cpp
// Battery voltage thresholds
#define BATTERY_FULL_VOLTAGE 4.2
#define BATTERY_CRITICAL_THRESHOLD 2.8
#define BATTERY_LOW_THRESHOLD 3.0

// Solar charging thresholds
#define SOLAR_CHARGING_VOLTAGE_MIN 4.0

// LoRa enable/disable
#define LORA_ENABLED false  // or true
```

## Testing Recommendations

1. **Power Management:**
   - Test with various battery voltage levels
   - Verify percentage calculation accuracy
   - Test with/without solar charging
   - Verify error handling when power manager unavailable

2. **CPU Temperature:**
   - Test on different ESP32 variants (ESP32, ESP32-S3)
   - Verify fallback behavior on unsupported platforms
   - Check temperature reading accuracy

3. **LoRa Signal:**
   - Test with LORA_ENABLED=true and LORA_ENABLED=false
   - Verify RSSI reading when LoRa hardware present
   - Test error handling when LoRa driver unavailable
   - Verify RSSI to signal strength conversion

4. **Error Tracking:**
   - Test error counter persistence across reboots
   - Verify periodic NVS saves (every 10 errors)
   - Test with NVS unavailable
   - Verify utility functions work correctly

5. **Camera Status:**
   - Test with camera initialized and operational
   - Test with camera failed or unavailable
   - Verify error detection from CameraStatus
   - Test null handling when camera handler not created

## Migration Notes

### Breaking Changes:
None. The API remains unchanged, only the internal implementation has been updated.

### Behavioral Changes:
1. Battery percentage now calculated from actual voltage readings
2. CPU temperature returns real sensor values instead of placeholder
3. LoRa signal properly returns 0 when disabled or unavailable
4. Error count persists across reboots
5. Camera status accurately reflects hardware state

### Backward Compatibility:
- All existing code using telemetry data structures will work unchanged
- New error tracking utilities are additive (no breaking changes)
- Fallback values ensure system continues operating even with sensor failures

## Future Enhancements

1. **Power Management:**
   - Add support for direct ADC reading when SolarManager unavailable
   - Implement more sophisticated battery percentage calculation (non-linear)
   - Add current consumption monitoring

2. **Temperature:**
   - Add support for external temperature sensors
   - Implement temperature trend analysis
   - Add thermal management alerts

3. **LoRa:**
   - Add SNR (Signal-to-Noise Ratio) reporting
   - Implement link quality analysis
   - Add automatic power adjustment based on signal strength

4. **Error Tracking:**
   - Add error categorization (sensor, network, storage, etc.)
   - Implement error rate analysis
   - Add automatic error reporting/alerting

5. **Camera:**
   - Add detailed camera health metrics
   - Implement image quality assessment
   - Add automatic recalibration on errors
