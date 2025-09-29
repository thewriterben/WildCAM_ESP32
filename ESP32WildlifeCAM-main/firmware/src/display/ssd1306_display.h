/**
 * @file ssd1306_display.h
 * @brief SSD1306 OLED display driver for T-Camera
 */

#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include "display_interface.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

class SSD1306Display : public DisplayInterface {
public:
    SSD1306Display(const DisplayConfig& config);
    virtual ~SSD1306Display();
    
    // Display identification
    DisplayType getDisplayType() const override { return DISPLAY_SSD1306_OLED; }
    const char* getDisplayName() const override { return "SSD1306 OLED"; }
    DisplayCapabilities getCapabilities() const override;
    
    // Hardware initialization
    bool init() override;
    bool isInitialized() const override { return initialized; }
    void reset() override;
    
    // Display control
    void clear() override;
    void display() override;
    void setBrightness(uint8_t brightness) override;
    void setPowerMode(bool on) override;
    
    // Text display
    void setTextSize(uint8_t size) override;
    void setTextColor(uint16_t color) override;
    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void println(const char* text) override;
    
    // Graphics
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    // Dimensions
    uint16_t getWidth() const override { return oled ? oled->width() : 128; }
    uint16_t getHeight() const override { return oled ? oled->height() : 64; }

private:
    Adafruit_SSD1306* oled;
    bool initializeI2C();
    void setupPins();
};

#endif // SSD1306_DISPLAY_H