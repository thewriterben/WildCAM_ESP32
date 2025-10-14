/**
 * @file wifi_lora_fallback.cpp
 * @brief Implementation of WiFi-LoRa fallback system
 */

#include "wifi_lora_fallback.h"
#include "debug_utils.h"

// Constructor
WiFiLoRaFallback::WiFiLoRaFallback()
    : wifiManager_(nullptr),
      initialized_(false),
      lastHealthCheck_(0),
      lastSwitchAttempt_(0),
      connectionStartTime_(0),
      wifiReconnectAttempts_(0),
      loraReconnectAttempts_(0),
      wifiSuccessCount_(0),
      wifiFailCount_(0),
      loraSuccessCount_(0),
      loraFailCount_(0) {
}

// Destructor
WiFiLoRaFallback::~WiFiLoRaFallback() {
    cleanup();
}

// Initialize fallback system
bool WiFiLoRaFallback::init(WiFiManager* wifiMgr, const FallbackConfig& config) {
    if (initialized_) {
        return true;
    }
    
    if (!wifiMgr) {
        DEBUG_PRINTLN("WiFi manager required for fallback system");
        return false;
    }
    
    DEBUG_PRINTLN("Initializing WiFi-LoRa Fallback System...");
    
    wifiManager_ = wifiMgr;
    config_ = config;
    
    // Initialize LoRa mesh
    if (!LoraMesh::init()) {
        DEBUG_PRINTLN("Warning: LoRa initialization failed");
        // Continue anyway - WiFi-only mode
    }
    
    state_.lastSwitch = millis();
    connectionStartTime_ = millis();
    
    initialized_ = true;
    DEBUG_PRINTLN("WiFi-LoRa Fallback System initialized");
    return true;
}

// Cleanup resources
void WiFiLoRaFallback::cleanup() {
    if (initialized_) {
        disconnect();
        LoraMesh::cleanup();
        wifiManager_ = nullptr;
        initialized_ = false;
    }
}

// Connect to available network
bool WiFiLoRaFallback::connect() {
    if (!initialized_) {
        return false;
    }
    
    DEBUG_PRINTLN("Attempting to connect...");
    
    // Try WiFi first if preferred
    if (config_.preferWiFi) {
        if (wifiManager_->connect()) {
            state_.activeNetwork = NETWORK_WIFI;
            state_.wifiConnected = true;
            DEBUG_PRINTLN("Connected via WiFi");
            return true;
        }
        
        // Fall back to LoRa
        if (config_.autoFallbackEnabled) {
            DEBUG_PRINTLN("WiFi failed, falling back to LoRa");
            state_.activeNetwork = NETWORK_LORA;
            state_.loraConnected = true;
            return true;
        }
    } else {
        // Try LoRa first
        state_.activeNetwork = NETWORK_LORA;
        state_.loraConnected = true;
        DEBUG_PRINTLN("Connected via LoRa");
        return true;
    }
    
    return false;
}

// Disconnect from all networks
void WiFiLoRaFallback::disconnect() {
    if (wifiManager_) {
        wifiManager_->disconnect();
    }
    
    state_.activeNetwork = NETWORK_NONE;
    state_.wifiConnected = false;
    state_.loraConnected = false;
}

// Update fallback system (call in main loop)
void WiFiLoRaFallback::update() {
    if (!initialized_) {
        return;
    }
    
    unsigned long now = millis();
    
    // Process LoRa messages
    LoraMesh::processMessages();
    
    // Periodic health check
    if (now - lastHealthCheck_ >= config_.healthCheckInterval) {
        performHealthCheck();
        lastHealthCheck_ = now;
    }
    
    // Update uptime
    updateUptime();
    
    // Check if automatic fallback is needed
    if (config_.autoFallbackEnabled) {
        if (shouldSwitchToLoRa() && canSwitch()) {
            switchToLoRa(REASON_SIGNAL_WEAK);
        } else if (shouldSwitchToWiFi() && canSwitch()) {
            switchToWiFi(REASON_SIGNAL_WEAK);
        }
    }
    
    // Attempt reconnection if disconnected
    if (!isConnected() && 
        now - lastSwitchAttempt_ >= config_.wifiReconnectInterval) {
        attemptReconnect();
        lastSwitchAttempt_ = now;
    }
}

// Perform health check on both networks
void WiFiLoRaFallback::performHealthCheck() {
    state_.wifiHealth = assessWiFiHealth();
    state_.loraHealth = assessLoRaHealth();
    
    // Update connection states
    state_.wifiConnected = wifiManager_->isConnected();
    state_.wifiRSSI = wifiManager_->getSignalStrength();
    
    MeshNetworkStatus loraStatus = LoraMesh::getNetworkStatus();
    state_.loraConnected = loraStatus.initialized && loraStatus.connectedNodes > 0;
    state_.loraRSSI = loraStatus.rssi;
}

// Assess WiFi health
HealthStatus WiFiLoRaFallback::assessWiFiHealth() {
    if (!wifiManager_->isConnected()) {
        return HEALTH_OFFLINE;
    }
    
    int rssi = wifiManager_->getSignalStrength();
    
    if (rssi >= -50) {
        return HEALTH_EXCELLENT;
    } else if (rssi >= -60) {
        return HEALTH_GOOD;
    } else if (rssi >= -70) {
        return HEALTH_DEGRADED;
    } else if (rssi >= config_.wifiRSSIThreshold) {
        return HEALTH_POOR;
    } else {
        return HEALTH_CRITICAL;
    }
}

// Assess LoRa health
HealthStatus WiFiLoRaFallback::assessLoRaHealth() {
    MeshNetworkStatus status = LoraMesh::getNetworkStatus();
    
    if (!status.initialized) {
        return HEALTH_OFFLINE;
    }
    
    if (status.connectedNodes == 0) {
        return HEALTH_CRITICAL;
    }
    
    int rssi = status.rssi;
    
    if (rssi >= -80) {
        return HEALTH_EXCELLENT;
    } else if (rssi >= -100) {
        return HEALTH_GOOD;
    } else if (rssi >= -120) {
        return HEALTH_DEGRADED;
    } else {
        return HEALTH_POOR;
    }
}

// Get WiFi health status
HealthStatus WiFiLoRaFallback::getWiFiHealth() const {
    return state_.wifiHealth;
}

// Get LoRa health status
HealthStatus WiFiLoRaFallback::getLoRaHealth() const {
    return state_.loraHealth;
}

// Get overall health status
HealthStatus WiFiLoRaFallback::getOverallHealth() const {
    if (state_.activeNetwork == NETWORK_WIFI) {
        return state_.wifiHealth;
    } else if (state_.activeNetwork == NETWORK_LORA) {
        return state_.loraHealth;
    } else if (state_.activeNetwork == NETWORK_BOTH) {
        // Return the better of the two
        return (state_.wifiHealth < state_.loraHealth) ? 
               state_.wifiHealth : state_.loraHealth;
    }
    return HEALTH_OFFLINE;
}

// Check if system is healthy
bool WiFiLoRaFallback::isHealthy() const {
    HealthStatus health = getOverallHealth();
    return health <= HEALTH_DEGRADED;
}

// Get active network
ActiveNetwork WiFiLoRaFallback::getActiveNetwork() const {
    return state_.activeNetwork;
}

// Get network state
NetworkState WiFiLoRaFallback::getNetworkState() const {
    return state_;
}

// Check if connected to any network
bool WiFiLoRaFallback::isConnected() const {
    return state_.wifiConnected || state_.loraConnected;
}

// Check if has any connectivity
bool WiFiLoRaFallback::hasConnectivity() const {
    return isConnected() && isHealthy();
}

// Get signal strength of active network
int WiFiLoRaFallback::getSignalStrength() const {
    if (state_.activeNetwork == NETWORK_WIFI) {
        return state_.wifiRSSI;
    } else if (state_.activeNetwork == NETWORK_LORA) {
        return state_.loraRSSI;
    }
    return -100;
}

// Get network information string
String WiFiLoRaFallback::getNetworkInfo() const {
    String info = "Active: " + networkToString(state_.activeNetwork) + "\n";
    info += "WiFi: " + healthToString(state_.wifiHealth) + 
            " (" + String(state_.wifiRSSI) + " dBm)\n";
    info += "LoRa: " + healthToString(state_.loraHealth) + 
            " (" + String(state_.loraRSSI) + " dBm)\n";
    return info;
}

// Send data via best available network
TransmissionResult WiFiLoRaFallback::sendData(const uint8_t* data, size_t length) {
    TransmissionResult result;
    unsigned long startTime = millis();
    
    if (state_.activeNetwork == NETWORK_WIFI && state_.wifiConnected) {
        result.success = sendViaWiFi(data, length);
        result.usedNetwork = NETWORK_WIFI;
    } else if (state_.activeNetwork == NETWORK_LORA && state_.loraConnected) {
        result.success = sendViaLoRa(data, length);
        result.usedNetwork = NETWORK_LORA;
    } else if (state_.activeNetwork == NETWORK_BOTH) {
        result.success = sendViaBoth(data, length);
        result.usedNetwork = NETWORK_BOTH;
    } else {
        // Try to find any available network
        if (state_.wifiConnected && sendViaWiFi(data, length)) {
            result.success = true;
            result.usedNetwork = NETWORK_WIFI;
        } else if (state_.loraConnected && sendViaLoRa(data, length)) {
            result.success = true;
            result.usedNetwork = NETWORK_LORA;
        } else {
            result.errorMessage = "No network available";
        }
    }
    
    result.transmissionTime = millis() - startTime;
    result.bytesSent = result.success ? length : 0;
    
    recordTransmission(result.usedNetwork, result.success);
    
    return result;
}

// Send data with priority consideration
TransmissionResult WiFiLoRaFallback::sendDataPriority(const uint8_t* data, 
                                                     size_t length,
                                                     bool highPriority) {
    if (highPriority && config_.allowDualMode) {
        // Send via both networks for critical data
        TransmissionResult result;
        unsigned long startTime = millis();
        result.success = sendViaBoth(data, length);
        result.usedNetwork = NETWORK_BOTH;
        result.transmissionTime = millis() - startTime;
        result.bytesSent = result.success ? length : 0;
        return result;
    }
    
    return sendData(data, length);
}

// Send data via WiFi
bool WiFiLoRaFallback::sendViaWiFi(const uint8_t* data, size_t length) {
    if (!state_.wifiConnected) {
        return false;
    }
    
    // Placeholder - actual implementation would use HTTP/MQTT/etc.
    DEBUG_PRINTF("Sending %d bytes via WiFi\n", length);
    delay(100); // Simulate transmission
    return true;
}

// Send data via LoRa
bool WiFiLoRaFallback::sendViaLoRa(const uint8_t* data, size_t length) {
    if (!state_.loraConnected) {
        return false;
    }
    
    // Use LoRa mesh to transmit
    String message = String((char*)data); // Simplified
    bool success = LoraMesh::queueMessage(message);
    DEBUG_PRINTF("Sending %d bytes via LoRa: %s\n", length, success ? "OK" : "FAILED");
    return success;
}

// Send via both networks (redundant mode)
bool WiFiLoRaFallback::sendViaBoth(const uint8_t* data, size_t length) {
    bool wifiSuccess = sendViaWiFi(data, length);
    bool loraSuccess = sendViaLoRa(data, length);
    
    // Consider successful if at least one succeeds
    return wifiSuccess || loraSuccess;
}

// Switch to WiFi network
bool WiFiLoRaFallback::switchToWiFi(FallbackReason reason) {
    if (!canSwitch()) {
        DEBUG_PRINTLN("Cannot switch networks yet (debounce)");
        return false;
    }
    
    DEBUG_PRINTF("Switching to WiFi (reason: %s)\n", reasonToString(reason).c_str());
    
    if (!wifiManager_->isConnected()) {
        if (!wifiManager_->connect()) {
            DEBUG_PRINTLN("WiFi connection failed");
            return false;
        }
    }
    
    state_.activeNetwork = NETWORK_WIFI;
    state_.lastSwitch = millis();
    state_.lastFallbackReason = reason;
    state_.totalSwitches++;
    
    return true;
}

// Switch to LoRa network
bool WiFiLoRaFallback::switchToLoRa(FallbackReason reason) {
    if (!canSwitch()) {
        DEBUG_PRINTLN("Cannot switch networks yet (debounce)");
        return false;
    }
    
    DEBUG_PRINTF("Switching to LoRa (reason: %s)\n", reasonToString(reason).c_str());
    
    state_.activeNetwork = NETWORK_LORA;
    state_.lastSwitch = millis();
    state_.lastFallbackReason = reason;
    state_.totalSwitches++;
    
    return true;
}

// Switch to both networks
bool WiFiLoRaFallback::switchToBoth() {
    if (!config_.allowDualMode) {
        DEBUG_PRINTLN("Dual mode not enabled");
        return false;
    }
    
    DEBUG_PRINTLN("Switching to dual network mode");
    state_.activeNetwork = NETWORK_BOTH;
    return true;
}

// Attempt to reconnect
bool WiFiLoRaFallback::attemptReconnect() {
    DEBUG_PRINTLN("Attempting reconnection...");
    
    // Try WiFi first
    if (config_.preferWiFi && wifiReconnectAttempts_ < config_.maxReconnectAttempts) {
        if (wifiManager_->connect()) {
            DEBUG_PRINTLN("WiFi reconnected");
            state_.activeNetwork = NETWORK_WIFI;
            state_.wifiConnected = true;
            wifiReconnectAttempts_ = 0;
            return true;
        }
        wifiReconnectAttempts_++;
    }
    
    // Fall back to LoRa
    if (loraReconnectAttempts_ < config_.maxReconnectAttempts) {
        DEBUG_PRINTLN("Using LoRa as fallback");
        state_.activeNetwork = NETWORK_LORA;
        state_.loraConnected = true;
        loraReconnectAttempts_ = 0;
        return true;
    }
    
    return false;
}

// Set configuration
void WiFiLoRaFallback::setConfig(const FallbackConfig& config) {
    config_ = config;
}

// Get configuration
FallbackConfig WiFiLoRaFallback::getConfig() const {
    return config_;
}

// Set auto fallback
void WiFiLoRaFallback::setAutoFallback(bool enabled) {
    config_.autoFallbackEnabled = enabled;
}

// Set WiFi preference
void WiFiLoRaFallback::setPreferWiFi(bool prefer) {
    config_.preferWiFi = prefer;
}

// Set dual mode
void WiFiLoRaFallback::setDualMode(bool enabled) {
    config_.allowDualMode = enabled;
}

// Get total switches
uint32_t WiFiLoRaFallback::getTotalSwitches() const {
    return state_.totalSwitches;
}

// Get WiFi uptime
uint32_t WiFiLoRaFallback::getWiFiUptime() const {
    return state_.wifiUptime;
}

// Get LoRa uptime
uint32_t WiFiLoRaFallback::getLoRaUptime() const {
    return state_.loraUptime;
}

// Get WiFi reliability
float WiFiLoRaFallback::getWiFiReliability() const {
    uint32_t total = wifiSuccessCount_ + wifiFailCount_;
    if (total == 0) return 0.0f;
    return static_cast<float>(wifiSuccessCount_) / total;
}

// Get LoRa reliability
float WiFiLoRaFallback::getLoRaReliability() const {
    uint32_t total = loraSuccessCount_ + loraFailCount_;
    if (total == 0) return 0.0f;
    return static_cast<float>(loraSuccessCount_) / total;
}

// Reset statistics
void WiFiLoRaFallback::resetStatistics() {
    state_.totalSwitches = 0;
    state_.wifiUptime = 0;
    state_.loraUptime = 0;
    wifiSuccessCount_ = 0;
    wifiFailCount_ = 0;
    loraSuccessCount_ = 0;
    loraFailCount_ = 0;
}

// Print status
void WiFiLoRaFallback::printStatus() const {
    DEBUG_PRINTLN("\n=== WiFi-LoRa Fallback Status ===");
    DEBUG_PRINTF("Active Network: %s\n", networkToString(state_.activeNetwork).c_str());
    DEBUG_PRINTF("WiFi: %s (%d dBm)\n", 
                healthToString(state_.wifiHealth).c_str(), state_.wifiRSSI);
    DEBUG_PRINTF("LoRa: %s (%d dBm)\n", 
                healthToString(state_.loraHealth).c_str(), state_.loraRSSI);
    DEBUG_PRINTF("Total Switches: %u\n", state_.totalSwitches);
    DEBUG_PRINTF("WiFi Uptime: %u s\n", state_.wifiUptime / 1000);
    DEBUG_PRINTF("LoRa Uptime: %u s\n", state_.loraUptime / 1000);
    DEBUG_PRINTF("WiFi Reliability: %.2f%%\n", getWiFiReliability() * 100);
    DEBUG_PRINTF("LoRa Reliability: %.2f%%\n", getLoRaReliability() * 100);
    DEBUG_PRINTLN("================================\n");
}

// Get diagnostics string
String WiFiLoRaFallback::getDiagnostics() const {
    String diag = "WiFi-LoRa Fallback System\n";
    diag += "Active: " + networkToString(state_.activeNetwork) + "\n";
    diag += "Switches: " + String(state_.totalSwitches) + "\n";
    diag += "WiFi: " + healthToString(state_.wifiHealth) + "\n";
    diag += "LoRa: " + healthToString(state_.loraHealth) + "\n";
    return diag;
}

// Check if should switch to LoRa
bool WiFiLoRaFallback::shouldSwitchToLoRa() {
    if (state_.activeNetwork == NETWORK_LORA) {
        return false; // Already on LoRa
    }
    
    // Switch if WiFi health is poor and LoRa is available
    if (state_.wifiHealth >= HEALTH_POOR && 
        state_.loraHealth <= HEALTH_GOOD &&
        state_.loraConnected) {
        return true;
    }
    
    // Switch if WiFi disconnected
    if (!state_.wifiConnected && state_.loraConnected) {
        return true;
    }
    
    return false;
}

// Check if should switch to WiFi
bool WiFiLoRaFallback::shouldSwitchToWiFi() {
    if (state_.activeNetwork == NETWORK_WIFI) {
        return false; // Already on WiFi
    }
    
    if (!config_.preferWiFi) {
        return false; // WiFi not preferred
    }
    
    // Switch if WiFi is available and healthy
    if (state_.wifiHealth <= HEALTH_GOOD && state_.wifiConnected) {
        return true;
    }
    
    return false;
}

// Check if can switch networks (debounce)
bool WiFiLoRaFallback::canSwitch() const {
    unsigned long timeSinceSwitch = millis() - state_.lastSwitch;
    return timeSinceSwitch >= config_.switchDebounceTime;
}

// Record transmission result
void WiFiLoRaFallback::recordTransmission(ActiveNetwork network, bool success) {
    if (network == NETWORK_WIFI) {
        if (success) {
            wifiSuccessCount_++;
        } else {
            wifiFailCount_++;
        }
    } else if (network == NETWORK_LORA) {
        if (success) {
            loraSuccessCount_++;
        } else {
            loraFailCount_++;
        }
    }
}

// Update network uptime
void WiFiLoRaFallback::updateUptime() {
    unsigned long now = millis();
    unsigned long elapsed = now - connectionStartTime_;
    
    if (state_.activeNetwork == NETWORK_WIFI && state_.wifiConnected) {
        state_.wifiUptime = elapsed;
    } else if (state_.activeNetwork == NETWORK_LORA && state_.loraConnected) {
        state_.loraUptime = elapsed;
    }
}

// Convert health status to string
String WiFiLoRaFallback::healthToString(HealthStatus health) const {
    switch (health) {
        case HEALTH_EXCELLENT: return "Excellent";
        case HEALTH_GOOD: return "Good";
        case HEALTH_DEGRADED: return "Degraded";
        case HEALTH_POOR: return "Poor";
        case HEALTH_CRITICAL: return "Critical";
        case HEALTH_OFFLINE: return "Offline";
        default: return "Unknown";
    }
}

// Convert network type to string
String WiFiLoRaFallback::networkToString(ActiveNetwork network) const {
    switch (network) {
        case NETWORK_NONE: return "None";
        case NETWORK_WIFI: return "WiFi";
        case NETWORK_LORA: return "LoRa";
        case NETWORK_BOTH: return "WiFi+LoRa";
        case NETWORK_SWITCHING: return "Switching";
        default: return "Unknown";
    }
}

// Convert fallback reason to string
String WiFiLoRaFallback::reasonToString(FallbackReason reason) const {
    switch (reason) {
        case REASON_NONE: return "None";
        case REASON_SIGNAL_WEAK: return "Weak Signal";
        case REASON_CONNECTION_LOST: return "Connection Lost";
        case REASON_TIMEOUT: return "Timeout";
        case REASON_MANUAL: return "Manual";
        case REASON_DATA_PRIORITY: return "Data Priority";
        case REASON_COST_OPTIMIZATION: return "Cost Optimization";
        default: return "Unknown";
    }
}
