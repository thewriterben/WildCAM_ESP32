/**
 * @file hmi_demo.cpp
 * @brief HMI system demonstration for T-Camera boards
 * 
 * This example demonstrates the HMI (Human Machine Interface) system
 * for LilyGO T-Camera and TTGO T-Camera boards with built-in displays.
 * 
 * Features demonstrated:
 * - Automatic display detection
 * - Status display with battery and system information
 * - Menu navigation system
 * - Power management integration
 * - Real-time system monitoring
 */

#include <Arduino.h>
#include "../../firmware/src/config.h"
#include "../../firmware/src/display/hmi_system.h"
#include "../../firmware/src/hal/board_detector.h"
#include "../../firmware/src/solar_manager.h"

// Global objects
HMISystem hmi;
std::unique_ptr<CameraBoard> board;
unsigned long lastUpdate = 0;

// Function declarations
void processCommand(char cmd);
void printSystemStatus();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("ESP32 Wildlife Camera - HMI Demo");
    Serial.println("========================================");
    
    // Detect camera board
    Serial.println("Detecting camera board...");
    BoardDetector detector;
    BoardType boardType = detector.detectBoard();
    board = detector.createBoard(boardType);
    
    if (!board) {
        Serial.println("ERROR: Could not detect camera board!");
        Serial.println("This demo requires a supported T-Camera board.");
        while (1) delay(1000);
    }
    
    Serial.printf("Detected board: %s\n", board->getBoardName());
    
    // Initialize board
    if (!board->init()) {
        Serial.println("WARNING: Board initialization failed");
    }
    
    // Initialize HMI system
    Serial.println("Initializing HMI system...");
    if (!hmi.init(board.get())) {
        Serial.println("ERROR: HMI initialization failed!");
        Serial.println("Check that your board has a supported display.");
        while (1) delay(1000);
    }
    
    if (hmi.hasDisplay()) {
        Serial.println("HMI system initialized successfully!");
        Serial.println("Display detected and ready");
        
        // Show welcome message
        hmi.showStatus();
        delay(2000);
        
        // Enter menu mode  
        hmi.setDisplayState(DISPLAY_MENU);
        hmi.showMainMenu();
    } else {
        Serial.println("No display detected - running without HMI");
    }
    
    // Initialize solar manager for power monitoring
    SolarManager::init();
    
    Serial.println("\nHMI Demo ready!");
    Serial.println("Commands:");
    Serial.println("  'u' - Navigate Up");
    Serial.println("  'd' - Navigate Down");
    Serial.println("  's' - Select/Enter");
    Serial.println("  'b' - Back");
    Serial.println("  'i' - Show System Info");
    Serial.println("  'p' - Show Power Status");
    Serial.println("  'h' - Show Status");
    Serial.println("  '+' - Increase Brightness");
    Serial.println("  '-' - Decrease Brightness");
    Serial.println("========================================\n");
}

void loop() {
    unsigned long now = millis();
    
    // Update HMI system
    if (hmi.isInitialized()) {
        hmi.updateDisplay();
        hmi.processInput();
    }
    
    // Update solar manager
    SolarManager::update();
    
    // Process serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        processCommand(cmd);
    }
    
    // Print system status every 10 seconds
    if (now - lastUpdate > 10000) {
        printSystemStatus();
        lastUpdate = now;
    }
    
    delay(100);
}

void processCommand(char cmd) {
    switch (cmd) {
        case 'u':
        case 'U':
            Serial.println("Command: Navigate Up");
            hmi.navigateMenu(ACTION_UP);
            break;
            
        case 'd':
        case 'D':
            Serial.println("Command: Navigate Down");
            hmi.navigateMenu(ACTION_DOWN);
            break;
            
        case 's':
        case 'S':
            Serial.println("Command: Select");
            hmi.navigateMenu(ACTION_SELECT);
            break;
            
        case 'b':
        case 'B':
            Serial.println("Command: Back");
            hmi.navigateMenu(ACTION_BACK);
            break;
            
        case 'i':
        case 'I':
            Serial.println("Command: Show System Info");
            hmi.setDisplayState(DISPLAY_MENU);
            hmi.showSystemInfo();
            break;
            
        case 'p':
        case 'P':
            Serial.println("Command: Show Power Status");
            hmi.setDisplayState(DISPLAY_MENU);
            hmi.showPowerStatus();
            break;
            
        case 'h':
        case 'H':
            Serial.println("Command: Show Status");
            hmi.setDisplayState(DISPLAY_STATUS);
            hmi.showStatus();
            break;
            
        case '+':
            Serial.println("Command: Increase Brightness");
            hmi.adjustBrightness(true);
            break;
            
        case '-':
            Serial.println("Command: Decrease Brightness");
            hmi.adjustBrightness(false);
            break;
            
        default:
            Serial.printf("Unknown command: %c\n", cmd);
            break;
    }
}

void printSystemStatus() {
    SystemStatus status = hmi.getSystemStatus();
    
    Serial.println("\n--- System Status ---");
    Serial.printf("Board: %s\n", board->getBoardName());
    Serial.printf("Camera: %s\n", status.camera_ready ? "Ready" : "Error");
    Serial.printf("Display: %s\n", status.display_ready ? "Ready" : "None");
    Serial.printf("WiFi: %s\n", status.wifi_connected ? "Connected" : "Disconnected");
    Serial.printf("Battery: %d%% (%.2fV)\n", status.battery_percentage, status.battery_voltage);
    Serial.printf("Charging: %s\n", status.charging ? "Yes" : "No");
    Serial.printf("Free Heap: %d KB\n", status.free_heap / 1024);
    Serial.printf("Uptime: %d seconds\n", status.uptime_seconds);
    Serial.println("--------------------\n");
}