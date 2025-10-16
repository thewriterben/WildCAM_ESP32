# Network Management Task Implementation

## Overview
The `networkManagementTask` function implements comprehensive network management for the WildCAM ESP32 platform, including WiFi connectivity, data uploads, OTA firmware updates, and LoRa mesh networking.

## Features Implemented

### 1. WiFi Connection Management
- **Automatic connection**: Attempts WiFi connection when enabled and not connected
- **Exponential backoff**: Implements exponential backoff retry mechanism to prevent overwhelming the network
  - Base delay: 1 second
  - Exponential increase with each retry
  - Maximum backoff: 60 seconds
- **Connection monitoring**: Continuously checks if WiFi is still connected
- **Retry limit**: Maximum of 5 retries before resetting counter

### 2. Data Upload
- **Conditional upload**: Only uploads when WiFi is connected
- **Scheduled uploads**: Uploads data every 5 minutes (configurable)
- **Pending data tracking**: Tracks number of pending uploads
- **Error handling**: Handles upload failures gracefully with logging

### 3. OTA Firmware Updates
- **Version checking**: Checks for new firmware versions periodically (every hour)
- **Update detection**: Compares available version with current firmware
- **Ready for implementation**: Framework in place for actual OTA update process
- **Non-blocking**: Checks run without blocking other tasks

### 4. LoRa Mesh Network Health
- **Periodic health checks**: Monitors mesh network every minute
- **Node tracking**: Keeps track of active mesh nodes
- **Status logging**: Logs mesh network health status
- **Framework ready**: Structure in place for full mesh implementation

### 5. Network Status Logging
- **Comprehensive reporting**: Logs complete network status every 5 minutes
- **Includes all subsystems**:
  - WiFi connection state and signal strength
  - Data upload status and pending uploads
  - OTA update availability
  - LoRa mesh health and active nodes

## Configuration

All network management settings are configurable via `config.h`:

```cpp
// WiFi Settings
#define WIFI_ENABLED true
#define WIFI_SSID "WildCAM_Network"
#define WIFI_PASSWORD "wildlife123"
#define WIFI_CONNECTION_TIMEOUT 10000
#define WIFI_MAX_RETRIES 5
#define WIFI_RETRY_BASE_DELAY 1000

// OTA Settings
#define OTA_ENABLED true
#define OTA_CHECK_INTERVAL 3600000

// LoRa Settings
#define LORA_ENABLED true
#define LORA_HEALTH_CHECK_INTERVAL 60000

// Data Upload Settings
#define DATA_UPLOAD_ENABLED true
#define DATA_UPLOAD_INTERVAL 300000
```

## System State Tracking

The implementation uses the `SystemState` structure to track network status:

- `network_connected`: Current WiFi connection state
- `last_wifi_attempt`: Timestamp of last connection attempt
- `wifi_retry_count`: Current retry attempt count
- `last_upload`: Timestamp of last data upload
- `pending_uploads`: Number of data items pending upload
- `last_ota_check`: Timestamp of last OTA check
- `ota_available`: Whether an OTA update is available
- `last_lora_check`: Timestamp of last LoRa health check
- `lora_active_nodes`: Number of active LoRa mesh nodes
- `last_network_status_log`: Timestamp of last status log

## Exponential Backoff Implementation

The WiFi retry mechanism implements exponential backoff:

```cpp
unsigned long backoffDelay = WIFI_RETRY_BASE_DELAY * (1 << wifi_retry_count);
if (backoffDelay > 60000) backoffDelay = 60000; // Cap at 60 seconds
```

Retry delays:
- Attempt 1: 1 second
- Attempt 2: 2 seconds
- Attempt 3: 4 seconds
- Attempt 4: 8 seconds
- Attempt 5: 16 seconds
- Subsequent: 60 seconds (capped)

## Task Execution

The network management task runs on Core 0 with:
- **Priority**: 1 (low priority)
- **Stack size**: 6144 bytes
- **Check interval**: Every 30 seconds

## Error Handling

All operations include comprehensive error handling:
- WiFi connection failures are logged with retry information
- Upload failures are logged with HTTP status codes
- OTA check failures are logged but don't stop the system
- LoRa issues are logged as warnings

## Future Enhancements

The implementation includes TODO markers for future work:
1. Complete data upload logic with actual data retrieval
2. Implement full OTA update process with signature verification
3. Add complete LoRa mesh health monitoring with node pinging
4. Add remote configuration sync capabilities

## Integration

To use this implementation:

1. Configure WiFi credentials in `config.h`
2. Set OTA update server URLs
3. Configure data upload API endpoint
4. Enable/disable features as needed via compile-time flags

The task is automatically created and started by the main system initialization in `setup()`.
