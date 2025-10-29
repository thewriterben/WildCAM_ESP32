#include "StorageManager.h"
#include "config.h"
#include <FS.h>
#include <Preferences.h>
#include <time.h>

StorageManager::StorageManager() 
    : initialized(false), basePath("/images"), imageCounter(0) {
}

bool StorageManager::init() {
    Serial.println("Initializing Storage Manager...");
    
    // Initialize SD card in 1-bit mode (uses less pins)
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("ERROR: SD Card Mount Failed");
        return false;
    }
    
    // Check if SD card is mounted
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("ERROR: No SD Card attached");
        return false;
    }
    
    // Get card type and print detailed card information
    Serial.println("=== SD Card Information ===");
    Serial.print("Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    // Get card size
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("Card Size: %llu MB\n", cardSize);
    Serial.printf("Total Space: %llu MB\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used Space: %llu MB\n", SD_MMC.usedBytes() / (1024 * 1024));
    Serial.printf("Free Space: %llu MB\n", (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
    Serial.println("===========================");
    
    // Create base directory if it doesn't exist
    if (!SD_MMC.exists(basePath)) {
        if (SD_MMC.mkdir(basePath)) {
            Serial.printf("Created base directory: %s\n", basePath.c_str());
        } else {
            Serial.printf("ERROR: Failed to create base directory: %s\n", basePath.c_str());
            return false;
        }
    } else {
        Serial.printf("Base directory exists: %s\n", basePath.c_str());
    }
    
    // Load image counter from preferences
    preferences.begin("storage", false);
    imageCounter = preferences.getULong("imageCounter", 0);
    Serial.printf("Loaded image counter: %lu\n", imageCounter);
    
    initialized = true;
    Serial.println("Storage Manager initialized successfully");
    
    return true;
}

String StorageManager::getCurrentDatePath() {
    char datePath[20];
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        // Format as /YYYYMMDD
        snprintf(datePath, sizeof(datePath), "/%04d%02d%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday);
    } else {
        // Fallback to epoch time if RTC not available
        unsigned long days = millis() / (1000 * 60 * 60 * 24);
        snprintf(datePath, sizeof(datePath), "/day_%05lu", days);
    }
    
    String fullPath = basePath + String(datePath);
    
    // Create directory if it doesn't exist
    if (!SD_MMC.exists(fullPath)) {
        if (SD_MMC.mkdir(fullPath)) {
            Serial.printf("Created date directory: %s\n", fullPath.c_str());
        } else {
            Serial.printf("WARNING: Failed to create date directory: %s\n", fullPath.c_str());
        }
    }
    
    return String(datePath);
}

String StorageManager::generateFilename() {
    char filename[30];
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        // Format: IMG_HHMMSS_XXX.jpg
        snprintf(filename, sizeof(filename), "IMG_%02d%02d%02d_%03lu.jpg",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec,
                 imageCounter % 1000);
    } else {
        // Fallback to counter-based naming
        snprintf(filename, sizeof(filename), "IMG_%08lu.jpg", imageCounter);
    }
    
    return String(filename);
}

String StorageManager::saveImage(camera_fb_t* fb, const String& customPath) {
    // Validate frame buffer pointer
    if (!initialized) {
        Serial.println("ERROR: Storage not initialized");
        return "";
    }
    
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println("ERROR: Invalid frame buffer");
        return "";
    }
    
    String fullPath;
    
    if (customPath.length() > 0) {
        // Use custom path if provided
        fullPath = customPath;
    } else {
        // Generate filename using timestamp or counter
        String filename = generateFilename();
        
        // Get current date path (YYYYMMDD format)
        String datePath = getCurrentDatePath();
        
        // Construct full path: /images/YYYYMMDD/IMG_HHMMSS_XXX.jpg
        fullPath = basePath + datePath + "/" + filename;
    }
    
    // Ensure uniqueness by checking if file exists
    int attempt = 0;
    String originalPath = fullPath;
    while (SD_MMC.exists(fullPath) && attempt < 1000) {
        // Modify filename to ensure uniqueness
        // Find the last dot, but not if it's at the beginning (hidden file)
        int dotIndex = originalPath.lastIndexOf('.');
        if (dotIndex > 0) {  // Dot is not at the beginning
            String baseName = originalPath.substring(0, dotIndex);
            String extension = originalPath.substring(dotIndex);
            fullPath = baseName + "_" + String(attempt) + extension;
        } else {
            // No extension or dot file - append attempt number
            fullPath = originalPath + "_" + String(attempt);
        }
        attempt++;
    }
    
    // Open file for writing
    File file = SD_MMC.open(fullPath, FILE_WRITE);
    if (!file) {
        Serial.printf("ERROR: Failed to open file for writing: %s\n", fullPath.c_str());
        return "";
    }
    
    // Write frame buffer data
    size_t written = file.write(fb->buf, fb->len);
    
    // Close file
    file.close();
    
    if (written != fb->len) {
        Serial.printf("ERROR: Failed to write complete image (wrote %d of %d bytes)\n", written, fb->len);
        return "";
    }
    
    // Increment and save counter
    imageCounter++;
    preferences.putULong("imageCounter", imageCounter);
    
    // Print save confirmation with file size
    Serial.printf("SUCCESS: Image saved: %s (%d bytes)\n", fullPath.c_str(), fb->len);
    
    return fullPath;
}

bool StorageManager::saveMetadata(const String& imagePath, JsonDocument& metadata) {
    if (!initialized) {
        Serial.println("ERROR: Storage not initialized");
        return false;
    }
    
    if (imagePath.length() == 0) {
        Serial.println("ERROR: Invalid image path");
        return false;
    }
    
    // Extract directory and filename from image path
    // Create JSON file path (same name, .json extension)
    String jsonPath = imagePath;
    // Find the last dot, but not if it's at the beginning (hidden file)
    int dotIndex = jsonPath.lastIndexOf('.');
    if (dotIndex > 0) {  // Dot is not at the beginning
        jsonPath = jsonPath.substring(0, dotIndex) + ".json";
    } else {
        // No extension or dot file - append .json
        jsonPath += ".json";
    }
    
    // Open file for writing
    File file = SD_MMC.open(jsonPath, FILE_WRITE);
    if (!file) {
        Serial.printf("ERROR: Failed to open metadata file for writing: %s\n", jsonPath.c_str());
        return false;
    }
    
    // Serialize JSON document to file
    size_t bytesWritten = serializeJson(metadata, file);
    
    // Close file
    file.close();
    
    if (bytesWritten == 0) {
        Serial.printf("ERROR: Failed to write metadata to: %s\n", jsonPath.c_str());
        return false;
    }
    
    Serial.printf("SUCCESS: Metadata saved: %s (%d bytes)\n", jsonPath.c_str(), bytesWritten);
    
    return true;
}

bool StorageManager::deleteOldFiles(int daysToKeep) {
    if (!initialized) {
        Serial.println("ERROR: Storage not initialized");
        return false;
    }
    
    Serial.printf("Cleanup requested, keeping files from last %d days\n", daysToKeep);
    
    // This is a placeholder implementation
    // In a full implementation, we would:
    // 1. Calculate cutoff time: millis() - (daysToKeep * 24UL * 60 * 60 * 1000)
    // 2. Iterate through all date directories
    // 3. Parse directory names to get dates
    // 4. Delete directories older than cutoff
    // 5. Return success/failure based on operations
    
    Serial.println("Note: deleteOldFiles is a placeholder - full implementation requires date parsing");
    
    return true;
}

unsigned long StorageManager::getFreeSpace() {
    if (!initialized) {
        return 0;
    }
    
    uint64_t total = SD_MMC.totalBytes();
    uint64_t used = SD_MMC.usedBytes();
    uint64_t free = (total > used) ? (total - used) : 0;
    
    // Note: Casting to unsigned long may overflow for SD cards > 4GB
    // Consider using uint64_t in the header if accurate reporting is needed for large cards
    return (unsigned long)free;
}

unsigned long StorageManager::getUsedSpace() {
    if (!initialized) {
        return 0;
    }
    
    uint64_t used = SD_MMC.usedBytes();
    
    // Note: Casting to unsigned long may overflow for SD cards > 4GB
    // Consider using uint64_t in the header if accurate reporting is needed for large cards
    return (unsigned long)used;
}

void StorageManager::printStorageInfo() {
    if (!initialized) {
        Serial.println("Storage not initialized");
        return;
    }
    
    Serial.println("=== Storage Information ===");
    Serial.printf("Base Path: %s\n", basePath.c_str());
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("Image Counter: %lu\n", imageCounter);
    
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    
    Serial.printf("Total Space: %llu MB (%llu bytes)\n", 
                  totalBytes / (1024 * 1024), totalBytes);
    Serial.printf("Used Space: %llu MB (%llu bytes)\n", 
                  usedBytes / (1024 * 1024), usedBytes);
    Serial.printf("Free Space: %llu MB (%llu bytes)\n", 
                  freeBytes / (1024 * 1024), freeBytes);
    
    if (totalBytes > 0) {
        float usedPercent = (float)usedBytes / totalBytes * 100.0;
        Serial.printf("Usage: %.2f%%\n", usedPercent);
    }
    
    Serial.println("===========================");
}
