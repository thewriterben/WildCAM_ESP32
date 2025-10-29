/**
 * @file Logger.cpp
 * @brief Implementation of comprehensive logging system
 */

#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>

// Initialize static members
LogLevel Logger::minLogLevel = LOG_INFO;
bool Logger::serialEnabled = true;
bool Logger::sdEnabled = false;
String Logger::logFilePath = "/system.log";
bool Logger::initialized = false;

bool Logger::init(LogLevel minLevel, bool enableSerial, bool enableSD, const char* logPath) {
    minLogLevel = minLevel;
    serialEnabled = enableSerial;
    sdEnabled = enableSD;
    logFilePath = String(logPath);
    initialized = true;
    
    if (serialEnabled) {
        Serial.println("Logger initialized");
        Serial.printf("  Log level: %s\n", levelToString(minLevel));
        Serial.printf("  Serial output: %s\n", enableSerial ? "enabled" : "disabled");
        Serial.printf("  SD output: %s\n", enableSD ? "enabled" : "disabled");
        if (enableSD) {
            Serial.printf("  Log file: %s\n", logPath);
        }
    }
    
    return true;
}

void Logger::log(LogLevel level, const char* file, const char* func, 
                int line, const char* format, ...) {
    // Check if logging is enabled at this level
    if (!initialized || level < minLogLevel) {
        return;
    }
    
    // Get timestamp
    char timestamp[32];
    getTimestamp(timestamp, sizeof(timestamp));
    
    // Get level string
    const char* levelStr = levelToString(level);
    
    // Format the message
    char message[256];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Extract just the filename from the full path
    const char* filename = strrchr(file, '/');
    if (filename) {
        filename++; // Move past the '/'
    } else {
        filename = strrchr(file, '\\');
        if (filename) {
            filename++; // Move past the '\'
        } else {
            filename = file; // No path separator found
        }
    }
    
    // Output to Serial if enabled
    if (serialEnabled) {
        writeToSerial(timestamp, levelStr, func, line, message);
    }
    
    // Output to SD card if enabled
    if (sdEnabled) {
        writeToSD(timestamp, levelStr, func, line, message);
    }
}

void Logger::setLogLevel(LogLevel level) {
    minLogLevel = level;
}

LogLevel Logger::getLogLevel() {
    return minLogLevel;
}

void Logger::setSerialOutput(bool enable) {
    serialEnabled = enable;
}

void Logger::setSDOutput(bool enable) {
    sdEnabled = enable;
}

void Logger::setLogFilePath(const char* path) {
    logFilePath = String(path);
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        default:        return "UNKNOWN";
    }
}

char* Logger::getTimestamp(char* buffer, size_t bufferSize) {
    // Use millis() for timestamp since RTC may not be available
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    hours = hours % 24;
    minutes = minutes % 60;
    seconds = seconds % 60;
    ms = ms % 1000;
    
    snprintf(buffer, bufferSize, "%lud %02lu:%02lu:%02lu.%03lu", 
             days, hours, minutes, seconds, ms);
    
    return buffer;
}

void Logger::writeToSerial(const char* timestamp, const char* levelStr,
                          const char* func, int line, const char* message) {
    // Format: [TIMESTAMP] [LEVEL] [func:line] message
    Serial.printf("[%s] [%s] [%s:%d] %s\n", 
                 timestamp, levelStr, func, line, message);
}

void Logger::writeToSD(const char* timestamp, const char* levelStr,
                      const char* func, int line, const char* message) {
    // Try to open the log file in append mode
    File logFile = SD_MMC.open(logFilePath.c_str(), FILE_APPEND);
    if (!logFile) {
        // If we can't open the file, silently fail to avoid infinite recursion
        return;
    }
    
    // Write log entry to file
    logFile.printf("[%s] [%s] [%s:%d] %s\n", 
                  timestamp, levelStr, func, line, message);
    
    // Close the file to ensure data is written
    logFile.close();
}

void Logger::flush() {
    // For future use - could implement buffered logging
    // Currently logs are written immediately to SD card
}
