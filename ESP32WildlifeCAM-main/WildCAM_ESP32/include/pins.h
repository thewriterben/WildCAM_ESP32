#ifndef PINS_H
#define PINS_H

// ===========================
// ESP32-CAM PIN DEFINITIONS
// AI-Thinker ESP32-CAM Module
// ===========================

// ===========================
// CAMERA PINS (OV2640)
// ===========================
#define PWDN_GPIO_NUM     32  // Power down pin
#define RESET_GPIO_NUM    -1  // Reset pin (not connected)
#define XCLK_GPIO_NUM      0  // External clock
#define SIOD_GPIO_NUM     26  // SDA pin
#define SIOC_GPIO_NUM     27  // SCL pin

#define Y9_GPIO_NUM       35  // Camera data pins
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25  // Vertical sync
#define HREF_GPIO_NUM     23  // Horizontal reference
#define PCLK_GPIO_NUM     22  // Pixel clock

// ===========================
// MOTION DETECTION PINS
// ===========================
// PIR_PIN corrected from GPIO 13 to GPIO 1 per AUDIT_REPORT.md
#define PIR_PIN           1   // PIR motion sensor pin (CORRECTED from GPIO 13)
// PIR_POWER_PIN moved to avoid conflict with SD_CS_PIN
#define PIR_POWER_PIN     17  // PIR sensor power control (moved from GPIO 12 to avoid SD conflict)

// ===========================
// POWER MANAGEMENT PINS - UPDATED TO AVOID CONFLICTS
// ===========================
#define BATTERY_VOLTAGE_PIN    34  // Battery voltage monitoring (shared with camera Y8, input-only)
#define SOLAR_VOLTAGE_PIN      32  // Solar panel voltage monitoring (shared with camera PWDN)
#define CHARGING_CONTROL_PIN   14  // Charging control pin (shared with SD_CLK - use conditionally)
#define POWER_LED_PIN          4   // Power status LED (built-in LED, shared with SD_D1)
#define CHARGING_LED_PIN       2   // Charging status LED (moved from GPIO 15 to avoid SD conflict)

// ===========================
// STORAGE PINS
// ===========================
// SD Card pins are fixed on ESP32-CAM:
// CMD  = GPIO 15
// CLK  = GPIO 14
// D0   = GPIO 2
// D1   = GPIO 4
// D2   = GPIO 12
// D3   = GPIO 13
// Note: GPIO 4 has built-in LED, GPIO 33 can be used for flash

#define SD_MMC_CMD    15  // SD card command pin
#define SD_MMC_CLK    14  // SD card clock pin
#define SD_MMC_D0      2  // SD card data 0
#define SD_MMC_D1      4  // SD card data 1 (also built-in LED)
#define SD_MMC_D2     12  // SD card data 2
#define SD_MMC_D3     13  // SD card data 3

// ===========================
// SENSOR PINS
// ===========================
#define BME280_SDA_PIN    26  // Environmental sensor SDA (shared with camera)
#define BME280_SCL_PIN    27  // Environmental sensor SCL (shared with camera)
#define BME280_ADDRESS    0x76 // BME280 I2C address

// ===========================
// NIGHT VISION PINS - DISABLED DUE TO PIN CONFLICTS
// ===========================
// #define IR_LED_PIN        16  // DISABLED - conflicts with LoRa CS and servo pins
#define LIGHT_SENSOR_PIN  33  // Light sensor ADC pin (shared with battery monitoring)

// ===========================
// COMMUNICATION PINS (Optional)
// ===========================
#define UART_TX_PIN       1   // UART TX (USB serial)
#define UART_RX_PIN       3   // UART RX (USB serial)

// Optional LoRa pins (for future expansion)
#define LORA_SCK_PIN      18  // LoRa SPI clock (shared with camera)
#define LORA_MISO_PIN     19  // LoRa SPI MISO (shared with camera)
#define LORA_MOSI_PIN     23  // LoRa SPI MOSI (shared with camera)
#define LORA_CS_PIN       16  // LoRa chip select
#define LORA_RST_PIN      17  // LoRa reset
#define LORA_DIO0_PIN     26  // LoRa DIO0 interrupt

// ===========================
// SERVO CONTROL PINS - DISABLED DUE TO PIN CONFLICTS
// ===========================
// Servo functionality disabled due to insufficient GPIO pins on AI-Thinker ESP32-CAM
// #define PAN_SERVO_PIN     16  // DISABLED - conflicts with LoRa CS and IR LED
// #define TILT_SERVO_PIN    17  // DISABLED - conflicts with LoRa RST and PIR power

// ===========================
// SPARE/MULTIPURPOSE PINS - AVAILABLE FOR LORA WHEN ENABLED
// ===========================
#define SPARE_GPIO_1      16  // Available for LoRa CS when LoRa enabled
#define SPARE_GPIO_2      17  // Available for LoRa RST when LoRa enabled

// ===========================
// PIN USAGE NOTES AND SHARING STRATEGY
// ===========================
/*
ESP32-CAM Pin Usage Summary and Conflict Resolution:

CAMERA PINS (Fixed by hardware, highest priority):
- GPIO 0: Camera XCLK (can be used for boot mode selection)
- GPIO 5: Camera Y2 (data bit 2)
- GPIO 18: Camera Y3 (data bit 3) 
- GPIO 19: Camera Y4 (data bit 4)
- GPIO 21: Camera Y5 (data bit 5)
- GPIO 22: Camera PCLK (pixel clock)
- GPIO 23: Camera HREF (horizontal reference)
- GPIO 25: Camera VSYNC (vertical sync)
- GPIO 26: Camera SIOD (I2C SDA)
- GPIO 27: Camera SIOC (I2C SCL)
- GPIO 32: Camera PWDN (power down)
- GPIO 34: Camera Y8 (data bit 8, input-only)
- GPIO 35: Camera Y9 (data bit 9, input-only)
- GPIO 36: Camera Y6 (data bit 6, input-only)
- GPIO 39: Camera Y7 (data bit 7, input-only)

SHARED PINS STRATEGY:
- GPIO 1: PIR_PIN (primary) / UART_TX (debug only, OK to share)
- GPIO 32: PWDN_GPIO_NUM (camera) / SOLAR_VOLTAGE_PIN (ADC input, compatible)
- GPIO 34: Y8_GPIO_NUM (camera) / BATTERY_VOLTAGE_PIN (both input-only, compatible)
- GPIO 26: SIOD_GPIO_NUM (camera I2C) / BME280_SDA_PIN (same I2C bus, compatible)
- GPIO 27: SIOC_GPIO_NUM (camera I2C) / BME280_SCL_PIN (same I2C bus, compatible)

CONDITIONAL PINS (used based on configuration):
- GPIO 2: SD_D0 (when SD enabled) / POWER_LED_PIN (when SD disabled)
- GPIO 14: SD_CLK (when SD enabled) / CHARGING_CONTROL_PIN (when SD disabled)
- GPIO 15: SD_CMD (when SD enabled) / CHARGING_LED_PIN (when SD disabled)
- GPIO 16: IR_LED_PIN (when IR enabled) / LORA_CS_PIN (when LoRa enabled, but conflicts with camera)

LORA CONFLICTS (LoRa disabled due to fundamental conflicts):
- GPIO 5: LORA_CS vs Y2_GPIO_NUM (camera data bit 2)
- GPIO 16: LORA_CS vs CHARGING_LED_PIN/IR_LED_PIN  
- GPIO 18: LORA_SCK vs Y3_GPIO_NUM (camera data bit 3)
- GPIO 19: LORA_MISO vs Y4_GPIO_NUM (camera data bit 4) 
- GPIO 23: LORA_MOSI vs HREF_GPIO_NUM (camera horizontal reference)
- GPIO 26: LORA_DIO0 vs SIOD_GPIO_NUM (camera I2C data)

CONSTRAINTS:
- GPIO 34, 35, 36, 39 are input-only (used for camera and voltage monitoring)
- GPIO 6-11 are connected to SPI flash (do not use)
- GPIO 12 has internal pull-down (affects boot if HIGH)
- GPIO 2 and 15 have internal pull-ups (affect boot mode)

RESOLUTION: Camera functionality takes priority. LoRa networking requires
ESP32-S3-CAM or ESP-EYE with more available GPIO pins.
*/

// ===========================
// VOLTAGE DIVIDER RATIOS
// ===========================
#define BATTERY_VOLTAGE_DIVIDER 2.0f  // 2:1 voltage divider for battery
#define SOLAR_VOLTAGE_DIVIDER   3.0f  // 3:1 voltage divider for solar panel

// ===========================
// ADC CONFIGURATION
// ===========================
#define ADC_RESOLUTION 12             // ADC resolution (12-bit = 0-4095)
#define ADC_VREF 3.3f                // ADC reference voltage
#define ADC_SAMPLES 16               // Number of samples for averaging

#endif // PINS_H