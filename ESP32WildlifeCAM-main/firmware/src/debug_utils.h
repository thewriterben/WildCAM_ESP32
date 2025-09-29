/**
 * ESP32 Wildlife Camera - Enhanced Debug Utilities
 * 
 * Provides comprehensive debugging capabilities including:
 * - Categorized logging with component-specific levels
 * - Timestamped debug output for timing analysis
 * - Memory and performance monitoring
 * - Runtime debug level control
 * - System health monitoring utilities
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.1.0 (Enhanced Debug)
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// Debug Categories
enum DebugCategory {
    DEBUG_SYSTEM    = 0,
    DEBUG_CAMERA    = 1,
    DEBUG_MOTION    = 2,
    DEBUG_POWER     = 3,
    DEBUG_WIFI      = 4,
    DEBUG_LORA      = 5,
    DEBUG_AI        = 6,
    DEBUG_MEMORY    = 7,
    DEBUG_SENSOR    = 8,
    DEBUG_NETWORK   = 9,
    DEBUG_COUNT     = 10  // Total number of categories
};

// Debug Levels
enum DebugLevel {
    DEBUG_LEVEL_NONE    = 0,
    DEBUG_LEVEL_ERROR   = 1,
    DEBUG_LEVEL_WARN    = 2,
    DEBUG_LEVEL_INFO    = 3,
    DEBUG_LEVEL_DEBUG   = 4,
    DEBUG_LEVEL_VERBOSE = 5
};

// Debug formatting options
struct DebugConfig {
    bool enableTimestamp;
    bool enableCategory;
    bool enableLevel;
    bool enableColor;
    bool enableMemoryInfo;
    uint32_t bufferSize;
    
    DebugConfig() : enableTimestamp(true), enableCategory(true), 
                   enableLevel(true), enableColor(false), 
                   enableMemoryInfo(false), bufferSize(512) {}
};

/**
 * Enhanced Debug System Class
 * Provides categorized, timestamped logging with runtime level control
 */
class EnhancedDebugSystem {
public:
    static void init();
    static void setLevel(DebugCategory category, DebugLevel level);
    static void setGlobalLevel(DebugLevel level);
    static DebugLevel getLevel(DebugCategory category);
    
    static void setConfig(const DebugConfig& config);
    static DebugConfig getConfig();
    
    // Core debug functions
    static void print(DebugCategory category, DebugLevel level, const char* message);
    static void printf(DebugCategory category, DebugLevel level, const char* format, ...);
    static void println(DebugCategory category, DebugLevel level, const char* message);
    
    // Convenience functions for each level
    static void error(DebugCategory category, const char* format, ...);
    static void warn(DebugCategory category, const char* format, ...);
    static void info(DebugCategory category, const char* format, ...);
    static void debug(DebugCategory category, const char* format, ...);
    static void verbose(DebugCategory category, const char* format, ...);
    
    // System monitoring
    static void printMemoryInfo();
    static void printSystemInfo();
    static void printNetworkInfo();
    static void printTaskInfo();
    
    // Performance monitoring
    static void startTimer(const char* name);
    static void endTimer(const char* name);
    static void printTimers();
    
    // Utility functions
    static const char* categoryToString(DebugCategory category);
    static const char* levelToString(DebugLevel level);
    
private:
    static DebugLevel categoryLevels[DEBUG_COUNT];
    static DebugConfig config;
    static char buffer[1024];
    static bool initialized;
    
    // Timer tracking
    struct Timer {
        char name[32];
        unsigned long startTime;
        unsigned long totalTime;
        uint32_t count;
        bool active;
    };
    static Timer timers[16];
    static uint8_t timerCount;
    
    static void formatMessage(char* output, size_t maxLen, DebugCategory category, 
                             DebugLevel level, const char* message);
    static unsigned long getTimestamp();
    static int findTimer(const char* name);
};

// Enhanced debug macros that replace the basic ones
#if DEBUG_ENABLED

// Category-specific debug macros
#define DEBUG_SYSTEM_ERROR(format, ...)   EnhancedDebugSystem::error(DEBUG_SYSTEM, format, ##__VA_ARGS__)
#define DEBUG_SYSTEM_WARN(format, ...)    EnhancedDebugSystem::warn(DEBUG_SYSTEM, format, ##__VA_ARGS__)
#define DEBUG_SYSTEM_INFO(format, ...)    EnhancedDebugSystem::info(DEBUG_SYSTEM, format, ##__VA_ARGS__)
#define DEBUG_SYSTEM_DEBUG(format, ...)   EnhancedDebugSystem::debug(DEBUG_SYSTEM, format, ##__VA_ARGS__)

#define DEBUG_CAMERA_ERROR(format, ...)   EnhancedDebugSystem::error(DEBUG_CAMERA, format, ##__VA_ARGS__)
#define DEBUG_CAMERA_WARN(format, ...)    EnhancedDebugSystem::warn(DEBUG_CAMERA, format, ##__VA_ARGS__)
#define DEBUG_CAMERA_INFO(format, ...)    EnhancedDebugSystem::info(DEBUG_CAMERA, format, ##__VA_ARGS__)
#define DEBUG_CAMERA_DEBUG(format, ...)   EnhancedDebugSystem::debug(DEBUG_CAMERA, format, ##__VA_ARGS__)

#define DEBUG_MOTION_ERROR(format, ...)   EnhancedDebugSystem::error(DEBUG_MOTION, format, ##__VA_ARGS__)
#define DEBUG_MOTION_WARN(format, ...)    EnhancedDebugSystem::warn(DEBUG_MOTION, format, ##__VA_ARGS__)
#define DEBUG_MOTION_INFO(format, ...)    EnhancedDebugSystem::info(DEBUG_MOTION, format, ##__VA_ARGS__)
#define DEBUG_MOTION_DEBUG(format, ...)   EnhancedDebugSystem::debug(DEBUG_MOTION, format, ##__VA_ARGS__)

#define DEBUG_POWER_ERROR(format, ...)    EnhancedDebugSystem::error(DEBUG_POWER, format, ##__VA_ARGS__)
#define DEBUG_POWER_WARN(format, ...)     EnhancedDebugSystem::warn(DEBUG_POWER, format, ##__VA_ARGS__)
#define DEBUG_POWER_INFO(format, ...)     EnhancedDebugSystem::info(DEBUG_POWER, format, ##__VA_ARGS__)
#define DEBUG_POWER_DEBUG(format, ...)    EnhancedDebugSystem::debug(DEBUG_POWER, format, ##__VA_ARGS__)

#define DEBUG_WIFI_ERROR(format, ...)     EnhancedDebugSystem::error(DEBUG_WIFI, format, ##__VA_ARGS__)
#define DEBUG_WIFI_WARN(format, ...)      EnhancedDebugSystem::warn(DEBUG_WIFI, format, ##__VA_ARGS__)
#define DEBUG_WIFI_INFO(format, ...)      EnhancedDebugSystem::info(DEBUG_WIFI, format, ##__VA_ARGS__)
#define DEBUG_WIFI_DEBUG(format, ...)     EnhancedDebugSystem::debug(DEBUG_WIFI, format, ##__VA_ARGS__)

#define DEBUG_LORA_ERROR(format, ...)     EnhancedDebugSystem::error(DEBUG_LORA, format, ##__VA_ARGS__)
#define DEBUG_LORA_WARN(format, ...)      EnhancedDebugSystem::warn(DEBUG_LORA, format, ##__VA_ARGS__)
#define DEBUG_LORA_INFO(format, ...)      EnhancedDebugSystem::info(DEBUG_LORA, format, ##__VA_ARGS__)
#define DEBUG_LORA_DEBUG(format, ...)     EnhancedDebugSystem::debug(DEBUG_LORA, format, ##__VA_ARGS__)

#define DEBUG_AI_ERROR(format, ...)       EnhancedDebugSystem::error(DEBUG_AI, format, ##__VA_ARGS__)
#define DEBUG_AI_WARN(format, ...)        EnhancedDebugSystem::warn(DEBUG_AI, format, ##__VA_ARGS__)
#define DEBUG_AI_INFO(format, ...)        EnhancedDebugSystem::info(DEBUG_AI, format, ##__VA_ARGS__)
#define DEBUG_AI_DEBUG(format, ...)       EnhancedDebugSystem::debug(DEBUG_AI, format, ##__VA_ARGS__)

#define DEBUG_MEMORY_ERROR(format, ...)   EnhancedDebugSystem::error(DEBUG_MEMORY, format, ##__VA_ARGS__)
#define DEBUG_MEMORY_WARN(format, ...)    EnhancedDebugSystem::warn(DEBUG_MEMORY, format, ##__VA_ARGS__)
#define DEBUG_MEMORY_INFO(format, ...)    EnhancedDebugSystem::info(DEBUG_MEMORY, format, ##__VA_ARGS__)
#define DEBUG_MEMORY_DEBUG(format, ...)   EnhancedDebugSystem::debug(DEBUG_MEMORY, format, ##__VA_ARGS__)

// Performance timing macros
#define DEBUG_TIMER_START(name)           EnhancedDebugSystem::startTimer(name)
#define DEBUG_TIMER_END(name)             EnhancedDebugSystem::endTimer(name)
#define DEBUG_TIMER_PRINT()               EnhancedDebugSystem::printTimers()

// System monitoring macros
#define DEBUG_PRINT_MEMORY()              EnhancedDebugSystem::printMemoryInfo()
#define DEBUG_PRINT_SYSTEM()              EnhancedDebugSystem::printSystemInfo()
#define DEBUG_PRINT_NETWORK()             EnhancedDebugSystem::printNetworkInfo()
#define DEBUG_PRINT_TASKS()               EnhancedDebugSystem::printTaskInfo()

// Backward compatibility macros (enhanced versions of existing ones)
#define DEBUG_PRINT(x)      EnhancedDebugSystem::print(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, x)
#define DEBUG_PRINTLN(x)    EnhancedDebugSystem::println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, x)
#define DEBUG_PRINTF(format, ...) EnhancedDebugSystem::printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, format, ##__VA_ARGS__)

#else

// Disabled debug macros when DEBUG_ENABLED is false
#define DEBUG_SYSTEM_ERROR(format, ...)   
#define DEBUG_SYSTEM_WARN(format, ...)    
#define DEBUG_SYSTEM_INFO(format, ...)    
#define DEBUG_SYSTEM_DEBUG(format, ...)   

#define DEBUG_CAMERA_ERROR(format, ...)   
#define DEBUG_CAMERA_WARN(format, ...)    
#define DEBUG_CAMERA_INFO(format, ...)    
#define DEBUG_CAMERA_DEBUG(format, ...)   

#define DEBUG_MOTION_ERROR(format, ...)   
#define DEBUG_MOTION_WARN(format, ...)    
#define DEBUG_MOTION_INFO(format, ...)    
#define DEBUG_MOTION_DEBUG(format, ...)   

#define DEBUG_POWER_ERROR(format, ...)    
#define DEBUG_POWER_WARN(format, ...)     
#define DEBUG_POWER_INFO(format, ...)     
#define DEBUG_POWER_DEBUG(format, ...)    

#define DEBUG_WIFI_ERROR(format, ...)     
#define DEBUG_WIFI_WARN(format, ...)      
#define DEBUG_WIFI_INFO(format, ...)      
#define DEBUG_WIFI_DEBUG(format, ...)     

#define DEBUG_LORA_ERROR(format, ...)     
#define DEBUG_LORA_WARN(format, ...)      
#define DEBUG_LORA_INFO(format, ...)      
#define DEBUG_LORA_DEBUG(format, ...)     

#define DEBUG_AI_ERROR(format, ...)       
#define DEBUG_AI_WARN(format, ...)        
#define DEBUG_AI_INFO(format, ...)        
#define DEBUG_AI_DEBUG(format, ...)       

#define DEBUG_MEMORY_ERROR(format, ...)   
#define DEBUG_MEMORY_WARN(format, ...)    
#define DEBUG_MEMORY_INFO(format, ...)    
#define DEBUG_MEMORY_DEBUG(format, ...)   

#define DEBUG_TIMER_START(name)           
#define DEBUG_TIMER_END(name)             
#define DEBUG_TIMER_PRINT()               

#define DEBUG_PRINT_MEMORY()              
#define DEBUG_PRINT_SYSTEM()              
#define DEBUG_PRINT_NETWORK()             
#define DEBUG_PRINT_TASKS()               

#define DEBUG_PRINT(x)      
#define DEBUG_PRINTLN(x)    
#define DEBUG_PRINTF(format, ...) 

#endif // DEBUG_ENABLED

#endif // DEBUG_UTILS_H