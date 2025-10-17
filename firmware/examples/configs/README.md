# Configuration Templates

This directory contains pre-configured templates for common WildCAM deployment scenarios. Each template is optimized for specific hardware and use cases.

## Available Templates

### 1. AI-Thinker Camera Only (`config_ai_thinker_camera_only.h`)

**Hardware**: AI-Thinker ESP32-CAM  
**Primary Function**: Wildlife camera trap with WiFi

**Features**:
- ✅ Camera (OV2640)
- ✅ SD Card storage
- ✅ PIR motion sensor
- ✅ WiFi connectivity
- ✅ BME280 environmental sensor
- ✅ Battery voltage monitoring
- ❌ LoRa (conflicts with camera)
- ❌ LED indicators (conflicts with SD)

**Best For**:
- Budget-conscious deployments
- Standalone camera traps
- WiFi-connected locations
- Solar-powered remote sites

**Available GPIO**: 1, 3, 16, 17, 33

---

### 2. ESP32-S3 Full Features (`config_esp32s3_full_features.h`)

**Hardware**: ESP32-S3-CAM  
**Primary Function**: Full-featured wildlife monitoring with mesh networking

**Features**:
- ✅ Camera (OV2640/OV5640)
- ✅ SD Card storage
- ✅ LoRa mesh networking
- ✅ PIR motion sensor
- ✅ WiFi connectivity
- ✅ BME280 environmental sensor
- ✅ Battery + Solar monitoring
- ✅ LED indicators
- ✅ AI/ML edge processing

**Best For**:
- Advanced deployments
- Mesh network camera nodes
- AI-powered species detection
- Complex sensor integration

**Available GPIO**: 40+ pins (no conflicts!)

---

### 3. LoRa Mesh Relay Node (`config_lora_mesh_node.h`)

**Hardware**: AI-Thinker ESP32-CAM (camera disabled) or ESP32-DevKit  
**Primary Function**: LoRa mesh network relay/gateway

**Features**:
- ❌ Camera (disabled to free GPIO)
- ✅ SD Card for data logging
- ✅ LoRa mesh networking (primary)
- ✅ PIR motion sensor
- ✅ WiFi connectivity
- ✅ BME280 environmental sensor
- ✅ Battery + Solar monitoring
- ✅ Gateway mode (LoRa-to-WiFi)

**Best For**:
- Mesh network relay nodes
- LoRa-to-WiFi gateways
- Environmental monitoring stations
- Extending network range

**Available GPIO**: Many (camera pins freed up)

---

## How to Use

### Method 1: Copy Template to config.h

```bash
# From firmware directory
cp examples/configs/config_ai_thinker_camera_only.h config.h
```

### Method 2: Use as Reference

Open the template file and copy relevant sections into your existing `config.h`.

### Method 3: Symlink (Development)

```bash
# From firmware directory
ln -sf examples/configs/config_ai_thinker_camera_only.h config.h
```

## Configuration Steps

1. **Choose Template**: Select based on hardware and features needed
2. **Copy File**: Copy template to `firmware/config.h`
3. **Customize**:
   - Update WiFi credentials
   - Adjust LoRa frequency for your region
   - Set API endpoints
   - Configure power management
4. **Compile**: Use appropriate PlatformIO environment
5. **Test**: Verify all features work as expected

## Build Environments

Each template works with specific PlatformIO environments:

| Template | PlatformIO Environment | Command |
|----------|----------------------|---------|
| AI-Thinker Camera | `esp32cam_advanced` | `pio run -e esp32cam_advanced` |
| ESP32-S3 Full | `esp32s3cam_ai` | `pio run -e esp32s3cam_ai` |
| LoRa Mesh Node | `esp32cam_mesh` | `pio run -e esp32cam_mesh` |

## Compile-Time Validation

All templates include compile-time checks for GPIO conflicts. If you enable incompatible features, the compiler will show an error with suggested solutions.

Example error:
```
❌ CONFIGURATION ERROR: LoRa and Camera cannot be enabled simultaneously on AI-Thinker ESP32-CAM
```

See [docs/GPIO_PIN_CONFLICTS.md](../../../docs/GPIO_PIN_CONFLICTS.md) for detailed conflict information.

## Customization Guide

### WiFi Settings

```cpp
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
```

### LoRa Frequency by Region

```cpp
// North America
#define LORA_FREQUENCY 915E6

// Europe
#define LORA_FREQUENCY 868E6

// Asia
#define LORA_FREQUENCY 433E6
```

### Camera Quality

```cpp
// High quality (larger files)
#define CAMERA_JPEG_QUALITY 5

// Balanced (default)
#define CAMERA_JPEG_QUALITY 10

// Low quality (smaller files, faster)
#define CAMERA_JPEG_QUALITY 20
```

### Power Management

```cpp
// Ultra-low power (long deployment)
#define DEEP_SLEEP_DURATION_SEC 7200  // 2 hours

// Balanced
#define DEEP_SLEEP_DURATION_SEC 3600  // 1 hour

// High activity
#define DEEP_SLEEP_DURATION_SEC 900   // 15 minutes
```

## Common Modifications

### Disable Feature

```cpp
// Comment out or set to false
// #define LORA_ENABLED true
#define LORA_ENABLED false
```

### Change Pin Assignment

```cpp
// See firmware/hal/board_detector.h for available pins
#define PIR_PIN 3  // Changed from default GPIO 1
```

### Add Custom Feature

```cpp
// Add your custom definitions
#define CUSTOM_SENSOR_ENABLED true
#define CUSTOM_SENSOR_PIN 17
```

## Testing Your Configuration

### 1. Compile Test

```bash
cd firmware
pio run -e esp32cam_advanced
```

### 2. Check Build Output

Look for configuration summary:
```
✓ Camera: ENABLED
✗ LoRa: DISABLED
✓ SD Card: ENABLED
Board: AI-Thinker ESP32-CAM (Limited GPIO)
```

### 3. Upload and Monitor

```bash
pio run -e esp32cam_advanced -t upload
pio device monitor
```

### 4. Verify Features

- Test camera capture
- Verify SD card writes
- Check sensor readings
- Test network connectivity

## Troubleshooting

### Compile Errors

**Error**: "LoRa and Camera cannot be enabled simultaneously"
- **Solution**: Choose one feature or upgrade to ESP32-S3-CAM

**Error**: "LED indicators and SD card conflict"
- **Solution**: Disable LED indicators on AI-Thinker

### Runtime Issues

**Camera not working**:
- Check if LoRa is enabled (conflict)
- Verify PWDN pin not used for other purpose

**SD card mount failed**:
- Check if LEDs are enabled (conflict)
- Verify SD card is inserted
- Try formatting SD card

**LoRa not communicating**:
- Ensure camera is disabled
- Check LoRa frequency matches network
- Verify antenna is connected

## Best Practices

1. **Start with Template**: Don't build from scratch
2. **Test Incrementally**: Enable features one at a time
3. **Read Documentation**: Check GPIO_PIN_CONFLICTS.md
4. **Version Control**: Keep your config.h in git
5. **Document Changes**: Comment your customizations
6. **Backup Originals**: Keep copy of working config

## Migration Between Templates

### From AI-Thinker to ESP32-S3

1. Copy new template
2. Update board definition
3. Enable additional features (LoRa, etc.)
4. Update pin assignments
5. Recompile with new environment

### From Camera to Mesh Node

1. Disable camera in config
2. Enable LoRa
3. Update to mesh environment
4. Configure mesh parameters
5. Test relay functionality

## Support

- **Documentation**: [docs/GPIO_PIN_CONFLICTS.md](../../../docs/GPIO_PIN_CONFLICTS.md)
- **Hardware Guide**: [docs/HARDWARE_SELECTION_GUIDE.md](../../../docs/HARDWARE_SELECTION_GUIDE.md)
- **Issues**: Report on GitHub
- **Community**: ESP32 forums

## Version History

- **v1.0** (2025-10-16): Initial configuration templates
