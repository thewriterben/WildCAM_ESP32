/**
 * @file config.h
 * @brief Network and system configuration for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// BOARD CONFIGURATION
// ===========================

// Board type selection - AUTO-DETECTED but can be overridden
// Uncomment to force specific board type
// #define CAMERA_MODEL_AI_THINKER
// #define CAMERA_MODEL_ESP32_S3_CAM
// #define CAMERA_MODEL_TTGO_T_CAMERA
// #define CAMERA_MODEL_ESP32_EYE

// ===========================
// FEATURE ENABLE FLAGS
// ===========================

// Camera Configuration
#ifndef CAMERA_ENABLED
#define CAMERA_ENABLED true               // Enable camera module
#endif

// Network Configuration
#define WIFI_ENABLED true                 // Enable WiFi connectivity
#define LORA_ENABLED false                // Enable LoRa mesh networking (conflicts with camera on AI-Thinker)
#define BLUETOOTH_ENABLED true            // Enable Bluetooth

// Sensor Configuration
#define PIR_MOTION_ENABLED true           // Enable PIR motion detection
#define I2C_SENSORS_ENABLED true          // Enable I2C environmental sensors (BME280, etc.)
#define VOLTAGE_MONITORING_ENABLED false  // Enable battery/solar voltage monitoring

// Control Configuration  
#define SERVO_CONTROL_ENABLED false       // Enable servo control (pan/tilt)
#define IR_LED_ENABLED false              // Enable IR LED for night vision

// Audio Configuration
#define AUDIO_CLASSIFICATION_ENABLED false // Enable audio wildlife classification

// Storage Configuration
#define SD_CARD_ENABLED true              // Enable SD card storage

// ===========================
// GPIO PIN CONFLICT CHECKS
// ===========================
// These compile-time checks prevent invalid hardware configurations
// See docs/GPIO_PIN_CONFLICTS.md for detailed conflict matrix
// See docs/HARDWARE_SELECTION_GUIDE.md for board recommendations

// Check 1: LoRa and Camera conflict on AI-Thinker ESP32-CAM
// LoRa requires SPI pins (GPIO 18, 19, 23) which overlap with camera data pins (Y3, Y4, HREF)
#if defined(CAMERA_MODEL_AI_THINKER) && defined(LORA_ENABLED) && LORA_ENABLED && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #error "LoRa and Camera cannot be enabled simultaneously on AI-Thinker ESP32-CAM due to GPIO conflicts. \
  Conflicting pins: GPIO 18 (Y3/SCK), GPIO 19 (Y4/MISO), GPIO 23 (HREF/MOSI). \
  Solution: Use ESP32-S3-CAM or disable one feature. \
  See docs/GPIO_PIN_CONFLICTS.md for details."
#endif

// Check 2: Solar voltage monitoring conflicts with camera on AI-Thinker
// GPIO 34 is used for camera Y8 data line and cannot be safely shared for voltage monitoring
#if defined(VOLTAGE_MONITORING_ENABLED) && VOLTAGE_MONITORING_ENABLED && defined(CAMERA_MODEL_AI_THINKER) && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #if defined(SOLAR_VOLTAGE_PIN) && SOLAR_VOLTAGE_PIN == 34
    #error "Solar voltage monitoring on GPIO 34 conflicts with camera Y8 pin on AI-Thinker ESP32-CAM. \
    GPIO 34 is input-only and used for camera data. \
    Solution: Use external I2C ADC (ADS1115), use ESP32-S3-CAM, or disable solar monitoring. \
    See docs/GPIO_PIN_CONFLICTS.md for alternative solutions."
  #endif
#endif

// Check 3: Battery voltage monitoring conflicts with camera on AI-Thinker
// GPIO 34 is used for camera Y8 data line
#if defined(VOLTAGE_MONITORING_ENABLED) && VOLTAGE_MONITORING_ENABLED && defined(CAMERA_MODEL_AI_THINKER) && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #if defined(BATTERY_VOLTAGE_PIN) && BATTERY_VOLTAGE_PIN == 34
    #error "Battery voltage monitoring on GPIO 34 conflicts with camera Y8 pin on AI-Thinker ESP32-CAM. \
    GPIO 34 is input-only and used for camera data. \
    Solution: Use external I2C ADC (ADS1115), use ESP32-S3-CAM, or disable battery monitoring. \
    See docs/GPIO_PIN_CONFLICTS.md for alternative solutions."
  #endif
#endif

// Check 4: SD Card and LED conflicts
// When SD card is enabled, GPIO 2, 4, 12-15 are reserved for SD interface
#if defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  #if defined(CHARGING_LED_PIN) && CHARGING_LED_PIN == 2
    #error "CHARGING_LED_PIN (GPIO 2) conflicts with SD card data line D0. \
    SD card uses GPIO 2 for data transmission. \
    Solution: Reassign LED to GPIO 33 or disable SD card. \
    See docs/GPIO_PIN_CONFLICTS.md for pin alternatives."
  #endif
  #if defined(POWER_LED_PIN) && POWER_LED_PIN == 4
    #warning "POWER_LED_PIN (GPIO 4) is shared with SD card data line D1. \
    LED control may interfere with SD card operations. \
    Recommendation: Reassign LED to GPIO 33 or verify compatibility. \
    See docs/GPIO_PIN_CONFLICTS.md for details."
  #endif
  #if defined(CHARGING_CONTROL_PIN) && CHARGING_CONTROL_PIN == 14
    #warning "CHARGING_CONTROL_PIN (GPIO 14) conflicts with SD card clock. \
    Charging control may interfere with SD card operations. \
    Recommendation: Use conditional pin assignment based on SD_CARD_ENABLED. \
    See docs/GPIO_PIN_CONFLICTS.md for details."
  #endif
#endif

// Check 5: Servo control conflicts on AI-Thinker ESP32-CAM
// Servo pins overlap with LoRa and other peripherals when all features enabled
#if defined(CAMERA_MODEL_AI_THINKER) && defined(SERVO_CONTROL_ENABLED) && SERVO_CONTROL_ENABLED
  #if defined(LORA_ENABLED) && LORA_ENABLED
    #error "Servo control and LoRa cannot coexist on AI-Thinker ESP32-CAM due to insufficient GPIO pins. \
    Servos typically use GPIO 16, 17 which are needed for LoRa CS and RST. \
    Solution: Use ESP32-S3-CAM for full features or disable one feature. \
    See docs/HARDWARE_SELECTION_GUIDE.md for board comparison."
  #endif
  #if (defined(PAN_SERVO_PIN) && PAN_SERVO_PIN == 16) && (defined(IR_LED_ENABLED) && IR_LED_ENABLED && defined(IR_LED_PIN) && IR_LED_PIN == 16)
    #error "Pan servo and IR LED both use GPIO 16. \
    Cannot enable both simultaneously. \
    Solution: Disable one feature or use time-multiplexing. \
    See docs/GPIO_PIN_CONFLICTS.md for alternatives."
  #endif
#endif

// Check 6: IR LED conflicts with LoRa chip select on AI-Thinker
#if defined(IR_LED_ENABLED) && IR_LED_ENABLED && defined(LORA_ENABLED) && LORA_ENABLED && defined(CAMERA_MODEL_AI_THINKER)
  #if defined(IR_LED_PIN) && defined(LORA_CS_PIN) && (IR_LED_PIN == LORA_CS_PIN)
    #error "IR LED pin conflicts with LoRa chip select (both use GPIO 16). \
    Cannot enable both IR night vision and LoRa mesh simultaneously on AI-Thinker ESP32-CAM. \
    Solution: Disable one feature or use ESP32-S3-CAM. \
    See docs/GPIO_PIN_CONFLICTS.md for details."
  #endif
#endif

// Check 7: Audio I2S conflicts with camera on AI-Thinker
// I2S pins overlap with camera control signals
#if defined(AUDIO_CLASSIFICATION_ENABLED) && AUDIO_CLASSIFICATION_ENABLED && defined(CAMERA_MODEL_AI_THINKER) && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #if (defined(I2S_SD_PIN) && I2S_SD_PIN == 22) || (defined(I2S_WS_PIN) && I2S_WS_PIN == 25) || (defined(I2S_SCK_PIN) && I2S_SCK_PIN == 26)
    #error "Audio I2S pins conflict with camera on AI-Thinker ESP32-CAM. \
    I2S uses GPIO 22 (PCLK), 25 (VSYNC), 26 (SIOD) which are camera pins. \
    Solution: Use ESP32-S3-CAM for simultaneous camera and audio. \
    See docs/GPIO_PIN_CONFLICTS.md for alternatives."
  #endif
#endif

// Check 8: Multiple simultaneous features on AI-Thinker
// AI-Thinker cannot support full feature set due to limited GPIO
#if defined(CAMERA_MODEL_AI_THINKER)
  #define ENABLED_FEATURE_COUNT \
    (defined(CAMERA_ENABLED) && CAMERA_ENABLED ? 1 : 0) + \
    (defined(LORA_ENABLED) && LORA_ENABLED ? 1 : 0) + \
    (defined(SERVO_CONTROL_ENABLED) && SERVO_CONTROL_ENABLED ? 1 : 0) + \
    (defined(IR_LED_ENABLED) && IR_LED_ENABLED ? 1 : 0) + \
    (defined(AUDIO_CLASSIFICATION_ENABLED) && AUDIO_CLASSIFICATION_ENABLED ? 1 : 0)
  
  #if ENABLED_FEATURE_COUNT > 2
    #warning "AI-Thinker ESP32-CAM has limited GPIO pins. Multiple features enabled may cause conflicts. \
    Recommended: ESP32-S3-CAM for full feature set. \
    See docs/HARDWARE_SELECTION_GUIDE.md for board comparison."
  #endif
#endif

// Check 9: Validation that essential features are not all disabled
#if !defined(CAMERA_ENABLED) || !CAMERA_ENABLED
  #if !defined(LORA_ENABLED) || !LORA_ENABLED
    #if !defined(WIFI_ENABLED) || !WIFI_ENABLED
      #warning "No network connectivity or camera enabled. Device will have limited functionality. \
      Enable at least one of: CAMERA_ENABLED, WIFI_ENABLED, or LORA_ENABLED."
    #endif
  #endif
#endif

// ===========================
// BOARD CAPABILITY DOCUMENTATION
// ===========================
/*
 * AI-THINKER ESP32-CAM LIMITATIONS:
 * - Limited GPIO pins available (5-8 usable pins depending on SD card usage)
 * - Camera + LoRa: IMPOSSIBLE (SPI pin conflicts with camera data lines)
 * - Camera + Audio: IMPOSSIBLE (I2S pin conflicts with camera control lines)
 * - Servos: Limited to 2-3 when SD disabled
 * - Voltage Monitoring: Conflicts with camera data lines (GPIO 34)
 * 
 * SUPPORTED CONFIGURATIONS:
 * - ✅ Camera + SD Card + Motion + Environmental Sensors + WiFi
 * - ✅ Camera + WiFi + Basic Power Management
 * - ✅ Camera + I2C Sensors (BME280, etc.) - shares I2C bus safely
 * 
 * NOT SUPPORTED:
 * - ❌ Camera + LoRa (pin conflicts)
 * - ❌ Camera + Pan/Tilt + LoRa (insufficient pins)
 * - ❌ Camera + Audio classification (pin conflicts)
 * - ❌ Full peripheral suite (need ESP32-S3-CAM)
 * 
 * ESP32-S3-CAM RECOMMENDED FOR:
 * - ✅ Camera + LoRa mesh networking
 * - ✅ Camera + Pan/Tilt servos
 * - ✅ Camera + Audio classification
 * - ✅ Full feature set (all features simultaneously)
 * - ✅ More RAM (8MB PSRAM) for advanced AI/ML
 * - ✅ Additional GPIO pins (20+) for peripheral expansion
 * - ✅ USB OTG for easy debugging
 * - ✅ Better long-term support and expandability
 * 
 * For detailed GPIO conflict matrix, see: docs/GPIO_PIN_CONFLICTS.md
 * For board selection guidance, see: docs/HARDWARE_SELECTION_GUIDE.md
 * For hardware guidelines, see: docs/HARDWARE_FEATURE_GUIDELINES.md
 */

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

// LoRa Mesh Network Settings (disabled by default on AI-Thinker due to conflicts)
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

#endif // CONFIG_H
