/**
 * @file config_lora_mesh_node.h
 * @brief Configuration template for LoRa mesh relay node (no camera)
 * @hardware AI-Thinker ESP32-CAM or ESP32-DevKit
 * @use_case LoRa mesh network relay/gateway node with sensors
 * 
 * FEATURES:
 *   ✗ Camera (disabled for LoRa)
 *   ✓ SD Card storage (data logging)
 *   ✓ LoRa mesh networking
 *   ✓ PIR motion sensor
 *   ✓ WiFi connectivity
 *   ✓ BME280 environmental sensor (I2C)
 *   ✓ Battery voltage monitoring
 *   ✓ Solar panel voltage monitoring
 *   ✓ LED indicators
 * 
 * AVAILABLE GPIO: Many (camera pins freed up)
 * 
 * To use this configuration:
 *   1. Copy this file to firmware/config.h
 *   2. Adjust LoRa frequency for your region
 *   3. Compile with: pio run -e esp32cam_mesh
 * 
 * NOTE: This configuration works on AI-Thinker ESP32-CAM with camera disabled
 *       to free up GPIO pins for LoRa module.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===========================
// HARDWARE CONFIGURATION
// ===========================

// Board Selection
#define CAMERA_MODEL_AI_THINKER       // AI-Thinker ESP32-CAM (camera disabled)

// Feature Enables
#define CAMERA_ENABLED false          // ✗ Camera DISABLED to free GPIO for LoRa
#define SD_CARD_ENABLED true          // ✓ SD card for data logging
#define LORA_ENABLED true             // ✓ LoRa mesh networking (primary function)
#define PIR_SENSOR_ENABLED true       // ✓ PIR motion detection
#define BME280_SENSOR_ENABLED true    // ✓ Environmental sensor via I2C
#define BATTERY_MONITOR_ENABLED true  // ✓ Battery voltage monitoring
#define SOLAR_VOLTAGE_MONITORING_ENABLED true // ✓ Solar panel monitoring
#define LED_INDICATORS_ENABLED false  // ✗ Conflicts with SD card

// ===========================
// LORA MESH CONFIGURATION
// ===========================

#define LORA_NODE_TYPE "RELAY"            // Node type: RELAY, GATEWAY, or SENSOR
#define LORA_FREQUENCY 915E6              // 915 MHz (NA), 868E6 (EU), 433E6 (Asia)
#define LORA_BANDWIDTH 125E3              // 125 kHz bandwidth
#define LORA_SPREADING_FACTOR 9           // SF9 for good range/speed balance
#define LORA_TX_POWER 20                  // Maximum transmit power (20 dBm)
#define LORA_CODING_RATE 5                // 4/5 coding rate
#define LORA_PREAMBLE_LENGTH 8            // Preamble length

// Mesh Network Settings
#define LORA_HEALTH_CHECK_INTERVAL 30000  // Check mesh health every 30 seconds
#define LORA_NODE_TIMEOUT 300000          // 5 minutes timeout
#define LORA_MAX_RETRIES 3                // Retry failed transmissions
#define LORA_RETRY_DELAY_MS 1000          // 1 second between retries
#define LORA_MAX_HOPS 5                   // Maximum mesh hops

// Relay Node Specific
#define LORA_RELAY_ENABLED true           // Enable message relaying
#define LORA_MESSAGE_CACHE_SIZE 50        // Cache to prevent loops
#define LORA_RELAY_PRIORITY_ROUTING true  // Prioritize by signal strength

// ===========================
// NETWORK CONFIGURATION
// ===========================

// WiFi Settings (for gateway nodes)
#define WIFI_ENABLED true
#define WIFI_SSID "YourWildlifeNetwork"   // ⚠️ CHANGE THIS
#define WIFI_PASSWORD "YourPassword"      // ⚠️ CHANGE THIS
#define WIFI_CONNECTION_TIMEOUT 10000
#define WIFI_MAX_RETRIES 5
#define WIFI_RETRY_BASE_DELAY 1000

// Gateway Mode (forward LoRa data to WiFi/Internet)
#define LORA_GATEWAY_MODE true            // Act as LoRa-to-WiFi gateway
#define GATEWAY_FORWARD_ALL_MESSAGES true // Forward all mesh messages

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
#define NETWORK_STATUS_LOG_INTERVAL 60000 // Log every minute

// ===========================
// POWER MANAGEMENT
// ===========================

#define DEEP_SLEEP_ENABLED false          // Stay awake for mesh relaying
#define LOW_BATTERY_THRESHOLD 3.3         // Volts
#define CRITICAL_BATTERY_THRESHOLD 3.0    // Volts
#define ADAPTIVE_DUTY_CYCLE true          // Adjust operation based on battery
#define SOLAR_CHARGING_ENABLED true       // Solar panel charging
#define POWER_SAVE_MODE_ENABLED true      // Reduce TX power when battery low

// ===========================
// STORAGE CONFIGURATION
// ===========================

#define SD_CARD_FORMAT_IF_FAILED true
#define MAX_LOG_FILES 100
#define AUTO_DELETE_OLD_LOGS true
#define KEEP_LOGS_DAYS 30                 // Keep 30 days of mesh logs

// ===========================
// SENSOR CONFIGURATION
// ===========================

#define BME280_I2C_ADDRESS 0x76
#define BME280_SAMPLE_INTERVAL_MS 300000  // Read every 5 minutes
#define SENSOR_DATA_LOGGING_ENABLED true
#define SENSOR_DATA_BROADCAST_ENABLED true // Broadcast sensor data over mesh

// PIR Configuration (detect activity near relay node)
#define PIR_TRIGGER_ENABLED true
#define PIR_COOLDOWN_MS 30000             // 30 seconds cooldown
#define PIR_LOG_ACTIVITY true             // Log detected motion

// ===========================
// MESH SECURITY
// ===========================

#define LORA_ENCRYPTION_ENABLED true      // Encrypt mesh messages
#define LORA_ENCRYPTION_KEY "YourSecretKey1234567890123456" // 32-byte key
#define LORA_AUTHENTICATE_NODES true      // Authenticate mesh nodes
#define LORA_NODE_ID_REQUIRED true        // Require unique node IDs

// ===========================
// DIAGNOSTICS
// ===========================

#define ENABLE_MESH_DIAGNOSTICS true      // Enable detailed mesh diagnostics
#define LOG_MESH_TOPOLOGY true            // Log mesh network topology
#define LOG_SIGNAL_STRENGTH true          // Log RSSI/SNR values
#define ENABLE_PERFORMANCE_STATS true     // Track relay performance

#endif // CONFIG_H