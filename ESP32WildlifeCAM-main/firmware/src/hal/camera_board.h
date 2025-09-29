/**
 * @file camera_board.h
 * @brief Base class for ESP32 camera board abstraction
 */

#ifndef CAMERA_BOARD_H
#define CAMERA_BOARD_H

#include <Arduino.h>
#include <esp_camera.h>

// Board types enumeration
enum BoardType {
    BOARD_UNKNOWN = 0,
    BOARD_AI_THINKER_ESP32_CAM,
    BOARD_ESP32_S3_CAM,
    BOARD_ESP_EYE,
    BOARD_M5STACK_TIMER_CAM,
    BOARD_TTGO_T_CAMERA,
    BOARD_XIAO_ESP32S3_SENSE,
    BOARD_FIREBEETLE_ESP32_CAM,
    BOARD_ESP32_S3_EYE,
    BOARD_FREENOVE_ESP32_WROVER_CAM,
    BOARD_M5STACK_ESP32CAM,
    BOARD_TTGO_T_JOURNAL,
    BOARD_LILYGO_T_CAMERA_PLUS,
    BOARD_LILYGO_T_CAMERA_PLUS_S3,
    BOARD_ESPRESSIF_ESP32_CAM_MB,
    BOARD_M5STACK_UNITCAM,
    BOARD_TTGO_T_CAMERA_V17
};

// Camera sensor types
enum SensorType {
    SENSOR_UNKNOWN = 0,
    SENSOR_OV2640,
    SENSOR_OV3660,
    SENSOR_OV5640,
    SENSOR_GC032A,
    SENSOR_HM01B0,
    SENSOR_SC031GS
};

// GPIO pin mapping structure
struct GPIOMap {
    int pwdn_pin;
    int reset_pin;
    int xclk_pin;
    int siod_pin;
    int sioc_pin;
    int y9_pin;
    int y8_pin;
    int y7_pin;
    int y6_pin;
    int y5_pin;
    int y4_pin;
    int y3_pin;
    int y2_pin;
    int vsync_pin;
    int href_pin;
    int pclk_pin;
    int led_pin;
    int flash_pin;
};

// Power management profile
struct PowerProfile {
    uint32_t sleep_current_ua;      // Sleep current in microamps
    uint32_t active_current_ma;     // Active current in milliamps
    uint32_t camera_current_ma;     // Camera active current in milliamps
    bool has_external_power;        // External power management
    bool supports_deep_sleep;       // Deep sleep capability
    float min_voltage;              // Minimum operating voltage
    float max_voltage;              // Maximum operating voltage
};

// Camera configuration for specific board
struct CameraConfig {
    framesize_t max_framesize;      // Maximum supported frame size
    pixformat_t pixel_format;       // Pixel format
    uint8_t jpeg_quality;           // Default JPEG quality
    uint8_t fb_count;               // Frame buffer count
    uint32_t xclk_freq_hz;          // Clock frequency
    bool psram_required;            // PSRAM requirement
    bool has_flash;                 // Flash capability
};

// Display configuration for board
struct DisplayProfile {
    bool has_display;              // Board has built-in display
    uint8_t display_type;          // Display type (SSD1306, ST7789, etc.)
    int display_sda_pin;           // I2C SDA pin for OLED
    int display_scl_pin;           // I2C SCL pin for OLED
    int display_cs_pin;            // SPI CS pin for TFT
    int display_dc_pin;            // SPI DC pin for TFT
    int display_rst_pin;           // Reset pin
    int display_bl_pin;            // Backlight pin
    uint8_t display_i2c_addr;      // I2C address for OLED
    uint32_t display_i2c_freq;     // I2C frequency
    uint32_t display_spi_freq;     // SPI frequency
    uint16_t display_width;        // Display width in pixels
    uint16_t display_height;       // Display height in pixels
};

/**
 * Abstract base class for camera board implementations
 */
class CameraBoard {
public:
    virtual ~CameraBoard() = default;
    
    // Board identification
    virtual BoardType getBoardType() const = 0;
    virtual const char* getBoardName() const = 0;
    virtual bool isSupported() const = 0;
    
    // Hardware initialization
    virtual bool init() = 0;
    virtual bool detectSensor() = 0;
    virtual SensorType getSensorType() const = 0;
    
    // Configuration
    virtual GPIOMap getGPIOMap() const = 0;
    virtual CameraConfig getCameraConfig() const = 0;
    virtual PowerProfile getPowerProfile() const = 0;
    virtual DisplayProfile getDisplayProfile() const = 0;
    
    // Camera operations
    virtual bool configureSensor(sensor_t* sensor) = 0;
    virtual bool setupPins() = 0;
    virtual void flashLED(bool state) = 0;
    
    // Power management
    virtual void enterSleepMode() = 0;
    virtual void exitSleepMode() = 0;
    virtual float getBatteryVoltage() = 0;
    
    // Chip information
    virtual bool hasPSRAM() const = 0;
    virtual uint32_t getChipId() const = 0;
    virtual const char* getChipModel() const = 0;

protected:
    BoardType board_type = BOARD_UNKNOWN;
    SensorType sensor_type = SENSOR_UNKNOWN;
    bool initialized = false;
};

#endif // CAMERA_BOARD_H