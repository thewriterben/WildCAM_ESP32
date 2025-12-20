#ifndef CONFIG_H
#define CONFIG_H

//==============================================================================
// FIRMWARE VERSION
//==============================================================================

/**
 * @brief Firmware version string
 * @note Displayed in welcome banner and system status
 */
#define FIRMWARE_VERSION "1.0.0"

//==============================================================================
// NETWORK SETTINGS
//==============================================================================

// Try to include secrets.h for WiFi credentials
// If secrets.h doesn't exist, fall back to default values below
// Note: Requires C++17 compiler support for __has_include directive
#if __has_include("secrets.h")
    #include "secrets.h"
#else
    /**
     * @brief WiFi SSID (network name)
     * @note For security, create include/secrets.h with your credentials
     * @note See include/secrets.h.example for template
     */
    #define WIFI_SSID "YourSSID"

    /**
     * @brief WiFi password
     * @note For security, create include/secrets.h with your credentials
     * @note See include/secrets.h.example for template
     */
    #define WIFI_PASSWORD "YourPassword"
#endif

/**
 * @brief Web server port
 * @note Default HTTP port is 80
 */
#define WEB_SERVER_PORT 80

/**
 * @brief WebSocket server port
 * @note Used for real-time communication
 */
#define WEB_SOCKET_PORT 81

//==============================================================================
// PIN DEFINITIONS
//==============================================================================

/**
 * @brief PIR motion sensor GPIO pin
 * @note GPIO pin for motion detection sensor
 */
#define PIR_SENSOR_PIN 13

/**
 * @brief Battery monitor ADC pin
 * @note Analog pin for reading battery voltage (changed from 34 to avoid Y8_GPIO_NUM conflict)
 */
#define BATTERY_MONITOR_PIN 33

/**
 * @brief SD card chip select pin (SPI mode)
 * @note GPIO pin for SD card SPI communication
 */
#define SD_CS_PIN 5

/**
 * @brief Flash LED GPIO pin
 * @note GPIO pin for camera flash LED control
 */
#define FLASH_LED_PIN 4

//==============================================================================
// TIMING CONSTANTS
//==============================================================================

/**
 * @brief Motion detection debounce time in milliseconds
 * @note Prevents multiple triggers from a single motion event
 * @default 2000ms (2 seconds)
 */
#define MOTION_DEBOUNCE_MS 2000

/**
 * @brief Deep sleep duration in seconds
 * @note Time to sleep in low-power mode between wake events
 * @default 300 seconds (5 minutes)
 */
#define DEEP_SLEEP_DURATION_SEC 300

/**
 * @brief Image capture delay in milliseconds
 * @note Delay before capturing image after motion detection
 * @default 500ms
 */
#define IMAGE_CAPTURE_DELAY_MS 500

/**
 * @brief Motion sensor cooldown time in milliseconds
 * @note Minimum time between motion triggers
 * @default 5000ms (5 seconds)
 */
#define MOTION_COOLDOWN_MS 5000

/**
 * @brief Startup delay in milliseconds
 * @note Initial delay after Serial.begin() for serial monitor to connect
 * @default 1000ms (1 second)
 */
#define STARTUP_DELAY_MS 1000

/**
 * @brief Error halt delay in milliseconds
 * @note Delay in error loop when system halts due to initialization failure
 * @default 1000ms (1 second)
 */
#define ERROR_HALT_DELAY_MS 1000

/**
 * @brief WiFi connection maximum attempts
 * @note Maximum number of connection attempts before giving up
 * @default 20 attempts
 */
#define WIFI_CONNECT_MAX_ATTEMPTS 20

/**
 * @brief WiFi connection retry delay in milliseconds
 * @note Delay between WiFi connection attempts
 * @default 500ms
 */
#define WIFI_CONNECT_RETRY_DELAY_MS 500

/**
 * @brief Battery check interval in milliseconds
 * @note How often to check battery voltage and percentage
 * @default 60000ms (60 seconds)
 */
#define BATTERY_CHECK_INTERVAL_MS 60000

/**
 * @brief Main loop delay in milliseconds
 * @note Small delay at end of main loop to prevent CPU spinning
 * @default 100ms
 */
#define MAIN_LOOP_DELAY_MS 100

//==============================================================================
// CAMERA SETTINGS
//==============================================================================

/**
 * @brief Camera model definition
 * @note AI-Thinker ESP32-CAM board
 */
#define CAMERA_MODEL_AI_THINKER

/**
 * @brief Default JPEG quality setting
 * @note Range: 1-63, lower values mean higher quality
 * @default 10 (high quality)
 */
#define DEFAULT_JPEG_QUALITY 10

/**
 * @brief Default camera frame size/resolution
 * @note FRAMESIZE_UXGA = 1600x1200 pixels
 * @default FRAMESIZE_UXGA
 */
#define DEFAULT_FRAME_SIZE FRAMESIZE_UXGA

/**
 * @brief Motion detection sensitivity
 * @note Range: 0-100, higher values are more sensitive
 * @default 50
 */
#define MOTION_SENSITIVITY 50

//==============================================================================
// POWER MANAGEMENT
//==============================================================================

/**
 * @brief Battery low voltage threshold
 * @note Voltage below this triggers low battery warning
 * @default 3.3V
 */
#define BATTERY_LOW_THRESHOLD 3.3

/**
 * @brief Battery critical voltage threshold
 * @note Voltage below this triggers emergency power saving
 * @default 3.0V
 */
#define BATTERY_CRITICAL_THRESHOLD 3.0

/**
 * @brief Battery minimum voltage
 * @note Minimum expected battery voltage
 * @default 3.3V
 */
#define BATTERY_VOLTAGE_MIN 3.3

/**
 * @brief Battery maximum voltage
 * @note Maximum expected battery voltage (fully charged)
 * @default 4.2V
 */
#define BATTERY_VOLTAGE_MAX 4.2

//==============================================================================
// LOGGING SETTINGS
//==============================================================================

/**
 * @brief Enable logging system
 * @note Set to true to enable comprehensive logging
 * @default true
 */
#define LOGGING_ENABLED true

/**
 * @brief Default log level
 * @note 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
 * @default LOG_INFO (1)
 */
#define DEFAULT_LOG_LEVEL 1

/**
 * @brief Enable Serial console logging
 * @note Output logs to Serial monitor
 * @default true
 */
#define LOG_TO_SERIAL true

/**
 * @brief Enable SD card logging
 * @note Write logs to SD card file
 * @default false (disabled by default to save SD card space)
 */
#define LOG_TO_SD false

/**
 * @brief Log file path on SD card
 * @note Path to the log file
 * @default "/system.log"
 */
#define LOG_FILE_PATH "/system.log"

//==============================================================================
// STORAGE SETTINGS
//==============================================================================

/**
 * @brief Maximum images to capture per day
 * @note Prevents SD card from filling up too quickly
 * @default 1000 images
 */
#define MAX_IMAGES_PER_DAY 1000

/**
 * @brief Maximum image size in bytes
 * @note Maximum allowed size for a single image file
 * @default 100000 bytes (~100KB)
 */
#define MAX_IMAGE_SIZE 100000

/**
 * @brief Automatic deletion after days
 * @note Images older than this will be automatically deleted
 * @default 7 days
 */
#define AUTO_DELETE_DAYS 7

//==============================================================================
// BACKWARD COMPATIBILITY ALIASES
//==============================================================================

/**
 * @brief Backward compatibility: Battery ADC pin
 * @deprecated Use BATTERY_MONITOR_PIN instead
 */
#define BATTERY_ADC_PIN BATTERY_MONITOR_PIN

/**
 * @brief Backward compatibility: JPEG quality
 * @deprecated Use DEFAULT_JPEG_QUALITY instead
 */
#define JPEG_QUALITY DEFAULT_JPEG_QUALITY

/**
 * @brief Backward compatibility: Frame size
 * @deprecated Use DEFAULT_FRAME_SIZE instead
 */
#define FRAME_SIZE DEFAULT_FRAME_SIZE

/**
 * @brief Backward compatibility: Deep sleep duration
 * @deprecated Use DEEP_SLEEP_DURATION_SEC instead
 */
#define DEEP_SLEEP_DURATION DEEP_SLEEP_DURATION_SEC

/**
 * @brief Backward compatibility: Low battery threshold
 * @deprecated Use BATTERY_LOW_THRESHOLD instead
 */
#define LOW_BATTERY_THRESHOLD BATTERY_LOW_THRESHOLD

//==============================================================================
// LORA MESH NETWORKING SETTINGS (Meshtastic/MeshCore Compatible)
//==============================================================================

/**
 * @brief Enable LoRa mesh networking
 * @note Set to true to enable long-range mesh communication
 * @default false (disabled by default due to GPIO conflicts on AI-Thinker)
 */
#define LORA_ENABLED false

/**
 * @brief LoRa radio frequency in Hz
 * @note 915MHz for US, 868MHz for EU, 433MHz for Asia
 * @default 915000000 (915 MHz US ISM band)
 */
#define LORA_FREQUENCY 915000000

/**
 * @brief LoRa bandwidth in Hz
 * @note Options: 125000, 250000, 500000
 * @default 125000 (125 kHz for best range)
 */
#define LORA_BANDWIDTH 125000

/**
 * @brief LoRa spreading factor
 * @note Range: 7-12, higher = more range but slower
 * @default 10 (good balance of range and speed)
 */
#define LORA_SPREADING_FACTOR 10

/**
 * @brief LoRa coding rate
 * @note Range: 5-8 (4/5 to 4/8)
 * @default 5 (4/5 coding rate)
 */
#define LORA_CODING_RATE 5

/**
 * @brief LoRa transmit power in dBm
 * @note Range: 2-20 dBm
 * @default 17 dBm
 */
#define LORA_TX_POWER 17

/**
 * @brief LoRa sync word for network isolation
 * @note Use 0x12 for private network, 0x34 for public LoRaWAN
 * @default 0x12 (private network)
 */
#define LORA_SYNC_WORD 0x12

/**
 * @brief Unique mesh node ID (0 = auto-generate from MAC)
 * @note Each node must have unique ID
 * @default 0 (auto-generate)
 */
#define MESH_NODE_ID 0

/**
 * @brief Maximum mesh network hops
 * @note Limits how far messages propagate
 * @default 5
 */
#define MESH_MAX_HOPS 5

/**
 * @brief Mesh beacon interval in milliseconds
 * @note How often to send discovery beacons
 * @default 30000 (30 seconds)
 */
#define MESH_BEACON_INTERVAL_MS 30000

/**
 * @brief Mesh node timeout in milliseconds
 * @note Time before considering a node offline
 * @default 300000 (5 minutes)
 */
#define MESH_NODE_TIMEOUT_MS 300000

/**
 * @brief Maximum nodes in mesh network
 * @note Memory allocation limit
 * @default 16
 */
#define MESH_MAX_NODES 16

/**
 * @brief Enable mesh encryption (AES-256)
 * @note Provides secure communication
 * @default true
 */
#define MESH_ENCRYPTION_ENABLED true

//==============================================================================
// TIME MANAGEMENT SETTINGS (NTP and RTC)
//==============================================================================

/**
 * @brief Enable time management features
 * @note When enabled, system will attempt NTP sync and RTC initialization
 * @default true
 */
#define TIME_MANAGEMENT_ENABLED true

/**
 * @brief Primary NTP server address
 * @note Used for time synchronization when WiFi is connected
 * @default "pool.ntp.org"
 */
#define NTP_SERVER_PRIMARY "pool.ntp.org"

/**
 * @brief Secondary NTP server address (backup)
 * @note Used if primary server is unreachable
 * @default "time.nist.gov"
 */
#define NTP_SERVER_SECONDARY "time.nist.gov"

/**
 * @brief NTP synchronization timeout in milliseconds
 * @note Maximum time to wait for NTP response
 * @default 10000 (10 seconds)
 */
#define NTP_SYNC_TIMEOUT_MS 10000

/**
 * @brief GMT offset in seconds
 * @note Example: -18000 for EST (UTC-5), 0 for UTC, 3600 for CET (UTC+1)
 * @default 0 (UTC)
 */
#define GMT_OFFSET_SEC 0

/**
 * @brief Daylight saving time offset in seconds
 * @note Set to 3600 if DST is observed, 0 otherwise
 * @default 0
 */
#define DAYLIGHT_OFFSET_SEC 0

/**
 * @brief Enable automatic NTP sync after WiFi connection
 * @note When enabled, time will be synced automatically when WiFi connects
 * @default true
 */
#define NTP_AUTO_SYNC true

/**
 * @brief NTP re-synchronization interval in milliseconds
 * @note How often to re-sync with NTP server (0 = only sync once at startup)
 * @default 3600000 (1 hour)
 */
#define NTP_RESYNC_INTERVAL_MS 3600000

/**
 * @brief Enable external DS3231 RTC module support
 * @note When enabled, system will attempt to use external RTC for persistent time
 * @default true
 */
#define RTC_DS3231_ENABLED true

/**
 * @brief I2C SDA pin for RTC communication
 * @note Default I2C pins may conflict with camera on AI-Thinker boards
 * @note Use GPIO 14 for AI-Thinker ESP32-CAM if default pins conflict
 * @default 21 (ESP32 default SDA)
 */
#define RTC_I2C_SDA_PIN 21

/**
 * @brief I2C SCL pin for RTC communication
 * @note Default I2C pins may conflict with camera on AI-Thinker boards
 * @note Use GPIO 15 for AI-Thinker ESP32-CAM if default pins conflict
 * @default 22 (ESP32 default SCL)
 */
#define RTC_I2C_SCL_PIN 22

//==============================================================================
// LORA PIN DEFINITIONS (ESP32-S3-CAM recommended, conflicts with AI-Thinker)
//==============================================================================

/**
 * @brief LoRa chip select (CS/NSS) pin
 * @note Use GPIO 10 for ESP32-S3, or external SPI for AI-Thinker
 */
#define LORA_CS_PIN 10

/**
 * @brief LoRa reset pin
 * @note Use -1 if not connected
 */
#define LORA_RST_PIN 9

/**
 * @brief LoRa DIO0/IRQ pin
 * @note Interrupt pin for receive complete
 */
#define LORA_DIO0_PIN 14

/**
 * @brief LoRa SPI clock pin (SCK)
 */
#define LORA_SCK_PIN 12

/**
 * @brief LoRa SPI data out pin (MOSI)
 */
#define LORA_MOSI_PIN 11

/**
 * @brief LoRa SPI data in pin (MISO)
 */
#define LORA_MISO_PIN 13

//==============================================================================
// CAMERA PIN DEFINITIONS (AI-Thinker ESP32-CAM)
//==============================================================================

/**
 * @brief Camera power down GPIO pin
 */
#define PWDN_GPIO_NUM     32

/**
 * @brief Camera reset GPIO pin (-1 = not used)
 */
#define RESET_GPIO_NUM    -1

/**
 * @brief Camera clock GPIO pin
 */
#define XCLK_GPIO_NUM      0

/**
 * @brief Camera SCCB data GPIO pin
 */
#define SIOD_GPIO_NUM     26

/**
 * @brief Camera SCCB clock GPIO pin
 */
#define SIOC_GPIO_NUM     27

/**
 * @brief Camera data bit 9 GPIO pin
 */
#define Y9_GPIO_NUM       35

/**
 * @brief Camera data bit 8 GPIO pin
 */
#define Y8_GPIO_NUM       34

/**
 * @brief Camera data bit 7 GPIO pin
 */
#define Y7_GPIO_NUM       39

/**
 * @brief Camera data bit 6 GPIO pin
 */
#define Y6_GPIO_NUM       36

/**
 * @brief Camera data bit 5 GPIO pin
 */
#define Y5_GPIO_NUM       21

/**
 * @brief Camera data bit 4 GPIO pin
 */
#define Y4_GPIO_NUM       19

/**
 * @brief Camera data bit 3 GPIO pin
 */
#define Y3_GPIO_NUM       18

/**
 * @brief Camera data bit 2 GPIO pin
 */
#define Y2_GPIO_NUM        5

/**
 * @brief Camera vertical sync GPIO pin
 */
#define VSYNC_GPIO_NUM    25

/**
 * @brief Camera horizontal reference GPIO pin
 */
#define HREF_GPIO_NUM     23

/**
 * @brief Camera pixel clock GPIO pin
 */
#define PCLK_GPIO_NUM     22

//==============================================================================
// ADDITIONAL SENSOR SETTINGS
//==============================================================================

/**
 * @brief Enable BME280 temperature/humidity/pressure sensor
 * @note Set to true to enable BME280 support
 * @default false (disabled by default to avoid I2C conflicts)
 */
// #define SENSOR_BME280_ENABLED

/**
 * @brief BME280 I2C address
 * @note Common addresses: 0x76 (SDO to GND) or 0x77 (SDO to VCC)
 * @default 0x76
 */
#define SENSOR_BME280_ADDRESS 0x76

/**
 * @brief Enable GPS location tagging (optional)
 * @note Set to true to enable GPS support
 * @default false (disabled by default)
 */
// #define SENSOR_GPS_ENABLED

/**
 * @brief GPS module RX pin (GPS TX -> ESP32 RX)
 * @note Use GPIO 16 for UART2 RX
 * @default 16
 */
#define SENSOR_GPS_RX_PIN 16

/**
 * @brief GPS module TX pin (GPS RX -> ESP32 TX)
 * @note Use GPIO 17 for UART2 TX (optional, some GPS modules don't need TX)
 * @default 17
 */
#define SENSOR_GPS_TX_PIN 17

/**
 * @brief GPS module baud rate
 * @note Most GPS modules use 9600 baud
 * @default 9600
 */
#define SENSOR_GPS_BAUD 9600

/**
 * @brief Enable light sensor for day/night detection
 * @note Set to true to enable light sensor support
 * @default false (disabled by default)
 */
// #define SENSOR_LIGHT_ENABLED

/**
 * @brief Light sensor analog pin (for LDR/photoresistor)
 * @note Use -1 for I2C BH1750 sensor
 * @note GPIO 32 is available on AI-Thinker ESP32-CAM (but conflicts with camera)
 * @default -1 (use I2C BH1750)
 */
// #define SENSOR_LIGHT_ANALOG_PIN 32

/**
 * @brief BH1750 I2C address
 * @note Common addresses: 0x23 (ADDR to GND) or 0x5C (ADDR to VCC)
 * @default 0x23
 */
#define SENSOR_BH1750_ADDRESS 0x23

/**
 * @brief I2C SDA pin for sensors
 * @note On AI-Thinker ESP32-CAM, GPIO 14 can be used (check for conflicts)
 * @default 14
 */
#define SENSOR_I2C_SDA_PIN 14

/**
 * @brief I2C SCL pin for sensors
 * @note On AI-Thinker ESP32-CAM, GPIO 15 can be used (check for conflicts)
 * @default 15
 */
#define SENSOR_I2C_SCL_PIN 15

/**
 * @brief Daytime light threshold in lux
 * @note Light level above this is considered daytime
 * @default 100 lux
 */
#define SENSOR_DAYTIME_THRESHOLD 100.0

/**
 * @brief Sensor read interval in milliseconds
 * @note How often to read environmental sensors
 * @default 60000ms (60 seconds)
 */
#define SENSOR_READ_INTERVAL_MS 60000

//==============================================================================
// CLOUD INTEGRATION SETTINGS
//==============================================================================

/**
 * @brief Enable cloud upload functionality
 * @note Set to true to enable automatic image upload to backend server
 * @default true
 */
#define CLOUD_UPLOAD_ENABLED true

/**
 * @brief Backend server URL for cloud uploads
 * @note Include protocol and port (e.g., "http://192.168.1.100:5000")
 * @note For production, use HTTPS with proper certificates
 * @note Define in secrets.h to override this default
 * @default "" (disabled until configured)
 */
#ifndef CLOUD_SERVER_URL
#define CLOUD_SERVER_URL ""
#endif

/**
 * @brief API key for cloud server authentication
 * @note Leave empty if server doesn't require authentication
 * @note For security, create include/secrets.h with your API key
 */
#ifndef CLOUD_API_KEY
#define CLOUD_API_KEY ""
#endif

/**
 * @brief Device name for cloud registration
 * @note Human-readable name shown in the dashboard
 * @default "WildCAM-ESP32"
 */
#define CLOUD_DEVICE_NAME "WildCAM-ESP32"

/**
 * @brief Maximum file size for cloud upload in bytes
 * @note Files larger than this will be rejected
 * @default 5242880 (5MB)
 */
#define CLOUD_MAX_FILE_SIZE 5242880

/**
 * @brief Upload on WiFi connection only
 * @note When true, uploads are queued until WiFi is available
 * @default true
 */
#define CLOUD_UPLOAD_ON_WIFI_ONLY true

/**
 * @brief Maximum upload queue size
 * @note Maximum number of images to queue for upload when offline
 * @default 50
 */
#define CLOUD_MAX_QUEUE_SIZE 50

/**
 * @brief Automatic status reporting interval in milliseconds
 * @note How often to report device status to cloud (0 = disabled)
 * @default 300000ms (5 minutes)
 */
#define CLOUD_STATUS_REPORT_INTERVAL_MS 300000

/**
 * @brief Upload timeout in milliseconds
 * @note Maximum time to wait for upload response
 * @default 30000ms (30 seconds)
 */
#define CLOUD_UPLOAD_TIMEOUT_MS 30000

/**
 * @brief Enable upload retry on failure
 * @note When true, failed uploads will be retried
 * @default true
 */
#define CLOUD_RETRY_ENABLED true

/**
 * @brief Maximum retry attempts for failed uploads
 * @default 3
 */
#define CLOUD_MAX_RETRIES 3

#endif // CONFIG_H
