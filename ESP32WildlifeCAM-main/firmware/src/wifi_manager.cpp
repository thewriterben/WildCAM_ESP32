/**
 * WiFi Manager Module
 * 
 * Manages WiFi connectivity, power management, and network operations
 * for the wildlife camera system.
 */

#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// Constructor
WiFiManager::WiFiManager() 
    : initialized(false), enabled(WIFI_ENABLED), currentStatus(WIFI_STATUS_DISABLED),
      lastConnectionAttempt(0), connectionStartTime(0), reconnectAttempts(0),
      apMode(false) {
    applyConfigurationSettings();
}

// Destructor
WiFiManager::~WiFiManager() {
    cleanup();
}

/**
 * Apply configuration settings from config.h
 */
void WiFiManager::applyConfigurationSettings() {
    config.ssid = WIFI_SSID;
    config.password = WIFI_PASSWORD;
    config.timeout = WIFI_TIMEOUT;
    config.retryCount = WIFI_RETRY_COUNT;
    config.powerSaveMode = WIFI_SLEEP_MODE;
}

/**
 * Initialize WiFi manager
 */
bool WiFiManager::init() {
    if (!enabled) {
        DEBUG_PRINTLN("WiFi disabled in configuration");
        currentStatus = WIFI_STATUS_DISABLED;
        return true;
    }

    DEBUG_PRINTLN("Initializing WiFi manager...");
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Set power save mode
    WiFi.setSleep(config.powerSaveMode);
    
    // Set hostname
    WiFi.setHostname(DEVICE_NAME);
    
    initialized = true;
    currentStatus = WIFI_STATUS_DISCONNECTED;
    
    DEBUG_PRINTLN("WiFi manager initialized");
    return true;
}

/**
 * Connect to WiFi network using configured credentials
 */
bool WiFiManager::connect() {
    return connect(config.ssid, config.password);
}

/**
 * Connect to WiFi network with specific credentials
 */
bool WiFiManager::connect(const String& ssid, const String& password) {
    if (!initialized || !enabled) {
        DEBUG_PRINTLN("WiFi not initialized or disabled");
        return false;
    }

    if (ssid.isEmpty()) {
        DEBUG_PRINTLN("WiFi SSID not configured");
        return false;
    }

    DEBUG_PRINTF("Connecting to WiFi: %s\n", ssid.c_str());
    
    currentStatus = WIFI_STATUS_CONNECTING;
    connectionStartTime = millis();
    reconnectAttempts = 0;
    
    // Start connection
    WiFi.begin(ssid.c_str(), password.c_str());
    
    return attemptConnection();
}

/**
 * Disconnect from WiFi network
 */
void WiFiManager::disconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTLN("Disconnecting from WiFi");
        WiFi.disconnect();
    }
    
    if (apMode) {
        stopAccessPoint();
    }
    
    currentStatus = WIFI_STATUS_DISCONNECTED;
}

/**
 * Check if WiFi is connected
 */
bool WiFiManager::isConnected() const {
    return (currentStatus == WIFI_STATUS_CONNECTED) && (WiFi.status() == WL_CONNECTED);
}

/**
 * Get WiFi signal strength (RSSI)
 */
int WiFiManager::getSignalStrength() const {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return -100; // Very weak signal indicator
}

/**
 * Get local IP address
 */
String WiFiManager::getIPAddress() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

/**
 * Get WiFi status information
 */
WiFiStatusInfo WiFiManager::getStatus() const {
    WiFiStatusInfo status;
    status.initialized = initialized;
    status.status = currentStatus;
    status.ssid = WiFi.SSID();
    status.ipAddress = getIPAddress();
    status.rssi = getSignalStrength();
    status.connectionTime = (currentStatus == WIFI_STATUS_CONNECTED) ? 
                           millis() - connectionStartTime : 0;
    status.reconnectAttempts = reconnectAttempts;
    status.apMode = apMode;
    
    return status;
}

/**
 * Enable or disable WiFi functionality
 */
void WiFiManager::setEnabled(bool enabled) {
    this->enabled = enabled;
    
    if (!enabled) {
        disconnect();
        currentStatus = WIFI_STATUS_DISABLED;
        DEBUG_PRINTLN("WiFi disabled");
    } else {
        currentStatus = WIFI_STATUS_DISCONNECTED;
        DEBUG_PRINTLN("WiFi enabled");
    }
}

/**
 * Set WiFi power save mode
 */
void WiFiManager::setPowerSaveMode(wifi_ps_type_t mode) {
    config.powerSaveMode = mode;
    if (initialized) {
        WiFi.setSleep(mode);
        DEBUG_PRINTF("WiFi power save mode set to: %d\n", mode);
    }
}

/**
 * Start access point mode for configuration
 */
bool WiFiManager::startAccessPoint(const String& apSSID, const String& apPassword) {
    DEBUG_PRINTF("Starting Access Point: %s\n", apSSID.c_str());
    
    // Stop WiFi client mode if active
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
    }
    
    // Configure AP mode
    WiFi.mode(WIFI_AP);
    
    bool success;
    if (apPassword.isEmpty()) {
        success = WiFi.softAP(apSSID.c_str());
    } else {
        success = WiFi.softAP(apSSID.c_str(), apPassword.c_str());
    }
    
    if (success) {
        apMode = true;
        apSSID = apSSID;
        DEBUG_PRINTF("Access Point started successfully. IP: %s\n", 
                     WiFi.softAPIP().toString().c_str());
    } else {
        DEBUG_PRINTLN("Failed to start Access Point");
    }
    
    return success;
}

/**
 * Stop access point mode
 */
void WiFiManager::stopAccessPoint() {
    if (apMode) {
        DEBUG_PRINTLN("Stopping Access Point");
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        apMode = false;
        apSSID = "";
    }
}

/**
 * Update WiFi status and handle reconnection
 */
void WiFiManager::update() {
    if (!initialized || !enabled) return;
    
    unsigned long now = millis();
    
    // Handle connection states
    switch (currentStatus) {
        case WIFI_STATUS_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                currentStatus = WIFI_STATUS_CONNECTED;
                logConnectionStatus();
            } else if (now - connectionStartTime > config.timeout) {
                handleConnectionTimeout();
            }
            break;
            
        case WIFI_STATUS_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                DEBUG_PRINTLN("WiFi connection lost");
                currentStatus = WIFI_STATUS_DISCONNECTED;
                lastConnectionAttempt = now;
            }
            break;
            
        case WIFI_STATUS_DISCONNECTED:
        case WIFI_STATUS_FAILED:
            // Attempt reconnection if configured and enough time has passed
            if (!config.ssid.isEmpty() && 
                (now - lastConnectionAttempt > 30000) && // 30 seconds between attempts
                (reconnectAttempts < config.retryCount)) {
                
                DEBUG_PRINTLN("Attempting WiFi reconnection...");
                connect();
            }
            break;
            
        default:
            break;
    }
}

/**
 * Scan for available networks
 */
int WiFiManager::scanNetworks() {
    if (!initialized) return 0;
    
    DEBUG_PRINTLN("Scanning for WiFi networks...");
    int networkCount = WiFi.scanNetworks();
    
    if (networkCount > 0) {
        DEBUG_PRINTF("Found %d networks\n", networkCount);
        for (int i = 0; i < networkCount; i++) {
            DEBUG_PRINTF("  %d: %s (%d dBm)\n", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    } else {
        DEBUG_PRINTLN("No networks found");
    }
    
    return networkCount;
}

/**
 * Get scanned network SSID by index
 */
String WiFiManager::getScannedSSID(int index) const {
    return WiFi.SSID(index);
}

/**
 * Get scanned network signal strength by index
 */
int WiFiManager::getScannedRSSI(int index) const {
    return WiFi.RSSI(index);
}

/**
 * Set WiFi configuration
 */
void WiFiManager::setConfig(const WiFiConfig& config) {
    this->config = config;
    DEBUG_PRINTF("WiFi configuration updated: SSID=%s\n", config.ssid.c_str());
}

/**
 * Get current WiFi configuration
 */
WiFiConfig WiFiManager::getConfig() const {
    return config;
}

/**
 * Reset WiFi settings to defaults
 */
void WiFiManager::resetSettings() {
    DEBUG_PRINTLN("Resetting WiFi settings to defaults");
    
    disconnect();
    applyConfigurationSettings();
    reconnectAttempts = 0;
    
    // Clear stored WiFi credentials
    WiFi.disconnect(true);
}

/**
 * Cleanup WiFi manager resources
 */
void WiFiManager::cleanup() {
    if (initialized) {
        disconnect();
        WiFi.mode(WIFI_OFF);
        initialized = false;
        DEBUG_PRINTLN("WiFi manager cleaned up");
    }
}

/**
 * Attempt WiFi connection with timeout handling
 */
bool WiFiManager::attemptConnection() {
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startTime < config.timeout) {
        delay(500);
        DEBUG_PRINT(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        currentStatus = WIFI_STATUS_CONNECTED;
        logConnectionStatus();
        return true;
    } else {
        handleConnectionTimeout();
        return false;
    }
}

/**
 * Handle connection timeout
 */
void WiFiManager::handleConnectionTimeout() {
    DEBUG_PRINTLN("\nWiFi connection timeout");
    
    reconnectAttempts++;
    currentStatus = WIFI_STATUS_FAILED;
    lastConnectionAttempt = millis();
    
    if (reconnectAttempts >= config.retryCount) {
        DEBUG_PRINTF("WiFi connection failed after %d attempts\n", reconnectAttempts);
        currentStatus = WIFI_STATUS_FAILED;
    }
}

/**
 * Log connection status
 */
void WiFiManager::logConnectionStatus() {
    DEBUG_PRINTLN("\nWiFi connected successfully!");
    DEBUG_PRINTF("SSID: %s\n", WiFi.SSID().c_str());
    DEBUG_PRINTF("IP Address: %s\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTF("Signal Strength: %d dBm\n", WiFi.RSSI());
    DEBUG_PRINTF("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    DEBUG_PRINTF("DNS: %s\n", WiFi.dnsIP().toString().c_str());
}