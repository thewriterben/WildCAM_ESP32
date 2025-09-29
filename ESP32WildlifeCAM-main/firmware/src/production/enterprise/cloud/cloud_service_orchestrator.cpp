/**
 * Cloud Service Orchestrator Implementation
 * 
 * Centralized management and coordination of all cloud services
 */

#include "cloud_service_orchestrator.h"
#include "enhanced_cloud_manager.h"
#include "research_collaboration_platform.h"
#include "intelligent_sync_manager.h"
#include "../../../../utils/logger.h"

// Global instance
CloudServiceOrchestrator* g_cloudServiceOrchestrator = nullptr;

CloudServiceOrchestrator::CloudServiceOrchestrator() : 
    initialized_(false),
    maintenanceMode_(false),
    lastGlobalHealthCheck_(0),
    lastResourceOptimization_(0),
    emergencyShutdownTime_(0),
    serviceEventCallback_(nullptr),
    systemHealthCallback_(nullptr),
    emergencyCallback_(nullptr) {
}

CloudServiceOrchestrator::~CloudServiceOrchestrator() {
    cleanup();
}

bool CloudServiceOrchestrator::initialize(const OrchestratorConfig& config) {
    if (initialized_) {
        Logger::warning("Cloud Service Orchestrator already initialized");
        return true;
    }
    
    Logger::info("Initializing Cloud Service Orchestrator");
    
    config_ = config;
    
    // Initialize resource monitoring
    updateResourceStats();
    
    initialized_ = true;
    Logger::info("Cloud Service Orchestrator initialized successfully");
    return true;
}

void CloudServiceOrchestrator::cleanup() {
    if (!initialized_) {
        return;
    }
    
    Logger::info("Cleaning up Cloud Service Orchestrator");
    
    // Stop all services
    stopAllServices();
    
    // Cleanup service instances
    for (auto& pair : serviceInstances_) {
        destroyServiceInstance(pair.first);
    }
    
    serviceInstances_.clear();
    serviceConfigs_.clear();
    serviceInfo_.clear();
    
    initialized_ = false;
    Logger::info("Cloud Service Orchestrator cleanup completed");
}

bool CloudServiceOrchestrator::registerService(CloudServiceType type, const ServiceConfig& config) {
    if (serviceConfigs_.find(type) != serviceConfigs_.end()) {
        Logger::warning("Service already registered: " + getServiceName(type));
        return false;
    }
    
    serviceConfigs_[type] = config;
    
    // Initialize service info
    ServiceInfo info;
    info.type = type;
    info.priority = config.priority;
    info.status = STATUS_STOPPED;
    serviceInfo_[type] = info;
    
    Logger::info("Service registered: " + getServiceName(type));
    
    // Auto-start if configured
    if (config.autoStart) {
        return startService(type);
    }
    
    return true;
}

bool CloudServiceOrchestrator::unregisterService(CloudServiceType type) {
    // Stop service if running
    stopService(type);
    
    // Remove from configurations
    serviceConfigs_.erase(type);
    serviceInfo_.erase(type);
    
    // Destroy service instance
    destroyServiceInstance(type);
    
    Logger::info("Service unregistered: " + getServiceName(type));
    return true;
}

bool CloudServiceOrchestrator::startService(CloudServiceType type) {
    auto configIt = serviceConfigs_.find(type);
    if (configIt == serviceConfigs_.end()) {
        Logger::error("Service not registered: " + getServiceName(type));
        return false;
    }
    
    auto infoIt = serviceInfo_.find(type);
    if (infoIt != serviceInfo_.end() && infoIt->second.status == STATUS_RUNNING) {
        Logger::warning("Service already running: " + getServiceName(type));
        return true;
    }
    
    Logger::info("Starting service: " + getServiceName(type));
    
    updateServiceInfo(type, STATUS_STARTING);
    
    // Create service instance if needed
    if (serviceInstances_.find(type) == serviceInstances_.end()) {
        if (!createServiceInstance(type)) {
            updateServiceInfo(type, STATUS_ERROR, "Failed to create service instance");
            return false;
        }
    }
    
    // Initialize and start service
    if (!initializeServiceInstance(type) || !startServiceInstance(type)) {
        updateServiceInfo(type, STATUS_ERROR, "Failed to start service");
        return false;
    }
    
    // Update service info
    ServiceInfo& info = serviceInfo_[type];
    info.status = STATUS_RUNNING;
    info.startTime = millis() / 1000;
    info.isHealthy = true;
    
    // Notify event
    notifyServiceEvent(type, EVENT_SERVICE_STARTED);
    
    Logger::info("Service started successfully: " + getServiceName(type));
    return true;
}

bool CloudServiceOrchestrator::stopService(CloudServiceType type) {
    auto infoIt = serviceInfo_.find(type);
    if (infoIt == serviceInfo_.end() || infoIt->second.status != STATUS_RUNNING) {
        Logger::warning("Service not running: " + getServiceName(type));
        return true;
    }
    
    Logger::info("Stopping service: " + getServiceName(type));
    
    updateServiceInfo(type, STATUS_STOPPING);
    
    // Stop service instance
    if (!stopServiceInstance(type)) {
        updateServiceInfo(type, STATUS_ERROR, "Failed to stop service");
        return false;
    }
    
    // Update service info
    serviceInfo_[type].status = STATUS_STOPPED;
    serviceInfo_[type].isHealthy = false;
    
    // Notify event
    notifyServiceEvent(type, EVENT_SERVICE_STOPPED);
    
    Logger::info("Service stopped successfully: " + getServiceName(type));
    return true;
}

bool CloudServiceOrchestrator::restartService(CloudServiceType type) {
    Logger::info("Restarting service: " + getServiceName(type));
    
    if (!stopService(type)) {
        return false;
    }
    
    // Wait a moment before restart
    delay(1000);
    
    bool success = startService(type);
    if (success) {
        serviceInfo_[type].restartCount++;
    }
    
    return success;
}

bool CloudServiceOrchestrator::startAllServices() {
    Logger::info("Starting all registered services");
    
    bool allStarted = true;
    
    // Start critical services first
    std::vector<CloudServiceType> criticalServices = getCriticalServices();
    for (CloudServiceType service : criticalServices) {
        if (!startService(service)) {
            allStarted = false;
            Logger::error("Failed to start critical service: " + getServiceName(service));
        }
    }
    
    // Start remaining services
    for (const auto& pair : serviceConfigs_) {
        CloudServiceType service = pair.first;
        if (std::find(criticalServices.begin(), criticalServices.end(), service) == criticalServices.end()) {
            if (!startService(service)) {
                allStarted = false;
                Logger::warning("Failed to start service: " + getServiceName(service));
            }
        }
    }
    
    return allStarted;
}

bool CloudServiceOrchestrator::stopAllServices() {
    Logger::info("Stopping all running services");
    
    bool allStopped = true;
    
    // Stop in reverse priority order
    std::vector<CloudServiceType> lowPriorityServices = getLowPriorityServices();
    for (CloudServiceType service : lowPriorityServices) {
        if (!stopService(service)) {
            allStopped = false;
        }
    }
    
    // Stop critical services last
    std::vector<CloudServiceType> criticalServices = getCriticalServices();
    for (CloudServiceType service : criticalServices) {
        if (!stopService(service)) {
            allStopped = false;
        }
    }
    
    return allStopped;
}

ServiceStatus CloudServiceOrchestrator::getServiceStatus(CloudServiceType type) const {
    auto it = serviceInfo_.find(type);
    if (it != serviceInfo_.end()) {
        return it->second.status;
    }
    return STATUS_STOPPED;
}

ServiceInfo CloudServiceOrchestrator::getServiceInfo(CloudServiceType type) const {
    auto it = serviceInfo_.find(type);
    if (it != serviceInfo_.end()) {
        return it->second;
    }
    return ServiceInfo();
}

std::vector<ServiceInfo> CloudServiceOrchestrator::getAllServiceInfo() const {
    std::vector<ServiceInfo> info;
    for (const auto& pair : serviceInfo_) {
        info.push_back(pair.second);
    }
    return info;
}

bool CloudServiceOrchestrator::isServiceHealthy(CloudServiceType type) const {
    auto it = serviceInfo_.find(type);
    return (it != serviceInfo_.end()) && it->second.isHealthy && (it->second.status == STATUS_RUNNING);
}

bool CloudServiceOrchestrator::areAllCriticalServicesHealthy() const {
    std::vector<CloudServiceType> criticalServices = getCriticalServices();
    for (CloudServiceType service : criticalServices) {
        if (!isServiceHealthy(service)) {
            return false;
        }
    }
    return true;
}

bool CloudServiceOrchestrator::performGlobalHealthCheck() {
    if (!initialized_) {
        return false;
    }
    
    Logger::info("Performing global health check");
    
    bool systemHealthy = true;
    uint32_t currentTime = millis() / 1000;
    
    for (auto& pair : serviceInfo_) {
        CloudServiceType service = pair.first;
        if (!performServiceHealthCheck(service)) {
            systemHealthy = false;
        }
    }
    
    // Update resource statistics
    updateResourceStats();
    
    lastGlobalHealthCheck_ = currentTime;
    
    // Check if system needs recovery
    if (!systemHealthy && config_.enableAutoRecovery) {
        Logger::warning("System health degraded, attempting recovery");
        // Implement recovery logic here
    }
    
    // Notify system health
    if (systemHealthCallback_) {
        systemHealthCallback_(systemHealthy, currentResourceStats_);
    }
    
    return systemHealthy;
}

bool CloudServiceOrchestrator::performServiceHealthCheck(CloudServiceType type) {
    auto it = serviceInfo_.find(type);
    if (it == serviceInfo_.end()) {
        return false;
    }
    
    bool isHealthy = checkServiceHealth(type);
    it->second.isHealthy = isHealthy;
    it->second.lastHealthCheck = millis() / 1000;
    
    if (!isHealthy && serviceConfigs_[type].autoRestart && shouldRestartService(type)) {
        Logger::warning("Service unhealthy, attempting restart: " + getServiceName(type));
        restartService(type);
    }
    
    return isHealthy;
}

bool CloudServiceOrchestrator::enterMaintenanceMode() {
    Logger::info("Entering maintenance mode");
    
    maintenanceMode_ = true;
    
    // Suspend low priority services
    suspendLowPriorityServices();
    
    // Notify event
    broadcastEvent(EVENT_MAINTENANCE_MODE, "System entering maintenance mode");
    
    return true;
}

bool CloudServiceOrchestrator::exitMaintenanceMode() {
    Logger::info("Exiting maintenance mode");
    
    maintenanceMode_ = false;
    
    // Resume suspended services
    resumeSuspendedServices();
    
    // Notify event
    broadcastEvent(EVENT_RECOVERY_COMPLETED, "System exiting maintenance mode");
    
    return true;
}

void CloudServiceOrchestrator::generateSystemReport(String& report) {
    report = "=== Cloud Service Orchestrator System Report ===\n";
    report += "System Status: " + String(isSystemHealthy() ? "Healthy" : "Degraded") + "\n";
    report += "Maintenance Mode: " + String(maintenanceMode_ ? "Yes" : "No") + "\n";
    report += "Total Services: " + String(serviceConfigs_.size()) + "\n";
    report += "Running Services: " + String(currentResourceStats_.activeServices) + "\n";
    report += "Healthy Services: " + String(currentResourceStats_.healthyServices) + "\n";
    report += "Critical Services: " + String(currentResourceStats_.criticalServices) + "\n\n";
    
    // Service details
    for (const auto& pair : serviceInfo_) {
        const ServiceInfo& info = pair.second;
        report += "Service: " + getServiceName(info.type) + "\n";
        report += "  Status: " + getStatusName(info.status) + "\n";
        report += "  Healthy: " + String(info.isHealthy ? "Yes" : "No") + "\n";
        report += "  Uptime: " + String(calculateServiceUptime(info.type)) + " seconds\n";
        report += "  Restarts: " + String(info.restartCount) + "\n";
        if (!info.lastError.isEmpty()) {
            report += "  Last Error: " + info.lastError + "\n";
        }
        report += "\n";
    }
}

// Private helper methods implementation

bool CloudServiceOrchestrator::createServiceInstance(CloudServiceType type) {
    void* instance = nullptr;
    
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER:
            instance = new EnhancedCloudManager();
            break;
        case SERVICE_RESEARCH_COLLABORATION:
            // instance = new ResearchCollaborationPlatform();
            break;
        case SERVICE_INTELLIGENT_SYNC:
            // instance = new IntelligentSyncManager();
            break;
        default:
            Logger::error("Unknown service type: " + String(static_cast<int>(type)));
            return false;
    }
    
    if (instance) {
        serviceInstances_[type] = instance;
        return true;
    }
    
    return false;
}

bool CloudServiceOrchestrator::destroyServiceInstance(CloudServiceType type) {
    auto it = serviceInstances_.find(type);
    if (it == serviceInstances_.end()) {
        return true;
    }
    
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER:
            delete static_cast<EnhancedCloudManager*>(it->second);
            break;
        default:
            // Handle other service types
            break;
    }
    
    serviceInstances_.erase(type);
    return true;
}

bool CloudServiceOrchestrator::initializeServiceInstance(CloudServiceType type) {
    auto it = serviceInstances_.find(type);
    if (it == serviceInstances_.end()) {
        return false;
    }
    
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER: {
            EnhancedCloudManager* manager = static_cast<EnhancedCloudManager*>(it->second);
            EnhancedCloudConfig config;
            return manager->initialize(config);
        }
        default:
            return true;
    }
}

bool CloudServiceOrchestrator::startServiceInstance(CloudServiceType type) {
    // Service-specific startup logic
    return true;
}

bool CloudServiceOrchestrator::stopServiceInstance(CloudServiceType type) {
    auto it = serviceInstances_.find(type);
    if (it == serviceInstances_.end()) {
        return true;
    }
    
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER: {
            EnhancedCloudManager* manager = static_cast<EnhancedCloudManager*>(it->second);
            manager->cleanup();
            return true;
        }
        default:
            return true;
    }
}

bool CloudServiceOrchestrator::checkServiceHealth(CloudServiceType type) {
    auto it = serviceInstances_.find(type);
    if (it == serviceInstances_.end()) {
        return false;
    }
    
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER: {
            EnhancedCloudManager* manager = static_cast<EnhancedCloudManager*>(it->second);
            return manager->isInitialized() && manager->getOverallHealth() != HEALTH_OFFLINE;
        }
        default:
            return true;
    }
}

void CloudServiceOrchestrator::updateServiceInfo(CloudServiceType type, ServiceStatus status, const String& error) {
    auto it = serviceInfo_.find(type);
    if (it != serviceInfo_.end()) {
        it->second.status = status;
        if (!error.isEmpty()) {
            it->second.lastError = error;
        }
    }
}

void CloudServiceOrchestrator::updateResourceStats() {
    currentResourceStats_.activeServices = 0;
    currentResourceStats_.healthyServices = 0;
    currentResourceStats_.criticalServices = 0;
    
    for (const auto& pair : serviceInfo_) {
        if (pair.second.status == STATUS_RUNNING) {
            currentResourceStats_.activeServices++;
        }
        if (pair.second.isHealthy) {
            currentResourceStats_.healthyServices++;
        }
        if (serviceConfigs_[pair.first].priority == PRIORITY_CRITICAL) {
            currentResourceStats_.criticalServices++;
        }
    }
}

std::vector<CloudServiceType> CloudServiceOrchestrator::getCriticalServices() const {
    std::vector<CloudServiceType> critical;
    for (const auto& pair : serviceConfigs_) {
        if (pair.second.priority == PRIORITY_CRITICAL) {
            critical.push_back(pair.first);
        }
    }
    return critical;
}

std::vector<CloudServiceType> CloudServiceOrchestrator::getLowPriorityServices() const {
    std::vector<CloudServiceType> lowPriority;
    for (const auto& pair : serviceConfigs_) {
        if (pair.second.priority == PRIORITY_LOW) {
            lowPriority.push_back(pair.first);
        }
    }
    return lowPriority;
}

void CloudServiceOrchestrator::notifyServiceEvent(CloudServiceType service, OrchestrationEvent event, const String& data) {
    if (serviceEventCallback_) {
        serviceEventCallback_(service, event, data);
    }
}

String CloudServiceOrchestrator::getServiceName(CloudServiceType type) const {
    switch (type) {
        case SERVICE_ENHANCED_CLOUD_MANAGER: return "Enhanced Cloud Manager";
        case SERVICE_RESEARCH_COLLABORATION: return "Research Collaboration Platform";
        case SERVICE_INTELLIGENT_SYNC: return "Intelligent Sync Manager";
        case SERVICE_ANALYTICS_ENGINE: return "Cloud Analytics Engine";
        case SERVICE_WILDLIFE_PIPELINE: return "Wildlife Cloud Pipeline";
        case SERVICE_CONFIG_MANAGER: return "Cloud Config Manager";
        default: return "Unknown Service";
    }
}

String CloudServiceOrchestrator::getStatusName(ServiceStatus status) const {
    switch (status) {
        case STATUS_STOPPED: return "Stopped";
        case STATUS_STARTING: return "Starting";
        case STATUS_RUNNING: return "Running";
        case STATUS_STOPPING: return "Stopping";
        case STATUS_ERROR: return "Error";
        case STATUS_MAINTENANCE: return "Maintenance";
        default: return "Unknown";
    }
}

uint32_t CloudServiceOrchestrator::calculateServiceUptime(CloudServiceType type) const {
    auto it = serviceInfo_.find(type);
    if (it != serviceInfo_.end() && it->second.status == STATUS_RUNNING) {
        uint32_t currentTime = millis() / 1000;
        return currentTime - it->second.startTime;
    }
    return 0;
}

bool CloudServiceOrchestrator::shouldRestartService(CloudServiceType type) const {
    auto it = serviceInfo_.find(type);
    if (it != serviceInfo_.end()) {
        return it->second.restartCount < config_.maxServiceRestarts;
    }
    return false;
}

// Global utility functions implementation

bool initializeCloudOrchestrator(const OrchestratorConfig& config) {
    if (g_cloudServiceOrchestrator != nullptr) {
        Logger::warning("Cloud Service Orchestrator already exists");
        return true;
    }
    
    g_cloudServiceOrchestrator = new CloudServiceOrchestrator();
    if (!g_cloudServiceOrchestrator->initialize(config)) {
        delete g_cloudServiceOrchestrator;
        g_cloudServiceOrchestrator = nullptr;
        return false;
    }
    
    return true;
}

bool startCloudServices() {
    if (g_cloudServiceOrchestrator == nullptr) {
        Logger::error("Cloud Service Orchestrator not initialized");
        return false;
    }
    
    return g_cloudServiceOrchestrator->startAllServices();
}

bool stopCloudServices() {
    if (g_cloudServiceOrchestrator == nullptr) {
        return true;
    }
    
    return g_cloudServiceOrchestrator->stopAllServices();
}

bool isCloudSystemHealthy() {
    if (g_cloudServiceOrchestrator == nullptr) {
        return false;
    }
    
    return g_cloudServiceOrchestrator->isSystemHealthy();
}

void cleanupCloudOrchestrator() {
    if (g_cloudServiceOrchestrator != nullptr) {
        g_cloudServiceOrchestrator->cleanup();
        delete g_cloudServiceOrchestrator;
        g_cloudServiceOrchestrator = nullptr;
    }
}