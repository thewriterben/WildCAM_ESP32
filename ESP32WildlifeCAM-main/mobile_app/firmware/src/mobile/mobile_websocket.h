/**
 * @file mobile_websocket.h
 * @brief Mobile WebSocket handler for real-time communication
 */

#ifndef MOBILE_WEBSOCKET_H
#define MOBILE_WEBSOCKET_H

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <map>
#include <set>

/**
 * Mobile client information tracking
 */
struct MobileClientInfo {
    uint32_t clientId;
    bool isMobile;
    bool connected;
    uint32_t lastSeen;
    bool compressionSupported;
    uint32_t messageCount;
    uint32_t bytesTransferred;
    bool hasErrors;
    
    MobileClientInfo() : clientId(0), isMobile(false), connected(false),
                        lastSeen(0), compressionSupported(false),
                        messageCount(0), bytesTransferred(0), hasErrors(false) {}
};

/**
 * Mobile WebSocket handler class
 */
class MobileWebSocketHandler {
private:
    AsyncWebSocket* webSocket_;
    std::map<uint32_t, MobileClientInfo> clientInfo_;
    std::map<String, std::set<uint32_t>> subscriptions_;
    
    // Statistics
    uint32_t lastHeartbeat_;
    uint32_t messagesSent_;
    uint32_t bytesTransferred_;
    bool compressionEnabled_;
    
public:
    MobileWebSocketHandler();
    ~MobileWebSocketHandler();
    
    // Initialization
    void init(AsyncWebSocket* webSocket);
    void cleanup();
    
    // Event handling
    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    // Client management
    void handleClientConnect(AsyncWebSocketClient* client);
    void handleClientDisconnect(AsyncWebSocketClient* client);
    void handleClientData(AsyncWebSocketClient* client, uint8_t* data, size_t len);
    void handleClientError(AsyncWebSocketClient* client);
    
    // Message handling
    void handleMobileMessage(AsyncWebSocketClient* client, const String& message);
    void handlePing(AsyncWebSocketClient* client, const JsonDocument& doc);
    void handleSubscribe(AsyncWebSocketClient* client, const JsonDocument& doc);
    void handleUnsubscribe(AsyncWebSocketClient* client, const JsonDocument& doc);
    void handleCaptureRequest(AsyncWebSocketClient* client, const JsonDocument& doc);
    void handleSettingsRequest(AsyncWebSocketClient* client, const JsonDocument& doc);
    void handleImageListRequest(AsyncWebSocketClient* client, const JsonDocument& doc);
    
    // Broadcasting
    void broadcastMobileSystemStatus();
    void broadcastMobileWildlifeDetection(const String& species, float confidence, const String& imagePath);
    void broadcastMobilePowerUpdate(int batteryPercentage, bool isCharging);
    void broadcastMobileStorageUpdate(uint32_t freeSpace, uint32_t totalSpace);
    void broadcastMobileAlert(const String& alertType, const String& message, const String& severity);
    
    // Channel management
    void broadcastToChannel(const String& channel, const JsonDocument& message);
    void broadcastToMobileClients(const JsonDocument& message);
    void broadcastToAllClients(const JsonDocument& message);
    
    // Client communication
    void sendToClient(AsyncWebSocketClient* client, const JsonDocument& message);
    void sendError(AsyncWebSocketClient* client, const String& error);
    void sendMobileWelcome(AsyncWebSocketClient* client);
    void sendMobileSystemStatus(AsyncWebSocketClient* client);
    void sendMobileSettings(AsyncWebSocketClient* client);
    
    // Utility methods
    bool isMobileClient(AsyncWebSocketClient* client);
    String getClientInfo(AsyncWebSocketClient* client);
    bool supportsCompression(AsyncWebSocketClient* client);
    bool shouldCompress(AsyncWebSocketClient* client, const String& message);
    void updateClientStats(AsyncWebSocketClient* client, size_t bytes);
    
    // Statistics
    uint32_t getConnectedMobileClientCount() const;
    uint32_t getTotalConnectedClientCount() const;
    uint32_t getMessagesSent() const { return messagesSent_; }
    uint32_t getBytesTransferred() const { return bytesTransferred_; }
    
    // Heartbeat and maintenance
    void startHeartbeat();
    void sendHeartbeat();
    void enableCompression(bool enable) { compressionEnabled_ = enable; }
};

// Global instance declaration
extern MobileWebSocketHandler* g_mobileWebSocket;

#endif // MOBILE_WEBSOCKET_H