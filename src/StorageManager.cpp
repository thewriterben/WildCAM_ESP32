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
static const char TAG_DUPLICATE_DETECTED[] PROGMEM = "INFO: Duplicate image detected - skipping save\n";
static const char TAG_SMART_DELETE[] PROGMEM = "Smart delete: removed %d files, freed %lu KB\n";

// Maximum files to scan per directory to prevent excessive processing
static const int MAX_FILES_PER_DIR = 1000;
// Minimum valid image size in bytes (rough threshold for valid JPEG)
static const size_t MIN_IMAGE_SIZE = 1024;

// Statistics tracking (session-based, resets on boot)
static unsigned long g_duplicatesSkipped = 0;
static unsigned long g_bytesCompressed = 0;

// PROGMEM strings for error handling
static const char TAG_RETRY[] PROGMEM = "SD operation retry %d/%d after %dms delay\n";
static const char TAG_REMOUNT_ATTEMPT[] PROGMEM = "Attempting SD card remount...\n";
static const char TAG_REMOUNT_SUCCESS[] PROGMEM = "SD card remount successful\n";
static const char TAG_REMOUNT_FAIL[] PROGMEM = "SD card remount failed\n";
static const char TAG_HEALTH_CHECK[] PROGMEM = "SD card health check: %s\n";
static const char TAG_LOW_MEMORY[] PROGMEM = "WARNING: Low memory detected (%d bytes free)\n";
static const char TAG_MEMORY_OPTIMIZED[] PROGMEM = "Memory optimized, freed ~%d bytes\n";

StorageManager::StorageManager() 
    : initialized(false), 
      basePath("/images"), 
      imageCounter(0),
      compressionQuality(STORAGE_DEFAULT_COMPRESSION_QUALITY),
      compressionEnabled(true),
      duplicateDetectionEnabled(true),
      lastError(SD_ERROR_NONE),
      maxRetries(SD_CARD_MAX_RETRIES),
      retryDelayMs(SD_CARD_RETRY_DELAY_MS),
      maxRetryDelayMs(SD_CARD_MAX_RETRY_DELAY_MS),
      autoRemountEnabled(SD_CARD_AUTO_REMOUNT),
      writeBuffer(nullptr),
      writeBufferSize(SD_WRITE_BUFFER_SIZE),
      memoryPoolEnabled(true) {
    // Initialize card health structure
    cardHealth.mounted = false;
    cardHealth.cardType = CARD_NONE;
    cardHealth.totalBytes = 0;
    cardHealth.usedBytes = 0;
    cardHealth.freeBytes = 0;
    cardHealth.lastHealthCheck = 0;
    cardHealth.consecutiveErrors = 0;
    cardHealth.totalErrors = 0;
    cardHealth.successfulOps = 0;
    cardHealth.errorRate = 0.0f;
}

StorageManager::~StorageManager() {
    freeWriteBuffer();
}

bool StorageManager::init() {
    Serial.println(FPSTR(TAG_INIT));
    
    // Initialize SD card with retry logic
    bool mountSuccess = false;
    unsigned int currentDelay = retryDelayMs;
    
    for (unsigned int attempt = 0; attempt <= maxRetries && !mountSuccess; attempt++) {
        if (attempt > 0) {
            Serial.printf_P(TAG_RETRY, attempt, maxRetries, currentDelay);
            delay(currentDelay);
            // Exponential backoff
            currentDelay = min(currentDelay * 2, maxRetryDelayMs);
        }
        
        // Initialize SD card in 1-bit mode (uses less pins: CLK, CMD, D0)
        // Second parameter 'true' enables 1-bit mode, reducing pin requirements
        if (SD_MMC.begin("/sdcard", true)) {
            mountSuccess = true;
        }
    }
    
    if (!mountSuccess) {
        Serial.println(FPSTR(TAG_ERROR_MOUNT));
        lastError = SD_ERROR_MOUNT_FAILED;
        lastErrorMessage = "SD card mount failed after retries";
        updateErrorStats(false, SD_ERROR_MOUNT_FAILED);
        return false;
    }
    
    // Verify SD card is physically present and detected
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(FPSTR(TAG_ERROR_NO_CARD));
        lastError = SD_ERROR_CARD_REMOVED;
        lastErrorMessage = "No SD card detected";
        updateErrorStats(false, SD_ERROR_CARD_REMOVED);
        return false;
    }
    
    // Print card type information
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
    
    // Display card capacity and usage statistics
    // Convert from bytes to megabytes for human readability
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf_P(TAG_CARD_SIZE, cardSize);
    Serial.printf_P(TAG_TOTAL_SPACE, SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf_P(TAG_USED_SPACE, SD_MMC.usedBytes() / (1024 * 1024));
    Serial.printf_P(TAG_FREE_SPACE, (SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024));
    Serial.println(FPSTR(TAG_SEPARATOR));
    
    // Initialize SD card health statistics
    cardHealth.mounted = true;
    cardHealth.cardType = cardType;
    cardHealth.totalBytes = SD_MMC.totalBytes();
    cardHealth.usedBytes = SD_MMC.usedBytes();
    cardHealth.freeBytes = cardHealth.totalBytes - cardHealth.usedBytes;
    cardHealth.lastHealthCheck = millis();
    
    // Create base directory structure for organizing images
    // This is the root directory where all images will be stored
    if (!SD_MMC.exists(basePath)) {
        if (SD_MMC.mkdir(basePath)) {
            Serial.printf_P(TAG_CREATED_DIR, basePath.c_str());
        } else {
            Serial.printf_P(TAG_ERROR_CREATE_DIR, basePath.c_str());
            lastError = SD_ERROR_DIR_CREATE;
            lastErrorMessage = "Failed to create base directory";
            updateErrorStats(false, SD_ERROR_DIR_CREATE);
            return false;
        }
    } else {
        Serial.printf_P(TAG_DIR_EXISTS, basePath.c_str());
    }
    
    // Load persistent image counter from preferences
    preferences.begin("storage", false);
    imageCounter = preferences.getULong("imageCounter", 0);
    Serial.printf_P(TAG_IMAGE_COUNTER, imageCounter);
    
    // Initialize memory pool if enabled
    if (memoryPoolEnabled) {
        allocateWriteBuffer();
    }
    
    initialized = true;
    updateErrorStats(true);
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
        unsigned long days = millis() / (1000UL * 60 * 60 * 24);
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

uint32_t StorageManager::calculateImageHash(const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 0;
    }
    
    // Use a simple but effective sampling-based hash
    // Sample ~64 bytes spread across the image for fast comparison
    uint32_t hash = 5381;  // DJB2 hash initial value
    
    // Always include first and last bytes
    hash = ((hash << 5) + hash) + data[0];
    hash = ((hash << 5) + hash) + data[length - 1];
    
    // Sample at regular intervals across the data
    size_t sampleCount = 62;  // Plus first and last = 64 samples
    size_t step = (length > sampleCount) ? (length / sampleCount) : 1;
    
    for (size_t i = step; i < length - 1; i += step) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    // Include length in hash to differentiate same-content different-size images
    hash = ((hash << 5) + hash) + (uint32_t)(length & 0xFFFFFFFF);
    
    return hash;
}

bool StorageManager::isDuplicateImage(uint32_t hash) {
    if (!duplicateDetectionEnabled || hash == 0) {
        return false;
    }
    
    // Check if this hash exists in recent cache
    auto it = recentImageHashes.find(hash);
    if (it != recentImageHashes.end()) {
        return true;  // Duplicate found
    }
    
    return false;
}

float StorageManager::calculateQualityScore(const uint8_t* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 0.0f;
    }
    
    float score = 0.0f;
    
    // Factor 1: File size (larger files generally indicate better quality)
    // Score 0-40 points based on size
    if (length > 100000) {
        score += 40.0f;  // Large image - high quality
    } else if (length > 50000) {
        score += 30.0f;  // Medium-large
    } else if (length > 20000) {
        score += 20.0f;  // Medium
    } else if (length > 10000) {
        score += 10.0f;  // Small
    } else {
        score += 5.0f;   // Very small - possibly low quality
    }
    
    // Factor 2: JPEG structure validation
    // Check for valid JPEG markers (SOI and EOI)
    bool validJpeg = false;
    if (length >= 4) {
        // Check Start Of Image marker (FFD8)
        if (data[0] == 0xFF && data[1] == 0xD8) {
            // Check End Of Image marker (FFD9) at end
            if (data[length - 2] == 0xFF && data[length - 1] == 0xD9) {
                validJpeg = true;
                score += 20.0f;  // Valid JPEG structure
            }
        }
    }
    
    // Factor 3: Estimate image complexity/detail by sampling variance
    // Higher variance suggests more detail in the image
    if (length > 1000 && validJpeg) {
        // Sample 100 bytes from the middle of the image
        size_t sampleStart = length / 3;
        size_t sampleEnd = (2 * length) / 3;
        size_t sampleSize = 100;
        size_t step = (sampleEnd - sampleStart) / sampleSize;
        
        if (step > 0) {
            uint32_t sum = 0;
            uint32_t sumSquares = 0;
            int count = 0;
            
            for (size_t i = sampleStart; i < sampleEnd && count < 100; i += step) {
                sum += data[i];
                sumSquares += (uint32_t)data[i] * data[i];
                count++;
            }
            
            if (count > 0) {
                float mean = (float)sum / count;
                float variance = ((float)sumSquares / count) - (mean * mean);
                
                // Higher variance = more detail = better quality
                // Normalize to 0-40 range
                float varianceScore = (variance / 1000.0f) * 40.0f;
                if (varianceScore > 40.0f) varianceScore = 40.0f;
                score += varianceScore;
            }
        }
    }
    
    // Clamp score to 0-100 range
    if (score > 100.0f) score = 100.0f;
    if (score < 0.0f) score = 0.0f;
    
    return score;
}

void StorageManager::collectImageInfo(const String& dirPath, std::vector<ImageQualityInfo>& images) {
    File dir = SD_MMC.open(dirPath);
    if (!dir || !dir.isDirectory()) {
        return;
    }
    
    File file = dir.openNextFile();
    int fileCount = 0;
    
    while (file && fileCount < MAX_FILES_PER_DIR) {
        fileCount++;
        
        String fileName = String(file.name());
        if (fileName.length() == 0) {
            file = dir.openNextFile();
            continue;
        }
        
        // Handle subdirectories recursively
        if (file.isDirectory()) {
            String subDirPath = dirPath + "/" + fileName;
            collectImageInfo(subDirPath, images);
        } else {
            // Check if this is an image file
            String lowerName = fileName;
            lowerName.toLowerCase();
            
            if (lowerName.endsWith(".jpg") || lowerName.endsWith(".jpeg")) {
                ImageQualityInfo info;
                info.path = dirPath + "/" + fileName;
                info.fileSize = file.size();
                info.timestamp = 0;  // Could parse from filename if needed
                info.isValid = (info.fileSize >= MIN_IMAGE_SIZE && info.fileSize <= MAX_IMAGE_SIZE);
                
                // For quality scoring, we'd need to read file content
                // For efficiency, estimate quality from file size
                if (info.isValid) {
                    info.qualityScore = (float)info.fileSize / 1000.0f;  // Simple size-based estimate
                    if (info.qualityScore > 100.0f) info.qualityScore = 100.0f;
                } else {
                    info.qualityScore = 0.0f;
                }
                
                info.hash = 0;  // Would need to read file for actual hash
                
                images.push_back(info);
            }
        }
        
        file = dir.openNextFile();
    }
    
    dir.close();
}

bool StorageManager::deleteImageAndMetadata(const String& imagePath) {
    if (!initialized || imagePath.length() == 0) {
        return false;
    }
    
    bool success = true;
    
    // Delete the image file
    if (SD_MMC.exists(imagePath)) {
        if (!SD_MMC.remove(imagePath)) {
            success = false;
        }
    }
    
    // Delete associated metadata file
    int dotIndex = imagePath.lastIndexOf('.');
    if (dotIndex > 0) {
        String metaPath = imagePath.substring(0, dotIndex) + ".json";
        if (SD_MMC.exists(metaPath)) {
            SD_MMC.remove(metaPath);
        }
    }
    
    return success;
}

String StorageManager::saveImage(camera_fb_t* fb, const String& customPath) {
    // Validate initialization state
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        updateErrorStats(false, SD_ERROR_NOT_MOUNTED);
        return "";
    }
    
    // Validate frame buffer pointer and data integrity
    // Frame buffer must exist, have valid data pointer, and non-zero length
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println(FPSTR(TAG_ERROR_INVALID_FB));
        return "";
    }
    
    // Check SD card accessibility before proceeding
    if (!checkSDCardAccess()) {
        if (autoRemountEnabled) {
            if (!remountSDCard()) {
                return "";
            }
        } else {
            return "";
        }
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
    
    // Implement retry logic for file operations
    bool writeSuccess = false;
    unsigned int currentDelay = retryDelayMs;
    
    for (unsigned int retryAttempt = 0; retryAttempt <= maxRetries && !writeSuccess; retryAttempt++) {
        if (retryAttempt > 0) {
            Serial.printf_P(TAG_RETRY, retryAttempt, maxRetries, currentDelay);
            delay(currentDelay);
            // Exponential backoff
            currentDelay = min(currentDelay * 2, maxRetryDelayMs);
        }
        
        // Open file for writing in binary mode
        File file = SD_MMC.open(fullPath, FILE_WRITE);
        if (!file) {
            Serial.printf_P(TAG_ERROR_OPEN_FILE, fullPath.c_str());
            lastError = SD_ERROR_FILE_OPEN;
            lastErrorMessage = "Failed to open file: " + fullPath;
            continue;  // Retry
        }
        
        // Write complete frame buffer data to file using buffered write
        // This reduces memory fragmentation and improves reliability
        size_t written;
        if (memoryPoolEnabled && writeBuffer != nullptr) {
            written = bufferedWrite(file, fb->buf, fb->len);
        } else {
            written = file.write(fb->buf, fb->len);
        }
        
        // Close file to ensure data is flushed to SD card
        file.close();
        
        // Verify all bytes were written successfully
        if (written == fb->len) {
            writeSuccess = true;
        } else {
            Serial.printf_P(TAG_ERROR_WRITE, (int)written, (int)fb->len);
            lastError = SD_ERROR_FILE_WRITE;
            lastErrorMessage = "Incomplete write: " + String(written) + " of " + String(fb->len) + " bytes";
            // Remove partial file
            SD_MMC.remove(fullPath);
        }
    }
    
    if (!writeSuccess) {
        updateErrorStats(false, lastError);
        return "";
    }
    
    // Increment counter for next image and persist to non-volatile storage
    // This ensures counter survives power cycles and device resets
    imageCounter++;
    preferences.putULong("imageCounter", imageCounter);
    
    updateErrorStats(true);
    Serial.printf_P(TAG_SUCCESS_SAVE, fullPath.c_str(), (int)fb->len);
    
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
    
    Serial.printf_P(TAG_SUCCESS_META, jsonPath.c_str(), (int)bytesWritten);
    
    return true;
}

bool StorageManager::deleteOldFiles(int daysToKeep) {
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return false;
    }
    
    Serial.printf_P(TAG_CLEANUP, daysToKeep);
    
    // Get current date for comparison
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("WARNING: Cannot get current time for date comparison");
        return false;
    }
    
    // Calculate cutoff date (today - daysToKeep)
    time_t now;
    time(&now);
    time_t cutoff = now - (daysToKeep * 24 * 60 * 60);
    struct tm cutoffTime;
    localtime_r(&cutoff, &cutoffTime);
    
    // Format cutoff date as YYYYMMDD for comparison
    char cutoffDateStr[9];
    snprintf(cutoffDateStr, sizeof(cutoffDateStr), "%04d%02d%02d",
             cutoffTime.tm_year + 1900,
             cutoffTime.tm_mon + 1,
             cutoffTime.tm_mday);
    String cutoffDate = String(cutoffDateStr);
    
    // Open base directory and iterate through date folders
    File baseDir = SD_MMC.open(basePath);
    if (!baseDir || !baseDir.isDirectory()) {
        return false;
    }
    
    std::vector<String> dirsToDelete;
    File dir = baseDir.openNextFile();
    
    while (dir) {
        if (dir.isDirectory()) {
            String dirName = String(dir.name());
            
            // Check if this is a date-formatted directory (YYYYMMDD)
            if (dirName.length() == 8 && dirName.charAt(0) >= '0' && dirName.charAt(0) <= '9') {
                // Compare directory date with cutoff
                if (dirName < cutoffDate) {
                    dirsToDelete.push_back(basePath + "/" + dirName);
                }
            }
            // Handle fallback directories (day_XXXXX format)
            else if (dirName.startsWith("day_")) {
                // For simplicity, keep last N day_ directories
                // Could implement more sophisticated logic if needed
            }
        }
        dir = baseDir.openNextFile();
    }
    baseDir.close();
    
    // Delete old directories and their contents
    int deletedCount = 0;
    for (const String& dirPath : dirsToDelete) {
        // First delete all files in the directory
        File oldDir = SD_MMC.open(dirPath);
        if (oldDir && oldDir.isDirectory()) {
            File file = oldDir.openNextFile();
            while (file) {
                String filePath = dirPath + "/" + String(file.name());
                file.close();
                SD_MMC.remove(filePath);
                file = oldDir.openNextFile();
            }
            oldDir.close();
        }
        
        // Then remove the empty directory
        if (SD_MMC.rmdir(dirPath)) {
            deletedCount++;
            Serial.printf("Deleted old directory: %s\n", dirPath.c_str());
        }
    }
    
    Serial.printf("Cleanup complete: deleted %d old directories\n", deletedCount);
    
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
    
    Serial.println(FPSTR(TAG_STORAGE_INFO));
    Serial.printf_P(TAG_BASE_PATH, basePath.c_str());
    Serial.printf_P(TAG_INITIALIZED, initialized ? "Yes" : "No");
    Serial.printf_P(TAG_IMG_COUNT, imageCounter);
    
    // Get raw byte values from SD card
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;
    
    Serial.printf_P(TAG_TOTAL_BYTES, totalBytes / (1024 * 1024), totalBytes);
    Serial.printf_P(TAG_USED_BYTES, usedBytes / (1024 * 1024), usedBytes);
    Serial.printf_P(TAG_FREE_BYTES, freeBytes / (1024 * 1024), freeBytes);
    
    // Calculate and display usage percentage
    // Avoid division by zero if total is somehow 0
    if (totalBytes > 0) {
        float usedPercent = (float)usedBytes / totalBytes * 100.0f;
        Serial.printf_P(TAG_USAGE, usedPercent);
    }
    
    // Print improved storage feature status
    Serial.printf("Compression: %s (quality: %d)\n", 
                  compressionEnabled ? "Enabled" : "Disabled",
                  compressionQuality);
    Serial.printf("Duplicate Detection: %s\n", 
                  duplicateDetectionEnabled ? "Enabled" : "Disabled");
    Serial.printf("Duplicates Skipped (session): %lu\n", g_duplicatesSkipped);
    Serial.printf("Bytes Saved (session): %lu\n", g_bytesCompressed);
    
    Serial.println(FPSTR(TAG_SEPARATOR));
}

std::vector<String> StorageManager::getImageFiles() {
    std::vector<String> imageFiles;
    
    // Check initialization status
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return imageFiles;
    }
    
    // Helper lambda to recursively scan directories
    std::function<void(const String&)> scanDirectory = [&](const String& dirPath) {
        File dir = SD_MMC.open(dirPath);
        if (!dir || !dir.isDirectory()) {
            return;
        }
        
        File file = dir.openNextFile();
        int fileCount = 0;
        
        while (file && fileCount < MAX_FILES_PER_DIR) {
            fileCount++;
            
            const char* rawName = file.name();
            if (rawName == nullptr) {
                file = dir.openNextFile();
                continue;
            }
            
            String fileName = String(rawName);
            if (fileName.length() == 0) {
                file = dir.openNextFile();
                continue;
            }
            
            if (file.isDirectory()) {
                // Recursively scan subdirectories
                String subDirPath = dirPath + "/" + fileName;
                scanDirectory(subDirPath);
            } else {
                // Check if this is a JPEG image file
                String lowerName = fileName;
                lowerName.toLowerCase();
                
                if (lowerName.endsWith(".jpg") || lowerName.endsWith(".jpeg")) {
                    size_t fileSize = file.size();
                    
                    // Validate file size is reasonable
                    if (fileSize >= MIN_IMAGE_SIZE && fileSize <= MAX_IMAGE_SIZE) {
                        String fullPath = dirPath + "/" + fileName;
                        imageFiles.push_back(fullPath);
                    }
                }
            }
            
            file = dir.openNextFile();
        }
        
        dir.close();
    };
    
    // Start scanning from base path
    scanDirectory(basePath);
    
    // Sort files in descending order (newest first)
    std::sort(imageFiles.begin(), imageFiles.end(), std::greater<String>());
    
    return imageFiles;
}

unsigned long StorageManager::getImageCount() {
    if (!initialized) {
        return 0;
    }
    return imageCounter;
}

// ============================================================================
// Improved Storage Features Implementation
// ============================================================================

void StorageManager::setCompressionEnabled(bool enable) {
    compressionEnabled = enable;
}

bool StorageManager::isCompressionEnabled() const {
    return compressionEnabled;
}

void StorageManager::setCompressionQuality(int quality) {
    // Clamp quality to valid JPEG range (1-63)
    if (quality < 1) quality = 1;
    if (quality > 63) quality = 63;
    compressionQuality = quality;
}

int StorageManager::getCompressionQuality() const {
    return compressionQuality;
}

void StorageManager::setDuplicateDetectionEnabled(bool enable) {
    duplicateDetectionEnabled = enable;
}

bool StorageManager::isDuplicateDetectionEnabled() const {
    return duplicateDetectionEnabled;
}

void StorageManager::clearDuplicateCache() {
    recentImageHashes.clear();
}

int StorageManager::smartDelete(unsigned long targetFreeSpaceKB) {
    if (!initialized) {
        return -1;
    }
    
    unsigned long currentFreeKB = getFreeSpace() / 1024;
    if (currentFreeKB >= targetFreeSpaceKB) {
        return 0;  // Already have enough free space
    }
    
    unsigned long spaceNeededKB = targetFreeSpaceKB - currentFreeKB;
    
    // Collect information about all images
    std::vector<ImageQualityInfo> allImages;
    collectImageInfo(basePath, allImages);
    
    if (allImages.empty()) {
        return 0;
    }
    
    // Sort by quality score (lowest first, so we delete worst quality first)
    std::sort(allImages.begin(), allImages.end(), 
              [](const ImageQualityInfo& a, const ImageQualityInfo& b) {
                  return a.qualityScore < b.qualityScore;
              });
    
    // Keep at least 10 images regardless of quality
    const size_t MIN_IMAGES_TO_KEEP = 10;
    size_t maxDeletable = (allImages.size() > MIN_IMAGES_TO_KEEP) 
                          ? (allImages.size() - MIN_IMAGES_TO_KEEP) 
                          : 0;
    
    int deletedCount = 0;
    unsigned long freedBytes = 0;
    
    for (size_t i = 0; i < maxDeletable && freedBytes < (spaceNeededKB * 1024); i++) {
        const ImageQualityInfo& info = allImages[i];
        
        // Don't delete if quality is above threshold (keep good quality images)
        if (info.qualityScore > 50.0f) {
            continue;
        }
        
        if (deleteImageAndMetadata(info.path)) {
            deletedCount++;
            freedBytes += info.fileSize;
        }
    }
    
    Serial.printf_P(TAG_SMART_DELETE, deletedCount, freedBytes / 1024);
    
    return deletedCount;
}

String StorageManager::saveImageWithCompression(camera_fb_t* fb, const String& customPath, bool skipDuplicates) {
    // Validate initialization state
    if (!initialized) {
        Serial.println(FPSTR(TAG_ERROR_NOT_INIT));
        return "";
    }
    
    // Validate frame buffer pointer and data integrity
    if (!fb || fb->buf == NULL || fb->len == 0) {
        Serial.println(FPSTR(TAG_ERROR_INVALID_FB));
        return "";
    }
    
    // Calculate hash for duplicate detection
    uint32_t imageHash = 0;
    if (duplicateDetectionEnabled && skipDuplicates) {
        imageHash = calculateImageHash(fb->buf, fb->len);
        
        if (isDuplicateImage(imageHash)) {
            Serial.println(FPSTR(TAG_DUPLICATE_DETECTED));
            g_duplicatesSkipped++;
            return "";  // Skip saving duplicate
        }
    }
    
    // Check if we need to free up space before saving
    unsigned long freeSpaceKB = getFreeSpace() / 1024;
    if (freeSpaceKB < STORAGE_MIN_FREE_SPACE_KB) {
        Serial.println("INFO: Low storage space, running smart delete...");
        smartDelete(STORAGE_MIN_FREE_SPACE_KB * 2);  // Try to free up 2x minimum
    }
    
    // Note: Actual JPEG re-compression would require the esp_camera to capture
    // at a different quality setting, or using a JPEG library to re-compress.
    // For now, we use the image as-is but track the quality setting for 
    // future captures via the camera manager.
    
    // Save the image using the standard method
    String savedPath = saveImage(fb, customPath);
    
    // If save was successful and we have a valid hash, add to cache
    if (savedPath.length() > 0 && imageHash != 0) {
        // Manage cache size - remove oldest entries if cache is full
        if (recentImageHashes.size() >= MAX_HASH_CACHE_SIZE) {
            // Remove first (oldest) entry
            recentImageHashes.erase(recentImageHashes.begin());
        }
        
        recentImageHashes[imageHash] = savedPath;
    }
    
    return savedPath;
}

void StorageManager::getStorageStats(unsigned long& totalImages, unsigned long& duplicatesSkipped, unsigned long& bytesCompressed) {
    totalImages = imageCounter;
    duplicatesSkipped = g_duplicatesSkipped;
    bytesCompressed = g_bytesCompressed;
}

// ============================================================================
// SD Card Error Handling and Retry Logic Implementation
// ============================================================================

const char* StorageManager::errorToString(SDCardError error) {
    switch (error) {
        case SD_ERROR_NONE:         return "No error";
        case SD_ERROR_NOT_MOUNTED:  return "SD card not mounted";
        case SD_ERROR_CARD_REMOVED: return "SD card removed";
        case SD_ERROR_MOUNT_FAILED: return "Mount failed";
        case SD_ERROR_FILE_OPEN:    return "File open failed";
        case SD_ERROR_FILE_WRITE:   return "File write failed";
        case SD_ERROR_FILE_READ:    return "File read failed";
        case SD_ERROR_DIR_CREATE:   return "Directory creation failed";
        case SD_ERROR_DIR_OPEN:     return "Directory open failed";
        case SD_ERROR_CARD_FULL:    return "SD card full";
        case SD_ERROR_TIMEOUT:      return "Operation timeout";
        case SD_ERROR_CORRUPTED:    return "Data corruption detected";
        default:                    return "Unknown error";
    }
}

SDCardError StorageManager::getLastError() const {
    return lastError;
}

String StorageManager::getLastErrorMessage() const {
    return lastErrorMessage;
}

void StorageManager::clearLastError() {
    lastError = SD_ERROR_NONE;
    lastErrorMessage = "";
}

void StorageManager::setMaxRetries(unsigned int retries) {
    // Clamp to reasonable range (1-10)
    if (retries < 1) retries = 1;
    if (retries > 10) retries = 10;
    maxRetries = retries;
}

unsigned int StorageManager::getMaxRetries() const {
    return maxRetries;
}

void StorageManager::setRetryDelay(unsigned int delayMs) {
    // Clamp to reasonable range (10-5000 ms)
    if (delayMs < 10) delayMs = 10;
    if (delayMs > 5000) delayMs = 5000;
    retryDelayMs = delayMs;
}

unsigned int StorageManager::getRetryDelay() const {
    return retryDelayMs;
}

void StorageManager::setAutoRemountEnabled(bool enable) {
    autoRemountEnabled = enable;
}

bool StorageManager::isAutoRemountEnabled() const {
    return autoRemountEnabled;
}

void StorageManager::updateErrorStats(bool success, SDCardError error) {
    if (success) {
        cardHealth.consecutiveErrors = 0;
        cardHealth.successfulOps++;
    } else {
        cardHealth.consecutiveErrors++;
        cardHealth.totalErrors++;
        lastError = error;
        lastErrorMessage = String(errorToString(error));
    }
    
    // Calculate error rate
    unsigned long totalOps = cardHealth.successfulOps + cardHealth.totalErrors;
    if (totalOps > 0) {
        cardHealth.errorRate = (float)cardHealth.totalErrors / totalOps * 100.0f;
    }
    
    // Check if auto-remount should be triggered
    if (autoRemountEnabled && cardHealth.consecutiveErrors >= SD_CARD_ERROR_THRESHOLD) {
        Serial.println("Auto-remount triggered due to consecutive errors");
        remountSDCard();
    }
}

bool StorageManager::checkSDCardAccess() {
    if (!initialized) {
        return false;
    }
    
    // Quick check if card is still present
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        lastError = SD_ERROR_CARD_REMOVED;
        lastErrorMessage = "SD card not detected";
        return false;
    }
    
    return true;
}

bool StorageManager::remountSDCard() {
    Serial.println(FPSTR(TAG_REMOUNT_ATTEMPT));
    
    // End current mount
    SD_MMC.end();
    delay(100);  // Brief delay for card to stabilize
    
    // Attempt to remount
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println(FPSTR(TAG_REMOUNT_FAIL));
        initialized = false;
        cardHealth.mounted = false;
        lastError = SD_ERROR_MOUNT_FAILED;
        lastErrorMessage = "Remount failed";
        return false;
    }
    
    // Verify card is present after remount
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(FPSTR(TAG_REMOUNT_FAIL));
        initialized = false;
        cardHealth.mounted = false;
        lastError = SD_ERROR_CARD_REMOVED;
        lastErrorMessage = "Card not detected after remount";
        return false;
    }
    
    // Update health stats
    cardHealth.mounted = true;
    cardHealth.cardType = cardType;
    cardHealth.consecutiveErrors = 0;
    cardHealth.totalBytes = SD_MMC.totalBytes();
    cardHealth.usedBytes = SD_MMC.usedBytes();
    cardHealth.freeBytes = cardHealth.totalBytes - cardHealth.usedBytes;
    
    Serial.println(FPSTR(TAG_REMOUNT_SUCCESS));
    clearLastError();
    return true;
}

bool StorageManager::forceRemount() {
    return remountSDCard();
}

SDCardHealth StorageManager::getSDCardHealth() const {
    return cardHealth;
}

bool StorageManager::performHealthCheck() {
    if (!initialized) {
        cardHealth.mounted = false;
        return false;
    }
    
    bool healthy = true;
    
    // Check card type
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        cardHealth.mounted = false;
        lastError = SD_ERROR_CARD_REMOVED;
        lastErrorMessage = "SD card not detected during health check";
        healthy = false;
    } else {
        cardHealth.mounted = true;
        cardHealth.cardType = cardType;
        
        // Update space statistics
        cardHealth.totalBytes = SD_MMC.totalBytes();
        cardHealth.usedBytes = SD_MMC.usedBytes();
        cardHealth.freeBytes = cardHealth.totalBytes - cardHealth.usedBytes;
        
        // Check if card is nearly full (less than 1% free)
        if (cardHealth.freeBytes < cardHealth.totalBytes / 100) {
            lastError = SD_ERROR_CARD_FULL;
            lastErrorMessage = "SD card nearly full";
            healthy = false;
        }
    }
    
    cardHealth.lastHealthCheck = millis();
    
    Serial.printf_P(TAG_HEALTH_CHECK, healthy ? "OK" : "Issues detected");
    
    return healthy;
}

void StorageManager::resetErrorStats() {
    cardHealth.consecutiveErrors = 0;
    cardHealth.totalErrors = 0;
    cardHealth.successfulOps = 0;
    cardHealth.errorRate = 0.0f;
    clearLastError();
}

// ============================================================================
// Memory Management Implementation
// ============================================================================

bool StorageManager::allocateWriteBuffer() {
    if (writeBuffer != nullptr) {
        return true;  // Already allocated
    }
    
    // Try to allocate from PSRAM first if available
    #ifdef BOARD_HAS_PSRAM
    writeBuffer = (uint8_t*)ps_malloc(writeBufferSize);
    #else
    writeBuffer = (uint8_t*)malloc(writeBufferSize);
    #endif
    
    if (writeBuffer == nullptr) {
        Serial.println("WARNING: Failed to allocate write buffer");
        return false;
    }
    
    return true;
}

void StorageManager::freeWriteBuffer() {
    if (writeBuffer != nullptr) {
        free(writeBuffer);
        writeBuffer = nullptr;
    }
}

size_t StorageManager::bufferedWrite(File& file, const uint8_t* data, size_t length) {
    if (!memoryPoolEnabled || writeBuffer == nullptr) {
        // Direct write without buffering
        return file.write(data, length);
    }
    
    size_t totalWritten = 0;
    size_t remaining = length;
    const uint8_t* dataPtr = data;
    
    while (remaining > 0) {
        size_t chunkSize = (remaining > writeBufferSize) ? writeBufferSize : remaining;
        
        // Copy to buffer and write
        memcpy(writeBuffer, dataPtr, chunkSize);
        size_t written = file.write(writeBuffer, chunkSize);
        
        if (written != chunkSize) {
            // Write error occurred
            break;
        }
        
        totalWritten += written;
        dataPtr += chunkSize;
        remaining -= chunkSize;
    }
    
    return totalWritten;
}

MemoryStats StorageManager::getMemoryStats() const {
    MemoryStats stats;
    
    stats.freeHeap = ESP.getFreeHeap();
    stats.minFreeHeap = ESP.getMinFreeHeap();
    stats.largestFreeBlock = ESP.getMaxAllocHeap();
    
    // Calculate fragmentation percentage
    // Fragmentation = 1 - (largest_free_block / total_free_heap)
    if (stats.freeHeap > 0) {
        stats.fragmentationPercent = (1.0f - (float)stats.largestFreeBlock / stats.freeHeap) * 100.0f;
    } else {
        stats.fragmentationPercent = 100.0f;
    }
    
    stats.lowMemoryWarning = (stats.freeHeap < MIN_FREE_HEAP_BYTES);
    
    if (stats.lowMemoryWarning) {
        Serial.printf_P(TAG_LOW_MEMORY, stats.freeHeap);
    }
    
    return stats;
}

void StorageManager::setMemoryPoolEnabled(bool enable) {
    memoryPoolEnabled = enable;
    
    if (enable) {
        allocateWriteBuffer();
    } else {
        freeWriteBuffer();
    }
}

bool StorageManager::isMemoryPoolEnabled() const {
    return memoryPoolEnabled;
}

bool StorageManager::isLowMemory() const {
    return ESP.getFreeHeap() < MIN_FREE_HEAP_BYTES;
}

size_t StorageManager::optimizeMemory() {
    size_t freedBytes = 0;
    size_t beforeHeap = ESP.getFreeHeap();
    
    // Clear duplicate detection cache
    recentImageHashes.clear();
    
    // Force garbage collection (if Arduino String fragmentation exists)
    String().reserve(0);
    
    // Calculate freed memory
    size_t afterHeap = ESP.getFreeHeap();
    if (afterHeap > beforeHeap) {
        freedBytes = afterHeap - beforeHeap;
    }
    
    Serial.printf_P(TAG_MEMORY_OPTIMIZED, freedBytes);
    
    return freedBytes;
}
