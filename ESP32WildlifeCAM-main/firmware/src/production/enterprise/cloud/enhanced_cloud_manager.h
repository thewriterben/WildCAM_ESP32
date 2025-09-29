/**
 * Enhanced Cloud Manager - Advanced Multi-Cloud Integration
 * 
 * Provides sophisticated cloud integration with failover capabilities,
 * intelligent load balancing, and comprehensive error handling for
 * enterprise wildlife monitoring deployments.
 * 
 * Features:
 * - Multi-cloud failover and load balancing
 * - Intelligent retry mechanisms with exponential backoff
 * - Bandwidth optimization and adaptive quality
 * - Real-time health monitoring and alerts
 * - Cost optimization algorithms
 * - Integration with existing cloud integrator
 */

#ifndef ENHANCED_CLOUD_MANAGER_H
#define ENHANCED_CLOUD_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <queue>
#include "../cloud_integrator.h"
#include "../../../../utils/logger.h"

// Cloud Provider Priority
enum CloudPriority {
    PRIORITY_PRIMARY = 1,
    PRIORITY_SECONDARY = 2,
    PRIORITY_BACKUP = 3,
    PRIORITY_EMERGENCY = 4
};

// Connection Quality Metrics
enum ConnectionQuality {
    QUALITY_EXCELLENT = 1,  // <100ms latency, >95% success
    QUALITY_GOOD = 2,       // <200ms latency, >90% success
    QUALITY_FAIR = 3,       // <500ms latency, >80% success
    QUALITY_POOR = 4        // >500ms latency or <80% success
};

// Cloud Health Status
enum CloudHealthStatus {
    HEALTH_OPTIMAL,
    HEALTH_DEGRADED,
    HEALTH_CRITICAL,
    HEALTH_OFFLINE
};

// Enhanced Cloud Configuration
struct EnhancedCloudConfig {
    std::vector<CloudConfig> providers;
    uint32_t healthCheckInterval;       // Health check interval in seconds
    uint32_t failoverTimeout;           // Failover timeout in seconds
    uint32_t maxRetryAttempts;          // Maximum retry attempts per operation
    uint32_t loadBalanceThreshold;      // Load balance threshold in bytes
    bool enableCostOptimization;        // Enable cost optimization algorithms
    bool enableBandwidthOptimization;   // Enable bandwidth optimization
    float maxMonthlyCost;               // Maximum monthly cost threshold
    
    EnhancedCloudConfig() : 
        healthCheckInterval(30), failoverTimeout(10), maxRetryAttempts(3),
        loadBalanceThreshold(1048576), enableCostOptimization(true),
        enableBandwidthOptimization(true), maxMonthlyCost(100.0) {}
};

// Cloud Provider Status
struct CloudProviderStatus {
    CloudPlatform platform;
    CloudHealthStatus health;
    ConnectionQuality quality;
    uint32_t responseTime;              // Average response time in ms
    float successRate;                  // Success rate percentage
    uint32_t totalRequests;             // Total requests made
    uint32_t failedRequests;            // Failed requests count
    uint64_t dataTransferred;           // Total data transferred in bytes
    float estimatedCost;                // Estimated cost for current period
    uint32_t lastHealthCheck;           // Last health check timestamp
    bool isAvailable;                   // Provider availability status
    
    CloudProviderStatus() : 
        platform(CLOUD_CUSTOM), health(HEALTH_OFFLINE), quality(QUALITY_POOR),
        responseTime(0), successRate(0.0), totalRequests(0), failedRequests(0),
        dataTransferred(0), estimatedCost(0.0), lastHealthCheck(0), isAvailable(false) {}
};

// Load Balancing Strategy
enum LoadBalanceStrategy {
    LOAD_BALANCE_ROUND_ROBIN,
    LOAD_BALANCE_LEAST_LOADED,
    LOAD_BALANCE_FASTEST_RESPONSE,
    LOAD_BALANCE_COST_OPTIMIZED
};

// Enhanced Upload Request
struct EnhancedUploadRequest {
    UploadRequest baseRequest;
    CloudPriority priority;
    uint32_t retryCount;
    uint32_t maxRetries;
    uint32_t estimatedSize;
    bool requiresEncryption;
    bool allowCompression;
    uint32_t deadline;                  // Upload deadline timestamp
    
    EnhancedUploadRequest() : 
        priority(PRIORITY_PRIMARY), retryCount(0), maxRetries(3),
        estimatedSize(0), requiresEncryption(false), allowCompression(true), deadline(0) {}
};

/**
 * Enhanced Cloud Manager Class
 * 
 * Advanced cloud management with multi-provider support, failover,
 * load balancing, and intelligent optimization
 */
class EnhancedCloudManager {
public:
    EnhancedCloudManager();
    ~EnhancedCloudManager();
    
    // Initialization and configuration
    bool initialize(const EnhancedCloudConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Provider management
    bool addCloudProvider(const CloudConfig& config, CloudPriority priority);
    bool removeCloudProvider(CloudPlatform platform);
    bool updateProviderConfig(CloudPlatform platform, const CloudConfig& config);
    std::vector<CloudProviderStatus> getProviderStatuses() const;
    
    // Upload operations with enhanced features
    bool uploadWithFailover(const EnhancedUploadRequest& request);
    bool batchUploadOptimized(const std::vector<EnhancedUploadRequest>& requests);
    bool uploadWithLoadBalancing(const EnhancedUploadRequest& request);
    
    // Health monitoring and diagnostics
    bool performHealthCheck();
    bool performHealthCheck(CloudPlatform platform);
    CloudHealthStatus getOverallHealth() const;
    ConnectionQuality assessConnectionQuality(CloudPlatform platform);
    
    // Failover and recovery
    bool triggerFailover(CloudPlatform fromProvider, CloudPlatform toProvider);
    bool recoverProvider(CloudPlatform platform);
    CloudPlatform selectOptimalProvider(uint32_t dataSize = 0);
    
    // Load balancing
    void setLoadBalanceStrategy(LoadBalanceStrategy strategy);
    LoadBalanceStrategy getLoadBalanceStrategy() const { return loadBalanceStrategy_; }
    bool redistributeLoad();
    
    // Cost optimization
    float calculateEstimatedCost(CloudPlatform platform, uint32_t dataSize);
    bool optimizeCosts();
    float getCurrentMonthlyCost() const;
    bool isWithinCostBudget() const;
    
    // Bandwidth optimization
    bool optimizeBandwidth();
    bool adjustQualityBasedOnBandwidth();
    uint32_t getOptimalChunkSize(CloudPlatform platform);
    
    // Statistics and reporting
    void generateHealthReport(String& report);
    void generateCostReport(String& report);
    void generatePerformanceReport(String& report);
    
    // Configuration management
    bool updateConfiguration(const EnhancedCloudConfig& config);
    EnhancedCloudConfig getConfiguration() const { return config_; }
    
    // Event callbacks
    typedef void (*ProviderFailoverCallback)(CloudPlatform from, CloudPlatform to);
    typedef void (*HealthChangeCallback)(CloudPlatform platform, CloudHealthStatus status);
    typedef void (*CostThresholdCallback)(float currentCost, float threshold);
    
    void setProviderFailoverCallback(ProviderFailoverCallback callback) { failoverCallback_ = callback; }
    void setHealthChangeCallback(HealthChangeCallback callback) { healthCallback_ = callback; }
    void setCostThresholdCallback(CostThresholdCallback callback) { costCallback_ = callback; }

private:
    // Configuration and state
    EnhancedCloudConfig config_;
    bool initialized_;
    LoadBalanceStrategy loadBalanceStrategy_;
    
    // Cloud providers management
    std::map<CloudPlatform, CloudIntegrator*> providers_;
    std::map<CloudPlatform, CloudProviderStatus> providerStatuses_;
    std::map<CloudPlatform, CloudPriority> providerPriorities_;
    
    // Load balancing and queuing
    std::queue<EnhancedUploadRequest> priorityQueue_;
    std::map<CloudPlatform, uint32_t> loadCounters_;
    uint32_t roundRobinIndex_;
    
    // Health monitoring
    uint32_t lastHealthCheck_;
    std::map<CloudPlatform, uint32_t> healthCheckCounters_;
    
    // Cost tracking
    std::map<CloudPlatform, float> monthlySpending_;
    uint32_t costPeriodStart_;
    
    // Callbacks
    ProviderFailoverCallback failoverCallback_;
    HealthChangeCallback healthCallback_;
    CostThresholdCallback costCallback_;
    
    // Internal helper methods
    bool initializeProvider(CloudPlatform platform, const CloudConfig& config);
    bool testProviderConnection(CloudPlatform platform);
    void updateProviderStatus(CloudPlatform platform, bool success, uint32_t responseTime);
    CloudPlatform selectProviderByStrategy(uint32_t dataSize);
    bool executeUploadWithRetry(CloudPlatform platform, const EnhancedUploadRequest& request);
    
    // Health and monitoring helpers
    void performSingleProviderHealthCheck(CloudPlatform platform);
    bool isProviderHealthy(CloudPlatform platform) const;
    void notifyHealthChange(CloudPlatform platform, CloudHealthStatus newStatus);
    
    // Cost optimization helpers
    void updateCostTracking(CloudPlatform platform, uint32_t dataSize);
    CloudPlatform selectCostOptimalProvider(uint32_t dataSize);
    void resetMonthlyCosts();
    
    // Load balancing helpers
    CloudPlatform selectRoundRobinProvider();
    CloudPlatform selectLeastLoadedProvider();
    CloudPlatform selectFastestProvider();
    
    // Utility methods
    uint32_t calculateExponentialBackoff(uint32_t attempt);
    bool shouldRetryOperation(const EnhancedUploadRequest& request);
    void logOperation(const String& operation, CloudPlatform platform, bool success);
};

// Global enhanced cloud manager instance
extern EnhancedCloudManager* g_enhancedCloudManager;

// Utility functions for easy integration
bool initializeEnhancedCloudManager(const EnhancedCloudConfig& config);
bool uploadWithEnhancedFeatures(const String& localPath, DataType dataType, CloudPriority priority = PRIORITY_PRIMARY);
bool triggerCloudFailover(CloudPlatform fromProvider);
CloudHealthStatus getCloudSystemHealth();
void cleanupEnhancedCloudManager();

#endif // ENHANCED_CLOUD_MANAGER_H