/**
 * @file StorageManager.h
 * @brief SD card storage management for ESP32-CAM wildlife camera system
 * 
 * This class provides comprehensive SD card management functionality for the
 * ESP32-CAM wildlife monitoring system. It handles image storage, metadata
 * management, automatic date-based directory organization, and storage
 * space monitoring with robust error handling.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <esp_camera.h>
#include <Preferences.h>
#include <vector>
#include <map>

// Default configuration values for improved storage features
#ifndef STORAGE_DEFAULT_COMPRESSION_QUALITY
#define STORAGE_DEFAULT_COMPRESSION_QUALITY 15  // Default JPEG quality (1-63, lower is higher quality)
#endif

#ifndef STORAGE_MIN_FREE_SPACE_KB
#define STORAGE_MIN_FREE_SPACE_KB 10240  // Minimum 10MB free space before triggering cleanup
#endif

#ifndef STORAGE_DUPLICATE_THRESHOLD
#define STORAGE_DUPLICATE_THRESHOLD 95  // Percentage similarity threshold for duplicate detection
#endif

// SD card error handling defaults (can be overridden in config.h)
#ifndef SD_CARD_MAX_RETRIES
#define SD_CARD_MAX_RETRIES 3
#endif

#ifndef SD_CARD_RETRY_DELAY_MS
#define SD_CARD_RETRY_DELAY_MS 100
#endif

#ifndef SD_CARD_MAX_RETRY_DELAY_MS
#define SD_CARD_MAX_RETRY_DELAY_MS 1000
#endif

#ifndef SD_CARD_AUTO_REMOUNT
#define SD_CARD_AUTO_REMOUNT true
#endif

#ifndef SD_CARD_ERROR_THRESHOLD
#define SD_CARD_ERROR_THRESHOLD 5
#endif

// Memory management defaults (can be overridden in config.h)
#ifndef SD_WRITE_BUFFER_SIZE
#define SD_WRITE_BUFFER_SIZE 4096
#endif

#ifndef MIN_FREE_HEAP_BYTES
#define MIN_FREE_HEAP_BYTES 32768
#endif

/**
 * @brief Enumeration of SD card operation error types
 * 
 * Used for categorizing errors and determining appropriate retry strategies.
 */
enum SDCardError {
    SD_ERROR_NONE = 0,           ///< No error - operation successful
    SD_ERROR_NOT_MOUNTED,        ///< SD card not mounted
    SD_ERROR_CARD_REMOVED,       ///< SD card physically removed
    SD_ERROR_MOUNT_FAILED,       ///< Failed to mount SD card
    SD_ERROR_FILE_OPEN,          ///< Failed to open file
    SD_ERROR_FILE_WRITE,         ///< Failed to write to file
    SD_ERROR_FILE_READ,          ///< Failed to read from file
    SD_ERROR_DIR_CREATE,         ///< Failed to create directory
    SD_ERROR_DIR_OPEN,           ///< Failed to open directory
    SD_ERROR_CARD_FULL,          ///< SD card is full
    SD_ERROR_TIMEOUT,            ///< Operation timed out
    SD_ERROR_CORRUPTED,          ///< Data corruption detected
    SD_ERROR_UNKNOWN             ///< Unknown error
};

/**
 * @brief Structure to hold SD card operation result with error details
 */
struct SDOperationResult {
    bool success;                ///< Whether the operation succeeded
    SDCardError error;           ///< Error code if operation failed
    int retryCount;              ///< Number of retries attempted
    unsigned long operationTime; ///< Time taken for the operation in ms
    String errorMessage;         ///< Human-readable error description
};

/**
 * @brief Structure to hold SD card health statistics
 */
struct SDCardHealth {
    bool mounted;                ///< Whether SD card is currently mounted
    uint8_t cardType;            ///< Type of SD card (MMC, SD, SDHC)
    uint64_t totalBytes;         ///< Total capacity in bytes
    uint64_t usedBytes;          ///< Used space in bytes
    uint64_t freeBytes;          ///< Free space in bytes
    unsigned long lastHealthCheck; ///< Timestamp of last health check
    unsigned int consecutiveErrors; ///< Count of consecutive errors
    unsigned int totalErrors;    ///< Total error count since init
    unsigned int successfulOps;  ///< Total successful operations
    float errorRate;             ///< Error rate percentage
};

/**
 * @brief Structure to hold memory management statistics
 */
struct MemoryStats {
    size_t freeHeap;             ///< Current free heap memory
    size_t minFreeHeap;          ///< Minimum free heap since boot
    size_t largestFreeBlock;     ///< Largest contiguous free block
    float fragmentationPercent;  ///< Heap fragmentation percentage
    bool lowMemoryWarning;       ///< True if memory is critically low
};

/**
 * @brief Structure to hold image quality metrics for smart deletion
 */
struct ImageQualityInfo {
    String path;           ///< Full path to the image file
    size_t fileSize;       ///< File size in bytes
    uint32_t timestamp;    ///< Unix timestamp when image was captured
    float qualityScore;    ///< Calculated quality score (0.0-100.0)
    uint32_t hash;         ///< Simple hash for duplicate detection
    bool isValid;          ///< Whether the image file is valid
};

/**
 * @class StorageManager
 * @brief Manages SD card storage operations for wildlife camera images and metadata
 * 
 * This class handles all SD card operations including initialization, image saving,
 * metadata management, and storage monitoring. It provides automatic directory
 * organization by date, persistent image counting, and comprehensive error handling.
 * 
 * Key Features:
 * - SD card initialization using 1-bit SD_MMC mode (fewer pins required)
 * - Automatic date-based directory structure (/images/YYYYMMDD/)
 * - Timestamp-based filename generation (IMG_HHMMSS_XXX.jpg)
 * - Persistent image counter across device resets
 * - JSON metadata storage alongside images
 * - Storage space monitoring (free/used space reporting)
 * - Recursive image file scanning and listing
 * - Automatic filename collision avoidance
 * - Support for custom save paths
 * - Old file cleanup functionality
 * 
 * Directory Structure:
 * @code
 * /images/
 *   ├── 20241029/
 *   │   ├── IMG_143052_001.jpg
 *   │   ├── IMG_143052_001.json
 *   │   ├── IMG_143053_002.jpg
 *   │   └── IMG_143053_002.json
 *   └── 20241030/
 *       └── IMG_080015_003.jpg
 * @endcode
 * 
 * Example usage:
 * @code
 * StorageManager storage;
 * 
 * // Initialize SD card
 * if (storage.init()) {
 *     // Capture and save image
 *     camera_fb_t* fb = esp_camera_fb_get();
 *     String imagePath = storage.saveImage(fb);
 *     
 *     if (imagePath.length() > 0) {
 *         // Create metadata
 *         StaticJsonDocument<256> metadata;
 *         metadata["timestamp"] = "2024-10-29T14:30:52Z";
 *         metadata["temperature"] = 22.5;
 *         metadata["species"] = "Deer";
 *         
 *         // Save metadata
 *         storage.saveMetadata(imagePath, metadata);
 *         
 *         // Print storage information
 *         storage.printStorageInfo();
 *     }
 *     
 *     esp_camera_fb_return(fb);
 *     
 *     // Cleanup old files (keep last 7 days)
 *     storage.deleteOldFiles(7);
 * }
 * @endcode
 */
class StorageManager {
private:
    bool initialized;              ///< Flag indicating whether SD card is successfully initialized
    String basePath;               ///< Base directory path for image storage (default: "/images")
    unsigned long imageCounter;    ///< Persistent counter for unique image numbering
    Preferences preferences;       ///< Non-volatile storage for persistent counter across reboots
    int compressionQuality;        ///< JPEG compression quality setting (1-63)
    bool compressionEnabled;       ///< Whether automatic compression is enabled
    bool duplicateDetectionEnabled;///< Whether duplicate detection is enabled
    std::map<uint32_t, String> recentImageHashes; ///< Cache of recent image hashes for duplicate detection
    static const size_t MAX_HASH_CACHE_SIZE = 50; ///< Maximum number of hashes to keep in cache
    
    // Auto-cleanup policy members
    bool autoCleanupEnabled;       ///< Whether automatic cleanup is enabled
    unsigned long autoCleanupThresholdKB; ///< Free space threshold that triggers cleanup (KB)
    int autoCleanupRetentionDays;  ///< Number of days to keep files during cleanup
    unsigned long lastAutoCleanupTime; ///< Timestamp of last auto cleanup
    static const unsigned long AUTO_CLEANUP_INTERVAL_MS = 300000; ///< Minimum time between auto cleanups (5 min)
    
    // Error handling and retry logic members
    SDCardHealth cardHealth;       ///< Current SD card health statistics
    SDCardError lastError;         ///< Last error code encountered
    String lastErrorMessage;       ///< Human-readable description of last error
    unsigned int maxRetries;       ///< Maximum retry attempts for operations
    unsigned int retryDelayMs;     ///< Base delay between retries in milliseconds
    unsigned int maxRetryDelayMs;  ///< Maximum delay cap for exponential backoff
    bool autoRemountEnabled;       ///< Whether automatic remount is enabled
    
    // Memory management members
    uint8_t* writeBuffer;          ///< Pre-allocated write buffer to reduce fragmentation
    size_t writeBufferSize;        ///< Size of the write buffer
    bool memoryPoolEnabled;        ///< Whether memory pooling is enabled
    
    /**
     * @brief Check if automatic cleanup should run and execute if needed
     * 
     * Checks free space and runs cleanup if below threshold.
     * Respects minimum interval between cleanup runs.
     * 
     * @return bool true if cleanup ran successfully or wasn't needed
     */
    bool checkAndRunAutoCleanup();
    
    /**
     * @brief Get the current date-based path component
     * @return String Date path component (e.g., "/20241029")
     */
    String getCurrentDatePath();
    
    /**
     * @brief Generate unique filename for image storage
     * 
     * Creates a filename in the format IMG_HHMMSS_XXX.jpg where:
     * - HHMMSS is the current time (hours, minutes, seconds)
     * - XXX is the last 3 digits of the image counter for uniqueness
     * 
     * If system time is unavailable, uses counter-based naming: IMG_XXXXXXXX.jpg
     * 
     * @return String Generated filename (e.g., "IMG_143052_001.jpg")
     * @note Filename is guaranteed to be unique within the same second via counter
     */
    String generateFilename();
    
    /**
     * @brief Calculate a simple hash of image data for duplicate detection
     * 
     * Uses a sampling-based hash to quickly identify potential duplicates
     * without requiring full image comparison.
     * 
     * @param data Pointer to image data buffer
     * @param length Length of image data in bytes
     * @return uint32_t Hash value for the image
     */
    uint32_t calculateImageHash(const uint8_t* data, size_t length);
    
    /**
     * @brief Check if an image is a duplicate of a recently saved image
     * 
     * @param hash Hash value of the image to check
     * @return bool true if image appears to be a duplicate, false otherwise
     */
    bool isDuplicateImage(uint32_t hash);
    
    /**
     * @brief Calculate quality score for an image
     * 
     * Scores image quality based on file size, estimated sharpness,
     * and other metrics to help with smart deletion decisions.
     * 
     * @param data Pointer to image data buffer
     * @param length Length of image data in bytes
     * @return float Quality score from 0.0 to 100.0
     */
    float calculateQualityScore(const uint8_t* data, size_t length);
    
    /**
     * @brief Scan directory and collect image quality information
     * 
     * @param dirPath Path to directory to scan
     * @param images Vector to populate with image information
     */
    void collectImageInfo(const String& dirPath, std::vector<ImageQualityInfo>& images);
    
    /**
     * @brief Delete a file and its associated metadata file
     * 
     * @param imagePath Path to the image file to delete
     * @return bool true if deletion successful, false otherwise
     */
    bool deleteImageAndMetadata(const String& imagePath);
    
    /**
     * @brief Execute an SD card operation with automatic retry logic
     * 
     * Wraps SD card operations with exponential backoff retry logic.
     * Automatically handles transient errors and attempts recovery.
     * 
     * @tparam T Return type of the operation
     * @param operation Lambda function that performs the SD card operation
     * @param operationName Name of the operation for logging
     * @return SDOperationResult Result containing success status and error details
     */
    template<typename T>
    SDOperationResult executeWithRetry(std::function<T()> operation, const char* operationName);
    
    /**
     * @brief Attempt to remount the SD card after an error
     * 
     * Unmounts and remounts the SD card to recover from certain error states.
     * 
     * @return bool true if remount successful, false otherwise
     */
    bool remountSDCard();
    
    /**
     * @brief Check if the SD card is currently accessible
     * 
     * Performs a quick health check on the SD card without full initialization.
     * 
     * @return bool true if SD card is accessible, false otherwise
     */
    bool checkSDCardAccess();
    
    /**
     * @brief Update error statistics after an operation
     * 
     * @param success Whether the operation succeeded
     * @param error Error code if operation failed
     */
    void updateErrorStats(bool success, SDCardError error = SD_ERROR_NONE);
    
    /**
     * @brief Allocate the write buffer for memory pool operations
     * 
     * @return bool true if allocation successful, false otherwise
     */
    bool allocateWriteBuffer();
    
    /**
     * @brief Free the write buffer
     */
    void freeWriteBuffer();
    
    /**
     * @brief Write data to file using buffered writes to reduce fragmentation
     * 
     * @param file File handle to write to
     * @param data Data buffer to write
     * @param length Number of bytes to write
     * @return size_t Number of bytes actually written
     */
    size_t bufferedWrite(File& file, const uint8_t* data, size_t length);
    
    /**
     * @brief Convert SDCardError enum to human-readable string
     * 
     * @param error Error code to convert
     * @return const char* Human-readable error description
     */
    const char* errorToString(SDCardError error);

public:
    /**
     * @brief Default constructor
     * 
     * Initializes the StorageManager with default settings:
     * - initialized flag set to false
     * - basePath set to "/images"
     * - imageCounter set to 0 (will be loaded from preferences on init)
     */
    StorageManager();
    
    /**
     * @brief Initialize SD card and storage system
     * 
     * Performs the following initialization steps:
     * 1. Mounts SD card in 1-bit SD_MMC mode
     * 2. Verifies card presence and type (MMC, SD, SDHC)
     * 3. Displays card information (size, free space, used space)
     * 4. Creates base directory structure if needed
     * 5. Loads persistent image counter from preferences
     * 
     * @return bool true if initialization successful, false otherwise
     * @note Uses 1-bit SD_MMC mode to minimize pin usage
     * @note Should be called once during system startup before other operations
     * @note Prints detailed diagnostic information to Serial
     */
    bool init();
    
    /**
     * @brief Save camera frame buffer to SD card
     * 
     * Saves a JPEG image from the camera to the SD card with automatic
     * filename generation and directory organization. Handles duplicate
     * filenames by appending a counter suffix.
     * 
     * @param fb Pointer to camera frame buffer containing JPEG data
     * @param customPath Optional custom path for saving (overrides automatic naming)
     * @return String Full path of saved image, or empty string on failure
     * 
     * @note Frame buffer must contain valid JPEG data with non-NULL buffer and len > 0
     * @note Automatically creates date-based subdirectories
     * @note Increments and persists image counter after successful save
     * @note Maximum 1000 collision resolution attempts for filename uniqueness
     * @note Returns empty string if storage not initialized or frame buffer invalid
     * 
     * Example:
     * @code
     * camera_fb_t* fb = esp_camera_fb_get();
     * String path = storage.saveImage(fb);
     * if (path.length() > 0) {
     *     Serial.printf("Saved to: %s\n", path.c_str());
     * }
     * esp_camera_fb_return(fb);
     * @endcode
     */
    String saveImage(camera_fb_t* fb, const String& customPath = "");
    
    /**
     * @brief Save image metadata as JSON file
     * 
     * Creates a JSON file alongside the image with the same base name
     * but .json extension. Useful for storing capture information like
     * timestamp, temperature, GPS coordinates, detected species, etc.
     * 
     * @param imagePath Full path of the image file
     * @param metadata JSON document containing metadata to save
     * @return bool true if metadata saved successfully, false otherwise
     * 
     * @note JSON file is created with same name as image but .json extension
     * @note Example: /images/20241029/IMG_143052_001.jpg → IMG_143052_001.json
     * @note Returns false if storage not initialized or imagePath is empty
     * 
     * Example:
     * @code
     * StaticJsonDocument<512> metadata;
     * metadata["timestamp"] = "2024-10-29T14:30:52Z";
     * metadata["temperature"] = 22.5;
     * metadata["humidity"] = 65;
     * metadata["species"] = "Deer";
     * metadata["confidence"] = 0.95;
     * storage.saveMetadata(imagePath, metadata);
     * @endcode
     */
    bool saveMetadata(const String& imagePath, JsonDocument& metadata);
    
    /**
     * @brief Delete old files to free up storage space
     * 
     * Removes image files and associated metadata older than the specified
     * number of days to prevent SD card from filling up.
     * 
     * @param daysToKeep Number of days of files to keep (default: 7)
     * @return bool true if cleanup successful, false otherwise
     * 
     * @note Current implementation is a placeholder
     * @note Full implementation would parse date directories and remove old ones
     * @note Returns false if storage not initialized
     * @note Future implementation will recursively delete old date directories
     */
    bool deleteOldFiles(int daysToKeep = 7);
    
    /**
     * @brief Get available free space on SD card
     * 
     * @return unsigned long Free space in bytes, or 0 if not initialized
     * 
     * @note Return value is cast from uint64_t to unsigned long
     * @note May overflow for SD cards larger than 4GB (unsigned long limit)
     * @note Consider using uint64_t if accurate reporting needed for large cards
     * @note Returns 0 if storage not initialized
     */
    unsigned long getFreeSpace();
    
    /**
     * @brief Get used space on SD card
     * 
     * @return unsigned long Used space in bytes, or 0 if not initialized
     * 
     * @note Return value is cast from uint64_t to unsigned long
     * @note May overflow for SD cards larger than 4GB (unsigned long limit)
     * @note Consider using uint64_t if accurate reporting needed for large cards
     * @note Returns 0 if storage not initialized
     */
    unsigned long getUsedSpace();
    
    /**
     * @brief Print detailed storage information to Serial
     * 
     * Displays comprehensive storage statistics including:
     * - Base directory path
     * - Initialization status
     * - Current image counter value
     * - Total storage capacity (MB and bytes)
     * - Used storage space (MB and bytes)
     * - Free storage space (MB and bytes)
     * - Storage usage percentage
     * 
     * @note Output is formatted with clear section headers
     * @note All sizes shown in both megabytes and bytes
     * @note Prints "Storage not initialized" if init() not called
     * 
     * Example output:
     * @code
     * === Storage Information ===
     * Base Path: /images
     * Initialized: Yes
     * Image Counter: 42
     * Total Space: 7580 MB (7948206080 bytes)
     * Used Space: 156 MB (163577856 bytes)
     * Free Space: 7424 MB (7784628224 bytes)
     * Usage: 2.06%
     * ===========================
     * @endcode
     */
    void printStorageInfo();
    
    /**
     * @brief Get list of all image files on SD card
     * 
     * Recursively scans the base directory and all subdirectories to find
     * all JPEG image files (.jpg, .jpeg, .JPG, .JPEG extensions).
     * Results are sorted in reverse order (newest first by filename).
     * 
     * @return std::vector<String> Vector of full paths to image files
     * 
     * @note Returns empty vector if storage not initialized
     * @note Files are sorted in descending order (newest timestamp first)
     * @note Recursively scans all subdirectories under base path
     * @note Matches files with .jpg, .jpeg, .JPG, and .JPEG extensions
     * @note Can be memory intensive for SD cards with many images
     * 
     * Example:
     * @code
     * std::vector<String> images = storage.getImageFiles();
     * Serial.printf("Found %d images:\n", images.size());
     * for (const String& path : images) {
     *     Serial.println(path);
     * }
     * @endcode
     */
    std::vector<String> getImageFiles();
    
    /**
     * @brief Get total count of images saved
     * 
     * Returns the persistent image counter value, which tracks the total
     * number of images saved across all device power cycles and resets.
     * 
     * @return unsigned long Total number of images saved since counter reset
     * 
     * @note Counter is persistent across reboots via Preferences
     * @note Returns 0 if storage not initialized
     * @note Counter increments with each successful saveImage() call
     * @note Can be manually reset by clearing preferences
     */
    unsigned long getImageCount();
    
    // ============================================================================
    // Improved Storage Features
    // ============================================================================
    
    /**
     * @brief Enable or disable automatic image compression
     * 
     * When enabled, images will be checked and potentially re-compressed
     * if they exceed the target quality setting to save storage space.
     * 
     * @param enable true to enable compression, false to disable
     */
    void setCompressionEnabled(bool enable);
    
    /**
     * @brief Check if compression is enabled
     * 
     * @return bool true if compression is enabled, false otherwise
     */
    bool isCompressionEnabled() const;
    
    /**
     * @brief Set the target compression quality
     * 
     * @param quality JPEG quality value (1-63, lower is higher quality)
     * @note Values outside range will be clamped to valid range
     */
    void setCompressionQuality(int quality);
    
    /**
     * @brief Get the current compression quality setting
     * 
     * @return int Current JPEG quality value (1-63)
     */
    int getCompressionQuality() const;
    
    /**
     * @brief Enable or disable duplicate detection
     * 
     * When enabled, new images will be compared against recently saved
     * images to detect and skip duplicates, saving storage space.
     * 
     * @param enable true to enable duplicate detection, false to disable
     */
    void setDuplicateDetectionEnabled(bool enable);
    
    /**
     * @brief Check if duplicate detection is enabled
     * 
     * @return bool true if duplicate detection is enabled, false otherwise
     */
    bool isDuplicateDetectionEnabled() const;
    
    /**
     * @brief Clear the duplicate detection hash cache
     * 
     * Clears the internal cache of recent image hashes. Useful when
     * starting a new capture session where duplicates from previous
     * sessions should not be considered.
     */
    void clearDuplicateCache();
    
    /**
     * @brief Perform smart deletion to free up storage space
     * 
     * Analyzes all stored images and deletes lower quality duplicates
     * and less important images to free up space while keeping the
     * best quality unique images.
     * 
     * @param targetFreeSpaceKB Target free space in kilobytes to achieve
     * @return int Number of files deleted, or -1 on error
     * 
     * @note Uses quality scoring to determine which images to keep
     * @note Prefers keeping recent images over older ones
     * @note Will not delete the last N images regardless of quality
     * 
     * Example:
     * @code
     * // Free up at least 50MB of space
     * int deleted = storage.smartDelete(51200);
     * Serial.printf("Deleted %d files to free space\n", deleted);
     * @endcode
     */
    int smartDelete(unsigned long targetFreeSpaceKB = STORAGE_MIN_FREE_SPACE_KB);
    
    /**
     * @brief Save image with improved storage features
     * 
     * Enhanced version of saveImage that includes:
     * - Optional automatic compression
     * - Duplicate detection and skipping
     * - Automatic cleanup when storage is low
     * 
     * @param fb Pointer to camera frame buffer containing JPEG data
     * @param customPath Optional custom path for saving (overrides automatic naming)
     * @param skipDuplicates Whether to skip saving if duplicate detected (default: true)
     * @return String Full path of saved image, empty string on failure or duplicate
     * 
     * @note Returns empty string if image is detected as duplicate and skipDuplicates is true
     */
    String saveImageWithCompression(camera_fb_t* fb, const String& customPath = "", bool skipDuplicates = true);
    
    /**
     * @brief Get storage statistics including improved storage metrics
     * 
     * @param totalImages Output: Total number of images stored
     * @param duplicatesSkipped Output: Number of duplicates skipped (since boot)
     * @param bytesCompressed Output: Estimated bytes saved through compression (since boot)
     */
    void getStorageStats(unsigned long& totalImages, unsigned long& duplicatesSkipped, unsigned long& bytesCompressed);
    
    // ============================================================================
    // Automatic Cleanup Policy (In Progress → Complete)
    // ============================================================================
    
    /**
     * @brief Enable or disable automatic storage cleanup
     * 
     * When enabled, the StorageManager will automatically delete old files
     * when free space drops below the configured threshold. This prevents
     * the SD card from filling up during extended deployments.
     * 
     * @param enable true to enable auto-cleanup, false to disable
     */
    void setAutoCleanupEnabled(bool enable);
    
    /**
     * @brief Check if automatic cleanup is enabled
     * 
     * @return bool true if auto-cleanup is enabled
     */
    bool isAutoCleanupEnabled() const;
    
    /**
     * @brief Set the free space threshold that triggers cleanup
     * 
     * When free space drops below this threshold and auto-cleanup is
     * enabled, old files will be automatically deleted.
     * 
     * @param thresholdKB Free space threshold in kilobytes
     */
    void setAutoCleanupThreshold(unsigned long thresholdKB);
    
    /**
     * @brief Get the current auto-cleanup threshold
     * 
     * @return unsigned long Threshold in kilobytes
     */
    unsigned long getAutoCleanupThreshold() const;
    
    /**
     * @brief Set the retention period for auto-cleanup
     * 
     * Files older than this number of days will be deleted during
     * automatic cleanup operations.
     * 
     * @param days Number of days to retain files (1-365)
     */
    void setAutoCleanupRetentionDays(int days);
    
    /**
     * @brief Get the current retention period
     * 
     * @return int Number of days files are retained
     */
    int getAutoCleanupRetentionDays() const;
    
    /**
     * @brief Manually trigger cleanup based on current policy
     * 
     * Runs the auto-cleanup process immediately, regardless of
     * the current free space level.
     * 
     * @return int Number of files deleted, or -1 on error
     */
    int runCleanup();
    
    /**
     * @brief Get information about the last cleanup operation
     * 
     * @param filesDeleted Output: Number of files deleted
     * @param bytesFreed Output: Bytes freed by cleanup
     * @param lastCleanupTime Output: Timestamp of last cleanup
     */
    void getLastCleanupInfo(int& filesDeleted, unsigned long& bytesFreed, unsigned long& lastCleanupTime);
    
    // ============================================================================
    // SD Card Error Handling and Retry Logic
    // ============================================================================
    
    /**
     * @brief Get the last SD card error that occurred
     * 
     * @return SDCardError The most recent error code
     */
    SDCardError getLastError() const;
    
    /**
     * @brief Get a human-readable description of the last error
     * 
     * @return String Human-readable error message
     */
    String getLastErrorMessage() const;
    
    /**
     * @brief Clear the last error state
     * 
     * Resets the error state to SD_ERROR_NONE and clears the error message.
     */
    void clearLastError();
    
    /**
     * @brief Set the maximum number of retry attempts for SD card operations
     * 
     * @param retries Maximum retry attempts (1-10)
     */
    void setMaxRetries(unsigned int retries);
    
    /**
     * @brief Get the current maximum retry setting
     * 
     * @return unsigned int Current maximum retries
     */
    unsigned int getMaxRetries() const;
    
    /**
     * @brief Set the base delay between retry attempts
     * 
     * @param delayMs Delay in milliseconds (10-5000)
     */
    void setRetryDelay(unsigned int delayMs);
    
    /**
     * @brief Get the current retry delay setting
     * 
     * @return unsigned int Current retry delay in milliseconds
     */
    unsigned int getRetryDelay() const;
    
    /**
     * @brief Enable or disable automatic SD card remounting on errors
     * 
     * When enabled, the StorageManager will attempt to remount the SD card
     * after certain types of errors to recover from transient issues.
     * 
     * @param enable true to enable auto-remount, false to disable
     */
    void setAutoRemountEnabled(bool enable);
    
    /**
     * @brief Check if automatic remounting is enabled
     * 
     * @return bool true if auto-remount is enabled
     */
    bool isAutoRemountEnabled() const;
    
    /**
     * @brief Manually trigger an SD card remount
     * 
     * Unmounts and remounts the SD card. Useful for recovery from
     * certain error states or after physically removing/reinserting the card.
     * 
     * @return bool true if remount successful, false otherwise
     */
    bool forceRemount();
    
    /**
     * @brief Get current SD card health statistics
     * 
     * Returns a structure containing health metrics including error rates,
     * consecutive errors, and space usage.
     * 
     * @return SDCardHealth Current health statistics
     */
    SDCardHealth getSDCardHealth() const;
    
    /**
     * @brief Perform an SD card health check
     * 
     * Verifies the SD card is accessible and updates health statistics.
     * Should be called periodically or after suspected errors.
     * 
     * @return bool true if SD card is healthy, false if issues detected
     */
    bool performHealthCheck();
    
    /**
     * @brief Reset SD card error statistics
     * 
     * Clears the error counters and consecutive error count.
     * Does not affect the card mount state.
     */
    void resetErrorStats();
    
    // ============================================================================
    // Memory Management
    // ============================================================================
    
    /**
     * @brief Get current memory statistics
     * 
     * Returns information about heap memory usage and fragmentation.
     * Useful for monitoring memory health in long-running applications.
     * 
     * @return MemoryStats Current memory statistics
     */
    MemoryStats getMemoryStats() const;
    
    /**
     * @brief Enable or disable memory pool usage
     * 
     * When enabled, uses pre-allocated buffers to reduce memory fragmentation
     * during SD card operations.
     * 
     * @param enable true to enable memory pooling, false to disable
     */
    void setMemoryPoolEnabled(bool enable);
    
    /**
     * @brief Check if memory pool is enabled
     * 
     * @return bool true if memory pool is enabled
     */
    bool isMemoryPoolEnabled() const;
    
    /**
     * @brief Check if memory is critically low
     * 
     * Returns true if free heap memory is below the configured threshold.
     * 
     * @return bool true if memory is critically low
     */
    bool isLowMemory() const;
    
    /**
     * @brief Perform memory optimization
     * 
     * Attempts to reduce memory fragmentation by clearing caches
     * and releasing non-essential allocations.
     * 
     * @return size_t Approximate bytes freed
     */
    size_t optimizeMemory();
    
    /**
     * @brief Destructor
     * 
     * Cleans up allocated resources including the write buffer.
     */
    ~StorageManager();
};

#endif // STORAGE_MANAGER_H
