/**
 * @file connectivity_orchestrator.cpp
 * @brief Implementation of Network Connectivity Orchestrator
 */

#include "connectivity_orchestrator.h"

// Constructor
ConnectivityOrchestrator::ConnectivityOrchestrator() 
    : wifiManager_(nullptr)
    , cellularManager_(nullptr)
    , activeConnection_(CONN_NONE)
    , currentStatus_(STATUS_DISCONNECTED)
    , currentQuality_(QUALITY_NONE)
    , initialized_(false)
    , lastHealthCheck_(0)
    , lastReconnectAttempt_(0)
    , connectionStartTime_(0)
    , lastStatusChange_(0)
{
}

// Destructor
ConnectivityOrchestrator::~ConnectivityOrchestrator() {
    cleanup();
}

// Initialize the orchestrator
bool ConnectivityOrchestrator::initialize(const ConnectivityConfig& config) {
    if (initialized_) {
        Serial.println("[ConnOrch] Already initialized");
        return true;
    }
    
    Serial.println("[ConnOrch] Initializing connectivity orchestrator...");
    config_ = config;
    
    // Initialize WiFi manager if enabled
    if (config_.enableWiFi) {
        wifiManager_ = new WiFiManager();
        if (!wifiManager_->init()) {
            Serial.println("[ConnOrch] WARNING: WiFi initialization failed");
            delete wifiManager_;
            wifiManager_ = nullptr;
        } else {
            Serial.println("[ConnOrch] WiFi manager initialized");
        }
    }
    
    // Initialize Cellular manager if enabled
    if (config_.enableCellular) {
        cellularManager_ = new CellularManager();
        if (!cellularManager_->initialize()) {
            Serial.println("[ConnOrch] WARNING: Cellular initialization failed");
            delete cellularManager_;
            cellularManager_ = nullptr;
        } else {
            Serial.println("[ConnOrch] Cellular manager initialized");
        }
    }
    
    if (!wifiManager_ && !cellularManager_) {
        Serial.println("[ConnOrch] ERROR: No connectivity options available");
        return false;
    }
    
    initialized_ = true;
    Serial.println("[ConnOrch] Initialization complete");
    return true;
}

// Cleanup resources
void ConnectivityOrchestrator::cleanup() {
    if (wifiManager_) {
        wifiManager_->disconnect();
        delete wifiManager_;
        wifiManager_ = nullptr;
    }
    
    if (cellularManager_) {
        delete cellularManager_;
        cellularManager_ = nullptr;
    }
    
    initialized_ = false;
    activeConnection_ = CONN_NONE;
    currentStatus_ = STATUS_DISCONNECTED;
}

// Connect to network (tries WiFi first, then cellular if enabled)
bool ConnectivityOrchestrator::connect() {
    if (!initialized_) {
        Serial.println("[ConnOrch] Not initialized");
        return false;
    }
    
    Serial.println("[ConnOrch] Attempting connection...");
    
    // Try WiFi first if enabled
    if (config_.enableWiFi && wifiManager_) {
        if (connectWiFi()) {
            return true;
        }
    }
    
    // Fallback to cellular if WiFi failed
    if (config_.enableCellular && cellularManager_) {
        if (config_.cellularAsFallbackOnly && config_.enableWiFi) {
            Serial.println("[ConnOrch] WiFi failed, falling back to cellular");
            stats_.automaticFallbacks++;
        }
        if (connectCellular()) {
            return true;
        }
    }
    
    currentStatus_ = STATUS_FAILED;
    stats_.failedConnections++;
    Serial.println("[ConnOrch] All connection attempts failed");
    return false;
}

// Connect to WiFi
bool ConnectivityOrchestrator::connectWiFi() {
    if (!wifiManager_) return false;
    
    Serial.println("[ConnOrch] Connecting to WiFi...");
    currentStatus_ = STATUS_CONNECTING;
    
    bool success = false;
    for (int i = 0; i < config_.wifiRetryAttempts && !success; i++) {
        if (i > 0) {
            Serial.printf("[ConnOrch] WiFi retry attempt %d/%d\n", i + 1, config_.wifiRetryAttempts);
            delay(1000);
        }
        success = wifiManager_->connect(config_.wifiSSID, config_.wifiPassword);
    }
    
    if (success) {
        activeConnection_ = CONN_WIFI;
        currentStatus_ = STATUS_CONNECTED;
        connectionStartTime_ = millis();
        lastStatusChange_ = millis();
        updateNetworkQuality();
        Serial.println("[ConnOrch] WiFi connected successfully");
        logConnectionChange(CONN_NONE, CONN_WIFI);
        return true;
    }
    
    Serial.println("[ConnOrch] WiFi connection failed");
    return false;
}

// Connect to Cellular
bool ConnectivityOrchestrator::connectCellular() {
    if (!cellularManager_) return false;
    
    Serial.println("[ConnOrch] Connecting to cellular network...");
    currentStatus_ = STATUS_CONNECTING;
    
    bool success = false;
    for (int i = 0; i < config_.cellularRetryAttempts && !success; i++) {
        if (i > 0) {
            Serial.printf("[ConnOrch] Cellular retry attempt %d/%d\n", i + 1, config_.cellularRetryAttempts);
            delay(2000);
        }
        success = cellularManager_->connectToNetwork();
    }
    
    if (success) {
        activeConnection_ = CONN_CELLULAR;
        currentStatus_ = STATUS_CONNECTED;
        connectionStartTime_ = millis();
        lastStatusChange_ = millis();
        updateNetworkQuality();
        Serial.println("[ConnOrch] Cellular connected successfully");
        logConnectionChange(CONN_NONE, CONN_CELLULAR);
        return true;
    }
    
    Serial.println("[ConnOrch] Cellular connection failed");
    return false;
}

// Disconnect from network
void ConnectivityOrchestrator::disconnect() {
    if (wifiManager_ && activeConnection_ == CONN_WIFI) {
        wifiManager_->disconnect();
    }
    
    activeConnection_ = CONN_NONE;
    currentStatus_ = STATUS_DISCONNECTED;
    currentQuality_ = QUALITY_NONE;
}

// Reconnect
bool ConnectivityOrchestrator::reconnect() {
    disconnect();
    delay(1000);
    return connect();
}

// Update (call in main loop)
void ConnectivityOrchestrator::update() {
    if (!initialized_) return;
    
    unsigned long now = millis();
    
    // Perform periodic health check
    if (now - lastHealthCheck_ >= config_.healthCheckInterval) {
        performHealthCheck();
        lastHealthCheck_ = now;
    }
    
    // Update connection stats
    if (currentStatus_ == STATUS_CONNECTED) {
        stats_.totalUptime += (now - lastStatusChange_) / 1000;
        if (activeConnection_ == CONN_WIFI) {
            stats_.wifiUptime += (now - lastStatusChange_) / 1000;
        } else if (activeConnection_ == CONN_CELLULAR) {
            stats_.cellularUptime += (now - lastStatusChange_) / 1000;
        }
        lastStatusChange_ = now;
    }
    
    // Auto-reconnect if disconnected
    if (currentStatus_ != STATUS_CONNECTED && 
        now - lastReconnectAttempt_ >= config_.reconnectInterval) {
        Serial.println("[ConnOrch] Auto-reconnecting...");
        connect();
        lastReconnectAttempt_ = now;
    }
}

// Check if connected
bool ConnectivityOrchestrator::isConnected() const {
    return currentStatus_ == STATUS_CONNECTED;
}

// Get active connection type
ConnectionType ConnectivityOrchestrator::getActiveConnection() const {
    return activeConnection_;
}

// Get connection status
ConnectionStatus ConnectivityOrchestrator::getConnectionStatus() const {
    return currentStatus_;
}

// Get network quality
NetworkQuality ConnectivityOrchestrator::getNetworkQuality() const {
    return currentQuality_;
}

// Get signal strength
int ConnectivityOrchestrator::getSignalStrength() const {
    if (activeConnection_ == CONN_WIFI && wifiManager_) {
        return wifiManager_->getSignalStrength();
    } else if (activeConnection_ == CONN_CELLULAR && cellularManager_) {
        return cellularManager_->getSignalStrength();
    }
    return -999;
}

// Get connection info string
String ConnectivityOrchestrator::getConnectionInfo() const {
    String info = "Connection: ";
    
    switch (activeConnection_) {
        case CONN_WIFI:
            info += "WiFi (RSSI: " + String(getSignalStrength()) + " dBm)";
            break;
        case CONN_CELLULAR:
            info += "Cellular (Signal: " + String(getSignalStrength()) + ")";
            break;
        default:
            info += "None";
            break;
    }
    
    info += ", Status: ";
    switch (currentStatus_) {
        case STATUS_CONNECTED: info += "Connected"; break;
        case STATUS_CONNECTING: info += "Connecting"; break;
        case STATUS_DEGRADED: info += "Degraded"; break;
        case STATUS_FAILED: info += "Failed"; break;
        default: info += "Disconnected"; break;
    }
    
    return info;
}

// Send data
bool ConnectivityOrchestrator::sendData(const uint8_t* data, size_t length) {
    if (!isConnected()) {
        Serial.println("[ConnOrch] Not connected, cannot send data");
        return false;
    }
    
    bool success = false;
    
    if (activeConnection_ == CONN_CELLULAR && cellularManager_) {
        success = cellularManager_->sendData(data, length);
        if (success) {
            trackDataUsage(length);
        }
    }
    // For WiFi, data is sent through standard HTTP/MQTT clients
    
    return success;
}

// Send data with optimization
bool ConnectivityOrchestrator::sendDataOptimized(const uint8_t* data, size_t length, bool isLargeUpload) {
    if (!isConnected()) {
        Serial.println("[ConnOrch] Not connected");
        return false;
    }
    
    // If large upload and cost optimization enabled, prefer WiFi
    if (isLargeUpload && config_.enableCostOptimization && config_.preferWiFiForLargeUploads) {
        if (activeConnection_ == CONN_CELLULAR && wifiManager_) {
            Serial.println("[ConnOrch] Large upload, attempting WiFi for cost savings");
            if (connectWiFi()) {
                logConnectionChange(CONN_CELLULAR, CONN_WIFI);
            }
        }
    }
    
    // Check cellular data limit
    if (activeConnection_ == CONN_CELLULAR && isCellularDataLimitReached()) {
        Serial.println("[ConnOrch] Cellular data limit reached, attempting WiFi");
        if (wifiManager_ && connectWiFi()) {
            logConnectionChange(CONN_CELLULAR, CONN_WIFI);
        } else {
            Serial.println("[ConnOrch] Cannot switch to WiFi, blocking cellular upload");
            return false;
        }
    }
    
    return sendData(data, length);
}

// Perform health check
void ConnectivityOrchestrator::performHealthCheck() {
    if (!isConnected()) return;
    
    updateNetworkQuality();
    
    // Check if we should fallback
    if (config_.autoFallback) {
        if (shouldFallbackToCellular()) {
            Serial.println("[ConnOrch] WiFi quality degraded, falling back to cellular");
            if (connectCellular()) {
                logConnectionChange(CONN_WIFI, CONN_CELLULAR);
                stats_.automaticFallbacks++;
            }
        } else if (shouldFallbackToWiFi()) {
            Serial.println("[ConnOrch] WiFi available, switching from cellular");
            if (connectWiFi()) {
                logConnectionChange(CONN_CELLULAR, CONN_WIFI);
                stats_.automaticFallbacks++;
            }
        }
    }
}

// Check if connection is healthy
bool ConnectivityOrchestrator::isHealthy() const {
    return isConnected() && currentQuality_ != QUALITY_POOR && currentQuality_ != QUALITY_NONE;
}

// Get connection reliability
float ConnectivityOrchestrator::getConnectionReliability() const {
    if (stats_.totalUptime == 0) return 0.0f;
    
    uint32_t totalAttempts = stats_.wifiReconnects + stats_.cellularReconnects + stats_.failedConnections + 1;
    float successRate = 1.0f - ((float)stats_.failedConnections / totalAttempts);
    
    return successRate;
}

// Get statistics
ConnectionStats ConnectivityOrchestrator::getStatistics() const {
    return stats_;
}

// Reset statistics
void ConnectivityOrchestrator::resetStatistics() {
    stats_ = ConnectionStats();
}

// Set configuration
void ConnectivityOrchestrator::setConfig(const ConnectivityConfig& config) {
    config_ = config;
}

// Get configuration
ConnectivityConfig ConnectivityOrchestrator::getConfig() const {
    return config_;
}

// Enable/disable auto fallback
void ConnectivityOrchestrator::enableAutoFallback(bool enable) {
    config_.autoFallback = enable;
}

// Set cellular data limit
void ConnectivityOrchestrator::setCellularDataLimit(size_t limitMB) {
    config_.cellularDailyDataLimit = limitMB;
}

// Force connection type
bool ConnectivityOrchestrator::forceConnectionType(ConnectionType type) {
    disconnect();
    
    if (type == CONN_WIFI && wifiManager_) {
        return connectWiFi();
    } else if (type == CONN_CELLULAR && cellularManager_) {
        return connectCellular();
    }
    
    return false;
}

// Prefer WiFi
void ConnectivityOrchestrator::preferWiFi() {
    if (activeConnection_ != CONN_WIFI && wifiManager_) {
        connectWiFi();
    }
}

// Prefer Cellular
void ConnectivityOrchestrator::preferCellular() {
    if (activeConnection_ != CONN_CELLULAR && cellularManager_) {
        connectCellular();
    }
}

// Update network quality
void ConnectivityOrchestrator::updateNetworkQuality() {
    int signal = getSignalStrength();
    
    if (activeConnection_ == CONN_WIFI) {
        if (signal > -50) currentQuality_ = QUALITY_EXCELLENT;
        else if (signal > -60) currentQuality_ = QUALITY_GOOD;
        else if (signal > -70) currentQuality_ = QUALITY_FAIR;
        else if (signal > -80) currentQuality_ = QUALITY_POOR;
        else currentQuality_ = QUALITY_POOR;
    } else if (activeConnection_ == CONN_CELLULAR) {
        // Cellular signal strength ranges typically 0-31 or as RSSI
        if (signal > 20 || signal > -70) currentQuality_ = QUALITY_EXCELLENT;
        else if (signal > 15 || signal > -80) currentQuality_ = QUALITY_GOOD;
        else if (signal > 10 || signal > -90) currentQuality_ = QUALITY_FAIR;
        else if (signal > 5 || signal > -100) currentQuality_ = QUALITY_POOR;
        else currentQuality_ = QUALITY_POOR;
    }
}

// Check if should fallback to cellular
bool ConnectivityOrchestrator::shouldFallbackToCellular() {
    if (activeConnection_ != CONN_WIFI || !cellularManager_) return false;
    
    int signal = getSignalStrength();
    return signal < config_.fallbackThreshold || currentQuality_ == QUALITY_POOR;
}

// Check if should fallback to WiFi
bool ConnectivityOrchestrator::shouldFallbackToWiFi() {
    if (activeConnection_ != CONN_CELLULAR || !wifiManager_) return false;
    if (!config_.cellularAsFallbackOnly) return false;
    
    // Try to scan and connect to WiFi if it's available
    return true;  // Attempt WiFi connection
}

// Check if cellular data limit reached
bool ConnectivityOrchestrator::isCellularDataLimitReached() const {
    if (!cellularManager_) return false;
    
    size_t usedMB = stats_.dataTransferredCellular / 1024;  // Convert KB to MB
    return usedMB >= config_.cellularDailyDataLimit;
}

// Track data usage
void ConnectivityOrchestrator::trackDataUsage(size_t bytes) {
    size_t kb = bytes / 1024;
    if (activeConnection_ == CONN_WIFI) {
        stats_.dataTransferredWiFi += kb;
    } else if (activeConnection_ == CONN_CELLULAR) {
        stats_.dataTransferredCellular += kb;
    }
}

// Log connection change
void ConnectivityOrchestrator::logConnectionChange(ConnectionType from, ConnectionType to) {
    Serial.print("[ConnOrch] Connection changed: ");
    
    switch (from) {
        case CONN_WIFI: Serial.print("WiFi"); break;
        case CONN_CELLULAR: Serial.print("Cellular"); break;
        default: Serial.print("None"); break;
    }
    
    Serial.print(" -> ");
    
    switch (to) {
        case CONN_WIFI: Serial.println("WiFi"); break;
        case CONN_CELLULAR: Serial.println("Cellular"); break;
        default: Serial.println("None"); break;
    }
    
    if (to == CONN_WIFI) {
        stats_.wifiReconnects++;
    } else if (to == CONN_CELLULAR) {
        stats_.cellularReconnects++;
    }
}

// Helper functions
bool initializeConnectivity(const ConnectivityConfig& config) {
    if (!g_connectivityOrchestrator) {
        g_connectivityOrchestrator = new ConnectivityOrchestrator();
    }
    return g_connectivityOrchestrator->initialize(config);
}

bool ensureConnected() {
    if (!g_connectivityOrchestrator) return false;
    
    if (!g_connectivityOrchestrator->isConnected()) {
        return g_connectivityOrchestrator->connect();
    }
    return true;
}

bool sendToCloud(const uint8_t* data, size_t length) {
    if (!g_connectivityOrchestrator) return false;
    return g_connectivityOrchestrator->sendData(data, length);
}

ConnectionType getCurrentConnection() {
    if (!g_connectivityOrchestrator) return CONN_NONE;
    return g_connectivityOrchestrator->getActiveConnection();
}

bool isCloudConnected() {
    if (!g_connectivityOrchestrator) return false;
    return g_connectivityOrchestrator->isConnected();
}
