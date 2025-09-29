/**
 * @file hmi_system.h
 * @brief Human Machine Interface system for wildlife camera
 */

#ifndef HMI_SYSTEM_H
#define HMI_SYSTEM_H

#include "display_manager.h"
#include "../hal/camera_board.h"
#include "../solar_manager.h"
#include <memory>

// Menu system enums
enum MenuState {
    MENU_MAIN = 0,
    MENU_CAMERA_SETTINGS,
    MENU_SYSTEM_INFO,
    MENU_POWER_STATUS,
    MENU_FILE_BROWSER,
    MENU_NETWORK_SETTINGS,
    MENU_COUNT
};

enum MenuAction {
    ACTION_NONE = 0,
    ACTION_UP,
    ACTION_DOWN,
    ACTION_SELECT,
    ACTION_BACK,
    ACTION_COUNT
};

// Display states
enum DisplayState {
    DISPLAY_OFF = 0,
    DISPLAY_STATUS,
    DISPLAY_MENU,
    DISPLAY_SETTINGS,
    DISPLAY_ERROR,
    DISPLAY_COUNT
};

// Menu item structure
struct MenuItem {
    const char* title;
    MenuState target_menu;
    bool (*action_handler)(void* context);
    bool enabled;
};

// System status structure
struct SystemStatus {
    bool camera_ready;
    bool display_ready;
    bool wifi_connected;
    bool sd_card_present;
    int battery_percentage;
    float battery_voltage;
    bool charging;
    uint32_t free_heap;
    uint32_t uptime_seconds;
};

class HMISystem {
public:
    HMISystem();
    ~HMISystem();
    
    // Initialization
    bool init(CameraBoard* board);
    bool isInitialized() const { return initialized; }
    
    // Display management
    bool hasDisplay() const { return display_manager && display_manager->hasDisplay(); }
    void updateDisplay();
    void setDisplayState(DisplayState state);
    void showStatus();
    void showError(const char* error);
    
    // Menu system
    void showMainMenu();
    void showCameraSettings();
    void showSystemInfo();
    void showPowerStatus();
    void navigateMenu(MenuAction action);
    
    // Input handling
    void handleButtonPress(uint8_t button);
    void processInput();
    
    // Status updates
    void updateSystemStatus();
    SystemStatus getSystemStatus() const { return system_status; }
    
    // Display brightness control
    void setBrightness(uint8_t brightness);
    void adjustBrightness(bool increase);
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();

private:
    std::unique_ptr<DisplayManager> display_manager;
    CameraBoard* current_board;
    
    // State management
    bool initialized;
    DisplayState current_display_state;
    MenuState current_menu;
    int menu_selection;
    uint32_t last_input_time;
    uint32_t last_update_time;
    uint8_t display_brightness;
    bool low_power_mode;
    
    // System status
    SystemStatus system_status;
    
    // Menu definitions
    static const MenuItem main_menu_items[];
    static const MenuItem camera_menu_items[];
    static const MenuItem system_menu_items[];
    
    // Display helpers
    void drawMenu(const MenuItem* items, int item_count);
    void drawStatusLine();
    void drawBatteryIcon(int x, int y, int percentage);
    void drawWiFiIcon(int x, int y, bool connected);
    void clearDisplay();
    void refreshDisplay();
    
    // Menu handlers
    static bool handleCameraSettings(void* context);
    static bool handleSystemInfo(void* context);
    static bool handlePowerStatus(void* context);
    static bool handleFileBrowser(void* context);
    static bool handleNetworkSettings(void* context);
    static bool handleBack(void* context);
    
    // Input processing
    void processMenuInput(MenuAction action);
    bool isInputTimeout() const;
    void resetInputTimeout();
    
    // Auto-off timer
    void checkAutoOff();
    static const uint32_t AUTO_OFF_TIMEOUT_MS = 30000;  // 30 seconds
    static const uint32_t UPDATE_INTERVAL_MS = 1000;    // 1 second
};

#endif // HMI_SYSTEM_H