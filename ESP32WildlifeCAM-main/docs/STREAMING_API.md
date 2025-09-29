# Streaming API Reference

## Overview

This document provides a complete API reference for the ESP32WildlifeCAM live streaming functionality. The streaming system provides RESTful HTTP endpoints and WebSocket integration for real-time communication.

## Base URL

All API endpoints are relative to the device's IP address:
```
http://<device-ip>/api/
```

For WebSocket connections:
```
ws://<device-ip>/ws
```

## Authentication

By default, the streaming API does not require authentication. For production deployments, enable authentication in the web server configuration:

```cpp
EnhancedWebConfig config;
config.enableAuth = true;
config.username = "admin";
config.password = "your-secure-password";
webServer.begin(config);
```

## HTTP API Endpoints

### Stream Control

#### Start Streaming
**`POST /api/stream/start`**

Starts a new streaming session with optional configuration parameters.

**Parameters (Query String):**
| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `fps` | integer | 5 | Frame rate (1-10 FPS) |
| `quality` | string | "medium" | Quality level: "low", "medium", "high", "auto" |
| `frameSize` | string | "vga" | Frame size: "qvga", "vga", "svga", "hd" |
| `motionOnly` | boolean | false | Enable motion-only streaming mode |

**Example Request:**
```bash
curl -X POST "http://192.168.1.100/api/stream/start?fps=7&quality=high&frameSize=vga&motionOnly=true"
```

**Response:**
```json
{
  "success": true,
  "message": "Stream started successfully",
  "status": {
    "streaming": true,
    "clients": 0,
    "fps": 7,
    "quality": "High",
    "frameSize": "VGA (640x480)",
    "motionTriggered": false,
    "motionOnlyMode": true,
    "powerAware": true
  }
}
```

**Error Response:**
```json
{
  "success": false,
  "error": "Insufficient power for streaming"
}
```

#### Stop Streaming
**`POST /api/stream/stop`**

Stops the current streaming session.

**Example Request:**
```bash
curl -X POST "http://192.168.1.100/api/stream/stop"
```

**Response:**
```json
{
  "success": true,
  "message": "Stream stopped successfully"
}
```

#### Update Stream Configuration
**`POST /api/stream/config`**

Updates streaming configuration parameters while streaming is active.

**Parameters:** Same as start streaming endpoint

**Example Request:**
```bash
curl -X POST "http://192.168.1.100/api/stream/config?fps=3&quality=low"
```

**Response:**
```json
{
  "success": true,
  "message": "Stream configuration updated"
}
```

### Stream Information

#### Get Stream Statistics
**`GET /api/stream/stats`**

Retrieves current streaming statistics and status information.

**Example Request:**
```bash
curl "http://192.168.1.100/api/stream/stats"
```

**Response:**
```json
{
  "status": {
    "streaming": true,
    "clients": 2,
    "fps": 5,
    "quality": "Medium",
    "frameSize": "VGA (640x480)",
    "motionTriggered": false,
    "motionOnlyMode": true,
    "powerAware": true,
    "duration": 180,
    "maxDuration": 1800
  },
  "stats": {
    "totalFrames": 900,
    "totalBytes": 4194304,
    "currentClients": 2,
    "peakClients": 3,
    "streamingTime": 180,
    "averageFrameSize": 4660,
    "averageFPS": 5.0,
    "droppedFrames": 5
  }
}
```

#### MJPEG Stream
**`GET /api/stream`**

Direct MJPEG stream endpoint for embedding in web pages or applications.

**Content-Type:** `multipart/x-mixed-replace; boundary=wildlifecam_stream`

**Example Usage:**
```html
<img src="http://192.168.1.100/api/stream" alt="Wildlife Camera Feed" />
```

**cURL Example:**
```bash
curl "http://192.168.1.100/api/stream" --output stream.mjpg
```

### System Information

#### Get System Status
**`GET /api/status`**

Retrieves overall system status including power, memory, and network information.

**Example Request:**
```bash
curl "http://192.168.1.100/api/status"
```

**Response:**
```json
{
  "battery": {
    "voltage": 3.8,
    "percentage": 75,
    "charging": true
  },
  "solar": {
    "voltage": 5.2
  },
  "power": {
    "state": 3
  },
  "memory": {
    "free": 180000,
    "total": 320000
  },
  "system": {
    "uptime": 86400,
    "temperature": 28.5
  },
  "storage": {
    "total": 32000000000,
    "used": 8000000000,
    "free": 24000000000,
    "images": 1250
  },
  "network": {
    "connected": true,
    "signal": -45,
    "ip": "192.168.1.100"
  },
  "camera": {
    "ready": true,
    "monitoring": true
  },
  "captures": {
    "daily": 24,
    "total": 1250
  },
  "wildlife": {
    "lastSpecies": "Red Fox",
    "lastConfidence": 85.3
  }
}
```

## WebSocket API

### Connection

Connect to the WebSocket endpoint to receive real-time updates:

```javascript
const ws = new WebSocket('ws://192.168.1.100/ws');

ws.onopen = function() {
    console.log('Connected to wildlife camera');
};

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    handleMessage(data);
};
```

### Message Types

#### STREAM_STATUS (Type 7)
Real-time streaming status updates sent whenever stream state changes.

**Message Structure:**
```json
{
  "type": "STREAM_STATUS",
  "data": {
    "streaming": true,
    "clients": 1,
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
      "totalBytes": 2097152,
      "averageFPS": 5.0,
      "droppedFrames": 1
    }
  }
}
```

#### SYSTEM_STATUS (Type 0)
Periodic system status updates including power, memory, and camera information.

#### WILDLIFE_DETECTION (Type 1)
Sent when wildlife is detected by the AI system.

**Message Structure:**
```json
{
  "type": "WILDLIFE_DETECTION",
  "data": {
    "species": "Red Fox",
    "confidence": 87.5,
    "imagePath": "/images/20231201_143022.jpg",
    "timestamp": 1701434622000
  }
}
```

#### POWER_UPDATE (Type 2)
Battery and power system status updates.

#### ERROR_ALERT (Type 5)
System error notifications and alerts.

**Message Structure:**
```json
{
  "type": "ERROR_ALERT",
  "data": {
    "message": "Low battery warning",
    "severity": "warning"
  }
}
```

### Client-to-Server Messages

Send commands to the server via WebSocket:

```javascript
// Request system status update
ws.send(JSON.stringify({
    action: "get_status"
}));

// Request stream statistics
ws.send(JSON.stringify({
    action: "get_stream_stats"
}));
```

## Error Codes and Status

### HTTP Status Codes

| Code | Meaning | Description |
|------|---------|-------------|
| 200 | OK | Request successful |
| 400 | Bad Request | Invalid parameters or configuration |
| 404 | Not Found | Endpoint not found |
| 500 | Internal Server Error | Server error occurred |
| 503 | Service Unavailable | Stream manager not available |

### Stream Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | STREAM_OK | No error |
| 1 | STREAM_ERROR_NO_CLIENTS | No clients connected |
| 2 | STREAM_ERROR_CAMERA_BUSY | Camera in use by another process |
| 3 | STREAM_ERROR_LOW_MEMORY | Insufficient memory for streaming |
| 4 | STREAM_ERROR_POWER_SAVE | Power too low for streaming |
| 5 | STREAM_ERROR_MOTION_REQUIRED | Motion required but not detected |
| 6 | STREAM_ERROR_MAX_DURATION | Maximum stream duration exceeded |
| 7 | STREAM_ERROR_NETWORK | Network error occurred |

## Rate Limiting

### API Rate Limits
- Stream control endpoints: 10 requests per minute
- Status endpoints: 60 requests per minute
- MJPEG stream: No limit (handled by client connections)

### WebSocket Limits
- Maximum 5 concurrent WebSocket connections
- Message rate: 100 messages per minute per connection

## Data Types

### StreamQuality Enum
```
0: STREAM_QUALITY_LOW
1: STREAM_QUALITY_MEDIUM
2: STREAM_QUALITY_HIGH
3: STREAM_QUALITY_AUTO
```

### StreamFrameSize Enum
```
0: STREAM_FRAMESIZE_QVGA (320x240)
1: STREAM_FRAMESIZE_VGA (640x480)
2: STREAM_FRAMESIZE_SVGA (800x600)
3: STREAM_FRAMESIZE_HD (1280x720)
```

### PowerState Enum
```
0: POWER_CRITICAL
1: POWER_LOW
2: POWER_GOOD
3: POWER_NORMAL
```

## Integration Examples

### Python Integration

```python
import requests
import json
import websocket

class WildlifeCameraAPI:
    def __init__(self, host):
        self.base_url = f"http://{host}/api"
        self.ws_url = f"ws://{host}/ws"
    
    def start_stream(self, fps=5, quality="medium", frame_size="vga", motion_only=False):
        params = {
            "fps": fps,
            "quality": quality,
            "frameSize": frame_size,
            "motionOnly": str(motion_only).lower()
        }
        response = requests.post(f"{self.base_url}/stream/start", params=params)
        return response.json()
    
    def stop_stream(self):
        response = requests.post(f"{self.base_url}/stream/stop")
        return response.json()
    
    def get_stream_stats(self):
        response = requests.get(f"{self.base_url}/stream/stats")
        return response.json()
    
    def get_system_status(self):
        response = requests.get(f"{self.base_url}/status")
        return response.json()

# Usage example
camera = WildlifeCameraAPI("192.168.1.100")
result = camera.start_stream(fps=7, quality="high")
print(f"Stream started: {result['success']}")
```

### JavaScript Integration

```javascript
class WildlifeCameraClient {
    constructor(host) {
        this.baseUrl = `http://${host}/api`;
        this.wsUrl = `ws://${host}/ws`;
        this.ws = null;
    }
    
    async startStream(config = {}) {
        const params = new URLSearchParams({
            fps: config.fps || 5,
            quality: config.quality || 'medium',
            frameSize: config.frameSize || 'vga',
            motionOnly: config.motionOnly || false
        });
        
        const response = await fetch(`${this.baseUrl}/stream/start?${params}`, {
            method: 'POST'
        });
        return await response.json();
    }
    
    async stopStream() {
        const response = await fetch(`${this.baseUrl}/stream/stop`, {
            method: 'POST'
        });
        return await response.json();
    }
    
    async getStreamStats() {
        const response = await fetch(`${this.baseUrl}/stream/stats`);
        return await response.json();
    }
    
    connectWebSocket(onMessage) {
        this.ws = new WebSocket(this.wsUrl);
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            onMessage(data);
        };
        return this.ws;
    }
}

// Usage example
const camera = new WildlifeCameraClient('192.168.1.100');

camera.startStream({ fps: 7, quality: 'high' })
    .then(result => console.log('Stream started:', result.success));

camera.connectWebSocket((data) => {
    if (data.type === 'STREAM_STATUS') {
        console.log('Stream update:', data.data);
    }
});
```

### cURL Examples

```bash
# Start high-quality stream
curl -X POST "http://192.168.1.100/api/stream/start?fps=10&quality=high&frameSize=hd"

# Get current stream statistics
curl "http://192.168.1.100/api/stream/stats" | jq '.'

# Stop streaming
curl -X POST "http://192.168.1.100/api/stream/stop"

# Get system status
curl "http://192.168.1.100/api/status" | jq '.battery'

# Save MJPEG stream to file
curl "http://192.168.1.100/api/stream" --output wildlife_stream.mjpg
```

## Best Practices

### Client Implementation
1. **Connection Management**: Implement automatic reconnection for WebSocket connections
2. **Error Handling**: Always check response status and handle errors gracefully
3. **Rate Limiting**: Respect API rate limits to avoid service interruption
4. **Resource Cleanup**: Properly close connections when done

### Stream Optimization
1. **Power Awareness**: Monitor battery level and adjust quality accordingly
2. **Motion Integration**: Use motion-only mode for power conservation
3. **Client Limits**: Limit concurrent connections to avoid overloading the device
4. **Quality Selection**: Use "auto" quality for best power/quality balance

### Security Considerations
1. **Network Security**: Use HTTPS/WSS in production environments
2. **Authentication**: Enable authentication for sensitive deployments
3. **Access Control**: Limit API access to authorized clients only
4. **Monitoring**: Monitor API usage for unusual patterns

## Troubleshooting

### Common API Issues

#### Stream Won't Start (Error 503)
```bash
curl "http://192.168.1.100/api/status" | jq '.power.state'
```
Check power state - streaming may be disabled due to low battery.

#### Poor Stream Quality
```bash
curl "http://192.168.1.100/api/stream/stats" | jq '.stats.droppedFrames'
```
Monitor dropped frames to identify network or performance issues.

#### WebSocket Connection Fails
Verify WebSocket endpoint and check for proxy/firewall issues:
```bash
curl -I "http://192.168.1.100/ws"
```

For additional troubleshooting, refer to the main [Streaming Implementation Guide](STREAMING_IMPLEMENTATION.md).