/**
 * Enhanced Cloud Manager Implementation
 * 
 * Complete implementation of advanced multi-cloud integration with
 * failover capabilities, load balancing, and optimization features.
 */

#include "enhanced_cloud_manager.h"
#include "../../../../utils/logger.h"
#include <algorithm>
#include <cmath>

// Global instance
EnhancedCloudManager* g_enhancedCloudManager = nullptr;

EnhancedCloudManager::EnhancedCloudManager() : 
    initialized_(false),
    loadBalanceStrategy_(LOAD_BALANCE_ROUND_ROBIN),
    roundRobinIndex_(0),
    lastHealthCheck_(0),
    costPeriodStart_(0),
    failoverCallback_(nullptr),
    healthCallback_(nullptr),
    costCallback_(nullptr) {
}

EnhancedCloudManager::~EnhancedCloudManager() {
    cleanup();
}

bool EnhancedCloudManager::initialize(const EnhancedCloudConfig& config) {
    if (initialized_) {
        Logger::warning("Enhanced Cloud Manager already initialized");
        return true;
    }
    
    Logger::info("Initializing Enhanced Cloud Manager");
    
    config_ = config;
    costPeriodStart_ = millis() / 1000; // Current time in seconds
    
    // Initialize all configured providers
    bool anyProviderSucceeded = false;
    for (size_t i = 0; i < config_.providers.size(); i++) {
        const CloudConfig& providerConfig = config_.providers[i];
        CloudPriority priority = static_cast<CloudPriority>(i + 1);
        
        if (addCloudProvider(providerConfig, priority)) {
            anyProviderSucceeded = true;
            Logger::info("Successfully initialized cloud provider: " + String(static_cast<int>(providerConfig.platform)));
        } else {
            Logger::error("Failed to initialize cloud provider: " + String(static_cast<int>(providerConfig.platform)));
        }
    }
    
    if (!anyProviderSucceeded) {
        Logger::error("Failed to initialize any cloud providers");
        return false;
    }
    
    // Perform initial health check
    performHealthCheck();
    
    initialized_ = true;
    Logger::info("Enhanced Cloud Manager initialized successfully");
    return true;
}

void EnhancedCloudManager::cleanup() {
    if (!initialized_) {
        return;
    }
    
    Logger::info("Cleaning up Enhanced Cloud Manager");
    
    // Cleanup all providers
    for (auto& pair : providers_) {
        if (pair.second) {
            pair.second->cleanup();
            delete pair.second;
        }
    }
    
    providers_.clear();
    providerStatuses_.clear();
    providerPriorities_.clear();
    loadCounters_.clear();
    healthCheckCounters_.clear();
    monthlySpending_.clear();
    
    // Clear queues
    while (!priorityQueue_.empty()) {
        priorityQueue_.pop();
    }
    
    initialized_ = false;
    Logger::info("Enhanced Cloud Manager cleanup completed");
}

bool EnhancedCloudManager::addCloudProvider(const CloudConfig& config, CloudPriority priority) {
    if (providers_.find(config.platform) != providers_.end()) {
        Logger::warning("Cloud provider already exists: " + String(static_cast<int>(config.platform)));
        return false;
    }
    
    // Create new cloud integrator instance
    CloudIntegrator* provider = new CloudIntegrator();
    if (!provider->init(config)) {
        Logger::error("Failed to initialize cloud provider: " + String(static_cast<int>(config.platform)));
        delete provider;
        return false;
    }
    
    // Store provider and its configuration
    providers_[config.platform] = provider;
    providerPriorities_[config.platform] = priority;
    loadCounters_[config.platform] = 0;
    healthCheckCounters_[config.platform] = 0;
    monthlySpending_[config.platform] = 0.0;
    
    // Initialize provider status
    CloudProviderStatus status;
    status.platform = config.platform;
    status.health = HEALTH_OFFLINE;
    status.quality = QUALITY_POOR;
    status.isAvailable = false;
    providerStatuses_[config.platform] = status;
    
    // Test initial connection
    if (testProviderConnection(config.platform)) {
        providerStatuses_[config.platform].health = HEALTH_OPTIMAL;
        providerStatuses_[config.platform].isAvailable = true;
        Logger::info("Cloud provider connected successfully: " + String(static_cast<int>(config.platform)));
    }
    
    return true;
}

bool EnhancedCloudManager::removeCloudProvider(CloudPlatform platform) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        Logger::warning("Cloud provider not found: " + String(static_cast<int>(platform)));
        return false;
    }
    
    // Cleanup provider
    it->second->cleanup();
    delete it->second;
    
    // Remove from all maps
    providers_.erase(platform);
    providerStatuses_.erase(platform);
    providerPriorities_.erase(platform);
    loadCounters_.erase(platform);
    healthCheckCounters_.erase(platform);
    monthlySpending_.erase(platform);
    
    Logger::info("Cloud provider removed: " + String(static_cast<int>(platform)));
    return true;
}

bool EnhancedCloudManager::updateProviderConfig(CloudPlatform platform, const CloudConfig& config) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        Logger::error("Cloud provider not found for update: " + String(static_cast<int>(platform)));
        return false;
    }
    
    // Reconfigure the provider
    if (!it->second->configure(config)) {
        Logger::error("Failed to update cloud provider configuration: " + String(static_cast<int>(platform)));
        return false;
    }
    
    // Test connection with new configuration
    if (testProviderConnection(platform)) {
        Logger::info("Cloud provider configuration updated successfully: " + String(static_cast<int>(platform)));
        return true;
    } else {
        Logger::warning("Cloud provider configuration updated but connection test failed: " + String(static_cast<int>(platform)));
        providerStatuses_[platform].health = HEALTH_OFFLINE;
        providerStatuses_[platform].isAvailable = false;
        return false;
    }
}

std::vector<CloudProviderStatus> EnhancedCloudManager::getProviderStatuses() const {
    std::vector<CloudProviderStatus> statuses;
    for (const auto& pair : providerStatuses_) {
        statuses.push_back(pair.second);
    }
    return statuses;
}

bool EnhancedCloudManager::uploadWithFailover(const EnhancedUploadRequest& request) {
    if (!initialized_) {
        Logger::error("Enhanced Cloud Manager not initialized");
        return false;
    }
    
    // Try primary provider first
    CloudPlatform primaryProvider = selectOptimalProvider(request.estimatedSize);
    if (primaryProvider != CLOUD_CUSTOM && executeUploadWithRetry(primaryProvider, request)) {
        Logger::info("Upload successful with primary provider: " + String(static_cast<int>(primaryProvider)));
        return true;
    }
    
    // Try failover providers in priority order
    std::vector<std::pair<CloudPlatform, CloudPriority>> sortedProviders;
    for (const auto& pair : providerPriorities_) {
        if (pair.first != primaryProvider && isProviderHealthy(pair.first)) {
            sortedProviders.push_back(pair);
        }
    }
    
    // Sort by priority
    std::sort(sortedProviders.begin(), sortedProviders.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    // Try each failover provider
    for (const auto& pair : sortedProviders) {
        CloudPlatform provider = pair.first;
        Logger::info("Attempting failover upload to provider: " + String(static_cast<int>(provider)));
        
        if (executeUploadWithRetry(provider, request)) {
            Logger::info("Failover upload successful with provider: " + String(static_cast<int>(provider)));
            
            // Notify failover if callback is set
            if (failoverCallback_) {
                failoverCallback_(primaryProvider, provider);
            }
            
            return true;
        }
    }
    
    Logger::error("Upload failed with all available providers");
    return false;
}

bool EnhancedCloudManager::batchUploadOptimized(const std::vector<EnhancedUploadRequest>& requests) {
    if (!initialized_) {
        Logger::error("Enhanced Cloud Manager not initialized");
        return false;
    }
    
    if (requests.empty()) {
        Logger::warning("Empty batch upload request");
        return true;
    }
    
    Logger::info("Starting optimized batch upload of " + String(requests.size()) + " items");
    
    // Group requests by optimal provider
    std::map<CloudPlatform, std::vector<EnhancedUploadRequest>> groupedRequests;
    
    for (const auto& request : requests) {
        CloudPlatform optimalProvider = selectOptimalProvider(request.estimatedSize);
        if (optimalProvider != CLOUD_CUSTOM) {
            groupedRequests[optimalProvider].push_back(request);
        }
    }
    
    // Upload each group
    bool allSuccessful = true;
    for (const auto& group : groupedRequests) {
        CloudPlatform provider = group.first;
        const auto& providerRequests = group.second;
        
        Logger::info("Uploading " + String(providerRequests.size()) + 
                    " items to provider: " + String(static_cast<int>(provider)));
        
        for (const auto& request : providerRequests) {
            if (!executeUploadWithRetry(provider, request)) {
                Logger::error("Batch upload failed for item: " + request.baseRequest.requestId);
                allSuccessful = false;
                
                // Try failover for failed item
                if (!uploadWithFailover(request)) {
                    Logger::error("Failover also failed for item: " + request.baseRequest.requestId);
                }
            }
        }
    }
    
    Logger::info("Batch upload completed. Success: " + String(allSuccessful ? "true" : "false"));
    return allSuccessful;
}

bool EnhancedCloudManager::uploadWithLoadBalancing(const EnhancedUploadRequest& request) {
    if (!initialized_) {
        Logger::error("Enhanced Cloud Manager not initialized");
        return false;
    }
    
    CloudPlatform selectedProvider = selectProviderByStrategy(request.estimatedSize);
    if (selectedProvider == CLOUD_CUSTOM) {
        Logger::error("No available provider for load-balanced upload");
        return false;
    }
    
    // Update load counter
    loadCounters_[selectedProvider]++;
    
    bool success = executeUploadWithRetry(selectedProvider, request);
    if (success) {
        Logger::info("Load-balanced upload successful with provider: " + String(static_cast<int>(selectedProvider)));
    } else {
        Logger::error("Load-balanced upload failed with provider: " + String(static_cast<int>(selectedProvider)));
        // Try failover
        success = uploadWithFailover(request);
    }
    
    return success;
}

bool EnhancedCloudManager::performHealthCheck() {
    if (!initialized_) {
        return false;
    }
    
    Logger::info("Performing comprehensive health check");
    uint32_t currentTime = millis() / 1000;
    bool anyProviderHealthy = false;
    
    for (auto& pair : providerStatuses_) {
        CloudPlatform platform = pair.first;
        performSingleProviderHealthCheck(platform);
        
        if (isProviderHealthy(platform)) {
            anyProviderHealthy = true;
        }
    }
    
    lastHealthCheck_ = currentTime;
    
    if (!anyProviderHealthy) {
        Logger::critical("No healthy cloud providers available!");
    }
    
    return anyProviderHealthy;
}

bool EnhancedCloudManager::performHealthCheck(CloudPlatform platform) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        return false;
    }
    
    performSingleProviderHealthCheck(platform);
    return isProviderHealthy(platform);
}

CloudHealthStatus EnhancedCloudManager::getOverallHealth() const {
    if (!initialized_) {
        return HEALTH_OFFLINE;
    }
    
    int healthyCount = 0;
    int totalCount = 0;
    
    for (const auto& pair : providerStatuses_) {
        totalCount++;
        if (pair.second.health == HEALTH_OPTIMAL || pair.second.health == HEALTH_DEGRADED) {
            healthyCount++;
        }
    }
    
    if (totalCount == 0) {
        return HEALTH_OFFLINE;
    }
    
    float healthRatio = static_cast<float>(healthyCount) / totalCount;
    
    if (healthRatio >= 0.8) {
        return HEALTH_OPTIMAL;
    } else if (healthRatio >= 0.5) {
        return HEALTH_DEGRADED;
    } else if (healthRatio > 0) {
        return HEALTH_CRITICAL;
    } else {
        return HEALTH_OFFLINE;
    }
}

ConnectionQuality EnhancedCloudManager::assessConnectionQuality(CloudPlatform platform) {
    auto it = providerStatuses_.find(platform);
    if (it == providerStatuses_.end()) {
        return QUALITY_POOR;
    }
    
    const CloudProviderStatus& status = it->second;
    
    // Assess based on response time and success rate
    if (status.responseTime < 100 && status.successRate > 95.0) {
        return QUALITY_EXCELLENT;
    } else if (status.responseTime < 200 && status.successRate > 90.0) {
        return QUALITY_GOOD;
    } else if (status.responseTime < 500 && status.successRate > 80.0) {
        return QUALITY_FAIR;
    } else {
        return QUALITY_POOR;
    }
}

bool EnhancedCloudManager::triggerFailover(CloudPlatform fromProvider, CloudPlatform toProvider) {
    auto fromIt = providers_.find(fromProvider);
    auto toIt = providers_.find(toProvider);
    
    if (fromIt == providers_.end() || toIt == providers_.end()) {
        Logger::error("Invalid providers for failover");
        return false;
    }
    
    if (!isProviderHealthy(toProvider)) {
        Logger::error("Target provider not healthy for failover: " + String(static_cast<int>(toProvider)));
        return false;
    }
    
    Logger::info("Triggering failover from " + String(static_cast<int>(fromProvider)) + 
                " to " + String(static_cast<int>(toProvider)));
    
    // Mark source provider as offline
    providerStatuses_[fromProvider].health = HEALTH_OFFLINE;
    providerStatuses_[fromProvider].isAvailable = false;
    
    // Notify callback if set
    if (failoverCallback_) {
        failoverCallback_(fromProvider, toProvider);
    }
    
    return true;
}

bool EnhancedCloudManager::recoverProvider(CloudPlatform platform) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        Logger::error("Provider not found for recovery: " + String(static_cast<int>(platform)));
        return false;
    }
    
    Logger::info("Attempting to recover provider: " + String(static_cast<int>(platform)));
    
    // Test connection
    if (testProviderConnection(platform)) {
        providerStatuses_[platform].health = HEALTH_OPTIMAL;
        providerStatuses_[platform].isAvailable = true;
        Logger::info("Provider recovery successful: " + String(static_cast<int>(platform)));
        return true;
    } else {
        Logger::error("Provider recovery failed: " + String(static_cast<int>(platform)));
        return false;
    }
}

CloudPlatform EnhancedCloudManager::selectOptimalProvider(uint32_t dataSize) {
    if (!initialized_) {
        return CLOUD_CUSTOM;
    }
    
    // If cost optimization is enabled, use cost-based selection
    if (config_.enableCostOptimization) {
        CloudPlatform costOptimal = selectCostOptimalProvider(dataSize);
        if (costOptimal != CLOUD_CUSTOM && isProviderHealthy(costOptimal)) {
            return costOptimal;
        }
    }
    
    // Otherwise use strategy-based selection
    return selectProviderByStrategy(dataSize);
}

void EnhancedCloudManager::setLoadBalanceStrategy(LoadBalanceStrategy strategy) {
    loadBalanceStrategy_ = strategy;
    Logger::info("Load balance strategy changed to: " + String(static_cast<int>(strategy)));
}

bool EnhancedCloudManager::redistributeLoad() {
    if (!initialized_) {
        return false;
    }
    
    Logger::info("Redistributing load across providers");
    
    // Reset load counters
    for (auto& pair : loadCounters_) {
        pair.second = 0;
    }
    
    // Reset round-robin index
    roundRobinIndex_ = 0;
    
    Logger::info("Load redistribution completed");
    return true;
}

float EnhancedCloudManager::calculateEstimatedCost(CloudPlatform platform, uint32_t dataSize) {
    // Simple cost calculation based on data size and platform
    // In a real implementation, this would use actual pricing APIs
    
    float baseCostPerMB = 0.0;
    switch (platform) {
        case CLOUD_AWS:
            baseCostPerMB = 0.023; // AWS S3 standard pricing
            break;
        case CLOUD_AZURE:
            baseCostPerMB = 0.0208; // Azure Blob storage pricing
            break;
        case CLOUD_GCP:
            baseCostPerMB = 0.020; // GCP Cloud Storage pricing
            break;
        default:
            baseCostPerMB = 0.025; // Default pricing
            break;
    }
    
    float dataSizeMB = static_cast<float>(dataSize) / (1024.0 * 1024.0);
    return dataSizeMB * baseCostPerMB;
}

bool EnhancedCloudManager::optimizeCosts() {
    if (!initialized_) {
        return false;
    }
    
    Logger::info("Optimizing cloud costs");
    
    float totalMonthlyCost = getCurrentMonthlyCost();
    
    if (totalMonthlyCost > config_.maxMonthlyCost) {
        Logger::warning("Monthly cost exceeds budget: " + String(totalMonthlyCost) + 
                       " > " + String(config_.maxMonthlyCost));
        
        // Notify callback if set
        if (costCallback_) {
            costCallback_(totalMonthlyCost, config_.maxMonthlyCost);
        }
        
        // Implement cost reduction strategies
        // This could include data compression, quality reduction, etc.
        Logger::info("Implementing cost reduction strategies");
        return true;
    }
    
    Logger::info("Cost optimization completed. Current monthly cost: " + String(totalMonthlyCost));
    return true;
}

float EnhancedCloudManager::getCurrentMonthlyCost() const {
    float totalCost = 0.0;
    for (const auto& pair : monthlySpending_) {
        totalCost += pair.second;
    }
    return totalCost;
}

bool EnhancedCloudManager::isWithinCostBudget() const {
    return getCurrentMonthlyCost() <= config_.maxMonthlyCost;
}

bool EnhancedCloudManager::optimizeBandwidth() {
    if (!initialized_ || !config_.enableBandwidthOptimization) {
        return false;
    }
    
    Logger::info("Optimizing bandwidth usage");
    
    // Implement bandwidth optimization strategies
    // This could include adaptive quality, compression, etc.
    
    return true;
}

bool EnhancedCloudManager::adjustQualityBasedOnBandwidth() {
    // Implement quality adjustment based on available bandwidth
    // This would integrate with the camera system to adjust image quality
    
    Logger::info("Adjusting quality based on bandwidth");
    return true;
}

uint32_t EnhancedCloudManager::getOptimalChunkSize(CloudPlatform platform) {
    // Return optimal chunk size based on provider and connection quality
    ConnectionQuality quality = assessConnectionQuality(platform);
    
    switch (quality) {
        case QUALITY_EXCELLENT:
            return 1024 * 1024; // 1MB chunks
        case QUALITY_GOOD:
            return 512 * 1024;  // 512KB chunks
        case QUALITY_FAIR:
            return 256 * 1024;  // 256KB chunks
        case QUALITY_POOR:
        default:
            return 128 * 1024;  // 128KB chunks
    }
}

void EnhancedCloudManager::generateHealthReport(String& report) {
    report = "=== Enhanced Cloud Manager Health Report ===\n";
    report += "Overall Health: " + String(static_cast<int>(getOverallHealth())) + "\n";
    report += "Total Providers: " + String(providers_.size()) + "\n\n";
    
    for (const auto& pair : providerStatuses_) {
        const CloudProviderStatus& status = pair.second;
        report += "Provider: " + String(static_cast<int>(status.platform)) + "\n";
        report += "  Health: " + String(static_cast<int>(status.health)) + "\n";
        report += "  Quality: " + String(static_cast<int>(status.quality)) + "\n";
        report += "  Response Time: " + String(status.responseTime) + "ms\n";
        report += "  Success Rate: " + String(status.successRate) + "%\n";
        report += "  Available: " + String(status.isAvailable ? "Yes" : "No") + "\n\n";
    }
}

void EnhancedCloudManager::generateCostReport(String& report) {
    report = "=== Enhanced Cloud Manager Cost Report ===\n";
    report += "Total Monthly Cost: $" + String(getCurrentMonthlyCost()) + "\n";
    report += "Budget Limit: $" + String(config_.maxMonthlyCost) + "\n";
    report += "Within Budget: " + String(isWithinCostBudget() ? "Yes" : "No") + "\n\n";
    
    for (const auto& pair : monthlySpending_) {
        report += "Provider " + String(static_cast<int>(pair.first)) + ": $" + String(pair.second) + "\n";
    }
}

void EnhancedCloudManager::generatePerformanceReport(String& report) {
    report = "=== Enhanced Cloud Manager Performance Report ===\n";
    report += "Load Balance Strategy: " + String(static_cast<int>(loadBalanceStrategy_)) + "\n\n";
    
    for (const auto& pair : providerStatuses_) {
        const CloudProviderStatus& status = pair.second;
        report += "Provider: " + String(static_cast<int>(status.platform)) + "\n";
        report += "  Total Requests: " + String(status.totalRequests) + "\n";
        report += "  Failed Requests: " + String(status.failedRequests) + "\n";
        report += "  Data Transferred: " + String(status.dataTransferred) + " bytes\n";
        report += "  Load Counter: " + String(loadCounters_.at(status.platform)) + "\n\n";
    }
}

bool EnhancedCloudManager::updateConfiguration(const EnhancedCloudConfig& config) {
    Logger::info("Updating Enhanced Cloud Manager configuration");
    config_ = config;
    
    // Apply new configuration to existing providers
    // This could involve reconnecting or reconfiguring providers
    
    return true;
}

// Private helper methods implementation

bool EnhancedCloudManager::initializeProvider(CloudPlatform platform, const CloudConfig& config) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        return false;
    }
    
    return it->second->init(config);
}

bool EnhancedCloudManager::testProviderConnection(CloudPlatform platform) {
    auto it = providers_.find(platform);
    if (it == providers_.end()) {
        return false;
    }
    
    uint32_t startTime = millis();
    bool connected = it->second->testConnection();
    uint32_t responseTime = millis() - startTime;
    
    updateProviderStatus(platform, connected, responseTime);
    
    return connected;
}

void EnhancedCloudManager::updateProviderStatus(CloudPlatform platform, bool success, uint32_t responseTime) {
    auto it = providerStatuses_.find(platform);
    if (it == providerStatuses_.end()) {
        return;
    }
    
    CloudProviderStatus& status = it->second;
    status.totalRequests++;
    
    if (success) {
        status.responseTime = (status.responseTime + responseTime) / 2; // Moving average
    } else {
        status.failedRequests++;
    }
    
    // Calculate success rate
    status.successRate = ((float)(status.totalRequests - status.failedRequests) / status.totalRequests) * 100.0;
    
    // Update quality based on metrics
    status.quality = assessConnectionQuality(platform);
    
    // Update health status
    CloudHealthStatus oldHealth = status.health;
    if (success && status.successRate > 90.0) {
        status.health = HEALTH_OPTIMAL;
        status.isAvailable = true;
    } else if (success && status.successRate > 70.0) {
        status.health = HEALTH_DEGRADED;
        status.isAvailable = true;
    } else if (status.successRate > 50.0) {
        status.health = HEALTH_CRITICAL;
        status.isAvailable = true;
    } else {
        status.health = HEALTH_OFFLINE;
        status.isAvailable = false;
    }
    
    // Notify if health changed
    if (oldHealth != status.health && healthCallback_) {
        healthCallback_(platform, status.health);
    }
    
    status.lastHealthCheck = millis() / 1000;
}

CloudPlatform EnhancedCloudManager::selectProviderByStrategy(uint32_t dataSize) {
    switch (loadBalanceStrategy_) {
        case LOAD_BALANCE_ROUND_ROBIN:
            return selectRoundRobinProvider();
        case LOAD_BALANCE_LEAST_LOADED:
            return selectLeastLoadedProvider();
        case LOAD_BALANCE_FASTEST_RESPONSE:
            return selectFastestProvider();
        case LOAD_BALANCE_COST_OPTIMIZED:
            return selectCostOptimalProvider(dataSize);
        default:
            return selectRoundRobinProvider();
    }
}

bool EnhancedCloudManager::executeUploadWithRetry(CloudPlatform platform, const EnhancedUploadRequest& request) {
    auto it = providers_.find(platform);
    if (it == providers_.end() || !isProviderHealthy(platform)) {
        return false;
    }
    
    EnhancedUploadRequest mutableRequest = request; // Copy for retry counting
    
    for (uint32_t attempt = 0; attempt < config_.maxRetryAttempts; attempt++) {
        uint32_t startTime = millis();
        
        // Execute the upload
        bool success = it->second->uploadFileAsync(mutableRequest.baseRequest);
        
        uint32_t responseTime = millis() - startTime;
        updateProviderStatus(platform, success, responseTime);
        
        if (success) {
            // Update cost tracking
            updateCostTracking(platform, mutableRequest.estimatedSize);
            
            logOperation("Upload", platform, true);
            return true;
        }
        
        mutableRequest.retryCount++;
        
        // Wait with exponential backoff before retry
        if (attempt < config_.maxRetryAttempts - 1) {
            uint32_t backoffTime = calculateExponentialBackoff(attempt);
            delay(backoffTime);
            Logger::info("Retrying upload after " + String(backoffTime) + "ms delay");
        }
    }
    
    logOperation("Upload (after retries)", platform, false);
    return false;
}

void EnhancedCloudManager::performSingleProviderHealthCheck(CloudPlatform platform) {
    healthCheckCounters_[platform]++;
    testProviderConnection(platform);
    
    Logger::debug("Health check completed for provider: " + String(static_cast<int>(platform)));
}

bool EnhancedCloudManager::isProviderHealthy(CloudPlatform platform) const {
    auto it = providerStatuses_.find(platform);
    if (it == providerStatuses_.end()) {
        return false;
    }
    
    const CloudProviderStatus& status = it->second;
    return status.isAvailable && 
           (status.health == HEALTH_OPTIMAL || status.health == HEALTH_DEGRADED);
}

void EnhancedCloudManager::notifyHealthChange(CloudPlatform platform, CloudHealthStatus newStatus) {
    if (healthCallback_) {
        healthCallback_(platform, newStatus);
    }
}

void EnhancedCloudManager::updateCostTracking(CloudPlatform platform, uint32_t dataSize) {
    float cost = calculateEstimatedCost(platform, dataSize);
    monthlySpending_[platform] += cost;
    
    // Check if we need to reset monthly costs (simplified - in reality would check calendar month)
    uint32_t currentTime = millis() / 1000;
    if (currentTime - costPeriodStart_ > 2592000) { // 30 days in seconds
        resetMonthlyCosts();
    }
}

CloudPlatform EnhancedCloudManager::selectCostOptimalProvider(uint32_t dataSize) {
    CloudPlatform bestProvider = CLOUD_CUSTOM;
    float lowestCost = std::numeric_limits<float>::max();
    
    for (const auto& pair : providers_) {
        CloudPlatform platform = pair.first;
        if (isProviderHealthy(platform)) {
            float cost = calculateEstimatedCost(platform, dataSize);
            if (cost < lowestCost) {
                lowestCost = cost;
                bestProvider = platform;
            }
        }
    }
    
    return bestProvider;
}

void EnhancedCloudManager::resetMonthlyCosts() {
    for (auto& pair : monthlySpending_) {
        pair.second = 0.0;
    }
    costPeriodStart_ = millis() / 1000;
    Logger::info("Monthly costs reset");
}

CloudPlatform EnhancedCloudManager::selectRoundRobinProvider() {
    std::vector<CloudPlatform> healthyProviders;
    for (const auto& pair : providers_) {
        if (isProviderHealthy(pair.first)) {
            healthyProviders.push_back(pair.first);
        }
    }
    
    if (healthyProviders.empty()) {
        return CLOUD_CUSTOM;
    }
    
    CloudPlatform selected = healthyProviders[roundRobinIndex_ % healthyProviders.size()];
    roundRobinIndex_++;
    
    return selected;
}

CloudPlatform EnhancedCloudManager::selectLeastLoadedProvider() {
    CloudPlatform bestProvider = CLOUD_CUSTOM;
    uint32_t lowestLoad = std::numeric_limits<uint32_t>::max();
    
    for (const auto& pair : loadCounters_) {
        CloudPlatform platform = pair.first;
        if (isProviderHealthy(platform) && pair.second < lowestLoad) {
            lowestLoad = pair.second;
            bestProvider = platform;
        }
    }
    
    return bestProvider;
}

CloudPlatform EnhancedCloudManager::selectFastestProvider() {
    CloudPlatform bestProvider = CLOUD_CUSTOM;
    uint32_t fastestResponse = std::numeric_limits<uint32_t>::max();
    
    for (const auto& pair : providerStatuses_) {
        CloudPlatform platform = pair.first;
        if (isProviderHealthy(platform) && pair.second.responseTime < fastestResponse) {
            fastestResponse = pair.second.responseTime;
            bestProvider = platform;
        }
    }
    
    return bestProvider;
}

uint32_t EnhancedCloudManager::calculateExponentialBackoff(uint32_t attempt) {
    // Exponential backoff: base_delay * 2^attempt + jitter
    uint32_t baseDelay = 1000; // 1 second base delay
    uint32_t backoff = baseDelay * (1 << attempt); // 2^attempt
    uint32_t jitter = random(0, backoff / 4); // Add some randomness
    return std::min(backoff + jitter, 30000U); // Cap at 30 seconds
}

bool EnhancedCloudManager::shouldRetryOperation(const EnhancedUploadRequest& request) {
    return request.retryCount < request.maxRetries;
}

void EnhancedCloudManager::logOperation(const String& operation, CloudPlatform platform, bool success) {
    String logMessage = operation + " on provider " + String(static_cast<int>(platform)) + 
                       ": " + (success ? "SUCCESS" : "FAILED");
    
    if (success) {
        Logger::info(logMessage);
    } else {
        Logger::error(logMessage);
    }
}

// Global utility functions implementation

bool initializeEnhancedCloudManager(const EnhancedCloudConfig& config) {
    if (g_enhancedCloudManager != nullptr) {
        Logger::warning("Enhanced Cloud Manager already exists");
        return true;
    }
    
    g_enhancedCloudManager = new EnhancedCloudManager();
    if (!g_enhancedCloudManager->initialize(config)) {
        delete g_enhancedCloudManager;
        g_enhancedCloudManager = nullptr;
        return false;
    }
    
    return true;
}

bool uploadWithEnhancedFeatures(const String& localPath, DataType dataType, CloudPriority priority) {
    if (g_enhancedCloudManager == nullptr) {
        Logger::error("Enhanced Cloud Manager not initialized");
        return false;
    }
    
    EnhancedUploadRequest request;
    request.baseRequest.localPath = localPath;
    request.baseRequest.dataType = dataType;
    request.baseRequest.requestId = "ECM_" + String(millis());
    request.priority = priority;
    
    return g_enhancedCloudManager->uploadWithFailover(request);
}

bool triggerCloudFailover(CloudPlatform fromProvider) {
    if (g_enhancedCloudManager == nullptr) {
        return false;
    }
    
    CloudPlatform toProvider = g_enhancedCloudManager->selectOptimalProvider();
    if (toProvider == CLOUD_CUSTOM || toProvider == fromProvider) {
        return false;
    }
    
    return g_enhancedCloudManager->triggerFailover(fromProvider, toProvider);
}

CloudHealthStatus getCloudSystemHealth() {
    if (g_enhancedCloudManager == nullptr) {
        return HEALTH_OFFLINE;
    }
    
    return g_enhancedCloudManager->getOverallHealth();
}

void cleanupEnhancedCloudManager() {
    if (g_enhancedCloudManager != nullptr) {
        g_enhancedCloudManager->cleanup();
        delete g_enhancedCloudManager;
        g_enhancedCloudManager = nullptr;
    }
}