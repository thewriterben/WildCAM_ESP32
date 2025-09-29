/**
 * @file display_manager.h
 * @brief Display management and abstraction layer
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "display_interface.h"
#include "ssd1306_display.h"
#include "st7789_display.h"
#include "../hal/camera_board.h"
#include <memory>

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();
    
    // Initialization
    bool init(CameraBoard* board);
    bool isInitialized() const { return display && display->isInitialized(); }
    
    // Display access
    DisplayInterface* getDisplay() const { return display.get(); }
    bool hasDisplay() const { return display != nullptr; }
    
    // Display detection
    DisplayType detectDisplay(CameraBoard* board);
    DisplayConfig getDisplayConfig(CameraBoard* board, DisplayType type);
    
    // Convenience methods
    void clear() { if (display) display->clear(); }
    void refresh() { if (display) display->display(); }
    void setBrightness(uint8_t brightness) { if (display) display->setBrightness(brightness); }
    void setPowerMode(bool on) { if (display) display->setPowerMode(on); }
    
    // Text output
    void print(const char* text) { if (display) display->print(text); }
    void println(const char* text) { if (display) display->println(text); }
    void printf(const char* format, ...);
    
    // Status display helpers
    void showStatus(const char* status);
    void showError(const char* error);
    void showBatteryLevel(int percentage);
    void showWiFiStatus(bool connected);

private:
    std::unique_ptr<DisplayInterface> display;
    CameraBoard* current_board;
    
    std::unique_ptr<DisplayInterface> createDisplay(DisplayType type, const DisplayConfig& config);
    DisplayConfig getDefaultOLEDConfig();
    DisplayConfig getDefaultTFTConfig();
    bool testI2CDisplay(uint8_t address);
    bool testSPIDisplay();
};

#endif // DISPLAY_MANAGER_H