# Enhanced OV5640 5MP Wildlife Camera Guide

Complete setup and usage guide for the ESP32 Wildlife Camera system with OV5640 5MP sensor and IR Cut filter integration.

## Table of Contents

1. [Overview](#overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Features](#software-features)
4. [Installation Guide](#installation-guide)
5. [Configuration](#configuration)
6. [Usage Instructions](#usage-instructions)
7. [API Reference](#api-reference)
8. [Troubleshooting](#troubleshooting)
9. [Performance Optimization](#performance-optimization)
10. [Field Deployment](#field-deployment)

## Overview

The Enhanced OV5640 Wildlife Camera system transforms the ESP32WildlifeCAM into a professional-grade wildlife monitoring solution capable of:

- **5MP High-Resolution Capture**: Full 2592x1944 resolution with the OV5640 sensor
- **Day/Night Operation**: Automatic IR Cut filter switching for 24/7 monitoring
- **Motion-Triggered Capture**: PIR sensor integration with configurable sensitivity
- **Solar Power Optimization**: Extended battery life with intelligent power management
- **Remote Monitoring**: WiFi connectivity with web interface and API access
- **Field-Ready Design**: Weatherproof operation with autonomous functionality

### Key Improvements Over Standard Implementation

- **Full OV5640 Support**: Optimized register settings for wildlife photography
- **IR Cut Integration**: Hardware-controlled day/night filter switching
- **Enhanced Motion Detection**: Advanced PIR sensor integration with weather filtering
- **Power Management**: Solar charging with deep sleep optimization
- **Professional Features**: Auto focus, image quality optimization, and metadata logging

## Hardware Requirements

### Primary Components

- **ESP32 Board**: LilyGO T-Camera Plus S3 (ESP32-S3 based)
- **Camera Sensor**: OV5640 5MP module with auto focus capability
- **IR Cut Filter**: AP1511B_FBC controlled via GPIO16
- **Motion Sensor**: PIR sensor connected to GPIO17
- **Storage**: MicroSD card (Class 10, 32GB+ recommended)
- **Power System**: Solar panel + LiPo battery with voltage monitoring

### Pin Configuration (T-Camera Plus S3)

| Component | GPIO Pin | Function | Notes |
|-----------|----------|----------|-------|
| IR Cut Filter | 16 | AP1511B_FBC Control | Day/night switching |
| PIR Sensor | 17 | Motion Detection | Configurable sensitivity |
| Camera XCLK | 15 | External Clock | 20MHz for OV5640 |
| Camera SDA | 4 | I2C Data | Sensor communication |
| Camera SCL | 5 | I2C Clock | Sensor communication |
| Camera Data | 8-18 | Parallel Data | 8-bit parallel interface |
| Camera VSYNC | 6 | Vertical Sync | Frame synchronization |
| Camera HREF | 7 | Horizontal Ref | Line synchronization |
| Camera PCLK | 13 | Pixel Clock | Data timing |
| Battery ADC | 1 | Voltage Monitor | Battery level sensing |
| Solar ADC | 2 | Voltage Monitor | Solar panel monitoring |

### Power System Specifications

- **Operating Voltage**: 3.3V (regulated from battery)
- **Battery**: 3.7V LiPo, 2000mAh minimum recommended
- **Solar Panel**: 6V, 2W minimum for continuous operation
- **Charging Circuit**: Built-in LiPo charging with overcurrent protection
- **Deep Sleep Current**: <10µA with PIR wake-up capability

## Software Features

### Camera System

#### OV5640 5MP Sensor Optimization
```cpp
// Supported resolutions
WILDLIFE_QVGA    // 320x240   - Low power mode
WILDLIFE_VGA     // 640x480   - Standard monitoring
WILDLIFE_SVGA    // 800x600   - Enhanced detail
WILDLIFE_HD      // 1280x720  - HD video quality
WILDLIFE_SXGA    // 1280x1024 - High resolution
WILDLIFE_UXGA    // 1600x1200 - Very high resolution
WILDLIFE_FHD     // 1920x1080 - Full HD
WILDLIFE_QSXGA   // 2592x1944 - Full 5MP resolution
```

#### Image Quality Settings
- **JPEG Quality**: 1-63 (lower = better quality)
- **Auto Focus**: Hardware-assisted focusing for sharp captures
- **Wildlife Optimization**: Color and exposure settings optimized for animals
- **Low Light Performance**: Enhanced settings for dawn/dusk captures

### IR Cut Filter Control

#### Automatic Day/Night Switching
```cpp
// Time-based switching
bool isDaytime() {
    return (current_hour >= 6 && current_hour < 18);
}

// Automatic adjustment
camera.autoAdjustIRCut();
```

#### Manual Control
```cpp
// Enable IR Cut filter (day mode)
camera.setIRCutFilter(IR_CUT_ENABLED);

// Disable IR Cut filter (night mode)
camera.setIRCutFilter(IR_CUT_DISABLED);

// Toggle current state
camera.toggleIRCutFilter();
```

### Motion Detection System

#### PIR Sensor Configuration
```cpp
// Initialize PIR sensor
pirSensor.initialize();

// Set sensitivity (0.0 - 1.0)
pirSensor.setSensitivity(0.8f);

// Check for motion
if (pirSensor.hasMotion()) {
    captureWildlifeImage("motion");
}
```

#### Advanced Motion Filtering
- **Weather Compensation**: Temperature and humidity filtering
- **False Positive Reduction**: Wind and environmental noise filtering
- **Debounce Control**: Configurable motion timeout periods
- **Sensitivity Adjustment**: Real-time sensitivity tuning

### Power Management

#### Solar Power Integration
```cpp
// Initialize power management
powerManager.init();

// Monitor battery status
float batteryVoltage = powerManager.getBatteryVoltage();
float solarVoltage = powerManager.getSolarVoltage();
bool isCharging = powerManager.isCharging();
```

#### Deep Sleep Operation
```cpp
// Configure wake-up sources
esp_sleep_enable_ext0_wakeup(PIR_SENSOR_PIN, HIGH);
esp_sleep_enable_timer_wakeup(SLEEP_DURATION_S * 1000000ULL);

// Enter deep sleep
esp_deep_sleep_start();
```

## Installation Guide

### Step 1: Hardware Assembly

1. **Mount the Camera Module**
   - Connect OV5640 camera to T-Camera Plus S3
   - Ensure all data lines are properly connected
   - Verify I2C connections (SDA/SCL)

2. **Install IR Cut Filter**
   - Connect AP1511B_FBC control to GPIO16
   - Mount filter mechanism in optical path
   - Test mechanical operation

3. **Connect PIR Sensor**
   - Wire PIR sensor to GPIO17
   - Add pull-up resistor if required
   - Position for optimal motion detection

4. **Power System Setup**
   - Install LiPo battery with proper polarity
   - Connect solar panel to charging circuit
   - Verify voltage monitoring on GPIO1/GPIO2

### Step 2: Software Installation

1. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

2. **Clone Repository**
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM
   ```

3. **Configure WiFi**
   ```bash
   cp wifi_config.h.template wifi_config.h
   # Edit wifi_config.h with your network settings
   ```

4. **Build and Upload**
   ```bash
   platformio run --target upload --environment esp32-cam
   ```

### Step 3: Initial Configuration

1. **Connect to Serial Monitor**
   ```bash
   platformio device monitor --baud 115200
   ```

2. **Verify Hardware Detection**
   - Check camera sensor detection
   - Verify IR Cut filter operation
   - Test PIR sensor functionality

3. **Configure Settings**
   - Set time zone and location
   - Adjust motion sensitivity
   - Configure capture intervals

## Configuration

### Basic Configuration Options

```cpp
// Wildlife capture settings
WildlifeCaptureSettings settings = {
    .frame_size = WILDLIFE_UXGA,        // 1600x1200 resolution
    .quality = 12,                      // Good quality/size balance
    .enable_autofocus = true,           // Enable auto focus
    .brightness = 0,                    // Standard brightness
    .contrast = 1,                      // Enhanced contrast
    .saturation = 1,                    // Natural colors
    .auto_ir_cut = true,                // Automatic day/night
    .capture_interval_s = 30            // 30 second intervals
};
```

### Advanced Configuration

#### Camera Optimization
```cpp
// High quality settings for detailed captures
WildlifeCaptureSettings high_quality = getHighQualityWildlifeSettings();
high_quality.frame_size = WILDLIFE_QSXGA;  // Full 5MP
high_quality.quality = 6;                   // Highest quality

// Low light settings for dawn/dusk
WildlifeCaptureSettings low_light = getLowLightWildlifeSettings();
low_light.brightness = 1;                   // Brighter for low light
low_light.frame_size = WILDLIFE_HD;         // Lower res for better SNR
```

#### Power Management
```cpp
// Configure sleep schedule
config.sleep_hour_start = 22;  // 10 PM
config.sleep_hour_end = 6;     // 6 AM

// Set battery thresholds
#define LOW_BATTERY_THRESHOLD 3.4    // Voltage threshold
#define CRITICAL_BATTERY_THRESHOLD 3.2
```

#### Motion Detection
```cpp
// PIR sensor tuning
config.motion_sensitivity = 80;      // 80% sensitivity
config.motion_timeout_ms = 10000;    // 10 second timeout
config.motion_cooldown_ms = 5000;    // 5 second cooldown
```

### Web Interface Configuration

Access the web interface at `http://[device-ip]/` to configure:

- **Capture Settings**: Resolution, quality, intervals
- **Motion Detection**: Sensitivity, timeout, filtering
- **Power Management**: Sleep schedule, battery thresholds
- **Network Settings**: WiFi, MQTT, cloud upload
- **System Monitoring**: Status, diagnostics, logs

## Usage Instructions

### Basic Operation

1. **Power On System**
   - System boots and initializes camera
   - Connects to configured WiFi network
   - Begins monitoring for motion

2. **Monitor Operation**
   - Check status LED for system health
   - Access web interface for remote monitoring
   - Review captured images on SD card

3. **Image Capture**
   - Motion-triggered captures occur automatically
   - Manual captures via web interface or API
   - Time-lapse mode for continuous monitoring

### Field Deployment

#### Site Preparation
1. **Location Selection**
   - Clear line of sight for camera
   - Protected from direct weather
   - Good cellular/WiFi signal if needed
   - Optimal solar panel positioning

2. **Mounting System**
   - Secure camera housing to tree/post
   - Ensure PIR sensor has clear detection zone
   - Position solar panel for maximum sun exposure
   - Weatherproof all connections

3. **Initial Testing**
   - Verify motion detection coverage
   - Test day/night operation
   - Check power system operation
   - Confirm remote connectivity

#### Long-Term Operation
1. **Monitoring Schedule**
   - Daily: Check battery voltage and capture count
   - Weekly: Review image quality and system logs
   - Monthly: Clean solar panel and camera lens
   - Seasonally: Update time settings and maintenance

2. **Data Management**
   - Regular SD card backup or remote upload
   - Image analysis and wildlife identification
   - System performance monitoring
   - Battery and solar system maintenance

### Troubleshooting Common Issues

#### Camera Issues
- **No images captured**: Check camera initialization and SD card
- **Poor image quality**: Verify lens cleanliness and settings
- **Auto focus problems**: Check mechanical focus mechanism

#### Motion Detection Issues
- **False positives**: Adjust sensitivity or add weather filtering
- **Missed detections**: Increase sensitivity or check PIR positioning
- **Continuous triggering**: Check for moving vegetation in detection zone

#### Power Issues
- **Short battery life**: Verify solar panel operation and sleep settings
- **Not charging**: Check solar panel connections and battery condition
- **Premature shutdown**: Monitor battery voltage and adjust thresholds

#### Network Issues
- **WiFi connection failure**: Verify credentials and signal strength
- **Web interface not accessible**: Check IP address and firewall settings
- **Remote uploads failing**: Verify cloud service credentials and connectivity

## API Reference

### REST API Endpoints

#### System Status
```http
GET /api/status
```
Returns current system status including battery, captures, and sensor states.

#### Manual Capture
```http
POST /api/capture
```
Triggers immediate image capture with current settings.

#### Configuration
```http
GET /api/config
POST /api/config
```
Retrieve or update system configuration settings.

#### IR Cut Control
```http
POST /api/ir_cut/enable
POST /api/ir_cut/disable
```
Manually control IR Cut filter state.

#### Image Management
```http
GET /api/images
GET /api/images/{filename}
DELETE /api/images/{filename}
```
List, download, or delete captured images.

### Library API

#### Enhanced Wildlife Camera
```cpp
#include "enhanced_wildlife_camera.h"

EnhancedWildlifeCamera camera;

// Initialize camera
bool success = camera.init(BOARD_LILYGO_T_CAMERA_PLUS);

// Configure settings
WildlifeCaptureSettings settings = getDefaultWildlifeSettings();
camera.configureSensor(settings);

// Capture image
camera_fb_t* image = camera.captureImage();

// Control IR Cut filter
camera.setIRCutFilter(IR_CUT_ENABLED);
```

#### PIR Sensor Integration
```cpp
#include "detection/pir_sensor.h"

PIRSensor pir;

// Initialize sensor
pir.initialize();

// Check for motion
if (pir.hasMotion()) {
    // Handle motion event
    pir.clearMotion();
}

// Configure sensitivity
pir.setSensitivity(0.8f);
```

#### Power Management
```cpp
#include "firmware/src/power_manager.h"

PowerManager power;

// Initialize power management
power.init();

// Monitor power status
float battery = power.getBatteryVoltage();
bool charging = power.isCharging();
PowerState state = power.getPowerState();
```

## Performance Optimization

### Image Quality Optimization

#### OV5640 Register Tuning
The OV5640 sensor offers extensive register-level control for optimal wildlife photography:

```cpp
// Wildlife-specific optimizations applied in applyOV5640Optimizations()
sensor->set_exposure_ctrl(sensor, 1);     // Enable exposure control
sensor->set_aec_value(sensor, 200);       // Optimized for wildlife lighting
sensor->set_agc_gain(sensor, 10);         // Enhanced gain for low light
sensor->set_gainceiling(sensor, GAINCEILING_4X);  // Noise vs. sensitivity balance
```

#### Frame Rate vs. Quality
- **High Quality**: Use QSXGA (5MP) at 5-10 FPS for detailed shots
- **Real-time**: Use HD (720p) at 15-30 FPS for live monitoring
- **Low Power**: Use VGA (480p) for extended battery operation

### Power Consumption Optimization

#### Sleep Mode Configuration
```cpp
// Deep sleep with PIR wake-up
esp_sleep_enable_ext0_wakeup(PIR_SENSOR_PIN, HIGH);
esp_sleep_enable_timer_wakeup(300 * 1000000ULL);  // 5 minute backup timer

// Optimize wake-up latency
esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
```

#### Solar Charging Optimization
- **Panel Sizing**: 2W minimum for continuous operation
- **Battery Capacity**: 2000mAh for 2-3 days autonomy
- **Charge Management**: MPPT charging for maximum efficiency

### Memory Management

#### PSRAM Utilization
```cpp
// Configure camera for PSRAM usage
camera_config_.fb_location = CAMERA_FB_IN_PSRAM;
camera_config_.fb_count = 2;  // Double buffering
```

#### SD Card Performance
- **Class 10 or UHS-1**: Minimum for continuous video recording
- **32GB+**: Recommended capacity for extended deployments
- **Wear Leveling**: Rotate write locations to extend card life

## Field Deployment

### Environmental Considerations

#### Temperature Range
- **Operating**: -20°C to +60°C
- **Storage**: -40°C to +85°C
- **Camera Performance**: Optimal 0°C to +40°C
- **Battery Performance**: Reduced capacity below 0°C

#### Weather Protection
- **IP65 Rating**: Minimum for outdoor deployment
- **Condensation Prevention**: Desiccant packs and ventilation
- **UV Protection**: Camera lens and housing materials
- **Lightning Protection**: Proper grounding and surge protection

#### Wildlife Safety
- **Non-reflective Housing**: Avoid disturbing wildlife behavior
- **Quiet Operation**: Minimize mechanical noise from IR Cut filter
- **Camouflage**: Blend with natural environment
- **Safe Mounting**: Secure against large animal interference

### Maintenance Schedule

#### Daily (Automated)
- Battery voltage monitoring
- Image capture count verification
- System health check logs
- Automatic diagnostics

#### Weekly (Remote)
- Review captured images
- Check system performance logs
- Verify network connectivity
- Monitor storage capacity

#### Monthly (On-site)
- Clean camera lens and housing
- Inspect and clean solar panel
- Check all cable connections
- Verify mounting security
- Update firmware if needed

#### Seasonal (Major)
- Complete system inspection
- Battery performance testing
- Weatherproofing maintenance
- Time zone adjustments
- Data backup and analysis

### Security Considerations

#### Physical Security
- **Tamper Detection**: Motion sensors and enclosure switches
- **Secure Mounting**: Anti-theft mounting systems
- **Camouflaged Design**: Reduce visibility to unauthorized users
- **Location Privacy**: Limit GPS data in image metadata

#### Network Security
- **WPA3 Encryption**: Strong WiFi security protocols
- **VPN Access**: Secure remote monitoring connections
- **Regular Updates**: Firmware and security patches
- **Access Control**: Strong passwords and user authentication

#### Data Protection
- **Encrypted Storage**: SD card encryption for sensitive data
- **Secure Transmission**: HTTPS/TLS for data uploads
- **Privacy Compliance**: Follow local wildlife monitoring regulations
- **Data Retention**: Automated cleanup of old files

## Conclusion

The Enhanced OV5640 Wildlife Camera system provides professional-grade capabilities for wildlife monitoring and research applications. With proper installation, configuration, and maintenance, the system delivers reliable 24/7 operation with high-quality 5MP image capture and intelligent power management.

For additional support and updates, visit the project repository at: https://github.com/thewriterben/ESP32WildlifeCAM

### Support Resources

- **Documentation**: Complete guides and API reference
- **Community Forum**: User discussions and troubleshooting
- **Issue Tracker**: Bug reports and feature requests
- **Video Tutorials**: Step-by-step installation and setup guides
- **Hardware Suppliers**: Recommended component sources