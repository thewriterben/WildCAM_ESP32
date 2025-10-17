# Security Tamper Detection Response Implementation

## Overview
This document describes the comprehensive tamper detection and response system implemented in WildCAM ESP32 v2.0. The system provides robust security monitoring with multiple layers of protection and detailed event logging.

## Features Implemented

### 1. Tamper Detection
- Hardware-based tamper detection via SecurityManager
- Continuous monitoring every 10 seconds
- Physical tampering detection using hardware security chip
- No false positives during normal operation

### 2. Security Response Actions

When tampering is detected, the system automatically executes the following security protocols:

#### A. Critical Alert Logging
- **Console Logging**: Immediate critical alert to serial console
- **File Logging**: Detailed event log saved to `/security.log` on SD card
- **Log Contents**:
  - Timestamp (from RTC or system time)
  - Battery voltage
  - Free heap memory
  - Event description

#### B. Evidence Capture
- **Automatic Image Capture**: Takes photo immediately upon tamper detection
- **Filename Format**: `/images/TAMPER_[timestamp].jpg`
- **Timestamp Format**: `YYYYMMDD_HHMMSS` or milliseconds-based
- **Storage**: Saved to SD card if available
- **Error Handling**: Continues operation even if capture fails

#### C. Network Alerting
- **Alert Transmission**: Sends critical alert if network available
- **Alert Contents**:
  - Event timestamp
  - Image capture status (SUCCESS/FAILED)
  - Battery voltage
  - Device identification
- **Transport Methods**: WiFi, cellular, or satellite (depending on configuration)
- **Graceful Degradation**: Alert saved locally if network unavailable

#### D. Persistent Event Counter
- **Storage**: Uses ESP32 NVS (Non-Volatile Storage)
- **Persistence**: Survives reboots and power loss
- **Key**: `tamper_count` in `wildcam` namespace
- **Auto-increment**: Increases by 1 for each tamper event
- **Access**: Retrievable via `getTamperCounter()` function

#### E. Lockdown Mode
- **Duration**: 1 hour (3,600,000 milliseconds)
- **Effect**: Disables AI image capture to prevent attacker interference
- **Activation**: Automatic when tamper detected
- **Deactivation**: Automatic after timeout expires
- **Configurable**: Can be disabled via `g_lockdown_enabled` flag
- **Status Monitoring**: Checked at start of AI processing loop

## Code Structure

### Key Functions

#### `handleTamperDetection()`
Main security response coordinator. Orchestrates all tamper response actions.

**Location**: `firmware/main.cpp` (lines 518-551)

**Responsibilities**:
- Critical alert logging
- Detailed file logging to `/security.log`
- Image capture coordination
- Network alert transmission
- Tamper counter increment
- Lockdown mode activation

#### `captureTamperImage()`
Captures and saves tamper event evidence image.

**Location**: `firmware/main.cpp` (lines 457-512)

**Process**:
1. Verify camera availability
2. Check SD card initialization
3. Capture frame via CameraManager
4. Generate timestamped filename
5. Save to SD card
6. Return success/failure status

#### `incrementTamperCounter()`
Increments and persists the tamper event counter.

**Location**: `firmware/main.cpp` (lines 417-422)

**Process**:
1. Read current counter from NVS
2. Increment by 1
3. Save to NVS
4. Log new counter value
5. Return updated counter

#### `getTamperCounter()`
Retrieves current tamper counter value from persistent storage.

**Location**: `firmware/main.cpp` (lines 409-411)

**Returns**: Current tamper count (0 if never set)

#### `sendCriticalAlert(const char* message)`
Sends security alert via available network.

**Location**: `firmware/main.cpp` (lines 438-451)

**Process**:
1. Check network availability
2. Format alert message
3. Transmit via WiFi/cellular/satellite
4. Return transmission status

#### `manageLockdownMode()`
Manages lockdown mode state and automatic expiration.

**Location**: `firmware/main.cpp` (lines 557-566)

**Process**:
1. Check if in lockdown
2. Calculate elapsed time
3. Deactivate if duration expired
4. Log state changes

### System State

The `SystemState` struct tracks all security and system states:

```cpp
struct SystemState {
    // Core system state
    bool ai_initialized;
    bool power_system_ok;
    bool security_active;
    bool network_connected;
    float battery_level;
    
    // Lockdown mode state
    bool in_lockdown;                  // Currently in lockdown
    unsigned long lockdown_start_time; // When lockdown started
    
    // ... additional network and monitoring fields
};
```

## Configuration

### Lockdown Mode
- **Enable/Disable**: Set `g_lockdown_enabled` (default: `true`)
- **Duration**: `LOCKDOWN_DURATION_MS` = 3,600,000 ms (1 hour)
- **Location**: `firmware/main.cpp` (line 59)

### Tamper Detection Frequency
- **Interval**: 10 seconds
- **Location**: `securityMonitoringTask` in `firmware/main.cpp` (line 575)

### Storage Paths
- **Images**: `/images/TAMPER_*.jpg`
- **Security Log**: `/security.log`
- **Metadata**: Same as image path with `.json` extension

## Security Features

### What IS Implemented ✅
- Hardware tamper detection
- Critical alert logging (console + file)
- Evidence image capture
- Network alert transmission
- Persistent event counter
- Automatic lockdown mode
- Graceful error handling
- Multi-layer logging

### What is NOT Implemented ❌
- **Data wipe**: INTENTIONALLY NOT IMPLEMENTED for safety
- **Automatic firmware reset**: Too risky for production
- **Remote lockdown override**: Security consideration
- **GPS location logging**: Requires GPS hardware (can be added)

## Testing

### Manual Testing
1. Trigger physical tamper detection (hardware dependent)
2. Verify console shows "TAMPER DETECTED" critical alert
3. Check `/security.log` on SD card for detailed log entry
4. Verify `/images/TAMPER_*.jpg` image exists
5. Confirm tamper counter incremented in NVS
6. Observe lockdown mode activation message
7. Wait 1 hour and verify lockdown deactivation

### Unit Tests
Test suite available at `tests/test_tamper_response.cpp`:
- Tamper counter initialization
- Counter increment and persistence
- Lockdown activation and duration
- Network availability checks
- Complete tamper response flow
- Multiple tamper events handling
- Safety verification (no data wipe)

### Running Tests
```bash
cd firmware
pio test -e test
```

## Troubleshooting

### Image Capture Fails
- **Check**: SD card initialized
- **Check**: Camera initialized
- **Check**: Sufficient free space on SD card
- **Solution**: System continues operation, logs error

### Network Alert Fails
- **Check**: WiFi/network connected
- **Check**: `system_state.network_connected` = true
- **Solution**: Alert queued for retry when network available

### Lockdown Not Deactivating
- **Check**: System uptime exceeds 1 hour
- **Check**: `manageLockdownMode()` being called
- **Solution**: Verify task is running, check system clock

### Tamper Counter Not Persisting
- **Check**: NVS initialized (`g_preferences.begin()`)
- **Check**: Flash storage not corrupted
- **Solution**: Clear NVS and restart: `g_preferences.clear()`

## Performance Impact

- **Memory**: ~2KB additional for state tracking
- **CPU**: <1% overhead for monitoring (10-second interval)
- **Storage**: ~100 bytes per tamper event (log entry)
- **Network**: ~256 bytes per alert transmission
- **Camera**: One-time capture, ~100-500KB JPEG

## Future Enhancements

Potential improvements (not currently implemented):
- GPS coordinates in log entries (requires GPS module)
- Tamper sensitivity adjustment via configuration
- Remote tamper log retrieval API
- Encrypted alert transmission
- Multiple image capture sequence
- Audio recording capability
- Backup power detection
- Case temperature monitoring

## API Reference

### Public Functions

```cpp
// Get current tamper count
uint32_t getTamperCounter();

// Increment tamper counter
uint32_t incrementTamperCounter();

// Check network availability
bool isNetworkAvailable();

// Send critical alert
bool sendCriticalAlert(const char* message);

// Capture tamper evidence
bool captureTamperImage();

// Handle tamper detection (main coordinator)
void handleTamperDetection();

// Manage lockdown state
void manageLockdownMode();
```

### Configuration Constants

```cpp
#define LOCKDOWN_DURATION_MS (3600000)  // 1 hour
#define SD_CS_PIN 13                     // SD card chip select
```

## Dependencies

Required libraries:
- `Preferences.h` - NVS persistent storage
- `SD.h` - SD card file operations
- `WiFi.h` - Network connectivity
- `HTTPClient.h` - Network alert transmission
- `esp_camera.h` - Camera frame buffer access

## Compliance

This implementation follows security best practices:
- ✅ No destructive operations (no data wipe)
- ✅ Fail-safe design (continues on errors)
- ✅ Audit trail (persistent logging)
- ✅ Evidence preservation (image capture)
- ✅ Multiple notification channels (console + file + network)
- ✅ Automatic response (no user intervention required)
- ✅ Configurable behavior (lockdown can be disabled)

## License

Part of WildCAM ESP32 v2.0 Advanced Wildlife Monitoring Platform
Copyright © 2025 WildCAM ESP32 Team

---

**Document Version**: 1.0
**Last Updated**: 2025-10-16
**Author**: WildCAM ESP32 Development Team
