/**
 * @file esp32_s3_cam_config.h
 * @brief ESP32-S3-CAM hardware configuration framework
 * @version 1.0.0
 * 
 * This file provides the foundational configuration framework for ESP32-S3-CAM
 * hardware support. It integrates with the existing comprehensive HAL system
 * in firmware/src/hal/ while providing simplified configuration management.
 */

#ifndef ESP32_S3_CAM_CONFIG_H
#define ESP32_S3_CAM_CONFIG_H

#include <stdint.h>

namespace ESP32S3CAM {

/**
 * @brief Camera sensor types supported by ESP32-S3-CAM
 */
enum class SensorType {
    UNKNOWN = 0,
    OV2640,         // 2MP camera sensor
    OV3660,         // 3MP camera sensor  
    OV5640,         // 5MP camera sensor
    GC032A,         // VGA camera sensor
    SC030IOT        // 0.3MP IoT camera sensor
};

/**
 * @brief Camera resolution settings
 */
enum class Resolution {
    QQVGA = 0,      // 160x120
    QCIF,           // 176x144
    HQVGA,          // 240x176
    QVGA,           // 320x240
    CIF,            // 400x296
    VGA,            // 640x480
    SVGA,           // 800x600
    XGA,            // 1024x768
    SXGA,           // 1280x1024
    UXGA            // 1600x1200
};

/**
 * @brief Camera frame formats
 */
enum class FrameFormat {
    JPEG = 0,
    RGB565,
    YUV422,
    GRAYSCALE,
    RGB888
};

/**
 * @brief Power management modes
 */
enum class PowerMode {
    ULTRA_LOW_POWER = 0,    // Minimal power consumption
    LOW_POWER,              // Balanced power saving
    NORMAL,                 // Standard operation
    HIGH_PERFORMANCE,       // Maximum performance
    ALWAYS_ON               // Continuous operation
};

/**
 * @brief GPIO pin configuration for ESP32-S3-CAM
 */
struct GPIOConfig {
    // Camera pins
    uint8_t xclk_pin = 10;      // External clock
    uint8_t siod_pin = 40;      // I2C SDA (SCCB)
    uint8_t sioc_pin = 39;      // I2C SCL (SCCB)
    uint8_t y9_pin = 48;        // Data bit 9
    uint8_t y8_pin = 11;        // Data bit 8
    uint8_t y7_pin = 12;        // Data bit 7
    uint8_t y6_pin = 14;        // Data bit 6
    uint8_t y5_pin = 16;        // Data bit 5
    uint8_t y4_pin = 18;        // Data bit 4
    uint8_t y3_pin = 17;        // Data bit 3
    uint8_t y2_pin = 15;        // Data bit 2
    uint8_t vsync_pin = 38;     // Vertical sync
    uint8_t href_pin = 47;      // Horizontal reference
    uint8_t pclk_pin = 13;      // Pixel clock
    uint8_t pwdn_pin = -1;      // Power down (not used on S3)
    uint8_t reset_pin = -1;     // Reset (not used on S3)
    
    // Flash/LED pins
    uint8_t flash_pin = 4;      // Camera flash LED
    uint8_t status_led_pin = 33; // Status LED
    
    // SD card pins (if used)
    uint8_t sd_miso_pin = 2;
    uint8_t sd_mosi_pin = 15;
    uint8_t sd_sclk_pin = 14;
    uint8_t sd_cs_pin = 21;
    
    // Additional peripheral pins
    uint8_t i2c_sda_pin = 1;    // General I2C SDA
    uint8_t i2c_scl_pin = 2;    // General I2C SCL
    uint8_t uart_tx_pin = 43;   // UART TX
    uint8_t uart_rx_pin = 44;   // UART RX
    
    // Power management pins
    uint8_t battery_adc_pin = 1;    // Battery voltage monitoring
    uint8_t charging_pin = 33;      // Charging status
    uint8_t power_enable_pin = 48;  // Power control
};

/**
 * @brief Camera sensor configuration
 */
struct CameraConfig {
    SensorType sensor_type = SensorType::OV2640;
    Resolution resolution = Resolution::VGA;
    FrameFormat format = FrameFormat::JPEG;
    uint8_t quality = 10;           // JPEG quality (0-63, lower = higher quality)
    uint8_t brightness = 0;         // -2 to 2
    int8_t contrast = 0;            // -2 to 2
    int8_t saturation = 0;          // -2 to 2
    bool auto_exposure = true;
    uint16_t exposure_value = 0;    // Manual exposure (if auto_exposure = false)
    bool auto_white_balance = true;
    uint8_t white_balance_mode = 0; // 0=Auto, 1=Sunny, 2=Cloudy, 3=Office, 4=Home
    bool auto_gain = true;
    uint8_t gain_value = 0;         // Manual gain (if auto_gain = false)
    bool horizontal_mirror = false;
    bool vertical_flip = false;
    uint8_t frame_rate = 10;        // Target frame rate (fps)
};

/**
 * @brief Power management configuration
 */
struct PowerConfig {
    PowerMode power_mode = PowerMode::NORMAL;
    bool enable_power_saving = true;
    uint32_t sleep_timeout_ms = 60000;      // Sleep after inactivity
    uint32_t deep_sleep_timeout_ms = 300000; // Deep sleep timeout
    bool enable_motion_wakeup = true;
    bool enable_timer_wakeup = true;
    uint32_t wakeup_interval_ms = 3600000;  // Hourly wakeup
    float battery_low_threshold = 3.3f;     // Low battery voltage
    float battery_critical_threshold = 3.0f; // Critical battery voltage
    bool enable_charging_detection = true;
    bool enable_solar_charging = false;
};

/**
 * @brief AI acceleration configuration
 */
struct AIConfig {
    bool enable_ai_acceleration = true;
    uint32_t ai_memory_allocation_kb = 512; // Memory for AI models
    bool enable_psram_for_ai = true;
    bool enable_dual_core_processing = true;
    uint8_t ai_processing_core = 1;         // 0 or 1
    uint32_t ai_stack_size_kb = 8;
    bool enable_tensorflow_lite = true;
    bool enable_edge_impulse = false;
    float ai_confidence_threshold = 0.6f;
    uint32_t ai_processing_timeout_ms = 2000;
};

/**
 * @brief Connectivity configuration
 */
struct ConnectivityConfig {
    bool enable_wifi = true;
    bool enable_bluetooth = false;
    bool enable_mesh_networking = false;
    bool enable_lora = false;
    char wifi_ssid[64] = "";
    char wifi_password[64] = "";
    bool wifi_sta_mode = true;              // Station mode
    bool wifi_ap_mode = false;              // Access Point mode
    char ap_ssid[32] = "WildlifeCam-S3";
    char ap_password[32] = "wildlife123";
    uint8_t wifi_channel = 1;
    int8_t wifi_tx_power = 20;              // dBm
    uint32_t connection_timeout_ms = 30000;
    bool enable_auto_reconnect = true;
};

/**
 * @brief Complete ESP32-S3-CAM configuration
 */
struct S3CAMConfig {
    GPIOConfig gpio;
    CameraConfig camera;
    PowerConfig power;
    AIConfig ai;
    ConnectivityConfig connectivity;
    
    // General settings
    char device_name[32] = "ESP32-S3-WildlifeCam";
    uint32_t device_id = 0;                 // 0 = auto-generate
    bool enable_debug_output = false;
    uint32_t debug_baud_rate = 115200;
    bool enable_watchdog = true;
    uint32_t watchdog_timeout_ms = 10000;
};

/**
 * @brief Hardware capability detection
 */
struct HardwareCapabilities {
    bool has_psram;
    uint32_t psram_size_mb;
    bool has_flash;
    uint32_t flash_size_mb;
    uint8_t cpu_cores;
    uint32_t cpu_frequency_mhz;
    bool has_wifi;
    bool has_bluetooth;
    SensorType detected_camera_sensor;
    bool has_sd_card_slot;
    float supply_voltage;
    int8_t chip_temperature;
};

/**
 * @brief ESP32-S3-CAM configuration manager
 */
class S3CAMConfigManager {
public:
    S3CAMConfigManager();
    ~S3CAMConfigManager();

    /**
     * @brief Initialize configuration manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Detect hardware capabilities
     * @return Hardware capabilities structure
     */
    HardwareCapabilities detectHardware();

    /**
     * @brief Load configuration from flash/EEPROM
     * @return true if configuration loaded successfully
     */
    bool loadConfiguration();

    /**
     * @brief Save configuration to flash/EEPROM
     * @return true if configuration saved successfully
     */
    bool saveConfiguration();

    /**
     * @brief Get current configuration
     * @return Current S3-CAM configuration
     */
    S3CAMConfig getConfiguration() const { return config_; }

    /**
     * @brief Update configuration
     * @param config New configuration
     * @return true if configuration is valid and applied
     */
    bool updateConfiguration(const S3CAMConfig& config);

    /**
     * @brief Reset configuration to defaults
     */
    void resetToDefaults();

    /**
     * @brief Validate configuration settings
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    bool validateConfiguration(const S3CAMConfig& config);

    /**
     * @brief Get optimal configuration for specific use case
     * @param use_case Use case identifier
     * @return Optimized configuration
     */
    S3CAMConfig getOptimalConfiguration(const char* use_case);

    /**
     * @brief Auto-configure based on detected hardware
     * @return true if auto-configuration successful
     */
    bool autoConfigureHardware();

    /**
     * @brief Check if configuration is initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    S3CAMConfig config_;
    HardwareCapabilities capabilities_;
    bool initialized_;

    // Internal methods
    bool detectCameraSensor();
    bool configurePins();
    bool validatePinConfiguration(const GPIOConfig& gpio_config);
    bool setupPowerManagement();
    bool initializeAIAcceleration();
    SensorType probeCameraSensor();
    uint32_t generateDeviceId();
    
    // Configuration presets
    S3CAMConfig getWildlifeMonitoringConfig();
    S3CAMConfig getSecurityCameraConfig();
    S3CAMConfig getLowPowerConfig();
    S3CAMConfig getHighPerformanceConfig();
};

/**
 * @brief Utility functions for ESP32-S3-CAM configuration
 */
namespace Utils {
    /**
     * @brief Convert sensor type to string
     */
    const char* sensorTypeToString(SensorType sensor);
    
    /**
     * @brief Convert resolution to string
     */
    const char* resolutionToString(Resolution resolution);
    
    /**
     * @brief Convert power mode to string
     */
    const char* powerModeToString(PowerMode mode);
    
    /**
     * @brief Get resolution dimensions
     */
    void getResolutionDimensions(Resolution resolution, uint16_t& width, uint16_t& height);
    
    /**
     * @brief Calculate estimated power consumption
     */
    uint32_t estimatePowerConsumption(const S3CAMConfig& config);
    
    /**
     * @brief Check GPIO pin compatibility
     */
    bool isGPIOCompatible(uint8_t pin, const char* function);
}

} // namespace ESP32S3CAM

#endif // ESP32_S3_CAM_CONFIG_H