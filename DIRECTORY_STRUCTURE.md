# ESP32-CAM Wildlife Camera - Directory Structure

This document describes the modular directory structure for the WildCAM ESP32 firmware project, following embedded systems best practices.

## Directory Structure

```
WildCAM_ESP32/
├── src/                    # Main source files
│   ├── main.cpp           # Application entry point
│   ├── MotionDetector.cpp # Motion detection implementation
│   ├── CameraManager.cpp  # Camera control and image capture
│   ├── StorageManager.cpp # SD card storage management
│   ├── PowerManager.cpp   # Battery and power management
│   └── WebServer.cpp      # WiFi and web interface
│
├── include/               # Header files
│   ├── config.h          # Configuration constants and pin definitions
│   ├── MotionDetector.h  # Motion detection interface
│   ├── CameraManager.h   # Camera management interface
│   ├── StorageManager.h  # Storage management interface
│   ├── PowerManager.h    # Power management interface
│   └── WebServer.h       # Web server interface
│
├── test/                  # Unit tests
│   └── test_main.cpp     # Main test suite using Unity framework
│
├── lib/                   # Custom libraries (empty, ready for use)
│   └── README.md         # Documentation for custom libraries
│
├── data/                  # Web interface files (HTML, CSS, JS)
│   └── README.md         # Documentation for web files
│
└── platformio.ini        # PlatformIO build configuration
```

## Module Descriptions

### Core Modules

#### MotionDetector
- **Purpose**: Detects motion using PIR sensor
- **Features**: 
  - Configurable sensitivity
  - Cooldown period to prevent multiple triggers
  - Auto-calibration on startup
  - Ready state detection

#### CameraManager
- **Purpose**: Controls the ESP32-CAM camera module
- **Features**:
  - Image capture with configurable quality
  - Frame size adjustment
  - Flash LED control
  - Power management (deinitialize when not in use)

#### StorageManager
- **Purpose**: Manages SD card storage for captured images
- **Features**:
  - Automatic filename generation with timestamps
  - Storage space monitoring
  - Image counting and listing
  - Cleanup of old images

#### PowerManager
- **Purpose**: Manages battery and power consumption
- **Features**:
  - Battery voltage monitoring
  - Battery percentage calculation
  - Low battery detection
  - Deep sleep mode with configurable wakeup
  - External wakeup from PIR sensor

#### WebServer
- **Purpose**: Provides WiFi connectivity and web interface
- **Features**:
  - WiFi connection management
  - Web-based control panel
  - REST API for status and configuration
  - Can be disabled for low-power operation

## Configuration

All configuration constants are centralized in `include/config.h`:

- WiFi credentials
- Camera settings (frame size, quality)
- Motion detection parameters
- Storage settings
- Power management thresholds
- Pin definitions for ESP32-CAM

## Building the Project

This project uses PlatformIO for building and uploading:

```bash
# Install PlatformIO
pip install platformio

# Build the project
pio run

# Upload to ESP32-CAM
pio run --target upload

# Open serial monitor
pio device monitor
```

## Testing

Unit tests are located in the `test/` directory and use the Unity test framework:

```bash
# Run tests
pio test
```

## Usage Example

The modular structure allows easy customization. Here's the basic flow in `main.cpp`:

```cpp
// Initialize all modules
power.begin();
camera.begin();
storage.begin();
motionDetector.begin();
webServer.begin();  // Optional, can be disabled for low power

// Main loop
while (true) {
    if (motionDetector.detectMotion()) {
        camera_fb_t* image = camera.captureImage();
        storage.saveImage(image, filepath);
        camera.releaseFrameBuffer(image);
    }
}
```

## Best Practices Implemented

1. **Separation of Concerns**: Each module handles a specific responsibility
2. **Header/Implementation Split**: Clean interface definitions in headers
3. **Consistent Naming**: Clear, descriptive names following C++ conventions
4. **Configuration Centralization**: All settings in one place
5. **Error Handling**: Return values and status checks throughout
6. **Power Efficiency**: Optional modules and deep sleep support
7. **Testing Infrastructure**: Unit test framework included
8. **Documentation**: README files in each directory

## Extending the System

### Adding a New Module

1. Create header file in `include/`
2. Create implementation in `src/`
3. Add any configuration constants to `include/config.h`
4. Include in `main.cpp` and instantiate
5. Write tests in `test/test_main.cpp`

### Adding Custom Libraries

Place third-party libraries in `lib/` directory with their own subdirectories.

### Adding Web Interface Files

Place HTML, CSS, and JavaScript files in `data/` directory. These can be uploaded to ESP32's SPIFFS filesystem.

## Hardware Requirements

- ESP32-CAM module (AI-Thinker or compatible)
- PIR motion sensor (connected to GPIO 13)
- MicroSD card for storage
- Battery with voltage divider for monitoring (connected to GPIO 34)
- Optional: Flash LED (GPIO 4)

## Pin Configuration

See `include/config.h` for complete pin assignments compatible with ESP32-CAM AI-Thinker board.

## License

[Project License Information]

## Contributors

[Project Contributors]
