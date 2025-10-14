# WiFi Management and Fallback Systems Guide

## Overview

The ESP32WildlifeCAM project includes comprehensive WiFi management and automatic fallback systems designed for robust field deployments. This guide covers the intelligent WiFi management features, automatic network switching, and health monitoring capabilities.

## Table of Contents

1. [Features](#features)
2. [Architecture](#architecture)
3. [Components](#components)
4. [Quick Start](#quick-start)
5. [API Reference](#api-reference)
6. [Configuration](#configuration)
7. [Examples](#examples)
8. [Troubleshooting](#troubleshooting)

## Features

### Intelligent WiFi Management
- **Smart AP Selection**: Automatically selects the best available access point based on signal strength, historical performance, and network load
- **Multi-Network Support**: Manages multiple known networks with priority and preference settings
- **Connection Optimization**: Continuously monitors and optimizes WiFi connections
- **Historical Performance Tracking**: Maintains connection success/failure history for informed decisions

### Automatic Network Fallback
- **WiFi-LoRa Switching**: Seamlessly switches between WiFi and LoRa mesh networks
- **Health-Based Triggers**: Automatically falls back based on signal quality, connection stability, and timeout conditions
- **Dual-Mode Support**: Optional simultaneous WiFi+LoRa transmission for critical data
- **Intelligent Reconnection**: Automatic reconnection with exponential backoff

### Network Health Monitoring
- **Real-Time Metrics**: Tracks uptime, packet loss, latency, throughput, and signal quality
- **Issue Detection**: Automatically detects and reports network problems
- **Predictive Analysis**: Predicts potential failures based on trend analysis
- **Performance Analytics**: Provides detailed performance metrics and recommendations

### REST API
- **Full API Access**: Query status, control networks, and access metrics via HTTP endpoints
- **JSON Responses**: All API responses in structured JSON format
- **Real-Time Updates**: WebSocket support for streaming updates (future enhancement)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  (Camera Control, Data Collection, Task Management)          │
└───────────────────────┬─────────────────────────────────────┘
                        │
┌───────────────────────┴─────────────────────────────────────┐
│                    Network API Layer                         │
│         (REST API, Configuration, Control)                   │
└───────────────────────┬─────────────────────────────────────┘
                        │
┌───────────────────────┴─────────────────────────────────────┐
│              WiFi-LoRa Fallback Coordinator                  │
│    (Automatic Switching, Health Monitoring, Routing)         │
└─────┬──────────────────────────┬────────────────────────────┘
      │                          │
┌─────┴──────┐          ┌────────┴────────┐
│ WiFi Mgmt  │          │   LoRa Mesh     │
│ + AP       │          │   Network       │
│ Selector   │          │                 │
└────────────┘          └─────────────────┘
      │                          │
┌─────┴──────────────────────────┴────────┐
│      Network Health Monitor              │
│  (Metrics, Alerts, Diagnostics)          │
└──────────────────────────────────────────┘
```

## Components

### WiFiAPSelector

Intelligent access point selection and management.

**Key Features:**
- Scans and ranks available networks
- Maintains historical performance data
- Calculates network quality scores
- Supports preferred networks

**Usage:**
```cpp
WiFiAPSelector apSelector;
apSelector.init();

// Scan for networks
apSelector.scanNetworks();

// Select best network
std::vector<String> knownNetworks = {"Network1", "Network2"};
String bestSSID = apSelector.selectBestSSID(knownNetworks);

// Get network metrics
NetworkMetrics metrics = apSelector.calculateMetrics(ap);
```

### WiFiLoRaFallback

Manages automatic switching between WiFi and LoRa networks.

**Key Features:**
- Automatic health-based fallback
- Manual network switching
- Dual-mode transmission
- Connection statistics

**Usage:**
```cpp
WiFiLoRaFallback fallback;
FallbackConfig config;
config.autoFallbackEnabled = true;
config.preferWiFi = true;

fallback.init(&wifiManager, config);
fallback.connect();

// Update in main loop
void loop() {
    fallback.update();
}

// Send data
TransmissionResult result = fallback.sendData(data, length);
```

### NetworkHealthMonitor

Monitors network health and performance.

**Key Features:**
- Real-time metric collection
- Issue detection and alerting
- Predictive failure analysis
- Performance recommendations

**Usage:**
```cpp
NetworkHealthMonitor monitor;
MonitorConfig config;
config.enablePredictiveAnalysis = true;

monitor.init(config);

// Record transmission
monitor.recordTransmission(success, bytes, latency);

// Check health
if (monitor.isHealthy()) {
    // Network is healthy
}

// Get diagnostic report
String report = monitor.getDiagnosticReport();
```

### NetworkAPI

REST API for network management and monitoring.

**Key Features:**
- HTTP endpoints for all operations
- JSON request/response format
- Configuration management
- Statistics access

**Usage:**
```cpp
NetworkAPI api;
api.init(&wifiManager, &apSelector, &fallback, &healthMonitor);

// Handle API requests
String response = api.getNetworkStatus();
String health = api.getHealthMetrics();
```

## Quick Start

### Basic Setup

```cpp
#include "wifi_manager.h"
#include "wifi_ap_selector.h"
#include "wifi_lora_fallback.h"
#include "network_health_monitor.h"

WiFiManager wifiManager;
WiFiAPSelector apSelector;
WiFiLoRaFallback fallback;
NetworkHealthMonitor healthMonitor;

void setup() {
    // Initialize WiFi manager
    wifiManager.init();
    
    // Initialize AP selector
    apSelector.init();
    
    // Initialize fallback system
    FallbackConfig fallbackConfig;
    fallback.init(&wifiManager, fallbackConfig);
    
    // Initialize health monitor
    MonitorConfig monitorConfig;
    healthMonitor.init(monitorConfig);
    
    // Connect to network
    fallback.connect();
}

void loop() {
    // Update systems
    fallback.update();
    healthMonitor.updateMetrics();
    
    // Your application code here
    delay(100);
}
```

### Sending Data with Automatic Fallback

```cpp
void sendTelemetry(const uint8_t* data, size_t length) {
    TransmissionResult result = fallback.sendData(data, length);
    
    if (result.success) {
        Serial.println("Data sent successfully");
        healthMonitor.recordTransmission(true, length, result.transmissionTime);
    } else {
        Serial.println("Transmission failed: " + result.errorMessage);
        healthMonitor.recordTransmission(false, length);
    }
}
```

## API Reference

### REST Endpoints

#### Network Status
```
GET /api/network/status
```
Returns complete network status including WiFi and LoRa information.

**Response:**
```json
{
  "success": true,
  "data": {
    "activeNetwork": "wifi",
    "connected": true,
    "healthy": true,
    "signalStrength": -65,
    "totalSwitches": 3,
    "wifi": {
      "connected": true,
      "health": 1,
      "rssi": -65,
      "uptime": 123456
    },
    "lora": {
      "connected": false,
      "health": 5,
      "rssi": -100,
      "uptime": 0
    }
  }
}
```

#### Health Metrics
```
GET /api/health/metrics
```
Returns detailed health metrics.

**Response:**
```json
{
  "success": true,
  "data": {
    "networkUptimePercentage": 98.5,
    "successfulTransmissions": 150,
    "failedTransmissions": 2,
    "packetLossRate": 0.013,
    "averageLatencyMs": 45,
    "throughputKbps": 250.5,
    "networkEfficiency": 0.987
  }
}
```

#### Scan Networks
```
POST /api/network/scan
```
Initiates a network scan.

#### Switch Network
```
POST /api/network/switch/wifi
POST /api/network/switch/lora
```
Manually switches to specified network.

#### Connect to Network
```
POST /api/network/connect
Content-Type: application/json

{
  "ssid": "MyNetwork",
  "password": "MyPassword"
}
```

## Configuration

### Fallback Configuration

```cpp
FallbackConfig config;
config.wifiRSSIThreshold = -75;           // Minimum WiFi signal (dBm)
config.loraRSSIThreshold = -100;          // Minimum LoRa signal (dBm)
config.connectionTimeout = 30000;         // Connection timeout (ms)
config.healthCheckInterval = 10000;       // Health check interval (ms)
config.autoFallbackEnabled = true;        // Enable auto-switching
config.preferWiFi = true;                 // Prefer WiFi when available
config.allowDualMode = false;             // Allow WiFi+LoRa simultaneously
config.switchDebounceTime = 5000;         // Min time between switches (ms)
config.maxReconnectAttempts = 3;          // Max reconnection attempts
```

### Health Monitor Configuration

```cpp
MonitorConfig config;
config.updateInterval = 10000;            // Metric update interval (ms)
config.latencyTestInterval = 60000;       // Latency test interval (ms)
config.throughputTestInterval = 300000;   // Throughput test interval (ms)
config.enablePredictiveAnalysis = true;   // Enable failure prediction
config.enableAutoRecovery = true;         // Enable auto-recovery
config.packetLossThreshold = 0.1;         // 10% packet loss threshold
config.latencyThreshold = 1000;           // 1 second latency threshold (ms)
```

### AP Selection Criteria

```cpp
SelectionCriteria criteria;
criteria.minRSSI = -75;                   // Minimum signal strength
criteria.preferStrongerSignal = true;     // Prioritize signal strength
criteria.considerHistory = true;          // Use historical data
criteria.avoidCongestion = true;          // Avoid congested channels
criteria.maxAge = 300000;                 // Max scan age (5 minutes)
```

## Examples

### Complete Integration Example

See [examples/wifi_management_example.cpp](../examples/wifi_management_example.cpp) for a complete working example demonstrating all features.

### Monitoring Network Health

```cpp
void monitorHealth() {
    healthMonitor.updateMetrics();
    
    if (!healthMonitor.isHealthy()) {
        Serial.println("Network issues detected:");
        
        auto issues = healthMonitor.getActiveIssues();
        for (const auto& issue : issues) {
            Serial.printf("  - %s: %s\n", 
                         issue.component.c_str(),
                         issue.description.c_str());
        }
        
        auto recommendations = healthMonitor.getDiagnosticRecommendations();
        Serial.println("\nRecommendations:");
        for (const auto& rec : recommendations) {
            Serial.println("  - " + rec);
        }
    }
}
```

### Manual Network Switching

```cpp
void switchNetworks() {
    // Check current network
    ActiveNetwork current = fallback.getActiveNetwork();
    
    if (current == NETWORK_WIFI) {
        // Switch to LoRa
        if (fallback.switchToLoRa(REASON_MANUAL)) {
            Serial.println("Switched to LoRa");
        }
    } else {
        // Switch to WiFi
        if (fallback.switchToWiFi(REASON_MANUAL)) {
            Serial.println("Switched to WiFi");
        }
    }
}
```

### Periodic Network Optimization

```cpp
void optimizeNetwork() {
    // Scan for networks
    apSelector.scanNetworks();
    
    // Get current SSID
    String currentSSID = wifiManager.getStatus().ssid;
    
    // Find best available network
    std::vector<String> knownNetworks = {"Net1", "Net2", "Net3"};
    String bestSSID = apSelector.selectBestSSID(knownNetworks);
    
    // Switch if better network found
    if (!bestSSID.isEmpty() && bestSSID != currentSSID) {
        Serial.printf("Better network found: %s\n", bestSSID.c_str());
        wifiManager.connect(bestSSID, "password");
    }
}
```

## Troubleshooting

### Common Issues

#### Network Won't Connect
1. Check SSID and password
2. Verify signal strength (> -75 dBm recommended)
3. Check if network is in range
4. Review connection logs

#### Frequent Network Switching
1. Increase `switchDebounceTime` in FallbackConfig
2. Adjust signal thresholds
3. Check for environmental interference
4. Review health check intervals

#### High Packet Loss
1. Check signal strength
2. Verify antenna connection
3. Look for interference sources
4. Consider relocating device

#### API Not Responding
1. Verify WiFi connection
2. Check API initialization
3. Review HTTP server configuration
4. Check for memory issues

### Debug Output

Enable debug output to troubleshoot issues:

```cpp
#define DEBUG_WIFI_MANAGER 1
#define DEBUG_FALLBACK 1
#define DEBUG_HEALTH_MONITOR 1
```

### Health Diagnostics

Get comprehensive diagnostic information:

```cpp
// Print diagnostic report
Serial.println(healthMonitor.getDiagnosticReport());

// Get specific metrics
Serial.printf("Uptime: %.2f%%\n", healthMonitor.getUptimePercentage());
Serial.printf("Packet Loss: %.2f%%\n", healthMonitor.getPacketLossRate() * 100);
Serial.printf("Avg Latency: %d ms\n", healthMonitor.getAverageLatency());

// Check for issues
auto issues = healthMonitor.getActiveIssues();
for (const auto& issue : issues) {
    Serial.printf("[%s] %s\n", issue.component.c_str(), 
                               issue.description.c_str());
}
```

## Best Practices

1. **Regular Health Checks**: Call `healthMonitor.updateMetrics()` at least every 10 seconds
2. **Fallback Updates**: Call `fallback.update()` in your main loop
3. **Periodic Scans**: Scan for networks every 5-10 minutes to find better options
4. **History Maintenance**: Let the system collect at least 10 samples before relying on historical data
5. **Signal Thresholds**: Adjust thresholds based on your deployment environment
6. **Auto-Recovery**: Enable auto-recovery for unattended deployments
7. **Logging**: Maintain logs of network switches and failures for analysis

## Future Enhancements

- WebSocket support for real-time metric streaming
- Machine learning-based failure prediction
- Automatic channel optimization
- Multi-gateway mesh routing
- Cloud synchronization of network preferences
- Mobile app for network management

## Support

For questions, issues, or contributions:
- GitHub Issues: [Report an Issue](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Discussions: [Join Discussion](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- Discord: [Community Chat](https://discord.gg/7cmrkFKx)

## License

This feature is part of the ESP32WildlifeCAM project and follows the same license terms.
