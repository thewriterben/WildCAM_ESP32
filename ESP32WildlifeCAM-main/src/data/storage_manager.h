#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "../include/config.h"

/**
 * @brief SD card storage management and optimization
 * 
 * Manages SD card operations, storage optimization,
 * and data cleanup for wildlife camera system.
 */
class StorageManager {
public:
    /**
     * @brief Storage statistics
     */
    struct StorageStats {
        uint64_t totalSpace;        // Total SD card space
        uint64_t usedSpace;         // Used space
        uint64_t freeSpace;         // Available space
        float usagePercentage;      // Usage percentage
        uint32_t totalFiles;        // Total files on card
        uint32_t imageFiles;        // Number of image files
        uint32_t metadataFiles;     // Number of metadata files
        uint32_t logFiles;          // Number of log files
    };

    /**
     * @brief Storage cleanup result
     */
    struct CleanupResult {
        bool success;
        uint32_t filesDeleted;
        uint64_t spaceFree;
        uint32_t processingTime;
    };

    /**
     * @brief Initialize storage manager
     * @return true if initialization successful
     */
    static bool initialize();

    /**
     * @brief Get current storage statistics
     * @return Storage statistics
     */
    static StorageStats getStatistics();

    /**
     * @brief Check if storage space is adequate
     * @param requiredSpace Required space in bytes
     * @return true if adequate space available
     */
    static bool hasAdequateSpace(size_t requiredSpace = 10 * 1024 * 1024); // Default 10MB

    /**
     * @brief Perform storage cleanup
     * @param aggressiveCleanup Enable aggressive cleanup
     * @return Cleanup result
     */
    static CleanupResult performCleanup(bool aggressiveCleanup = false);

    /**
     * @brief Delete old files based on age
     * @param maxAgeDays Maximum age in days
     * @param filePattern File pattern to match
     * @return Number of files deleted
     */
    static uint32_t deleteOldFiles(uint8_t maxAgeDays = 30, const String& filePattern = "");

    /**
     * @brief Optimize storage by removing low-quality data
     * @return Number of files removed
     */
    static uint32_t optimizeStorage();

    /**
     * @brief Create directory structure
     * @return true if successful
     */
    static bool createDirectoryStructure();

    /**
     * @brief Get file age in days
     * @param filename File path
     * @return Age in days
     */
    static uint16_t getFileAge(const String& filename);

    /**
     * @brief Check if file exists
     * @param filename File path
     * @return true if file exists
     */
    static bool fileExists(const String& filename);

    /**
     * @brief Get file size
     * @param filename File path
     * @return File size in bytes
     */
    static size_t getFileSize(const String& filename);

    /**
     * @brief List files in directory
     * @param directory Directory path
     * @param fileList Output file list
     * @param maxFiles Maximum files to list
     * @return Number of files found
     */
    static uint16_t listFiles(const String& directory, String* fileList, uint16_t maxFiles);

    /**
     * @brief Format storage warning threshold
     * @param percentage Warning threshold (0-100)
     */
    static void setWarningThreshold(uint8_t percentage) { warningThreshold = percentage; }

    /**
     * @brief Check if warning threshold exceeded
     * @return true if storage usage exceeds warning threshold
     */
    static bool isWarningThresholdExceeded();

private:
    static bool initialized;
    static uint8_t warningThreshold;

    /**
     * @brief Count files recursively
     * @param directory Directory to count
     * @param stats Storage statistics to update
     */
    static void countFiles(const String& directory, StorageStats& stats);

    /**
     * @brief Calculate directory size
     * @param directory Directory path
     * @return Size in bytes
     */
    static uint64_t calculateDirectorySize(const String& directory);

    /**
     * @brief Delete files recursively
     * @param directory Directory path
     * @param maxAge Maximum age in days
     * @param pattern File pattern
     * @return Number of files deleted
     */
    static uint32_t deleteFilesRecursive(const String& directory, uint8_t maxAge, const String& pattern);
};

#endif // STORAGE_MANAGER_H