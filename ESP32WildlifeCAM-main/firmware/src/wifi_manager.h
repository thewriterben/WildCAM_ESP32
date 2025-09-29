#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi connection status enumeration
enum WiFiStatus {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING = 1,
    WIFI_STATUS_CONNECTED = 2,
    WIFI_STATUS_FAILED = 3,
    WIFI_STATUS_DISABLED = 4
};

// WiFi configuration structure
struct WiFiConfig {
    String ssid;
    String password;
    int timeout;
    int retryCount;
    wifi_ps_type_t powerSaveMode;
};

// WiFi status structure
struct WiFiStatusInfo {
    bool initialized;
    WiFiStatus status;
    String ssid;
    String ipAddress;
    int rssi;
    unsigned long connectionTime;
    int reconnectAttempts;
    bool apMode;
};

/**
 * @class WiFiManager
 * @brief Manages WiFi connectivity and network operations
 * 
 * This class encapsulates WiFi functionality for the wildlife camera,
 * including connection management, reconnection logic, and power saving.
 */
class WiFiManager {
public:
    /**
     * @brief Constructor
     */
    WiFiManager();

    /**
     * @brief Destructor
     */
    ~WiFiManager();

    /**
     * @brief Initialize WiFi manager
     * @return true if initialization successful, false otherwise
     */
    bool init();

    /**
     * @brief Connect to WiFi network
     * @return true if connection successful, false otherwise
     */
    bool connect();

    /**
     * @brief Connect to WiFi network with specific credentials
     * @param ssid Network SSID
     * @param password Network password
     * @return true if connection successful, false otherwise
     */
    bool connect(const String& ssid, const String& password);

    /**
     * @brief Disconnect from WiFi network
     */
    void disconnect();

    /**
     * @brief Check if WiFi is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Get WiFi signal strength (RSSI)
     * @return Signal strength in dBm
     */
    int getSignalStrength() const;

    /**
     * @brief Get local IP address
     * @return IP address as string
     */
    String getIPAddress() const;

    /**
     * @brief Get WiFi status information
     * @return WiFi status structure
     */
    WiFiStatusInfo getStatus() const;

    /**
     * @brief Enable or disable WiFi functionality
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);

    /**
     * @brief Set WiFi power save mode
     * @param mode Power save mode
     */
    void setPowerSaveMode(wifi_ps_type_t mode);

    /**
     * @brief Start access point mode for configuration
     * @param apSSID Access point SSID
     * @param apPassword Access point password (optional)
     * @return true if AP started successfully
     */
    bool startAccessPoint(const String& apSSID, const String& apPassword = "");

    /**
     * @brief Stop access point mode
     */
    void stopAccessPoint();

    /**
     * @brief Update WiFi status and handle reconnection
     */
    void update();

    /**
     * @brief Scan for available networks
     * @return Number of networks found
     */
    int scanNetworks();

    /**
     * @brief Get scanned network SSID by index
     * @param index Network index from scan
     * @return Network SSID
     */
    String getScannedSSID(int index) const;

    /**
     * @brief Get scanned network signal strength by index
     * @param index Network index from scan
     * @return Signal strength in dBm
     */
    int getScannedRSSI(int index) const;

    /**
     * @brief Set WiFi configuration
     * @param config WiFi configuration structure
     */
    void setConfig(const WiFiConfig& config);

    /**
     * @brief Get current WiFi configuration
     * @return WiFi configuration structure
     */
    WiFiConfig getConfig() const;

    /**
     * @brief Reset WiFi settings to defaults
     */
    void resetSettings();

    /**
     * @brief Cleanup WiFi manager resources
     */
    void cleanup();

    /**
     * @brief Check if WiFi manager is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }

    /**
     * @brief Check if in access point mode
     * @return true if in AP mode, false otherwise
     */
    bool isAPMode() const { return apMode; }

private:
    // Member variables
    bool initialized;
    bool enabled;
    WiFiStatus currentStatus;
    WiFiConfig config;
    unsigned long lastConnectionAttempt;
    unsigned long connectionStartTime;
    int reconnectAttempts;
    bool apMode;
    String apSSID;
    
    // Private methods
    void applyConfigurationSettings();
    bool attemptConnection();
    void handleConnectionTimeout();
    void logConnectionStatus();
};

#endif // WIFI_MANAGER_H