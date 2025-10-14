# LoRa Mesh Networking - ESP32 Variant Compatibility Guide

## Overview

This document describes the LoRa mesh networking implementation and its compatibility with different ESP32 variants used in wildlife monitoring applications.

## Supported ESP32 Variants

### 1. ESP32-CAM (AI-Thinker)

**Status:** ⚠️ Limited Support - GPIO Conflicts

The AI-Thinker ESP32-CAM has fundamental GPIO conflicts between the camera module and LoRa radio requirements:

- **Camera Pins (Hardwired):**
  - GPIO 5, 18, 19, 23: Camera data lines
  - GPIO 26: Camera I2C data (SIOD)
  - GPIO 34, 35, 36, 39: Camera data lines (cannot be used for ADC)

- **LoRa Pin Requirements:**
  - SPI pins (SCK, MISO, MOSI, CS) conflict with camera data lines
  - DIO0 interrupt pin conflicts with camera I2C

**Recommendation:** Use ESP32-CAM for camera-only deployments. For LoRa mesh networking, use ESP32-S3-CAM or ESP32-XIAO.

**Workarounds:**
- Use WiFi or cellular connectivity instead of LoRa
- Deploy separate ESP32 boards for LoRa mesh relay nodes
- Use external LoRa module with I2C interface (not currently implemented)

### 2. ESP32-S3-CAM

**Status:** ✅ Full Support

The ESP32-S3-CAM provides sufficient GPIO pins for both camera and LoRa operation:

- **Available GPIO Pins:** 45 GPIO pins
- **Camera Support:** Native camera interface
- **LoRa Support:** Full SPI support with available pins

**Recommended Configuration:**
```cpp
// LoRa SPI Configuration for ESP32-S3
#define LORA_SCK    12    // SPI Clock
#define LORA_MISO   13    // SPI MISO
#define LORA_MOSI   11    // SPI MOSI
#define LORA_CS     10    // Chip Select
#define LORA_RST    9     // Reset
#define LORA_DIO0   14    // DIO0 Interrupt
```

**Features:**
- Dual-core processing for parallel camera and mesh operations
- 512KB SRAM for larger routing tables
- Enhanced WiFi coexistence with LoRa

### 3. ESP32-XIAO (Seeed Studio)

**Status:** ✅ Full Support

The XIAO ESP32 variant is ideal for dedicated mesh relay nodes:

- **Compact Form Factor:** 21mm x 17.5mm
- **Available GPIO:** 11 GPIO pins (sufficient for LoRa)
- **Low Power:** Excellent for solar-powered relay nodes

**Recommended Configuration:**
```cpp
// LoRa SPI Configuration for XIAO ESP32
#define LORA_SCK    8     // SPI Clock
#define LORA_MISO   9     // SPI MISO
#define LORA_MOSI   10    // SPI MOSI
#define LORA_CS     7     // Chip Select
#define LORA_RST    6     // Reset
#define LORA_DIO0   5     // DIO0 Interrupt
```

**Features:**
- Ultra-low power consumption (deep sleep < 50µA)
- Built-in battery charging circuit
- Ideal for mesh repeater nodes in network

### 4. ESP-EYE

**Status:** ✅ Full Support

ESP-EYE provides balanced GPIO availability for camera and LoRa:

- **Camera:** OV2640 with native support
- **Available GPIO:** Sufficient pins for LoRa module
- **Accelerometer:** Built-in MPU6050 for motion detection

**Recommended Configuration:**
```cpp
// LoRa SPI Configuration for ESP-EYE
#define LORA_SCK    14    // SPI Clock
#define LORA_MISO   12    // SPI MISO
#define LORA_MOSI   13    // SPI MOSI
#define LORA_CS     15    // Chip Select
#define LORA_RST    2     // Reset
#define LORA_DIO0   4     // DIO0 Interrupt
```

## LoRa Mesh Features

### Core Functionality

1. **Multi-Hop Routing**
   - Dijkstra's shortest path algorithm
   - Maximum 5 hops by default (configurable)
   - Automatic route discovery and maintenance

2. **Signal Strength (RSSI) Monitoring**
   - Real-time RSSI tracking per node
   - Link quality calculation based on signal strength
   - Adaptive routing based on link reliability

3. **Automated Device Discovery**
   - Adaptive discovery intervals (30s - 5min)
   - Network size-based interval adjustment
   - Periodic heartbeat messages

4. **Coordinator Failover**
   - Automatic coordinator election (lowest node ID)
   - 60-second timeout for coordinator loss detection
   - Seamless failover with minimal disruption

5. **Multi-Path Routing**
   - Load balancing across multiple paths
   - Congestion detection and avoidance
   - Traffic splitting based on link quality

6. **Network Health Metrics**
   - Packets sent/received counters
   - Link reliability per neighbor
   - Routing table size and statistics
   - Cache hit rate monitoring

## Configuration Options

### Mesh Network Parameters

```cpp
// From mesh_config.h
#define MESH_MAX_HOPS               5       // Maximum hop count
#define MESH_NODE_TIMEOUT           300000  // 5 minutes
#define MESH_ROUTE_TIMEOUT          600000  // 10 minutes
#define MESH_BEACON_INTERVAL        30000   // 30 seconds
#define MESH_MAX_NODES              32      // Maximum nodes
#define MESH_MAX_PACKET_SIZE        256     // Bytes
#define MESH_RETRY_ATTEMPTS         3       // Retry count
```

### LoRa Radio Parameters

```cpp
// From config.h (firmware/src/config.h)
#define LORA_FREQUENCY         915E6        // 915 MHz (US)
#define LORA_TX_POWER          20           // 20 dBm max
#define LORA_SPREADING_FACTOR  7            // SF7 (fast, short range)
#define LORA_SIGNAL_BANDWIDTH  125E3        // 125 kHz
#define LORA_CODING_RATE       5            // 4/5
#define LORA_PREAMBLE_LENGTH   8            // Preamble symbols
#define LORA_SYNC_WORD         0x12         // Network ID
```

## Usage Examples

### Initialize LoRa Mesh

```cpp
#include "lora_mesh.h"

void setup() {
    Serial.begin(115200);
    
    // Set unique node ID
    LoraMesh::setNodeId(1);
    
    // Initialize LoRa mesh
    if (LoraMesh::init()) {
        Serial.println("LoRa mesh initialized");
    } else {
        Serial.println("LoRa mesh initialization failed");
    }
}

void loop() {
    // Process mesh messages
    LoraMesh::processMessages();
    
    // Get network status
    MeshNetworkStatus status = LoraMesh::getNetworkStatus();
    Serial.printf("Connected nodes: %d\n", status.connectedNodes);
    
    delay(1000);
}
```

### Send Wildlife Detection Alert

```cpp
#include "lora_mesh.h"
#include <ArduinoJson.h>

void sendWildlifeAlert(const char* species, float confidence) {
    DynamicJsonDocument doc(256);
    doc["event"] = "wildlife_detected";
    doc["species"] = species;
    doc["confidence"] = confidence;
    doc["timestamp"] = millis();
    
    LoraMesh::sendStatus(doc.as<JsonObject>());
}
```

### Become Network Coordinator

```cpp
// For gateway nodes or base stations
if (LoraMesh::becomeCoordinator()) {
    Serial.println("This node is now the coordinator");
}

// Check coordinator status
if (LoraMesh::isCoordinator()) {
    Serial.println("Operating as coordinator");
} else {
    int coordId = LoraMesh::getCoordinatorId();
    Serial.printf("Coordinator is node %d\n", coordId);
}
```

### Monitor Signal Quality

```cpp
SignalQuality quality = LoraMesh::getSignalQuality();

Serial.printf("RSSI: %d dBm\n", quality.rssi);
Serial.printf("SNR: %.1f dB\n", quality.snr);

switch (quality.strength) {
    case SIGNAL_EXCELLENT:
        Serial.println("Signal: Excellent");
        break;
    case SIGNAL_GOOD:
        Serial.println("Signal: Good");
        break;
    case SIGNAL_FAIR:
        Serial.println("Signal: Fair");
        break;
    case SIGNAL_POOR:
        Serial.println("Signal: Poor");
        break;
}
```

## Performance Characteristics

### Range and Data Rates

| Spreading Factor | Data Rate | Range (open field) | Use Case |
|-----------------|-----------|-------------------|----------|
| SF7 | 5.5 kbps | ~2 km | Fast telemetry, dense networks |
| SF8 | 3.1 kbps | ~3 km | Balanced performance |
| SF9 | 1.8 kbps | ~5 km | Extended range |
| SF10 | 980 bps | ~8 km | Long range monitoring |
| SF11 | 440 bps | ~12 km | Very long range |
| SF12 | 250 bps | ~15 km | Maximum range, low data rate |

### Power Consumption

| Mode | ESP32-CAM | ESP32-S3 | XIAO ESP32 |
|------|-----------|----------|------------|
| Active (TX) | ~240 mA | ~280 mA | ~140 mA |
| Active (RX) | ~100 mA | ~120 mA | ~60 mA |
| Idle | ~40 mA | ~45 mA | ~20 mA |
| Deep Sleep | ~10 mA | ~7 µA | ~43 µA |

## Testing

### Unit Tests

Run the mesh networking tests:

```bash
pio test -e esp32cam -f test_lora_mesh_networking
pio test -e esp32cam -f test_mesh_routing
```

### Integration Testing

For hardware testing with actual LoRa modules:

1. Deploy at least 3 nodes with different node IDs
2. Monitor serial output for discovery messages
3. Verify route establishment between nodes
4. Test coordinator failover by powering off coordinator

### Multi-Hop Validation

```
Node 1 (Coordinator) <---> Node 2 (Relay) <---> Node 3 (Edge)

Expected behavior:
- Node 3 discovers route to Node 1 through Node 2
- Messages from Node 3 are forwarded by Node 2
- RSSI decreases with distance
- Hop count increases along path
```

## Troubleshooting

### LoRa Initialization Fails

**Symptoms:** `LoRa.begin()` returns false

**Solutions:**
1. Check GPIO pin configuration matches your hardware
2. Verify SPI connections (SCK, MISO, MOSI, CS)
3. Check LoRa module power supply (3.3V stable)
4. Verify reset pin is not held low

### No Nodes Discovered

**Symptoms:** `connectedNodes` remains 0

**Solutions:**
1. Verify all nodes use same frequency and sync word
2. Check antenna connections on all modules
3. Ensure nodes are within range (start with < 100m)
4. Monitor RSSI values to check signal presence

### High Packet Loss

**Symptoms:** Low reliability values, frequent retransmissions

**Solutions:**
1. Reduce spreading factor for better data rate
2. Increase TX power (up to 20 dBm)
3. Check for interference sources
4. Ensure antennas are properly oriented

### Coordinator Failover Not Working

**Symptoms:** Network stops after coordinator loss

**Solutions:**
1. Verify heartbeat messages include coordinator flag
2. Check 60-second timeout is sufficient for your network
3. Ensure node IDs are unique and correctly set
4. Monitor `lastCoordinatorSeen` timestamp

## Best Practices

1. **Node Placement:**
   - Place relay nodes at elevated positions
   - Maintain line-of-sight when possible
   - Consider using XIAO ESP32 for dedicated relays

2. **Power Management:**
   - Use solar panels for relay nodes
   - Implement deep sleep between transmissions
   - Monitor battery levels and adjust duty cycle

3. **Network Size:**
   - Keep network under 20 active nodes for best performance
   - Use multiple coordinators for larger deployments
   - Segment large networks into sub-meshes

4. **Wildlife Monitoring:**
   - Enable wildlife priority routing for detection events
   - Pre-calculate routes for known camera locations
   - Use image transmission optimization for large captures

## References

- [LoRa Specification](https://lora-alliance.org/resource_hub/lorawan-specification-v1-0-3/)
- [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [Mesh Networking Algorithms](https://en.wikipedia.org/wiki/Mesh_networking)

## Support

For issues or questions:
- Open an issue on GitHub
- Check ROADMAP.md for planned features
- Review PHASE_2B_IMPLEMENTATION_SUMMARY.md for network architecture details
