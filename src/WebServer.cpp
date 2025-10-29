#include "WebServer.h"
#include "StorageManager.h"
#include "CameraManager.h"
#include "PowerManager.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>
#include <WiFi.h>
#include <SD_MMC.h>

WebServer::WebServer(int serverPort)
    : port(serverPort), storage(nullptr), camera(nullptr), power(nullptr) {
    server = new AsyncWebServer(port);
}

bool WebServer::init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef) {
    storage = storageRef;
    camera = cameraRef;
    power = powerRef;
    
    // Validate all references are not null
    if (!storage || !camera || !power) {
        Serial.println("ERROR: WebServer init failed - null reference(s) provided");
        return false;
    }
    
    Serial.println("WebServer initialized successfully");
    return true;
}

void WebServer::begin() {
    // Enable CORS for all routes
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Serve index.html from root
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", 
            "<!DOCTYPE html><html><head><title>WildCAM ESP32</title></head>"
            "<body><h1>WildCAM ESP32</h1><p>Camera web server is running</p>"
            "<ul><li><a href='/status'>Status</a></li>"
            "<li><a href='/latest'>Latest Image</a></li></ul></body></html>");
    });
    
    // Setup RESTful API endpoints
    server->on("/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    server->on("/latest", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLatestImage(request);
    });
    
    server->on("/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleCapture(request);
    });
    
    server->on("/reboot", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleReboot(request);
    });
    
    // Handle 404
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", "{\"error\":\"Not found\"}");
    });
    
    server->begin();
    
    // Print server URL
    Serial.println("==============================");
    Serial.println("Web Server Started");
    Serial.print("Server URL: http://");
    Serial.print(WiFi.localIP().toString());
    Serial.print(":");
    Serial.println(port);
    Serial.println("==============================");
}

void WebServer::handleStatus(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    
    // System status
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Power status
    if (power) {
        doc["batteryVoltage"] = power->getBatteryVoltage();
        doc["batteryPercentage"] = power->getBatteryPercentage();
    } else {
        doc["batteryVoltage"] = 0.0;
        doc["batteryPercentage"] = 0;
    }
    
    // Storage status
    if (storage) {
        doc["sdCardFreeSpace"] = storage->getFreeSpace();
        doc["sdCardUsedSpace"] = storage->getUsedSpace();
        doc["imageCount"] = storage->getImageCount();
    } else {
        doc["sdCardFreeSpace"] = 0;
        doc["sdCardUsedSpace"] = 0;
        doc["imageCount"] = 0;
    }
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleLatestImage(AsyncWebServerRequest* request) {
    if (!storage) {
        request->send(503, "application/json", "{\"error\":\"Storage not available\"}");
        return;
    }
    
    // Get list of image files from SD card
    std::vector<String> imageFiles = storage->getImageFiles();
    
    // Check if any images exist
    if (imageFiles.empty()) {
        request->send(404, "application/json", "{\"error\":\"No images found\"}");
        return;
    }
    
    // Get the most recent image (first in the sorted list)
    String latestImagePath = imageFiles[0];
    
    // Send file with image/jpeg content type
    request->send(SD_MMC, latestImagePath, "image/jpeg");
}

void WebServer::handleCapture(AsyncWebServerRequest* request) {
    if (!camera || !camera->isInitialized()) {
        request->send(503, "application/json", "{\"error\":\"Camera not initialized\"}");
        return;
    }
    
    // Trigger immediate image capture
    camera_fb_t* fb = camera->captureImage();
    if (!fb) {
        request->send(500, "application/json", "{\"error\":\"Capture failed\"}");
        return;
    }
    
    String imagePath;
    size_t imageSize = fb->len;
    
    // Save image to SD card
    if (storage) {
        imagePath = storage->saveImage(fb);
        if (imagePath.length() == 0) {
            camera->releaseFrameBuffer(fb);
            request->send(500, "application/json", "{\"error\":\"Failed to save image\"}");
            return;
        }
    }
    
    camera->releaseFrameBuffer(fb);
    
    // Return JSON with image path and size
    DynamicJsonDocument doc(256);
    doc["success"] = true;
    doc["path"] = imagePath;
    doc["size"] = imageSize;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleReboot(AsyncWebServerRequest* request) {
    // Send confirmation response
    request->send(200, "application/json", "{\"message\":\"Rebooting...\"}");
    
    // Delay 1 second
    delay(1000);
    
    // Call ESP.restart()
    ESP.restart();
}
