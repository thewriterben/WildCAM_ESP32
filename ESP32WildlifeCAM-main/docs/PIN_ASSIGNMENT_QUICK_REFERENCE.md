# Hardware Pin Assignment Quick Reference

> 📖 **Full Guidelines**: See [HARDWARE_FEATURE_GUIDELINES.md](HARDWARE_FEATURE_GUIDELINES.md) for comprehensive documentation.

## Quick Pin Availability Check

### AI-Thinker ESP32-CAM

```
✅ AVAILABLE (6 pins):
   GPIO 1, 3    - UART TX/RX (use with caution - needed for debugging)
   GPIO 12-16   - General purpose (when SD card disabled)
   GPIO 2       - Built-in LED (when SD card disabled)

❌ RESERVED - DO NOT USE:
   GPIO 0, 5, 18-27, 32, 34-36, 39  - Camera interface
   GPIO 2, 4, 12-15                  - SD card (when enabled)
   GPIO 6-11                         - Internal flash (WILL BRICK DEVICE)

⚠️  BOOT SENSITIVE:
   GPIO 0       - Must be HIGH at boot
   GPIO 12      - Must be LOW at boot (has pull-down)
   GPIO 2, 15   - Affect boot mode (have pull-ups)

🔍 INPUT ONLY:
   GPIO 34-39   - Cannot be used for output
```

### ESP32-S3-CAM

```
✅ ABUNDANT GPIO:
   GPIO 1-21, 33-48 (board-dependent)
   USB OTG: GPIO 19, 20
   More flexible - NO CONFLICTS!
```

## Feature Compatibility Matrix

### AI-Thinker ESP32-CAM

| Feature Combination | Status | Notes |
|---------------------|--------|-------|
| Camera + SD Card | ✅ Works | Basic configuration |
| Camera + WiFi | ✅ Works | No pin conflicts |
| Camera + PIR Motion | ✅ Works | Uses GPIO 1 |
| Camera + I2C Sensors | ✅ Works | Shares I2C bus |
| Camera + LoRa | ❌ Conflict | SPI pins overlap |
| Camera + Servos | ⚠️ Limited | Only 2-3 servos max |
| Camera + LoRa + Servos | ❌ Impossible | Need ESP32-S3-CAM |

### ESP32-S3-CAM

| Feature Combination | Status | Notes |
|---------------------|--------|-------|
| All features | ✅ Works | No conflicts! |

## Adding New Hardware: 5-Step Checklist

```
□ 1. Check pins.h for existing assignments
□ 2. Verify pin is in "AVAILABLE" list above
□ 3. Add pin definition to pins.h with comment
□ 4. Add compile-time conflict check to config.h
□ 5. Update documentation
```

## Compile-Time Check Template

```cpp
// In include/config.h

// Check: YOUR_FEATURE conflicts with EXISTING_FEATURE
#if defined(YOUR_FEATURE_ENABLED) && YOUR_FEATURE_ENABLED
  #if defined(EXISTING_FEATURE_ENABLED) && EXISTING_FEATURE_ENABLED
    #if defined(YOUR_FEATURE_PIN) && (YOUR_FEATURE_PIN == CONFLICTING_PIN)
      #error "YOUR_FEATURE conflicts with EXISTING_FEATURE (GPIO X). Disable one or use ESP32-S3-CAM."
    #endif
  #endif
#endif
```

## Common Mistakes to Avoid

```
❌ Using GPIO 6-11 (flash) → Bricks device
❌ Not checking pins.h first → Pin conflicts
❌ Enabling LoRa on AI-Thinker → Camera won't work
❌ Missing conflict checks → Runtime failures
❌ Not documenting pins → Maintenance nightmare
```

## When to Recommend ESP32-S3-CAM

Recommend ESP32-S3-CAM if user needs:
- ✅ Camera + LoRa simultaneously
- ✅ Camera + Pan/Tilt servos
- ✅ More than 2-3 additional sensors
- ✅ Better AI/ML performance
- ✅ USB OTG debugging

## Need Help?

1. **Read first**: [HARDWARE_FEATURE_GUIDELINES.md](HARDWARE_FEATURE_GUIDELINES.md)
2. **Check examples**: See pins.h lines 114-164 for conflict notes
3. **Test locally**: Use test/test_pin_conflict_checks.cpp
4. **Ask questions**: Open GitHub issue with "hardware" label

---

**Remember**: When in doubt, check the guidelines or ask! Pin conflicts can cause hard-to-debug issues.
