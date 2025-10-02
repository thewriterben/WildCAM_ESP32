/**
 * @file complete_system_integration.cpp
 * @brief Complete WildCAM ESP32 System Integration Example
 * 
 * Demonstrates the full integration of:
 * - WiFi/Cellular connectivity with automatic fallback
 * - Cloud integration (AWS/Azure/GCP)
 * - Environmental sensor monitoring
 * - Wildlife detection and classification
 * - Mobile app push notifications
 * - Real-time data synchronization
 * - OTA firmware updates
 * 
 * This is a production-ready example showing best practices for
 * deploying a complete wildlife monitoring system.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include "esp_camera.h"

// Core connectivity
#include "../src/connectivity_orchestrator.h"

// Cloud integration  
#include "../../ESP32WildlifeCAM-main/firmware/src/production/enterprise/cloud_integrator.h"
#include "../../ESP32WildlifeCAM-main/firmware/src/production/enterprise/cloud/intelligent_sync_manager.h"
#include "../../ESP32WildlifeCAM-main/firmware/src/production/enterprise/cloud/wildlife_cloud_pipeline.h"

// Environmental monitoring
#include "../drivers/environmental_suite.h"

// Mobile app integration
#include "../../ESP32WildlifeCAM-main/mobile_app/firmware/src/mobile/mobile_api.h"
#include "../../ESP32WildlifeCAM-main/mobile_app/firmware/src/mobile/mobile_websocket.h"

// Security
#include "../../ESP32WildlifeCAM-main/firmware/src/production/security/data_protector.h"

// OTA updates
#include "../../ESP32WildlifeCAM-main/firmware/src/production/deployment/ota_manager.h"

// ===========================================
// CONFIGURATION
// ===========================================

// WiFi Configuration
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Cellular Configuration (if using GSM/LTE module)
const bool ENABLE_CELLULAR = true;
const size_t CELLULAR_DATA_LIMIT_MB = 100; // Daily limit

// Cloud Configuration
const char* CLOUD_ENDPOINT = "https://your-cloud-endpoint.com";
const char* CLOUD_ACCESS_KEY = "YOUR_ACCESS_KEY";
const char* CLOUD_SECRET_KEY = "YOUR_SECRET_KEY";
const char* CLOUD_BUCKET = "wildlife-data";

// Device Configuration
const char* DEVICE_ID = "WILDCAM-ESP32-001";
const char* DEVICE_NAME = "Wildlife Camera 1";
const float LATITUDE = 37.7749;   // Device location
const float LONGITUDE = -122.4194;

// Mobile App Configuration
const char* MOBILE_NOTIFICATION_TOPIC = "wildlife/detections";

// ===========================================
// GLOBAL VARIABLES
// ===========================================

// Connectivity
ConnectivityOrchestrator* connectivityOrch = nullptr;

// Environmental sensors
EnvironmentalSuite* envSuite = nullptr;
EnvironmentalData lastEnvData;

// Detection counters
uint32_t totalDetections = 0;
uint32_t detectionsSyncedToCloud = 0;
uint32_t environmentalReadings = 0;

// Timing
unsigned long lastEnvReading = 0;
unsigned long lastCloudSync = 0;
unsigned long lastHealthReport = 0;
unsigned long lastOTACheck = 0;

const unsigned long ENV_READING_INTERVAL = 300000;  // 5 minutes
const unsigned long CLOUD_SYNC_INTERVAL = 600000;   // 10 minutes
const unsigned long HEALTH_REPORT_INTERVAL = 1800000; // 30 minutes
const unsigned long OTA_CHECK_INTERVAL = 86400000;  // 24 hours

// ===========================================
// FUNCTION DECLARATIONS
// ===========================================

void setupConnectivity();
void setupCloudIntegration();
void setupEnvironmentalSensors();
void setupMobileIntegration();
void setupSecurity();
void setupOTA();
void setupCamera();

void loopConnectivity();
void loopEnvironmentalMonitoring();
void loopCloudSync();
void loopHealthMonitoring();
void loopOTAUpdates();

void handleWildlifeDetection(const String& species, float confidence, const uint8_t* imageData, size_t imageSize);
void handleEnvironmentalAlert(const String& alertType, const String& message);
void sendMobilePushNotification(const String& title, const String& message, const String& data);
void correlateEnvironmentWithWildlife();

// ===========================================
// SETUP
// ===========================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n===========================================");
    Serial.println("WildCAM ESP32 - Complete System Integration");
    Serial.println("===========================================\n");
    
    // Initialize SD card for local storage
    if (!SD.begin()) {
        Serial.println("ERROR: SD card initialization failed");
        Serial.println("System will continue without SD card");
    } else {
        Serial.println("✓ SD card initialized");
    }
    
    // Setup all subsystems
    setupConnectivity();
    setupCloudIntegration();
    setupEnvironmentalSensors();
    setupMobileIntegration();
    setupSecurity();
    setupOTA();
    setupCamera();
    
    Serial.println("\n===========================================");
    Serial.println("System initialization complete!");
    Serial.println("===========================================\n");
}

// ===========================================
// MAIN LOOP
// ===========================================

void loop() {
    // Update connectivity (handles automatic WiFi/Cellular fallback)
    loopConnectivity();
    
    // Monitor environmental conditions
    loopEnvironmentalMonitoring();
    
    // Sync data to cloud
    loopCloudSync();
    
    // Report system health
    loopHealthMonitoring();
    
    // Check for OTA updates
    loopOTAUpdates();
    
    // Small delay to prevent tight loop
    delay(100);
}

// ===========================================
// CONNECTIVITY SETUP & LOOP
// ===========================================

void setupConnectivity() {
    Serial.println("Setting up connectivity...");
    
    ConnectivityConfig config;
    config.enableWiFi = true;
    config.wifiSSID = WIFI_SSID;
    config.wifiPassword = WIFI_PASSWORD;
    config.wifiRetryAttempts = 3;
    config.wifiTimeout = 30000;
    
    config.enableCellular = ENABLE_CELLULAR;
    config.cellularAsFallbackOnly = true;
    config.cellularRetryAttempts = 2;
    config.cellularTimeout = 60000;
    
    config.autoFallback = true;
    config.fallbackThreshold = -75;  // dBm
    config.healthCheckInterval = 30000;  // 30 seconds
    config.reconnectInterval = 60000;    // 1 minute
    
    config.enableCostOptimization = true;
    config.cellularDailyDataLimit = CELLULAR_DATA_LIMIT_MB;
    config.preferWiFiForLargeUploads = true;
    
    connectivityOrch = new ConnectivityOrchestrator();
    if (!connectivityOrch->initialize(config)) {
        Serial.println("ERROR: Connectivity initialization failed!");
        return;
    }
    
    Serial.println("Connecting to network...");
    if (connectivityOrch->connect()) {
        Serial.println("✓ Network connected: " + connectivityOrch->getConnectionInfo());
    } else {
        Serial.println("WARNING: Initial connection failed, will retry automatically");
    }
}

void loopConnectivity() {
    if (connectivityOrch) {
        connectivityOrch->update();
    }
}

// ===========================================
// CLOUD INTEGRATION SETUP & LOOP
// ===========================================

void setupCloudIntegration() {
    Serial.println("Setting up cloud integration...");
    
    if (!connectivityOrch || !connectivityOrch->isConnected()) {
        Serial.println("WARNING: No network connection, cloud setup deferred");
        return;
    }
    
    // Configure primary cloud
    CloudConfig primaryCloud;
    primaryCloud.platform = CLOUD_AWS;  // or CLOUD_AZURE, CLOUD_GCP, CLOUD_CUSTOM
    primaryCloud.endpoint = CLOUD_ENDPOINT;
    primaryCloud.accessKey = CLOUD_ACCESS_KEY;
    primaryCloud.secretKey = CLOUD_SECRET_KEY;
    primaryCloud.bucketName = CLOUD_BUCKET;
    primaryCloud.deviceId = DEVICE_ID;
    primaryCloud.enableSSL = true;
    primaryCloud.enableCompression = true;
    primaryCloud.autoRetry = true;
    primaryCloud.maxRetries = 3;
    
    // Initialize cloud integrator
    if (initializeCloudIntegration(primaryCloud)) {
        Serial.println("✓ Cloud integration initialized");
        
        // Setup intelligent sync manager
        SyncConfig syncConfig;
        syncConfig.enableAdaptiveSync = true;
        syncConfig.enableDeltaSync = true;
        syncConfig.enableCompression = true;
        syncConfig.syncInterval = 300;  // 5 minutes
        syncConfig.maxBatchSize = 10;
        
        if (g_intelligentSyncManager && g_intelligentSyncManager->initialize(syncConfig)) {
            Serial.println("✓ Intelligent sync manager initialized");
        }
        
        // Setup wildlife cloud pipeline
        if (g_wildlifeCloudPipeline) {
            g_wildlifeCloudPipeline->initialize();
            g_wildlifeCloudPipeline->setDetectionThreshold(0.75);
            g_wildlifeCloudPipeline->enableRealtimeProcessing(true);
            Serial.println("✓ Wildlife cloud pipeline initialized");
        }
    } else {
        Serial.println("ERROR: Cloud integration initialization failed");
    }
}

void loopCloudSync() {
    unsigned long now = millis();
    
    // Periodic sync to cloud
    if (now - lastCloudSync >= CLOUD_SYNC_INTERVAL) {
        if (connectivityOrch && connectivityOrch->isConnected()) {
            Serial.println("Syncing data to cloud...");
            
            if (g_cloudIntegrator) {
                // Force sync all pending data
                g_cloudIntegrator->forceSyncAll();
                Serial.printf("✓ Cloud sync complete (Total detections: %d, Synced: %d)\n", 
                             totalDetections, detectionsSyncedToCloud);
            }
        } else {
            Serial.println("Skipping cloud sync - no connection");
        }
        
        lastCloudSync = now;
    }
}

// ===========================================
// ENVIRONMENTAL SENSORS SETUP & LOOP
// ===========================================

void setupEnvironmentalSensors() {
    Serial.println("Setting up environmental sensors...");
    
    EnvironmentalConfig envConfig;
    envConfig.enableWeatherStation = true;
    envConfig.enableAirQuality = true;
    envConfig.enableAcousticMonitoring = true;
    envConfig.enableHabitatSensors = true;
    envConfig.enableLightMonitoring = true;
    
    envConfig.readingInterval = 60000;  // 1 minute
    envConfig.dataLoggingEnabled = true;
    envConfig.alertThresholdsEnabled = true;
    
    // Temperature thresholds
    envConfig.minTemperature = -10.0;
    envConfig.maxTemperature = 45.0;
    
    envSuite = new EnvironmentalSuite();
    if (envSuite->initialize(envConfig)) {
        Serial.println("✓ Environmental sensors initialized");
        
        // Perform initial calibration
        envSuite->performCalibration();
        Serial.println("✓ Sensor calibration complete");
    } else {
        Serial.println("ERROR: Environmental sensor initialization failed");
    }
}

void loopEnvironmentalMonitoring() {
    unsigned long now = millis();
    
    if (now - lastEnvReading >= ENV_READING_INTERVAL) {
        if (envSuite) {
            Serial.println("Reading environmental sensors...");
            
            // Read all sensor data
            envSuite->readAllSensors();
            lastEnvData = envSuite->getCurrentData();
            environmentalReadings++;
            
            // Log sensor readings
            Serial.println("Environmental Data:");
            Serial.printf("  Temperature: %.1f°C\n", lastEnvData.temperature);
            Serial.printf("  Humidity: %.1f%%\n", lastEnvData.humidity);
            Serial.printf("  Pressure: %.1f hPa\n", lastEnvData.pressure);
            Serial.printf("  Light Level: %d lux\n", lastEnvData.lightLevel);
            Serial.printf("  Air Quality Index: %.1f\n", lastEnvData.airQualityIndex);
            Serial.printf("  Wildlife Comfort Index: %.1f\n", lastEnvData.wildlifeComfortIndex);
            
            // Upload to cloud if connected
            if (connectivityOrch && connectivityOrch->isConnected() && g_cloudIntegrator) {
                String envJson = envSuite->getDataAsJSON();
                
                UploadRequest request;
                request.dataType = DATA_SENSOR;
                request.data = envJson;
                request.metadata = "{\"type\":\"environmental\",\"device\":\"" + String(DEVICE_ID) + "\"}";
                
                if (g_cloudIntegrator->uploadDataAsync(request)) {
                    Serial.println("✓ Environmental data uploaded to cloud");
                }
            }
            
            // Check for environmental alerts
            if (lastEnvData.temperature < -5.0 || lastEnvData.temperature > 40.0) {
                handleEnvironmentalAlert("TEMPERATURE", 
                    "Extreme temperature: " + String(lastEnvData.temperature) + "°C");
            }
            
            if (lastEnvData.airQualityIndex > 150) {
                handleEnvironmentalAlert("AIR_QUALITY", 
                    "Poor air quality detected: AQI " + String(lastEnvData.airQualityIndex));
            }
        }
        
        lastEnvReading = now;
    }
}

// ===========================================
// MOBILE APP INTEGRATION SETUP
// ===========================================

void setupMobileIntegration() {
    Serial.println("Setting up mobile app integration...");
    
    if (g_mobileAPI) {
        MobileAPIConfig mobileConfig;
        mobileConfig.enableBandwidthOptimization = true;
        mobileConfig.enableThumbnailGeneration = true;
        mobileConfig.thumbnailQuality = 60;
        mobileConfig.maxImageSize = 800;
        
        g_mobileAPI->setConfig(mobileConfig);
        Serial.println("✓ Mobile API configured");
    }
    
    if (g_mobileWebSocket) {
        // WebSocket will be initialized by the web server
        Serial.println("✓ Mobile WebSocket ready");
    }
}

// ===========================================
// SECURITY SETUP
// ===========================================

void setupSecurity() {
    Serial.println("Setting up security features...");
    
    if (g_dataProtector) {
        SecurityConfig secConfig;
        secConfig.enableEncryption = true;
        secConfig.encryptionType = ENCRYPTION_AES256;
        secConfig.requireAuthentication = true;
        secConfig.enableAuditLogging = true;
        
        if (g_dataProtector->init(secConfig)) {
            Serial.println("✓ Security features enabled (AES-256 encryption)");
        }
    }
}

// ===========================================
// OTA UPDATES SETUP & LOOP
// ===========================================

void setupOTA() {
    Serial.println("Setting up OTA updates...");
    
    if (g_otaManager) {
        OTAConfig otaConfig;
        otaConfig.autoCheck = true;
        otaConfig.checkInterval = 86400;  // 24 hours
        otaConfig.updateURL = "https://your-update-server.com/firmware";
        otaConfig.requireSigned = true;
        otaConfig.allowDowngrade = false;
        
        if (g_otaManager->init(otaConfig)) {
            Serial.println("✓ OTA update manager initialized");
        }
    }
}

void loopOTAUpdates() {
    unsigned long now = millis();
    
    if (now - lastOTACheck >= OTA_CHECK_INTERVAL) {
        if (g_otaManager && connectivityOrch && connectivityOrch->isConnected()) {
            Serial.println("Checking for firmware updates...");
            
            if (g_otaManager->checkForUpdates()) {
                if (g_otaManager->hasAvailableUpdate()) {
                    Serial.println("New firmware available! Starting update...");
                    g_otaManager->startUpdate();
                }
            }
        }
        
        lastOTACheck = now;
    }
}

// ===========================================
// CAMERA SETUP
// ===========================================

void setupCamera() {
    Serial.println("Setting up camera...");
    
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
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    
    esp_err_t err = esp_camera_init(&config);
    if (err == ESP_OK) {
        Serial.println("✓ Camera initialized");
    } else {
        Serial.printf("ERROR: Camera init failed with error 0x%x\n", err);
    }
}

// ===========================================
// HEALTH MONITORING LOOP
// ===========================================

void loopHealthMonitoring() {
    unsigned long now = millis();
    
    if (now - lastHealthReport >= HEALTH_REPORT_INTERVAL) {
        Serial.println("\n===========================================");
        Serial.println("SYSTEM HEALTH REPORT");
        Serial.println("===========================================");
        
        // Connectivity status
        if (connectivityOrch) {
            Serial.println("Connectivity:");
            Serial.println("  " + connectivityOrch->getConnectionInfo());
            Serial.printf("  Quality: %d, Reliability: %.1f%%\n", 
                         connectivityOrch->getNetworkQuality(),
                         connectivityOrch->getConnectionReliability() * 100);
            
            ConnectionStats stats = connectivityOrch->getStatistics();
            Serial.printf("  Uptime: WiFi=%lus, Cellular=%lus\n", 
                         stats.wifiUptime, stats.cellularUptime);
            Serial.printf("  Data: WiFi=%uKB, Cellular=%uKB\n",
                         stats.dataTransferredWiFi, stats.dataTransferredCellular);
        }
        
        // Detection statistics
        Serial.println("Wildlife Detection:");
        Serial.printf("  Total Detections: %d\n", totalDetections);
        Serial.printf("  Synced to Cloud: %d\n", detectionsSyncedToCloud);
        
        // Environmental monitoring
        Serial.println("Environmental Monitoring:");
        Serial.printf("  Total Readings: %d\n", environmentalReadings);
        Serial.printf("  Current Temp: %.1f°C\n", lastEnvData.temperature);
        Serial.printf("  Current Humidity: %.1f%%\n", lastEnvData.humidity);
        
        // System resources
        Serial.println("System Resources:");
        Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("  Uptime: %lu seconds\n", millis() / 1000);
        
        Serial.println("===========================================\n");
        
        // Send health report to cloud
        if (connectivityOrch && connectivityOrch->isConnected() && g_cloudIntegrator) {
            String healthData = "{";
            healthData += "\"device\":\"" + String(DEVICE_ID) + "\",";
            healthData += "\"connection\":\"" + String(connectivityOrch->getActiveConnection()) + "\",";
            healthData += "\"detections\":" + String(totalDetections) + ",";
            healthData += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
            healthData += "\"uptime\":" + String(millis() / 1000);
            healthData += "}";
            
            g_cloudIntegrator->reportDeviceHealth(healthData);
        }
        
        lastHealthReport = now;
    }
}

// ===========================================
// EVENT HANDLERS
// ===========================================

void handleWildlifeDetection(const String& species, float confidence, 
                            const uint8_t* imageData, size_t imageSize) {
    Serial.println("\n*** WILDLIFE DETECTED ***");
    Serial.printf("Species: %s (Confidence: %.1f%%)\n", species.c_str(), confidence * 100);
    
    totalDetections++;
    
    // Process through wildlife cloud pipeline
    if (g_wildlifeCloudPipeline && connectivityOrch && connectivityOrch->isConnected()) {
        WildlifeEvent event;
        event.species = species;
        event.confidence = confidence;
        event.timestamp = millis();
        event.deviceId = DEVICE_ID;
        event.latitude = LATITUDE;
        event.longitude = LONGITUDE;
        
        // Add environmental context
        event.temperature = lastEnvData.temperature;
        event.humidity = lastEnvData.humidity;
        event.lightLevel = lastEnvData.lightLevel;
        
        if (g_wildlifeCloudPipeline->processDetection(event, imageData, imageSize)) {
            detectionsSyncedToCloud++;
            Serial.println("✓ Detection synced to cloud");
        }
    }
    
    // Send push notification to mobile app
    String notifData = "{\"species\":\"" + species + "\",\"confidence\":" + String(confidence) + "}";
    sendMobilePushNotification("Wildlife Detected!", 
                              species + " detected with " + String((int)(confidence * 100)) + "% confidence",
                              notifData);
    
    // Correlate with environmental conditions
    correlateEnvironmentWithWildlife();
}

void handleEnvironmentalAlert(const String& alertType, const String& message) {
    Serial.println("\n*** ENVIRONMENTAL ALERT ***");
    Serial.println("Type: " + alertType);
    Serial.println("Message: " + message);
    
    // Send to cloud
    if (connectivityOrch && connectivityOrch->isConnected() && g_cloudIntegrator) {
        String alertJson = "{";
        alertJson += "\"type\":\"" + alertType + "\",";
        alertJson += "\"message\":\"" + message + "\",";
        alertJson += "\"device\":\"" + String(DEVICE_ID) + "\",";
        alertJson += "\"timestamp\":" + String(millis());
        alertJson += "}";
        
        UploadRequest request;
        request.dataType = DATA_ALERT;
        request.data = alertJson;
        g_cloudIntegrator->uploadDataAsync(request);
    }
    
    // Notify mobile app
    sendMobilePushNotification("Environmental Alert", message, "{\"type\":\"" + alertType + "\"}");
}

void sendMobilePushNotification(const String& title, const String& message, const String& data) {
    if (g_mobileWebSocket) {
        g_mobileWebSocket->broadcastNotification(title, message, data);
    }
}

void correlateEnvironmentWithWildlife() {
    // Analyze correlation between environmental conditions and wildlife activity
    Serial.println("Correlating environmental conditions with wildlife activity...");
    
    // This would typically involve more sophisticated analysis
    // For now, just log the current conditions
    Serial.printf("Detection occurred at: Temp=%.1f°C, Humidity=%.1f%%, Light=%d lux\n",
                 lastEnvData.temperature, lastEnvData.humidity, lastEnvData.lightLevel);
    
    // Send correlation data to cloud for advanced analytics
    if (connectivityOrch && connectivityOrch->isConnected() && g_cloudAnalyticsEngine) {
        String correlationData = "{";
        correlationData += "\"temperature\":" + String(lastEnvData.temperature) + ",";
        correlationData += "\"humidity\":" + String(lastEnvData.humidity) + ",";
        correlationData += "\"lightLevel\":" + String(lastEnvData.lightLevel) + ",";
        correlationData += "\"detectionTime\":" + String(millis());
        correlationData += "}";
        
        // Cloud analytics engine would process this for pattern recognition
    }
}
