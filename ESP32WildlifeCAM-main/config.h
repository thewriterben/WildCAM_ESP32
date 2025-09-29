/**
 * @file config.h
 * @brief Global configuration for ESP32WildlifeCAM
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef CONFIG_H
#define CONFIG_H

// Include pin definitions
#include "include/pins.h"

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