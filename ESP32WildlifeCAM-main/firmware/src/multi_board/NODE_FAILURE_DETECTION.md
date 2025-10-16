# Node Failure Detection and Task Reassignment

## Overview
This document describes the node failure detection and task reassignment feature implemented in the multi-board coordination system.

## Implementation Details

### Constants
- `NODE_FAILURE_TIMEOUT`: 60000 ms (60 seconds) - Maximum time without heartbeat before marking a node as failed

### Core Components

#### 1. checkFailedNodes() - Main Detection Method
**Location**: `board_coordinator.cpp:545-590`

**Functionality**:
- Scans all managed nodes for heartbeat timeouts
- Compares current time (`millis()`) against each node's `lastSeen` timestamp
- Marks nodes as failed by setting `isActive = false`
- Logs detailed failure information including:
  - Node ID
  - Time since last heartbeat
  - Node role and signal strength
  - Hop count
- Triggers task reassignment for failed nodes
- Broadcasts updated network topology

**Called From**: `processNodeManagement()` method during coordinator's active state

#### 2. reassignTasksFromFailedNode() - Task Reassignment
**Location**: `board_coordinator.cpp:592-641`

**Functionality**:
- Identifies all active tasks assigned to a failed node
- Selects optimal healthy nodes for each task
- Updates task assignments in the active task list
- Sends task assignment messages to new nodes
- Handles cases where no healthy nodes are available
- Extends task deadlines if reassignment is not possible
- Provides detailed logging of all reassignment operations

#### 3. selectHealthyNodeForTask() - Intelligent Node Selection
**Location**: `board_coordinator.cpp:643-692`

**Functionality**:
Uses a scoring algorithm to select the best node for a task based on:

| Factor | Weight | Description |
|--------|--------|-------------|
| Task-specific capability | +30 to +50 | AI inference, storage, relay role matching |
| Battery level | +0 to +25 | Higher battery preferred |
| Signal strength | Variable | Closer nodes (better RSSI) preferred |
| Hop count | -5 per hop | Fewer hops preferred |
| Current load | -10 per task | Load balancing across nodes |

**Scoring Details**:
- **AI_INFERENCE tasks**: +50 points for nodes with AI capability
- **IMAGE_STORAGE tasks**: +40 points for nodes with SD card
- **RELAY tasks**: +30 points for nodes with relay role
- **Battery**: batteryLevel / 4 points (0-25 range)
- **Signal**: (signalStrength + 120) / 2 (converts dBm to positive score)
- **Load balancing**: -10 points per active task on the node

#### 4. sendNetworkTopology() - Topology Broadcasting
**Location**: `board_coordinator.cpp:694-701`

**Functionality**:
- Creates topology message with current node list
- Broadcasts to all nodes via LoRaMesh
- Logs success/failure of broadcast

## Integration Points

### Heartbeat Tracking
The system uses the existing `lastSeen` field in the `NetworkNode` structure, which is updated by:
1. `updateNodeStatus()` method when status messages are received
2. Discovery protocol when nodes respond to discovery messages
3. Any message reception from a node updates its `lastSeen` timestamp

### Process Loop Integration
```
BoardCoordinator::process()
  └─> processNodeManagement() [COORD_ACTIVE state]
       └─> checkFailedNodes()
            ├─> Scan all nodes for timeouts
            ├─> Mark failed nodes as inactive
            ├─> reassignTasksFromFailedNode()
            │    └─> selectHealthyNodeForTask()
            └─> sendNetworkTopology()
```

## Error Handling

### Race Conditions
- Sequential processing in single-threaded `process()` loop prevents concurrent access
- Node state changes are atomic (simple boolean flag)
- Task reassignment happens after marking nodes failed (deterministic order)

### Edge Cases Handled

1. **No Healthy Nodes Available**
   - Task deadline is extended by `taskTimeout` duration
   - Warning logged to serial console
   - Task remains in active list for future reassignment

2. **Coordinator Self-Check**
   - Coordinator skips itself in failure detection (`if (node.nodeId == nodeId_)`)
   - Prevents coordinator from marking itself as failed

3. **Network Degradation**
   - System logs number of failed nodes and remaining active nodes
   - Continues operation with reduced capacity
   - Automatic recovery when nodes come back online

4. **Message Delivery Failure**
   - Logs reassignment failures when messages can't be sent
   - Tasks remain in system for retry
   - Uses LoRaMesh's built-in retry mechanism

## Log Messages

### Node Failure Detection
```
❌ NODE FAILURE DETECTED: Node 3 - Last seen 62345 ms ago (timeout: 60000 ms)
   Node details: Role=AI_PROCESSOR, Signal=-75 dBm, HopCount=2
```

### Task Reassignment Success
```
   Reassigning 3 task(s) from failed node 3
   ✓ Task 101 (AI_INFERENCE) reassigned: 3 -> 5 (Priority: 2)
```

### Task Reassignment Failure
```
   ⚠️  No healthy node available to reassign task 102 (IMAGE_STORAGE)
```

### Network Status Update
```
⚠️  Network degradation: 1 node(s) failed, 4 active nodes remaining
📡 Network topology broadcast: 5 nodes
```

## Testing and Verification

### Manual Testing Procedure

1. **Setup Multi-Board Network**
   - Deploy at least 3 nodes with one coordinator
   - Assign tasks to various nodes
   - Monitor serial output

2. **Simulate Node Failure**
   - Power off one node or block its communications
   - Wait for 60+ seconds
   - Observe coordinator detecting the failure

3. **Verify Task Reassignment**
   - Check that tasks are reassigned to healthy nodes
   - Verify load balancing across available nodes
   - Confirm new nodes receive task assignments

4. **Test Recovery**
   - Restore failed node power/communication
   - Verify node rejoins network
   - Check that node can receive new tasks

5. **Stress Testing**
   - Fail multiple nodes simultaneously
   - Verify coordinator handles degraded network
   - Test edge case: fail all nodes except coordinator

### Expected Behaviors

✅ **Success Criteria**:
- Failed nodes detected within 60-90 seconds (heartbeat interval + timeout)
- Tasks reassigned to healthy nodes automatically
- Network remains operational with degraded capacity
- Detailed logs available for debugging
- System recovers when failed nodes return

❌ **Failure Indicators**:
- Tasks stuck on failed nodes
- No reassignment attempted
- Coordinator crashes or hangs
- No log messages about failures

## Performance Considerations

### Computational Cost
- **Per node check**: O(1) - simple timestamp comparison
- **Failed node scan**: O(n) where n = number of managed nodes
- **Task reassignment**: O(m × n) where m = tasks on failed node
- **Node selection**: O(n × t) where t = total active tasks (for load calculation)

### Memory Usage
- `std::vector<int> failedNodeIds`: Temporary, ~4 bytes per failed node
- `std::vector<Task> tasksToReassign`: Temporary, typically small (< 10 tasks per node)
- No persistent memory overhead

### Timing
- Detection delay: 60 seconds (configurable via `NODE_FAILURE_TIMEOUT`)
- Reassignment: Near-instantaneous (< 100ms for typical networks)
- Total recovery time: ~60-65 seconds from failure to reassignment

## Configuration

### Adjusting Timeout Value
To modify the failure detection timeout, edit `board_coordinator.h`:

```cpp
static const unsigned long NODE_FAILURE_TIMEOUT = 60000;   // 60 seconds
```

Recommended range: 30000-120000 ms (30 seconds to 2 minutes)
- Lower values: Faster detection but more false positives
- Higher values: More tolerant of network delays but slower recovery

### Disabling Feature
To disable node failure detection, comment out the call in `processNodeManagement()`:

```cpp
// checkFailedNodes();  // Disabled
```

## Future Enhancements

Potential improvements for future versions:

1. **Configurable Timeout**: Make `NODE_FAILURE_TIMEOUT` part of `NetworkConfig`
2. **Grace Period**: Add configurable grace period before first failure check
3. **Failure Count**: Track number of failures per node for reliability metrics
4. **Automatic Recovery**: Automatic role reassignment when failed nodes return
5. **Priority-Based Reassignment**: Prioritize critical tasks for reassignment
6. **Health Scores**: Track node reliability over time
7. **Notification System**: Alert external systems about node failures

## Related Files

- `board_coordinator.h` - Class declaration and constants
- `board_coordinator.cpp` - Implementation
- `message_protocol.h` - Message structures and node capabilities
- `discovery_protocol.h` - Node discovery and heartbeat handling
- `message_protocol.cpp` - createTopologyMessage() implementation

## Version History

- **v1.0** (2025-10-16): Initial implementation
  - 60-second failure detection
  - Automatic task reassignment
  - Intelligent node selection with load balancing
  - Comprehensive logging
