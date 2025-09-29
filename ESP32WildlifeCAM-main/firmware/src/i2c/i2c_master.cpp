/**
 * @file i2c_master.cpp
 * @brief ESP-IDF I2C master implementation
 */

#include "i2c_master.h"
#include "../debug_utils.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

static const char* TAG = "I2C_MASTER";

// Master configuration storage
static i2c_bus_config_t master_configs[I2C_NUM_MAX];
static bool master_initialized[I2C_NUM_MAX] = {false};
static SemaphoreHandle_t master_mutex[I2C_NUM_MAX] = {NULL};

// Private function declarations
static esp_err_t validate_master_config(const i2c_bus_config_t* config);
static TickType_t ms_to_ticks(uint32_t timeout_ms);
static esp_err_t create_i2c_cmd_link_write(i2c_port_t port, uint8_t device_addr, 
                                           const uint8_t* data, size_t length);
static esp_err_t create_i2c_cmd_link_read(i2c_port_t port, uint8_t device_addr,
                                          uint8_t* data, size_t length);

esp_err_t i2c_master_init(const i2c_bus_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = validate_master_config(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid I2C master configuration");
        return ret;
    }
    
    i2c_port_t port = config->port;
    
    if (master_initialized[port]) {
        ESP_LOGW(TAG, "I2C master port %d already initialized", port);
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing I2C master on port %d, SDA=%d, SCL=%d, Speed=%d Hz",
             port, config->pins.sda_pin, config->pins.scl_pin, config->pins.speed);
    
    // Configure I2C parameters
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config->pins.sda_pin,
        .scl_io_num = config->pins.scl_pin,
        .sda_pullup_en = config->pins.pullup_enable,
        .scl_pullup_en = config->pins.pullup_enable,
        .master = {
            .clk_speed = config->pins.speed
        }
    };
    
    // Configure I2C
    ret = i2c_param_config(port, &i2c_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C parameters: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Install I2C driver
    ret = i2c_driver_install(port, I2C_MODE_MASTER, 
                            config->rx_buf_len, config->tx_buf_len, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create mutex for this port
    master_mutex[port] = xSemaphoreCreateMutex();
    if (!master_mutex[port]) {
        ESP_LOGE(TAG, "Failed to create mutex for I2C master port %d", port);
        i2c_driver_delete(port);
        return ESP_ERR_NO_MEM;
    }
    
    // Store configuration
    master_configs[port] = *config;
    master_initialized[port] = true;
    
    ESP_LOGI(TAG, "I2C master port %d initialized successfully", port);
    return ESP_OK;
}

esp_err_t i2c_master_deinit(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !master_initialized[port]) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Deinitializing I2C master port %d", port);
    
    // Delete mutex
    if (master_mutex[port]) {
        vSemaphoreDelete(master_mutex[port]);
        master_mutex[port] = NULL;
    }
    
    // Delete I2C driver
    esp_err_t ret = i2c_driver_delete(port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete I2C driver: %s", esp_err_to_name(ret));
    }
    
    master_initialized[port] = false;
    return ret;
}

esp_err_t i2c_master_probe_device(i2c_port_t port, uint8_t device_addr) {
    if (port >= I2C_NUM_MAX || !master_initialized[port]) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!master_mutex[port]) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(master_mutex[port], portMAX_DELAY);
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        xSemaphoreGive(master_mutex[port]);
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    xSemaphoreGive(master_mutex[port]);
    
    return ret;
}

esp_err_t i2c_master_write(i2c_port_t port, uint8_t device_addr, const uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!master_mutex[port]) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(master_mutex[port], portMAX_DELAY);
    
    esp_err_t ret = create_i2c_cmd_link_write(port, device_addr, data, length);
    
    xSemaphoreGive(master_mutex[port]);
    
    return ret;
}

esp_err_t i2c_master_read(i2c_port_t port, uint8_t device_addr, uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!master_mutex[port]) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(master_mutex[port], portMAX_DELAY);
    
    esp_err_t ret = create_i2c_cmd_link_read(port, device_addr, data, length);
    
    xSemaphoreGive(master_mutex[port]);
    
    return ret;
}

esp_err_t i2c_master_write_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr, 
                               const uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!master_mutex[port]) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(master_mutex[port], portMAX_DELAY);
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        xSemaphoreGive(master_mutex[port]);
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
    
    if (length > 0) {
        i2c_master_write(cmd, data, length, I2C_ACK_CHECK_EN);
    }
    
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    xSemaphoreGive(master_mutex[port]);
    
    return ret;
}

esp_err_t i2c_master_read_reg(i2c_port_t port, uint8_t device_addr, uint8_t reg_addr,
                              uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!master_mutex[port]) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(master_mutex[port], portMAX_DELAY);
    
    esp_err_t ret;
    
    // Write register address
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        xSemaphoreGive(master_mutex[port]);
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        xSemaphoreGive(master_mutex[port]);
        return ret;
    }
    
    // Read data
    cmd = i2c_cmd_link_create();
    if (!cmd) {
        xSemaphoreGive(master_mutex[port]);
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, I2C_ACK_CHECK_EN);
    
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + length - 1, I2C_NACK_VAL);
    
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    xSemaphoreGive(master_mutex[port]);
    
    return ret;
}

esp_err_t i2c_master_write_regs(i2c_port_t port, uint8_t device_addr, 
                                const uint8_t* reg_data, size_t num_regs) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !reg_data || num_regs == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // reg_data format: [reg_addr1, data1, reg_addr2, data2, ...]
    for (size_t i = 0; i < num_regs * 2; i += 2) {
        esp_err_t ret = i2c_master_write_reg(port, device_addr, reg_data[i], &reg_data[i + 1], 1);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg_data[i], esp_err_to_name(ret));
            return ret;
        }
    }
    
    return ESP_OK;
}

esp_err_t i2c_master_read_regs(i2c_port_t port, uint8_t device_addr, uint8_t start_reg,
                               uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // For many devices, we can read multiple registers in sequence
    return i2c_master_read_reg(port, device_addr, start_reg, data, length);
}

esp_err_t i2c_master_recover_bus(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !master_initialized[port]) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGW(TAG, "Recovering I2C bus %d", port);
    
    // Store current configuration
    i2c_bus_config_t config = master_configs[port];
    
    // Deinitialize and reinitialize the bus
    esp_err_t ret = i2c_master_deinit(port);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Small delay to let bus settle
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ret = i2c_master_init(&config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C bus %d recovered successfully", port);
    } else {
        ESP_LOGE(TAG, "Failed to recover I2C bus %d: %s", port, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_master_set_speed(i2c_port_t port, i2c_speed_t speed) {
    if (port >= I2C_NUM_MAX || !master_initialized[port]) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Update stored configuration
    master_configs[port].pins.speed = speed;
    
    // Apply new speed (requires reconfiguration)
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = master_configs[port].pins.sda_pin,
        .scl_io_num = master_configs[port].pins.scl_pin,
        .sda_pullup_en = master_configs[port].pins.pullup_enable,
        .scl_pullup_en = master_configs[port].pins.pullup_enable,
        .master = {
            .clk_speed = speed
        }
    };
    
    esp_err_t ret = i2c_param_config(port, &i2c_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C speed changed to %d Hz on port %d", speed, port);
    } else {
        ESP_LOGE(TAG, "Failed to change I2C speed: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_master_get_config(i2c_port_t port, i2c_bus_config_t* config) {
    if (port >= I2C_NUM_MAX || !master_initialized[port] || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *config = master_configs[port];
    return ESP_OK;
}

esp_err_t i2c_master_set_clock_stretching(i2c_port_t port, bool enable) {
    if (port >= I2C_NUM_MAX || !master_initialized[port]) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // ESP-IDF supports clock stretching by default
    // This is more of a configuration hint for timeout adjustments
    if (enable) {
        // Increase timeout for slow devices
        master_configs[port].timeout_ms = master_configs[port].timeout_ms * 2;
        ESP_LOGI(TAG, "Clock stretching enabled for port %d, timeout increased", port);
    } else {
        // Reset to normal timeout
        master_configs[port].timeout_ms = I2C_MASTER_TIMEOUT_MS;
        ESP_LOGI(TAG, "Clock stretching disabled for port %d, timeout reset", port);
    }
    
    return ESP_OK;
}

// Private function implementations
static esp_err_t validate_master_config(const i2c_bus_config_t* config) {
    if (config->port >= I2C_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid I2C port number: %d", config->port);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->mode != I2C_MODE_MASTER) {
        ESP_LOGE(TAG, "Config mode must be I2C_MODE_MASTER");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->pins.sda_pin == config->pins.scl_pin) {
        ESP_LOGE(TAG, "SDA and SCL pins cannot be the same");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->pins.speed == 0 || config->pins.speed > I2C_SPEED_FAST_PLUS) {
        ESP_LOGE(TAG, "Invalid I2C speed: %d", config->pins.speed);
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

static TickType_t ms_to_ticks(uint32_t timeout_ms) {
    return pdMS_TO_TICKS(timeout_ms);
}

static esp_err_t create_i2c_cmd_link_write(i2c_port_t port, uint8_t device_addr, 
                                           const uint8_t* data, size_t length) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
    i2c_master_write(cmd, data, length, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}

static esp_err_t create_i2c_cmd_link_read(i2c_port_t port, uint8_t device_addr,
                                          uint8_t* data, size_t length) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) {
        return ESP_ERR_NO_MEM;
    }
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (device_addr << 1) | I2C_MASTER_READ, I2C_ACK_CHECK_EN);
    
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + length - 1, I2C_NACK_VAL);
    
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, ms_to_ticks(master_configs[port].timeout_ms));
    i2c_cmd_link_delete(cmd);
    
    return ret;
}