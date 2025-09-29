#include "storage_manager.h"
#include "../utils/logger.h"
#include <time.h>

// Static member definitions
bool StorageManager::initialized = false;
uint8_t StorageManager::warningThreshold = 85;

bool StorageManager::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing storage manager...");

    // Check if SD card is mounted
    if (!SD_MMC.begin()) {
        LOG_ERROR("SD card not available for storage management");
        return false;
    }

    // Create directory structure
    if (!createDirectoryStructure()) {
        LOG_WARNING("Some directories could not be created");
    }

    initialized = true;
    LOG_INFO("Storage manager initialized successfully");

    // Log initial statistics
    StorageStats stats = getStatistics();
    LOG_INFO("Storage capacity: " + String((unsigned long)(stats.totalSpace / (1024 * 1024))) + "MB");
    LOG_INFO("Storage used: " + String(stats.usagePercentage, 1) + "%");

    return true;
}

StorageManager::StorageStats StorageManager::getStatistics() {
    StorageStats stats = {};

    if (!initialized) {
        return stats;
    }

    // Get SD card capacity
    stats.totalSpace = SD_MMC.totalBytes();
    stats.usedSpace = SD_MMC.usedBytes();
    stats.freeSpace = stats.totalSpace - stats.usedSpace;
    stats.usagePercentage = (float)stats.usedSpace / stats.totalSpace * 100.0f;

    // Count files
    countFiles("/", stats);

    return stats;
}

bool StorageManager::hasAdequateSpace(size_t requiredSpace) {
    if (!initialized) {
        return false;
    }

    StorageStats stats = getStatistics();
    return stats.freeSpace >= requiredSpace;
}

StorageManager::CleanupResult StorageManager::performCleanup(bool aggressiveCleanup) {
    CleanupResult result = {};
    uint32_t startTime = millis();

    if (!initialized) {
        return result;
    }

    LOG_INFO("Starting storage cleanup...");

    uint64_t initialFreeSpace = SD_MMC.totalBytes() - SD_MMC.usedBytes();

    // Delete old files (30 days for normal, 7 days for aggressive)
    uint8_t maxAge = aggressiveCleanup ? 7 : 30;
    result.filesDeleted += deleteOldFiles(maxAge);

    // Optimize storage by removing low-quality data
    if (aggressiveCleanup) {
        result.filesDeleted += optimizeStorage();
    }

    // Calculate space freed
    uint64_t finalFreeSpace = SD_MMC.totalBytes() - SD_MMC.usedBytes();
    result.spaceFree = finalFreeSpace - initialFreeSpace;
    result.processingTime = millis() - startTime;
    result.success = result.filesDeleted > 0 || result.spaceFree > 0;

    LOG_INFO("Storage cleanup completed: " + String(result.filesDeleted) + 
             " files deleted, " + String((unsigned long)(result.spaceFree / 1024)) + "KB freed");

    return result;
}

uint32_t StorageManager::deleteOldFiles(uint8_t maxAgeDays, const String& filePattern) {
    if (!initialized) {
        return 0;
    }

    uint32_t filesDeleted = 0;

    // Delete old files from images directory
    filesDeleted += deleteFilesRecursive(IMAGE_FOLDER, maxAgeDays, filePattern);

    // Delete old log files
    filesDeleted += deleteFilesRecursive(LOG_FOLDER, maxAgeDays, "*.log");

    return filesDeleted;
}

uint32_t StorageManager::optimizeStorage() {
    if (!initialized) {
        return 0;
    }

    uint32_t filesRemoved = 0;

    // This is a placeholder for storage optimization
    // Real implementation might:
    // - Remove duplicate images
    // - Delete images with very low AI confidence
    // - Compress old images
    // - Remove images without wildlife detection

    LOG_DEBUG("Storage optimization completed: " + String(filesRemoved) + " files optimized");

    return filesRemoved;
}

bool StorageManager::createDirectoryStructure() {
    bool success = true;

    // Create main directories
    if (!SD_MMC.exists(IMAGE_FOLDER)) {
        success &= SD_MMC.mkdir(IMAGE_FOLDER);
    }

    if (!SD_MMC.exists(LOG_FOLDER)) {
        success &= SD_MMC.mkdir(LOG_FOLDER);
    }

    if (!SD_MMC.exists(DATA_FOLDER)) {
        success &= SD_MMC.mkdir(DATA_FOLDER);
    }

    // Create species subdirectories
    String speciesDirs[] = {
        "/deer", "/bear", "/fox", "/wolf", "/raccoon", 
        "/coyote", "/elk", "/moose", "/turkey", "/eagle", "/unknown"
    };

    for (const String& speciesDir : speciesDirs) {
        String fullPath = String(IMAGE_FOLDER) + speciesDir;
        if (!SD_MMC.exists(fullPath.c_str())) {
            success &= SD_MMC.mkdir(fullPath.c_str());
        }
    }

    return success;
}

uint16_t StorageManager::getFileAge(const String& filename) {
    if (!fileExists(filename)) {
        return 0;
    }

    // Get current time
    time_t now;
    time(&now);

    // Get file modification time (simplified)
    // In a real implementation, you would read the actual file timestamp
    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        return 0;
    }

    // Extract date from filename if it follows our naming convention
    // Format: prefix_YYYYMMDD_HHMMSS_counter.ext
    String basename = filename;
    int lastSlash = basename.lastIndexOf('/');
    if (lastSlash >= 0) {
        basename = basename.substring(lastSlash + 1);
    }

    // Extract date part (YYYYMMDD)
    int dateStart = basename.indexOf('_');
    if (dateStart >= 0) {
        String datePart = basename.substring(dateStart + 1, dateStart + 9);
        if (datePart.length() == 8) {
            // Parse date
            int year = datePart.substring(0, 4).toInt();
            int month = datePart.substring(4, 6).toInt();
            int day = datePart.substring(6, 8).toInt();

            // Calculate age in days (simplified)
            struct tm fileTime = {};
            fileTime.tm_year = year - 1900;
            fileTime.tm_mon = month - 1;
            fileTime.tm_mday = day;

            time_t fileTimestamp = mktime(&fileTime);
            uint32_t ageDays = (now - fileTimestamp) / (24 * 3600);

            file.close();
            return ageDays;
        }
    }

    file.close();
    return 0;
}

bool StorageManager::fileExists(const String& filename) {
    return SD_MMC.exists(filename.c_str());
}

size_t StorageManager::getFileSize(const String& filename) {
    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        return 0;
    }

    size_t size = file.size();
    file.close();
    return size;
}

uint16_t StorageManager::listFiles(const String& directory, String* fileList, uint16_t maxFiles) {
    if (!initialized || fileList == nullptr) {
        return 0;
    }

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    uint16_t fileCount = 0;
    File file = dir.openNextFile();

    while (file && fileCount < maxFiles) {
        if (!file.isDirectory()) {
            fileList[fileCount] = String(file.name());
            fileCount++;
        }
        file.close();
        file = dir.openNextFile();
    }

    dir.close();
    return fileCount;
}

bool StorageManager::isWarningThresholdExceeded() {
    StorageStats stats = getStatistics();
    return stats.usagePercentage >= warningThreshold;
}

void StorageManager::countFiles(const String& directory, StorageStats& stats) {
    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            stats.totalFiles++;

            String filename = String(file.name());
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
                stats.imageFiles++;
            } else if (filename.endsWith(".json")) {
                stats.metadataFiles++;
            } else if (filename.endsWith(".log")) {
                stats.logFiles++;
            }
        } else {
            // Recursively count files in subdirectories
            String subDir = directory;
            if (!subDir.endsWith("/")) {
                subDir += "/";
            }
            subDir += file.name();
            countFiles(subDir, stats);
        }

        file.close();
        file = dir.openNextFile();
    }

    dir.close();
}

uint64_t StorageManager::calculateDirectorySize(const String& directory) {
    uint64_t totalSize = 0;

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            totalSize += file.size();
        } else {
            // Recursively calculate size of subdirectories
            String subDir = directory;
            if (!subDir.endsWith("/")) {
                subDir += "/";
            }
            subDir += file.name();
            totalSize += calculateDirectorySize(subDir);
        }

        file.close();
        file = dir.openNextFile();
    }

    dir.close();
    return totalSize;
}

uint32_t StorageManager::deleteFilesRecursive(const String& directory, uint8_t maxAge, const String& pattern) {
    uint32_t filesDeleted = 0;

    File dir = SD_MMC.open(directory.c_str());
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    File file = dir.openNextFile();
    while (file) {
        String filename = String(file.name());
        String fullPath = directory;
        if (!fullPath.endsWith("/")) {
            fullPath += "/";
        }
        fullPath += filename;

        if (!file.isDirectory()) {
            // Check file age and pattern
            uint16_t age = getFileAge(fullPath);
            bool matchesPattern = pattern.isEmpty() || filename.indexOf(pattern.substring(1)) >= 0; // Simple pattern matching

            if (age >= maxAge && matchesPattern) {
                file.close();
                if (SD_MMC.remove(fullPath.c_str())) {
                    filesDeleted++;
                    LOG_DEBUG("Deleted old file: " + fullPath);
                }
            } else {
                file.close();
            }
        } else {
            file.close();
            // Recursively delete files in subdirectories
            filesDeleted += deleteFilesRecursive(fullPath, maxAge, pattern);
        }

        file = dir.openNextFile();
    }

    dir.close();
    return filesDeleted;
}