/**
 * @file i2c_hal.cpp
 * @brief Hardware Abstraction Layer implementation for I2C on different ESP32 camera boards
 */

#include "i2c_hal.h"
#include "../debug_utils.h"
#include <driver/gpio.h>

// Board-specific I2C pin configurations to avoid camera conflicts
static const board_i2c_config_t board_i2c_configs[] = {
    // AI-Thinker ESP32-CAM - avoid camera pins (21, 22, 26, 27 used by camera)
    {
        .board_name = "AI-Thinker ESP32-CAM",
        .master_pins = {
            .sda_pin = GPIO_NUM_14,  // SD CLK (alternative use)
            .scl_pin = GPIO_NUM_15,  // SD CMD (alternative use) 
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_2,   // LED pin (can be shared)
            .scl_pin = GPIO_NUM_4,   // SD D1 (alternative use)
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = true
    },
    
    // ESP32-S3-CAM - has more pins available
    {
        .board_name = "ESP32-S3-CAM",
        .master_pins = {
            .sda_pin = GPIO_NUM_4,   // More GPIO available on S3
            .scl_pin = GPIO_NUM_5,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_6,
            .scl_pin = GPIO_NUM_7,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = true
    },
    
    // ESP-EYE - already has OLED on I2C
    {
        .board_name = "ESP-EYE", 
        .master_pins = {
            .sda_pin = GPIO_NUM_18,  // Existing OLED pins
            .scl_pin = GPIO_NUM_23,
            .pullup_enable = false,  // External pullups present
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_19,  // Alternative pins
            .scl_pin = GPIO_NUM_20,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = true
    },
    
    // M5Stack Timer CAM
    {
        .board_name = "M5Stack Timer CAM",
        .master_pins = {
            .sda_pin = GPIO_NUM_12,
            .scl_pin = GPIO_NUM_13,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_12,  // Shared bus
            .scl_pin = GPIO_NUM_13,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = false
    },
    
    // TTGO T-Camera
    {
        .board_name = "TTGO T-Camera",
        .master_pins = {
            .sda_pin = GPIO_NUM_21,  // Standard I2C pins if available
            .scl_pin = GPIO_NUM_22,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_16,  // Alternative pins
            .scl_pin = GPIO_NUM_17,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = true
    },
    
    // XIAO ESP32S3 Sense
    {
        .board_name = "XIAO ESP32S3 Sense",
        .master_pins = {
            .sda_pin = GPIO_NUM_5,
            .scl_pin = GPIO_NUM_6,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .slave_pins = {
            .sda_pin = GPIO_NUM_7,
            .scl_pin = GPIO_NUM_8,
            .pullup_enable = true,
            .speed = I2C_SPEED_FAST
        },
        .has_separate_slave_bus = true
    }
};

static const size_t num_board_configs = sizeof(board_i2c_configs) / sizeof(board_i2c_config_t);

esp_err_t i2c_hal_init(BoardType board_type) {
    DEBUG_PRINTF("Initializing I2C HAL for board type: %d\n", board_type);
    
    // Validate board type
    if (board_type == BOARD_UNKNOWN) {
        DEBUG_PRINTLN("Warning: Unknown board type for I2C HAL initialization");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get board configuration
    i2c_bus_config_t master_config, slave_config;
    esp_err_t ret = i2c_hal_get_board_config(board_type, &master_config, &slave_config);
    if (ret != ESP_OK) {
        DEBUG_PRINTF("Failed to get I2C configuration for board type %d\n", board_type);
        return ret;
    }
    
    DEBUG_PRINTF("I2C HAL initialized successfully for board: %s\n", 
                 i2c_hal_get_board_name(board_type));
    return ESP_OK;
}

esp_err_t i2c_hal_get_board_config(BoardType board_type, 
                                   i2c_bus_config_t* master_config,
                                   i2c_bus_config_t* slave_config) {
    if (!master_config || !slave_config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Find board configuration
    const board_i2c_config_t* board_config = NULL;
    for (size_t i = 0; i < num_board_configs; i++) {
        if (i == (size_t)board_type - 1) { // Adjust for enum offset
            board_config = &board_i2c_configs[i];
            break;
        }
    }
    
    if (!board_config) {
        // Use default configuration for unknown boards
        DEBUG_PRINTLN("Using default I2C configuration for unknown board");
        board_config = &board_i2c_configs[0]; // Default to AI-Thinker config
    }
    
    // Setup master configuration
    master_config->port = I2C_MASTER_NUM;
    master_config->mode = I2C_MODE_MASTER;
    master_config->pins = board_config->master_pins;
    master_config->slave_addr = 0; // Not used in master mode
    master_config->tx_buf_len = I2C_MASTER_TX_BUF_LEN;
    master_config->rx_buf_len = I2C_MASTER_RX_BUF_LEN;
    master_config->timeout_ms = I2C_MASTER_TIMEOUT_MS;
    master_config->enable_dma = false; // Can be enabled later
    
    // Setup slave configuration
    slave_config->port = board_config->has_separate_slave_bus ? I2C_SLAVE_NUM : I2C_MASTER_NUM;
    slave_config->mode = I2C_MODE_SLAVE;
    slave_config->pins = board_config->slave_pins;
    slave_config->slave_addr = MULTIBOARD_NODE_BASE_ADDR; // Default node address
    slave_config->tx_buf_len = I2C_SLAVE_TX_BUF_LEN;
    slave_config->rx_buf_len = I2C_SLAVE_RX_BUF_LEN;
    slave_config->timeout_ms = I2C_SLAVE_TIMEOUT_MS;
    slave_config->enable_dma = false;
    
    return ESP_OK;
}

bool i2c_hal_check_pin_availability(BoardType board_type, gpio_num_t sda_pin, gpio_num_t scl_pin) {
    // Camera pins that are typically occupied (board-specific)
    gpio_num_t camera_pins[] = {
        GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_26, GPIO_NUM_27, // AI-Thinker camera I2C
        GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_5, GPIO_NUM_4,   // Camera data pins
        GPIO_NUM_34, GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_39, // Input-only camera pins
        GPIO_NUM_25, GPIO_NUM_23, GPIO_NUM_0                // Camera control pins
    };
    
    size_t num_camera_pins = sizeof(camera_pins) / sizeof(gpio_num_t);
    
    // Check if SDA or SCL conflicts with camera pins
    for (size_t i = 0; i < num_camera_pins; i++) {
        if (sda_pin == camera_pins[i] || scl_pin == camera_pins[i]) {
            DEBUG_PRINTF("I2C pin conflict detected: pin %d or %d conflicts with camera\n", 
                        sda_pin, scl_pin);
            return false;
        }
    }
    
    // Check if pins are input-only (ESP32 limitation)
    if (sda_pin >= GPIO_NUM_34 && sda_pin <= GPIO_NUM_39) {
        DEBUG_PRINTF("SDA pin %d is input-only, cannot be used for I2C\n", sda_pin);
        return false;
    }
    
    if (scl_pin >= GPIO_NUM_34 && scl_pin <= GPIO_NUM_39) {
        DEBUG_PRINTF("SCL pin %d is input-only, cannot be used for I2C\n", scl_pin);
        return false;
    }
    
    // Check strapping pins that might cause boot issues
    gpio_num_t strapping_pins[] = {GPIO_NUM_0, GPIO_NUM_2, GPIO_NUM_5, GPIO_NUM_12, GPIO_NUM_15};
    size_t num_strapping_pins = sizeof(strapping_pins) / sizeof(gpio_num_t);
    
    for (size_t i = 0; i < num_strapping_pins; i++) {
        if (sda_pin == strapping_pins[i] || scl_pin == strapping_pins[i]) {
            DEBUG_PRINTF("Warning: I2C pin %d or %d is a strapping pin\n", sda_pin, scl_pin);
            // Don't fail, but warn
        }
    }
    
    return true;
}

esp_err_t i2c_hal_get_optimal_pins(BoardType board_type,
                                   i2c_pin_config_t* master_pins,
                                   i2c_pin_config_t* slave_pins) {
    if (!master_pins || !slave_pins) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get board configuration first
    i2c_bus_config_t master_config, slave_config;
    esp_err_t ret = i2c_hal_get_board_config(board_type, &master_config, &slave_config);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Check if configured pins are available
    if (!i2c_hal_check_pin_availability(board_type, 
                                        master_config.pins.sda_pin,
                                        master_config.pins.scl_pin)) {
        DEBUG_PRINTLN("Configured master I2C pins have conflicts, searching for alternatives");
        
        // Try to find alternative pins for this board type
        // This is a simplified fallback - in production, this would be more sophisticated
        gpio_num_t alt_pins[][2] = {
            {GPIO_NUM_16, GPIO_NUM_17},
            {GPIO_NUM_32, GPIO_NUM_33},
            {GPIO_NUM_14, GPIO_NUM_15},
            {GPIO_NUM_2, GPIO_NUM_4}
        };
        
        bool found = false;
        for (size_t i = 0; i < 4; i++) {
            if (i2c_hal_check_pin_availability(board_type, alt_pins[i][0], alt_pins[i][1])) {
                master_pins->sda_pin = alt_pins[i][0];
                master_pins->scl_pin = alt_pins[i][1];
                master_pins->pullup_enable = true;
                master_pins->speed = I2C_SPEED_FAST;
                found = true;
                DEBUG_PRINTF("Found alternative master I2C pins: SDA=%d, SCL=%d\n",
                            master_pins->sda_pin, master_pins->scl_pin);
                break;
            }
        }
        
        if (!found) {
            DEBUG_PRINTLN("No suitable I2C pins found for this board");
            return ESP_ERR_NOT_FOUND;
        }
    } else {
        *master_pins = master_config.pins;
    }
    
    // Similar check for slave pins
    if (!i2c_hal_check_pin_availability(board_type,
                                        slave_config.pins.sda_pin,
                                        slave_config.pins.scl_pin)) {
        // Use master pins for slave if no separate slave bus
        *slave_pins = *master_pins;
        DEBUG_PRINTLN("Using shared bus for I2C slave mode");
    } else {
        *slave_pins = slave_config.pins;
    }
    
    return ESP_OK;
}

esp_err_t i2c_hal_validate_config(BoardType board_type, const i2c_bus_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Validate pin availability
    if (!i2c_hal_check_pin_availability(board_type, config->pins.sda_pin, config->pins.scl_pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Validate I2C speed
    if (config->pins.speed > I2C_SPEED_FAST_PLUS) {
        DEBUG_PRINTLN("Warning: I2C speed may be too high for some devices");
    }
    
    // Validate buffer sizes
    if (config->tx_buf_len == 0 || config->rx_buf_len == 0) {
        DEBUG_PRINTLN("I2C buffer sizes must be non-zero");
        return ESP_ERR_INVALID_ARG;
    }
    
    return ESP_OK;
}

uint32_t i2c_hal_get_capabilities(BoardType board_type) {
    uint32_t capabilities = I2C_CAP_MASTER; // All boards support master mode
    
    switch (board_type) {
        case BOARD_ESP32_S3_CAM:
        case BOARD_ESP32_S3_EYE:
        case BOARD_XIAO_ESP32S3_SENSE:
            capabilities |= I2C_CAP_SLAVE | I2C_CAP_DUAL_BUS | I2C_CAP_HIGH_SPEED;
            break;
            
        case BOARD_ESP_EYE:
            capabilities |= I2C_CAP_SLAVE | I2C_CAP_DUAL_BUS | I2C_CAP_PULLUPS;
            break;
            
        case BOARD_AI_THINKER_ESP32_CAM:
            capabilities |= I2C_CAP_SLAVE; // Limited due to pin constraints
            break;
            
        default:
            capabilities |= I2C_CAP_SLAVE; // Assume basic slave support
            break;
    }
    
    return capabilities;
}

static const char* i2c_hal_get_board_name(BoardType board_type) {
    for (size_t i = 0; i < num_board_configs; i++) {
        if (i == (size_t)board_type - 1) {
            return board_i2c_configs[i].board_name;
        }
    }
    return "Unknown Board";
}