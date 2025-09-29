# Budget Conservation Deployment Guide

## Overview

Budget conservation deployments focus on cost-effective wildlife monitoring for small conservation organizations, community groups, and individual researchers. This scenario emphasizes low cost, simplicity, and reliable basic functionality.

## Recommended Hardware Configuration

### Primary Board: AI-Thinker ESP32-CAM
- **Processor**: ESP32 with optional 4MB PSRAM
- **Camera**: OV2640 (2MP) - proven and cost-effective
- **Storage**: 32GB microSD card (standard grade)
- **Power**: DIY solar panel (5W) + 18650 Li-ion battery (2000mAh)
- **Connectivity**: LoRa mesh (basic) or WiFi hotspot
- **Enclosure**: 3D printed or weatherproof IP65 box

### Cost-Optimized BOM (Bill of Materials)

#### Single Node Components ($60-80 total)
- **AI-Thinker ESP32-CAM**: $8-12
- **OV2640 Camera Module**: Included
- **32GB microSD Card**: $8-10
- **18650 Li-ion Battery**: $5-8
- **TP4056 Charging Module**: $2-3
- **5W Solar Panel**: $15-20
- **PIR Motion Sensor**: $3-5
- **Weatherproof Enclosure**: $10-15
- **Mounting Hardware**: $5-10

#### Optional Upgrades ($20-40 additional)
- **LoRa SX1276 Module**: $8-12 (for mesh networking)
- **GPS Module**: $10-15 (for location tracking)
- **Environmental Sensors**: $10-20 (temperature, humidity)

## 3D Printable Enclosures

### Basic Weather Protection
```stl
// Files available in 3d_models/budget_enclosures/
- basic_wildlife_cam_case.stl      // Main enclosure
- solar_panel_mount.stl            // Panel mounting bracket
- camera_lens_hood.stl             // Lens protection
- battery_compartment.stl          // Secure battery housing

// Common components in 3d_models/common/
- mounting_hardware.stl            // Universal mounting brackets
- cable_management.stl             // Cable routing solutions
- weatherproofing_gaskets.stl      // Flexible sealing gaskets
```

### Camouflage Designs
- **Tree Bark Pattern**: Natural woodland camouflage
- **Rock Texture**: Desert and mountain environments
- **Moss Coverage**: Temperate forest blending
- **Custom Colors**: PLA filament in earth tones

### Assembly Guide
1. Print enclosure parts in PETG for weather resistance
2. Install threaded inserts for secure mounting
3. Apply silicone gasket for waterproofing
4. Install desiccant pack for moisture control

## Power Management Strategy

### DIY Solar System Design

#### Solar Panel Selection
- **Monocrystalline**: 5W panel (optimal for budget builds)
- **Size**: 200x130mm typical footprint
- **Voltage**: 6V output (charges 3.7V battery through TP4056)
- **Cost**: $15-20 from electronics suppliers

#### Battery Configuration
```cpp
// Budget power management settings
#define BUDGET_POWER_PROFILE
#define CAPTURE_INTERVAL_NORMAL 600000    // 10 minutes (good weather)
#define CAPTURE_INTERVAL_POWER_SAVE 1800000 // 30 minutes (low battery)
#define DEEP_SLEEP_THRESHOLD 3.3          // Enter deep sleep below 3.3V
#define POWER_SAVE_THRESHOLD 3.6          // Reduce activity below 3.6V
#define CHARGING_VOLTAGE_MIN 4.0          // Minimum for normal operation
```

### Extended Battery Life Tips
1. **Optimize Sleep Current**: <10µA in deep sleep
2. **Disable Unused Peripherals**: WiFi, Bluetooth when not needed
3. **Intelligent Wake**: PIR-triggered operation only
4. **Adaptive Intervals**: Longer intervals in poor weather
5. **Voltage Monitoring**: Automatic power-save modes

## Simple Network Configuration

### Standalone Mode (Simplest)
- **No mesh networking required**
- **Local storage on microSD card**
- **Periodic WiFi hotspot for data download**
- **Mobile app for configuration and data retrieval**

### Basic LoRa Mesh (Intermediate)
```cpp
// Simple 2-3 node mesh for budget deployments
#define BUDGET_LORA_CONFIG
#define LORA_FREQUENCY 915E6              // US frequency
#define LORA_TX_POWER 14                  // Reduced power consumption
#define LORA_SPREADING_FACTOR 10          // Balance of range/power
#define LORA_BANDWIDTH 125E3              // Standard bandwidth
#define MAX_MESH_NODES 5                  // Limit network complexity
```

### WiFi Hotspot Mode
- **Smartphone Configuration**: Use phone hotspot for setup
- **Scheduled Upload**: Connect daily for data sync
- **Remote Access**: Basic web interface for monitoring
- **Minimal Data Usage**: Compressed images and metadata only

## Camera Configuration Presets

### Survey Mode (Default)
```cpp
// Optimized for wildlife surveys and general monitoring
camera_config_t survey_config = {
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,        // 800x600 - good balance
    .jpeg_quality = 15,                  // Moderate compression
    .fb_count = 1,                       // Single buffer (memory saving)
    .fb_location = CAMERA_FB_IN_DRAM,    // Use DRAM if no PSRAM
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .xclk_freq_hz = 10000000            // Lower frequency for power saving
};
```

### Motion Detection Mode
```cpp
// Fast capture for motion-triggered events
camera_config_t motion_config = {
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_VGA,         // 640x480 - faster processing
    .jpeg_quality = 18,                  // Higher compression for speed
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_DRAM,
    .grab_mode = CAMERA_GRAB_LATEST,
    .xclk_freq_hz = 8000000             // Power optimized
};
```

### Night Mode
```cpp
// Extended exposure for low-light conditions
camera_config_t night_config = {
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_VGA,         // Smaller size for low light
    .jpeg_quality = 12,                  // Higher quality for detail
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_DRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    .xclk_freq_hz = 6000000             // Slower for better low light
};
```

## Community-Friendly Setup

### Mobile App Configuration
```javascript
// Simple React Native app for field setup
const CameraConfig = {
    setupWizard: [
        'Power on camera and wait for WiFi hotspot',
        'Connect smartphone to ESP32WildlifeCam network',
        'Open app and scan for camera',
        'Configure basic settings (interval, quality)',
        'Test camera and motion detection',
        'Set deployment location and schedule'
    ],
    
    basicSettings: {
        captureInterval: '10 minutes',
        imageQuality: 'Medium',
        motionSensitivity: 'Medium',
        nightMode: 'Auto',
        powerSaving: 'Enabled'
    }
};
```

### Web-Based Setup Interface
```html
<!-- Simple HTML interface for budget deployments -->
<!DOCTYPE html>
<html>
<head>
    <title>Wildlife Camera Setup</title>
    <style>
        body { font-family: Arial; margin: 20px; }
        .setting { margin: 10px 0; }
        button { padding: 10px 20px; margin: 5px; }
    </style>
</head>
<body>
    <h1>ESP32 Wildlife Camera Configuration</h1>
    
    <div class="setting">
        <label>Capture Interval:</label>
        <select id="interval">
            <option value="300">5 minutes</option>
            <option value="600" selected>10 minutes</option>
            <option value="1800">30 minutes</option>
        </select>
    </div>
    
    <div class="setting">
        <label>Image Quality:</label>
        <select id="quality">
            <option value="10">High</option>
            <option value="15" selected>Medium</option>
            <option value="20">Low (power saving)</option>
        </select>
    </div>
    
    <button onclick="saveConfig()">Save Configuration</button>
    <button onclick="testCamera()">Test Camera</button>
</body>
</html>
```

## Deployment Automation

### Quick Setup Script
```bash
#!/bin/bash
# budget-deploy.sh - Simple deployment for conservation projects

echo "ESP32 Wildlife Camera - Budget Deployment Setup"
echo "=============================================="

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo "Installing PlatformIO..."
    pip3 install platformio
fi

# Set budget configuration
export DEPLOYMENT_SCENARIO="budget"
export TARGET_BOARD="ai-thinker-esp32-cam"
export NETWORK_MODE="standalone"
export POWER_PROFILE="budget-extended"

# Build and flash firmware
echo "Building budget conservation firmware..."
cd firmware/
cp ../examples/deployment-configs/conservation-config.h src/config.h

echo "Flashing firmware (connect ESP32-CAM to USB programmer)..."
pio run -e ai-thinker-esp32-cam --target upload

echo "Deployment complete! Camera will start in 30 seconds."
echo "Connect to 'ESP32WildlifeCam' WiFi network to configure."
```

### Arduino IDE Setup (Alternative)
```cpp
// Simple Arduino IDE configuration for beginners
#include "esp_camera.h"
#include "WiFi.h"
#include "SD_MMC.h"

// Budget deployment configuration
#define CAMERA_MODEL_AI_THINKER
#define BUDGET_MODE
#define SIMPLE_OPERATION

// Pin definitions for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
    Serial.begin(115200);
    setupCamera();
    setupSDCard();
    setupWiFi();
    Serial.println("Budget Wildlife Camera Ready!");
}

void loop() {
    // Simple capture every 10 minutes
    captureAndStore();
    enterDeepSleep(600); // 10 minutes
}
```

## Data Management

### Local Storage Strategy
```cpp
// Simple file naming and organization
struct ImageFile {
    String filename;    // YYYYMMDD_HHMMSS.jpg
    String path;        // /wildlife/YYYY/MM/DD/
    size_t fileSize;
    time_t timestamp;
};

// Create organized directory structure
void setupSDCardStructure() {
    SD.mkdir("/wildlife");
    SD.mkdir("/config");
    SD.mkdir("/logs");
    
    // Year/Month/Day organization
    String year = String(year());
    String month = String(month());
    String day = String(day());
    
    SD.mkdir("/wildlife/" + year);
    SD.mkdir("/wildlife/" + year + "/" + month);
    SD.mkdir("/wildlife/" + year + "/" + month + "/" + day);
}
```

### Data Retrieval Methods

#### Method 1: Direct SD Card Access
- Remove SD card from camera
- Insert into computer or smartphone
- Copy images and log files
- Analyze data with basic software

#### Method 2: WiFi Download
```cpp
// Simple web server for data download
void startWebServer() {
    server.on("/", handleRoot);
    server.on("/images", handleImageList);
    server.on("/download", handleImageDownload);
    server.on("/config", handleConfiguration);
    server.begin();
}

void handleImageList() {
    String html = "<html><body><h1>Wildlife Images</h1><ul>";
    
    File root = SD.open("/wildlife");
    while (File file = root.openNextFile()) {
        if (file.name().endsWith(".jpg")) {
            html += "<li><a href='/download?file=" + String(file.name()) + "'>";
            html += file.name() + "</a> (" + String(file.size()) + " bytes)</li>";
        }
    }
    
    html += "</ul></body></html>";
    server.send(200, "text/html", html);
}
```

#### Method 3: Mobile App Sync
- Smartphone app connects to camera WiFi
- Automatic sync of new images
- Basic species identification using phone AI
- Cloud backup through phone data connection

## Community Setup Procedures

### Group Deployment Workshop
1. **Planning Session** (2 hours)
   - Site selection and mapping
   - Hardware assembly workshop
   - Configuration training
   - Deployment schedule

2. **Assembly Day** (4 hours)
   - Build enclosures and mount hardware
   - Flash firmware and test operation
   - Configure cameras for deployment sites
   - Create deployment documentation

3. **Deployment Day** (6 hours)
   - Install cameras at selected sites
   - Test operation and data collection
   - Train local team on maintenance
   - Set up monitoring schedule

### Maintenance Training
```markdown
## Monthly Maintenance Checklist

### Visual Inspection
- [ ] Check enclosure for damage or intrusion
- [ ] Verify camera lens is clean and unobstructed
- [ ] Confirm solar panel is clean and positioned correctly
- [ ] Inspect mounting hardware for looseness

### System Status
- [ ] Check battery voltage (should be >3.6V)
- [ ] Verify SD card space (>10% free recommended)
- [ ] Test motion detection and image capture
- [ ] Download and review recent images

### Seasonal Tasks
- [ ] Clean solar panel (monthly)
- [ ] Replace desiccant packs (quarterly)
- [ ] Update firmware (biannually)
- [ ] Battery replacement (annually)
```

## Cost Analysis

### Single Node Deployment
- **Basic Hardware**: $60-80
- **3D Printed Enclosure**: $5-10 (material cost)
- **Installation Time**: 2-3 hours
- **Annual Maintenance**: $10-20

### 5-Node Conservation Network
- **Total Hardware**: $300-400
- **Group Workshop Setup**: $50-100 (materials)
- **Annual Operating Cost**: $50-100
- **Training and Support**: Community volunteer time

### Budget Scaling Options
1. **Single Camera**: $80 total cost
2. **Basic Pair**: $150 (2 cameras, simple mesh)
3. **Small Network**: $350 (5 cameras, basic coordination)
4. **Conservation Area**: $600 (10 cameras, comprehensive coverage)

## Performance Expectations

### Battery Life Projections
- **Optimal Conditions**: 45-60 days (good solar, minimal activity)
- **Average Conditions**: 30-45 days (mixed weather, normal activity)
- **Poor Conditions**: 15-30 days (limited solar, high activity)
- **Winter Operation**: 10-20 days (reduced solar, cold temperatures)

### Image Quality Standards
- **Resolution**: 800x600 pixels (SVGA) standard
- **Compression**: JPEG quality 15 (good detail, manageable file size)
- **File Size**: 50-150KB per image
- **Daily Storage**: 2-8MB (10-50 images per day)

### Detection Performance
- **Motion Sensitivity**: Adjustable (animals >cat size reliable)
- **False Positive Rate**: 10-20% (wind, lighting changes)
- **Detection Range**: 5-10 meters (depends on animal size)
- **Night Performance**: Limited (no IR illumination)

## Integration with Conservation Tools

### iNaturalist Integration
```python
# Simple Python script for iNaturalist upload
import requests
import json
from datetime import datetime

def upload_to_inaturalist(image_path, latitude, longitude, species=None):
    """Upload wildlife images to iNaturalist for community identification"""
    
    url = "https://api.inaturalist.org/v1/observations"
    
    # Basic observation data
    observation = {
        'observation': {
            'observed_on': datetime.now().isoformat(),
            'latitude': latitude,
            'longitude': longitude,
            'description': 'Automated wildlife camera capture',
            'tag_list': 'wildlife-camera,conservation,esp32'
        }
    }
    
    # Add species if known
    if species:
        observation['observation']['species_guess'] = species
    
    # Upload image and metadata
    files = {'file': open(image_path, 'rb')}
    response = requests.post(url, data=observation, files=files)
    
    return response.json()
```

### eBird Integration
```python
# Submit bird observations to eBird
def submit_to_ebird(species, count, location, date):
    """Submit bird sightings to eBird database"""
    
    checklist = {
        'speciesCode': species,
        'count': count,
        'locationId': location,
        'obsDt': date,
        'obsTime': '12:00',
        'subProtocol': 'Camera'
    }
    
    # Submit through eBird API (requires authentication)
    return submit_ebird_checklist(checklist)
```

## Troubleshooting Guide

### Common Issues and Solutions

#### Camera Not Starting
- **Check Power**: Verify battery voltage >3.6V
- **SD Card**: Ensure card is properly inserted and formatted
- **Connections**: Verify all jumper connections are secure
- **Reset**: Hold reset button for 10 seconds

#### Poor Image Quality
- **Clean Lens**: Remove dust and water spots from camera
- **Adjust Position**: Avoid direct sunlight and backlighting
- **Check Settings**: Verify image quality setting in configuration
- **Focus**: Ensure camera is focused at appropriate distance

#### Short Battery Life
- **Solar Position**: Ensure panel faces south (northern hemisphere)
- **Power Settings**: Enable power-saving mode
- **Capture Interval**: Increase interval between captures
- **Weather**: Account for seasonal solar variations

#### No Images Captured
- **Motion Detection**: Adjust PIR sensor sensitivity
- **Positioning**: Ensure camera points at animal paths
- **Time Settings**: Verify clock is set correctly
- **Storage**: Check SD card has free space

### Emergency Recovery
```cpp
// Emergency mode for field troubleshooting
void enterEmergencyMode() {
    // Start WiFi hotspot for configuration
    WiFi.softAP("ESP32-Emergency", "wildlife123");
    
    // Minimal web server for diagnostics
    server.on("/status", [](){
        String status = "Battery: " + String(getBatteryVoltage()) + "V\n";
        status += "SD Card: " + String(SD.cardSize() / 1024 / 1024) + "MB\n";
        status += "Free Space: " + String(SD.totalBytes() - SD.usedBytes()) + "\n";
        status += "Last Image: " + getLastImageTime() + "\n";
        server.send(200, "text/plain", status);
    });
    
    // Flash LED pattern to indicate emergency mode
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}
```

## Success Stories and Examples

### Community Conservation Project
- **Location**: Local nature preserve
- **Deployment**: 8 AI-Thinker ESP32-CAM nodes
- **Cost**: $600 total investment
- **Results**: Documented 15 species, identified optimal trail locations
- **Impact**: Informed habitat management decisions

### School Science Project
- **Location**: School campus and nearby woods
- **Deployment**: 3 cameras for student research
- **Cost**: $250 including materials for student assembly
- **Results**: Year-long wildlife behavior study
- **Impact**: Students presented findings at science fair

### Individual Researcher
- **Location**: Private property wildlife corridor
- **Deployment**: Single camera monitoring deer paths
- **Cost**: $80 for complete system
- **Results**: 6 months of deer movement data
- **Impact**: Informed property management for wildlife access

## Next Steps and Upgrades

### Upgrade Paths
1. **Add LoRa Mesh**: Connect multiple cameras ($20/node upgrade)
2. **GPS Tracking**: Add location services ($15 upgrade)
3. **Environmental Sensors**: Temperature, humidity monitoring ($20 upgrade)
4. **Cellular Connectivity**: Remote data access ($40 upgrade)
5. **AI Processing**: Basic species identification ($30 upgrade)

### Scaling Considerations
- Start with single camera to learn system
- Add nodes gradually to build mesh network
- Upgrade to research-grade hardware as budget allows
- Consider solar panel upgrades for harsh environments

---

*Budget conservation deployments prove that effective wildlife monitoring doesn't require expensive equipment. With careful planning and community effort, conservation organizations can achieve meaningful results within tight budget constraints.*