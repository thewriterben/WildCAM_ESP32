# ESP32 Wildlife Camera API Reference

This is a placeholder API reference for the ESP32 Wildlife Camera project.

## Core Classes

### CameraHandler
*Camera management and image capture functionality.*

#### Methods
- `init()` - Initialize camera system
- `captureImage()` - Capture a single image
- `getStatus()` - Get camera status

### MotionFilter
*Motion detection and filtering system.*

#### Methods
- `init()` - Initialize motion detection
- `checkMotion()` - Check for motion events
- `setThreshold()` - Configure motion sensitivity

### SolarManager
*Solar power management and battery monitoring.*

#### Methods
- `init()` - Initialize power management
- `getBatteryLevel()` - Get current battery level
- `getSolarVoltage()` - Get solar panel voltage

### LoraMesh
*LoRa mesh networking functionality.*

#### Methods
- `init()` - Initialize LoRa network
- `sendMessage()` - Send data over LoRa
- `receiveMessage()` - Receive LoRa messages

## Data Structures

### CameraMetadata
```cpp
struct CameraMetadata {
    unsigned long timestamp;
    int batteryLevel;
    bool motionDetected;
    float temperature;
    float humidity;
    int lightLevel;
    int imageWidth;
    int imageHeight;
    float compressionRatio;
};
```

### Configuration Options
*Configuration parameters and their descriptions.*

## Examples

### Basic Usage
```cpp
#include "camera_handler.h"

void setup() {
    CameraHandler::init();
}

void loop() {
    if (motionDetected) {
        CameraHandler::captureImage();
    }
}
```

## Error Codes

*Error codes and their meanings will be documented here.*

---

*This API reference is currently a placeholder and will be expanded with complete documentation.*