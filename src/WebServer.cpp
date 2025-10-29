#include "WebServer.h"
#include "config.h"

WebServer::WebServer(const String& ssid, const String& password, int port)
    : ssid(ssid), password(password), serverStarted(false) {
    server = new AsyncWebServer(port);
}

bool WebServer::begin() {
    // Connect to WiFi
    Serial.printf("Connecting to WiFi: %s\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to WiFi");
        return false;
    }
    
    Serial.printf("\nConnected to WiFi! IP: %s\n", WiFi.localIP().toString().c_str());
    
    // Setup routes
    setupRoutes();
    
    // Start server
    server->begin();
    serverStarted = true;
    
    Serial.println("Web server started");
    return true;
}

void WebServer::setupRoutes() {
    // Root page
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });
    
    // Status API
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    // Configuration API
    server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleConfig(request);
    });
    
    // Handle 404
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "text/plain", "Not found");
    });
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>WildCAM ESP32</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += "h1{color:#333;}.card{background:white;padding:20px;margin:10px 0;border-radius:5px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
    html += "button{background:#4CAF50;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;}";
    html += "button:hover{background:#45a049;}</style>";
    html += "</head><body>";
    html += "<h1>🦌 WildCAM ESP32 Control Panel</h1>";
    html += "<div class='card'><h2>System Status</h2>";
    html += "<p>Camera: Active</p>";
    html += "<p>WiFi: Connected</p>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p></div>";
    html += "<div class='card'><h2>Controls</h2>";
    html += "<button onclick='capture()'>Capture Image</button> ";
    html += "<button onclick='location.reload()'>Refresh</button></div>";
    html += "<script>function capture(){fetch('/api/capture').then(r=>r.text()).then(d=>alert(d));}</script>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
}

void WebServer::handleStatus(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;
    doc["free_heap"] = ESP.getFreeHeap();
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleConfig(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(512);
    
    doc["ssid"] = ssid;
    doc["version"] = "1.0.0";
    doc["device"] = "ESP32-CAM";
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

bool WebServer::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WebServer::getIPAddress() {
    return WiFi.localIP().toString();
}

void WebServer::stop() {
    if (serverStarted) {
        server->end();
        WiFi.disconnect();
        serverStarted = false;
        Serial.println("Web server stopped");
    }
}

bool WebServer::isRunning() {
    return serverStarted;
}

void WebServer::updateStatus(const String& status) {
    Serial.printf("Status update: %s\n", status.c_str());
}
