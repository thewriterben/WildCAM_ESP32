/**
 * @file st7789_display.h
 * @brief ST7789 TFT display driver for T-Camera Plus
 */

#ifndef ST7789_DISPLAY_H
#define ST7789_DISPLAY_H

#include "display_interface.h"
#include <TFT_eSPI.h>

class ST7789Display : public DisplayInterface {
public:
    ST7789Display(const DisplayConfig& config);
    virtual ~ST7789Display();
    
    // Display identification
    DisplayType getDisplayType() const override { return DISPLAY_ST7789_TFT; }
    const char* getDisplayName() const override { return "ST7789 TFT"; }
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
    uint16_t getWidth() const override { return tft ? tft->width() : 240; }
    uint16_t getHeight() const override { return tft ? tft->height() : 240; }

private:
    TFT_eSPI* tft;
    uint8_t current_brightness;
    void setupPins();
    void configureTFT();
};

#endif // ST7789_DISPLAY_H