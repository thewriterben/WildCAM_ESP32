/**
 * ESP32 Wildlife Camera - Enhanced Debug Utilities Implementation
 * 
 * Implementation of comprehensive debugging capabilities for the ESP32 wildlife camera.
 * Provides categorized logging, performance monitoring, and system health utilities.
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.1.0 (Enhanced Debug)
 */

#include "debug_utils.h"
#include <stdarg.h>
#include <esp_system.h>
#include <esp_task_wdt.h>

// Static member initialization
DebugLevel EnhancedDebugSystem::categoryLevels[DEBUG_COUNT];
DebugConfig EnhancedDebugSystem::config;
char EnhancedDebugSystem::buffer[1024];
bool EnhancedDebugSystem::initialized = false;
EnhancedDebugSystem::Timer EnhancedDebugSystem::timers[16];
uint8_t EnhancedDebugSystem::timerCount = 0;

/**
 * Initialize the enhanced debug system
 */
void EnhancedDebugSystem::init() {
    if (initialized) return;
    
    // Set default debug levels based on config.h settings
    DebugLevel defaultLevel = static_cast<DebugLevel>(CURRENT_LOG_LEVEL + 1);
    
    for (int i = 0; i < DEBUG_COUNT; i++) {
        categoryLevels[i] = defaultLevel;
    }
    
    // Initialize timers
    for (int i = 0; i < 16; i++) {
        timers[i].active = false;
        timers[i].count = 0;
        timers[i].totalTime = 0;
        memset(timers[i].name, 0, sizeof(timers[i].name));
    }
    timerCount = 0;
    
    // Set default configuration
    config = DebugConfig();
    
    initialized = true;
    
    // Print initialization message
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Enhanced Debug System initialized");
    printSystemInfo();
}

/**
 * Set debug level for a specific category
 */
void EnhancedDebugSystem::setLevel(DebugCategory category, DebugLevel level) {
    if (category < DEBUG_COUNT) {
        categoryLevels[category] = level;
    }
}

/**
 * Set debug level for all categories
 */
void EnhancedDebugSystem::setGlobalLevel(DebugLevel level) {
    for (int i = 0; i < DEBUG_COUNT; i++) {
        categoryLevels[i] = level;
    }
}

/**
 * Get debug level for a category
 */
DebugLevel EnhancedDebugSystem::getLevel(DebugCategory category) {
    if (category < DEBUG_COUNT) {
        return categoryLevels[category];
    }
    return DEBUG_LEVEL_NONE;
}

/**
 * Set debug configuration
 */
void EnhancedDebugSystem::setConfig(const DebugConfig& newConfig) {
    config = newConfig;
}

/**
 * Get current debug configuration
 */
DebugConfig EnhancedDebugSystem::getConfig() {
    return config;
}

/**
 * Core print function with category and level checking
 */
void EnhancedDebugSystem::print(DebugCategory category, DebugLevel level, const char* message) {
    if (!DEBUG_ENABLED || !initialized) return;
    if (category >= DEBUG_COUNT) return;
    if (level > categoryLevels[category]) return;
    
    formatMessage(buffer, sizeof(buffer), category, level, message);
    Serial.print(buffer);
}

/**
 * Core printf function with category and level checking
 */
void EnhancedDebugSystem::printf(DebugCategory category, DebugLevel level, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    if (category >= DEBUG_COUNT) return;
    if (level > categoryLevels[category]) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    formatMessage(buffer, sizeof(buffer), category, level, messageBuffer);
    Serial.print(buffer);
    
    va_end(args);
}

/**
 * Core println function with category and level checking
 */
void EnhancedDebugSystem::println(DebugCategory category, DebugLevel level, const char* message) {
    if (!DEBUG_ENABLED || !initialized) return;
    if (category >= DEBUG_COUNT) return;
    if (level > categoryLevels[category]) return;
    
    formatMessage(buffer, sizeof(buffer), category, level, message);
    Serial.print(buffer);
    Serial.println();
}

/**
 * Error level convenience function
 */
void EnhancedDebugSystem::error(DebugCategory category, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    println(category, DEBUG_LEVEL_ERROR, messageBuffer);
    
    va_end(args);
}

/**
 * Warning level convenience function
 */
void EnhancedDebugSystem::warn(DebugCategory category, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    println(category, DEBUG_LEVEL_WARN, messageBuffer);
    
    va_end(args);
}

/**
 * Info level convenience function
 */
void EnhancedDebugSystem::info(DebugCategory category, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    println(category, DEBUG_LEVEL_INFO, messageBuffer);
    
    va_end(args);
}

/**
 * Debug level convenience function
 */
void EnhancedDebugSystem::debug(DebugCategory category, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    println(category, DEBUG_LEVEL_DEBUG, messageBuffer);
    
    va_end(args);
}

/**
 * Verbose level convenience function
 */
void EnhancedDebugSystem::verbose(DebugCategory category, const char* format, ...) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    va_list args;
    va_start(args, format);
    
    char messageBuffer[512];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    println(category, DEBUG_LEVEL_VERBOSE, messageBuffer);
    
    va_end(args);
}

/**
 * Print comprehensive memory information
 */
void EnhancedDebugSystem::printMemoryInfo() {
    if (!DEBUG_ENABLED || !initialized) return;
    
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
    
    println(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "=== Memory Information ===");
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Free heap: %u bytes", ESP.getFreeHeap());
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Min free heap: %u bytes", ESP.getMinFreeHeap());
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Heap size: %u bytes", ESP.getHeapSize());
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Free PSRAM: %u bytes", ESP.getFreePsram());
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "PSRAM size: %u bytes", ESP.getPsramSize());
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Largest free block: %u bytes", info.largest_free_block);
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Total allocated: %u bytes", info.total_allocated_bytes);
    printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Total free: %u bytes", info.total_free_bytes);
    
    // Calculate fragmentation percentage
    if (info.total_free_bytes > 0) {
        float fragmentation = (1.0f - (float)info.largest_free_block / (float)info.total_free_bytes) * 100.0f;
        printf(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "Heap fragmentation: %.1f%%", fragmentation);
    }
    
    println(DEBUG_MEMORY, DEBUG_LEVEL_INFO, "==========================");
}

/**
 * Print comprehensive system information
 */
void EnhancedDebugSystem::printSystemInfo() {
    if (!DEBUG_ENABLED || !initialized) return;
    
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "=== System Information ===");
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Chip: %s Rev %d", 
           CONFIG_IDF_TARGET, chip_info.revision);
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "CPU Cores: %d", chip_info.cores);
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "CPU Frequency: %u MHz", ESP.getCpuFreqMHz());
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Flash: %u MB %s", 
           ESP.getFlashChipSize() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "SDK Version: %s", ESP.getSdkVersion());
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Reset Reason: %d", esp_reset_reason());
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Uptime: %lu seconds", millis() / 1000);
    
    // Temperature (if available)
    #ifdef SOC_TEMP_SENSOR_SUPPORTED
    float temp = temperatureRead();
    if (temp != NAN) {
        printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "CPU Temperature: %.1f°C", temp);
    }
    #endif
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "==========================");
}

/**
 * Print network information
 */
void EnhancedDebugSystem::printNetworkInfo() {
    if (!DEBUG_ENABLED || !initialized) return;
    
    println(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "=== Network Information ===");
    
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "WiFi Mode: %d", mode);
    
    if (WiFi.status() == WL_CONNECTED) {
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "WiFi Status: Connected");
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "SSID: %s", WiFi.SSID().c_str());
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "IP Address: %s", WiFi.localIP().toString().c_str());
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "Gateway: %s", WiFi.gatewayIP().toString().c_str());
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "Subnet: %s", WiFi.subnetMask().toString().c_str());
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "RSSI: %d dBm", WiFi.RSSI());
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "MAC Address: %s", WiFi.macAddress().c_str());
    } else {
        printf(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "WiFi Status: Disconnected (%d)", WiFi.status());
    }
    
    println(DEBUG_NETWORK, DEBUG_LEVEL_INFO, "===========================");
}

/**
 * Print task information
 */
void EnhancedDebugSystem::printTaskInfo() {
    if (!DEBUG_ENABLED || !initialized) return;
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "=== Task Information ===");
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Current task: %s", pcTaskGetTaskName(NULL));
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "High water mark: %u words", uxTaskGetStackHighWaterMark(NULL));
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Task count: %u", uxTaskGetNumberOfTasks());
    
    // Print available stack space for current task
    UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
    printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "Stack remaining: %u bytes", stackRemaining * 4);
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "========================");
}

/**
 * Start a performance timer
 */
void EnhancedDebugSystem::startTimer(const char* name) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    int index = findTimer(name);
    if (index == -1 && timerCount < 16) {
        index = timerCount++;
        strncpy(timers[index].name, name, sizeof(timers[index].name) - 1);
        timers[index].name[sizeof(timers[index].name) - 1] = '\0';
        timers[index].totalTime = 0;
        timers[index].count = 0;
    }
    
    if (index != -1) {
        timers[index].startTime = millis();
        timers[index].active = true;
    }
}

/**
 * End a performance timer
 */
void EnhancedDebugSystem::endTimer(const char* name) {
    if (!DEBUG_ENABLED || !initialized) return;
    
    unsigned long endTime = millis();
    int index = findTimer(name);
    
    if (index != -1 && timers[index].active) {
        unsigned long duration = endTime - timers[index].startTime;
        timers[index].totalTime += duration;
        timers[index].count++;
        timers[index].active = false;
        
        verbose(DEBUG_SYSTEM, "Timer '%s': %lu ms", name, duration);
    }
}

/**
 * Print all timer statistics
 */
void EnhancedDebugSystem::printTimers() {
    if (!DEBUG_ENABLED || !initialized) return;
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "=== Performance Timers ===");
    
    for (int i = 0; i < timerCount; i++) {
        if (timers[i].count > 0) {
            unsigned long avgTime = timers[i].totalTime / timers[i].count;
            printf(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "%-20s: %u calls, %lu ms total, %lu ms avg", 
                   timers[i].name, timers[i].count, timers[i].totalTime, avgTime);
        }
    }
    
    println(DEBUG_SYSTEM, DEBUG_LEVEL_INFO, "===========================");
}

/**
 * Convert category enum to string
 */
const char* EnhancedDebugSystem::categoryToString(DebugCategory category) {
    switch (category) {
        case DEBUG_SYSTEM:  return "SYS";
        case DEBUG_CAMERA:  return "CAM";
        case DEBUG_MOTION:  return "MOT";
        case DEBUG_POWER:   return "PWR";
        case DEBUG_WIFI:    return "WiFi";
        case DEBUG_LORA:    return "LoRa";
        case DEBUG_AI:      return "AI";
        case DEBUG_MEMORY:  return "MEM";
        case DEBUG_SENSOR:  return "SEN";
        case DEBUG_NETWORK: return "NET";
        default:            return "UNK";
    }
}

/**
 * Convert level enum to string
 */
const char* EnhancedDebugSystem::levelToString(DebugLevel level) {
    switch (level) {
        case DEBUG_LEVEL_ERROR:   return "ERROR";
        case DEBUG_LEVEL_WARN:    return "WARN ";
        case DEBUG_LEVEL_INFO:    return "INFO ";
        case DEBUG_LEVEL_DEBUG:   return "DEBUG";
        case DEBUG_LEVEL_VERBOSE: return "VERB ";
        default:                  return "NONE ";
    }
}

/**
 * Format debug message with timestamp, category, and level
 */
void EnhancedDebugSystem::formatMessage(char* output, size_t maxLen, DebugCategory category, 
                                       DebugLevel level, const char* message) {
    size_t pos = 0;
    
    // Add timestamp if enabled
    if (config.enableTimestamp && pos < maxLen - 1) {
        unsigned long timestamp = getTimestamp();
        pos += snprintf(output + pos, maxLen - pos, "[%8lu] ", timestamp);
    }
    
    // Add level if enabled
    if (config.enableLevel && pos < maxLen - 1) {
        pos += snprintf(output + pos, maxLen - pos, "%s ", levelToString(level));
    }
    
    // Add category if enabled
    if (config.enableCategory && pos < maxLen - 1) {
        pos += snprintf(output + pos, maxLen - pos, "[%s] ", categoryToString(category));
    }
    
    // Add memory info if enabled
    if (config.enableMemoryInfo && pos < maxLen - 1) {
        pos += snprintf(output + pos, maxLen - pos, "<%uK> ", ESP.getFreeHeap() / 1024);
    }
    
    // Add the actual message
    if (pos < maxLen - 1) {
        snprintf(output + pos, maxLen - pos, "%s", message);
    }
    
    // Ensure null termination
    output[maxLen - 1] = '\0';
}

/**
 * Get timestamp for debug messages
 */
unsigned long EnhancedDebugSystem::getTimestamp() {
    return millis();
}

/**
 * Find timer by name
 */
int EnhancedDebugSystem::findTimer(const char* name) {
    for (int i = 0; i < timerCount; i++) {
        if (strcmp(timers[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}