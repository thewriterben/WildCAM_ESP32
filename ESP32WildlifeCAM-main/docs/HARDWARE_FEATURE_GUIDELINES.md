# Hardware Feature Implementation Guidelines

## Overview

This document provides comprehensive guidelines for implementing new hardware features on ESP32-based WildCAM platforms, with a focus on avoiding pin conflicts and ensuring compile-time safety.

## Table of Contents

1. [Quick Reference: Available GPIO Pins](#quick-reference-available-gpio-pins)
2. [Before Adding New Features](#before-adding-new-features)
3. [Pin Conflict Resolution Strategy](#pin-conflict-resolution-strategy)
4. [Board-Specific Limitations](#board-specific-limitations)
5. [Compile-Time Safety Checks](#compile-time-safety-checks)
6. [Best Practices](#best-practices)
7. [Examples](#examples)

## Quick Reference: Available GPIO Pins

### AI-Thinker ESP32-CAM

**Available GPIO Pins (Limited):**
- GPIO 1, 3 (UART TX/RX - use with caution, needed for debugging)
- GPIO 12, 13, 15, 16 (available when SD card disabled)
- GPIO 2 (built-in LED, can be repurposed if SD card disabled)

**Reserved Pins (DO NOT USE):**
- GPIO 0, 5, 18-27, 32, 34-36, 39: Camera interface (OV2640)
- GPIO 2, 4, 12-15: SD card interface (when SD_CARD_ENABLED)
- GPIO 6-11: Internal flash memory (NEVER use these)

**Input-Only Pins (ADC only):**
- GPIO 34, 35, 36, 39: Cannot be used for output or internal pull-up/pull-down

**Boot Mode Pins (Special Handling Required):**
- GPIO 0: Must be HIGH during boot (connected to camera XCLK)
- GPIO 12: Boot failure if HIGH during boot (has internal pull-down)
- GPIO 2, 15: Affect boot mode selection (have internal pull-ups)

### ESP32-S3-CAM (Recommended)

**Available GPIO Pins (Abundant):**
- GPIO 1-21, 33-48 (depending on specific board variant)
- More flexible pin assignments
- USB OTG on GPIO 19, 20
- Additional ADC channels

**Advantages:**
- ✅ Can run Camera + LoRa + Servos + All Sensors simultaneously
- ✅ No pin conflicts between major peripherals
- ✅ More RAM (8MB PSRAM typical)
- ✅ Faster CPU and better AI/ML performance

## Before Adding New Features

### Step 1: Check Existing Pin Assignments

**Always review these files first:**
1. `include/pins.h` - All pin definitions and conflicts documented
2. `include/config.h` - Feature flags and compile-time checks
3. Pin conflict notes in pins.h (lines 114-164)

### Step 2: Verify Pin Availability

**Check the following:**
- Is the pin physically available on your board?
- Is the pin already used by camera, SD card, or flash?
- Does the pin have special boot mode requirements?
- Is the pin input-only or bidirectional?
- Are there existing conditional assignments?

### Step 3: Review Feature Conflicts

**Common conflicts:**
- **Camera + LoRa**: Share SPI pins (GPIO 18, 19, 23)
- **SD Card + Custom LEDs**: Share GPIO 2, 4, 12-15
- **Servos + LoRa**: Both need GPIO 16, 17
- **IR LED + LoRa CS**: Both want GPIO 16

### Step 4: Document Your Changes

**Required documentation:**
1. Add pin definition with clear comment in `include/pins.h`
2. Add feature flag in `include/config.h`
3. Add compile-time conflict check (see examples below)
4. Update pin conflict notes section
5. Document in feature-specific README if applicable

## Pin Conflict Resolution Strategy

### Strategy 1: Conditional Pin Usage

Use pins conditionally based on enabled features:

```cpp
// In pins.h
#if defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  // SD card uses GPIO 2 for data
  #define SD_MMC_D0 2
#else
  // When SD disabled, GPIO 2 available for LED
  #define STATUS_LED_PIN 2
#endif
```

### Strategy 2: Shared Pin Functions

Some pins can be safely shared:

```cpp
// GPIO 34 is input-only, can be shared for multiple ADC reads
#define BATTERY_VOLTAGE_PIN 34  // ADC input
#define LIGHT_SENSOR_PIN 34     // Also ADC input (read at different times)
```

### Strategy 3: Time-Multiplexing

Switch pin function at runtime:

```cpp
// Use GPIO 16 for different purposes at different times
#if defined(ADAPTIVE_PIN_USAGE)
  #define MULTI_PURPOSE_PIN 16
  // Configure as IR LED during night
  // Configure as servo control during day
#endif
```

### Strategy 4: Board Upgrade Recommendation

For incompatible features, recommend hardware upgrade:

```cpp
#if defined(CAMERA_MODEL_AI_THINKER) && defined(NEED_LORA_AND_SERVO)
  #error "AI-Thinker ESP32-CAM cannot support this feature combination. Upgrade to ESP32-S3-CAM."
#endif
```

## Board-Specific Limitations

### AI-Thinker ESP32-CAM

**Supported Configurations:**
- ✅ Camera + SD Card + Motion Detection + Environmental Sensors
- ✅ Camera + WiFi + Basic Power Management
- ✅ Camera + I2C Sensors (BME280, etc.)

**NOT Supported (Pin Conflicts):**
- ❌ Camera + LoRa (SPI pin conflicts)
- ❌ Camera + Servos + LoRa (insufficient pins)
- ❌ Full peripheral suite (need ESP32-S3-CAM)

**Recommended Use Cases:**
- Budget-friendly basic wildlife monitoring
- Educational projects
- Simple motion-triggered camera traps
- Locations with WiFi access

### ESP32-S3-CAM

**Supported Configurations:**
- ✅ ALL features simultaneously
- ✅ Camera + LoRa Mesh + Servos + All Sensors
- ✅ Advanced AI/ML with more RAM
- ✅ USB OTG for easy debugging

**Recommended Use Cases:**
- Production deployments
- Remote locations with LoRa mesh networking
- Pan/tilt camera applications
- Advanced AI/ML processing
- Research-grade monitoring systems

## Compile-Time Safety Checks

### Adding New Conflict Checks

Always add compile-time checks for new features in `include/config.h`:

```cpp
// Template for new feature conflict check
#if defined(YOUR_NEW_FEATURE) && YOUR_NEW_FEATURE
  #if defined(CONFLICTING_FEATURE) && CONFLICTING_FEATURE
    #if defined(YOUR_NEW_FEATURE_PIN) && defined(CONFLICTING_PIN) && (YOUR_NEW_FEATURE_PIN == CONFLICTING_PIN)
      #error "YOUR_NEW_FEATURE conflicts with CONFLICTING_FEATURE (both use GPIO X). Disable one or use ESP32-S3-CAM."
    #endif
  #endif
#endif
```

### Example Checks

**Check for LoRa + Camera conflict:**
```cpp
#if defined(CAMERA_MODEL_AI_THINKER) && defined(LORA_ENABLED) && LORA_ENABLED
  #error "LoRa and Camera cannot be enabled simultaneously on AI-Thinker ESP32-CAM due to pin conflicts (GPIO 18, 19, 23). Use ESP32-S3-CAM or disable one feature."
#endif
```

**Check for SD + LED conflict:**
```cpp
#if defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  #if defined(CHARGING_LED_PIN) && CHARGING_LED_PIN == 2
    #error "CHARGING_LED_PIN conflicts with SD card data line (GPIO 2). Reassign LED or disable SD card."
  #endif
#endif
```

## Best Practices

### DO:
1. ✅ Always check `pins.h` before adding features
2. ✅ Use only documented available GPIO pins
3. ✅ Add compile-time checks for conflicts
4. ✅ Document all pin assignments clearly
5. ✅ Test on actual hardware with conflicts intentionally enabled
6. ✅ Recommend ESP32-S3-CAM when needed
7. ✅ Use conditional compilation for board-specific code
8. ✅ Consider shared pin strategies when possible

### DON'T:
1. ❌ Use GPIO 6-11 (connected to flash, will brick device)
2. ❌ Assume pins are free without checking documentation
3. ❌ Add features without conflict checks
4. ❌ Use boot-sensitive pins without understanding implications
5. ❌ Skip documentation updates
6. ❌ Force incompatible features on AI-Thinker ESP32-CAM
7. ❌ Ignore compiler warnings about pin conflicts

## Examples

### Example 1: Adding a Temperature Sensor

**Good Implementation:**
```cpp
// In pins.h
// Temperature sensor uses I2C, which shares camera I2C bus (safe)
#define TEMP_SENSOR_SDA 26  // Shared with camera SIOD
#define TEMP_SENSOR_SCL 27  // Shared with camera SIOC
#define TEMP_SENSOR_ADDR 0x48

// In config.h
#define TEMP_SENSOR_ENABLED true

// No conflict check needed - I2C can be shared
```

### Example 2: Adding a Status LED

**Good Implementation with Conflict Check:**
```cpp
// In pins.h
#if defined(SD_CARD_ENABLED) && !SD_CARD_ENABLED
  #define STATUS_LED_PIN 2  // Use GPIO 2 when SD card disabled
#else
  #define STATUS_LED_PIN 33  // Fallback pin when SD enabled
#endif

// In config.h
#define STATUS_LED_ENABLED true

// Conflict check
#if defined(STATUS_LED_ENABLED) && defined(SD_CARD_ENABLED)
  #if STATUS_LED_PIN == 2 && SD_CARD_ENABLED
    #warning "STATUS_LED on GPIO 2 conflicts with SD card. LED will be disabled or moved."
  #endif
#endif
```

### Example 3: Adding LoRa Support

**Good Implementation with Board Check:**
```cpp
// In pins.h
#if defined(CAMERA_MODEL_AI_THINKER)
  // LoRa disabled by default on AI-Thinker due to conflicts
  #define LORA_SUPPORTED 0
#elif defined(CAMERA_MODEL_ESP32_S3)
  // LoRa fully supported on ESP32-S3
  #define LORA_SUPPORTED 1
  #define LORA_SCK_PIN 18
  #define LORA_MISO_PIN 19
  #define LORA_MOSI_PIN 23
  #define LORA_CS_PIN 5
  #define LORA_RST_PIN 14
  #define LORA_DIO0_PIN 2
#endif

// In config.h
#if defined(CAMERA_MODEL_AI_THINKER) && defined(LORA_ENABLED) && LORA_ENABLED
  #error "LoRa requires ESP32-S3-CAM. AI-Thinker ESP32-CAM has insufficient GPIO pins."
#endif
```

## Testing Your Changes

### Compile-Time Tests

1. **Test valid configuration:**
   - Compile with your new feature enabled
   - Verify no errors

2. **Test conflict detection:**
   - Enable conflicting features
   - Verify compilation fails with clear error
   - Check error message is helpful

3. **Test on different boards:**
   - Test on AI-Thinker ESP32-CAM
   - Test on ESP32-S3-CAM if available
   - Verify board-specific behavior

### Runtime Tests

1. **Pin functionality test:**
   - Upload code to hardware
   - Verify pin outputs/inputs work correctly
   - Check for interference with other features

2. **Conflict testing:**
   - Enable features simultaneously
   - Verify no electrical conflicts
   - Monitor for unexpected behavior

## Need Help?

- **Review existing implementations** in `include/pins.h`
- **Check documentation** in `ESP32WildlifeCAM-main/docs/`
- **Ask in GitHub Issues** if unsure about pin assignment
- **Refer to ESP32 datasheet** for electrical characteristics

## Summary Checklist

Before submitting code with new hardware features:

- [ ] Reviewed `include/pins.h` for existing assignments
- [ ] Verified pin availability on target board
- [ ] Checked for conflicts with camera, SD, flash
- [ ] Added pin definitions with clear comments
- [ ] Added feature flag in config.h
- [ ] Added compile-time conflict checks
- [ ] Updated pin conflict documentation
- [ ] Tested compilation with conflicts enabled
- [ ] Tested on actual hardware (if available)
- [ ] Documented board-specific limitations
- [ ] Recommended ESP32-S3-CAM if needed
- [ ] Updated relevant README files

---

*For questions or clarifications, please open a GitHub issue with the label "hardware".*
