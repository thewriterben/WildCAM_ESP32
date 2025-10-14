# Satellite Communication Network Integration

## Overview

The ESP32 WildCAM satellite communication system provides seamless integration between multiple network types, enabling reliable wildlife monitoring in remote areas. The system automatically selects the optimal network based on availability, cost, power consumption, and message priority.

## Supported Networks

### Network Priority Hierarchy

1. **WiFi** (Highest Priority)
   - Free or minimal cost
   - High bandwidth
   - Low power consumption
   - Ideal for accessible locations

2. **LoRa Mesh**
   - Free
   - Long range (5+ km)
   - Low power consumption
   - Ideal for distributed sensor networks

3. **Cellular**
   - Moderate cost
   - Wide coverage
   - Medium power consumption
   - Good for semi-remote areas

4. **Satellite** (Fallback for Remote Areas)
   - Highest cost
   - Global coverage
   - High power consumption
   - Essential for remote locations

## Supported Satellite Modems

### Iridium
- **Coverage**: Global
- **Message Size**: 160 bytes (SBD)
- **Cost**: ~$0.95/message
- **Best For**: Emergency communications, reliable global coverage

### Swarm M138
- **Coverage**: Global (LEO constellation)
- **Message Size**: 192 bytes
- **Cost**: ~$0.05/message
- **Best For**: Cost-effective remote monitoring

### RockBLOCK
- **Coverage**: Global (Iridium network)
- **Message Size**: 340 bytes
- **Cost**: ~$0.95/message
- **Best For**: Compatible with Iridium network

## Integration Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     Wildlife Camera System                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              Network Selector                             │  │
│  │  • Automatic network detection                            │  │
│  │  • Intelligent fallback                                   │  │
│  │  • Cost optimization                                      │  │
│  │  • Power management                                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│           │              │              │              │         │
│           ▼              ▼              ▼              ▼         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐       │
│  │   WiFi   │  │   LoRa   │  │ Cellular │  │Satellite │       │
│  │ Manager  │  │   Mesh   │  │ Manager  │  │   Comm   │       │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘       │
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              Emergency Protocol                           │  │
│  │  • Automatic fallback on failure                          │  │
│  │  • Retry with exponential backoff                         │  │
│  │  • Multi-network redundancy                               │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Network Selection Logic

### Normal Operation
```cpp
// Priority: WiFi > LoRa > Cellular > Satellite
NetworkType selected = networkSelector.selectOptimalNetwork(dataSize, PRIORITY_NORMAL);
```

### Emergency Operation
```cpp
// Priority: Satellite (most reliable) > WiFi > Cellular > LoRa
NetworkType selected = networkSelector.selectOptimalNetwork(dataSize, PRIORITY_EMERGENCY);
```

### Cost Optimization
```cpp
// Enable cost optimization (prefer free networks)
networkSelector.setCostOptimization(true);
```

## Configuration

### Basic Satellite Configuration

```cpp
#include "satellite_integration.h"

// Initialize satellite communication
SatelliteConfig config;
config.enabled = true;
config.module = MODULE_SWARM;              // Use Swarm for lower cost
config.transmissionInterval = 3600;        // 1 hour between transmissions
config.maxDailyMessages = 24;              // Limit to 24 messages/day
config.maxDailyCost = 25.0;                // $25 daily budget
config.costOptimization = true;            // Enable cost optimization
config.prioritizeEmergency = true;         // Emergency messages bypass limits

satelliteIntegration.updateSatelliteConfig(config);
```

### Network Selector Configuration

```cpp
#include "network_selector.h"

NetworkSelector networkSelector;
SatelliteComm satelliteComm;
CellularManager cellularManager;

// Initialize with all available network interfaces
networkSelector.initialize(&cellularManager, &satelliteComm);

// Enable cost optimization
networkSelector.setCostOptimization(true);

// Scan for available networks
networkSelector.scanAvailableNetworks();
```

## Usage Examples

### Wildlife Detection Alert

```cpp
// High-confidence wildlife detection
if (confidence > 0.8) {
    String alert = "WILDLIFE:DEER,CONF:0.87,LOC:N47.123W122.456";
    
    // Send via optimal network
    bool success = networkSelector.sendData(
        (const uint8_t*)alert.c_str(),
        alert.length(),
        PRIORITY_HIGH
    );
    
    if (success) {
        Serial.println("Wildlife alert sent successfully");
    }
}
```

### Periodic Status Update

```cpp
// Low-priority status message
String status = "STATUS:BAT=85%,TEMP=22C,IMAGES=42";

bool success = networkSelector.sendData(
    (const uint8_t*)status.c_str(),
    status.length(),
    PRIORITY_LOW  // Can wait for optimal window
);
```

### Emergency Alert

```cpp
// Critical emergency condition
satelliteComm.enableEmergencyMode();

String emergency = "EMERGENCY:LOW_BATTERY:12%";

// Will try all available networks
bool success = networkSelector.sendData(
    (const uint8_t*)emergency.c_str(),
    emergency.length(),
    PRIORITY_EMERGENCY
);

if (success) {
    Serial.println("Emergency alert delivered");
} else {
    // Store locally for retry
    satelliteComm.storeMessage(emergency, SAT_PRIORITY_EMERGENCY);
}

satelliteComm.disableEmergencyMode();
```

## Fallback Protocol

The system implements intelligent fallback with automatic retry:

### Normal Message Flow
```
1. Select optimal network based on priority and cost
2. Attempt transmission
3. On failure, store message for later retry
4. Wait for next optimal window
5. Retry with exponential backoff
```

### Emergency Message Flow
```
1. Enable emergency mode
2. Try satellite (most reliable in remote areas)
3. If fails, try WiFi
4. If fails, try cellular
5. If fails, try LoRa mesh
6. Retry immediately with short delay
7. Continue until successful
```

## Power Management

### Low Battery Mode

When battery drops below threshold, the system automatically:
- Reduces satellite transmission frequency
- Queues messages for optimal transmission windows
- Prefers low-power networks (WiFi, LoRa)

```cpp
// Automatic power optimization based on battery level
satelliteComm.optimizePowerConsumption(batteryLevel);

if (satelliteComm.isInLowPowerMode()) {
    Serial.println("Low power mode active");
    // Transmission interval doubled
    // Non-emergency messages deferred
}
```

### Power Consumption by Network

| Network   | Idle    | Transmit | Notes                          |
|-----------|---------|----------|--------------------------------|
| WiFi      | 80 mA   | 120 mA   | Depends on signal strength     |
| LoRa      | 2 mA    | 120 mA   | Very low idle consumption      |
| Cellular  | 10 mA   | 200 mA   | Varies by signal and network   |
| Satellite | 50 mA   | 500 mA   | High power during transmission |

## Cost Optimization

### Daily Cost Tracking

```cpp
// Check current cost
float cost = satelliteComm.getTodayCost();
Serial.printf("Today's cost: $%.2f\n", cost);

// Check if within budget
if (satelliteComm.isWithinCostLimit()) {
    // Satellite available
} else {
    // Budget exceeded, use other networks
}
```

### Scheduled Transmission

```cpp
// Wait for optimal satellite pass
TransmissionWindow window = satelliteComm.getOptimalTransmissionWindow();

switch (window) {
    case WINDOW_IMMEDIATE:
        // Send now
        break;
    case WINDOW_NEXT_PASS:
        // Wait for next satellite pass
        time_t nextPass = satelliteComm.getNextPassTime();
        break;
    case WINDOW_SCHEDULED:
        // Wait for scheduled transmission time
        break;
}
```

## Field Deployment Scenarios

### Scenario 1: Research Station (WiFi Available)

```cpp
// WiFi is primary, satellite as backup
SatelliteConfig researchConfig;
researchConfig.enabled = true;
researchConfig.module = MODULE_SWARM;      // Cost-effective
researchConfig.transmissionInterval = 7200; // 2 hours
researchConfig.maxDailyMessages = 12;      // Limited satellite use
researchConfig.costOptimization = true;

// WiFi will be preferred when available
```

### Scenario 2: Remote Wilderness (No Infrastructure)

```cpp
// Satellite is primary communication
SatelliteConfig remoteConfig;
remoteConfig.enabled = true;
remoteConfig.module = MODULE_IRIDIUM;      // More reliable
remoteConfig.transmissionInterval = 3600;  // 1 hour
remoteConfig.maxDailyMessages = 24;        // Regular updates
remoteConfig.prioritizeEmergency = true;
remoteConfig.autoPassPrediction = true;    // Optimize timing

// LoRa mesh for inter-device communication
```

### Scenario 3: Emergency Monitoring

```cpp
// Immediate satellite access
SatelliteConfig emergencyConfig;
emergencyConfig.enabled = true;
emergencyConfig.module = MODULE_IRIDIUM;
emergencyConfig.transmissionInterval = 900;  // 15 minutes
emergencyConfig.maxDailyMessages = 96;       // No practical limit
emergencyConfig.costOptimization = false;    // Reliability over cost
emergencyConfig.prioritizeEmergency = true;
```

## Testing and Validation

### Integration Tests

Run the comprehensive test suite:

```bash
cd ESP32WildlifeCAM-main/test
pio test -e native
```

### Field Testing Checklist

- [ ] WiFi connectivity in accessible areas
- [ ] LoRa mesh communication between devices
- [ ] Cellular connection and data transmission
- [ ] Satellite modem initialization
- [ ] Emergency fallback protocol
- [ ] Cost tracking accuracy
- [ ] Power consumption optimization
- [ ] Message storage and retry
- [ ] Pass prediction accuracy
- [ ] Low battery behavior

## Troubleshooting

### Satellite Not Connecting

1. Check power supply (satellite modems require stable power)
2. Verify antenna connection
3. Check for obstructions (satellite needs clear sky view)
4. Verify SIM card (for Iridium/RockBLOCK)
5. Check module initialization in serial monitor

### High Cost

1. Enable cost optimization: `config.costOptimization = true`
2. Increase transmission interval
3. Use Swarm instead of Iridium for lower cost
4. Enable pass prediction for optimal timing
5. Reduce maxDailyMessages limit

### Messages Not Sending

1. Check network availability: `scanAvailableNetworks()`
2. Verify message size limits
3. Check daily message limit
4. Review cost limits
5. Check stored message queue

### Battery Draining Fast

1. Enable low power mode
2. Increase transmission intervals
3. Use scheduled transmission windows
4. Prefer WiFi/LoRa over satellite
5. Check for continuous scanning

## Best Practices

1. **Always configure fallback networks** - Don't rely on a single network type
2. **Enable cost optimization** - Unless in emergency scenarios
3. **Use scheduled transmissions** - Wait for optimal satellite passes
4. **Implement message queuing** - Store messages when networks unavailable
5. **Monitor battery levels** - Adjust behavior based on power status
6. **Test emergency protocols** - Ensure fallback works before deployment
7. **Set appropriate limits** - Configure daily message and cost limits
8. **Use compression** - Enable message compression for satellite
9. **Prioritize messages** - Use appropriate priority levels
10. **Log all transmissions** - Track success/failure for debugging

## API Reference

### NetworkSelector

```cpp
class NetworkSelector {
public:
    // Initialize with network interfaces
    bool initialize(CellularManager* cellular, SatelliteComm* satellite);
    
    // Scan for available networks
    bool scanAvailableNetworks();
    
    // Select optimal network
    NetworkType selectOptimalNetwork(size_t dataSize, MessagePriority priority);
    
    // Send data via optimal network
    bool sendData(const uint8_t* data, size_t length, MessagePriority priority);
    
    // Get current network
    NetworkType getCurrentNetwork();
    
    // Get available networks
    std::vector<NetworkInfo> getAvailableNetworks();
    
    // Configure cost optimization
    void setCostOptimization(bool enabled);
};
```

### SatelliteComm

```cpp
class SatelliteComm {
public:
    // Initialize satellite module
    bool initialize(SatelliteModule module);
    
    // Configure satellite settings
    bool configure(const SatelliteConfig& config);
    
    // Send message with priority
    bool sendMessage(const String& message, SatelliteMessagePriority priority);
    
    // Send emergency alert
    bool sendEmergencyAlert(const String& alertData);
    
    // Check satellite availability
    bool checkSatelliteAvailability();
    
    // Get signal quality
    int checkSignalQuality();
    
    // Power management
    void optimizePowerConsumption(uint8_t batteryLevel);
    bool isInLowPowerMode() const;
    
    // Cost tracking
    float getTodayCost() const;
    bool isWithinCostLimit() const;
    
    // Pass prediction
    bool predictNextPass();
    time_t getNextPassTime();
    TransmissionWindow getOptimalTransmissionWindow();
};
```

## Further Reading

- [Satellite Configuration Guide](SATELLITE_CONFIG.md)
- [Power Management Guide](POWER_MANAGEMENT.md)
- [Network Topology Manager](NETWORK_TOPOLOGY.md)
- [Field Deployment Guide](DEPLOYMENT_GUIDE.md)
- [API Documentation](API_REFERENCE.md)

## Support

For issues and questions:
- GitHub Issues: [WildCAM_ESP32 Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Discord: [Community Chat](https://discord.gg/7cmrkFKx)
- Email: benjamin.j.snider@gmail.com

---

**Last Updated**: October 14, 2025  
**Version**: 1.0  
**Status**: Production Ready
