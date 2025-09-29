/**
 * @file pin_manager.h
 * @brief Centralized pin management system for ESP32WildlifeCAM
 * @author thewriterben
 * @date 2025-09-02
 * @version 2.5.0
 * 
 * This file provides centralized GPIO pin management with conflict detection
 * and hardware-specific pin allocation for AI-Thinker ESP32-CAM.
 */

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#include <Arduino.h>

// ===========================
// VALIDATED PIN ALLOCATION FOR AI-THINKER ESP32-CAM
// Based on AUDIT_REPORT.md findings
// ===========================

// ===========================
// CAMERA PINS (Fixed by hardware - highest priority)
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
// VALIDATED GPIO ALLOCATION (No conflicts)
// ===========================
#define PIR_PIN           1   // PIR motion sensor pin (CORRECTED from GPIO 13)
#define CHARGING_LED_PIN  2   // Charging status LED (moved from GPIO 16)
#define LORA_MISO        12   // Available for LoRa when enabled
#define AVAILABLE_PIN_13 13   // Available for future use  
#define LORA_SCK         14   // Available for LoRa when enabled
#define LORA_MOSI        15   // Available for LoRa when enabled
#define LORA_CS          16   // Available for LoRa when enabled
#define LORA_RST         17   // Available for LoRa when enabled
#define LORA_DIO0        33   // LoRa interrupt (shared with voltage monitoring)

// ===========================
// SHARED PINS (Documented sharing strategy)
// ===========================
// GPIO 32: Camera PWDN / Solar voltage monitoring (ADC input, compatible)
#define SOLAR_VOLTAGE_PIN      32  // Shared with camera PWDN
// GPIO 34: Camera Y8 / Battery voltage monitoring (input-only, compatible)  
#define BATTERY_VOLTAGE_PIN    34  // Shared with camera Y8
// GPIO 35: Camera Y9 / Secondary voltage monitoring (input-only, compatible)
#define SECONDARY_VOLTAGE_PIN  35  // Shared with camera Y9

// ===========================
// SD CARD PINS (MMC mode - fixed assignment)
// ===========================
#define SD_MMC_CMD    15  // SD card command pin
#define SD_MMC_CLK    14  // SD card clock pin  
#define SD_MMC_D0      2  // SD card data 0
#define SD_MMC_D1      4  // SD card data 1 (also built-in LED)
#define SD_MMC_D2     12  // SD card data 2
#define SD_MMC_D3     13  // SD card data 3

// ===========================
// FEATURE ENABLE/DISABLE FLAGS
// ===========================
#define LORA_ENABLED false        // Disabled due to camera pin conflicts
#define SD_CARD_ENABLED true      // Enabled when LoRa disabled
#define SERVO_ENABLED false       // Disabled due to insufficient pins
#define IR_LED_ENABLED false      // Disabled due to pin conflicts
#define VIBRATION_ENABLED false   // Disabled due to pin conflicts

// ===========================
// PIN CONFLICT DETECTION
// ===========================
typedef struct {
    uint8_t pin;
    const char* function1;
    const char* function2;
    bool is_conflict;
} pin_conflict_t;

// Function prototypes
bool validatePinAllocation(void);
void printPinAllocation(void);
bool isPinAvailable(uint8_t pin);
bool assignPin(uint8_t pin, const char* function);
void detectPinConflicts(void);

// ===========================
// HARDWARE CONSTRAINTS DOCUMENTATION
// ===========================
/*
ESP32-CAM Hardware Constraints (AI-Thinker):
- GPIO 34, 35, 36, 39: Input-only pins (used for camera and voltage monitoring)
- GPIO 6-11: Connected to SPI flash (do not use)
- GPIO 12: Has internal pull-down (affects boot if HIGH) 
- GPIO 2, 15: Have internal pull-ups (affect boot mode)
- GPIO 0: Used for boot mode selection and camera XCLK

Available GPIO pins for peripherals: 1, 2, 12, 13, 14, 15, 16, 17, 33
Camera uses: 0, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 34, 35, 36, 39

RESOLUTION: Priority given to camera functionality. LoRa requires 
ESP32-S3-CAM or ESP-EYE with more available GPIO pins.
*/

// ===========================
// CONDITIONAL PIN DEFINITIONS
// ===========================
#if SD_CARD_ENABLED
  // SD card pins take priority over other functions
  #define SD_CS_PIN       SD_MMC_D3    // GPIO 13
  #define SD_CLK_PIN      SD_MMC_CLK   // GPIO 14  
  #define SD_MOSI_PIN     SD_MMC_CMD   // GPIO 15
  #define SD_MISO_PIN     SD_MMC_D0    // GPIO 2
#endif

#if LORA_ENABLED && !SD_CARD_ENABLED
  // LoRa pins only available when SD card is disabled
  #warning "LoRa enabled but conflicts with camera pins on AI-Thinker ESP32-CAM"
#endif

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

#endif // PIN_MANAGER_H