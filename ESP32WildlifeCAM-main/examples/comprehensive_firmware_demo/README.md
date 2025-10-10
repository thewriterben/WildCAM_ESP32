# Comprehensive Firmware Demo

This example demonstrates the complete integration of all major features from the comprehensive firmware overhaul.

## Features Demonstrated

### 1. Power Management with State Machine
- **PowerHardware Configuration**: Explicit hardware capability declaration
- **State Machine**: NORMAL, CRITICAL, CHARGING states with automatic transitions
- **Intelligent Deep Sleep**: Sleep duration adjusted based on power state
- **Advanced Metrics**: Support for optional INA219 current sensor

### 2. Profile-Based Camera Control
- **HIGH_QUALITY**: Maximum resolution (UXGA 1600x1200), Quality 10
- **BALANCED**: Medium resolution (SVGA 800x600), Quality 12
- **FAST_CAPTURE**: Low resolution (VGA 640x480), Quality 15 for motion detection

### 3. Enhanced File Management
- **YYYY-MM Directory Structure**: Automatic organization by month
- **Power Telemetry in Metadata**: Every capture includes battery voltage, percentage, and power state
- **Graceful Fallback**: Works without SD card if needed

### 4. HYBRID_PIR_THEN_FRAME Motion Detection
- **Low-Power Trigger**: PIR sensor checks first (minimal power consumption)
- **Visual Confirmation**: Frame analysis only if PIR triggers
- **Smart Profile Switching**: FAST_CAPTURE for analysis, HIGH_QUALITY for final capture
- **False Positive Reduction**: Dramatically reduces environmental false triggers

### 5. AI Classifier with Graceful Degradation
- **On-Device Inference**: TensorFlow Lite running on ESP32
- **Graceful Degradation**: System works as standard camera trap if model missing
- **Image Preprocessing**: Uses `image_utils` module for JPEG decode, scaling, normalization
- **Placeholder Scaling**: Currently uses nearest-neighbor (noted for improvement)

## Hardware Requirements

### Minimum Configuration
- ESP32-CAM (AI-Thinker or compatible)
- PIR Motion Sensor (GPIO 1)
- SD Card (for image storage)
- Battery (3.7V LiPo recommended)

### Optional Enhancements
- INA219 Current Sensor (for advanced power metrics)
- Solar Panel with Charging Controller
- Multi-zone PIR sensors

## Setup Instructions

1. **Hardware Connections**:
   - Connect PIR sensor to GPIO 1
   - Connect battery voltage divider to GPIO 33
   - (Optional) Connect solar panel to GPIO 32
   - Insert SD card

2. **Software Configuration**:
   - Open `comprehensive_firmware_demo.ino` in Arduino IDE
   - Select board: "AI Thinker ESP32-CAM"
   - Upload to device

3. **Model Deployment** (Optional):
   - Place `model.tflite` in `/models/` on SD card
   - System will auto-detect and enable AI classification
   - If missing, system operates as standard camera trap

## Usage

### Monitoring Output
Connect serial monitor at 115200 baud to see:
- System initialization status
- Power state changes
- Motion detection events
- AI classification results
- Capture statistics

### Example Output
```
========================================
WildCAM ESP32 - Comprehensive Firmware Demo
========================================

Step 1: Initializing Power Management...
✅ Power management initialized with state machine

Step 2: Initializing SD Card...
✅ SD Card initialized
   Free space: 15234 MB

Step 3: Initializing Camera...
✅ Camera initialized with BALANCED profile

Step 4: Initializing Motion Detector...
✅ Motion detector initialized with HYBRID_PIR_THEN_FRAME mode
   - PIR triggers first (low power)
   - Visual confirmation only if PIR triggered
   - Smart profile switching (FAST -> HIGH_QUALITY)

Step 5: Initializing AI Classifier...
✅ AI classifier initialized
   Graceful degradation: Enabled
   If model.tflite missing: Works without AI

========================================
System Initialization Complete!
========================================

Features Enabled:
  [✓] Power state machine (NORMAL/CRITICAL/CHARGING)
  [✓] Camera profiles (HIGH_QUALITY/BALANCED/FAST_CAPTURE)
  [✓] YYYY-MM directory organization
  [✓] Power telemetry in metadata
  [✓] HYBRID_PIR_THEN_FRAME motion detection
  [✓] AI classification with graceful degradation
  [✓] Image preprocessing with image_utils

Waiting for motion...

📊 Power Status:
   Battery: 3.85V (75.0%)
   State: NORMAL ✓

🎯 Motion Detected!
   Confidence: 0.85
   Description: PIR trigger + visual confirmation: Motion confirmed
   PIR triggered: Yes
   Frame confirmed: Yes
   Process time: 450 ms

📸 Capturing image with HIGH_QUALITY profile...
✅ Image captured successfully
   Filename: /images/2025-01/IMG_20250127_143022_0001.jpg
   Size: 245678 bytes
   Time: 892 ms
   Saved to YYYY-MM directory structure
   Metadata includes power telemetry

🧠 Running AI classification...
   Species: White-tailed Deer
   Confidence: 0.87
   Inference time: 1234 ms

📈 Total captures: 1
```

## File Structure

After running, your SD card will contain:
```
/images/
  └── 2025-01/
      ├── IMG_20250127_143022_0001.jpg
      ├── IMG_20250127_143022_0001.json  (metadata with power telemetry)
      ├── IMG_20250127_150315_0002.jpg
      └── IMG_20250127_150315_0002.json
```

## Metadata Example

Each capture includes a JSON metadata file:
```json
{
  "image_file": "/images/2025-01/IMG_20250127_143022_0001.jpg",
  "timestamp": 1234567890,
  "width": 1600,
  "height": 1200,
  "size_bytes": 245678,
  "format": 3,
  "camera_profile": "HIGH_QUALITY",
  "battery_voltage": 3.85,
  "battery_percentage": 75.0,
  "power_state": "NORMAL",
  "is_charging": false
}
```

## Power Consumption

The HYBRID_PIR_THEN_FRAME mode significantly reduces power consumption:

| Mode | Avg Current | Battery Life (3000mAh) |
|------|-------------|------------------------|
| Continuous Frame Analysis | ~200mA | ~15 hours |
| PIR Only | ~5mA | ~600 hours |
| **HYBRID_PIR_THEN_FRAME** | **~8mA** | **~375 hours** |

*Note: Actual consumption varies based on capture frequency*

## Troubleshooting

### Camera Fails to Initialize
- Check camera ribbon cable connection
- Ensure proper power supply (5V, >500mA)
- Reset device and try again

### No AI Classification
- This is expected if model.tflite is not on SD card
- System operates normally as camera trap
- Add model to enable AI features

### Motion Detection Too Sensitive
- Adjust PIR sensor potentiometer
- Increase confidence threshold in code
- Check PIR sensor positioning

## Future Improvements

As noted in the code, the image scaling currently uses nearest-neighbor interpolation. For better quality:
- Implement bilinear interpolation in `image_utils.cpp`
- Add bicubic interpolation option
- Consider hardware-accelerated scaling

## License

Part of the WildCAM ESP32 project. See main repository LICENSE for details.
