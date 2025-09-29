/**
 * @file mesh_config.h
 * @brief LoRa and Meshtastic Configuration for ESP32WildlifeCAM
 * 
 * Comprehensive configuration for LoRa mesh networking with support for:
 * - Multiple LoRa modules (SX1262/SX1268, SX1276/RFM95, SX1280, E22-900M30S)
 * - Regional frequency support (US, EU, AU, CN, JP, 2.4GHz)
 * - Wildlife-specific optimizations
 * - Power management integration
 * - GPIO pin conflict resolution
 */

#ifndef MESH_CONFIG_H
#define MESH_CONFIG_H

#include <Arduino.h>

// ===========================
// BOARD CONFIGURATION
// ===========================

// Board type detection for GPIO configuration
enum BoardType {
    BOARD_ESP32_CAM_AI_THINKER,
    BOARD_ESP32_S3_CAM,
    BOARD_ESP_EYE,
    BOARD_M5STACK_PSRAM,
    BOARD_CUSTOM
};

// GPIO pin configurations for different boards
#ifdef CAMERA_MODEL_AI_THINKER
    #define BOARD_TYPE BOARD_ESP32_CAM_AI_THINKER
    // AI-Thinker ESP32-CAM: Use alternative SPI pins to avoid camera conflicts
    #define LORA_SPI_SCLK   12  // Alternative SPI clock (was GPIO 18)
    #define LORA_SPI_MISO   13  // Alternative SPI MISO (was GPIO 19) 
    #define LORA_SPI_MOSI   15  // Alternative SPI MOSI (was GPIO 23)
    #define LORA_CS         14  // Chip select (was GPIO 5)
    #define LORA_RST        2   // Reset pin (was GPIO 14)
    #define LORA_DIO0       4   // DIO0 interrupt (was GPIO 26)
    #define LORA_DIO1       16  // DIO1 interrupt (optional)
    #define LORA_DIO2       17  // DIO2 interrupt (optional)
#elif defined(CAMERA_MODEL_ESP32S3_EYE)
    #define BOARD_TYPE BOARD_ESP32_S3_CAM
    // ESP32-S3: More GPIO pins available, use standard SPI
    #define LORA_SPI_SCLK   18
    #define LORA_SPI_MISO   19
    #define LORA_SPI_MOSI   23
    #define LORA_CS         5
    #define LORA_RST        14
    #define LORA_DIO0       26
    #define LORA_DIO1       27
    #define LORA_DIO2       33
#else
    #define BOARD_TYPE BOARD_CUSTOM
    // Default configuration - user must define pins
    #ifndef LORA_SPI_SCLK
        #define LORA_SPI_SCLK   18
        #define LORA_SPI_MISO   19
        #define LORA_SPI_MOSI   23
        #define LORA_CS         5
        #define LORA_RST        14
        #define LORA_DIO0       26
        #define LORA_DIO1       35
        #define LORA_DIO2       34
    #endif
#endif

// ===========================
// REGIONAL FREQUENCY CONFIGURATION
// ===========================

// Regional frequency bands
enum LoRaRegion {
    REGION_US915,       // United States: 902-928 MHz
    REGION_EU868,       // Europe: 863-870 MHz  
    REGION_EU433,       // Europe: 433 MHz
    REGION_AU915,       // Australia: 915-928 MHz
    REGION_CN470,       // China: 470-510 MHz
    REGION_JP920,       // Japan: 920-923 MHz
    REGION_2_4GHZ       // 2.4 GHz ISM band
};

// Default region (can be overridden)
#ifndef LORA_REGION
    #define LORA_REGION REGION_US915
#endif

// Frequency definitions by region
#if LORA_REGION == REGION_US915
    #define LORA_FREQUENCY 915E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 30      // 30 dBm max in US
    #define LORA_DUTY_CYCLE_LIMIT 100  // No duty cycle limit in US
#elif LORA_REGION == REGION_EU868
    #define LORA_FREQUENCY 868E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 14      // 14 dBm max in EU
    #define LORA_DUTY_CYCLE_LIMIT 1    // 1% duty cycle in EU
#elif LORA_REGION == REGION_EU433
    #define LORA_FREQUENCY 433E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 10      // 10 dBm max at 433 MHz
    #define LORA_DUTY_CYCLE_LIMIT 10   // 10% duty cycle at 433 MHz
#elif LORA_REGION == REGION_AU915
    #define LORA_FREQUENCY 915E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 30      // 30 dBm max in AU
    #define LORA_DUTY_CYCLE_LIMIT 100  // No duty cycle limit in AU
#elif LORA_REGION == REGION_CN470
    #define LORA_FREQUENCY 470E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 17      // 17 dBm max in CN
    #define LORA_DUTY_CYCLE_LIMIT 100  // No duty cycle limit in CN
#elif LORA_REGION == REGION_JP920
    #define LORA_FREQUENCY 920E6
    #define LORA_BANDWIDTH 125E3
    #define LORA_MAX_POWER 13      // 13 dBm max in JP
    #define LORA_DUTY_CYCLE_LIMIT 100  // No duty cycle limit in JP
#elif LORA_REGION == REGION_2_4GHZ
    #define LORA_FREQUENCY 2450E6
    #define LORA_BANDWIDTH 812E3   // 812.5 kHz for 2.4 GHz
    #define LORA_MAX_POWER 10      // 10 dBm for 2.4 GHz
    #define LORA_DUTY_CYCLE_LIMIT 100  // No duty cycle limit
#endif

// ===========================
// LORA MODULE CONFIGURATION
// ===========================

// Supported LoRa modules
enum LoRaModule {
    MODULE_SX1276,      // SX1276/RFM95/RFM96/RFM98
    MODULE_SX1262,      // SX1262/SX1268
    MODULE_SX1280,      // SX1280 (2.4 GHz)
    MODULE_E22_900M30S, // E22-900M30S
    MODULE_AUTO_DETECT  // Auto-detect module type
};

#ifndef LORA_MODULE_TYPE
    #define LORA_MODULE_TYPE MODULE_AUTO_DETECT
#endif

// ===========================
// MESHTASTIC PROTOCOL CONFIGURATION
// ===========================

// Channel presets (from Meshtastic protocol)
enum MeshChannelPreset {
    CHANNEL_LONG_SLOW,    // SF=11, BW=125, CR=4/8 - Maximum range
    CHANNEL_LONG_FAST,    // SF=11, BW=250, CR=4/6 - Good range, faster
    CHANNEL_MEDIUM_SLOW,  // SF=10, BW=125, CR=4/8 - Balanced
    CHANNEL_MEDIUM_FAST,  // SF=10, BW=250, CR=4/6 - Balanced, faster
    CHANNEL_SHORT_SLOW,   // SF=9,  BW=125, CR=4/8 - Short range
    CHANNEL_SHORT_FAST,   // SF=7,  BW=250, CR=4/5 - Short range, fastest
    CHANNEL_CUSTOM        // User-defined parameters
};

#ifndef MESH_CHANNEL_PRESET
    #define MESH_CHANNEL_PRESET CHANNEL_LONG_SLOW  // Default to maximum range
#endif

// Channel configurations
#if MESH_CHANNEL_PRESET == CHANNEL_LONG_SLOW
    #define LORA_SPREADING_FACTOR 11
    #define LORA_SIGNAL_BANDWIDTH 125E3
    #define LORA_CODING_RATE 8
    #define LORA_TX_POWER 20
#elif MESH_CHANNEL_PRESET == CHANNEL_LONG_FAST
    #define LORA_SPREADING_FACTOR 11
    #define LORA_SIGNAL_BANDWIDTH 250E3
    #define LORA_CODING_RATE 6
    #define LORA_TX_POWER 20
#elif MESH_CHANNEL_PRESET == CHANNEL_MEDIUM_SLOW
    #define LORA_SPREADING_FACTOR 10
    #define LORA_SIGNAL_BANDWIDTH 125E3
    #define LORA_CODING_RATE 8
    #define LORA_TX_POWER 17
#elif MESH_CHANNEL_PRESET == CHANNEL_MEDIUM_FAST
    #define LORA_SPREADING_FACTOR 10
    #define LORA_SIGNAL_BANDWIDTH 250E3
    #define LORA_CODING_RATE 6
    #define LORA_TX_POWER 17
#elif MESH_CHANNEL_PRESET == CHANNEL_SHORT_SLOW
    #define LORA_SPREADING_FACTOR 9
    #define LORA_SIGNAL_BANDWIDTH 125E3
    #define LORA_CODING_RATE 8
    #define LORA_TX_POWER 14
#elif MESH_CHANNEL_PRESET == CHANNEL_SHORT_FAST
    #define LORA_SPREADING_FACTOR 7
    #define LORA_SIGNAL_BANDWIDTH 250E3
    #define LORA_CODING_RATE 5
    #define LORA_TX_POWER 14
#elif MESH_CHANNEL_PRESET == CHANNEL_CUSTOM
    // Use user-defined values or defaults
    #ifndef LORA_SPREADING_FACTOR
        #define LORA_SPREADING_FACTOR 11
    #endif
    #ifndef LORA_SIGNAL_BANDWIDTH
        #define LORA_SIGNAL_BANDWIDTH 125E3
    #endif
    #ifndef LORA_CODING_RATE
        #define LORA_CODING_RATE 8
    #endif
    #ifndef LORA_TX_POWER
        #define LORA_TX_POWER 20
    #endif
#endif

// Additional LoRa parameters
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYNC_WORD 0x12        // Private networks use 0x12, LoRaWAN uses 0x34
#define LORA_CRC_ENABLED true
#define LORA_EXPLICIT_HEADER true

// ===========================
// MESH NETWORK CONFIGURATION
// ===========================

// Node configuration
#define MESH_NODE_ID_AUTO true     // Auto-generate node ID from MAC address
#define MESH_MAX_NODES 255         // Maximum nodes in network
#define MESH_MAX_HOPS 7            // Maximum message hops
#define MESH_HOP_LIMIT 3           // Default hop limit for new messages

// Timing configuration
#define MESH_BEACON_INTERVAL 60000      // 1 minute beacon interval
#define MESH_ROUTE_TIMEOUT 300000       // 5 minutes route timeout
#define MESH_ACK_TIMEOUT 5000           // 5 seconds acknowledgment timeout
#define MESH_RETRY_COUNT 3              // Message retry attempts
#define MESH_NEIGHBOR_TIMEOUT 180000    // 3 minutes neighbor timeout

// Message queue configuration
#define MESH_MESSAGE_QUEUE_SIZE 32      // Message queue size
#define MESH_MAX_PACKET_SIZE 255        // Maximum packet size
#define MESH_FRAGMENT_SIZE 200          // Fragment size for large messages

// Encryption configuration
#define MESH_ENCRYPTION_ENABLED true
#define MESH_CHANNEL_KEY "WildlifeCam2024"  // Default channel key
#define MESH_KEY_LENGTH 16              // AES-128 key length

// ===========================
// WILDLIFE-SPECIFIC CONFIGURATION
// ===========================

// Wildlife telemetry intervals
#define WILDLIFE_TELEMETRY_INTERVAL 300000     // 5 minutes telemetry
#define WILDLIFE_STATUS_INTERVAL 60000         // 1 minute status updates
#define WILDLIFE_HEARTBEAT_INTERVAL 30000      // 30 seconds heartbeat

// Image transmission configuration
#define IMAGE_TRANSMISSION_ENABLED true
#define IMAGE_CHUNK_SIZE 200                   // Bytes per chunk
#define IMAGE_COMPRESSION_LEVEL 75             // JPEG compression level
#define IMAGE_THUMBNAIL_SIZE 64                // Thumbnail width/height
#define IMAGE_MAX_RETRIES 5                    // Maximum retransmission attempts

// Environmental sensor configuration
#define ENV_SENSOR_INTERVAL 60000              // 1 minute sensor readings
#define ENV_SENSOR_BUFFER_SIZE 10              // Buffer for sensor readings
#define BATTERY_STATUS_INTERVAL 300000         // 5 minutes battery status

// Motion detection mesh integration
#define MOTION_MESH_ALERT_ENABLED true
#define MOTION_MESH_ALERT_TIMEOUT 30000        // 30 seconds alert timeout
#define MOTION_MESH_COOLDOWN 60000             // 1 minute cooldown between alerts

// GPS configuration
#define GPS_MESH_ENABLED false                 // GPS disabled by default
#define GPS_UPDATE_INTERVAL 600000             // 10 minutes GPS updates
#define GPS_COORDINATE_PRECISION 6             // Decimal places for coordinates

// ===========================
// POWER MANAGEMENT CONFIGURATION
// ===========================

// Power modes for mesh operation
enum MeshPowerMode {
    POWER_MODE_ALWAYS_ON,       // Always listening (high power)
    POWER_MODE_SCHEDULED,       // Scheduled wake/sleep cycles
    POWER_MODE_TRIGGERED,       // Wake on motion/event only
    POWER_MODE_ULTRA_LOW        // Minimal mesh participation
};

#define MESH_POWER_MODE POWER_MODE_TRIGGERED   // Default power mode

// Power-aware mesh timing
#define MESH_SLEEP_DURATION 300                // 5 minutes sleep in triggered mode
#define MESH_WAKE_DURATION 60                  // 1 minute wake time
#define MESH_LOW_BATTERY_THRESHOLD 3.3         // Voltage threshold for power saving
#define MESH_CRITICAL_BATTERY_THRESHOLD 3.0    // Voltage for emergency mode

// Solar power optimization
#define SOLAR_MESH_OPTIMIZATION true
#define SOLAR_PEAK_HOURS_START 10              // 10 AM
#define SOLAR_PEAK_HOURS_END 16                // 4 PM
#define SOLAR_MIN_VOLTAGE 3.7                  // Minimum voltage for full operation

// ===========================
// MQTT BRIDGE CONFIGURATION
// ===========================

#define MQTT_BRIDGE_ENABLED false             // MQTT bridge disabled by default
#define MQTT_BRIDGE_NODE_ID 1                 // Node ID for MQTT bridge
#define MQTT_BROKER_HOST "mqtt.meshtastic.org"
#define MQTT_BROKER_PORT 1883
#define MQTT_TOPIC_PREFIX "wildlife/"
#define MQTT_KEEPALIVE 60
#define MQTT_QOS 1

// ===========================
// DEBUG AND DIAGNOSTICS
// ===========================

#define MESH_DEBUG_ENABLED true
#define MESH_STATS_ENABLED true
#define MESH_DIAGNOSTICS_INTERVAL 600000      // 10 minutes diagnostics
#define MESH_LOG_LEVEL 2                      // 0=ERROR, 1=WARN, 2=INFO, 3=DEBUG

// Performance monitoring
#define MESH_MONITOR_RSSI true
#define MESH_MONITOR_SNR true
#define MESH_MONITOR_PACKET_LOSS true
#define MESH_MONITOR_ROUTING_TABLE true

#endif // MESH_CONFIG_H