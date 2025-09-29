#include "logger.h"
#include <time.h>

// Static member definitions
bool Logger::initialized = false;
bool Logger::fileLoggingEnabled = true;
bool Logger::serialLoggingEnabled = true;
Logger::Level Logger::currentLogLevel = Level::DEBUG;
String Logger::logFilePath = "/logs/system.log";

bool Logger::initialize(bool enableFileLogging, const String& logFilePath) {
    if (initialized) {
        return true;
    }

    Logger::fileLoggingEnabled = enableFileLogging;
    Logger::logFilePath = logFilePath;
    
    #if DEBUG_ENABLED
    currentLogLevel = Level::DEBUG;
    #else
    currentLogLevel = Level::INFO;
    #endif

    // Ensure log directory exists if file logging is enabled
    if (fileLoggingEnabled) {
        if (!ensureLogDirectory()) {
            Serial.println("Warning: Failed to create log directory, file logging disabled");
            fileLoggingEnabled = false;
        }
    }

    initialized = true;
    
    if (serialLoggingEnabled) {
        Serial.println("Logger initialized");
    }
    
    log(Level::INFO, "Logger system started");
    
    return true;
}

void Logger::debug(const String& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const String& message) {
    log(Level::INFO, message);
}

void Logger::warning(const String& message) {
    log(Level::WARNING, message);
}

void Logger::error(const String& message) {
    log(Level::ERROR, message);
}

void Logger::critical(const String& message) {
    log(Level::CRITICAL, message);
}

void Logger::log(Level level, const String& message) {
    if (!initialized || level < currentLogLevel) {
        return;
    }

    writeLog(level, message);
}

void Logger::setLogLevel(Level level) {
    currentLogLevel = level;
    log(Level::INFO, "Log level changed to: " + getLevelString(level));
}

void Logger::setFileLogging(bool enable) {
    fileLoggingEnabled = enable;
    log(Level::INFO, String("File logging ") + (enable ? "enabled" : "disabled"));
}

void Logger::setSerialLogging(bool enable) {
    serialLoggingEnabled = enable;
    if (enable) {
        Serial.println("Serial logging enabled");
    }
}

String Logger::getLevelString(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        case Level::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void Logger::flush() {
    if (serialLoggingEnabled) {
        Serial.flush();
    }
    // File flushing is handled automatically by SD library
}

void Logger::cleanup() {
    if (initialized) {
        log(Level::INFO, "Logger system shutdown");
        flush();
        initialized = false;
    }
}

void Logger::writeLog(Level level, const String& message) {
    String timestamp = getTimestamp();
    String levelStr = getLevelString(level);
    String logEntry = timestamp + " [" + levelStr + "] " + message;

    // Output to serial if enabled
    if (serialLoggingEnabled) {
        Serial.println(logEntry);
    }

    // Output to file if enabled
    if (fileLoggingEnabled) {
        File logFile = SD_MMC.open(logFilePath.c_str(), FILE_APPEND);
        if (logFile) {
            logFile.println(logEntry);
            logFile.close();
        } else if (serialLoggingEnabled) {
            Serial.println("Error: Failed to write to log file");
        }
    }
}

String Logger::getTimestamp() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    return String(timestamp);
}

bool Logger::ensureLogDirectory() {
    // Extract directory from log file path
    int lastSlash = logFilePath.lastIndexOf('/');
    if (lastSlash == -1) {
        return true; // No directory specified
    }

    String logDir = logFilePath.substring(0, lastSlash);
    
    if (SD_MMC.exists(logDir.c_str())) {
        return true;
    }

    return SD_MMC.mkdir(logDir.c_str());
}