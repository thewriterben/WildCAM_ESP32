# Burst and Video Capture Mode Implementation

This document describes the implementation of advanced capture modes for the ESP32 Wildlife Camera system, including burst photography, video recording, time-lapse, and enhanced night vision capabilities.

## Overview

The enhanced `CameraManager` class now supports:
- **Burst Capture Mode**: Rapid sequence photography for wildlife behavior analysis
- **Video Recording**: Short video clips with configurable parameters
- **Time-lapse Photography**: Automated long-term capture sequences
- **Enhanced Night Vision**: IR LED control and automatic day/night switching

## Configuration

### Configuration Constants (config.h)

```cpp
// Burst mode configuration
#define BURST_MODE_ENABLED true
#define BURST_DEFAULT_COUNT 3
#define BURST_DEFAULT_INTERVAL_MS 1000
#define BURST_MAX_COUNT 10

// Video recording configuration  
#define VIDEO_MODE_ENABLED true
#define VIDEO_DEFAULT_DURATION_S 10
#define VIDEO_FRAME_RATE 10

// Time-lapse configuration
#define TIMELAPSE_MODE_ENABLED true
#define TIMELAPSE_DEFAULT_INTERVAL_S 60

// Night vision configuration
#define IR_LED_ENABLED true
#define IR_LED_PIN 4
#define LIGHT_SENSOR_PIN 35
```

## Burst Capture Mode

### Configuration Structure

```cpp
struct BurstConfig {
    uint8_t count = BURST_DEFAULT_COUNT;           // Number of images (1-10)
    uint16_t intervalMs = BURST_DEFAULT_INTERVAL_MS; // Interval between captures (ms)
    uint8_t quality = CAMERA_JPEG_QUALITY;         // JPEG quality
    bool saveIndividual = true;                     // Save individual images
    bool createSequence = false;                    // Create sequence metadata
};
```

### Usage Example

```cpp
CameraManager camera;

// Configure burst capture
CameraManager::BurstConfig config;
config.count = 5;
config.intervalMs = 500;
config.createSequence = true;

// Capture burst
CameraManager::BurstResult result = camera.captureBurst(config, "/burst");

if (result.success) {
    Serial.printf("Captured %d images in %d ms\n", 
                 result.capturedCount, result.totalTime);
}
```

### Features

- **Memory Efficient**: Reuses existing frame buffer management
- **Sequence Metadata**: Optional JSON metadata with capture details
- **Configurable Timing**: 100ms to 5 second intervals
- **Wildlife Optimized**: Fast capture for behavior analysis

## Video Recording

### Configuration Structure

```cpp
struct VideoConfig {
    uint16_t durationSeconds = VIDEO_DEFAULT_DURATION_S; // Recording duration
    uint8_t frameRate = VIDEO_FRAME_RATE;               // Frames per second
    uint8_t quality = VIDEO_QUALITY;                    // Video quality
    framesize_t frameSize = FRAMESIZE_VGA;              // Video frame size
};
```

### Usage Example

```cpp
// Configure video recording
CameraManager::VideoConfig config;
config.durationSeconds = 10;
config.frameRate = 10;
config.frameSize = FRAMESIZE_VGA;

// Record video
CameraManager::VideoResult result = camera.recordVideo(config, "/videos");

if (result.success) {
    Serial.printf("Video saved: %s (%d frames, %d bytes)\n", 
                 result.filename.c_str(), result.frameCount, result.fileSize);
}
```

### Video Format

- **Container**: Simple MJPEG format
- **Encoding**: JPEG frames with timing headers
- **File Extension**: `.mjpg`
- **Frame Headers**: 4-byte size + 4-byte timestamp + frame data

## Time-lapse Photography

### Configuration Structure

```cpp
struct TimeLapseConfig {
    uint32_t intervalSeconds = TIMELAPSE_DEFAULT_INTERVAL_S; // Interval between captures
    uint32_t totalDuration = 0;                             // Total duration (0 = indefinite)
    uint16_t maxImages = 1000;                              // Maximum images
    bool autoCleanup = true;                                // Auto-delete old images
    String folder = "/timelapse";                           // Storage folder
};
```

### Usage Example

```cpp
// Start time-lapse
CameraManager::TimeLapseConfig config;
config.intervalSeconds = 60; // Every minute
config.maxImages = 100;

CameraManager::TimeLapseResult result = camera.startTimeLapse(config);

// In main loop
void loop() {
    if (camera.isTimeLapseActive()) {
        camera.updateTimeLapse(); // Call periodically
    }
}

// Stop when needed
camera.stopTimeLapse();
```

### Features

- **Non-blocking**: Requires periodic `updateTimeLapse()` calls
- **Automatic Management**: Storage cleanup and limits
- **Long-term Operation**: Designed for extended monitoring

## Night Vision Enhancement

### IR LED Control

```cpp
// Manual control
camera.setIRLED(true);  // Enable IR LED
camera.setIRLED(false); // Disable IR LED

// Automatic control
camera.autoAdjustNightMode(); // Auto-adjust based on light sensor
```

### Light Level Detection

```cpp
uint8_t lightLevel = camera.getLightLevel(); // 0-100%
Serial.printf("Light level: %d%%\n", lightLevel);
```

### Features

- **Automatic Switching**: Based on ambient light sensor
- **GPIO Control**: Configurable IR LED pin
- **Light Thresholds**: Night mode < 30%, IR LED < 20%
- **Camera Optimization**: Adjusted exposure and gain for night mode

## Memory Management

### Frame Buffer Efficiency

The implementation reuses the existing memory-efficient frame buffer management:

- **Single Buffer**: Capture, process, save, return
- **No Memory Leaks**: Automatic frame buffer return
- **PSRAM Support**: Leverages existing PSRAM allocation

### Storage Optimization

- **Directory Structure**: Organized by capture type
- **Filename Generation**: Timestamp-based with sequence numbers
- **Metadata Files**: JSON format for burst and time-lapse sequences

## Power Optimization

### Burst Mode Power Savings

- **Minimal Delays**: Only necessary intervals between captures
- **Efficient Processing**: Reuse of existing optimized capture pipeline
- **Statistics Tracking**: Monitor power impact through timing metrics

### Video Recording Considerations

- **Frame Rate Limits**: Configurable to balance quality vs. power
- **Duration Limits**: Maximum recording time to prevent excessive power drain
- **Quality Settings**: Adjustable compression for storage/power trade-offs

## Integration with Existing System

### Backward Compatibility

All existing `CameraManager` functionality remains unchanged:
- `captureImage()` - Single image capture
- `captureToBuffer()` - Frame buffer capture
- `configureSensor()` - Sensor configuration
- Statistics and configuration methods

### Motion Detection Integration

```cpp
// Example: Motion-triggered burst
void onMotionDetected() {
    CameraManager::BurstConfig config;
    config.count = 3;
    config.intervalMs = 200; // Fast burst
    
    camera.captureBurst(config, "/motion");
}
```

## Error Handling

### Robust Operation

- **Validation**: Configuration parameter validation
- **Graceful Degradation**: Partial success handling
- **Resource Management**: Proper cleanup on failures
- **Logging**: Detailed serial output for debugging

### Recovery Mechanisms

- **Frame Buffer Management**: Automatic return on errors
- **File System**: Directory creation and error handling
- **Timeout Protection**: Recording duration limits

## Performance Metrics

### Typical Performance

- **Burst Capture**: ~200-500ms per image (depending on quality)
- **Video Recording**: 10 FPS at VGA resolution
- **Time-lapse**: Minimal overhead between intervals
- **Memory Usage**: Same as single capture (no additional buffers)

### Optimization Recommendations

1. **Burst Mode**: Use lower quality for faster capture
2. **Video Mode**: Reduce frame rate or resolution for longer recordings
3. **Time-lapse**: Consider storage limits for long-term operation
4. **Night Mode**: Use IR LED only when necessary to save power

## Future Enhancements

### Planned Features

- **HDR Capture**: Multiple exposure bracketing
- **Focus Stacking**: Macro photography support
- **Audio Recording**: Sound capture with video
- **Advanced Codecs**: H.264 encoding support
- **Streaming**: Real-time preview capabilities

### Integration Opportunities

- **AI Analysis**: Real-time behavior detection during burst/video
- **Cloud Upload**: Automatic upload of sequences
- **Remote Control**: WiFi/LoRa triggered capture modes
- **Environmental Triggers**: Weather-based capture scheduling

## Troubleshooting

### Common Issues

1. **SD Card Space**: Monitor storage for video/time-lapse
2. **Power Consumption**: Adjust recording parameters
3. **Frame Rate**: Reduce FPS if captures are skipped
4. **File Size**: Balance quality vs. storage requirements

### Debug Information

The system provides detailed logging for troubleshooting:
- Capture timing and success rates
- File sizes and storage usage
- GPIO status for IR LED and light sensor
- Configuration validation results