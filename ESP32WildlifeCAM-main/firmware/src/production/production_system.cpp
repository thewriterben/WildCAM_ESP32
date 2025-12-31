/**
 * Production System Implementation
 * 
 * Main coordinator for Phase 4 production deployment features
 * Including satellite communication integration for remote area connectivity
 */

#include "production_system.h"
#include "../debug_utils.h"
#include "../satellite_comm.h"
#include "../satellite_integration.h"
#include "../network_selector.h"

// Global instance
ProductionSystem* g_productionSystem = nullptr;

// Constructor
ProductionSystem::ProductionSystem() 
    : status_(PROD_INITIALIZING), initialized_(false), startTime_(0),
      otaManager_(nullptr), configManager_(nullptr), environmentalAdapter_(nullptr),
      speciesDetector_(nullptr), cloudIntegrator_(nullptr), dataProtector_(nullptr),
      satelliteComm_(nullptr), networkSelector_(nullptr), satelliteInitialized_(false),
      lastSatelliteCheck_(0), lastSatelliteQueueProcess_(0),
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
    
    // Initialize Satellite Communication
    if (config_.enableSatellite) {
        if (!initializeSatelliteModule()) {
            DEBUG_PRINTLN("WARNING: Satellite Communication initialization failed");
            // Satellite is optional, don't fail initialization
            satelliteInitialized_ = false;
        } else {
            satelliteInitialized_ = true;
            logProductionEvent("Satellite", "Satellite communication initialized successfully");
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
    
    // Fallback to satellite if cloud sync fails and satellite auto-fallback is enabled
    if (config_.satelliteAutoFallback && satelliteInitialized_) {
        // Check if cloud sync was successful or if we should use satellite anyway
        if (!healthMetrics_.cloudHealthy || !healthMetrics_.networkConnected) {
            String fallbackMessage = "WILDLIFE:" + detection.speciesName + 
                                    ",CONF:" + String(detection.confidence, 2) +
                                    ",TIME:" + String(detection.timestamp);
            
            SatelliteMessagePriority priority = detection.conservationConcern ? 
                                                SAT_PRIORITY_HIGH : SAT_PRIORITY_NORMAL;
            
            attemptSatelliteFallback(fallbackMessage, priority);
        }
    }
    
    // Send conservation alerts via satellite for endangered species
    if (config_.conservationMode && detection.conservationConcern && 
        satelliteInitialized_ && config_.enableSatellite) {
        sendWildlifeAlertViaSatellite(detection.speciesName, detection.confidence);
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
    
    // Check satellite communication system
    if (satelliteInitialized_ && satelliteComm_) {
        // Check satellite availability periodically
        if (millis() - lastSatelliteCheck_ > 300000) { // Every 5 minutes
            checkSatelliteNetwork();
        }
        
        // Update satellite health metrics
        updateSatelliteHealthMetrics();
        
        // Process satellite queue periodically
        if (millis() - lastSatelliteQueueProcess_ > 600000) { // Every 10 minutes
            processSatelliteQueue();
        }
        
        // Optimize satellite usage
        optimizeSatelliteUsage();
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
    
    // Satellite health check (lower weight since it's a backup network)
    if (satelliteInitialized_ && satelliteComm_) {
        componentCount++;
        if (!healthMetrics_.satelliteHealthy) score -= 5.0;
        
        // Additional penalty if satellite is the only available network
        if (!healthMetrics_.networkConnected && !healthMetrics_.satelliteAvailable) {
            score -= 10.0; // No connectivity at all
        }
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
    
    // Cleanup satellite communication
    if (satelliteComm_) {
        satelliteComm_->enterSleepMode();
        delete satelliteComm_;
        satelliteComm_ = nullptr;
    }
    
    if (networkSelector_) {
        delete networkSelector_;
        networkSelector_ = nullptr;
    }
    
    satelliteInitialized_ = false;
    
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

// ============================================================================
// Satellite Communication Implementation
// ============================================================================

// Initialize satellite module
bool ProductionSystem::initializeSatelliteModule() {
    DEBUG_PRINTLN("Initializing satellite communication module...");
    
    // Create satellite communication instance
    satelliteComm_ = new SatelliteComm();
    if (!satelliteComm_) {
        DEBUG_PRINTLN("ERROR: Failed to create SatelliteComm instance");
        return false;
    }
    
    // Configure satellite settings based on production config
    SatelliteConfig satConfig;
    satConfig.enabled = config_.enableSatellite;
    satConfig.module = config_.satelliteModule;
    satConfig.transmissionInterval = config_.satelliteTransmitInterval;
    satConfig.maxDailyMessages = config_.maxSatelliteMessagesDaily;
    satConfig.maxDailyCost = config_.maxSatelliteCostDaily;
    satConfig.costOptimization = config_.satelliteCostOptimization;
    satConfig.prioritizeEmergency = true;
    satConfig.enableMessageCompression = true;
    satConfig.useScheduledTransmission = !config_.satelliteEmergencyOnly;
    satConfig.autoPassPrediction = true;
    
    // Configure based on deployment scenario
    configureSatelliteForScenario(config_.scenario);
    
    if (!satelliteComm_->configure(satConfig)) {
        DEBUG_PRINTLN("ERROR: Failed to configure satellite communication");
        delete satelliteComm_;
        satelliteComm_ = nullptr;
        return false;
    }
    
    // Initialize the satellite module
    if (!satelliteComm_->initialize(config_.satelliteModule)) {
        DEBUG_PRINTLN("WARNING: Satellite module not responding - will retry later");
        // Don't fail here - satellite may not be available yet
    }
    
    // Create network selector for multi-network fallback
    networkSelector_ = new NetworkSelector();
    if (networkSelector_) {
        // Initialize with satellite comm for fallback
        networkSelector_->initialize(nullptr, satelliteComm_);
        networkSelector_->setCostOptimization(config_.satelliteCostOptimization);
    }
    
    lastSatelliteCheck_ = millis();
    lastSatelliteQueueProcess_ = millis();
    
    DEBUG_PRINTLN("Satellite communication module initialized");
    return true;
}

// Initialize satellite communication (public interface)
bool ProductionSystem::initializeSatelliteCommunication() {
    if (satelliteInitialized_) {
        return true;
    }
    
    if (!config_.enableSatellite) {
        DEBUG_PRINTLN("Satellite communication is disabled in configuration");
        return false;
    }
    
    return initializeSatelliteModule();
}

// Send message via satellite
bool ProductionSystem::sendViaSatellite(const String& message, SatelliteMessagePriority priority) {
    if (!satelliteInitialized_ || !satelliteComm_) {
        DEBUG_PRINTLN("Satellite communication not initialized");
        return false;
    }
    
    // Check if satellite-only mode and not emergency
    if (config_.satelliteEmergencyOnly && priority != SAT_PRIORITY_EMERGENCY) {
        DEBUG_PRINTLN("Non-emergency message blocked - satellite emergency-only mode");
        return false;
    }
    
    bool success = satelliteComm_->sendMessage(message, priority);
    
    if (success) {
        logProductionEvent("Satellite", "Message sent: " + message.substring(0, 30) + "...");
        updateSatelliteHealthMetrics();
    } else {
        logProductionEvent("Satellite", "Message send failed, queued for retry");
    }
    
    return success;
}

// Send wildlife alert via satellite
bool ProductionSystem::sendWildlifeAlertViaSatellite(const String& species, float confidence) {
    if (!satelliteInitialized_ || !satelliteComm_) {
        return false;
    }
    
    String alertMessage = "WILDLIFE:" + species + ",CONF:" + String(confidence, 2);
    
    // Determine priority based on species and confidence
    SatelliteMessagePriority priority = SAT_PRIORITY_NORMAL;
    if (confidence > 0.9) {
        priority = SAT_PRIORITY_HIGH;
    }
    
    // Check for conservation concern species
    if (config_.conservationMode && config_.endangeredSpeciesAlert) {
        // High priority for potential endangered species
        if (species.indexOf("ENDANGERED") != -1 || species.indexOf("RARE") != -1) {
            priority = SAT_PRIORITY_HIGH;
            alertMessage += ",CONSERVATION:TRUE";
        }
    }
    
    bool success = satelliteComm_->sendWildlifeAlert(alertMessage);
    
    if (success) {
        logProductionEvent("Satellite", "Wildlife alert sent: " + species);
    }
    
    return success;
}

// Send emergency via satellite
bool ProductionSystem::sendEmergencyViaSatellite(const String& emergency) {
    if (!satelliteComm_) {
        // Try to initialize if not done
        if (!initializeSatelliteModule()) {
            DEBUG_PRINTLN("CRITICAL: Cannot send emergency - satellite unavailable");
            return false;
        }
    }
    
    // Enable emergency mode
    satelliteComm_->enableEmergencyMode();
    
    bool success = satelliteComm_->sendEmergencyAlert(emergency);
    
    if (success) {
        logProductionEvent("Satellite", "EMERGENCY sent: " + emergency, true);
    } else {
        logProductionEvent("Satellite", "EMERGENCY failed, attempting fallback", true);
        
        // Try network selector fallback
        if (networkSelector_) {
            const uint8_t* emergencyData = (const uint8_t*)emergency.c_str();
            success = networkSelector_->sendData(emergencyData, emergency.length(), PRIORITY_EMERGENCY);
        }
    }
    
    return success;
}

// Check satellite network availability
bool ProductionSystem::checkSatelliteNetwork() {
    if (!satelliteComm_) {
        return false;
    }
    
    bool available = satelliteComm_->checkSatelliteAvailability();
    int signalQuality = satelliteComm_->checkSignalQuality();
    
    // Update health metrics
    healthMetrics_.satelliteAvailable = available;
    healthMetrics_.satelliteSignalStrength = signalQuality;
    
    lastSatelliteCheck_ = millis();
    
    return available;
}

// Optimize satellite usage
bool ProductionSystem::optimizeSatelliteUsage() {
    if (!satelliteComm_) {
        return false;
    }
    
    // Optimize based on battery level
    float batteryLevel = healthMetrics_.batteryLevel;
    satelliteComm_->optimizePowerConsumption((uint8_t)batteryLevel);
    
    // Process stored messages during optimal windows
    if (satelliteComm_->getStoredMessageCount() > 0) {
        TransmissionWindow window = satelliteComm_->getOptimalTransmissionWindow();
        if (window == WINDOW_IMMEDIATE) {
            satelliteComm_->sendStoredMessages();
        }
    }
    
    return true;
}

// Process satellite message queue
bool ProductionSystem::processSatelliteQueue() {
    if (!satelliteComm_) {
        return false;
    }
    
    size_t storedCount = satelliteComm_->getStoredMessageCount();
    if (storedCount == 0) {
        return true;
    }
    
    // Check if satellite is available
    if (!satelliteComm_->checkSatelliteAvailability()) {
        DEBUG_PRINTLN("Satellite not available, keeping messages queued");
        return false;
    }
    
    bool success = satelliteComm_->sendStoredMessages();
    
    if (success) {
        logProductionEvent("Satellite", "Processed queued messages");
    }
    
    lastSatelliteQueueProcess_ = millis();
    return success;
}

// Update satellite configuration
bool ProductionSystem::updateSatelliteConfiguration(const SatelliteConfig& config) {
    if (!satelliteComm_) {
        return false;
    }
    
    satelliteComm_->updateConfiguration(config);
    logProductionEvent("Satellite", "Configuration updated");
    
    return true;
}

// Get satellite configuration
SatelliteConfig ProductionSystem::getSatelliteConfiguration() const {
    if (satelliteComm_) {
        return satelliteComm_->getConfiguration();
    }
    return SatelliteConfig();
}

// Check if satellite is available
bool ProductionSystem::isSatelliteAvailable() const {
    return satelliteInitialized_ && satelliteComm_ && healthMetrics_.satelliteAvailable;
}

// Check if satellite is in emergency mode
bool ProductionSystem::isSatelliteEmergencyMode() const {
    return satelliteComm_ ? satelliteComm_->isEmergencyMode() : false;
}

// Get satellite cost today
float ProductionSystem::getSatelliteCostToday() const {
    return satelliteComm_ ? satelliteComm_->getTodayCost() : 0.0f;
}

// Get satellite stored message count
size_t ProductionSystem::getSatelliteStoredMessageCount() const {
    return satelliteComm_ ? satelliteComm_->getStoredMessageCount() : 0;
}

// Update satellite health metrics
void ProductionSystem::updateSatelliteHealthMetrics() {
    if (!satelliteComm_) {
        healthMetrics_.satelliteHealthy = false;
        return;
    }
    
    healthMetrics_.satelliteHealthy = true;
    healthMetrics_.satelliteMessagesSent = satelliteComm_->getMessageCount();
    healthMetrics_.satelliteCostToday = satelliteComm_->getTodayCost();
    healthMetrics_.storedSatelliteMessages = satelliteComm_->getStoredMessageCount();
    healthMetrics_.satelliteEmergencyMode = satelliteComm_->isEmergencyMode();
    
    // Check if satellite is within cost limits
    if (!satelliteComm_->isWithinCostLimit()) {
        healthMetrics_.satelliteHealthy = false;
        logProductionEvent("Satellite", "Daily cost limit reached", false);
    }
}

// Configure satellite for deployment scenario
bool ProductionSystem::configureSatelliteForScenario(DeploymentScenario scenario) {
    if (!satelliteComm_) {
        return false;
    }
    
    SatelliteConfig satConfig = satelliteComm_->getConfiguration();
    
    switch (scenario) {
        case SCENARIO_RESEARCH:
            // Research: more frequent updates, higher budget
            satConfig.transmissionInterval = 1800;  // 30 minutes
            satConfig.maxDailyMessages = 48;
            satConfig.maxDailyCost = 50.0;
            satConfig.useScheduledTransmission = true;
            break;
            
        case SCENARIO_CONSERVATION:
            // Conservation: moderate updates, focus on alerts
            satConfig.transmissionInterval = 3600;  // 1 hour
            satConfig.maxDailyMessages = 24;
            satConfig.maxDailyCost = 25.0;
            satConfig.prioritizeEmergency = true;
            break;
            
        case SCENARIO_COMMERCIAL:
            // Commercial: cost-optimized
            satConfig.transmissionInterval = 7200;  // 2 hours
            satConfig.maxDailyMessages = 12;
            satConfig.maxDailyCost = 15.0;
            satConfig.costOptimization = true;
            break;
            
        case SCENARIO_EDUCATION:
            // Education: minimal satellite use
            satConfig.transmissionInterval = 14400;  // 4 hours
            satConfig.maxDailyMessages = 6;
            satConfig.maxDailyCost = 10.0;
            break;
            
        case SCENARIO_EMERGENCY:
            // Emergency: maximum reliability
            satConfig.transmissionInterval = 300;   // 5 minutes
            satConfig.maxDailyMessages = 288;       // No practical limit
            satConfig.maxDailyCost = 200.0;
            satConfig.costOptimization = false;
            satConfig.prioritizeEmergency = true;
            break;
            
        default:
            // Use configured values
            break;
    }
    
    satelliteComm_->updateConfiguration(satConfig);
    return true;
}

// Attempt satellite fallback
bool ProductionSystem::attemptSatelliteFallback(const String& message, SatelliteMessagePriority priority) {
    if (!config_.satelliteAutoFallback) {
        return false;
    }
    
    if (!satelliteComm_) {
        // Try to initialize on demand
        if (!initializeSatelliteModule()) {
            return false;
        }
    }
    
    DEBUG_PRINTLN("Attempting satellite fallback transmission...");
    return sendViaSatellite(message, priority);
}

// ============================================================================
// Satellite Utility Functions
// ============================================================================

bool initializeProductionSatellite() {
    return g_productionSystem ? g_productionSystem->initializeSatelliteCommunication() : false;
}

bool sendProductionSatelliteMessage(const String& message, SatelliteMessagePriority priority) {
    return g_productionSystem ? g_productionSystem->sendViaSatellite(message, priority) : false;
}

bool sendProductionEmergencyAlert(const String& emergency) {
    return g_productionSystem ? g_productionSystem->sendEmergencyViaSatellite(emergency) : false;
}

bool isProductionSatelliteAvailable() {
    return g_productionSystem ? g_productionSystem->isSatelliteAvailable() : false;
}

float getProductionSatelliteCost() {
    return g_productionSystem ? g_productionSystem->getSatelliteCostToday() : 0.0f;
}

bool checkAndUpdateFirmware() {
    if (g_productionSystem && g_productionSystem->isOperational()) {
        return g_productionSystem->performOTAUpdate();
    }
    return false;
}

bool isCloudConnected() {
    if (g_productionSystem) {
        SystemHealthMetrics health = g_productionSystem->getSystemHealth();
        return health.cloudHealthy && health.networkConnected;
    }
    return false;
}

bool syncAllData() {
    return g_productionSystem ? g_productionSystem->syncWithCloud() : false;
}