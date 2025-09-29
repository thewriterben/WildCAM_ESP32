# 🚀 ESP32WildlifeCAM Quick Start Guide

*Get your wildlife camera up and running in 30 minutes!*

## 🎯 What You'll Build

By the end of this guide, you'll have a functional solar-powered wildlife camera that:
- ✅ Detects motion with PIR sensors
- ✅ Captures high-quality wildlife photos
- ✅ Stores images on SD card with metadata
- ✅ Operates autonomously on solar power
- ✅ Connects to WiFi for remote monitoring

## 📦 Required Components

### Essential Hardware
| Component | Quantity | Purpose | Estimated Cost |
|-----------|----------|---------|----------------|
| **AI-Thinker ESP32-CAM** | 1 | Main controller + camera | $12-15 |
| **PIR Motion Sensor (HC-SR501)** | 1 | Motion detection | $3-5 |
| **MicroSD Card (32GB Class 10)** | 1 | Image storage | $8-10 |
| **5W Solar Panel** | 1 | Power source | $15-20 |
| **3000mAh LiPo Battery** | 1 | Energy storage | $8-12 |
| **Breadboard & Jumper Wires** | 1 set | Prototyping | $5-10 |

**Total Basic Setup**: ~$56-77

### Optional Enhancements
- **Waterproof Enclosure** - $10-15 (or 3D print from our files)
- **LoRa Module (SX1276)** - $8-12 (for long-range networking)
- **Environmental Sensors (BME280)** - $5-8 (weather monitoring)

## 📋 Step 1: Hardware Assembly (15 minutes)

### Basic Wiring Diagram
```
ESP32-CAM Connections:
┌─────────────────┐
│    ESP32-CAM    │
│                 │
│ GPIO1  ←─ PIR   │ (Motion Detection)
│ GPIO32 ←─ BATT+ │ (Battery Monitoring)
│ GPIO33 ←─ SOLAR+│ (Solar Panel Monitoring)
│ 5V     ←─ VCC   │ (Power Input)
│ GND    ←─ GND   │ (Common Ground)
└─────────────────┘
```

### Assembly Steps
1. **Insert SD Card** - Format as FAT32, insert into ESP32-CAM
2. **Connect PIR Sensor** - VCC to 5V, GND to GND, OUT to GPIO1
3. **Connect Power Monitoring** - Battery+ to GPIO32, Solar+ to GPIO33
4. **Power Supply** - Connect 5V and GND from power source

⚠️ **Important**: GPIO1 is shared with SD card. Our firmware handles this conflict automatically.

## 💻 Step 2: Software Setup (10 minutes)

### Install Development Environment
```bash
# Install PlatformIO (recommended)
pip install platformio

# Alternative: Use Arduino IDE
# Download from: https://www.arduino.cc/en/software
```

### Download and Setup Project
```bash
# Clone the repository
git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
cd ESP32WildlifeCAM

# Validate environment
python3 validate_fixes.py
```

### Quick Configuration
```bash
# Copy default configuration
cp firmware/src/config.h firmware/src/config_local.h

# Edit config_local.h to set:
# - WiFi credentials
# - Camera settings
# - Power management options
```

## ⚙️ Step 3: Build and Upload (5 minutes)

### Using PlatformIO (Recommended)
```bash
# Build firmware
pio run -e esp32cam

# Upload to device
pio run -e esp32cam -t upload

# Monitor serial output
pio device monitor
```

### Using Arduino IDE
1. Open `src/main.cpp` in Arduino IDE
2. Select **ESP32 Dev Module** board
3. Set **Partition Scheme** to "Huge APP (3MB No OTA)"
4. Upload to device

### Expected Output
```
[INFO] ESP32WildlifeCAM v2.5 Starting...
[INFO] Camera initialized successfully
[INFO] PIR sensor configured on GPIO1
[INFO] SD Card mounted: 29.8GB available
[INFO] Power management enabled
[INFO] WiFi connecting...
[INFO] System ready for wildlife monitoring!
```

## 🎛️ Step 4: Initial Testing

### Test Camera Function
```bash
# Force a photo capture via serial monitor
# Send command: "capture"
# Look for: "Image saved: /wildlife/IMG_001.jpg"
```

### Test Motion Detection
1. Wave hand in front of PIR sensor
2. Check serial output for: "Motion detected!"
3. Verify image is captured and saved

### Test Power Monitoring
- Check battery voltage reading
- Verify solar panel voltage (in sunlight)
- Confirm power management status

## 📱 Step 5: WiFi Configuration

### Connect to Camera WiFi
1. Camera creates AP: "ESP32WildlifeCAM-XXXX"
2. Connect with password: "wildlife123"
3. Open browser to: http://192.168.4.1

### Configure WiFi Settings
1. Enter your WiFi network credentials
2. Set timezone and location
3. Configure capture settings
4. Save configuration

### Verify Remote Access
- Camera will restart and connect to your WiFi
- Access via assigned IP address
- Check live camera feed and settings

## 🏞️ Step 6: Deployment Setup

### Outdoor Placement
1. **Camera Angle** - 45° downward, facing north (avoid direct sun)
2. **PIR Sensor** - Clear line of sight, 3-5 meter detection range
3. **Solar Panel** - Maximum sun exposure, tilted toward sun
4. **Height** - 2-3 meters high, secure mounting

### Weather Protection
- Use waterproof enclosure or 3D printed housing
- Ensure cable entry points are sealed
- Add desiccant packets to prevent condensation
- Position to avoid direct rain on camera lens

### Security Considerations
- Use hidden/camouflaged mounting
- Enable WiFi password protection
- Set up motion detection alerts
- Regular data backup

## 📊 Monitoring & Maintenance

### Check System Status
```bash
# Web interface: http://[camera-ip]/status
# Shows:
# - Battery level
# - Solar charging status
# - Images captured
# - Storage usage
# - System uptime
```

### Regular Maintenance
- **Weekly**: Check battery level and image quality
- **Monthly**: Clean camera lens and solar panel
- **Quarterly**: Download images and clear storage
- **Annually**: Replace desiccant and check seals

## 🔧 Troubleshooting

### Camera Not Working
```bash
# Check connections and power
# Verify in serial monitor:
[ERROR] Camera init failed - check wiring
[INFO] Retrying camera initialization...
```

**Solution**: 
- Verify GPIO connections
- Check power supply (5V, 1A minimum)
- Reseat camera module connection

### Motion Detection Issues
```bash
# PIR not triggering
[DEBUG] PIR sensor status: INACTIVE
```

**Solutions**:
- Adjust PIR sensitivity potentiometer
- Check PIR power (5V) and signal (GPIO1)
- Verify detection range and positioning

### Storage Problems
```bash
[ERROR] SD card mount failed
[ERROR] Insufficient storage space
```

**Solutions**:
- Reformat SD card (FAT32)
- Use Class 10 SD card
- Enable automatic file cleanup
- Check available space regularly

### Power Issues
```bash
[WARNING] Battery low: 3.2V
[INFO] Entering power save mode
```

**Solutions**:
- Check solar panel connections
- Verify battery health
- Adjust power save settings
- Increase solar panel wattage

## 🎓 Next Steps

### Explore Advanced Features
- **AI Wildlife Detection** - [docs/ai/ai_architecture.md](docs/ai/ai_architecture.md)
- **LoRa Mesh Networking** - [docs/deployment/](docs/deployment/)
- **Satellite Communication** - [docs/SATELLITE_COMMUNICATION.md](docs/SATELLITE_COMMUNICATION.md)
- **3D Printed Enclosures** - [3d_models/](3d_models/)

### Join the Community
- **GitHub Discussions** - [Ask questions and share experiences](https://github.com/thewriterben/ESP32WildlifeCAM/discussions)
- **Discord Community** - [Real-time help and collaboration](https://discord.gg/7cmrkFKx)
- **Contribute** - See our [Contributing Guide](CONTRIBUTING.md)

### Educational Resources
- **Complete Documentation** - [docs/README.md](docs/README.md)
- **Hardware Guides** - [docs/hardware/](docs/hardware/)
- **Deployment Guides** - [docs/deployment/](docs/deployment/)
- **Troubleshooting** - [docs/troubleshooting.md](docs/troubleshooting.md)

## ⚡ Quick Reference

### Serial Monitor Commands
```
capture          - Force image capture
status          - Show system status
reboot          - Restart system
config          - Show current configuration
test_camera     - Test camera functionality
test_pir        - Test PIR sensor
test_power      - Test power monitoring
```

### Important File Locations
```
/wildlife/      - Captured images
/config/        - System configuration
/logs/          - System logs
/backup/        - Backup files
```

### Default Settings
- **Image Quality**: High (JPEG, 85% quality)
- **Motion Sensitivity**: Medium
- **Power Save**: Enabled
- **WiFi Timeout**: 30 seconds
- **Deep Sleep**: 5 minutes (no motion)

## 🎉 Success!

Congratulations! You now have a fully functional ESP32WildlifeCAM system. Your camera is ready to:

- 🔋 **Operate autonomously** on solar power
- 📸 **Capture wildlife photos** automatically
- 📡 **Connect remotely** for monitoring
- 💾 **Store images** with metadata
- 🔧 **Expand capabilities** with advanced features

**What's Next?**
1. Deploy in your favorite wildlife area
2. Monitor and collect amazing wildlife photos
3. Share your experiences with the community
4. Explore advanced features and customizations

---

**Navigation**: [🏠 Main README](README.md) | [📚 Documentation Index](docs/README.md) | [🤝 Contributing](CONTRIBUTING.md) | [🗺️ Roadmap](ROADMAP.md)

*Happy Wildlife Monitoring! 🦌📸*