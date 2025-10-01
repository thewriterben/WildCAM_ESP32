# Cloud Platform Monitoring and Health Check Guide

## Overview

This guide covers comprehensive monitoring strategies for WildCAM ESP32 cloud integration, including health checks, performance metrics, alerting, and troubleshooting.

## Monitoring Architecture

```
┌─────────────────────────────────────────────────┐
│              Device Monitoring                  │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐│
│  │  Battery   │  │   Memory   │  │  Network   ││
│  │  Monitor   │  │   Monitor  │  │  Quality   ││
│  └────────────┘  └────────────┘  └────────────┘│
└────────────────────┬────────────────────────────┘
                     │ Telemetry
┌────────────────────▼────────────────────────────┐
│            Cloud Monitoring Layer               │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐│
│  │ Connection │  │    Sync    │  │  Service   ││
│  │   Health   │  │Statistics  │  │   Health   ││
│  └────────────┘  └────────────┘  └────────────┘│
└────────────────────┬────────────────────────────┘
                     │ Metrics
┌────────────────────▼────────────────────────────┐
│         Cloud Platform Dashboards               │
│  AWS CloudWatch / Azure Monitor / GCP Monitoring│
└─────────────────────────────────────────────────┘
```

## Device-Level Monitoring

### System Health Monitoring

```cpp
#include "production/enterprise/cloud_integrator.h"

// System health structure
struct SystemHealth {
    // Hardware metrics
    float batteryVoltage;
    int batteryPercent;
    float temperature;
    uint32_t freeHeap;
    uint32_t minFreeHeap;
    
    // Network metrics
    bool wifiConnected;
    int wifiRSSI;
    String ipAddress;
    uint32_t dataTransferred;
    
    // Cloud metrics
    bool cloudConnected;
    uint32_t syncQueueSize;
    uint32_t uploadsFailed;
    uint32_t uploadsSuccess;
    float successRate;
    
    // Camera metrics
    bool cameraWorking;
    uint32_t captureCount;
    uint32_t captureErrors;
    
    // Detection metrics
    uint32_t detectionsToday;
    uint32_t totalDetections;
    String lastDetection;
    uint32_t lastDetectionTime;
};

SystemHealth g_systemHealth;

void collectSystemMetrics() {
    // Hardware metrics
    g_systemHealth.batteryVoltage = analogRead(35) * 3.3 / 4095.0 * 2.0;
    g_systemHealth.batteryPercent = calculateBatteryPercent(g_systemHealth.batteryVoltage);
    g_systemHealth.temperature = readTemperature();
    g_systemHealth.freeHeap = ESP.getFreeHeap();
    g_systemHealth.minFreeHeap = ESP.getMinFreeHeap();
    
    // Network metrics
    g_systemHealth.wifiConnected = (WiFi.status() == WL_CONNECTED);
    g_systemHealth.wifiRSSI = WiFi.RSSI();
    g_systemHealth.ipAddress = WiFi.localIP().toString();
    
    // Cloud metrics
    g_systemHealth.cloudConnected = g_cloudIntegrator->isConnected();
    g_systemHealth.syncQueueSize = g_intelligentSyncManager->getQueueSize();
    
    SyncStats stats = g_cloudIntegrator->getSyncStatistics();
    g_systemHealth.uploadsFailed = stats.failedUploads;
    g_systemHealth.uploadsSuccess = stats.successfulUploads;
    g_systemHealth.successRate = stats.successRate;
}

void reportSystemHealth() {
    collectSystemMetrics();
    
    // Create health report JSON
    String healthReport = "{";
    healthReport += "\"deviceId\":\"" + WiFi.macAddress() + "\",";
    healthReport += "\"timestamp\":" + String(millis()) + ",";
    healthReport += "\"battery\":{";
    healthReport += "\"voltage\":" + String(g_systemHealth.batteryVoltage) + ",";
    healthReport += "\"percent\":" + String(g_systemHealth.batteryPercent);
    healthReport += "},";
    healthReport += "\"memory\":{";
    healthReport += "\"free\":" + String(g_systemHealth.freeHeap) + ",";
    healthReport += "\"minFree\":" + String(g_systemHealth.minFreeHeap);
    healthReport += "},";
    healthReport += "\"network\":{";
    healthReport += "\"connected\":" + String(g_systemHealth.wifiConnected ? "true" : "false") + ",";
    healthReport += "\"rssi\":" + String(g_systemHealth.wifiRSSI) + ",";
    healthReport += "\"ip\":\"" + g_systemHealth.ipAddress + "\"";
    healthReport += "},";
    healthReport += "\"cloud\":{";
    healthReport += "\"connected\":" + String(g_systemHealth.cloudConnected ? "true" : "false") + ",";
    healthReport += "\"queueSize\":" + String(g_systemHealth.syncQueueSize) + ",";
    healthReport += "\"successRate\":" + String(g_systemHealth.successRate);
    healthReport += "},";
    healthReport += "\"detections\":{";
    healthReport += "\"today\":" + String(g_systemHealth.detectionsToday) + ",";
    healthReport += "\"total\":" + String(g_systemHealth.totalDetections);
    healthReport += "}";
    healthReport += "}";
    
    // Send to cloud
    g_cloudIntegrator->reportDeviceHealth(healthReport);
    
    // Log locally
    Serial.println("\n=== System Health Report ===");
    Serial.printf("Battery: %.2fV (%d%%)\n", 
                  g_systemHealth.batteryVoltage, 
                  g_systemHealth.batteryPercent);
    Serial.printf("Memory: %d bytes free\n", g_systemHealth.freeHeap);
    Serial.printf("WiFi: %s (RSSI: %d)\n", 
                  g_systemHealth.wifiConnected ? "Connected" : "Disconnected",
                  g_systemHealth.wifiRSSI);
    Serial.printf("Cloud: %s\n", 
                  g_systemHealth.cloudConnected ? "Connected" : "Disconnected");
    Serial.printf("Sync Queue: %d items\n", g_systemHealth.syncQueueSize);
    Serial.printf("Upload Success Rate: %.1f%%\n", g_systemHealth.successRate);
}
```

### Continuous Health Monitoring

```cpp
void setupHealthMonitoring() {
    // Report health every 5 minutes
    static uint32_t lastHealthReport = 0;
    
    if (millis() - lastHealthReport > 300000) {
        reportSystemHealth();
        lastHealthReport = millis();
    }
}

void monitorCriticalMetrics() {
    // Battery level monitoring
    if (g_systemHealth.batteryPercent < 20) {
        Serial.println("⚠️ WARNING: Battery level critical!");
        sendAlert("battery_critical", "Battery below 20%");
        
        // Enable power saving mode
        enablePowerSaving();
    }
    
    // Memory monitoring
    if (g_systemHealth.freeHeap < 20000) {
        Serial.println("⚠️ WARNING: Low memory!");
        sendAlert("memory_low", "Free heap below 20KB");
        
        // Clear caches and optimize
        optimizeMemoryUsage();
    }
    
    // Connection monitoring
    if (!g_systemHealth.wifiConnected) {
        Serial.println("⚠️ WARNING: WiFi disconnected!");
        // Attempt reconnect
        WiFi.reconnect();
    }
    
    if (!g_systemHealth.cloudConnected) {
        Serial.println("⚠️ WARNING: Cloud disconnected!");
        // Attempt cloud reconnect
        g_cloudIntegrator->connect();
    }
    
    // Sync queue monitoring
    if (g_systemHealth.syncQueueSize > 100) {
        Serial.println("⚠️ WARNING: Large sync queue!");
        sendAlert("sync_queue_large", "Queue size over 100");
        
        // Increase sync frequency
        forceSyncHighPriority();
    }
    
    // Upload success rate monitoring
    if (g_systemHealth.successRate < 80.0) {
        Serial.println("⚠️ WARNING: Low upload success rate!");
        sendAlert("upload_failures", "Success rate below 80%");
    }
}
```

## Cloud-Level Monitoring

### Connection Quality Metrics

```cpp
void monitorConnectionQuality() {
    ConnectionMetrics metrics = g_intelligentSyncManager->measureConnectionQuality();
    
    Serial.println("\n=== Connection Quality ===");
    Serial.printf("Bandwidth: %.2f KB/s\n", metrics.bandwidth / 1024.0);
    Serial.printf("Latency: %d ms\n", metrics.latency);
    Serial.printf("Packet Loss: %.2f%%\n", metrics.packetLoss);
    Serial.printf("Stability: %.0f%%\n", metrics.stability * 100);
    Serial.printf("Signal Strength: %d%%\n", metrics.signalStrength);
    Serial.printf("Metered: %s\n", metrics.isMetered ? "Yes" : "No");
    
    // Adjust sync strategy based on connection quality
    if (metrics.stability < 0.5) {
        Serial.println("⚠️ Unstable connection - reducing sync frequency");
        g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_CONSERVATIVE);
    } else if (metrics.stability > 0.9) {
        Serial.println("✓ Excellent connection - enabling aggressive sync");
        g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_AGGRESSIVE);
    }
    
    // Report to cloud
    String metricsJson = "{";
    metricsJson += "\"bandwidth\":" + String(metrics.bandwidth) + ",";
    metricsJson += "\"latency\":" + String(metrics.latency) + ",";
    metricsJson += "\"packetLoss\":" + String(metrics.packetLoss) + ",";
    metricsJson += "\"stability\":" + String(metrics.stability) + ",";
    metricsJson += "\"signalStrength\":" + String(metrics.signalStrength);
    metricsJson += "}";
    
    g_cloudIntegrator->uploadAnalyticsData(metricsJson);
}
```

### Synchronization Statistics

```cpp
void monitorSyncPerformance() {
    SyncStats stats = g_cloudIntegrator->getSyncStatistics();
    
    Serial.println("\n=== Sync Performance ===");
    Serial.printf("Total Uploads: %d\n", stats.totalUploads);
    Serial.printf("Successful: %d (%.1f%%)\n", 
                  stats.successfulUploads, stats.successRate);
    Serial.printf("Failed: %d\n", stats.failedUploads);
    Serial.printf("Retries: %d\n", stats.retryUploads);
    Serial.printf("Data Uploaded: %.2f MB\n", 
                  stats.totalDataUploaded / 1048576.0);
    Serial.printf("Avg Response Time: %d ms\n", stats.averageResponseTime);
    Serial.printf("Last Sync: %d seconds ago\n", 
                  (millis() - stats.lastSyncTime) / 1000);
    
    if (!stats.lastError.isEmpty()) {
        Serial.printf("Last Error: %s\n", stats.lastError.c_str());
    }
    
    // Performance alerts
    if (stats.averageResponseTime > 5000) {
        sendAlert("slow_response", "Average response time over 5 seconds");
    }
    
    if (stats.successRate < 90.0) {
        sendAlert("low_success_rate", "Upload success rate below 90%");
    }
}
```

### Service Health Status

```cpp
void checkServiceHealth() {
    Serial.println("\n=== Service Health Status ===");
    
    // Check cloud integrator
    if (g_cloudIntegrator->isConnected()) {
        Serial.println("✓ Cloud Integrator: HEALTHY");
    } else {
        Serial.println("✗ Cloud Integrator: DISCONNECTED");
    }
    
    // Check intelligent sync manager
    if (g_intelligentSyncManager->isInitialized()) {
        Serial.println("✓ Sync Manager: HEALTHY");
        Serial.printf("  Queue: %d items\n", 
                     g_intelligentSyncManager->getQueueSize());
        Serial.printf("  Paused: %s\n", 
                     g_intelligentSyncManager->isPaused() ? "Yes" : "No");
    } else {
        Serial.println("✗ Sync Manager: NOT INITIALIZED");
    }
    
    // Check analytics engine
    if (g_cloudAnalyticsEngine->isInitialized()) {
        Serial.println("✓ Analytics Engine: HEALTHY");
        Serial.printf("  Realtime: %s\n", 
                     g_cloudAnalyticsEngine->isRealtimeEnabled() ? "Enabled" : "Disabled");
    } else {
        Serial.println("✗ Analytics Engine: NOT INITIALIZED");
    }
    
    // Check wildlife pipeline
    if (g_wildlifeCloudPipeline->isInitialized()) {
        Serial.println("✓ Wildlife Pipeline: HEALTHY");
        std::vector<ConservationAlert> alerts = 
            g_wildlifeCloudPipeline->getPendingAlerts();
        Serial.printf("  Pending Alerts: %d\n", alerts.size());
    } else {
        Serial.println("✗ Wildlife Pipeline: NOT INITIALIZED");
    }
    
    // Overall system health
    bool allHealthy = g_cloudIntegrator->isConnected() &&
                      g_intelligentSyncManager->isInitialized() &&
                      g_cloudAnalyticsEngine->isInitialized() &&
                      g_wildlifeCloudPipeline->isInitialized();
    
    if (allHealthy) {
        Serial.println("\n✅ Overall System: HEALTHY");
        g_cloudIntegrator->updateDeviceStatus("healthy");
    } else {
        Serial.println("\n⚠️ Overall System: DEGRADED");
        g_cloudIntegrator->updateDeviceStatus("degraded");
    }
}
```

## Alert System

### Alert Types and Thresholds

```cpp
enum AlertSeverity {
    SEVERITY_INFO,
    SEVERITY_WARNING,
    SEVERITY_ERROR,
    SEVERITY_CRITICAL
};

struct Alert {
    AlertSeverity severity;
    String type;
    String message;
    uint32_t timestamp;
    String deviceId;
};

void sendAlert(const String& type, const String& message, 
               AlertSeverity severity = SEVERITY_WARNING) {
    Alert alert;
    alert.severity = severity;
    alert.type = type;
    alert.message = message;
    alert.timestamp = millis();
    alert.deviceId = WiFi.macAddress();
    
    // Format alert
    String alertJson = "{";
    alertJson += "\"severity\":\"" + getSeverityString(severity) + "\",";
    alertJson += "\"type\":\"" + type + "\",";
    alertJson += "\"message\":\"" + message + "\",";
    alertJson += "\"timestamp\":" + String(alert.timestamp) + ",";
    alertJson += "\"deviceId\":\"" + alert.deviceId + "\"";
    alertJson += "}";
    
    // Send to cloud
    sendCloudEvent("device_alert", alertJson);
    
    // Log locally
    Serial.println("\n⚠️ ALERT ⚠️");
    Serial.printf("Type: %s\n", type.c_str());
    Serial.printf("Message: %s\n", message.c_str());
    Serial.printf("Severity: %s\n", getSeverityString(severity).c_str());
}

String getSeverityString(AlertSeverity severity) {
    switch (severity) {
        case SEVERITY_INFO: return "INFO";
        case SEVERITY_WARNING: return "WARNING";
        case SEVERITY_ERROR: return "ERROR";
        case SEVERITY_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}
```

## Troubleshooting Guides

### Common Issues and Solutions

#### 1. Connection Failures

```cpp
void diagnoseConnectionIssue() {
    Serial.println("\n=== Connection Diagnostics ===");
    
    // Check WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Issue: WiFi not connected");
        Serial.println("Solution: Attempting WiFi reconnect...");
        WiFi.reconnect();
        delay(5000);
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("✓ WiFi reconnected successfully");
        } else {
            Serial.println("✗ WiFi reconnection failed");
            Serial.println("Action: Check WiFi credentials and signal strength");
        }
    }
    
    // Check cloud connection
    if (!g_cloudIntegrator->isConnected()) {
        Serial.println("Issue: Cloud not connected");
        
        // Test basic connectivity
        if (g_cloudIntegrator->testConnection()) {
            Serial.println("Network reachable, checking authentication...");
            
            if (!g_cloudIntegrator->authenticateConnection()) {
                Serial.println("✗ Authentication failed");
                Serial.println("Action: Check cloud credentials");
                
                // Attempt token refresh
                if (g_cloudIntegrator->refreshAuthToken()) {
                    Serial.println("✓ Auth token refreshed, retrying connection...");
                    g_cloudIntegrator->connect();
                }
            }
        } else {
            Serial.println("✗ Network unreachable");
            Serial.println("Action: Check internet connectivity and endpoint URL");
        }
    }
}
```

#### 2. Upload Failures

```cpp
void diagnoseUploadIssues() {
    Serial.println("\n=== Upload Diagnostics ===");
    
    SyncStats stats = g_cloudIntegrator->getSyncStatistics();
    
    if (stats.failedUploads > 0) {
        Serial.printf("Failed uploads: %d\n", stats.failedUploads);
        Serial.printf("Last error: %s\n", stats.lastError.c_str());
        
        // Check queue
        uint32_t queueSize = g_cloudIntegrator->getPendingUploads();
        Serial.printf("Pending uploads: %d\n", queueSize);
        
        // Check connection quality
        ConnectionMetrics metrics = g_intelligentSyncManager->measureConnectionQuality();
        if (metrics.stability < 0.5) {
            Serial.println("Issue: Unstable connection");
            Serial.println("Solution: Reducing sync frequency");
            g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_CONSERVATIVE);
        }
        
        // Check bandwidth
        if (metrics.bandwidth < 10000) { // < 10 KB/s
            Serial.println("Issue: Low bandwidth");
            Serial.println("Solution: Enabling compression and delta sync");
            g_intelligentSyncManager->enableCompression(true);
            g_intelligentSyncManager->enableDeltaSync(true);
        }
        
        // Retry failed uploads
        Serial.println("Retrying failed uploads...");
        g_cloudIntegrator->forceSyncAll();
    }
}
```

#### 3. Memory Issues

```cpp
void diagnoseMemoryIssues() {
    Serial.println("\n=== Memory Diagnostics ===");
    
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    uint32_t heapSize = ESP.getHeapSize();
    
    Serial.printf("Free heap: %d bytes (%.1f%%)\n", 
                  freeHeap, (float)freeHeap / heapSize * 100);
    Serial.printf("Minimum free heap: %d bytes\n", minFreeHeap);
    Serial.printf("Total heap: %d bytes\n", heapSize);
    
    if (freeHeap < 30000) {
        Serial.println("⚠️ Low memory detected");
        
        // Clear sync queue
        Serial.println("Clearing sync queue...");
        g_intelligentSyncManager->clearSyncQueue();
        
        // Reduce buffer sizes
        Serial.println("Reducing buffer sizes...");
        
        // Force garbage collection
        Serial.println("Running garbage collection...");
        
        Serial.printf("Free heap after optimization: %d bytes\n", 
                     ESP.getFreeHeap());
    }
}
```

## Performance Optimization

### Bandwidth Optimization

```cpp
void optimizeBandwidthUsage() {
    ConnectionMetrics metrics = g_intelligentSyncManager->measureConnectionQuality();
    
    // Adjust based on connection quality
    if (metrics.bandwidth < 50000) { // < 50 KB/s
        // Low bandwidth - use conservative settings
        g_intelligentSyncManager->setBandwidthLimit(30000); // 30 KB/s
        g_intelligentSyncManager->enableCompression(true);
        g_intelligentSyncManager->enableDeltaSync(true);
        Serial.println("Bandwidth: Conservative mode");
    } else if (metrics.bandwidth < 200000) { // < 200 KB/s
        // Medium bandwidth - balanced settings
        g_intelligentSyncManager->setBandwidthLimit(100000); // 100 KB/s
        g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_ADAPTIVE);
        Serial.println("Bandwidth: Balanced mode");
    } else {
        // High bandwidth - aggressive sync
        g_intelligentSyncManager->setBandwidthLimit(0); // No limit
        g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_AGGRESSIVE);
        Serial.println("Bandwidth: Aggressive mode");
    }
}
```

## Dashboard Integration

### Custom Metrics for Cloud Dashboards

```cpp
void sendCustomMetrics() {
    // Wildlife detection metrics
    String detectionMetrics = "{";
    detectionMetrics += "\"namespace\":\"WildlifeCamera\",";
    detectionMetrics += "\"metrics\":[";
    detectionMetrics += "{\"name\":\"DetectionCount\",\"value\":" + String(g_systemHealth.detectionsToday) + ",\"unit\":\"Count\"},";
    detectionMetrics += "{\"name\":\"SuccessRate\",\"value\":" + String(g_systemHealth.successRate) + ",\"unit\":\"Percent\"},";
    detectionMetrics += "{\"name\":\"BatteryLevel\",\"value\":" + String(g_systemHealth.batteryPercent) + ",\"unit\":\"Percent\"},";
    detectionMetrics += "{\"name\":\"SyncQueueSize\",\"value\":" + String(g_systemHealth.syncQueueSize) + ",\"unit\":\"Count\"}";
    detectionMetrics += "]}";
    
    g_cloudIntegrator->uploadAnalyticsData(detectionMetrics);
}
```

## Conclusion

Regular monitoring ensures:
- Early detection of issues
- Optimal performance
- Reliable data collection
- Efficient resource usage
- Proactive maintenance

Implement automated health checks, set up appropriate alerts, and review metrics regularly for best results.
