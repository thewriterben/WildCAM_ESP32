/**
 * @file i2c_manager.h
 * @brief Main I2C management system for ESP32WildlifeCAM
 */

#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include "i2c_config.h"
#include "i2c_hal.h"
#include "../hal/camera_board.h"

#ifdef __cplusplus
extern "C" {
#endif

// I2C Manager state
typedef enum {
    I2C_STATE_UNINITIALIZED = 0,
    I2C_STATE_INITIALIZING,
    I2C_STATE_READY,
    I2C_STATE_ERROR,
    I2C_STATE_SUSPENDED
} i2c_manager_state_t;

// I2C Manager statistics
typedef struct {
    uint32_t total_transactions;
    uint32_t successful_transactions;
    uint32_t failed_transactions;
    uint32_t timeout_errors;
    uint32_t bus_errors;
    uint32_t device_not_found_errors;
    uint32_t last_error_code;
    uint32_t uptime_ms;
} i2c_manager_stats_t;

/**
 * @brief Initialize I2C manager with board-specific configuration
 * @param board_type Detected board type
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_init(BoardType board_type);

/**
 * @brief Deinitialize I2C manager and release resources
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_deinit(void);

/**
 * @brief Get current I2C manager state
 * @return Current state
 */
i2c_manager_state_t i2c_manager_get_state(void);

/**
 * @brief Get I2C manager statistics
 * @param stats Pointer to stats structure to fill
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_get_stats(i2c_manager_stats_t* stats);

/**
 * @brief Reset I2C manager statistics
 */
void i2c_manager_reset_stats(void);

/**
 * @brief Perform I2C device scan on specified bus
 * @param bus_port I2C bus port to scan
 * @param devices Array to store found device addresses
 * @param max_devices Maximum number of devices to find
 * @param num_found Pointer to store number of devices found
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_scan_devices(i2c_port_t bus_port, 
                                   uint8_t* devices, 
                                   size_t max_devices,
                                   size_t* num_found);

/**
 * @brief Check if device is present on I2C bus
 * @param bus_port I2C bus port
 * @param device_addr Device I2C address
 * @return true if device responds, false otherwise
 */
bool i2c_manager_device_present(i2c_port_t bus_port, uint8_t device_addr);

/**
 * @brief Register a device with the I2C manager
 * @param device_config Device configuration
 * @param device_handle Output device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_register_device(const i2c_device_config_t* device_config,
                                      uint32_t* device_handle);

/**
 * @brief Unregister a device from the I2C manager
 * @param device_handle Device handle to unregister
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_unregister_device(uint32_t device_handle);

/**
 * @brief Write data to I2C device
 * @param device_handle Device handle
 * @param data Data to write
 * @param length Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_write(uint32_t device_handle, const uint8_t* data, size_t length);

/**
 * @brief Read data from I2C device
 * @param device_handle Device handle
 * @param data Buffer to store read data
 * @param length Number of bytes to read
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_read(uint32_t device_handle, uint8_t* data, size_t length);

/**
 * @brief Write register on I2C device
 * @param device_handle Device handle
 * @param reg_addr Register address
 * @param data Data to write
 * @param length Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_write_reg(uint32_t device_handle, uint8_t reg_addr, 
                                const uint8_t* data, size_t length);

/**
 * @brief Read register from I2C device
 * @param device_handle Device handle
 * @param reg_addr Register address
 * @param data Buffer to store read data
 * @param length Number of bytes to read
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_manager_read_reg(uint32_t device_handle, uint8_t reg_addr,
                               uint8_t* data, size_t length);

/**
 * @brief Perform I2C bus recovery (reset)
 * @param bus_port I2C bus port to recover
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_recover_bus(i2c_port_t bus_port);

/**
 * @brief Enable/disable power management for I2C
 * @param enable true to enable power management
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_set_power_management(bool enable);

/**
 * @brief Suspend I2C operations (for low power mode)
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_suspend(void);

/**
 * @brief Resume I2C operations from suspension
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_resume(void);

/**
 * @brief Get I2C bus configuration
 * @param bus_port I2C bus port
 * @param config Pointer to store configuration
 * @return ESP_OK on success
 */
esp_err_t i2c_manager_get_bus_config(i2c_port_t bus_port, i2c_bus_config_t* config);

/**
 * @brief Check if I2C manager is ready for operations
 * @return true if ready, false otherwise
 */
bool i2c_manager_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_MANAGER_H