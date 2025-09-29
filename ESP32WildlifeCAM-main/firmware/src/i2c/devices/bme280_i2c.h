/**
 * @file bme280_i2c.h
 * @brief BME280 Environmental Sensor I2C Driver using ESP-IDF
 */

#ifndef BME280_I2C_H
#define BME280_I2C_H

#include "../i2c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// BME280 Sensor readings structure
typedef struct {
    float temperature;      // Temperature in Celsius
    float pressure;         // Pressure in hPa
    float humidity;         // Humidity in %RH
    uint32_t timestamp_ms;  // Timestamp of reading
    bool valid;             // Reading validity flag
} bme280_reading_t;

// BME280 Configuration
typedef struct {
    i2c_device_config_t i2c_config;
    uint8_t temp_oversampling;      // Temperature oversampling (0-5)
    uint8_t press_oversampling;     // Pressure oversampling (0-5) 
    uint8_t hum_oversampling;       // Humidity oversampling (0-5)
    uint8_t filter_coefficient;     // IIR filter coefficient (0-7)
    uint8_t standby_time;           // Standby time in normal mode (0-7)
    bool enable_spi3w;              // Enable 3-wire SPI (for I2C should be false)
} bme280_config_t;

// BME280 Status
typedef enum {
    BME280_STATUS_UNINITIALIZED = 0,
    BME280_STATUS_READY,
    BME280_STATUS_READING,
    BME280_STATUS_ERROR,
    BME280_STATUS_SLEEPING
} bme280_status_t;

/**
 * @brief Initialize BME280 sensor
 * @param config BME280 configuration
 * @param device_handle Output device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_init(const bme280_config_t* config, uint32_t* device_handle);

/**
 * @brief Deinitialize BME280 sensor
 * @param device_handle Device handle
 * @return ESP_OK on success
 */
esp_err_t bme280_deinit(uint32_t device_handle);

/**
 * @brief Check if BME280 is present and responding
 * @param device_handle Device handle
 * @return true if device is present, false otherwise
 */
bool bme280_is_present(uint32_t device_handle);

/**
 * @brief Read sensor ID and verify it's BME280
 * @param device_handle Device handle
 * @param chip_id Pointer to store chip ID
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_read_chip_id(uint32_t device_handle, uint8_t* chip_id);

/**
 * @brief Reset BME280 sensor
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_reset(uint32_t device_handle);

/**
 * @brief Configure BME280 sensor parameters
 * @param device_handle Device handle
 * @param config Configuration parameters
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_configure(uint32_t device_handle, const bme280_config_t* config);

/**
 * @brief Trigger a measurement (forced mode)
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_trigger_measurement(uint32_t device_handle);

/**
 * @brief Check if measurement is ready
 * @param device_handle Device handle
 * @param ready Pointer to store ready status
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_is_measurement_ready(uint32_t device_handle, bool* ready);

/**
 * @brief Read all sensor values (temperature, pressure, humidity)
 * @param device_handle Device handle
 * @param reading Pointer to store sensor readings
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_read_all(uint32_t device_handle, bme280_reading_t* reading);

/**
 * @brief Read only temperature
 * @param device_handle Device handle
 * @param temperature Pointer to store temperature in Celsius
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_read_temperature(uint32_t device_handle, float* temperature);

/**
 * @brief Read only pressure
 * @param device_handle Device handle
 * @param pressure Pointer to store pressure in hPa
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_read_pressure(uint32_t device_handle, float* pressure);

/**
 * @brief Read only humidity
 * @param device_handle Device handle
 * @param humidity Pointer to store humidity in %RH
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_read_humidity(uint32_t device_handle, float* humidity);

/**
 * @brief Set BME280 power mode
 * @param device_handle Device handle
 * @param sleep true to enter sleep mode, false to wake up
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_set_sleep_mode(uint32_t device_handle, bool sleep);

/**
 * @brief Get BME280 status
 * @param device_handle Device handle
 * @param status Pointer to store current status
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bme280_get_status(uint32_t device_handle, bme280_status_t* status);

/**
 * @brief Calculate altitude from pressure reading
 * @param pressure_hpa Pressure in hPa
 * @param sea_level_hpa Sea level pressure in hPa (default 1013.25)
 * @return Altitude in meters
 */
float bme280_calculate_altitude(float pressure_hpa, float sea_level_hpa);

/**
 * @brief Calculate sea level pressure from altitude
 * @param pressure_hpa Current pressure in hPa
 * @param altitude_m Known altitude in meters
 * @return Sea level pressure in hPa
 */
float bme280_calculate_sea_level_pressure(float pressure_hpa, float altitude_m);

/**
 * @brief Create default BME280 configuration
 * @param i2c_port I2C port number
 * @param device_addr I2C device address (BME280_I2C_ADDR_PRIMARY or BME280_I2C_ADDR_SECONDARY)
 * @return Default configuration structure
 */
bme280_config_t bme280_create_default_config(i2c_port_t i2c_port, uint8_t device_addr);

// BME280 Register definitions
#define BME280_CHIP_ID              0x60    // Expected chip ID
#define BME280_REG_CHIP_ID          0xD0    // Chip ID register
#define BME280_REG_RESET            0xE0    // Reset register
#define BME280_REG_CTRL_HUM         0xF2    // Humidity control register
#define BME280_REG_STATUS           0xF3    // Status register
#define BME280_REG_CTRL_MEAS        0xF4    // Measurement control register
#define BME280_REG_CONFIG           0xF5    // Configuration register
#define BME280_REG_PRESS_MSB        0xF7    // Pressure data registers
#define BME280_REG_TEMP_MSB         0xFA    // Temperature data registers
#define BME280_REG_HUM_MSB          0xFD    // Humidity data registers

// BME280 Calibration register addresses
#define BME280_REG_CALIB_T1_LSB     0x88
#define BME280_REG_CALIB_H1         0xA1
#define BME280_REG_CALIB_H2_LSB     0xE1

// BME280 Commands
#define BME280_RESET_CMD            0xB6    // Soft reset command

// BME280 Power modes
#define BME280_MODE_SLEEP           0x00
#define BME280_MODE_FORCED          0x01
#define BME280_MODE_NORMAL          0x03

// BME280 Oversampling settings
#define BME280_OVERSAMPLING_SKIP    0x00
#define BME280_OVERSAMPLING_1X      0x01
#define BME280_OVERSAMPLING_2X      0x02
#define BME280_OVERSAMPLING_4X      0x03
#define BME280_OVERSAMPLING_8X      0x04
#define BME280_OVERSAMPLING_16X     0x05

// BME280 Filter coefficients
#define BME280_FILTER_OFF           0x00
#define BME280_FILTER_COEFF_2       0x01
#define BME280_FILTER_COEFF_4       0x02
#define BME280_FILTER_COEFF_8       0x03
#define BME280_FILTER_COEFF_16      0x04

// BME280 Standby times (ms)
#define BME280_STANDBY_0_5_MS       0x00
#define BME280_STANDBY_62_5_MS      0x01
#define BME280_STANDBY_125_MS       0x02
#define BME280_STANDBY_250_MS       0x03
#define BME280_STANDBY_500_MS       0x04
#define BME280_STANDBY_1000_MS      0x05
#define BME280_STANDBY_10_MS        0x06
#define BME280_STANDBY_20_MS        0x07

#ifdef __cplusplus
}
#endif

#endif // BME280_I2C_H