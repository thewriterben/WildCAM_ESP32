/**
 * Production System Implementation
 * 
 * Main coordinator for Phase 4 production deployment features
 */

#include "production_system.h"
#include "../debug_utils.h"

// Global instance
ProductionSystem* g_productionSystem = nullptr;

// Constructor
ProductionSystem::ProductionSystem() 
    : status_(PROD_INITIALIZING), initialized_(false), startTime_(0),
      otaManager_(nullptr), configManager_(nullptr), environmentalAdapter_(nullptr),
      speciesDetector_(nullptr), cloudIntegrator_(nullptr), dataProtector_(nullptr),
      lastHealthCheck_(0), lastPerformanceOptimization_(0),
      eventCallback_(nullptr), healthCallback_(nullptr), errorCallback_(nullptr),
      conservationCallback_(nullptr) {
}

// Destructor
ProductionSystem::~ProductionSystem() {
    cleanup();
}

// Initialize production system
bool ProductionSystem::init(const ProductionConfig& config) {
    if (initialized_) return true;
    
    DEBUG_PRINTLN("Initializing Production System Phase 4...");
    
    config_ = config;
    status_ = PROD_INITIALIZING;
    startTime_ = millis();
    
    // Initialize components based on configuration
    if (!initializeProductionComponents()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize production components");
        return false;
    }
    
    // Load deployment configuration
    if (!loadDeploymentConfiguration()) {
        DEBUG_PRINTLN("WARNING: Using default deployment configuration");
    }
    
    // Validate configuration
    if (!validateConfiguration()) {
        DEBUG_PRINTLN("ERROR: Configuration validation failed");
        return false;
    }
    
    // Initialize health metrics
    healthMetrics_ = SystemHealthMetrics();
    healthMetrics_.status = PROD_INITIALIZING;
    
    initialized_ = true;
    status_ = PROD_OPERATIONAL;
    
    logProductionEvent("System", "Production System Phase 4 initialized successfully");
    DEBUG_PRINTLN("Production System Phase 4 initialized successfully");
    
    return true;
}

// Initialize production components
bool ProductionSystem::initializeProductionComponents() {
    bool success = true;
    
    // Initialize OTA Manager
    if (config_.enableOTA) {
        OTAConfig otaConfig;
        otaConfig.deviceID = config_.deploymentId;
        otaConfig.networkID = config_.networkId;
        
        if (!initializeOTA(otaConfig)) {
            DEBUG_PRINTLN("WARNING: OTA Manager initialization failed");
            success = false;
        } else {
            otaManager_ = g_otaManager;
        }
    }
    
    // Initialize Configuration Manager
    if (!initializeConfigManager(config_.deploymentId, config_.networkId)) {
        DEBUG_PRINTLN("WARNING: Configuration Manager initialization failed");
        success = false;
    } else {
        configManager_ = g_configManager;
    }
    
    // Initialize Environmental Adapter
    if (config_.enableEnvironmentalAdaptation) {
        AdaptationConfig adaptConfig;
        adaptConfig.weatherAwareMode = true;
        adaptConfig.seasonalOptimization = true;
        adaptConfig.thermalPowerMgmt = true;
        
        if (!initializeEnvironmentalAdapter(adaptConfig)) {
            DEBUG_PRINTLN("WARNING: Environmental Adapter initialization failed");
            success = false;
        } else {
            environmentalAdapter_ = g_environmentalAdapter;
        }
    }
    
    // Initialize Species Detector
    if (config_.enableAdvancedAI) {
        if (!initializeSpeciesDetector()) {
            DEBUG_PRINTLN("WARNING: Species Detector initialization failed");
            success = false;
        } else {
            speciesDetector_ = g_speciesDetector;
        }
    }
    
    // Initialize Cloud Integrator
    if (config_.enableCloudSync) {
        CloudConfig cloudConfig;
        cloudConfig.platform = CLOUD_CUSTOM;
        cloudConfig.syncMode = SYNC_OFFLINE_FIRST;
        cloudConfig.useSSL = true;
        
        if (!initializeCloudIntegration(cloudConfig)) {
            DEBUG_PRINTLN("WARNING: Cloud Integrator initialization failed");
            success = false;
        } else {
            cloudIntegrator_ = g_cloudIntegrator;
        }
    }
    
    // Initialize Data Protector
    if (config_.enableSecurity) {
        SecurityConfig secConfig;
        secConfig.securityLevel = SECURITY_STANDARD;
        secConfig.encryptionType = ENCRYPT_AES_256;
        secConfig.privacyMode = PRIVACY_ADVANCED;
        
        if (!initializeDataProtection(secConfig)) {
            DEBUG_PRINTLN("WARNING: Data Protector initialization failed");
            success = false;
        } else {
            dataProtector_ = g_dataProtector;
        }
    }
    
    return success;
}

// Start production operations
bool ProductionSystem::startProduction() {
    if (!initialized_ || status_ == PROD_OPERATIONAL) {
        return status_ == PROD_OPERATIONAL;
    }
    
    DEBUG_PRINTLN("Starting production operations...");
    
    // Start all components
    if (!startAllComponents()) {
        DEBUG_PRINTLN("ERROR: Failed to start all components");
        status_ = PROD_ERROR;
        return false;
    }
    
    // Perform initial health check
    if (!performHealthCheck()) {
        DEBUG_PRINTLN("WARNING: Initial health check failed");
    }
    
    // Join production network if multi-site enabled
    if (config_.enableMultiSite) {
        if (!joinProductionNetwork()) {
            DEBUG_PRINTLN("WARNING: Failed to join production network");
        }
    }
    
    status_ = PROD_OPERATIONAL;
    logProductionEvent("System", "Production operations started");
    
    if (healthCallback_) {
        healthCallback_(healthMetrics_);
    }
    
    return true;
}

// Process wildlife detection with full production pipeline
bool ProductionSystem::processWildlifeDetection(const uint8_t* imageData, uint16_t width, uint16_t height) {
    if (!isOperational() || !speciesDetector_) {
        return false;
    }
    
    uint32_t startTime = millis();
    
    // Perform species detection
    DetectionEvent detection = speciesDetector_->detectSpecies(imageData, width, height);
    
    // Apply environmental context
    if (environmentalAdapter_) {
        EnvironmentalData envData = environmentalAdapter_->getCurrentData();
        detection.temperature = envData.temperature;
        detection.lightLevel = envData.lightLevel;
        detection.weather = environmentalAdapter_->detectWeatherCondition();
    }
    
    // Apply security and privacy protection
    if (dataProtector_ && config_.enableSecurity) {
        // Protect sensitive location data for endangered species
        if (detection.conservationConcern) {
            String locationData = String(detection.latitude) + "," + String(detection.longitude);
            PrivacyResult privacyResult = dataProtector_->applyPrivacyProtection(locationData, DATA_CONFIDENTIAL);
            
            if (privacyResult.locationFuzzed) {
                logProductionEvent("Security", "Location data protected for endangered species");
            }
        }
    }
    
    // Log detection event
    logProductionEvent("Detection", "Species detected: " + detection.speciesName);
    
    // Check for conservation alerts
    if (config_.conservationMode && detection.conservationConcern) {
        String alert = "Conservation concern: " + detection.speciesName + " detected";
        logProductionEvent("Conservation", alert, true);
        
        if (conservationCallback_) {
            conservationCallback_(alert, detection.behaviorDescription);
        }
    }
    
    // Sync with cloud if enabled
    if (cloudIntegrator_ && config_.enableCloudSync) {
        // Create detection event for cloud upload
        RealTimeEvent cloudEvent;
        cloudEvent.eventType = "wildlife_detection";
        cloudEvent.deviceId = config_.deploymentId;
        cloudEvent.siteId = config_.siteId;
        cloudEvent.timestamp = detection.timestamp;
        cloudEvent.critical = detection.conservationConcern;
        
        cloudIntegrator_->sendRealTimeEvent(cloudEvent);
    }
    
    // Update performance metrics
    uint32_t processingTime = millis() - startTime;
    componentResponseTimes_["detection"] = processingTime;
    healthMetrics_.detectionCount24h++;
    
    return true;
}

// Perform comprehensive health check
bool ProductionSystem::performHealthCheck() {
    DEBUG_PRINTLN("Performing system health check...");
    
    lastHealthCheck_ = millis();
    bool overallHealthy = true;
    
    // Check OTA system
    if (otaManager_) {
        healthMetrics_.otaHealthy = (otaManager_->getStatus() != OTA_FAILED);
        if (!healthMetrics_.otaHealthy) overallHealthy = false;
    }
    
    // Check configuration system
    if (configManager_) {
        healthMetrics_.configHealthy = (configManager_->getStatus() != CONFIG_FAILED);
        if (!healthMetrics_.configHealthy) overallHealthy = false;
    }
    
    // Check environmental adaptation
    if (environmentalAdapter_) {
        healthMetrics_.environmentalHealthy = environmentalAdapter_->isOperationalConditions();
        if (!healthMetrics_.environmentalHealthy) overallHealthy = false;
    }
    
    // Check species detection
    if (speciesDetector_) {
        healthMetrics_.detectionHealthy = (speciesDetector_->getModelAccuracy() > 0.7);
        if (!healthMetrics_.detectionHealthy) overallHealthy = false;
    }
    
    // Check cloud connectivity
    if (cloudIntegrator_) {
        healthMetrics_.cloudHealthy = cloudIntegrator_->isConnected();
        if (!healthMetrics_.cloudHealthy) overallHealthy = false;
    }
    
    // Check security system
    if (dataProtector_) {
        healthMetrics_.securityHealthy = dataProtector_->isSecurityEnabled();
        if (!healthMetrics_.securityHealthy) overallHealthy = false;
    }
    
    // Update overall health score
    healthMetrics_.overallHealth = calculateOverallHealth();
    healthMetrics_.status = overallHealthy ? PROD_OPERATIONAL : PROD_ERROR;
    
    // Update system metrics
    healthMetrics_.uptime = (millis() - startTime_) / 1000;
    healthMetrics_.memoryUsage = (ESP.getFreeHeap() / (float)ESP.getHeapSize()) * 100.0;
    
    if (healthCallback_) {
        healthCallback_(healthMetrics_);
    }
    
    return overallHealthy;
}

// Calculate overall health score
float ProductionSystem::calculateOverallHealth() const {
    float score = 100.0;
    int componentCount = 0;
    
    if (otaManager_) {
        componentCount++;
        if (!healthMetrics_.otaHealthy) score -= 15.0;
    }
    
    if (configManager_) {
        componentCount++;
        if (!healthMetrics_.configHealthy) score -= 20.0;
    }
    
    if (environmentalAdapter_) {
        componentCount++;
        if (!healthMetrics_.environmentalHealthy) score -= 10.0;
    }
    
    if (speciesDetector_) {
        componentCount++;
        if (!healthMetrics_.detectionHealthy) score -= 25.0;
    }
    
    if (cloudIntegrator_) {
        componentCount++;
        if (!healthMetrics_.cloudHealthy) score -= 15.0;
    }
    
    if (dataProtector_) {
        componentCount++;
        if (!healthMetrics_.securityHealthy) score -= 15.0;
    }
    
    return max(0.0f, score);
}

// Log production event
void ProductionSystem::logProductionEvent(const String& component, const String& message, bool critical) {
    ProductionEvent event;
    event.eventId = generateEventId();
    event.timestamp = millis();
    event.component = component;
    event.message = message;
    event.critical = critical;
    event.resolved = false;
    
    eventHistory_.push_back(event);
    
    if (critical) {
        criticalEvents_.push_back(event);
        
        if (errorCallback_) {
            errorCallback_(component, message);
        }
    }
    
    // Limit event history size
    while (eventHistory_.size() > 1000) {
        eventHistory_.erase(eventHistory_.begin());
    }
    
    DEBUG_PRINTLN("Production Event [" + component + "]: " + message);
    
    if (eventCallback_) {
        eventCallback_(event);
    }
}

// Generate unique event ID
String ProductionSystem::generateEventId() {
    static uint32_t eventCounter = 0;
    return String(millis()) + "_" + String(++eventCounter);
}

// Load deployment configuration
bool ProductionSystem::loadDeploymentConfiguration() {
    // Load scenario-specific configuration
    return loadDeploymentProfile(config_.scenario);
}

// Load deployment profile for scenario
bool ProductionSystem::loadDeploymentProfile(DeploymentScenario scenario) {
    switch (scenario) {
        case SCENARIO_RESEARCH:
            config_.targetUptime = 99.99;
            config_.maxDetectionLatency = 1000;  // 1 second for research
            config_.conservationMode = true;
            config_.enableAdvancedAI = true;
            break;
            
        case SCENARIO_CONSERVATION:
            config_.targetUptime = 99.9;
            config_.maxDetectionLatency = 2000;  // 2 seconds
            config_.conservationMode = true;
            config_.endangeredSpeciesAlert = true;
            config_.poachingDetection = true;
            break;
            
        case SCENARIO_COMMERCIAL:
            config_.targetUptime = 99.5;
            config_.maxDetectionLatency = 3000;  // 3 seconds
            config_.enableCloudSync = true;
            config_.enableMultiSite = true;
            break;
            
        case SCENARIO_EDUCATION:
            config_.targetUptime = 99.0;
            config_.maxDetectionLatency = 5000;  // 5 seconds
            config_.enableSecurity = false;  // Simplified for education
            break;
            
        case SCENARIO_EMERGENCY:
            config_.targetUptime = 99.99;
            config_.maxDetectionLatency = 500;   // 0.5 seconds for emergency
            config_.conservationMode = true;
            config_.poachingDetection = true;
            break;
            
        default:
            return false;
    }
    
    return true;
}

// Validate configuration
bool ProductionSystem::validateConfiguration() const {
    // Basic validation checks
    if (config_.deploymentId.isEmpty()) {
        DEBUG_PRINTLN("ERROR: Deployment ID is required");
        return false;
    }
    
    if (config_.targetUptime < 90.0 || config_.targetUptime > 100.0) {
        DEBUG_PRINTLN("ERROR: Invalid target uptime");
        return false;
    }
    
    if (config_.maxDetectionLatency < 100 || config_.maxDetectionLatency > 30000) {
        DEBUG_PRINTLN("ERROR: Invalid detection latency target");
        return false;
    }
    
    return true;
}

// Start all components
bool ProductionSystem::startAllComponents() {
    bool success = true;
    
    // Start components in dependency order
    if (dataProtector_ && !dataProtector_->isSecurityEnabled()) {
        DEBUG_PRINTLN("WARNING: Security system not active");
    }
    
    if (configManager_) {
        // Configuration manager is already initialized
    }
    
    if (environmentalAdapter_) {
        if (!environmentalAdapter_->readSensors()) {
            DEBUG_PRINTLN("WARNING: Environmental sensors not responding");
            success = false;
        }
    }
    
    if (speciesDetector_) {
        // Species detector initialization handled in init
    }
    
    if (cloudIntegrator_) {
        if (!cloudIntegrator_->connect()) {
            DEBUG_PRINTLN("WARNING: Cloud connection failed");
            success = false;
        }
    }
    
    if (otaManager_) {
        if (!otaManager_->checkForUpdates()) {
            DEBUG_PRINTLN("INFO: No updates available");
        }
    }
    
    return success;
}

// Cleanup resources
void ProductionSystem::cleanup() {
    if (!initialized_) return;
    
    DEBUG_PRINTLN("Cleaning up Production System...");
    
    status_ = PROD_SHUTDOWN;
    
    // Cleanup all components
    if (otaManager_) {
        cleanupOTA();
        otaManager_ = nullptr;
    }
    
    if (configManager_) {
        cleanupConfigManager();
        configManager_ = nullptr;
    }
    
    if (environmentalAdapter_) {
        cleanupEnvironmentalAdapter();
        environmentalAdapter_ = nullptr;
    }
    
    if (speciesDetector_) {
        cleanupSpeciesDetector();
        speciesDetector_ = nullptr;
    }
    
    if (cloudIntegrator_) {
        cleanupCloudIntegration();
        cloudIntegrator_ = nullptr;
    }
    
    if (dataProtector_) {
        cleanupDataProtection();
        dataProtector_ = nullptr;
    }
    
    initialized_ = false;
    DEBUG_PRINTLN("Production System cleanup complete");
}

// Global utility functions
bool initializeProductionSystem(const ProductionConfig& config) {
    if (!g_productionSystem) {
        g_productionSystem = new ProductionSystem();
    }
    return g_productionSystem->init(config);
}

bool startWildlifeProduction() {
    return g_productionSystem ? g_productionSystem->startProduction() : false;
}

SystemHealthMetrics getProductionHealth() {
    return g_productionSystem ? g_productionSystem->getSystemHealth() : SystemHealthMetrics();
}

bool isProductionOperational() {
    return g_productionSystem ? g_productionSystem->isOperational() : false;
}

String getProductionStatus() {
    return g_productionSystem ? g_productionSystem->getStatusSummary() : "Not initialized";
}

void cleanupProductionSystem() {
    if (g_productionSystem) {
        delete g_productionSystem;
        g_productionSystem = nullptr;
    }
}