# Camera Module Enhancement Checklist

## Image Quality Improvements
- [ ] Implement auto-exposure control
- [ ] Add white balance adjustment
- [ ] Create preset modes (daylight/cloudy/night)
- [ ] Optimize JPEG quality vs file size
- [ ] Add lens distortion correction

## Capture Modes
- [ ] Single shot mode
- [ ] Burst mode (3-5 images)
- [ ] Time-lapse mode with intervals
- [ ] Video clip mode (5-10 seconds)
- [ ] Motion-triggered capture

## Night Vision Support
- [ ] Add IR LED control GPIO
- [ ] Implement IR cut filter detection
- [ ] Create auto-switch day/night mode
- [ ] Adjust exposure for IR illumination

## Current Implementation Status
Based on existing codebase analysis:

### ✅ Already Implemented
- Complete CameraHandler class with sensor configuration
- Wildlife-optimized camera settings in `camera_handler.cpp`
- Auto-exposure and white balance controls
- Lens correction support (configurable)
- Motion-triggered capture via PIR integration
- JPEG quality optimization (configurable 10-63 range)

### 🔄 Enhancement Opportunities
- Multi-shot burst mode implementation
- Video recording capabilities
- Advanced night vision with IR LED control
- Time-lapse scheduling with intervals
- More sophisticated auto-exposure algorithms

## Integration Points
- **Camera Control**: `firmware/src/camera_handler.cpp`
- **Configuration**: `firmware/src/config.h` (lines 24-47)
- **Motion Integration**: `firmware/src/motion_filter.cpp`
- **Main Application**: `firmware/src/main.cpp` handleMotionDetection()

## Supported Camera Modules
Current implementation supports:
- AI-Thinker ESP32-CAM (primary)
- ESP32-S3 variants
- OV2640 and OV5640 sensors
- Multiple board profiles in `configs/board_profiles.h`

## Enhancement Implementation
- [ ] Add burst capture function to CameraHandler
- [ ] Implement video recording with configurable duration
- [ ] Create IR LED control system
- [ ] Add time-lapse scheduler
- [ ] Enhance auto-exposure for wildlife detection

## Validation Tests
- [ ] Image quality comparison across lighting conditions
- [ ] Burst mode timing and memory usage
- [ ] Night vision performance with IR
- [ ] Video recording stability and file sizes
- [ ] Time-lapse accuracy over 24-hour periods