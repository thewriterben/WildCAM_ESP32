/**
 * @file rtc_i2c.h
 * @brief External RTC I2C Driver using ESP-IDF (DS3231/PCF8563)
 */

#ifndef RTC_I2C_H
#define RTC_I2C_H

#include "../i2c_config.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// RTC Types
typedef enum {
    RTC_TYPE_DS3231 = 0,    // DS3231 RTC with temperature compensation
    RTC_TYPE_PCF8563        // PCF8563 RTC (lower power)
} rtc_type_t;

// RTC Configuration
typedef struct {
    i2c_device_config_t i2c_config;
    rtc_type_t rtc_type;
    bool enable_32khz_output;       // Enable 32kHz square wave output
    bool enable_alarm_interrupt;    // Enable alarm interrupt
    bool enable_battery_backup;     // Enable battery backup mode
} rtc_config_t;

// RTC Time structure
typedef struct {
    uint8_t second;     // 0-59
    uint8_t minute;     // 0-59
    uint8_t hour;       // 0-23
    uint8_t day;        // 1-31
    uint8_t month;      // 1-12
    uint16_t year;      // Full year (e.g., 2024)
    uint8_t weekday;    // 0-6 (0=Sunday)
} rtc_time_t;

// RTC Alarm structure
typedef struct {
    uint8_t second;     // 0-59 (255 = don't care)
    uint8_t minute;     // 0-59 (255 = don't care)
    uint8_t hour;       // 0-23 (255 = don't care)
    uint8_t day;        // 1-31 (255 = don't care)
    bool enabled;       // Alarm enable flag
} rtc_alarm_t;

// RTC Status
typedef enum {
    RTC_STATUS_OK = 0,
    RTC_STATUS_TIME_INVALID,
    RTC_STATUS_OSCILLATOR_STOPPED,
    RTC_STATUS_BATTERY_LOW,
    RTC_STATUS_COMMUNICATION_ERROR
} rtc_status_t;

/**
 * @brief Initialize RTC
 * @param config RTC configuration
 * @param device_handle Output device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_init(const rtc_config_t* config, uint32_t* device_handle);

/**
 * @brief Deinitialize RTC
 * @param device_handle Device handle
 * @return ESP_OK on success
 */
esp_err_t rtc_deinit(uint32_t device_handle);

/**
 * @brief Check if RTC is present and responding
 * @param device_handle Device handle
 * @return true if RTC is present, false otherwise
 */
bool rtc_is_present(uint32_t device_handle);

/**
 * @brief Set RTC time
 * @param device_handle Device handle
 * @param time Time to set
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_set_time(uint32_t device_handle, const rtc_time_t* time);

/**
 * @brief Get RTC time
 * @param device_handle Device handle
 * @param time Pointer to store current time
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_get_time(uint32_t device_handle, rtc_time_t* time);

/**
 * @brief Set time from Unix timestamp
 * @param device_handle Device handle
 * @param timestamp Unix timestamp (seconds since 1970)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_set_time_from_timestamp(uint32_t device_handle, time_t timestamp);

/**
 * @brief Get time as Unix timestamp
 * @param device_handle Device handle
 * @param timestamp Pointer to store timestamp
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_get_timestamp(uint32_t device_handle, time_t* timestamp);

/**
 * @brief Synchronize ESP32 system time with RTC
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_sync_system_time(uint32_t device_handle);

/**
 * @brief Update RTC from ESP32 system time
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_update_from_system_time(uint32_t device_handle);

/**
 * @brief Set alarm
 * @param device_handle Device handle
 * @param alarm Alarm configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_set_alarm(uint32_t device_handle, const rtc_alarm_t* alarm);

/**
 * @brief Get alarm configuration
 * @param device_handle Device handle
 * @param alarm Pointer to store alarm configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_get_alarm(uint32_t device_handle, rtc_alarm_t* alarm);

/**
 * @brief Check if alarm triggered
 * @param device_handle Device handle
 * @param triggered Pointer to store alarm status
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_check_alarm(uint32_t device_handle, bool* triggered);

/**
 * @brief Clear alarm flag
 * @param device_handle Device handle
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_clear_alarm(uint32_t device_handle);

/**
 * @brief Get RTC temperature (DS3231 only)
 * @param device_handle Device handle
 * @param temperature Pointer to store temperature in Celsius
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_get_temperature(uint32_t device_handle, float* temperature);

/**
 * @brief Get RTC status
 * @param device_handle Device handle
 * @param status Pointer to store RTC status
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_get_status(uint32_t device_handle, rtc_status_t* status);

/**
 * @brief Enable/disable square wave output
 * @param device_handle Device handle
 * @param enable true to enable, false to disable
 * @param frequency Frequency in Hz (1, 1024, 4096, 8192 for DS3231)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t rtc_set_square_wave(uint32_t device_handle, bool enable, uint16_t frequency);

/**
 * @brief Create default RTC configuration
 * @param i2c_port I2C port number
 * @param rtc_type Type of RTC chip
 * @return Default configuration structure
 */
rtc_config_t rtc_create_default_config(i2c_port_t i2c_port, rtc_type_t rtc_type);

/**
 * @brief Convert time structure to string
 * @param time Time structure
 * @param buffer Buffer to store string
 * @param buffer_size Buffer size
 * @return ESP_OK on success
 */
esp_err_t rtc_time_to_string(const rtc_time_t* time, char* buffer, size_t buffer_size);

/**
 * @brief Calculate day of week from date
 * @param year Year (full year, e.g., 2024)
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return Day of week (0=Sunday, 6=Saturday)
 */
uint8_t rtc_calculate_day_of_week(uint16_t year, uint8_t month, uint8_t day);

// DS3231 Register definitions
#define DS3231_REG_SECONDS      0x00
#define DS3231_REG_MINUTES      0x01
#define DS3231_REG_HOURS        0x02
#define DS3231_REG_DAY          0x03
#define DS3231_REG_DATE         0x04
#define DS3231_REG_MONTH        0x05
#define DS3231_REG_YEAR         0x06
#define DS3231_REG_ALARM1_SEC   0x07
#define DS3231_REG_ALARM2_MIN   0x0B
#define DS3231_REG_CONTROL      0x0E
#define DS3231_REG_STATUS       0x0F
#define DS3231_REG_TEMP_MSB     0x11
#define DS3231_REG_TEMP_LSB     0x12

// PCF8563 Register definitions
#define PCF8563_REG_CONTROL1    0x00
#define PCF8563_REG_CONTROL2    0x01
#define PCF8563_REG_SECONDS     0x02
#define PCF8563_REG_MINUTES     0x03
#define PCF8563_REG_HOURS       0x04
#define PCF8563_REG_DAYS        0x05
#define PCF8563_REG_WEEKDAYS    0x06
#define PCF8563_REG_MONTHS      0x07
#define PCF8563_REG_YEARS       0x08

#ifdef __cplusplus
}
#endif

#endif // RTC_I2C_H