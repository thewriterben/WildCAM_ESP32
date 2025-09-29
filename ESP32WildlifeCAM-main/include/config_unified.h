#ifndef CONFIG_UNIFIED_H
#define CONFIG_UNIFIED_H

// ===========================
// ESP32 WILDLIFE CAMERA UNIFIED CONFIGURATION
// Centralized configuration for all subsystems
// 
// This file consolidates configuration from multiple sources:
// - firmware/src/config.h (main configuration)
// - wifi_config.h (WiFi settings)  
// - firmware/src/debug_config.h (debug settings)
// - src/streaming/stream_config.h (streaming settings)
// - include/blockchain/blockchain_config.h (blockchain settings)
// - firmware/src/i2c/i2c_config.h (I2C settings)
// - firmware/src/enclosure_config.h (enclosure settings)
// - firmware/src/meshtastic/mesh_config.h (LoRa/mesh settings)
// ===========================

// ===========================
// CAMERA CONFIGURATION
// ===========================

// Camera Board Selection (choose one)
#define CAMERA_MODEL_AI_THINKER  // AI-Thinker ESP32-CAM
// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_V2_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
// #define CAMERA_MODEL_M5STACK_ESP32CAM
// #define CAMERA_MODEL_TTGO_T_JOURNAL

// Camera Capture Settings
#define CAMERA_FRAME_SIZE FRAMESIZE_UXGA  // Image resolution: 1600x1200
#define CAMERA_JPEG_QUALITY 12            // JPEG quality: 10-63 (lower = higher quality)
#define CAMERA_FB_COUNT 2                 // Frame buffer count for smooth capture
#define CAMERA_PIXEL_FORMAT PIXFORMAT_JPEG // Image format
#define CAMERA_GRAB_MODE CAMERA_GRAB_LATEST // Frame grab mode

// Camera Sensor Configuration
#define CAMERA_BRIGHTNESS_DEFAULT 0       // Brightness: -2 to 2
#define CAMERA_CONTRAST_DEFAULT 0         // Contrast: -2 to 2  
#define CAMERA_SATURATION_DEFAULT 0       // Saturation: -2 to 2
#define CAMERA_SHARPNESS_DEFAULT 0        // Sharpness: -2 to 2
#define CAMERA_DENOISE_DEFAULT 0          // Denoise: 0 = disable, 1 = enable
#define CAMERA_AEC_VALUE_DEFAULT 300      // Auto exposure control value
#define CAMERA_AGC_GAIN_DEFAULT 0         // Auto gain control value
#define CAMERA_AWB_GAIN_DEFAULT 1         // Auto white balance gain
#define CAMERA_WB_MODE_DEFAULT 0          // White balance mode
#define CAMERA_AE_LEVEL_DEFAULT 0         // Auto exposure level
#define CAMERA_GAIN_CEILING_DEFAULT GAINCEILING_2X // Maximum sensor gain

// Camera Feature Enables
#define AUTO_EXPOSURE_ENABLED true        // Enable automatic exposure control
#define AUTO_WHITE_BALANCE_ENABLED true   // Enable automatic white balance
#define LENS_CORRECTION_ENABLED true      // Enable lens distortion correction

// ===========================
// MOTION DETECTION CONFIGURATION
// ===========================

// PIR Sensor Settings - CORRECTED PIN ASSIGNMENT (was GPIO 13, now GPIO 1 per AUDIT_REPORT.md)
// Pin assignments are now in pins.h
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define PIR_TRIGGER_MODE RISING          // Interrupt trigger mode
#define MOTION_DETECTION_ENABLED true    // Enable/disable motion detection
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive
#define MOTION_TIMEOUT 30000             // ms - motion detection timeout
#define MOTION_CONSECUTIVE_THRESHOLD 3   // Number of consecutive motions to confirm

// Advanced Motion Detection
#define MULTI_ZONE_PIR_ENABLED true       // Enable multi-zone PIR system
#define MAX_PIR_ZONES 4                   // Maximum number of PIR zones
#define DEFAULT_PIR_ZONES 3               // Default number of zones to configure
#define PIR_ZONE_SENSITIVITY 0.5f         // Default zone sensitivity
#define PIR_ZONE_PRIORITY_STEP 64         // Priority step between zones

// Frame Difference Settings
#define MOTION_THRESHOLD 10               // Motion detection threshold
#define MOTION_DETECTION_BLOCKS 20        // Number of blocks to analyze
#define MOTION_MIN_AREA 100               // Minimum motion area
#define ADVANCED_FRAME_ANALYSIS true      // Enable advanced frame analysis
#define BACKGROUND_SUBTRACTION_ENABLED true // Enable background subtraction
#define MOTION_VECTOR_ANALYSIS true       // Enable motion vector analysis
#define OBJECT_SIZE_FILTERING true        // Enable object size filtering
#define MIN_OBJECT_SIZE_PX 25             // Minimum object size in pixels
#define MAX_OBJECT_SIZE_PX 5000           // Maximum object size in pixels

// Background Model Settings
#define BACKGROUND_LEARNING_RATE 0.1f     // Background adaptation rate
#define BACKGROUND_UPDATE_THRESHOLD 0.3f  // Threshold for background updates
#define BACKGROUND_UPDATE_INTERVAL 5000   // Minimum time between updates (ms)
#define MAX_FRAMES_WITHOUT_BG_UPDATE 100  // Force update after this many frames

// Weather Filtering Settings
#define WEATHER_FILTERING_ENABLED true   // Enable weather-based motion filtering
#define WIND_THRESHOLD 15.0              // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5               // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true           // Temperature compensation for PIR
#define TEMP_STABILITY_THRESHOLD 2.0     // °C - temperature change threshold
#define WEATHER_READING_INTERVAL 60000   // ms - how often to read weather sensors

// Hybrid Motion Detection
#define HYBRID_MOTION_ENABLED true        // Enable PIR + frame difference
#define MOTION_CONFIRMATION_TIME 1000     // Time to confirm motion (ms)

// Enhanced Detection Modes
#define DEFAULT_ENHANCED_MODE 3           // 0=Legacy, 1=MultiZone, 2=Advanced, 3=Full, 4=Adaptive
#define ADAPTIVE_MODE_ENABLED true        // Enable automatic mode selection
#define PERFORMANCE_MONITORING true       // Monitor detection performance

// Analytics Features
#define MOTION_ANALYTICS_ENABLED true     // Enable motion analytics
#define MOTION_HEATMAP_ENABLED false      // Disable heatmap by default (memory intensive)
#define DIRECTION_TRACKING_ENABLED true   // Enable direction tracking
#define SPEED_ESTIMATION_ENABLED true     // Enable speed estimation
#define DWELL_TIME_ANALYSIS_ENABLED true  // Enable dwell time calculation
#define HEATMAP_UPDATE_INTERVAL 10000     // Heatmap update interval (ms)
#define TRACKING_HISTORY_SIZE 100         // Number of tracking points to keep

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// ADC and Voltage Monitoring - DISABLED due to camera pin conflicts
// Note: ESP32-CAM camera uses GPIO 34, 35, 36, 39 for camera data lines
// These pins cannot be used for ADC voltage monitoring simultaneously
#define SOLAR_VOLTAGE_MONITORING_ENABLED false  // Disabled due to GPIO 34 conflict with Y8_GPIO_NUM
#define BATTERY_VOLTAGE_MONITORING_ENABLED false // Disabled due to GPIO 35 conflict with Y9_GPIO_NUM
#define ADC_RESOLUTION 12                // ADC resolution in bits (12-bit = 0-4095)
#define ADC_REFERENCE_VOLTAGE 3.3        // ADC reference voltage
#define VOLTAGE_DIVIDER_RATIO 2.0        // Voltage divider ratio for scaling
#define VOLTAGE_READINGS_COUNT 10        // Number of readings to average
#define VOLTAGE_READING_DELAY 10         // ms - delay between voltage readings

// Battery Thresholds (in volts)
#define BATTERY_FULL_VOLTAGE 4.2         // Maximum battery voltage
#define BATTERY_NORMAL_VOLTAGE 3.8       // Normal operation threshold
#define BATTERY_GOOD_VOLTAGE 3.4         // Good battery level
#define BATTERY_LOW_THRESHOLD 3.0        // Low battery warning threshold
#define BATTERY_CRITICAL_THRESHOLD 2.8   // Critical battery level
#define BATTERY_SHUTDOWN_VOLTAGE 2.5     // Emergency shutdown voltage

// Solar Charging Configuration  
#define SOLAR_VOLTAGE_THRESHOLD 3.2      // V - minimum solar voltage for charging
#define SOLAR_CHARGING_VOLTAGE_MIN 4.0   // V - minimum solar voltage to start charging
#define SOLAR_OPTIMAL_VOLTAGE 5.0        // V - optimal solar voltage

// Power Management Timings
#define VOLTAGE_CHECK_INTERVAL 5000      // ms - how often to check voltages
#define POWER_LOG_INTERVAL 60000         // ms - how often to log power status
#define DEEP_SLEEP_DURATION 300          // seconds - sleep between checks
#define LOW_POWER_CPU_FREQ 80            // MHz - reduced CPU frequency for power saving

// Power Modes
#define POWER_SAVE_MODE_ENABLED true      // Enable power saving
#define ADAPTIVE_DUTY_CYCLE true          // Enable adaptive duty cycling
#define MAX_CPU_FREQ_MHZ 240              // Maximum CPU frequency
#define MIN_CPU_FREQ_MHZ 80               // Minimum CPU frequency for power saving
#define BALANCED_CPU_FREQ_MHZ 160         // Balanced performance frequency

// ===========================
// AI/ML CONFIGURATION
// ===========================

// TensorFlow Lite Settings
#define TFLITE_ARENA_SIZE (512 * 1024)    // 512KB tensor arena
#define MODEL_INPUT_SIZE 224               // Model input dimensions
#define INFERENCE_TIMEOUT_MS 5000          // Maximum inference time

// Species Classification
#define SPECIES_CONFIDENCE_THRESHOLD 0.7f  // Minimum confidence for species
#define MAX_SPECIES_COUNT 50               // Maximum supported species

// Behavior Analysis
#define BEHAVIOR_CONFIDENCE_THRESHOLD 0.6f // Minimum confidence for behavior
#define BEHAVIOR_HISTORY_SIZE 10           // Number of frames for temporal analysis

// Model Management
#define ADAPTIVE_MODEL_SELECTION true     // Enable adaptive model selection
#define MODEL_CALIBRATION_ENABLED true    // Enable runtime calibration
#define MODEL_CACHE_SIZE 3                 // Number of models to cache

// Machine Learning False Positive Reduction
#define ML_FALSE_POSITIVE_FILTERING true  // Enable ML filtering
#define ML_LEARNING_RATE 0.05f            // ML adaptation rate
#define ML_CONFIDENCE_THRESHOLD 0.6f      // ML confidence threshold
#define ML_TRAINING_PERIOD 7200000        // Training period (2 hours in ms)
#define ML_PATTERN_MEMORY_SIZE 1000       // Number of patterns to remember

// ===========================
// LORA MESH NETWORK CONFIGURATION
// ===========================

// LoRa Module Enable/Disable
// CRITICAL: AI-Thinker ESP32-CAM has insufficient GPIO pins for camera + LoRa simultaneously
// Camera pins are hardwired and cannot be changed. LoRa conflicts with camera pins:
// - GPIO 5: LORA_CS conflicts with Y2_GPIO_NUM (camera data bit 2)
// - GPIO 18: LORA_SCK conflicts with Y3_GPIO_NUM (camera data bit 3)
// - GPIO 19: LORA_MISO conflicts with Y4_GPIO_NUM (camera data bit 4)
// - GPIO 23: LORA_MOSI conflicts with HREF_GPIO_NUM (camera horizontal reference)
// - GPIO 26: LORA_DIO0 conflicts with SIOD_GPIO_NUM (camera I2C data)
// 
// RESOLUTION: This configuration prioritizes camera functionality.
// For LoRa networking, use ESP32-S3-CAM or ESP-EYE with more available GPIO pins.
// With LoRa disabled, SD card storage and charging LED are enabled as alternatives.
#define LORA_ENABLED false               // Disabled due to fundamental GPIO conflicts with camera

// LoRa Radio Configuration (for future expansion with compatible boards)
#define LORA_FREQUENCY 915E6             // Frequency: 433E6, 868E6, 915E6 Hz
#define LORA_TX_POWER 20                 // TX Power: 5-20 dBm
#define LORA_SPREADING_FACTOR 7          // Spreading Factor: 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3      // Bandwidth: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
#define LORA_CODING_RATE 5               // Coding Rate: 5-8
#define LORA_PREAMBLE_LENGTH 8           // Preamble Length: 6-65535
#define LORA_SYNC_WORD 0x12              // Network sync word (network ID)
#define LORA_CRC_ENABLED true            // Enable CRC checking

// Mesh Network Configuration
#define NODE_ID 1                        // Unique node identifier (1-255)
#define MAX_MESH_NODES 10                // Maximum nodes in mesh network
#define MESH_RETRY_COUNT 3               // Transmission retry attempts
#define MESH_ACK_TIMEOUT 5000            // ms - acknowledgment timeout
#define MESH_ROUTE_TIMEOUT 30000         // ms - route discovery timeout
#define MESH_HEARTBEAT_INTERVAL 60000    // ms - heartbeat message interval
#define MESH_MAX_HOPS 5                  // Maximum hops for message forwarding

// Message Configuration
#define LORA_MAX_PACKET_SIZE 255         // Maximum LoRa packet size
#define LORA_MESSAGE_QUEUE_SIZE 10       // Maximum queued messages
#define LORA_TRANSMISSION_TIMEOUT 10000  // ms - transmission timeout

// ===========================
// WIFI CONFIGURATION
// ===========================

#define WIFI_ENABLED false               // Enable/disable WiFi functionality
#define WIFI_TIMEOUT 10000               // ms - WiFi connection timeout
#define WIFI_RETRY_COUNT 3               // WiFi connection retry attempts
#define WIFI_SLEEP_MODE WIFI_PS_MIN_MODEM // WiFi power save mode

// WiFi Credentials (see wifi_config.h for security notes)
#define WIFI_SSID ""                     // WiFi network name (configure in wifi_config.h)
#define WIFI_PASSWORD ""                 // WiFi network password (configure in wifi_config.h)

// Web Server Configuration
#define WEB_SERVER_PORT 80
#define WEB_USERNAME "admin"
#define WEB_PASSWORD "wildlife2024"

// ===========================
// STREAMING CONFIGURATION
// ===========================

// Live Streaming Settings
#define MJPEG_BOUNDARY "wildlifecam_stream"
#define MJPEG_CONTENT_TYPE "multipart/x-mixed-replace; boundary=" MJPEG_BOUNDARY
#define MJPEG_FRAME_HEADER "\r\n--" MJPEG_BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: "

// Streaming Performance Limits
#define STREAM_MAX_CLIENTS 3              // Maximum concurrent viewers
#define STREAM_MAX_FPS 10                 // Maximum frames per second
#define STREAM_MIN_FPS 1                  // Minimum frames per second
#define STREAM_DEFAULT_FPS 5              // Default frame rate
#define STREAM_FRAME_TIMEOUT 10000        // Frame timeout in milliseconds
#define STREAM_CLIENT_BUFFER_SIZE 32768   // Client buffer size (32KB)
#define STREAM_MAX_FRAME_SIZE 65536       // Maximum frame size (64KB)

// Motion Detection Integration for Streaming
#define STREAM_MOTION_TRIGGER_ENABLED true
#define STREAM_MOTION_AUTO_START_DELAY 1000    // ms - delay before starting stream after motion
#define STREAM_MOTION_AUTO_STOP_DELAY 30000    // ms - delay before stopping stream after no motion
#define STREAM_MOTION_MINIMUM_CONFIDENCE 50    // Minimum motion confidence to trigger stream

// ===========================
// FILE SYSTEM CONFIGURATION
// ===========================

// SD Card Configuration - ENABLED when LoRa is disabled
#define SD_CARD_ENABLED true             // SD card enabled when LoRa disabled
#if SD_CARD_ENABLED
// SD card pins are defined in pins.h
#define SD_SPI_FREQ 40000000            // SD card SPI frequency (40MHz)
#endif

// Storage Paths and Limits
#define IMAGE_FOLDER "/images"           // Folder for captured images
#define LOG_FOLDER "/logs"               // Folder for system logs
#define CONFIG_FOLDER "/config"          // Folder for configuration files
#define DATA_FOLDER "/data"              // Data storage folder
#define MAX_FILES_PER_DAY 100            // Maximum images per day
#define MAX_DAILY_TRIGGERS 100           // Maximum triggers per day
#define MAX_FILE_SIZE 1048576            // Maximum file size (1MB)
#define FILENAME_TIMESTAMP_FORMAT "%Y%m%d_%H%M%S" // Timestamp format for filenames

// Data Management
#define IMAGE_COMPRESSION_ENABLED true    // Enable image compression
#define METADATA_ENABLED true             // Save metadata with images
#define SPECIES_FOLDERS_ENABLED true      // Organize by species
#define TIME_BASED_FOLDERS true           // Organize by time
#define AUTO_DELETE_OLD_DATA true         // Delete old data when storage full

// ===========================
// I2C CONFIGURATION
// ===========================

// I2C Bus Configuration
#define I2C_MASTER_FREQ_HZ 400000       // I2C master clock frequency
#define I2C_MASTER_TX_BUF_LEN 512       // I2C master TX buffer length
#define I2C_MASTER_RX_BUF_LEN 512       // I2C master RX buffer length
#define I2C_MASTER_TIMEOUT_MS 1000      // I2C master timeout in ms

// ESP-IDF I2C System Configuration
#define ESP_IDF_I2C_ENABLED true         // Enable ESP-IDF native I2C system
#define I2C_MASTER_ENABLED true          // Enable I2C master mode
#define I2C_SLAVE_ENABLED true           // Enable I2C slave mode for multi-board communication
#define I2C_AUTO_PIN_ASSIGNMENT true     // Automatically resolve pin conflicts per board

// I2C Device Addresses
#define BME280_I2C_ADDR_PRIMARY 0x76     // BME280 primary I2C address
#define BME280_I2C_ADDR_SECONDARY 0x77   // BME280 secondary I2C address
#define SSD1306_I2C_ADDR 0x3C            // SSD1306 OLED I2C address
#define RTC_DS3231_I2C_ADDR 0x68         // DS3231 RTC I2C address

// ===========================
// ENVIRONMENTAL SENSORS CONFIGURATION
// ===========================

// BME280 Weather Sensor - NOW ENABLED with ESP-IDF I2C system
#define BME280_ENABLED true              // ENABLED with smart pin assignment
#define BME280_READING_INTERVAL 30000    // ms - sensor reading interval

// Additional Environmental Sensors - DISABLED due to camera pin conflicts
#define LIGHT_SENSOR_ENABLED false       // Disabled due to GPIO 36 conflict with Y6_GPIO_NUM
#define WIND_SENSOR_ENABLED false        // Disabled due to GPIO 39 conflict with Y7_GPIO_NUM

// Vibration Sensor - DISABLED due to LoRa CS pin conflict
#define VIBRATION_ENABLED false          // Disabled due to pin conflicts with LoRa

// IR LED Night Vision - DISABLED due to LoRa DIO0 pin conflict  
#define IR_LED_ENABLED false             // Disabled due to pin conflicts with LoRa

// Satellite Communication - DISABLED due to multiple pin conflicts
#define SATELLITE_ENABLED false          // Disabled due to extensive pin conflicts

// ===========================
// BLOCKCHAIN CONFIGURATION
// ===========================

// Blockchain Core Configuration
#define BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY 5      // Maximum blocks kept in RAM
#define BLOCKCHAIN_MAX_TRANSACTIONS_PER_BLOCK 10   // Maximum transactions per block
#define BLOCKCHAIN_MAX_BLOCK_SIZE 8192    // Maximum block size in bytes
#define BLOCKCHAIN_DEFAULT_BLOCK_INTERVAL 300    // Default block creation interval (seconds)
#define BLOCKCHAIN_HASH_SIZE 32          // SHA-256 hash size in bytes

// Blockchain Storage Configuration
#define BLOCKCHAIN_STORAGE_DIR "/blockchain"  // Storage directory on SD card
#define BLOCKCHAIN_BLOCK_FILE_PREFIX "block_" // Block file prefix
#define BLOCKCHAIN_CHAIN_FILE "chain.json"   // Main chain file
#define BLOCKCHAIN_CONFIG_FILE "config.json" // Configuration file

// Blockchain Performance Configuration
#define BLOCKCHAIN_ASYNC_OPERATIONS true    // Enable async blockchain ops
#define BLOCKCHAIN_SELECTIVE_HASHING true   // Enable selective hashing
#define BLOCKCHAIN_POWER_SAVE_MODE true     // Enable power saving optimizations
#define BLOCKCHAIN_AUTO_VERIFY true         // Auto-verify blockchain integrity
#define BLOCKCHAIN_VERIFY_INTERVAL 3600     // Verification interval (seconds)

// ===========================
// TIME MANAGEMENT CONFIGURATION
// ===========================

// NTP Configuration
#define NTP_ENABLED false                // Enable NTP time synchronization
#define NTP_SERVER "pool.ntp.org"        // NTP server address
#define GMT_OFFSET_SEC 0                 // GMT offset in seconds
#define DAYLIGHT_OFFSET_SEC 3600         // Daylight saving offset in seconds
#define NTP_UPDATE_INTERVAL 86400000     // ms - NTP update interval (24 hours)

// RTC Configuration - CAN NOW BE ENABLED with ESP-IDF I2C system
#define RTC_ENABLED true                 // NOW ENABLED with smart I2C pin assignment
#define RTC_TYPE_DS3231 true             // Use DS3231 RTC (more accurate)
#define RTC_TYPE_PCF8563 false           // Alternative RTC option

// ===========================
// TRIGGER AND TIMING CONFIGURATION
// ===========================

// Active Hours Configuration
#define TRIGGER_ACTIVE_HOURS_START 6     // Hour (24h format) - start active period
#define TRIGGER_ACTIVE_HOURS_END 20      // Hour (24h format) - end active period
#define NIGHT_MODE_ENABLED false         // Enable night photography
#define MAX_DAILY_TRIGGERS 50            // Maximum triggers per day
#define TRIGGER_COOLDOWN_PERIOD 5000     // ms - minimum time between triggers

// Schedule Configuration
#define DAILY_RESET_HOUR 0               // Hour to reset daily counters
#define MAINTENANCE_HOUR 3               // Hour for maintenance tasks
#define STATUS_REPORT_INTERVAL 300000    // ms - system status reporting interval (5 min)

// ===========================
// IMAGE PROCESSING CONFIGURATION
// ===========================

// Image Enhancement
#define IMAGE_TIMESTAMP_ENABLED true     // Add timestamp overlay to images
#define IMAGE_COMPRESSION_ENABLED true   // Enable image compression for transmission
#define THUMBNAIL_ENABLED true           // Generate thumbnails for quick preview
#define THUMBNAIL_SIZE 160               // Thumbnail width in pixels
#define WATERMARK_ENABLED false          // Add watermark to images

// Image Processing Settings
#define AUTO_EXPOSURE_ENABLED true       // Enable automatic exposure adjustment
#define AUTO_WHITE_BALANCE_ENABLED true  // Enable automatic white balance
#define NOISE_REDUCTION_ENABLED true     // Enable noise reduction
#define LENS_CORRECTION_ENABLED true     // Enable lens distortion correction

// Advanced Capture Modes
#define BURST_MODE_ENABLED true
#define BURST_DEFAULT_COUNT 3
#define BURST_DEFAULT_INTERVAL_MS 1000
#define BURST_MAX_COUNT 10
#define BURST_MIN_INTERVAL_MS 100
#define BURST_MAX_INTERVAL_MS 5000

#define VIDEO_MODE_ENABLED true
#define VIDEO_DEFAULT_DURATION_S 10
#define VIDEO_MAX_DURATION_S 60
#define VIDEO_FRAME_RATE 10
#define VIDEO_QUALITY 12

#define TIMELAPSE_MODE_ENABLED true
#define TIMELAPSE_MIN_INTERVAL_S 10
#define TIMELAPSE_MAX_INTERVAL_H 24
#define TIMELAPSE_DEFAULT_INTERVAL_S 60

// ===========================
// AUDIO MONITORING CONFIGURATION
// ===========================

// Audio System Enable/Disable
#define AUDIO_MONITORING_ENABLED true    // Enable comprehensive audio monitoring
#define AUDIO_TRIGGERED_CAPTURE true     // Enable sound-triggered camera activation
#define AUDIO_RECORDING_ENABLED true     // Enable audio file recording
#define AUDIO_WILDLIFE_DETECTION true    // Enable wildlife sound classification

// Hardware Configuration
#define I2S_MICROPHONE_ENABLED true      // Enable I2S digital microphone support
#define ANALOG_MICROPHONE_ENABLED false  // Enable analog microphone support (ADC-based)

// I2S Microphone Configuration (INMP441 and similar)
// Note: I2S pins reuse SD card pins when SD is disabled for LoRa
// CONDITIONAL PIN ASSIGNMENT: Only define I2S pins when SD card is disabled
#if !SD_CARD_ENABLED
// I2S pins would be defined here when SD card is disabled
#else
// When SD card is enabled, I2S digital microphone is disabled to avoid conflicts
#undef I2S_MICROPHONE_ENABLED
#define I2S_MICROPHONE_ENABLED false
#endif

#define I2S_SAMPLE_RATE 16000             // Default sample rate (Hz)
#define I2S_BITS_PER_SAMPLE 16            // Bits per audio sample
#define I2S_CHANNELS 1                    // Number of audio channels (mono)

// Audio Processing Configuration
#define AUDIO_BUFFER_SIZE_MS 100          // Audio buffer size in milliseconds
#define AUDIO_PROCESSING_INTERVAL 50      // Audio processing interval in milliseconds
#define SOUND_DETECTION_THRESHOLD 0.1     // Sound detection threshold (0.0-1.0)
#define WILDLIFE_DETECTION_THRESHOLD 0.7  // Wildlife sound confidence threshold

// ===========================
// HMI (HUMAN MACHINE INTERFACE) CONFIGURATION
// ===========================

// HMI System Configuration
#define HMI_ENABLED true                 // Enable HMI system (display and user interface)
#define HMI_AUTO_BRIGHTNESS true         // Enable automatic brightness adjustment
#define HMI_DEFAULT_BRIGHTNESS 200       // Default display brightness (0-255)
#define HMI_AUTO_OFF_TIMEOUT 30000       // Auto-off timeout in milliseconds (30 seconds)
#define HMI_UPDATE_INTERVAL 1000         // Display update interval in milliseconds

// Display Configuration
#define DISPLAY_AUTO_DETECT true         // Auto-detect display type based on board
#define DISPLAY_I2C_SPEED 400000         // I2C speed for OLED displays
#define DISPLAY_SPI_SPEED 27000000       // SPI speed for TFT displays

// ===========================
// MULTI-BOARD COMMUNICATION CONFIGURATION
// ===========================

// Multi-Board System Configuration
#define MULTIBOARD_ENABLED true          // Enable multi-board coordination system
#define MULTIBOARD_NODE_ID 1             // Default node ID (should be unique per device)
#define MULTIBOARD_DISCOVERY_TIMEOUT 120000 // ms - Discovery timeout (2 minutes)
#define MULTIBOARD_HEARTBEAT_INTERVAL 60000 // ms - Heartbeat interval (1 minute)
#define MULTIBOARD_TASK_TIMEOUT 300000   // ms - Task execution timeout (5 minutes)

// Multi-Board Features
#define MULTIBOARD_AUTO_ROLE_SELECTION true // Enable automatic role selection based on capabilities
#define MULTIBOARD_STANDALONE_FALLBACK true // Enable standalone mode when coordinator unavailable
#define MULTIBOARD_TASK_EXECUTION true   // Enable task execution on nodes
#define MULTIBOARD_LOAD_BALANCING true   // Enable intelligent task distribution

// ===========================
// DEBUG AND LOGGING CONFIGURATION
// ===========================

// Debug Settings
#define DEBUG_ENABLED true               // Enable/disable debug output
#define SERIAL_BAUD_RATE 115200          // Serial communication baud rate
#define DEBUG_BUFFER_SIZE 256            // Debug message buffer size

// Enhanced Debug System Configuration
#define DEBUG_ENHANCED_ENABLED true      // Enable enhanced debug features
#define DEBUG_TIMESTAMP_ENABLED true     // Enable timestamps in debug output
#define DEBUG_CATEGORY_ENABLED true      // Enable category tags in debug output
#define DEBUG_MEMORY_TRACKING true       // Enable memory usage tracking
#define DEBUG_PERFORMANCE_TIMING true    // Enable performance timing features

// Logging Levels
#define LOG_LEVEL_ERROR 0                // Error messages only
#define LOG_LEVEL_WARN 1                 // Warning and error messages
#define LOG_LEVEL_INFO 2                 // Info, warning, and error messages
#define LOG_LEVEL_DEBUG 3                // All messages including debug
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO // Current logging level

// Logging Configuration
#define LOG_TO_SERIAL true               // Enable serial logging
#define LOG_TO_FILE false                // Enable file logging
#define LOG_FILE_MAX_SIZE 1048576        // Maximum log file size (1MB)
#define LOG_ROTATION_ENABLED false       // Enable log file rotation

// ===========================
// FIRMWARE INFORMATION
// ===========================

#define FIRMWARE_VERSION "2.5.0"         // Firmware version string (aligned with project version)
#define HARDWARE_VERSION "1.0"           // Hardware version string
#define BUILD_DATE __DATE__               // Build date (automatically set)
#define BUILD_TIME __TIME__               // Build time (automatically set)
#define DEVICE_NAME "WildlifeCam"         // Device identification name

// ===========================
// UTILITY MACROS AND FUNCTIONS
// ===========================

// Debug and logging macros
#define DEBUG_PRINT(x) if(DEBUG_ENABLED) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_ENABLED) Serial.println(x)
#define DEBUG_PRINTF(format, ...) if(DEBUG_ENABLED) Serial.printf(format, ##__VA_ARGS__)

// Voltage and ADC conversion macros
#define VOLTAGE_TO_ADC(voltage) ((voltage) * 4095 / ADC_REFERENCE_VOLTAGE)
#define ADC_TO_VOLTAGE(adc) ((adc) * ADC_REFERENCE_VOLTAGE / 4095)
#define PERCENTAGE(value, max_value) ((value) * 100 / (max_value))

// Time conversion macros
#define SECONDS_TO_MS(sec) ((sec) * 1000)
#define MINUTES_TO_MS(min) ((min) * 60 * 1000)
#define HOURS_TO_MS(hour) ((hour) * 60 * 60 * 1000)

// Memory and size macros
#define KB_TO_BYTES(kb) ((kb) * 1024)
#define MB_TO_BYTES(mb) ((mb) * 1024 * 1024)

// Configuration validation macros
#define VALIDATE_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#endif // CONFIG_UNIFIED_H