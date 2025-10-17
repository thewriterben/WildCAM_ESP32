/**
 * @file config_ai_thinker_camera_only.h
 * @brief Configuration template for AI-Thinker ESP32-CAM as standalone camera
 * @hardware AI-Thinker ESP32-CAM
 * @use_case Wildlife camera trap with SD storage and WiFi
 * 
 * FEATURES:
 *   ✓ Camera (OV2640)
 *   ✓ SD Card storage
 *   ✓ PIR motion sensor
 *   ✓ WiFi connectivity
 *   ✓ BME280 environmental sensor (I2C)
 *   ✓ Battery voltage monitoring
 *   ✗ LoRa (conflicts with camera)
 *   ✗ LED indicators (conflicts with SD card)
 * 
 * AVAILABLE GPIO: 1, 3, 16, 17, 33
 * 
 * To use this configuration:
 *   1. Copy this file to firmware/config.h
 *   2. Adjust WiFi credentials
 *   3. Compile with: pio run -e esp32cam_advanced
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// Board Selection
#define CAMERA_MODEL_AI_THINKER       // AI-Thinker ESP32-CAM

// Feature Enables
#define CAMERA_ENABLED true           // ✓ Camera enabled
#define SD_CARD_ENABLED true          // ✓ SD card for local storage
// #define LORA_ENABLED               // ✗ LoRa DISABLED (conflicts with camera)
#define PIR_SENSOR_ENABLED true       // ✓ PIR motion detection on GPIO 1
#define BME280_SENSOR_ENABLED true    // ✓ Environmental sensor via I2C
#define BATTERY_MONITOR_ENABLED true  // ✓ Battery voltage on GPIO 33
#define SOLAR_VOLTAGE_MONITORING_ENABLED false // ✗ Conflicts with camera PWDN
#define LED_INDICATORS_ENABLED false  // ✗ Conflicts with SD card

// ===========================
// CAMERA CONFIGURATION
// ===========================

#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // 1600x1200 pixels
#define CAMERA_JPEG_QUALITY 10            // Quality 1-63 (lower = better)
#define CAMERA_BRIGHTNESS 0               // -2 to +2
#define CAMERA_CONTRAST 0                 // -2 to +2
#define CAMERA_SATURATION 0               // -2 to +2

// ===========================
// MOTION DETECTION
// ===========================

#define PIR_TRIGGER_ENABLED true
#define PIR_COOLDOWN_MS 5000              // 5 seconds between triggers
#define MOTION_CAPTURE_COUNT 3            // Capture 3 images per trigger
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

// ===========================
// POWER MANAGEMENT
// ===========================

#define DEEP_SLEEP_ENABLED true
#define DEEP_SLEEP_DURATION_SEC 3600      // Sleep 1 hour between checks
#define LOW_BATTERY_THRESHOLD 3.3         // Volts
#define CRITICAL_BATTERY_THRESHOLD 3.0    // Volts

// ===========================
// STORAGE CONFIGURATION
// ===========================

#define SD_CARD_FORMAT_IF_FAILED true
#define MAX_IMAGES_PER_DAY 100
#define AUTO_DELETE_OLD_IMAGES true
#define KEEP_IMAGES_DAYS 7

// ===========================
// SENSOR CONFIGURATION
// ===========================

#define BME280_I2C_ADDRESS 0x76
#define BME280_SAMPLE_INTERVAL_MS 60000   // Read every minute
#define SENSOR_DATA_LOGGING_ENABLED true

#endif // CONFIG_H