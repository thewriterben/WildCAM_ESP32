#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "../include/config.h"

/**
 * @brief Logging utility for ESP32 Wildlife Camera
 * 
 * Provides logging functionality with different levels,
 * file output, and debug message management.
 */
class Logger {
public:
    /**
     * @brief Log levels
     */
    enum class Level {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    /**
     * @brief Initialize logger
     * @param enableFileLogging Enable logging to SD card
     * @param logFilePath Log file path
     * @return true if initialization successful
     */
    static bool initialize(bool enableFileLogging = true, const String& logFilePath = "/logs/system.log");

    /**
     * @brief Log debug message
     * @param message Message to log
     */
    static void debug(const String& message);

    /**
     * @brief Log info message
     * @param message Message to log
     */
    static void info(const String& message);

    /**
     * @brief Log warning message
     * @param message Message to log
     */
    static void warning(const String& message);

    /**
     * @brief Log error message
     * @param message Message to log
     */
    static void error(const String& message);

    /**
     * @brief Log critical message
     * @param message Message to log
     */
    static void critical(const String& message);

    /**
     * @brief Log message with specified level
     * @param level Log level
     * @param message Message to log
     */
    static void log(Level level, const String& message);

    /**
     * @brief Set minimum log level
     * @param level Minimum level to log
     */
    static void setLogLevel(Level level);

    /**
     * @brief Enable or disable file logging
     * @param enable Enable file logging
     */
    static void setFileLogging(bool enable);

    /**
     * @brief Enable or disable serial logging
     * @param enable Enable serial logging
     */
    static void setSerialLogging(bool enable);

    /**
     * @brief Get log level string
     * @param level Log level
     * @return Level string
     */
    static String getLevelString(Level level);

    /**
     * @brief Flush log buffers
     */
    static void flush();

    /**
     * @brief Cleanup logger resources
     */
    static void cleanup();

private:
    static bool initialized;
    static bool fileLoggingEnabled;
    static bool serialLoggingEnabled;
    static Level currentLogLevel;
    static String logFilePath;

    /**
     * @brief Write log entry
     * @param level Log level
     * @param message Log message
     */
    static void writeLog(Level level, const String& message);

    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp
     */
    static String getTimestamp();

    /**
     * @brief Ensure log directory exists
     * @return true if directory exists or was created
     */
    static bool ensureLogDirectory();
};

// Convenience macros for logging
#if DEBUG_ENABLED
    #define LOG_DEBUG(msg) Logger::debug(msg)
    #define LOG_INFO(msg) Logger::info(msg)
    #define LOG_WARNING(msg) Logger::warning(msg)
    #define LOG_ERROR(msg) Logger::error(msg)
    #define LOG_CRITICAL(msg) Logger::critical(msg)
#else
    #define LOG_DEBUG(msg)
    #define LOG_INFO(msg) Logger::info(msg)
    #define LOG_WARNING(msg) Logger::warning(msg)
    #define LOG_ERROR(msg) Logger::error(msg)
    #define LOG_CRITICAL(msg) Logger::critical(msg)
#endif

#endif // LOGGER_H