# 3D Reconstruction from Multi-Camera Captures

## Overview

The WildCAM ESP32 system now supports creating 3D models from synchronized multi-camera captures. This advanced capability enables volumetric analysis of wildlife subjects, spatial measurements, and enhanced research opportunities.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Quick Start](#quick-start)
- [Multi-Camera Setup](#multi-camera-setup)
- [Firmware Configuration](#firmware-configuration)
- [Capture Workflow](#capture-workflow)
- [Post-Processing](#post-processing)
- [3D Reconstruction Tools](#3d-reconstruction-tools)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)
- [Examples](#examples)

## Features

- **Synchronized Capture**: Coordinate image capture across multiple ESP32 boards
- **Spatial Metadata**: Store camera position and orientation with each capture
- **Flexible Setup**: Support for various camera array configurations
- **Easy Export**: Automated aggregation and export for popular 3D reconstruction tools
- **Integration**: Seamlessly works with existing multi-board communication system

## Hardware Requirements

### Minimum Configuration
- **3+ ESP32-CAM boards** (more cameras = better 3D models)
- **Network connectivity** (LoRa mesh or WiFi)
- **SD card** on each board for image storage
- **Mounting hardware** to position cameras around subject

### Recommended Configuration
- **5-8 ESP32-CAM boards** for optimal coverage
- **One coordinator board** for synchronization
- **External power** or large batteries for extended sessions
- **Calibrated mounting positions** for accurate spatial data

## Quick Start

### 1. Hardware Setup

Position cameras in a semi-circular or spherical arrangement around your subject:

```
       Camera 2
           |
Camera 1 - Subject - Camera 3
           |
       Camera 4
```

### 2. Enable Multi-Board System

In your firmware `config.h`:

```cpp
#define MULTIBOARD_ENABLED true
#define MULTIBOARD_NODE_ID 1              // Unique for each board
#define MULTIBOARD_PREFERRED_ROLE ROLE_NODE
```

### 3. Configure Camera Positions

Record the position and orientation of each camera relative to a common origin point (e.g., the expected subject location).

### 4. Trigger Synchronized Capture

From the coordinator board:

```cpp
// Trigger 3D capture across all nodes
DynamicJsonDocument params(512);
params["session_id"] = "wildlife_session_001";
params["position_x"] = 2.0;  // meters
params["position_y"] = 0.0;
params["position_z"] = 1.5;
params["orientation_pitch"] = 0.0;  // degrees
params["orientation_yaw"] = 45.0;
params["orientation_roll"] = 0.0;
params["sync_delay_ms"] = 100;  // Synchronization delay

multiboardSystem.sendTaskToNode(1, "3d_capture", params.as<JsonObject>());
// Repeat for each camera node with appropriate positions
```

### 5. Aggregate Images

After capture, use the provided Python tool:

```bash
python3 tools/3d_reconstruction_aggregator.py \
    --session wildlife_session_001 \
    --input /path/to/sd_card/3d_captures \
    --output ./3d_output
```

### 6. Reconstruct 3D Model

Run the generated reconstruction script:

```bash
cd 3d_output
./run_meshroom.sh  # or ./run_colmap.sh
```

## Multi-Camera Setup

### Camera Array Configurations

#### Arc Configuration (3-5 cameras)
Cameras arranged in a 180° arc around the subject:
- Best for: Profile views, side-to-side movement
- Spacing: 30-45° between cameras
- Distance: 2-5 meters from subject

```
     C2      C3      C4
      \      |      /
       \     |     /
        \    |    /
      C1  Subject  C5
```

#### Ring Configuration (6-8 cameras)
Cameras in a 360° ring around the subject:
- Best for: Full 3D capture, rotating subjects
- Spacing: 45-60° between cameras
- Distance: 2-5 meters from subject

```
        C3    C4
    C2            C5
         Subject
    C1            C6
        C8    C7
```

#### Multi-Level Configuration (8+ cameras)
Multiple rings at different heights:
- Best for: Large animals, detailed captures
- Levels: 2-3 height levels
- Per level: 4-6 cameras

### Position Coordinate System

Define a consistent coordinate system for all camera positions:

- **Origin (0,0,0)**: Center of capture area (expected subject location)
- **X-axis**: East-West direction (positive = East)
- **Y-axis**: Vertical direction (positive = Up)
- **Z-axis**: North-South direction (positive = North)
- **Units**: Meters

### Orientation Angles

Camera orientations use standard aerospace conventions:

- **Pitch**: Up/down angle (-90° to +90°)
  - 0° = horizontal
  - Positive = angled up
  - Negative = angled down
  
- **Yaw**: Left/right angle (0° to 360°)
  - 0° = facing North
  - 90° = facing East
  - 180° = facing South
  - 270° = facing West
  
- **Roll**: Tilt angle (-180° to +180°)
  - 0° = level
  - Positive = clockwise rotation
  - Negative = counter-clockwise rotation

## Firmware Configuration

### Task Parameters

The `3d_capture` task supports the following parameters:

```cpp
{
  "session_id": "unique_session_identifier",  // Required: Session identifier
  "position_x": 2.0,                          // Required: X coordinate (meters)
  "position_y": 0.0,                          // Required: Y coordinate (meters)
  "position_z": 1.5,                          // Required: Z coordinate (meters)
  "orientation_pitch": 0.0,                   // Required: Pitch angle (degrees)
  "orientation_yaw": 45.0,                    // Required: Yaw angle (degrees)
  "orientation_roll": 0.0,                    // Required: Roll angle (degrees)
  "sync_delay_ms": 100,                       // Optional: Sync delay (milliseconds)
  "folder": "/3d_captures"                    // Optional: Storage folder
}
```

### Example Coordinator Code

```cpp
#include "multi_board/multi_board_system.h"

void trigger3DCapture() {
    String sessionId = "session_" + String(millis());
    
    // Camera 1: Front-left
    DynamicJsonDocument params1(512);
    params1["session_id"] = sessionId;
    params1["position_x"] = -2.0;
    params1["position_y"] = 0.0;
    params1["position_z"] = 2.0;
    params1["orientation_pitch"] = 0.0;
    params1["orientation_yaw"] = 45.0;
    params1["orientation_roll"] = 0.0;
    params1["sync_delay_ms"] = 100;
    multiboardSystem.sendTaskToNode(1, "3d_capture", params1.as<JsonObject>());
    
    // Camera 2: Front-right
    DynamicJsonDocument params2(512);
    params2["session_id"] = sessionId;
    params2["position_x"] = 2.0;
    params2["position_y"] = 0.0;
    params2["position_z"] = 2.0;
    params2["orientation_pitch"] = 0.0;
    params2["orientation_yaw"] = 315.0;
    params2["orientation_roll"] = 0.0;
    params2["sync_delay_ms"] = 100;
    multiboardSystem.sendTaskToNode(2, "3d_capture", params2.as<JsonObject>());
    
    // Camera 3: Top
    DynamicJsonDocument params3(512);
    params3["session_id"] = sessionId;
    params3["position_x"] = 0.0;
    params3["position_y"] = 3.0;
    params3["position_z"] = 0.0;
    params3["orientation_pitch"] = -45.0;
    params3["orientation_yaw"] = 0.0;
    params3["orientation_roll"] = 0.0;
    params3["sync_delay_ms"] = 100;
    multiboardSystem.sendTaskToNode(3, "3d_capture", params3.as<JsonObject>());
    
    Serial.println("3D capture triggered for session: " + sessionId);
}
```

## Capture Workflow

### Step-by-Step Process

1. **Setup Phase**
   - Position all camera nodes
   - Measure and record positions/orientations
   - Power on all boards
   - Verify network connectivity

2. **Calibration Phase**
   - Point all cameras at a common calibration target
   - Capture test images
   - Verify synchronization timing

3. **Capture Phase**
   - Trigger synchronized capture when subject is in position
   - Monitor capture completion on each node
   - Verify images were saved successfully

4. **Collection Phase**
   - Remove SD cards from each node
   - Copy images to computer
   - Verify all images and metadata are present

5. **Processing Phase**
   - Run aggregation script
   - Execute 3D reconstruction
   - Export/analyze results

### Automated Capture

For repeated captures (e.g., time-lapse 3D):

```cpp
void setup3DTimeLapse() {
    // Configure periodic 3D captures
    unsigned long interval = 300000;  // Every 5 minutes
    
    while (true) {
        trigger3DCapture();
        delay(interval);
    }
}
```

## Post-Processing

### Image Aggregation Tool

The `3d_reconstruction_aggregator.py` script automates the collection and preparation of images:

**Features:**
- Finds all images for a capture session
- Validates synchronization timing
- Copies images with standardized naming
- Exports camera calibration data
- Generates reconstruction scripts

**Usage:**

```bash
# List available sessions
python3 tools/3d_reconstruction_aggregator.py --list --input /sd/3d_captures

# Process a specific session
python3 tools/3d_reconstruction_aggregator.py \
    --session session_001 \
    --input /sd/3d_captures \
    --output ./reconstruction_output
```

**Output Structure:**
```
reconstruction_output/
├── images/                      # Synchronized images
│   ├── cam_001_0000.jpg
│   ├── cam_002_0001.jpg
│   └── cam_003_0002.jpg
├── metadata/
│   └── camera_calibration.json  # Camera positions/orientations
├── run_meshroom.sh              # Meshroom script
├── run_colmap.sh                # COLMAP script
└── README.md                    # Session documentation
```

## 3D Reconstruction Tools

### Option 1: Meshroom (Recommended for Beginners)

**Meshroom** is a free, open-source photogrammetry application with a user-friendly GUI.

**Installation:**
- Download from: https://alicevision.org/#meshroom
- Available for Windows, Linux, macOS
- Requires CUDA-capable GPU for best performance

**Usage:**
1. Open Meshroom
2. Drag and drop images from `reconstruction_output/images/`
3. Click "Start" to begin reconstruction
4. Export mesh when complete

**Or use the provided script:**
```bash
cd reconstruction_output
./run_meshroom.sh
```

### Option 2: COLMAP (Advanced Users)

**COLMAP** is a powerful command-line SfM (Structure-from-Motion) and MVS (Multi-View Stereo) pipeline.

**Installation:**
- Download from: https://colmap.github.io/
- Available for Windows, Linux, macOS
- Command-line interface

**Usage:**
```bash
cd reconstruction_output
./run_colmap.sh
```

**Output:**
- Sparse reconstruction: `colmap_workspace/sparse/0/`
- Dense point cloud: `colmap_workspace/dense/fused.ply`

### Option 3: Other Tools

Additional compatible tools:
- **OpenMVG**: Open Multiple View Geometry
- **VisualSFM**: Visual Structure from Motion
- **Regard3D**: Open-source photogrammetry
- **Agisoft Metashape**: Professional (commercial)

## Best Practices

### Camera Placement

✅ **Do:**
- Maintain 30-60° spacing between adjacent cameras
- Ensure significant overlap in camera fields of view
- Position cameras at consistent heights (for basic setups)
- Use multiple height levels for complex subjects
- Keep cameras 2-5 meters from subject

❌ **Don't:**
- Place cameras too close together (< 20° spacing)
- Point cameras away from subject
- Mix drastically different camera heights without planning
- Position cameras too far (> 10 meters for wildlife)

### Synchronization

- **Target timing**: < 500ms between all captures
- **Wildlife subjects**: Faster is better (< 100ms ideal)
- **Stationary subjects**: More tolerance (< 2s acceptable)
- **Use sync_delay_ms**: Accounts for network latency

### Lighting

- **Consistent lighting**: Crucial for good reconstruction
- **Avoid direct sunlight**: Can cause overexposure
- **Diffused light**: Best for even illumination
- **Consider time of day**: Golden hour is ideal
- **Avoid shadows**: Can confuse reconstruction algorithms

### Subject Requirements

**Ideal subjects:**
- Stationary or slow-moving animals
- Distinct textures and patterns
- Good size (fills 30-50% of frame)
- Well-lit

**Challenging subjects:**
- Fast-moving animals (motion blur)
- Uniform coloring (hard to match features)
- Very small subjects (< 5% of frame)
- Reflective or transparent surfaces

## Troubleshooting

### Issue: Images Not Synchronized

**Symptoms:**
- Time differences > 1 second between captures
- Visible subject movement between images

**Solutions:**
- Check network connectivity between nodes
- Reduce sync_delay_ms value
- Use wired triggers instead of wireless
- Verify coordinator is sending tasks simultaneously

### Issue: Poor Reconstruction Quality

**Symptoms:**
- Sparse or incomplete 3D model
- Noisy point cloud
- Failed reconstruction

**Solutions:**
- Increase number of cameras
- Improve camera overlap
- Ensure consistent lighting
- Use higher resolution images
- Verify camera calibration data
- Check for motion blur

### Issue: Missing Metadata

**Symptoms:**
- Aggregator can't find camera positions
- Missing *_3d_meta.json files

**Solutions:**
- Verify SD card is working
- Check available storage space
- Ensure proper folder permissions
- Confirm task parameters were sent correctly

### Issue: Reconstruction Crashes

**Symptoms:**
- Meshroom/COLMAP fails during processing
- Out of memory errors

**Solutions:**
- Reduce image resolution
- Use fewer images
- Ensure sufficient RAM (16GB+ recommended)
- Close other applications
- Use cloud processing services

## Examples

### Example 1: Basic Wildlife Monitoring

**Setup:**
- 4 cameras in arc formation
- 3 meters from subject area
- 45° spacing
- Ground level

**Code:**
```cpp
void captureWildlife3D() {
    String session = "wildlife_" + String(millis());
    float distance = 3.0;
    
    for (int i = 0; i < 4; i++) {
        float angle = -67.5 + (i * 45.0);  // -67.5, -22.5, 22.5, 67.5
        float x = distance * sin(radians(angle));
        float z = distance * cos(radians(angle));
        
        DynamicJsonDocument params(512);
        params["session_id"] = session;
        params["position_x"] = x;
        params["position_y"] = 0.0;
        params["position_z"] = z;
        params["orientation_yaw"] = angle + 180.0;
        params["sync_delay_ms"] = 50;
        
        multiboardSystem.sendTaskToNode(i+1, "3d_capture", params.as<JsonObject>());
    }
}
```

### Example 2: Bird Monitoring

**Setup:**
- 6 cameras in hexagonal formation
- 2 meters from feeder
- Multi-level (2 heights)

**Code:**
```cpp
void captureBird3D() {
    String session = "bird_" + String(millis());
    
    // Lower ring (3 cameras)
    for (int i = 0; i < 3; i++) {
        float angle = i * 120.0;
        float x = 2.0 * sin(radians(angle));
        float z = 2.0 * cos(radians(angle));
        
        DynamicJsonDocument params(512);
        params["session_id"] = session;
        params["position_x"] = x;
        params["position_y"] = 0.5;
        params["position_z"] = z;
        params["orientation_pitch"] = 10.0;
        params["orientation_yaw"] = angle + 180.0;
        params["sync_delay_ms"] = 30;
        
        multiboardSystem.sendTaskToNode(i+1, "3d_capture", params.as<JsonObject>());
    }
    
    // Upper ring (3 cameras)
    for (int i = 0; i < 3; i++) {
        float angle = 60.0 + (i * 120.0);
        float x = 2.0 * sin(radians(angle));
        float z = 2.0 * cos(radians(angle));
        
        DynamicJsonDocument params(512);
        params["session_id"] = session;
        params["position_x"] = x;
        params["position_y"] = 1.5;
        params["position_z"] = z;
        params["orientation_pitch"] = -15.0;
        params["orientation_yaw"] = angle + 180.0;
        params["sync_delay_ms"] = 30;
        
        multiboardSystem.sendTaskToNode(i+4, "3d_capture", params.as<JsonObject>());
    }
}
```

### Example 3: Research Study

**Setup:**
- 8 cameras, full 360° coverage
- Calibrated positions
- Automated time-lapse captures

**Code:**
```cpp
void research3DTimeLapse() {
    // Schedule captures every 30 minutes for 24 hours
    unsigned long captureInterval = 1800000;  // 30 minutes
    unsigned long studyDuration = 86400000;   // 24 hours
    unsigned long startTime = millis();
    
    int captureCount = 0;
    
    while (millis() - startTime < studyDuration) {
        String session = "research_" + String(captureCount);
        
        // Trigger 360° capture
        for (int i = 0; i < 8; i++) {
            float angle = i * 45.0;
            float x = 4.0 * sin(radians(angle));
            float z = 4.0 * cos(radians(angle));
            
            DynamicJsonDocument params(512);
            params["session_id"] = session;
            params["position_x"] = x;
            params["position_y"] = 1.0;
            params["position_z"] = z;
            params["orientation_yaw"] = angle + 180.0;
            params["sync_delay_ms"] = 20;
            
            multiboardSystem.sendTaskToNode(i+1, "3d_capture", params.as<JsonObject>());
        }
        
        captureCount++;
        delay(captureInterval);
    }
}
```

## Advanced Topics

### Camera Calibration

For best results, perform camera calibration to determine intrinsic parameters:

1. Use a checkerboard calibration pattern
2. Capture multiple images from different angles
3. Use OpenCV or MATLAB calibration tools
4. Store calibration parameters in metadata

### Real-Time Preview

Implement live preview to verify camera alignment:

```cpp
void preview3DSetup() {
    // Stream low-res preview from each camera
    // Display overlays showing coverage
    // Adjust positions before final capture
}
```

### Automated Subject Detection

Trigger 3D capture when wildlife is detected:

```cpp
void auto3DCapture() {
    if (motionDetected() && speciesIdentified()) {
        trigger3DCapture();
    }
}
```

## References

- [AliceVision Meshroom](https://alicevision.org/)
- [COLMAP Documentation](https://colmap.github.io/)
- [Photogrammetry Basics](https://en.wikipedia.org/wiki/Photogrammetry)
- [Multi-View Stereo](https://en.wikipedia.org/wiki/3D_reconstruction_from_multiple_images)

## Support

For questions and support:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: See `firmware/src/multi_board/README.md`

---

**Last Updated:** 2025-10-14  
**Version:** 1.0.0
