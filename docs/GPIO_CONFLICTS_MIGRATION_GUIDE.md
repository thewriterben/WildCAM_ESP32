# GPIO Conflicts - Migration Guide for Existing Deployments

## Overview

If you have existing WildCAM ESP32 deployments that may be affected by GPIO conflicts, this guide helps you:
1. Assess your current configuration
2. Identify potential conflicts
3. Migrate to a conflict-free configuration
4. Upgrade hardware if needed

## Step 1: Assess Your Current Configuration

### Check Your Hardware

```bash
# Look for board definition in platformio.ini
grep "board =" firmware/platformio.ini

# Outputs:
# board = esp32cam         → AI-Thinker ESP32-CAM
# board = esp32s3dev       → ESP32-S3-CAM
# board = esp32dev         → ESP32-DevKit (no camera)
```

### Check Your Features

```bash
# Check your config.h for enabled features
grep -E "ENABLED.*true" firmware/config.h

# Look for:
# CAMERA_ENABLED true
# LORA_ENABLED true
# SD_CARD_ENABLED true
# LED_INDICATORS_ENABLED true
# etc.
```

## Step 2: Identify Conflicts

### Run the Conflict Checker

```bash
cd firmware
./test_gpio_conflicts.sh
```

### Manual Conflict Check

Use this table to identify issues:

| Your Config | Board | Status | Action |
|------------|-------|--------|--------|
| Camera + LoRa | AI-Thinker | ❌ CONFLICT | Migrate (see below) |
| Camera + LoRa | ESP32-S3 | ✅ OK | No action |
| Camera + SD | Any | ✅ OK | No action |
| SD + LEDs | AI-Thinker | ❌ CONFLICT | Disable LEDs |
| Camera + Battery Mon | AI-Thinker | ⚠️ SHARED | Review timing |

## Step 3: Choose Migration Path

### Path A: Keep AI-Thinker, Disable Conflicting Features

**Best for**: Budget deployments, camera-focused use cases

**Steps**:
1. Choose primary feature (camera OR LoRa)
2. Disable conflicting features
3. Update config.h
4. Recompile and test

**Example**: Camera-only deployment
```bash
cd firmware
cp examples/configs/config_ai_thinker_camera_only.h config.h
# Edit WiFi credentials
nano config.h
pio run -e esp32cam_advanced -t upload
```

### Path B: Upgrade to ESP32-S3-CAM

**Best for**: Advanced features, no compromises

**Steps**:
1. Purchase ESP32-S3-CAM boards (~$20 each)
2. Update configuration
3. Reflash firmware
4. Test all features

**Hardware Required**:
- ESP32-S3-CAM board(s)
- USB-C cable (native USB)
- Same peripherals (sensors, LoRa module, etc.)

**Detailed Steps** (see Section 5)

### Path C: Mixed Deployment (Camera + Relay Nodes)

**Best for**: Mesh networks, cost-effective scaling

**Architecture**:
- Keep AI-Thinker boards as camera nodes (WiFi only)
- Add ESP32-DevKit boards as LoRa relay nodes
- Connect via WiFi + LoRa hybrid network

**Steps**:
1. Configure existing AI-Thinker boards for camera-only
2. Add ESP32-DevKit boards with LoRa for relaying
3. Set up hybrid network topology
4. Test data flow

## Step 4: Configuration Migration Examples

### Example 1: Camera + LoRa on AI-Thinker (INVALID)

**Current Config** (causes conflict):
```cpp
#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define LORA_ENABLED true          // ❌ Conflicts!
#define SD_CARD_ENABLED true
```

**Migration Option A** - Camera Only:
```cpp
#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
// #define LORA_ENABLED true       // ✗ Disabled
#define SD_CARD_ENABLED true
#define WIFI_ENABLED true           // Use WiFi instead
```

**Migration Option B** - Upgrade Hardware:
```cpp
#define CAMERA_MODEL_ESP32S3_EYE   // ✓ Changed to ESP32-S3
#define CAMERA_ENABLED true
#define LORA_ENABLED true           // ✓ Now works!
#define SD_CARD_ENABLED true
#define WIFI_ENABLED true
```

**Migration Option C** - Split Functions:
- Camera node (AI-Thinker):
  ```cpp
  #define CAMERA_MODEL_AI_THINKER
  #define CAMERA_ENABLED true
  // #define LORA_ENABLED true
  #define WIFI_ENABLED true         // Upload via WiFi
  ```
- Relay node (ESP32-DevKit):
  ```cpp
  #define CAMERA_ENABLED false
  #define LORA_ENABLED true          // Relay node
  #define WIFI_ENABLED true          // Gateway to WiFi
  ```

### Example 2: LEDs + SD Card (INVALID)

**Current Config** (causes conflict):
```cpp
#define CAMERA_MODEL_AI_THINKER
#define SD_CARD_ENABLED true
#define LED_INDICATORS_ENABLED true  // ❌ Conflicts with SD!
```

**Migrated Config**:
```cpp
#define CAMERA_MODEL_AI_THINKER
#define SD_CARD_ENABLED true
#define LED_INDICATORS_ENABLED false // ✗ Disabled
// Use network status instead of physical LEDs
#define NETWORK_STATUS_LED_ENABLED false
#define WEB_UI_STATUS_ENABLED true   // ✓ Web interface shows status
```

### Example 3: Solar Voltage Monitoring (WARNING)

**Current Config** (GPIO 32 conflict):
```cpp
#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define SOLAR_VOLTAGE_MONITORING_ENABLED true  // ⚠️ GPIO 32 conflict
```

**Migrated Config** - Use GPIO 33:
```cpp
#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define SOLAR_VOLTAGE_MONITORING_ENABLED true
#define SOLAR_VOLTAGE_PIN 33         // ✓ Changed from GPIO 32
```

Or use pins.h update:
```cpp
// In pins.h or config
#define SOLAR_VOLTAGE_PIN 33  // Instead of 32
```

## Step 5: Hardware Upgrade Procedure

### ESP32-S3-CAM Hardware Migration

#### Prerequisites
- [ ] ESP32-S3-CAM board
- [ ] USB-C cable
- [ ] All existing peripherals (sensors, LoRa, etc.)
- [ ] Backup of current config.h

#### Pin Mapping Changes

| Function | AI-Thinker Pin | ESP32-S3 Pin | Notes |
|----------|---------------|--------------|-------|
| LoRa CS | 12 (conflict) | 21 | Now available |
| LoRa RST | 16 | 38 | Dedicated pin |
| LoRa IRQ | 4 (conflict) | 39 | Dedicated pin |
| PIR | 1 | 1 | Same |
| Battery ADC | 34 | 4 | Different pin |
| Solar ADC | 32/33 | 2 | Different pin |
| LED | N/A | 2, 46 | Now available |

#### Migration Steps

1. **Backup Current Configuration**
   ```bash
   cd firmware
   cp config.h config.h.ai-thinker-backup
   ```

2. **Update PlatformIO Environment**
   ```bash
   # Edit platformio.ini
   # Change environment to esp32s3cam_ai
   ```

3. **Update Configuration**
   ```bash
   cp examples/configs/config_esp32s3_full_features.h config.h
   # Edit WiFi credentials and other settings
   nano config.h
   ```

4. **Update Pin Definitions** (if custom pins used)
   ```cpp
   // Update any custom pin definitions
   // See firmware/hal/board_detector.cpp for ESP32-S3 defaults
   ```

5. **Compile**
   ```bash
   pio run -e esp32s3cam_ai
   ```

6. **Upload**
   ```bash
   pio run -e esp32s3cam_ai -t upload
   # ESP32-S3 has native USB, no adapter needed!
   ```

7. **Test All Features**
   - [ ] Camera capture
   - [ ] LoRa communication
   - [ ] SD card read/write
   - [ ] PIR sensor
   - [ ] Battery monitoring
   - [ ] WiFi connectivity

#### Rollback Procedure

If issues occur:
```bash
# Restore AI-Thinker config
cp config.h.ai-thinker-backup config.h

# Recompile for AI-Thinker
pio run -e esp32cam_advanced -t upload
```

## Step 6: Testing Post-Migration

### Validation Checklist

#### Basic Functionality
- [ ] Device boots successfully
- [ ] Serial output shows correct board detection
- [ ] WiFi connects
- [ ] SD card mounts (if enabled)

#### Camera Tests
- [ ] Camera initializes without errors
- [ ] Test capture produces valid image
- [ ] Image quality acceptable
- [ ] Frame rate meets requirements

#### Network Tests
- [ ] WiFi connection stable
- [ ] LoRa transmission works (if enabled)
- [ ] Data upload successful
- [ ] OTA updates work

#### Sensor Tests
- [ ] PIR triggers correctly
- [ ] BME280 readings valid
- [ ] Battery voltage accurate
- [ ] Solar voltage accurate (if enabled)

#### Power Tests
- [ ] Deep sleep works
- [ ] Wake-up triggers correct
- [ ] Power consumption within limits
- [ ] Battery life acceptable

### Common Post-Migration Issues

| Issue | Likely Cause | Solution |
|-------|-------------|----------|
| Camera not working | Pin mismatch | Check camera pins in board_detector.cpp |
| LoRa no communication | Wrong pins | Verify LoRa SPI pins updated |
| Boot loop | Boot mode pins | Check GPIO 0, 2, 12, 15 states |
| High power consumption | Peripherals not sleeping | Review power management code |
| SD mount fails | Wrong pins or format | Verify SD pins, reformat card |

## Step 7: Update Documentation

After successful migration:

1. **Update Deployment Docs**
   - Document new hardware used
   - Update pin assignments
   - Note any configuration changes

2. **Update Monitoring**
   - Update expected power consumption
   - Update battery life estimates
   - Update network topology

3. **Train Team**
   - Share new configuration
   - Document new features available
   - Update troubleshooting guides

## Step 8: Long-Term Recommendations

### For New Deployments

1. **Start with ESP32-S3-CAM**
   - Future-proof
   - No compromises
   - Worth the small cost increase

2. **Plan for Scalability**
   - Use mixed deployments for cost optimization
   - Standardize configurations
   - Document everything

3. **Test Thoroughly**
   - Validate all features before field deployment
   - Test power consumption
   - Verify remote update capability

### For Existing Deployments

1. **Gradual Migration**
   - Replace devices as they need service
   - Start with most critical nodes
   - Keep AI-Thinker for simple camera nodes

2. **Maintain Compatibility**
   - Keep old configs for reference
   - Test interactions between old/new hardware
   - Document mixed network topology

3. **Monitor Performance**
   - Track reliability differences
   - Compare power consumption
   - Gather user feedback

## Cost Analysis

### Single Node Migration

| Component | AI-Thinker | ESP32-S3 | Difference |
|-----------|-----------|----------|------------|
| Board | $10 | $20 | +$10 |
| Programming | $5 (FTDI) | $0 (USB) | -$5 |
| Development time | 2 hrs | 1 hr | -1 hr |
| Troubleshooting | High | Low | Better |
| **Total** | $15 + time | $20 | +$5 net |

**ROI**: ESP32-S3 pays for itself in reduced development time and fewer issues.

### 10-Node Deployment Migration

| Scenario | Cost | Notes |
|----------|------|-------|
| Keep all AI-Thinker | $0 | Disable conflicting features |
| Replace all with ESP32-S3 | $100 | Full features everywhere |
| Mixed: 8 AI-Thinker + 2 relay | $40 | Best balance |

**Recommendation**: Mixed deployment for best cost/feature ratio.

## Support Resources

### Documentation
- `docs/GPIO_PIN_CONFLICTS.md` - Complete conflict details
- `docs/HARDWARE_SELECTION_GUIDE.md` - Hardware decision guide
- `firmware/examples/configs/` - Configuration templates

### Tools
- `firmware/test_gpio_conflicts.sh` - Conflict validation
- `firmware/test/test_config_conflicts.cpp` - Test suite

### Community
- GitHub Issues - Report problems
- ESP32 Forums - Community support
- Project Wiki - Shared knowledge

## Conclusion

GPIO conflicts on AI-Thinker ESP32-CAM are a hardware limitation, not a software bug. This migration guide provides clear paths to resolve conflicts while minimizing disruption and cost.

**Key Takeaways**:
1. Assess before migrating
2. Choose appropriate path (disable features, upgrade, or split)
3. Test thoroughly after migration
4. Document changes
5. Consider ESP32-S3 for new deployments

---

**Questions?** Check `docs/GPIO_PIN_CONFLICTS.md` or open a GitHub issue.

**Version**: 1.0 | **Date**: 2025-10-16
