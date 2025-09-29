# 📋 ESP32 Wildlife Camera Setup Guide

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Hardware Setup](#hardware-setup)
3. [Software Installation](#software-installation)
4. [Configuration](#configuration)
5. [Testing and Validation](#testing-and-validation)
6. [Deployment](#deployment)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Tools
- **PlatformIO IDE** or **Arduino IDE** (2.0+)
- **Python 3.8+** for validation scripts
- **Git** for repository management
- **Serial monitor** (built into IDEs)

### Hardware Requirements
- **ESP32-CAM** or **ESP32-S3** development board
- **MicroSD card** (Class 10, 32GB+ recommended)
- **PIR motion sensor** (HC-SR501 or compatible)
- **Power supply** (5V 2A minimum) or solar setup
- **WiFi network** access for initial setup

## Hardware Setup

### 1. ESP32-CAM Wiring
```
ESP32-CAM Connections:
├── GPIO 12 → PIR Motion Sensor (Data)
├── GPIO 13 → Status LED
├── GPIO 4  → Flash/IR LED
├── GPIO 14 → SD Card CLK
├── GPIO 15 → SD Card CMD
├── GPIO 2  → SD Card D0
└── Power pins (5V, GND)
```

### 2. PIR Sensor Connection
```
PIR Sensor (HC-SR501):
├── VCC → ESP32 5V
├── GND → ESP32 GND
└── OUT → ESP32 GPIO 12
```

### 3. Power Configuration
Choose one option:

**Option A: USB Power (Development)**
- Connect ESP32-CAM to USB programmer
- Set power jumper to 5V position

**Option B: External Power (Field Deployment)**
- Use 5V 2A power adapter
- Connect to ESP32-CAM power pins
- Optional: Add solar panel with charge controller

### 4. SD Card Setup
1. Format SD card as FAT32
2. Insert into ESP32-CAM slot
3. Verify proper seating

## Software Installation

### 1. Development Environment Setup

**Option A: PlatformIO (Recommended)**
```bash
# Install PlatformIO Core
pip install platformio

# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/ESP32WildlifeCAM-main

# Install dependencies
pio lib install
```

**Option B: Arduino IDE**
```bash
# Install ESP32 board package in Arduino IDE
# URL: https://dl.espressif.com/dl/package_esp32_index.json

# Install required libraries:
# - ArduinoJson (6.21.3+)
# - LoRa (0.8.0+)
# - Adafruit BME280 Library (2.2.2+)
```

### 2. Firmware Installation
```bash
# Navigate to project directory
cd ESP32WildlifeCAM-main

# Build and upload firmware
pio run --target upload --environment esp32cam

# Monitor serial output
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

## Configuration

### 1. WiFi Configuration
Create `src/config/wifi_config.h`:
```cpp
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
#define WIFI_TIMEOUT_MS 30000
```

### 2. Camera Settings
Edit `src/config/camera_config.h`:
```cpp
#define CAMERA_MODEL_AI_THINKER
#define CAMERA_RESOLUTION FRAMESIZE_UXGA
#define CAMERA_QUALITY 10  // 0-63, lower = higher quality
#define BURST_MODE_ENABLED true
#define VIDEO_MODE_ENABLED true
```

### 3. Motion Detection
Configure `src/config/motion_config.h`:
```cpp
#define PIR_PIN 12
#define MOTION_SENSITIVITY 80
#define MOTION_DELAY_MS 5000
#define ADVANCED_MOTION_ENABLED true
```

### 4. Storage Configuration
Set up `src/config/storage_config.h`:
```cpp
#define SD_CARD_ENABLED true
#define MAX_FILE_SIZE_MB 10
#define LOW_SPACE_THRESHOLD_MB 100
#define AUTO_CLEANUP_ENABLED true
```

## Testing and Validation

### 1. Hardware Validation
```bash
# Run hardware test script
cd scripts
./hardware_test.sh --verbose

# Expected output:
# ✓ ESP32-CAM detected
# ✓ SD card initialized
# ✓ PIR sensor responsive
# ✓ Camera module functional
```

### 2. Integration Testing
```bash
# Run comprehensive integration tests
./scripts/integration_test.sh --simulation

# Test categories:
# - Camera-Motion Integration
# - Power-Storage Integration
# - Sensor-Environment Integration
```

### 3. Functional Testing
```bash
# Test motion detection workflow
python3 scripts/integration_test_motion.py --verbose

# Verify camera capture
# Trigger PIR sensor and check for image capture
```

### 4. Performance Validation
```bash
# Run performance benchmarks
python3 validate_advanced_implementation.py

# Check system metrics:
# - Memory usage
# - Processing speed
# - Power consumption
```

## Deployment

### 1. Field Deployment Checklist
- [ ] Hardware connections secure
- [ ] SD card formatted and inserted
- [ ] Power supply validated
- [ ] WiFi credentials configured
- [ ] Motion detection tested
- [ ] Camera functionality verified
- [ ] Enclosure weatherproofing complete

### 2. Remote Configuration
```cpp
// Enable web interface for remote management
#define WEB_SERVER_ENABLED true
#define WEB_SERVER_PORT 80

// Access via: http://[ESP32_IP_ADDRESS]
```

### 3. Monitoring Setup
- Configure analytics dashboard
- Set up data collection endpoints
- Enable remote logging
- Test alert systems

## Troubleshooting

### Common Issues

**1. Camera Initialization Failed**
```
Solution:
- Check power supply (min 5V 2A)
- Verify camera module connection
- Reset ESP32-CAM
- Check GPIO assignments
```

**2. SD Card Not Detected**
```
Solution:
- Reformat SD card as FAT32
- Check card compatibility (Class 10+)
- Verify physical connection
- Try different SD card
```

**3. WiFi Connection Issues**
```
Solution:
- Verify SSID and password
- Check network signal strength
- Increase connection timeout
- Try 2.4GHz network only
```

**4. Motion Detection Not Working**
```
Solution:
- Check PIR sensor power (5V)
- Verify GPIO 12 connection
- Adjust sensitivity potentiometer
- Test with LED indicator
```

**5. Low Performance/Memory Issues**
```
Solution:
- Enable PSRAM usage
- Reduce camera resolution
- Optimize image quality settings
- Clear SD card space
```

### Advanced Diagnostics
```bash
# Enable debug output
export DEBUG_LEVEL=3

# Monitor system health
pio device monitor --filter esp32_exception_decoder

# Check memory usage
# Look for heap allocation errors in serial output
```

### Getting Help
- **Documentation**: See [docs/README.md](README.md) for comprehensive guides
- **Issues**: Report bugs on GitHub issues
- **Community**: Join discussions in repository
- **Logs**: Always include serial output when reporting issues

---

## Next Steps
- [Hardware Integration Guide](hardware_selection_guide.md)
- [API Reference](api_reference.md)
- [Analytics Dashboard Setup](ANALYTICS_DASHBOARD.md)
- [Advanced Configuration](software/README.md)