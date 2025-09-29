/**
 * @file i2c_manager.cpp
 * @brief Main I2C management system implementation
 */

#include "i2c_manager.h"
#include "i2c_master.h"
#include "i2c_slave.h"
#include "../debug_utils.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <string.h>

static const char* TAG = "I2C_MANAGER";

// Manager state
static i2c_manager_state_t manager_state = I2C_STATE_UNINITIALIZED;
static BoardType current_board_type = BOARD_UNKNOWN;
static i2c_bus_config_t master_bus_config;
static i2c_bus_config_t slave_bus_config;
static bool power_management_enabled = false;

// Statistics
static i2c_manager_stats_t manager_stats = {0};

// Device registry
#define MAX_REGISTERED_DEVICES 16
static i2c_device_config_t registered_devices[MAX_REGISTERED_DEVICES];
static bool device_slots_used[MAX_REGISTERED_DEVICES];
static uint32_t next_device_handle = 1;

// Synchronization
static SemaphoreHandle_t manager_mutex = NULL;
static SemaphoreHandle_t stats_mutex = NULL;

// Forward declarations
static esp_err_t initialize_buses(void);
static esp_err_t deinitialize_buses(void);
static uint32_t allocate_device_handle(void);
static int find_device_slot(uint32_t handle);
static void update_stats(bool success, esp_err_t error_code);

esp_err_t i2c_manager_init(BoardType board_type) {
    if (manager_state != I2C_STATE_UNINITIALIZED) {
        DEBUG_PRINTLN("I2C Manager already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    manager_state = I2C_STATE_INITIALIZING;
    current_board_type = board_type;
    
    DEBUG_PRINTF("Initializing I2C Manager for board type: %d\n", board_type);
    
    // Create mutexes
    manager_mutex = xSemaphoreCreateMutex();
    stats_mutex = xSemaphoreCreateMutex();
    if (!manager_mutex || !stats_mutex) {
        ESP_LOGE(TAG, "Failed to create mutexes");
        manager_state = I2C_STATE_ERROR;
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize HAL
    esp_err_t ret = i2c_hal_init(board_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C HAL: %s", esp_err_to_name(ret));
        manager_state = I2C_STATE_ERROR;
        return ret;
    }
    
    // Get board-specific configurations
    ret = i2c_hal_get_board_config(board_type, &master_bus_config, &slave_bus_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get board I2C config: %s", esp_err_to_name(ret));
        manager_state = I2C_STATE_ERROR;
        return ret;
    }
    
    // Initialize I2C buses
    ret = initialize_buses();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C buses: %s", esp_err_to_name(ret));
        manager_state = I2C_STATE_ERROR;
        return ret;
    }
    
    // Reset device registry
    memset(registered_devices, 0, sizeof(registered_devices));
    memset(device_slots_used, false, sizeof(device_slots_used));
    
    // Reset statistics
    memset(&manager_stats, 0, sizeof(manager_stats));
    manager_stats.uptime_ms = esp_log_timestamp();
    
    manager_state = I2C_STATE_READY;
    ESP_LOGI(TAG, "I2C Manager initialized successfully");
    
    return ESP_OK;
}

esp_err_t i2c_manager_deinit(void) {
    if (manager_state == I2C_STATE_UNINITIALIZED) {
        return ESP_OK;
    }
    
    xSemaphoreTake(manager_mutex, portMAX_DELAY);
    
    manager_state = I2C_STATE_UNINITIALIZED;
    
    // Deinitialize buses
    deinitialize_buses();
    
    // Clear device registry
    memset(registered_devices, 0, sizeof(registered_devices));
    memset(device_slots_used, false, sizeof(device_slots_used));
    
    xSemaphoreGive(manager_mutex);
    
    // Delete mutexes
    if (manager_mutex) {
        vSemaphoreDelete(manager_mutex);
        manager_mutex = NULL;
    }
    if (stats_mutex) {
        vSemaphoreDelete(stats_mutex);
        stats_mutex = NULL;
    }
    
    ESP_LOGI(TAG, "I2C Manager deinitialized");
    return ESP_OK;
}

i2c_manager_state_t i2c_manager_get_state(void) {
    return manager_state;
}

esp_err_t i2c_manager_get_stats(i2c_manager_stats_t* stats) {
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!stats_mutex) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(stats_mutex, portMAX_DELAY);
    memcpy(stats, &manager_stats, sizeof(i2c_manager_stats_t));
    stats->uptime_ms = esp_log_timestamp() - manager_stats.uptime_ms;
    xSemaphoreGive(stats_mutex);
    
    return ESP_OK;
}

void i2c_manager_reset_stats(void) {
    if (stats_mutex) {
        xSemaphoreTake(stats_mutex, portMAX_DELAY);
        memset(&manager_stats, 0, sizeof(manager_stats));
        manager_stats.uptime_ms = esp_log_timestamp();
        xSemaphoreGive(stats_mutex);
    }
}

esp_err_t i2c_manager_scan_devices(i2c_port_t bus_port, 
                                   uint8_t* devices, 
                                   size_t max_devices,
                                   size_t* num_found) {
    if (!devices || !num_found || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *num_found = 0;
    
    DEBUG_PRINTF("Scanning I2C devices on bus %d\n", bus_port);
    
    for (uint8_t addr = 1; addr < 127 && *num_found < max_devices; addr++) {
        if (i2c_manager_device_present(bus_port, addr)) {
            devices[*num_found] = addr;
            (*num_found)++;
            DEBUG_PRINTF("Found I2C device at address 0x%02X\n", addr);
        }
    }
    
    ESP_LOGI(TAG, "Found %d I2C devices on bus %d", *num_found, bus_port);
    return ESP_OK;
}

bool i2c_manager_device_present(i2c_port_t bus_port, uint8_t device_addr) {
    if (manager_state != I2C_STATE_READY) {
        return false;
    }
    
    esp_err_t ret = i2c_master_probe_device(bus_port, device_addr);
    update_stats(ret == ESP_OK, ret);
    
    return (ret == ESP_OK);
}

esp_err_t i2c_manager_register_device(const i2c_device_config_t* device_config,
                                      uint32_t* device_handle) {
    if (!device_config || !device_handle || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(manager_mutex, portMAX_DELAY);
    
    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_REGISTERED_DEVICES; i++) {
        if (!device_slots_used[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        xSemaphoreGive(manager_mutex);
        ESP_LOGE(TAG, "No free device slots available");
        return ESP_ERR_NO_MEM;
    }
    
    // Allocate handle and register device
    uint32_t handle = allocate_device_handle();
    registered_devices[slot] = *device_config;
    device_slots_used[slot] = true;
    *device_handle = handle;
    
    xSemaphoreGive(manager_mutex);
    
    ESP_LOGI(TAG, "Registered I2C device 0x%02X on bus %d with handle %lu",
             device_config->device_addr, device_config->bus_port, handle);
    
    return ESP_OK;
}

esp_err_t i2c_manager_unregister_device(uint32_t device_handle) {
    if (manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(manager_mutex, portMAX_DELAY);
    
    int slot = find_device_slot(device_handle);
    if (slot >= 0) {
        device_slots_used[slot] = false;
        memset(&registered_devices[slot], 0, sizeof(i2c_device_config_t));
        ESP_LOGI(TAG, "Unregistered I2C device with handle %lu", device_handle);
    }
    
    xSemaphoreGive(manager_mutex);
    
    return (slot >= 0) ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t i2c_manager_write(uint32_t device_handle, const uint8_t* data, size_t length) {
    if (!data || length == 0 || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        update_stats(false, ESP_ERR_NOT_FOUND);
        return ESP_ERR_NOT_FOUND;
    }
    
    const i2c_device_config_t* device = &registered_devices[slot];
    esp_err_t ret = i2c_master_write(device->bus_port, device->device_addr, data, length);
    
    update_stats(ret == ESP_OK, ret);
    return ret;
}

esp_err_t i2c_manager_read(uint32_t device_handle, uint8_t* data, size_t length) {
    if (!data || length == 0 || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        update_stats(false, ESP_ERR_NOT_FOUND);
        return ESP_ERR_NOT_FOUND;
    }
    
    const i2c_device_config_t* device = &registered_devices[slot];
    esp_err_t ret = i2c_master_read(device->bus_port, device->device_addr, data, length);
    
    update_stats(ret == ESP_OK, ret);
    return ret;
}

esp_err_t i2c_manager_write_reg(uint32_t device_handle, uint8_t reg_addr, 
                                const uint8_t* data, size_t length) {
    if (!data || length == 0 || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        update_stats(false, ESP_ERR_NOT_FOUND);
        return ESP_ERR_NOT_FOUND;
    }
    
    const i2c_device_config_t* device = &registered_devices[slot];
    esp_err_t ret = i2c_master_write_reg(device->bus_port, device->device_addr, 
                                         reg_addr, data, length);
    
    update_stats(ret == ESP_OK, ret);
    return ret;
}

esp_err_t i2c_manager_read_reg(uint32_t device_handle, uint8_t reg_addr,
                               uint8_t* data, size_t length) {
    if (!data || length == 0 || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        update_stats(false, ESP_ERR_NOT_FOUND);
        return ESP_ERR_NOT_FOUND;
    }
    
    const i2c_device_config_t* device = &registered_devices[slot];
    esp_err_t ret = i2c_master_read_reg(device->bus_port, device->device_addr,
                                        reg_addr, data, length);
    
    update_stats(ret == ESP_OK, ret);
    return ret;
}

esp_err_t i2c_manager_recover_bus(i2c_port_t bus_port) {
    if (manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGW(TAG, "Recovering I2C bus %d", bus_port);
    esp_err_t ret = i2c_master_recover_bus(bus_port);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2C bus %d recovered successfully", bus_port);
    } else {
        ESP_LOGE(TAG, "Failed to recover I2C bus %d: %s", bus_port, esp_err_to_name(ret));
    }
    
    return ret;
}

esp_err_t i2c_manager_set_power_management(bool enable) {
    power_management_enabled = enable;
    ESP_LOGI(TAG, "I2C power management %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

esp_err_t i2c_manager_suspend(void) {
    if (manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(manager_mutex, portMAX_DELAY);
    manager_state = I2C_STATE_SUSPENDED;
    
    // Suspend I2C operations if power management is enabled
    if (power_management_enabled) {
        // This would implement power-down of I2C peripherals
        ESP_LOGI(TAG, "I2C operations suspended for power management");
    }
    
    xSemaphoreGive(manager_mutex);
    return ESP_OK;
}

esp_err_t i2c_manager_resume(void) {
    if (manager_state != I2C_STATE_SUSPENDED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    xSemaphoreTake(manager_mutex, portMAX_DELAY);
    
    // Resume I2C operations
    if (power_management_enabled) {
        // This would implement power-up and re-initialization of I2C peripherals
        ESP_LOGI(TAG, "I2C operations resumed from power management");
    }
    
    manager_state = I2C_STATE_READY;
    xSemaphoreGive(manager_mutex);
    return ESP_OK;
}

esp_err_t i2c_manager_get_bus_config(i2c_port_t bus_port, i2c_bus_config_t* config) {
    if (!config || manager_state != I2C_STATE_READY) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (bus_port == master_bus_config.port) {
        *config = master_bus_config;
    } else if (bus_port == slave_bus_config.port) {
        *config = slave_bus_config;
    } else {
        return ESP_ERR_NOT_FOUND;
    }
    
    return ESP_OK;
}

bool i2c_manager_is_ready(void) {
    return (manager_state == I2C_STATE_READY);
}

// Private functions
static esp_err_t initialize_buses(void) {
    esp_err_t ret;
    
    // Initialize master bus
    ret = i2c_master_init(&master_bus_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize slave bus if different from master
    if (slave_bus_config.port != master_bus_config.port) {
        ret = i2c_slave_init(&slave_bus_config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize I2C slave: %s", esp_err_to_name(ret));
            i2c_master_deinit(master_bus_config.port);
            return ret;
        }
    }
    
    ESP_LOGI(TAG, "I2C buses initialized successfully");
    return ESP_OK;
}

static esp_err_t deinitialize_buses(void) {
    esp_err_t ret1 = i2c_master_deinit(master_bus_config.port);
    esp_err_t ret2 = ESP_OK;
    
    if (slave_bus_config.port != master_bus_config.port) {
        ret2 = i2c_slave_deinit(slave_bus_config.port);
    }
    
    return (ret1 == ESP_OK && ret2 == ESP_OK) ? ESP_OK : ESP_FAIL;
}

static uint32_t allocate_device_handle(void) {
    return next_device_handle++;
}

static int find_device_slot(uint32_t handle) {
    for (int i = 0; i < MAX_REGISTERED_DEVICES; i++) {
        if (device_slots_used[i] && handle == (next_device_handle - MAX_REGISTERED_DEVICES + i)) {
            return i;
        }
    }
    return -1;
}

static void update_stats(bool success, esp_err_t error_code) {
    if (stats_mutex) {
        xSemaphoreTake(stats_mutex, portMAX_DELAY);
        
        manager_stats.total_transactions++;
        if (success) {
            manager_stats.successful_transactions++;
        } else {
            manager_stats.failed_transactions++;
            manager_stats.last_error_code = error_code;
            
            // Categorize errors
            switch (error_code) {
                case ESP_ERR_TIMEOUT:
                    manager_stats.timeout_errors++;
                    break;
                case ESP_ERR_NOT_FOUND:
                    manager_stats.device_not_found_errors++;
                    break;
                default:
                    manager_stats.bus_errors++;
                    break;
            }
        }
        
        xSemaphoreGive(stats_mutex);
    }
}