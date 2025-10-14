# Automatic Network Discovery Example

This example demonstrates the automatic network discovery and device coordination capabilities of the ESP32 Wildlife Camera system.

## Overview

The automatic network discovery system allows multiple ESP32 camera boards to:
- **Automatically discover** each other and form a mesh network
- **Dynamically assign roles** based on device capabilities (battery, AI, storage, etc.)
- **Coordinate in real-time** with automatic topology updates
- **Monitor network health** and handle device failures gracefully

## Features Demonstrated

### 1. Automatic Device Joining
- New devices automatically join the mesh network when powered on
- No manual configuration needed beyond setting a unique NODE_ID
- Immediate topology broadcast to all nodes when new device joins

### 2. Dynamic Role Assignment
The system automatically assigns roles based on device capabilities:
- **AI Processor**: High-resolution cameras with AI capabilities and sufficient power
- **Hub**: High-resolution with large storage capacity
- **Stealth**: Low-power devices or low battery (< 30%)
- **Relay**: Devices with solar power and good battery (> 50%)
- **Portable**: Devices with cellular/satellite connectivity
- **Node**: Standard camera nodes (default)

### 3. Real-Time Coordination
- Continuous topology monitoring even after discovery completes
- Automatic coordinator election based on device scores
- Role reassignment when device capabilities change
- Network health monitoring with real-time metrics

### 4. Network Health Monitoring
Health score calculation based on:
- Active node percentage (40% weight)
- Average signal strength (40% weight)
- Network hop efficiency (20% weight)

## Hardware Requirements

- **Required**: ESP32-CAM or compatible ESP32 board
- **Recommended**: LoRa module for long-range mesh networking
- **Optional**: SD card for storage, solar panel for continuous operation

## Setup Instructions

### 1. Configure Node ID
Edit the example sketch and set a unique NODE_ID for each device:
```cpp
#define NODE_ID 1  // Change for each device: 1, 2, 3, etc.
```

### 2. Set Preferred Role (Optional)
Choose a preferred role for the device:
```cpp
#define PREFERRED_ROLE ROLE_NODE  // or ROLE_COORDINATOR, ROLE_RELAY, etc.
```

Note: The system will automatically assign the optimal role based on capabilities, but the preferred role is used as a hint.

### 3. Flash to Multiple Boards
Flash the sketch to at least 2-3 ESP32 boards with different NODE_IDs.

### 4. Power On and Monitor
Power on the boards and monitor the serial output (115200 baud) to observe:
- Discovery process
- Network formation
- Role assignments
- Topology updates

## Expected Output

### Initial Discovery
```
=================================
ESP32 Wildlife Camera
Automatic Network Discovery Demo
=================================

Initializing Node 1 with preferred role: Node
✓ Multi-board system initialized
✓ LoRa mesh integration enabled
✓ Multi-board system started

🔍 Starting automatic network discovery...

🔍 Discovering network...
  Nodes discovered: 0
  ⏳ Searching for coordinator...
```

### Device Joins Network
```
✓ New device joined network: Node 2 (role: Node, score: 65.0)
⚡ Topology changed: 1 managed nodes
✓ Discovery complete: Found 1 nodes
```

### Coordinator Active
```
👑 Operating as Coordinator
  Managing 2 nodes
  Active tasks: 0
  Network efficiency: 100.0%
```

### Role Assignment
```
Node capabilities: Relay (Solar=5.2V, Battery=75%)
✓ Role assignment: Node -> Relay (from coordinator node 1)
```

## Network Topology Visualization

```
┌─────────────────┐
│  Coordinator    │  Node 1: High score (90.0)
│   (Node 1)      │  Role: Coordinator
└────────┬────────┘  Battery: 85%
         │
    ┌────┴────┬────────────┬──────────┐
    │         │            │          │
┌───┴───┐ ┌──┴───┐  ┌─────┴────┐ ┌───┴────┐
│ Node 2│ │Node 3│  │  Node 4  │ │ Node 5 │
│ Relay │ │ Node │  │ AI Proc  │ │Stealth │
└───────┘ └──────┘  └──────────┘ └────────┘
Solar=5V  Bat=60%   AI+PSRAM     Bat=20%
Bat=75%             Bat=70%      Low power
```

## Serial Commands

Type commands in the serial monitor to interact with the system:

- `status` - Print detailed system status including topology
- `discover` - Trigger a new network discovery
- `help` - Show available commands

## Key Metrics

### System Status Report (every 2 minutes)
```
=================================
SYSTEM STATUS REPORT
=================================
Node ID: 1
System State: 3
Current Role: Coordinator
Uptime: 120 seconds

Network Status:
  Networked Nodes: 3
  Active Tasks: 0
  Messages Processed: 45
  Network Efficiency: 100.0%

Topology:
  Total Nodes: 3
  Active Nodes: 3
  Coordinator: Node 1
  Network Stable: Yes
  Last Update: 5000 ms ago

Discovered Nodes:
  Node 2: Relay, RSSI=-55, Hops=1, Battery=75%
  Node 3: Node, RSSI=-62, Hops=1, Battery=60%
  Node 4: AI Processor, RSSI=-58, Hops=1, Battery=70%
=================================
```

## Configuration Options

### Discovery Timing
```cpp
#define DISCOVERY_INTERVAL 30000      // How often to scan (30s)
#define ADVERTISEMENT_INTERVAL 60000  // How often to advertise (60s)
#define STATUS_REPORT_INTERVAL 120000 // Status reporting (2min)
```

### System Configuration
```cpp
config.enableAutomaticRoleSelection = true;  // Auto-assign roles
config.enableStandaloneFallback = true;      // Standalone if no network
config.discoveryTimeout = 120000;            // Discovery timeout (2min)
config.roleChangeTimeout = 60000;            // Role change timeout (1min)
```

## Troubleshooting

### No Devices Discovered
- Check that NODE_IDs are unique
- Verify LoRa module is connected and working
- Increase DISCOVERY_INTERVAL for slower networks
- Check antenna connections and placement

### Coordinator Not Elected
- Verify at least one device has sufficient battery (> 50%)
- Check that devices have different coordinator scores
- Ensure discovery process completes (wait 2+ minutes)

### Roles Not Assigned
- Enable automatic role selection in config
- Verify coordinator is active
- Check that devices have capability differences
- Monitor serial output for role assignment messages

### Network Unstable
- Reduce node timeout in discovery protocol
- Improve antenna placement and orientation
- Add relay nodes to improve connectivity
- Check for interference on LoRa frequency

## Advanced Usage

### Custom Role Assignment Logic
Modify `determineOptimalRole()` in `board_coordinator.cpp` to customize role assignment based on your specific requirements.

### Network Health Thresholds
Adjust health calculation in `getNetworkHealth()` in `discovery_protocol.cpp` to weight factors differently.

### Discovery Intervals
Tune discovery and advertisement intervals based on:
- Network size (more nodes = longer intervals)
- Movement speed (mobile = shorter intervals)
- Power budget (low power = longer intervals)

## Related Documentation

- [Discovery Protocol](../../firmware/src/multi_board/discovery_protocol.h)
- [Board Coordinator](../../firmware/src/multi_board/board_coordinator.h)
- [Message Protocol](../../firmware/src/multi_board/message_protocol.h)
- [Multi-Board System](../../firmware/src/multi_board/multi_board_system.h)

## Success Criteria

The system demonstrates successful automatic network discovery when:
- ✓ New devices join mesh network automatically
- ✓ Topology updates in real-time when devices join/leave
- ✓ Node capabilities are assessed and roles assigned dynamically
- ✓ Coordinator is elected automatically based on scores
- ✓ Network health is monitored and reported
- ✓ System operates reliably with 3+ nodes

## License

This example is part of the ESP32 Wildlife Camera project and is released under the same license.
