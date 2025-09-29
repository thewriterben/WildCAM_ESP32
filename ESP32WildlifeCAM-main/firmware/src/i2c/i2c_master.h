/**
 * @file i2c_master.h
 * @brief ESP-IDF I2C master implementation for ESP32WildlifeCAM
 */

#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include "i2c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C master with given configuration
 * @param config I2C bus configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_init(const i2c_bus_config_t* config);

/**
 * @brief Deinitialize I2C master
 * @param port I2C port number
 * @return ESP_OK on success
 */
esp_err_t i2c_master_deinit(i2c_port_t port);

/**
 * @brief Probe I2C device (check if device responds)
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @return ESP_OK if device responds, error code otherwise
 */
esp_err_t i2c_master_probe_device(i2c_port_t port, uint8_t device_addr);

/**
 * @brief Write data to I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param data Data to write
 * @param length Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_write(i2c_port_t port, uint8_t device_addr, const uint8_t* data, size_t length);

/**
 * @brief Read data from I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param data Buffer to store read data
 * @param length Number of bytes to read
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_read(i2c_port_t port, uint8_t device_addr, uint8_t* data, size_t length);

/**
 * @brief Write register on I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param reg_addr Register address
 * @param data Data to write
 * @param length Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_write_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, 
                               const uint8_t* data, size_t length);

/**
 * @brief Read register from I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param reg_addr Register address
 * @param data Buffer to store read data
 * @param length Number of bytes to read
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_read_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr,
                              uint8_t* data, size_t length);

/**
 * @brief Write multiple registers on I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param reg_data Array of register address/data pairs
 * @param num_regs Number of registers to write
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_write_regs(i2c_port_t port, uint8_t device_addr, 
                                const uint8_t* reg_data, size_t num_regs);

/**
 * @brief Read multiple registers from I2C device
 * @param port I2C port number
 * @param device_addr Device I2C address
 * @param start_reg Starting register address
 * @param data Buffer to store read data
 * @param length Number of bytes to read
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_read_regs(i2c_port_t port, uint8_t device_addr, uint8_t start_reg,
                               uint8_t* data, size_t length);

/**
 * @brief Perform I2C bus recovery (clock stretching, reset)
 * @param port I2C port number
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_recover_bus(i2c_port_t port);

/**
 * @brief Change I2C master speed
 * @param port I2C port number
 * @param speed New I2C speed
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_set_speed(i2c_port_t port, i2c_speed_t speed);

/**
 * @brief Get current I2C master configuration
 * @param port I2C port number
 * @param config Pointer to store configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_get_config(i2c_port_t port, i2c_bus_config_t* config);

/**
 * @brief Enable/disable clock stretching for slow devices
 * @param port I2C port number
 * @param enable true to enable clock stretching
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_master_set_clock_stretching(i2c_port_t port, bool enable);

#ifdef __cplusplus
}
#endif

#endif // I2C_MASTER_H