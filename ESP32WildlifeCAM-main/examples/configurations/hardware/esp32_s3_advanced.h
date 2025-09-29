/**
 * @file esp32_s3_advanced.h
 * @brief Advanced ESP32-S3 configuration for professional wildlife monitoring
 * 
 * This configuration includes:
 * - AI-powered wildlife classification
 * - Federated learning capabilities
 * - Multi-communication protocols
 * - Advanced power management
 * - Professional-grade features
 */

#ifndef ESP32_S3_ADVANCED_CONFIG_H
#define ESP32_S3_ADVANCED_CONFIG_H

// Hardware Configuration
#define BOARD_TYPE "ESP32-S3-CAM"
#define CAMERA_MODEL_ESP32S3_EYE
#define PSRAM_ENABLED true
#define PSRAM_SIZE_MB 8

// Enhanced Pin Definitions
#define PIR_SENSOR_PIN 12
#define STATUS_LED_PIN 33
#define FLASH_LED_PIN 4
#define IR_CUT_FILTER_PIN 38
#define SOLAR_VOLTAGE_PIN 1
#define BATTERY_VOLTAGE_PIN 2
#define TEMPERATURE_SENSOR_PIN 3
#define HUMIDITY_SENSOR_PIN 4

// Advanced Camera Settings
#define CAMERA_RESOLUTION FRAMESIZE_QXGA    // 2048x1536 (higher resolution)
#define CAMERA_QUALITY 8                    // Higher quality for AI
#define CAMERA_FRAMERATE 5                  // FPS for video mode
#define NIGHT_VISION_ENABLED true
#define HDR_ENABLED true
#define AUTO_EXPOSURE_ENABLED true
#define AUTO_WHITE_BALANCE_ENABLED true

// Capture Modes
#define BURST_MODE_ENABLED true
#define BURST_COUNT 5
#define BURST_INTERVAL_MS 500
#define VIDEO_MODE_ENABLED true
#define VIDEO_MAX_DURATION_SEC 30
#define TIMELAPSE_MODE_ENABLED true
#define TIMELAPSE_INTERVAL_SEC 300

// Advanced Motion Detection
#define MOTION_DETECTION_ENABLED true
#define ADVANCED_MOTION_ANALYSIS true
#define AI_MOTION_FILTERING true
#define MOTION_CONFIDENCE_THRESHOLD 0.75
#define PIR_THRESHOLD 70
#define OPTICAL_FLOW_ENABLED true
#define BACKGROUND_SUBTRACTION_ENABLED true

// AI and Machine Learning
#define AI_CLASSIFICATION_ENABLED true
#define WILDLIFE_SPECIES_COUNT 20
#define CLASSIFICATION_CONFIDENCE_THRESHOLD 0.8
#define EDGE_AI_OPTIMIZED true
#define MODEL_QUANTIZATION_ENABLED true
#define INFERENCE_ACCELERATION true

// Federated Learning
#define FEDERATED_LEARNING_ENABLED true
#define FL_CONTRIBUTION_ENABLED true
#define FL_MODEL_UPDATE_INTERVAL_HOURS 24
#define FL_PRIVACY_PRESERVATION true
#define FL_DIFFERENTIAL_PRIVACY_EPSILON 1.0

// Advanced Power Management
#define POWER_SAVE_MODE false               // Disabled for advanced features
#define INTELLIGENT_POWER_MANAGEMENT true
#define SOLAR_CHARGING_ENABLED true
#define BATTERY_CAPACITY_MAH 10000
#define POWER_BUDGET_OPTIMIZATION true
#define THERMAL_MANAGEMENT_ENABLED true
#define THERMAL_SHUTDOWN_TEMP 75            // Celsius

// Enhanced Storage
#define SD_CARD_ENABLED true
#define STORAGE_ENCRYPTION_ENABLED true
#define MAX_FILE_SIZE_MB 50                 // Larger files for high-res
#define STORAGE_COMPRESSION_ENABLED true
#define RAID_REDUNDANCY_ENABLED false       // Single SD for now
#define CLOUD_BACKUP_ENABLED true

// Multi-Protocol Communication
#define WIFI_ENABLED true
#define WIFI_MESH_ENABLED true
#define CELLULAR_ENABLED true
#define LORA_ENABLED true
#define SATELLITE_ENABLED true
#define BLUETOOTH_ENABLED true
#define ZIGBEE_ENABLED false                // Optional

// WiFi Advanced Settings
#define WIFI_DUAL_BAND_ENABLED true
#define WIFI_POWER_SAVE_MODE false
#define WIFI_MESH_MAX_NODES 20
#define WIFI_ENTERPRISE_SECURITY true

// Cellular Configuration
#define CELLULAR_TECHNOLOGY LTE_CAT_M1
#define CELLULAR_APN "wildlife.data"
#define CELLULAR_DATA_LIMIT_MB 1000
#define CELLULAR_ROAMING_ENABLED false

// LoRa Configuration
#define LORA_FREQUENCY 915000000            // 915 MHz (adjust by region)
#define LORA_BANDWIDTH 125000               // 125 kHz
#define LORA_SPREADING_FACTOR 12
#define LORA_CODING_RATE 8
#define LORA_TX_POWER 20                    // dBm
#define LORA_MESH_ENABLED true

// Advanced Features
#define WEB_SERVER_ENABLED true
#define REST_API_ENABLED true
#define WEBSOCKET_ENABLED true
#define STREAMING_ENABLED true
#define ANALYTICS_DASHBOARD_ENABLED true
#define REAL_TIME_MONITORING_ENABLED true
#define REMOTE_CONFIGURATION_ENABLED true

// Analytics and Monitoring
#define PERFORMANCE_MONITORING_ENABLED true
#define SYSTEM_HEALTH_MONITORING true
#define WILDLIFE_BEHAVIOR_ANALYSIS true
#define ENVIRONMENTAL_DATA_LOGGING true
#define PREDICTIVE_ANALYTICS_ENABLED true

// Security Features
#define SECURITY_HARDENED true
#define SECURE_BOOT_ENABLED true
#define ENCRYPTION_AES_256 true
#define DIGITAL_SIGNATURES_ENABLED true
#define INTRUSION_DETECTION_ENABLED true
#define API_AUTHENTICATION_REQUIRED true

// Environmental Adaptation
#define ENVIRONMENTAL_VARIANT_ENABLED true
#define AUTO_ENVIRONMENT_DETECTION true
#define WEATHER_COMPENSATION_ENABLED true
#define SEASONAL_ADJUSTMENT_ENABLED true

// Professional Timing
#define CAPTURE_INTERVAL_MINUTES 5          // More frequent captures
#define STATUS_UPDATE_INTERVAL_MINUTES 10   // Frequent status updates
#define HEARTBEAT_INTERVAL_MINUTES 5        // Regular heartbeat
#define SYSTEM_HEALTH_CHECK_INTERVAL_SEC 60 // Health monitoring

// Advanced Debug and Logging
#define DEBUG_ENABLED false                 // Production setting
#define VERBOSE_LOGGING_ENABLED false       // Performance optimization
#define PERFORMANCE_PROFILING_ENABLED true
#define TELEMETRY_ENABLED true
#define LOG_LEVEL LOG_LEVEL_WARNING

// Professional Environmental Limits
#define OPERATING_TEMP_MIN_C -30
#define OPERATING_TEMP_MAX_C 70
#define OPERATING_HUMIDITY_MAX 95
#define ALTITUDE_COMPENSATION_ENABLED true
#define UV_PROTECTION_ENABLED true

// Memory Management
#define HEAP_MONITORING_ENABLED true
#define MEMORY_FRAGMENTATION_PREVENTION true
#define GARBAGE_COLLECTION_OPTIMIZED true
#define STACK_OVERFLOW_PROTECTION true

// Professional Identification
#define DEVICE_NAME "WildlifeCam_Pro_S3"
#define FIRMWARE_VERSION "2.0.0"
#define HARDWARE_VERSION "ESP32-S3-CAM-Pro"
#define DEPLOYMENT_PROFILE "PROFESSIONAL"

// Quality Assurance
#define SELF_DIAGNOSTICS_ENABLED true
#define COMPONENT_HEALTH_MONITORING true
#define AUTOMATIC_CALIBRATION_ENABLED true
#define PERFORMANCE_BENCHMARKING_ENABLED true

#endif // ESP32_S3_ADVANCED_CONFIG_H