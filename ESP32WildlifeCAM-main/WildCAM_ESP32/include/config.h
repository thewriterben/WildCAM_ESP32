#ifndef CONFIG_H
#define CONFIG_H

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

// Forward compatibility - include pins.h for pin definitions
#include "pins.h"

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
#define BEHAVIOR_HISTORY_SIZE 10           // Number of frames for temporal analysis

// Model Management
#define ADAPTIVE_MODEL_SELECTION true     // Enable adaptive model selection
#define MODEL_CALIBRATION_ENABLED true    // Enable runtime calibration
#define MODEL_CACHE_SIZE 3                 // Number of models to cache

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// Battery Monitoring
#define BATTERY_LOW_THRESHOLD 3.3f        // Low battery voltage
#define BATTERY_CRITICAL_THRESHOLD 3.0f   // Critical battery voltage
#define BATTERY_FULL_VOLTAGE 4.2f         // Full battery voltage

// Solar Charging
#define SOLAR_VOLTAGE_THRESHOLD 5.0f      // Minimum solar voltage for charging
#define CHARGING_CURRENT_LIMIT 500        // Maximum charging current (mA)

// Power Modes
#define POWER_SAVE_MODE_ENABLED true      // Enable power saving
#define DEEP_SLEEP_DURATION 60000         // Deep sleep duration (ms)
#define ADAPTIVE_DUTY_CYCLE true          // Enable adaptive duty cycling

// CPU Frequency Settings
#define MAX_CPU_FREQ_MHZ 240              // Maximum CPU frequency
#define MIN_CPU_FREQ_MHZ 80               // Minimum CPU frequency for power saving
#define BALANCED_CPU_FREQ_MHZ 160         // Balanced performance frequency

// ===========================
// DATA COLLECTION CONFIGURATION
// ===========================

// Storage Settings
#define SD_CARD_ENABLED true              // Enable SD card storage
#define IMAGE_FOLDER "/images"            // Image storage folder
#define LOG_FOLDER "/logs"                // Log storage folder
#define DATA_FOLDER "/data"               // Data storage folder

// Image Management
#define MAX_DAILY_TRIGGERS 100            // Maximum images per day
#define IMAGE_COMPRESSION_ENABLED true    // Enable image compression
#define METADATA_ENABLED true             // Save metadata with images

// Data Organization
#define SPECIES_FOLDERS_ENABLED true      // Organize by species
#define TIME_BASED_FOLDERS true           // Organize by time
#define AUTO_DELETE_OLD_DATA true         // Delete old data when storage full

// ===========================
// SYSTEM CONFIGURATION
// ===========================

// Active Hours (24-hour format)
#define ACTIVE_START_HOUR 6               // Start monitoring at 6 AM
#define ACTIVE_END_HOUR 22                // Stop monitoring at 10 PM
#define NIGHT_MODE_ENABLED true           // Enable night mode operation

// System Timing
#define STATUS_CHECK_INTERVAL 30000       // Status check every 30 seconds
#define SENSOR_UPDATE_INTERVAL 5000       // Sensor update every 5 seconds
#define AI_ANALYSIS_INTERVAL 2000         // AI analysis every 2 seconds

// Error Handling
#define MAX_RETRY_ATTEMPTS 3              // Maximum retry attempts
#define ERROR_LOG_ENABLED true            // Enable error logging
#define WATCHDOG_ENABLED true             // Enable watchdog timer

// Debug Configuration
#define DEBUG_ENABLED true                // Enable debug output
#define DEBUG_LEVEL 2                     // Debug level (0-3)
#define SERIAL_BAUDRATE 115200            // Serial communication speed

// ===========================
// NETWORK CONFIGURATION (Optional)
// ===========================

// WiFi Settings (for future expansion)
#define WIFI_ENABLED false                // WiFi disabled for power saving
#define WIFI_SSID ""                      // WiFi network name
#define WIFI_PASSWORD ""                  // WiFi password

// LoRa Settings (for future expansion)
#define LORA_ENABLED false                // LoRa disabled by default
#define LORA_FREQUENCY 915E6              // LoRa frequency (915 MHz)

// ===========================
// ADVANCED CAPTURE MODE CONFIGURATION
// ===========================

// Burst mode configuration
#define BURST_MODE_ENABLED true
#define BURST_DEFAULT_COUNT 3
#define BURST_DEFAULT_INTERVAL_MS 1000
#define BURST_MAX_COUNT 10
#define BURST_MIN_INTERVAL_MS 100
#define BURST_MAX_INTERVAL_MS 5000

// Video recording configuration  
#define VIDEO_MODE_ENABLED true
#define VIDEO_DEFAULT_DURATION_S 10
#define VIDEO_MAX_DURATION_S 60
#define VIDEO_FRAME_RATE 10
#define VIDEO_QUALITY 12

// Time-lapse configuration
#define TIMELAPSE_MODE_ENABLED true
#define TIMELAPSE_MIN_INTERVAL_S 10
#define TIMELAPSE_MAX_INTERVAL_H 24
#define TIMELAPSE_DEFAULT_INTERVAL_S 60

// Night vision configuration
#define IR_LED_ENABLED true
#define IR_CUT_FILTER_ENABLED true
#define AUTO_NIGHT_MODE_ENABLED true
// Pin definitions are in pins.h

// ===========================
// AUDIO CLASSIFICATION CONFIGURATION
// ===========================

// Audio System Enable/Disable
#define AUDIO_CLASSIFICATION_ENABLED true // Enable audio-based wildlife classification
#define I2S_MICROPHONE_ENABLED true       // Enable I2S digital microphone
#define ANALOG_MICROPHONE_ENABLED false   // Enable analog microphone (alternative)

// I2S Microphone Configuration
#define I2S_WS_PIN 25                     // I2S Word Select pin
#define I2S_SCK_PIN 26                    // I2S Serial Clock pin (shared with camera I2C)
#define I2S_SD_PIN 22                     // I2S Serial Data pin (shared with camera)
#define I2S_SAMPLE_RATE 16000             // Audio sample rate (Hz)
#define I2S_BITS_PER_SAMPLE_16BIT 16      // Bits per sample
#define AUDIO_DMA_BUFFER_COUNT 4          // Number of DMA buffers
#define AUDIO_DMA_BUFFER_SIZE 1024        // Size of each DMA buffer

// Audio Classification Settings
#define AUDIO_CLASSIFICATION_SENSITIVITY 0.7f  // Classification sensitivity (0.0-1.0)
#define AUDIO_NOISE_FILTERING true             // Enable noise filtering
#define AUDIO_BUFFER_SIZE 4096                 // Audio processing buffer size
#define AUDIO_ANALYSIS_INTERVAL_MS 100         // Audio analysis interval

// Multi-Modal Detection
#define MULTIMODAL_DETECTION_ENABLED true      // Enable audio+visual fusion
#define AUDIO_VISUAL_CORRELATION_TIMEOUT 5000  // Max time between audio/visual detections (ms)
#define COMBINED_CONFIDENCE_THRESHOLD 0.8f     // Minimum combined confidence for positive detection

// Wildlife Audio Signatures
#define PRELOAD_BIRD_SIGNATURES true           // Load bird audio signatures
#define PRELOAD_MAMMAL_SIGNATURES true         // Load mammal audio signatures
#define CUSTOM_SPECIES_SIGNATURES true         // Enable custom species signatures

// Audio Power Management
#define AUDIO_POWER_MANAGEMENT true            // Enable audio power saving
#define AUDIO_IDLE_TIMEOUT_MS 30000           // Audio idle timeout
#define AUDIO_LOW_POWER_MODE true              // Enable audio low power mode

// ===========================
// SERVO CONTROL CONFIGURATION
// ===========================

// Pan/Tilt System
#define PAN_TILT_ENABLED true             // Enable pan/tilt servo system
#define PAN_SERVO_PIN 16                  // GPIO pin for pan servo (SPARE_GPIO_1)
#define TILT_SERVO_PIN 17                 // GPIO pin for tilt servo (SPARE_GPIO_2)

// Servo Settings
#define SERVO_POWER_TIMEOUT_MS 5000       // Auto-detach timeout for power saving
#define SERVO_MOVEMENT_SPEED 90.0f        // Default movement speed (degrees/second)
#define SERVO_SMOOTH_MOVEMENT true        // Enable smooth movements by default

// Pan Servo Configuration
#define PAN_MIN_ANGLE 0.0f                // Minimum pan angle (degrees)
#define PAN_MAX_ANGLE 180.0f              // Maximum pan angle (degrees)
#define PAN_CENTER_ANGLE 90.0f            // Center pan position (degrees)
#define PAN_INVERT_DIRECTION false        // Invert pan direction

// Tilt Servo Configuration
#define TILT_MIN_ANGLE 45.0f              // Minimum tilt angle (degrees) - looking up
#define TILT_MAX_ANGLE 135.0f             // Maximum tilt angle (degrees) - looking down
#define TILT_CENTER_ANGLE 90.0f           // Center tilt position (degrees) - horizontal
#define TILT_INVERT_DIRECTION false       // Invert tilt direction

// Scanning Configuration
#define AUTO_SCANNING_ENABLED true        // Enable automatic scanning when idle
#define SCAN_DWELL_TIME_MS 2000           // Time to pause at each scan position
#define SCAN_MOVE_TIME_MS 1000            // Time allowed for movement between positions
#define SCAN_PAN_STEP 30.0f               // Pan angle step for scanning
#define SCAN_TILT_STEP 30.0f              // Tilt angle step for scanning
#define DEFAULT_SCAN_PATTERN 1            // 0=None, 1=Horizontal, 2=Vertical, 3=Grid, 4=Spiral, 5=Random

// Tracking Configuration
#define WILDLIFE_TRACKING_ENABLED true    // Enable automatic wildlife tracking
#define TRACKING_TIMEOUT_MS 10000         // Stop tracking if no updates for this time
#define TRACKING_MIN_CONFIDENCE 0.6f      // Minimum confidence to start/continue tracking
#define TRACKING_PREDICTION_TIME 1000     // Predict target movement for this time (ms)

// Power Management Integration
#define SERVO_POWER_MANAGEMENT true       // Integrate servo power with main power management
#define SERVO_LOW_POWER_MODE true         // Enable servo-specific power saving modes
#define SERVO_SLEEP_MODE_DELAY 30000      // Delay before entering servo sleep mode (ms)

// ===========================
// ADVANCED FEATURES
// ===========================

// Weather Filtering
#define WEATHER_FILTERING_ENABLED true    // Enable weather-based filtering
#define WIND_THRESHOLD 15                 // Wind speed threshold (km/h)
#define RAIN_THRESHOLD 0.5                // Rain threshold (mm/h)

// Intelligent Scheduling
#define INTELLIGENT_SCHEDULING true       // Enable AI-based scheduling
#define LEARNING_MODE_ENABLED true        // Enable continuous learning
#define ADAPTATION_ENABLED true           // Enable environmental adaptation

#endif // CONFIG_H