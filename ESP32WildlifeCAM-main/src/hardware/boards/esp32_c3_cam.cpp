/**
 * @file esp32_c3_cam.cpp
 * @brief ESP32-C3 ultra-low-power variant camera board support
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 * 
 * Implements support for ESP32-C3 based camera modules optimized for
 * wildlife monitoring with ultra-low power consumption.
 */

#include "esp32_c3_cam.h"
#include "../../../utils/logger.h"

namespace Wildlife {
namespace Hardware {

ESP32C3CAM::ESP32C3CAM() : 
    board_type_(BoardType::ESP32_C3_CAM),
    power_mode_(PowerMode::ULTRA_LOW_POWER),
    camera_initialized_(false),
    sleep_enabled_(true) {
    
    // Initialize ESP32-C3 specific pin configuration
    initializePinConfig();
}

ESP32C3CAM::~ESP32C3CAM() {
    if (camera_initialized_) {
        deinitializeCamera();
    }
}

bool ESP32C3CAM::initialize() {
    Logger::info("Initializing ESP32-C3-CAM board");
    
    // ESP32-C3 specific initialization
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
    
    Logger::info("ESP32-C3-CAM board initialized successfully");
    return true;
}

bool ESP32C3CAM::initializePower() {
    // ESP32-C3 ultra-low power configuration
    // Single-core RISC-V processor at 160MHz max
    setCpuFrequency(80); // Reduce frequency for power savings
    
    // Configure deep sleep capabilities
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(pin_config_.pir_pin), 1);
    
    return true;
}

bool ESP32C3CAM::initializeCamera() {
    camera_config_t config;
    
    // ESP32-C3 camera pin configuration
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
    config.xclk_freq_hz = 10000000; // Lower frequency for power savings
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Optimized settings for wildlife monitoring
    config.frame_size = FRAMESIZE_HD;
    config.jpeg_quality = 12;
    config.fb_count = 1; // Single frame buffer for C3's limited RAM
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Logger::error("Camera init failed with error 0x%x", err);
        return false;
    }
    
    camera_initialized_ = true;
    Logger::info("ESP32-C3 camera initialized");
    return true;
}

bool ESP32C3CAM::initializeConnectivity() {
    // ESP32-C3 supports WiFi 4 and Bluetooth 5 LE
    // Configure for low-power wireless operation
    
    // Initialize WiFi in station mode with power saving
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true); // Enable power saving mode
    
    Logger::info("ESP32-C3 connectivity initialized");
    return true;
}

void ESP32C3CAM::initializePinConfig() {
    // ESP32-C3 specific pin assignments
    // C3 has fewer GPIO pins, so careful mapping required
    
    // Camera pins (using available GPIO)
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
    pin_config_.camera_reset = -1; // Not used
    
    // Other peripherals
    pin_config_.pir_pin = 1;
    pin_config_.led_pin = 2;
    pin_config_.battery_pin = 3;
    pin_config_.solar_pin = 4;
    
    // I2C for sensors
    pin_config_.i2c_sda = 8;
    pin_config_.i2c_scl = 9;
    
    // SD card (SPI)
    pin_config_.sd_miso = 6;
    pin_config_.sd_mosi = 7;
    pin_config_.sd_clk = 10;
    pin_config_.sd_cs = 20;
}

void ESP32C3CAM::enterDeepSleep(uint64_t sleep_time_us) {
    if (!sleep_enabled_) {
        return;
    }
    
    Logger::info("ESP32-C3 entering deep sleep for %llu microseconds", sleep_time_us);
    
    // Configure wake-up sources
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(pin_config_.pir_pin), 1);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

float ESP32C3CAM::getBatteryVoltage() {
    // Read battery voltage using ESP32-C3 ADC
    int raw_value = analogRead(pin_config_.battery_pin);
    float voltage = (raw_value / 4095.0) * 3.3 * 2.0; // Voltage divider
    return voltage;
}

float ESP32C3CAM::getSolarVoltage() {
    // Read solar panel voltage
    int raw_value = analogRead(pin_config_.solar_pin);
    float voltage = (raw_value / 4095.0) * 3.3 * 3.0; // Voltage divider for higher voltage
    return voltage;
}

bool ESP32C3CAM::captureImage(uint8_t** buffer, size_t* length) {
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
    
    Logger::info("Image captured: %zu bytes", fb->len);
    return true;
}

void ESP32C3CAM::releaseImageBuffer() {
    esp_camera_fb_return(esp_camera_fb_get());
}

bool ESP32C3CAM::deinitializeCamera() {
    if (camera_initialized_) {
        esp_camera_deinit();
        camera_initialized_ = false;
        Logger::info("ESP32-C3 camera deinitialized");
    }
    return true;
}

BoardType ESP32C3CAM::getBoardType() const {
    return board_type_;
}

const char* ESP32C3CAM::getBoardName() const {
    return "ESP32-C3-CAM";
}

bool ESP32C3CAM::hasFeature(Feature feature) const {
    switch (feature) {
        case Feature::CAMERA: return true;
        case Feature::WIFI: return true;
        case Feature::BLUETOOTH_LE: return true;
        case Feature::LOW_POWER: return true;
        case Feature::DEEP_SLEEP: return true;
        case Feature::ADC: return true;
        case Feature::SPI: return true;
        case Feature::I2C: return true;
        case Feature::UART: return true;
        case Feature::PWM: return true;
        
        // Features not available on ESP32-C3
        case Feature::BLUETOOTH_CLASSIC: return false;
        case Feature::ETHERNET: return false;
        case Feature::CAN: return false;
        case Feature::DAC: return false;
        case Feature::TOUCH: return false;
        case Feature::HALL_SENSOR: return false;
        
        default: return false;
    }
}

void ESP32C3CAM::setCpuFrequency(uint32_t freq_mhz) {
    // ESP32-C3 frequency scaling for power management
    // Valid frequencies: 80MHz, 160MHz
    if (freq_mhz == 80 || freq_mhz == 160) {
        setCpuFrequencyMhz(freq_mhz);
        Logger::info("CPU frequency set to %u MHz", freq_mhz);
    } else {
        Logger::warning("Invalid frequency %u MHz, using 80 MHz", freq_mhz);
        setCpuFrequencyMhz(80);
    }
}

} // namespace Hardware
} // namespace Wildlife