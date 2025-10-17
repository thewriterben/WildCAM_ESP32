# GPIO Pin Conflicts Documentation

## Overview

This document provides a comprehensive matrix of GPIO pin conflicts on ESP32-CAM boards, specifically focusing on the AI-Thinker ESP32-CAM module. Understanding these conflicts is critical for hardware integration and preventing runtime failures.

## Table of Contents

1. [Complete GPIO Pin Matrix](#complete-gpio-pin-matrix)
2. [Conflict Matrix by Feature](#conflict-matrix-by-feature)
3. [Detailed Pin Analysis](#detailed-pin-analysis)
4. [Resolution Strategies](#resolution-strategies)
5. [Board-Specific Capabilities](#board-specific-capabilities)

---

## Complete GPIO Pin Matrix

### AI-Thinker ESP32-CAM Pin Assignments

| GPIO | Primary Use | Secondary Use | Conflicts | Input/Output | Boot Sensitive | Notes |
|------|-------------|---------------|-----------|--------------|----------------|-------|
| 0 | Camera XCLK | - | - | I/O | YES - Must be HIGH | Boot mode pin |
| 1 | UART TX | PIR Motion | - | I/O | NO | Debug serial TX |
| 2 | SD Card D0 | Power LED | SD Card | I/O | YES - Boot mode | Built-in LED |
| 3 | UART RX | - | - | I/O | NO | Debug serial RX |
| 4 | SD Card D1 | Power LED | SD Card | I/O | NO | Built-in flash LED |
| 5 | Camera Y2 | - | LoRa CS | I/O | NO | Camera data bit |
| 6-11 | **FLASH** | - | **ALL** | I/O | **CRITICAL** | **NEVER USE - WILL BRICK** |
| 12 | SD Card D2 | PIR Power | SD Card | I/O | YES - Must be LOW | Internal pull-down |
| 13 | SD Card D3 | - | SD Card | I/O | NO | - |
| 14 | SD Card CLK | Charge Control | SD Card | I/O | NO | - |
| 15 | SD Card CMD | Charge LED | SD Card | I/O | YES - Boot mode | Internal pull-up |
| 16 | IR LED / LoRa CS | Servo | LoRa, Servo | I/O | NO | Multi-conflict pin |
| 17 | LoRa RST | PIR Power / Servo | LoRa, Servo | I/O | NO | - |
| 18 | Camera Y3 | LoRa SCK | LoRa | I/O | NO | Camera data bit |
| 19 | Camera Y4 | LoRa MISO | LoRa | I/O | NO | Camera data bit |
| 21 | Camera Y5 | - | - | I/O | NO | Camera data bit |
| 22 | Camera PCLK | I2S SD | Audio | I/O | NO | Camera pixel clock |
| 23 | Camera HREF | LoRa MOSI | LoRa | I/O | NO | Camera H-ref |
| 25 | Camera VSYNC | I2S WS | Audio | I/O | NO | Camera V-sync |
| 26 | Camera SIOD (I2C SDA) | BME280 SDA / I2S SCK / LoRa DIO0 | LoRa, Audio | I/O | NO | Shared I2C bus OK |
| 27 | Camera SIOC (I2C SCL) | BME280 SCL | - | I/O | NO | Shared I2C bus OK |
| 32 | Camera PWDN | Solar Voltage | - | I/O | NO | Shared with ADC OK |
| 33 | - | Light Sensor | - | I/O | NO | Available |
| 34 | Camera Y8 | Battery Voltage | Voltage Monitor | Input Only | NO | Input-only ADC |
| 35 | Camera Y9 | - | - | Input Only | NO | Input-only ADC |
| 36 | Camera Y6 | - | - | Input Only | NO | Input-only ADC |
| 39 | Camera Y7 | - | - | Input Only | NO | Input-only ADC |

---

## Conflict Matrix by Feature

### Camera vs. Other Features

#### ✅ **Compatible** (Can work simultaneously)

| Feature | Shared Pins | Strategy | Notes |
|---------|-------------|----------|-------|
| **WiFi** | None | - | No conflicts, fully compatible |
| **SD Card** | GPIO 2, 4, 12-15 | Dedicated interface | Compatible - SD uses separate pins |
| **PIR Motion** | GPIO 1 (UART TX) | Shared UART | Compatible if debug disabled |
| **I2C Sensors (BME280)** | GPIO 26, 27 | Shared I2C bus | Compatible - same I2C bus as camera |
| **Battery Monitoring** | GPIO 34 | Input-only ADC | Compatible - both input uses |
| **Solar Monitoring** | GPIO 32 | Shared with PWDN | Compatible - PWDN is output, solar is input |

#### ❌ **Incompatible** (Pin conflicts)

| Feature | Conflicting Pins | Reason | Resolution |
|---------|-----------------|--------|------------|
| **LoRa SPI** | GPIO 5, 18, 19, 23 | LoRa SPI overlaps camera data pins | Use ESP32-S3-CAM or disable camera |
| **LoRa Interrupt** | GPIO 26 | LoRa DIO0 conflicts with camera I2C | Reassign or use ESP32-S3-CAM |
| **Audio I2S** | GPIO 22, 25, 26 | I2S overlaps camera sync pins | Reassign I2S or use ESP32-S3-CAM |

### SD Card vs. Other Features

| Feature | Conflicting Pins | Compatible? | Notes |
|---------|-----------------|-------------|-------|
| **Power LED** | GPIO 2, 4 | ⚠️ Partial | LED can interfere with SD operations |
| **Charging LED** | GPIO 2, 15 | ❌ No | Direct conflict with SD data lines |
| **Charging Control** | GPIO 14 | ❌ No | Conflicts with SD clock |
| **Camera** | None | ✅ Yes | Fully compatible |

### LoRa vs. Other Features

| Feature | Conflicting Pins | Compatible? | Notes |
|---------|-----------------|-------------|-------|
| **Camera** | GPIO 5, 18, 19, 23, 26 | ❌ No | Multiple SPI conflicts with camera |
| **IR LED** | GPIO 16 | ❌ No | Both need GPIO 16 for CS/control |
| **Servos** | GPIO 16, 17 | ❌ No | Servo pins overlap LoRa CS and RST |
| **Audio I2S** | GPIO 26 | ❌ No | LoRa DIO0 conflicts with I2S SCK |

### Servo Control vs. Other Features

| Feature | Conflicting Pins | Compatible? | Notes |
|---------|-----------------|-------------|-------|
| **LoRa** | GPIO 16, 17 | ❌ No | Servo pins needed for LoRa CS/RST |
| **IR LED** | GPIO 16 | ❌ No | Pan servo conflicts with IR control |
| **PIR Power** | GPIO 17 | ⚠️ Partial | Tilt servo conflicts with PIR power |
| **Camera** | None | ✅ Yes | Compatible if LoRa disabled |

---

## Detailed Pin Analysis

### Critical Pins (DO NOT USE)

#### GPIO 6-11: Internal Flash Memory
- **Status**: ⛔ **ABSOLUTELY FORBIDDEN**
- **Reason**: Connected to SPI flash memory chip
- **Impact**: Using these pins will **brick your device**
- **Recovery**: Requires USB-to-serial programmer to reflash

### Boot-Sensitive Pins

#### GPIO 0: Boot Mode Selection
- **Primary Use**: Camera XCLK
- **Boot Requirement**: Must be HIGH during boot for normal operation
- **Conflict Risk**: LOW
- **Notes**: Safe to use as camera XCLK (output)

#### GPIO 2: Boot Mode / SD Card D0
- **Boot Requirement**: Must be in correct state for flash boot mode
- **Conflict Risk**: MEDIUM
- **Conflicts**: SD Card, Power LED
- **Notes**: Internal pull-up, affects boot mode selection

#### GPIO 12: Boot Voltage Selection
- **Boot Requirement**: Must be LOW during boot (pulls down flash voltage to 3.3V)
- **Conflict Risk**: MEDIUM
- **Notes**: Has internal pull-down; boot fails if HIGH

#### GPIO 15: Boot Mode / SD Card CMD
- **Boot Requirement**: Affects boot mode selection
- **Conflict Risk**: MEDIUM
- **Notes**: Internal pull-up, used for SD card CMD

### Input-Only Pins

#### GPIO 34, 35, 36, 39: ADC Input Only
- **Capabilities**: Input only, no output possible
- **Features**: ADC, cannot use internal pull-up/pull-down
- **Best Use**: Voltage monitoring, analog sensors
- **Camera Use**: Data lines Y6, Y7, Y8, Y9
- **Shared Use**: Can be shared for multiple ADC inputs (read at different times)

### Available GPIO Pins (AI-Thinker ESP32-CAM)

#### When SD Card is Disabled:
- **GPIO 2**: Available for LEDs or outputs
- **GPIO 12-15**: Available for general purpose I/O
- Total: ~5 usable pins

#### When SD Card is Enabled:
- **GPIO 1, 3**: Available but shared with UART (use with caution)
- **GPIO 16, 17**: Available for servos, sensors, or LoRa (if camera disabled)
- **GPIO 33**: Available for analog or digital I/O
- Total: ~4 usable pins

---

## Resolution Strategies

### Strategy 1: Conditional Feature Compilation

Disable conflicting features at compile time:

```cpp
// In firmware/config.h

// Disable LoRa on AI-Thinker when camera is enabled
#if defined(CAMERA_MODEL_AI_THINKER) && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #undef LORA_ENABLED
  #define LORA_ENABLED false
  #warning "LoRa disabled due to pin conflicts with camera on AI-Thinker ESP32-CAM"
#endif
```

### Strategy 2: Conditional Pin Assignments

Assign pins based on enabled features:

```cpp
// In include/pins.h

#if defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  // SD card enabled - use alternative pins
  #define STATUS_LED_PIN 33
#else
  // SD card disabled - can use SD pins
  #define STATUS_LED_PIN 2
#endif
```

### Strategy 3: Time-Multiplexed Pin Usage

Share pins between non-simultaneous functions:

```cpp
// Use GPIO 16 for different purposes
#define MULTI_PURPOSE_PIN 16

void setup() {
  if (nightMode) {
    // Configure as IR LED at night
    pinMode(MULTI_PURPOSE_PIN, OUTPUT);
  } else {
    // Configure as servo during day
    // Servo initialization
  }
}
```

### Strategy 4: Board Upgrade

For incompatible feature combinations, recommend ESP32-S3-CAM:

```cpp
#if defined(CAMERA_MODEL_AI_THINKER)
  #if defined(LORA_ENABLED) && LORA_ENABLED && defined(CAMERA_ENABLED) && CAMERA_ENABLED
    #error "Camera + LoRa requires ESP32-S3-CAM. AI-Thinker ESP32-CAM has insufficient GPIO pins."
  #endif
#endif
```

---

## Board-Specific Capabilities

### AI-Thinker ESP32-CAM

#### Specifications
- **Chip**: ESP32-DOWDQ6 (Dual-core @ 240 MHz)
- **PSRAM**: 4MB
- **Flash**: 4MB
- **Available GPIO**: ~5-8 pins (depending on SD card usage)
- **Camera**: OV2640 (fixed pins)

#### Supported Feature Combinations

✅ **Working Configurations:**
1. Camera + SD Card + WiFi + Motion Detection + I2C Sensors
2. Camera + WiFi + PIR + BME280 + Battery Monitoring
3. Camera + SD Card + Solar Charging + Environmental Sensors

❌ **NOT Supported:**
1. Camera + LoRa (SPI pin conflicts)
2. Camera + Audio I2S (pin conflicts)
3. Camera + LoRa + Servos (insufficient pins)
4. Full feature set simultaneously

#### Recommended Use Cases
- 📸 Basic wildlife camera trap
- 🏫 Educational projects
- 💰 Budget-friendly monitoring
- 📶 WiFi-accessible locations
- 🌡️ Environmental monitoring with camera

### ESP32-S3-CAM

#### Specifications
- **Chip**: ESP32-S3 (Dual-core @ 240 MHz)
- **PSRAM**: 8MB (2x more than AI-Thinker)
- **Flash**: 8MB
- **Available GPIO**: ~20+ pins
- **USB OTG**: Built-in USB debugging
- **Better AI**: More RAM for ML models

#### Supported Feature Combinations

✅ **ALL Configurations Supported:**
- Camera + LoRa + WiFi + SD Card
- Camera + Servos + LoRa + All Sensors
- Camera + Audio + LoRa + Motion Detection
- Full feature set simultaneously

#### Recommended Use Cases
- 🏭 Production deployments
- 🌲 Remote locations with LoRa mesh
- 🤖 Pan/tilt camera systems
- 🧠 Advanced AI/ML processing
- 🔬 Research-grade monitoring
- 📡 Satellite communication integration

---

## Compile-Time Conflict Detection

The firmware includes comprehensive compile-time checks to prevent invalid configurations. These checks are in `firmware/config.h`:

### Implemented Checks

1. **Camera + LoRa Conflict** (AI-Thinker)
   - Prevents: Simultaneous camera and LoRa on AI-Thinker
   - Error: Clear message directing to ESP32-S3-CAM

2. **SD Card + LED Conflicts**
   - Prevents: LED pin assignments that conflict with SD card
   - Action: Warning or error depending on severity

3. **Voltage Monitoring Conflicts**
   - Prevents: Using camera data pins for voltage monitoring
   - Recommendation: Use external ADC or ESP32-S3-CAM

4. **Servo + LoRa Conflicts**
   - Prevents: Enabling servos and LoRa simultaneously on AI-Thinker
   - Recommendation: Choose one or upgrade board

### Adding New Conflict Checks

When adding new features, add compile-time checks:

```cpp
// Template for new conflict check
#if defined(NEW_FEATURE_ENABLED) && NEW_FEATURE_ENABLED
  #if defined(CONFLICTING_FEATURE) && CONFLICTING_FEATURE
    #if NEW_FEATURE_PIN == CONFLICTING_PIN
      #error "NEW_FEATURE conflicts with CONFLICTING_FEATURE (both use GPIO X). Solution: [describe solution]"
    #endif
  #endif
#endif
```

---

## Quick Decision Guide

### "Can I add this feature to AI-Thinker ESP32-CAM?"

Use this flowchart:

```
Does it need GPIO pins?
├─ NO → ✅ Safe to add (software only)
└─ YES → Are the pins currently free?
    ├─ YES → Check boot sensitivity
    │   ├─ Not boot sensitive → ✅ Safe to add
    │   └─ Boot sensitive → ⚠️  Add with caution
    └─ NO → Feature conflicts detected
        ├─ Can use conditional compilation? → ⚠️  Add with #ifdef guards
        ├─ Can share pins safely? → ⚠️  Document shared usage
        └─ Neither possible → ❌ Recommend ESP32-S3-CAM
```

---

## Common Questions

### Q: Why can't I use LoRa with the camera on AI-Thinker ESP32-CAM?

**A:** LoRa requires SPI pins (GPIO 18, 19, 23) which are hardwired to the camera's data lines (Y3, Y4, HREF). These pins cannot be remapped. The ESP32-S3-CAM has more GPIO pins and can support both simultaneously.

### Q: Can I monitor battery voltage while using the camera?

**A:** Partially. GPIO 34 is used for camera Y8 data, but since it's input-only, you can share it for ADC voltage monitoring if you read voltage when camera is idle. However, this is not recommended for production use. Better solution: Use external I2C ADC or ESP32-S3-CAM.

### Q: How many servos can I control with AI-Thinker ESP32-CAM?

**A:** Maximum 2-3 servos when SD card is disabled (using GPIO 12, 13, 15, 16, 17). However, this limits other features. For pan/tilt systems, ESP32-S3-CAM is strongly recommended.

### Q: What happens if I use GPIO 6-11?

**A:** ⚠️ **Your device will be bricked!** These pins are connected to the internal SPI flash. Using them will corrupt the firmware and require a USB-to-serial programmer to recover.

### Q: Can I use UART pins (GPIO 1, 3) for other purposes?

**A:** Yes, but with caution. These are used for serial debugging. If you disable serial output, you can use them for other purposes. Recommended: Keep GPIO 3 (RX) free for emergency debugging.

---

## Best Practices Summary

### ✅ DO:
1. Check this document before adding hardware features
2. Add compile-time conflict checks for new features
3. Test configurations on actual hardware
4. Document all pin assignments clearly
5. Recommend ESP32-S3-CAM for complex projects
6. Use conditional compilation for conflicting features
7. Share pins only when electrically safe
8. Read datasheets and verify compatibility

### ❌ DON'T:
1. Use GPIO 6-11 (flash) under any circumstances
2. Assume pins are free without checking
3. Enable camera + LoRa on AI-Thinker
4. Mix output and input on shared pins
5. Ignore boot-sensitive pin requirements
6. Skip compile-time safety checks
7. Force incompatible features
8. Forget to document pin assignments

---

## Additional Resources

- **Hardware Guidelines**: `docs/HARDWARE_FEATURE_GUIDELINES.md`
- **Hardware Selection Guide**: `docs/HARDWARE_SELECTION_GUIDE.md`
- **Pin Definitions**: `firmware/include/pins.h`
- **Configuration File**: `firmware/config.h`
- **ESP32 Technical Reference**: [Espressif Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

---

## Support

For questions or issues related to GPIO conflicts:
1. Check this documentation first
2. Review existing issues on GitHub
3. Open a new issue with label `hardware` and `gpio-conflict`
4. Provide: Board type, enabled features, error messages

---

**Last Updated**: 2025-10-16  
**Maintainer**: WildCAM ESP32 Team  
**License**: MIT
