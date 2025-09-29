#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class ConfigServer {
private:
    AsyncWebServer server;
    struct Config {
        String wifi_ssid;
        String wifi_password;
        String api_endpoint;
        String device_id;
        int capture_interval;
        int motion_sensitivity;
        bool night_mode;
        bool cloud_upload;
    } config;
    
public:
    ConfigServer() : server(80) {}
    
    void begin() {
        // Serve configuration web page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/index.html", "text/html");
        });
        
        // API endpoints
        server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request){
            StaticJsonDocument<512> doc;
            doc["wifi_ssid"] = config.wifi_ssid;
            doc["api_endpoint"] = config.api_endpoint;
            doc["device_id"] = config.device_id;
            doc["capture_interval"] = config.capture_interval;
            doc["motion_sensitivity"] = config.motion_sensitivity;
            doc["night_mode"] = config.night_mode;
            doc["cloud_upload"] = config.cloud_upload;
            
            String response;
            serializeJson(doc, response);
            request->send(200, "application/json", response);
        });
        
        server.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest *request){
            // Handle configuration updates
        });
        
        server.on("/api/capture", HTTP_GET, [](AsyncWebServerRequest *request){
            // Trigger manual capture
        });
        
        server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
            // Return device status
        });
        
        server.begin();
    }
};