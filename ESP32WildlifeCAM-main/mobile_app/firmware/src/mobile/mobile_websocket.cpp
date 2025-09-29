/**
 * @file mobile_websocket.cpp
 * @brief Mobile-optimized WebSocket handler for ESP32 Wildlife Camera
 * 
 * Provides optimized WebSocket communication for mobile devices with
 * bandwidth consideration and mobile-specific message types.
 */

#include "mobile_websocket.h"
#include <esp_log.h>
#include <ArduinoJson.h>

static const char* TAG = "MobileWebSocket";

// Global instance
MobileWebSocketHandler* g_mobileWebSocket = nullptr;

MobileWebSocketHandler::MobileWebSocketHandler() {
    lastHeartbeat_ = 0;
    messagesSent_ = 0;
    bytesTransferred_ = 0;
    compressionEnabled_ = true;
    
    MOBILE_API_LOG("Mobile WebSocket handler initialized");
}

MobileWebSocketHandler::~MobileWebSocketHandler() {
    cleanup();
}

void MobileWebSocketHandler::init(AsyncWebSocket* webSocket) {
    webSocket_ = webSocket;
    
    // Set up WebSocket event handler
    webSocket_->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, 
                               AwsEventType type, void* arg, uint8_t* data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });
    
    // Start background tasks
    startHeartbeat();
    
    MOBILE_API_LOG("Mobile WebSocket initialized with server");
}

void MobileWebSocketHandler::onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                   AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            handleClientConnect(client);
            break;
            
        case WS_EVT_DISCONNECT:
            handleClientDisconnect(client);
            break;
            
        case WS_EVT_DATA:
            handleClientData(client, data, len);
            break;
            
        case WS_EVT_ERROR:
            handleClientError(client);
            break;
            
        default:
            break;
    }
}

void MobileWebSocketHandler::handleClientConnect(AsyncWebSocketClient* client) {
    String clientInfo = getClientInfo(client);
    MOBILE_API_LOG("Mobile client connected: %u [%s]", client->id(), clientInfo.c_str());
    
    // Check if it's a mobile client
    bool isMobile = isMobileClient(client);
    
    // Store client info
    MobileClientInfo info;
    info.clientId = client->id();
    info.isMobile = isMobile;
    info.connected = true;
    info.lastSeen = millis();
    info.compressionSupported = supportsCompression(client);
    info.messageCount = 0;
    info.bytesTransferred = 0;
    
    clientInfo_[client->id()] = info;
    
    // Send initial mobile status for mobile clients
    if (isMobile) {
        sendMobileWelcome(client);
        sendMobileSystemStatus(client);
    }
}

void MobileWebSocketHandler::handleClientDisconnect(AsyncWebSocketClient* client) {
    MOBILE_API_LOG("Mobile client disconnected: %u", client->id());
    
    // Remove client info
    clientInfo_.erase(client->id());
}

void MobileWebSocketHandler::handleClientData(AsyncWebSocketClient* client, uint8_t* data, size_t len) {
    // Update client stats
    if (clientInfo_.find(client->id()) != clientInfo_.end()) {
        clientInfo_[client->id()].lastSeen = millis();
        clientInfo_[client->id()].messageCount++;
    }
    
    // Parse message
    String message = String((char*)data, len);
    handleMobileMessage(client, message);
}

void MobileWebSocketHandler::handleClientError(AsyncWebSocketClient* client) {
    MOBILE_API_LOG("Mobile client error: %u", client->id());
    
    // Mark client as having issues
    if (clientInfo_.find(client->id()) != clientInfo_.end()) {
        clientInfo_[client->id()].hasErrors = true;
    }
}

void MobileWebSocketHandler::handleMobileMessage(AsyncWebSocketClient* client, const String& message) {
    DynamicJsonDocument doc(512);
    
    if (deserializeJson(doc, message) != DeserializationError::Ok) {
        MOBILE_API_LOG("Invalid JSON from mobile client %u", client->id());
        sendError(client, "Invalid JSON message");
        return;
    }
    
    String action = doc["action"] | "";
    
    if (action == "ping") {
        handlePing(client, doc);
    } else if (action == "subscribe") {
        handleSubscribe(client, doc);
    } else if (action == "unsubscribe") {
        handleUnsubscribe(client, doc);
    } else if (action == "capture") {
        handleCaptureRequest(client, doc);
    } else if (action == "settings") {
        handleSettingsRequest(client, doc);
    } else if (action == "get_status") {
        sendMobileSystemStatus(client);
    } else if (action == "get_images") {
        handleImageListRequest(client, doc);
    } else {
        sendError(client, "Unknown action: " + action);
    }
}

void MobileWebSocketHandler::handlePing(AsyncWebSocketClient* client, const JsonDocument& doc) {
    // Respond with pong
    DynamicJsonDocument response(256);
    response["type"] = "pong";
    response["timestamp"] = millis();
    response["server_time"] = doc["timestamp"] | 0;
    
    sendToClient(client, response);
}

void MobileWebSocketHandler::handleSubscribe(AsyncWebSocketClient* client, const JsonDocument& doc) {
    String channel = doc["channel"] | "";
    
    if (channel.length() == 0) {
        sendError(client, "Missing channel parameter");
        return;
    }
    
    // Add client to channel subscription
    subscriptions_[channel].insert(client->id());
    
    MOBILE_API_LOG("Client %u subscribed to channel: %s", client->id(), channel.c_str());
    
    // Send confirmation
    DynamicJsonDocument response(256);
    response["type"] = "subscribed";
    response["channel"] = channel;
    response["success"] = true;
    
    sendToClient(client, response);
}

void MobileWebSocketHandler::handleUnsubscribe(AsyncWebSocketClient* client, const JsonDocument& doc) {
    String channel = doc["channel"] | "";
    
    if (channel.length() == 0) {
        sendError(client, "Missing channel parameter");
        return;
    }
    
    // Remove client from channel subscription
    subscriptions_[channel].erase(client->id());
    
    MOBILE_API_LOG("Client %u unsubscribed from channel: %s", client->id(), channel.c_str());
    
    // Send confirmation
    DynamicJsonDocument response(256);
    response["type"] = "unsubscribed";
    response["channel"] = channel;
    response["success"] = true;
    
    sendToClient(client, response);
}

void MobileWebSocketHandler::handleCaptureRequest(AsyncWebSocketClient* client, const JsonDocument& doc) {
    String mode = doc["mode"] | "single";
    int quality = doc["quality"] | 85;
    
    // Validate mobile capture request
    if (quality < 10 || quality > 100) {
        sendError(client, "Invalid quality parameter");
        return;
    }
    
    // Process capture request (placeholder)
    bool success = true; // Would implement actual capture
    
    DynamicJsonDocument response(256);
    response["type"] = "capture_response";
    response["success"] = success;
    response["mode"] = mode;
    response["quality"] = quality;
    response["timestamp"] = millis();
    
    if (success) {
        response["message"] = "Capture initiated";
        response["image_id"] = "IMG_" + String(millis());
    } else {
        response["error"] = "Capture failed";
    }
    
    sendToClient(client, response);
}

void MobileWebSocketHandler::handleSettingsRequest(AsyncWebSocketClient* client, const JsonDocument& doc) {
    String type = doc["type"] | "";
    
    if (type == "get") {
        // Return current settings
        sendMobileSettings(client);
    } else if (type == "set") {
        // Update settings (placeholder)
        DynamicJsonDocument response(256);
        response["type"] = "settings_updated";
        response["success"] = true;
        response["timestamp"] = millis();
        
        sendToClient(client, response);
    } else {
        sendError(client, "Invalid settings request type");
    }
}

void MobileWebSocketHandler::handleImageListRequest(AsyncWebSocketClient* client, const JsonDocument& doc) {
    int page = doc["page"] | 0;
    int pageSize = doc["page_size"] | 20;
    
    // Limit page size for mobile
    pageSize = min(pageSize, 50);
    
    // Generate mobile image list
    DynamicJsonDocument response(1024);
    response["type"] = "image_list";
    response["page"] = page;
    response["page_size"] = pageSize;
    response["total_images"] = 0; // Would get actual count
    
    JsonArray images = response.createNestedArray("images");
    
    // Add placeholder images
    for (int i = 0; i < min(pageSize, 5); i++) {
        JsonObject img = images.createNestedObject();
        img["id"] = page * pageSize + i;
        img["filename"] = "IMG_" + String(img["id"].as<int>()) + ".jpg";
        img["thumbnail"] = "/api/mobile/thumbnail?image=" + img["filename"].as<String>();
        img["timestamp"] = millis() - (i * 3600000);
        img["size"] = 1024 * (100 + i * 50);
    }
    
    sendToClient(client, response);
}

// Broadcasting methods
void MobileWebSocketHandler::broadcastMobileSystemStatus() {
    DynamicJsonDocument doc(768);
    doc["type"] = "system_status";
    doc["timestamp"] = millis();
    
    // Add mobile-optimized system status
    if (g_mobileAPI) {
        String statusJSON = g_mobileAPI->generateMobileStatusJSON();
        DynamicJsonDocument statusDoc(768);
        deserializeJson(statusDoc, statusJSON);
        doc["data"] = statusDoc;
    }
    
    broadcastToMobileClients(doc);
}

void MobileWebSocketHandler::broadcastMobileWildlifeDetection(const String& species, 
                                                              float confidence, 
                                                              const String& imagePath) {
    DynamicJsonDocument doc(512);
    doc["type"] = "wildlife_detection";
    doc["timestamp"] = millis();
    doc["species"] = species;
    doc["confidence"] = confidence;
    doc["image_path"] = imagePath;
    doc["thumbnail"] = "/api/mobile/thumbnail?image=" + imagePath;
    
    broadcastToChannel("wildlife", doc);
    
    // Also send to all mobile clients
    broadcastToMobileClients(doc);
}

void MobileWebSocketHandler::broadcastMobilePowerUpdate(int batteryPercentage, bool isCharging) {
    DynamicJsonDocument doc(256);
    doc["type"] = "power_update";
    doc["timestamp"] = millis();
    doc["battery"] = batteryPercentage;
    doc["charging"] = isCharging;
    doc["low_battery"] = batteryPercentage < 20;
    
    broadcastToChannel("power", doc);
}

void MobileWebSocketHandler::broadcastMobileStorageUpdate(uint32_t freeSpace, uint32_t totalSpace) {
    DynamicJsonDocument doc(256);
    doc["type"] = "storage_update";
    doc["timestamp"] = millis();
    doc["free_space"] = freeSpace;
    doc["total_space"] = totalSpace;
    doc["usage_percent"] = ((totalSpace - freeSpace) * 100) / totalSpace;
    doc["low_storage"] = (freeSpace * 100) / totalSpace < 10;
    
    broadcastToChannel("storage", doc);
}

void MobileWebSocketHandler::broadcastMobileAlert(const String& alertType, 
                                                  const String& message, 
                                                  const String& severity) {
    DynamicJsonDocument doc(512);
    doc["type"] = "alert";
    doc["timestamp"] = millis();
    doc["alert_type"] = alertType;
    doc["message"] = message;
    doc["severity"] = severity;
    doc["mobile_optimized"] = true;
    
    broadcastToChannel("alerts", doc);
    broadcastToMobileClients(doc);
}

// Channel management
void MobileWebSocketHandler::broadcastToChannel(const String& channel, const JsonDocument& message) {
    if (subscriptions_.find(channel) == subscriptions_.end()) {
        return; // No subscribers
    }
    
    for (uint32_t clientId : subscriptions_[channel]) {
        AsyncWebSocketClient* client = webSocket_->client(clientId);
        if (client && client->status() == WS_CONNECTED) {
            sendToClient(client, message);
        } else {
            // Remove disconnected client
            subscriptions_[channel].erase(clientId);
        }
    }
}

void MobileWebSocketHandler::broadcastToMobileClients(const JsonDocument& message) {
    for (auto& pair : clientInfo_) {
        if (pair.second.isMobile && pair.second.connected) {
            AsyncWebSocketClient* client = webSocket_->client(pair.first);
            if (client && client->status() == WS_CONNECTED) {
                sendToClient(client, message);
            } else {
                pair.second.connected = false;
            }
        }
    }
}

void MobileWebSocketHandler::broadcastToAllClients(const JsonDocument& message) {
    for (auto& pair : clientInfo_) {
        if (pair.second.connected) {
            AsyncWebSocketClient* client = webSocket_->client(pair.first);
            if (client && client->status() == WS_CONNECTED) {
                sendToClient(client, message);
            } else {
                pair.second.connected = false;
            }
        }
    }
}

// Client communication
void MobileWebSocketHandler::sendToClient(AsyncWebSocketClient* client, const JsonDocument& message) {
    String messageStr;
    serializeJson(message, messageStr);
    
    // Check if compression should be applied
    bool compress = shouldCompress(client, messageStr);
    
    if (compress) {
        // Would implement compression here
        client->text(messageStr);
    } else {
        client->text(messageStr);
    }
    
    // Update statistics
    updateClientStats(client, messageStr.length());
}

void MobileWebSocketHandler::sendError(AsyncWebSocketClient* client, const String& error) {
    DynamicJsonDocument doc(256);
    doc["type"] = "error";
    doc["error"] = error;
    doc["timestamp"] = millis();
    
    sendToClient(client, doc);
}

void MobileWebSocketHandler::sendMobileWelcome(AsyncWebSocketClient* client) {
    DynamicJsonDocument doc(512);
    doc["type"] = "welcome";
    doc["server"] = "ESP32 Wildlife Camera";
    doc["version"] = "1.0.0";
    doc["mobile_optimized"] = true;
    doc["timestamp"] = millis();
    
    // Available channels
    JsonArray channels = doc.createNestedArray("channels");
    channels.add("system");
    channels.add("wildlife");
    channels.add("power");
    channels.add("storage");
    channels.add("alerts");
    
    // Mobile features
    JsonObject features = doc.createNestedObject("features");
    features["compression"] = compressionEnabled_;
    features["burst_mode"] = true;
    features["push_notifications"] = true;
    features["offline_sync"] = true;
    
    sendToClient(client, doc);
}

void MobileWebSocketHandler::sendMobileSystemStatus(AsyncWebSocketClient* client) {
    DynamicJsonDocument doc(768);
    doc["type"] = "system_status";
    doc["timestamp"] = millis();
    
    if (g_mobileAPI) {
        String statusJSON = g_mobileAPI->generateMobileStatusJSON();
        DynamicJsonDocument statusDoc(768);
        deserializeJson(statusDoc, statusJSON);
        doc["data"] = statusDoc;
    }
    
    sendToClient(client, doc);
}

void MobileWebSocketHandler::sendMobileSettings(AsyncWebSocketClient* client) {
    DynamicJsonDocument doc(768);
    doc["type"] = "settings";
    doc["timestamp"] = millis();
    
    if (g_mobileAPI) {
        String settingsJSON = g_mobileAPI->generateMobileSettingsJSON();
        DynamicJsonDocument settingsDoc(768);
        deserializeJson(settingsDoc, settingsJSON);
        doc["data"] = settingsDoc;
    }
    
    sendToClient(client, doc);
}

// Utility methods
bool MobileWebSocketHandler::isMobileClient(AsyncWebSocketClient* client) {
    // Check user agent if available (placeholder implementation)
    return true; // Would implement actual mobile detection
}

String MobileWebSocketHandler::getClientInfo(AsyncWebSocketClient* client) {
    return "Mobile:" + client->remoteIP().toString();
}

bool MobileWebSocketHandler::supportsCompression(AsyncWebSocketClient* client) {
    // Check if client supports compression (placeholder)
    return compressionEnabled_;
}

bool MobileWebSocketHandler::shouldCompress(AsyncWebSocketClient* client, const String& message) {
    if (!compressionEnabled_) return false;
    
    // Compress messages larger than 1KB
    if (message.length() > 1024) return true;
    
    // Check client compression support
    auto it = clientInfo_.find(client->id());
    return (it != clientInfo_.end() && it->second.compressionSupported);
}

void MobileWebSocketHandler::updateClientStats(AsyncWebSocketClient* client, size_t bytes) {
    auto it = clientInfo_.find(client->id());
    if (it != clientInfo_.end()) {
        it->second.bytesTransferred += bytes;
        bytesTransferred_ += bytes;
    }
    
    messagesSent_++;
}

// Heartbeat and maintenance
void MobileWebSocketHandler::startHeartbeat() {
    // Would start a timer task for heartbeat
    MOBILE_API_LOG("Mobile WebSocket heartbeat started");
}

void MobileWebSocketHandler::sendHeartbeat() {
    DynamicJsonDocument doc(128);
    doc["type"] = "heartbeat";
    doc["timestamp"] = millis();
    doc["clients"] = getConnectedMobileClientCount();
    
    broadcastToMobileClients(doc);
    lastHeartbeat_ = millis();
}

uint32_t MobileWebSocketHandler::getConnectedMobileClientCount() const {
    uint32_t count = 0;
    for (const auto& pair : clientInfo_) {
        if (pair.second.isMobile && pair.second.connected) {
            count++;
        }
    }
    return count;
}

uint32_t MobileWebSocketHandler::getTotalConnectedClientCount() const {
    uint32_t count = 0;
    for (const auto& pair : clientInfo_) {
        if (pair.second.connected) {
            count++;
        }
    }
    return count;
}

void MobileWebSocketHandler::cleanup() {
    clientInfo_.clear();
    subscriptions_.clear();
    MOBILE_API_LOG("Mobile WebSocket handler cleaned up");
}