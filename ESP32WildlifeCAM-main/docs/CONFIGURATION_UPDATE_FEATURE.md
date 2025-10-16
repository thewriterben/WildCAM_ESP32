# Configuration Update Feature - Board Node Implementation

## Overview

The board node configuration update feature allows the coordinator to dynamically update node configurations across the multi-board wildlife monitoring network. This enables centralized network management without requiring physical access to individual nodes.

## Implementation Details

### Location
- **File**: `firmware/src/multi_board/board_node.cpp`
- **Function**: `handleConfigUpdate(const MultiboardMessage& msg)` (lines 505-644)

### Configurable Parameters

The following parameters can be updated remotely:

| Parameter | Type | Valid Range | Description |
|-----------|------|-------------|-------------|
| `heartbeatInterval` | unsigned long | 10,000 - 600,000 ms | Interval between node heartbeat messages |
| `coordinatorTimeout` | unsigned long | 60,000 - 1,800,000 ms | Time before coordinator considered unavailable |
| `taskTimeout` | unsigned long | 30,000 - 1,200,000 ms | Maximum time allowed for task execution |
| `maxRetries` | int | 0 - 10 | Maximum number of retry attempts for failed operations |
| `enableAutonomousMode` | bool | true/false | Enable node to operate independently without coordinator |
| `enableTaskExecution` | bool | true/false | Enable or disable task execution on the node |

## Validation Rules

### Range Validation
All numeric parameters are validated against safe operating ranges:
- **heartbeatInterval**: Must be between 10 seconds and 10 minutes to ensure regular network presence without excessive battery drain
- **coordinatorTimeout**: Must be between 1 minute and 30 minutes to balance responsiveness and tolerance for network disruptions
- **taskTimeout**: Must be between 30 seconds and 20 minutes to allow sufficient time for complex tasks while preventing indefinite hangs
- **maxRetries**: Limited to 0-10 attempts to prevent infinite retry loops

### Type Validation
Boolean parameters are verified to ensure they are actual boolean types, not strings or numbers that could cause unexpected behavior.

### All-or-Nothing Validation
If ANY parameter fails validation, the ENTIRE configuration update is rejected. This ensures nodes maintain consistent, valid configurations at all times.

## Message Flow

```
┌──────────────┐                    ┌──────────────┐
│  Coordinator │                    │  Board Node  │
└──────┬───────┘                    └──────┬───────┘
       │                                   │
       │  MSG_CONFIG_UPDATE                │
       │  {                                │
       │    heartbeatInterval: 45000,      │
       │    maxRetries: 5,                 │
       │    ...                            │
       │  }                                │
       │──────────────────────────────────>│
       │                                   │
       │                           Validate parameters
       │                           Apply if valid
       │                                   │
       │  MSG_STATUS (Acknowledgment)      │
       │  {                                │
       │    config_update_status: "success"│
       │    acknowledged: true             │
       │  }                                │
       │<──────────────────────────────────│
       │                                   │
```

## Usage Example

### From Coordinator

```cpp
// Create configuration update
DynamicJsonDocument doc(512);
JsonObject config = doc.to<JsonObject>();

config["heartbeatInterval"] = 45000UL;     // 45 seconds
config["coordinatorTimeout"] = 300000UL;   // 5 minutes
config["taskTimeout"] = 180000UL;          // 3 minutes
config["maxRetries"] = 5;
config["enableAutonomousMode"] = true;
config["enableTaskExecution"] = true;

// Broadcast to all nodes
coordinator.broadcastConfigUpdate(config);
```

### Node Processing

The node automatically:
1. Receives the MSG_CONFIG_UPDATE message
2. Validates each parameter against acceptable ranges
3. Creates a temporary configuration with proposed changes
4. If all validations pass, applies the new configuration
5. Sends acknowledgment back to coordinator
6. Logs all actions for audit trail

## Acknowledgment Messages

### Success
```json
{
  "type": "MSG_STATUS",
  "source_node": 1,
  "target_node": 0,
  "timestamp": 1234567890,
  "data": {
    "config_update_status": "success",
    "acknowledged": true
  }
}
```

### Failure
```json
{
  "type": "MSG_STATUS",
  "source_node": 1,
  "target_node": 0,
  "timestamp": 1234567890,
  "data": {
    "config_update_status": "failed",
    "acknowledged": false,
    "error": "heartbeatInterval out of range (10s-10min); maxRetries out of range (0-10);"
  }
}
```

## Logging

The implementation provides detailed logging for troubleshooting:

### Successful Update
```
Received configuration update from coordinator
  ✓ heartbeatInterval: 45000 ms
  ✓ coordinatorTimeout: 300000 ms
  ✓ taskTimeout: 180000 ms
  ✓ maxRetries: 5
  ✓ enableAutonomousMode: true
  ✓ enableTaskExecution: true
✓ Configuration update applied successfully
  Updated configuration:
    - heartbeatInterval: 45000 ms
    - coordinatorTimeout: 300000 ms
    - taskTimeout: 180000 ms
    - maxRetries: 5
    - enableAutonomousMode: true
    - enableTaskExecution: true
✓ Configuration acknowledgment sent to coordinator
```

### Failed Update
```
Received configuration update from coordinator
  ✗ Invalid heartbeatInterval: 5000 ms
  ✗ Invalid maxRetries: 15
✗ Configuration update rejected due to validation errors:
  heartbeatInterval out of range (10s-10min); maxRetries out of range (0-10);
✓ Configuration acknowledgment sent to coordinator
```

## Error Handling

### Network Errors
If the acknowledgment message fails to send due to network issues:
- Error is logged: "✗ Failed to send configuration acknowledgment (network error)"
- Configuration changes are still applied (if valid)
- Coordinator contact time is updated to maintain network state

### Invalid Configuration
- Configuration is NOT applied
- Original configuration remains unchanged
- Detailed error message sent to coordinator
- All errors logged for debugging

## Security Considerations

1. **Range Validation**: Prevents malicious or accidental configurations that could harm node operation
2. **Type Checking**: Prevents type confusion attacks
3. **All-or-Nothing**: Ensures nodes never end up in partially-configured invalid states
4. **Audit Logging**: All configuration changes are logged with timestamps for security auditing

## Testing

Comprehensive test suite available in `test/test_board_node_config_update.cpp`:
- Valid configuration updates (all parameters)
- Partial configuration updates
- Invalid range tests for each parameter
- Invalid type tests for boolean fields
- Mixed valid/invalid parameter tests
- Acknowledgment message format verification

## Example Code

See `examples/multi_board_communication/config_update_example.cpp` for a complete working example demonstrating:
- Valid configuration updates
- Invalid configuration updates  
- Partial configuration updates
- Configuration status display

## Future Enhancements

Possible improvements for future releases:
1. Configuration versioning to track changes over time
2. Rollback capability to revert to previous configurations
3. Configuration profiles for different operational modes
4. Scheduled configuration updates
5. Configuration backup and restore functionality
