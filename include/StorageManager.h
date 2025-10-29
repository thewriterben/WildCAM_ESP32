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
};

#endif // STORAGE_MANAGER_H
