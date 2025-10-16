/**
 * @file storage_manager.cpp
 * @brief Robust storage management implementation with fallback mechanisms
 * @author thewriterben
 * @date 2025-09-02
 * @version 2.5.0
 */

#include "storage_manager.h"
#include "../utils/logger.h"

// Global storage instance
StorageManager g_storage;

StorageManager::StorageManager() 
    : m_activeStorage(STORAGE_NONE), m_sdReady(false), m_littleFSReady(false) {
    memset(m_lastError, 0, sizeof(m_lastError));
}

StorageManager::~StorageManager() {
    // Cleanup if needed
}

bool StorageManager::initialize() {
    Logger::info("Initializing storage system...");
    
    // Try SD card first
    if (initializeSD()) {
        m_activeStorage = STORAGE_SD_CARD;
        Logger::info("Using SD card as primary storage");
    } else if (initializeLittleFS()) {
        m_activeStorage = STORAGE_LITTLEFS;
        Logger::warning("SD card unavailable, using LittleFS as fallback");
    } else {
        setError("No storage available - both SD and LittleFS failed");
        Logger::error(m_lastError);
        return false;
    }
    
    // Create required directories
    createDirectories();
    
    Logger::info("Storage system initialized successfully");
    return true;
}

bool StorageManager::initializeSD() {
    #if STORAGE_SD_ENABLED
    if (SD_MMC.begin()) {
        uint8_t cardType = SD_MMC.cardType();
        if (cardType != CARD_NONE) {
            uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
            Logger::info("SD Card initialized: %lluMB", cardSize);
            m_sdReady = true;
            return true;
        } else {
            Logger::warning("SD Card detected but not accessible");
            SD_MMC.end();
        }
    } else {
        Logger::warning("SD Card initialization failed");
    }
    #endif
    
    m_sdReady = false;
    return false;
}

bool StorageManager::initializeLittleFS() {
    #if STORAGE_LITTLEFS_ENABLED
    if (!LittleFS.begin()) {
        Logger::warning("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            if (LittleFS.begin()) {
                Logger::info("LittleFS formatted and mounted as fallback storage");
                m_littleFSReady = true;
                return true;
            } else {
                Logger::error("LittleFS mount failed after format");
            }
        } else {
            Logger::error("LittleFS format failed");
        }
    } else {
        Logger::info("LittleFS mounted successfully");
        m_littleFSReady = true;
        return true;
    }
    #endif
    
    m_littleFSReady = false;
    return false;
}

bool StorageManager::createDirectories() {
    const char* dirs[] = {IMAGES_PATH, LOGS_PATH, CONFIG_PATH, DATA_PATH, TEMP_PATH};
    
    for (const char* dir : dirs) {
        if (m_activeStorage == STORAGE_SD_CARD && m_sdReady) {
            SD_MMC.mkdir(dir);
        } else if (m_activeStorage == STORAGE_LITTLEFS && m_littleFSReady) {
            // LittleFS doesn't need explicit mkdir
        }
    }
    
    return true;
}

storage_result_t StorageManager::saveImage(const uint8_t* data, size_t length, const char* filename) {
    if (!isReady()) {
        return STORAGE_ERROR_INIT;
    }
    
    char fullPath[MAX_PATH_LENGTH];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", IMAGES_PATH, filename);
    
    return writeFile(fullPath, data, length);
}

storage_result_t StorageManager::saveLog(const char* message, const char* filename) {
    if (!isReady()) {
        return STORAGE_ERROR_INIT;
    }
    
    char fullPath[MAX_PATH_LENGTH];
    if (filename) {
        snprintf(fullPath, sizeof(fullPath), "%s/%s", LOGS_PATH, filename);
    } else {
        // Generate timestamp-based filename
        snprintf(fullPath, sizeof(fullPath), "%s/log_%lu.txt", LOGS_PATH, millis());
    }
    
    return writeFile(fullPath, (const uint8_t*)message, strlen(message));
}

storage_result_t StorageManager::writeFile(const char* path, const uint8_t* data, size_t length) {
    File file;
    
    if (m_activeStorage == STORAGE_SD_CARD && m_sdReady) {
        file = SD_MMC.open(path, FILE_WRITE);
    } else if (m_activeStorage == STORAGE_LITTLEFS && m_littleFSReady) {
        file = LittleFS.open(path, FILE_WRITE);
    } else {
        return STORAGE_ERROR_INIT;
    }
    
    if (!file) {
        setError("Failed to open file for writing");
        return STORAGE_ERROR_WRITE;
    }
    
    size_t written = file.write(data, length);
    file.close();
    
    if (written != length) {
        setError("Failed to write complete data");
        return STORAGE_ERROR_WRITE;
    }
    
    return STORAGE_SUCCESS;
}

bool StorageManager::isReady() {
    return (m_activeStorage != STORAGE_NONE) && 
           ((m_activeStorage == STORAGE_SD_CARD && m_sdReady) ||
            (m_activeStorage == STORAGE_LITTLEFS && m_littleFSReady));
}

storage_type_t StorageManager::getActiveStorage() {
    return m_activeStorage;
}

const char* StorageManager::getLastError() {
    return m_lastError;
}

void StorageManager::setError(const char* error) {
    strncpy(m_lastError, error, sizeof(m_lastError) - 1);
    m_lastError[sizeof(m_lastError) - 1] = '\0';
}

void StorageManager::clearError() {
    memset(m_lastError, 0, sizeof(m_lastError));
}

uint64_t StorageManager::getFreeSpace() {
    if (m_activeStorage == STORAGE_SD_CARD && m_sdReady) {
        return SD_MMC.totalBytes() - SD_MMC.usedBytes();
    } else if (m_activeStorage == STORAGE_LITTLEFS && m_littleFSReady) {
        return LittleFS.totalBytes() - LittleFS.usedBytes();
    }
    return 0;
}

uint64_t StorageManager::getTotalSpace() {
    if (m_activeStorage == STORAGE_SD_CARD && m_sdReady) {
        return SD_MMC.totalBytes();
    } else if (m_activeStorage == STORAGE_LITTLEFS && m_littleFSReady) {
        return LittleFS.totalBytes();
    }
    return 0;
}

float StorageManager::getUsagePercentage() {
    uint64_t total = getTotalSpace();
    if (total == 0) return 0.0f;
    
    uint64_t used = total - getFreeSpace();
    return (float)used / total * 100.0f;
}