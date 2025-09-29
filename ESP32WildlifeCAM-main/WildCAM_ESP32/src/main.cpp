#include <Arduino.h>
#include "src/core/system_manager.h"
#include "src/utils/logger.h"
#include "hardware/board_detector.h"

// Global system manager instance
SystemManager* g_system = nullptr;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect
    }
    
    // Print startup banner
    Logger::info("===================================");
    Logger::info("ESP32WildlifeCAM v2.5.0");
    Logger::info("Wildlife Camera System Test");
    Logger::info("===================================");
    
    // Detect board type
    BoardDetector::BoardType board = BoardDetector::detectBoard();
    Logger::info("Detected board: %s", BoardDetector::getBoardName(board));
    
    // Create and initialize system manager
    g_system = new SystemManager(board);
    if (!g_system->initialize()) {
        Logger::error("Failed to initialize system!");
        Logger::error("Last error: %s", g_system->getLastError());
        return; // Stay in setup - don't proceed to loop
    }
    
    Logger::info("System initialization complete!");
    Logger::info("Camera ready: %s", g_system->isCameraReady() ? "Yes" : "No");
    Logger::info("Storage ready: %s", g_system->isStorageReady() ? "Yes" : "No");
    
    // Test camera capture on successful initialization
    if (g_system->isCameraReady()) {
        Logger::info("Testing camera capture...");
        if (g_system->captureImage("/test")) {
            Logger::info("✅ Camera test successful!");
        } else {
            Logger::warning("⚠️ Camera test failed - but system initialized");
        }
    }
}

void loop() {
    if (g_system) {
        // Run main system loop
        g_system->update();
        
        // Test image capture every 30 seconds if camera is ready
        static unsigned long lastCapture = 0;
        unsigned long now = millis();
        
        if (g_system->isCameraReady() && (now - lastCapture > 30000)) {
            lastCapture = now;
            Logger::info("Periodic camera test...");
            g_system->captureImage("/wildlife");
        }
    }
    
    // Check for serial commands
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "capture" || command == "c") {
            if (g_system && g_system->isCameraReady()) {
                Logger::info("Manual capture requested...");
                if (g_system->captureImage("/manual")) {
                    Logger::info("✅ Manual capture successful!");
                } else {
                    Logger::error("❌ Manual capture failed!");
                }
            } else {
                Logger::error("Camera not ready for capture");
            }
        } else if (command == "status" || command == "s") {
            if (g_system) {
                Logger::info("=== System Status ===");
                Logger::info("Camera: %s", g_system->isCameraReady() ? "Ready" : "Not Ready");
                Logger::info("Storage: %s", g_system->isStorageReady() ? "Ready" : "Not Ready");
                Logger::info("Network: %s", g_system->isNetworkReady() ? "Ready" : "Not Ready");
                Logger::info("Free Heap: %d bytes", ESP.getFreeHeap());
                if (psramFound()) {
                    Logger::info("PSRAM: %d bytes", ESP.getPsramSize());
                }
            }
        } else if (command == "help" || command == "h") {
            Logger::info("=== Available Commands ===");
            Logger::info("c, capture - Take a photo");
            Logger::info("s, status - Show system status");
            Logger::info("h, help - Show this help");
        }
    }
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}