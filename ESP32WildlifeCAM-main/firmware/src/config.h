#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// ESP32 WILDLIFE CAMERA CONFIGURATION
// Centralized configuration for all subsystems
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
// #define CAMERA_MODEL_LILYGO_T_CAMERA_PLUS_S3  // LilyGO T-Camera Plus S3 OV5640 V1.1

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

// PIR Sensor Settings
#define PIR_PIN 1                        // PIR sensor GPIO pin (CORRECTED from GPIO 13 per AUDIT_REPORT.md)
#define PIR_DEBOUNCE_TIME 2000           // ms - prevent multiple triggers
#define PIR_TRIGGER_MODE RISING          // Interrupt trigger mode
#define MOTION_DETECTION_ENABLED true    // Enable/disable motion detection
#define MOTION_SENSITIVITY 50            // 0-100, higher = more sensitive
#define MOTION_TIMEOUT 30000             // ms - motion detection timeout
#define MOTION_CONSECUTIVE_THRESHOLD 3   // Number of consecutive motions to confirm

// Weather Filtering Settings
#define WEATHER_FILTERING_ENABLED true   // Enable weather-based motion filtering
#define WIND_THRESHOLD 15.0              // km/h - ignore motion above this wind speed
#define RAIN_THRESHOLD 0.5               // mm/h - ignore motion during rain
#define TEMP_COMP_ENABLED true           // Temperature compensation for PIR
#define TEMP_STABILITY_THRESHOLD 2.0     // °C - temperature change threshold
#define WEATHER_READING_INTERVAL 60000   // ms - how often to read weather sensors

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// ADC and Voltage Monitoring - DISABLED due to camera pin conflicts
// Note: ESP32-CAM camera uses GPIO 34, 35, 36, 39 for camera data lines
// These pins cannot be used for ADC voltage monitoring simultaneously
#define SOLAR_VOLTAGE_MONITORING_ENABLED false  // Disabled due to GPIO 34 conflict with Y8_GPIO_NUM
#define BATTERY_VOLTAGE_MONITORING_ENABLED false // Disabled due to GPIO 35 conflict with Y9_GPIO_NUM
// #define SOLAR_VOLTAGE_PIN 34          // CONFLICTS with Y8_GPIO_NUM
// #define BATTERY_VOLTAGE_PIN 35        // CONFLICTS with Y9_GPIO_NUM
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

// Power Indicator LED - Alternative pin to avoid SD card conflict
#define CHARGING_LED_PIN 16              // Charging indicator LED pin (GPIO 16 available on ESP32-CAM)
#define CHARGING_LED_ENABLED true        // Enabled when LoRa disabled

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

// LoRa Radio Configuration
#define LORA_FREQUENCY 915E6             // Frequency: 433E6, 868E6, 915E6 Hz
#define LORA_TX_POWER 20                 // TX Power: 5-20 dBm
#define LORA_SPREADING_FACTOR 7          // Spreading Factor: 6-12
#define LORA_SIGNAL_BANDWIDTH 125E3      // Bandwidth: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3
#define LORA_CODING_RATE 5               // Coding Rate: 5-8
#define LORA_PREAMBLE_LENGTH 8           // Preamble Length: 6-65535
#define LORA_SYNC_WORD 0x12              // Network sync word (network ID)
#define LORA_CRC_ENABLED true            // Enable CRC checking

// LoRa GPIO Pin Configuration (DISABLED - pins available for other uses)
// #define LORA_SCK 18                   // SPI clock pin - CONFLICTS with Y3_GPIO_NUM
// #define LORA_MISO 19                  // SPI MISO pin - CONFLICTS with Y4_GPIO_NUM
// #define LORA_MOSI 23                  // SPI MOSI pin - CONFLICTS with HREF_GPIO_NUM
// #define LORA_CS 5                     // Chip select pin - CONFLICTS with Y2_GPIO_NUM
// #define LORA_RST 14                   // Reset pin - now available for SD_CLK_PIN
// #define LORA_DIO0 26                  // DIO0 interrupt pin - CONFLICTS with SIOD_GPIO_NUM

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
#define WIFI_SSID ""                     // WiFi network name
#define WIFI_PASSWORD ""                 // WiFi network password
#define WIFI_TIMEOUT 10000               // ms - WiFi connection timeout
#define WIFI_RETRY_COUNT 3               // WiFi connection retry attempts
#define WIFI_SLEEP_MODE WIFI_PS_MIN_MODEM // WiFi power save mode

// ===========================
// FILE SYSTEM CONFIGURATION
// ===========================

// SD Card Configuration - ENABLED when LoRa is disabled
#define SD_CARD_ENABLED true             // SD card enabled when LoRa disabled
#if SD_CARD_ENABLED
#define SD_CS_PIN 12                     // SD card chip select pin
#define SD_MOSI_PIN 15                   // SD card MOSI pin
#define SD_CLK_PIN 14                    // SD card clock pin (available when LORA_RST disabled)
#define SD_MISO_PIN 2                    // SD card MISO pin (available when charging LED disabled)
#define SD_SPI_FREQ 40000000            // SD card SPI frequency (40MHz)
#endif

// Storage Paths and Limits
#define IMAGE_FOLDER "/images"           // Folder for captured images
#define LOG_FOLDER "/logs"               // Folder for system logs
#define CONFIG_FOLDER "/config"          // Folder for configuration files
#define MAX_FILES_PER_DAY 100            // Maximum images per day
#define MAX_FILE_SIZE 1048576            // Maximum file size (1MB)
#define FILENAME_TIMESTAMP_FORMAT "%Y%m%d_%H%M%S" // Timestamp format for filenames

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
// GPS CONFIGURATION
// ===========================

// GPS Module Configuration
#define GPS_ENABLED true                 // Enable GPS module for location tracking
#define GPS_BAUD_RATE 9600              // GPS module baud rate (typically 9600 for NMEA)
#define GPS_UPDATE_INTERVAL 1000        // ms - GPS update interval
#define GPS_FIX_TIMEOUT 60000           // ms - timeout waiting for GPS fix (60 seconds)
#define GPS_COLD_START_TIMEOUT 120000   // ms - cold start timeout for GPS module

// GPS Pin Configuration - CONDITIONAL based on board and feature conflicts
// Note: GPS pin availability varies by board and enabled features
#if defined(CAMERA_MODEL_AI_THINKER)
    // AI-Thinker ESP32-CAM has limited GPIO availability
    // WARNING: Some pins conflict with camera functionality
    
    #if GPS_ENABLED
        // GPS Serial Communication Pins
        // GPS_RX_PIN: ESP32 receives data from GPS module TX
        // GPS_TX_PIN: ESP32 transmits data to GPS module RX
        
        // Default GPS pins for AI-Thinker (with conflict notes):
        #define GPS_RX_PIN 33           // GPIO 33 - Available, no camera conflict
        #define GPS_TX_PIN 13           // GPIO 13 - Available (strapping pin, use caution)
        #define GPS_EN_PIN 12           // GPIO 12 - GPS enable/power pin (conflicts with SD_CS when SD enabled)
        
        // Alternative pin assignments if SD card is disabled:
        // #define GPS_EN_PIN 14        // Use SD_CLK_PIN when SD card disabled
        // #define GPS_TX_PIN 2         // Use SD_MISO_PIN when SD card disabled
        
        // PIN CONFLICT WARNINGS:
        // - Do NOT use GPIO 32 (PWDN_GPIO_NUM - camera power down)
        // - Do NOT use GPIO 25 (VSYNC_GPIO_NUM - camera vertical sync)
        // - GPIO 12 conflicts with SD_CS_PIN when SD card is enabled
        // - GPIO 13 is a strapping pin (affects boot mode if held low)
    #endif
    
#elif defined(CAMERA_MODEL_LILYGO_T_CAMERA_PLUS_S3)
    // LilyGO T-Camera Plus S3 has more available GPIO pins
    #if GPS_ENABLED
        #define GPS_RX_PIN 16           // GPIO 16 - Available on ESP32-S3
        #define GPS_TX_PIN 15           // GPIO 15 - Available on ESP32-S3
        #define GPS_EN_PIN 10           // GPIO 10 - GPS enable/power pin
    #endif
    
#else
    // Default GPS pin configuration for other boards
    #if GPS_ENABLED
        #define GPS_RX_PIN 16           // Default RX pin
        #define GPS_TX_PIN 17           // Default TX pin
        #define GPS_EN_PIN 18           // Default enable pin
    #endif
#endif

// GPS Features Configuration
#define GPS_LOCATION_LOGGING true       // Log GPS coordinates with images
#define GPS_MOVEMENT_DETECTION true     // Detect camera movement/theft
#define GPS_MOVEMENT_THRESHOLD 5.0      // meters - movement detection threshold
#define GPS_GEOFENCING_ENABLED false    // Enable geofencing alerts
#define GPS_TIME_SYNC_ENABLED true      // Use GPS for accurate time synchronization

// GPS Power Management
#define GPS_POWER_SAVE_ENABLED true     // Enable GPS power saving mode
#define GPS_STANDBY_AFTER_FIX true      // Put GPS in standby after obtaining fix
#define GPS_PERIODIC_UPDATES true       // Wake GPS periodically for position updates
#define GPS_UPDATE_PERIOD 300000        // ms - period for GPS updates when in power save (5 minutes)

// GPS Data Storage
#define GPS_EMBED_IN_EXIF true          // Embed GPS coordinates in JPEG EXIF data
#define GPS_LOG_TRACK true              // Log GPS track for camera movement history
#define GPS_TRACK_LOG_INTERVAL 60000    // ms - interval for track logging

// ===========================
// ENVIRONMENTAL SENSORS CONFIGURATION
// ===========================

// ===========================
// ESP-IDF I2C SYSTEM CONFIGURATION
// ===========================

// ESP-IDF I2C System - Replaces Arduino Wire library with intelligent pin management
#define ESP_IDF_I2C_ENABLED true         // Enable ESP-IDF native I2C system
#define I2C_MASTER_ENABLED true          // Enable I2C master mode
#define I2C_SLAVE_ENABLED true           // Enable I2C slave mode for multi-board communication
#define I2C_AUTO_PIN_ASSIGNMENT true     // Automatically resolve pin conflicts per board

// BME280 Weather Sensor - NOW ENABLED with ESP-IDF I2C system
#define BME280_ENABLED true              // ENABLED with smart pin assignment
#define BME280_READING_INTERVAL 30000    // ms - sensor reading interval

// Additional Environmental Sensors - DISABLED due to camera pin conflicts
#define LIGHT_SENSOR_ENABLED false       // Disabled due to GPIO 36 conflict with Y6_GPIO_NUM
#define WIND_SENSOR_ENABLED false        // Disabled due to GPIO 39 conflict with Y7_GPIO_NUM
// #define LIGHT_SENSOR_PIN 36           // CONFLICTS with Y6_GPIO_NUM
// #define WIND_SENSOR_PIN 39            // CONFLICTS with Y7_GPIO_NUM

// Vibration Sensor - DISABLED due to LoRa CS pin conflict
#define VIBRATION_ENABLED false          // Disabled due to pin conflicts with LoRa
// #define VIBRATION_SENSOR_PIN 5        // Would conflict with LORA_CS pin

// IR LED Night Vision - DISABLED due to LoRa DIO0 pin conflict  
#define IR_LED_ENABLED false             // Disabled due to pin conflicts with LoRa
// #define IR_LED_PIN 26                 // Would conflict with LORA_DIO0 pin

// Satellite Communication - DISABLED due to multiple pin conflicts
#define SATELLITE_ENABLED false          // Disabled due to extensive pin conflicts
// Satellite communication would require multiple pins that conflict with camera and LoRa

// ===========================
// GPS CONFIGURATION
// ===========================

// GPS Module Enable/Disable
// GPS provides precise location data for wildlife camera images and enables
// geolocation features for captured images and events
#define GPS_ENABLED true                 // Enable GPS module for location tracking

// GPS Serial Communication Pins
// These pins are used for UART communication with the GPS module
// Note: Pins 32 and 33 are available on AI-Thinker ESP32-CAM and don't conflict with camera
#define GPS_RX_PIN 32                    // GPS module TX -> ESP32 RX (GPIO 32)
#define GPS_TX_PIN 33                    // GPS module RX -> ESP32 TX (GPIO 33)

// GPS Module Configuration
#define GPS_BAUD_RATE 9600               // GPS serial baud rate (standard for most GPS modules)
#define GPS_UPDATE_RATE 1000             // ms - GPS position update rate
#define GPS_FIX_TIMEOUT 60000            // ms - timeout waiting for GPS fix (60 seconds)
#define GPS_ENABLE_PIN 25                // Optional: GPS module enable/power control pin (GPIO 25)
#define GPS_POWER_CONTROL_ENABLED false  // Enable power control for GPS module

// GPS Feature Configuration
#define GPS_USE_TINYGPS_PLUS true        // Use TinyGPS++ library for NMEA parsing
#define GPS_SAVE_LAST_POSITION true      // Save and return last known position on fix loss
#define GPS_ALTITUDE_ENABLED true        // Include altitude in GPS data
#define GPS_SATELLITE_COUNT_ENABLED true // Track number of satellites in view

// GPS Power Management
#define GPS_AUTO_SLEEP_ENABLED false     // Automatically sleep GPS when not in use
#define GPS_SLEEP_TIMEOUT 300000         // ms - GPS sleep timeout (5 minutes)

// GPS Data Format
#define GPS_DECIMAL_PLACES 6             // Number of decimal places for lat/lon (6 = ~0.1m precision)
#define GPS_COORDINATE_FORMAT_DMS false  // false = decimal degrees, true = degrees/minutes/seconds

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

// Debug Mode Presets (uncomment one)
// #define DEBUG_DEVELOPMENT_MODE        // Verbose debugging for development
// #define DEBUG_PRODUCTION_MODE         // Minimal debugging for production
// #define DEBUG_PERFORMANCE_MODE        // Focus on performance metrics
// Default: Field deployment mode (balanced)

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
#define MICROPHONE_AMPLIFIER_ENABLED false // Enable external microphone amplifier

// I2S Microphone Configuration (INMP441 and similar)
// Note: I2S pins reuse SD card pins when SD is disabled for LoRa
// CONDITIONAL PIN ASSIGNMENT: Only define I2S pins when SD card is disabled
#if !SD_CARD_ENABLED
#define I2S_WS_PIN 15                     // I2S Word Select (LR Clock) pin - SD_MOSI_PIN when SD disabled
#define I2S_SCK_PIN 14                    // I2S Serial Clock pin - SD_CLK_PIN when SD disabled  
#define I2S_SD_PIN 2                      // I2S Serial Data pin - SD_MISO_PIN when SD disabled
#else
// When SD card is enabled, I2S digital microphone is disabled to avoid conflicts
#undef I2S_MICROPHONE_ENABLED
#define I2S_MICROPHONE_ENABLED false
#endif
#define I2S_PORT I2S_NUM_0                // I2S port number
#define I2S_SAMPLE_RATE 16000             // Default sample rate (Hz)
#define I2S_BITS_PER_SAMPLE 16            // Bits per audio sample
#define I2S_CHANNELS 1                    // Number of audio channels (mono)

// Analog Microphone Configuration (when enabled)
// Note: Analog microphone reuses camera Y6 pin when camera is in digital-only mode
// CONDITIONAL PIN ASSIGNMENT: Only define analog mic pin when camera Y6 is not needed
#if !defined(CAMERA_MODEL_AI_THINKER) || ANALOG_MICROPHONE_ENABLED == false
// Analog microphone disabled on AI-Thinker ESP32-CAM due to camera Y6 pin conflict
#undef ANALOG_MICROPHONE_ENABLED
#define ANALOG_MICROPHONE_ENABLED false
#else
#define ANALOG_MIC_PIN 36                 // ADC pin for analog microphone - Y6_GPIO_NUM when camera disabled
#endif
#define ANALOG_MIC_GAIN 1.0               // Analog microphone gain multiplier
#define ANALOG_MIC_BIAS_VOLTAGE 1.65      // Bias voltage for electret microphones
#define ADC_SAMPLE_RATE 8000              // ADC sampling rate for analog microphone

// Audio Processing Configuration
#define AUDIO_BUFFER_SIZE_MS 100          // Audio buffer size in milliseconds
#define AUDIO_PROCESSING_INTERVAL 50      // Audio processing interval in milliseconds
#define AUDIO_FFT_SIZE 512                // FFT size for frequency analysis
#define AUDIO_WINDOW_OVERLAP 0.5          // Window overlap for spectral analysis

// Sound Detection Thresholds
#define SOUND_DETECTION_THRESHOLD 0.1     // Sound detection threshold (0.0-1.0)
#define WILDLIFE_DETECTION_THRESHOLD 0.7  // Wildlife sound confidence threshold
#define NOISE_GATE_THRESHOLD 0.05         // Noise gate threshold to filter quiet sounds
#define SOUND_TRIGGER_DURATION_MS 500     // Minimum sound duration to trigger camera (ms)

// Audio Quality and Compression
#define AUDIO_SAMPLE_RATES {8000, 16000, 44100} // Supported sample rates
#define AUDIO_DEFAULT_SAMPLE_RATE 16000   // Default sample rate for recording
#define AUDIO_COMPRESSION_ENABLED true    // Enable audio compression for storage
#define AUDIO_COMPRESSION_QUALITY 4       // Compression quality (1-10, higher=better)

// Recording Configuration
#define AUDIO_MAX_RECORDING_DURATION 300  // Maximum recording duration in seconds
#define AUDIO_PRE_TRIGGER_DURATION 2      // Pre-trigger audio buffer duration in seconds
#define AUDIO_POST_TRIGGER_DURATION 5     // Post-trigger recording duration in seconds
#define AUDIO_FILE_FORMAT_WAV true        // Enable WAV file format
#define AUDIO_FILE_ROTATION_SIZE_MB 10    // Audio file rotation size in MB

// Frequency Analysis Configuration
#define WILDLIFE_FREQ_MIN 100             // Minimum frequency for wildlife detection (Hz)
#define WILDLIFE_FREQ_MAX 8000            // Maximum frequency for wildlife detection (Hz)
#define RAPTOR_FREQ_MIN 200               // Minimum frequency for raptor calls (Hz)
#define RAPTOR_FREQ_MAX 4000              // Maximum frequency for raptor calls (Hz)
#define BIRD_FREQ_MIN 500                 // Minimum frequency for general bird calls (Hz)
#define BIRD_FREQ_MAX 6000                // Maximum frequency for general bird calls (Hz)

// Environmental Sound Filtering
#define WIND_NOISE_FILTER_ENABLED true    // Enable wind noise filtering
#define RAIN_NOISE_FILTER_ENABLED true    // Enable rain noise filtering
#define HUMAN_VOICE_FILTER_ENABLED true   // Enable human voice filtering
#define VEHICLE_NOISE_FILTER_ENABLED true // Enable vehicle noise filtering

// Power Management for Audio
#define AUDIO_LOW_POWER_MODE true         // Enable low power audio monitoring
#define AUDIO_DEEP_SLEEP_DURATION 30      // Audio system sleep duration in seconds
#define AUDIO_CONTINUOUS_MONITORING false // Enable 24/7 audio monitoring (high power)
#define AUDIO_SCHEDULED_MONITORING true   // Enable scheduled audio monitoring
#define AUDIO_ACTIVE_HOURS_START 5        // Start hour for audio monitoring
#define AUDIO_ACTIVE_HOURS_END 22         // End hour for audio monitoring

// Data Storage and Logging
#define AUDIO_EVENT_LOGGING true          // Enable audio event logging
#define AUDIO_METADATA_LOGGING true       // Enable detailed audio metadata logging
#define AUDIO_SPECTROGRAM_STORAGE false   // Enable spectrogram image storage (high storage)
#define AUDIO_RAW_DATA_STORAGE false      // Enable raw audio data storage (very high storage)

// Performance and Memory Configuration
#define AUDIO_TASK_PRIORITY 2             // Audio processing task priority
#define AUDIO_TASK_STACK_SIZE 8192        // Audio task stack size
#define AUDIO_DMA_BUFFER_COUNT 4          // Number of DMA buffers for I2S
#define AUDIO_DMA_BUFFER_SIZE 1024        // Size of each DMA buffer

// Pin Conflict Resolution Notes:
// - I2S pins use SD card pins when SD is disabled for LoRa
// - Analog microphone uses camera Y6 pin when analog mode is selected
// - Audio system automatically switches between I2S and analog based on configuration
// - LoRa mesh networking takes priority over audio when both enabled

// ===========================
// HMI (HUMAN MACHINE INTERFACE) CONFIGURATION
// ===========================

// HMI System Configuration
#define HMI_ENABLED true                 // Enable HMI system (display and user interface)
#define HMI_AUTO_BRIGHTNESS true         // Enable automatic brightness adjustment
#define HMI_DEFAULT_BRIGHTNESS 200       // Default display brightness (0-255)
#define HMI_AUTO_OFF_TIMEOUT 30000       // Auto-off timeout in milliseconds (30 seconds)
#define HMI_UPDATE_INTERVAL 1000         // Display update interval in milliseconds
#define HMI_LOW_POWER_MODE true          // Enable low power mode for display

// Display Configuration
#define DISPLAY_AUTO_DETECT true         // Auto-detect display type based on board
#define DISPLAY_FORCE_TYPE 0             // Force specific display type (0=auto, 1=SSD1306, 2=ST7789)
#define DISPLAY_I2C_SPEED 400000         // I2C speed for OLED displays
#define DISPLAY_SPI_SPEED 27000000       // SPI speed for TFT displays

// Menu System Configuration
#define HMI_MENU_TIMEOUT 30000           // Menu timeout in milliseconds
#define HMI_STATUS_DISPLAY_TIME 5000     // Status display time in milliseconds
#define HMI_ERROR_DISPLAY_TIME 10000     // Error display time in milliseconds

// Button Input Configuration (if hardware buttons are available)
#define HMI_BUTTON_DEBOUNCE_MS 50        // Button debounce time
#define HMI_BUTTON_LONG_PRESS_MS 1000    // Long press detection time

// ===========================
// MULTI-BOARD COMMUNICATION CONFIGURATION
// ===========================

// Multi-Board System Configuration
#define MULTIBOARD_ENABLED true          // Enable multi-board coordination system
#define MULTIBOARD_NODE_ID 1             // Default node ID (should be unique per device)
#define MULTIBOARD_PREFERRED_ROLE ROLE_NODE // Default preferred role
#define MULTIBOARD_DISCOVERY_TIMEOUT 120000 // ms - Discovery timeout (2 minutes)
#define MULTIBOARD_ROLE_CHANGE_TIMEOUT 60000 // ms - Role change timeout (1 minute)
#define MULTIBOARD_HEARTBEAT_INTERVAL 60000 // ms - Heartbeat interval (1 minute)
#define MULTIBOARD_TASK_TIMEOUT 300000   // ms - Task execution timeout (5 minutes)
#define MULTIBOARD_COORDINATOR_TIMEOUT 600000 // ms - Coordinator availability timeout (10 minutes)

// Multi-Board Features
#define MULTIBOARD_AUTO_ROLE_SELECTION true // Enable automatic role selection based on capabilities
#define MULTIBOARD_STANDALONE_FALLBACK true // Enable standalone mode when coordinator unavailable
#define MULTIBOARD_TASK_EXECUTION true   // Enable task execution on nodes
#define MULTIBOARD_LOAD_BALANCING true   // Enable intelligent task distribution
#define MULTIBOARD_TOPOLOGY_MAPPING true // Enable network topology mapping

// Integration Settings
#define MULTIBOARD_LORA_INTEGRATION true // Integrate with existing LoRa mesh system
#define MULTIBOARD_NETWORK_SELECTOR_INTEGRATION true // Integrate with network selector

// ===========================
// FIRMWARE INFORMATION
// ===========================

#define FIRMWARE_VERSION "2.5.0"         // Firmware version string (aligned with project version)
#define HARDWARE_VERSION "1.0"           // Hardware version string
#define BUILD_DATE __DATE__               // Build date (automatically set)
#define BUILD_TIME __TIME__               // Build time (automatically set)
#define DEVICE_NAME "WildlifeCam"         // Device identification name

// ===========================
// PIN DEFINITIONS BY CAMERA MODEL
// ===========================

// AI-Thinker ESP32-CAM Pin Definitions
#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32             // Power down pin
#define RESET_GPIO_NUM    -1             // Reset pin (not connected)
#define XCLK_GPIO_NUM      0             // External clock pin
#define SIOD_GPIO_NUM     26             // I2C data pin for camera - CONFLICTS with LORA_DIO0
#define SIOC_GPIO_NUM     27             // I2C clock pin for camera

// Camera Data Pins
#define Y9_GPIO_NUM       35             // Camera data bit 9
#define Y8_GPIO_NUM       34             // Camera data bit 8
#define Y7_GPIO_NUM       39             // Camera data bit 7
#define Y6_GPIO_NUM       36             // Camera data bit 6
#define Y5_GPIO_NUM       21             // Camera data bit 5 - CONFLICTS with RTC_SDA/BME280_SDA
#define Y4_GPIO_NUM       19             // Camera data bit 4 - CONFLICTS with LORA_MISO
#define Y3_GPIO_NUM       18             // Camera data bit 3 - CONFLICTS with LORA_SCK
#define Y2_GPIO_NUM        5             // Camera data bit 2 - CONFLICTS with LORA_CS
#define VSYNC_GPIO_NUM    25             // Vertical sync pin
#define HREF_GPIO_NUM     23             // Horizontal reference pin - CONFLICTS with LORA_MOSI
#define PCLK_GPIO_NUM     22             // Pixel clock pin - CONFLICTS with RTC_SCL/BME280_SCL

#define CAMERA_LED_PIN     4             // Built-in camera LED pin
#endif

// LilyGO T-Camera Plus S3 OV5640 V1.1 Pin Definitions
#if defined(CAMERA_MODEL_LILYGO_T_CAMERA_PLUS_S3)
#define PWDN_GPIO_NUM     -1             // Power down pin (not used)
#define RESET_GPIO_NUM    -1             // Reset pin (not used)
#define XCLK_GPIO_NUM     40             // External clock pin
#define SIOD_GPIO_NUM     17             // I2C data pin (SDA) for camera
#define SIOC_GPIO_NUM     18             // I2C clock pin (SCL) for camera

// Camera Data Pins
#define Y9_GPIO_NUM       39             // Camera data bit 9
#define Y8_GPIO_NUM       41             // Camera data bit 8
#define Y7_GPIO_NUM       42             // Camera data bit 7
#define Y6_GPIO_NUM       12             // Camera data bit 6
#define Y5_GPIO_NUM        3             // Camera data bit 5
#define Y4_GPIO_NUM       14             // Camera data bit 4
#define Y3_GPIO_NUM       47             // Camera data bit 3
#define Y2_GPIO_NUM       13             // Camera data bit 2
#define VSYNC_GPIO_NUM    21             // Vertical sync pin
#define HREF_GPIO_NUM     38             // Horizontal reference pin
#define PCLK_GPIO_NUM     11             // Pixel clock pin

#define CAMERA_LED_PIN    48             // Built-in status LED pin
#endif

// TODO: Add pin definitions for other camera models as needed

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

#endif // CONFIG_H