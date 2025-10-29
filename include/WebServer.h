#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServer {
private:
    AsyncWebServer* server;
    bool serverStarted;
    String ssid;
    String password;
    
    // Setup routes
    void setupRoutes();
    
    // Handle root page
    void handleRoot(AsyncWebServerRequest* request);
    
    // Handle status API
    void handleStatus(AsyncWebServerRequest* request);
    
    // Handle configuration API
    void handleConfig(AsyncWebServerRequest* request);

public:
    WebServer(const String& ssid, const String& password, int port = 80);
    
    // Initialize WiFi and web server
    bool begin();
    
    // Check WiFi connection
    bool isConnected();
    
    // Get IP address
    String getIPAddress();
    
    // Stop web server
    void stop();
    
    // Get server status
    bool isRunning();
    
    // Update system status for web interface
    void updateStatus(const String& status);
};

#endif // WEB_SERVER_H
