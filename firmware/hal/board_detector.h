/**
 * @file board_detector.h
 * @brief Hardware board detection and capability management
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef BOARD_DETECTOR_H
#define BOARD_DETECTOR_H

#include <Arduino.h>
#include <map>
#include <vector>

class BoardDetector {
public:
    enum BoardType {
        UNKNOWN = 0,
        AI_THINKER_CAM,
        ESP32_S3_CAM,
        TTGO_T_CAMERA,
        CUSTOM
    };
    
    enum Feature {
        FEATURE_CAMERA,
        FEATURE_SD_CARD,
        FEATURE_PSRAM,
        FEATURE_LORA,
        FEATURE_WIFI,
        FEATURE_BLUETOOTH,
        FEATURE_DISPLAY,
        FEATURE_USB,
        FEATURE_BATTERY_MONITOR
    };
    
    struct PinConfig {
        // Camera pins
        int8_t cam_pwdn = -1;
        int8_t cam_reset = -1;
        int8_t cam_xclk = -1;
        int8_t cam_siod = -1;
        int8_t cam_sioc = -1;
        int8_t cam_y9 = -1;
        int8_t cam_y8 = -1;
        int8_t cam_y7 = -1;
        int8_t cam_y6 = -1;
        int8_t cam_y5 = -1;
        int8_t cam_y4 = -1;
        int8_t cam_y3 = -1;
        int8_t cam_y2 = -1;
        int8_t cam_vsync = -1;
        int8_t cam_href = -1;
        int8_t cam_pclk = -1;
        
        // SD card pins
        int8_t sd_cs = -1;
        int8_t sd_mosi = -1;
        int8_t sd_miso = -1;
        int8_t sd_clk = -1;
        
        // LoRa pins
        int8_t lora_cs = -1;
        int8_t lora_rst = -1;
        int8_t lora_irq = -1;
        int8_t lora_mosi = -1;
        int8_t lora_miso = -1;
        int8_t lora_sck = -1;
        
        // Other pins
        int8_t pir_pin = -1;
        int8_t led_pin = -1;
        int8_t battery_pin = -1;
        int8_t solar_pin = -1;
        int8_t i2c_sda = -1;
        int8_t i2c_scl = -1;
    };
    
    static BoardType detectBoard();
    static const char* getBoardName(BoardType board);
    static bool hasFeature(BoardType board, Feature feature);
    static bool hasFeature(Feature feature); // Uses detected board
    static PinConfig getPinConfig(BoardType board);
    static PinConfig getPinConfig(); // Uses detected board
    static std::vector<int8_t> getUsedPins(BoardType board);
    static std::vector<int8_t> getAvailablePins(BoardType board);
    
private:
    static BoardType s_detectedBoard;
    static std::map<BoardType, PinConfig> s_boardConfigs;
    static std::map<BoardType, std::vector<Feature>> s_boardFeatures;
    
    static void initializeBoardConfigs();
    static BoardType detectByChipModel();
    static BoardType detectByPSRAM();
    static BoardType detectByGPIO();
    static BoardType detectByI2CScan();
    
    // Hardware validation functions
    static bool testGPIOPin(int pin);
    static bool scanI2CBus(int sda, int scl);
    static std::vector<uint8_t> scanI2CDevices(int sda, int scl);
    static bool validateCameraPins(const PinConfig& config);
};

#endif // BOARD_DETECTOR_H