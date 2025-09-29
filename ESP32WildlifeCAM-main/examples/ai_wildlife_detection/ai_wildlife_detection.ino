/**
 * @file ai_wildlife_detection.ino
 * @brief Complete AI Wildlife Detection Example
 * 
 * Demonstrates the Phase 1 Core AI Enhancement Integration including:
 * - TensorFlow Lite Micro wildlife classification
 * - XPowersLib advanced power management
 * - OTA updates via AsyncElegantOTA
 * 
 * This example shows how to use all three Phase 1 enhancements together
 * in a complete wildlife monitoring system.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <SD_MMC.h>

// Phase 1 AI Enhancement Integration
#include "ai/tensorflow_lite_implementation.h"
#include "power/xpowers_manager.h"
#include "network/ota_manager.h"

// Configuration
const char* WIFI_SSID = "WildlifeCAM_Network";
const char* WIFI_PASSWORD = "wildlife123";
const char* OTA_USERNAME = "admin";
const char* OTA_PASSWORD = "wildlife";

// Global components
TensorFlowLiteImplementation* aiSystem = nullptr;
XPowersManager* powerManager = nullptr;
NetworkOTAManager* otaManager = nullptr;

// Timing
unsigned long lastAIAnalysis = 0;
unsigned long lastPowerCheck = 0;
unsigned long lastStatusReport = 0;

const unsigned long AI_ANALYSIS_INTERVAL = 10000;    // 10 seconds
const unsigned long POWER_CHECK_INTERVAL = 5000;     // 5 seconds
const unsigned long STATUS_REPORT_INTERVAL = 30000;  // 30 seconds

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=================================");
    Serial.println("ESP32 Wildlife Camera - Phase 1");
    Serial.println("AI Enhancement Integration Demo");
    Serial.println("=================================");
    
    // Initialize camera
    if (!initializeCamera()) {
        Serial.println("ERROR: Camera initialization failed");
        return;
    }
    
    // Initialize SD card
    if (!initializeSDCard()) {
        Serial.println("WARNING: SD card initialization failed");
    }
    
    // Initialize WiFi
    if (!initializeWiFi()) {
        Serial.println("WARNING: WiFi initialization failed");
    }
    
    // Initialize Phase 1 AI Enhancement - TensorFlow Lite Micro
    if (!initializeTensorFlowLite()) {
        Serial.println("WARNING: TensorFlow Lite initialization failed");
    } else {
        aiSystem = g_tensorflowImplementation;
        
        // Load wildlife models
        if (loadWildlifeModels("/models")) {
            Serial.println("Wildlife AI models loaded successfully");
        } else {
            Serial.println("WARNING: Failed to load some AI models");
        }
    }
    
    // Initialize Phase 1 Power Enhancement - XPowersLib
    if (!initializeXPowers()) {
        Serial.println("WARNING: XPowersLib initialization failed");
    } else {
        powerManager = g_xpowersManager;
        Serial.println("Advanced power management initialized");
        
        // Configure charging for wildlife camera deployment
        ChargingConfig chargingConfig;
        chargingConfig.mode = CHARGE_AUTO;
        chargingConfig.currentLimit = 500;  // 500mA for gentle charging
        chargingConfig.voltageLimit = 4.2f;
        chargingConfig.enabled = true;
        
        powerManager->configureCharging(chargingConfig);
        powerManager->enableAutoOptimization(true);
    }
    
    // Initialize Phase 1 OTA Enhancement - AsyncElegantOTA
    if (WiFi.status() == WL_CONNECTED) {
        if (!initializeNetworkOTA()) {
            Serial.println("WARNING: Network OTA initialization failed");
        } else {
            otaManager = g_networkOTAManager;
            
            if (startOTAServer(80, OTA_USERNAME, OTA_PASSWORD)) {
                Serial.println("OTA server started successfully");
                Serial.print("OTA Update URL: ");
                Serial.println(getOTAUpdateURL());
            }
        }
    }
    
    Serial.println("\nSystem initialization complete!");
    Serial.println("Wildlife camera ready for AI-enhanced monitoring.");
    
    printSystemStatus();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check for motion detection or scheduled AI analysis
    if (currentTime - lastAIAnalysis >= AI_ANALYSIS_INTERVAL) {
        performAIWildlifeAnalysis();
        lastAIAnalysis = currentTime;
    }
    
    // Update power management and optimization
    if (currentTime - lastPowerCheck >= POWER_CHECK_INTERVAL) {
        updatePowerManagement();
        lastPowerCheck = currentTime;
    }
    
    // Print system status periodically
    if (currentTime - lastStatusReport >= STATUS_REPORT_INTERVAL) {
        printSystemStatus();
        lastStatusReport = currentTime;
    }
    
    // Check for critical conditions
    checkCriticalConditions();
    
    delay(100);
}

bool initializeCamera() {
    Serial.println("Initializing camera...");
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200 for high quality
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    Serial.println("Camera initialized successfully");
    return true;
}

bool initializeSDCard() {
    Serial.println("Initializing SD card...");
    
    if (!SD_MMC.begin()) {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }
    
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    
    // Create directories
    if (!SD_MMC.exists("/images")) {
        SD_MMC.mkdir("/images");
    }
    if (!SD_MMC.exists("/models")) {
        SD_MMC.mkdir("/models");
    }
    
    return true;
}

bool initializeWiFi() {
    Serial.println("Connecting to WiFi...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed");
        return false;
    }
}

void performAIWildlifeAnalysis() {
    Serial.println("\n--- AI Wildlife Analysis ---");
    
    if (!aiSystem || !aiSystem->isInitialized()) {
        Serial.println("AI system not available");
        return;
    }
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    Serial.printf("Image captured: %dx%d, %d bytes\n", fb->width, fb->height, fb->len);
    
    // Convert to CameraFrame for AI processing
    CameraFrame frame;
    frame.data = fb->buf;
    frame.width = fb->width;
    frame.height = fb->height;
    frame.format = PIXFORMAT_JPEG;
    frame.timestamp = millis();
    
    // Run AI analysis
    if (aiSystem->isModelLoaded(MODEL_SPECIES_CLASSIFIER)) {
        InferenceResult speciesResult = aiSystem->detectSpecies(frame);
        if (speciesResult.valid && speciesResult.confidence > 0.5f) {
            Serial.printf("Species detected: %s (%.1f%% confidence)\n", 
                         speciesResult.className.c_str(), speciesResult.confidence * 100);
            
            // Save high-confidence detections
            saveDetection(fb, speciesResult);
        } else {
            Serial.println("No species detected with sufficient confidence");
        }
    }
    
    if (aiSystem->isModelLoaded(MODEL_HUMAN_DETECTOR)) {
        InferenceResult humanResult = aiSystem->detectHuman(frame);
        if (humanResult.valid && humanResult.confidence > 0.7f) {
            Serial.printf("Human detected: %.1f%% confidence\n", humanResult.confidence * 100);
        }
    }
    
    if (aiSystem->isModelLoaded(MODEL_BEHAVIOR_ANALYZER)) {
        InferenceResult behaviorResult = aiSystem->analyzeBehavior(frame);
        if (behaviorResult.valid && behaviorResult.confidence > 0.6f) {
            Serial.printf("Behavior: %s (%.1f%% confidence)\n", 
                         behaviorResult.className.c_str(), behaviorResult.confidence * 100);
        }
    }
    
    esp_camera_fb_return(fb);
}

void updatePowerManagement() {
    if (!powerManager || !powerManager->isInitialized()) {
        return;
    }
    
    // Update power measurements
    powerManager->updateMeasurements();
    XPowerMetrics metrics = powerManager->getPowerMetrics();
    
    // Optimize power based on conditions
    if (powerManager->isAutoOptimizationEnabled()) {
        powerManager->optimizeCharging();
        powerManager->optimizeSolarCharging();
    }
    
    // Check for low battery conditions
    if (powerManager->isBatteryLow()) {
        Serial.println("WARNING: Low battery detected");
        // Reduce AI analysis frequency
        // Could implement power-saving measures here
    }
    
    // Monitor solar charging
    if (powerManager->isSolarConnected() && powerManager->getSolarPower() > 100) {
        // Good solar conditions - can increase AI processing
        Serial.printf("Solar charging: %.1fW\n", powerManager->getSolarPower() / 1000.0f);
    }
}

void printSystemStatus() {
    Serial.println("\n=== System Status ===");
    
    // Basic system info
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi: Connected (%s, RSSI: %d dBm)\n", 
                     WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        Serial.println("WiFi: Disconnected");
    }
    
    // AI system status
    if (aiSystem && aiSystem->isInitialized()) {
        Serial.println("AI System: Active");
        Serial.printf("  Species classifier: %s\n", 
                     aiSystem->isModelLoaded(MODEL_SPECIES_CLASSIFIER) ? "Loaded" : "Not loaded");
        Serial.printf("  Human detector: %s\n", 
                     aiSystem->isModelLoaded(MODEL_HUMAN_DETECTOR) ? "Loaded" : "Not loaded");
        Serial.printf("  Behavior analyzer: %s\n", 
                     aiSystem->isModelLoaded(MODEL_BEHAVIOR_ANALYZER) ? "Loaded" : "Not loaded");
    } else {
        Serial.println("AI System: Inactive");
    }
    
    // Power system status
    if (powerManager && powerManager->isInitialized()) {
        XPowerMetrics metrics = powerManager->getPowerMetrics();
        Serial.println("Power System: Active");
        Serial.printf("  Battery: %.2fV (%d%%) %s\n", 
                     metrics.batteryVoltage, metrics.batteryPercentage,
                     metrics.isCharging ? "(Charging)" : "(Discharging)");
        Serial.printf("  Solar: %.2fV (%.1fmA)\n", 
                     metrics.solarVoltage, metrics.solarCurrent);
        Serial.printf("  System: %.1fmA\n", metrics.systemCurrent);
    } else {
        Serial.println("Power System: Basic mode");
    }
    
    // OTA status
    if (otaManager && otaManager->isInitialized()) {
        Serial.printf("OTA System: %s\n", otaManager->getStatusString().c_str());
        if (otaManager->isRunning()) {
            Serial.printf("  Update URL: %s\n", otaManager->getUpdateURL().c_str());
        }
    } else {
        Serial.println("OTA System: Inactive");
    }
    
    Serial.println("====================");
}

void saveDetection(camera_fb_t* fb, const InferenceResult& result) {
    if (!SD_MMC.exists("/images")) {
        return;
    }
    
    // Generate filename with timestamp and species
    String filename = "/images/wildlife_" + String(millis()) + "_" + 
                     result.className + "_" + String((int)(result.confidence * 100)) + ".jpg";
    
    // Save image
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        Serial.printf("Detection saved: %s\n", filename.c_str());
    }
}

void checkCriticalConditions() {
    // Check for critical battery level
    if (powerManager && powerManager->isBatteryCritical()) {
        Serial.println("CRITICAL: Battery level critical - entering emergency mode");
        // Implement emergency power saving
        esp_deep_sleep(60 * 1000000); // Sleep for 1 minute
    }
    
    // Check for overtemperature
    if (powerManager && powerManager->isOvertemperature()) {
        Serial.println("WARNING: High temperature detected");
        // Could reduce AI processing frequency
    }
}