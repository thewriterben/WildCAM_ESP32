#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <esp_camera.h>
#include <Preferences.h>
#include <vector>

class StorageManager {
private:
    bool initialized;
    String basePath;
    unsigned long imageCounter;
    Preferences preferences;
    
    // Returns /YYYYMMDD
    String getCurrentDatePath();
    
    // Returns IMG_HHMMSS_XXX.jpg
    String generateFilename();

public:
    StorageManager();
    
    // Initialize SD card
    bool init();
    
    // Save image to SD card
    String saveImage(camera_fb_t* fb, const String& customPath = "");
    
    // Save metadata as JSON
    bool saveMetadata(const String& imagePath, JsonDocument& metadata);
    
    // Delete old files (days to keep)
    bool deleteOldFiles(int daysToKeep = 7);
    
    // Get free space in bytes
    unsigned long getFreeSpace();
    
    // Get used space in bytes
    unsigned long getUsedSpace();
    
    // Print storage information to Serial
    void printStorageInfo();
    
    // Get list of image files (sorted by name, newest first)
    std::vector<String> getImageFiles();
    
    // Get total image count
    unsigned long getImageCount();
};

#endif // STORAGE_MANAGER_H
