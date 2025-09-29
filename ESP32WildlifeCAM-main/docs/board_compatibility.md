# Board Compatibility Guide

This document provides comprehensive information about ESP32 camera board compatibility for the Wildlife Monitoring System.

## Supported Boards

### Currently Implemented

| Board | Status | Chip | Camera | PSRAM | Flash | Power Mgmt |
|-------|--------|------|--------|-------|-------|------------|
| AI-Thinker ESP32-CAM | ✅ Full | ESP32 | OV2640 | Optional | 4MB | External |
| ESP-EYE | ✅ Full | ESP32 | OV2640 | 8MB | 16MB | Advanced |
| LilyGO T-Camera Plus S3 OV5640 V1.1 | ✅ Full | ESP32-S3 | OV5640 | 8MB | 16MB | Integrated |

### Planned Support

| Board | Status | Chip | Camera | PSRAM | Flash | Power Mgmt |
|-------|--------|------|--------|-------|-------|------------|
| ESP32-S3-CAM | 🔄 In Progress | ESP32-S3 | OV2640/OV5640 | 8MB | 16MB | Integrated |
| M5Stack Timer Camera | 🔄 Planned | ESP32 | OV3660 | 4MB | 16MB | Battery |
| TTGO T-Camera | 🔄 Planned | ESP32 | OV2640 | 4MB | 16MB | Display |
| XIAO ESP32S3 Sense | 🔄 Planned | ESP32-S3 | OV2640 | 8MB | 8MB | Ultra-compact |
| FireBeetle ESP32-E | 🔄 Planned | ESP32 | OV2640 | 4MB | 16MB | Professional |
| ESP32-S3-EYE | 🔄 Planned | ESP32-S3 | OV2640 | 8MB | 16MB | AI Vision |
| Freenove ESP32-WROVER | 🔄 Planned | ESP32 | OV2640 | 8MB | 16MB | Educational |

## Board Detection

The system automatically detects board type using multiple methods:

### 1. Hardware Fingerprinting
- GPIO pin configuration analysis
- I2C device scanning
- Pin availability testing

### 2. Chip Identification
- ESP32 vs ESP32-S3 vs ESP32-S2 detection
- PSRAM availability check
- Flash memory size detection

### 3. Camera Sensor Probing
- I2C communication test
- Sensor ID reading
- Capability detection

## Usage Examples

### Automatic Detection
```cpp
#include "camera_handler.h"

void setup() {
    // Automatic board detection and initialization
    if (CameraHandler::init()) {
        Serial.println("Camera initialized successfully");
        
        // Get detected board information
        CameraStatus status = CameraHandler::getStatus();
        Serial.printf("Board: %s\n", status.boardName);
        Serial.printf("Sensor: %s\n", status.sensorName);
    }
}
```

### Manual Board Selection
```cpp
#include "camera_handler.h"
#include "hal/camera_board.h"

void setup() {
    // Force specific board type
    if (CameraHandler::init(BOARD_AI_THINKER_ESP32_CAM)) {
        Serial.println("AI-Thinker ESP32-CAM initialized");
    }
    
    // Or use LilyGO T-Camera Plus S3 OV5640 V1.1
    if (CameraHandler::init(BOARD_LILYGO_T_CAMERA_PLUS_S3)) {
        Serial.println("LilyGO T-Camera Plus S3 OV5640 V1.1 initialized");
    }
}
```

### Direct HAL Usage
```cpp
#include "hal/board_detector.h"

void setup() {
    // Create board instance directly
    auto board = BoardDetector::createBoard();
    if (board && board->init()) {
        Serial.printf("Board: %s\n", board->getBoardName());
        
        // Get board-specific configuration
        GPIOMap gpio_map = board->getGPIOMap();
        CameraConfig cam_config = board->getCameraConfig();
        PowerProfile power_profile = board->getPowerProfile();
        
        // Use board-specific features
        board->flashLED(true);
        delay(500);
        board->flashLED(false);
    }
}
```

## Board-Specific Features

### AI-Thinker ESP32-CAM
- **GPIO Layout**: Standard ESP32-CAM pinout
- **Power**: External 5V or 3.3V supply required
- **Camera**: OV2640 (2MP) standard
- **Flash**: Built-in LED on GPIO 4
- **PSRAM**: Optional, detected automatically
- **Use Cases**: Budget deployments, prototyping

### ESP32-S3-CAM (Planned)
- **GPIO Layout**: Enhanced S3 pinout with more GPIOs
- **Power**: Improved power management
- **Camera**: OV2640/OV5640 support
- **Flash**: High-power LED support
- **PSRAM**: 8MB standard
- **Use Cases**: High-resolution monitoring, AI processing

### ESP-EYE
- **GPIO Layout**: Optimized for vision applications
- **Power**: Advanced power management
- **Camera**: OV2640 with enhanced optics
- **Flash**: Professional flash system on GPIO 21
- **PSRAM**: 8MB for vision processing  
- **Use Cases**: AI-enhanced wildlife recognition, scientific monitoring

## Migration Guide

### From Hardcoded to HAL

**Before (Old Method):**
```cpp
// Hardcoded pin definitions
#define CAMERA_MODEL_AI_THINKER
#include "config.h"

void setup() {
    // Manual camera initialization
    camera_config_t config;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    // ... more manual configuration
}
```

**After (HAL Method):**
```cpp
#include "camera_handler.h"

void setup() {
    // Automatic detection and configuration
    CameraHandler::init();
}
```

### Backward Compatibility

The system maintains full backward compatibility:
- Old `#define CAMERA_MODEL_AI_THINKER` still works
- Existing pin definitions remain valid
- Legacy camera handler functions unchanged

## Troubleshooting

### Board Not Detected
1. Check GPIO connections
2. Verify power supply voltage
3. Test I2C communication
4. Manual board type override

### Camera Initialization Failed
1. Verify camera module connection
2. Check PSRAM requirements
3. Test with lower resolution settings
4. Check power supply current capacity

### Performance Issues
1. Enable PSRAM if available
2. Adjust frame size and quality
3. Optimize power management settings
4. Use board-specific optimizations

## Configuration Files

### Board Profiles (`configs/board_profiles.h`)
- GPIO pin mappings for each board
- Power consumption profiles
- Hardware capability definitions

### Sensor Configurations (`configs/sensor_configs.h`)
- Camera sensor capabilities
- Wildlife photography optimizations
- Power consumption settings

## Adding New Board Support

1. Create new board class inheriting from `CameraBoard`
2. Implement required virtual methods
3. Add GPIO mapping to `board_profiles.h`
4. Update board detector logic
5. Add board type to enumeration
6. Test with hardware

## Performance Comparison

| Board | Power (mA) | Max Resolution | Max FPS | PSRAM | Cost |
|-------|------------|---------------|---------|-------|------|
| AI-Thinker | 200 | 1600x1200 | 30 | Optional | $ |
| ESP32-S3-CAM | 180 | 2592x1944 | 15 | 8MB | $$ |
| ESP-EYE | 190 | 1600x1200 | 30 | 8MB | $$$ |
| M5Stack Timer | 150 | 2048x1536 | 15 | 4MB | $$$ |

## Future Roadmap

- [ ] Complete ESP32-S3-CAM implementation
- [ ] Add ESP-EYE support
- [ ] Implement M5Stack Timer Camera
- [ ] Add TTGO T-Camera support
- [ ] Create XIAO ESP32S3 Sense driver
- [ ] Professional FireBeetle support
- [ ] ESP32-S3-EYE AI integration
- [ ] Educational Freenove support