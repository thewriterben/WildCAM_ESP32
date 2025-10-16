# Node Failure Detection and Task Reassignment

## Overview

The Node Failure Detection system enhances the reliability and robustness of the multi-board coordination system by automatically detecting failed nodes and reassigning their tasks to healthy nodes.

## Features

### 1. Heartbeat Tracking
- Each node's last heartbeat time is tracked in the `NetworkNode` structure via the `lastSeen` timestamp
- Heartbeats are updated when STATUS messages are received from nodes
- The coordinator node maintains this information for all managed nodes

### 2. Node Failure Detection
- **Timeout Duration**: 60 seconds (configurable via `NODE_FAILURE_TIMEOUT`)
- **Detection Method**: Periodic health checks during `processNodeManagement()`
- **Trigger**: If a node hasn't sent a heartbeat for more than 60 seconds, it's marked as failed
- **Implementation**: `checkNodeHealth()` method runs automatically as part of coordinator processing

### 3. Automatic Task Reassignment
When a node fails, all its active tasks are automatically reassigned to healthy nodes:

#### Task Selection Algorithm
The `selectHealthyNodeForTask()` method intelligently selects the best node based on:
- **AI Capabilities**: Nodes with AI processors are preferred for processing, detection, and analysis tasks (+50 points)
- **Battery Level**: Higher battery levels are preferred (up to +50 points)
- **Signal Strength**: Stronger RSSI signals are preferred (up to +50 points)
- **Current Load**: Nodes with fewer active tasks are preferred (-10 points per task)

#### Reassignment Process
1. Failed node is detected and marked as inactive
2. All active tasks assigned to the failed node are identified
3. For each task:
   - A healthy node is selected using the selection algorithm
   - Task assignment is updated with the new node ID
   - Task deadline is reset to current time + task timeout
   - New task assignment message is sent to the selected node
4. Reassignment statistics are logged

### 4. Comprehensive Logging

#### Node Failure Events
When a node fails, a detailed log entry is created containing:
- **Timestamp**: Current system time in milliseconds
- **Node ID**: The ID of the failed node
- **Reason**: Cause of failure (e.g., "Heartbeat timeout")
- **Uptime**: System uptime at the time of failure
- **Active Nodes**: Count of remaining active nodes
- **Tasks to Reassign**: Number of tasks that need to be reassigned

Example log output:
```
========================================
NODE FAILURE EVENT
========================================
Timestamp: 1234567 ms
Node ID: 5
Reason: Heartbeat timeout
Uptime at failure: 1234567 ms
Active nodes remaining: 3
Tasks to reassign: 2
========================================
```

## Implementation Details

### Configuration
```cpp
// Node failure timeout - configurable in board_coordinator.h
static const unsigned long NODE_FAILURE_TIMEOUT = 60000; // 60 seconds
```

### Key Methods

#### `checkNodeHealth()`
Scans all managed nodes and detects failures based on heartbeat timeout.

#### `markNodeAsFailed(int nodeId)`
Marks a node as inactive and triggers task reassignment.

#### `reassignTasksFromFailedNode(int failedNodeId)`
Reassigns all active tasks from a failed node to healthy nodes.

#### `selectHealthyNodeForTask(const String& taskType)`
Selects the best available healthy node for a given task type.

#### `logNodeFailure(int nodeId, const char* reason)`
Logs detailed information about a node failure event.

## Error Handling

### Race Conditions
- Health checks are performed during regular coordinator processing cycles
- Node status updates are atomic operations
- Task reassignment is done sequentially to avoid conflicts

### Network Delays
- The 60-second timeout accounts for temporary network issues
- Nodes can recover by sending heartbeats again
- Recovered nodes are automatically reactivated when heartbeats resume

### No Healthy Nodes Available
- If no healthy node is available for task reassignment, a warning is logged
- The task remains in the active tasks list
- Future health checks will attempt reassignment again if nodes become available

## System Stability

### Acceptance Criteria
✓ Heartbeat tracking implemented for all nodes
✓ Node marked as failed if heartbeat missed for 60 seconds
✓ Tasks reassigned to healthy nodes automatically
✓ Node failure events logged for audit
✓ System remains stable if one or more nodes fail

### Failover Scenarios

#### Single Node Failure
- Tasks are redistributed to remaining healthy nodes
- System continues normal operation
- Failed node can rejoin when it recovers

#### Multiple Node Failures
- Each failure is detected independently
- Tasks are reassigned based on available healthy nodes
- System continues operation as long as at least one node remains active

#### Coordinator Failure
- Handled by existing coordinator election mechanism
- Not part of this node failure detection system

## Testing

Comprehensive unit tests are provided in `test/test_node_failure_detection.cpp`:

- Heartbeat tracking initialization
- Node failure detection after timeout
- Task reassignment on node failure
- Healthy node selection algorithm
- Multiple simultaneous node failures
- Node recovery after failure
- Task deadline reset on reassignment
- No healthy nodes available scenario
- Node failure logging

## Usage Example

```cpp
// Initialize coordinator
BoardCoordinator coordinator;
coordinator.init(1);
coordinator.startCoordinator();

// In main loop
void loop() {
    // Process coordinator (includes automatic health checks)
    coordinator.process();
    
    // Node failure detection and task reassignment happen automatically
}
```

## Monitoring

Monitor node health via serial output:
```
⚠ Node 5 failure detected: no heartbeat for 65000 ms
✗ Node 5 marked as FAILED
↻ Reassigning task 3 from node 5 to node 2
↻ Reassigning task 7 from node 5 to node 3
✓ Reassigned 2 tasks from failed node 5
```

## Future Enhancements

Potential improvements for future versions:
- Configurable failure thresholds per node type
- Exponential backoff for repeated failures
- Node failure prediction based on trends
- Automatic node recovery testing
- Failure event persistence for post-mortem analysis
- Integration with external monitoring systems

## References

- [Board Coordinator Implementation](../firmware/src/multi_board/board_coordinator.cpp)
- [Message Protocol](../firmware/src/multi_board/message_protocol.h)
- [Discovery Protocol](../firmware/src/multi_board/discovery_protocol.h)
- [Test Suite](../test/test_node_failure_detection.cpp)
