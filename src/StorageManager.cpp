#include "StorageManager.h"
#include "config.h"
#include <FS.h>

StorageManager::StorageManager() 
    : sdInitialized(false), baseDirectory("/wildcam"), imageCounter(0) {
}

bool StorageManager::begin() {
    // Initialize SD card in 1-bit mode (uses less pins)
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD Card attached");
        return false;
    }
    
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    
    // Create base directory if it doesn't exist
    if (!SD_MMC.exists(baseDirectory)) {
        SD_MMC.mkdir(baseDirectory);
        Serial.printf("Created directory: %s\n", baseDirectory.c_str());
    }
    
    sdInitialized = true;
    Serial.printf("SD Card Size: %llu MB\n", SD_MMC.cardSize() / (1024 * 1024));
    
    return true;
}

String StorageManager::generateFilename() {
    char filename[50];
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        // Format: YYYYMMDD_HHMMSS.jpg
        snprintf(filename, sizeof(filename), "%s/%04d%02d%02d_%02d%02d%02d.jpg",
                 baseDirectory.c_str(),
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
    } else {
        // Fallback to counter-based naming
        snprintf(filename, sizeof(filename), "%s/IMG_%05lu.jpg",
                 baseDirectory.c_str(), imageCounter++);
    }
    
    return String(filename);
}

bool StorageManager::saveImage(camera_fb_t* fb, String& filepath) {
    if (!sdInitialized || !fb) {
        return false;
    }
    
    filepath = generateFilename();
    return saveImage(fb, filepath, filepath);
}

bool StorageManager::saveImage(camera_fb_t* fb, const String& filename, String& filepath) {
    if (!sdInitialized || !fb) {
        return false;
    }
    
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written == fb->len) {
        filepath = filename;
        Serial.printf("Image saved: %s (%d bytes)\n", filename.c_str(), fb->len);
        return true;
    } else {
        Serial.println("Failed to write complete image");
        return false;
    }
}

uint64_t StorageManager::getAvailableSpace() {
    if (!sdInitialized) {
        return 0;
    }
    return (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024);
}

uint64_t StorageManager::getTotalSpace() {
    if (!sdInitialized) {
        return 0;
    }
    return SD_MMC.totalBytes() / (1024 * 1024);
}

int StorageManager::getImageCount() {
    if (!sdInitialized) {
        return 0;
    }
    
    File root = SD_MMC.open(baseDirectory);
    if (!root) {
        return 0;
    }
    
    int count = 0;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (filename.endsWith(".jpg") || filename.endsWith(".JPG")) {
                count++;
            }
        }
        file = root.openNextFile();
    }
    
    return count;
}

bool StorageManager::cleanupOldImages(int keepCount) {
    // Implementation for deleting old images
    // This is a placeholder - would need to sort by date and delete oldest
    Serial.printf("Cleanup requested, keeping %d images\n", keepCount);
    return true;
}

bool StorageManager::isReady() {
    return sdInitialized;
}

void StorageManager::listFiles(const String& dirname) {
    File root = SD_MMC.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }
    
    Serial.printf("Listing directory: %s\n", dirname.c_str());
    
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s\n", file.name());
        } else {
            Serial.printf("  FILE: %s\tSIZE: %d\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}
