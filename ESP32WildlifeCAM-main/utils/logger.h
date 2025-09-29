/**
 * @file logger.h
 * @brief Logging utility for ESP32WildlifeCAM
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

class Logger {
public:
    enum LogLevel {
        LEVEL_DEBUG = 0,
        LEVEL_INFO,
        LEVEL_WARNING,
        LEVEL_ERROR,
        LEVEL_CRITICAL
    };
    
    static void setLevel(LogLevel level);
    static void enableSerial(bool enable);
    static void enableFile(bool enable);
    static void setLogFile(const char* path);
    
    // Storage initialization
    static bool initializeStorage();
    static bool isStorageAvailable();
    
    static void debug(const char* format, ...);
    static void info(const char* format, ...);
    static void warning(const char* format, ...);
    static void error(const char* format, ...);
    static void critical(const char* format, ...);
    
    static void hexDump(const uint8_t* data, size_t len, const char* label = nullptr);
    
private:
    static LogLevel s_currentLevel;
    static bool s_serialEnabled;
    static bool s_fileEnabled;
    static String s_logFilePath;
    static bool s_sdCardAvailable;
    static bool s_littleFSAvailable;
    
    static void log(LogLevel level, const char* format, va_list args);
    static const char* getLevelString(LogLevel level);
    static String getTimestamp();
    static bool writeToFile(const String& logLine);
};

#endif // LOGGER_H