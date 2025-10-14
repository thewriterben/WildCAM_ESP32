/**
 * @file wifi_lora_fallback.h
 * @brief Automatic Fallback System between WiFi and LoRa Networks
 * 
 * Manages intelligent switching between WiFi and LoRa based on
 * connectivity health, signal quality, and data priority.
 */

#ifndef WIFI_LORA_FALLBACK_H
#define WIFI_LORA_FALLBACK_H

#include <Arduino.h>
#include "wifi_manager.h"
#include "lora_mesh.h"

// Active network type
enum ActiveNetwork {
    NETWORK_NONE = 0,
    NETWORK_WIFI,
    NETWORK_LORA,
    NETWORK_BOTH,           // Redundant transmission mode
    NETWORK_SWITCHING       // In transition
};

// Fallback trigger reasons
enum FallbackReason {
    REASON_NONE = 0,
    REASON_SIGNAL_WEAK,
    REASON_CONNECTION_LOST,
    REASON_TIMEOUT,
    REASON_MANUAL,
    REASON_DATA_PRIORITY,
    REASON_COST_OPTIMIZATION
};

// Network health status
enum HealthStatus {
    HEALTH_EXCELLENT = 0,
    HEALTH_GOOD,
    HEALTH_DEGRADED,
    HEALTH_POOR,
    HEALTH_CRITICAL,
    HEALTH_OFFLINE
};

// Fallback configuration
struct FallbackConfig {
    // Health thresholds
    int wifiRSSIThreshold;          // Minimum WiFi signal strength
    int loraRSSIThreshold;          // Minimum LoRa signal strength
    uint32_t connectionTimeout;     // Max time without response (ms)
    uint32_t healthCheckInterval;   // Time between health checks (ms)
    
    // Fallback behavior
    bool autoFallbackEnabled;       // Enable automatic switching
    bool preferWiFi;                // Prefer WiFi over LoRa when both available
    bool allowDualMode;             // Allow simultaneous WiFi + LoRa
    uint32_t switchDebounceTime;    // Minimum time between switches (ms)
    
    // Reconnection strategy
    uint32_t wifiReconnectInterval; // Time between WiFi reconnect attempts
    uint32_t loraReconnectInterval; // Time between LoRa reconnect attempts
    int maxReconnectAttempts;       // Max reconnection attempts before giving up
    
    FallbackConfig() :
        wifiRSSIThreshold(-75),
        loraRSSIThreshold(-100),
        connectionTimeout(30000),
        healthCheckInterval(10000),
        autoFallbackEnabled(true),
        preferWiFi(true),
        allowDualMode(false),
        switchDebounceTime(5000),
        wifiReconnectInterval(30000),
        loraReconnectInterval(60000),
        maxReconnectAttempts(3) {}
};

// Network state information
struct NetworkState {
    ActiveNetwork activeNetwork;
    HealthStatus wifiHealth;
    HealthStatus loraHealth;
    int wifiRSSI;
    int loraRSSI;
    bool wifiConnected;
    bool loraConnected;
    unsigned long lastSwitch;
    FallbackReason lastFallbackReason;
    uint32_t wifiUptime;
    uint32_t loraUptime;
    uint32_t totalSwitches;
    
    NetworkState() :
        activeNetwork(NETWORK_NONE),
        wifiHealth(HEALTH_OFFLINE),
        loraHealth(HEALTH_OFFLINE),
        wifiRSSI(-100),
        loraRSSI(-100),
        wifiConnected(false),
        loraConnected(false),
        lastSwitch(0),
        lastFallbackReason(REASON_NONE),
        wifiUptime(0),
        loraUptime(0),
        totalSwitches(0) {}
};

// Data transmission result
struct TransmissionResult {
    bool success;
    ActiveNetwork usedNetwork;
    uint32_t transmissionTime;
    size_t bytesSent;
    String errorMessage;
    
    TransmissionResult() :
        success(false),
        usedNetwork(NETWORK_NONE),
        transmissionTime(0),
        bytesSent(0) {}
};

/**
 * @class WiFiLoRaFallback
 * @brief Manages automatic fallback between WiFi and LoRa networks
 */
class WiFiLoRaFallback {
public:
    WiFiLoRaFallback();
    ~WiFiLoRaFallback();
    
    // Initialization
    bool init(WiFiManager* wifiMgr, const FallbackConfig& config = FallbackConfig());
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Network management
    bool connect();
    void disconnect();
    void update();  // Call regularly in main loop
    
    // Health monitoring
    void performHealthCheck();
    HealthStatus getWiFiHealth() const;
    HealthStatus getLoRaHealth() const;
    HealthStatus getOverallHealth() const;
    bool isHealthy() const;
    
    // Network status
    ActiveNetwork getActiveNetwork() const;
    NetworkState getNetworkState() const;
    bool isConnected() const;
    bool hasConnectivity() const;
    int getSignalStrength() const;
    String getNetworkInfo() const;
    
    // Data transmission
    TransmissionResult sendData(const uint8_t* data, size_t length);
    TransmissionResult sendDataPriority(const uint8_t* data, size_t length, 
                                       bool highPriority);
    bool sendViaWiFi(const uint8_t* data, size_t length);
    bool sendViaLoRa(const uint8_t* data, size_t length);
    bool sendViaBoth(const uint8_t* data, size_t length); // Redundant mode
    
    // Network switching
    bool switchToWiFi(FallbackReason reason = REASON_MANUAL);
    bool switchToLoRa(FallbackReason reason = REASON_MANUAL);
    bool switchToBoth();
    bool attemptReconnect();
    
    // Configuration
    void setConfig(const FallbackConfig& config);
    FallbackConfig getConfig() const;
    void setAutoFallback(bool enabled);
    void setPreferWiFi(bool prefer);
    void setDualMode(bool enabled);
    
    // Statistics
    uint32_t getTotalSwitches() const;
    uint32_t getWiFiUptime() const;
    uint32_t getLoRaUptime() const;
    float getWiFiReliability() const;
    float getLoRaReliability() const;
    void resetStatistics();
    
    // Diagnostics
    void printStatus() const;
    String getDiagnostics() const;
    
private:
    // Component references
    WiFiManager* wifiManager_;
    
    // Configuration
    FallbackConfig config_;
    NetworkState state_;
    
    // Internal state
    bool initialized_;
    unsigned long lastHealthCheck_;
    unsigned long lastSwitchAttempt_;
    unsigned long connectionStartTime_;
    int wifiReconnectAttempts_;
    int loraReconnectAttempts_;
    
    // Statistics tracking
    uint32_t wifiSuccessCount_;
    uint32_t wifiFailCount_;
    uint32_t loraSuccessCount_;
    uint32_t loraFailCount_;
    
    // Internal methods
    HealthStatus assessWiFiHealth();
    HealthStatus assessLoRaHealth();
    bool shouldSwitchToLoRa();
    bool shouldSwitchToWiFi();
    bool canSwitch() const;
    void updateActiveNetwork();
    void recordTransmission(ActiveNetwork network, bool success);
    void updateUptime();
    String healthToString(HealthStatus health) const;
    String networkToString(ActiveNetwork network) const;
    String reasonToString(FallbackReason reason) const;
};

#endif // WIFI_LORA_FALLBACK_H
