/**
 * @file board_detector.cpp
 * @brief Board detection implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "board_detector.h"
#include "../utils/logger.h"
#include <soc/efuse_reg.h>
#include <Wire.h>
#include <algorithm>

// Static member initialization
BoardDetector::BoardType BoardDetector::s_detectedBoard = BoardDetector::UNKNOWN;
std::map<BoardDetector::BoardType, BoardDetector::PinConfig> BoardDetector::s_boardConfigs;
std::map<BoardDetector::BoardType, std::vector<BoardDetector::Feature>> BoardDetector::s_boardFeatures;

void BoardDetector::initializeBoardConfigs() {
    // AI-Thinker ESP32-CAM configuration
    PinConfig aiThinkerConfig;
    aiThinkerConfig.cam_pwdn = 32;
    aiThinkerConfig.cam_reset = -1;
    aiThinkerConfig.cam_xclk = 0;
    aiThinkerConfig.cam_siod = 26;
    aiThinkerConfig.cam_sioc = 27;
    aiThinkerConfig.cam_y9 = 35;
    aiThinkerConfig.cam_y8 = 34;
    aiThinkerConfig.cam_y7 = 39;
    aiThinkerConfig.cam_y6 = 36;
    aiThinkerConfig.cam_y5 = 21;
    aiThinkerConfig.cam_y4 = 19;
    aiThinkerConfig.cam_y3 = 18;
    aiThinkerConfig.cam_y2 = 5;
    aiThinkerConfig.cam_vsync = 25;
    aiThinkerConfig.cam_href = 23;
    aiThinkerConfig.cam_pclk = 22;
    
    // SD card (shares SPI with LoRa - needs careful management)
    aiThinkerConfig.sd_cs = 13;
    aiThinkerConfig.sd_mosi = 15;
    aiThinkerConfig.sd_miso = 2;
    aiThinkerConfig.sd_clk = 14;
    
    // LoRa module
    aiThinkerConfig.lora_cs = 12;
    aiThinkerConfig.lora_rst = 16;
    aiThinkerConfig.lora_irq = 4;
    aiThinkerConfig.lora_mosi = 15; // Shared with SD
    aiThinkerConfig.lora_miso = 2;  // Shared with SD
    aiThinkerConfig.lora_sck = 14;  // Shared with SD
    
    // Other peripherals  
    aiThinkerConfig.pir_pin = 1;  // Corrected from 33 to 1 per audit report
    aiThinkerConfig.led_pin = 4;
    aiThinkerConfig.battery_pin = 33; // ADC1_CH5
    aiThinkerConfig.solar_pin = 32;   // Shared with camera PWDN
    aiThinkerConfig.i2c_sda = 26;     // Shared with camera SIOD
    aiThinkerConfig.i2c_scl = 27;     // Shared with camera SIOC
    
    s_boardConfigs[AI_THINKER_CAM] = aiThinkerConfig;
    
    // AI-Thinker features
    s_boardFeatures[AI_THINKER_CAM] = {
        FEATURE_CAMERA,
        FEATURE_SD_CARD,
        FEATURE_PSRAM,
        FEATURE_WIFI,
        FEATURE_BLUETOOTH,
        FEATURE_BATTERY_MONITOR
    };
    
    // ESP32-S3-CAM configuration
    PinConfig s3CamConfig;
    // Camera pins (typical ESP32-S3-CAM)
    s3CamConfig.cam_pwdn = 38;
    s3CamConfig.cam_reset = -1;
    s3CamConfig.cam_xclk = 15;
    s3CamConfig.cam_siod = 4;
    s3CamConfig.cam_sioc = 5;
    s3CamConfig.cam_y9 = 16;
    s3CamConfig.cam_y8 = 17;
    s3CamConfig.cam_y7 = 18;
    s3CamConfig.cam_y6 = 12;
    s3CamConfig.cam_y5 = 10;
    s3CamConfig.cam_y4 = 8;
    s3CamConfig.cam_y3 = 9;
    s3CamConfig.cam_y2 = 11;
    s3CamConfig.cam_vsync = 6;
    s3CamConfig.cam_href = 7;
    s3CamConfig.cam_pclk = 13;
    
    // SD card pins
    s3CamConfig.sd_cs = 21;
    s3CamConfig.sd_mosi = 47;
    s3CamConfig.sd_miso = 14;
    s3CamConfig.sd_clk = 48;
    
    // Other peripherals
    s3CamConfig.pir_pin = 1;
    s3CamConfig.led_pin = 2;
    s3CamConfig.battery_pin = 1;  // ADC
    s3CamConfig.solar_pin = 2;
    s3CamConfig.i2c_sda = 4;      // Shared with camera
    s3CamConfig.i2c_scl = 5;      // Shared with camera
    
    s_boardConfigs[ESP32_S3_CAM] = s3CamConfig;
    
    // ESP32-S3-CAM features
    s_boardFeatures[ESP32_S3_CAM] = {
        FEATURE_CAMERA,
        FEATURE_SD_CARD,
        FEATURE_PSRAM,
        FEATURE_WIFI,
        FEATURE_BLUETOOTH,
        FEATURE_USB,
        FEATURE_BATTERY_MONITOR
    };
    
    // TTGO T-Camera configuration  
    PinConfig ttgoCamConfig;
    // Camera pins (TTGO T-Camera)
    ttgoCamConfig.cam_pwdn = 26;
    ttgoCamConfig.cam_reset = -1;
    ttgoCamConfig.cam_xclk = 32;
    ttgoCamConfig.cam_siod = 13;
    ttgoCamConfig.cam_sioc = 12;
    ttgoCamConfig.cam_y9 = 39;
    ttgoCamConfig.cam_y8 = 36;
    ttgoCamConfig.cam_y7 = 23;
    ttgoCamConfig.cam_y6 = 18;
    ttgoCamConfig.cam_y5 = 15;
    ttgoCamConfig.cam_y4 = 4;
    ttgoCamConfig.cam_y3 = 14;
    ttgoCamConfig.cam_y2 = 5;
    ttgoCamConfig.cam_vsync = 27;
    ttgoCamConfig.cam_href = 25;
    ttgoCamConfig.cam_pclk = 19;
    
    // SD card pins
    ttgoCamConfig.sd_cs = 0;
    ttgoCamConfig.sd_mosi = 15;
    ttgoCamConfig.sd_miso = 2;
    ttgoCamConfig.sd_clk = 14;
    
    // Other peripherals
    ttgoCamConfig.pir_pin = 33;
    ttgoCamConfig.led_pin = 2;
    ttgoCamConfig.battery_pin = 35;
    ttgoCamConfig.solar_pin = 34;
    ttgoCamConfig.i2c_sda = 21;
    ttgoCamConfig.i2c_scl = 22;
    
    s_boardConfigs[TTGO_T_CAMERA] = ttgoCamConfig;
    
    // TTGO T-Camera features
    s_boardFeatures[TTGO_T_CAMERA] = {
        FEATURE_CAMERA,
        FEATURE_SD_CARD,
        FEATURE_PSRAM,
        FEATURE_WIFI,
        FEATURE_BLUETOOTH,
        FEATURE_DISPLAY,
        FEATURE_BATTERY_MONITOR
    };
}

BoardDetector::BoardType BoardDetector::detectBoard() {
    if (s_detectedBoard != UNKNOWN) {
        return s_detectedBoard;
    }
    
    initializeBoardConfigs();
    
    Logger::info("Starting board detection...");
    
    // Try detection methods in order of reliability
    s_detectedBoard = detectByChipModel();
    if (s_detectedBoard != UNKNOWN) {
        Logger::info("Board detected by chip model: %s", getBoardName(s_detectedBoard));
        return s_detectedBoard;
    }
    
    s_detectedBoard = detectByPSRAM();
    if (s_detectedBoard != UNKNOWN) {
        Logger::info("Board detected by PSRAM: %s", getBoardName(s_detectedBoard));
        return s_detectedBoard;
    }
    
    s_detectedBoard = detectByI2CScan();
    if (s_detectedBoard != UNKNOWN) {
        Logger::info("Board detected by I2C scan: %s", getBoardName(s_detectedBoard));
        return s_detectedBoard;
    }
    
    s_detectedBoard = detectByGPIO();
    Logger::info("Board detected by GPIO: %s", getBoardName(s_detectedBoard));
    
    return s_detectedBoard;
}

BoardDetector::BoardType BoardDetector::detectByChipModel() {
    uint32_t chip_model = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t chip_revision = ESP.getChipRevision();
    
    Logger::debug("Chip model: 0x%X, revision: %d", chip_model, chip_revision);
    
    // Check for specific board identifiers
    #ifdef BOARD_AI_THINKER_CAM
        return AI_THINKER_CAM;
    #elif defined(BOARD_ESP32S3_CAM)
        return ESP32_S3_CAM;
    #elif defined(BOARD_TTGO_T_CAMERA)
        return TTGO_T_CAMERA;
    #endif
    
    return UNKNOWN;
}

BoardDetector::BoardType BoardDetector::detectByPSRAM() {
    if (psramFound()) {
        size_t psram_size = ESP.getPsramSize();
        Logger::debug("PSRAM found: %d bytes", psram_size);
        
        // AI-Thinker typically has 4MB PSRAM
        if (psram_size >= 4 * 1024 * 1024) {
            return AI_THINKER_CAM;
        }
    }
    
    return UNKNOWN;
}

BoardDetector::BoardType BoardDetector::detectByGPIO() {
    Logger::debug("Detecting board by GPIO testing...");
    
    // Test AI-Thinker specific pins
    if (testGPIOPin(0) && testGPIOPin(26) && testGPIOPin(27)) {
        // Test camera-specific pins for AI-Thinker
        if (testGPIOPin(4) && testGPIOPin(32)) {
            // GPIO4 has flash LED on AI-Thinker
            // GPIO32 is camera PWDN on AI-Thinker
            Logger::debug("AI-Thinker camera pins validated");
            
            // Validate camera pin configuration
            PinConfig aiConfig = s_boardConfigs[AI_THINKER_CAM];
            if (validateCameraPins(aiConfig)) {
                return AI_THINKER_CAM;
            }
        }
    }
    
    // Test ESP32-S3 specific pins
    if (testGPIOPin(15) && testGPIOPin(4) && testGPIOPin(5)) {
        // ESP32-S3-CAM typical pins
        Logger::debug("ESP32-S3 camera pins detected");
        return ESP32_S3_CAM;
    }
    
    // Test TTGO T-Camera specific pins
    if (testGPIOPin(32) && testGPIOPin(13) && testGPIOPin(12)) {
        // TTGO T-Camera specific configuration
        Logger::debug("TTGO T-Camera pins detected");
        return TTGO_T_CAMERA;
    }
    
    // Default to AI-Thinker if basic camera pins work
    if (testGPIOPin(0) && testGPIOPin(26) && testGPIOPin(27)) {
        Logger::debug("Defaulting to AI-Thinker based on basic pin test");
        return AI_THINKER_CAM;
    }
    
    Logger::warning("No known board pattern detected");
    return UNKNOWN;
}

const char* BoardDetector::getBoardName(BoardType board) {
    switch (board) {
        case AI_THINKER_CAM: return "AI-Thinker ESP32-CAM";
        case ESP32_S3_CAM: return "ESP32-S3-CAM";
        case TTGO_T_CAMERA: return "TTGO T-Camera";
        case CUSTOM: return "Custom Board";
        default: return "Unknown Board";
    }
}

BoardDetector::BoardType BoardDetector::detectByI2CScan() {
    Logger::debug("Detecting board by I2C scan...");
    
    // Test different I2C configurations for different boards
    std::vector<uint8_t> devices;
    
    // Try AI-Thinker I2C pins (26, 27)
    devices = scanI2CDevices(26, 27);
    if (!devices.empty()) {
        Logger::debug("I2C devices found on pins 26,27 (AI-Thinker style)");
        for (uint8_t addr : devices) {
            Logger::debug("  Device at 0x%02X", addr);
        }
        return AI_THINKER_CAM;
    }
    
    // Try ESP32-S3-CAM I2C pins (4, 5)
    devices = scanI2CDevices(4, 5);
    if (!devices.empty()) {
        Logger::debug("I2C devices found on pins 4,5 (ESP32-S3 style)");
        return ESP32_S3_CAM;
    }
    
    // Try TTGO T-Camera I2C pins (21, 22)
    devices = scanI2CDevices(21, 22);
    if (!devices.empty()) {
        Logger::debug("I2C devices found on pins 21,22 (TTGO style)");
        return TTGO_T_CAMERA;
    }
    
    return UNKNOWN;
}

bool BoardDetector::testGPIOPin(int pin) {
    if (pin < 0 || pin > 39) {
        return false;
    }
    
    // Skip pins connected to flash (6-11)
    if (pin >= 6 && pin <= 11) {
        return false;
    }
    
    // Test if we can configure the pin
    pinMode(pin, INPUT);
    delay(1);
    
    // For input-only pins (34-39), just verify they can be read
    if (pin >= 34 && pin <= 39) {
        digitalRead(pin);
        return true;
    }
    
    // For regular GPIO, test output capability
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
    delay(1);
    pinMode(pin, INPUT_PULLUP);
    bool high = digitalRead(pin);
    
    digitalWrite(pin, LOW);
    delay(1);
    pinMode(pin, INPUT_PULLDOWN);
    bool low = digitalRead(pin);
    
    // Reset to input
    pinMode(pin, INPUT);
    
    return (high && !low);
}

bool BoardDetector::scanI2CBus(int sda, int scl) {
    if (!testGPIOPin(sda) || !testGPIOPin(scl)) {
        return false;
    }
    
    Wire.begin(sda, scl);
    delay(100);
    
    bool found = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            found = true;
            break;
        }
    }
    
    Wire.end();
    return found;
}

std::vector<uint8_t> BoardDetector::scanI2CDevices(int sda, int scl) {
    std::vector<uint8_t> devices;
    
    if (!testGPIOPin(sda) || !testGPIOPin(scl)) {
        return devices;
    }
    
    Wire.begin(sda, scl);
    delay(100);
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            devices.push_back(addr);
        }
        delay(1);
    }
    
    Wire.end();
    return devices;
}

bool BoardDetector::validateCameraPins(const PinConfig& config) {
    // Test critical camera pins
    if (!testGPIOPin(config.cam_xclk)) {
        Logger::error("Camera XCLK pin %d test failed", config.cam_xclk);
        return false;
    }
    
    if (!testGPIOPin(config.cam_siod)) {
        Logger::error("Camera SIOD pin %d test failed", config.cam_siod);
        return false;
    }
    
    if (!testGPIOPin(config.cam_sioc)) {
        Logger::error("Camera SIOC pin %d test failed", config.cam_sioc);
        return false;
    }
    
    // Test data pins (input-only pins 34-39 are OK)
    int dataPins[] = {config.cam_y2, config.cam_y3, config.cam_y4, config.cam_y5,
                      config.cam_y6, config.cam_y7, config.cam_y8, config.cam_y9};
    
    for (int pin : dataPins) {
        if (pin >= 0 && !testGPIOPin(pin)) {
            Logger::warning("Camera data pin %d test failed", pin);
        }
    }
    
    return true;
}

bool BoardDetector::hasFeature(BoardType board, Feature feature) {
    if (s_boardFeatures.find(board) == s_boardFeatures.end()) {
        return false;
    }
    
    const std::vector<Feature>& features = s_boardFeatures[board];
    return std::find(features.begin(), features.end(), feature) != features.end();
}

bool BoardDetector::hasFeature(Feature feature) {
    BoardType board = detectBoard();
    return hasFeature(board, feature);
}

BoardDetector::PinConfig BoardDetector::getPinConfig(BoardType board) {
    if (s_boardConfigs.find(board) != s_boardConfigs.end()) {
        return s_boardConfigs[board];
    }
    
    // Return empty config for unknown boards
    return PinConfig();
}

BoardDetector::PinConfig BoardDetector::getPinConfig() {
    BoardType board = detectBoard();
    return getPinConfig(board);
}

std::vector<int8_t> BoardDetector::getUsedPins(BoardType board) {
    std::vector<int8_t> usedPins;
    PinConfig config = getPinConfig(board);
    
    // Add all configured pins to the used list
    if (config.cam_pwdn >= 0) usedPins.push_back(config.cam_pwdn);
    if (config.cam_reset >= 0) usedPins.push_back(config.cam_reset);
    if (config.cam_xclk >= 0) usedPins.push_back(config.cam_xclk);
    if (config.cam_siod >= 0) usedPins.push_back(config.cam_siod);
    if (config.cam_sioc >= 0) usedPins.push_back(config.cam_sioc);
    if (config.cam_y9 >= 0) usedPins.push_back(config.cam_y9);
    if (config.cam_y8 >= 0) usedPins.push_back(config.cam_y8);
    if (config.cam_y7 >= 0) usedPins.push_back(config.cam_y7);
    if (config.cam_y6 >= 0) usedPins.push_back(config.cam_y6);
    if (config.cam_y5 >= 0) usedPins.push_back(config.cam_y5);
    if (config.cam_y4 >= 0) usedPins.push_back(config.cam_y4);
    if (config.cam_y3 >= 0) usedPins.push_back(config.cam_y3);
    if (config.cam_y2 >= 0) usedPins.push_back(config.cam_y2);
    if (config.cam_vsync >= 0) usedPins.push_back(config.cam_vsync);
    if (config.cam_href >= 0) usedPins.push_back(config.cam_href);
    if (config.cam_pclk >= 0) usedPins.push_back(config.cam_pclk);
    
    if (config.sd_cs >= 0) usedPins.push_back(config.sd_cs);
    if (config.sd_mosi >= 0) usedPins.push_back(config.sd_mosi);
    if (config.sd_miso >= 0) usedPins.push_back(config.sd_miso);
    if (config.sd_clk >= 0) usedPins.push_back(config.sd_clk);
    
    if (config.pir_pin >= 0) usedPins.push_back(config.pir_pin);
    if (config.led_pin >= 0) usedPins.push_back(config.led_pin);
    if (config.battery_pin >= 0) usedPins.push_back(config.battery_pin);
    if (config.solar_pin >= 0) usedPins.push_back(config.solar_pin);
    if (config.i2c_sda >= 0) usedPins.push_back(config.i2c_sda);
    if (config.i2c_scl >= 0) usedPins.push_back(config.i2c_scl);
    
    // Remove duplicates and sort
    std::sort(usedPins.begin(), usedPins.end());
    usedPins.erase(std::unique(usedPins.begin(), usedPins.end()), usedPins.end());
    
    return usedPins;
}

std::vector<int8_t> BoardDetector::getAvailablePins(BoardType board) {
    std::vector<int8_t> availablePins;
    std::vector<int8_t> usedPins = getUsedPins(board);
    
    // ESP32 has pins 0-39, but some are restricted
    for (int8_t pin = 0; pin <= 39; pin++) {
        // Skip flash pins (6-11)
        if (pin >= 6 && pin <= 11) continue;
        
        // Skip if already used
        if (std::find(usedPins.begin(), usedPins.end(), pin) != usedPins.end()) continue;
        
        // Add to available pins
        availablePins.push_back(pin);
    }
    
    return availablePins;
}

bool BoardDetector::hasFeature(BoardType board, Feature feature) {
    auto it = s_boardFeatures.find(board);
    if (it != s_boardFeatures.end()) {
        const auto& features = it->second;
        return std::find(features.begin(), features.end(), feature) != features.end();
    }
    return false;
}

bool BoardDetector::hasFeature(Feature feature) {
    if (s_detectedBoard == UNKNOWN) {
        detectBoard();
    }
    return hasFeature(s_detectedBoard, feature);
}

BoardDetector::PinConfig BoardDetector::getPinConfig(BoardType board) {
    auto it = s_boardConfigs.find(board);
    if (it != s_boardConfigs.end()) {
        return it->second;
    }
    return PinConfig(); // Return default config
}

BoardDetector::PinConfig BoardDetector::getPinConfig() {
    if (s_detectedBoard == UNKNOWN) {
        detectBoard();
    }
    return getPinConfig(s_detectedBoard);
}

std::vector<int8_t> BoardDetector::getUsedPins(BoardType board) {
    std::vector<int8_t> used_pins;
    PinConfig config = getPinConfig(board);
    
    // Add all configured pins to the used list
    if (config.cam_pwdn >= 0) used_pins.push_back(config.cam_pwdn);
    if (config.cam_xclk >= 0) used_pins.push_back(config.cam_xclk);
    if (config.cam_siod >= 0) used_pins.push_back(config.cam_siod);
    if (config.cam_sioc >= 0) used_pins.push_back(config.cam_sioc);
    // ... add all other pins
    
    // Remove duplicates
    std::sort(used_pins.begin(), used_pins.end());
    used_pins.erase(std::unique(used_pins.begin(), used_pins.end()), used_pins.end());
    
    return used_pins;
}

std::vector<int8_t> BoardDetector::getAvailablePins(BoardType board) {
    std::vector<int8_t> all_pins;
    std::vector<int8_t> available_pins;
    
    // Define available GPIO pins for ESP32
    for (int i = 0; i <= 39; i++) {
        // Skip pins that are not available
        if (i == 20 || i == 24 || (i >= 28 && i <= 31)) continue;
        // Skip input-only pins for general use
        if (i >= 34 && i <= 39) continue;
        all_pins.push_back(i);
    }
    
    std::vector<int8_t> used = getUsedPins(board);
    
    // Find pins not in the used list
    for (int8_t pin : all_pins) {
        if (std::find(used.begin(), used.end(), pin) == used.end()) {
            available_pins.push_back(pin);
        }
    }
    
    return available_pins;
}