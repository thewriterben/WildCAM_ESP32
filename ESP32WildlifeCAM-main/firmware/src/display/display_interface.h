/**
 * @file display_interface.h
 * @brief Abstract display interface for HMI system
 */

#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include <Arduino.h>

// Display types enumeration
enum DisplayType {
    DISPLAY_NONE = 0,
    DISPLAY_SSD1306_OLED,
    DISPLAY_ST7789_TFT,
    DISPLAY_EXTERNAL_I2C,
    DISPLAY_EXTERNAL_SPI
};

// Display capabilities structure
struct DisplayCapabilities {
    uint16_t width;
    uint16_t height;
    bool supports_color;
    bool supports_touch;
    bool supports_graphics;
    uint8_t brightness_levels;
};

// Display configuration structure
struct DisplayConfig {
    DisplayType type;
    int sda_pin;        // For I2C displays
    int scl_pin;        // For I2C displays
    int cs_pin;         // For SPI displays
    int dc_pin;         // For SPI displays
    int rst_pin;        // Reset pin
    int bl_pin;         // Backlight pin
    uint32_t i2c_freq;  // I2C frequency
    uint32_t spi_freq;  // SPI frequency
    uint8_t i2c_addr;   // I2C address for OLED
};

/**
 * Abstract base class for display drivers
 */
class DisplayInterface {
public:
    virtual ~DisplayInterface() = default;
    
    // Display identification
    virtual DisplayType getDisplayType() const = 0;
    virtual const char* getDisplayName() const = 0;
    virtual DisplayCapabilities getCapabilities() const = 0;
    
    // Hardware initialization
    virtual bool init() = 0;
    virtual bool isInitialized() const = 0;
    virtual void reset() = 0;
    
    // Display control
    virtual void clear() = 0;
    virtual void display() = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void setPowerMode(bool on) = 0;
    
    // Text display
    virtual void setTextSize(uint8_t size) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual void println(const char* text) = 0;
    
    // Graphics (basic)
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    
    // Status and information
    virtual bool hasDisplay() const { return initialized; }
    virtual uint16_t getWidth() const = 0;
    virtual uint16_t getHeight() const = 0;

protected:
    bool initialized = false;
    DisplayConfig config;
};

#endif // DISPLAY_INTERFACE_H