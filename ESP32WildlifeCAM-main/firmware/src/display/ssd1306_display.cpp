/**
 * @file ssd1306_display.cpp
 * @brief SSD1306 OLED display driver implementation
 */

#include "ssd1306_display.h"
#include "../debug_config.h"

SSD1306Display::SSD1306Display(const DisplayConfig& cfg) : oled(nullptr) {
    config = cfg;
}

SSD1306Display::~SSD1306Display() {
    if (oled) {
        delete oled;
        oled = nullptr;
    }
}

DisplayCapabilities SSD1306Display::getCapabilities() const {
    return {
        .width = 128,
        .height = 64,
        .supports_color = false,  // Monochrome display
        .supports_touch = false,
        .supports_graphics = true,
        .brightness_levels = 256
    };
}

bool SSD1306Display::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing SSD1306 OLED display...");
    
    setupPins();
    
    if (!initializeI2C()) {
        DEBUG_PRINTLN("Failed to initialize I2C for SSD1306");
        return false;
    }
    
    // Create OLED object
    oled = new Adafruit_SSD1306(getWidth(), getHeight(), &Wire, config.rst_pin);
    if (!oled) {
        DEBUG_PRINTLN("Failed to create SSD1306 object");
        return false;
    }
    
    // Initialize OLED
    if (!oled->begin(SSD1306_SWITCHCAPVCC, config.i2c_addr)) {
        DEBUG_PRINTLN("SSD1306 allocation failed");
        delete oled;
        oled = nullptr;
        return false;
    }
    
    // Clear the buffer
    oled->clearDisplay();
    oled->setTextSize(1);
    oled->setTextColor(SSD1306_WHITE);
    oled->setCursor(0, 0);
    oled->display();
    
    initialized = true;
    DEBUG_PRINTLN("SSD1306 OLED initialized successfully");
    
    return true;
}

void SSD1306Display::reset() {
    if (config.rst_pin >= 0) {
        digitalWrite(config.rst_pin, LOW);
        delay(10);
        digitalWrite(config.rst_pin, HIGH);
        delay(10);
    }
}

void SSD1306Display::clear() {
    if (oled) {
        oled->clearDisplay();
    }
}

void SSD1306Display::display() {
    if (oled) {
        oled->display();
    }
}

void SSD1306Display::setBrightness(uint8_t brightness) {
    if (oled) {
        // SSD1306 doesn't have direct brightness control
        // Could implement contrast control here
        oled->ssd1306_command(SSD1306_SETCONTRAST);
        oled->ssd1306_command(brightness);
    }
}

void SSD1306Display::setPowerMode(bool on) {
    if (oled) {
        if (on) {
            oled->ssd1306_command(SSD1306_DISPLAYON);
        } else {
            oled->ssd1306_command(SSD1306_DISPLAYOFF);
        }
    }
}

void SSD1306Display::setTextSize(uint8_t size) {
    if (oled) {
        oled->setTextSize(size);
    }
}

void SSD1306Display::setTextColor(uint16_t color) {
    if (oled) {
        // SSD1306 is monochrome, so we use white for non-zero colors
        oled->setTextColor(color > 0 ? SSD1306_WHITE : SSD1306_BLACK);
    }
}

void SSD1306Display::setCursor(int16_t x, int16_t y) {
    if (oled) {
        oled->setCursor(x, y);
    }
}

void SSD1306Display::print(const char* text) {
    if (oled) {
        oled->print(text);
    }
}

void SSD1306Display::println(const char* text) {
    if (oled) {
        oled->println(text);
    }
}

void SSD1306Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (oled) {
        oled->drawPixel(x, y, color > 0 ? SSD1306_WHITE : SSD1306_BLACK);
    }
}

void SSD1306Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (oled) {
        oled->drawLine(x0, y0, x1, y1, color > 0 ? SSD1306_WHITE : SSD1306_BLACK);
    }
}

void SSD1306Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (oled) {
        oled->drawRect(x, y, w, h, color > 0 ? SSD1306_WHITE : SSD1306_BLACK);
    }
}

void SSD1306Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (oled) {
        oled->fillRect(x, y, w, h, color > 0 ? SSD1306_WHITE : SSD1306_BLACK);
    }
}

bool SSD1306Display::initializeI2C() {
    // Initialize I2C with custom pins if specified
    if (config.sda_pin >= 0 && config.scl_pin >= 0) {
        Wire.begin(config.sda_pin, config.scl_pin);
    } else {
        Wire.begin();
    }
    
    // Set I2C frequency
    Wire.setClock(config.i2c_freq > 0 ? config.i2c_freq : 400000);
    
    // Test I2C communication
    Wire.beginTransmission(config.i2c_addr);
    return (Wire.endTransmission() == 0);
}

void SSD1306Display::setupPins() {
    // Setup reset pin if specified
    if (config.rst_pin >= 0) {
        pinMode(config.rst_pin, OUTPUT);
        digitalWrite(config.rst_pin, HIGH);
    }
    
    // Setup SDA/SCL pins if specified
    if (config.sda_pin >= 0) {
        pinMode(config.sda_pin, INPUT_PULLUP);
    }
    if (config.scl_pin >= 0) {
        pinMode(config.scl_pin, INPUT_PULLUP);
    }
}