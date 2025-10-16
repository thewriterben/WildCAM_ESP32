/**
 * @file storage_manager.h
 * @brief Robust storage management with fallback mechanisms
 * @author thewriterben
 * @date 2025-09-02
 * @version 2.5.0
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <LittleFS.h>

// ===========================
// STORAGE CONFIGURATION
// ===========================
#define STORAGE_SD_ENABLED true
#define STORAGE_LITTLEFS_ENABLED true
#define STORAGE_FALLBACK_ENABLED true

#define MAX_FILENAME_LENGTH 64
#define MAX_PATH_LENGTH 256
#define STORAGE_RETRY_COUNT 3
#define STORAGE_TIMEOUT_MS 5000

// ===========================
// STORAGE PATHS (UNIFIED)
// ===========================
#define IMAGES_PATH "/wildlife/images"
#define LOGS_PATH "/wildlife/logs"
#define CONFIG_PATH "/wildlife/config"
#define DATA_PATH "/wildlife/data"
#define TEMP_PATH "/wildlife/temp"

// ===========================
// STORAGE TYPES
// ===========================
typedef enum {
    STORAGE_SD_CARD,
    STORAGE_LITTLEFS,
    STORAGE_NONE
} storage_type_t;

typedef enum {
    STORAGE_SUCCESS,
    STORAGE_ERROR_INIT,
    STORAGE_ERROR_WRITE,
    STORAGE_ERROR_READ,
    STORAGE_ERROR_FULL,
    STORAGE_ERROR_NOT_FOUND,
    STORAGE_ERROR_TIMEOUT
} storage_result_t;

// ===========================
// STORAGE MANAGER CLASS
// ===========================
class StorageManager {
public:
    StorageManager();
    ~StorageManager();
    
    // Initialization
    bool initialize();
    bool isReady();
    storage_type_t getActiveStorage();
    
    // File operations
    storage_result_t saveImage(const uint8_t* data, size_t length, const char* filename);
    storage_result_t saveLog(const char* message, const char* filename = nullptr);
    storage_result_t saveConfig(const char* key, const char* value);
    storage_result_t readConfig(const char* key, char* value, size_t max_length);
    
    // Directory operations
    bool createDirectories();
    bool cleanupOldFiles(uint32_t max_age_days = 30);
    
    // Status information
    uint64_t getUsedSpace();
    uint64_t getFreeSpace();
    uint64_t getTotalSpace();
    float getUsagePercentage();
    
    // Error handling
    const char* getLastError();
    void clearError();

private:
    storage_type_t m_activeStorage;
    char m_lastError[128];
    bool m_sdReady;
    bool m_littleFSReady;
    
    // Internal methods
    bool initializeSD();
    bool initializeLittleFS();
    bool switchToFallback();
    storage_result_t writeFile(const char* path, const uint8_t* data, size_t length);
    storage_result_t readFile(const char* path, uint8_t* buffer, size_t max_length, size_t* bytes_read);
    bool createPath(const char* path);
    void setError(const char* error);
    bool isSDAvailable();
    bool isLittleFSAvailable();
};

// ===========================
// GLOBAL STORAGE INSTANCE
// ===========================
extern StorageManager g_storage;

// ===========================
// CONVENIENCE MACROS
// ===========================
#define SAVE_IMAGE(data, length, filename) g_storage.saveImage(data, length, filename)
#define SAVE_LOG(message) g_storage.saveLog(message)
#define STORAGE_READY() g_storage.isReady()

#endif // STORAGE_MANAGER_H