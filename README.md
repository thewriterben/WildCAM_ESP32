# WildCAM_ESP32 - Motion-Activated Wildlife Camera

[![Version](https://img.shields.io/badge/version-0.1.0-green.svg)](https://github.com/thewriterben/WildCAM_ESP32)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32--CAM-orange.svg)](https://www.espressif.com/en/products/socs/esp32)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-compatible-blue.svg)](https://platformio.org/)

> 📘 **Documentation Version:** This README covers the MVP v0.1.0 release with core functionality.  
> For advanced enterprise features (v3.0+), see [README_ADVANCED.md](README_ADVANCED.md).

## Project Status

**MVP v0.1.0 - Functional but in Early Development**

This is a minimal viable product (MVP) release of WildCAM_ESP32, a motion-activated wildlife camera built on the ESP32-CAM platform. The core functionality is working and ready for testing, but this is an early-stage project under active development.

### What Works ✅

- **Motion-triggered image capture** - PIR sensor detects wildlife and triggers camera
- **High-quality images** - UXGA resolution (1600x1200) with adjustable JPEG quality
- **SD card storage** - Automatic file organization with timestamps
- **JSON metadata** - Each image includes timestamp, battery level, and file info
- **Battery monitoring** - Real-time voltage and percentage tracking
- **Web interface** - Simple HTTP server for status and image viewing
- **Deep sleep mode** - Power-efficient sleep between captures
- **Watchdog timer** - Automatic recovery from crashes

### What Doesn't Work Yet ⚠️

- **Advanced AI features** - Species detection and classification (planned for future)
- **LoRa mesh networking** - Long-range communication (hardware dependent)
- **Cloud integration** - Remote data sync and cloud storage
- **Mobile app** - Dedicated mobile application
- **MPPT solar charging** - Advanced battery management (basic monitoring only)
- **Advanced analytics** - Data visualization and reporting

This MVP provides a solid foundation for wildlife monitoring. We're actively working on additional features and welcome community contributions!

## Features

### Core Capabilities

- 🎯 **Motion-triggered Image Capture** - PIR sensor detects motion and automatically captures images
- 💾 **SD Card Storage with Metadata** - Images saved with JSON metadata including timestamps, battery level, and capture count
- 🔋 **Battery Monitoring** - Real-time voltage monitoring and percentage calculation
- 🌐 **Simple Web Interface** - Access camera status, view images, and check system health via web browser
- ⚡ **Deep Sleep Power Management** - Efficient power usage with configurable sleep intervals
- 🔁 **Automatic Recovery** - Watchdog timer prevents system hangs
- 📊 **Status Logging** - Serial output for debugging and monitoring

## Hardware Requirements

### Required Components

| Component | Specification | Estimated Cost | Notes |
|-----------|---------------|----------------|-------|
| **ESP32-CAM Board** | AI-Thinker ESP32-CAM with OV2640 camera | $10-15 | Main controller with integrated camera |
| **MicroSD Card** | Class 10, 16GB-32GB, FAT32 formatted | $5-10 | For image storage |
| **PIR Motion Sensor** | HC-SR501 or similar | $2-5 | Motion detection |
| **USB-to-Serial Adapter** | FTDI, CP2102, or CH340 | $3-8 | For programming (not needed for ESP32-S3) |
| **Power Supply** | 5V 2A USB adapter or 3.7V LiPo battery | $5-15 | Development or field deployment |
| **MicroUSB Cable** | Data + power capable | $2-5 | For power and programming |
| **Jumper Wires** | Female-to-female, 10cm | $2-5 | For PIR sensor connection |

### Optional Components

| Component | Specification | Estimated Cost | Purpose |
|-----------|---------------|----------------|---------|
| **LiPo Battery** | 3.7V 3000mAh+ with protection circuit | $8-15 | Portable power |
| **Solar Panel** | 5V 5W with charge controller | $15-25 | Solar charging |
| **Weatherproof Enclosure** | IP65 rated, clear window | $10-20 | Outdoor protection |
| **Battery Monitor** | Voltage divider circuit or dedicated module | $2-5 | Better voltage accuracy |

**Total Cost Estimate:**
- **Basic Setup:** $25-50 (ESP32-CAM, SD card, PIR sensor, programmer, cables)
- **Field Deployment:** $60-100 (includes battery, solar panel, enclosure)

## Wiring Diagram

### Pin Assignments Table

| Component | ESP32-CAM Pin | Notes |
|-----------|---------------|-------|
| **PIR Sensor** | GPIO 13 | Motion detection input |
| **Battery Monitor** | GPIO 33 (ADC) | Voltage divider recommended (2:1 ratio) |
| **Flash LED** | GPIO 4 | Built-in flash control |
| **SD Card** | Built-in SD_MMC | Uses pins 2, 4, 12, 13, 14, 15 (1-bit mode) |
| **Camera** | Built-in | Uses dedicated camera pins |

### Connection Diagram (ASCII)

```
┌─────────────────────────────────────────────┐
│           ESP32-CAM (AI-Thinker)            │
│                                             │
│  ┌──────┐                                   │
│  │Camera│                                   │
│  │OV2640│                                   │
│  └──────┘                                   │
│                                             │
│  [GPIO 13] ──────────────┐                  │
│                          │                  │
│  [GPIO 33] ──────────────┼──────────────┐   │
│                          │              │   │
│  [GPIO 4]  ──> Flash LED │              │   │
│                          │              │   │
│  [5V]  ────────────────┐ │              │   │
│  [GND] ────────────────┼─┼──────────────┼───┤
│                        │ │              │   │
│  [MicroSD Slot]        │ │              │   │
│    └─ SD Card          │ │              │   │
│                        │ │              │   │
└────────────────────────┼─┼──────────────┼───┘
                         │ │              │
                         │ │              │
    ┌────────────────────┘ │              │
    │  ┌───────────────────┘              │
    │  │                                  │
    │  │  ┌───────────────────────────────┘
    │  │  │
    ▼  ▼  ▼
  ┌──────────┐         ┌──────────────┐
  │   PIR    │         │   Battery    │
  │  Sensor  │         │   Monitor    │
  │ HC-SR501 │         │  (optional)  │
  ├──────────┤         ├──────────────┤
  │ VCC  5V  │◄────────┤ 3.7V Battery │
  │ OUT  D13 │         │   Voltage    │
  │ GND  GND │         │   Divider    │
  └──────────┘         └──────────────┘
```

### Wiring Instructions

1. **PIR Sensor Connection:**
   - VCC → ESP32-CAM 5V pin
   - OUT → ESP32-CAM GPIO 13
   - GND → ESP32-CAM GND

2. **Battery Monitor (Optional):**
   - Create voltage divider: Battery+ → 10kΩ → GPIO 33 → 10kΩ → GND
   - This allows monitoring up to 6.6V battery voltage

3. **Power Supply:**
   - **USB Power:** Connect 5V 2A USB adapter to MicroUSB port
   - **Battery Power:** Connect 3.7V LiPo to 5V and GND pins (use appropriate regulator)

4. **SD Card:**
   - Insert FAT32-formatted MicroSD card (32GB max recommended)
   - No wiring needed - uses built-in SD_MMC interface

⚠️ **Important Notes:**
- Never connect more than 3.3V directly to GPIO pins
- Use voltage divider for battery monitoring (GPIO 33 is analog input)
- PIR sensor requires 5V but output is 3.3V compatible
- Ensure common ground connection for all components

## Software Requirements

### Required Software

1. **PlatformIO** (Recommended)
   - Install via Visual Studio Code extension, or
   - Install via command line: `pip install platformio`
   - Download: https://platformio.org/install

2. **Git** (For cloning repository)
   - Download: https://git-scm.com/downloads

3. **USB Drivers** (If using USB-to-Serial adapter)
   - CP2102: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
   - CH340: http://www.wch.cn/downloads/CH341SER_ZIP.html
   - FTDI: Usually pre-installed on most systems

### Required Libraries (Auto-installed by PlatformIO)

The following libraries are automatically installed when you build the project:

- `espressif/esp32-camera@^2.0.4` - ESP32 camera driver
- `bblanchon/ArduinoJson@^6.21.3` - JSON serialization for metadata
- `me-no-dev/ESP Async WebServer@^1.2.3` - Asynchronous web server
- `me-no-dev/AsyncTCP@^1.1.1` - Async TCP library for web server

These dependencies are defined in `platformio.ini` and will be automatically downloaded during the first build.

## Installation

Follow these step-by-step instructions to get your WildCAM_ESP32 up and running:

### Step 1: Clone the Repository

```bash
# Clone the repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git

# Navigate to the project directory
cd WildCAM_ESP32
```

### Step 2: Install PlatformIO

Choose one of the following methods:

**Method A: PlatformIO IDE (Recommended for beginners)**
1. Install Visual Studio Code from https://code.visualstudio.com/
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Search for "PlatformIO IDE"
5. Click Install
6. Restart VS Code when prompted

**Method B: Command Line**
```bash
# Install Python 3 if not already installed
# Then install PlatformIO
pip install platformio

# Verify installation
pio --version
```

### Step 3: Edit Configuration

1. Open the project in VS Code or your preferred editor
2. Edit `include/config.h` to configure WiFi and settings:

```cpp
// WiFi Settings (required for web interface)
#define WIFI_SSID "YourWiFiNetworkName"
#define WIFI_PASSWORD "YourWiFiPassword"

// Pin Configuration (use defaults unless you have custom wiring)
#define PIR_SENSOR_PIN 13
#define BATTERY_MONITOR_PIN 33
#define FLASH_LED_PIN 4

// Camera Settings
#define DEFAULT_JPEG_QUALITY 10  // Lower = better quality (1-63)
#define DEFAULT_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200

// Power Management
#define DEEP_SLEEP_DURATION_SEC 300  // Sleep for 5 minutes
#define BATTERY_LOW_THRESHOLD 3.3    // Low battery voltage
```

3. Save the file

### Step 4: Build the Firmware

**Using PlatformIO IDE:**
1. Open the project folder in VS Code
2. Click the PlatformIO icon on the left sidebar
3. Under "Project Tasks", expand "esp32cam"
4. Click "Build"
5. Wait for compilation to complete

**Using Command Line:**
```bash
# Build the firmware
pio run -e esp32cam

# This will compile the code and download required libraries
```

### Step 5: Upload to ESP32-CAM

**Hardware Setup for Upload:**
1. Connect your USB-to-Serial adapter to the ESP32-CAM:
   - Adapter TX → ESP32-CAM RX (GPIO 3)
   - Adapter RX → ESP32-CAM TX (GPIO 1)
   - Adapter GND → ESP32-CAM GND
   - Adapter 5V → ESP32-CAM 5V
2. Connect GPIO 0 to GND on ESP32-CAM (enables programming mode)
3. Connect USB adapter to computer

**Upload Firmware:**

Using PlatformIO IDE:
1. Click "Upload" in PlatformIO toolbar
2. Press the RESET button on ESP32-CAM when prompted
3. Wait for upload to complete
4. Disconnect GPIO 0 from GND
5. Press RESET button to start firmware

Using Command Line:
```bash
# Upload to ESP32-CAM
pio run -e esp32cam -t upload

# If upload fails, try:
# 1. Press and hold BOOT/GPIO0 button
# 2. Press RESET button
# 3. Release RESET button
# 4. Run upload command
# 5. Release BOOT button when "Connecting..." appears
```

**Troubleshooting Upload Issues:**
- **"Failed to connect"**: Make sure GPIO 0 is connected to GND
- **"No serial port found"**: Install USB drivers for your adapter
- **"Hash of data verified"**: Upload successful! Disconnect GPIO 0 and reset

### Step 6: Verify Installation

After upload, open the serial monitor to verify operation:

**Using PlatformIO IDE:**
- Click "Serial Monitor" in PlatformIO toolbar
- Select baud rate: 115200

**Using Command Line:**
```bash
pio device monitor -b 115200
```

**Expected Output:**
```
======================================
   WildCAM ESP32 Wildlife Camera
   Firmware Version: 1.0.0
======================================

[INIT] Initializing Power Manager...
Power Manager initialized
Battery ADC: 2048, Voltage: 3.80V
Battery Level: 65%

[INIT] Initializing Camera...
Camera initialized successfully

[INIT] Initializing Storage...
SD Card mounted successfully
Storage Manager initialized

[INIT] Initializing Motion Detector...
Motion Detector initialized

[INIT] Initializing Web Server...
Connecting to WiFi: YourWiFiNetworkName
Connected to WiFi
IP Address: 192.168.1.XXX
WebServer initialized successfully
Web server started on port 80

[READY] System initialization complete!
```

If you see this output, your WildCAM_ESP32 is successfully installed! 🎉

## Usage

### Powering On

1. **Remove GPIO 0 from GND** (if still connected from programming)
2. **Power the device:**
   - Via USB: Connect 5V 2A USB adapter
   - Via Battery: Connect charged 3.7V LiPo battery
3. **Press RESET button** (if needed)
4. **Wait for initialization** (about 5-10 seconds)
5. **Check status LED** - Should blink during startup

### Accessing the Web Interface

The WildCAM_ESP32 provides a simple web interface for monitoring and configuration.

#### Step 1: Find the IP Address

Check the serial monitor for the IP address:
```
Connected to WiFi
IP Address: 192.168.1.123
```

#### Step 2: Open Web Browser

1. Connect your computer/phone to the same WiFi network
2. Open a web browser
3. Navigate to the IP address: `http://192.168.1.123`

#### Step 3: Available Web Endpoints

- **`/`** - Home page with basic info and navigation
- **`/status`** - System status (JSON format):
  ```json
  {
    "camera": "ok",
    "storage": "ok", 
    "motion": "ready",
    "battery": 78,
    "uptime": 1234,
    "imageCount": 42
  }
  ```
- **`/latest`** - View the most recently captured image
- **`/capture`** - Manually trigger a photo capture
- **`/images`** - List of all captured images

### Retrieving Images from SD Card

There are two ways to access your captured wildlife images:

#### Method 1: Remove SD Card (Recommended)

1. **Power off the ESP32-CAM** (disconnect power)
2. **Remove the MicroSD card** from the slot
3. **Insert SD card** into computer's card reader
4. **Navigate to the images folder:**
   - Images: `/WILDLIFE/IMAGES/`
   - Metadata: `/WILDLIFE/METADATA/`
5. **Copy images** to your computer
6. **Safely eject** the SD card
7. **Reinsert** into ESP32-CAM and power on

#### Method 2: Web Interface

1. Open web browser to device IP
2. Navigate to `/images` endpoint
3. Click on image links to view/download
4. Right-click and "Save As" to download

### Image File Organization

Images are automatically organized on the SD card:

```
/WILDLIFE/
├── IMAGES/
│   ├── IMG_20241029_143022.jpg
│   ├── IMG_20241029_143145.jpg
│   └── IMG_20241029_150301.jpg
└── METADATA/
    ├── IMG_20241029_143022.json
    ├── IMG_20241029_143145.json
    └── IMG_20241029_150301.json
```

**Image Filename Format:** `IMG_YYYYMMDD_HHMMSS.jpg`
- Example: `IMG_20241029_143022.jpg` = Image taken on Oct 29, 2024 at 14:30:22

**Metadata JSON Format:**
```json
{
  "timestamp": "2024-10-29T14:30:22",
  "battery_voltage": 3.85,
  "battery_percent": 75,
  "image_size": 98234,
  "image_count": 42
}
```

### Normal Operation

Once powered on and initialized:

1. **Standby Mode:**
   - System waits for motion detection
   - Low power consumption
   - PIR sensor actively monitoring

2. **Motion Detected:**
   - LED blinks to indicate motion
   - Short delay for stabilization (500ms)
   - Image captured at UXGA resolution
   - Image saved to SD card with timestamp
   - Metadata JSON file created
   - System returns to standby

3. **Deep Sleep Mode:**
   - Activates after no motion for configured duration
   - Ultra-low power consumption
   - Wakes up on motion or timer
   - Preserves battery life

### LED Status Indicators

- **Rapid blinking** - Camera initialization
- **Single blink** - Motion detected, capturing image
- **Solid on** - Image being saved
- **Off** - Standby or deep sleep mode

## Configuration

You can customize the behavior of your WildCAM_ESP32 by editing `include/config.h`. Here are the key configuration options:

### WiFi Configuration

```cpp
// WiFi network credentials
#define WIFI_SSID "YourNetworkName"      // Your WiFi SSID
#define WIFI_PASSWORD "YourPassword"     // Your WiFi password

// Web server settings
#define WEB_SERVER_PORT 80               // HTTP server port (default: 80)
#define WEB_SOCKET_PORT 81              // WebSocket port (default: 81)
```

### Camera Settings

```cpp
// Image quality settings
#define DEFAULT_JPEG_QUALITY 10          // JPEG quality: 1-63 (lower = better)
#define DEFAULT_FRAME_SIZE FRAMESIZE_UXGA // Camera resolution

// Available frame sizes:
// - FRAMESIZE_UXGA (1600x1200) - Highest quality, larger files
// - FRAMESIZE_SXGA (1280x1024) - High quality, medium files  
// - FRAMESIZE_XGA (1024x768)   - Good quality, smaller files
// - FRAMESIZE_SVGA (800x600)   - Standard quality, small files
// - FRAMESIZE_VGA (640x480)    - Lower quality, smallest files
```

**Quality vs. File Size Trade-off:**
- Quality 10 + UXGA: ~100KB per image, best quality
- Quality 15 + SXGA: ~60KB per image, good quality
- Quality 20 + XGA: ~40KB per image, acceptable quality

### Motion Detection Settings

```cpp
// Motion sensor configuration
#define PIR_SENSOR_PIN 13                // GPIO pin for PIR sensor
#define MOTION_DEBOUNCE_MS 2000          // Ignore motion for 2s after trigger
#define MOTION_COOLDOWN_MS 5000          // Wait 5s between captures
#define IMAGE_CAPTURE_DELAY_MS 500       // Delay before capture (stabilization)

// Motion sensitivity
#define MOTION_SENSITIVITY 50            // 0-100 (higher = more sensitive)
```

**Tuning Tips:**
- **High false positives?** Increase `MOTION_COOLDOWN_MS` to 10000 (10 seconds)
- **Missing fast animals?** Decrease `IMAGE_CAPTURE_DELAY_MS` to 200
- **Too many triggers?** Increase `MOTION_DEBOUNCE_MS` to 3000

### Power Management

```cpp
// Battery monitoring
#define BATTERY_MONITOR_PIN 33           // ADC pin for voltage reading
#define BATTERY_LOW_THRESHOLD 3.3        // Low battery warning (volts)
#define BATTERY_CRITICAL_THRESHOLD 3.0   // Critical battery shutdown (volts)
#define BATTERY_VOLTAGE_MIN 3.3          // Minimum voltage (empty)
#define BATTERY_VOLTAGE_MAX 4.2          // Maximum voltage (full)

// Deep sleep settings
#define DEEP_SLEEP_DURATION_SEC 300      // Sleep duration (seconds)
                                         // 300 = 5 minutes
                                         // 600 = 10 minutes
                                         // 1800 = 30 minutes
```

**Battery Life Optimization:**
- Increase `DEEP_SLEEP_DURATION_SEC` for longer battery life
- Lower `DEFAULT_JPEG_QUALITY` number increases file size and processing time
- Reduce `DEFAULT_FRAME_SIZE` to save processing power
- Disable WiFi if not needed (comment out `#define WIFI_ENABLED`)

### Storage Settings

```cpp
// SD card storage limits
#define MAX_IMAGES_PER_DAY 1000          // Maximum images per day
#define MAX_IMAGE_SIZE 100000            // Max file size in bytes (~100KB)
#define AUTO_DELETE_DAYS 7               // Auto-delete images older than 7 days
```

**Storage Capacity:**
- 16GB SD card: ~10,000-15,000 images (at default quality)
- 32GB SD card: ~20,000-30,000 images (at default quality)

### Pin Configuration

```cpp
// Hardware pin assignments
#define PIR_SENSOR_PIN 13                // Motion sensor
#define BATTERY_MONITOR_PIN 33           // Battery voltage (ADC)
#define FLASH_LED_PIN 4                  // Flash LED
#define SD_CS_PIN 5                      // SD card chip select (not used in SD_MMC mode)
```

⚠️ **Warning:** Only change pin assignments if you have custom hardware. Incorrect pin configuration can prevent the camera from working.

### Applying Configuration Changes

After editing `config.h`:

1. **Save the file**
2. **Rebuild the firmware:**
   ```bash
   pio run -e esp32cam
   ```
3. **Upload to ESP32-CAM:**
   ```bash
   pio run -e esp32cam -t upload
   ```
4. **Reset the device** and verify changes via serial monitor

### Configuration Examples

**Example 1: Long-term Wildlife Monitoring**
```cpp
#define DEFAULT_JPEG_QUALITY 15           // Medium quality
#define DEFAULT_FRAME_SIZE FRAMESIZE_SXGA // 1280x1024
#define DEEP_SLEEP_DURATION_SEC 600       // 10 minute sleep
#define MOTION_COOLDOWN_MS 10000          // 10 second cooldown
#define AUTO_DELETE_DAYS 14               // Keep 2 weeks
```

**Example 2: High-Quality Research**
```cpp
#define DEFAULT_JPEG_QUALITY 8            // Best quality
#define DEFAULT_FRAME_SIZE FRAMESIZE_UXGA // 1600x1200
#define DEEP_SLEEP_DURATION_SEC 300       // 5 minute sleep
#define MOTION_COOLDOWN_MS 3000           // 3 second cooldown
#define AUTO_DELETE_DAYS 0                // Never auto-delete
```

**Example 3: Maximum Battery Life**
```cpp
#define DEFAULT_JPEG_QUALITY 20           // Lower quality
#define DEFAULT_FRAME_SIZE FRAMESIZE_SVGA // 800x600
#define DEEP_SLEEP_DURATION_SEC 1800      // 30 minute sleep
#define MOTION_COOLDOWN_MS 15000          // 15 second cooldown
```

## Known Issues

This is an early MVP release. Here are the current known limitations and issues:

### Hardware Limitations

- **SD Card Compatibility:** Some SD cards (especially >32GB or non-Class 10) may not mount properly
  - **Workaround:** Use 16GB or 32GB Class 10 cards, formatted as FAT32
  
- **PIR Sensor False Positives:** HC-SR501 sensors can trigger on temperature changes, sunlight, or plants
  - **Workaround:** Adjust sensor sensitivity pot, increase `MOTION_COOLDOWN_MS`, enable two-factor detection in future versions

- **Battery Voltage Reading Inaccuracy:** Simple voltage divider has ±10% accuracy
  - **Workaround:** Calibrate voltage divider resistors, or use dedicated battery monitor IC

- **Camera Power Consumption:** Camera draws 200-300mA during image capture
  - **Impact:** Limits battery-only operation to a few days without solar
  - **Workaround:** Use larger battery (5000mAh+) or add solar panel

### Software Issues

- **WiFi Connection Delays:** First WiFi connection can take 10-30 seconds
  - **Status:** Normal behavior, no fix planned
  
- **Time/Date Not Persistent:** ESP32 lacks RTC, timestamps reset on power cycle
  - **Planned Fix:** Add NTP sync over WiFi or external RTC module support

- **Web Interface Basic:** Current web UI is minimal HTML, not mobile-optimized
  - **Planned Fix:** Improved responsive web interface in v0.2.0

- **No Image Thumbnails:** Full images loaded in web interface can be slow
  - **Planned Fix:** Thumbnail generation in future version

- **Limited Error Recovery:** Some SD card errors require manual reset
  - **Planned Fix:** Better error handling and automatic retry logic

### Performance Issues

- **Slow SD Card Writes:** Image saving can take 1-3 seconds with slow SD cards
  - **Workaround:** Use Class 10 or UHS-I rated cards

- **Memory Fragmentation:** After hundreds of captures, system may become unstable
  - **Workaround:** Periodic resets (deep sleep helps)
  - **Planned Fix:** Better memory management in v0.2.0

- **Serial Monitor Spam:** Verbose logging can slow down operation
  - **Workaround:** Reduce debug level in `config.h` (set `CORE_DEBUG_LEVEL=1`)

### Field Deployment Considerations

- **Temperature Range:** Components rated for -20°C to 60°C only
  - **Limitation:** Not suitable for extreme climates without specialized hardware

- **Weatherproofing:** No official enclosure design provided yet
  - **Workaround:** Use generic IP65 enclosure with clear window for camera

- **Solar Charging:** Basic battery monitoring only, no MPPT charge controller
  - **Limitation:** Inefficient solar charging
  - **Planned Fix:** MPPT support in Phase 2

### Reporting Issues

Found a bug not listed here? Please help us improve!

**Before reporting:**
1. Check if the issue is already listed above
2. Search [existing GitHub issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
3. Try basic troubleshooting (reboot, re-upload firmware, check connections)

**To report a new issue:**
1. Go to [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues/new)
2. Provide:
   - WildCAM version (v0.1.0)
   - Hardware used (ESP32-CAM model, SD card brand/size, etc.)
   - Steps to reproduce
   - Expected vs actual behavior
   - Serial monitor output (if applicable)
   - Photos of setup (if hardware related)

We appreciate detailed bug reports - they help us fix issues faster! 🐛

## Roadmap

WildCAM_ESP32 is under active development. Here's what we're planning:

### Phase 2: Enhanced Features (Q1 2025)

- **Improved Power Management**
  - MPPT solar charge controller support
  - Advanced battery analytics
  - Adaptive sleep schedules based on motion patterns
  
- **Better Time Keeping**
  - NTP time sync over WiFi
  - External RTC module support (DS3231)
  - Persistent timestamps across power cycles

- **Enhanced Web Interface**
  - Mobile-responsive design
  - Image gallery with thumbnails
  - Real-time statistics dashboard
  - Configuration page (no code editing needed)

- **Additional Sensors**
  - BME280 temperature/humidity/pressure
  - GPS location tagging (optional)
  - Light sensor for day/night detection

### Phase 3: Intelligence (Q2-Q3 2025)

- **Basic AI Features**
  - On-device motion detection (reduce false positives)
  - Simple animal vs. non-animal classification
  - Size estimation

- **Data Analytics**
  - Activity pattern graphs
  - Species frequency counts
  - Time-of-day activity charts

- **Improved Storage**
  - Automatic image compression
  - Duplicate detection
  - Smart deletion (keep best images)

### Phase 4: Connectivity (Q4 2025)

- **LoRa Mesh Networking** (ESP32-S3 only)
  - Multi-camera coordination
  - Extended range communication
  - Shared detection alerts

- **Cloud Integration**
  - Automatic image upload (WiFi)
  - Remote monitoring dashboard
  - Email/SMS alerts

- **Mobile App**
  - iOS and Android companion app
  - Real-time notifications
  - Remote configuration

### Long-Term Vision (2026+)

- **Advanced AI**
  - Species-specific detection (deer, bear, fox, etc.)
  - Behavioral analysis (feeding, running, resting)
  - Individual animal recognition

- **Conservation Platform**
  - Multi-site wildlife tracking
  - Researcher collaboration tools
  - Citizen science integration (iNaturalist, eBird)

- **Professional Features**
  - Video recording support
  - Thermal camera integration
  - Advanced weatherproofing designs

### Community Requests

We're listening to the community! Vote on features or suggest new ones:
- [Feature Requests](https://github.com/thewriterben/WildCAM_ESP32/discussions/categories/feature-requests)
- [Discussion Forum](https://github.com/thewriterben/WildCAM_ESP32/discussions)

**Want to contribute?** See the [Contributing](#contributing) section below! 🚀

## Contributing

We welcome contributions from the community! Whether you're a developer, wildlife enthusiast, or conservation researcher, there are many ways to help improve WildCAM_ESP32.

### Ways to Contribute

- 💻 **Code Contributions** - Bug fixes, new features, optimizations
- 📝 **Documentation** - Improve guides, add tutorials, fix typos
- 🧪 **Testing** - Field testing, bug reports, hardware compatibility
- 🎨 **Design** - Web interface, enclosure designs, diagrams
- 📊 **Data** - Share field deployment results and wildlife captures
- 💡 **Ideas** - Feature suggestions and use case discussions

### Getting Started

1. **Read the Contributing Guide:** [CONTRIBUTING.md](CONTRIBUTING.md)
2. **Check Open Issues:** [Good First Issues](https://github.com/thewriterben/WildCAM_ESP32/labels/good%20first%20issue)
3. **Join Discussions:** [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)

### Development Setup

```bash
# Fork the repository on GitHub

# Clone your fork
git clone https://github.com/YOUR_USERNAME/WildCAM_ESP32.git
cd WildCAM_ESP32

# Create a new branch
git checkout -b feature/your-feature-name

# Make your changes and test thoroughly

# Commit with clear message
git commit -m "Add feature: brief description"

# Push to your fork
git push origin feature/your-feature-name

# Open a Pull Request on GitHub
```

### Code Style Guidelines

- Follow existing code formatting and style
- Add comments for complex logic
- Update documentation for new features
- Test on real hardware before submitting
- Include example usage in comments

### Pull Request Process

1. **Ensure your PR:**
   - Has a clear description of changes
   - Links to related issues (if applicable)
   - Includes updated documentation
   - Has been tested on ESP32-CAM hardware

2. **PR Review:**
   - Maintainers will review within 1-2 weeks
   - Address any requested changes
   - Once approved, PR will be merged

3. **After Merge:**
   - You'll be credited in release notes
   - Your changes will be in the next version

### Community Guidelines

- Be respectful and constructive
- Help newcomers get started
- Share your field deployment experiences
- Report bugs with detailed information
- Suggest features with clear use cases

### Recognition

Contributors will be acknowledged in:
- Release notes
- CONTRIBUTORS.md file
- Project documentation

**Thank you for helping make WildCAM_ESP32 better!** 🙏

For detailed contributing guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### What This Means

✅ **You can:**
- Use this project for commercial purposes
- Modify and distribute your changes
- Use in private projects
- Sublicense the code

❌ **You must:**
- Include the original copyright notice
- Include a copy of the license

⚠️ **No Warranty:**
- Software provided "as is" without warranty
- Authors not liable for any damages

### Third-Party Licenses

This project uses the following open-source libraries:

- **ESP32 Arduino Core** - LGPL 2.1
- **ESP32 Camera Driver** - Apache 2.0
- **ArduinoJson** - MIT License
- **ESPAsyncWebServer** - LGPL 3.0
- **AsyncTCP** - LGPL 3.0

Full license texts for dependencies can be found in their respective repositories.

---

## Support & Community

### Getting Help

- 📖 **Documentation:** You're reading it!
- 🐛 **Bug Reports:** [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- 💬 **Discussions:** [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- 📧 **Email:** wildlife-tech@example.com

### Stay Updated

- ⭐ **Star this repo** to get updates
- 👀 **Watch** for new releases
- 🔔 **Subscribe** to discussions for community posts

### Show Your Support

If you find WildCAM_ESP32 useful for wildlife conservation or research:

- ⭐ Star this repository
- 🐦 Share on social media
- 📸 Share your wildlife photos captured with WildCAM!
- 💝 Sponsor the project (GitHub Sponsors - coming soon)

### Photo Gallery

We'd love to see what you capture with WildCAM_ESP32! Share your best wildlife photos:
- Post in [Show and Tell](https://github.com/thewriterben/WildCAM_ESP32/discussions/categories/show-and-tell)
- Tag us on social media with #WildCAM_ESP32
- Contribute to citizen science platforms (iNaturalist, eBird)

---

<div align="center">

**🦌 Help us protect wildlife through accessible technology! 🦌**

Made with ❤️ for wildlife conservation

[![GitHub stars](https://img.shields.io/github/stars/thewriterben/WildCAM_ESP32.svg?style=social&label=Star)](https://github.com/thewriterben/WildCAM_ESP32)
[![GitHub forks](https://img.shields.io/github/forks/thewriterben/WildCAM_ESP32.svg?style=social&label=Fork)](https://github.com/thewriterben/WildCAM_ESP32/fork)

</div>
