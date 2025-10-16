/**
 * @file config.h
 * @brief Network and system configuration for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// NETWORK CONFIGURATION
// ===========================

// WiFi Settings
#define WIFI_ENABLED true                 // Enable WiFi connectivity
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

// LoRa Mesh Network Settings
#define LORA_ENABLED true                 // Enable LoRa mesh networking
#define LORA_FREQUENCY 915E6              // LoRa frequency (915 MHz for North America)
#define LORA_HEALTH_CHECK_INTERVAL 60000  // Check mesh health every minute (ms)
#define LORA_NODE_TIMEOUT 300000          // Consider node dead after 5 minutes

// Data Upload Settings
#define DATA_UPLOAD_ENABLED true          // Enable automatic data upload
#define DATA_UPLOAD_INTERVAL 300000       // Upload data every 5 minutes (ms)
#define DATA_API_ENDPOINT "http://api.wildcam.local/upload"
#define MAX_PENDING_UPLOADS 100           // Maximum number of pending uploads to queue

// Network Status Logging
#define NETWORK_STATUS_LOG_INTERVAL 300000 // Log network status every 5 minutes (ms)

// ===========================
// GPS CONFIGURATION
// ===========================

// GPS Module Enable/Disable
#define GPS_ENABLED true                  // Enable GPS module for location tracking

// GPS Serial Communication Pins
#define GPS_RX_PIN 32                     // GPS module TX -> ESP32 RX (GPIO 32)
#define GPS_TX_PIN 33                     // GPS module RX -> ESP32 TX (GPIO 33)

// GPS Module Configuration
#define GPS_BAUD_RATE 9600                // GPS serial baud rate (most modules use 9600)
#define GPS_UPDATE_RATE 1000              // ms - GPS position update rate
#define GPS_FIX_TIMEOUT 60000             // ms - timeout waiting for GPS fix (60 seconds)

// GPS Feature Configuration
#define GPS_USE_TINYGPS_PLUS true         // Use TinyGPS++ library
#define GPS_SAVE_LAST_POSITION true       // Save last known position on fix loss
#define GPS_ALTITUDE_ENABLED true         // Include altitude in GPS data
#define GPS_SATELLITE_COUNT_ENABLED true  // Track satellites in view

#endif // CONFIG_H
