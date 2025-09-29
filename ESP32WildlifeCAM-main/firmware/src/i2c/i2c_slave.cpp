/**
 * @file i2c_slave.cpp
 * @brief ESP-IDF I2C slave implementation for multi-board communication
 */

#include "i2c_slave.h"
#include "../debug_utils.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <string.h>

static const char* TAG = "I2C_SLAVE";

// Slave state structure
typedef struct {
    bool initialized;
    bool enabled;
    i2c_bus_config_t config;
    i2c_slave_receive_cb_t receive_callback;
    i2c_slave_request_cb_t request_callback;
    uint32_t last_activity_ms;
    QueueHandle_t rx_queue;
    QueueHandle_t tx_queue;
    SemaphoreHandle_t mutex;
    uint8_t* rx_buffer;
    uint8_t* tx_buffer;
    size_t rx_buffer_size;
    size_t tx_buffer_size;
} i2c_slave_state_t;

// Slave states for each port
static i2c_slave_state_t slave_states[I2C_NUM_MAX];

// Data structures for queues
typedef struct {
    uint8_t data[256]; // Maximum I2C transaction size
    size_t length;
    uint32_t timestamp;
} i2c_slave_message_t;

// Private function declarations
static esp_err_t validate_slave_config(const i2c_bus_config_t* config);
static void i2c_slave_task(void* parameter);
static esp_err_t create_slave_task(i2c_port_t port);
static void destroy_slave_resources(i2c_port_t port);

esp_err_t i2c_slave_init(const i2c_bus_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = validate_slave_config(config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid I2C slave configuration");
        return ret;
    }
    
    i2c_port_t port = config->port;
    
    if (slave_states[port].initialized) {
        ESP_LOGW(TAG, "I2C slave port %d already initialized", port);
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing I2C slave on port %d, addr=0x%02X, SDA=%d, SCL=%d",
             port, config->slave_addr, config->pins.sda_pin, config->pins.scl_pin);
    
    // Initialize slave state
    memset(&slave_states[port], 0, sizeof(i2c_slave_state_t));
    slave_states[port].config = *config;
    
    // Configure I2C parameters
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = config->pins.sda_pin,
        .scl_io_num = config->pins.scl_pin,
        .sda_pullup_en = config->pins.pullup_enable,
        .scl_pullup_en = config->pins.pullup_enable,
        .slave = {
            .addr_10bit_en = 0,
            .slave_addr = config->slave_addr,
            .maximum_speed = config->pins.speed
        }
    };
    
    // Configure I2C
    ret = i2c_param_config(port, &i2c_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C slave parameters: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Install I2C driver
    ret = i2c_driver_install(port, I2C_MODE_SLAVE, 
                            config->rx_buf_len, config->tx_buf_len, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C slave driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create synchronization objects
    slave_states[port].mutex = xSemaphoreCreateMutex();
    slave_states[port].rx_queue = xQueueCreate(8, sizeof(i2c_slave_message_t));
    slave_states[port].tx_queue = xQueueCreate(4, sizeof(i2c_slave_message_t));
    
    if (!slave_states[port].mutex || !slave_states[port].rx_queue || !slave_states[port].tx_queue) {
        ESP_LOGE(TAG, "Failed to create synchronization objects for slave port %d", port);
        destroy_slave_resources(port);
        return ESP_ERR_NO_MEM;
    }
    
    // Allocate buffers
    slave_states[port].rx_buffer_size = config->rx_buf_len;
    slave_states[port].tx_buffer_size = config->tx_buf_len;
    slave_states[port].rx_buffer = (uint8_t*)malloc(config->rx_buf_len);
    slave_states[port].tx_buffer = (uint8_t*)malloc(config->tx_buf_len);
    
    if (!slave_states[port].rx_buffer || !slave_states[port].tx_buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffers for slave port %d", port);
        destroy_slave_resources(port);
        return ESP_ERR_NO_MEM;
    }
    
    // Create slave processing task
    ret = create_slave_task(port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create slave task for port %d", port);
        destroy_slave_resources(port);
        return ret;
    }
    
    slave_states[port].initialized = true;
    slave_states[port].enabled = true;
    slave_states[port].last_activity_ms = esp_log_timestamp();
    
    ESP_LOGI(TAG, "I2C slave port %d initialized successfully", port);
    return ESP_OK;
}

esp_err_t i2c_slave_deinit(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Deinitializing I2C slave port %d", port);
    
    slave_states[port].enabled = false;
    slave_states[port].initialized = false;
    
    // Destroy resources
    destroy_slave_resources(port);
    
    // Delete I2C driver
    esp_err_t ret = i2c_driver_delete(port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete I2C slave driver: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_slave_set_receive_callback(i2c_port_t port, i2c_slave_receive_cb_t callback) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (slave_states[port].mutex) {
        xSemaphoreTake(slave_states[port].mutex, portMAX_DELAY);
        slave_states[port].receive_callback = callback;
        xSemaphoreGive(slave_states[port].mutex);
    }
    
    ESP_LOGI(TAG, "Receive callback set for slave port %d", port);
    return ESP_OK;
}

esp_err_t i2c_slave_set_request_callback(i2c_port_t port, i2c_slave_request_cb_t callback) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (slave_states[port].mutex) {
        xSemaphoreTake(slave_states[port].mutex, portMAX_DELAY);
        slave_states[port].request_callback = callback;
        xSemaphoreGive(slave_states[port].mutex);
    }
    
    ESP_LOGI(TAG, "Request callback set for slave port %d", port);
    return ESP_OK;
}

size_t i2c_slave_get_available_data(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized || !slave_states[port].rx_queue) {
        return 0;
    }
    
    return uxQueueMessagesWaiting(slave_states[port].rx_queue);
}

size_t i2c_slave_read(i2c_port_t port, uint8_t* data, size_t max_length, uint32_t timeout_ms) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized || !data || max_length == 0) {
        return 0;
    }
    
    if (!slave_states[port].rx_queue) {
        return 0;
    }
    
    i2c_slave_message_t message;
    if (xQueueReceive(slave_states[port].rx_queue, &message, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
        size_t copy_length = (message.length < max_length) ? message.length : max_length;
        memcpy(data, message.data, copy_length);
        return copy_length;
    }
    
    return 0;
}

esp_err_t i2c_slave_write(i2c_port_t port, const uint8_t* data, size_t length) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!slave_states[port].tx_queue || length > sizeof(i2c_slave_message_t().data)) {
        return ESP_ERR_INVALID_SIZE;
    }
    
    i2c_slave_message_t message;
    memcpy(message.data, data, length);
    message.length = length;
    message.timestamp = esp_log_timestamp();
    
    if (xQueueSend(slave_states[port].tx_queue, &message, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to queue data for transmission on port %d", port);
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}

esp_err_t i2c_slave_set_address(i2c_port_t port, uint8_t new_address) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // To change address, we need to reconfigure the I2C
    slave_states[port].config.slave_addr = new_address;
    
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = slave_states[port].config.pins.sda_pin,
        .scl_io_num = slave_states[port].config.pins.scl_pin,
        .sda_pullup_en = slave_states[port].config.pins.pullup_enable,
        .scl_pullup_en = slave_states[port].config.pins.pullup_enable,
        .slave = {
            .addr_10bit_en = 0,
            .slave_addr = new_address,
            .maximum_speed = slave_states[port].config.pins.speed
        }
    };
    
    esp_err_t ret = i2c_param_config(port, &i2c_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C slave address changed to 0x%02X on port %d", new_address, port);
    } else {
        ESP_LOGE(TAG, "Failed to change I2C slave address: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_slave_get_address(i2c_port_t port, uint8_t* address) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized || !address) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *address = slave_states[port].config.slave_addr;
    return ESP_OK;
}

esp_err_t i2c_slave_enable(i2c_port_t port, bool enable) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    slave_states[port].enabled = enable;
    ESP_LOGI(TAG, "I2C slave port %d %s", port, enable ? "enabled" : "disabled");
    return ESP_OK;
}

esp_err_t i2c_slave_get_status(i2c_port_t port, bool* enabled, uint32_t* last_activity_ms) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (enabled) {
        *enabled = slave_states[port].enabled;
    }
    
    if (last_activity_ms) {
        *last_activity_ms = slave_states[port].last_activity_ms;
    }
    
    return ESP_OK;
}

esp_err_t i2c_slave_process(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized || !slave_states[port].enabled) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // This function is called from the main loop to process any pending operations
    // The actual I2C processing is done in the slave task
    return ESP_OK;
}

esp_err_t i2c_slave_reset_buffers(i2c_port_t port) {
    if (port >= I2C_NUM_MAX || !slave_states[port].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (slave_states[port].rx_queue) {
        xQueueReset(slave_states[port].rx_queue);
    }
    
    if (slave_states[port].tx_queue) {
        xQueueReset(slave_states[port].tx_queue);
    }
    
    ESP_LOGI(TAG, "I2C slave buffers reset for port %d", port);
    return ESP_OK;
}

// Private function implementations
static esp_err_t validate_slave_config(const i2c_bus_config_t* config) {
    if (config->port >= I2C_NUM_MAX) {
        ESP_LOGE(TAG, "Invalid I2C port number: %d", config->port);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->mode != I2C_MODE_SLAVE) {
        ESP_LOGE(TAG, "Config mode must be I2C_MODE_SLAVE");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->slave_addr == 0 || config->slave_addr >= 128) {
        ESP_LOGE(TAG, "Invalid slave address: 0x%02X", config->slave_addr);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->pins.sda_pin == config->pins.scl_pin) {
        ESP_LOGE(TAG, "SDA and SCL pins cannot be the same");
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

static void i2c_slave_task(void* parameter) {
    i2c_port_t port = (i2c_port_t)(intptr_t)parameter;
    i2c_slave_state_t* state = &slave_states[port];
    
    ESP_LOGI(TAG, "I2C slave task started for port %d", port);
    
    while (state->initialized && state->enabled) {
        // Check for incoming data from master
        int rx_size = i2c_slave_read_buffer(port, state->rx_buffer, state->rx_buffer_size, pdMS_TO_TICKS(100));
        
        if (rx_size > 0) {
            state->last_activity_ms = esp_log_timestamp();
            
            // Queue received data
            if (state->rx_queue) {
                i2c_slave_message_t message;
                size_t copy_size = (rx_size < sizeof(message.data)) ? rx_size : sizeof(message.data);
                memcpy(message.data, state->rx_buffer, copy_size);
                message.length = copy_size;
                message.timestamp = state->last_activity_ms;
                
                if (xQueueSend(state->rx_queue, &message, 0) != pdTRUE) {
                    ESP_LOGW(TAG, "RX queue full on port %d", port);
                }
            }
            
            // Call receive callback if set
            if (state->receive_callback) {
                state->receive_callback(state->rx_buffer, rx_size);
            }
        }
        
        // Check for data to send to master
        if (state->tx_queue) {
            i2c_slave_message_t message;
            if (xQueueReceive(state->tx_queue, &message, 0) == pdTRUE) {
                // Prepare data for transmission
                memcpy(state->tx_buffer, message.data, message.length);
                
                // Write to slave buffer (will be sent when master reads)
                i2c_slave_write_buffer(port, state->tx_buffer, message.length, 0);
            }
        }
        
        // Small delay to prevent task from consuming too much CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "I2C slave task terminated for port %d", port);
    vTaskDelete(NULL);
}

static esp_err_t create_slave_task(i2c_port_t port) {
    char task_name[16];
    snprintf(task_name, sizeof(task_name), "i2c_slave_%d", port);
    
    BaseType_t result = xTaskCreate(
        i2c_slave_task,
        task_name,
        2048, // Stack size
        (void*)(intptr_t)port,
        5, // Priority
        NULL
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "Failed to create slave task for port %d", port);
        return ESP_ERR_NO_MEM;
    }
    
    return ESP_OK;
}

static void destroy_slave_resources(i2c_port_t port) {
    i2c_slave_state_t* state = &slave_states[port];
    
    if (state->mutex) {
        vSemaphoreDelete(state->mutex);
        state->mutex = NULL;
    }
    
    if (state->rx_queue) {
        vQueueDelete(state->rx_queue);
        state->rx_queue = NULL;
    }
    
    if (state->tx_queue) {
        vQueueDelete(state->tx_queue);
        state->tx_queue = NULL;
    }
    
    if (state->rx_buffer) {
        free(state->rx_buffer);
        state->rx_buffer = NULL;
    }
    
    if (state->tx_buffer) {
        free(state->tx_buffer);
        state->tx_buffer = NULL;
    }
}