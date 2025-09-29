/**
 * @file st7789_display.cpp
 * @brief ST7789 TFT display driver implementation
 */

#include "st7789_display.h"
#include "../debug_config.h"

ST7789Display::ST7789Display(const DisplayConfig& cfg) : tft(nullptr), current_brightness(255) {
    config = cfg;
}

ST7789Display::~ST7789Display() {
    if (tft) {
        delete tft;
        tft = nullptr;
    }
}

DisplayCapabilities ST7789Display::getCapabilities() const {
    return {
        .width = 240,
        .height = 240,
        .supports_color = true,   // Full color display
        .supports_touch = false,  // No touch by default
        .supports_graphics = true,
        .brightness_levels = 256
    };
}

bool ST7789Display::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing ST7789 TFT display...");
    
    setupPins();
    
    // Create TFT object
    tft = new TFT_eSPI();
    if (!tft) {
        DEBUG_PRINTLN("Failed to create TFT_eSPI object");
        return false;
    }
    
    // Initialize TFT
    tft->init();
    tft->setRotation(0);  // Portrait mode
    tft->fillScreen(TFT_BLACK);
    
    // Set default text properties
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    tft->setCursor(0, 0);
    
    // Configure backlight
    setBrightness(200);  // 80% brightness
    
    initialized = true;
    DEBUG_PRINTLN("ST7789 TFT initialized successfully");
    
    return true;
}

void ST7789Display::reset() {
    if (config.rst_pin >= 0) {
        digitalWrite(config.rst_pin, LOW);
        delay(10);
        digitalWrite(config.rst_pin, HIGH);
        delay(10);
    }
}

void ST7789Display::clear() {
    if (tft) {
        tft->fillScreen(TFT_BLACK);
    }
}

void ST7789Display::display() {
    // TFT displays update immediately, no buffer to flush
    // This method is here for interface compatibility
}

void ST7789Display::setBrightness(uint8_t brightness) {
    current_brightness = brightness;
    
    if (config.bl_pin >= 0) {
        // Use PWM for backlight control
        analogWrite(config.bl_pin, brightness);
    }
}

void ST7789Display::setPowerMode(bool on) {
    if (tft) {
        if (on) {
            setBrightness(current_brightness);
        } else {
            setBrightness(0);
        }
    }
}

void ST7789Display::setTextSize(uint8_t size) {
    if (tft) {
        tft->setTextSize(size);
    }
}

void ST7789Display::setTextColor(uint16_t color) {
    if (tft) {
        tft->setTextColor(color, TFT_BLACK);
    }
}

void ST7789Display::setCursor(int16_t x, int16_t y) {
    if (tft) {
        tft->setCursor(x, y);
    }
}

void ST7789Display::print(const char* text) {
    if (tft) {
        tft->print(text);
    }
}

void ST7789Display::println(const char* text) {
    if (tft) {
        tft->println(text);
    }
}

void ST7789Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (tft) {
        tft->drawPixel(x, y, color);
    }
}

void ST7789Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (tft) {
        tft->drawLine(x0, y0, x1, y1, color);
    }
}

void ST7789Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (tft) {
        tft->drawRect(x, y, w, h, color);
    }
}

void ST7789Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (tft) {
        tft->fillRect(x, y, w, h, color);
    }
}

void ST7789Display::setupPins() {
    // Setup reset pin if specified
    if (config.rst_pin >= 0) {
        pinMode(config.rst_pin, OUTPUT);
        digitalWrite(config.rst_pin, HIGH);
    }
    
    // Setup backlight pin if specified
    if (config.bl_pin >= 0) {
        pinMode(config.bl_pin, OUTPUT);
        analogWrite(config.bl_pin, 255);  // Full brightness initially
    }
    
    // Setup CS pin if specified
    if (config.cs_pin >= 0) {
        pinMode(config.cs_pin, OUTPUT);
        digitalWrite(config.cs_pin, HIGH);
    }
    
    // Setup DC pin if specified
    if (config.dc_pin >= 0) {
        pinMode(config.dc_pin, OUTPUT);
    }
}

void ST7789Display::configureTFT() {
    // TFT_eSPI configuration is typically done through User_Setup.h
    // or platformio.ini build flags. This method can be used for
    // runtime configuration if needed.
}