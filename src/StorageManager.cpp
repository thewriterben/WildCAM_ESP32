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

StorageManager::StorageManager() 
    : initialized(false), basePath("/images"), imageCounter(0) {
}

bool StorageManager::init() {
    Serial.println("Initializing Storage Manager...");
    
    // Initialize SD card in 1-bit mode (uses less pins: CLK, CMD, D0)
    // Second parameter 'true' enables 1-bit mode, reducing pin requirements
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("ERROR: SD Card Mount Failed");
        return false;
    }
    
    // Verify SD card is physically present and detected
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("ERROR: No SD Card attached");
        return false;
    }
    
    // Display card type and detailed information for diagnostics
    Serial.println("=== SD Card Information ===");
    Serial.print("Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");  // Standard Capacity SD Card (up to 2GB)
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");  // High Capacity SD Card (4GB to 32GB)
    } else {
        Serial.println("UNKNOWN");
    }
    
    // Display card capacity and usage statistics
    // Convert from bytes to megabytes for human readability
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("Card Size: %llu MB\n", cardSize);
    Serial.printf("Total Space: %llu MB\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used Space: %llu MB\n", SD_MMC.usedBytes() / (1024 * 1024));
    Serial.printf("Free Space: %llu MB\n", (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
    Serial.println("===========================");
    
    // Create base directory structure for organizing images
    // This is the root directory where all images will be stored
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
    
    // Load persistent image counter from non-volatile storage
    // This counter survives power cycles and device resets
    // "storage" is the namespace, "imageCounter" is the key
    preferences.begin("storage", false);  // false = read-write mode
    imageCounter = preferences.getULong("imageCounter", 0);  // 0 is default if key doesn't exist
    Serial.printf("Loaded image counter: %lu\n", imageCounter);
    
    initialized = true;
    Serial.println("Storage Manager initialized successfully");
    
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
        Serial.println("ERROR: Storage not initialized");
        return "";
    }
    
    // Validate frame buffer pointer and data integrity
    // Frame buffer must exist, have valid data pointer, and non-zero length
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println("ERROR: Invalid frame buffer");
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
        Serial.printf("ERROR: Failed to open file for writing: %s\n", fullPath.c_str());
        return "";
    }
    
    // Write complete frame buffer data to file
    // This is the actual JPEG image data from the camera
    size_t written = file.write(fb->buf, fb->len);
    
    // Close file to ensure data is flushed to SD card
    file.close();
    
    // Verify all bytes were written successfully
    if (written != fb->len) {
        Serial.printf("ERROR: Failed to write complete image (wrote %d of %d bytes)\n", written, fb->len);
        return "";
    }
    
    // Increment counter for next image and persist to non-volatile storage
    // This ensures counter survives power cycles and device resets
    imageCounter++;
    preferences.putULong("imageCounter", imageCounter);
    
    // Log successful save with path and file size for diagnostics
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
        Serial.printf("ERROR: Failed to open metadata file for writing: %s\n", jsonPath.c_str());
        return false;
    }
    
    // Serialize JSON document directly to file
    // This is more memory-efficient than serializing to string first
    size_t bytesWritten = serializeJson(metadata, file);
    
    // Close file to flush data to SD card
    file.close();
    
    // Verify JSON was actually written (should be at least a few bytes)
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
    // Full implementation would require:
    // 1. Calculate cutoff timestamp: current_time - (daysToKeep * 24 hours)
    // 2. Iterate through all date-based subdirectories (e.g., /images/20241029/)
    // 3. Parse directory names to extract dates (YYYYMMDD format)
    // 4. Compare directory dates against cutoff timestamp
    // 5. Recursively delete directories older than cutoff (including all files)
    // 6. Handle fallback directories (day_XXXXX format) based on creation time
    // 7. Return success if all old directories deleted, failure if any errors occurred
    
    Serial.println("Note: deleteOldFiles is a placeholder - full implementation requires date parsing");
    
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
        Serial.println("Storage not initialized");
        return;
    }
    
    // Display formatted storage information with clear section headers
    Serial.println("=== Storage Information ===");
    Serial.printf("Base Path: %s\n", basePath.c_str());
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("Image Counter: %lu\n", imageCounter);
    
    // Get raw byte values from SD card
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    
    // Display sizes in both megabytes (human-readable) and bytes (precise)
    Serial.printf("Total Space: %llu MB (%llu bytes)\n", 
                  totalBytes / (1024 * 1024), totalBytes);
    Serial.printf("Used Space: %llu MB (%llu bytes)\n", 
                  usedBytes / (1024 * 1024), usedBytes);
    Serial.printf("Free Space: %llu MB (%llu bytes)\n", 
                  freeBytes / (1024 * 1024), freeBytes);
    
    // Calculate and display usage percentage
    // Avoid division by zero if total is somehow 0
    if (totalBytes > 0) {
        float usedPercent = (float)usedBytes / totalBytes * 100.0;
        Serial.printf("Usage: %.2f%%\n", usedPercent);
    }
    
    Serial.println("===========================");
}

std::vector<String> StorageManager::getImageFiles() {
    std::vector<String> imageFiles;
    
    if (!initialized) {
        Serial.println("Storage not initialized");
        return imageFiles;
    }
    
    // Define recursive lambda function to scan directories
    // Lambda captures imageFiles vector by reference to accumulate results
    std::function<void(const String&)> scanDirectory = [&](const String& path) {
        File dir = SD_MMC.open(path);
        if (!dir || !dir.isDirectory()) {
            return;  // Skip if path is invalid or not a directory
        }
        
        // Iterate through all files and subdirectories in current directory
        File file = dir.openNextFile();
        while (file) {
            String fileName = String(file.name());
            
            if (file.isDirectory()) {
                // Recursively scan subdirectories to find all images
                // This handles the date-based directory structure (e.g., /images/20241029/)
                scanDirectory(fileName);
            } else {
                // Check if file has image extension (.jpg or .jpeg)
                // Case-insensitive check to handle different naming conventions
                if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") || 
                    fileName.endsWith(".JPG") || fileName.endsWith(".JPEG")) {
                    imageFiles.push_back(fileName);
                }
            }
            
            // Get next file in directory
            file = dir.openNextFile();
        }
        dir.close();
    };
    
    // Start recursive scan from base path (/images)
    scanDirectory(basePath);
    
    // Sort files in reverse alphabetical order
    // Since filenames include timestamps (IMG_HHMMSS_XXX.jpg or IMG_XXXXXXXX.jpg),
    // reverse alphabetical order = newest first (higher timestamps = later files)
    std::sort(imageFiles.begin(), imageFiles.end(), std::greater<String>());
    
    return imageFiles;
}

unsigned long StorageManager::getImageCount() {
    if (!initialized) {
        return 0;
    }
    return imageCounter;
}
