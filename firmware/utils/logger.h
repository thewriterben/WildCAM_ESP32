/**
 * @file logger.h
 * @brief Simple logging utility for WildCAM ESP32
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/**
 * @class Logger
 * @brief Simple serial logging with levels
 */
class Logger {
public:
    static void begin() {
        // Serial already initialized in main
    }
    
    static void info(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Serial.print("[INFO] ");
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.println(buffer);
        va_end(args);
    }
    
    static void warning(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Serial.print("[WARN] ");
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.println(buffer);
        va_end(args);
    }
    
    static void error(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Serial.print("[ERROR] ");
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.println(buffer);
        va_end(args);
    }
};

#endif // LOGGER_H
