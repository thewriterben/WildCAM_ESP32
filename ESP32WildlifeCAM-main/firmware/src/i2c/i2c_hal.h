/**
 * @file i2c_hal.h
 * @brief Hardware Abstraction Layer for I2C on different ESP32 camera boards
 */

#ifndef I2C_HAL_H
#define I2C_HAL_H

#include "i2c_config.h"
#include "../hal/camera_board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C HAL for specific board type
 * @param board_type The detected board type
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_init(BoardType board_type);

/**
 * @brief Get I2C configuration for specific board
 * @param board_type The board type
 * @param master_config Output master I2C configuration
 * @param slave_config Output slave I2C configuration (if supported)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t i2c_hal_get_board_config(BoardType board_type, 
                                   i2c_bus_config_t* master_config,
                                   i2c_bus_config_t* slave_config);

/**
 * @brief Check if pins are available for I2C (no conflicts with camera)
 * @param board_type The board type
 * @param sda_pin SDA pin to check
 * @param scl_pin SCL pin to check
 * @return true if pins are available, false if conflict exists
 */
bool i2c_hal_check_pin_availability(BoardType board_type, gpio_num_t sda_pin, gpio_num_t scl_pin);

/**
 * @brief Get optimal I2C pin assignment for board
 * @param board_type The board type
 * @param master_pins Output master I2C pin configuration
 * @param slave_pins Output slave I2C pin configuration  
 * @return ESP_OK on success, error code if no suitable pins found
 */
esp_err_t i2c_hal_get_optimal_pins(BoardType board_type,
                                   i2c_pin_config_t* master_pins,
                                   i2c_pin_config_t* slave_pins);

/**
 * @brief Validate I2C configuration against board constraints
 * @param board_type The board type
 * @param config I2C configuration to validate
 * @return ESP_OK if valid, error code otherwise
 */
esp_err_t i2c_hal_validate_config(BoardType board_type, const i2c_bus_config_t* config);

/**
 * @brief Get board-specific I2C capabilities
 * @param board_type The board type
 * @return Capability flags
 */
uint32_t i2c_hal_get_capabilities(BoardType board_type);

// Capability flags
#define I2C_CAP_MASTER          (1 << 0)   /*!< Supports I2C master mode */
#define I2C_CAP_SLAVE           (1 << 1)   /*!< Supports I2C slave mode */
#define I2C_CAP_DUAL_BUS        (1 << 2)   /*!< Supports dual I2C buses */
#define I2C_CAP_HIGH_SPEED      (1 << 3)   /*!< Supports high-speed I2C */
#define I2C_CAP_PULLUPS         (1 << 4)   /*!< Has external pullup resistors */
#define I2C_CAP_LEVEL_SHIFT     (1 << 5)   /*!< Has level shifters */

#ifdef __cplusplus
}
#endif

#endif // I2C_HAL_H