/**
 * @file esp32_c3_cam.h
 * @brief ESP32-C3 ultra-low-power variant camera board support
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef ESP32_C3_CAM_H
#define ESP32_C3_CAM_H

#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <esp_sleep.h>

namespace Wildlife {
namespace Hardware {

/**
 * Board types supported by the system
 */
enum class BoardType {
    UNKNOWN = 0,
    AI_THINKER_CAM,
    ESP32_S3_CAM,
    ESP32_C3_CAM,      // New ESP32-C3 variant
    ESP32_C6_CAM,      // WiFi 6 variant
    ESP32_S2_CAM,      // Single-core variant
    TTGO_T_CAMERA,
    CUSTOM
};

/**
 * Power management modes
 */
enum class PowerMode {
    HIGH_PERFORMANCE,
    BALANCED,
    LOW_POWER,
    ULTRA_LOW_POWER
};

/**
 * Hardware features enumeration
 */
enum class Feature {
    CAMERA,
    WIFI,
    BLUETOOTH_LE,
    BLUETOOTH_CLASSIC,
    ETHERNET,
    LOW_POWER,
    DEEP_SLEEP,
    ADC,
    DAC,
    SPI,
    I2C,
    UART,
    PWM,
    CAN,
    TOUCH,
    HALL_SENSOR
};

/**
 * Pin configuration structure for ESP32-C3
 */
struct PinConfig {
    // Camera pins
    int8_t camera_d0 = -1;
    int8_t camera_d1 = -1;
    int8_t camera_d2 = -1;
    int8_t camera_d3 = -1;
    int8_t camera_d4 = -1;
    int8_t camera_d5 = -1;
    int8_t camera_d6 = -1;
    int8_t camera_d7 = -1;
    int8_t camera_xclk = -1;
    int8_t camera_pclk = -1;
    int8_t camera_vsync = -1;
    int8_t camera_href = -1;
    int8_t camera_sda = -1;
    int8_t camera_scl = -1;
    int8_t camera_pwdn = -1;
    int8_t camera_reset = -1;
    
    // Peripheral pins
    int8_t pir_pin = -1;
    int8_t led_pin = -1;
    int8_t battery_pin = -1;
    int8_t solar_pin = -1;
    
    // I2C pins
    int8_t i2c_sda = -1;
    int8_t i2c_scl = -1;
    
    // SD card pins
    int8_t sd_miso = -1;
    int8_t sd_mosi = -1;
    int8_t sd_clk = -1;
    int8_t sd_cs = -1;
};

/**
 * ESP32-C3 Camera Board Implementation
 * 
 * Optimized for ultra-low power wildlife monitoring applications.
 * Features single-core RISC-V processor with WiFi 4 and Bluetooth 5 LE.
 */
class ESP32C3CAM {
public:
    ESP32C3CAM();
    ~ESP32C3CAM();
    
    // Board initialization and configuration
    bool initialize();
    bool initializePower();
    bool initializeCamera();
    bool initializeConnectivity();
    
    // Board identification
    BoardType getBoardType() const;
    const char* getBoardName() const;
    bool hasFeature(Feature feature) const;
    
    // Camera operations
    bool captureImage(uint8_t** buffer, size_t* length);
    void releaseImageBuffer();
    bool deinitializeCamera();
    
    // Power management
    void enterDeepSleep(uint64_t sleep_time_us);
    void setCpuFrequency(uint32_t freq_mhz);
    float getBatteryVoltage();
    float getSolarVoltage();
    
    // Pin configuration
    const PinConfig& getPinConfig() const { return pin_config_; }
    
private:
    BoardType board_type_;
    PowerMode power_mode_;
    PinConfig pin_config_;
    bool camera_initialized_;
    bool sleep_enabled_;
    
    void initializePinConfig();
};

} // namespace Hardware
} // namespace Wildlife

#endif // ESP32_C3_CAM_H