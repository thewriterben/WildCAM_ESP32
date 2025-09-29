/**
 * Cloud Service Orchestrator - Unified Cloud Services Coordination
 * 
 * Provides centralized management and coordination of all cloud services
 * including enhanced cloud manager, research collaboration, intelligent sync,
 * analytics engine, and wildlife pipeline integration.
 * 
 * Features:
 * - Unified service lifecycle management
 * - Cross-service communication and coordination
 * - Centralized configuration management
 * - Service health monitoring and failover
 * - Resource optimization across services
 * - Event-driven service coordination
 */

#ifndef CLOUD_SERVICE_ORCHESTRATOR_H
#define CLOUD_SERVICE_ORCHESTRATOR_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <functional>
#include "enhanced_cloud_manager.h"

// Forward declarations for service classes
class ResearchCollaborationPlatform;
class IntelligentSyncManager;
class CloudAnalyticsEngine;
class WildlifeCloudPipeline;
class CloudConfigManager;

// Service Types
enum CloudServiceType {
    SERVICE_ENHANCED_CLOUD_MANAGER,
    SERVICE_RESEARCH_COLLABORATION,
    SERVICE_INTELLIGENT_SYNC,
    SERVICE_ANALYTICS_ENGINE,
    SERVICE_WILDLIFE_PIPELINE,
    SERVICE_CONFIG_MANAGER
};

// Service Status
enum ServiceStatus {
    STATUS_STOPPED,
    STATUS_STARTING,
    STATUS_RUNNING,
    STATUS_STOPPING,
    STATUS_ERROR,
    STATUS_MAINTENANCE
};

// Service Priority Levels
enum ServicePriority {
    PRIORITY_CRITICAL = 1,    // Core functionality - must always run
    PRIORITY_HIGH = 2,        // Important features - high availability
    PRIORITY_MEDIUM = 3,      // Standard features - balanced resources
    PRIORITY_LOW = 4          // Optional features - can be suspended
};

// Orchestration Events
enum OrchestrationEvent {
    EVENT_SERVICE_STARTED,
    EVENT_SERVICE_STOPPED,
    EVENT_SERVICE_ERROR,
    EVENT_CONFIGURATION_CHANGED,
    EVENT_RESOURCE_CONSTRAINT,
    EVENT_EMERGENCY_SHUTDOWN,
    EVENT_MAINTENANCE_MODE,
    EVENT_RECOVERY_COMPLETED
};

// Service Configuration
struct ServiceConfig {
    CloudServiceType type;
    ServicePriority priority;
    bool autoStart;
    bool autoRestart;
    uint32_t healthCheckInterval;
    uint32_t maxMemoryUsage;
    uint32_t maxCpuUsage;
    std::map<String, String> parameters;
    
    ServiceConfig() : 
        type(SERVICE_ENHANCED_CLOUD_MANAGER), priority(PRIORITY_MEDIUM), 
        autoStart(true), autoRestart(true), healthCheckInterval(30),
        maxMemoryUsage(0), maxCpuUsage(0) {}
};

// Service Information
struct ServiceInfo {
    CloudServiceType type;
    ServiceStatus status;
    ServicePriority priority;
    uint32_t startTime;
    uint32_t lastHealthCheck;
    uint32_t memoryUsage;
    uint32_t cpuUsage;
    uint32_t restartCount;
    String lastError;
    bool isHealthy;
    
    ServiceInfo() : 
        type(SERVICE_ENHANCED_CLOUD_MANAGER), status(STATUS_STOPPED), 
        priority(PRIORITY_MEDIUM), startTime(0), lastHealthCheck(0),
        memoryUsage(0), cpuUsage(0), restartCount(0), lastError(""), isHealthy(false) {}
};

// Orchestrator Configuration
struct OrchestratorConfig {
    bool enableAutoRecovery;
    bool enableResourceOptimization;
    bool enableServiceBalancing;
    uint32_t globalHealthCheckInterval;
    uint32_t emergencyShutdownThreshold;
    uint32_t maxServiceRestarts;
    uint32_t resourceMonitoringInterval;
    bool enableMaintenanceMode;
    
    OrchestratorConfig() : 
        enableAutoRecovery(true), enableResourceOptimization(true),
        enableServiceBalancing(true), globalHealthCheckInterval(60),
        emergencyShutdownThreshold(90), maxServiceRestarts(3),
        resourceMonitoringInterval(30), enableMaintenanceMode(false) {}
};

// Resource Usage Statistics
struct ResourceStats {
    uint32_t totalMemoryUsage;
    uint32_t totalCpuUsage;
    uint32_t availableMemory;
    uint32_t availableCpu;
    uint32_t activeServices;
    uint32_t healthyServices;
    uint32_t criticalServices;
    
    ResourceStats() : 
        totalMemoryUsage(0), totalCpuUsage(0), availableMemory(0),
        availableCpu(0), activeServices(0), healthyServices(0), criticalServices(0) {}
};

/**
 * Cloud Service Orchestrator Class
 * 
 * Centralized management and coordination of all cloud services
 */
class CloudServiceOrchestrator {
public:
    CloudServiceOrchestrator();
    ~CloudServiceOrchestrator();
    
    // Initialization and configuration
    bool initialize(const OrchestratorConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Service lifecycle management
    bool registerService(CloudServiceType type, const ServiceConfig& config);
    bool unregisterService(CloudServiceType type);
    bool startService(CloudServiceType type);
    bool stopService(CloudServiceType type);
    bool restartService(CloudServiceType type);
    bool startAllServices();
    bool stopAllServices();
    
    // Service configuration management
    bool updateServiceConfig(CloudServiceType type, const ServiceConfig& config);
    ServiceConfig getServiceConfig(CloudServiceType type) const;
    bool applyGlobalConfiguration(const std::map<String, String>& globalConfig);
    
    // Service status and monitoring
    ServiceStatus getServiceStatus(CloudServiceType type) const;
    ServiceInfo getServiceInfo(CloudServiceType type) const;
    std::vector<ServiceInfo> getAllServiceInfo() const;
    bool isServiceHealthy(CloudServiceType type) const;
    bool areAllCriticalServicesHealthy() const;
    
    // Health monitoring and diagnostics
    bool performGlobalHealthCheck();
    bool performServiceHealthCheck(CloudServiceType type);
    ResourceStats getResourceStatistics() const;
    bool isSystemHealthy() const;
    
    // Emergency and maintenance operations
    bool enterMaintenanceMode();
    bool exitMaintenanceMode();
    bool isInMaintenanceMode() const { return maintenanceMode_; }
    bool performEmergencyShutdown();
    bool recoverFromEmergency();
    
    // Resource optimization
    bool optimizeResourceUsage();
    bool balanceServiceLoad();
    bool suspendLowPriorityServices();
    bool resumeSuspendedServices();
    
    // Service communication and coordination
    bool broadcastEvent(OrchestrationEvent event, const String& data = "");
    bool sendServiceMessage(CloudServiceType from, CloudServiceType to, const String& message);
    bool coordinateServiceOperation(const String& operation, const std::vector<CloudServiceType>& services);
    
    // Integration with camera system
    bool handleWildlifeDetection(const String& detectionData);
    bool handleImageCapture(const String& imagePath);
    bool handleSystemEvent(const String& eventType, const String& eventData);
    
    // Reporting and analytics
    void generateSystemReport(String& report);
    void generateServiceReport(CloudServiceType type, String& report);
    void generateResourceReport(String& report);
    
    // Event callbacks
    typedef void (*ServiceEventCallback)(CloudServiceType service, OrchestrationEvent event, const String& data);
    typedef void (*SystemHealthCallback)(bool isHealthy, const ResourceStats& stats);
    typedef void (*EmergencyCallback)(const String& reason);
    
    void setServiceEventCallback(ServiceEventCallback callback) { serviceEventCallback_ = callback; }
    void setSystemHealthCallback(SystemHealthCallback callback) { systemHealthCallback_ = callback; }
    void setEmergencyCallback(EmergencyCallback callback) { emergencyCallback_ = callback; }
    
    // Service access methods
    EnhancedCloudManager* getEnhancedCloudManager() const;
    ResearchCollaborationPlatform* getResearchCollaborationPlatform() const;
    IntelligentSyncManager* getIntelligentSyncManager() const;
    CloudAnalyticsEngine* getCloudAnalyticsEngine() const;
    WildlifeCloudPipeline* getWildlifeCloudPipeline() const;
    CloudConfigManager* getCloudConfigManager() const;

private:
    // Configuration and state
    OrchestratorConfig config_;
    bool initialized_;
    bool maintenanceMode_;
    uint32_t lastGlobalHealthCheck_;
    uint32_t lastResourceOptimization_;
    uint32_t emergencyShutdownTime_;
    
    // Service management
    std::map<CloudServiceType, ServiceConfig> serviceConfigs_;
    std::map<CloudServiceType, ServiceInfo> serviceInfo_;
    std::map<CloudServiceType, void*> serviceInstances_;
    
    // Resource monitoring
    ResourceStats currentResourceStats_;
    std::map<CloudServiceType, uint32_t> serviceResourceUsage_;
    
    // Event callbacks
    ServiceEventCallback serviceEventCallback_;
    SystemHealthCallback systemHealthCallback_;
    EmergencyCallback emergencyCallback_;
    
    // Internal service management
    bool createServiceInstance(CloudServiceType type);
    bool destroyServiceInstance(CloudServiceType type);
    bool initializeServiceInstance(CloudServiceType type);
    bool startServiceInstance(CloudServiceType type);
    bool stopServiceInstance(CloudServiceType type);
    
    // Health monitoring helpers
    bool checkServiceHealth(CloudServiceType type);
    void updateServiceInfo(CloudServiceType type, ServiceStatus status, const String& error = "");
    void updateResourceStats();
    bool isResourceConstrained() const;
    
    // Resource optimization helpers
    bool suspendService(CloudServiceType type);
    bool resumeService(CloudServiceType type);
    std::vector<CloudServiceType> getLowPriorityServices() const;
    std::vector<CloudServiceType> getCriticalServices() const;
    
    // Event and messaging helpers
    void notifyServiceEvent(CloudServiceType service, OrchestrationEvent event, const String& data = "");
    void notifySystemHealth(bool isHealthy);
    void notifyEmergency(const String& reason);
    
    // Service coordination helpers
    bool coordinateServiceStartup();
    bool coordinateServiceShutdown();
    bool handleServiceDependencies(CloudServiceType type, bool starting);
    
    // Utility methods
    String getServiceName(CloudServiceType type) const;
    String getStatusName(ServiceStatus status) const;
    String getEventName(OrchestrationEvent event) const;
    uint32_t calculateServiceUptime(CloudServiceType type) const;
    bool shouldRestartService(CloudServiceType type) const;
};

// Global orchestrator instance
extern CloudServiceOrchestrator* g_cloudServiceOrchestrator;

// Utility functions for easy integration
bool initializeCloudOrchestrator(const OrchestratorConfig& config);
bool startCloudServices();
bool stopCloudServices();
bool isCloudSystemHealthy();
bool handleWildlifeEvent(const String& eventType, const String& eventData);
void cleanupCloudOrchestrator();

#endif // CLOUD_SERVICE_ORCHESTRATOR_H