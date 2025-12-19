#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>

// Forward declarations
class StorageManager;
class CameraManager;
class PowerManager;

/**
 * @class WebServer
 * @brief Enhanced web server for ESP32-CAM wildlife camera system
 * 
 * Provides a mobile-responsive web interface with:
 * - Real-time statistics dashboard
 * - Image gallery with thumbnails
 * - Configuration page
 * - RESTful API endpoints
 */
class WebServer {
private:
    AsyncWebServer* server;
    StorageManager* storage;
    CameraManager* camera;
    PowerManager* power;
    int port;
    
    // Configuration values (runtime modifiable)
    int captureInterval;
    int motionSensitivity;
    bool nightMode;
    bool cloudUpload;

public:
    WebServer(int serverPort = 80);
    ~WebServer();
    
    bool init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef);
    void begin();
    
    // Page handlers
    void handleIndex(AsyncWebServerRequest* request);
    void handleGallery(AsyncWebServerRequest* request);
    void handleConfig(AsyncWebServerRequest* request);
    
    // API handlers
    void handleStatus(AsyncWebServerRequest* request);
    void handleLatestImage(AsyncWebServerRequest* request);
    void handleCapture(AsyncWebServerRequest* request);
    void handleReboot(AsyncWebServerRequest* request);
    void handleImagesList(AsyncWebServerRequest* request);
    void handleImage(AsyncWebServerRequest* request, const String& imagePath);
    void handleThumbnail(AsyncWebServerRequest* request, const String& imagePath);
    void handleGetConfig(AsyncWebServerRequest* request);
    void handlePostConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len);
    
    // Static content
    static const char* getIndexHTML();
    static const char* getGalleryHTML();
    static const char* getConfigHTML();
    static const char* getStyleCSS();
};

#endif // WEB_SERVER_H
