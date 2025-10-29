# Logging System Integration Guide

## Quick Start

The logging system has been integrated into WildCAM ESP32. Here's how to use it in your code:

### 1. Include the Logger Header

```cpp
#include "Logger.h"
```

### 2. Initialize in setup()

The logger is automatically initialized in `main.cpp` based on `config.h` settings:

```cpp
#if LOGGING_ENABLED
Logger::init((LogLevel)DEFAULT_LOG_LEVEL, LOG_TO_SERIAL, LOG_TO_SD, LOG_FILE_PATH);
#endif
```

### 3. Use Logging Macros

```cpp
LOG_DEBUG("Detailed debug info: %d", value);
LOG_INFO("General information: %s", message);
LOG_WARN("Warning condition: %.2f", voltage);
LOG_ERROR("Error occurred: %s", error);
```

## Configuration

Edit `include/config.h`:

```cpp
// Enable/disable logging
#define LOGGING_ENABLED true

// Set minimum log level (0=DEBUG, 1=INFO, 2=WARN, 3=ERROR)
#define DEFAULT_LOG_LEVEL 1

// Enable Serial output
#define LOG_TO_SERIAL true

// Enable SD card logging
#define LOG_TO_SD false

// Log file path
#define LOG_FILE_PATH "/system.log"
```

## Key Features

✅ **Multiple Log Levels**: DEBUG, INFO, WARN, ERROR
✅ **Timestamps**: Automatic timestamp on every log entry
✅ **Function/Line Tracking**: Shows where log was called
✅ **Filtering**: Only logs at or above configured level
✅ **Dual Output**: Serial console and/or SD card
✅ **Format Strings**: Full printf-style formatting support

## Integration in main.cpp

The logging system has been integrated into all critical operations:

- System initialization
- Power management
- Camera operations
- Motion detection
- Image capture and storage
- Battery monitoring
- WiFi connection
- Error handling

## Example Output

```
[0d 00:00:12.345] [INFO] [setup:118] WildCAM ESP32 Starting Up - Version 1.0.0
[0d 00:00:12.456] [INFO] [setup:125] Initializing watchdog timer...
[0d 00:00:12.567] [INFO] [setup:132] Watchdog timer initialized
[0d 00:00:12.678] [INFO] [setup:137] Initializing Power Manager...
[0d 00:00:13.123] [INFO] [setup:146] Power Manager initialized
[0d 00:00:13.234] [INFO] [setup:151] Battery status: 3.95V (85%)
```

## Testing

Run the logger tests:
```bash
platformio test -e test -f test_logger
```

## See Also

- `LOGGING_SYSTEM_README.md` - Comprehensive documentation
- `examples/logging_example.cpp` - Working example code
- `test/test_logger.cpp` - Test suite
