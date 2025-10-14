/**
 * @file network_health_monitor.cpp
 * @brief Implementation of network health monitoring system
 */

#include "network_health_monitor.h"
#include "debug_utils.h"
#include <algorithm>

// Global instance
NetworkHealthMonitor* g_networkHealthMonitor = nullptr;

// Constructor
NetworkHealthMonitor::NetworkHealthMonitor()
    : initialized_(false),
      lastUpdate_(0),
      lastLatencyTest_(0),
      lastThroughputTest_(0),
      totalTransmissions_(0),
      totalBytes_(0),
      uptimeStart_(0),
      totalUptime_(0),
      totalDowntime_(0) {
}

// Destructor
NetworkHealthMonitor::~NetworkHealthMonitor() {
    cleanup();
}

// Initialize monitor
bool NetworkHealthMonitor::init(const MonitorConfig& config) {
    if (initialized_) {
        return true;
    }
    
    DEBUG_PRINTLN("Initializing Network Health Monitor...");
    
    config_ = config;
    uptimeStart_ = millis();
    metrics_.collectionStartTime = uptimeStart_;
    
    initialized_ = true;
    DEBUG_PRINTLN("Network Health Monitor initialized");
    return true;
}

// Cleanup resources
void NetworkHealthMonitor::cleanup() {
    if (initialized_) {
        issues_.clear();
        alerts_.clear();
        latencyHistory_.clear();
        signalHistory_.clear();
        transmissionHistory_.clear();
        initialized_ = false;
    }
}

// Update all metrics
void NetworkHealthMonitor::updateMetrics() {
    if (!initialized_) {
        return;
    }
    
    unsigned long now = millis();
    
    if (now - lastUpdate_ < config_.updateInterval) {
        return; // Not time yet
    }
    
    collectMetrics();
    
    if (config_.enablePredictiveAnalysis) {
        analyzeLatencyTrends();
        analyzeSignalTrends();
    }
    
    checkThresholds();
    detectNetworkIssues();
    
    metrics_.lastUpdateTime = now;
    lastUpdate_ = now;
}

// Collect current metrics
void NetworkHealthMonitor::collectMetrics() {
    // Calculate uptime percentage
    unsigned long now = millis();
    unsigned long totalTime = now - uptimeStart_;
    if (totalTime > 0) {
        metrics_.networkUptimePercentage = 
            (static_cast<float>(totalUptime_) / totalTime) * 100.0f;
    }
    
    // Update packet loss rate
    updatePacketLoss();
    
    // Update efficiency
    updateEfficiency();
}

// Record transmission result
void NetworkHealthMonitor::recordTransmission(bool success, size_t bytes, 
                                             uint32_t latencyMs) {
    if (!initialized_) {
        return;
    }
    
    totalTransmissions_++;
    
    if (success) {
        metrics_.successfulTransmissions++;
        totalBytes_ += bytes;
        
        if (latencyMs > 0) {
            updateAverageLatency(latencyMs);
        }
    } else {
        metrics_.failedTransmissions++;
    }
    
    // Add to transmission history
    addToHistory(transmissionHistory_, success);
    
    // Update uptime tracking
    if (success) {
        totalUptime_ += config_.updateInterval;
    } else {
        totalDowntime_ += config_.updateInterval;
    }
}

// Record signal strength
void NetworkHealthMonitor::recordSignalStrength(float rssi) {
    if (!initialized_) {
        return;
    }
    
    metrics_.signalStrengthDbm = rssi;
    addToHistory(signalHistory_, rssi);
}

// Record latency measurement
void NetworkHealthMonitor::recordLatency(uint32_t latencyMs) {
    if (!initialized_) {
        return;
    }
    
    updateAverageLatency(latencyMs);
    addToHistory(latencyHistory_, latencyMs);
}

// Update topology metrics
void NetworkHealthMonitor::updateTopologyMetrics(uint8_t nodes, uint8_t diameter, 
                                                float density) {
    if (!initialized_) {
        return;
    }
    
    metrics_.activeNodes = nodes;
    metrics_.networkDiameter = diameter;
    metrics_.networkDensity = density;
}

// Get current metrics
NetworkHealthMetrics NetworkHealthMonitor::getCurrentMetrics() const {
    return metrics_;
}

// Get uptime percentage
float NetworkHealthMonitor::getUptimePercentage() const {
    return metrics_.networkUptimePercentage;
}

// Get packet loss rate
float NetworkHealthMonitor::getPacketLossRate() const {
    return metrics_.packetLossRate;
}

// Get average latency
uint32_t NetworkHealthMonitor::getAverageLatency() const {
    return metrics_.averageLatencyMs;
}

// Get throughput
float NetworkHealthMonitor::getThroughput() const {
    return metrics_.throughputKbps;
}

// Get network efficiency
float NetworkHealthMonitor::getNetworkEfficiency() const {
    return metrics_.networkEfficiency;
}

// Check if network is healthy
bool NetworkHealthMonitor::isHealthy() const {
    return getOverallHealthScore() >= 70.0f;
}

// Calculate overall health score
float NetworkHealthMonitor::getOverallHealthScore() const {
    return calculateHealthScore();
}

// Get health status string
String NetworkHealthMonitor::getHealthStatus() const {
    float score = getOverallHealthScore();
    
    if (score >= 90.0f) {
        return "Excellent";
    } else if (score >= 75.0f) {
        return "Good";
    } else if (score >= 60.0f) {
        return "Fair";
    } else if (score >= 40.0f) {
        return "Poor";
    } else {
        return "Critical";
    }
}

// Get health warnings
std::vector<String> NetworkHealthMonitor::getHealthWarnings() const {
    std::vector<String> warnings;
    
    if (metrics_.packetLossRate > config_.packetLossThreshold) {
        warnings.push_back("High packet loss rate");
    }
    
    if (metrics_.averageLatencyMs > config_.latencyThreshold) {
        warnings.push_back("High network latency");
    }
    
    if (metrics_.signalStrengthDbm < -80.0f) {
        warnings.push_back("Weak signal strength");
    }
    
    if (metrics_.networkEfficiency < 0.5f) {
        warnings.push_back("Low network efficiency");
    }
    
    return warnings;
}

// Detect network issues
bool NetworkHealthMonitor::detectNetworkIssues() {
    bool issuesDetected = false;
    
    // Check for high packet loss
    if (metrics_.packetLossRate > config_.packetLossThreshold) {
        detectIssue(SEVERITY_WARNING, "High packet loss detected", "Network");
        issuesDetected = true;
    }
    
    // Check for high latency
    if (metrics_.averageLatencyMs > config_.latencyThreshold) {
        detectIssue(SEVERITY_WARNING, "High latency detected", "Network");
        issuesDetected = true;
    }
    
    // Check for weak signal
    if (metrics_.signalStrengthDbm < -85.0f) {
        detectIssue(SEVERITY_ERROR, "Weak signal strength", "Radio");
        issuesDetected = true;
    }
    
    return issuesDetected;
}

// Get active issues
std::vector<NetworkIssue> NetworkHealthMonitor::getActiveIssues() const {
    std::vector<NetworkIssue> active;
    for (const auto& issue : issues_) {
        if (!issue.resolved) {
            active.push_back(issue);
        }
    }
    return active;
}

// Get resolved issues
std::vector<NetworkIssue> NetworkHealthMonitor::getResolvedIssues() const {
    std::vector<NetworkIssue> resolved;
    for (const auto& issue : issues_) {
        if (issue.resolved) {
            resolved.push_back(issue);
        }
    }
    return resolved;
}

// Acknowledge issue
void NetworkHealthMonitor::acknowledgeIssue(size_t issueIndex) {
    if (issueIndex < issues_.size()) {
        issues_[issueIndex].resolved = true;
        issues_[issueIndex].resolvedTime = millis();
    }
}

// Clear resolved issues
void NetworkHealthMonitor::clearResolvedIssues() {
    issues_.erase(
        std::remove_if(issues_.begin(), issues_.end(),
                      [](const NetworkIssue& issue) { return issue.resolved; }),
        issues_.end()
    );
}

// Get active alerts
std::vector<HealthAlert> NetworkHealthMonitor::getActiveAlerts() const {
    std::vector<HealthAlert> active;
    for (const auto& alert : alerts_) {
        if (!alert.acknowledged) {
            active.push_back(alert);
        }
    }
    return active;
}

// Acknowledge alert
void NetworkHealthMonitor::acknowledgeAlert(size_t alertIndex) {
    if (alertIndex < alerts_.size()) {
        alerts_[alertIndex].acknowledged = true;
    }
}

// Clear acknowledged alerts
void NetworkHealthMonitor::clearAcknowledgedAlerts() {
    alerts_.erase(
        std::remove_if(alerts_.begin(), alerts_.end(),
                      [](const HealthAlert& alert) { return alert.acknowledged; }),
        alerts_.end()
    );
}

// Get diagnostic report
String NetworkHealthMonitor::getDiagnosticReport() const {
    String report = "=== Network Health Diagnostic Report ===\n";
    report += "Overall Health: " + getHealthStatus() + 
              " (" + String(getOverallHealthScore(), 1) + "/100)\n";
    report += "Uptime: " + String(metrics_.networkUptimePercentage, 2) + "%\n";
    report += "Packet Loss: " + String(metrics_.packetLossRate * 100, 2) + "%\n";
    report += "Avg Latency: " + String(metrics_.averageLatencyMs) + " ms\n";
    report += "Signal: " + String(metrics_.signalStrengthDbm, 1) + " dBm\n";
    report += "Throughput: " + String(metrics_.throughputKbps, 2) + " Kbps\n";
    report += "Active Issues: " + String(getActiveIssues().size()) + "\n";
    report += "Active Alerts: " + String(getActiveAlerts().size()) + "\n";
    return report;
}

// Get metrics summary
String NetworkHealthMonitor::getMetricsSummary() const {
    String summary = "Network Metrics:\n";
    summary += "  Success: " + String(metrics_.successfulTransmissions) + "\n";
    summary += "  Failed: " + String(metrics_.failedTransmissions) + "\n";
    summary += "  Loss Rate: " + String(metrics_.packetLossRate * 100, 2) + "%\n";
    summary += "  Efficiency: " + String(metrics_.networkEfficiency * 100, 2) + "%\n";
    return summary;
}

// Print metrics to debug output
void NetworkHealthMonitor::printMetrics() const {
    DEBUG_PRINTLN(getDiagnosticReport());
}

// Print issues to debug output
void NetworkHealthMonitor::printIssues() const {
    auto activeIssues = getActiveIssues();
    DEBUG_PRINTF("Active Issues: %d\n", activeIssues.size());
    for (const auto& issue : activeIssues) {
        DEBUG_PRINTF("  [%s] %s: %s\n", 
                    issue.component.c_str(),
                    (issue.severity == SEVERITY_CRITICAL ? "CRITICAL" :
                     issue.severity == SEVERITY_ERROR ? "ERROR" :
                     issue.severity == SEVERITY_WARNING ? "WARNING" : "INFO"),
                    issue.description.c_str());
    }
}

// Get diagnostic recommendations
std::vector<String> NetworkHealthMonitor::getDiagnosticRecommendations() const {
    std::vector<String> recommendations;
    
    if (metrics_.packetLossRate > 0.2f) {
        recommendations.push_back("Consider relocating device for better signal");
    }
    
    if (metrics_.averageLatencyMs > 2000) {
        recommendations.push_back("Network congestion detected - reduce transmission frequency");
    }
    
    if (metrics_.signalStrengthDbm < -85.0f) {
        recommendations.push_back("Signal strength critical - check antenna");
    }
    
    if (metrics_.networkEfficiency < 0.3f) {
        recommendations.push_back("Low efficiency - optimize transmission protocols");
    }
    
    return recommendations;
}

// Predict network failures
bool NetworkHealthMonitor::predictNetworkFailures() {
    if (!config_.enablePredictiveAnalysis) {
        return false;
    }
    
    // Simple prediction based on trends
    bool latencyIncreasing = isLatencyIncreasing();
    bool signalDeteriorating = isSignalDeteriorating();
    
    if (latencyIncreasing && signalDeteriorating) {
        createAlert("Predictive", "Network failure likely within 30 minutes", 
                   SEVERITY_WARNING);
        return true;
    }
    
    return false;
}

// Get failure probability
float NetworkHealthMonitor::getFailureProbability() const {
    float prob = 0.0f;
    
    // Factor in packet loss
    prob += metrics_.packetLossRate * 0.3f;
    
    // Factor in signal strength
    if (metrics_.signalStrengthDbm < -85.0f) {
        prob += 0.3f;
    }
    
    // Factor in latency
    if (metrics_.averageLatencyMs > 2000) {
        prob += 0.2f;
    }
    
    // Factor in efficiency
    prob += (1.0f - metrics_.networkEfficiency) * 0.2f;
    
    return std::min(prob, 1.0f);
}

// Set configuration
void NetworkHealthMonitor::setConfig(const MonitorConfig& config) {
    config_ = config;
}

// Get configuration
MonitorConfig NetworkHealthMonitor::getConfig() const {
    return config_;
}

// Reset statistics
void NetworkHealthMonitor::resetStatistics() {
    metrics_ = NetworkHealthMetrics();
    totalTransmissions_ = 0;
    totalBytes_ = 0;
    totalUptime_ = 0;
    totalDowntime_ = 0;
    uptimeStart_ = millis();
    metrics_.collectionStartTime = uptimeStart_;
    
    latencyHistory_.clear();
    signalHistory_.clear();
    transmissionHistory_.clear();
}

// Update average latency
void NetworkHealthMonitor::updateAverageLatency(uint32_t latencyMs) {
    if (metrics_.averageLatencyMs == 0) {
        metrics_.averageLatencyMs = latencyMs;
    } else {
        // Exponential moving average
        metrics_.averageLatencyMs = 
            (metrics_.averageLatencyMs * 4 + latencyMs) / 5;
    }
    
    if (latencyMs > metrics_.peakLatencyMs) {
        metrics_.peakLatencyMs = latencyMs;
    }
}

// Update packet loss rate
void NetworkHealthMonitor::updatePacketLoss() {
    if (totalTransmissions_ > 0) {
        metrics_.packetLossRate = 
            static_cast<float>(metrics_.failedTransmissions) / totalTransmissions_;
    }
}

// Update network efficiency
void NetworkHealthMonitor::updateEfficiency() {
    if (totalTransmissions_ > 0) {
        metrics_.networkEfficiency = 
            static_cast<float>(metrics_.successfulTransmissions) / totalTransmissions_;
    }
}

// Detect and record issue
void NetworkHealthMonitor::detectIssue(IssueSeverity severity, 
                                      const String& description,
                                      const String& component) {
    // Check if issue already exists
    for (auto& issue : issues_) {
        if (!issue.resolved && issue.description == description) {
            return; // Already tracking this issue
        }
    }
    
    NetworkIssue issue;
    issue.severity = severity;
    issue.description = description;
    issue.component = component;
    issue.detectedTime = millis();
    
    issues_.push_back(issue);
    
    // Also create an alert
    createAlert("Issue", description, severity);
}

// Create health alert
void NetworkHealthMonitor::createAlert(const String& type, const String& message,
                                      IssueSeverity severity) {
    HealthAlert alert;
    alert.alertType = type;
    alert.message = message;
    alert.severity = severity;
    alert.timestamp = millis();
    
    alerts_.push_back(alert);
}

// Check threshold violations
void NetworkHealthMonitor::checkThresholds() {
    if (metrics_.packetLossRate > config_.packetLossThreshold) {
        createAlert("Threshold", "Packet loss threshold exceeded", SEVERITY_WARNING);
    }
    
    if (metrics_.averageLatencyMs > config_.latencyThreshold) {
        createAlert("Threshold", "Latency threshold exceeded", SEVERITY_WARNING);
    }
}

// Analyze latency trends
void NetworkHealthMonitor::analyzeLatencyTrends() {
    if (latencyHistory_.size() < 10) {
        return; // Not enough data
    }
    
    if (isLatencyIncreasing()) {
        createAlert("Trend", "Latency is increasing", SEVERITY_INFO);
    }
}

// Analyze signal trends
void NetworkHealthMonitor::analyzeSignalTrends() {
    if (signalHistory_.size() < 10) {
        return; // Not enough data
    }
    
    if (isSignalDeteriorating()) {
        createAlert("Trend", "Signal strength is deteriorating", SEVERITY_INFO);
    }
}

// Calculate health score
float NetworkHealthMonitor::calculateHealthScore() const {
    float score = 100.0f;
    
    // Deduct for packet loss (max 30 points)
    score -= metrics_.packetLossRate * 30.0f;
    
    // Deduct for latency (max 20 points)
    if (metrics_.averageLatencyMs > config_.latencyThreshold) {
        score -= 20.0f;
    }
    
    // Deduct for signal strength (max 25 points)
    if (metrics_.signalStrengthDbm < -85.0f) {
        score -= 25.0f;
    } else if (metrics_.signalStrengthDbm < -75.0f) {
        score -= 15.0f;
    }
    
    // Deduct for efficiency (max 25 points)
    score -= (1.0f - metrics_.networkEfficiency) * 25.0f;
    
    return std::max(0.0f, score);
}

// Check if latency is increasing
bool NetworkHealthMonitor::isLatencyIncreasing() const {
    if (latencyHistory_.size() < 10) {
        return false;
    }
    
    // Compare recent average to older average
    size_t halfSize = latencyHistory_.size() / 2;
    float recentAvg = 0.0f;
    float olderAvg = 0.0f;
    
    for (size_t i = halfSize; i < latencyHistory_.size(); i++) {
        recentAvg += latencyHistory_[i];
    }
    recentAvg /= halfSize;
    
    for (size_t i = 0; i < halfSize; i++) {
        olderAvg += latencyHistory_[i];
    }
    olderAvg /= halfSize;
    
    return recentAvg > olderAvg * 1.2f; // 20% increase
}

// Check if signal is deteriorating
bool NetworkHealthMonitor::isSignalDeteriorating() const {
    if (signalHistory_.size() < 10) {
        return false;
    }
    
    // Compare recent average to older average
    size_t halfSize = signalHistory_.size() / 2;
    float recentAvg = 0.0f;
    float olderAvg = 0.0f;
    
    for (size_t i = halfSize; i < signalHistory_.size(); i++) {
        recentAvg += signalHistory_[i];
    }
    recentAvg /= halfSize;
    
    for (size_t i = 0; i < halfSize; i++) {
        olderAvg += signalHistory_[i];
    }
    olderAvg /= halfSize;
    
    return recentAvg < olderAvg * 0.8f; // 20% decrease
}

// Add value to history with size limit
void NetworkHealthMonitor::addToHistory(std::vector<uint32_t>& history, uint32_t value) {
    if (history.size() >= HISTORY_SIZE) {
        history.erase(history.begin());
    }
    history.push_back(value);
}

void NetworkHealthMonitor::addToHistory(std::vector<float>& history, float value) {
    if (history.size() >= HISTORY_SIZE) {
        history.erase(history.begin());
    }
    history.push_back(value);
}

void NetworkHealthMonitor::addToHistory(std::vector<bool>& history, bool value) {
    if (history.size() >= HISTORY_SIZE) {
        history.erase(history.begin());
    }
    history.push_back(value);
}

// Utility function implementations
bool initializeNetworkHealthMonitor(const MonitorConfig& config) {
    if (!g_networkHealthMonitor) {
        g_networkHealthMonitor = new NetworkHealthMonitor();
    }
    return g_networkHealthMonitor->init(config);
}

void cleanupNetworkHealthMonitor() {
    if (g_networkHealthMonitor) {
        g_networkHealthMonitor->cleanup();
        delete g_networkHealthMonitor;
        g_networkHealthMonitor = nullptr;
    }
}

NetworkHealthMetrics getCurrentNetworkHealth() {
    if (g_networkHealthMonitor) {
        return g_networkHealthMonitor->getCurrentMetrics();
    }
    return NetworkHealthMetrics();
}

bool isNetworkHealthy() {
    if (g_networkHealthMonitor) {
        return g_networkHealthMonitor->isHealthy();
    }
    return false;
}

String getNetworkHealthReport() {
    if (g_networkHealthMonitor) {
        return g_networkHealthMonitor->getDiagnosticReport();
    }
    return "Monitor not initialized";
}
