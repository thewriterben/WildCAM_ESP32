/**
 * @file config.h
 * @brief Global configuration for ESP32WildlifeCAM
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// HARDWARE FEATURE IMPLEMENTATION GUIDELINES
// ===========================
/*
 * BEFORE ADDING NEW HARDWARE FEATURES:
 * 
 * 1. CHECK FOR PIN CONFLICTS
 *    - Review include/pins.h for existing pin assignments
 *    - Verify your pins don't conflict with camera, SD card, or other features
 *    - Check the available GPIO pins list below
 * 
 * 2. AVAILABLE GPIO PINS ON AI-THINKER ESP32-CAM:
 *    - GPIO 1, 3 (UART TX/RX - use with caution, needed for debugging)
 *    - GPIO 12, 13, 15, 16 (available, but GPIO 12 affects boot if HIGH)
 *    - GPIO 2 (built-in LED, can be repurposed if not using SD card)
 *    
 *    RESERVED PINS (DO NOT USE):
 *    - GPIO 0, 5, 18-27, 32, 34-36, 39: Camera interface
 *    - GPIO 2, 4, 12-15: SD card (when SD_CARD_ENABLED)
 *    - GPIO 6-11: Internal flash (NEVER use these)
 * 
 * 3. FOR FULL FEATURE SET:
 *    Consider upgrading to ESP32-S3-CAM which has more available GPIO pins
 *    and can support concurrent camera, SD card, LoRa, and other peripherals.
 * 
 * 4. DOCUMENT ALL NEW PIN ASSIGNMENTS:
 *    - Add clear comments in include/pins.h
 *    - Update pin conflict checks below
 *    - Document in feature-specific sections
 * 
 * 5. ADD COMPILE-TIME CHECKS:
 *    - Use #error directives to prevent conflicting configurations
 *    - See examples in "PIN CONFLICT CHECKS" section below
 */

// Include pin definitions
#include "include/pins.h"

// ===========================
// PIN CONFLICT CHECKS
// These compile-time checks prevent invalid hardware configurations
// ===========================

// Check 1: LoRa and Camera cannot coexist on AI-Thinker ESP32-CAM
// LoRa requires SPI pins (GPIO 18, 19, 23) which conflict with camera data pins
#if defined(HAS_LORA) && HAS_LORA && defined(HAS_WIFI)
  // Note: This is a simplified check. Full conflict analysis in include/config.h
  #ifdef CAMERA_MODEL_AI_THINKER
    #warning "LoRa on AI-Thinker ESP32-CAM conflicts with camera pins. Consider ESP32-S3-CAM for full features."
  #endif
#endif

// Version information
#define VERSION "2.5.0"
#define VERSION_MAJOR 2
#define VERSION_MINOR 5
#define VERSION_PATCH 0

// System configuration
#define SERIAL_BAUD_RATE 115200
#define WATCHDOG_TIMEOUT_S 30
#define TASK_STACK_SIZE 4096

// Camera configuration defaults
#define DEFAULT_FRAMESIZE FRAMESIZE_UXGA  // 1600x1200
#define DEFAULT_JPEG_QUALITY 12
#define CAMERA_FB_COUNT 1

// Network configuration
#define LORA_FREQUENCY 915E6  // 915 MHz for US
#define LORA_SPREADING_FACTOR 7
#define LORA_BANDWIDTH 125E3
#define LORA_CODING_RATE 5
#define LORA_TX_POWER 20

// Storage configuration
#define SD_MOUNT_POINT "/sdcard"
#define IMAGE_PATH "/sdcard/WILDLIFE/IMAGES"
#define LOG_PATH "/sdcard/WILDLIFE/LOGS"
#define CONFIG_PATH "/sdcard/WILDLIFE/CONFIG"

// Power management
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2
#define SOLAR_MIN_VOLTAGE 4.5
#define DEEP_SLEEP_DURATION_S 300  // 5 minutes

// Motion detection
#define PIR_TRIGGER_DELAY_MS 2000
#define PIR_COOLDOWN_MS 5000
#define MOTION_SENSITIVITY_DEFAULT 5

// Debug flags
#define DEBUG_SERIAL 1
#define DEBUG_CAMERA 1
#define DEBUG_NETWORK 1
#define DEBUG_STORAGE 1
#define DEBUG_POWER 1

// Feature flags (can be overridden by board detection)
#ifndef HAS_LORA
#define HAS_LORA 1
#endif

#ifndef HAS_WIFI
#define HAS_WIFI 1
#endif

#ifndef HAS_SD_CARD
#define HAS_SD_CARD 1
#endif

#ifndef HAS_DISPLAY
#define HAS_DISPLAY 0
#endif

#endif // CONFIG_H