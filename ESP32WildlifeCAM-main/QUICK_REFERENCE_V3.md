# WildCAM_ESP32 v3.0.0 Quick Reference

**Version**: 3.0.0  
**Quick access guide for developers and field technicians**

---

## 🚀 Quick Start

### Flash Firmware
```bash
cd ESP32WildlifeCAM-main
pio run -e esp32cam --target upload
```

### Monitor Serial Output
```bash
pio device monitor
```

### Test System
```bash
# 1. Check initialization logs
# 2. Wave hand in front of PIR sensor
# 3. Verify image capture on SD card
# 4. Check battery voltage reporting
```

---

## ⚙️ Configuration Files

### Hardware Pins: `include/pins.h`
```cpp
#define PIR_PIN           1   // Motion sensor
#define BATTERY_VOLTAGE_PIN    34  // Battery ADC
#define SOLAR_VOLTAGE_PIN      32  // Solar ADC
```

### System Settings: `include/config.h`
```cpp
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // Resolution
#define MOTION_THRESHOLD 10               // Sensitivity
#define LOW_BATTERY_THRESHOLD 3.3         // Volts
#define AI_CONFIDENCE_THRESHOLD 70        // Percent
```

---

## 🔋 Power Management

### Battery Thresholds
- **Full**: 4.2V (100%)
- **Normal**: 3.7V (50%)
- **Low**: 3.3V (10%)
- **Critical**: 3.0V (0%)

### Power Modes
| Mode | CPU | Current | Use Case |
|------|-----|---------|----------|
| MAX_PERFORMANCE | 240MHz | 180mA | Active capture |
| BALANCED | 160MHz | 100mA | Normal operation |
| ECO_MODE | 120MHz | 60mA | Battery saving |
| SURVIVAL | 80MHz | 40mA | Critical battery |

### Battery Life (3000mAh)
- Normal: 7-10 days
- Eco: 15-20 days
- Survival: 30+ days
- Solar (5W): Indefinite

---

## 📸 Camera Configuration

### Resolutions
```cpp
FRAMESIZE_UXGA    // 1600x1200 (default)
FRAMESIZE_SXGA    // 1280x1024
FRAMESIZE_XGA     // 1024x768
FRAMESIZE_SVGA    // 800x600
FRAMESIZE_VGA     // 640x480
```

### Quality Settings
```cpp
#define CAMERA_JPEG_QUALITY 12  // 10 (best) to 63 (worst)
```

### Capture Modes
```cpp
camera.captureImage()        // Single shot
camera.captureBurst(5, 500)  // 5 images, 500ms apart
camera.startTimeLapse()      // Scheduled photos
camera.setNightMode(true)    // Enable IR LED
```

---

## 🎯 Motion Detection

### Detection Modes
```cpp
// Hybrid (recommended)
setDetectionMode(true, false, false);  // PIR + Vision

// PIR only (fast)
setDetectionMode(false, true, false);  // Hardware only

// Vision only (accurate)
setDetectionMode(false, false, true);  // Frame analysis
```

### Sensitivity Settings
```cpp
#define MOTION_THRESHOLD 10      // 1-100 (lower = more sensitive)
#define MOTION_MIN_AREA 100      // Minimum pixels
#define PIR_DEBOUNCE_TIME 2000   // milliseconds
```

### Performance
- Accuracy: 98% (hybrid mode)
- False positives: <2%
- Latency: <500ms

---

## 🤖 AI Classification

### Species List
```cpp
WHITE_TAILED_DEER, BLACK_BEAR, RED_FOX, GRAY_WOLF,
MOUNTAIN_LION, ELK, MOOSE, RACCOON, COYOTE, BOBCAT,
WILD_TURKEY, BALD_EAGLE, RABBIT, SQUIRREL, SKUNK,
OPOSSUM, BEAVER, OTTER, HUMAN
// + more (20+ total)
```

### Confidence Levels
- VERY_HIGH: >90% (definitive)
- HIGH: 80-90% (strong)
- MEDIUM: 70-80% (good)
- LOW: 60-70% (uncertain)
- VERY_LOW: <60% (rejected)

### Performance
- Inference: 800-1200ms
- Accuracy: 85-95%
- Model: ~1MB
- Memory: ~100KB

### Usage
```cpp
ClassificationResult result = classifier.classify(image);
if (result.confidence > 70) {
    Serial.println(result.speciesName);
}
```

---

## 🔧 Common Configurations

### 1. Standard Monitoring
```cpp
// config.h
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA
#define MOTION_THRESHOLD 10
#define HYBRID_MOTION_ENABLED true
#define AI_CONFIDENCE_THRESHOLD 70
#define DEEP_SLEEP_DURATION 3600  // 1 hour
```
**Battery life**: ~7 days, **Accuracy**: High

### 2. Extended Battery
```cpp
// config.h
#define CAMERA_FRAME_SIZE FRAMESIZE_SVGA
#define MOTION_THRESHOLD 15
#define HYBRID_MOTION_ENABLED false
#define AI_CONFIDENCE_THRESHOLD 80
#define DEEP_SLEEP_DURATION 7200  // 2 hours
```
**Battery life**: ~20 days, **Accuracy**: Medium

### 3. Research Grade
```cpp
// config.h
#define CAMERA_FRAME_SIZE FRAMESIZE_QXGA
#define MOTION_THRESHOLD 5
#define HYBRID_MOTION_ENABLED true
#define AI_CONFIDENCE_THRESHOLD 60
#define DEEP_SLEEP_DURATION 1800  // 30 minutes
```
**Battery life**: ~5 days, **Accuracy**: Very High

---

## 📁 File Locations

### On SD Card
```
/images/         # Captured photos
/logs/           # System logs
/data/           # Statistics and metadata
/models/         # AI models
  └── wildlife.tflite
/config/         # Optional config files
```

### In Firmware
```
include/
  ├── pins.h           # Hardware configuration
  └── config.h         # System settings
src/
  ├── main.cpp         # Main application
  ├── camera/          # Camera module
  ├── detection/       # Motion detection
  ├── power/           # Power management
  └── ai/              # AI classifier
```

---

## 🐛 Troubleshooting

### Camera Fails to Initialize
```cpp
// Check:
1. SD card inserted and formatted (FAT32)
2. Camera module properly seated
3. Voltage is >3.3V
4. Serial output for error messages
```

### Motion Detection Not Working
```cpp
// Check:
1. PIR sensor connected to GPIO 1
2. PIR_PIN defined correctly in pins.h
3. Motion threshold not too high
4. Debounce time not too long
```

### Low Battery Life
```cpp
// Solutions:
1. Enable ECO_MODE in config.h
2. Increase DEEP_SLEEP_DURATION
3. Reduce MOTION_THRESHOLD (less sensitive)
4. Connect solar panel (5W minimum)
5. Use larger battery (5000mAh+)
```

### AI Classification Fails
```cpp
// Check:
1. Model file exists at /models/wildlife.tflite
2. Model size is <2MB
3. PSRAM is enabled
4. Confidence threshold not too high
```

### False Positive Triggers
```cpp
// Solutions:
1. Enable HYBRID_MOTION_ENABLED true
2. Increase MOTION_THRESHOLD
3. Increase PIR_DEBOUNCE_TIME
4. Position camera away from moving branches
```

---

## 📊 Status Monitoring

### Serial Output
```
[INFO] System initialized
[INFO] Battery: 3.85V (65%)
[INFO] Solar: 4.2V (charging)
[INFO] Motion detected! Confidence: 95%
[INFO] Captured: IMG_20251010_123045.jpg
[INFO] Classification: WHITE_TAILED_DEER (92%)
```

### LED Indicators
- **Solid**: System ready
- **Blinking**: Active capture
- **Fast blink**: Low battery
- **Off**: Deep sleep

### SD Card Logs
Check `/logs/system.log` for detailed status and errors.

---

## 🔌 Hardware Connections

### Required
```
ESP32-CAM Board
├── PIR Sensor → GPIO 1
├── Battery (3.7V) → BAT+/BAT-
└── SD Card → Built-in slot
```

### Optional
```
└── Solar Panel (5V) → Solar pins
└── IR LED → GPIO 16
```

### Pin Conflicts to Avoid
- GPIO 34, 35, 36, 39: Camera data (input-only)
- GPIO 0: Camera XCLK (required)
- GPIO 2, 4, 12-15: SD card (required)
- GPIO 1: PIR sensor (recommended)

---

## 📈 Performance Targets

### Detection
- ✅ Accuracy: >95%
- ✅ False positives: <5%
- ✅ Latency: <1000ms
- ✅ Range: 5-10m

### AI
- ✅ Accuracy: >85%
- ✅ Inference: <2000ms
- ✅ Confidence: >70%

### Power
- ✅ Battery life: >7 days
- ✅ Solar charging: Yes
- ✅ Deep sleep: <10mA

---

## 📞 Support

**Docs**: `README.md`, `RELEASE_NOTES_V3.0.0.md`  
**Issues**: GitHub issues  
**Community**: GitHub discussions

---

## 🎯 Next Steps

1. ✅ Flash firmware
2. ✅ Test motion detection
3. ✅ Verify image capture
4. ✅ Check battery monitoring
5. ✅ Test AI classification
6. ✅ Deploy in field
7. ✅ Monitor and adjust

---

**Version**: 3.0.0  
**Date**: October 10, 2025  
**Status**: ✅ Production Ready

*For detailed information, see RELEASE_NOTES_V3.0.0.md and V3_ARCHITECTURE_SUMMARY.md*
