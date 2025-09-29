/**
 * @file esp32_c6_cam.h
 * @brief ESP32-C6 with WiFi 6 and Thread support camera board
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef ESP32_C6_CAM_H
#define ESP32_C6_CAM_H

#include "esp32_c3_cam.h" // Inherit common definitions
#include <esp_pm.h>

namespace Wildlife {
namespace Hardware {

/**
 * Extended features for ESP32-C6
 */
enum class ExtendedFeature {
    WIFI_6 = 100,         // 802.11ax support
    THREAD,               // Thread mesh networking
    HIGH_PERFORMANCE,     // Enhanced processing capability
    ADVANCED_ADC,         // Improved ADC with calibration
    MESH_NETWORKING       // Advanced mesh capabilities
};

/**
 * ESP32-C6 Camera Board Implementation
 * 
 * Advanced wireless connectivity with WiFi 6 and Thread networking.
 * Optimized for high-performance wildlife monitoring with mesh capabilities.
 */
class ESP32C6CAM {
public:
    ESP32C6CAM();
    ~ESP32C6CAM();
    
    // Board initialization and configuration
    bool initialize();
    bool initializePower();
    bool initializeCamera();
    bool initializeConnectivity();
    
    // Advanced connectivity initialization
    bool initializeWiFi6();
    bool initializeThread();
    bool initializeBluetoothLE();
    
    // Board identification
    BoardType getBoardType() const;
    const char* getBoardName() const;
    bool hasFeature(Feature feature) const;
    
    // Camera operations
    bool captureImage(uint8_t** buffer, size_t* length);
    void releaseImageBuffer();
    bool deinitializeCamera();
    
    // Network operations
    bool connectWiFi6(const char* ssid, const char* password);
    bool joinThreadNetwork(const char* network_key);
    
    // Power management
    void setCpuFrequency(uint32_t freq_mhz);
    void enableHighPerformanceMode();
    void enablePowerSavingMode();
    
    // Enhanced sensor readings
    float getBatteryVoltage();
    float getSolarVoltage();
    float calibrateVoltage(float raw_voltage);
    
    // Pin configuration
    const PinConfig& getPinConfig() const { return pin_config_; }
    
private:
    BoardType board_type_;
    PowerMode power_mode_;
    PinConfig pin_config_;
    bool camera_initialized_;
    bool wifi6_enabled_;
    bool thread_enabled_;
    
    void initializePinConfig();
};

} // namespace Hardware
} // namespace Wildlife

#endif // ESP32_C6_CAM_H