/**
 * @file i2c_slave.h
 * @brief ESP-IDF I2C slave implementation for multi-board communication
 */

#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include "i2c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C Slave callback types
typedef void (*i2c_slave_receive_cb_t)(uint8_t* data, size_t length);
typedef void (*i2c_slave_request_cb_t)(uint8_t* data, size_t* length, size_t max_length);

/**
 * @brief Initialize I2C slave with given configuration
 * @param config I2C bus configuration for slave mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_init(const i2c_bus_config_t* config);

/**
 * @brief Deinitialize I2C slave
 * @param port I2C port number
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_deinit(i2c_port_t port);

/**
 * @brief Set slave receive callback (called when master writes to slave)
 * @param port I2C port number
 * @param callback Callback function
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_set_receive_callback(i2c_port_t port, i2c_slave_receive_cb_t callback);

/**
 * @brief Set slave request callback (called when master reads from slave)
 * @param port I2C port number
 * @param callback Callback function
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_set_request_callback(i2c_port_t port, i2c_slave_request_cb_t callback);

/**
 * @brief Check if there is data available to read from master
 * @param port I2C port number
 * @return Number of bytes available
 */
size_t i2c_slave_get_available_data(i2c_port_t port);

/**
 * @brief Read data sent by master (non-blocking)
 * @param port I2C port number
 * @param data Buffer to store received data
 * @param max_length Maximum bytes to read
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes actually read
 */
size_t i2c_slave_read(i2c_port_t port, uint8_t* data, size_t max_length, uint32_t timeout_ms);

/**
 * @brief Write data to be sent to master when requested
 * @param port I2C port number
 * @param data Data to send
 * @param length Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_write(i2c_port_t port, const uint8_t* data, size_t length);

/**
 * @brief Change slave address
 * @param port I2C port number
 * @param new_address New slave address
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_set_address(i2c_port_t port, uint8_t new_address);

/**
 * @brief Get current slave address
 * @param port I2C port number
 * @param address Pointer to store current address
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_get_address(i2c_port_t port, uint8_t* address);

/**
 * @brief Enable/disable slave mode
 * @param port I2C port number
 * @param enable true to enable slave mode
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_enable(i2c_port_t port, bool enable);

/**
 * @brief Get slave status (enabled/disabled, last activity, etc.)
 * @param port I2C port number
 * @param enabled Pointer to store enabled status
 * @param last_activity_ms Pointer to store last activity timestamp
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_get_status(i2c_port_t port, bool* enabled, uint32_t* last_activity_ms);

/**
 * @brief Process pending I2C slave operations (call from main loop)
 * @param port I2C port number
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_slave_process(i2c_port_t port);

/**
 * @brief Reset slave buffers
 * @param port I2C port number
 * @return ESP_OK on success
 */
esp_err_t i2c_slave_reset_buffers(i2c_port_t port);

#ifdef __cplusplus
}
#endif

#endif // I2C_SLAVE_H