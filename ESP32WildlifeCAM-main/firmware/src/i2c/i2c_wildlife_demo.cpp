/**
 * @file i2c_wildlife_demo.cpp
 * @brief Demonstration of ESP-IDF I2C integration with wildlife monitoring
 */

#include "i2c_integration.h"
#include "../config.h"
#include "../debug_utils.h"
#include "../hal/board_detector.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "I2C_WILDLIFE_DEMO";

// Global handles
static uint32_t bme280_handle = 0;
static bool environmental_monitoring_active = false;

void i2c_wildlife_demo_init(void) {
    ESP_LOGI(TAG, "Starting ESP-IDF I2C Wildlife Monitoring Demo");
    
#if ESP_IDF_I2C_ENABLED
    // Detect board type
    BoardType board_type = BoardDetector::detectBoardType();
    ESP_LOGI(TAG, "Detected board type: %d", board_type);
    
    // Initialize I2C system with board-specific configuration
    esp_err_t ret = i2c_system_init(board_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C system: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "I2C system initialized successfully");
    
#if BME280_ENABLED
    // Try to initialize BME280
    ret = i2c_init_bme280(&bme280_handle);
    if (ret == ESP_OK) {
        environmental_monitoring_active = true;
        ESP_LOGI(TAG, "BME280 environmental sensor activated!");
        ESP_LOGI(TAG, "Wildlife monitoring now includes temperature, humidity, and pressure");
    } else {
        ESP_LOGW(TAG, "BME280 not found - continuing without environmental monitoring");
    }
#endif

    ESP_LOGI(TAG, "I2C wildlife monitoring system ready");
    
#else
    ESP_LOGW(TAG, "ESP-IDF I2C system disabled in config.h");
#endif
}

void i2c_wildlife_demo_task(void* parameter) {
    ESP_LOGI(TAG, "I2C Wildlife Demo Task started");
    
    while (true) {
        if (environmental_monitoring_active && bme280_handle != 0) {
            bme280_reading_t reading;
            esp_err_t ret = i2c_get_environmental_reading(&reading);
            
            if (ret == ESP_OK && reading.valid) {
                ESP_LOGI(TAG, "Environmental conditions:");
                ESP_LOGI(TAG, "  Temperature: %.2f°C", reading.temperature);
                ESP_LOGI(TAG, "  Humidity: %.1f%%", reading.humidity);
                ESP_LOGI(TAG, "  Pressure: %.2f hPa", reading.pressure);
                
                // Calculate altitude (assuming sea level = 1013.25 hPa)
                float altitude = bme280_calculate_altitude(reading.pressure, 1013.25f);
                ESP_LOGI(TAG, "  Estimated altitude: %.1f m", altitude);
                
                // Environmental analysis for wildlife monitoring
                if (reading.temperature < 0.0f) {
                    ESP_LOGI(TAG, "Wildlife Alert: Freezing conditions - reduced animal activity expected");
                } else if (reading.temperature > 35.0f) {
                    ESP_LOGI(TAG, "Wildlife Alert: High temperature - animals may seek shade");
                }
                
                if (reading.humidity > 90.0f) {
                    ESP_LOGI(TAG, "Wildlife Alert: High humidity - fog possible, camera visibility may be reduced");
                } else if (reading.humidity < 20.0f) {
                    ESP_LOGI(TAG, "Wildlife Alert: Low humidity - dry conditions, increased fire risk");
                }
                
                // Pressure analysis for weather prediction
                static float last_pressure = 0.0f;
                if (last_pressure != 0.0f) {
                    float pressure_change = reading.pressure - last_pressure;
                    if (pressure_change < -3.0f) {
                        ESP_LOGI(TAG, "Weather Alert: Pressure dropping rapidly - storm approaching");
                    } else if (pressure_change > 3.0f) {
                        ESP_LOGI(TAG, "Weather Alert: Pressure rising rapidly - clearing weather");
                    }
                }
                last_pressure = reading.pressure;
                
            } else {
                ESP_LOGW(TAG, "Failed to read environmental data: %s", esp_err_to_name(ret));
            }
        }
        
        // Check I2C system statistics
        if (i2c_system_is_available()) {
            i2c_manager_stats_t stats;
            if (i2c_manager_get_stats(&stats) == ESP_OK) {
                ESP_LOGD(TAG, "I2C Stats: %lu total, %lu successful, %lu failed transactions",
                        stats.total_transactions, stats.successful_transactions, stats.failed_transactions);
            }
        }
        
        // Wait before next reading (configurable interval)
        vTaskDelay(pdMS_TO_TICKS(BME280_READING_INTERVAL));
    }
}

void i2c_wildlife_demo_start_task(void) {
    xTaskCreate(
        i2c_wildlife_demo_task,
        "i2c_wildlife_demo",
        4096, // Stack size
        NULL,
        5,    // Priority
        NULL
    );
}

void i2c_wildlife_demo_get_environmental_status(char* status_buffer, size_t buffer_size) {
    if (!status_buffer || buffer_size == 0) {
        return;
    }
    
    if (!environmental_monitoring_active || bme280_handle == 0) {
        snprintf(status_buffer, buffer_size, "Environmental monitoring: DISABLED");
        return;
    }
    
    bme280_reading_t reading;
    esp_err_t ret = i2c_get_environmental_reading(&reading);
    
    if (ret == ESP_OK && reading.valid) {
        snprintf(status_buffer, buffer_size, 
                "Env: %.1f°C, %.1f%%, %.1fhPa", 
                reading.temperature, reading.humidity, reading.pressure);
    } else {
        snprintf(status_buffer, buffer_size, "Environmental monitoring: ERROR");
    }
}

bool i2c_wildlife_demo_is_environmental_ok_for_capture(void) {
    if (!environmental_monitoring_active || bme280_handle == 0) {
        return true; // If no environmental monitoring, always allow capture
    }
    
    bme280_reading_t reading;
    esp_err_t ret = i2c_get_environmental_reading(&reading);
    
    if (ret != ESP_OK || !reading.valid) {
        return true; // If can't read sensors, allow capture
    }
    
    // Define environmental thresholds for optimal wildlife photography
    bool temp_ok = (reading.temperature > -20.0f && reading.temperature < 50.0f);
    bool humidity_ok = (reading.humidity < 95.0f); // Very high humidity may cause condensation
    bool pressure_ok = (reading.pressure > 900.0f && reading.pressure < 1100.0f); // Extreme weather check
    
    return temp_ok && humidity_ok && pressure_ok;
}