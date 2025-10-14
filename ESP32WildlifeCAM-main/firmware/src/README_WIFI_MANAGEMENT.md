# WiFi Management and Fallback Systems

## Quick Reference

This directory contains the intelligent WiFi management and automatic fallback system implementation for ESP32WildlifeCAM.

## Core Components

### 1. WiFi AP Selector (`wifi_ap_selector.h/cpp`)
Smart access point selection with multi-criteria scoring:
- Signal strength analysis
- Historical performance tracking
- Channel congestion detection
- Network quality metrics

### 2. WiFi-LoRa Fallback (`wifi_lora_fallback.h/cpp`)
Automatic network switching coordinator:
- Health-based WiFi-LoRa switching
- Configurable fallback triggers
- Dual-mode transmission support
- Connection statistics tracking

### 3. Network Health Monitor (`network_health_monitor.h/cpp`)
Real-time monitoring and diagnostics:
- Connectivity metrics collection
- Performance analytics
- Issue detection and alerting
- Predictive failure analysis

### 4. Network API (`network_api.h/cpp`)
REST API for remote management:
- 20+ HTTP endpoints
- JSON request/response
- Configuration management
- Statistics access

## Quick Start

### Basic Usage

```cpp
#include "wifi_manager.h"
#include "wifi_ap_selector.h"
#include "wifi_lora_fallback.h"
#include "network_health_monitor.h"

// Initialize components
WiFiManager wifiMgr;
WiFiAPSelector apSelector;
WiFiLoRaFallback fallback;
NetworkHealthMonitor healthMon;

void setup() {
    wifiMgr.init();
    apSelector.init();
    
    FallbackConfig config;
    config.autoFallbackEnabled = true;
    fallback.init(&wifiMgr, config);
    
    healthMon.init();
    fallback.connect();
}

void loop() {
    fallback.update();
    healthMon.updateMetrics();
}
```

### Select Best Network

```cpp
// Scan for networks
apSelector.scanNetworks();

// Select best from known networks
std::vector<String> networks = {"Net1", "Net2"};
String best = apSelector.selectBestSSID(networks);
```

### Send Data with Fallback

```cpp
const uint8_t* data = ...;
size_t length = ...;

TransmissionResult result = fallback.sendData(data, length);
if (result.success) {
    Serial.println("Sent via " + String(result.usedNetwork));
}
```

### Check Network Health

```cpp
healthMon.updateMetrics();

if (!healthMon.isHealthy()) {
    auto recommendations = healthMon.getDiagnosticRecommendations();
    for (const auto& rec : recommendations) {
        Serial.println(rec);
    }
}
```

## Documentation

- **Complete Guide**: See [../../docs/WIFI_MANAGEMENT_GUIDE.md](../../docs/WIFI_MANAGEMENT_GUIDE.md)
- **Implementation Details**: See [../../WIFI_MANAGEMENT_IMPLEMENTATION.md](../../WIFI_MANAGEMENT_IMPLEMENTATION.md)
- **Example Code**: See [../../examples/wifi_management_example.cpp](../../examples/wifi_management_example.cpp)

## API Endpoints

### Network Status
- `GET /api/network/status` - Complete status
- `GET /api/network/wifi` - WiFi status
- `GET /api/network/lora` - LoRa status

### Health Monitoring
- `GET /api/health/metrics` - Health metrics
- `GET /api/health/status` - Health status
- `GET /api/health/alerts` - Active alerts

### Network Control
- `POST /api/network/scan` - Scan networks
- `POST /api/network/connect` - Connect to network
- `POST /api/network/switch/wifi` - Switch to WiFi
- `POST /api/network/switch/lora` - Switch to LoRa

See [network_api.h](network_api.h) for complete API reference.

## Configuration

### Fallback Configuration

```cpp
FallbackConfig config;
config.wifiRSSIThreshold = -75;      // dBm
config.autoFallbackEnabled = true;
config.preferWiFi = true;
config.healthCheckInterval = 10000;  // ms
```

### Health Monitor Configuration

```cpp
MonitorConfig config;
config.updateInterval = 10000;       // ms
config.enablePredictiveAnalysis = true;
config.packetLossThreshold = 0.1;    // 10%
config.latencyThreshold = 1000;      // ms
```

## Features

✅ Intelligent AP selection  
✅ Automatic WiFi-LoRa fallback  
✅ Real-time health monitoring  
✅ Predictive failure analysis  
✅ Complete REST API  
✅ Network statistics  
✅ Issue detection and alerts  
✅ Historical performance tracking  

## Memory Usage

Approximate RAM requirements:
- WiFiAPSelector: ~2KB + history
- WiFiLoRaFallback: ~1KB
- NetworkHealthMonitor: ~3KB + history
- NetworkAPI: ~1KB + JSON buffers
- **Total**: ~7KB base + ~5KB buffers

## Dependencies

- Arduino Core for ESP32
- WiFi library (built-in)
- ArduinoJson library
- Existing WiFiManager class
- Existing LoRa Mesh implementation

## Integration

This system integrates with existing components:
- `wifi_manager.h/cpp` - Base WiFi management
- `lora_mesh.h/cpp` - LoRa mesh networking
- Web server (for API endpoints)

No changes required to existing code - components work independently.

## Testing

Run the example to test all features:
```bash
cd examples
# Build and upload wifi_management_example.cpp
```

## Troubleshooting

### Network won't connect
- Check signal strength (> -75 dBm)
- Verify credentials
- Check scan results

### Frequent switching
- Increase `switchDebounceTime`
- Adjust signal thresholds
- Check for interference

### High packet loss
- Check signal strength
- Verify antenna
- Check for interference

See [troubleshooting guide](../../docs/WIFI_MANAGEMENT_GUIDE.md#troubleshooting) for details.

## Support

- Issues: [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Discussions: [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- Discord: [Community Chat](https://discord.gg/7cmrkFKx)

## License

Part of ESP32WildlifeCAM project - same license applies.

## Author

Implementation by GitHub Copilot for thewriterben  
Date: October 14, 2025
