/**
 * @file ota_update_demo.ino
 * @brief OTA Update Demo with AsyncElegantOTA
 * 
 * Demonstrates the Phase 1 OTA Enhancement Integration:
 * - AsyncElegantOTA web-based firmware updates
 * - Simple web interface for remote updates
 * - Progress monitoring and status reporting
 * - Integration with existing WiFi systems
 * 
 * This example focuses specifically on the OTA functionality
 * and can be used as a standalone OTA update system.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Phase 1 OTA Enhancement Integration
#include "network/ota_manager.h"

// Configuration
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
const char* OTA_USERNAME = "admin";
const char* OTA_PASSWORD = "wildlife";
const uint16_t OTA_PORT = 80;

// Components
AsyncWebServer server(OTA_PORT);
NetworkOTAManager* otaManager = nullptr;

// Status tracking
unsigned long lastStatusCheck = 0;
const unsigned long STATUS_CHECK_INTERVAL = 5000; // 5 seconds

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("===============================");
    Serial.println("ESP32 Wildlife Camera");
    Serial.println("OTA Update Demo - Phase 1");
    Serial.println("===============================");
    
    // Initialize WiFi
    if (!initializeWiFi()) {
        Serial.println("ERROR: WiFi initialization failed");
        Serial.println("OTA updates require WiFi connection");
        return;
    }
    
    // Initialize OTA Manager
    if (!initializeOTA()) {
        Serial.println("ERROR: OTA initialization failed");
        return;
    }
    
    // Setup additional web pages
    setupWebInterface();
    
    // Start the web server
    server.begin();
    
    Serial.println("\nOTA Update Demo ready!");
    Serial.println("==============================");
    Serial.print("Device IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("OTA Update URL: ");
    Serial.println(getOTAUpdateURL());
    Serial.print("Status Page: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/status");
    Serial.print("Info Page: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/info");
    Serial.println("==============================");
    Serial.println();
    Serial.println("You can now:");
    Serial.println("1. Visit the update URL to upload new firmware");
    Serial.println("2. Check /status for current system status");
    Serial.println("3. Check /info for device information");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check OTA status periodically
    if (currentTime - lastStatusCheck >= STATUS_CHECK_INTERVAL) {
        checkOTAStatus();
        lastStatusCheck = currentTime;
    }
    
    // Keep WiFi connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, attempting reconnection...");
        initializeWiFi();
    }
    
    delay(100);
}

bool initializeWiFi() {
    Serial.print("Connecting to WiFi");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed!");
        return false;
    }
}

bool initializeOTA() {
    Serial.println("Initializing OTA system...");
    
    // Create OTA configuration
    NetworkOTAConfig config;
    config.enabled = true;
    config.port = OTA_PORT;
    config.username = OTA_USERNAME;
    config.password = OTA_PASSWORD;
    config.path = "/update";
    config.requireAuth = true;
    config.autoReboot = true;
    config.timeoutMs = 30000;
    
    // Initialize network OTA
    if (!initializeNetworkOTA(config)) {
        Serial.println("ERROR: Failed to initialize Network OTA");
        return false;
    }
    
    otaManager = g_networkOTAManager;
    
    // Set up OTA event callbacks
    otaManager->onStart([]() {
        Serial.println("OTA Update Started");
        Serial.println("WARNING: Do not power off the device during update!");
    });
    
    otaManager->onProgress([](size_t current, size_t total) {
        float progress = (float)current / total * 100.0;
        Serial.printf("OTA Progress: %.1f%% (%d/%d bytes)\n", progress, current, total);
    });
    
    otaManager->onEnd([](bool success) {
        if (success) {
            Serial.println("OTA Update completed successfully!");
            Serial.println("Device will reboot in 3 seconds...");
            delay(3000);
            ESP.restart();
        } else {
            Serial.println("OTA Update failed!");
            Serial.println("Device will continue with current firmware.");
        }
    });
    
    otaManager->onError([]() {
        Serial.println("OTA Update error occurred!");
        Serial.println("Please check the firmware file and try again.");
    });
    
    // Start OTA server with the web server
    if (!otaManager->begin(&server)) {
        Serial.println("ERROR: Failed to start OTA server");
        return false;
    }
    
    Serial.println("OTA system initialized successfully");
    return true;
}

void setupWebInterface() {
    // Status page
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = generateStatusPage();
        request->send(200, "text/html", html);
    });
    
    // Device info page
    server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = generateInfoPage();
        request->send(200, "text/html", html);
    });
    
    // API endpoint for status JSON
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = generateStatusJSON();
        request->send(200, "application/json", json);
    });
    
    // Root page with navigation
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = generateRootPage();
        request->send(200, "text/html", html);
    });
    
    // Reboot endpoint
    server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<h1>Rebooting...</h1><p>Device will restart in 3 seconds.</p>");
        delay(3000);
        ESP.restart();
    });
}

void checkOTAStatus() {
    if (!otaManager) return;
    
    NetworkOTAStatus status = otaManager->getStatus();
    static NetworkOTAStatus lastStatus = NET_OTA_IDLE;
    
    // Only print status changes to avoid spam
    if (status != lastStatus) {
        Serial.printf("OTA Status: %s\n", otaManager->getStatusString().c_str());
        
        if (status == NET_OTA_UPDATING) {
            Serial.printf("Update progress: %.1f%%\n", otaManager->getProgress());
        }
        
        if (status == NET_OTA_FAILED) {
            Serial.printf("OTA Error: %s\n", otaManager->getLastError().c_str());
        }
        
        lastStatus = status;
    }
}

String generateRootPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>ESP32 Wildlife Camera - OTA Demo</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f8ff; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
    html += "h1 { color: #2e8b57; text-align: center; }";
    html += ".nav { display: flex; gap: 10px; margin: 20px 0; }";
    html += ".nav a { padding: 10px 20px; background: #2e8b57; color: white; text-decoration: none; border-radius: 5px; }";
    html += ".nav a:hover { background: #228b22; }";
    html += ".status { background: #e6ffe6; padding: 15px; border-left: 5px solid #2e8b57; margin: 10px 0; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>🦌 ESP32 Wildlife Camera</h1>";
    html += "<h2>OTA Update Demo - Phase 1</h2>";
    
    html += "<div class='nav'>";
    html += "<a href='/update'>📥 Firmware Update</a>";
    html += "<a href='/status'>📊 System Status</a>";
    html += "<a href='/info'>ℹ️ Device Info</a>";
    html += "<a href='/api/status'>🔗 Status API</a>";
    html += "</div>";
    
    html += "<div class='status'>";
    html += "<h3>Current Status</h3>";
    html += "<p><strong>Device:</strong> ESP32 Wildlife Camera</p>";
    html += "<p><strong>Firmware:</strong> " + String(otaManager ? otaManager->getCurrentVersion() : "Unknown") + "</p>";
    html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>WiFi Signal:</strong> " + String(WiFi.RSSI()) + " dBm</p>";
    html += "<p><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</p>";
    html += "<p><strong>Free Memory:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "</div>";
    
    html += "<div class='status'>";
    html += "<h3>About This Demo</h3>";
    html += "<p>This demo showcases the Phase 1 OTA Enhancement Integration using AsyncElegantOTA.</p>";
    html += "<p><strong>Features:</strong></p>";
    html += "<ul>";
    html += "<li>Web-based firmware updates</li>";
    html += "<li>Real-time progress monitoring</li>";
    html += "<li>Secure authentication</li>";
    html += "<li>Automatic reboot after update</li>";
    html += "<li>Status monitoring and reporting</li>";
    html += "</ul>";
    html += "</div>";
    
    html += "</div></body></html>";
    return html;
}

String generateStatusPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>System Status - ESP32 Wildlife Camera</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='5'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f8ff; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }";
    html += "h1 { color: #2e8b57; }";
    html += ".metric { background: #f9f9f9; padding: 10px; margin: 5px 0; border-left: 3px solid #2e8b57; }";
    html += ".good { border-color: #28a745; }";
    html += ".warning { border-color: #ffc107; }";
    html += ".error { border-color: #dc3545; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>📊 System Status</h1>";
    html += "<p><a href='/'>← Back to Main</a> | <em>Auto-refresh every 5 seconds</em></p>";
    
    // System Information
    html += "<h2>System Information</h2>";
    html += "<div class='metric good'><strong>Uptime:</strong> " + String(millis() / 1000) + " seconds</div>";
    html += "<div class='metric good'><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</div>";
    html += "<div class='metric good'><strong>Chip Model:</strong> " + String(ESP.getChipModel()) + "</div>";
    html += "<div class='metric good'><strong>CPU Frequency:</strong> " + String(ESP.getCpuFreqMHz()) + " MHz</div>";
    
    // Network Information
    html += "<h2>Network Information</h2>";
    if (WiFi.status() == WL_CONNECTED) {
        html += "<div class='metric good'><strong>WiFi Status:</strong> Connected</div>";
        html += "<div class='metric good'><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</div>";
        html += "<div class='metric good'><strong>Signal Strength:</strong> " + String(WiFi.RSSI()) + " dBm</div>";
        html += "<div class='metric good'><strong>MAC Address:</strong> " + WiFi.macAddress() + "</div>";
    } else {
        html += "<div class='metric error'><strong>WiFi Status:</strong> Disconnected</div>";
    }
    
    // OTA Information
    html += "<h2>OTA System</h2>";
    if (otaManager && otaManager->isInitialized()) {
        html += "<div class='metric good'><strong>OTA Status:</strong> " + otaManager->getStatusString() + "</div>";
        html += "<div class='metric good'><strong>Current Version:</strong> " + otaManager->getCurrentVersion() + "</div>";
        html += "<div class='metric good'><strong>Free Space:</strong> " + String(otaManager->getFreeSpace()) + " bytes</div>";
        
        if (otaManager->getStatus() == NET_OTA_UPDATING) {
            html += "<div class='metric warning'><strong>Update Progress:</strong> " + String(otaManager->getProgress()) + "%</div>";
        }
        
        if (!otaManager->getLastError().isEmpty()) {
            html += "<div class='metric error'><strong>Last Error:</strong> " + otaManager->getLastError() + "</div>";
        }
    } else {
        html += "<div class='metric error'><strong>OTA Status:</strong> Not initialized</div>";
    }
    
    html += "<h2>Actions</h2>";
    html += "<p><a href='/update' style='background: #007bff; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px;'>Update Firmware</a></p>";
    html += "<form method='post' action='/reboot' style='display: inline;'>";
    html += "<button type='submit' style='background: #dc3545; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer;'>Reboot Device</button>";
    html += "</form>";
    
    html += "</div></body></html>";
    return html;
}

String generateInfoPage() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Device Information - ESP32 Wildlife Camera</title>";
    html += "<meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f0f8ff; }";
    html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }";
    html += "h1 { color: #2e8b57; }";
    html += "table { width: 100%; border-collapse: collapse; margin: 10px 0; }";
    html += "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }";
    html += "th { background-color: #2e8b57; color: white; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>ℹ️ Device Information</h1>";
    html += "<p><a href='/'>← Back to Main</a></p>";
    
    html += "<table>";
    html += "<tr><th>Property</th><th>Value</th></tr>";
    html += "<tr><td>Device Name</td><td>ESP32 Wildlife Camera</td></tr>";
    html += "<tr><td>Firmware Version</td><td>" + String(otaManager ? otaManager->getCurrentVersion() : "Unknown") + "</td></tr>";
    html += "<tr><td>Chip Model</td><td>" + String(ESP.getChipModel()) + "</td></tr>";
    html += "<tr><td>Chip Revision</td><td>" + String(ESP.getChipRevision()) + "</td></tr>";
    html += "<tr><td>Flash Size</td><td>" + String(ESP.getFlashChipSize()) + " bytes</td></tr>";
    html += "<tr><td>Flash Speed</td><td>" + String(ESP.getFlashChipSpeed()) + " Hz</td></tr>";
    html += "<tr><td>CPU Frequency</td><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
    html += "<tr><td>Free Heap</td><td>" + String(ESP.getFreeHeap()) + " bytes</td></tr>";
    html += "<tr><td>Largest Free Block</td><td>" + String(ESP.getMaxAllocHeap()) + " bytes</td></tr>";
    html += "<tr><td>SDK Version</td><td>" + String(ESP.getSdkVersion()) + "</td></tr>";
    html += "<tr><td>MAC Address</td><td>" + WiFi.macAddress() + "</td></tr>";
    html += "<tr><td>IP Address</td><td>" + WiFi.localIP().toString() + "</td></tr>";
    html += "<tr><td>Boot Count</td><td>" + String(ESP.getBootCount()) + "</td></tr>";
    html += "<tr><td>Uptime</td><td>" + String(millis() / 1000) + " seconds</td></tr>";
    html += "</table>";
    
    html += "</div></body></html>";
    return html;
}

String generateStatusJSON() {
    String json = "{";
    json += "\"device\":\"ESP32 Wildlife Camera\",";
    json += "\"firmware\":\"" + String(otaManager ? otaManager->getCurrentVersion() : "Unknown") + "\",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    if (WiFi.status() == WL_CONNECTED) {
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    }
    if (otaManager) {
        json += "\"ota_status\":\"" + otaManager->getStatusString() + "\",";
        json += "\"ota_progress\":" + String(otaManager->getProgress()) + ",";
    }
    json += "\"timestamp\":" + String(millis());
    json += "}";
    return json;
}