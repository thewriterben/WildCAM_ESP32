/**
 * @file logger.cpp
 * @brief Logger implementation with SD card and LittleFS support
 * @author thewriterben
 * @date 2025-08-31
 */

#include "logger.h"
#include <SD_MMC.h>
#include <LittleFS.h>
#include <time.h>

// Static member initialization
Logger::LogLevel Logger::s_currentLevel = Logger::LEVEL_INFO;
bool Logger::s_serialEnabled = true;
bool Logger::s_fileEnabled = false;
String Logger::s_logFilePath = "/wildlife/logs/system.log";
bool Logger::s_sdCardAvailable = false;
bool Logger::s_littleFSAvailable = false;

void Logger::setLevel(LogLevel level) {
    s_currentLevel = level;
}

void Logger::enableSerial(bool enable) {
    s_serialEnabled = enable;
}

void Logger::enableFile(bool enable) {
    s_fileEnabled = enable;
    if (enable && !s_sdCardAvailable && !s_littleFSAvailable) {
        initializeStorage();
    }
}

bool Logger::initializeStorage() {
    // Try SD card first
    if (SD_MMC.begin()) {
        uint8_t cardType = SD_MMC.cardType();
        if (cardType != CARD_NONE) {
            s_sdCardAvailable = true;
            
            // Create log directory
            SD_MMC.mkdir("/wildlife");
            SD_MMC.mkdir("/wildlife/logs");
            
            return true;
        } else {
            SD_MMC.end();
        }
    }
    
    // Fallback to LittleFS
    if (!LittleFS.begin()) {
        // Try formatting
        if (LittleFS.format()) {
            if (LittleFS.begin()) {
                s_littleFSAvailable = true;
                return true;
            }
        }
        return false;
    } else {
        s_littleFSAvailable = true;
        return true;
    }
}

bool Logger::isStorageAvailable() {
    return s_sdCardAvailable || s_littleFSAvailable;
}

void Logger::setLogFile(const char* path) {
    s_logFilePath = String(path);
}

void Logger::debug(const char* format, ...) {
    if (s_currentLevel > LEVEL_DEBUG) return;
    
    va_list args;
    va_start(args, format);
    log(LEVEL_DEBUG, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    if (s_currentLevel > LEVEL_INFO) return;
    
    va_list args;
    va_start(args, format);
    log(LEVEL_INFO, format, args);
    va_end(args);
}

void Logger::warning(const char* format, ...) {
    if (s_currentLevel > LEVEL_WARNING) return;
    
    va_list args;
    va_start(args, format);
    log(LEVEL_WARNING, format, args);
    va_end(args);
}

void Logger::error(const char* format, ...) {
    if (s_currentLevel > LEVEL_ERROR) return;
    
    va_list args;
    va_start(args, format);
    log(LEVEL_ERROR, format, args);
    va_end(args);
}

void Logger::critical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LEVEL_CRITICAL, format, args);
    va_end(args);
}

void Logger::log(LogLevel level, const char* format, va_list args) {
    String timestamp = getTimestamp();
    String levelStr = getLevelString(level);
    
    // Format the message
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    String logLine = timestamp + " [" + levelStr + "] " + String(buffer);
    
    // Output to serial
    if (s_serialEnabled) {
        Serial.println(logLine);
    }
    
    // Output to file
    if (s_fileEnabled) {
        writeToFile(logLine);
    }
}

bool Logger::writeToFile(const String& logLine) {
    if (!s_fileEnabled) {
        return false;
    }
    
    // Try SD card first
    if (s_sdCardAvailable) {
        File logFile = SD_MMC.open(s_logFilePath.c_str(), FILE_APPEND);
        if (logFile) {
            logFile.println(logLine);
            logFile.close();
            return true;
        } else {
            // SD card failed, try to reinitialize
            s_sdCardAvailable = false;
            if (!s_littleFSAvailable) {
                initializeStorage();
            }
        }
    }
    
    // Try LittleFS as fallback
    if (s_littleFSAvailable) {
        File logFile = LittleFS.open(s_logFilePath.c_str(), "a");
        if (logFile) {
            logFile.println(logLine);
            logFile.close();
            return true;
        } else {
            // LittleFS failed
            s_littleFSAvailable = false;
        }
    }
    
    // If we get here, both storage methods failed
    // Try to reinitialize storage
    initializeStorage();
    return false;
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LEVEL_DEBUG: return "DEBUG";
        case LEVEL_INFO: return "INFO";
        case LEVEL_WARNING: return "WARN";
        case LEVEL_ERROR: return "ERROR";
        case LEVEL_CRITICAL: return "CRIT";
        default: return "UNKNOWN";
    }
}

String Logger::getTimestamp() {
    char buffer[32];
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu.%03lu",
             hours % 24, minutes % 60, seconds % 60, ms % 1000);
    
    return String(buffer);
}

void Logger::hexDump(const uint8_t* data, size_t len, const char* label) {
    if (s_currentLevel > LEVEL_DEBUG) return;
    
    if (label) {
        debug("HexDump: %s (%d bytes)", label, len);
    }
    
    char buffer[80];
    for (size_t i = 0; i < len; i += 16) {
        sprintf(buffer, "%04X: ", i);
        
        // Hex bytes
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                sprintf(buffer + strlen(buffer), "%02X ", data[i + j]);
            } else {
                strcat(buffer, "   ");
            }
        }
        
        strcat(buffer, " ");
        
        // ASCII representation
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                char c = data[i + j];
                sprintf(buffer + strlen(buffer), "%c", (c >= 32 && c < 127) ? c : '.');
            }
        }
        
        debug("%s", buffer);
    }
}