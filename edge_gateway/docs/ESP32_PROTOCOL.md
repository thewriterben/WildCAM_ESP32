# ESP32 Protocol Integration Guide

Communication protocol between Jetson Edge Gateway and ESP32 field nodes.

## Overview

The gateway supports three communication methods:
1. **HTTP REST API** - For data upload and configuration
2. **WebSocket** - For real-time bidirectional communication
3. **LoRa** - For long-range wireless communication in remote areas

## HTTP REST API

### Base URL
```
http://<gateway-ip>:8080/api
```

### Endpoints

#### Register Node
```http
POST /api/node/register

{
  "node_id": "esp32_001",
  "name": "Trail Camera 1",
  "firmware_version": "1.0.0",
  "battery_level": 85,
  "metadata": {
    "location": "North Trail",
    "hardware": "ESP32-CAM"
  }
}

Response:
{
  "status": "registered",
  "node_id": "esp32_001",
  "server_time": "2024-01-15T10:30:00Z"
}
```

#### Upload Data
```http
POST /api/node/{node_id}/data

{
  "timestamp": 1705315800,
  "battery_voltage": 3.7,
  "temperature": 22.5,
  "detections": [
    {
      "class": "deer",
      "confidence": 0.92,
      "timestamp": 1705315800
    }
  ]
}

Response:
{
  "status": "received"
}
```

#### Upload Image
```http
POST /api/node/{node_id}/image
Content-Type: multipart/form-data

Form fields:
- image: <binary image data>
- metadata: {
    "timestamp": 1705315800,
    "format": "jpeg",
    "resolution": [1600, 1200]
  }

Response:
{
  "status": "received"
}
```

#### Get Configuration
```http
GET /api/node/{node_id}/config

Response:
{
  "capture_interval": 300,
  "detection_enabled": true,
  "upload_enabled": true,
  "quality": 10
}
```

#### Update Configuration
```http
POST /api/node/{node_id}/config

{
  "capture_interval": 600,
  "detection_enabled": true
}

Response:
{
  "status": "updated"
}
```

## WebSocket Communication

### Connection
```javascript
// Connect to WebSocket
ws = new WebSocket('ws://<gateway-ip>:8081');

// Register node
ws.send(JSON.stringify({
  event: 'register',
  data: {
    node_id: 'esp32_001',
    name: 'Trail Camera 1'
  }
}));
```

### Events

#### Client → Server

**Register**
```json
{
  "event": "register",
  "data": {
    "node_id": "esp32_001",
    "name": "Trail Camera 1"
  }
}
```

**Send Data**
```json
{
  "event": "data",
  "data": {
    "timestamp": 1705315800,
    "battery_voltage": 3.7,
    "detections": [...]
  }
}
```

**Heartbeat**
```json
{
  "event": "heartbeat",
  "data": {}
}
```

#### Server → Client

**Registered**
```json
{
  "event": "registered",
  "data": {
    "node_id": "esp32_001"
  }
}
```

**Command**
```json
{
  "event": "command",
  "data": {
    "command": "capture",
    "params": {
      "quality": 10
    }
  }
}
```

**Heartbeat Acknowledgment**
```json
{
  "event": "heartbeat_ack",
  "data": {}
}
```

## LoRa Communication

### Packet Format

```
[Node ID (16 bytes)][Message Type (2 bytes)][Payload Length (2 bytes)][Payload (N bytes)][Checksum (2 bytes)]
```

**Field Descriptions:**
- **Node ID**: Null-padded string identifier (e.g., "esp32_001")
- **Message Type**: 1=data, 2=config, 3=command, 4=ack
- **Payload Length**: Size of payload in bytes
- **Payload**: JSON-encoded message data
- **Checksum**: Simple sum of payload bytes (16-bit)

### Message Types

#### Data Message (Type 1)
```json
{
  "type": "data",
  "timestamp": 1705315800,
  "battery_voltage": 3.7,
  "temperature": 22.5,
  "detections": [
    {"class": "deer", "confidence": 0.92}
  ]
}
```

#### Configuration Request (Type 2)
```json
{
  "type": "config_request"
}
```

#### Command (Type 3)
```json
{
  "type": "command",
  "command": "capture",
  "params": {
    "quality": 10
  }
}
```

#### Acknowledgment (Type 4)
```json
{
  "type": "ack",
  "message_id": "msg_12345"
}
```

## ESP32 Implementation Examples

### Arduino/PlatformIO (HTTP)

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

const char* gateway_url = "http://192.168.1.50:8080";
const char* node_id = "esp32_001";

void registerNode() {
  HTTPClient http;
  http.begin(String(gateway_url) + "/api/node/register");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"node_id\":\"" + String(node_id) + "\","
                   "\"name\":\"Trail Camera 1\","
                   "\"firmware_version\":\"1.0.0\","
                   "\"battery_level\":" + String(getBatteryLevel()) + "}";
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    Serial.println("Node registered successfully");
  }
  
  http.end();
}

void uploadData() {
  HTTPClient http;
  http.begin(String(gateway_url) + "/api/node/" + node_id + "/data");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"timestamp\":" + String(millis()/1000) + ","
                   "\"battery_voltage\":" + String(getBatteryVoltage()) + ","
                   "\"temperature\":" + String(getTemperature()) + "}";
  
  int httpCode = http.POST(payload);
  http.end();
}
```

### Arduino/PlatformIO (WebSocket)

```cpp
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      // Register node
      webSocket.sendTXT("{\"event\":\"register\",\"data\":{\"node_id\":\"esp32_001\"}}");
      break;
      
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      // Handle commands from gateway
      break;
  }
}

void setup() {
  webSocket.begin("192.168.1.50", 8081, "/");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  
  // Send heartbeat every 30 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 30000) {
    webSocket.sendTXT("{\"event\":\"heartbeat\",\"data\":{}}");
    lastHeartbeat = millis();
  }
}
```

### LoRa Communication

```cpp
#include <LoRa.h>

void sendLoRaData() {
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["type"] = "data";
  doc["timestamp"] = millis() / 1000;
  doc["battery_voltage"] = getBatteryVoltage();
  
  String payload;
  serializeJson(doc, payload);
  
  // Create packet
  uint8_t packet[128];
  
  // Node ID (16 bytes)
  memcpy(packet, "esp32_001", 9);
  memset(packet + 9, 0, 7);
  
  // Message type (2 bytes) - 1 for data
  packet[16] = 0x01;
  packet[17] = 0x00;
  
  // Payload length (2 bytes)
  uint16_t payload_len = payload.length();
  packet[18] = payload_len & 0xFF;
  packet[19] = (payload_len >> 8) & 0xFF;
  
  // Payload
  memcpy(packet + 20, payload.c_str(), payload_len);
  
  // Checksum
  uint16_t checksum = 0;
  for (int i = 0; i < payload_len; i++) {
    checksum += payload[i];
  }
  packet[20 + payload_len] = checksum & 0xFF;
  packet[21 + payload_len] = (checksum >> 8) & 0xFF;
  
  // Send via LoRa
  LoRa.beginPacket();
  LoRa.write(packet, 22 + payload_len);
  LoRa.endPacket();
}
```

## Communication Patterns

### Periodic Data Upload (HTTP)

```
ESP32                          Gateway
  |                               |
  |-- POST /api/node/register --> |
  |<-- 200 OK -------------------- |
  |                               |
  (every 5 minutes)
  |-- POST /api/node/{id}/data -> |
  |<-- 200 OK -------------------- |
```

### Real-time Monitoring (WebSocket)

```
ESP32                          Gateway
  |                               |
  |-- WS Connect ---------------> |
  |<-- Connected ----------------- |
  |-- register -----------------> |
  |<-- registered ---------------- |
  |                               |
  |<-- command -------------------- |
  |-- data ----------------------> |
  |                               |
  (heartbeat every 30s)
  |-- heartbeat ----------------> |
  |<-- heartbeat_ack ------------- |
```

### LoRa Low-Power Mode

```
ESP32                          Gateway
  |                               |
  (wake from sleep)
  |-- LoRa data packet ---------> |
  |<-- LoRa ack ------------------- |
  (go to sleep)
```

## Best Practices

1. **Use HTTP for**: Periodic uploads, configuration changes
2. **Use WebSocket for**: Real-time monitoring, commands, low-latency
3. **Use LoRa for**: Remote deployments, power-constrained setups

4. **Implement retry logic** for failed uploads
5. **Cache data locally** if gateway is unavailable
6. **Use heartbeats** to detect connection loss
7. **Compress large payloads** before transmission
8. **Validate checksums** on LoRa packets

## Security Considerations

- Use HTTPS/WSS in production (requires SSL certificates)
- Implement API key authentication
- Encrypt sensitive data in transit
- Validate all incoming data
- Rate-limit API requests

## Troubleshooting

### Connection Issues
```bash
# Test HTTP connectivity from ESP32
curl http://gateway-ip:8080/api/health

# Test WebSocket
wscat -c ws://gateway-ip:8081
```

### LoRa Issues
```bash
# Check serial port on gateway
ls -l /dev/ttyUSB*

# Monitor LoRa packets
python3 -m serial.tools.miniterm /dev/ttyUSB0 115200
```

## See Also

- [Gateway API Documentation](API.md)
- [Deployment Guide](DEPLOYMENT.md)
- [ESP32 Example Firmware](../../firmware/esp32_gateway_client/)
