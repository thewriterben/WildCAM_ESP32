#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include <esp_camera.h>

class CameraManager {
private:
    camera_config_t config;
    bool initialized;
    int flashPin;
    
    // Setup camera configuration
    void setupConfig();

public:
    CameraManager();
    
    // Initialize camera with configuration
    bool begin();
    
    // Capture image and return frame buffer
    camera_fb_t* captureImage();
    
    // Release frame buffer
    void releaseFrameBuffer(camera_fb_t* fb);
    
    // Set image quality (10-63, lower is better)
    bool setQuality(int quality);
    
    // Set frame size
    bool setFrameSize(framesize_t size);
    
    // Enable/disable flash
    void setFlash(bool enable);
    
    // Get camera status
    bool isInitialized();
    
    // Deinitialize camera to save power
    void deinit();
};

#endif // CAMERA_MANAGER_H
