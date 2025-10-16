# Time Manager Implementation

## Overview
This directory contains the stub implementation of the time manager for the WildCAM ESP32 firmware. The time manager provides time synchronization and formatting functions for the wildlife camera system.

## Files
- `time_manager.h` - Header file with function declarations
- `time_manager.cpp` - Implementation file with stub functions

## Functions

### `bool initializeTimeManager()`
Initializes the time manager system.

**Current Implementation:**
- Logs initialization message
- Sets timezone offset to 0 (UTC)
- Returns `true`

**TODO:**
- Initialize RTC if available (e.g., DS3231, PCF8523)
- Set up time synchronization callbacks
- Configure ESP32 system time with proper timezone

### `time_t getCurrentTime()`
Gets the current system time.

**Current Implementation:**
- Returns the result of `time(nullptr)`
- Logs function call
- Checks for invalid time and logs warning

**TODO:**
- Add validation to check if time has been synchronized
- Consider falling back to RTC if system time is invalid

### `bool syncWithRTC()`
Synchronizes time with RTC (Real-Time Clock).

**Current Implementation:**
- Logs attempt
- Returns `false` (stub)

**TODO:**
- Initialize I2C/SPI communication with RTC module
- Read time from RTC (e.g., DS3231, PCF8523)
- Validate RTC time data
- Update ESP32 system time with RTC time
- Apply timezone offset if needed

### `bool syncWithNTP()`
Synchronizes time with NTP (Network Time Protocol) server.

**Current Implementation:**
- Logs attempt
- Returns `false` (stub)

**TODO:**
- Check WiFi connection status
- Configure NTP server (e.g., pool.ntp.org, time.google.com)
- Send NTP request and wait for response
- Parse NTP response and extract time data
- Update ESP32 system time with NTP time
- Apply timezone offset if needed

### `String getFormattedTime()`
Gets formatted time string in "YYYY-MM-DD HH:MM:SS" format.

**Current Implementation:**
- Uses ESP32 `getLocalTime()` function
- Returns formatted string using `strftime()`
- Returns default "0000-00-00 00:00:00" if time is invalid

**TODO:**
- Add support for custom format strings
- Handle invalid time scenarios gracefully
- Consider adding milliseconds support

### `void setTimezone(int offset)`
Sets the timezone offset in hours from UTC.

**Current Implementation:**
- Logs timezone setting
- Stores offset in global variable
- Calls `configTime()` to configure ESP32 timezone

**TODO:**
- Validate offset range (typically -12 to +14)
- Update ESP32 timezone configuration
- Adjust DST (Daylight Saving Time) if applicable
- Store timezone setting to persistent storage (e.g., EEPROM, SPIFFS)

## Usage Example

```cpp
#include "time_manager.h"
#include "../../utils/logger.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize time manager
    if (initializeTimeManager()) {
        Logger::info("Time manager ready");
    }
    
    // Set timezone (e.g., EST = UTC-5)
    setTimezone(-5);
    
    // Try to sync with NTP (currently returns false)
    if (!syncWithNTP()) {
        Logger::warning("NTP sync not available");
    }
    
    // Get current time
    time_t now = getCurrentTime();
    String formatted = getFormattedTime();
    
    Logger::info("Current time: %s", formatted.c_str());
}
```

## Dependencies
- `Arduino.h` - Arduino framework
- `time.h` - Standard C time library
- `../../utils/logger.h` - Logging utility

## Testing
Unit tests are available in `firmware/test/test_time_manager.cpp`.

To run tests:
```bash
cd firmware
pio test -e test
```

## Future Development
The stub implementation is ready for integration with:
- RTC modules (DS3231, PCF8523, etc.)
- NTP client for network time synchronization
- Persistent storage for timezone settings
- DST (Daylight Saving Time) support
- Custom time format strings

## Notes
- All functions include TODO comments indicating future implementation steps
- Logging is used throughout for debugging and monitoring
- Safe default values are returned to prevent crashes
- The implementation is ready to compile as part of the firmware
