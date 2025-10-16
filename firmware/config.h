/**
 * @file config.h
 * @brief Network and system configuration for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// Board Selection - Define ONE of these
// #define CAMERA_MODEL_AI_THINKER    // AI-Thinker ESP32-CAM (default)
// #define CAMERA_MODEL_ESP32S3_EYE   // ESP32-S3-CAM
// #define CAMERA_MODEL_TTGO_T_CAMERA // TTGO T-Camera

// Feature Enables - Configure based on your hardware
#define CAMERA_ENABLED true               // Enable camera functionality
#define SD_CARD_ENABLED true              // Enable SD card storage
// #define LORA_ENABLED true              // Enable LoRa mesh networking (conflicts with camera on AI-Thinker)
#define PIR_SENSOR_ENABLED true           // Enable PIR motion sensor
#define BME280_SENSOR_ENABLED true        // Enable BME280 environmental sensor
#define BATTERY_MONITOR_ENABLED true      // Enable battery voltage monitoring
#define SOLAR_VOLTAGE_MONITORING_ENABLED false // Enable solar panel voltage monitoring
#define LED_INDICATORS_ENABLED false      // Enable LED indicators (conflicts with SD card)

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
#ifdef LORA_ENABLED
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
// COMPILE-TIME CONFLICT CHECKS
// ===========================
// These checks prevent incompatible feature combinations from compiling
// See docs/GPIO_PIN_CONFLICTS.md for detailed explanation

// Check 1: LoRa + Camera conflict on AI-Thinker ESP32-CAM
#if defined(LORA_ENABLED) && defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #if defined(CAMERA_MODEL_AI_THINKER) || defined(BOARD_AI_THINKER_CAM)
    #error "❌ CONFIGURATION ERROR: LoRa and Camera cannot be enabled simultaneously on AI-Thinker ESP32-CAM due to GPIO conflicts. \n\
    \n\
    CONFLICT DETAILS:\n\
      - LoRa SCK (GPIO 18) conflicts with Camera Y3\n\
      - LoRa MISO (GPIO 19) conflicts with Camera Y4\n\
      - LoRa MOSI (GPIO 23) conflicts with Camera HREF\n\
      - LoRa DIO0 (GPIO 26) conflicts with Camera SIOD\n\
    \n\
    SOLUTIONS:\n\
      1. Disable LoRa: Comment out '#define LORA_ENABLED' in config.h\n\
      2. Disable Camera: Set 'CAMERA_ENABLED false' in config.h\n\
      3. Use ESP32-S3-CAM: Upgrade hardware to ESP32-S3-CAM which has sufficient GPIO pins\n\
    \n\
    See docs/GPIO_PIN_CONFLICTS.md and docs/HARDWARE_SELECTION_GUIDE.md for details."
  #endif
#endif

// Check 2: Solar voltage monitoring on GPIO 34 with camera
#if defined(SOLAR_VOLTAGE_MONITORING_ENABLED) && SOLAR_VOLTAGE_MONITORING_ENABLED && \
    defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #if defined(CAMERA_MODEL_AI_THINKER) || defined(BOARD_AI_THINKER_CAM)
    // GPIO 34 is used for Camera Y8 and Battery Voltage (input-only, can share with coordination)
    // GPIO 32 is used for Camera PWDN (output) and Solar Voltage (input) - CONFLICT
    #warning "⚠️  CONFIGURATION WARNING: Solar voltage monitoring conflicts with Camera PWDN on GPIO 32. \n\
    \n\
    CONFLICT DETAILS:\n\
      - GPIO 32 is Camera PWDN (power-down control, OUTPUT)\n\
      - GPIO 32 requested for solar voltage monitoring (INPUT)\n\
      - Cannot use same pin for both input and output\n\
    \n\
    RECOMMENDATIONS:\n\
      1. Use GPIO 33 for solar voltage monitoring instead\n\
      2. Use external ADC via I2C for additional analog inputs\n\
      3. Disable camera power-down control (camera always on)\n\
    \n\
    See docs/GPIO_PIN_CONFLICTS.md section 'Solar/Battery Monitoring vs Camera'."
  #endif
#endif

// Check 3: LED indicators with SD card
#if defined(LED_INDICATORS_ENABLED) && LED_INDICATORS_ENABLED && \
    defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  #if defined(CAMERA_MODEL_AI_THINKER) || defined(BOARD_AI_THINKER_CAM)
    #error "❌ CONFIGURATION ERROR: LED indicators and SD card cannot be enabled simultaneously on AI-Thinker ESP32-CAM. \n\
    \n\
    CONFLICT DETAILS:\n\
      - GPIO 2 (Power LED) conflicts with SD card D0\n\
      - GPIO 4 (Built-in LED) conflicts with SD card D1\n\
    \n\
    SOLUTIONS:\n\
      1. Disable LEDs: Set 'LED_INDICATORS_ENABLED false' in config.h\n\
      2. Disable SD card: Set 'SD_CARD_ENABLED false' in config.h (not recommended)\n\
      3. Use different GPIO for LEDs: GPIO 16 or 17 (if available)\n\
    \n\
    See docs/GPIO_PIN_CONFLICTS.md section 'LED Indicators vs SD Card'."
  #endif
#endif

// Check 4: Verify board model is defined
#if !defined(CAMERA_MODEL_AI_THINKER) && \
    !defined(CAMERA_MODEL_ESP32S3_EYE) && \
    !defined(CAMERA_MODEL_TTGO_T_CAMERA) && \
    !defined(BOARD_AI_THINKER_CAM) && \
    !defined(BOARD_ESP32S3_CAM) && \
    !defined(BOARD_TTGO_T_CAMERA) && \
    defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #warning "⚠️  BOARD MODEL NOT DEFINED: No camera board model specified. Defaulting to AI-Thinker ESP32-CAM. \n\
  \n\
  To explicitly set board model, define ONE of these in platformio.ini or config.h:\n\
    - CAMERA_MODEL_AI_THINKER or BOARD_AI_THINKER_CAM\n\
    - CAMERA_MODEL_ESP32S3_EYE or BOARD_ESP32S3_CAM\n\
    - CAMERA_MODEL_TTGO_T_CAMERA or BOARD_TTGO_T_CAMERA\n\
  \n\
  See docs/HARDWARE_SELECTION_GUIDE.md for board selection guidance."
  // Default to AI-Thinker if not specified
  #define CAMERA_MODEL_AI_THINKER
#endif

// Check 5: PIR sensor on GPIO 1 vs UART debugging
#if defined(PIR_SENSOR_ENABLED) && PIR_SENSOR_ENABLED
  #if defined(DEBUG_MODE) || defined(ENABLE_SERIAL_DEBUG)
    #warning "⚠️  CONFIGURATION WARNING: PIR sensor on GPIO 1 conflicts with UART TX debugging. \n\
    \n\
    IMPACT:\n\
      - Serial debugging may interfere with PIR readings\n\
      - PIR signals may appear in serial output\n\
    \n\
    RECOMMENDATIONS:\n\
      1. Disable serial debugging in production builds\n\
      2. Use WiFi/telnet debugging instead\n\
      3. Move PIR to GPIO 3 if UART RX not needed\n\
    \n\
    See docs/GPIO_PIN_CONFLICTS.md section 'PIR Sensor vs UART Debug'."
  #endif
#endif

// ===========================
// CONFIGURATION SUMMARY
// ===========================
// This section shows active configuration (visible in build output)

#if defined(CAMERA_ENABLED) && CAMERA_ENABLED
  #pragma message "✓ Camera: ENABLED"
#else
  #pragma message "✗ Camera: DISABLED"
#endif

#if defined(LORA_ENABLED)
  #pragma message "✓ LoRa: ENABLED"
#else
  #pragma message "✗ LoRa: DISABLED"
#endif

#if defined(SD_CARD_ENABLED) && SD_CARD_ENABLED
  #pragma message "✓ SD Card: ENABLED"
#else
  #pragma message "✗ SD Card: DISABLED"
#endif

#if defined(CAMERA_MODEL_AI_THINKER) || defined(BOARD_AI_THINKER_CAM)
  #pragma message "Board: AI-Thinker ESP32-CAM (Limited GPIO)"
#elif defined(CAMERA_MODEL_ESP32S3_EYE) || defined(BOARD_ESP32S3_CAM)
  #pragma message "Board: ESP32-S3-CAM (Full Features)"
#elif defined(CAMERA_MODEL_TTGO_T_CAMERA) || defined(BOARD_TTGO_T_CAMERA)
  #pragma message "Board: TTGO T-Camera"
#endif

#pragma message "Configuration validated. See docs/GPIO_PIN_CONFLICTS.md for pin usage details."

#endif // CONFIG_H
