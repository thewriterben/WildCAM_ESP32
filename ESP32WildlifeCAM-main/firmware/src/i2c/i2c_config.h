/**
 * @file i2c_config.h
 * @brief I2C configuration definitions for ESP32WildlifeCAM
 */

#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

#include <driver/i2c.h>
#include <esp_err.h>

// I2C Controller definitions
#define I2C_MASTER_NUM          I2C_NUM_0    /*!< I2C master port number */
#define I2C_SLAVE_NUM           I2C_NUM_1    /*!< I2C slave port number */

// I2C Configuration constants
#define I2C_MASTER_FREQ_HZ      400000       /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_LEN   512          /*!< I2C master TX buffer length */
#define I2C_MASTER_RX_BUF_LEN   512          /*!< I2C master RX buffer length */
#define I2C_SLAVE_TX_BUF_LEN    1024         /*!< I2C slave TX buffer length */
#define I2C_SLAVE_RX_BUF_LEN    1024         /*!< I2C slave RX buffer length */

// I2C Timeout settings
#define I2C_MASTER_TIMEOUT_MS   1000         /*!< I2C master timeout in ms */
#define I2C_SLAVE_TIMEOUT_MS    1000         /*!< I2C slave timeout in ms */
#define I2C_ACK_CHECK_EN        0x1          /*!< I2C master will check ack from slave*/
#define I2C_ACK_CHECK_DIS       0x0          /*!< I2C master will not check ack from slave */
#define I2C_ACK_VAL             0x0          /*!< I2C ack value */
#define I2C_NACK_VAL            0x1          /*!< I2C nack value */

// I2C Speed configurations
typedef enum {
    I2C_SPEED_STANDARD = 100000,    /*!< 100 kHz - Standard mode */
    I2C_SPEED_FAST = 400000,        /*!< 400 kHz - Fast mode */
    I2C_SPEED_FAST_PLUS = 1000000   /*!< 1 MHz - Fast mode plus */
} i2c_speed_t;

// I2C Device addresses
#define BME280_I2C_ADDR_PRIMARY     0x76    /*!< BME280 primary I2C address */
#define BME280_I2C_ADDR_SECONDARY   0x77    /*!< BME280 secondary I2C address */
#define SSD1306_I2C_ADDR            0x3C    /*!< SSD1306 OLED I2C address */
#define SSD1306_I2C_ADDR_ALT        0x3D    /*!< SSD1306 OLED alternative address */
#define RTC_DS3231_I2C_ADDR         0x68    /*!< DS3231 RTC I2C address */
#define RTC_PCF8563_I2C_ADDR        0x51    /*!< PCF8563 RTC I2C address */

// Multi-board communication addresses
#define MULTIBOARD_COORDINATOR_ADDR 0x10    /*!< Coordinator board address */
#define MULTIBOARD_NODE_BASE_ADDR   0x20    /*!< Base address for node boards */
#define MULTIBOARD_MAX_NODES        8       /*!< Maximum number of node boards */

// I2C Pin configuration structure
typedef struct {
    gpio_num_t sda_pin;     /*!< SDA pin number */
    gpio_num_t scl_pin;     /*!< SCL pin number */
    bool pullup_enable;     /*!< Enable internal pullups */
    i2c_speed_t speed;      /*!< I2C speed */
} i2c_pin_config_t;

// I2C Bus configuration
typedef struct {
    i2c_port_t port;                /*!< I2C port number */
    i2c_mode_t mode;                /*!< I2C mode (master/slave) */
    i2c_pin_config_t pins;          /*!< Pin configuration */
    uint16_t slave_addr;            /*!< Slave address (for slave mode) */
    size_t tx_buf_len;              /*!< TX buffer length */
    size_t rx_buf_len;              /*!< RX buffer length */
    uint32_t timeout_ms;            /*!< Timeout in milliseconds */
    bool enable_dma;                /*!< Enable DMA support */
} i2c_bus_config_t;

// Device configuration structure
typedef struct {
    uint8_t device_addr;            /*!< I2C device address */
    i2c_port_t bus_port;            /*!< I2C bus port */
    uint32_t timeout_ms;            /*!< Device-specific timeout */
    bool auto_retry;                /*!< Enable automatic retry on failure */
    uint8_t max_retries;            /*!< Maximum retry attempts */
} i2c_device_config_t;

// Error handling
typedef enum {
    I2C_ERROR_NONE = 0,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_DEVICE_NOT_FOUND,
    I2C_ERROR_BUS_BUSY,
    I2C_ERROR_INVALID_CONFIG,
    I2C_ERROR_DRIVER_NOT_INSTALLED,
    I2C_ERROR_COMMUNICATION_FAILED,
    I2C_ERROR_PIN_CONFLICT
} i2c_error_t;

// Board-specific I2C pin definitions (to resolve conflicts)
typedef struct {
    const char* board_name;
    i2c_pin_config_t master_pins;   /*!< Master I2C pins */
    i2c_pin_config_t slave_pins;    /*!< Slave I2C pins (if different) */
    bool has_separate_slave_bus;    /*!< Whether slave uses separate bus */
} board_i2c_config_t;

#endif // I2C_CONFIG_H