/**
 * @file connectivity_orchestrator.h
 * @brief Network Connectivity Orchestrator for WildCAM ESP32
 * 
 * Manages automatic failover between WiFi and Cellular networks,
 * ensuring continuous cloud connectivity for wildlife monitoring.
 * 
 * Features:
 * - Automatic WiFi to Cellular fallback
 * - Network quality monitoring
 * - Cost-aware data transmission
 * - Connection health tracking
 * - Bandwidth optimization
 */

#ifndef CONNECTIVITY_ORCHESTRATOR_H
#define CONNECTIVITY_ORCHESTRATOR_H

#include <Arduino.h>
#include "ESP32WildlifeCAM-main/firmware/src/wifi_manager.h"
#include "ESP32WildlifeCAM-main/firmware/src/cellular_manager.h"

// Connection types
enum ConnectionType {
    CONN_NONE = 0,
    CONN_WIFI,
    CONN_CELLULAR,
    CONN_WIFI_FALLBACK_PENDING,
    CONN_CELLULAR_FALLBACK_PENDING
};

// Connection status
enum ConnectionStatus {
    STATUS_DISCONNECTED = 0,
    STATUS_CONNECTING,
    STATUS_CONNECTED,
    STATUS_DEGRADED,
    STATUS_FAILED
};

// Network quality levels
enum NetworkQuality {
    QUALITY_EXCELLENT = 0,
    QUALITY_GOOD,
    QUALITY_FAIR,
    QUALITY_POOR,
    QUALITY_NONE
};

// Configuration structure
struct ConnectivityConfig {
    // WiFi settings
    bool enableWiFi;
    String wifiSSID;
    String wifiPassword;
    int wifiRetryAttempts;
    int wifiTimeout;
    
    // Cellular settings
    bool enableCellular;
    bool cellularAsFallbackOnly;
    int cellularRetryAttempts;
    int cellularTimeout;
    
    // Orchestration settings
    bool autoFallback;
    int fallbackThreshold;        // Signal strength threshold for fallback
    int healthCheckInterval;      // ms between health checks
    int reconnectInterval;        // ms between reconnection attempts
    
    // Cost management
    bool enableCostOptimization;
    size_t cellularDailyDataLimit; // MB per day
    bool preferWiFiForLargeUploads;
    
    ConnectivityConfig() :
        enableWiFi(true),
        wifiSSID(""),
        wifiPassword(""),
        wifiRetryAttempts(3),
        wifiTimeout(30000),
        enableCellular(true),
        cellularAsFallbackOnly(true),
        cellularRetryAttempts(2),
        cellularTimeout(60000),
        autoFallback(true),
        fallbackThreshold(-75),
        healthCheckInterval(30000),
        reconnectInterval(60000),
        enableCostOptimization(true),
        cellularDailyDataLimit(100),
        preferWiFiForLargeUploads(true) {}
};

// Connection statistics
struct ConnectionStats {
    unsigned long totalUptime;
    unsigned long wifiUptime;
    unsigned long cellularUptime;
    uint32_t wifiReconnects;
    uint32_t cellularReconnects;
    uint32_t automaticFallbacks;
    uint32_t dataTransferredWiFi;    // KB
    uint32_t dataTransferredCellular; // KB
    uint32_t failedConnections;
    
    ConnectionStats() : 
        totalUptime(0), wifiUptime(0), cellularUptime(0),
        wifiReconnects(0), cellularReconnects(0), automaticFallbacks(0),
        dataTransferredWiFi(0), dataTransferredCellular(0), failedConnections(0) {}
};

/**
 * @class ConnectivityOrchestrator
 * @brief Orchestrates WiFi and Cellular connectivity with automatic failover
 */
class ConnectivityOrchestrator {
public:
    ConnectivityOrchestrator();
    ~ConnectivityOrchestrator();
    
    // Initialization
    bool initialize(const ConnectivityConfig& config);
    void cleanup();
    
    // Connection management
    bool connect();
    void disconnect();
    bool reconnect();
    void update();  // Call in main loop
    
    // Status queries
    bool isConnected() const;
    ConnectionType getActiveConnection() const;
    ConnectionStatus getConnectionStatus() const;
    NetworkQuality getNetworkQuality() const;
    int getSignalStrength() const;
    String getConnectionInfo() const;
    
    // Network operations
    bool sendData(const uint8_t* data, size_t length);
    bool sendDataOptimized(const uint8_t* data, size_t length, bool isLargeUpload = false);
    
    // Health monitoring
    void performHealthCheck();
    bool isHealthy() const;
    float getConnectionReliability() const; // 0.0 - 1.0
    
    // Statistics
    ConnectionStats getStatistics() const;
    void resetStatistics();
    
    // Configuration
    void setConfig(const ConnectivityConfig& config);
    ConnectivityConfig getConfig() const;
    void enableAutoFallback(bool enable);
    void setCellularDataLimit(size_t limitMB);
    
    // Manual control
    bool forceConnectionType(ConnectionType type);
    void preferWiFi();
    void preferCellular();
    
private:
    // Managers
    WiFiManager* wifiManager_;
    CellularManager* cellularManager_;
    
    // Configuration
    ConnectivityConfig config_;
    
    // State
    ConnectionType activeConnection_;
    ConnectionStatus currentStatus_;
    NetworkQuality currentQuality_;
    bool initialized_;
    
    // Timing
    unsigned long lastHealthCheck_;
    unsigned long lastReconnectAttempt_;
    unsigned long connectionStartTime_;
    unsigned long lastStatusChange_;
    
    // Statistics
    ConnectionStats stats_;
    
    // Internal methods
    bool connectWiFi();
    bool connectCellular();
    bool attemptFallback();
    void updateNetworkQuality();
    bool shouldFallbackToCellular();
    bool shouldFallbackToWiFi();
    void handleConnectionFailure();
    bool isCellularDataLimitReached() const;
    void trackDataUsage(size_t bytes);
    void logConnectionChange(ConnectionType from, ConnectionType to);
};

// Global instance for easy access
extern ConnectivityOrchestrator* g_connectivityOrchestrator;

// Helper functions
bool initializeConnectivity(const ConnectivityConfig& config);
bool ensureConnected();
bool sendToCloud(const uint8_t* data, size_t length);
ConnectionType getCurrentConnection();
bool isCloudConnected();

#endif // CONNECTIVITY_ORCHESTRATOR_H
