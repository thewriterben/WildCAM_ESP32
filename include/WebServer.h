#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>

// Forward declarations
class StorageManager;
class CameraManager;
class PowerManager;

class WebServer {
private:
    AsyncWebServer* server;
    StorageManager* storage;
    CameraManager* camera;
    PowerManager* power;
    int port;

public:
    WebServer(int serverPort = 80);
    
    bool init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef);
    void begin();
    void handleStatus(AsyncWebServerRequest* request);
    void handleLatestImage(AsyncWebServerRequest* request);
    void handleCapture(AsyncWebServerRequest* request);
    void handleReboot(AsyncWebServerRequest* request);
};

#endif // WEB_SERVER_H
