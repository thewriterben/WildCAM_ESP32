/**
 * @file esp32_s2_cam.h
 * @brief ESP32-S2 single-core high-performance variant camera board
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef ESP32_S2_CAM_H
#define ESP32_S2_CAM_H

#include "esp32_c3_cam.h" // Inherit common definitions
#include <esp_pm.h>

namespace Wildlife {
namespace Hardware {

/**
 * ESP32-S2 specific pin configuration
 */
struct ESP32S2PinConfig : public PinConfig {
    // USB OTG pins (native support)
    int8_t usb_dm = -1;  // USB D-
    int8_t usb_dp = -1;  // USB D+
};

/**
 * ESP32-S2 specific features
 */
enum class ESP32S2Feature {
    USB_OTG = 200,        // Native USB OTG support
    SECURE_BOOT,          // Hardware security features
    SINGLE_CORE_PERF,     // Optimized single-core performance
    TOUCH_SENSOR,         // Capacitive touch sensing
    DAC_OUTPUT            // Digital-to-analog converter
};

/**
 * ESP32-S2 Camera Board Implementation
 * 
 * Single-core high-performance processor with enhanced security features
 * and native USB OTG support for advanced connectivity options.
 */
class ESP32S2CAM {
public:
    ESP32S2CAM();
    ~ESP32S2CAM();
    
    // Board initialization and configuration
    bool initialize();
    bool initializePower();
    bool initializeCamera();
    bool initializeConnectivity();
    bool initializeUSB();
    
    // Board identification
    BoardType getBoardType() const;
    const char* getBoardName() const;
    bool hasFeature(Feature feature) const;
    
    // Camera operations
    bool captureImage(uint8_t** buffer, size_t* length);
    void releaseImageBuffer();
    bool deinitializeCamera();
    
    // Power management
    void setCpuFrequency(uint32_t freq_mhz);
    void enableHighPerformanceMode();
    void enablePowerSavingMode();
    void enterDeepSleep(uint64_t sleep_time_us);
    
    // Enhanced sensor readings
    float getBatteryVoltage();
    float getSolarVoltage();
    
    // Security features
    bool enableSecureBoot();
    
    // USB OTG operations
    bool connectUSBDevice();
    
    // Pin configuration
    const ESP32S2PinConfig& getPinConfig() const { return pin_config_; }
    
private:
    BoardType board_type_;
    PowerMode power_mode_;
    ESP32S2PinConfig pin_config_;
    bool camera_initialized_;
    bool secure_boot_enabled_;
    bool usb_otg_enabled_;
    
    void initializePinConfig();
};

} // namespace Hardware
} // namespace Wildlife

#endif // ESP32_S2_CAM_H