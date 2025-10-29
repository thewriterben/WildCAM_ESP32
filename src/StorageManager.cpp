#include "StorageManager.h"
#include "config.h"
#include <FS.h>
#include <Preferences.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <functional>

// Store constant strings in PROGMEM to save RAM
static const char TAG_INIT[] PROGMEM = "Initializing Storage Manager...";
static const char TAG_ERROR_MOUNT[] PROGMEM = "ERROR: SD Card Mount Failed";
static const char TAG_ERROR_NO_CARD[] PROGMEM = "ERROR: No SD Card attached";
static const char TAG_CARD_INFO[] PROGMEM = "=== SD Card Information ===";
static const char TAG_CARD_TYPE[] PROGMEM = "Card Type: ";
static const char TAG_CARD_MMC[] PROGMEM = "MMC";
static const char TAG_CARD_SD[] PROGMEM = "SDSC";
static const char TAG_CARD_SDHC[] PROGMEM = "SDHC";
static const char TAG_CARD_UNKNOWN[] PROGMEM = "UNKNOWN";
static const char TAG_CARD_SIZE[] PROGMEM = "Card Size: %llu MB\n";
static const char TAG_TOTAL_SPACE[] PROGMEM = "Total Space: %llu MB\n";
static const char TAG_USED_SPACE[] PROGMEM = "Used Space: %llu MB\n";
static const char TAG_FREE_SPACE[] PROGMEM = "Free Space: %llu MB\n";
static const char TAG_SEPARATOR[] PROGMEM = "===========================";
static const char TAG_CREATED_DIR[] PROGMEM = "Created base directory: %s\n";
static const char TAG_ERROR_CREATE_DIR[] PROGMEM = "ERROR: Failed to create base directory: %s\n";
static const char TAG_DIR_EXISTS[] PROGMEM = "Base directory exists: %s\n";
static const char TAG_IMAGE_COUNTER[] PROGMEM = "Loaded image counter: %lu\n";
static const char TAG_SUCCESS_INIT[] PROGMEM = "Storage Manager initialized successfully";
static const char TAG_ERROR_NOT_INIT[] PROGMEM = "ERROR: Storage not initialized";
static const char TAG_ERROR_INVALID_FB[] PROGMEM = "ERROR: Invalid frame buffer";
static const char TAG_ERROR_OPEN_FILE[] PROGMEM = "ERROR: Failed to open file for writing: %s\n";
static const char TAG_ERROR_WRITE[] PROGMEM = "ERROR: Failed to write complete image (wrote %d of %d bytes)\n";
static const char TAG_SUCCESS_SAVE[] PROGMEM = "SUCCESS: Image saved: %s (%d bytes)\n";
static const char TAG_ERROR_INVALID_PATH[] PROGMEM = "ERROR: Invalid image path";
static const char TAG_ERROR_OPEN_META[] PROGMEM = "ERROR: Failed to open metadata file for writing: %s\n";
static const char TAG_ERROR_WRITE_META[] PROGMEM = "ERROR: Failed to write metadata to: %s\n";
static const char TAG_SUCCESS_META[] PROGMEM = "SUCCESS: Metadata saved: %s (%d bytes)\n";
static const char TAG_CLEANUP[] PROGMEM = "Cleanup requested, keeping files from last %d days\n";
static const char TAG_CLEANUP_NOTE[] PROGMEM = "Note: deleteOldFiles is a placeholder - full implementation requires date parsing";
static const char TAG_NOT_INIT[] PROGMEM = "Storage not initialized";
static const char TAG_STORAGE_INFO[] PROGMEM = "=== Storage Information ===";
static const char TAG_BASE_PATH[] PROGMEM = "Base Path: %s\n";
static const char TAG_INITIALIZED[] PROGMEM = "Initialized: %s\n";
static const char TAG_IMG_COUNT[] PROGMEM = "Image Counter: %lu\n";
static const char TAG_TOTAL_BYTES[] PROGMEM = "Total Space: %llu MB (%llu bytes)\n";
static const char TAG_USED_BYTES[] PROGMEM = "Used Space: %llu MB (%llu bytes)\n";
static const char TAG_FREE_BYTES[] PROGMEM = "Free Space: %llu MB (%llu bytes)\n";
static const char TAG_USAGE[] PROGMEM = "Usage: %.2f%%\n";
static const char TAG_CREATED_DATE_DIR[] PROGMEM = "Created date directory: %s\n";
static const char TAG_WARN_CREATE_DATE_DIR[] PROGMEM = "WARNING: Failed to create date directory: %s\n";

StorageManager::StorageManager() 
    : initialized(false), basePath("/images"), imageCounter(0) {
}

bool StorageManager::init() {
    Serial.println(FPSTR(TAG_INIT));
    
    // Initialize SD card in 1-bit mode (uses less pins)
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println(FPSTR(TAG_ERROR_MOUNT));
        return false;
    }
    
    // Check if SD card is mounted
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(FPSTR(TAG_ERROR_NO_CARD));
        return false;
    }
    
    // Get card type and print detailed card information
    Serial.println(FPSTR(TAG_CARD_INFO));
    Serial.print(FPSTR(TAG_CARD_TYPE));
    if (cardType == CARD_MMC) {
        Serial.println(FPSTR(TAG_CARD_MMC));
    } else if (cardType == CARD_SD) {
        Serial.println(FPSTR(TAG_CARD_SD));
    } else if (cardType == CARD_SDHC) {
        Serial.println(FPSTR(TAG_CARD_SDHC));
    } else {
        Serial.println(FPSTR(TAG_CARD_UNKNOWN));
    }
    
    // Get card size
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf_P(TAG_CARD_SIZE, cardSize);
    Serial.printf_P(TAG_TOTAL_SPACE, SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf_P(TAG_USED_SPACE, SD_MMC.usedBytes() / (1024 * 1024));
    Serial.printf_P(TAG_FREE_SPACE, (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
    Serial.println(FPSTR(TAG_SEPARATOR));
    
    // Create base directory if it doesn't exist
    if (!SD_MMC.exists(basePath)) {
        if (SD_MMC.mkdir(basePath)) {
            Serial.printf_P(TAG_CREATED_DIR, basePath.c_str());
        } else {
            Serial.printf_P(TAG_ERROR_CREATE_DIR, basePath.c_str());
            return false;
        }
    } else {
        Serial.printf_P(TAG_DIR_EXISTS, basePath.c_str());
    }
    
    // Load image counter from preferences
    preferences.begin("storage", false);
    imageCounter = preferences.getULong("imageCounter", 0);
    Serial.printf_P(TAG_IMAGE_COUNTER, imageCounter);
    
    initialized = true;
    Serial.println(FPSTR(TAG_SUCCESS_INIT));
    
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
            Serial.printf_P(TAG_CREATED_DATE_DIR, fullPath.c_str());
        } else {
            Serial.printf_P(TAG_WARN_CREATE_DATE_DIR, fullPath.c_str());
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
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return "";
    }
    
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println(FPSTR(TAG_ERROR_INVALID_FB));
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
        Serial.printf_P(TAG_ERROR_OPEN_FILE, fullPath.c_str());
        return "";
    }
    
    // Write frame buffer data
    size_t written = file.write(fb->buf, fb->len);
    
    // Close file
    file.close();
    
    if (written != fb->len) {
        Serial.printf_P(TAG_ERROR_WRITE, written, fb->len);
        return "";
    }
    
    // Increment and save counter
    imageCounter++;
    preferences.putULong("imageCounter", imageCounter);
    
    // Print save confirmation with file size
    Serial.printf_P(TAG_SUCCESS_SAVE, fullPath.c_str(), fb->len);
    
    return fullPath;
}

bool StorageManager::saveMetadata(const String& imagePath, JsonDocument& metadata) {
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return false;
    }
    
    if (imagePath.length() == 0) {
        Serial.println(FPSTR(TAG_ERROR_INVALID_PATH));
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
        Serial.printf_P(TAG_ERROR_OPEN_META, jsonPath.c_str());
        return false;
    }
    
    // Serialize JSON document to file
    size_t bytesWritten = serializeJson(metadata, file);
    
    // Close file
    file.close();
    
    if (bytesWritten == 0) {
        Serial.printf_P(TAG_ERROR_WRITE_META, jsonPath.c_str());
        return false;
    }
    
    Serial.printf_P(TAG_SUCCESS_META, jsonPath.c_str(), bytesWritten);
    
    return true;
}

bool StorageManager::deleteOldFiles(int daysToKeep) {
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return false;
    }
    
    Serial.printf_P(TAG_CLEANUP, daysToKeep);
    
    // This is a placeholder implementation
    // In a full implementation, we would:
    // 1. Calculate cutoff time: millis() - (daysToKeep * 24UL * 60 * 60 * 1000)
    // 2. Iterate through all date directories
    // 3. Parse directory names to get dates
    // 4. Delete directories older than cutoff
    // 5. Return success/failure based on operations
    
    Serial.println(FPSTR(TAG_CLEANUP_NOTE));
    
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
        Serial.println(FPSTR(TAG_NOT_INIT));
        return;
    }
    
    Serial.println(FPSTR(TAG_STORAGE_INFO));
    Serial.printf_P(TAG_BASE_PATH, basePath.c_str());
    Serial.printf_P(TAG_INITIALIZED, initialized ? "Yes" : "No");
    Serial.printf_P(TAG_IMG_COUNT, imageCounter);
    
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    
    Serial.printf_P(TAG_TOTAL_BYTES, totalBytes / (1024 * 1024), totalBytes);
    Serial.printf_P(TAG_USED_BYTES, usedBytes / (1024 * 1024), usedBytes);
    Serial.printf_P(TAG_FREE_BYTES, freeBytes / (1024 * 1024), freeBytes);
    
    if (totalBytes > 0) {
        float usedPercent = (float)usedBytes / totalBytes * 100.0;
        Serial.printf_P(TAG_USAGE, usedPercent);
    }
    
    Serial.println(FPSTR(TAG_SEPARATOR));
}

std::vector<String> StorageManager::getImageFiles() {
    std::vector<String> imageFiles;
    
    if (!initialized) {
        Serial.println(FPSTR(TAG_NOT_INIT));
        return imageFiles;
    }
    
    // Helper function to recursively scan directories
    // Use reference capture for imageFiles to avoid copies
    std::function<void(const String&)> scanDirectory = [&imageFiles](const String& path) {
        File dir = SD_MMC.open(path);
        if (!dir || !dir.isDirectory()) {
            return;
        }
        
        File file = dir.openNextFile();
        while (file) {
            String fileName = String(file.name());
            
            if (file.isDirectory()) {
                // Recursively scan subdirectories
                scanDirectory(fileName);
            } else {
                // Check if file has .jpg or .jpeg extension
                if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") || 
                    fileName.endsWith(".JPG") || fileName.endsWith(".JPEG")) {
                    imageFiles.push_back(fileName);
                }
            }
            
            file = dir.openNextFile();
        }
        dir.close();
    };
    
    // Start scanning from base path
    scanDirectory(basePath);
    
    // Sort files in reverse order (newest first by filename)
    std::sort(imageFiles.begin(), imageFiles.end(), std::greater<String>());
    
    return imageFiles;
}

unsigned long StorageManager::getImageCount() {
    if (!initialized) {
        return 0;
    }
    return imageCounter;
}
