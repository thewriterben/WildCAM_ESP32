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

/**
 * @brief WiFi SSID (network name)
 * @note Replace with your WiFi network name
 */
#define WIFI_SSID "YourSSID"

/**
 * @brief WiFi password
 * @note Replace with your WiFi network password
 */
#define WIFI_PASSWORD "YourPassword"

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

#endif // CONFIG_H
