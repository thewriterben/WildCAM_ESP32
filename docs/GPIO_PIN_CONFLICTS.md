# GPIO Pin Conflicts and Resolution Guide

## Overview

The AI-Thinker ESP32-CAM has limited available GPIO pins, which creates conflicts between various hardware features (camera, LoRa, sensors, storage). This document provides a comprehensive analysis of pin conflicts and solutions.

## ⚠️ Critical Information

**IMPORTANT**: The AI-Thinker ESP32-CAM cannot simultaneously support all features due to GPIO limitations. Certain feature combinations are **physically impossible** without external GPIO expanders or different hardware.

## GPIO Pin Availability

### Total GPIO Pins on ESP32
- **Total GPIO**: 34 pins (GPIO 0-39)
- **Reserved for Flash**: 6 pins (GPIO 6-11) - **NEVER USE**
- **Input-Only**: 4 pins (GPIO 34, 35, 36, 39)
- **Available on AI-Thinker ESP32-CAM**: ~6 pins after camera configuration

### Boot Strapping Pins (Use with Caution)
- **GPIO 0**: Camera XCLK, boot mode selection
- **GPIO 2**: SD card D0, internal pull-up, affects boot mode
- **GPIO 12**: SD card D2, internal pull-down, affects flash voltage
- **GPIO 15**: SD card CMD, internal pull-up, affects boot mode

## Visual Pin Conflict Diagram

```
AI-Thinker ESP32-CAM GPIO Conflict Map
========================================

CAMERA PINS (Fixed, Highest Priority):
┌──────────────────────────────────────┐
│ GPIO 0  : XCLK    ●──────────────────┤ Boot mode
│ GPIO 5  : Y2      ●──────────────────┤ LoRa CS (CONFLICT!)
│ GPIO 18 : Y3      ●──────────────────┤ LoRa SCK (CONFLICT!)
│ GPIO 19 : Y4      ●──────────────────┤ LoRa MISO (CONFLICT!)
│ GPIO 21 : Y5      ●                  │
│ GPIO 22 : PCLK    ●                  │
│ GPIO 23 : HREF    ●──────────────────┤ LoRa MOSI (CONFLICT!)
│ GPIO 25 : VSYNC   ●                  │
│ GPIO 26 : SIOD    ●──────────────────┤ I2C SDA / LoRa DIO0
│ GPIO 27 : SIOC    ●──────────────────┤ I2C SCL
│ GPIO 32 : PWDN    ●──────────────────┤ Solar voltage (CONFLICT!)
│ GPIO 34 : Y8      ●──────────────────┤ Battery voltage (shareable)
│ GPIO 35 : Y9      ●                  │
│ GPIO 36 : Y6      ●                  │
│ GPIO 39 : Y7      ●                  │
└──────────────────────────────────────┘

SD CARD PINS (When Enabled):
┌──────────────────────────────────────┐
│ GPIO 2  : D0      ●──────────────────┤ Power LED (CONFLICT!)
│ GPIO 4  : D1      ●──────────────────┤ Built-in LED (CONFLICT!)
│ GPIO 12 : D2      ●──────────────────┤ LoRa CS (shareable with caution)
│ GPIO 13 : D3/CS   ●                  │
│ GPIO 14 : CLK     ●──────────────────┤ Charging control
│ GPIO 15 : CMD     ●──────────────────┤ Charging LED
└──────────────────────────────────────┘

AVAILABLE PINS (After Camera + SD):
┌──────────────────────────────────────┐
│ GPIO 1  : Available ●────────────────┤ PIR / UART TX
│ GPIO 3  : Available ●────────────────┤ UART RX
│ GPIO 16 : Available ●────────────────┤ LoRa RST / IR LED
│ GPIO 17 : Available ●────────────────┤ PIR power / Servo
│ GPIO 33 : Available ●────────────────┤ Battery/Light sensor (ADC)
└──────────────────────────────────────┘

RESERVED (NEVER USE):
┌──────────────────────────────────────┐
│ GPIO 6-11 : FLASH MEMORY             │
│ (Connected to SPI flash - DO NOT USE)│
└──────────────────────────────────────┘

Legend:
  ● = Pin in use
  ── = Conflict/shared usage
  (CONFLICT!) = Incompatible functions
  (shareable) = Can share with coordination
```

## Complete GPIO Pin Matrix for AI-Thinker ESP32-CAM

| GPIO | Primary Function | Alternative/Shared Function | Type | Conflict Group |
|------|-----------------|----------------------------|------|----------------|
| 0 | Camera XCLK | Boot mode selection | I/O | Camera |
| 1 | PIR sensor | UART TX (debug) | I/O | Available |
| 2 | SD card D0 | Power LED (when SD disabled) | I/O | SD/LED |
| 3 | — | UART RX (debug) | I/O | Available |
| 4 | SD card D1 | Built-in LED, LoRa IRQ | I/O | SD/LED/LoRa |
| 5 | Camera Y2 | LoRa CS (conflicts!) | I/O | Camera/LoRa |
| 12 | SD card D2 | LoRa CS, PIR power | I/O | SD/LoRa |
| 13 | SD card D3 | — | I/O | SD |
| 14 | SD card CLK | Charging control | I/O | SD/Power |
| 15 | SD card CMD | Charging LED | I/O | SD/Power |
| 16 | — | LoRa RST, IR LED, Servo | I/O | LoRa/IR/Servo |
| 17 | — | PIR power, Servo | I/O | Available |
| 18 | Camera Y3 | LoRa SCK (conflicts!) | I/O | Camera/LoRa |
| 19 | Camera Y4 | LoRa MISO (conflicts!) | I/O | Camera/LoRa |
| 21 | Camera Y5 | — | I/O | Camera |
| 22 | Camera PCLK | — | I/O | Camera |
| 23 | Camera HREF | LoRa MOSI (conflicts!) | I/O | Camera/LoRa |
| 25 | Camera VSYNC | — | I/O | Camera |
| 26 | Camera SIOD (I2C SDA) | BME280 SDA, LoRa DIO0 | I/O | Camera/I2C/LoRa |
| 27 | Camera SIOC (I2C SCL) | BME280 SCL | I/O | Camera/I2C |
| 32 | Camera PWDN | Solar voltage (ADC, compatible) | I/O | Camera/Analog |
| 33 | — | Battery/Light sensor (ADC) | I/O | Available |
| 34 | Camera Y8 | Battery voltage (input-only) | Input | Camera/Analog |
| 35 | Camera Y9 | — | Input | Camera |
| 36 | Camera Y6 | — | Input | Camera |
| 39 | Camera Y7 | — | Input | Camera |

## Critical Conflicts

### 1. LoRa vs Camera (UNRESOLVABLE on AI-Thinker)

**The Problem**: LoRa SPI pins directly conflict with camera data pins.

| Feature | GPIO Pin | Conflict |
|---------|----------|----------|
| LoRa SCK | 18 | Camera Y3 (data bit 3) |
| LoRa MISO | 19 | Camera Y4 (data bit 4) |
| LoRa MOSI | 23 | Camera HREF (sync signal) |
| LoRa DIO0 | 26 | Camera SIOD (I2C data) |
| LoRa CS | 5 or 12 | Camera Y2 or SD card D2 |

**Impact**: Camera produces corrupted images or fails to initialize when LoRa is active.

**Resolution**: 
- ✅ Use ESP32-S3-CAM (has more GPIO pins)
- ✅ Use external SPI GPIO expander (MCP23S17)
- ❌ Cannot use both on AI-Thinker without hardware modification

### 2. SD Card vs LoRa (SHARED SPI BUS)

**The Problem**: SD card and LoRa share SPI pins but need different chip selects.

| Feature | MOSI | MISO | SCK | CS |
|---------|------|------|-----|-----|
| SD Card | 15 | 2 | 14 | 13 |
| LoRa | 15 | 2 | 14 | 12 |

**Impact**: Can work with careful SPI bus management but has timing conflicts.

**Resolution**:
- ✅ Share SPI bus with different CS pins (GPIO 12 for LoRa, GPIO 13 for SD)
- ✅ Use software CS control to prevent conflicts
- ⚠️ Requires careful synchronization in code
- ⚠️ Camera must be disabled for this to work

### 3. Solar/Battery Monitoring vs Camera

**The Problem**: Voltage monitoring shares input-only pins with camera.

| Feature | GPIO Pin | Conflict |
|---------|----------|----------|
| Battery voltage | 34 | Camera Y8 (data bit 8) |
| Solar voltage | 32 | Camera PWDN (power control) |

**Impact**: 
- GPIO 34: Both camera and battery use same pin (input-only, **READ-ONLY sharing OK**)
- GPIO 32: Camera PWDN is output, solar voltage is input (**INCOMPATIBLE**)

**Resolution**:
- ✅ GPIO 34: Can share if camera doesn't use during ADC reads
- ❌ GPIO 32: Must choose between solar monitoring and camera power control
- ✅ Alternative: Use GPIO 33 for solar voltage monitoring

### 4. PIR Sensor vs UART Debug

**The Problem**: PIR on GPIO 1 conflicts with UART TX.

**Impact**: Cannot use serial debugging when PIR is active.

**Resolution**:
- ✅ Disable serial debugging in production
- ✅ Use GPIO 3 (UART RX) for PIR if bidirectional debug not needed
- ✅ Use telnet/WiFi debugging instead

### 5. LED Indicators vs SD Card

**The Problem**: LED pins conflict with SD card data lines.

| Feature | GPIO Pin | Conflict |
|---------|----------|----------|
| Power LED | 2 | SD card D0 |
| Charging LED | 4 | SD card D1 |
| Built-in LED | 4 | SD card D1 |

**Impact**: Cannot use LEDs when SD card is enabled.

**Resolution**:
- ✅ Disable LEDs when SD card is needed
- ✅ Use GPIO 16/17 for LEDs instead
- ✅ Use WiFi/network status instead of physical LEDs

## Feature Compatibility Matrix

| Feature Combination | AI-Thinker Compatibility | Notes |
|---------------------|-------------------------|-------|
| Camera + SD Card | ✅ COMPATIBLE | Default configuration |
| Camera + LoRa | ❌ INCOMPATIBLE | Critical pin conflicts |
| Camera + Battery Monitor | ⚠️ PARTIAL | Shared input pin, works with coordination |
| Camera + Solar Monitor | ⚠️ PARTIAL | GPIO 32 conflict, use GPIO 33 instead |
| Camera + PIR | ✅ COMPATIBLE | GPIO 1 available |
| Camera + I2C Sensors | ✅ COMPATIBLE | Shares I2C bus with camera |
| SD Card + LoRa | ⚠️ PARTIAL | Shared SPI, needs careful management |
| LoRa + Sensors | ✅ COMPATIBLE | If camera is disabled |
| All Features | ❌ IMPOSSIBLE | Insufficient GPIO pins |

## Available Pins Summary

### When Camera is Enabled:
- **GPIO 1**: PIR sensor, UART TX
- **GPIO 3**: UART RX
- **GPIO 16**: LoRa RST, IR LED, Servo control
- **GPIO 17**: PIR power, Servo control
- **GPIO 33**: Battery/Light sensor (ADC)

### When Camera is Disabled:
All camera data pins become available (but lose camera functionality):
- GPIO 5, 18, 19, 21, 22, 23, 25, 34, 35, 36, 39
- Enables full LoRa + sensor deployment

### When SD Card is Disabled:
- **GPIO 2**: Power LED
- **GPIO 4**: Built-in LED
- **GPIO 12**: LoRa CS
- **GPIO 13**: Available
- **GPIO 14**: Charging control
- **GPIO 15**: Charging LED

## Configuration Templates

### Template 1: Wildlife Camera (Maximum Features)
**Hardware**: AI-Thinker ESP32-CAM

```cpp
// Features
#define CAMERA_ENABLED true
#define SD_CARD_ENABLED true
#define PIR_SENSOR_ENABLED true
#define BME280_SENSOR_ENABLED true
#define BATTERY_MONITOR_ENABLED true

// Disabled features
#define LORA_ENABLED false
#define LED_INDICATORS_ENABLED false
```

**Available GPIO**: 1, 3, 16, 17, 33

### Template 2: LoRa Mesh Node (No Camera)
**Hardware**: AI-Thinker ESP32-CAM

```cpp
// Features
#define LORA_ENABLED true
#define SD_CARD_ENABLED true
#define PIR_SENSOR_ENABLED true
#define BME280_SENSOR_ENABLED true
#define BATTERY_MONITOR_ENABLED true
#define SOLAR_MONITOR_ENABLED true

// Disabled features
#define CAMERA_ENABLED false
```

**Available GPIO**: Many camera pins now free for sensors

### Template 3: Full-Featured System
**Hardware**: ESP32-S3-CAM (REQUIRED)

```cpp
// All features can be enabled
#define CAMERA_ENABLED true
#define SD_CARD_ENABLED true
#define LORA_ENABLED true
#define PIR_SENSOR_ENABLED true
#define BME280_SENSOR_ENABLED true
#define BATTERY_MONITOR_ENABLED true
#define SOLAR_MONITOR_ENABLED true
#define LED_INDICATORS_ENABLED true
```

**Available GPIO**: 40+ pins on ESP32-S3

## Pin Sharing Strategies

### 1. Time-Division Multiplexing
Share pins between features that don't operate simultaneously:

```cpp
// Example: Share GPIO 34 between camera and battery monitoring
void readBatteryVoltage() {
    if (camera.isCapturing()) {
        return; // Skip reading during capture
    }
    // Safe to read when camera is idle
    float voltage = analogRead(BATTERY_VOLTAGE_PIN);
}
```

### 2. Conditional Compilation
Enable features based on use case:

```cpp
#if defined(LORA_ENABLED) && LORA_ENABLED
    #define CAMERA_ENABLED false  // Auto-disable camera
#endif
```

### 3. External GPIO Expansion
Use I2C/SPI GPIO expanders:

- **MCP23017**: I2C, 16 additional GPIO pins
- **MCP23S17**: SPI, 16 additional GPIO pins
- **PCF8574**: I2C, 8 additional GPIO pins

### 4. Multiplexer ICs
Use analog multiplexers for ADC channels:

- **CD4051**: 8-channel analog multiplexer
- Share single ADC pin for multiple voltage monitors

## Recommended Solutions

### For Current AI-Thinker Users:

1. **Choose Your Primary Function**:
   - Wildlife camera with storage → Disable LoRa
   - Mesh networking → Disable camera
   - Cannot have both without hardware upgrade

2. **Use Pin Sharing Carefully**:
   - GPIO 34: Camera Y8 / Battery monitoring (coordinate timing)
   - GPIO 26/27: Camera I2C / Sensor I2C (same bus, OK)
   - GPIO 32: Camera PWDN / Solar voltage (choose one)

3. **Add GPIO Expander** (if needed):
   - Connect MCP23017 to I2C bus (GPIO 26/27)
   - Gain 16 additional GPIO pins for LEDs, sensors, etc.

### For New Deployments:

1. **Upgrade to ESP32-S3-CAM**:
   - 45 GPIO pins available
   - All features work simultaneously
   - USB OTG support
   - Better processing power

2. **Use Dedicated Boards**:
   - ESP32-CAM for camera applications
   - Standard ESP32 DevKit for LoRa mesh nodes
   - Coordinate via WiFi/LoRa network

## Testing Pin Conflicts

### Compile-Time Validation
The firmware includes automatic conflict detection:

```cpp
// Example from config.h
#if defined(LORA_ENABLED) && LORA_ENABLED && \
    defined(CAMERA_ENABLED) && CAMERA_ENABLED && \
    defined(CAMERA_MODEL_AI_THINKER)
    #error "LoRa and Camera cannot coexist on AI-Thinker ESP32-CAM"
#endif
```

### Runtime Detection
Use the BoardDetector class to check available features:

```cpp
#include "hal/board_detector.h"

if (!BoardDetector::hasFeature(BoardDetector::FEATURE_LORA)) {
    Serial.println("LoRa not available on this board configuration");
}
```

## Migration Path

### From AI-Thinker to ESP32-S3-CAM:

1. **Pin Mapping Changes**:
   - Update all pin definitions in `config.h`
   - Review camera pin assignments
   - Update LoRa SPI pins

2. **Code Changes**:
   - Update `platformio.ini` environment
   - Change board definition to `esp32s3dev`
   - Update camera model to `CAMERA_MODEL_ESP32S3_EYE`

3. **Testing**:
   - Verify camera functionality
   - Test LoRa communication
   - Validate all sensors
   - Check power consumption

## Troubleshooting

### Camera Not Working
- ✓ Check if LoRa is enabled (conflict)
- ✓ Verify PWDN pin not used for other purpose
- ✓ Ensure camera data pins (GPIO 5, 18, 19, 23) are free

### LoRa Communication Fails
- ✓ Check if camera is enabled (conflict)
- ✓ Verify SPI pins not shared with active SD card
- ✓ Check CS pin not conflicting with SD card

### ADC Readings Incorrect
- ✓ Ensure ADC not read during camera capture
- ✓ Check voltage divider values
- ✓ Verify no other device using same GPIO

### Boot Failures
- ✓ Check GPIO 0, 2, 12, 15 states during boot
- ✓ Ensure pull-up/pull-down resistors correct
- ✓ Verify flash voltage selection (GPIO 12)

## References

- [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [AI-Thinker ESP32-CAM Schematic](http://wiki.ai-thinker.com/esp32-cam)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- WildCAM Project: `firmware/hal/board_detector.h`
- WildCAM Project: `ESP32WildlifeCAM-main/include/pins.h`

## Version History

- **v1.0** (2025-10-16): Initial comprehensive pin conflict documentation
