/**
 * @file i2c_integration.cpp
 * @brief Integration implementation for ESP-IDF I2C system
 */

#include "i2c_integration.h"
#include "../debug_utils.h"
#include <esp_log.h>

static const char* TAG = "I2C_INTEGRATION";

// Global state
static bool i2c_system_initialized = false;
static uint32_t bme280_device_handle = 0;
static BoardType current_board = BOARD_UNKNOWN;

esp_err_t i2c_system_init(BoardType board_type) {
    if (i2c_system_initialized) {
        ESP_LOGW(TAG, "I2C system already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Initializing I2C system for board type %d", board_type);
    current_board = board_type;
    
    // Initialize I2C manager
    esp_err_t ret = i2c_manager_init(board_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C manager: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Wait for I2C to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Scan for devices
    int devices_found = i2c_scan_and_report_devices();
    ESP_LOGI(TAG, "Found %d I2C devices", devices_found);
    
    i2c_system_initialized = true;
    ESP_LOGI(TAG, "I2C system initialized successfully");
    
    return ESP_OK;
}

esp_err_t i2c_system_deinit(void) {
    if (!i2c_system_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing I2C system");
    
    // Deinitialize BME280 if it was initialized
    if (bme280_device_handle != 0) {
        bme280_deinit(bme280_device_handle);
        bme280_device_handle = 0;
    }
    
    // Deinitialize I2C manager
    esp_err_t ret = i2c_manager_deinit();
    
    i2c_system_initialized = false;
    current_board = BOARD_UNKNOWN;
    
    return ret;
}

esp_err_t i2c_init_bme280(uint32_t* bme280_handle) {
    if (!i2c_system_initialized) {
        ESP_LOGE(TAG, "I2C system not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!bme280_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Attempting to initialize BME280 sensor");
    
    // Try primary I2C address first
    bme280_config_t config = bme280_create_default_config(I2C_MASTER_NUM, BME280_I2C_ADDR_PRIMARY);
    
    esp_err_t ret = bme280_init(&config, bme280_handle);
    if (ret != ESP_OK) {
        // Try secondary address
        ESP_LOGW(TAG, "BME280 not found at primary address, trying secondary");
        config.i2c_config.device_addr = BME280_I2C_ADDR_SECONDARY;
        ret = bme280_init(&config, bme280_handle);
    }
    
    if (ret == ESP_OK) {
        bme280_device_handle = *bme280_handle;
        ESP_LOGI(TAG, "BME280 initialized successfully at address 0x%02X", 
                 config.i2c_config.device_addr);
    } else {
        ESP_LOGE(TAG, "Failed to initialize BME280: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

bool i2c_system_is_available(void) {
    return i2c_system_initialized && i2c_manager_is_ready();
}

esp_err_t i2c_get_environmental_reading(bme280_reading_t* reading) {
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (bme280_device_handle == 0) {
        ESP_LOGE(TAG, "BME280 not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Trigger measurement
    esp_err_t ret = bme280_trigger_measurement(bme280_device_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Wait for measurement to complete
    bool ready = false;
    int attempts = 0;
    while (!ready && attempts < 10) {
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = bme280_is_measurement_ready(bme280_device_handle, &ready);
        if (ret != ESP_OK) {
            return ret;
        }
        attempts++;
    }
    
    if (!ready) {
        ESP_LOGE(TAG, "BME280 measurement timeout");
        return ESP_ERR_TIMEOUT;
    }
    
    // Read the results
    return bme280_read_all(bme280_device_handle, reading);
}

int i2c_scan_and_report_devices(void) {
    if (!i2c_system_initialized) {
        return 0;
    }
    
    ESP_LOGI(TAG, "Scanning I2C devices...");
    
    uint8_t devices[127];
    size_t num_found = 0;
    
    esp_err_t ret = i2c_manager_scan_devices(I2C_MASTER_NUM, devices, sizeof(devices), &num_found);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C scan failed: %s", esp_err_to_name(ret));
        return 0;
    }
    
    if (num_found == 0) {
        ESP_LOGI(TAG, "No I2C devices found");
        return 0;
    }
    
    ESP_LOGI(TAG, "Found %d I2C device(s):", num_found);
    for (size_t i = 0; i < num_found; i++) {
        const char* device_name = "Unknown";
        
        // Identify known devices
        switch (devices[i]) {
            case BME280_I2C_ADDR_PRIMARY:
            case BME280_I2C_ADDR_SECONDARY:
                device_name = "BME280 Environmental Sensor";
                break;
            case SSD1306_I2C_ADDR:
            case SSD1306_I2C_ADDR_ALT:
                device_name = "SSD1306 OLED Display";
                break;
            case RTC_DS3231_I2C_ADDR:
                device_name = "DS3231 RTC";
                break;
            case RTC_PCF8563_I2C_ADDR:
                device_name = "PCF8563 RTC";
                break;
            case 0x21:
                device_name = "Camera Sensor (OV2640)";
                break;
            case 0x68:
                device_name = "MPU6050 Accelerometer or DS3231 RTC";
                break;
        }
        
        ESP_LOGI(TAG, "  0x%02X: %s", devices[i], device_name);
    }
    
    return (int)num_found;
}