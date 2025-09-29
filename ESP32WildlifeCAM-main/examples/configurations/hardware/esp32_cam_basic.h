/**
 * @file esp32_cam_basic.h
 * @brief Basic ESP32-CAM configuration for wildlife monitoring
 * 
 * This configuration is suitable for:
 * - Standard ESP32-CAM boards
 * - Basic wildlife monitoring setups
 * - Power-efficient operations
 * - Beginner deployments
 */

#ifndef ESP32_CAM_BASIC_CONFIG_H
#define ESP32_CAM_BASIC_CONFIG_H

// Hardware Configuration
#define BOARD_TYPE "ESP32-CAM"
#define CAMERA_MODEL_AI_THINKER
#define PSRAM_ENABLED true

// Pin Definitions
#define PIR_SENSOR_PIN 12
#define STATUS_LED_PIN 33
#define FLASH_LED_PIN 4

// Camera Settings
#define CAMERA_RESOLUTION FRAMESIZE_UXGA    // 1600x1200
#define CAMERA_QUALITY 10                   // 0-63, lower = better quality
#define CAMERA_BRIGHTNESS 0                 // -2 to +2
#define CAMERA_CONTRAST 0                   // -2 to +2
#define CAMERA_SATURATION 0                 // -2 to +2

// Motion Detection
#define MOTION_DETECTION_ENABLED true
#define PIR_THRESHOLD 80                    // 0-100, higher = less sensitive
#define MOTION_DELAY_MS 5000                // Delay between motion triggers
#define ADVANCED_MOTION_ANALYSIS false      // Disable for basic setup

// Power Management
#define POWER_SAVE_MODE true
#define BATTERY_MONITORING_ENABLED true
#define LOW_BATTERY_THRESHOLD 20            // Percentage
#define SLEEP_MODE_ENABLED true
#define DEEP_SLEEP_DURATION_SEC 300         // 5 minutes

// Storage Configuration
#define SD_CARD_ENABLED true
#define MAX_FILE_SIZE_MB 5
#define LOW_SPACE_THRESHOLD_MB 100
#define AUTO_CLEANUP_ENABLED true
#define FILE_NAMING_PATTERN "CAM_%Y%m%d_%H%M%S"

// Network Settings
#define WIFI_ENABLED true
#define WIFI_TIMEOUT_MS 30000
#define AP_MODE_FALLBACK true
#define AP_SSID "WildlifeCam_Setup"
#define AP_PASSWORD "wildcam123"

// Features (Basic Setup)
#define WEB_SERVER_ENABLED true
#define WEB_SERVER_PORT 80
#define OTA_UPDATES_ENABLED true
#define ANALYTICS_ENABLED false             // Disable for basic setup
#define AI_CLASSIFICATION_ENABLED false     // Disable for basic setup
#define FEDERATED_LEARNING_ENABLED false    // Disable for basic setup
#define STREAMING_ENABLED false             // Disable for basic setup

// Timing Configuration
#define CAPTURE_INTERVAL_MINUTES 10         // Regular capture interval
#define STATUS_UPDATE_INTERVAL_MINUTES 30   // Status report interval
#define HEARTBEAT_INTERVAL_MINUTES 60       // Heartbeat signal

// Debug and Logging
#define DEBUG_ENABLED false                 // Disable debug in production
#define SERIAL_BAUD_RATE 115200
#define LOG_LEVEL LOG_LEVEL_INFO

// Environmental Limits
#define OPERATING_TEMP_MIN_C -10
#define OPERATING_TEMP_MAX_C 50
#define OPERATING_HUMIDITY_MAX 85           // Percentage

// Example WiFi Credentials (CHANGE THESE!)
#define EXAMPLE_WIFI_SSID "YourNetworkName"
#define EXAMPLE_WIFI_PASSWORD "YourPassword"

// Device Identification
#define DEVICE_NAME "WildlifeCam_Basic"
#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "ESP32-CAM-v1"

#endif // ESP32_CAM_BASIC_CONFIG_H