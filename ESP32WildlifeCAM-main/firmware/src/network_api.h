/**
 * @file network_api.h
 * @brief REST API for Network State and Health Metrics
 * 
 * Provides HTTP endpoints for querying and managing network connectivity,
 * accessing health metrics, and controlling fallback behavior.
 */

#ifndef NETWORK_API_H
#define NETWORK_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "wifi_manager.h"
#include "wifi_ap_selector.h"
#include "wifi_lora_fallback.h"
#include "network_health_monitor.h"

/**
 * @class NetworkAPI
 * @brief REST API interface for network management
 */
class NetworkAPI {
public:
    NetworkAPI();
    ~NetworkAPI();
    
    // Initialization
    bool init(WiFiManager* wifiMgr, 
             WiFiAPSelector* apSelector,
             WiFiLoRaFallback* fallback,
             NetworkHealthMonitor* healthMonitor);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // API endpoint handlers (return JSON strings)
    
    // Network status endpoints
    String getNetworkStatus();              // GET /api/network/status
    String getWiFiStatus();                 // GET /api/network/wifi
    String getLoRaStatus();                 // GET /api/network/lora
    String getActiveNetwork();              // GET /api/network/active
    
    // Network scanning and discovery
    String scanNetworks();                  // POST /api/network/scan
    String getAvailableNetworks();          // GET /api/network/available
    String getNetworkMetrics(const String& ssid); // GET /api/network/metrics?ssid=
    
    // Health monitoring endpoints
    String getHealthMetrics();              // GET /api/health/metrics
    String getHealthStatus();               // GET /api/health/status
    String getHealthAlerts();               // GET /api/health/alerts
    String getHealthIssues();               // GET /api/health/issues
    String getDiagnosticReport();           // GET /api/health/report
    
    // Network control endpoints
    String connectToNetwork(const String& ssid, const String& password); // POST /api/network/connect
    String disconnectNetwork();             // POST /api/network/disconnect
    String switchToWiFi();                  // POST /api/network/switch/wifi
    String switchToLoRa();                  // POST /api/network/switch/lora
    String enableAutoFallback(bool enable); // POST /api/network/fallback/auto
    
    // Configuration endpoints
    String getConfiguration();              // GET /api/config
    String setConfiguration(const String& config); // POST /api/config
    String resetConfiguration();            // POST /api/config/reset
    
    // Statistics endpoints
    String getStatistics();                 // GET /api/stats
    String resetStatistics();               // POST /api/stats/reset
    
    // Command execution
    String executeCommand(const String& command, const String& params = "");
    
private:
    // Component references
    WiFiManager* wifiManager_;
    WiFiAPSelector* apSelector_;
    WiFiLoRaFallback* fallback_;
    NetworkHealthMonitor* healthMonitor_;
    
    // Internal state
    bool initialized_;
    
    // Helper methods for JSON formatting
    String createSuccessResponse(const JsonDocument& data);
    String createSuccessResponse(const String& message);
    String createErrorResponse(const String& error, int code = 400);
    String formatWiFiStatus(const WiFiStatusInfo& status);
    String formatNetworkMetrics(const NetworkHealthMetrics& metrics);
    String formatAPInfo(const APInfo& ap);
    String formatNetworkIssue(const NetworkIssue& issue);
    String formatHealthAlert(const HealthAlert& alert);
    
    // Configuration helpers
    bool validateConfiguration(const JsonDocument& config);
    void applyConfiguration(const JsonDocument& config);
};

// Helper functions for route handling
String handleAPIRequest(const String& method, const String& path, 
                       const String& body = "");
bool registerAPIRoutes();

#endif // NETWORK_API_H
