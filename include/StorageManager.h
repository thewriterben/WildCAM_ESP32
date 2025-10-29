#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_camera.h>

class StorageManager {
private:
    bool sdInitialized;
    String baseDirectory;
    unsigned long imageCounter;
    
    // Generate unique filename
    String generateFilename();

public:
    StorageManager();
    
    // Initialize SD card
    bool begin();
    
    // Save image to SD card
    bool saveImage(camera_fb_t* fb, String& filepath);
    
    // Save image with custom filename
    bool saveImage(camera_fb_t* fb, const String& filename, String& filepath);
    
    // Get available space in MB
    uint64_t getAvailableSpace();
    
    // Get total space in MB
    uint64_t getTotalSpace();
    
    // Get number of stored images
    int getImageCount();
    
    // Delete old images if storage is low
    bool cleanupOldImages(int keepCount);
    
    // Check if SD card is ready
    bool isReady();
    
    // List files in directory
    void listFiles(const String& dirname);
};

#endif // STORAGE_MANAGER_H
