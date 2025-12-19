# Time Keeping Module Documentation

## Overview

The WildCAM ESP32 Time Keeping module provides accurate and persistent timestamps for wildlife camera operations. This module supports:

- **NTP Time Synchronization** - Automatic time sync over WiFi connection
- **External RTC Support** - DS3231 real-time clock module for persistent time keeping
- **Fallback Mechanisms** - Graceful degradation when NTP or RTC is unavailable
- **Timezone Support** - Configurable GMT offset and daylight saving time

## Features

### NTP Time Sync

When WiFi is connected, the system automatically synchronizes time with an NTP server. This provides accurate timestamps without requiring any additional hardware.

**Advantages:**
- No additional hardware required
- Highly accurate time (within seconds of UTC)
- Automatic correction for clock drift

**Limitations:**
- Requires WiFi connection
- Time is lost on power cycle if external RTC not present

### DS3231 External RTC

The DS3231 is a high-precision real-time clock that maintains time even when the ESP32 is powered off. It includes:

- Battery backup (CR2032) for years of operation without main power
- Temperature-compensated oscillator for high accuracy (±2 ppm from 0°C to +40°C)
- Built-in temperature sensor
- I2C interface

**Advantages:**
- Maintains time across power cycles
- High accuracy even without network
- Built-in temperature monitoring

**Limitations:**
- Requires additional hardware ($2-5)
- Requires I2C pins (may conflict with some configurations)

## Hardware Setup

### DS3231 RTC Module Wiring

Connect the DS3231 module to your ESP32-CAM:

| DS3231 Pin | ESP32-CAM Pin | Notes |
|------------|---------------|-------|
| VCC | 3.3V | Can also use 5V if module has regulator |
| GND | GND | Common ground |
| SDA | GPIO 21 | I2C data (configurable) |
| SCL | GPIO 22 | I2C clock (configurable) |

**Note for AI-Thinker ESP32-CAM:** The default I2C pins (GPIO 21/22) may conflict with other peripherals. If needed, configure alternative pins in `config.h`:

```cpp
#define RTC_I2C_SDA_PIN 14
#define RTC_I2C_SCL_PIN 15
```

### Wiring Diagram

```
┌─────────────────────┐      ┌─────────────────────┐
│     ESP32-CAM       │      │   DS3231 RTC        │
│                     │      │                     │
│  GPIO 21 (SDA) ─────┼──────┼─── SDA              │
│  GPIO 22 (SCL) ─────┼──────┼─── SCL              │
│  3.3V ──────────────┼──────┼─── VCC              │
│  GND ───────────────┼──────┼─── GND              │
│                     │      │                     │
└─────────────────────┘      └─────────────────────┘
                                    │
                              ┌─────┴─────┐
                              │  CR2032   │
                              │  Battery  │
                              └───────────┘
```

## Configuration

### Basic Configuration in config.h

```cpp
//==============================================================================
// TIME MANAGEMENT SETTINGS (NTP and RTC)
//==============================================================================

// Enable/disable time management features
#define TIME_MANAGEMENT_ENABLED true

// NTP Server Settings
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.nist.gov"
#define NTP_SYNC_TIMEOUT_MS 10000

// Timezone Settings
#define GMT_OFFSET_SEC 0           // Offset from GMT in seconds
#define DAYLIGHT_OFFSET_SEC 0      // DST offset in seconds (usually 3600)

// Automatic NTP sync when WiFi connects
#define NTP_AUTO_SYNC true

// External RTC Configuration
#define RTC_DS3231_ENABLED true
#define RTC_I2C_SDA_PIN 21
#define RTC_I2C_SCL_PIN 22
```

### Timezone Examples

| Location | GMT_OFFSET_SEC | DAYLIGHT_OFFSET_SEC |
|----------|----------------|---------------------|
| UTC/GMT | 0 | 0 |
| US Eastern (EST) | -18000 | 3600 (if DST) |
| US Pacific (PST) | -28800 | 3600 (if DST) |
| UK (GMT/BST) | 0 | 3600 (if DST) |
| Central Europe (CET) | 3600 | 3600 (if DST) |
| Japan (JST) | 32400 | 0 |
| Australia Eastern (AEST) | 36000 | 3600 (if DST) |

## Usage

### Automatic Operation

When properly configured, the TimeManager operates automatically:

1. **On Boot:**
   - Initializes I2C for RTC communication
   - Attempts to detect DS3231 external RTC
   - If RTC found and has valid time, loads time from RTC
   - If no RTC, waits for WiFi and NTP sync

2. **When WiFi Connects:**
   - Automatically syncs time with NTP server
   - Saves synced time to external RTC (if available)

3. **During Operation:**
   - All timestamps use accurate system time
   - Metadata includes ISO 8601 formatted timestamps
   - Time source is recorded in metadata

### Accessing Time Programmatically

If you need to access time in your code:

```cpp
#include "TimeManager.h"

// Global instance (already created in main.cpp)
extern TimeManager timeManager;

// Get ISO 8601 timestamp
char timestamp[30];
timeManager.getTimestamp(timestamp, sizeof(timestamp));
Serial.printf("Current time: %s\n", timestamp);

// Get Unix timestamp
time_t now = timeManager.getUnixTime();

// Get date string for directory naming (YYYYMMDD format)
char dateStr[10];
timeManager.getDateString(dateStr, sizeof(dateStr));

// Get time string for filenames (HHMMSS format)
char timeStr[10];
timeManager.getTimeString(timeStr, sizeof(timeStr));

// Check if time is valid
if (timeManager.isTimeSet()) {
    Serial.println("Time is valid");
    Serial.printf("Time source: %s\n", timeManager.getTimeSourceString());
}

// Get RTC temperature (if DS3231 is connected)
if (timeManager.hasExternalRTC()) {
    float temp = timeManager.getRTCTemperature();
    Serial.printf("RTC temperature: %.2f°C\n", temp);
}

// Manually set time
timeManager.setTime(2024, 12, 25, 12, 0, 0);  // Christmas 2024 at noon

// Print full status
timeManager.printStatus();
```

## Image Metadata Format

When time is available, image metadata includes accurate timestamps:

```json
{
  "timestamp": "2024-12-25T12:30:45Z",
  "unix_timestamp": 1735130445,
  "time_source": "NTP",
  "image_path": "/images/20241225/IMG_123045_001.jpg",
  "battery_voltage": 3.85,
  "battery_percent": 75,
  "image_size": 98234,
  "image_count": 42
}
```

### Time Source Values

| Value | Description |
|-------|-------------|
| `NTP` | Time synchronized via NTP server |
| `External RTC (DS3231)` | Time loaded from external RTC |
| `Internal RTC` | Time from ESP32 internal RTC (not persistent) |
| `Manual` | Time set manually |
| `millis` | No valid time, using uptime counter |

## Troubleshooting

### Time Not Syncing via NTP

1. **Check WiFi connection:** Ensure WiFi is connected before NTP sync
2. **Check NTP server:** Try an alternative server
3. **Increase timeout:** Set `NTP_SYNC_TIMEOUT_MS` to 15000 or higher
4. **Check firewall:** Ensure UDP port 123 is not blocked

### External RTC Not Detected

1. **Check wiring:** Verify SDA/SCL connections
2. **Check I2C address:** DS3231 uses address 0x68
3. **Check power:** Ensure RTC module has power
4. **Check pull-ups:** Some modules need external 4.7kΩ pull-up resistors

### RTC Showing Wrong Time

1. **Replace battery:** CR2032 battery may be depleted
2. **Re-sync with NTP:** Connect to WiFi to update RTC time
3. **Check timezone:** Verify GMT offset settings

### Time Resets on Power Cycle

1. **Install external RTC:** DS3231 module required for persistent time
2. **Check RTC battery:** Ensure CR2032 is installed and not depleted
3. **Verify RTC detection:** Check serial output for "External RTC detected"

## Serial Output Examples

### Successful Initialization with External RTC

```
Initializing Time Manager...
   ✓ Time Manager initialized
   External RTC (DS3231) detected
   Current time: 2024-12-25T12:30:45Z
```

### NTP Sync Success

```
   Synchronizing time via NTP...
   ✓ Time synchronized via NTP
   Current time: 2024-12-25T12:30:45Z
   Time saved to external RTC
```

### Without External RTC

```
Initializing Time Manager...
   ✓ Time Manager initialized
   External RTC not found - using internal RTC
   Time not set - will sync via NTP when WiFi connects
```

## API Reference

### TimeManager Class

| Method | Description |
|--------|-------------|
| `init(gmtOffset, dstOffset)` | Initialize time management system |
| `syncNTP(server, timeout)` | Synchronize time with NTP server |
| `isTimeSet()` | Check if valid time is available |
| `getUnixTime()` | Get current Unix timestamp |
| `getTime(tm*)` | Get time as tm structure |
| `getTimestamp(buf, size)` | Get ISO 8601 timestamp string |
| `getDateString(buf, size)` | Get date string (YYYYMMDD) |
| `getTimeString(buf, size)` | Get time string (HHMMSS) |
| `getTimeSource()` | Get current time source enum |
| `getTimeSourceString()` | Get time source as string |
| `saveToRTC()` | Save system time to external RTC |
| `loadFromRTC()` | Load time from external RTC |
| `setTime(year,mon,day,h,m,s)` | Set time manually |
| `setTime(time_t)` | Set time from Unix timestamp |
| `hasExternalRTC()` | Check if DS3231 is available |
| `getRTCTemperature()` | Get temperature from DS3231 |
| `printStatus()` | Print time status to Serial |

## Power Consumption

| State | Current Draw | Notes |
|-------|-------------|-------|
| DS3231 Active | ~200 µA | When powered from main supply |
| DS3231 Battery Backup | ~3 µA | When running from CR2032 only |
| CR2032 Battery Life | ~8-10 years | In backup mode |

## Version History

- **v1.0.0** (December 2024)
  - Initial implementation
  - NTP time synchronization
  - DS3231 RTC support
  - Timezone configuration
  - Automatic time persistence
