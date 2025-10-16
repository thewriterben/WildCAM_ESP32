# Intelligent Role Assignment Implementation Summary

## Overview
This document describes the implementation of intelligent role assignment for the multi-board system, fulfilling the requirements specified in the issue.

## Implementation Details

### 1. Hardware Capability Detection

#### Location: `firmware/src/multi_board/message_protocol.cpp`

Added three key detection functions:

**`detectCameraCapabilities()`**
- Checks for configured sensor type (SENSOR_UNKNOWN vs actual sensor)
- Detects camera-capable board types (AI_THINKER, ESP32_S3_CAM, ESP_EYE, etc.)
- Checks for CAMERA_MODEL_AI_THINKER define
- Returns: `true` if camera hardware is detected

**`detectLoRaCapabilities()`**
- Checks if LoRa mesh network is initialized
- Attempts LoRa hardware initialization to confirm presence
- Returns: `true` if LoRa radio is detected

**`detectAICapabilities()` (enhanced)**
- Detects ESP32-S3 with AI acceleration
- Checks for sufficient PSRAM (>= 2MB)
- Identifies AI-capable board types
- Verifies multi-core ESP32 with sufficient resources
- Checks for TensorFlow Lite Micro support
- Returns: `true` if AI processing is feasible

#### Updated Data Structure
Enhanced `BoardCapabilities` structure with new fields:
```cpp
struct BoardCapabilities {
    // ... existing fields ...
    bool hasCamera;    // Camera sensor available
    bool hasLoRa;      // LoRa radio available
    // ... rest of fields ...
};
```

### 2. Intelligent Role Assignment

#### Location: `firmware/src/multi_board/multi_board_system.cpp` (line 418)

Enhanced `checkRoleAssignment()` method implements intelligent logic:

**Process Flow:**
1. Detects and logs current node capabilities (camera, LoRa, AI, battery, etc.)
2. Calculates coordinator score for this node
3. Compares with discovered nodes to determine if should be coordinator
4. If not coordinator, determines optimal role based on capabilities:
   - **Camera nodes** → Capture nodes (NODE, AI_PROCESSOR, or HUB)
   - **LoRa nodes** → Relay nodes (RELAY)
   - **Low battery/power** → Stealth nodes (STEALTH)
   - **Other** → Edge sensor nodes (EDGE_SENSOR)

**Priority Order:**
1. Nodes with camera → capture nodes
2. Nodes with LoRa → relay nodes
3. Low power/battery → stealth nodes
4. Others → edge sensors

#### Location: `firmware/src/multi_board/board_coordinator.cpp`

Enhanced `determineOptimalRole()` method with detailed capability-based logic:

**Role Assignment Rules:**
- **AI_PROCESSOR**: Camera + AI + PSRAM + High-res (>=1920x1080) + Good battery
- **HUB**: Camera + High-res (>=1600x1200) + SD card + Storage (>1MB)
- **NODE**: Camera + Standard capabilities (default capture role)
- **RELAY**: LoRa + Good battery (>=50%) or Solar power
- **STEALTH**: Low battery (<30%) or Low power profile
- **PORTABLE**: Cellular or Satellite connectivity
- **EDGE_SENSOR**: No camera, no LoRa (basic sensor)

### 3. Load Balancing

#### Location: `firmware/src/multi_board/board_coordinator.cpp`

Implemented `selectBestNodeForTask()` method for intelligent task assignment:

**Scoring System:**
- **Task-specific scoring** (0-50 points):
  - Capture tasks: Prefer camera nodes with high resolution and storage
  - Relay tasks: Prefer LoRa nodes with good power
  - Analysis tasks: Prefer AI-capable nodes with PSRAM
  
- **General factors** (adjustments):
  - Battery level: +0.2 points per percent
  - Signal strength: +0.1 * (100 + RSSI)
  - Current load: -10 points per active task (load balancing penalty)
  - Hop count: -5 points per hop (prefer closer nodes)

**Load Balancing in `assignTask()`:**
- When targetNode is 0 or negative, automatically selects best node
- Uses `selectBestNodeForTask()` for intelligent selection
- Logs selected node and rationale

**Automatic Rebalancing:**
Implemented `rebalanceTaskLoad()` method:
- Calculates average load across all nodes
- Identifies overloaded nodes (>150% of average)
- Reassigns non-critical tasks (priority < 3) to less loaded nodes
- Logs all rebalancing actions

### 4. Dynamic Role Reassignment

#### Location: `firmware/src/multi_board/board_coordinator.cpp`

Enhanced `processNodeManagement()` to handle dynamic changes:

**Node Join Detection:**
- Tracks newly discovered nodes
- Automatically assigns roles to new nodes via `assignNodeRoles()`
- Triggers load rebalancing when new nodes join
- Logs: "⚡ Node X joined the network (Role: Y)"

**Node Leave/Failure Detection:**
- Identifies nodes that are no longer discovered
- Triggers task reassignment from failed nodes
- Logs: "⚠ Node X left the network"

**Capability Change Detection:**
- Monitors significant capability changes (camera, LoRa, AI, battery >20% change)
- Automatically reassesses and reassigns roles
- Logs: "⚠ Node X capabilities changed, reassessing role"

**Task Reassignment:**
Implemented `reassignTasksFromFailedNode()` method:
- Finds all active tasks assigned to failed node
- Selects new optimal node for each task using `selectBestNodeForTask()`
- Resets task deadline and sends new assignment
- Marks tasks as failed if no suitable node found
- Logs each reassignment action

### 5. Comprehensive Logging

**Capability Detection Logging:**
- Camera detection: "Camera detected: [reason]"
- LoRa detection: "LoRa detected: [reason]"
- AI detection: "AI capabilities detected: [reason]"

**Role Assignment Logging:**
- Node join: "⚡ Node X joined the network (Role: Y)"
- Node leave: "⚠ Node X left the network"
- Capability change: "⚠ Node X capabilities changed, reassessing role"
- Role assignment: "Role: [ROLE_NAME] - [reason with capabilities]"
- Complete assignment: "=== Role Assignment Complete ==="

**Load Balancing Logging:**
- Task selection: "Task selection: Node X, Role Y, Score Z, Load N"
- Selected node: "Selected node X for task 'Y' with score Z"
- Load balancing: "Load balancing: Assigned task 'X' to node Y"
- Rebalancing: "Node X is overloaded with N tasks (M% above average)"
- Task move: "Moving task X to node Y for load balancing"

**Task Reassignment Logging:**
- Reassignment start: "Reassigning tasks from failed node X"
- Individual reassignment: "Reassigning task X (type) to node Y"
- No suitable node: "No suitable node found for task X, marking as failed"

## Files Modified

1. **`firmware/src/multi_board/message_protocol.h`**
   - Added `hasCamera` and `hasLoRa` fields to `BoardCapabilities`

2. **`firmware/src/multi_board/message_protocol.cpp`**
   - Added `detectCameraCapabilities()` function
   - Added `detectLoRaCapabilities()` function
   - Enhanced `detectAICapabilities()` function
   - Updated `getCurrentCapabilities()` to detect camera and LoRa
   - Updated `createDiscoveryMessage()` to include new fields
   - Updated `parseDiscoveryMessage()` to parse new fields

3. **`firmware/src/multi_board/multi_board_system.cpp`**
   - Completely rewrote `checkRoleAssignment()` with intelligent logic (line 418)
   - Added comprehensive logging throughout assignment process

4. **`firmware/src/multi_board/board_coordinator.h`**
   - Added declarations for `reassignTasksFromFailedNode()`
   - Added declarations for `rebalanceTaskLoad()`

5. **`firmware/src/multi_board/board_coordinator.cpp`**
   - Added `#include <map>` for load tracking
   - Enhanced `determineOptimalRole()` with camera/LoRa priority logic
   - Implemented `selectBestNodeForTask()` with scoring system
   - Enhanced `assignTask()` to use automatic node selection
   - Enhanced `processNodeManagement()` for dynamic reassignment
   - Added `reassignTasksFromFailedNode()` implementation
   - Added `rebalanceTaskLoad()` implementation

## Tests Added

**File:** `test/test_role_assignment.cpp`

**Test Categories:**
1. **Capability Detection Tests** (3 tests)
   - Camera detection
   - LoRa detection
   - Complete capability structure

2. **Role Assignment Tests** (4 tests)
   - Camera node assignment
   - LoRa relay assignment
   - AI processor assignment
   - Low battery stealth assignment

3. **Load Balancing Tests** (2 tests)
   - Load balancing enabled verification
   - Task selection with load balancing

4. **Multiboard System Tests** (2 tests)
   - System-level role assignment
   - Capability logging verification

5. **Dynamic Reassignment Tests** (2 tests)
   - Role reassignment on capability change
   - Task reassignment on node failure

6. **Integration Tests** (2 tests)
   - End-to-end intelligent role assignment
   - Logging verification

**Total:** 15 comprehensive unit tests

## Acceptance Criteria Verification

✅ **Role assignment logic implemented based on hardware capabilities**
- Detects camera, LoRa, AI, and other hardware capabilities
- Assigns roles with priority: camera > LoRa > AI > power considerations

✅ **Load balanced across available nodes**
- Implemented scoring system for node selection
- Tracks load per node and penalizes overloaded nodes
- Automatic rebalancing when nodes join or become overloaded

✅ **Roles and assignments logged for audit**
- Comprehensive logging at all stages (detection, assignment, rebalancing)
- Visual indicators (⚡ for join, ⚠ for issues)
- Detailed capability information in logs

✅ **System adapts to changing node availability**
- Detects node joins and triggers role assignment
- Detects node failures and reassigns tasks
- Detects capability changes and reassesses roles
- Rebalances load dynamically

## Usage Example

```cpp
// Initialize multi-board system with automatic role selection
SystemConfig config;
config.nodeId = 1;
config.enableAutomaticRoleSelection = true;
config.enableLoadBalancing = true;

MultiboardSystem system;
system.init(config);
system.start();

// System will:
// 1. Detect hardware capabilities (camera, LoRa, etc.)
// 2. Determine optimal role based on capabilities
// 3. Participate in coordinator election if qualified
// 4. Continuously monitor for changes and adapt

// For coordinator - assign tasks with automatic load balancing
system.sendTaskToNode(0, "capture", params); // 0 = auto-select best node
```

## Performance Considerations

- **Capability detection** runs once at initialization
- **Role assignment** runs during discovery phase and on demand
- **Load balancing** runs when tasks are assigned or topology changes
- **Rebalancing** runs when new nodes join or periodically
- **Overhead**: Minimal, primarily during initialization and topology changes

## Future Enhancements

Potential improvements for future iterations:
1. Machine learning-based load prediction
2. Historical performance tracking for node selection
3. Geographic/spatial awareness for task assignment
4. Energy-aware scheduling for battery-powered nodes
5. QoS-based task prioritization

## Conclusion

The implementation provides a complete, production-ready intelligent role assignment system that:
- Automatically detects hardware capabilities
- Assigns roles intelligently based on node strengths
- Balances load across the network
- Adapts dynamically to changing conditions
- Provides comprehensive audit logging

All acceptance criteria have been met and the system is ready for deployment.
