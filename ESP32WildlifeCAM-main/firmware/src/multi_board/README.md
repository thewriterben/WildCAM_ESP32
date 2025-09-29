# Multi-Board Communication System

The ESP32 Wildlife Camera now includes a comprehensive multi-board communication system that enables multiple camera boards to work together as a coordinated wildlife monitoring network.

## Features

### Core Capabilities
- **Automatic Board Discovery**: Boards automatically discover each other on network startup
- **Role Assignment**: Dynamic assignment of coordinator and node roles based on board capabilities
- **Task Distribution**: Coordinator can assign tasks to nodes for load balancing
- **Network Topology**: Real-time mapping and monitoring of network structure
- **Failover Support**: Automatic coordinator election and standalone mode fallback

### Board Roles
- **Coordinator**: Master board managing the network, assigning roles and tasks
- **Node**: Camera board reporting to coordinator, executing assigned tasks
- **Edge Sensor**: Low-power motion detection with basic capture
- **AI Processor**: High-capability board handling AI analysis tasks
- **Hub**: High-resolution capture with local storage management
- **Stealth**: Ultra-low power monitoring with minimal RF emissions
- **Portable**: Mobile monitoring with cellular connectivity

### Message Types
- **Discovery**: Board capability advertisement and network joining
- **Heartbeat**: Regular status updates and network health monitoring
- **Role Assignment**: Coordinator assigns optimal roles to nodes
- **Task Assignment**: Coordinator distributes workload across network
- **Topology Updates**: Network structure and status synchronization
- **Configuration**: Centralized configuration management

## Quick Start

### Basic Integration

```cpp
#include "multi_board/multi_board_system.h"

void setup() {
    // Initialize multi-board system
    if (initializeMultiboardSystem(nodeId, ROLE_NODE)) {
        Serial.println("Multi-board system ready");
    }
}

void loop() {
    // Process multi-board coordination
    processMultiboardSystem();
    
    // Your existing code here
    delay(100);
}
```

### Advanced Integration

```cpp
#include "multi_board/multi_board_system.h"

MultiboardSystem multiboardSystem;

void setup() {
    // Configure system
    SystemConfig config;
    config.nodeId = 1;
    config.preferredRole = ROLE_AI_PROCESSOR;
    config.enableAutomaticRoleSelection = true;
    config.enableStandaloneFallback = true;
    
    // Initialize and start
    multiboardSystem.init(config);
    multiboardSystem.start();
    multiboardSystem.integrateWithLoraMesh(true);
}

void loop() {
    multiboardSystem.process();
    
    // Check if we're a coordinator
    if (multiboardSystem.getCurrentRole() == ROLE_COORDINATOR) {
        // Assign tasks to nodes
        DynamicJsonDocument params(256);
        params["resolution"] = "UXGA";
        params["quality"] = 10;
        
        multiboardSystem.sendTaskToNode(2, "image_capture", params.as<JsonObject>());
    }
}
```

## Configuration

### Enable Multi-Board Support

Add to your `config.h`:

```cpp
#define MULTIBOARD_ENABLED true
#define MULTIBOARD_NODE_ID 1              // Unique node ID
#define MULTIBOARD_PREFERRED_ROLE ROLE_NODE
#define MULTIBOARD_AUTO_ROLE_SELECTION true
#define MULTIBOARD_STANDALONE_FALLBACK true
```

### Network Settings

```cpp
#define MULTIBOARD_DISCOVERY_TIMEOUT 120000    // 2 minutes
#define MULTIBOARD_HEARTBEAT_INTERVAL 60000    // 1 minute
#define MULTIBOARD_TASK_TIMEOUT 300000         // 5 minutes
```

## Task Types

The system supports various task types that can be assigned to nodes:

- **`image_capture`**: Capture and store images
- **`motion_detection`**: Monitor for wildlife motion
- **`ai_analysis`**: Perform AI-based species identification
- **`data_transmission`**: Upload data to cloud/base station
- **`system_status`**: Report system health and diagnostics

## Network Topology

The system automatically maps network topology and maintains information about:

- Connected nodes and their capabilities
- Signal strength and hop count to each node
- Active/inactive status
- Coordinator election status
- Network stability metrics

## Integration with Existing Systems

The multi-board system integrates seamlessly with existing infrastructure:

- **LoRa Mesh**: Extends existing LoRa mesh with coordination messages
- **Network Selector**: Uses network selector for transport optimization
- **Board Abstraction**: Leverages existing board detection and HAL
- **Power Management**: Considers power profiles in role assignment

## Examples

See the `examples/multi_board_communication/` directory for:

- **`multi_board_communication_example.cpp`**: Complete standalone example
- **`main_with_multiboard.cpp`**: Integration with existing main.cpp

## API Reference

### Global Functions

```cpp
bool initializeMultiboardSystem(int nodeId, BoardRole preferredRole);
void processMultiboardSystem();
SystemStats getMultiboardStats();
bool sendMultiboardTask(int nodeId, const String& taskType, const JsonObject& parameters);
void cleanupMultiboardSystem();
```

### MultiboardSystem Class

```cpp
class MultiboardSystem {
public:
    bool init(const SystemConfig& config);
    bool start();
    void process();
    
    SystemState getSystemState() const;
    BoardRole getCurrentRole() const;
    SystemStats getSystemStats() const;
    
    bool sendTaskToNode(int nodeId, const String& taskType, const JsonObject& parameters);
    bool broadcastConfiguration(const JsonObject& config);
    
    std::vector<NetworkNode> getDiscoveredNodes() const;
    TopologyInfo getNetworkTopology() const;
};
```

## Performance Considerations

- **Memory Usage**: ~8KB RAM for typical 5-node network
- **Network Overhead**: ~200 bytes per message, configurable intervals
- **Discovery Time**: 1-2 minutes for full network discovery
- **Task Latency**: 1-5 seconds depending on network conditions

## Troubleshooting

### Common Issues

1. **No Coordinator Found**
   - Check LoRa mesh initialization
   - Verify boards are within communication range
   - Enable standalone fallback mode

2. **Role Assignment Failed**
   - Verify board capabilities detection
   - Check automatic role selection settings
   - Manually force role if needed

3. **Tasks Not Executing**
   - Check task execution is enabled
   - Verify task parameters are valid
   - Check node is active and responsive

### Debug Output

Enable debug output by setting:

```cpp
#define DEBUG_ENABLED true
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG
```

## Future Enhancements

Planned features for upcoming releases:

- **Phase 2**: Multi-transport support (ESP-NOW, WiFi Direct)
- **Phase 3**: Advanced load balancing and QoS
- **Phase 4**: Cloud integration and remote management

## Contributing

The multi-board system is designed to be extensible. To add new features:

1. Extend message protocol for new message types
2. Add handlers in coordinator or node classes
3. Update system manager for new capabilities
4. Add tests and examples

## License

This multi-board communication system is part of the ESP32 Wildlife Camera project and follows the same license terms.