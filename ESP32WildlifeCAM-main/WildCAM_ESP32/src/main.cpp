#include <Arduino.h>

// Minimal WildCAM ESP32 firmware for testing builds
// TODO: Integrate full system components after dependency resolution

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        ; // Wait for serial port to connect
    }
    
    Serial.println("===================================");
    Serial.println("WildCAM ESP32 Firmware v2.5.0");
    Serial.println("Build Test - Basic Functionality");
    Serial.println("===================================");
    
    // Basic board detection
    #ifdef BOARD_AI_THINKER_CAM
        Serial.println("Board: AI-Thinker ESP32-CAM");
    #elif defined(BOARD_ESP32S3_CAM)
        Serial.println("Board: ESP32-S3-CAM");
    #elif defined(BOARD_TTGO_T_CAMERA)
        Serial.println("Board: TTGO T-Camera");
    #else
        Serial.println("Board: Generic ESP32");
    #endif
    
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
    if (psramFound()) {
        Serial.print("PSRAM Size: ");
        Serial.print(ESP.getPsramSize());
        Serial.println(" bytes");
    }
    
    Serial.println("System ready - type 'help' for commands");
}

void loop() {
    // Check for serial commands
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "status" || command == "s") {
            Serial.println("=== System Status ===");
            Serial.print("Free Heap: ");
            Serial.print(ESP.getFreeHeap());
            Serial.println(" bytes");
            
            if (psramFound()) {
                Serial.print("PSRAM: ");
                Serial.print(ESP.getPsramSize());
                Serial.println(" bytes");
            }
            
            Serial.print("Uptime: ");
            Serial.print(millis() / 1000);
            Serial.println(" seconds");
            
        } else if (command == "help" || command == "h") {
            Serial.println("=== Available Commands ===");
            Serial.println("s, status - Show system status");
            Serial.println("h, help - Show this help");
            Serial.println("r, restart - Restart system");
            
        } else if (command == "restart" || command == "r") {
            Serial.println("Restarting system...");
            ESP.restart();
        }
    }
    
    // Heartbeat every 10 seconds
    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();
    
    if (now - lastHeartbeat > 10000) {
        lastHeartbeat = now;
        Serial.print(".");
    }
    
    // Small delay to prevent excessive CPU usage
    delay(100);
}