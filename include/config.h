#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"

// Camera Configuration
#define CAMERA_MODEL_AI_THINKER
#define FRAME_SIZE FRAMESIZE_UXGA  // UXGA(1600x1200)
#define JPEG_QUALITY 10  // 10-63, lower means higher quality

// Motion Detection Configuration
#define PIR_SENSOR_PIN 13
#define MOTION_SENSITIVITY 50  // 0-100
#define MOTION_COOLDOWN_MS 5000  // Minimum time between triggers

// Storage Configuration
#define SD_CS_PIN 5
#define MAX_IMAGE_SIZE 100000  // Maximum image size in bytes
#define MAX_IMAGES_PER_DAY 1000

// Power Management Configuration
#define BATTERY_ADC_PIN 33  // Changed from 34 to avoid conflict with Y8_GPIO_NUM
#define BATTERY_VOLTAGE_MIN 3.3
#define BATTERY_VOLTAGE_MAX 4.2
#define DEEP_SLEEP_DURATION 60  // Seconds
#define LOW_BATTERY_THRESHOLD 3.5  // Volts

// Web Server Configuration
#define WEB_SERVER_PORT 80
#define WEB_SOCKET_PORT 81

// Camera Pin Definitions for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Flash LED
#define FLASH_LED_PIN 4

#endif // CONFIG_H
