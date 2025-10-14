# Reliable Transmission Protocol Integration Example

This example demonstrates how to integrate the Reliable Transmission Protocol (RTP) with the existing image transmission and telemetry systems for reliable wildlife monitoring data delivery.

## Overview

The example shows:
- Integration of RTP with `ImageMesh` and `WildlifeTelemetry`
- Reliable transmission of images and telemetry data
- Priority-based scheduling for different data types
- Error handling and retry logic
- Statistics tracking and monitoring
- Interactive command interface for testing

## Hardware Requirements

- ESP32 board (any variant supported by the project)
- LoRa radio module (SX1276/SX1278 or compatible)
- Optional: SD card for image storage
- Optional: Camera module (OV2640 or compatible)

## Software Requirements

- Arduino IDE or PlatformIO
- ESP32 board support
- Required libraries:
  - LoRa (for radio communication)
  - ArduinoJson
  - ESP32 camera (if using camera)

## Configuration

### Node Configuration

Update the node ID and name in the sketch:

```cpp
const uint32_t NODE_ID = 0x12345678;  // Change to your node ID
const char* NODE_NAME = "WildCAM-01";  // Change to your node name
```

### Network Configuration

Choose appropriate configuration profile:

```cpp
// For remote, low-bandwidth deployments
ReliableTransmissionConfig config = createLowBandwidthConfig();

// For high-reliability critical monitoring
ReliableTransmissionConfig config = createHighReliabilityConfig();

// For default balanced operation
ReliableTransmissionConfig config = createDefaultRTPConfig();
```

## Usage

### Upload and Run

1. Open the sketch in Arduino IDE or PlatformIO
2. Select your ESP32 board
3. Upload the sketch
4. Open Serial Monitor at 115200 baud

### Interactive Commands

Once running, use the following commands:

- `m` - Simulate motion detection event
- `w` - Simulate wildlife detection (normal species)
- `e` - Simulate endangered species detection (critical priority)
- `i` - Transmit test image
- `s` - Print transmission statistics
- `r` - Reset statistics
- `h` or `?` - Print help

### Example Session

```
=== Reliable Transmission Protocol Integration Example ===

Initializing mesh interface...
✓ Mesh interface initialized
Initializing reliable transmission protocol...
✓ RTP initialized
Configuring for low-bandwidth deployment...
✓ RTP configured
Initializing telemetry system...
✓ Telemetry system initialized

=== System Ready ===

Commands:
  m - Simulate motion detection
  w - Simulate wildlife detection
  e - Simulate endangered species detection
  i - Transmit test image
  s - Print statistics
  r - Reset statistics
  h - Print help

>>> Simulating wildlife detection

=== Wildlife Detection ===
Species: Deer
Confidence: 92.0%
Count: 1

--- Transmitting Image: wildlife_123456.jpg ---
Image transmission queued: ID=00000001, Size=5000 bytes
→ Transmission 00000001: 20.0% complete
→ Transmission 00000001: 40.0% complete
→ Transmission 00000001: 60.0% complete
→ Transmission 00000001: 80.0% complete
→ Transmission 00000001: 100.0% complete
✓ Transmission 00000001 completed successfully

=== Transmission Statistics ===
Total: 1
Successful: 1
Failed: 0
Success Rate: 100.0%
Retries: 0

=== Bandwidth Statistics ===
Packets TX: 25
Packets RX: 0
Packets Lost: 0
Loss Rate: 0.00%
Avg Latency: 152.3 ms
Throughput: 345.6 bytes/s
```

## Integration Details

### Image Transmission

Images are transmitted with reliable delivery:

```cpp
uint32_t transmitImageReliably(const String& filename, uint32_t destination = 0) {
    // Read image from SD card
    std::vector<uint8_t> imageData = readImageFile(filename);
    
    // Transmit with high priority and acknowledgment
    uint32_t txId = rtp->transmitData(
        destination,
        PACKET_TYPE_IMAGE,
        imageData,
        PRIORITY_HIGH,
        true  // Require acknowledgment
    );
    
    return txId;
}
```

### Telemetry Transmission

Telemetry data is transmitted with configurable priority:

```cpp
uint32_t transmitTelemetryReliably(TelemetryType type, 
                                   const std::vector<uint8_t>& data,
                                   TransmissionPriority priority = PRIORITY_NORMAL) {
    return rtp->transmitData(
        0,  // Broadcast
        PACKET_TYPE_TELEMETRY,
        data,
        priority,
        true
    );
}
```

### Priority Handling

Different data types use appropriate priorities:

- **PRIORITY_CRITICAL**: Endangered species alerts
- **PRIORITY_HIGH**: Wildlife detections, capture events
- **PRIORITY_NORMAL**: Motion events, standard telemetry
- **PRIORITY_LOW**: Routine status updates
- **PRIORITY_BACKGROUND**: Periodic health checks

## Monitoring and Debugging

### Transmission Status

Monitor individual transmissions:

```cpp
TransmissionStatus status = rtp->getTransmissionStatus(txId);
Serial.printf("State: %s\n", stateToString(status.state));
Serial.printf("Progress: %.1f%%\n", status.progressPercentage);
Serial.printf("Packets: %d/%d\n", status.packetsAcknowledged, status.totalPackets);
```

### Network Statistics

Track network performance:

```cpp
BandwidthStats stats = rtp->getStatistics();
Serial.printf("Loss Rate: %.2f%%\n", stats.packetLossRate * 100);
Serial.printf("Avg Latency: %.1f ms\n", stats.averageLatency);
Serial.printf("Throughput: %.1f bytes/s\n", stats.throughput);
```

### Callbacks

Use callbacks for real-time notifications:

```cpp
rtp->setTransmissionCompleteCallback([](uint32_t txId, bool success) {
    if (success) {
        Serial.println("Transmission successful!");
    } else {
        Serial.println("Transmission failed!");
    }
});

rtp->setTransmissionErrorCallback([](uint32_t txId, TransmissionResult error) {
    Serial.printf("Error: %s\n", resultToString(error));
});
```

## Performance Tuning

### For Long-Range LoRa

Use low-bandwidth configuration:

```cpp
ReliableTransmissionConfig config = createLowBandwidthConfig();
config.maxRetries = 7;
config.initialRetryDelay = 2000;
config.maxBytesPerSecond = 512;
rtp->configure(config);
```

### For High-Throughput WiFi

Use best-effort configuration:

```cpp
ReliableTransmissionConfig config = createBestEffortConfig();
config.maxBytesPerSecond = 10240;  // 10 KB/s
rtp->configure(config);
```

### For Critical Monitoring

Use high-reliability configuration:

```cpp
ReliableTransmissionConfig config = createHighReliabilityConfig();
config.maxRetries = 10;
config.ackTimeout = 10000;
config.adaptiveRetry = true;
rtp->configure(config);
```

## Troubleshooting

### High Packet Loss

If you see high packet loss rates:

1. Check LoRa signal strength
2. Verify antenna connections
3. Increase retry delay
4. Reduce bandwidth limit

### Low Throughput

If throughput is lower than expected:

1. Check network congestion
2. Verify bandwidth configuration
3. Monitor CPU usage
4. Check for interference

### Transmission Failures

If transmissions consistently fail:

1. Verify destination node is reachable
2. Check network topology
3. Increase max retries
4. Review error messages

## Advanced Features

### Custom Priority Scheduling

Implement custom priority logic:

```cpp
TransmissionPriority calculatePriority(const Detection& detection) {
    if (detection.endangered) return PRIORITY_CRITICAL;
    if (detection.confidence > 0.9) return PRIORITY_HIGH;
    if (detection.confidence > 0.7) return PRIORITY_NORMAL;
    return PRIORITY_LOW;
}
```

### Adaptive Bandwidth Management

Adjust bandwidth based on conditions:

```cpp
void adjustBandwidth() {
    float lossRate = rtp->getPacketLossRate();
    
    if (lossRate > 0.1) {
        // High loss - reduce bandwidth
        rtp->setBandwidthLimit(512);
    } else if (lossRate < 0.02) {
        // Low loss - can increase
        rtp->setBandwidthLimit(2048);
    }
}
```

### Queue Management

Handle queue overflow:

```cpp
uint32_t txId = rtp->transmitData(...);
if (txId == 0) {
    // Queue full - implement backpressure
    Serial.println("Queue full - delaying transmission");
    delay(1000);
    // Retry or store for later
}
```

## See Also

- [Reliable Transmission Protocol Documentation](../../docs/RELIABLE_TRANSMISSION_PROTOCOL.md)
- [Image Mesh Documentation](../../firmware/src/meshtastic/image_mesh.h)
- [Wildlife Telemetry Documentation](../../firmware/src/meshtastic/wildlife_telemetry.h)
- [Mesh Interface Documentation](../../firmware/src/meshtastic/mesh_interface.h)

## License

Part of the ESP32WildlifeCAM project.
