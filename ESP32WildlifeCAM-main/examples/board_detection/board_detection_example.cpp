/**
 * @file board_detection_example.cpp
 * @brief Example demonstrating automatic board detection and HAL usage
 */

#include <Arduino.h>
#include "../firmware/src/hal/board_detector.h"
#include "../firmware/src/hal/camera_board.h"
#include "../firmware/src/camera_handler.h"

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Wildlife Camera - Board Detection Example");
    Serial.println("================================================");
    Serial.println("Supports: AI-Thinker ESP32-CAM, ESP32-S3-CAM, ESP-EYE");
    Serial.println();
    
    // Get chip information
    Serial.printf("Chip Model: %s\n", BoardDetector::getChipModel());
    Serial.printf("Chip ID: 0x%08X\n", BoardDetector::getChipId());
    Serial.printf("PSRAM Available: %s\n", BoardDetector::hasPSRAM() ? "Yes" : "No");
    Serial.println();
    
    // Detect board type
    BoardType detected_board = BoardDetector::detectBoardType();
    Serial.printf("Detected Board: %s\n", BoardDetector::getBoardName(detected_board));
    Serial.printf("Board Supported: %s\n", BoardDetector::isBoardSupported(detected_board) ? "Yes" : "No");
    Serial.println();
    
    // Create board instance
    auto board = BoardDetector::createBoard();
    if (board) {
        Serial.println("Board Information:");
        Serial.printf("  Name: %s\n", board->getBoardName());
        Serial.printf("  Type: %d\n", board->getBoardType());
        Serial.printf("  Supported: %s\n", board->isSupported() ? "Yes" : "No");
        
        // Initialize board
        if (board->init()) {
            Serial.println("  Initialization: SUCCESS");
            
            // Get GPIO mapping
            GPIOMap gpio_map = board->getGPIOMap();
            Serial.println("  GPIO Mapping:");
            Serial.printf("    PWDN: %d\n", gpio_map.pwdn_pin);
            Serial.printf("    RESET: %d\n", gpio_map.reset_pin);
            Serial.printf("    XCLK: %d\n", gpio_map.xclk_pin);
            Serial.printf("    SIOD: %d\n", gpio_map.siod_pin);
            Serial.printf("    SIOC: %d\n", gpio_map.sioc_pin);
            Serial.printf("    LED: %d\n", gpio_map.led_pin);
            
            // Get camera configuration
            CameraConfig cam_config = board->getCameraConfig();
            Serial.println("  Camera Configuration:");
            Serial.printf("    Max Frame Size: %d\n", cam_config.max_framesize);
            Serial.printf("    JPEG Quality: %d\n", cam_config.jpeg_quality);
            Serial.printf("    Frame Buffers: %d\n", cam_config.fb_count);
            Serial.printf("    XCLK Freq: %lu Hz\n", cam_config.xclk_freq_hz);
            Serial.printf("    PSRAM Required: %s\n", cam_config.psram_required ? "Yes" : "No");
            Serial.printf("    Has Flash: %s\n", cam_config.has_flash ? "Yes" : "No");
            
            // Get power profile
            PowerProfile power_profile = board->getPowerProfile();
            Serial.println("  Power Profile:");
            Serial.printf("    Sleep Current: %lu µA\n", power_profile.sleep_current_ua);
            Serial.printf("    Active Current: %lu mA\n", power_profile.active_current_ma);
            Serial.printf("    Camera Current: %lu mA\n", power_profile.camera_current_ma);
            Serial.printf("    External Power: %s\n", power_profile.has_external_power ? "Yes" : "No");
            Serial.printf("    Deep Sleep: %s\n", power_profile.supports_deep_sleep ? "Yes" : "No");
            Serial.printf("    Voltage Range: %.1fV - %.1fV\n", power_profile.min_voltage, power_profile.max_voltage);
            
            // Detect sensor
            if (board->detectSensor()) {
                Serial.printf("  Sensor Type: %d\n", board->getSensorType());
                Serial.println("  Sensor Detection: SUCCESS");
            } else {
                Serial.println("  Sensor Detection: FAILED");
            }
            
        } else {
            Serial.println("  Initialization: FAILED");
        }
    } else {
        Serial.println("Failed to create board instance");
    }
    
    Serial.println();
    Serial.println("Initializing camera handler with HAL...");
    
    // Test camera handler with HAL
    if (CameraHandler::init()) {
        Serial.println("Camera Handler: SUCCESS");
        
        // Get camera status
        CameraStatus status = CameraHandler::getStatus();
        Serial.println("Camera Status:");
        Serial.printf("  Initialized: %s\n", status.initialized ? "Yes" : "No");
        Serial.printf("  Board: %s\n", status.boardName);
        Serial.printf("  Sensor: %s\n", status.sensorName);
        Serial.printf("  Images Captured: %d\n", status.imageCount);
        
        // Get board instance from camera handler
        CameraBoard* cam_board = CameraHandler::getBoard();
        if (cam_board) {
            Serial.println("Camera board instance available");
            
            // Test LED flash
            Serial.println("Testing LED flash...");
            cam_board->flashLED(true);
            delay(500);
            cam_board->flashLED(false);
        }
        
    } else {
        Serial.println("Camera Handler: FAILED");
    }
    
    Serial.println("\nBoard detection example completed!");
}

void loop() {
    // Nothing to do in loop for this example
    delay(1000);
}