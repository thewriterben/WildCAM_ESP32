/**
 * @file federated_web_interface.h
 * @brief Web Interface Extensions for Federated Learning
 * 
 * Extends the existing web interface to provide federated learning
 * monitoring, configuration, and control capabilities.
 */

#ifndef FEDERATED_WEB_INTERFACE_H
#define FEDERATED_WEB_INTERFACE_H

#include "federated_learning/federated_learning_system.h"
#include "ai_federated_integration.h"
#include "../web/web_server.h"
#include <Arduino.h>

/**
 * Web Interface Configuration for Federated Learning
 */
struct FederatedWebConfig {
    bool enableFederatedDashboard;
    bool enableConfigurationUI;
    bool enableRealTimeMetrics;
    bool enableExpertValidation;
    bool requireAuthentication;
    String authUsername;
    String authPassword;
    uint32_t metricsUpdateInterval;
    
    FederatedWebConfig() : enableFederatedDashboard(true),
                          enableConfigurationUI(true),
                          enableRealTimeMetrics(true),
                          enableExpertValidation(true),
                          requireAuthentication(true),
                          authUsername("admin"),
                          authPassword("federated"),
                          metricsUpdateInterval(5000) {}
};

/**
 * Federated Learning Web Interface Extension
 * 
 * Provides web-based interface for federated learning:
 * - Real-time dashboard with federated learning metrics
 * - Configuration management interface
 * - Expert validation interface
 * - Network topology visualization
 * - Privacy settings management
 */
class FederatedWebInterface {
public:
    FederatedWebInterface();
    ~FederatedWebInterface();
    
    // Initialization
    bool init(const FederatedWebConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Web server integration
    bool attachToWebServer(WebServer* webServer);
    void detachFromWebServer();
    bool registerEndpoints();
    
    // Dashboard endpoints
    String generateFederatedDashboard();
    String generateConfigurationPage();
    String generateMetricsJSON();
    String generateNetworkTopologyJSON();
    String generatePrivacyStatusJSON();
    
    // API endpoints
    String handleFederatedStatus(const String& method, const String& body);
    String handleConfigurationUpdate(const String& method, const String& body);
    String handleExpertValidation(const String& method, const String& body);
    String handleNetworkControl(const String& method, const String& body);
    String handlePrivacySettings(const String& method, const String& body);
    
    // Real-time updates
    void broadcastMetricsUpdate();
    void broadcastStatusChange(FLSystemStatus oldStatus, FLSystemStatus newStatus);
    void broadcastNetworkChange();
    void broadcastExpertValidationRequest(const String& validationId);
    
    // Configuration management
    bool updateFederatedConfig(const String& configJson);
    String getFederatedConfigAsJSON();
    bool resetToDefaults();
    bool exportConfiguration(const String& filename);
    bool importConfiguration(const String& filename);

private:
    // Configuration
    FederatedWebConfig config_;
    bool initialized_;
    
    // Web server integration
    WebServer* webServer_;
    bool webServerAttached_;
    
    // Component references
    FederatedLearningSystem* federatedSystem_;
    AIFederatedIntegration* aiIntegration_;
    
    // Private methods - Dashboard generation
    String generateDashboardHTML();
    String generateMetricCards();
    String generateNetworkVisualization();
    String generateRecentActivity();
    String generatePrivacyStatus();
    
    // Private methods - Configuration UI
    String generateConfigFormHTML();
    String generateNetworkConfigSection();
    String generatePrivacyConfigSection();
    String generateTrainingConfigSection();
    
    // Private methods - Expert validation
    String generateValidationQueueHTML();
    String generateValidationInterface(const String& validationId);
    bool processValidationSubmission(const String& validationId, const String& decision);
    
    // Private methods - API handling
    bool authenticate(const String& request);
    String formatAPIResponse(bool success, const String& message, const String& data = "");
    String handleGETRequest(const String& endpoint, const String& params);
    String handlePOSTRequest(const String& endpoint, const String& body);
    
    // Private methods - Real-time features
    void setupWebSocket();
    void handleWebSocketMessage(const String& message);
    void sendWebSocketUpdate(const String& type, const String& data);
    
    // Private methods - Utilities
    String escapeHTML(const String& text);
    String formatTimestamp(uint32_t timestamp);
    String formatFileSize(uint32_t bytes);
    String formatDuration(uint32_t milliseconds);
    bool validateJSONConfig(const String& json);
};

// Inline CSS for federated learning dashboard
const char FEDERATED_CSS[] PROGMEM = R"(
.fl-dashboard {
    padding: 20px;
    font-family: Arial, sans-serif;
}

.fl-metrics-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
    margin-bottom: 30px;
}

.fl-metric-card {
    background: #f8f9fa;
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 20px;
    text-align: center;
}

.fl-metric-value {
    font-size: 2em;
    font-weight: bold;
    color: #007bff;
}

.fl-metric-label {
    font-size: 0.9em;
    color: #6c757d;
    margin-top: 5px;
}

.fl-status-active { color: #28a745; }
.fl-status-inactive { color: #6c757d; }
.fl-status-error { color: #dc3545; }

.fl-network-viz {
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 20px;
    margin-bottom: 20px;
    min-height: 300px;
}

.fl-config-section {
    margin-bottom: 25px;
    padding: 20px;
    border: 1px solid #dee2e6;
    border-radius: 8px;
}

.fl-privacy-indicator {
    display: inline-block;
    padding: 4px 8px;
    border-radius: 4px;
    font-size: 0.8em;
    font-weight: bold;
    text-transform: uppercase;
}

.privacy-none { background: #dc3545; color: white; }
.privacy-basic { background: #ffc107; color: black; }
.privacy-standard { background: #17a2b8; color: white; }
.privacy-high { background: #28a745; color: white; }
.privacy-maximum { background: #6f42c1; color: white; }
)";

// Inline JavaScript for real-time updates
const char FEDERATED_JS[] PROGMEM = R"(
class FederatedDashboard {
    constructor() {
        this.ws = null;
        this.metricsUpdateInterval = 5000;
        this.init();
    }
    
    init() {
        this.setupWebSocket();
        this.setupEventListeners();
        this.startMetricsUpdates();
    }
    
    setupWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsUrl = `${protocol}//${window.location.host}/ws`;
        
        this.ws = new WebSocket(wsUrl);
        
        this.ws.onopen = () => {
            console.log('Federated Learning WebSocket connected');
            this.updateConnectionStatus(true);
        };
        
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            this.handleWebSocketMessage(data);
        };
        
        this.ws.onclose = () => {
            console.log('Federated Learning WebSocket disconnected');
            this.updateConnectionStatus(false);
            setTimeout(() => this.setupWebSocket(), 5000);
        };
    }
    
    handleWebSocketMessage(data) {
        switch(data.type) {
            case 'metrics_update':
                this.updateMetrics(data.metrics);
                break;
            case 'status_change':
                this.updateStatus(data.status);
                break;
            case 'network_change':
                this.updateNetworkVisualization(data.network);
                break;
            case 'validation_request':
                this.showValidationRequest(data.validation);
                break;
        }
    }
    
    updateMetrics(metrics) {
        document.getElementById('total-rounds').textContent = metrics.totalRounds;
        document.getElementById('success-rate').textContent = 
            (metrics.successfulRounds / Math.max(metrics.totalRounds, 1) * 100).toFixed(1) + '%';
        document.getElementById('accuracy-improvement').textContent = 
            (metrics.averageAccuracyImprovement * 100).toFixed(2) + '%';
        document.getElementById('privacy-budget').textContent = 
            metrics.remainingPrivacyBudget.toFixed(3);
    }
    
    startMetricsUpdates() {
        setInterval(() => {
            fetch('/api/federated/metrics')
                .then(response => response.json())
                .then(data => this.updateMetrics(data))
                .catch(error => console.error('Metrics update failed:', error));
        }, this.metricsUpdateInterval);
    }
}

// Initialize dashboard when page loads
document.addEventListener('DOMContentLoaded', () => {
    new FederatedDashboard();
});
)";

// Global instance
extern FederatedWebInterface* g_federatedWebInterface;

// Utility functions
bool initializeFederatedWebInterface(const FederatedWebConfig& config);
void cleanupFederatedWebInterface();
bool enableFederatedWebDashboard(bool enable);
String getCurrentFederatedWebStatus();

#endif // FEDERATED_WEB_INTERFACE_H