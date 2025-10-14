/**
 * @file network_health_monitor.h
 * @brief Real-time Network Health Monitoring and Metrics Collection
 * 
 * Provides comprehensive monitoring of network connectivity health,
 * quality metrics, and performance analytics for field deployments.
 */

#ifndef NETWORK_HEALTH_MONITOR_H
#define NETWORK_HEALTH_MONITOR_H

#include <Arduino.h>
#include <vector>

// Network health metrics structure
struct NetworkHealthMetrics {
    // Connectivity metrics
    float networkUptimePercentage;      // 0.0 - 100.0
    uint32_t successfulTransmissions;
    uint32_t failedTransmissions;
    float packetLossRate;               // 0.0 - 1.0
    
    // Performance metrics
    uint32_t averageLatencyMs;
    uint32_t peakLatencyMs;
    float throughputKbps;
    float networkEfficiency;            // 0.0 - 1.0
    
    // Quality metrics
    float signalStrengthDbm;
    float signalToNoiseRatio;
    uint32_t bitErrorRate;
    float jitterMs;
    
    // Topology metrics
    uint8_t activeNodes;
    uint8_t networkDiameter;            // Maximum hops
    float networkDensity;               // Connectivity ratio
    uint8_t criticalNodes;              // Single points of failure
    
    // Time tracking
    unsigned long lastUpdateTime;
    unsigned long collectionStartTime;
    
    NetworkHealthMetrics() :
        networkUptimePercentage(0.0f),
        successfulTransmissions(0),
        failedTransmissions(0),
        packetLossRate(0.0f),
        averageLatencyMs(0),
        peakLatencyMs(0),
        throughputKbps(0.0f),
        networkEfficiency(0.0f),
        signalStrengthDbm(-100.0f),
        signalToNoiseRatio(0.0f),
        bitErrorRate(0),
        jitterMs(0.0f),
        activeNodes(0),
        networkDiameter(0),
        networkDensity(0.0f),
        criticalNodes(0),
        lastUpdateTime(0),
        collectionStartTime(0) {}
};

// Network issue severity
enum IssueSeverity {
    SEVERITY_INFO = 0,
    SEVERITY_WARNING,
    SEVERITY_ERROR,
    SEVERITY_CRITICAL
};

// Network issue structure
struct NetworkIssue {
    IssueSeverity severity;
    String description;
    String component;           // "WiFi", "LoRa", "Cellular", etc.
    unsigned long detectedTime;
    bool resolved;
    unsigned long resolvedTime;
    
    NetworkIssue() : 
        severity(SEVERITY_INFO),
        detectedTime(0),
        resolved(false),
        resolvedTime(0) {}
};

// Health alert structure
struct HealthAlert {
    String alertType;
    String message;
    IssueSeverity severity;
    unsigned long timestamp;
    bool acknowledged;
    
    HealthAlert() :
        severity(SEVERITY_INFO),
        timestamp(0),
        acknowledged(false) {}
};

// Monitoring configuration
struct MonitorConfig {
    uint32_t updateInterval;            // ms between metric updates
    uint32_t latencyTestInterval;       // ms between latency tests
    uint32_t throughputTestInterval;    // ms between throughput tests
    bool enablePredictiveAnalysis;
    bool enableAutoRecovery;
    float packetLossThreshold;          // Threshold for alerts
    uint32_t latencyThreshold;          // ms threshold for alerts
    
    MonitorConfig() :
        updateInterval(10000),          // 10 seconds
        latencyTestInterval(60000),     // 1 minute
        throughputTestInterval(300000), // 5 minutes
        enablePredictiveAnalysis(true),
        enableAutoRecovery(true),
        packetLossThreshold(0.1f),      // 10%
        latencyThreshold(1000) {}        // 1 second
};

/**
 * @class NetworkHealthMonitor
 * @brief Monitors and reports on network health and performance
 */
class NetworkHealthMonitor {
public:
    NetworkHealthMonitor();
    ~NetworkHealthMonitor();
    
    // Initialization
    bool init(const MonitorConfig& config = MonitorConfig());
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Metric collection
    void updateMetrics();
    void recordTransmission(bool success, size_t bytes, uint32_t latencyMs = 0);
    void recordSignalStrength(float rssi);
    void recordLatency(uint32_t latencyMs);
    void updateTopologyMetrics(uint8_t nodes, uint8_t diameter, float density);
    
    // Metric retrieval
    NetworkHealthMetrics getCurrentMetrics() const;
    float getUptimePercentage() const;
    float getPacketLossRate() const;
    uint32_t getAverageLatency() const;
    float getThroughput() const;
    float getNetworkEfficiency() const;
    
    // Health assessment
    bool isHealthy() const;
    float getOverallHealthScore() const;      // 0.0 - 100.0
    String getHealthStatus() const;
    std::vector<String> getHealthWarnings() const;
    
    // Issue detection and tracking
    bool detectNetworkIssues();
    std::vector<NetworkIssue> getActiveIssues() const;
    std::vector<NetworkIssue> getResolvedIssues() const;
    void acknowledgeIssue(size_t issueIndex);
    void clearResolvedIssues();
    
    // Alerts
    std::vector<HealthAlert> getActiveAlerts() const;
    void acknowledgeAlert(size_t alertIndex);
    void clearAcknowledgedAlerts();
    
    // Diagnostics
    String getDiagnosticReport() const;
    String getMetricsSummary() const;
    void printMetrics() const;
    void printIssues() const;
    
    // Recommendations
    std::vector<String> getDiagnosticRecommendations() const;
    bool recommendTopologyChanges() const;
    std::vector<String> suggestPerformanceOptimizations() const;
    
    // Predictive analysis
    bool predictNetworkFailures();
    float getFailureProbability() const;      // 0.0 - 1.0
    uint32_t getEstimatedTimeToFailure() const; // seconds
    
    // Configuration
    void setConfig(const MonitorConfig& config);
    MonitorConfig getConfig() const;
    void setUpdateInterval(uint32_t interval);
    void enablePredictiveAnalysis(bool enable);
    
    // Statistics reset
    void resetStatistics();
    void resetAlerts();
    
private:
    // Internal state
    bool initialized_;
    MonitorConfig config_;
    NetworkHealthMetrics metrics_;
    
    // Issue and alert tracking
    std::vector<NetworkIssue> issues_;
    std::vector<HealthAlert> alerts_;
    
    // Historical data for analysis
    static const int HISTORY_SIZE = 50;
    std::vector<uint32_t> latencyHistory_;
    std::vector<float> signalHistory_;
    std::vector<bool> transmissionHistory_;
    
    // Timing
    unsigned long lastUpdate_;
    unsigned long lastLatencyTest_;
    unsigned long lastThroughputTest_;
    
    // Internal tracking
    uint32_t totalTransmissions_;
    uint32_t totalBytes_;
    unsigned long uptimeStart_;
    unsigned long totalUptime_;
    unsigned long totalDowntime_;
    
    // Internal methods
    void collectMetrics();
    void updateAverageLatency(uint32_t latencyMs);
    void updateThroughput(size_t bytes, uint32_t durationMs);
    void updatePacketLoss();
    void updateEfficiency();
    void detectIssue(IssueSeverity severity, const String& description, 
                    const String& component);
    void createAlert(const String& type, const String& message, 
                    IssueSeverity severity);
    void checkThresholds();
    void analyzeLatencyTrends();
    void analyzeSignalTrends();
    float calculateHealthScore() const;
    bool isLatencyIncreasing() const;
    bool isSignalDeteriorating() const;
    void addToHistory(std::vector<uint32_t>& history, uint32_t value);
    void addToHistory(std::vector<float>& history, float value);
    void addToHistory(std::vector<bool>& history, bool value);
};

// Global instance accessor
extern NetworkHealthMonitor* g_networkHealthMonitor;

// Utility functions
bool initializeNetworkHealthMonitor(const MonitorConfig& config = MonitorConfig());
void cleanupNetworkHealthMonitor();
NetworkHealthMetrics getCurrentNetworkHealth();
bool isNetworkHealthy();
String getNetworkHealthReport();

#endif // NETWORK_HEALTH_MONITOR_H
