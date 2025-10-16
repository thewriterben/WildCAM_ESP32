# System Diagnostics Module

## Overview
The system diagnostics module provides comprehensive hardware testing at boot time to ensure all critical systems are operational before field deployment.

## Features

### Critical System Tests
1. **Camera Initialization** - Verifies camera is detected and can capture frames
2. **SD Card Read/Write** - Tests SD card mounting, writing, reading, and deletion
3. **PIR Sensor** - Checks for logical sensor response
4. **Power Management** - Reads battery voltage, charge state, and solar input
5. **Memory Check** - Verifies available heap, PSRAM, and flash memory

### Optional System Tests
6. **BME280 Sensor** - Tests temperature, humidity, and pressure readings (if enabled)
7. **RTC** - Verifies real-time clock functionality (if enabled)
8. **GPS** - Checks fix status and coordinates (if enabled)

## Usage

### In main.cpp
The diagnostics function is automatically called during system startup:

```cpp
#include "src/diagnostics.h"

void setup() {
    // ... initialization code ...
    
    // Run system diagnostics
    Logger::info("Running system diagnostics...");
    bool diagnosticsPass = runSystemDiagnostics();
    
    if (!diagnosticsPass) {
        Logger::warning("System diagnostics detected issues - review diagnostics.log");
    }
    
    // ... rest of setup ...
}
```

### Standalone Usage
You can also call individual test functions:

```cpp
#include "src/diagnostics.h"

// Test camera only
if (testCameraInit()) {
    Serial.println("Camera OK");
}

// Test SD card only
if (testSDCardReadWrite()) {
    Serial.println("SD Card OK");
}

// Test all systems
if (runSystemDiagnostics()) {
    Serial.println("All critical systems operational");
}
```

## Output

### Serial Console
Diagnostics results are printed to serial console in real-time:
```
[INFO] Testing camera initialization...
[INFO] Camera Init: PASS - Resolution: 320x240
[INFO] Testing SD card read/write...
[INFO] SD Card R/W: PASS - Size: 8192 MB, Type: 1
...
```

### Log File
Results are also saved to `/diagnostics.log` on the SD card:
```
[2025-01-15 10:30:45] Camera Init: PASS - Resolution: 320x240
[2025-01-15 10:30:46] SD Card R/W: PASS - Size: 8192 MB, Type: 1
[2025-01-15 10:30:47] PIR Sensor: PASS - State: 0
...

=== DIAGNOSTICS SUMMARY ===
Timestamp: 2025-01-15 10:30:50
Critical: Camera=OK SD=OK PIR=OK Power=OK Memory=OK
Optional: BME280=OK RTC=OK GPS=OK
Duration: 1234 ms
===========================
```

## Performance

- Execution time: < 2 seconds for all tests (unless waiting for sensor readings)
- Memory footprint: Minimal (~8KB code, ~2KB stack)
- Non-blocking: Does not interfere with system startup

## Return Values

- `true` - All critical systems passed
- `false` - One or more critical systems failed (camera, SD card, PIR, power, or memory)

Optional systems (BME280, RTC, GPS) do not affect the return value.

## Configuration

### Enable/Disable Optional Tests
Edit the diagnostics.cpp file to enable/disable optional tests:

```cpp
#define ENABLE_BME280  // Enable BME280 sensor test
#define ENABLE_GPS     // Enable GPS test
```

### Pin Definitions
PIR sensor pin is defined in diagnostics.cpp:
```cpp
#define PIR_PIN 13
```

### Memory Thresholds
Minimum free heap threshold:
```cpp
if (freeHeap < 50000) {  // Less than 50KB is concerning
    // Log warning
}
```

## Error Handling

- Individual test failures are logged but don't halt the system
- Critical system failures are clearly marked in logs
- Safe mode entry can be triggered based on diagnostic results
- Network alerts can be sent for critical failures (if network available)

## Testing

Unit tests are available in `test/test_diagnostics.cpp`:

```bash
# Run diagnostics tests
pio test -e test -f test_diagnostics
```

## Troubleshooting

### Camera Fails
- Check camera connection
- Verify camera model in config.h
- Ensure sufficient power supply

### SD Card Fails
- Check SD card is inserted
- Try reformatting SD card (FAT32)
- Verify SD_CS_PIN is correct

### Power Management Fails
- Check battery connection
- Verify voltage divider circuit
- Check power management IC

### Memory Check Fails
- Insufficient free heap (< 50KB)
- Memory leak in other modules
- PSRAM not detected

## Integration with Other Systems

The diagnostics module integrates with:
- **Logger** - For formatted output
- **StorageManager** - For log file writing
- **CameraManager** - For camera testing
- **PowerManagement** - For power readings
- **TimeManager** - For timestamps

## Future Enhancements

- [ ] Add support for thermal camera testing
- [ ] Add network connectivity tests (WiFi, LoRa)
- [ ] Add encryption/security system tests
- [ ] Add configurable test thresholds
- [ ] Add remote diagnostics via API
- [ ] Add historical diagnostics tracking
- [ ] Add automatic repair/recovery actions
