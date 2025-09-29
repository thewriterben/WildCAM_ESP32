/**
 * @file i2c_integration.h
 * @brief Integration header for ESP-IDF I2C system with existing codebase
 */

#ifndef I2C_INTEGRATION_H
#define I2C_INTEGRATION_H

#include "i2c_manager.h"
#include "devices/bme280_i2c.h"
#include "devices/rtc_i2c.h"
#include "devices/display_i2c.h"
#include "multiboard/i2c_coordinator.h"
#include "../hal/camera_board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the complete I2C system for the detected board
 * @param board_type Detected board type
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_system_init(BoardType board_type);

/**
 * @brief Deinitialize the I2C system
 * @return ESP_OK on success
 */
esp_err_t i2c_system_deinit(void);

/**
 * @brief Initialize all available I2C devices (BME280, RTC, Display)
 * @param bme280_handle Output handle for BME280 device (can be NULL)
 * @param rtc_handle Output handle for RTC device (can be NULL)  
 * @param display_handle Output handle for display device (can be NULL)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_init_all_devices(uint32_t* bme280_handle, uint32_t* rtc_handle, uint32_t* display_handle);

/**
 * @brief Initialize BME280 environmental sensor if available
 * @param bme280_handle Output handle for BME280 device
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if not available
 */
esp_err_t i2c_init_bme280(uint32_t* bme280_handle);

/**
 * @brief Initialize external RTC if available
 * @param rtc_handle Output handle for RTC device
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if not available
 */
esp_err_t i2c_init_rtc(uint32_t* rtc_handle);

/**
 * @brief Initialize I2C display if available
 * @param display_handle Output handle for display device
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if not available
 */
esp_err_t i2c_init_display(uint32_t* display_handle);

/**
 * @brief Initialize multi-board coordination system
 * @param role Coordinator or node role
 * @param node_addr This node's I2C address
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_init_multiboard(multiboard_role_t role, uint8_t node_addr);

/**
 * @brief Check if I2C system is available and ready
 * @return true if ready, false otherwise
 */
bool i2c_system_is_available(void);

/**
 * @brief Get environmental sensor reading (BME280)
 * @param reading Pointer to store sensor reading
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_get_environmental_reading(bme280_reading_t* reading);

/**
 * @brief Get current time from RTC
 * @param time Pointer to store current time
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_get_rtc_time(rtc_time_t* time);

/**
 * @brief Update display with wildlife monitoring status
 * @param detections Number of wildlife detections
 * @param environmental_data Environmental sensor data (can be NULL)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_update_wildlife_display(uint32_t detections, const bme280_reading_t* environmental_data);

/**
 * @brief Scan for available I2C devices and print results
 * @return Number of devices found
 */
int i2c_scan_and_report_devices(void);

/**
 * @brief Replace existing Wire library usage with ESP-IDF I2C
 * @note This function helps transition from Arduino Wire to ESP-IDF I2C
 * @return ESP_OK on success
 */
esp_err_t i2c_replace_wire_usage(void);

/**
 * @brief Get I2C system status summary
 * @param status_buffer Buffer to store status string
 * @param buffer_size Buffer size
 * @return ESP_OK on success
 */
esp_err_t i2c_get_system_status(char* status_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // I2C_INTEGRATION_H