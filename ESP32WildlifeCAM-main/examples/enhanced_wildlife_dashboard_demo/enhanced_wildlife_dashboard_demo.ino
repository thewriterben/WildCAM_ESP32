/**
 * @file enhanced_wildlife_dashboard_demo.ino
 * @brief Enhanced Wildlife Monitoring with Real-Time Web Dashboard
 * 
 * Demonstrates the enhanced web dashboard with:
 * - Real-time WebSocket updates
 * - Modern responsive interface
 * - Live system metrics
 * - Wildlife detection notifications
 * - Power and storage monitoring
 * - Image gallery with thumbnails
 * 
 * This example builds upon existing wildlife monitoring capabilities
 * and adds the new enhanced web dashboard functionality.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include <esp_camera.h>
#include <time.h>

// Include enhanced web server
#include "firmware/src/web/enhanced_web_server.h"

// Include existing system components
#include "firmware/src/power_manager.h"
#include "src/data/storage_manager.h"
#include "src/ai/wildlife_classifier.h"

// Configuration
#include "wifi_config.h" // Create this file with your WiFi credentials

// Pin definitions for ESP32-CAM
#define PIR_SENSOR_PIN 13
#define FLASH_LED_PIN 4
#define STATUS_LED_PIN 33

// System state
struct SystemState {
    bool cameraReady = false;
    bool wifiConnected = false;
    bool sdCardReady = false;
    bool monitoringActive = false;
    bool motionDetected = false;
    uint32_t totalCaptures = 0;
    uint32_t dailyCaptures = 0;
    String lastDetectedSpecies = "";
    float lastDetectionConfidence = 0.0;
    unsigned long lastCaptureTime = 0;
    unsigned long lastUpdateTime = 0;
} systemState;

// Components
EnhancedWebServer* webServer = nullptr;
PowerManager* powerManager = nullptr;
WildlifeClassifier* aiClassifier = nullptr;

// Configuration
const uint32_t CAPTURE_INTERVAL_MS = 30000;    // 30 seconds between captures
const uint32_t MOTION_TIMEOUT_MS = 10000;      // 10 seconds motion timeout
const uint32_t UPDATE_INTERVAL_MS = 5000;      // 5 seconds update interval
const uint32_t HEARTBEAT_INTERVAL_MS = 30000;  // 30 seconds heartbeat

// Timing
unsigned long lastMotionCheck = 0;
unsigned long lastSystemUpdate = 0;
unsigned long lastHeartbeat = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("ESP32 Wildlife Camera - Enhanced Dashboard");
    Serial.println("Real-time monitoring with WebSocket updates");
    Serial.println("========================================");
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    // Initialize PIR sensor
    pinMode(PIR_SENSOR_PIN, INPUT);
    
    // Initialize flash LED
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, LOW);
    
    // Initialize components
    if (!initializeCamera()) {
        Serial.println("ERROR: Camera initialization failed");
        return;
    }
    
    if (!initializeStorage()) {
        Serial.println("WARNING: SD card initialization failed");
    }
    
    if (!initializeWiFi()) {
        Serial.println("ERROR: WiFi initialization failed");
        return;
    }
    
    if (!initializePowerManager()) {
        Serial.println("WARNING: Power manager initialization failed");
    }
    
    if (!initializeAI()) {
        Serial.println("WARNING: AI classifier initialization failed");
    }
    
    if (!initializeWebServer()) {
        Serial.println("ERROR: Web server initialization failed");
        return;
    }
    
    // Set up time
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
    // Initialize system state
    systemState.cameraReady = true;
    systemState.wifiConnected = true;
    systemState.sdCardReady = true;
    systemState.monitoringActive = true;
    
    Serial.println("\n✅ System initialization complete!");
    Serial.println("🌐 Dashboard available at: http://" + WiFi.localIP().toString() + "/");
    Serial.println("📡 WebSocket endpoint: ws://" + WiFi.localIP().toString() + "/ws");
    Serial.println("📱 Mobile-friendly responsive design");
    Serial.println("🔄 Real-time updates enabled");
    Serial.println();
    
    // Signal ready
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // Send initial notification
    if (webServer) {
        webServer->broadcastSystemStatus();
    }
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check for motion detection
    if (currentTime - lastMotionCheck >= 1000) { // Check every second
        checkMotion();
        lastMotionCheck = currentTime;
    }
    
    // Periodic system updates
    if (currentTime - lastSystemUpdate >= UPDATE_INTERVAL_MS) {
        updateSystemMetrics();
        lastSystemUpdate = currentTime;
    }
    
    // Heartbeat for dashboard
    if (currentTime - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        sendHeartbeat();
        lastHeartbeat = currentTime;
    }
    
    // Handle camera capture if motion detected
    if (systemState.motionDetected && 
        currentTime - systemState.lastCaptureTime >= CAPTURE_INTERVAL_MS) {
        captureAndAnalyze();
        systemState.motionDetected = false;
    }
    
    // Small delay to prevent overwhelming the system
    delay(100);
}

bool initializeCamera() {
    Serial.print("Initializing camera... ");
    
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
        Serial.printf("FAILED (0x%x)\n", err);
        return false;
    }
    
    Serial.println("✅ SUCCESS");
    return true;
}

bool initializeStorage() {
    Serial.print("Initializing SD card... ");
    
    if (!SD_MMC.begin()) {
        Serial.println("❌ FAILED");
        return false;
    }
    
    // Check card type
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("❌ No SD card attached");
        return false;
    }
    
    // Create directories
    SD_MMC.mkdir("/wildlife");
    SD_MMC.mkdir("/thumbnails");
    SD_MMC.mkdir("/config");
    SD_MMC.mkdir("/logs");
    
    Serial.println("✅ SUCCESS");
    return true;
}

bool initializeWiFi() {
    Serial.print("Connecting to WiFi");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(" ❌ FAILED");
        return false;
    }
    
    Serial.println(" ✅ SUCCESS");
    Serial.println("IP address: " + WiFi.localIP().toString());
    return true;
}

bool initializePowerManager() {
    Serial.print("Initializing power manager... ");
    
    powerManager = new PowerManager();
    if (!powerManager->initialize()) {
        Serial.println("❌ FAILED");
        return false;
    }
    
    Serial.println("✅ SUCCESS");
    return true;
}

bool initializeAI() {
    Serial.print("Initializing AI classifier... ");
    
    aiClassifier = new WildlifeClassifier();
    if (!aiClassifier->initialize()) {
        Serial.println("❌ FAILED");
        return false;
    }
    
    Serial.println("✅ SUCCESS");
    return true;
}

bool initializeWebServer() {
    Serial.print("Starting enhanced web server... ");
    
    // Configure enhanced web server
    EnhancedWebConfig config;
    config.port = 80;
    config.enableAuth = false;
    config.enableCORS = true;
    config.wsHeartbeatInterval = HEARTBEAT_INTERVAL_MS;
    config.systemUpdateInterval = UPDATE_INTERVAL_MS;
    
    webServer = new EnhancedWebServer();
    
    // Set up logging callback
    webServer->onLog([](const String& message) {
        Serial.println("[WebServer] " + message);
    });
    
    if (!webServer->begin(config)) {
        Serial.println("❌ FAILED");
        return false;
    }
    
    Serial.println("✅ SUCCESS");
    return true;
}

void checkMotion() {
    bool motionNow = digitalRead(PIR_SENSOR_PIN) == HIGH;
    
    if (motionNow && !systemState.motionDetected) {
        systemState.motionDetected = true;
        Serial.println("🚨 Motion detected!");
        
        // Broadcast motion detection to dashboard
        if (webServer) {
            DynamicJsonDocument alert(256);
            alert["message"] = "Motion detected";
            alert["timestamp"] = millis();
            webServer->broadcastErrorAlert("Motion detected - preparing to capture");
        }
        
        // Flash status LED
        for (int i = 0; i < 3; i++) {
            digitalWrite(STATUS_LED_PIN, LOW);
            delay(100);
            digitalWrite(STATUS_LED_PIN, HIGH);
            delay(100);
        }
    }
}

void captureAndAnalyze() {
    Serial.println("📸 Capturing image...");
    
    // Flash for better image quality
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(100);
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    
    digitalWrite(FLASH_LED_PIN, LOW);
    
    if (!fb) {
        Serial.println("❌ Camera capture failed");
        return;
    }
    
    // Generate filename
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    char filename[64];
    strftime(filename, sizeof(filename), "/wildlife/IMG_%Y%m%d_%H%M%S.jpg", &timeinfo);
    
    // Save to SD card
    bool saved = false;
    if (systemState.sdCardReady) {
        File file = SD_MMC.open(filename, FILE_WRITE);
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
            saved = true;
            Serial.printf("💾 Image saved: %s (%d bytes)\n", filename, fb->len);
        }
    }
    
    // Analyze with AI if available
    String detectedSpecies = "unknown";
    float confidence = 0.0;
    
    if (aiClassifier) {
        auto result = aiClassifier->classifyImage(fb->buf, fb->len);
        if (result.confidence > 0.7) { // Confidence threshold
            detectedSpecies = result.species;
            confidence = result.confidence;
            Serial.printf("🧠 AI Detection: %s (%.2f confidence)\n", 
                         detectedSpecies.c_str(), confidence);
        }
    }
    
    // Update system state
    systemState.totalCaptures++;
    systemState.dailyCaptures++;
    systemState.lastCaptureTime = millis();
    systemState.lastDetectedSpecies = detectedSpecies;
    systemState.lastDetectionConfidence = confidence;
    
    // Broadcast wildlife detection to dashboard
    if (webServer && saved) {
        webServer->broadcastWildlifeDetection(detectedSpecies, confidence, filename);
    }
    
    // Release camera buffer
    esp_camera_fb_return(fb);
    
    Serial.printf("📊 Total captures: %d (today: %d)\n", 
                  systemState.totalCaptures, systemState.dailyCaptures);
}

void updateSystemMetrics() {
    if (!webServer) return;
    
    // Update power metrics
    if (powerManager) {
        webServer->triggerPowerUpdate();
    }
    
    // Update storage metrics
    webServer->triggerStorageUpdate();
    
    // Update system status
    webServer->triggerSystemUpdate();
    
    systemState.lastUpdateTime = millis();
}

void sendHeartbeat() {
    if (!webServer) return;
    
    // Send heartbeat to maintain WebSocket connections
    Serial.printf("💓 Heartbeat - Connected clients: %d\n", 
                  webServer->getConnectedClients());
}

// API endpoint handlers that integrate with the enhanced web server
void handleManualCapture() {
    // This would be called by the enhanced web server's capture endpoint
    Serial.println("📱 Manual capture requested from dashboard");
    
    if (systemState.cameraReady) {
        captureAndAnalyze();
    }
}

void handleConfigUpdate(const String& setting, const String& value) {
    // This would be called by the enhanced web server's config endpoint
    Serial.printf("⚙️  Configuration update: %s = %s\n", setting.c_str(), value.c_str());
    
    // Handle configuration changes
    if (setting == "motionSensitivity") {
        // Update motion sensitivity
    } else if (setting == "captureInterval") {
        // Update capture interval
    }
}

// Status reporting function
void printSystemStatus() {
    Serial.println("\n📊 === SYSTEM STATUS ===");
    Serial.printf("🔌 WiFi: %s (IP: %s)\n", 
                  systemState.wifiConnected ? "Connected" : "Disconnected",
                  WiFi.localIP().toString().c_str());
    Serial.printf("📷 Camera: %s\n", systemState.cameraReady ? "Ready" : "Not Ready");
    Serial.printf("💾 SD Card: %s\n", systemState.sdCardReady ? "Ready" : "Not Ready");
    Serial.printf("🎯 Monitoring: %s\n", systemState.monitoringActive ? "Active" : "Inactive");
    Serial.printf("📸 Captures: %d total, %d today\n", 
                  systemState.totalCaptures, systemState.dailyCaptures);
    
    if (systemState.lastDetectedSpecies.length() > 0) {
        Serial.printf("🦌 Last Detection: %s (%.2f confidence)\n", 
                      systemState.lastDetectedSpecies.c_str(), 
                      systemState.lastDetectionConfidence);
    }
    
    if (webServer) {
        Serial.printf("🌐 Web Server: Running on port 80\n");
        Serial.printf("📡 WebSocket Clients: %d\n", webServer->getConnectedClients());
    }
    
    if (powerManager) {
        PowerSystemStatus powerStatus = powerManager->getSystemStatus();
        Serial.printf("🔋 Battery: %.2fV (%d%%) %s\n", 
                      powerStatus.batteryVoltage,
                      powerStatus.batteryPercentage,
                      powerStatus.isCharging ? "⚡Charging" : "");
    }
    
    Serial.printf("⏱️  Uptime: %s\n", formatUptime(millis()).c_str());
    Serial.printf("💾 Free Memory: %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.println("========================\n");
}

String formatUptime(unsigned long uptime) {
    unsigned long seconds = uptime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    String result = "";
    if (days > 0) result += String(days) + "d ";
    if (hours % 24 > 0) result += String(hours % 24) + "h ";
    if (minutes % 60 > 0) result += String(minutes % 60) + "m ";
    if (result.length() == 0) result = String(seconds % 60) + "s";
    
    return result.substring(0, result.length() - 1); // Remove trailing space
}