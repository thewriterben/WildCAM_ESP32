# WildCAM ESP32 Logging System

## Overview

The WildCAM ESP32 logging system provides comprehensive logging capabilities with multiple log levels, timestamps, filtering, and optional SD card storage.

## Features

1. **Multiple Log Levels**
   - `DEBUG`: Detailed debug information
   - `INFO`: General information messages
   - `WARN`: Warning messages
   - `ERROR`: Error messages

2. **Timestamps**
   - All log messages include timestamps in format: `DDd HH:MM:SS.mmm`
   - Based on system uptime (millis())

3. **Log Filtering**
   - Filter logs by minimum level
   - Only logs at or above the configured level are output

4. **Dual Output Options**
   - Serial console output (for debugging)
   - SD card file output (for persistent logs)

5. **Metadata Tracking**
   - Automatic inclusion of function name
   - Automatic inclusion of line number
   - Source file tracking

6. **Configurable**
   - All settings can be configured in `config.h`
   - Runtime configuration changes supported

## Configuration

### In `config.h`

```cpp
#define LOGGING_ENABLED true        // Enable/disable logging system
#define DEFAULT_LOG_LEVEL 1         // 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
#define LOG_TO_SERIAL true          // Enable Serial console output
#define LOG_TO_SD false             // Enable SD card output
#define LOG_FILE_PATH "/system.log" // SD card log file path
```

### Runtime Configuration

```cpp
// Initialize logger
Logger::init(LOG_INFO, true, false, "/system.log");

// Change log level
Logger::setLogLevel(LOG_DEBUG);

// Enable/disable outputs
Logger::setSerialOutput(true);
Logger::setSDOutput(true);

// Change log file path
Logger::setLogFilePath("/custom.log");
```

## Usage

### Basic Logging

```cpp
#include "Logger.h"

// Simple messages
LOG_DEBUG("This is a debug message");
LOG_INFO("System initialized");
LOG_WARN("Battery low");
LOG_ERROR("Failed to initialize camera");
```

### Format String Support

```cpp
// With variables
int value = 42;
float voltage = 3.7;
const char* status = "OK";

LOG_INFO("Sensor value: %d", value);
LOG_WARN("Battery voltage: %.2fV", voltage);
LOG_ERROR("Status: %s, Code: %d", status, value);
```

### Example Output

```
[0d 00:00:12.345] [INFO] [setup:125] WildCAM ESP32 Starting Up - Version 1.0.0
[0d 00:00:12.456] [INFO] [setup:142] Power Manager initialized
[0d 00:00:12.567] [INFO] [setup:148] Battery status: 3.95V (85%)
[0d 00:00:13.123] [WARN] [loop:267] Battery low! Disabling web server
[0d 00:00:15.789] [INFO] [loop:238] Motion detected
[0d 00:00:16.234] [INFO] [loop:251] Image saved: /IMG_0001.jpg (size: 45678 bytes)
[0d 00:00:16.345] [ERROR] [loop:289] Failed to save metadata
```

## Log Levels

### DEBUG (0)
Use for detailed debugging information that is typically only useful during development.

**Example:**
```cpp
LOG_DEBUG("Stabilizing camera for %d ms", IMAGE_CAPTURE_DELAY_MS);
LOG_DEBUG("Metadata saved for %s", filepath.c_str());
```

### INFO (1)
Use for general informational messages about system operation.

**Example:**
```cpp
LOG_INFO("WildCAM ESP32 Starting Up - Version %s", FIRMWARE_VERSION);
LOG_INFO("Camera initialized");
LOG_INFO("Motion detected");
```

### WARN (2)
Use for warning conditions that don't prevent operation but should be noted.

**Example:**
```cpp
LOG_WARN("Battery low! Disabling web server to save power");
LOG_WARN("Failed to save metadata for %s", filepath.c_str());
LOG_WARN("WiFi connection failed after %d attempts", attempts);
```

### ERROR (3)
Use for error conditions that prevent normal operation.

**Example:**
```cpp
LOG_ERROR("Camera initialization failed!");
LOG_ERROR("Failed to save image to SD card");
LOG_ERROR("Storage initialization failed!");
```

## Integration Example

The logging system is integrated into `main.cpp` to log all critical operations:

```cpp
void setup() {
    // Initialize logging first
    #if LOGGING_ENABLED
    Logger::init((LogLevel)DEFAULT_LOG_LEVEL, LOG_TO_SERIAL, LOG_TO_SD, LOG_FILE_PATH);
    LOG_INFO("WildCAM ESP32 Starting Up - Version %s", FIRMWARE_VERSION);
    #endif
    
    // Log initialization steps
    LOG_INFO("Initializing Power Manager...");
    if (!power.init(BATTERY_ADC_PIN)) {
        LOG_ERROR("Power Manager initialization failed!");
        // Handle error...
    }
    LOG_INFO("Power Manager initialized");
    
    // Log battery status
    LOG_INFO("Battery status: %.2fV (%d%%)", batteryVoltage, batteryPercent);
}

void loop() {
    // Log motion events
    if (motionDetector.isMotionDetected()) {
        LOG_INFO("Motion detected");
        // Handle motion...
    }
    
    // Log periodic checks
    LOG_INFO("Battery check: %.2fV (%d%%)", voltage, percent);
}
```

## Best Practices

1. **Choose appropriate log levels**
   - Use DEBUG for verbose development information
   - Use INFO for normal operational messages
   - Use WARN for concerning but non-critical issues
   - Use ERROR for failures that need attention

2. **Disable SD logging by default**
   - SD card writes consume power and wear the card
   - Enable only when persistent logs are needed
   - Consider rotating or clearing logs periodically

3. **Use format strings efficiently**
   - Include relevant context in log messages
   - Use variable substitution for dynamic data
   - Keep messages concise but informative

4. **Set appropriate minimum log level**
   - Use DEBUG during development
   - Use INFO or WARN in production
   - Use ERROR for minimal logging in power-critical deployments

5. **Monitor log file size**
   - Logs can grow large over time
   - Implement log rotation if needed
   - Clear old logs periodically

## Performance Considerations

- **Serial Output**: Minimal performance impact (~1ms per message)
- **SD Card Output**: Higher latency (~10-50ms per write)
- **Log Filtering**: Near-zero overhead for filtered messages
- **Format Strings**: Standard printf overhead applies

## Testing

Run the logging system tests:

```bash
platformio test -e test -f test_logger
```

Test coverage includes:
- Logger initialization
- Log level filtering
- Timestamp generation
- Format string support
- Configuration options
- Serial/SD output toggling

## Troubleshooting

### Logs not appearing on Serial
- Check that `LOG_TO_SERIAL` is `true` in config.h
- Verify minimum log level allows the message
- Ensure Serial baud rate is 115200

### Logs not writing to SD card
- Verify SD card is initialized before logger
- Check that `LOG_TO_SD` is `true`
- Ensure log file path is valid
- Check SD card has free space

### Missing timestamps
- Logger must be initialized before use
- Timestamps are based on millis() uptime

## Future Enhancements

Potential future improvements:
- Log rotation (automatic old log deletion)
- Compressed log storage
- Network log streaming
- Real-time clock (RTC) integration for actual timestamps
- Configurable log format templates
- Log level runtime changes via web interface
