/**
 * Complete Cloud Integration Example
 * 
 * Demonstrates full cloud platform integration with:
 * - Automated data backup
 * - Remote device management
 * - Advanced analytics
 * - Real-time wildlife detection processing
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include "esp_camera.h"
#include "production/enterprise/cloud_integrator.h"
#include "production/enterprise/cloud/cloud_service_orchestrator.h"
#include "production/enterprise/cloud/enhanced_cloud_manager.h"
#include "production/enterprise/cloud/intelligent_sync_manager.h"
#include "production/enterprise/cloud/cloud_analytics_engine.h"
#include "production/enterprise/cloud/wildlife_cloud_pipeline.h"
#include "production/enterprise/cloud/research_collaboration_platform.h"

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Cloud configuration
CloudConfig primaryCloud;
CloudConfig backupCloud;

// Detection counters
uint32_t detectionCount = 0;
uint32_t lastSyncTime = 0;
uint32_t syncInterval = 300000; // 5 minutes

// Function declarations
void setupWiFi();
void setupCamera();
void setupCloudPlatform();
void setupAdvancedServices();
void captureAndProcessImage();
void syncDataToCloud();
void handleWildlifeDetection(const String& species, float confidence);
void monitorSystemHealth();
void handleRemoteCommands();

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nWildCAM ESP32 - Cloud Integration Example");
    Serial.println("==========================================");
    
    // Initialize WiFi
    setupWiFi();
    
    // Initialize camera
    setupCamera();
    
    // Initialize SD card
    if (!SD.begin()) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card initialized");
    
    // Setup cloud platform
    setupCloudPlatform();
    
    // Setup advanced cloud services
    setupAdvancedServices();
    
    Serial.println("\nSetup complete! Starting wildlife monitoring...\n");
}

void loop() {
    // Main wildlife monitoring loop
    
    // 1. Capture and process images
    captureAndProcessImage();
    
    // 2. Check if it's time to sync data
    if (millis() - lastSyncTime > syncInterval) {
        syncDataToCloud();
        lastSyncTime = millis();
    }
    
    // 3. Handle remote commands
    handleRemoteCommands();
    
    // 4. Monitor system health
    if (millis() % 60000 == 0) { // Every minute
        monitorSystemHealth();
    }
    
    // 5. Process sync queue in background
    if (g_intelligentSyncManager->shouldSyncNow()) {
        g_intelligentSyncManager->syncNow();
    }
    
    delay(1000); // Main loop delay
}

void setupWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

void setupCamera() {
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
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }
    Serial.println("Camera initialized");
}

void setupCloudPlatform() {
    Serial.println("\n=== Cloud Platform Setup ===");
    
    // Configure primary cloud (AWS)
    primaryCloud.platform = CLOUD_AWS;
    primaryCloud.region = "us-west-2";
    primaryCloud.endpoint = "https://s3.us-west-2.amazonaws.com";
    primaryCloud.accessKey = "YOUR_AWS_ACCESS_KEY";
    primaryCloud.secretKey = "YOUR_AWS_SECRET_KEY";
    primaryCloud.bucketName = "wildlife-camera-data";
    primaryCloud.useSSL = true;
    primaryCloud.compressData = true;
    primaryCloud.encryptData = true;
    primaryCloud.syncMode = SYNC_OFFLINE_FIRST;
    primaryCloud.syncInterval = 300;
    
    // Configure backup cloud (Azure)
    backupCloud.platform = CLOUD_AZURE;
    backupCloud.region = "westus2";
    backupCloud.endpoint = "https://wildlifestorage.blob.core.windows.net";
    backupCloud.accessKey = "YOUR_AZURE_ACCOUNT";
    backupCloud.secretKey = "YOUR_AZURE_KEY";
    backupCloud.bucketName = "camera-backup";
    backupCloud.useSSL = true;
    backupCloud.compressData = true;
    backupCloud.syncMode = SYNC_BACKUP_ONLY;
    
    // Initialize cloud orchestrator
    OrchestratorConfig orchestratorConfig;
    orchestratorConfig.enableAutoRecovery = true;
    orchestratorConfig.enableResourceOptimization = true;
    orchestratorConfig.enableHealthMonitoring = true;
    
    if (initializeCloudOrchestrator(orchestratorConfig)) {
        Serial.println("✓ Cloud orchestrator initialized");
    } else {
        Serial.println("✗ Cloud orchestrator initialization failed");
        return;
    }
    
    // Initialize enhanced cloud manager with multi-cloud support
    EnhancedCloudConfig cloudConfig;
    cloudConfig.enableAutoRecovery = true;
    cloudConfig.enableResourceOptimization = true;
    cloudConfig.enableBandwidthOptimization = true;
    cloudConfig.maxMonthlyCost = 50.0; // $50 budget
    cloudConfig.providers.push_back(primaryCloud);
    cloudConfig.providers.push_back(backupCloud);
    
    if (initializeEnhancedCloudManager(cloudConfig)) {
        Serial.println("✓ Enhanced cloud manager initialized");
    } else {
        Serial.println("✗ Enhanced cloud manager initialization failed");
        return;
    }
    
    // Start all cloud services
    if (startCloudServices()) {
        Serial.println("✓ All cloud services started");
    } else {
        Serial.println("✗ Failed to start cloud services");
        return;
    }
    
    // Register device
    if (g_cloudIntegrator->registerDevice()) {
        Serial.println("✓ Device registered with cloud");
        Serial.print("Device ID: ");
        Serial.println(WiFi.macAddress());
    }
}

void setupAdvancedServices() {
    Serial.println("\n=== Advanced Services Setup ===");
    
    // Initialize intelligent sync manager
    SyncConfig syncConfig;
    syncConfig.enableAdaptiveSync = true;
    syncConfig.enableDeltaSync = true;
    syncConfig.enableCompression = true;
    syncConfig.enableEncryption = true;
    syncConfig.maxBandwidthUsage = 70; // Use max 70% of bandwidth
    syncConfig.qualityThreshold = 50; // Minimum connection quality
    syncConfig.enableOfflineQueue = true;
    syncConfig.maxQueueSize = 100;
    
    if (g_intelligentSyncManager->initialize(syncConfig)) {
        Serial.println("✓ Intelligent sync manager initialized");
        
        // Set bandwidth strategy
        g_intelligentSyncManager->setBandwidthStrategy(BANDWIDTH_ADAPTIVE);
        
        // Measure initial connection quality
        ConnectionMetrics metrics = g_intelligentSyncManager->measureConnectionQuality();
        Serial.printf("  Bandwidth: %.2f KB/s\n", metrics.bandwidth / 1024.0);
        Serial.printf("  Latency: %d ms\n", metrics.latency);
        Serial.printf("  Stability: %.0f%%\n", metrics.stability * 100);
    }
    
    // Initialize cloud analytics engine
    if (g_cloudAnalyticsEngine->initialize()) {
        Serial.println("✓ Cloud analytics engine initialized");
        g_cloudAnalyticsEngine->enableRealtimeAnalytics(true);
        
        // Load ML models
        g_cloudAnalyticsEngine->loadModel(MODEL_YOLO_DETECTION, "/models/yolo-tiny.tflite");
        g_cloudAnalyticsEngine->loadModel(MODEL_CLASSIFICATION, "/models/species-classifier.tflite");
    }
    
    // Initialize wildlife cloud pipeline
    if (g_wildlifeCloudPipeline->initialize()) {
        Serial.println("✓ Wildlife cloud pipeline initialized");
        g_wildlifeCloudPipeline->enableRealtimeProcessing(true);
        g_wildlifeCloudPipeline->setDetectionThreshold(0.8); // 80% confidence
    }
    
    // Initialize research collaboration platform
    if (g_researchCollaborationPlatform->initialize()) {
        Serial.println("✓ Research collaboration platform initialized");
        g_researchCollaborationPlatform->enableDataAnonymization(true);
    }
    
    Serial.println("\nAll systems ready!");
}

void captureAndProcessImage() {
    // Simulate motion detection or scheduled capture
    static uint32_t lastCapture = 0;
    if (millis() - lastCapture < 10000) { // Capture every 10 seconds for demo
        return;
    }
    lastCapture = millis();
    
    Serial.println("\n--- Capturing Image ---");
    
    // Capture image from camera
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    Serial.printf("Image captured: %d bytes\n", fb->len);
    
    // Save to SD card
    String filename = "/detection_" + String(detectionCount) + ".jpg";
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.write(fb->buf, fb->len);
        file.close();
        Serial.printf("Image saved: %s\n", filename.c_str());
        
        // Process with cloud analytics
        AnalyticsResult result;
        if (g_cloudAnalyticsEngine->processWildlifeImage(filename, result)) {
            Serial.printf("Analytics: Species detected with %.0f%% confidence\n", 
                         result.confidence * 100);
            
            // If high confidence detection, trigger wildlife event
            if (result.confidence > 0.8) {
                handleWildlifeDetection("deer", result.confidence);
            }
        }
        
        // Queue for cloud upload
        SyncItem syncItem;
        syncItem.itemId = "img-" + String(detectionCount);
        syncItem.localPath = filename;
        syncItem.remotePath = "images/2024/01/" + filename;
        syncItem.priority = PRIORITY_HIGH;
        syncItem.syncType = SYNC_COMPRESSED;
        syncItem.dataSize = fb->len;
        syncItem.lastModified = millis();
        
        if (g_intelligentSyncManager->addSyncItem(syncItem)) {
            Serial.println("Image queued for cloud upload");
        }
        
        detectionCount++;
    } else {
        Serial.println("Failed to save image to SD card");
    }
    
    // Return frame buffer
    esp_camera_fb_return(fb);
}

void handleWildlifeDetection(const String& species, float confidence) {
    Serial.println("\n!!! Wildlife Detection !!!");
    Serial.printf("Species: %s\n", species.c_str());
    Serial.printf("Confidence: %.2f%%\n", confidence * 100);
    
    // Create detection event
    WildlifeDetectionEvent event;
    event.eventId = "det-" + String(millis());
    event.species = species;
    event.confidence = confidence;
    event.imagePath = "/detection_" + String(detectionCount) + ".jpg";
    event.timestamp = millis();
    event.latitude = 45.5231; // Example coordinates
    event.longitude = -122.6765;
    
    // Process through wildlife pipeline
    if (g_wildlifeCloudPipeline->processDetectionEvent(event)) {
        Serial.println("Detection event processed");
        
        // Check for conservation alerts
        std::vector<ConservationAlert> alerts = 
            g_wildlifeCloudPipeline->getPendingAlerts();
        
        for (const auto& alert : alerts) {
            Serial.println("\n*** CONSERVATION ALERT ***");
            Serial.printf("Type: %s\n", alert.alertType.c_str());
            Serial.printf("Severity: %s\n", alert.severity.c_str());
            Serial.printf("Description: %s\n", alert.description.c_str());
            
            // Send alert to cloud
            sendCloudEvent("conservation_alert", alert.description);
        }
    }
    
    // Send real-time event to cloud
    RealTimeEvent rtEvent;
    rtEvent.eventType = "wildlife_detection";
    rtEvent.deviceId = WiFi.macAddress();
    rtEvent.siteId = "site-001";
    rtEvent.timestamp = millis();
    rtEvent.payload = "{\"species\":\"" + species + "\",\"confidence\":" + String(confidence) + "}";
    rtEvent.priority = 2;
    rtEvent.critical = (confidence > 0.95);
    
    if (g_cloudIntegrator->sendRealTimeEvent(rtEvent)) {
        Serial.println("Real-time event sent to cloud");
    }
}

void syncDataToCloud() {
    Serial.println("\n=== Starting Cloud Synchronization ===");
    
    // Check connection quality
    ConnectionMetrics metrics = g_intelligentSyncManager->measureConnectionQuality();
    Serial.printf("Connection: %.0f%% quality, %d ms latency\n", 
                  metrics.stability * 100, metrics.latency);
    
    // Sync all pending data
    if (syncAllData()) {
        Serial.println("✓ Data synchronized successfully");
        
        // Get sync statistics
        SyncStats stats = g_cloudIntegrator->getSyncStatistics();
        Serial.printf("Total uploads: %d\n", stats.totalUploads);
        Serial.printf("Success rate: %.1f%%\n", stats.successRate);
        Serial.printf("Data uploaded: %.2f MB\n", stats.totalDataUploaded / 1048576.0);
        Serial.printf("Avg response: %d ms\n", stats.averageResponseTime);
    } else {
        Serial.println("✗ Synchronization failed");
        Serial.println("Retrying failed uploads...");
        g_cloudIntegrator->forceSyncAll();
    }
    
    // Report device health
    String healthData = "{";
    healthData += "\"battery\":" + String(analogRead(35)) + ",";
    healthData += "\"temperature\":25.5,";
    healthData += "\"signalStrength\":" + String(WiFi.RSSI()) + ",";
    healthData += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    healthData += "\"detectionCount\":" + String(detectionCount);
    healthData += "}";
    
    g_cloudIntegrator->reportDeviceHealth(healthData);
}

void handleRemoteCommands() {
    // Check for configuration updates from cloud
    static uint32_t lastCommandCheck = 0;
    if (millis() - lastCommandCheck < 60000) { // Check every minute
        return;
    }
    lastCommandCheck = millis();
    
    if (g_cloudIntegrator->requestConfiguration()) {
        Serial.println("Configuration update received from cloud");
        // Process configuration update
        // Update device settings
        // Restart services if needed
    }
}

void monitorSystemHealth() {
    Serial.println("\n=== System Health Check ===");
    
    // Check cloud system health
    bool healthy = isCloudSystemHealthy();
    Serial.printf("Cloud System: %s\n", healthy ? "HEALTHY" : "DEGRADED");
    
    // Check individual services
    if (g_cloudIntegrator->isConnected()) {
        Serial.println("✓ Cloud connection active");
    } else {
        Serial.println("✗ Cloud connection lost - attempting reconnect");
        g_cloudIntegrator->connect();
    }
    
    // Check sync queue
    uint32_t queueSize = g_intelligentSyncManager->getQueueSize();
    Serial.printf("Sync queue: %d items\n", queueSize);
    
    if (queueSize > 50) {
        Serial.println("⚠ Sync queue getting large - increasing sync frequency");
        syncInterval = 120000; // Reduce to 2 minutes
    }
    
    // Check memory
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (ESP.getFreeHeap() < 20000) {
        Serial.println("⚠ Low memory - optimizing");
        g_intelligentSyncManager->clearSyncQueue();
    }
    
    // Update device status in cloud
    String status = healthy ? "healthy" : "degraded";
    g_cloudIntegrator->updateDeviceStatus(status);
}
