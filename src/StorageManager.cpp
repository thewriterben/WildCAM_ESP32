/**
 * @file StorageManager.cpp
 * @brief Implementation of SD card storage management for ESP32-CAM
 * 
 * This file implements the StorageManager class functionality including
 * SD card initialization, image and metadata storage, file management,
 * and storage monitoring operations for the wildlife camera system.
 * 
 * @author WildCAM Project
 * @date 2024
 */

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
    
    // Initialize SD card in 1-bit mode (uses less pins: CLK, CMD, D0)
    // Second parameter 'true' enables 1-bit mode, reducing pin requirements
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println(FPSTR(TAG_ERROR_MOUNT));
        return false;
    }
    
    // Verify SD card is physically present and detected
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(FPSTR(TAG_ERROR_NO_CARD));
        return false;
    }
    

    if (cardType == CARD_MMC) {
        Serial.println(FPSTR(TAG_CARD_MMC));
    } else if (cardType == CARD_SD) {

    } else {
        Serial.println(FPSTR(TAG_CARD_UNKNOWN));
    }
    
    // Display card capacity and usage statistics
    // Convert from bytes to megabytes for human readability
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf_P(TAG_CARD_SIZE, cardSize);
    Serial.printf_P(TAG_TOTAL_SPACE, SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf_P(TAG_USED_SPACE, SD_MMC.usedBytes() / (1024 * 1024));
    Serial.printf_P(TAG_FREE_SPACE, (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
    Serial.println(FPSTR(TAG_SEPARATOR));
    
    // Create base directory structure for organizing images
    // This is the root directory where all images will be stored
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
    

    
    initialized = true;
    Serial.println(FPSTR(TAG_SUCCESS_INIT));
    
    return true;
}

String StorageManager::getCurrentDatePath() {
    char datePath[20];
    struct tm timeinfo;
    
    // Attempt to get current time from RTC (Real-Time Clock)
    if (getLocalTime(&timeinfo)) {
        // Format date as /YYYYMMDD for daily organization
        // Year: tm_year is years since 1900, so add 1900 to get actual year
        // Month: tm_mon is 0-11, so add 1 to get 1-12
        snprintf(datePath, sizeof(datePath), "/%04d%02d%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday);
    } else {
        // RTC not configured or time not available
        // Fallback to uptime-based day counter
        // millis() returns milliseconds since boot
        // Divide by milliseconds per day: 1000 ms/s * 60 s/min * 60 min/hr * 24 hr/day = 86,400,000
        unsigned long days = millis() / (1000 * 60 * 60 * 24);
        snprintf(datePath, sizeof(datePath), "/day_%05lu", days);
    }
    
    String fullPath = basePath + String(datePath);
    
    // Ensure the date directory exists before returning
    // This allows images to be saved immediately without explicit directory creation
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
    
    // Try to use current time for meaningful filenames
    if (getLocalTime(&timeinfo)) {
        // Format: IMG_HHMMSS_XXX.jpg where:
        // - HH = hour (00-23)
        // - MM = minute (00-59)
        // - SS = second (00-59)
        // - XXX = last 3 digits of counter (000-999) for uniqueness within same second
        snprintf(filename, sizeof(filename), "IMG_%02d%02d%02d_%03lu.jpg",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec,
                 imageCounter % 1000);  // Modulo ensures we stay within 3 digits
    } else {
        // No time available - use pure counter-based naming
        // Format: IMG_XXXXXXXX.jpg where XXXXXXXX is 8-digit counter (zero-padded)
        snprintf(filename, sizeof(filename), "IMG_%08lu.jpg", imageCounter);
    }
    
    return String(filename);
}

String StorageManager::saveImage(camera_fb_t* fb, const String& customPath) {
    // Validate initialization state
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return "";
    }
    
    // Validate frame buffer pointer and data integrity
    // Frame buffer must exist, have valid data pointer, and non-zero length
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println(FPSTR(TAG_ERROR_INVALID_FB));
        return "";
    }
    
    String fullPath;
    
    if (customPath.length() > 0) {
        // Use custom path if provided by caller
        // Allows for special naming or organization schemes
        fullPath = customPath;
    } else {
        // Generate standard filename using timestamp or counter
        String filename = generateFilename();
        
        // Get current date path for daily organization (e.g., /20241029)
        String datePath = getCurrentDatePath();
        
        // Construct full path: /images/YYYYMMDD/IMG_HHMMSS_XXX.jpg
        fullPath = basePath + datePath + "/" + filename;
    }
    
    // Handle filename collisions by appending a suffix counter
    // This ensures we never overwrite existing files
    // Maximum 1000 attempts to find unique filename
    int attempt = 0;
    String originalPath = fullPath;
    while (SD_MMC.exists(fullPath) && attempt < 1000) {
        // Modify filename to ensure uniqueness
        // Split filename at last dot to preserve extension
        int dotIndex = originalPath.lastIndexOf('.');
        if (dotIndex > 0) {  // Dot is not at the beginning (not a hidden file)
            String baseName = originalPath.substring(0, dotIndex);
            String extension = originalPath.substring(dotIndex);
            fullPath = baseName + "_" + String(attempt) + extension;
        } else {
            // No extension found or dot file - append attempt number directly
            fullPath = originalPath + "_" + String(attempt);
        }
        attempt++;
    }
    
    // Open file for writing in binary mode
    File file = SD_MMC.open(fullPath, FILE_WRITE);
    if (!file) {
        Serial.printf_P(TAG_ERROR_OPEN_FILE, fullPath.c_str());
        return "";
    }
    
    // Write complete frame buffer data to file
    // This is the actual JPEG image data from the camera
    size_t written = file.write(fb->buf, fb->len);
    
    // Close file to ensure data is flushed to SD card
    file.close();
    
    // Verify all bytes were written successfully
    if (written != fb->len) {
        Serial.printf_P(TAG_ERROR_WRITE, written, fb->len);
        return "";
    }
    
    // Increment counter for next image and persist to non-volatile storage
    // This ensures counter survives power cycles and device resets
    imageCounter++;
    preferences.putULong("imageCounter", imageCounter);
    
    
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
    
    // Create JSON file path with same base name but .json extension
    // Example: /images/20241029/IMG_143052_001.jpg → IMG_143052_001.json
    String jsonPath = imagePath;
    
    // Find the last dot to locate the file extension
    int dotIndex = jsonPath.lastIndexOf('.');
    if (dotIndex > 0) {  // Dot is not at the beginning (not a hidden file)
        // Replace extension with .json
        jsonPath = jsonPath.substring(0, dotIndex) + ".json";
    } else {
        // No extension found or dot file - append .json
        jsonPath += ".json";
    }
    
    // Open JSON file for writing
    File file = SD_MMC.open(jsonPath, FILE_WRITE);
    if (!file) {
        Serial.printf_P(TAG_ERROR_OPEN_META, jsonPath.c_str());
        return false;
    }
    
    // Serialize JSON document directly to file
    // This is more memory-efficient than serializing to string first
    size_t bytesWritten = serializeJson(metadata, file);
    
    // Close file to flush data to SD card
    file.close();
    
    // Verify JSON was actually written (should be at least a few bytes)
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
    // Full implementation would require:
    // 1. Calculate cutoff timestamp: current_time - (daysToKeep * 24 hours)
    // 2. Iterate through all date-based subdirectories (e.g., /images/20241029/)
    // 3. Parse directory names to extract dates (YYYYMMDD format)
    // 4. Compare directory dates against cutoff timestamp
    // 5. Recursively delete directories older than cutoff (including all files)
    // 6. Handle fallback directories (day_XXXXX format) based on creation time
    // 7. Return success if all old directories deleted, failure if any errors occurred
    
    Serial.println(FPSTR(TAG_CLEANUP_NOTE));
    
    return true;
}

unsigned long StorageManager::getFreeSpace() {
    if (!initialized) {
        return 0;
    }
    
    // Get storage statistics from SD card
    uint64_t total = SD_MMC.totalBytes();
    uint64_t used = SD_MMC.usedBytes();
    
    // Calculate free space, ensuring no underflow if used > total (shouldn't happen)
    uint64_t free = (total > used) ? (total - used) : 0;
    
    // Note: Casting from uint64_t to unsigned long (32-bit on ESP32)
    // This may overflow for SD cards larger than 4GB (4,294,967,295 bytes)
    // Consider changing return type to uint64_t in header for accurate large card support
    return (unsigned long)free;
}

unsigned long StorageManager::getUsedSpace() {
    if (!initialized) {
        return 0;
    }
    
    // Get used bytes directly from SD card filesystem
    uint64_t used = SD_MMC.usedBytes();
    
    // Note: Casting from uint64_t to unsigned long (32-bit on ESP32)
    // This may overflow for SD cards with >4GB of used space
    // Consider changing return type to uint64_t in header for accurate large card support
    return (unsigned long)used;
}

void StorageManager::printStorageInfo() {
    if (!initialized) {
        Serial.println(FPSTR(TAG_NOT_INIT));
        return;
    }
    

    
    // Get raw byte values from SD card
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    

    
    // Calculate and display usage percentage
    // Avoid division by zero if total is somehow 0
    if (totalBytes > 0) {
        float usedPercent = (float)usedBytes / totalBytes * 100.0;
        Serial.printf_P(TAG_USAGE, usedPercent);
    }
    
    Serial.println(FPSTR(TAG_SEPARATOR));
}

std::vector<String> StorageManager::getImageFiles() {
    std::vector<String> imageFiles;
    
    // 1. Check initialization status (pointer/state validation)
    if (!initialized) {

        File file = dir.openNextFile();
        int fileCount = 0;
        
        while (file) {
            // 10. Sanity check for infinite loops
            fileCount++;
            if (fileCount > MAX_FILES_PER_DIR) {
                Serial.printf("WARNING: Directory contains more than %d files: %s\n", 
                             MAX_FILES_PER_DIR, path.c_str());
                Serial.println("RECOVERY: Stopping scan of this directory to prevent excessive processing");
                break;
            }
            
            // 11. Validate file name
            const char* rawName = file.name();
            if (rawName == nullptr) {
                Serial.println("WARNING: Encountered file with null name - skipping");
                file = dir.openNextFile();
                continue;
            }
            
            String fileName = String(rawName);
            
            // 12. Validate filename is not empty
            if (fileName.length() == 0) {
                Serial.println("WARNING: Encountered file with empty name - skipping");
                file = dir.openNextFile();
                continue;
            }
            
            // 13. Process subdirectories recursively
            if (file.isDirectory()) {

                if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") || 
                    fileName.endsWith(".JPG") || fileName.endsWith(".JPEG")) {
                    
                    // 15. Additional validation - check file size is reasonable
                    size_t fileSize = file.size();
                    
                    if (fileSize > MAX_IMAGE_SIZE) {
                        Serial.printf("WARNING: Image file too large (%d bytes): %s\n", 
                                    fileSize, fileName.c_str());
                        Serial.println("RECOVERY: Skipping this file - may be corrupted");
                    } else if (fileSize < MIN_IMAGE_SIZE) {
                        Serial.printf("WARNING: Image file too small (%d bytes): %s\n", 
                                    fileSize, fileName.c_str());
                        Serial.println("RECOVERY: Skipping this file - may be corrupted or empty");
                    } else {
                        // File appears valid - add to list
                        imageFiles.push_back(fileName);
                        totalFilesFound++;
                    }
                }
            }
            

        }
        
        // 17. Close directory handle to free resources
        dir.close();
    };
    

    
    return imageFiles;
}

unsigned long StorageManager::getImageCount() {
    if (!initialized) {
        return 0;
    }
    return imageCounter;
}
