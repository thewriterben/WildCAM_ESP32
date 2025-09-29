/**
 * @file display_i2c.h
 * @brief ESP-IDF I2C Display Driver (SSD1306 OLED)
 */

#ifndef DISPLAY_I2C_H
#define DISPLAY_I2C_H

#include "../i2c_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display configuration
typedef struct {
    i2c_device_config_t i2c_config;
    uint8_t width;                  // Display width in pixels
    uint8_t height;                 // Display height in pixels
    bool external_vcc;              // External VCC supply
    uint8_t reset_pin;              // Reset GPIO pin (-1 if not used)
} display_i2c_config_t;

// Display buffer management
typedef struct {
    uint8_t* buffer;                // Display buffer
    size_t buffer_size;             // Buffer size in bytes
    bool buffer_dirty;              // Buffer needs update
    uint32_t last_update;           // Last update timestamp
} display_buffer_t;

/**
 * @brief Initialize I2C display (SSD1306)
 * @param config Display configuration
 * @param device_handle Output device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_init(const display_i2c_config_t* config, uint32_t* device_handle);

/**
 * @brief Deinitialize I2C display
 * @param device_handle Device handle
 * @return ESP_OK on success
 */
esp_err_t display_i2c_deinit(uint32_t device_handle);

/**
 * @brief Clear display
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_clear(uint32_t device_handle);

/**
 * @brief Update display with current buffer
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_update(uint32_t device_handle);

/**
 * @brief Set pixel in display buffer
 * @param device_handle Device handle
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color (0 = off, 1 = on)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_set_pixel(uint32_t device_handle, uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief Draw text on display
 * @param device_handle Device handle
 * @param x X position
 * @param y Y position
 * @param text Text to draw
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_draw_text(uint32_t device_handle, uint8_t x, uint8_t y, const char* text);

/**
 * @brief Set display contrast
 * @param device_handle Device handle
 * @param contrast Contrast level (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_set_contrast(uint32_t device_handle, uint8_t contrast);

/**
 * @brief Turn display on/off
 * @param device_handle Device handle
 * @param on true to turn on, false to turn off
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_set_power(uint32_t device_handle, bool on);

/**
 * @brief Create default display configuration
 * @param i2c_port I2C port number
 * @param device_addr I2C device address
 * @return Default configuration
 */
display_i2c_config_t display_i2c_create_default_config(i2c_port_t i2c_port, uint8_t device_addr);

/**
 * @brief Display wildlife monitoring status
 * @param device_handle Device handle
 * @param temp Temperature in Celsius
 * @param humidity Humidity percentage
 * @param detections Number of wildlife detections
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_i2c_show_wildlife_status(uint32_t device_handle, float temp, float humidity, uint32_t detections);

// SSD1306 Commands
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_CHARGEPUMP          0x8D

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_I2C_H