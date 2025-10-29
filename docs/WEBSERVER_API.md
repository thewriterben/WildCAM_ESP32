# WebServer API Documentation

## Overview
The WebServer class provides a RESTful API for managing the WildCAM ESP32 wildlife camera system over HTTP.

## Initialization

### Constructor
```cpp
WebServer(int serverPort = 80)
```
Creates a WebServer instance on the specified port (default: 80).

### init()
```cpp
bool init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef)
```
Initializes the web server with references to other system managers.

**Parameters:**
- `storageRef` - Pointer to StorageManager instance
- `cameraRef` - Pointer to CameraManager instance  
- `powerRef` - Pointer to PowerManager instance

**Returns:** `true` if all references are valid, `false` if any are null

**Example:**
```cpp
WebServer server(80);
if (server.init(&storage, &camera, &power)) {
    server.begin();
}
```

### begin()
```cpp
void begin()
```
Starts the web server and sets up all routes. Also prints the server URL to Serial.

## API Routes

### GET /
Serves a simple HTML homepage with links to status and latest image.

**Response:** HTML page

---

### GET /status
Returns system status information in JSON format.

**Response (200 OK):**
```json
{
  "uptime": 123456,
  "freeHeap": 234567,
  "batteryVoltage": 3.85,
  "batteryPercentage": 75,
  "sdCardFreeSpace": 1048576000,
  "sdCardUsedSpace": 524288000,
  "imageCount": 42
}
```

**Fields:**
- `uptime` - System uptime in milliseconds
- `freeHeap` - Available heap memory in bytes
- `batteryVoltage` - Battery voltage in volts
- `batteryPercentage` - Battery charge percentage (0-100)
- `sdCardFreeSpace` - Free space on SD card in bytes
- `sdCardUsedSpace` - Used space on SD card in bytes
- `imageCount` - Total number of images captured

---

### GET /latest
Retrieves the most recently captured image.

**Response (200 OK):** JPEG image file with `Content-Type: image/jpeg`

**Error Responses:**
- `503 Service Unavailable` - Storage not available
- `404 Not Found` - No images found on SD card

---

### POST /capture
Triggers an immediate image capture and saves it to SD card.

**Response (200 OK):**
```json
{
  "success": true,
  "path": "/images/20240315/IMG_143022_001.jpg",
  "size": 45678
}
```

**Fields:**
- `success` - Always true on success
- `path` - Full path where image was saved
- `size` - Image file size in bytes

**Error Responses:**
- `503 Service Unavailable` - Camera not initialized
- `500 Internal Server Error` - Capture failed or save failed

---

### POST /reboot
Reboots the ESP32 device after 1 second delay.

**Response (200 OK):**
```json
{
  "message": "Rebooting..."
}
```

**Note:** Connection will be lost as device reboots.

---

## CORS Support

All routes include CORS headers:
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

This allows the API to be accessed from web applications on different domains.

## Error Handling

All error responses use JSON format:
```json
{
  "error": "Error message description"
}
```

Common HTTP status codes:
- `200 OK` - Request successful
- `404 Not Found` - Resource not found
- `500 Internal Server Error` - Server-side error
- `503 Service Unavailable` - Required service not available

## Usage Example

```cpp
#include <WiFi.h>
#include "WebServer.h"
#include "StorageManager.h"
#include "CameraManager.h"
#include "PowerManager.h"

// Create instances
WebServer server(80);
StorageManager storage;
CameraManager camera;
PowerManager power;

void setup() {
    // Initialize WiFi
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    // Initialize managers
    storage.init();
    camera.init();
    power.init(33);
    
    // Initialize and start web server
    if (server.init(&storage, &camera, &power)) {
        server.begin();
        Serial.print("Server running at http://");
        Serial.println(WiFi.localIP());
    }
}

void loop() {
    // Web server runs asynchronously
    delay(10);
}
```

## Testing the API

Using curl:
```bash
# Get system status
curl http://192.168.1.100/status

# Get latest image
curl http://192.168.1.100/latest -o latest.jpg

# Trigger image capture
curl -X POST http://192.168.1.100/capture

# Reboot device
curl -X POST http://192.168.1.100/reboot
```

Using a web browser:
- Navigate to `http://<device-ip>/` for the homepage
- Navigate to `http://<device-ip>/status` to view system status
- Navigate to `http://<device-ip>/latest` to view the latest image
