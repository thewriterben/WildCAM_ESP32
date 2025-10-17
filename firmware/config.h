/**
 * @file config.h
 * @brief Network and system configuration for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================


// ===========================
// NETWORK CONFIGURATION
// ===========================

// WiFi Settings
#define WIFI_SSID "WildCAM_Network"       // Default WiFi SSID (should be configured per deployment)
#define WIFI_PASSWORD "wildlife123"       // Default WiFi password (should be changed)
#define WIFI_CONNECTION_TIMEOUT 10000     // WiFi connection timeout in milliseconds
#define WIFI_MAX_RETRIES 5                // Maximum WiFi connection retry attempts
#define WIFI_RETRY_BASE_DELAY 1000        // Base delay for exponential backoff (ms)

// OTA (Over-The-Air) Update Settings
#define OTA_ENABLED true                  // Enable OTA firmware updates
#define OTA_CHECK_INTERVAL 3600000        // Check for updates every hour (ms)
#define OTA_UPDATE_URL "http://updates.wildcam.local/firmware.bin"
#define OTA_VERSION_URL "http://updates.wildcam.local/version.txt"

#define LORA_FREQUENCY 915E6              // LoRa frequency (915 MHz for North America)
#define LORA_HEALTH_CHECK_INTERVAL 60000  // Check mesh health every minute (ms)
#define LORA_NODE_TIMEOUT 300000          // Consider node dead after 5 minutes
#endif

// Data Upload Settings
#define DATA_UPLOAD_ENABLED true          // Enable automatic data upload
#define DATA_UPLOAD_INTERVAL 300000       // Upload data every 5 minutes (ms)
#define DATA_API_ENDPOINT "http://api.wildcam.local/upload"
#define MAX_PENDING_UPLOADS 100           // Maximum number of pending uploads to queue

// Network Status Logging
#define NETWORK_STATUS_LOG_INTERVAL 300000 // Log network status every 5 minutes (ms)

// ===========================


#endif // CONFIG_H
