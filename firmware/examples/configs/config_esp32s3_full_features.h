/**
 * @file config_esp32s3_full_features.h
 * @brief Configuration template for ESP32-S3-CAM with all features enabled
 * @hardware ESP32-S3-CAM
 * @use_case Full-featured wildlife monitoring with LoRa mesh networking
 * 
 * FEATURES:
 *   ✓ Camera (OV2640/OV5640)
 *   ✓ SD Card storage
 *   ✓ LoRa mesh networking
 *   ✓ PIR motion sensor
 *   ✓ WiFi connectivity
 *   ✓ BME280 environmental sensor (I2C)
 *   ✓ Battery voltage monitoring
 *   ✓ Solar panel voltage monitoring
 *   ✓ LED indicators
 * 
 * AVAILABLE GPIO: 40+ pins (no conflicts!)
 * 
 * To use this configuration:
 *   1. Copy this file to firmware/config.h
 *   2. Adjust WiFi and LoRa settings
 *   3. Compile with: pio run -e esp32s3cam_ai
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// Board Selection
#define CAMERA_MODEL_ESP32S3_EYE      // ESP32-S3-CAM

// Feature Enables - ALL FEATURES SUPPORTED!
#define CAMERA_ENABLED true           // ✓ Camera enabled
#define SD_CARD_ENABLED true          // ✓ SD card for local storage
#define LORA_ENABLED true             // ✓ LoRa mesh networking
#define PIR_SENSOR_ENABLED true       // ✓ PIR motion detection
#define BME280_SENSOR_ENABLED true    // ✓ Environmental sensor via I2C
#define BATTERY_MONITOR_ENABLED true  // ✓ Battery voltage monitoring
#define SOLAR_VOLTAGE_MONITORING_ENABLED true // ✓ Solar panel monitoring
#define LED_INDICATORS_ENABLED true   // ✓ Status LEDs

// ===========================
// CAMERA CONFIGURATION
// ===========================

#define CAMERA_FRAME_SIZE FRAMESIZE_QXGA  // 2048x1536 (3MP) - higher res available
#define CAMERA_JPEG_QUALITY 8             // Quality 1-63 (lower = better)
#define CAMERA_BRIGHTNESS 0               // -2 to +2
#define CAMERA_CONTRAST 0                 // -2 to +2
#define CAMERA_SATURATION 0               // -2 to +2
#define CAMERA_PSRAM_ENABLED true         // Use 8MB PSRAM

// ===========================
// MOTION DETECTION
// ===========================

#define PIR_TRIGGER_ENABLED true
#define PIR_COOLDOWN_MS 5000              // 5 seconds between triggers
#define MOTION_CAPTURE_COUNT 5            // Capture 5 images per trigger
#define MOTION_CAPTURE_INTERVAL_MS 500    // 500ms between captures

// ===========================
// NETWORK CONFIGURATION
// ===========================

// WiFi Settings
#define WIFI_ENABLED true
#define WIFI_SSID "YourWildlifeNetwork"   // ⚠️ CHANGE THIS
#define WIFI_PASSWORD "YourPassword"      // ⚠️ CHANGE THIS
#define WIFI_CONNECTION_TIMEOUT 10000
#define WIFI_MAX_RETRIES 5
#define WIFI_RETRY_BASE_DELAY 1000

// LoRa Mesh Network Settings
#define LORA_FREQUENCY 915E6              // 915 MHz for North America (868E6 for EU)
#define LORA_BANDWIDTH 125E3              // 125 kHz bandwidth
#define LORA_SPREADING_FACTOR 7           // SF7-12 (higher = longer range, slower)
#define LORA_TX_POWER 20                  // Transmit power in dBm (2-20)
#define LORA_HEALTH_CHECK_INTERVAL 60000  // Check mesh health every minute
#define LORA_NODE_TIMEOUT 300000          // Consider node dead after 5 minutes
#define LORA_MAX_RETRIES 3                // Retry failed transmissions

// OTA Updates
#define OTA_ENABLED true
#define OTA_CHECK_INTERVAL 3600000        // Check hourly
#define OTA_UPDATE_URL "http://updates.wildcam.local/firmware.bin"
#define OTA_VERSION_URL "http://updates.wildcam.local/version.txt"

// Data Upload
#define DATA_UPLOAD_ENABLED true
#define DATA_UPLOAD_INTERVAL 300000       // Upload every 5 minutes
#define DATA_API_ENDPOINT "http://api.wildcam.local/upload"
#define MAX_PENDING_UPLOADS 100

// Network Status Logging
#define NETWORK_STATUS_LOG_INTERVAL 300000 // Log every 5 minutes

// ===========================
// POWER MANAGEMENT
// ===========================

#define DEEP_SLEEP_ENABLED true
#define DEEP_SLEEP_DURATION_SEC 3600      // Sleep 1 hour between checks
#define LOW_BATTERY_THRESHOLD 3.4         // Volts
#define CRITICAL_BATTERY_THRESHOLD 3.2    // Volts
#define ADAPTIVE_DUTY_CYCLE true          // Adjust sleep based on battery
#define SOLAR_CHARGING_ENABLED true       // Solar panel charging

// ===========================
// STORAGE CONFIGURATION
// ===========================

#define SD_CARD_FORMAT_IF_FAILED true
#define MAX_IMAGES_PER_DAY 200
#define AUTO_DELETE_OLD_IMAGES true
#define KEEP_IMAGES_DAYS 14               // Keep 2 weeks on SD card

// ===========================
// SENSOR CONFIGURATION
// ===========================

#define BME280_I2C_ADDRESS 0x76
#define BME280_SAMPLE_INTERVAL_MS 60000   // Read every minute
#define SENSOR_DATA_LOGGING_ENABLED true

// ===========================
// LED INDICATOR CONFIGURATION
// ===========================

#define POWER_LED_ENABLED true
#define CHARGING_LED_ENABLED true
#define NETWORK_LED_ENABLED true
#define ERROR_LED_ENABLED true

// ===========================
// ADVANCED FEATURES
// ===========================

#define AI_EDGE_PROCESSING_ENABLED true   // Enable on-device AI
#define SPECIES_DETECTION_ENABLED true    // Automatic species detection
#define IMAGE_COMPRESSION_ENABLED true    // Advanced compression
#define ENCRYPTION_ENABLED true           // Encrypt stored images
#define MESH_ENCRYPTION_ENABLED true      // Encrypt LoRa mesh

#endif // CONFIG_H