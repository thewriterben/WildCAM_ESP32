/**
 * @file network_api.cpp
 * @brief Implementation of Network REST API
 */

#include "network_api.h"
#include "debug_utils.h"

// Constructor
NetworkAPI::NetworkAPI()
    : wifiManager_(nullptr),
      apSelector_(nullptr),
      fallback_(nullptr),
      healthMonitor_(nullptr),
      initialized_(false) {
}

// Destructor
NetworkAPI::~NetworkAPI() {
    cleanup();
}

// Initialize API
bool NetworkAPI::init(WiFiManager* wifiMgr, 
                     WiFiAPSelector* apSelector,
                     WiFiLoRaFallback* fallback,
                     NetworkHealthMonitor* healthMonitor) {
    if (initialized_) {
        return true;
    }
    
    if (!wifiMgr || !apSelector || !fallback || !healthMonitor) {
        DEBUG_PRINTLN("All components required for Network API");
        return false;
    }
    
    wifiManager_ = wifiMgr;
    apSelector_ = apSelector;
    fallback_ = fallback;
    healthMonitor_ = healthMonitor;
    
    initialized_ = true;
    DEBUG_PRINTLN("Network API initialized");
    return true;
}

// Cleanup API
void NetworkAPI::cleanup() {
    if (initialized_) {
        wifiManager_ = nullptr;
        apSelector_ = nullptr;
        fallback_ = nullptr;
        healthMonitor_ = nullptr;
        initialized_ = false;
    }
}

// Get network status
String NetworkAPI::getNetworkStatus() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(1024);
    
    NetworkState state = fallback_->getNetworkState();
    
    doc["activeNetwork"] = state.activeNetwork == NETWORK_WIFI ? "wifi" :
                          state.activeNetwork == NETWORK_LORA ? "lora" :
                          state.activeNetwork == NETWORK_BOTH ? "both" : "none";
    doc["connected"] = fallback_->isConnected();
    doc["healthy"] = fallback_->isHealthy();
    doc["signalStrength"] = fallback_->getSignalStrength();
    doc["totalSwitches"] = state.totalSwitches;
    
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["connected"] = state.wifiConnected;
    wifi["health"] = (int)state.wifiHealth;
    wifi["rssi"] = state.wifiRSSI;
    wifi["uptime"] = state.wifiUptime;
    
    JsonObject lora = doc.createNestedObject("lora");
    lora["connected"] = state.loraConnected;
    lora["health"] = (int)state.loraHealth;
    lora["rssi"] = state.loraRSSI;
    lora["uptime"] = state.loraUptime;
    
    return createSuccessResponse(doc);
}

// Get WiFi status
String NetworkAPI::getWiFiStatus() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    WiFiStatusInfo status = wifiManager_->getStatus();
    return formatWiFiStatus(status);
}

// Get LoRa status
String NetworkAPI::getLoRaStatus() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(512);
    
    MeshNetworkStatus loraStatus = LoraMesh::getNetworkStatus();
    
    doc["initialized"] = loraStatus.initialized;
    doc["nodeId"] = loraStatus.nodeId;
    doc["connectedNodes"] = loraStatus.connectedNodes;
    doc["rssi"] = loraStatus.rssi;
    doc["snr"] = loraStatus.snr;
    doc["packetsReceived"] = loraStatus.packetsReceived;
    doc["packetsSent"] = loraStatus.packetsSent;
    doc["lastMessageTime"] = loraStatus.lastMessageTime;
    
    return createSuccessResponse(doc);
}

// Get active network
String NetworkAPI::getActiveNetwork() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(256);
    
    ActiveNetwork active = fallback_->getActiveNetwork();
    doc["active"] = active == NETWORK_WIFI ? "wifi" :
                   active == NETWORK_LORA ? "lora" :
                   active == NETWORK_BOTH ? "both" : "none";
    doc["connected"] = fallback_->isConnected();
    
    return createSuccessResponse(doc);
}

// Scan networks
String NetworkAPI::scanNetworks() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    if (!apSelector_->scanNetworks()) {
        return createErrorResponse("Network scan failed", 500);
    }
    
    DynamicJsonDocument doc(256);
    doc["message"] = "Network scan started";
    doc["count"] = apSelector_->getScannedNetworkCount();
    
    return createSuccessResponse(doc);
}

// Get available networks
String NetworkAPI::getAvailableNetworks() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(2048);
    
    auto networks = apSelector_->getAvailableNetworks();
    JsonArray networksArray = doc.createNestedArray("networks");
    
    for (const auto& ap : networks) {
        JsonObject net = networksArray.createNestedObject();
        net["ssid"] = ap.ssid;
        net["bssid"] = ap.bssid;
        net["rssi"] = ap.rssi;
        net["channel"] = ap.channel;
        net["encryption"] = ap.encryption;
        
        NetworkMetrics metrics = apSelector_->calculateMetrics(ap);
        JsonObject metricsObj = net.createNestedObject("metrics");
        metricsObj["signalQuality"] = metrics.signalQuality;
        metricsObj["performanceScore"] = metrics.performanceScore;
        metricsObj["estimatedThroughput"] = metrics.estimatedThroughput;
        metricsObj["stable"] = metrics.isStable;
    }
    
    return createSuccessResponse(doc);
}

// Get network metrics for specific SSID
String NetworkAPI::getNetworkMetrics(const String& ssid) {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    auto networks = apSelector_->getAvailableNetworks();
    for (const auto& ap : networks) {
        if (ap.ssid == ssid) {
            DynamicJsonDocument doc(512);
            NetworkMetrics metrics = apSelector_->calculateMetrics(ap);
            
            doc["ssid"] = ssid;
            doc["signalQuality"] = metrics.signalQuality;
            doc["connectionReliability"] = metrics.connectionReliability;
            doc["performanceScore"] = metrics.performanceScore;
            doc["estimatedThroughput"] = metrics.estimatedThroughput;
            doc["stable"] = metrics.isStable;
            
            return createSuccessResponse(doc);
        }
    }
    
    return createErrorResponse("Network not found", 404);
}

// Get health metrics
String NetworkAPI::getHealthMetrics() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    NetworkHealthMetrics metrics = healthMonitor_->getCurrentMetrics();
    return formatNetworkMetrics(metrics);
}

// Get health status
String NetworkAPI::getHealthStatus() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(512);
    
    doc["healthy"] = healthMonitor_->isHealthy();
    doc["status"] = healthMonitor_->getHealthStatus();
    doc["score"] = healthMonitor_->getOverallHealthScore();
    doc["uptimePercentage"] = healthMonitor_->getUptimePercentage();
    doc["packetLossRate"] = healthMonitor_->getPacketLossRate();
    doc["averageLatency"] = healthMonitor_->getAverageLatency();
    
    JsonArray warnings = doc.createNestedArray("warnings");
    for (const auto& warning : healthMonitor_->getHealthWarnings()) {
        warnings.add(warning);
    }
    
    return createSuccessResponse(doc);
}

// Get health alerts
String NetworkAPI::getHealthAlerts() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(2048);
    
    auto alerts = healthMonitor_->getActiveAlerts();
    JsonArray alertsArray = doc.createNestedArray("alerts");
    
    for (const auto& alert : alerts) {
        alertsArray.add(formatHealthAlert(alert));
    }
    
    return createSuccessResponse(doc);
}

// Get health issues
String NetworkAPI::getHealthIssues() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(2048);
    
    auto issues = healthMonitor_->getActiveIssues();
    JsonArray issuesArray = doc.createNestedArray("issues");
    
    for (const auto& issue : issues) {
        issuesArray.add(formatNetworkIssue(issue));
    }
    
    return createSuccessResponse(doc);
}

// Get diagnostic report
String NetworkAPI::getDiagnosticReport() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(1024);
    
    doc["report"] = healthMonitor_->getDiagnosticReport();
    doc["timestamp"] = millis();
    
    JsonArray recommendations = doc.createNestedArray("recommendations");
    for (const auto& rec : healthMonitor_->getDiagnosticRecommendations()) {
        recommendations.add(rec);
    }
    
    return createSuccessResponse(doc);
}

// Connect to network
String NetworkAPI::connectToNetwork(const String& ssid, const String& password) {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    if (ssid.isEmpty()) {
        return createErrorResponse("SSID required", 400);
    }
    
    bool success = wifiManager_->connect(ssid, password);
    
    DynamicJsonDocument doc(256);
    doc["success"] = success;
    doc["ssid"] = ssid;
    
    if (success) {
        doc["message"] = "Connected successfully";
        return createSuccessResponse(doc);
    } else {
        return createErrorResponse("Connection failed", 500);
    }
}

// Disconnect network
String NetworkAPI::disconnectNetwork() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    fallback_->disconnect();
    return createSuccessResponse("Disconnected");
}

// Switch to WiFi
String NetworkAPI::switchToWiFi() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    bool success = fallback_->switchToWiFi(REASON_MANUAL);
    
    if (success) {
        return createSuccessResponse("Switched to WiFi");
    } else {
        return createErrorResponse("Failed to switch to WiFi", 500);
    }
}

// Switch to LoRa
String NetworkAPI::switchToLoRa() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    bool success = fallback_->switchToLoRa(REASON_MANUAL);
    
    if (success) {
        return createSuccessResponse("Switched to LoRa");
    } else {
        return createErrorResponse("Failed to switch to LoRa", 500);
    }
}

// Enable auto fallback
String NetworkAPI::enableAutoFallback(bool enable) {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    fallback_->setAutoFallback(enable);
    
    DynamicJsonDocument doc(256);
    doc["autoFallback"] = enable;
    doc["message"] = enable ? "Auto fallback enabled" : "Auto fallback disabled";
    
    return createSuccessResponse(doc);
}

// Get configuration
String NetworkAPI::getConfiguration() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(1024);
    
    FallbackConfig config = fallback_->getConfig();
    JsonObject fallbackConfig = doc.createNestedObject("fallback");
    fallbackConfig["autoFallbackEnabled"] = config.autoFallbackEnabled;
    fallbackConfig["preferWiFi"] = config.preferWiFi;
    fallbackConfig["wifiRSSIThreshold"] = config.wifiRSSIThreshold;
    fallbackConfig["loraRSSIThreshold"] = config.loraRSSIThreshold;
    fallbackConfig["connectionTimeout"] = config.connectionTimeout;
    fallbackConfig["healthCheckInterval"] = config.healthCheckInterval;
    
    MonitorConfig monitorConfig = healthMonitor_->getConfig();
    JsonObject healthConfig = doc.createNestedObject("healthMonitor");
    healthConfig["updateInterval"] = monitorConfig.updateInterval;
    healthConfig["enablePredictiveAnalysis"] = monitorConfig.enablePredictiveAnalysis;
    healthConfig["packetLossThreshold"] = monitorConfig.packetLossThreshold;
    healthConfig["latencyThreshold"] = monitorConfig.latencyThreshold;
    
    return createSuccessResponse(doc);
}

// Get statistics
String NetworkAPI::getStatistics() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    DynamicJsonDocument doc(1024);
    
    doc["wifiUptime"] = fallback_->getWiFiUptime();
    doc["loraUptime"] = fallback_->getLoRaUptime();
    doc["totalSwitches"] = fallback_->getTotalSwitches();
    doc["wifiReliability"] = fallback_->getWiFiReliability();
    doc["loraReliability"] = fallback_->getLoRaReliability();
    
    NetworkHealthMetrics metrics = healthMonitor_->getCurrentMetrics();
    doc["successfulTransmissions"] = metrics.successfulTransmissions;
    doc["failedTransmissions"] = metrics.failedTransmissions;
    doc["averageLatency"] = metrics.averageLatencyMs;
    doc["throughput"] = metrics.throughputKbps;
    
    return createSuccessResponse(doc);
}

// Reset statistics
String NetworkAPI::resetStatistics() {
    if (!initialized_) {
        return createErrorResponse("API not initialized", 500);
    }
    
    fallback_->resetStatistics();
    healthMonitor_->resetStatistics();
    
    return createSuccessResponse("Statistics reset");
}

// Create success response
String NetworkAPI::createSuccessResponse(const JsonDocument& data) {
    DynamicJsonDocument doc(data.capacity() + 128);
    doc["success"] = true;
    doc["data"] = data;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    return response;
}

String NetworkAPI::createSuccessResponse(const String& message) {
    DynamicJsonDocument doc(256);
    doc["success"] = true;
    doc["message"] = message;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    return response;
}

// Create error response
String NetworkAPI::createErrorResponse(const String& error, int code) {
    DynamicJsonDocument doc(256);
    doc["success"] = false;
    doc["error"] = error;
    doc["code"] = code;
    doc["timestamp"] = millis();
    
    String response;
    serializeJson(doc, response);
    return response;
}

// Format WiFi status
String NetworkAPI::formatWiFiStatus(const WiFiStatusInfo& status) {
    DynamicJsonDocument doc(512);
    
    doc["initialized"] = status.initialized;
    doc["status"] = (int)status.status;
    doc["ssid"] = status.ssid;
    doc["ipAddress"] = status.ipAddress;
    doc["rssi"] = status.rssi;
    doc["connectionTime"] = status.connectionTime;
    doc["reconnectAttempts"] = status.reconnectAttempts;
    doc["apMode"] = status.apMode;
    
    return createSuccessResponse(doc);
}

// Format network metrics
String NetworkAPI::formatNetworkMetrics(const NetworkHealthMetrics& metrics) {
    DynamicJsonDocument doc(1024);
    
    doc["networkUptimePercentage"] = metrics.networkUptimePercentage;
    doc["successfulTransmissions"] = metrics.successfulTransmissions;
    doc["failedTransmissions"] = metrics.failedTransmissions;
    doc["packetLossRate"] = metrics.packetLossRate;
    doc["averageLatencyMs"] = metrics.averageLatencyMs;
    doc["peakLatencyMs"] = metrics.peakLatencyMs;
    doc["throughputKbps"] = metrics.throughputKbps;
    doc["networkEfficiency"] = metrics.networkEfficiency;
    doc["signalStrengthDbm"] = metrics.signalStrengthDbm;
    doc["activeNodes"] = metrics.activeNodes;
    doc["networkDiameter"] = metrics.networkDiameter;
    doc["networkDensity"] = metrics.networkDensity;
    
    return createSuccessResponse(doc);
}

// Format network issue
String NetworkAPI::formatNetworkIssue(const NetworkIssue& issue) {
    DynamicJsonDocument doc(256);
    
    doc["severity"] = (int)issue.severity;
    doc["description"] = issue.description;
    doc["component"] = issue.component;
    doc["detectedTime"] = issue.detectedTime;
    doc["resolved"] = issue.resolved;
    if (issue.resolved) {
        doc["resolvedTime"] = issue.resolvedTime;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

// Format health alert
String NetworkAPI::formatHealthAlert(const HealthAlert& alert) {
    DynamicJsonDocument doc(256);
    
    doc["alertType"] = alert.alertType;
    doc["message"] = alert.message;
    doc["severity"] = (int)alert.severity;
    doc["timestamp"] = alert.timestamp;
    doc["acknowledged"] = alert.acknowledged;
    
    String result;
    serializeJson(doc, result);
    return result;
}
