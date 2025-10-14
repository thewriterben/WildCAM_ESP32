# LoRa Mesh Networking Demo

This example demonstrates the complete LoRa mesh networking functionality for wildlife monitoring applications.

## Features Demonstrated

1. **LoRa Mesh Initialization** - Set up mesh networking with proper configuration
2. **Device Discovery** - Automatic discovery of nearby mesh nodes
3. **Signal Strength Monitoring** - Real-time RSSI and SNR tracking
4. **Coordinator Election** - Automatic coordinator selection and failover
5. **Wildlife Alert Transmission** - Send detection events across the mesh
6. **Network Health Reporting** - Track connected nodes and packet statistics

## Hardware Requirements

### Supported Boards

- **ESP32-S3-CAM** (Recommended) - Full camera and LoRa support
- **ESP-EYE** - Camera and LoRa compatible
- **ESP32-XIAO** - Excellent for relay nodes (no camera)

### LoRa Module

- **LoRa Radio:** SX1276/SX1278 or compatible
- **Frequency:** 915 MHz (US) or 868 MHz (EU)
- **Antenna:** Quarter-wave or better

### Connections

For ESP32-S3-CAM:
```
LoRa Module   ESP32-S3
-----------   --------
SCK      ->   GPIO 12
MISO     ->   GPIO 13
MOSI     ->   GPIO 11
CS       ->   GPIO 10
RST      ->   GPIO 9
DIO0     ->   GPIO 14
3.3V     ->   3.3V
GND      ->   GND
```

## Software Requirements

- **PlatformIO** or **Arduino IDE 2.x**
- **Libraries:**
  - LoRa by Sandeep Mistry
  - ArduinoJson by Benoit Blanchon

## Setup Instructions

### 1. Configure Node ID

Edit the sketch and set a unique node ID for each device:

```cpp
#define NODE_ID 1  // Change to 2, 3, 4, etc. for other nodes
```

### 2. Upload to Boards

Deploy to at least 3 nodes for best mesh demonstration:

- Node 1: Coordinator (automatically elected)
- Node 2: Relay node
- Node 3: Edge node

### 3. Open Serial Monitor

Set baud rate to **115200** and observe the network formation.

## Expected Output

```
=================================
LoRa Mesh Networking Demo
Wildlife Monitoring System
=================================

Node ID: 1
Initializing LoRa mesh network...
✓ LoRa mesh initialized successfully

Attempting to become network coordinator...
✓ This node is now the coordinator

Starting mesh operations...

Network Status:
  Node ID: 1
  Connected Nodes: 2
  Packets RX/TX: 45 / 38
  Role: COORDINATOR
  Health: ✓ GOOD (mesh formed)

Signal Quality:
  RSSI: -72 dBm
  SNR: 8.5 dB
  Strength: ✓ GOOD (-70 to -85 dBm)
  Estimated Range: 1-3 km

Wildlife Detection Event:
  Species: Deer (87% confidence)
  Sending alert to mesh network...
  ✓ Alert queued for transmission

-----------------------------------
```

## Testing Scenarios

### Scenario 1: Network Formation

1. Power on Node 1 (becomes coordinator)
2. Power on Node 2 (discovers Node 1)
3. Power on Node 3 (discovers both nodes)
4. Observe mesh formation in serial output

### Scenario 2: Multi-Hop Communication

1. Deploy 3+ nodes in a line
2. Ensure edge nodes can't directly reach each other
3. Verify messages route through intermediate nodes
4. Check hop count in routing table

### Scenario 3: Coordinator Failover

1. Start with Node 1 as coordinator
2. Power off Node 1
3. Wait 60 seconds
4. Node with lowest remaining ID becomes coordinator
5. Verify in serial output

### Scenario 4: Signal Quality Monitoring

1. Move nodes to different distances
2. Observe RSSI changes in real-time
3. Note automatic route optimization
4. Check link reliability values

## Troubleshooting

### No LoRa Initialization

**Problem:** "LoRa mesh initialization failed"

**Solutions:**
- Check GPIO pin definitions match your hardware
- Verify LoRa module has stable 3.3V power
- Ensure SPI connections are correct
- Check antenna is attached

### No Nodes Discovered

**Problem:** "Connected Nodes: 0"

**Solutions:**
- Verify all nodes use same frequency (915/868 MHz)
- Check sync word matches (0x12 by default)
- Ensure nodes are within range (start < 100m)
- Confirm antennas are attached

### Poor Signal Quality

**Problem:** RSSI < -100 dBm

**Solutions:**
- Move nodes closer together
- Check antenna orientation
- Increase TX power (up to 20 dBm)
- Verify no metal obstructions

### Coordinator Not Detected

**Problem:** "Coordinator: Not detected"

**Solutions:**
- Ensure Node 1 is powered and initialized
- Wait for initial heartbeat (30 seconds)
- Check serial output on coordinator node
- Verify coordinator is calling becomeCoordinator()

## Performance Notes

### Range Estimates

| Environment | Expected Range |
|-------------|----------------|
| Open field | 2-5 km |
| Light forest | 1-3 km |
| Dense forest | 500m - 1.5 km |
| Urban area | 200m - 800m |

### Power Consumption

| Mode | Current Draw |
|------|--------------|
| Active TX | ~140 mA |
| Active RX | ~60 mA |
| Idle | ~20 mA |

### Message Latency

| Hops | Typical Latency |
|------|----------------|
| 1 hop | < 500 ms |
| 2 hops | < 1 second |
| 3 hops | < 2 seconds |
| 4-5 hops | < 5 seconds |

## Next Steps

### Production Deployment

1. **Add deep sleep** for power conservation
2. **Implement image compression** for wildlife photos
3. **Add GPS coordinates** for location tracking
4. **Enable encryption** for secure communication
5. **Integrate solar panels** for autonomous operation

### Advanced Features

- Custom message types for specific use cases
- Image transmission over mesh (chunked)
- Time-synchronized captures across nodes
- Dynamic spreading factor adjustment
- Adaptive power management

## Related Documentation

- [LORA_MESH_COMPATIBILITY.md](../../LORA_MESH_COMPATIBILITY.md) - Detailed compatibility guide
- [mesh_config.h](../../include/network/mesh_config.h) - Configuration parameters
- [lora_mesh.h](../../firmware/src/lora_mesh.h) - API reference

## Support

For issues or questions:
- Check the troubleshooting section above
- Review the compatibility guide
- Open an issue on GitHub with serial output logs
