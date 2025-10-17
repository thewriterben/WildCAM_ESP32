# Hardware Selection Guide for WildCAM ESP32

## Overview

This guide helps you select the right ESP32-based hardware for your wildlife monitoring deployment based on feature requirements, power constraints, and budget.

## Quick Selection Table

| Use Case | Recommended Hardware | Cost | Complexity |
|----------|---------------------|------|------------|
| Basic wildlife camera | AI-Thinker ESP32-CAM | $ | Low |
| Camera + LoRa mesh | ESP32-S3-CAM | $$ | Medium |
| Full-featured deployment | ESP32-S3-CAM | $$ | Medium |
| Power-constrained remote | AI-Thinker + Solar | $ | Low |
| Development/prototyping | ESP32-CAM-MB | $ | Low |
| Production mesh network | Multiple ESP32-DevKit | $ | Medium |

**Cost Scale**: $ = $5-15, $$ = $15-30, $$$ = $30+

## Supported Hardware Platforms

### 1. AI-Thinker ESP32-CAM

**Status**: ✅ Primary platform, fully supported

**Specifications**:
- **Chip**: ESP32 (Dual-core Xtensa LX6 @ 240MHz)
- **RAM**: 520KB SRAM + 4MB PSRAM
- **Flash**: 4MB
- **Camera**: OV2640 (2MP, up to UXGA 1600×1200)
- **GPIO**: ~6 available after camera
- **Power**: 3.3V, ~500mA peak, ~100mA average
- **Cost**: $8-12 USD

**✅ Strengths**:
- Low cost and widely available
- Excellent camera quality
- Built-in PSRAM for image processing
- Large community support
- Good power efficiency

**❌ Limitations**:
- Very limited GPIO pins (only ~6 free)
- Cannot run camera + LoRa simultaneously
- No USB (needs USB-to-serial adapter)
- Requires external 3.3V regulator (5V not recommended)
- Boot mode pin sensitivity

**Best For**:
- Standalone camera traps
- WiFi-only deployments
- Budget-conscious projects
- Simple sensor integration (I2C sensors only)
- Solar-powered remote cameras

**Not Suitable For**:
- LoRa mesh networks with camera
- Multi-sensor arrays
- Complex peripheral integration
- Servo/motor control while camera active

**GPIO Availability**:
```
Available when Camera + SD enabled:
- GPIO 1 (PIR, conflicts with UART TX)
- GPIO 3 (UART RX)
- GPIO 16 (LoRa RST / IR LED)
- GPIO 17 (PIR power / Servo)
- GPIO 33 (ADC for battery/light)

Shared (conditional):
- GPIO 26/27 (I2C with camera)
- GPIO 34 (ADC with Camera Y8)
```

### 2. ESP32-S3-CAM

**Status**: ✅ Recommended for full features

**Specifications**:
- **Chip**: ESP32-S3 (Dual-core Xtensa LX7 @ 240MHz)
- **RAM**: 512KB SRAM + 8MB PSRAM
- **Flash**: 8MB
- **Camera**: OV2640 or OV5640 (5MP)
- **GPIO**: 45 pins (35+ available)
- **USB**: Native USB OTG
- **Power**: 3.3V, ~600mA peak, ~120mA average
- **Cost**: $15-25 USD

**✅ Strengths**:
- Abundant GPIO pins (no conflicts!)
- Native USB support (no adapter needed)
- More PSRAM and Flash
- Faster processing (LX7 cores)
- Better AI/ML performance
- Hardware vector instructions
- All features work simultaneously

**❌ Limitations**:
- Higher cost than AI-Thinker
- Slightly higher power consumption
- Less community documentation
- Fewer pre-built enclosures

**Best For**:
- Camera + LoRa deployments
- Complex multi-sensor systems
- AI/ML edge processing
- Development and prototyping
- Systems requiring many peripherals
- USB programming/debugging

**GPIO Availability**:
```
Available GPIO (selection):
- 40+ pins available
- Multiple SPI/I2C/UART buses
- Dedicated pins for peripherals
- No conflicts between features
```

### 3. TTGO T-Camera

**Status**: ⚠️ Limited support

**Specifications**:
- **Chip**: ESP32 (Dual-core @ 240MHz)
- **Display**: 1.3" TFT LCD (240x240)
- **Camera**: OV2640
- **Battery**: Built-in LiPo management
- **GPIO**: Limited (display uses many)
- **Cost**: $20-30 USD

**✅ Strengths**:
- Built-in display
- Battery management included
- Nice form factor
- Good for field UI

**❌ Limitations**:
- Display uses many GPIO pins
- Limited expansion options
- Higher cost
- Battery is small

**Best For**:
- Interactive field devices
- User-configurable cameras
- Preview/review in field
- Educational projects

### 4. ESP32-DevKit (No Camera)

**Status**: ✅ Supported for mesh nodes

**Specifications**:
- **Chip**: ESP32 (Dual-core @ 240MHz)
- **GPIO**: 30+ pins available
- **Cost**: $5-10 USD

**Best For**:
- LoRa mesh relay nodes
- Sensor-only nodes
- Environmental monitoring
- Gateway nodes

## Board Comparison Matrix

| Feature | AI-Thinker | ESP32-S3-CAM | TTGO T-Cam | ESP32-DevKit |
|---------|-----------|--------------|------------|--------------|
| **Hardware** |
| Camera | ✅ OV2640 | ✅ OV2640/5640 | ✅ OV2640 | ❌ No |
| PSRAM | ✅ 4MB | ✅ 8MB | ✅ 4MB | ❌/⚠️ Optional |
| Flash | 4MB | 8MB | 4MB | 4MB |
| Display | ❌ | ❌ | ✅ LCD | ❌ |
| USB | ❌ Serial only | ✅ Native | ✅ Native | ❌ Serial |
| **GPIO Availability** |
| Available pins | 6 | 40+ | 10 | 30+ |
| ADC channels | 4 | 20 | 4 | 18 |
| SPI buses | 1 (shared) | 3 | 1 (shared) | 3 |
| I2C buses | 1 (shared) | 2 | 1 (shared) | 2 |
| **Power** |
| Voltage | 3.3V | 3.3V/5V | 3.3V/5V | 3.3V/5V |
| Idle current | ~50mA | ~60mA | ~80mA | ~40mA |
| Active current | ~400mA | ~500mA | ~600mA | ~200mA |
| **Features Compatible** |
| Camera + SD | ✅ | ✅ | ✅ | ❌ |
| Camera + LoRa | ❌ | ✅ | ❌ | ❌ |
| Camera + Sensors | ✅ Limited | ✅ All | ✅ Limited | ❌ |
| LoRa + Sensors | ⚠️ No camera | ✅ | ⚠️ No camera | ✅ |
| All features | ❌ | ✅ | ❌ | ⚠️ No camera |
| **Cost & Availability** |
| Cost (USD) | $8-12 | $15-25 | $20-30 | $5-10 |
| Availability | ✅ High | ⚠️ Medium | ⚠️ Medium | ✅ High |
| Documentation | ✅ Excellent | ⚠️ Good | ⚠️ Limited | ✅ Excellent |

## Feature Requirements Analysis

### Scenario 1: Basic Wildlife Camera
**Requirements**: Camera, PIR sensor, SD card, WiFi

**Recommended**: AI-Thinker ESP32-CAM
- ✅ All features supported
- ✅ Lowest cost
- ✅ Good power efficiency
- ✅ Simple to deploy

**Configuration**:
```cpp
#define CAMERA_ENABLED true
#define SD_CARD_ENABLED true
#define PIR_SENSOR_ENABLED true
#define WIFI_ENABLED true
#define LORA_ENABLED false  // Not possible
```

### Scenario 2: Remote Mesh Network
**Requirements**: Camera, LoRa mesh, sensors, solar power

**Recommended**: ESP32-S3-CAM
- ✅ Camera + LoRa work together
- ✅ Enough GPIO for all sensors
- ✅ Better processing for images
- ❌ Higher cost (worth it)

**Configuration**:
```cpp
#define CAMERA_ENABLED true
#define LORA_ENABLED true
#define SD_CARD_ENABLED true
#define SOLAR_MONITOR_ENABLED true
#define BME280_SENSOR_ENABLED true
```

**Alternative**: Mixed deployment
- Use AI-Thinker for camera nodes
- Use ESP32-DevKit for LoRa relays
- Connect via WiFi + LoRa network

### Scenario 3: High-Resolution Research
**Requirements**: High-res camera, AI processing, multiple sensors

**Recommended**: ESP32-S3-CAM
- ✅ Better camera support (5MP)
- ✅ More PSRAM for AI models
- ✅ Hardware acceleration
- ✅ All sensors supported

### Scenario 4: Ultra-Low Power
**Requirements**: Solar-powered, minimal features, long battery life

**Recommended**: AI-Thinker ESP32-CAM
- ✅ Lower power consumption
- ✅ Simpler firmware (less overhead)
- ✅ Good deep sleep support
- ✅ Lower cost = more units

**Configuration**:
```cpp
#define CAMERA_ENABLED true
#define WIFI_ENABLED false  // WiFi-off mode
#define SD_CARD_ENABLED true
#define DEEP_SLEEP_ENABLED true
```

### Scenario 5: Sensor-Only Mesh Nodes
**Requirements**: No camera, LoRa, multiple sensors

**Recommended**: ESP32-DevKit
- ✅ No camera = lower cost
- ✅ Many GPIO available
- ✅ Simple power management
- ✅ Easy prototyping

## Power Consumption Analysis

### AI-Thinker ESP32-CAM

| Mode | Current | Duration | Energy |
|------|---------|----------|--------|
| Deep sleep | 10µA | 23h 59m | 0.24mAh |
| Idle (WiFi off) | 50mA | - | - |
| Camera capture | 400mA | 2s | 0.22mAh |
| WiFi transmit | 200mA | 5s | 0.28mAh |
| **Total per hour** | - | - | ~0.98mAh |

**Battery Life** (with 10,000mAh battery):
- 1 capture/hour: ~417 days
- 1 capture/15min: ~104 days
- Continuous operation: ~20 hours

### ESP32-S3-CAM

| Mode | Current | Duration | Energy |
|------|---------|----------|--------|
| Deep sleep | 15µA | - | - |
| Idle (WiFi off) | 60mA | - | - |
| Camera capture | 500mA | 2s | 0.28mAh |
| LoRa transmit | 150mA | 1s | 0.04mAh |
| **Total per hour** | - | - | ~1.2mAh |

**Battery Life** (with 10,000mAh battery):
- 1 capture/hour: ~347 days
- 1 capture/15min: ~87 days

### Solar Power Requirements

**Minimum Solar Panel**:
- **AI-Thinker**: 5V 200mA (1W panel sufficient)
- **ESP32-S3**: 5V 300mA (1.5W panel recommended)

**Battery Capacity**:
- **Minimum**: 5,000mAh (18Wh) for 2-3 days backup
- **Recommended**: 10,000mAh (37Wh) for 5-7 days backup
- **Extended**: 20,000mAh (74Wh) for 10-14 days backup

## Cost Analysis

### Single Camera Node

| Component | AI-Thinker | ESP32-S3 | Notes |
|-----------|-----------|----------|-------|
| Board | $10 | $20 | |
| LoRa module | - | $8 | Not compatible with AI-Thinker + camera |
| Solar panel | $5 | $5 | 5W panel |
| Battery | $8 | $8 | 10,000mAh |
| Enclosure | $10 | $10 | Weatherproof |
| Sensors | $5 | $5 | PIR + BME280 |
| **Total** | **$38** | **$56** | Per node |

### 10-Node Deployment

| Configuration | Cost | Features |
|--------------|------|----------|
| 10× AI-Thinker (WiFi only) | $380 | Camera + WiFi + Sensors |
| 10× ESP32-S3 (Full) | $560 | Camera + LoRa + All sensors |
| Mixed: 8× AI-Thinker + 2× ESP32-DevKit (LoRa relays) | $460 | Camera nodes + LoRa mesh |

**Recommendation**: Mixed deployment offers best cost/feature balance.

## Procurement Guide

### Where to Buy

**AI-Thinker ESP32-CAM**:
- Amazon, AliExpress, eBay
- Look for "AI-Thinker" branding
- Include OV2640 camera
- ~$8-12 USD

**ESP32-S3-CAM**:
- Amazon, official distributors
- Verify PSRAM included (8MB)
- Check camera module (OV2640 or OV5640)
- ~$15-25 USD

**ESP32-CAM-MB** (Programmer board):
- Highly recommended for AI-Thinker
- USB programming without FTDI adapter
- ~$3-5 USD additional

### Quality Checklist

✅ Verify PSRAM size (4MB or 8MB)
✅ Check camera module (OV2640 genuine)
✅ Test all GPIO pins
✅ Verify flash size (4MB minimum)
✅ Check for proper 3.3V regulation
✅ Test camera image quality
✅ Verify WiFi antenna connection

## Migration Strategy

### From AI-Thinker to ESP32-S3-CAM

**1. Hardware Changes**:
- Update all pin definitions
- Connect new peripherals to different pins
- Update power supply if needed

**2. Software Changes**:
```ini
; platformio.ini
[env:esp32s3cam]
board = esp32s3dev
build_flags = 
    -DBOARD_ESP32S3_CAM
    -DCAMERA_MODEL_ESP32S3_EYE
```

**3. Pin Remapping**:
| Function | AI-Thinker | ESP32-S3 |
|----------|-----------|----------|
| LoRa CS | N/A (conflict) | GPIO 21 |
| LoRa RST | N/A | GPIO 38 |
| LoRa IRQ | N/A | GPIO 39 |
| PIR | GPIO 1 | GPIO 1 |
| Battery ADC | GPIO 34 | GPIO 4 |

**4. Testing Plan**:
- [ ] Camera initialization
- [ ] LoRa communication
- [ ] SD card read/write
- [ ] Sensor readings
- [ ] Power consumption
- [ ] WiFi connectivity

## Deployment Recommendations

### Small Scale (1-5 nodes)
**Use**: AI-Thinker ESP32-CAM
- WiFi mesh or point-to-point
- Simple setup
- Low cost

### Medium Scale (5-20 nodes)
**Use**: Mixed deployment
- AI-Thinker for camera nodes
- ESP32-DevKit for relay nodes
- LoRa mesh network
- Cost-effective scaling

### Large Scale (20+ nodes)
**Use**: ESP32-S3-CAM
- Full features on all nodes
- Better reliability
- Easier maintenance
- Worth the investment

### Extreme Remote
**Use**: AI-Thinker + Iridium satellite
- Ultra-low power
- Satellite backup
- Solar + battery
- Long deployment cycles

## Support and Resources

### Documentation
- [GPIO Pin Conflicts](GPIO_PIN_CONFLICTS.md)
- [ESP32-CAM Datasheet](http://wiki.ai-thinker.com/esp32-cam)
- [ESP32-S3 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)

### Community
- WildCAM GitHub Issues
- ESP32 Forum (esp32.com)
- Reddit: r/esp32

### Troubleshooting
- See `GPIO_PIN_CONFLICTS.md` for pin issues
- Check `HARDWARE_REQUIREMENTS.md` for specifications
- Review example configurations in `firmware/examples/`

## Decision Flowchart

```
Start: What features do you need?

┌─ Need camera? ──────────────────────────┐
│   Yes                                    │ No → ESP32-DevKit
│                                          │
├─ Need LoRa mesh? ──────────────────────┤
│   Yes → ESP32-S3-CAM (required)        │
│   No ↓                                  │
│                                         │
├─ Need many sensors? ───────────────────┤
│   Yes → ESP32-S3-CAM                   │
│   No ↓                                  │
│                                         │
├─ Budget constrained? ──────────────────┤
│   Yes → AI-Thinker ESP32-CAM           │
│   No → ESP32-S3-CAM                    │
│                                         │
└─────────────────────────────────────────┘
```

## Conclusion

**For most wildlife camera applications**: Start with **AI-Thinker ESP32-CAM**
- Excellent value
- Proven reliability
- Sufficient for basic needs

**For advanced deployments**: Upgrade to **ESP32-S3-CAM**
- Worth the investment
- Future-proof
- No compromises

**For mesh networks**: Use **mixed deployment**
- AI-Thinker for cameras
- ESP32-DevKit for relays
- Best cost/performance ratio

## Version History

- **v1.0** (2025-10-16): Initial hardware selection guide
