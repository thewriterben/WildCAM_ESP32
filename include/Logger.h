#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <time.h>

/**
 * @file Logger.h
 * @brief Comprehensive logging system for WildCAM ESP32
 * 
 * This logging system provides:
 * - Multiple log levels (DEBUG, INFO, WARN, ERROR)
 * - Timestamps for each log entry
 * - Log filtering by level
 * - Optional SD card logging
 * - Function name and line number tracking
 * - Configurable via config.h
 */

//==============================================================================
// LOG LEVELS
//==============================================================================

/**
 * @brief Log level enumeration
 */
enum LogLevel {
    LOG_DEBUG = 0,  ///< Detailed debug information
    LOG_INFO = 1,   ///< General information messages
    LOG_WARN = 2,   ///< Warning messages
    LOG_ERROR = 3   ///< Error messages
};

//==============================================================================
// LOGGING MACROS
//==============================================================================

/**
 * @brief Log a debug message with file, function, and line number
 * @param msg Message to log (supports format strings)
 * @param ... Optional format arguments
 */
#define LOG_DEBUG(msg, ...) Logger::log(LOG_DEBUG, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)

/**
 * @brief Log an info message with file, function, and line number
 * @param msg Message to log (supports format strings)
 * @param ... Optional format arguments
 */
#define LOG_INFO(msg, ...) Logger::log(LOG_INFO, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)

/**
 * @brief Log a warning message with file, function, and line number
 * @param msg Message to log (supports format strings)
 * @param ... Optional format arguments
 */
#define LOG_WARN(msg, ...) Logger::log(LOG_WARN, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)

/**
 * @brief Log an error message with file, function, and line number
 * @param msg Message to log (supports format strings)
 * @param ... Optional format arguments
 */
#define LOG_ERROR(msg, ...) Logger::log(LOG_ERROR, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)

//==============================================================================
// LOGGER CLASS
//==============================================================================

/**
 * @brief Main logger class for system-wide logging
 */
class Logger {
public:
    /**
     * @brief Initialize the logger system
     * @param minLevel Minimum log level to display (filters out lower levels)
     * @param enableSerial Enable output to Serial console
     * @param enableSD Enable output to SD card
     * @param logFilePath Path to log file on SD card (default: "/system.log")
     * @return true if initialization successful, false otherwise
     */
    static bool init(LogLevel minLevel = LOG_INFO, 
                    bool enableSerial = true, 
                    bool enableSD = false,
                    const char* logFilePath = "/system.log");
    
    /**
     * @brief Log a message at specified level
     * @param level Log level
     * @param file Source file name
     * @param func Function name
     * @param line Line number
     * @param format Message format string (printf-style)
     * @param ... Format arguments
     */
    static void log(LogLevel level, const char* file, const char* func, 
                   int line, const char* format, ...);
    
    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     */
    static void setLogLevel(LogLevel level);
    
    /**
     * @brief Get the current minimum log level
     * @return Current minimum log level
     */
    static LogLevel getLogLevel();
    
    /**
     * @brief Enable or disable Serial logging
     * @param enable true to enable, false to disable
     */
    static void setSerialOutput(bool enable);
    
    /**
     * @brief Enable or disable SD card logging
     * @param enable true to enable, false to disable
     */
    static void setSDOutput(bool enable);
    
    /**
     * @brief Set the SD card log file path
     * @param path Path to log file
     */
    static void setLogFilePath(const char* path);
    
    /**
     * @brief Get a human-readable string for a log level
     * @param level Log level
     * @return String representation (e.g., "DEBUG", "INFO", "WARN", "ERROR")
     */
    static const char* levelToString(LogLevel level);
    
    /**
     * @brief Get formatted timestamp string
     * @param buffer Buffer to write timestamp to
     * @param bufferSize Size of buffer
     * @return Pointer to buffer
     */
    static char* getTimestamp(char* buffer, size_t bufferSize);
    
    /**
     * @brief Flush any pending logs to SD card
     */
    static void flush();
    
private:
    static LogLevel minLogLevel;        ///< Minimum log level to output
    static bool serialEnabled;          ///< Serial output enabled flag
    static bool sdEnabled;              ///< SD card output enabled flag
    static String logFilePath;          ///< Path to log file on SD card
    static bool initialized;            ///< Initialization flag
    
    /**
     * @brief Write a log entry to Serial
     * @param timestamp Timestamp string
     * @param levelStr Log level string
     * @param func Function name
     * @param line Line number
     * @param message Formatted message
     */
    static void writeToSerial(const char* timestamp, const char* levelStr,
                            const char* func, int line, const char* message);
    
    /**
     * @brief Write a log entry to SD card
     * @param timestamp Timestamp string
     * @param levelStr Log level string
     * @param func Function name
     * @param line Line number
     * @param message Formatted message
     */
    static void writeToSD(const char* timestamp, const char* levelStr,
                         const char* func, int line, const char* message);
};

#endif // LOGGER_H
