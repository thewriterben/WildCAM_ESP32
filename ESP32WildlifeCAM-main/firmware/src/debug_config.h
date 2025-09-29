/**
 * ESP32 Wildlife Camera - Debug Configuration
 * 
 * This file provides compile-time and runtime debug configuration
 * for the enhanced debugging system.
 * 
 * Author: ESP32WildlifeCAM Project
 * Version: 2.1.0 (Enhanced Debug)
 */

#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include "debug_utils.h"

/**
 * Default debug levels for each category
 * These can be overridden at runtime using EnhancedDebugSystem::setLevel()
 */
struct DefaultDebugLevels {
    static const DebugLevel SYSTEM    = DEBUG_LEVEL_INFO;
    static const DebugLevel CAMERA    = DEBUG_LEVEL_INFO;
    static const DebugLevel MOTION    = DEBUG_LEVEL_INFO;
    static const DebugLevel POWER     = DEBUG_LEVEL_INFO;
    static const DebugLevel WIFI      = DEBUG_LEVEL_WARN;   // Reduce WiFi verbosity
    static const DebugLevel LORA      = DEBUG_LEVEL_INFO;
    static const DebugLevel AI        = DEBUG_LEVEL_INFO;
    static const DebugLevel MEMORY    = DEBUG_LEVEL_WARN;   // Only warnings and errors for memory
    static const DebugLevel SENSOR    = DEBUG_LEVEL_INFO;
    static const DebugLevel NETWORK   = DEBUG_LEVEL_WARN;   // Reduce network verbosity
};

/**
 * Debug configuration presets for different deployment scenarios
 */
class DebugPresets {
public:
    /**
     * Development preset - verbose debugging for all components
     */
    static void setDevelopmentMode() {
        EnhancedDebugSystem::setGlobalLevel(DEBUG_LEVEL_DEBUG);
        
        // Enable all debug features
        DebugConfig config = EnhancedDebugSystem::getConfig();
        config.enableTimestamp = true;
        config.enableCategory = true;
        config.enableLevel = true;
        config.enableMemoryInfo = true;
        EnhancedDebugSystem::setConfig(config);
    }
    
    /**
     * Production preset - minimal debugging, errors and warnings only
     */
    static void setProductionMode() {
        EnhancedDebugSystem::setGlobalLevel(DEBUG_LEVEL_WARN);
        
        // Reduce debug overhead
        DebugConfig config = EnhancedDebugSystem::getConfig();
        config.enableTimestamp = true;
        config.enableCategory = true;
        config.enableLevel = true;
        config.enableMemoryInfo = false;
        EnhancedDebugSystem::setConfig(config);
    }
    
    /**
     * Field deployment preset - balanced debugging for troubleshooting
     */
    static void setFieldMode() {
        // Set specific levels for field deployment
        EnhancedDebugSystem::setLevel(DEBUG_SYSTEM, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_CAMERA, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_MOTION, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_POWER, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_WIFI, DEBUG_LEVEL_WARN);
        EnhancedDebugSystem::setLevel(DEBUG_LORA, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_AI, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_MEMORY, DEBUG_LEVEL_WARN);
        EnhancedDebugSystem::setLevel(DEBUG_SENSOR, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_NETWORK, DEBUG_LEVEL_WARN);
        
        // Balanced configuration
        DebugConfig config = EnhancedDebugSystem::getConfig();
        config.enableTimestamp = true;
        config.enableCategory = true;
        config.enableLevel = true;
        config.enableMemoryInfo = false;  // Disable to reduce overhead
        EnhancedDebugSystem::setConfig(config);
    }
    
    /**
     * Performance testing preset - focus on timing and memory
     */
    static void setPerformanceMode() {
        // Minimal logging except for performance metrics
        EnhancedDebugSystem::setGlobalLevel(DEBUG_LEVEL_ERROR);
        EnhancedDebugSystem::setLevel(DEBUG_MEMORY, DEBUG_LEVEL_INFO);
        EnhancedDebugSystem::setLevel(DEBUG_SYSTEM, DEBUG_LEVEL_WARN);
        
        // Enable performance tracking
        DebugConfig config = EnhancedDebugSystem::getConfig();
        config.enableTimestamp = true;
        config.enableCategory = true;
        config.enableLevel = false;       // Skip level to reduce output
        config.enableMemoryInfo = true;   // Enable for performance monitoring
        EnhancedDebugSystem::setConfig(config);
    }
    
    /**
     * Silent mode - emergency/critical errors only
     */
    static void setSilentMode() {
        EnhancedDebugSystem::setGlobalLevel(DEBUG_LEVEL_ERROR);
        
        // Minimal configuration
        DebugConfig config = EnhancedDebugSystem::getConfig();
        config.enableTimestamp = false;
        config.enableCategory = false;
        config.enableLevel = false;
        config.enableMemoryInfo = false;
        EnhancedDebugSystem::setConfig(config);
    }
};

/**
 * Runtime debug control via serial commands
 * This allows changing debug levels without reflashing firmware
 */
class DebugController {
public:
    /**
     * Process debug commands from serial input
     * Commands:
     * - "debug level <category> <level>" - Set category debug level
     * - "debug global <level>" - Set global debug level
     * - "debug preset <preset>" - Apply debug preset
     * - "debug status" - Show current debug configuration
     * - "debug memory" - Show memory information
     * - "debug system" - Show system information
     * - "debug timers" - Show performance timers
     */
    static void processSerialCommands() {
        if (!Serial.available()) return;
        
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.startsWith("debug ")) {
            handleDebugCommand(command.substring(6));
        }
    }
    
private:
    static void handleDebugCommand(const String& cmd) {
        if (cmd.startsWith("level ")) {
            handleLevelCommand(cmd.substring(6));
        } else if (cmd.startsWith("global ")) {
            handleGlobalCommand(cmd.substring(7));
        } else if (cmd.startsWith("preset ")) {
            handlePresetCommand(cmd.substring(7));
        } else if (cmd == "status") {
            showDebugStatus();
        } else if (cmd == "memory") {
            EnhancedDebugSystem::printMemoryInfo();
        } else if (cmd == "system") {
            EnhancedDebugSystem::printSystemInfo();
        } else if (cmd == "timers") {
            EnhancedDebugSystem::printTimers();
        } else {
            showHelp();
        }
    }
    
    static void handleLevelCommand(const String& params) {
        int spaceIndex = params.indexOf(' ');
        if (spaceIndex == -1) return;
        
        String categoryStr = params.substring(0, spaceIndex);
        int level = params.substring(spaceIndex + 1).toInt();
        
        DebugCategory category = stringToCategory(categoryStr);
        if (category != DEBUG_COUNT && level >= 0 && level <= 5) {
            EnhancedDebugSystem::setLevel(category, static_cast<DebugLevel>(level));
            Serial.printf("Set %s debug level to %d\n", categoryStr.c_str(), level);
        }
    }
    
    static void handleGlobalCommand(const String& levelStr) {
        int level = levelStr.toInt();
        if (level >= 0 && level <= 5) {
            EnhancedDebugSystem::setGlobalLevel(static_cast<DebugLevel>(level));
            Serial.printf("Set global debug level to %d\n", level);
        }
    }
    
    static void handlePresetCommand(const String& preset) {
        if (preset == "dev") {
            DebugPresets::setDevelopmentMode();
            Serial.println("Applied development debug preset");
        } else if (preset == "prod") {
            DebugPresets::setProductionMode();
            Serial.println("Applied production debug preset");
        } else if (preset == "field") {
            DebugPresets::setFieldMode();
            Serial.println("Applied field debug preset");
        } else if (preset == "perf") {
            DebugPresets::setPerformanceMode();
            Serial.println("Applied performance debug preset");
        } else if (preset == "silent") {
            DebugPresets::setSilentMode();
            Serial.println("Applied silent debug preset");
        }
    }
    
    static void showDebugStatus() {
        Serial.println("=== Debug Status ===");
        for (int i = 0; i < DEBUG_COUNT; i++) {
            DebugCategory cat = static_cast<DebugCategory>(i);
            Serial.printf("%s: %s\n", 
                         EnhancedDebugSystem::categoryToString(cat),
                         EnhancedDebugSystem::levelToString(EnhancedDebugSystem::getLevel(cat)));
        }
        
        DebugConfig config = EnhancedDebugSystem::getConfig();
        Serial.printf("Timestamp: %s\n", config.enableTimestamp ? "ON" : "OFF");
        Serial.printf("Category: %s\n", config.enableCategory ? "ON" : "OFF");
        Serial.printf("Level: %s\n", config.enableLevel ? "ON" : "OFF");
        Serial.printf("Memory Info: %s\n", config.enableMemoryInfo ? "ON" : "OFF");
        Serial.println("===================");
    }
    
    static void showHelp() {
        Serial.println("Debug Commands:");
        Serial.println("  debug level <category> <level> - Set category debug level");
        Serial.println("  debug global <level> - Set global debug level");
        Serial.println("  debug preset <preset> - Apply preset (dev/prod/field/perf/silent)");
        Serial.println("  debug status - Show current configuration");
        Serial.println("  debug memory - Show memory information");
        Serial.println("  debug system - Show system information");
        Serial.println("  debug timers - Show performance timers");
        Serial.println("Categories: sys, cam, mot, pwr, wifi, lora, ai, mem, sen, net");
        Serial.println("Levels: 0=none, 1=error, 2=warn, 3=info, 4=debug, 5=verbose");
    }
    
    static DebugCategory stringToCategory(const String& str) {
        if (str == "sys") return DEBUG_SYSTEM;
        if (str == "cam") return DEBUG_CAMERA;
        if (str == "mot") return DEBUG_MOTION;
        if (str == "pwr") return DEBUG_POWER;
        if (str == "wifi") return DEBUG_WIFI;
        if (str == "lora") return DEBUG_LORA;
        if (str == "ai") return DEBUG_AI;
        if (str == "mem") return DEBUG_MEMORY;
        if (str == "sen") return DEBUG_SENSOR;
        if (str == "net") return DEBUG_NETWORK;
        return DEBUG_COUNT; // Invalid
    }
};

#endif // DEBUG_CONFIG_H