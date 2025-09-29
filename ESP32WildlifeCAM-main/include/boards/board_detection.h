/**
 * @file board_detection.h
 * @brief Automatic board detection and configuration system
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 * 
 * Enhanced board detection system supporting all ESP32 variants
 * for Phase 3 platform diversification.
 */

#ifndef BOARD_DETECTION_H
#define BOARD_DETECTION_H

#include <Arduino.h>
#include <vector>
#include <map>

namespace Wildlife {
namespace Hardware {

/**
 * Extended board types for Phase 3 expansion
 */
enum class BoardType {
    UNKNOWN = 0,
    AI_THINKER_CAM,
    ESP32_S3_CAM,
    ESP32_C3_CAM,      // Ultra-low-power variant
    ESP32_C6_CAM,      // WiFi 6 and Thread support
    ESP32_S2_CAM,      // Single-core high-performance
    TTGO_T_CAMERA,
    CUSTOM
};

/**
 * Hardware capabilities and features
 */
enum class Feature {
    // Core features
    CAMERA,
    WIFI,
    BLUETOOTH_LE,
    BLUETOOTH_CLASSIC,
    
    // Advanced wireless
    WIFI_6,
    THREAD,
    ZIGBEE,
    
    // Power management
    LOW_POWER,
    DEEP_SLEEP,
    POWER_MANAGEMENT,
    
    // Peripherals
    ADC,
    DAC,
    SPI,
    I2C,
    UART,
    PWM,
    CAN,
    
    // Sensors
    TOUCH,
    HALL_SENSOR,
    TEMPERATURE_SENSOR,
    
    // Connectivity
    USB_OTG,
    ETHERNET,
    
    // Security
    SECURE_BOOT,
    FLASH_ENCRYPTION,
    
    // Performance
    HIGH_PERFORMANCE,
    DUAL_CORE,
    SINGLE_CORE,
    VECTOR_INSTRUCTIONS,
    
    // Storage
    PSRAM,
    FLASH_MEMORY,
    SD_CARD
};

/**
 * Board detection methods
 */
enum class DetectionMethod {
    CHIP_MODEL,        // ESP32 chip model detection
    PSRAM_SIZE,        // PSRAM availability and size
    GPIO_CONFIG,       // GPIO pin configuration testing
    I2C_SCAN,          // I2C device scanning
    FLASH_SIZE,        // Flash memory size
    USB_DETECTION,     // USB capabilities
    WIFI_CAPABILITIES, // WiFi feature detection
    EFUSE_READING      // eFuse register reading
};

/**
 * Pin configuration structure
 */
struct PinConfiguration {
    // Camera pins
    int8_t camera_pins[16];
    
    // Peripheral pins
    int8_t pir_pin = -1;
    int8_t led_pin = -1;
    int8_t battery_pin = -1;
    int8_t solar_pin = -1;
    
    // Communication pins
    int8_t i2c_sda = -1;
    int8_t i2c_scl = -1;
    int8_t spi_pins[4]; // MISO, MOSI, CLK, CS
    
    // USB pins (for S2/C6)
    int8_t usb_dm = -1;
    int8_t usb_dp = -1;
    
    // LoRa pins
    int8_t lora_pins[6]; // CS, RST, IRQ, MOSI, MISO, SCK
};

/**
 * Board information structure
 */
struct BoardInfo {
    BoardType type;
    const char* name;
    const char* chip_model;
    uint32_t flash_size;
    uint32_t psram_size;
    std::vector<Feature> features;
    PinConfiguration pins;
    bool validated;
};

/**
 * Enhanced Board Detection System
 * 
 * Automatically detects ESP32 board variants and configures
 * appropriate pin mappings and feature sets.
 */
class BoardDetection {
public:
    BoardDetection();
    ~BoardDetection() = default;
    
    // Main detection interface
    BoardInfo detectBoard();
    BoardInfo detectBoard(DetectionMethod method);
    
    // Validation functions
    bool validateBoard(const BoardInfo& board);
    bool validatePinConfiguration(const PinConfiguration& pins);
    bool validateFeatures(const std::vector<Feature>& features);
    
    // Board-specific detection
    BoardInfo detectESP32Original();
    BoardInfo detectESP32S2();
    BoardInfo detectESP32S3();
    BoardInfo detectESP32C3();
    BoardInfo detectESP32C6();
    
    // Capability detection
    std::vector<Feature> detectFeatures(BoardType type);
    PinConfiguration detectPinConfiguration(BoardType type);
    
    // Utility functions
    const char* getBoardName(BoardType type);
    const char* getFeatureName(Feature feature);
    bool boardHasFeature(BoardType type, Feature feature);
    
    // Configuration management
    void saveDetectedConfiguration(const BoardInfo& board);
    BoardInfo loadSavedConfiguration();
    void clearSavedConfiguration();
    
private:
    BoardInfo current_board_;
    std::map<BoardType, BoardInfo> board_database_;
    
    // Detection methods
    BoardType detectByChipModel();
    BoardType detectByPSRAM();
    BoardType detectByGPIOTest();
    BoardType detectByI2CScan();
    BoardType detectByUSBCapability();
    BoardType detectByWiFiCapability();
    
    // Hardware testing
    bool testGPIOPin(int pin, bool expected_state);
    bool scanI2CBus(int sda, int scl);
    std::vector<uint8_t> scanI2CDevices(int sda, int scl);
    bool testUSBOTG();
    bool testWiFi6Capability();
    
    // Pin configuration generators
    PinConfiguration generateESP32Pins();
    PinConfiguration generateESP32S2Pins();
    PinConfiguration generateESP32S3Pins();
    PinConfiguration generateESP32C3Pins();
    PinConfiguration generateESP32C6Pins();
    
    // Feature detection
    std::vector<Feature> generateESP32Features();
    std::vector<Feature> generateESP32S2Features();
    std::vector<Feature> generateESP32S3Features();
    std::vector<Feature> generateESP32C3Features();
    std::vector<Feature> generateESP32C6Features();
    
    // Board database initialization
    void initializeBoardDatabase();
    void addBoardToDatabase(const BoardInfo& board);
    
    // Validation helpers
    bool validateCameraPins(const PinConfiguration& pins);
    bool validatePowerPins(const PinConfiguration& pins);
    bool validateCommunicationPins(const PinConfiguration& pins);
    
    // Configuration persistence
    bool saveConfigurationToNVS(const BoardInfo& board);
    bool loadConfigurationFromNVS(BoardInfo& board);
};

} // namespace Hardware
} // namespace Wildlife

#endif // BOARD_DETECTION_H