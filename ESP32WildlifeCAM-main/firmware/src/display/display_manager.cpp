/**
 * @file display_manager.cpp
 * @brief Display management and abstraction layer implementation
 */

#include "display_manager.h"
#include "../debug_config.h"
#include <Wire.h>
#include <SPI.h>
#include <stdarg.h>

DisplayManager::DisplayManager() : current_board(nullptr) {
}

DisplayManager::~DisplayManager() {
    display.reset();
}

bool DisplayManager::init(CameraBoard* board) {
    if (!board) {
        DEBUG_PRINTLN("No board provided for display initialization");
        return false;
    }
    
    current_board = board;
    
    DEBUG_PRINTLN("Initializing display manager...");
    
    // Detect display type
    DisplayType type = detectDisplay(board);
    if (type == DISPLAY_NONE) {
        DEBUG_PRINTLN("No display detected");
        return false;
    }
    
    // Get configuration for detected display
    DisplayConfig config = getDisplayConfig(board, type);
    
    // Create and initialize display
    display = createDisplay(type, config);
    if (!display) {
        DEBUG_PRINTLN("Failed to create display driver");
        return false;
    }
    
    if (!display->init()) {
        DEBUG_PRINTLN("Failed to initialize display");
        display.reset();
        return false;
    }
    
    DEBUG_PRINTF("Display initialized: %s\n", display->getDisplayName());
    
    // Show initial welcome message
    showStatus("Display Ready");
    
    return true;
}

DisplayType DisplayManager::detectDisplay(CameraBoard* board) {
    if (!board) return DISPLAY_NONE;
    
    BoardType boardType = board->getBoardType();
    
    // Board-specific display detection
    switch (boardType) {
        case BOARD_TTGO_T_CAMERA:
            // TTGO T-Camera has 0.96" OLED (SSD1306)
            if (testI2CDisplay(0x3C)) {
                DEBUG_PRINTLN("Detected SSD1306 OLED on TTGO T-Camera");
                return DISPLAY_SSD1306_OLED;
            }
            break;
            
        case BOARD_LILYGO_T_CAMERA_PLUS:
            // LilyGO T-Camera Plus has 1.14" TFT (ST7789)
            if (testSPIDisplay()) {
                DEBUG_PRINTLN("Detected ST7789 TFT on LilyGO T-Camera Plus");
                return DISPLAY_ST7789_TFT;
            }
            break;
            
        default:
            // Try generic I2C OLED detection
            if (testI2CDisplay(0x3C) || testI2CDisplay(0x3D)) {
                DEBUG_PRINTLN("Detected generic I2C OLED");
                return DISPLAY_SSD1306_OLED;
            }
            break;
    }
    
    return DISPLAY_NONE;
}

DisplayConfig DisplayManager::getDisplayConfig(CameraBoard* board, DisplayType type) {
    if (!board) return {};
    
    BoardType boardType = board->getBoardType();
    
    switch (type) {
        case DISPLAY_SSD1306_OLED:
            if (boardType == BOARD_TTGO_T_CAMERA) {
                // TTGO T-Camera OLED configuration
                return {
                    .type = DISPLAY_SSD1306_OLED,
                    .sda_pin = 21,    // Standard I2C SDA
                    .scl_pin = 22,    // Standard I2C SCL
                    .cs_pin = -1,
                    .dc_pin = -1,
                    .rst_pin = 16,    // Reset pin
                    .bl_pin = -1,
                    .i2c_freq = 400000,
                    .spi_freq = 0,
                    .i2c_addr = 0x3C
                };
            } else {
                return getDefaultOLEDConfig();
            }
            
        case DISPLAY_ST7789_TFT:
            if (boardType == BOARD_LILYGO_T_CAMERA_PLUS) {
                // LilyGO T-Camera Plus TFT configuration
                return {
                    .type = DISPLAY_ST7789_TFT,
                    .sda_pin = -1,
                    .scl_pin = -1,
                    .cs_pin = 12,     // SPI CS
                    .dc_pin = 13,     // Data/Command
                    .rst_pin = 14,    // Reset
                    .bl_pin = 15,     // Backlight
                    .i2c_freq = 0,
                    .spi_freq = 27000000,  // 27MHz
                    .i2c_addr = 0
                };
            } else {
                return getDefaultTFTConfig();
            }
            
        default:
            return {};
    }
}

std::unique_ptr<DisplayInterface> DisplayManager::createDisplay(DisplayType type, const DisplayConfig& config) {
    switch (type) {
        case DISPLAY_SSD1306_OLED:
            return std::make_unique<SSD1306Display>(config);
            
        case DISPLAY_ST7789_TFT:
            return std::make_unique<ST7789Display>(config);
            
        default:
            return nullptr;
    }
}

DisplayConfig DisplayManager::getDefaultOLEDConfig() {
    return {
        .type = DISPLAY_SSD1306_OLED,
        .sda_pin = 21,
        .scl_pin = 22,
        .cs_pin = -1,
        .dc_pin = -1,
        .rst_pin = -1,
        .bl_pin = -1,
        .i2c_freq = 400000,
        .spi_freq = 0,
        .i2c_addr = 0x3C
    };
}

DisplayConfig DisplayManager::getDefaultTFTConfig() {
    return {
        .type = DISPLAY_ST7789_TFT,
        .sda_pin = -1,
        .scl_pin = -1,
        .cs_pin = 5,
        .dc_pin = 2,
        .rst_pin = 4,
        .bl_pin = 15,
        .i2c_freq = 0,
        .spi_freq = 27000000,
        .i2c_addr = 0
    };
}

bool DisplayManager::testI2CDisplay(uint8_t address) {
    Wire.begin();
    Wire.setClock(100000);  // Slow speed for detection
    
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

bool DisplayManager::testSPIDisplay() {
    // For now, assume SPI display is present on LilyGO boards
    // A more sophisticated test could try to read display ID
    return true;
}

void DisplayManager::printf(const char* format, ...) {
    if (!display) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    display->print(buffer);
}

void DisplayManager::showStatus(const char* status) {
    if (!display) return;
    
    display->clear();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->setTextColor(0xFFFF);  // White
    display->println("Wildlife Camera");
    display->println("");
    display->print("Status: ");
    display->println(status);
    display->display();
}

void DisplayManager::showError(const char* error) {
    if (!display) return;
    
    display->clear();
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->setTextColor(0xF800);  // Red
    display->println("ERROR");
    display->setTextColor(0xFFFF);  // White
    display->println("");
    display->println(error);
    display->display();
}

void DisplayManager::showBatteryLevel(int percentage) {
    if (!display) return;
    
    uint16_t width = display->getWidth();
    uint16_t height = display->getHeight();
    
    // Draw battery icon and percentage
    display->setCursor(width - 50, 0);
    display->setTextSize(1);
    display->printf("Bat:%d%%", percentage);
    
    // Draw battery bar
    int barWidth = 30;
    int barHeight = 8;
    int barX = width - barWidth - 5;
    int barY = 10;
    
    display->drawRect(barX, barY, barWidth, barHeight, 0xFFFF);
    int fillWidth = (barWidth - 2) * percentage / 100;
    if (fillWidth > 0) {
        uint16_t color = percentage > 20 ? 0x07E0 : 0xF800;  // Green or Red
        display->fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, color);
    }
}

void DisplayManager::showWiFiStatus(bool connected) {
    if (!display) return;
    
    uint16_t width = display->getWidth();
    
    display->setCursor(0, 0);
    display->setTextSize(1);
    display->setTextColor(connected ? 0x07E0 : 0xF800);  // Green or Red
    display->print(connected ? "WiFi:ON" : "WiFi:OFF");
}