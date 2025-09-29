# Live Streaming Implementation Guide

## Overview

The ESP32WildlifeCAM now features a comprehensive live streaming system that provides real-time MJPEG video streaming with power-aware operation, motion detection integration, and multi-client support. This document provides complete implementation details, API reference, and usage instructions.

## Architecture

### Core Components

#### StreamManager Class
The main streaming management class located in `src/streaming/stream_manager.h/.cpp` provides:
- MJPEG protocol implementation for web browser compatibility
- Power-aware streaming profiles based on battery level
- Motion-triggered streaming activation
- Multi-client support (up to 3 concurrent viewers)
- Real-time statistics and performance monitoring

#### Stream Configuration
Configuration constants and power profiles defined in `src/streaming/stream_config.h`:
- Streaming performance limits and quality levels
- Power-aware streaming profiles for different battery levels
- Motion detection integration settings
- Stream statistics tracking structures

#### Enhanced Web Server Integration
Extended `firmware/src/web/enhanced_web_server.h/.cpp` with:
- New MJPEG streaming endpoints
- Stream control API endpoints
- WebSocket broadcasting for real-time updates
- Client management and error handling

### Integration Points

#### Existing System Components
- **CameraManager**: Extends existing `captureToBuffer()` for streaming frames
- **PowerManager**: Battery level monitoring for power-aware streaming profiles
- **MotionDetectionManager**: Automatic stream activation on wildlife detection
- **EnhancedWebServer**: WebSocket support for real-time dashboard updates

## Power-Aware Streaming Profiles

The system implements five battery-level-based streaming profiles:

### High Battery (>70%)
- **Frame Rate**: 10 FPS
- **Quality**: High (HD 1280x720)
- **Duration**: 60 minutes
- **Mode**: Continuous streaming
- **JPEG Quality**: 12 (best quality)

### Good Battery (50-70%)
- **Frame Rate**: 7 FPS
- **Quality**: Medium (VGA 640x480)
- **Duration**: 40 minutes
- **Mode**: Continuous streaming
- **JPEG Quality**: 15

### Medium Battery (30-50%)
- **Frame Rate**: 5 FPS
- **Quality**: Medium (VGA 640x480)
- **Duration**: 30 minutes
- **Mode**: Motion-only
- **JPEG Quality**: 18

### Low Battery (15-30%)
- **Frame Rate**: 3 FPS
- **Quality**: Low (QVGA 320x240)
- **Duration**: 15 minutes
- **Mode**: Motion-only
- **JPEG Quality**: 25

### Critical Battery (<15%)
- **Frame Rate**: 1 FPS
- **Quality**: Low (QVGA 320x240)
- **Duration**: 5 minutes
- **Mode**: Motion-only
- **JPEG Quality**: 35

## API Reference

### Streaming Endpoints

#### GET /api/stream
**MJPEG Streaming Endpoint**
- **Description**: Main MJPEG stream for direct browser viewing
- **Content-Type**: `multipart/x-mixed-replace; boundary=wildlifecam_stream`
- **Usage**: Direct embedding in HTML `<img>` tag or video player

#### POST /api/stream/start
**Start Streaming Session**
- **Parameters** (query string):
  - `fps`: Frame rate (1-10)
  - `quality`: Stream quality (low/medium/high/auto)
  - `frameSize`: Frame size (qvga/vga/svga/hd)
  - `motionOnly`: Motion-only mode (true/false)
- **Response**: JSON with success status and stream information

#### POST /api/stream/stop
**Stop Streaming Session**
- **Response**: JSON with success status

#### GET /api/stream/stats
**Get Streaming Statistics**
- **Response**: JSON with current streaming status and statistics

#### POST /api/stream/config
**Update Stream Configuration**
- **Parameters** (query string): Same as start endpoint
- **Response**: JSON with success status

### WebSocket Messages

#### STREAM_STATUS
Real-time streaming status updates:
```json
{
  "type": "STREAM_STATUS",
  "data": {
    "streaming": true,
    "clients": 2,
    "state": {
      "motionTriggered": false,
      "fps": 5,
      "quality": "Medium",
      "frameSize": "VGA (640x480)",
      "motionOnly": true,
      "duration": 120,
      "maxDuration": 1800
    },
    "stats": {
      "totalFrames": 600,
      "totalBytes": 1048576,
      "averageFPS": 5.0,
      "droppedFrames": 2
    }
  }
}
```

## Integration Guide

### Basic Integration

```cpp
#include "src/streaming/stream_manager.h"
#include "src/camera/camera_manager.h"
#include "firmware/src/power_manager.h"
#include "src/detection/motion_detection_manager.h"

// Initialize components
CameraManager cameraManager;
PowerManager powerManager;
MotionDetectionManager motionManager;
StreamManager streamManager;

void setup() {
    // Initialize base components
    cameraManager.initialize();
    powerManager.init();
    motionManager.initialize(&cameraManager);
    
    // Initialize streaming with integrations
    streamManager.initialize(&cameraManager, &powerManager, &motionManager);
    
    // Set up event callback
    streamManager.setEventCallback([](StreamEvent event, const String& message) {
        Serial.printf("Stream event: %d - %s\n", event, message.c_str());
    });
}

void loop() {
    // Update all components
    powerManager.update();
    streamManager.update();
    
    // Handle motion detection
    auto motionResult = motionManager.detectMotion();
    if (motionResult.motionDetected) {
        streamManager.handleMotionEvent(true, motionResult.confidenceScore);
    }
    
    // Update power status
    streamManager.updatePowerStatus(powerManager.getBatteryPercentage(), 
                                   powerManager.isCharging());
}
```

### Web Server Integration

```cpp
#include "firmware/src/web/enhanced_web_server.h"

EnhancedWebServer webServer;
StreamManager streamManager;

void setup() {
    // Initialize components
    webServer.begin();
    streamManager.initialize(&cameraManager, &powerManager, &motionManager);
    
    // Link stream manager to web server
    webServer.setStreamManager(&streamManager);
    
    // Set up stream event callback for WebSocket broadcasts
    streamManager.setEventCallback([](StreamEvent event, const String& message) {
        webServer.broadcastStreamStatus();
    });
}
```

### Motion-Triggered Streaming

```cpp
// Configure motion-triggered streaming
StreamConfig config;
config.motionTriggerEnabled = true;
config.powerAwareMode = true;
config.targetFPS = 5;
config.quality = STREAM_QUALITY_AUTO;

streamManager.setStreamConfig(config);

// Motion detection integration
void handleMotionDetection() {
    auto result = motionManager.detectMotion();
    if (result.motionDetected && result.confidenceScore > 60.0) {
        // Trigger streaming on motion
        streamManager.handleMotionEvent(true, result.confidenceScore);
    }
}
```

## Web Interface Usage

### Streaming Dashboard

The web interface is available at `/streaming.html` and provides:

#### Live Stream Viewer
- Real-time MJPEG video display
- Automatic reconnection on connection loss
- Stream overlay with status information

#### Stream Controls
- Start/Stop streaming buttons
- Frame rate adjustment (1-10 FPS)
- Quality selection (Low/Medium/High/Auto)
- Frame size selection (QVGA/VGA/SVGA/HD)
- Motion-only mode toggle

#### Real-time Statistics
- Current viewer count
- Frame rate and quality display
- Total frames sent and data transferred
- Stream duration and dropped frames
- Battery level and power state
- Motion detection status

#### System Monitoring
- Battery level with visual indicator
- Power state display
- Memory usage
- Network connectivity status

### WebSocket Integration

The dashboard uses WebSocket connections for real-time updates:

```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://camera-ip/ws');

// Handle stream status updates
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    if (data.type === 'STREAM_STATUS') {
        updateStreamDisplay(data.data);
    }
};

// Start streaming programmatically
fetch('/api/stream/start?fps=5&quality=medium', {method: 'POST'})
    .then(response => response.json())
    .then(data => console.log('Stream started:', data));
```

## Performance Optimization

### Frame Rate Optimization
- Adaptive frame rate based on battery level
- Real-time FPS adjustment via API
- Dropped frame monitoring and reporting

### Bandwidth Management
- Configurable JPEG quality levels
- Frame size adaptation based on power state
- Client connection limiting (max 3 concurrent)

### Power Conservation
- Motion-only streaming for low battery
- Automatic stream timeout based on battery level
- Power-aware quality degradation

### Memory Management
- Efficient frame buffer reuse
- Client cleanup for disconnected viewers
- Automatic resource management

## Troubleshooting

### Common Issues

#### Stream Not Starting
1. Check camera initialization: `cameraManager.initialize()`
2. Verify power conditions: `powerManager.getPowerState()`
3. Check client count: `streamManager.getClientCount()`
4. Review error logs in WebSocket messages

#### Poor Stream Quality
1. Check battery level - may trigger quality reduction
2. Verify frame rate settings
3. Check network bandwidth
4. Review JPEG quality settings

#### High Power Consumption
1. Enable power-aware mode: `config.powerAwareMode = true`
2. Use motion-only mode for low battery
3. Reduce frame rate and quality
4. Check for excessive client connections

#### Motion Detection Not Triggering
1. Verify motion manager initialization
2. Check motion confidence threshold
3. Review PIR sensor connections
4. Validate motion detection settings

### Debug Commands

```cpp
// Get current stream status
String status = streamManager.getStatusJSON();
Serial.println("Status: " + status);

// Get stream statistics
String stats = streamManager.getStatsJSON();
Serial.println("Stats: " + stats);

// Check power conditions
bool powerOK = streamManager.shouldStreamBeActive();
Serial.printf("Power adequate: %s\n", powerOK ? "Yes" : "No");

// Reset statistics
streamManager.resetStats();
```

## Security Considerations

### Network Security
- Use HTTPS/WSS in production environments
- Implement authentication for sensitive deployments
- Consider VPN access for remote monitoring

### Access Control
- Enable web server authentication if needed
- Limit client connections to prevent overload
- Monitor and log access attempts

### Power Security
- Implement battery protection thresholds
- Monitor for excessive power drain
- Set conservative streaming limits for unattended operation

## Future Enhancements

### Potential Additions
1. **Audio Streaming**: Integration with existing audio monitoring
2. **Recording Integration**: Simultaneous streaming and recording
3. **Multi-Camera Support**: Support for multiple camera feeds
4. **Cloud Streaming**: Integration with cloud platforms
5. **Advanced Analytics**: Stream-based wildlife behavior analysis

### Mobile App Integration
The streaming system is designed to integrate with the planned mobile app:
- RESTful API compatibility
- WebSocket support for real-time updates
- Standardized streaming protocols
- Power-aware mobile streaming profiles

## Conclusion

The live streaming implementation provides a comprehensive solution for real-time wildlife monitoring while maintaining the system's focus on power efficiency and reliability. The modular design ensures easy integration with existing components and provides a foundation for future enhancements.

For additional support or custom modifications, refer to the source code documentation and the ESP32WildlifeCAM project documentation.