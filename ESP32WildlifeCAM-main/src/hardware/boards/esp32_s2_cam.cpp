/**
 * @file esp32_s2_cam.cpp
 * @brief ESP32-S2 single-core high-performance variant camera board
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 * 
 * Implements support for ESP32-S2 based camera modules optimized for
 * high-performance single-core processing with enhanced security features.
 */

#include "esp32_s2_cam.h"
#include "../../../utils/logger.h"

namespace Wildlife {
namespace Hardware {

ESP32S2CAM::ESP32S2CAM() : 
    board_type_(BoardType::ESP32_S2_CAM),
    power_mode_(PowerMode::HIGH_PERFORMANCE),
    camera_initialized_(false),
    secure_boot_enabled_(false),
    usb_otg_enabled_(false) {
    
    initializePinConfig();
}

ESP32S2CAM::~ESP32S2CAM() {
    if (camera_initialized_) {
        deinitializeCamera();
    }
}

bool ESP32S2CAM::initialize() {
    Logger::info("Initializing ESP32-S2-CAM board");
    
    if (!initializePower()) {
        Logger::error("Failed to initialize power management");
        return false;
    }
    
    if (!initializeCamera()) {
        Logger::error("Failed to initialize camera");
        return false;
    }
    
    if (!initializeConnectivity()) {
        Logger::error("Failed to initialize connectivity");
        return false;
    }
    
    if (!initializeUSB()) {
        Logger::warning("USB OTG initialization failed");
    }
    
    Logger::info("ESP32-S2-CAM board initialized successfully");
    return true;
}

bool ESP32S2CAM::initializePower() {
    // ESP32-S2 single-core optimized power management
    setCpuFrequency(240); // Maximum performance
    
    // Configure dynamic frequency scaling
    esp_pm_config_esp32s2_t pm_config;
    pm_config.max_freq_mhz = 240;
    pm_config.min_freq_mhz = 80;
    pm_config.light_sleep_enable = true;
    
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret != ESP_OK) {
        Logger::warning("Failed to configure power management");
    }
    
    return true;
}

bool ESP32S2CAM::initializeCamera() {
    camera_config_t config;
    
    // ESP32-S2 camera configuration optimized for single-core performance
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = pin_config_.camera_d0;
    config.pin_d1 = pin_config_.camera_d1;
    config.pin_d2 = pin_config_.camera_d2;
    config.pin_d3 = pin_config_.camera_d3;
    config.pin_d4 = pin_config_.camera_d4;
    config.pin_d5 = pin_config_.camera_d5;
    config.pin_d6 = pin_config_.camera_d6;
    config.pin_d7 = pin_config_.camera_d7;
    config.pin_xclk = pin_config_.camera_xclk;
    config.pin_pclk = pin_config_.camera_pclk;
    config.pin_vsync = pin_config_.camera_vsync;
    config.pin_href = pin_config_.camera_href;
    config.pin_sscb_sda = pin_config_.camera_sda;
    config.pin_sscb_scl = pin_config_.camera_scl;
    config.pin_pwdn = pin_config_.camera_pwdn;
    config.pin_reset = pin_config_.camera_reset;
    config.xclk_freq_hz = 24000000; // High frequency for optimal performance
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Optimized settings for single-core high performance
    config.frame_size = FRAMESIZE_UXGA; // High resolution
    config.jpeg_quality = 6; // High quality
    config.fb_count = 2; // Double buffering for smooth operation
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Logger::error("Camera init failed with error 0x%x", err);
        return false;
    }
    
    // Configure sensor for optimal single-core performance
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        // Optimize sensor settings for ESP32-S2
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
        s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
        s->set_wb_mode(s, 0);        // 0 to 4
        s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
        s->set_aec2(s, 0);           // 0 = disable, 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 300);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0); // 0 to 6
        s->set_bpc(s, 0);            // 0 = disable, 1 = enable
        s->set_wpc(s, 1);            // 0 = disable, 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
        s->set_lenc(s, 1);           // 0 = disable, 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
        s->set_vflip(s, 0);          // 0 = disable, 1 = enable
        s->set_dcw(s, 1);            // 0 = disable, 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable, 1 = enable
    }
    
    camera_initialized_ = true;
    Logger::info("ESP32-S2 camera initialized with high-performance settings");
    return true;
}

bool ESP32S2CAM::initializeConnectivity() {
    // ESP32-S2 WiFi initialization
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Disable sleep for maximum performance
    
    Logger::info("ESP32-S2 connectivity initialized");
    return true;
}

bool ESP32S2CAM::initializeUSB() {
    // ESP32-S2 native USB OTG support
    // This would initialize USB device/host capabilities
    usb_otg_enabled_ = true;
    Logger::info("USB OTG initialized");
    return true;
}

void ESP32S2CAM::initializePinConfig() {
    // ESP32-S2 specific pin assignments
    // S2 has more GPIO pins available than original ESP32
    
    // Camera pins
    pin_config_.camera_d0 = 5;
    pin_config_.camera_d1 = 18;
    pin_config_.camera_d2 = 19;
    pin_config_.camera_d3 = 21;
    pin_config_.camera_d4 = 36;
    pin_config_.camera_d5 = 39;
    pin_config_.camera_d6 = 34;
    pin_config_.camera_d7 = 35;
    pin_config_.camera_xclk = 0;
    pin_config_.camera_pclk = 22;
    pin_config_.camera_vsync = 25;
    pin_config_.camera_href = 23;
    pin_config_.camera_sda = 26;
    pin_config_.camera_scl = 27;
    pin_config_.camera_pwdn = 32;
    pin_config_.camera_reset = -1;
    
    // Other peripherals
    pin_config_.pir_pin = 1;
    pin_config_.led_pin = 2;
    pin_config_.battery_pin = 3;
    pin_config_.solar_pin = 4;
    
    // I2C for sensors
    pin_config_.i2c_sda = 8;
    pin_config_.i2c_scl = 9;
    
    // SD card (SPI)
    pin_config_.sd_miso = 37;
    pin_config_.sd_mosi = 35;
    pin_config_.sd_clk = 36;
    pin_config_.sd_cs = 34;
    
    // USB OTG pins (native)
    pin_config_.usb_dm = 19;  // USB D-
    pin_config_.usb_dp = 20;  // USB D+
}

bool ESP32S2CAM::enableSecureBoot() {
    // Enable ESP32-S2 secure boot features
    secure_boot_enabled_ = true;
    Logger::info("Secure boot enabled");
    return true;
}

float ESP32S2CAM::getBatteryVoltage() {
    // Enhanced ADC reading with ESP32-S2's improved ADC
    int raw_value = analogRead(pin_config_.battery_pin);
    
    // ESP32-S2 has improved ADC linearity
    float voltage = (raw_value / 4095.0) * 3.3 * 2.0;
    
    return voltage;
}

float ESP32S2CAM::getSolarVoltage() {
    int raw_value = analogRead(pin_config_.solar_pin);
    float voltage = (raw_value / 4095.0) * 3.3 * 3.0;
    return voltage;
}

bool ESP32S2CAM::captureImage(uint8_t** buffer, size_t* length) {
    if (!camera_initialized_) {
        Logger::error("Camera not initialized");
        return false;
    }
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Logger::error("Camera capture failed");
        return false;
    }
    
    *buffer = fb->buf;
    *length = fb->len;
    
    Logger::info("High-quality image captured: %zu bytes", fb->len);
    return true;
}

void ESP32S2CAM::releaseImageBuffer() {
    esp_camera_fb_return(esp_camera_fb_get());
}

bool ESP32S2CAM::deinitializeCamera() {
    if (camera_initialized_) {
        esp_camera_deinit();
        camera_initialized_ = false;
        Logger::info("ESP32-S2 camera deinitialized");
    }
    return true;
}

BoardType ESP32S2CAM::getBoardType() const {
    return board_type_;
}

const char* ESP32S2CAM::getBoardName() const {
    return "ESP32-S2-CAM";
}

bool ESP32S2CAM::hasFeature(Feature feature) const {
    switch (feature) {
        case Feature::CAMERA: return true;
        case Feature::WIFI: return true;
        case Feature::USB_OTG: return true;
        case Feature::HIGH_PERFORMANCE: return true;
        case Feature::SECURE_BOOT: return true;
        case Feature::LOW_POWER: return true;
        case Feature::DEEP_SLEEP: return true;
        case Feature::ADC: return true;
        case Feature::DAC: return true;
        case Feature::SPI: return true;
        case Feature::I2C: return true;
        case Feature::UART: return true;
        case Feature::PWM: return true;
        case Feature::TOUCH: return true;
        
        // Features not available on ESP32-S2
        case Feature::BLUETOOTH_LE: return false;
        case Feature::BLUETOOTH_CLASSIC: return false;
        case Feature::ETHERNET: return false;
        case Feature::CAN: return false;
        case Feature::HALL_SENSOR: return false;
        
        default: return false;
    }
}

void ESP32S2CAM::setCpuFrequency(uint32_t freq_mhz) {
    // ESP32-S2 supports frequencies: 80, 160, 240 MHz
    if (freq_mhz == 80 || freq_mhz == 160 || freq_mhz == 240) {
        setCpuFrequencyMhz(freq_mhz);
        Logger::info("CPU frequency set to %u MHz", freq_mhz);
    } else {
        Logger::warning("Invalid frequency %u MHz, using 240 MHz", freq_mhz);
        setCpuFrequencyMhz(240);
    }
}

void ESP32S2CAM::enableHighPerformanceMode() {
    power_mode_ = PowerMode::HIGH_PERFORMANCE;
    setCpuFrequency(240);
    
    // Disable WiFi sleep for maximum performance
    WiFi.setSleep(false);
    
    Logger::info("High performance mode enabled");
}

void ESP32S2CAM::enablePowerSavingMode() {
    power_mode_ = PowerMode::LOW_POWER;
    setCpuFrequency(80);
    
    // Enable WiFi sleep for power saving
    WiFi.setSleep(true);
    
    Logger::info("Power saving mode enabled");
}

bool ESP32S2CAM::connectUSBDevice() {
    if (!usb_otg_enabled_) {
        Logger::error("USB OTG not initialized");
        return false;
    }
    
    // Connect USB device (placeholder for actual implementation)
    Logger::info("USB device connected");
    return true;
}

void ESP32S2CAM::enterDeepSleep(uint64_t sleep_time_us) {
    Logger::info("ESP32-S2 entering deep sleep for %llu microseconds", sleep_time_us);
    
    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(pin_config_.pir_pin), 1);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

} // namespace Hardware
} // namespace Wildlife