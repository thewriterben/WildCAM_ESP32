# Configuration Update Quick Reference

## Sending Configuration Updates (Coordinator)

```cpp
// Create configuration
DynamicJsonDocument doc(512);
JsonObject config = doc.to<JsonObject>();

// Set parameters (all optional)
config["heartbeatInterval"] = 45000UL;     // 10s - 10min
config["coordinatorTimeout"] = 300000UL;   // 1min - 30min
config["taskTimeout"] = 180000UL;          // 30s - 20min
config["maxRetries"] = 5;                  // 0 - 10
config["enableAutonomousMode"] = true;
config["enableTaskExecution"] = true;

// Broadcast to all nodes
coordinator.broadcastConfigUpdate(config);
```

## Valid Ranges

| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| heartbeatInterval | 10,000 | 600,000 | milliseconds |
| coordinatorTimeout | 60,000 | 1,800,000 | milliseconds |
| taskTimeout | 30,000 | 1,200,000 | milliseconds |
| maxRetries | 0 | 10 | count |

## Node Behavior

- ✅ Valid config → Applied immediately, ACK sent
- ❌ Invalid config → Rejected, error ACK sent
- ⚠️ Partial config → Only specified params updated
- 🔒 Mixed valid/invalid → Entire update rejected

## Checking Current Configuration

```cpp
const NodeConfig& config = node.getNodeConfig();
Serial.printf("Heartbeat: %lu ms\n", config.heartbeatInterval);
Serial.printf("Coordinator timeout: %lu ms\n", config.coordinatorTimeout);
Serial.printf("Task timeout: %lu ms\n", config.taskTimeout);
Serial.printf("Max retries: %d\n", config.maxRetries);
Serial.printf("Autonomous: %s\n", config.enableAutonomousMode ? "ON" : "OFF");
Serial.printf("Task execution: %s\n", config.enableTaskExecution ? "ON" : "OFF");
```

## Common Use Cases

### Reduce Network Traffic
```cpp
config["heartbeatInterval"] = 120000UL;  // 2 minutes instead of default 1 min
```

### Extend Coordinator Timeout (Poor Network)
```cpp
config["coordinatorTimeout"] = 900000UL;  // 15 minutes
```

### Emergency Mode (Disable Autonomous Operation)
```cpp
config["enableAutonomousMode"] = false;
config["enableTaskExecution"] = false;
```

### Performance Mode (Quick Tasks)
```cpp
config["taskTimeout"] = 60000UL;    // 1 minute
config["maxRetries"] = 2;            // Fail fast
```

### Reliability Mode (Poor Conditions)
```cpp
config["taskTimeout"] = 600000UL;   // 10 minutes
config["maxRetries"] = 8;            // More retries
```
