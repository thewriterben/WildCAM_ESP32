#include "WebServer.h"
#include "StorageManager.h"
#include "CameraManager.h"
#include "PowerManager.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>

WebServer::WebServer(int serverPort)
    : port(serverPort), storage(nullptr), camera(nullptr), power(nullptr) {
    server = new AsyncWebServer(port);
}

void WebServer::init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef) {
    storage = storageRef;
    camera = cameraRef;
    power = powerRef;
}

void WebServer::begin() {
    // Enable CORS for all routes
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Setup RESTful API endpoints
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    server->on("/api/latest-image", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLatestImage(request);
    });
    
    server->on("/api/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleCapture(request);
    });
    
    server->on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleReboot(request);
    });
    
    // Handle 404
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", "{\"error\":\"Not found\"}");
    });
    
    server->begin();
}

void WebServer::handleStatus(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    
    // System status
    doc["uptime"] = millis() / 1000;
    doc["free_heap"] = ESP.getFreeHeap();
    
    // Camera status
    if (camera) {
        doc["camera_initialized"] = camera->isInitialized();
    }
    
    // Power status
    if (power) {
        doc["battery_voltage"] = power->getBatteryVoltage();
        doc["battery_percentage"] = power->getBatteryPercentage();
        doc["low_battery"] = power->isLowBattery();
    }
    
    // Storage status
    if (storage) {
        doc["storage_free"] = storage->getFreeSpace();
        doc["storage_used"] = storage->getUsedSpace();
    }
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleLatestImage(AsyncWebServerRequest* request) {
    request->send(501, "application/json", "{\"error\":\"Not implemented\"}");
}

void WebServer::handleCapture(AsyncWebServerRequest* request) {
    if (!camera || !camera->isInitialized()) {
        request->send(503, "application/json", "{\"error\":\"Camera not initialized\"}");
        return;
    }
    
    camera_fb_t* fb = camera->captureImage();
    if (!fb) {
        request->send(500, "application/json", "{\"error\":\"Capture failed\"}");
        return;
    }
    
    String imagePath;
    if (storage) {
        imagePath = storage->saveImage(fb);
    }
    
    camera->releaseFrameBuffer(fb);
    
    DynamicJsonDocument doc(256);
    doc["success"] = true;
    doc["path"] = imagePath;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleReboot(AsyncWebServerRequest* request) {
    request->send(200, "application/json", "{\"message\":\"Rebooting...\"}");
    delay(100);
    ESP.restart();
}
