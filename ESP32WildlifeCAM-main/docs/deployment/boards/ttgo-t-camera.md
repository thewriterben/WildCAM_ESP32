# TTGO T-Camera Deployment Guide

## Overview

The TTGO T-Camera is an ESP32 camera board with an integrated OLED display and modular design, making it ideal for interactive wildlife monitoring applications, field research requiring visual feedback, and educational projects where real-time display capabilities enhance user engagement.

## Prerequisites

### Hardware Requirements
- **TTGO T-Camera board** - ESP32 camera with OLED display
- **USB cable** (Micro-USB) - For programming and power
- **MicroSD card** (8-32GB, Class 10 recommended)
- **Computer** with Arduino IDE or PlatformIO installed
- **PIR sensor** (optional, for motion detection)
- **External antenna** (optional, for improved WiFi range)

### Software Requirements
- **Arduino IDE** (version 1.8.19 or later) or **PlatformIO**
- **ESP32 Board Package** - Latest version from Espressif
- **TTGO Libraries** - For display and board-specific functions
- **Required Libraries:**
  - ESP32 Camera library (built-in)
  - U8g2 library (for OLED display)
  - WiFi library (built-in)
  - SD library (for storage)
  - ArduinoJson (for configuration)

### Skill Level
- **Beginner to Intermediate** - OLED display programming helpful
- **Programming experience** - Arduino/C++ understanding recommended

## Step-by-Step Instructions

### 1. Setting Up the Arduino IDE

1. **Download and Install Arduino IDE**
   - Download from [arduino.cc/en/software](https://www.arduino.cc/en/software)
   - Install following platform-specific instructions

2. **Add ESP32 Board Package**
   - Open Arduino IDE
   - Go to `File` > `Preferences`
   - Add this URL to "Additional Boards Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` > `Board` > `Boards Manager`
   - Search for "ESP32" and install "esp32 by Espressif Systems"

3. **Select Board and Configuration**
   - Go to `Tools` > `Board` > `ESP32 Arduino`
   - Select "ESP32 Dev Module"
   - Set `Tools` > `Flash Size` to "4MB (32Mb)"
   - Set `Tools` > `Partition Scheme` to "Huge APP (3MB No OTA/1MB SPIFFS)"

### 2. Installing Libraries and Board Packages

1. **Install Required Libraries**
   - Go to `Tools` > `Manage Libraries`
   - Install: U8g2, ArduinoJson, AsyncTCP, ESPAsyncWebServer

2. **Install Display Libraries**
   - U8g2 by oliver (for OLED display support)
   - Adafruit SSD1306 (alternative display library)

### 3. Connecting the Board

1. **USB Connection**
   - Connect TTGO T-Camera using Micro-USB cable
   - Board should automatically enter programming mode
   - Check for device recognition in device manager

2. **Programming Notes**
   - Usually requires manual reset for programming
   - Hold GPIO 0 low while resetting if auto-programming fails

### 4. Loading the Code

1. **Download Project Code**
   ```bash
   git clone https://github.com/thewriterben/ESP32WildlifeCAM.git
   cd ESP32WildlifeCAM/firmware
   ```

2. **Configure Board Settings**
   - Ensure `#define CAMERA_MODEL_TTGO_T_CAMERA` is uncommented
   - Configure display settings for OLED
   - Set up camera and sensor parameters

### 5. Compiling and Uploading

1. **Compile Settings**
   - Board: "ESP32 Dev Module"
   - Flash Size: "4MB (32Mb)"
   - Upload Speed: "921600"

2. **Upload the Code**
   - Click upload button and monitor progress
   - Reset board manually if upload fails

### 6. Testing the Deployment

1. **Test Display Functionality**
   - Verify OLED display shows startup messages
   - Check camera preview on display
   - Test menu navigation if implemented

2. **Test Camera Functions**
   - Capture test images
   - Verify SD card storage
   - Check WiFi connectivity display

## Hardware Specifications

### Board Features
- **Microcontroller**: ESP32 (240MHz dual-core)
- **Camera**: OV2640 2MP sensor
- **Display**: 0.96" OLED (128x64 pixels)
- **Memory**: 520KB SRAM + 4MB PSRAM (some models)
- **Flash**: 4MB SPI Flash
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth
- **Expansion**: GPIO headers for sensors
- **Dimensions**: 54mm x 54mm

### Camera Specifications
- **Sensor**: OV2640 CMOS
- **Resolution**: Up to 1600x1200 (UXGA)
- **Formats**: JPEG, RGB565, YUV422
- **Features**: Auto exposure, auto white balance
- **Frame Rate**: Up to 60fps at lower resolutions

## Pin Configuration

### TTGO T-Camera GPIO Map
```cpp
// TTGO T-Camera Pin Definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26  // SDA
#define SIOC_GPIO_NUM     27  // SCL

// Camera data pins
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

// OLED Display pins
#define OLED_SDA          21
#define OLED_SCL          22
#define OLED_RST          16

// Available GPIO
#define PIR_SENSOR_PIN    13
#define STATUS_LED_PIN     2
```

## Troubleshooting

### Common Issues

#### Display Not Working
1. **OLED initialization failed**
   - Check SDA/SCL connections (pins 21/22)
   - Verify display library configuration
   - Test with simple display example

2. **Display garbled or blank**
   - Check display address (usually 0x3C)
   - Verify power supply to display
   - Test display reset functionality

#### Camera + Display Conflicts
1. **I2C conflicts**
   - Camera and display share I2C bus
   - Ensure proper I2C addressing
   - Check for timing conflicts

### Pin Conflicts
- **I2C sharing**: Camera and display use same I2C pins
- **Limited GPIO**: Display uses several pins

### Power Issues
- **Display power consumption**: OLED adds power usage
- **Battery optimization**: Implement display sleep modes

## Advanced Configuration

### Display Programming
```cpp
#include <U8g2lib.h>

// OLED display setup
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 22, /* data=*/ 21);

void setupDisplay() {
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,10,"Wildlife CAM");
    u8g2.sendBuffer();
}

void updateDisplay(String status) {
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"Wildlife CAM");
    u8g2.drawStr(0,25,status.c_str());
    u8g2.sendBuffer();
}
```

### Interactive Menu System
```cpp
// Simple menu system for T-Camera
enum MenuState {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_CAPTURE,
    MENU_STATUS
};

void handleMenuNavigation() {
    // Implement button navigation
    // Update display based on menu state
    // Handle user input
}
```

## Conclusion

The TTGO T-Camera provides excellent value for interactive wildlife monitoring with its integrated OLED display, making it perfect for applications requiring immediate visual feedback and user interaction capabilities.

### Key Advantages
- **Integrated display**: Real-time status and image preview
- **Compact design**: All-in-one solution
- **Interactive capability**: Menu systems and user feedback
- **Cost-effective**: Good value for display-equipped camera

---

*This guide covers the TTGO T-Camera deployment with emphasis on display integration. Adapt pin configurations based on your specific TTGO T-Camera variant.*