/**
 * @file hmi_system.cpp
 * @brief Human Machine Interface system implementation
 */

#include "hmi_system.h"
#include "../debug_config.h"
#include "../../src/data/storage_manager.h"
#include <esp_heap_caps.h>
#include <WiFi.h>

// Main menu items
const MenuItem HMISystem::main_menu_items[] = {
    {"Camera Settings", MENU_CAMERA_SETTINGS, handleCameraSettings, true},
    {"System Info", MENU_SYSTEM_INFO, handleSystemInfo, true},
    {"Power Status", MENU_POWER_STATUS, handlePowerStatus, true},
    {"File Browser", MENU_FILE_BROWSER, handleFileBrowser, true},
    {"Network", MENU_NETWORK_SETTINGS, handleNetworkSettings, true}
};

// Camera settings menu items
const MenuItem HMISystem::camera_menu_items[] = {
    {"Resolution", MENU_MAIN, nullptr, true},
    {"Quality", MENU_MAIN, nullptr, true},
    {"Flash", MENU_MAIN, nullptr, true},
    {"Timer", MENU_MAIN, nullptr, true},
    {"Back", MENU_MAIN, handleBack, true}
};

// System info menu items
const MenuItem HMISystem::system_menu_items[] = {
    {"Board Info", MENU_MAIN, nullptr, true},
    {"Memory", MENU_MAIN, nullptr, true},
    {"Version", MENU_MAIN, nullptr, true},
    {"Back", MENU_MAIN, handleBack, true}
};

HMISystem::HMISystem() :
    current_board(nullptr),
    initialized(false),
    current_display_state(DISPLAY_OFF),
    current_menu(MENU_MAIN),
    menu_selection(0),
    last_input_time(0),
    last_update_time(0),
    display_brightness(200),
    low_power_mode(false)
{
    memset(&system_status, 0, sizeof(system_status));
}

HMISystem::~HMISystem() {
    display_manager.reset();
}

bool HMISystem::init(CameraBoard* board) {
    if (!board) {
        DEBUG_PRINTLN("No board provided for HMI initialization");
        return false;
    }
    
    current_board = board;
    
    DEBUG_PRINTLN("Initializing HMI system...");
    
    // Initialize display manager
    display_manager = std::make_unique<DisplayManager>();
    if (!display_manager->init(board)) {
        DEBUG_PRINTLN("No display available - HMI will run without display");
        // Continue without display for headless operation
    }
    
    // Initialize system status
    updateSystemStatus();
    
    // Set initial display state
    if (hasDisplay()) {
        setDisplayState(DISPLAY_STATUS);
        showStatus();
    }
    
    last_input_time = millis();
    last_update_time = millis();
    
    initialized = true;
    DEBUG_PRINTLN("HMI system initialized successfully");
    
    return true;
}

void HMISystem::updateDisplay() {
    if (!hasDisplay() || !initialized) return;
    
    uint32_t now = millis();
    if (now - last_update_time < UPDATE_INTERVAL_MS) return;
    
    last_update_time = now;
    
    // Update system status
    updateSystemStatus();
    
    // Update display based on current state
    switch (current_display_state) {
        case DISPLAY_STATUS:
            showStatus();
            break;
            
        case DISPLAY_MENU:
            switch (current_menu) {
                case MENU_MAIN:
                    showMainMenu();
                    break;
                case MENU_CAMERA_SETTINGS:
                    showCameraSettings();
                    break;
                case MENU_SYSTEM_INFO:
                    showSystemInfo();
                    break;
                case MENU_POWER_STATUS:
                    showPowerStatus();
                    break;
                default:
                    showMainMenu();
                    break;
            }
            break;
            
        case DISPLAY_OFF:
            // Display is off, check for auto-wake conditions
            break;
            
        default:
            break;
    }
    
    // Check for auto-off
    checkAutoOff();
}

void HMISystem::setDisplayState(DisplayState state) {
    if (!hasDisplay()) return;
    
    current_display_state = state;
    
    if (state == DISPLAY_OFF) {
        display_manager->setPowerMode(false);
        enterLowPowerMode();
    } else {
        display_manager->setPowerMode(true);
        exitLowPowerMode();
    }
}

void HMISystem::showStatus() {
    if (!hasDisplay()) return;
    
    display_manager->clear();
    
    // Title
    display_manager->getDisplay()->setCursor(0, 0);
    display_manager->getDisplay()->setTextSize(1);
    display_manager->print("Wildlife Camera");
    
    // Status line
    drawStatusLine();
    
    // Camera status
    display_manager->getDisplay()->setCursor(0, 20);
    display_manager->printf("Camera: %s", system_status.camera_ready ? "Ready" : "Error");
    
    // Battery info
    display_manager->getDisplay()->setCursor(0, 30);
    display_manager->printf("Battery: %d%% %.2fV", 
                           system_status.battery_percentage, 
                           system_status.battery_voltage);
    
    // Memory info
    display_manager->getDisplay()->setCursor(0, 40);
    display_manager->printf("Free RAM: %dKB", system_status.free_heap / 1024);
    
    // Uptime
    display_manager->getDisplay()->setCursor(0, 50);
    uint32_t hours = system_status.uptime_seconds / 3600;
    uint32_t minutes = (system_status.uptime_seconds % 3600) / 60;
    display_manager->printf("Uptime: %dh%dm", hours, minutes);
    
    display_manager->refresh();
}

void HMISystem::showMainMenu() {
    if (!hasDisplay()) return;
    
    display_manager->clear();
    
    // Title
    display_manager->getDisplay()->setCursor(0, 0);
    display_manager->getDisplay()->setTextSize(1);
    display_manager->print("Main Menu");
    
    drawStatusLine();
    drawMenu(main_menu_items, 5);
    
    display_manager->refresh();
}

void HMISystem::showCameraSettings() {
    if (!hasDisplay()) return;
    
    display_manager->clear();
    
    display_manager->getDisplay()->setCursor(0, 0);
    display_manager->print("Camera Settings");
    
    drawStatusLine();
    drawMenu(camera_menu_items, 5);
    
    display_manager->refresh();
}

void HMISystem::showSystemInfo() {
    if (!hasDisplay()) return;
    
    display_manager->clear();
    
    display_manager->getDisplay()->setCursor(0, 0);
    display_manager->print("System Info");
    
    drawStatusLine();
    
    // Board information
    display_manager->getDisplay()->setCursor(0, 20);
    display_manager->printf("Board: %s", current_board->getBoardName());
    
    display_manager->getDisplay()->setCursor(0, 30);
    display_manager->printf("Chip: %s", current_board->getChipModel());
    
    display_manager->getDisplay()->setCursor(0, 40);
    display_manager->printf("PSRAM: %s", current_board->hasPSRAM() ? "Yes" : "No");
    
    display_manager->getDisplay()->setCursor(0, 50);
    display_manager->printf("Heap: %d bytes", system_status.free_heap);
    
    display_manager->refresh();
}

void HMISystem::showPowerStatus() {
    if (!hasDisplay()) return;
    
    display_manager->clear();
    
    display_manager->getDisplay()->setCursor(0, 0);
    display_manager->print("Power Status");
    
    drawStatusLine();
    
    // Battery details
    display_manager->getDisplay()->setCursor(0, 20);
    display_manager->printf("Battery: %.2fV (%d%%)", 
                           system_status.battery_voltage,
                           system_status.battery_percentage);
    
    display_manager->getDisplay()->setCursor(0, 30);
    display_manager->printf("Charging: %s", system_status.charging ? "Yes" : "No");
    
    // Solar voltage if available
    float solar_voltage = SolarManager::getSolarVoltage();
    display_manager->getDisplay()->setCursor(0, 40);
    display_manager->printf("Solar: %.2fV", solar_voltage);
    
    // Power state
    display_manager->getDisplay()->setCursor(0, 50);
    const char* power_state = "Unknown";
    switch (SolarManager::getPowerState()) {
        case PowerState::POWER_FULL: power_state = "Full"; break;
        case PowerState::POWER_GOOD: power_state = "Good"; break;
        case PowerState::POWER_LOW: power_state = "Low"; break;
        case PowerState::POWER_CRITICAL: power_state = "Critical"; break;
    }
    display_manager->printf("State: %s", power_state);
    
    display_manager->refresh();
}

void HMISystem::navigateMenu(MenuAction action) {
    if (!hasDisplay() || current_display_state != DISPLAY_MENU) return;
    
    resetInputTimeout();
    processMenuInput(action);
}

void HMISystem::handleButtonPress(uint8_t button) {
    if (!initialized) return;
    
    resetInputTimeout();
    
    // Wake up display if it's off
    if (current_display_state == DISPLAY_OFF) {
        setDisplayState(DISPLAY_STATUS);
        return;
    }
    
    // Convert button to menu action
    MenuAction action = ACTION_NONE;
    switch (button) {
        case 0: action = ACTION_UP; break;
        case 1: action = ACTION_DOWN; break;
        case 2: action = ACTION_SELECT; break;
        case 3: action = ACTION_BACK; break;
    }
    
    if (action != ACTION_NONE) {
        navigateMenu(action);
    }
}

void HMISystem::updateSystemStatus() {
    system_status.camera_ready = current_board && current_board->isSupported();
    system_status.display_ready = hasDisplay();
    system_status.wifi_connected = WiFi.status() == WL_CONNECTED;
    system_status.sd_card_present = StorageManager::initialize(); // Check SD card status
    system_status.battery_percentage = SolarManager::getBatteryPercentage();
    system_status.battery_voltage = SolarManager::getBatteryVoltage();
    system_status.charging = SolarManager::isCharging();
    system_status.free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    system_status.uptime_seconds = millis() / 1000;
}

void HMISystem::setBrightness(uint8_t brightness) {
    display_brightness = brightness;
    if (hasDisplay()) {
        display_manager->setBrightness(brightness);
    }
}

void HMISystem::adjustBrightness(bool increase) {
    if (increase && display_brightness < 255) {
        display_brightness += 25;
    } else if (!increase && display_brightness > 25) {
        display_brightness -= 25;
    }
    setBrightness(display_brightness);
}

void HMISystem::enterLowPowerMode() {
    if (low_power_mode) return;
    low_power_mode = true;
    DEBUG_PRINTLN("HMI entering low power mode");
}

void HMISystem::exitLowPowerMode() {
    if (!low_power_mode) return;
    low_power_mode = false;
    DEBUG_PRINTLN("HMI exiting low power mode");
}

void HMISystem::drawMenu(const MenuItem* items, int item_count) {
    if (!items || !hasDisplay()) return;
    
    int start_y = 20;
    int line_height = 10;
    
    for (int i = 0; i < item_count; i++) {
        display_manager->getDisplay()->setCursor(0, start_y + i * line_height);
        
        // Highlight selected item
        if (i == menu_selection) {
            display_manager->print("> ");
        } else {
            display_manager->print("  ");
        }
        
        display_manager->print(items[i].title);
    }
}

void HMISystem::drawStatusLine() {
    if (!hasDisplay()) return;
    
    int y = 10;
    
    // WiFi status
    drawWiFiIcon(0, y, system_status.wifi_connected);
    
    // Battery status
    drawBatteryIcon(100, y, system_status.battery_percentage);
}

void HMISystem::drawBatteryIcon(int x, int y, int percentage) {
    if (!hasDisplay()) return;
    
    DisplayInterface* display = display_manager->getDisplay();
    
    // Battery outline
    display->drawRect(x, y, 20, 8, 0xFFFF);
    display->drawRect(x + 20, y + 2, 2, 4, 0xFFFF);
    
    // Battery fill
    int fill_width = (18 * percentage) / 100;
    if (fill_width > 0) {
        uint16_t color = percentage > 20 ? 0x07E0 : 0xF800;  // Green or Red
        display->fillRect(x + 1, y + 1, fill_width, 6, color);
    }
}

void HMISystem::drawWiFiIcon(int x, int y, bool connected) {
    if (!hasDisplay()) return;
    
    DisplayInterface* display = display_manager->getDisplay();
    uint16_t color = connected ? 0x07E0 : 0xF800;  // Green or Red
    
    // Simple WiFi icon (3 arcs)
    for (int i = 0; i < 3; i++) {
        int radius = 3 + i * 2;
        display->drawPixel(x + radius, y + 8 - i, color);
    }
}

void HMISystem::processMenuInput(MenuAction action) {
    switch (action) {
        case ACTION_UP:
            if (menu_selection > 0) menu_selection--;
            break;
            
        case ACTION_DOWN:
            // TODO: Get actual menu item count
            if (menu_selection < 4) menu_selection++;
            break;
            
        case ACTION_SELECT:
            // TODO: Handle menu selection
            if (current_menu == MENU_MAIN && menu_selection == 0) {
                current_menu = MENU_CAMERA_SETTINGS;
                menu_selection = 0;
            }
            break;
            
        case ACTION_BACK:
            if (current_menu != MENU_MAIN) {
                current_menu = MENU_MAIN;
                menu_selection = 0;
            } else {
                setDisplayState(DISPLAY_STATUS);
            }
            break;
            
        default:
            break;
    }
}

void HMISystem::checkAutoOff() {
    if (isInputTimeout() && current_display_state != DISPLAY_OFF) {
        DEBUG_PRINTLN("HMI auto-off timeout");
        setDisplayState(DISPLAY_OFF);
    }
}

bool HMISystem::isInputTimeout() const {
    return (millis() - last_input_time) > AUTO_OFF_TIMEOUT_MS;
}

void HMISystem::resetInputTimeout() {
    last_input_time = millis();
}

// Static menu handlers
bool HMISystem::handleCameraSettings(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_CAMERA_SETTINGS;
    hmi->menu_selection = 0;
    return true;
}

bool HMISystem::handleSystemInfo(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_SYSTEM_INFO;
    hmi->menu_selection = 0;
    return true;
}

bool HMISystem::handlePowerStatus(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_POWER_STATUS;
    hmi->menu_selection = 0;
    return true;
}

bool HMISystem::handleFileBrowser(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_FILE_BROWSER;
    hmi->menu_selection = 0;
    return true;
}

bool HMISystem::handleNetworkSettings(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_NETWORK_SETTINGS;
    hmi->menu_selection = 0;
    return true;
}

bool HMISystem::handleBack(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    hmi->current_menu = MENU_MAIN;
    hmi->menu_selection = 0;
    return true;
}