/**
 * @file satellite_network_integration_example.cpp
 * @brief Example demonstrating seamless integration between satellite, WiFi, mesh, and cellular networks
 * 
 * This example shows:
 * - Automatic network selection based on availability and priority
 * - Seamless fallback between WiFi, mesh, cellular, and satellite
 * - Emergency communication protocols with satellite backup
 * - Low-power satellite operations
 */

#include <Arduino.h>
#include <WiFi.h>

// Include network components
#include "network_selector.h"
#include "satellite_comm.h"
#include "satellite_integration.h"
#include "cellular_manager.h"

// Global objects
NetworkSelector networkSelector;
SatelliteComm satelliteComm;
CellularManager cellularManager;

// Configuration
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Timing
unsigned long lastNetworkCheck = 0;
const unsigned long NETWORK_CHECK_INTERVAL = 30000; // 30 seconds

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Satellite Network Integration Example ===");
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi initialized in station mode");
    
    // Configure satellite communication
    SatelliteConfig satConfig;
    satConfig.enabled = true;
    satConfig.module = MODULE_SWARM; // Use Swarm for lower cost
    satConfig.transmissionInterval = 3600;
    satConfig.maxDailyMessages = 24;
    satConfig.maxDailyCost = 25.0;
    satConfig.costOptimization = true;
    satConfig.prioritizeEmergency = true;
    
    if (satelliteComm.configure(satConfig)) {
        Serial.println("Satellite communication configured");
        
        // Initialize satellite modem
        if (satelliteComm.initialize(satConfig.module)) {
            Serial.println("Satellite modem initialized successfully");
        } else {
            Serial.println("Warning: Satellite modem initialization failed");
        }
    }
    
    // Initialize cellular (if available)
    if (cellularManager.initialize()) {
        Serial.println("Cellular manager initialized");
    }
    
    // Initialize network selector with all available networks
    if (networkSelector.initialize(&cellularManager, &satelliteComm)) {
        Serial.println("Network selector initialized");
    }
    
    // Enable cost optimization
    networkSelector.setCostOptimization(true);
    
    Serial.println("\n=== Network Integration Ready ===");
    demonstrateNetworkPriority();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Periodic network health check
    if (currentTime - lastNetworkCheck > NETWORK_CHECK_INTERVAL) {
        performNetworkHealthCheck();
        lastNetworkCheck = currentTime;
    }
    
    // Demonstrate different scenarios every minute
    static unsigned long lastDemo = 0;
    if (currentTime - lastDemo > 60000) {
        demonstrateScenarios();
        lastDemo = currentTime;
    }
    
    delay(1000);
}

void performNetworkHealthCheck() {
    Serial.println("\n--- Network Health Check ---");
    
    // Scan available networks
    networkSelector.scanAvailableNetworks();
    
    // Get and display available networks
    std::vector<NetworkInfo> networks = networkSelector.getAvailableNetworks();
    
    Serial.printf("Available Networks: %d\n", networks.size());
    for (const auto& network : networks) {
        Serial.printf("  - %s: Signal=%d%%, Cost=$%.2f, Power=%s, Available=%s\n",
                     networkSelector.getNetworkTypeName(network.type),
                     network.signalStrength,
                     network.estimatedCost,
                     getPowerName(network.powerConsumption),
                     network.available ? "YES" : "NO");
    }
    
    // Check current network
    NetworkType current = networkSelector.getCurrentNetwork();
    Serial.printf("Current Network: %s\n", networkSelector.getNetworkTypeName(current));
    
    // Satellite-specific status
    if (satelliteComm.checkSatelliteAvailability()) {
        Serial.printf("Satellite Status:\n");
        Serial.printf("  Signal Quality: %d%%\n", satelliteComm.checkSignalQuality());
        Serial.printf("  Messages Today: %d\n", satelliteComm.getStoredMessageCount());
        Serial.printf("  Cost Today: $%.2f\n", satelliteComm.getTodayCost());
        Serial.printf("  Low Power Mode: %s\n", satelliteComm.isInLowPowerMode() ? "YES" : "NO");
    }
}

void demonstrateNetworkPriority() {
    Serial.println("\n=== Network Priority Demonstration ===");
    
    // Scenario 1: WiFi Available (Highest Priority)
    Serial.println("\nScenario 1: WiFi Available");
    Serial.println("Expected: WiFi selected (free, reliable)");
    NetworkType selected = networkSelector.selectOptimalNetwork(1024, PRIORITY_NORMAL);
    Serial.printf("Selected: %s\n", networkSelector.getNetworkTypeName(selected));
    
    // Scenario 2: Small Message
    Serial.println("\nScenario 2: Small Message (100 bytes)");
    Serial.println("Expected: WiFi or LoRa (free options preferred)");
    selected = networkSelector.selectOptimalNetwork(100, PRIORITY_NORMAL);
    Serial.printf("Selected: %s\n", networkSelector.getNetworkTypeName(selected));
    
    // Scenario 3: Large Message
    Serial.println("\nScenario 3: Large Message (1MB)");
    Serial.println("Expected: WiFi or Cellular (satellite not good for large data)");
    selected = networkSelector.selectOptimalNetwork(1024 * 1024, PRIORITY_NORMAL);
    Serial.printf("Selected: %s\n", networkSelector.getNetworkTypeName(selected));
    
    // Scenario 4: Emergency Message
    Serial.println("\nScenario 4: Emergency Message");
    Serial.println("Expected: Satellite (most reliable in remote areas)");
    selected = networkSelector.selectOptimalNetwork(256, PRIORITY_EMERGENCY);
    Serial.printf("Selected: %s\n", networkSelector.getNetworkTypeName(selected));
}

void demonstrateScenarios() {
    Serial.println("\n=== Real-World Scenario Demonstration ===");
    
    // Scenario: Wildlife Detection Alert
    demonstrateWildlifeAlert();
    
    // Scenario: Status Update
    demonstrateStatusUpdate();
    
    // Scenario: Emergency Condition
    demonstrateEmergencyAlert();
}

void demonstrateWildlifeAlert() {
    Serial.println("\n--- Wildlife Detection Alert ---");
    
    // Simulate wildlife detection data
    String alertMessage = "WILDLIFE:DEER,CONF:0.87,LOC:N47.123W122.456,TIME:" + String(millis());
    
    // Send via optimal network
    bool success = networkSelector.sendData((const uint8_t*)alertMessage.c_str(), 
                                           alertMessage.length(), 
                                           PRIORITY_HIGH);
    
    if (success) {
        Serial.println("Wildlife alert sent successfully");
    } else {
        Serial.println("Wildlife alert failed - will retry with fallback");
    }
}

void demonstrateStatusUpdate() {
    Serial.println("\n--- Periodic Status Update ---");
    
    // Create status message
    String statusMessage = "STATUS:BAT=85%,TEMP=22C,IMG_COUNT=42,UPTIME=" + String(millis()/1000);
    
    // Send via optimal network (low priority - can wait)
    bool success = networkSelector.sendData((const uint8_t*)statusMessage.c_str(),
                                           statusMessage.length(),
                                           PRIORITY_LOW);
    
    if (success) {
        Serial.println("Status update sent successfully");
    } else {
        Serial.println("Status update deferred - will send later");
    }
}

void demonstrateEmergencyAlert() {
    Serial.println("\n--- Emergency Alert (Low Battery) ---");
    
    // Simulate emergency condition
    String emergencyMessage = "EMERGENCY:LOW_BATTERY:12%,NEED_REPLACEMENT";
    
    // Enable emergency mode on satellite
    satelliteComm.enableEmergencyMode();
    
    // Send via emergency protocol (will try all networks)
    bool success = networkSelector.sendData((const uint8_t*)emergencyMessage.c_str(),
                                           emergencyMessage.length(),
                                           PRIORITY_EMERGENCY);
    
    if (success) {
        Serial.println("Emergency alert sent via fallback system");
    } else {
        Serial.println("CRITICAL: Emergency alert failed on all networks");
    }
    
    // Disable emergency mode
    satelliteComm.disableEmergencyMode();
}

const char* getPowerName(PowerConsumption power) {
    switch (power) {
        case POWER_LOW: return "LOW";
        case POWER_MEDIUM: return "MEDIUM";
        case POWER_HIGH: return "HIGH";
        default: return "UNKNOWN";
    }
}

// Advanced Integration Examples

void demonstrateIntelligentFallback() {
    Serial.println("\n=== Intelligent Network Fallback ===");
    
    // Simulate WiFi failure
    Serial.println("Simulating WiFi failure...");
    
    // Network selector will automatically try next best option
    String message = "TEST:Fallback Test Message";
    bool success = networkSelector.sendData((const uint8_t*)message.c_str(),
                                           message.length(),
                                           PRIORITY_NORMAL);
    
    if (success) {
        NetworkType used = networkSelector.getCurrentNetwork();
        Serial.printf("Message sent via fallback: %s\n", 
                     networkSelector.getNetworkTypeName(used));
    }
}

void demonstrateCostOptimization() {
    Serial.println("\n=== Cost Optimization Example ===");
    
    // Get current satellite cost
    float currentCost = satelliteComm.getTodayCost();
    Serial.printf("Current daily cost: $%.2f\n", currentCost);
    
    // Check if within budget
    if (satelliteComm.isWithinCostLimit()) {
        Serial.println("Within cost limit - satellite available");
    } else {
        Serial.println("Cost limit reached - satellite disabled for non-emergency");
    }
    
    // Demonstrate scheduled transmission
    TransmissionWindow window = satelliteComm.getOptimalTransmissionWindow();
    Serial.printf("Optimal transmission window: %d\n", window);
    
    if (window == WINDOW_IMMEDIATE) {
        Serial.println("Send now - satellite pass available");
    } else if (window == WINDOW_NEXT_PASS) {
        time_t nextPass = satelliteComm.getNextPassTime();
        Serial.printf("Wait for next pass at: %ld\n", nextPass);
    }
}

void demonstratePowerOptimization() {
    Serial.println("\n=== Power Optimization Example ===");
    
    // Simulate battery levels
    uint8_t batteryLevels[] = {90, 50, 20, 10};
    
    for (uint8_t level : batteryLevels) {
        Serial.printf("\nBattery Level: %d%%\n", level);
        
        // Optimize satellite power consumption
        satelliteComm.optimizePowerConsumption(level);
        
        if (satelliteComm.isInLowPowerMode()) {
            Serial.println("  Satellite: LOW POWER MODE");
            Serial.println("  - Reduced transmission frequency");
            Serial.println("  - Messages queued for optimal windows");
        } else {
            Serial.println("  Satellite: NORMAL MODE");
        }
        
        // Network selector will automatically prefer low-power networks
        if (level < 20) {
            Serial.println("  Network Priority: WiFi/LoRa only (power conservation)");
        } else if (level < 50) {
            Serial.println("  Network Priority: WiFi > LoRa > Cellular");
        } else {
            Serial.println("  Network Priority: Normal operation");
        }
    }
}
