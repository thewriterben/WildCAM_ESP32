/**
 * ESP32 Wildlife Camera - Main Application with Phase 4 Production Deployment
 * 
 * A solar-powered wildlife trail camera with LoRa mesh networking,
 * intelligent motion detection, weather filtering, and comprehensive
 * AI-powered wildlife monitoring capabilities.
 * 
 * Features (Phases 1-4):
 * - ESP32-S3 with camera module and AI acceleration
 * - Solar power management with AI optimization
 * - LoRa mesh networking and multi-board communication
 * - Weather-aware motion filtering
 * - Deep sleep power optimization
 * - AI-powered species classification and behavior analysis
 * - TensorFlow Lite Micro integration
 * - Edge Impulse platform support
 * - Production deployment and OTA updates (Phase 4)
 * - Enterprise cloud integration and analytics (Phase 4)
 * - Advanced security and privacy protection (Phase 4)
 * - Real-world environmental adaptation (Phase 4)
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 3.0.0 (Phase 4 Production Ready)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <SD_MMC.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"
#include "debug_utils.h"
#include "debug_config.h"
#include "camera_handler.h"
#include "motion_filter.h"
#include "power_manager.h"
#include "wifi_manager.h"
#include "lora_mesh.h"
#include "display/hmi_system.h"
#include "hal/board_detector.h"
#include "audio/acoustic_detection.h"
#include "i18n/language_manager.h"
#include "web/web_language_integration.h"

// Environmental Sensors Integration
#include "sensors/advanced_environmental_sensors.h"

// Meshtastic Integration
#include "meshtastic/mesh_config.h"
#include "meshtastic/lora_driver.h"
#include "meshtastic/mesh_interface.h"
#include "meshtastic/wildlife_telemetry.h"
#include "meshtastic/image_mesh.h"

// Environmental Integration
extern bool initializeEnvironmentalIntegration();
extern void processEnvironmentalData();
extern void performEnvironmentalDiagnostics();

// AI/ML Integration (conditionally compiled)
#ifdef ESP32_AI_ENABLED
#include "ai/ai_wildlife_system.h"
#include "ai/ai_common.h"
#endif

// Phase 4 Production Deployment Integration
#include "production/production_system.h"

/**
 * @class SystemManager
 * @brief Central coordinator for all wildlife camera subsystems with Phase 4 production features
 * 
 * This class manages the initialization, coordination, and lifecycle
 * of all camera subsystems, providing a clean interface for the main loop.
 * Includes comprehensive AI/ML capabilities and Phase 4 production deployment features.
 */
class SystemManager {
public:
    SystemManager();
    ~SystemManager();
    
    bool init();
    void update();
    void handleMotionDetection();
    void enterDeepSleep();
    void handleLowPower();
    void logSystemStatus();
    void cleanup();
    
    // Subsystem access methods
    CameraHandler& getCamera() { return cameraHandler; }
    MotionFilter& getMotionFilter() { return motionFilter; }
    PowerManager& getPowerManager() { return powerManager; }
    WiFiManager& getWiFiManager() { return wifiManager; }
    HMISystem& getHMISystem() { return hmiSystem; }
    AcousticDetection& getAudioSystem() { return audioSystem; }
    
    // Status methods
    bool isCameraInitialized() const { return cameraHandler.isInitialized(); }
    bool isSDCardInitialized() const { return sdCardInitialized; }
    bool isLoRaInitialized() const { return loraInitialized; }
    bool isAudioInitialized() const { return audioInitialized; }
    int getDailyTriggerCount() const { return dailyTriggerCount; }
    unsigned long getBootTime() const { return bootTime; }
    
#ifdef ESP32_AI_ENABLED
    // AI-specific methods
    bool isAISystemInitialized() const { return aiSystemInitialized; }
    void handleAIAnalysis();
    void handleIntelligentCapture();
    void saveAIAnalysisMetadata(const String& filename, const WildlifeAnalysisResult& analysis);
#endif

    // Phase 4 Production methods
    bool isProductionSystemInitialized() const { return productionSystemInitialized; }
    void handleProductionUpdate();
    void performSystemHealthCheck();
    void handleProductionEvent(const ProductionEvent& event);
    void checkForOTAUpdates();
    void syncWithCloud();
    bool processWildlifeDetectionProduction(const uint8_t* imageData, uint16_t width, uint16_t height);

private:
    // Subsystem instances
    CameraHandler cameraHandler;
    MotionFilter motionFilter;
    PowerManager powerManager;
    WiFiManager wifiManager;
    HMISystem hmiSystem;
    AcousticDetection audioSystem;
    std::unique_ptr<CameraBoard> detectedBoard;
    
    // Language support
    WebLanguageIntegration* webLanguageIntegration;
    
    // Meshtastic subsystems
    LoRaDriver* loraDriver;
    MeshInterface* meshInterface;
    WildlifeTelemetry* wildlifeTelemetry;
    ImageMesh* imageMesh;
    
    // System state
    bool sdCardInitialized;
    bool loraInitialized;
    bool audioInitialized;
    bool meshInitialized;
    unsigned long lastMotionTime;
    unsigned long bootTime;
    int dailyTriggerCount;
    unsigned long lastStatusCheck;
    
#ifdef ESP32_AI_ENABLED
    // AI-related state
    bool aiSystemInitialized;
    unsigned long lastAIAnalysis;
    WildlifeAnalysisResult lastAnalysisResult;
#endif

    // Phase 4 Production state
    bool productionSystemInitialized;
    unsigned long lastProductionUpdate;
    unsigned long lastHealthCheck;
    unsigned long lastOTACheck;
    SystemHealthMetrics lastHealthMetrics;
    
    // Private methods
    bool initializeFileSystem();
    bool initializeSDCard();
    bool initializeLoRa();
    bool initializeMeshNetwork();
    bool initializeWildlifeTelemetry();
    bool initializeImageMesh();
    bool initializeAudioSystem();
    bool initializeProductionSystem();
    bool isWithinActiveHours();
    void resetDailyCounts();
    String createImageFilename();
    
#ifdef ESP32_AI_ENABLED
    bool initializeAISystem();
#endif
};

// Global system manager instance
SystemManager systemManager;

/**
 * Arduino Setup Function
 * Initializes all system components and prepares for operation
 */
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);  // Allow serial to initialize
    
    // Initialize enhanced debug system first
    EnhancedDebugSystem::init();
    
    // Apply appropriate debug preset based on build configuration
    #ifdef DEBUG_DEVELOPMENT_MODE
    DebugPresets::setDevelopmentMode();
    #elif defined(DEBUG_PRODUCTION_MODE)
    DebugPresets::setProductionMode();
    #elif defined(DEBUG_PERFORMANCE_MODE)
    DebugPresets::setPerformanceMode();
    #else
    DebugPresets::setFieldMode(); // Default for wildlife camera deployment
    #endif
    
    DEBUG_SYSTEM_INFO("\n========================================");
    DEBUG_SYSTEM_INFO("ESP32 Wildlife Camera Starting...");
    DEBUG_SYSTEM_INFO("Firmware Version: %s", FIRMWARE_VERSION);
    DEBUG_SYSTEM_INFO("Build Date: %s %s", BUILD_DATE, BUILD_TIME);
    DEBUG_SYSTEM_INFO("========================================\n");
    
    // Initialize system manager
    if (!systemManager.init()) {
        DEBUG_SYSTEM_ERROR("CRITICAL: System initialization failed!");
        DEBUG_SYSTEM_ERROR("Entering emergency deep sleep...");
        esp_deep_sleep(60 * 1000000); // Sleep for 1 minute and retry
    }
    
    // Log initial system status
    systemManager.logSystemStatus();
    
    DEBUG_SYSTEM_INFO("System initialization complete!");
    DEBUG_SYSTEM_INFO("Wildlife camera ready for operation.\n");
}

/**
 * Arduino Main Loop
 * Handles motion detection, power management, and system monitoring
 */
void loop() {
    // Process debug commands from serial input
    DebugController::processSerialCommands();
    
    // Update all subsystems
    systemManager.update();
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}

// SystemManager implementation

/**
 * SystemManager Constructor
 */
SystemManager::SystemManager() 
    : webLanguageIntegration(nullptr), sdCardInitialized(false), loraInitialized(false), audioInitialized(false), 
      lastMotionTime(0), bootTime(0), dailyTriggerCount(0), lastStatusCheck(0)
#ifdef ESP32_AI_ENABLED
    , aiSystemInitialized(false), lastAIAnalysis(0)
#endif
    , productionSystemInitialized(false), lastProductionUpdate(0), 
      lastHealthCheck(0), lastOTACheck(0)
{
}

/**
 * SystemManager Destructor
 */
SystemManager::~SystemManager() {
    cleanup();
}

/**
 * Initialize all system components
 */
bool SystemManager::init() {
    DEBUG_SYSTEM_INFO("Initializing system components...");
    
    bootTime = millis();
    
    // Initialize file system first
    if (!initializeFileSystem()) {
        DEBUG_SYSTEM_ERROR("File system initialization failed");
        return false;
    }
    
    // Initialize power management
    DEBUG_TIMER_START("power_init");
    if (!powerManager.init()) {
        DEBUG_POWER_ERROR("Power management initialization failed");
        return false;
    }
    DEBUG_TIMER_END("power_init");
    
    // Initialize motion detection
    DEBUG_TIMER_START("motion_init");
    if (!motionFilter.init()) {
        DEBUG_MOTION_ERROR("Motion filter initialization failed");
        return false;
    }
    DEBUG_TIMER_END("motion_init");
    
    // Initialize environmental sensors system
    DEBUG_TIMER_START("environmental_init");
    if (initializeEnvironmentalIntegration()) {
        DEBUG_SYSTEM_INFO("Environmental sensors system initialized successfully");
    } else {
        DEBUG_SYSTEM_WARN("Warning: Environmental sensors initialization failed");
        // Not critical - system can continue with basic functionality
    }
    DEBUG_TIMER_END("environmental_init");
    
    // Initialize camera
    DEBUG_TIMER_START("camera_init");
    if (!cameraHandler.init()) {
        DEBUG_CAMERA_ERROR("Camera initialization failed");
        return false;
    }
    DEBUG_TIMER_END("camera_init");
    
    // Detect and initialize board-specific features
    DEBUG_TIMER_START("board_init");
    BoardDetector detector;
    BoardType boardType = detector.detectBoard();
    detectedBoard = detector.createBoard(boardType);
    if (detectedBoard) {
        DEBUG_SYSTEM_INFO("Detected board: %s", detectedBoard->getBoardName());
        if (!detectedBoard->init()) {
            DEBUG_SYSTEM_WARN("Warning: Board initialization failed");
        }
    } else {
        DEBUG_SYSTEM_WARN("Warning: Could not detect board type");
    }
    DEBUG_TIMER_END("board_init");
    
    // Initialize HMI system (display and user interface)
    DEBUG_TIMER_START("hmi_init");
    if (detectedBoard && hmiSystem.init(detectedBoard.get())) {
        DEBUG_SYSTEM_INFO("HMI system initialized successfully");
    } else {
        DEBUG_SYSTEM_INFO("No display detected - running in headless mode");
    }
    DEBUG_TIMER_END("hmi_init");
    
    // Initialize SD card storage
    if (!initializeSDCard()) {
        DEBUG_SYSTEM_WARN("Warning: SD card initialization failed");
        // Not critical - system can continue without SD card
    }
    
    // Initialize WiFi if enabled
    if (WIFI_ENABLED) {
        DEBUG_TIMER_START("wifi_init");
        if (!wifiManager.init()) {
            DEBUG_WIFI_WARN("Warning: WiFi initialization failed");
            // Not critical - system can continue without WiFi
        }
        DEBUG_TIMER_END("wifi_init");
    }
    
    // Initialize language manager for multi-language support
    DEBUG_TIMER_START("language_init");
    if (!g_languageManager.begin(LanguageCode::EN)) {
        DEBUG_SYSTEM_WARN("Warning: Language manager initialization failed");
        // Not critical - system can continue with default language
    } else {
        DEBUG_SYSTEM_INFO("Language manager initialized - Current language: %s", 
                         g_languageManager.getLanguageString().c_str());
        
        // Initialize web language integration
        webLanguageIntegration = new WebLanguageIntegration(g_languageManager);
        if (webLanguageIntegration) {
            DEBUG_SYSTEM_INFO("Web language integration initialized");
        }
    }
    DEBUG_TIMER_END("language_init");
    
    // Initialize LoRa mesh networking if enabled
    if (LORA_ENABLED) {
        DEBUG_TIMER_START("lora_init");
        loraInitialized = initializeLoRa();
        DEBUG_TIMER_END("lora_init");
        if (loraInitialized) {
            DEBUG_LORA_INFO("LoRa mesh network initialized");
        } else {
            DEBUG_LORA_WARN("Warning: LoRa initialization failed");
        }
    }
    
    // Initialize audio monitoring system if enabled
    if (AUDIO_MONITORING_ENABLED) {
        DEBUG_TIMER_START("audio_init");
        audioInitialized = initializeAudioSystem();
        DEBUG_TIMER_END("audio_init");
        if (audioInitialized) {
            DEBUG_SYSTEM_INFO("Audio monitoring system initialized");
        } else {
            DEBUG_SYSTEM_WARN("Warning: Audio system initialization failed");
        }
    }
    
#ifdef ESP32_AI_ENABLED
    // Initialize AI system if enabled
    DEBUG_TIMER_START("ai_init");
    if (initializeAISystem()) {
        aiSystemInitialized = true;
        DEBUG_AI_INFO("AI Wildlife System initialized successfully");
    } else {
        aiSystemInitialized = false;
        DEBUG_AI_WARN("Warning: AI system initialization failed - running without AI");
    }
    DEBUG_TIMER_END("ai_init");
#endif

    // Initialize Phase 4 Production System
    DEBUG_TIMER_START("production_init");
    if (initializeProductionSystem()) {
        productionSystemInitialized = true;
        DEBUG_SYSTEM_INFO("Phase 4 Production System initialized successfully");
    } else {
        productionSystemInitialized = false;
        DEBUG_SYSTEM_WARN("Warning: Production system initialization failed - running in basic mode");
    }
    DEBUG_TIMER_END("production_init");
    
    DEBUG_TIMER_PRINT();
    return true;
}

/**
 * Update all subsystems
 */
void SystemManager::update() {
    unsigned long currentTime = millis();
    
    // Update power management
    powerManager.update();
    
    // Process environmental data
    processEnvironmentalData();
    
    // Update HMI system (display, menus, input)
    if (hmiSystem.isInitialized()) {
        hmiSystem.updateDisplay();
        hmiSystem.processInput();
    }
    
    // Update WiFi if enabled
    if (WIFI_ENABLED) {
        wifiManager.update();
    }
    
    // Check system status every 30 seconds
    if (currentTime - lastStatusCheck > STATUS_REPORT_INTERVAL) {
        logSystemStatus();
        lastStatusCheck = currentTime;
        
        // Reset daily counters at midnight
        resetDailyCounts();
        
        // Handle low power situations
        if (powerManager.getBatteryVoltage() < BATTERY_CRITICAL_THRESHOLD) {
            handleLowPower();
            return;
        }
    }
    
    // Check for motion detection
    if (motionFilter.isMotionDetected()) {
        DEBUG_MOTION_INFO("Motion detected!");
        
        // Apply weather filtering
        if (motionFilter.isValidMotion()) {
            DEBUG_MOTION_INFO("Valid motion after filtering");
            
#ifdef ESP32_AI_ENABLED
            // Use AI-enhanced motion analysis if available
            if (aiSystemInitialized) {
                handleAIAnalysis();
            } else {
                handleMotionDetection();
            }
#else
            handleMotionDetection();
#endif
        } else {
            DEBUG_MOTION_DEBUG("Motion filtered out (weather conditions)");
        }
    }
    
    // Check for audio triggers if audio monitoring is enabled
    if (audioInitialized && audioSystem.isOperational()) {
        // Audio processing is handled in background task
        // Check for high-confidence wildlife detections
        auto recentDetections = audioSystem.getCallHistory(5); // Get last 5 detections
        for (const auto& detection : recentDetections) {
            if (detection.raptorCallDetected && 
                detection.confidence > WILDLIFE_DETECTION_THRESHOLD &&
                AUDIO_TRIGGERED_CAPTURE) {
                
                const char* speciesName = "Unknown";
                switch (detection.likelySpecies) {
                    case RaptorSpecies::EAGLE: speciesName = "Eagle"; break;
                    case RaptorSpecies::HAWK: speciesName = "Hawk"; break;
                    case RaptorSpecies::FALCON: speciesName = "Falcon"; break;
                    case RaptorSpecies::HARRIER: speciesName = "Harrier"; break;
                    case RaptorSpecies::KITE: speciesName = "Kite"; break;
                    case RaptorSpecies::BUZZARD: speciesName = "Buzzard"; break;
                    default: speciesName = "Unknown Raptor"; break;
                }
                
                DEBUG_SYSTEM_INFO("Audio triggered capture: %s (%.2f confidence)", 
                                 speciesName, detection.confidence);
                
                // Trigger camera capture for audio detection
                handleMotionDetection(); // Reuse motion detection handler
                break; // Only trigger once per update cycle
            }
        }
    }
    
#ifdef ESP32_AI_ENABLED
    // Perform intelligent capture check (AI-driven)
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldTriggerCapture()) {
        DEBUG_PRINTLN("AI triggered intelligent capture");
        handleIntelligentCapture();
    }
#endif
    
    // Handle LoRa mesh communication
    if (loraInitialized) {
        LoraMesh::processMessages();
    }
    
#ifdef ESP32_AI_ENABLED
    // AI-optimized deep sleep decision
    if (aiSystemInitialized && g_aiSystem && g_aiSystem->shouldEnterDeepSleep()) {
        DEBUG_PRINTLN("AI recommends deep sleep");
        enterDeepSleep();
        return;
    }
#endif

    // Update Phase 4 Production System
    if (productionSystemInitialized) {
        handleProductionUpdate();
    }
    
    // Enter deep sleep if no activity and low power
    if (currentTime - lastMotionTime > DEEP_SLEEP_DURATION * 1000 && 
        powerManager.shouldEnterLowPower()) {
        enterDeepSleep();
    }
}

/**
 * Handle motion detection event
 */
void SystemManager::handleMotionDetection() {
    lastMotionTime = millis();
    
    // Check if within active hours
    if (!isWithinActiveHours()) {
        DEBUG_MOTION_DEBUG("Motion detected outside active hours");
        return;
    }
    
    // Check daily trigger limit
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        DEBUG_MOTION_WARN("Daily trigger limit reached");
        return;
    }
    
    DEBUG_TIMER_START("image_capture");
    
    // Capture image
    if (cameraHandler.isInitialized()) {
        camera_fb_t* fb = cameraHandler.captureImage();
        if (fb) {
            dailyTriggerCount++;
            
            // Save image to SD card
            if (sdCardInitialized) {
                String filename = cameraHandler.saveImage(fb, IMAGE_FOLDER);
                DEBUG_CAMERA_INFO("Image saved: %s", filename.c_str());
                
                // Transmit via LoRa if enabled
                if (loraInitialized && IMAGE_COMPRESSION_ENABLED) {
                    DEBUG_LORA_DEBUG("Transmitting image via LoRa: %s", filename.c_str());
                    LoraMesh::transmitImage(fb, filename);
                }
            }
            
            esp_camera_fb_return(fb);
        } else {
            DEBUG_CAMERA_ERROR("Failed to capture image");
        }
    }
    
    DEBUG_TIMER_END("image_capture");
    
    // Flash LED to indicate capture
    digitalWrite(CHARGING_LED_PIN, HIGH);
    delay(100);
    digitalWrite(CHARGING_LED_PIN, LOW);
}

/**
 * Enter deep sleep mode to conserve power
 */
void SystemManager::enterDeepSleep() {
    DEBUG_SYSTEM_INFO("Entering deep sleep mode...");
    
    // Print final system stats before sleep
    DEBUG_PRINT_MEMORY();
    DEBUG_TIMER_PRINT();
    
    // Cleanup all subsystems
    cleanup();
    
    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 1000000); // Convert to microseconds
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1); // PIR sensor wake-up
    
    DEBUG_SYSTEM_INFO("Sleeping for %d seconds", DEEP_SLEEP_DURATION);
    delay(100); // Allow debug message to be sent
    
    esp_deep_sleep_start();
}

/**
 * Handle critical low power situation
 */
void SystemManager::handleLowPower() {
    DEBUG_POWER_WARN("Critical low power - entering emergency mode");
    
    // Enter power saving mode
    powerManager.enterPowerSaving();
    
    // Disable non-essential systems
    if (wifiManager.isConnected()) {
        DEBUG_WIFI_INFO("Disconnecting WiFi for power saving");
        wifiManager.disconnect();
    }
    
    // Reduce camera quality if possible
    // This would require additional camera configuration methods
    
    // Enter deep sleep for longer period
    DEBUG_POWER_INFO("Entering extended deep sleep...");
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION * 2 * 1000000); // Double sleep time
    esp_deep_sleep_start();
}

/**
 * Log current system status
 */
void SystemManager::logSystemStatus() {
    PowerStats powerStats = powerManager.getPowerStats();
    MotionFilterStatus motionStatus = motionFilter.getStatus();
    CameraStatus cameraStatus = cameraHandler.getStatus();
    WiFiStatusInfo wifiStatus = wifiManager.getStatus();
    
    DEBUG_SYSTEM_INFO("=== System Status ===");
    DEBUG_SYSTEM_INFO("Uptime: %lu seconds", (millis() - bootTime) / 1000);
    DEBUG_POWER_INFO("Battery: %.2fV (%d%%) %s", 
                     powerStats.batteryVoltage, powerStats.batteryPercentage,
                     powerStats.isCharging ? "(Charging)" : "(Discharging)");
    DEBUG_POWER_INFO("Solar: %.2fV", powerStats.solarVoltage);
    DEBUG_SYSTEM_INFO("Daily triggers: %d/%d", dailyTriggerCount, MAX_DAILY_TRIGGERS);
    DEBUG_CAMERA_INFO("Camera: %s (%s)", 
                      cameraStatus.initialized ? "OK" : "Error",
                      cameraStatus.boardName);
    DEBUG_SYSTEM_INFO("SD Card: %s", sdCardInitialized ? "OK" : "Error");
    DEBUG_LORA_INFO("LoRa: %s", loraInitialized ? "OK" : "Disabled");
    
    if (wifiStatus.initialized) {
        if (wifiStatus.status == WIFI_STATUS_CONNECTED) {
            DEBUG_WIFI_INFO("WiFi: Connected to %s, IP: %s, RSSI: %d dBm", 
                           wifiStatus.ssid.c_str(), wifiStatus.ipAddress.c_str(), wifiStatus.rssi);
        } else {
            DEBUG_WIFI_INFO("WiFi: Enabled (Disconnected)");
        }
    } else {
        DEBUG_WIFI_INFO("WiFi: Disabled");
    }
    
    DEBUG_MOTION_INFO("Motion Filter: %s", motionStatus.initialized ? "Active" : "Inactive");
    DEBUG_SYSTEM_INFO("Audio System: %s", audioInitialized ? "Active" : "Disabled");
    if (audioInitialized && audioSystem.isOperational()) {
        auto performance = audioSystem.getPerformanceStats();
        DEBUG_SYSTEM_INFO("Audio: %d samples processed, %d calls detected", 
                         performance.samplesProcessed, performance.callsDetected);
    }
    DEBUG_MEMORY_INFO("Free heap: %d bytes", ESP.getFreeHeap());
    DEBUG_SYSTEM_INFO("====================");
    
    // Print detailed memory info periodically
    static unsigned long lastMemoryPrint = 0;
    if (millis() - lastMemoryPrint > 300000) { // Every 5 minutes
        DEBUG_PRINT_MEMORY();
        lastMemoryPrint = millis();
    }
}

/**
 * Cleanup all system resources
 */
void SystemManager::cleanup() {
    DEBUG_SYSTEM_INFO("Cleaning up system resources...");
    
    cameraHandler.cleanup();
    motionFilter.cleanup();
    powerManager.cleanup();
    wifiManager.cleanup();
    
    if (webLanguageIntegration) {
        DEBUG_SYSTEM_DEBUG("Cleaning up web language integration");
        delete webLanguageIntegration;
        webLanguageIntegration = nullptr;
    }
    
    if (audioInitialized) {
        DEBUG_SYSTEM_DEBUG("Cleaning up audio system");
        // Audio system cleanup is handled in the destructor
        audioInitialized = false;
    }
    
    if (loraInitialized) {
        DEBUG_LORA_DEBUG("Cleaning up LoRa subsystem");
        LoraMesh::cleanup();
        loraInitialized = false;
    }
}

/**
 * Initialize LittleFS file system
 */
bool SystemManager::initializeFileSystem() {
    DEBUG_SYSTEM_INFO("Initializing file system...");
    
    if (!LittleFS.begin()) {
        DEBUG_SYSTEM_WARN("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            DEBUG_SYSTEM_INFO("LittleFS formatted successfully");
            if (!LittleFS.begin()) {
                DEBUG_SYSTEM_ERROR("LittleFS mount failed after format");
                return false;
            }
        } else {
            DEBUG_SYSTEM_ERROR("LittleFS format failed");
            return false;
        }
    }
    
    DEBUG_SYSTEM_INFO("File system initialized");
    return true;
}

/**
 * Initialize SD card storage
 */
bool SystemManager::initializeSDCard() {
    DEBUG_SYSTEM_INFO("Initializing SD card...");
    
    if (!SD_MMC.begin()) {
        DEBUG_SYSTEM_ERROR("SD Card initialization failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        DEBUG_SYSTEM_ERROR("No SD card attached");
        return false;
    }
    
    const char* cardTypeStr = "UNKNOWN";
    if (cardType == CARD_MMC) {
        cardTypeStr = "MMC";
    } else if (cardType == CARD_SD) {
        cardTypeStr = "SDSC";
    } else if (cardType == CARD_SDHC) {
        cardTypeStr = "SDHC";
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    DEBUG_SYSTEM_INFO("SD Card Type: %s, Size: %lluMB", cardTypeStr, cardSize);
    
    // Create required directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        SD_MMC.mkdir(IMAGE_FOLDER);
        DEBUG_SYSTEM_INFO("Created directory: %s", IMAGE_FOLDER);
    }
    
    if (!SD_MMC.exists(LOG_FOLDER)) {
        SD_MMC.mkdir(LOG_FOLDER);
        DEBUG_SYSTEM_INFO("Created directory: %s", LOG_FOLDER);
    }
    
    sdCardInitialized = true;
    DEBUG_SYSTEM_INFO("SD card initialized successfully");
    return true;
}

/**
 * Initialize LoRa mesh networking
 */
bool SystemManager::initializeLoRa() {
    DEBUG_LORA_INFO("Initializing LoRa mesh networking...");
    return LoraMesh::init();
}

/**
 * Initialize audio monitoring system
 */
bool SystemManager::initializeAudioSystem() {
    DEBUG_SYSTEM_INFO("Initializing audio monitoring system...");
    
    // Configure audio system based on scenario
    RaptorScenario scenario = RaptorScenario::GENERAL_MONITORING;
    
    // Initialize with default audio configuration
    AudioConfig audioConfig;
    audioConfig.sampleRate_Hz = AUDIO_DEFAULT_SAMPLE_RATE;
    audioConfig.enableVAD = true;
    audioConfig.vadThreshold = SOUND_DETECTION_THRESHOLD;
    audioConfig.enableNoiseReduction = WIND_NOISE_FILTER_ENABLED;
    
    // Initialize the acoustic detection system
    if (!audioSystem.init(scenario, audioConfig)) {
        DEBUG_SYSTEM_ERROR("Failed to initialize acoustic detection system");
        return false;
    }
    
    DEBUG_SYSTEM_INFO("Audio monitoring system ready");
    return true;
}

/**
 * Check if current time is within active hours
 */
bool SystemManager::isWithinActiveHours() {
    if (!NTP_ENABLED) {
        return true; // Always active if no time reference
    }
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return true; // Default to active if time not available
    }
    
    int currentHour = timeinfo.tm_hour;
    
    if (TRIGGER_ACTIVE_HOURS_START <= TRIGGER_ACTIVE_HOURS_END) {
        // Normal case: start < end (e.g., 6 AM to 8 PM)
        return (currentHour >= TRIGGER_ACTIVE_HOURS_START && 
                currentHour < TRIGGER_ACTIVE_HOURS_END);
    } else {
        // Overnight case: start > end (e.g., 8 PM to 6 AM)
        return (currentHour >= TRIGGER_ACTIVE_HOURS_START || 
                currentHour < TRIGGER_ACTIVE_HOURS_END);
    }
}

/**
 * Reset daily counters at midnight
 */
void SystemManager::resetDailyCounts() {
    if (!NTP_ENABLED) return;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    static int lastDay = -1;
    if (lastDay != timeinfo.tm_mday) {
        if (lastDay != -1) { // Not the first check
            DEBUG_SYSTEM_INFO("Resetting daily counters");
            dailyTriggerCount = 0;
            
#ifdef ESP32_AI_ENABLED
            // Reset AI system daily metrics if available
            if (aiSystemInitialized && g_aiSystem) {
                DEBUG_AI_INFO("Resetting AI daily metrics");
                // This would reset daily AI metrics if implemented
            }
#endif
            powerManager.resetStats();
            motionFilter.resetStats();
        }
        lastDay = timeinfo.tm_mday;
    }
}

#ifdef ESP32_AI_ENABLED
/**
 * Initialize AI Wildlife System
 */
bool SystemManager::initializeAISystem() {
    DEBUG_PRINTLN("Initializing AI Wildlife System...");
    
    // Initialize the global AI system
    if (::initializeAISystem()) {
        DEBUG_PRINTLN("AI Wildlife System initialized successfully");
        
        // Load AI models if available
        if (g_aiSystem) {
            // Try to load models from SD card or flash
            bool modelsLoaded = g_aiSystem->loadModels("/models");
            if (modelsLoaded) {
                DEBUG_PRINTLN("AI models loaded successfully");
            } else {
                DEBUG_PRINTLN("Warning: Some AI models failed to load");
            }
            
            // Configure AI system for wildlife monitoring
            AIProcessingConfig aiConfig;
            aiConfig.enableSpeciesClassification = true;
            aiConfig.enableBehaviorAnalysis = true;
            aiConfig.enableMotionDetection = true;
            aiConfig.enableHumanDetection = true;
            aiConfig.powerOptimized = true;
            aiConfig.confidenceThreshold = 0.7f;
            aiConfig.processingInterval = 3000; // 3 seconds between AI analyses
            
            g_aiSystem->configure(aiConfig);
            
            // Enable intelligent scheduling for power optimization
            g_aiSystem->enableIntelligentScheduling(true);
            g_aiSystem->enableContinuousLearning(true);
        }
        return true;
    } else {
        DEBUG_PRINTLN("Warning: AI system initialization failed - running without AI");
        return false;
    }
}

/**
 * Handle AI-enhanced motion analysis
 */
void SystemManager::handleAIAnalysis() {
    if (!aiSystemInitialized || !g_aiSystem) {
        // Fallback to traditional motion handling
        handleMotionDetection();
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Check if enough time has passed since last AI analysis
    if (currentTime - lastAIAnalysis < 2000) { // Minimum 2 seconds between analyses
        return;
    }
    
    lastAIAnalysis = currentTime;
    lastMotionTime = currentTime;
    
    DEBUG_PRINTLN("Starting AI-enhanced motion analysis...");
    
    // Check if within active hours
    if (!isWithinActiveHours()) {
        DEBUG_PRINTLN("Motion detected outside active hours");
        return;
    }
    
    // Check daily trigger limit
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        DEBUG_PRINTLN("Daily trigger limit reached");
        return;
    }
    
    // Capture image for AI analysis
    if (cameraHandler.isInitialized()) {
        camera_fb_t* fb = cameraHandler.captureImage();
        if (fb) {
            // Convert to CameraFrame for AI processing
            CameraFrame frame(fb);
            
            // Perform AI analysis
            WildlifeAnalysisResult analysis = g_aiSystem->analyzeFrame(frame);
            lastAnalysisResult = analysis;
            
            // Log analysis results
            if (analysis.species.species != SpeciesType::UNKNOWN) {
                DEBUG_PRINTF("AI Analysis - Species: %s, Confidence: %.2f%%\n",
                           speciesTypeToString(analysis.species.species),
                           analysis.species.confidence * 100);
                
                if (analysis.behavior.primaryBehavior != BehaviorType::UNKNOWN) {
                    DEBUG_PRINTF("AI Analysis - Behavior: %s, Confidence: %.2f%%\n",
                               behaviorTypeToString(analysis.behavior.primaryBehavior),
                               analysis.behavior.confidence * 100);
                }
            }
            
            // Decide whether to save the image based on AI analysis
            bool shouldSave = false;
            
            if (analysis.overallConfidence >= 0.6f) {
                shouldSave = true;
                DEBUG_PRINTLN("High confidence detection - saving image");
            } else if (analysis.species.species != SpeciesType::UNKNOWN && 
                      analysis.species.confidence >= 0.4f) {
                shouldSave = true;
                DEBUG_PRINTLN("Potential wildlife detected - saving image");
            } else if (analysis.humanPresenceDetected) {
                shouldSave = true;
                DEBUG_PRINTLN("Human presence detected - saving image");
            } else if (analysis.threatDetected) {
                shouldSave = true;
                DEBUG_PRINTLN("Threat detected - saving image");
            }
            
            if (shouldSave && sdCardInitialized) {
                dailyTriggerCount++;
                
                String filename = cameraHandler.saveImage(fb, IMAGE_FOLDER);
                DEBUG_PRINTF("AI-triggered image saved: %s\n", filename.c_str());
                
                // Save AI analysis metadata
                String metadataFilename = filename;
                metadataFilename.replace(".jpg", "_ai.json");
                saveAIAnalysisMetadata(metadataFilename, analysis);
                
                // Notify AI system of image capture
                g_aiSystem->onImageCaptured(frame, filename);
                
                // Transmit via LoRa if enabled and high priority
                if (loraInitialized && IMAGE_COMPRESSION_ENABLED && 
                    analysis.overallConfidence >= 0.8f) {
                    LoraMesh::transmitImage(fb, filename);
                }
            } else {
                DEBUG_PRINTLN("AI analysis below threshold - not saving image");
            }
            
            esp_camera_fb_return(fb);
        } else {
            DEBUG_PRINTLN("Error: Failed to capture image for AI analysis");
        }
    }
}

/**
 * Handle intelligent capture (AI-triggered without motion)
 */
void SystemManager::handleIntelligentCapture() {
    DEBUG_PRINTLN("Processing intelligent capture trigger...");
    
    // Check basic conditions
    if (!isWithinActiveHours()) {
        DEBUG_PRINTLN("Intelligent capture outside active hours");
        return;
    }
    
    if (dailyTriggerCount >= MAX_DAILY_TRIGGERS) {
        DEBUG_PRINTLN("Daily trigger limit reached for intelligent capture");
        return;
    }
    
    // Perform the AI analysis and capture
    handleAIAnalysis();
}

/**
 * Save AI analysis metadata to JSON file
 */
void SystemManager::saveAIAnalysisMetadata(const String& filename, const WildlifeAnalysisResult& analysis) {
    if (!sdCardInitialized) return;
    
    DynamicJsonDocument doc(1024);
    
    // Species information
    if (analysis.species.species != SpeciesType::UNKNOWN) {
        JsonObject species = doc.createNestedObject("species");
        species["type"] = speciesTypeToString(analysis.species.species);
        species["confidence"] = analysis.species.confidence;
        species["confidence_level"] = confidenceLevelToString(analysis.species.confidenceLevel);
        species["count"] = analysis.species.animalCount;
    }
    
    // Behavior information
    if (analysis.behavior.primaryBehavior != BehaviorType::UNKNOWN) {
        JsonObject behavior = doc.createNestedObject("behavior");
        behavior["primary"] = behaviorTypeToString(analysis.behavior.primaryBehavior);
        behavior["confidence"] = analysis.behavior.confidence;
        behavior["activity_level"] = analysis.behavior.activity_level;
        behavior["stress_level"] = analysis.behavior.stress_level;
    }
    
    // Analysis metadata
    doc["overall_confidence"] = analysis.overallConfidence;
    doc["motion_detected"] = analysis.motionDetected;
    doc["threat_detected"] = analysis.threatDetected;
    doc["human_detected"] = analysis.humanPresenceDetected;
    doc["processing_time"] = analysis.processingTime;
    doc["timestamp"] = analysis.timestamp;
    
    // Environmental data
    JsonObject environment = doc.createNestedObject("environment");
    PowerStats powerStats = powerManager.getPowerStats();
    environment["battery_voltage"] = powerStats.batteryVoltage;
    environment["solar_voltage"] = powerStats.solarVoltage;
    environment["is_charging"] = powerStats.isCharging;
    
    // Save to file
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (file) {
        serializeJson(doc, file);
        file.close();
        DEBUG_PRINTF("AI metadata saved: %s\n", filename.c_str());
    } else {
        DEBUG_PRINTF("Error: Failed to save AI metadata: %s\n", filename.c_str());
    }
}
#endif

/**
 * Initialize Phase 4 Production System
 */
bool SystemManager::initializeProductionSystem() {
    DEBUG_SYSTEM_INFO("Initializing Phase 4 Production System...");
    
    // Configure production deployment based on deployment scenario
    ProductionConfig config;
    config.deploymentId = "ESP32CAM_" + String(ESP.getEfuseMac(), HEX);
    config.siteId = "WILDLIFE_SITE_001"; // Could be configured via setup
    config.networkId = "WILDLIFE_NETWORK";
    config.scenario = SCENARIO_CONSERVATION; // Default to conservation mode
    
    // Enable all production features
    config.enableOTA = true;
    config.enableCloudSync = true;
    config.enableAdvancedAI = true;
    config.enableEnvironmentalAdaptation = true;
    config.enableSecurity = true;
    config.enableMultiSite = false; // Disable for single-node testing
    
    // Set production targets
    config.targetUptime = 99.9;
    config.maxResponseTime = 5000;
    config.maxDetectionLatency = 2000;
    config.minBatteryLife = 30.0;
    
    // Conservation-specific settings
    config.conservationMode = true;
    config.endangeredSpeciesAlert = true;
    config.poachingDetection = true;
    config.habitatMonitoring = true;
    
    // Initialize production system
    if (!initializeProductionSystem(config)) {
        DEBUG_SYSTEM_ERROR("Failed to initialize production system");
        return false;
    }
    
    // Start production operations
    if (!startWildlifeProduction()) {
        DEBUG_SYSTEM_ERROR("Failed to start production operations");
        return false;
    }
    
    lastProductionUpdate = millis();
    lastHealthCheck = millis();
    lastOTACheck = millis();
    
    DEBUG_SYSTEM_INFO("Phase 4 Production System initialized successfully");
    return true;
}

/**
 * Handle Phase 4 Production System Updates
 */
void SystemManager::handleProductionUpdate() {
    unsigned long currentTime = millis();
    
    // Check for OTA updates every hour
    if (currentTime - lastOTACheck > 3600000) { // 1 hour
        checkForOTAUpdates();
        lastOTACheck = currentTime;
    }
    
    // Perform health check every 10 minutes
    if (currentTime - lastHealthCheck > 600000) { // 10 minutes
        performSystemHealthCheck();
        lastHealthCheck = currentTime;
    }
    
    // Sync with cloud every 5 minutes (if enabled)
    if (currentTime - lastProductionUpdate > 300000) { // 5 minutes
        syncWithCloud();
        lastProductionUpdate = currentTime;
    }
    
    // Process any production events
    if (g_productionSystem && g_productionSystem->isOperational()) {
        // Production system handles its own updates
    }
}

/**
 * Perform System Health Check
 */
void SystemManager::performSystemHealthCheck() {
    if (!productionSystemInitialized || !g_productionSystem) {
        return;
    }
    
    SystemHealthMetrics health = getProductionHealth();
    
    // Log health status
    DEBUG_SYSTEM_INFO("System Health: %.1f%% (Status: %s)", 
                      health.overallHealth, 
                      health.status == PROD_OPERATIONAL ? "Operational" : "Warning");
    
    // Check for critical issues
    if (health.overallHealth < 80.0) {
        DEBUG_SYSTEM_WARN("System health below threshold: %.1f%%", health.overallHealth);
        
        // Log specific component issues
        if (!health.otaHealthy) DEBUG_SYSTEM_WARN("OTA system unhealthy");
        if (!health.configHealthy) DEBUG_SYSTEM_WARN("Configuration system unhealthy");
        if (!health.environmentalHealthy) DEBUG_SYSTEM_WARN("Environmental system unhealthy");
        if (!health.detectionHealthy) DEBUG_SYSTEM_WARN("Detection system unhealthy");
        if (!health.cloudHealthy) DEBUG_SYSTEM_WARN("Cloud system unhealthy");
        if (!health.securityHealthy) DEBUG_SYSTEM_WARN("Security system unhealthy");
    }
    
    lastHealthMetrics = health;
}

/**
 * Check for OTA Updates
 */
void SystemManager::checkForOTAUpdates() {
    if (!productionSystemInitialized) {
        return;
    }
    
    DEBUG_SYSTEM_INFO("Checking for OTA updates...");
    
    // Check if OTA updates are available
    if (checkAndUpdateFirmware()) {
        DEBUG_SYSTEM_INFO("OTA update initiated");
    } else {
        DEBUG_SYSTEM_DEBUG("No OTA updates available");
    }
}

/**
 * Sync with Cloud
 */
void SystemManager::syncWithCloud() {
    if (!productionSystemInitialized) {
        return;
    }
    
    // Check cloud connectivity
    if (isCloudConnected()) {
        // Sync detection data, system status, etc.
        if (syncAllData()) {
            DEBUG_SYSTEM_DEBUG("Cloud sync completed successfully");
        } else {
            DEBUG_SYSTEM_WARN("Cloud sync failed");
        }
    } else {
        DEBUG_SYSTEM_DEBUG("Cloud not connected - skipping sync");
    }
}

/**
 * Process Wildlife Detection with Production Pipeline
 */
bool SystemManager::processWildlifeDetectionProduction(const uint8_t* imageData, uint16_t width, uint16_t height) {
    if (!productionSystemInitialized || !g_productionSystem) {
        return false;
    }
    
    // Use production system's enhanced detection pipeline
    return g_productionSystem->processWildlifeDetection(imageData, width, height);
}

/**
 * Handle Production Events
 */
void SystemManager::handleProductionEvent(const ProductionEvent& event) {
    DEBUG_SYSTEM_INFO("Production Event [%s]: %s", 
                      event.component.c_str(), event.message.c_str());
    
    if (event.critical) {
        DEBUG_SYSTEM_ERROR("Critical production event: %s", event.message.c_str());
        
        // Handle critical events
        if (event.component == "Security") {
            // Handle security incidents
            DEBUG_SYSTEM_WARN("Security incident detected");
        } else if (event.component == "Conservation") {
            // Handle conservation alerts
            DEBUG_SYSTEM_INFO("Conservation alert: %s", event.details.c_str());
        }
    }
}
#endif
