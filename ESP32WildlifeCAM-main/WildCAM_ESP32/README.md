# 🦌 WildCAM_ESP32

**Solar-Powered AI Wildlife Monitoring Platform**

A streamlined ESP32-based wildlife camera system with essential features for autonomous monitoring.

## 🎯 Overview

WildCAM_ESP32 is a focused implementation of an ESP32 wildlife monitoring platform, containing only the current and essential code for:

- **Motion Detection** - PIR sensors with intelligent algorithms
- **AI Wildlife Classification** - On-device species identification 
- **Power Management** - Solar charging with optimized battery life
- **Image Capture** - High-quality wildlife photography
- **Data Storage** - Local SD card storage and wireless transmission

## 🚀 Quick Start

### Hardware Requirements
- ESP32-CAM module (AI-Thinker, ESP32-S3, or compatible)
- PIR motion sensor
- Solar panel and battery system
- MicroSD card (16GB+ recommended)

### Software Setup
1. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

2. **Clone and Build**
   ```bash
   git clone [repository_url]
   cd WildCAM_ESP32
   pio run -e esp32cam
   ```

3. **Upload Firmware**
   ```bash
   pio run -e esp32cam --target upload
   ```

### Configuration
- Update `wifi_config.h` with your network credentials
- Adjust camera settings in `config.h`
- Configure power management parameters for your solar setup

## 📁 Project Structure

```
WildCAM_ESP32/
├── src/                    # Core source code
│   ├── ai_detection/      # Wildlife AI classification
│   ├── power/             # Power management
│   └── utils/             # Utility functions
├── include/               # Header files
├── hardware/              # Board definitions
├── main.cpp              # Main application entry
├── config.h              # System configuration
└── platformio.ini        # Build configuration
```

## 🔧 Supported Boards

- **ESP32-CAM** (AI-Thinker) - Default configuration
- **ESP32-S3-CAM** - Enhanced performance variant
- **TTGO T-Camera** - Alternative board option

## 🔋 Power Management

The system includes intelligent power management for extended field operation:
- Solar panel integration
- Battery monitoring
- Adaptive duty cycling
- Low-power sleep modes

## 📸 Image Capture

- Triggered by motion detection
- Configurable quality settings
- Burst mode for action sequences
- Timestamp and metadata logging

## 🤖 AI Features

- On-device wildlife classification
- Species identification
- Behavior pattern recognition
- Optimized for ESP32 hardware constraints

## 📊 Data Management

- Local storage on microSD
- Wireless data transmission
- Configurable retention policies
- Image and sensor data logging

## 🛠️ Development

### Building for Different Boards
```bash
# ESP32-CAM (default)
pio run -e esp32cam

# ESP32-S3-CAM
pio run -e esp32s3cam

# TTGO T-Camera
pio run -e ttgo-t-camera
```

### Monitoring and Debugging
```bash
pio device monitor
```

## 📋 Configuration Options

Key configuration parameters in `config.h`:
- Camera resolution and quality
- Motion detection sensitivity
- Power management thresholds
- Wi-Fi and network settings
- AI model parameters

## 🔗 Dependencies

Core libraries automatically managed by PlatformIO:
- ESP32 Arduino Framework
- ArduinoJson for data handling
- LoRa library for wireless communication
- TensorFlow Lite for AI processing

## 📄 License

See LICENSE file for details.

## 🤝 Contributing

This is a streamlined version focused on essential functionality. For extended features and development discussions, see the main ESP32WildlifeCAM repository.

---

**Version:** 2.5.0 (Essential)  
**Author:** @thewriterben  
**Status:** Production Ready