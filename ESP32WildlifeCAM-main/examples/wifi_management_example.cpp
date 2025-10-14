/**
 * @file wifi_management_example.cpp
 * @brief Example demonstrating intelligent WiFi management and fallback systems
 * 
 * This example shows how to use the WiFi AP Selector, WiFi-LoRa Fallback,
 * and Network Health Monitor for robust field deployments.
 */

#include <Arduino.h>
#include "../firmware/src/wifi_manager.h"
#include "../firmware/src/wifi_ap_selector.h"
#include "../firmware/src/wifi_lora_fallback.h"
#include "../firmware/src/network_health_monitor.h"
#include "../firmware/src/lora_mesh.h"

// Configuration
const char* KNOWN_NETWORKS[] = {
    "FieldStation_Primary",
    "FieldStation_Backup",
    "RemoteHub_WiFi"
};
const int NUM_KNOWN_NETWORKS = 3;

// Component instances
WiFiManager wifiManager;
WiFiAPSelector apSelector;
WiFiLoRaFallback fallbackSystem;
NetworkHealthMonitor healthMonitor;

// Timing
unsigned long lastHealthUpdate = 0;
unsigned long lastDataTransmission = 0;
const unsigned long HEALTH_UPDATE_INTERVAL = 10000;  // 10 seconds
const unsigned long DATA_INTERVAL = 60000;           // 1 minute

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== WiFi Management System Example ===\n");
    
    // Initialize WiFi manager
    Serial.println("1. Initializing WiFi Manager...");
    if (!wifiManager.init()) {
        Serial.println("   ERROR: WiFi Manager initialization failed!");
        return;
    }
    Serial.println("   WiFi Manager initialized");
    
    // Initialize AP selector
    Serial.println("\n2. Initializing WiFi AP Selector...");
    if (!apSelector.init()) {
        Serial.println("   ERROR: AP Selector initialization failed!");
        return;
    }
    Serial.println("   AP Selector initialized");
    
    // Scan for available networks
    Serial.println("\n3. Scanning for WiFi networks...");
    if (apSelector.scanNetworks()) {
        Serial.printf("   Found %d networks\n", apSelector.getScannedNetworkCount());
        apSelector.printAvailableNetworks();
    }
    
    // Select best network from known list
    Serial.println("\n4. Selecting best network...");
    std::vector<String> knownNetworks;
    for (int i = 0; i < NUM_KNOWN_NETWORKS; i++) {
        knownNetworks.push_back(KNOWN_NETWORKS[i]);
    }
    
    SelectionCriteria criteria;
    criteria.minRSSI = -75;
    criteria.preferStrongerSignal = true;
    criteria.considerHistory = true;
    
    String bestSSID = apSelector.selectBestSSID(knownNetworks, criteria);
    if (bestSSID.isEmpty()) {
        Serial.println("   No suitable network found");
    } else {
        Serial.printf("   Selected network: %s\n", bestSSID.c_str());
        
        // Get network metrics
        for (int i = 0; i < apSelector.getScannedNetworkCount(); i++) {
            APInfo ap = apSelector.getNetworkInfo(i);
            if (ap.ssid == bestSSID) {
                NetworkMetrics metrics = apSelector.calculateMetrics(ap);
                Serial.printf("   Signal Quality: %.2f\n", metrics.signalQuality);
                Serial.printf("   Performance Score: %.2f\n", metrics.performanceScore);
                Serial.printf("   Est. Throughput: %d Kbps\n", metrics.estimatedThroughput);
                break;
            }
        }
    }
    
    // Initialize fallback system
    Serial.println("\n5. Initializing WiFi-LoRa Fallback System...");
    FallbackConfig fallbackConfig;
    fallbackConfig.autoFallbackEnabled = true;
    fallbackConfig.preferWiFi = true;
    fallbackConfig.wifiRSSIThreshold = -75;
    fallbackConfig.healthCheckInterval = 10000;
    
    if (!fallbackSystem.init(&wifiManager, fallbackConfig)) {
        Serial.println("   ERROR: Fallback system initialization failed!");
        return;
    }
    Serial.println("   Fallback system initialized");
    
    // Connect to network
    Serial.println("\n6. Connecting to network...");
    if (fallbackSystem.connect()) {
        Serial.println("   Connected successfully");
        Serial.println(fallbackSystem.getNetworkInfo());
    } else {
        Serial.println("   Connection failed");
    }
    
    // Initialize health monitor
    Serial.println("\n7. Initializing Network Health Monitor...");
    MonitorConfig monitorConfig;
    monitorConfig.updateInterval = 10000;
    monitorConfig.enablePredictiveAnalysis = true;
    monitorConfig.packetLossThreshold = 0.1f;
    monitorConfig.latencyThreshold = 1000;
    
    if (!healthMonitor.init(monitorConfig)) {
        Serial.println("   ERROR: Health monitor initialization failed!");
        return;
    }
    Serial.println("   Health monitor initialized");
    
    Serial.println("\n=== System Ready ===\n");
}

void loop() {
    unsigned long now = millis();
    
    // Update fallback system
    fallbackSystem.update();
    
    // Update health metrics
    if (now - lastHealthUpdate >= HEALTH_UPDATE_INTERVAL) {
        healthMonitor.updateMetrics();
        
        // Record signal strength
        int rssi = fallbackSystem.getSignalStrength();
        healthMonitor.recordSignalStrength(rssi);
        
        // Check overall health
        if (healthMonitor.isHealthy()) {
            Serial.println("Network Health: Good");
        } else {
            Serial.println("Network Health: Issues detected");
            healthMonitor.printIssues();
            
            // Get recommendations
            auto recommendations = healthMonitor.getDiagnosticRecommendations();
            if (!recommendations.empty()) {
                Serial.println("\nRecommendations:");
                for (const auto& rec : recommendations) {
                    Serial.println("  - " + rec);
                }
            }
        }
        
        // Print health summary
        Serial.println("\n" + healthMonitor.getMetricsSummary());
        
        lastHealthUpdate = now;
    }
    
    // Simulate data transmission
    if (now - lastDataTransmission >= DATA_INTERVAL) {
        Serial.println("\n--- Sending Data ---");
        
        // Create sample data
        String data = "Sample telemetry data from camera";
        const uint8_t* dataBytes = (const uint8_t*)data.c_str();
        size_t dataLength = data.length();
        
        unsigned long txStart = millis();
        TransmissionResult result = fallbackSystem.sendData(dataBytes, dataLength);
        unsigned long txTime = millis() - txStart;
        
        // Record transmission in health monitor
        healthMonitor.recordTransmission(result.success, dataLength, txTime);
        
        if (result.success) {
            Serial.printf("Transmission successful via %s\n", 
                         result.usedNetwork == NETWORK_WIFI ? "WiFi" :
                         result.usedNetwork == NETWORK_LORA ? "LoRa" : "Unknown");
            Serial.printf("Bytes sent: %d\n", result.bytesSent);
            Serial.printf("Time: %d ms\n", result.transmissionTime);
            
            // Record successful connection for AP selector
            if (result.usedNetwork == NETWORK_WIFI) {
                String ssid = wifiManager.getStatus().ssid;
                apSelector.recordConnectionSuccess(ssid, "");
            }
        } else {
            Serial.println("Transmission failed: " + result.errorMessage);
            
            // Record failure
            if (result.usedNetwork == NETWORK_WIFI) {
                String ssid = wifiManager.getStatus().ssid;
                apSelector.recordConnectionFailure(ssid, "");
            }
            
            // Try fallback if not already attempted
            Serial.println("Attempting fallback...");
            if (fallbackSystem.getActiveNetwork() == NETWORK_WIFI) {
                fallbackSystem.switchToLoRa(REASON_CONNECTION_LOST);
            } else {
                fallbackSystem.switchToWiFi(REASON_CONNECTION_LOST);
            }
        }
        
        lastDataTransmission = now;
    }
    
    // Check for network alerts
    auto alerts = healthMonitor.getActiveAlerts();
    if (!alerts.empty()) {
        Serial.println("\n=== ACTIVE ALERTS ===");
        for (size_t i = 0; i < alerts.size(); i++) {
            const auto& alert = alerts[i];
            Serial.printf("[%s] %s: %s\n",
                         alert.severity == SEVERITY_CRITICAL ? "CRITICAL" :
                         alert.severity == SEVERITY_ERROR ? "ERROR" :
                         alert.severity == SEVERITY_WARNING ? "WARNING" : "INFO",
                         alert.alertType.c_str(),
                         alert.message.c_str());
        }
        Serial.println("====================\n");
        
        // Auto-acknowledge informational alerts
        for (size_t i = 0; i < alerts.size(); i++) {
            if (alerts[i].severity == SEVERITY_INFO) {
                healthMonitor.acknowledgeAlert(i);
            }
        }
    }
    
    // Periodic network scan and optimization
    static unsigned long lastScan = 0;
    if (now - lastScan >= 300000) { // Every 5 minutes
        Serial.println("\n--- Periodic Network Scan ---");
        
        if (apSelector.scanNetworks()) {
            Serial.printf("Found %d networks\n", apSelector.getScannedNetworkCount());
            
            // Check if current network is still optimal
            std::vector<String> knownNetworks;
            for (int i = 0; i < NUM_KNOWN_NETWORKS; i++) {
                knownNetworks.push_back(KNOWN_NETWORKS[i]);
            }
            
            String currentSSID = wifiManager.getStatus().ssid;
            String newBestSSID = apSelector.selectBestSSID(knownNetworks);
            
            if (!newBestSSID.isEmpty() && newBestSSID != currentSSID) {
                Serial.printf("Better network found: %s\n", newBestSSID.c_str());
                Serial.println("Consider switching networks for better performance");
            }
        }
        
        lastScan = now;
    }
    
    // Print status summary periodically
    static unsigned long lastStatus = 0;
    if (now - lastStatus >= 60000) { // Every minute
        Serial.println("\n=== Status Summary ===");
        fallbackSystem.printStatus();
        Serial.println(healthMonitor.getDiagnosticReport());
        Serial.println("=====================\n");
        
        lastStatus = now;
    }
    
    delay(100);
}

void printNetworkInfo() {
    Serial.println("\n=== Network Information ===");
    
    // WiFi info
    if (wifiManager.isConnected()) {
        WiFiStatusInfo status = wifiManager.getStatus();
        Serial.println("WiFi:");
        Serial.printf("  SSID: %s\n", status.ssid.c_str());
        Serial.printf("  IP: %s\n", status.ipAddress.c_str());
        Serial.printf("  RSSI: %d dBm\n", status.rssi);
    } else {
        Serial.println("WiFi: Not connected");
    }
    
    // LoRa info
    MeshNetworkStatus loraStatus = LoraMesh::getNetworkStatus();
    Serial.println("LoRa:");
    Serial.printf("  Initialized: %s\n", loraStatus.initialized ? "Yes" : "No");
    Serial.printf("  Node ID: %d\n", loraStatus.nodeId);
    Serial.printf("  Connected Nodes: %d\n", loraStatus.connectedNodes);
    Serial.printf("  RSSI: %d dBm\n", loraStatus.rssi);
    Serial.printf("  SNR: %.2f\n", loraStatus.snr);
    
    // Fallback system
    Serial.println("Active Network: " + 
        String(fallbackSystem.getActiveNetwork() == NETWORK_WIFI ? "WiFi" :
               fallbackSystem.getActiveNetwork() == NETWORK_LORA ? "LoRa" : "None"));
    
    Serial.println("=========================\n");
}

void demonstrateManualFallback() {
    Serial.println("\n=== Manual Fallback Demo ===");
    
    Serial.println("Switching to LoRa...");
    if (fallbackSystem.switchToLoRa(REASON_MANUAL)) {
        Serial.println("Switched to LoRa successfully");
        delay(5000);
    }
    
    Serial.println("Switching to WiFi...");
    if (fallbackSystem.switchToWiFi(REASON_MANUAL)) {
        Serial.println("Switched to WiFi successfully");
        delay(5000);
    }
    
    Serial.println("Enabling dual mode...");
    if (fallbackSystem.switchToBoth()) {
        Serial.println("Dual mode enabled");
        delay(5000);
    }
    
    Serial.println("===========================\n");
}
