/**
 * @file bme280_i2c.cpp
 * @brief BME280 Environmental Sensor I2C Driver implementation
 */

#include "bme280_i2c.h"
#include "../i2c_manager.h"
#include "../../debug_utils.h"
#include <esp_log.h>
#include <math.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "BME280_I2C";

// BME280 Calibration parameters structure
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_data_t;

// Device state structure  
typedef struct {
    uint32_t device_handle;
    bme280_config_t config;
    bme280_calib_data_t calib_data;
    bme280_status_t status;
    bool calibration_loaded;
    int32_t t_fine; // Temperature fine value for pressure/humidity calculation
} bme280_device_t;

// Device registry
#define MAX_BME280_DEVICES 4
static bme280_device_t devices[MAX_BME280_DEVICES];
static bool device_slots_used[MAX_BME280_DEVICES] = {false};

// Private function declarations
static int find_device_slot(uint32_t device_handle);
static int allocate_device_slot(void);
static esp_err_t load_calibration_data(bme280_device_t* device);
static esp_err_t write_register(uint32_t device_handle, uint8_t reg, uint8_t value);
static esp_err_t read_register(uint32_t device_handle, uint8_t reg, uint8_t* value);
static esp_err_t read_registers(uint32_t device_handle, uint8_t reg, uint8_t* data, size_t length);
static int32_t compensate_temperature(bme280_device_t* device, int32_t adc_T);
static uint32_t compensate_pressure(bme280_device_t* device, int32_t adc_P);
static uint32_t compensate_humidity(bme280_device_t* device, int32_t adc_H);

esp_err_t bme280_init(const bme280_config_t* config, uint32_t* device_handle) {
    if (!config || !device_handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing BME280 sensor at I2C address 0x%02X", config->i2c_config.device_addr);
    
    // Allocate device slot
    int slot = allocate_device_slot();
    if (slot < 0) {
        ESP_LOGE(TAG, "No free BME280 device slots");
        return ESP_ERR_NO_MEM;
    }
    
    // Register device with I2C manager
    esp_err_t ret = i2c_manager_register_device(&config->i2c_config, device_handle);
    if (ret != ESP_OK) {
        device_slots_used[slot] = false;
        ESP_LOGE(TAG, "Failed to register BME280 with I2C manager: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize device structure
    bme280_device_t* device = &devices[slot];
    memset(device, 0, sizeof(bme280_device_t));
    device->device_handle = *device_handle;
    device->config = *config;
    device->status = BME280_STATUS_UNINITIALIZED;
    
    // Check if device is present
    uint8_t chip_id;
    ret = bme280_read_chip_id(*device_handle, &chip_id);
    if (ret != ESP_OK || chip_id != BME280_CHIP_ID) {
        ESP_LOGE(TAG, "BME280 not found or invalid chip ID: 0x%02X (expected 0x%02X)", 
                 chip_id, BME280_CHIP_ID);
        i2c_manager_unregister_device(*device_handle);
        device_slots_used[slot] = false;
        return ESP_ERR_NOT_FOUND;
    }
    
    // Reset sensor
    ret = bme280_reset(*device_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset BME280: %s", esp_err_to_name(ret));
        i2c_manager_unregister_device(*device_handle);
        device_slots_used[slot] = false;
        return ret;
    }
    
    // Wait for reset to complete
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Load calibration data
    ret = load_calibration_data(device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load BME280 calibration data: %s", esp_err_to_name(ret));
        i2c_manager_unregister_device(*device_handle);
        device_slots_used[slot] = false;
        return ret;
    }
    
    // Configure sensor
    ret = bme280_configure(*device_handle, config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure BME280: %s", esp_err_to_name(ret));
        i2c_manager_unregister_device(*device_handle);
        device_slots_used[slot] = false;
        return ret;
    }
    
    device->status = BME280_STATUS_READY;
    ESP_LOGI(TAG, "BME280 initialized successfully with handle %lu", *device_handle);
    
    return ESP_OK;
}

esp_err_t bme280_deinit(uint32_t device_handle) {
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    // Set to sleep mode
    bme280_set_sleep_mode(device_handle, true);
    
    // Unregister from I2C manager
    esp_err_t ret = i2c_manager_unregister_device(device_handle);
    
    // Free device slot
    device_slots_used[slot] = false;
    memset(&devices[slot], 0, sizeof(bme280_device_t));
    
    ESP_LOGI(TAG, "BME280 deinitialized");
    return ret;
}

bool bme280_is_present(uint32_t device_handle) {
    uint8_t chip_id;
    esp_err_t ret = bme280_read_chip_id(device_handle, &chip_id);
    return (ret == ESP_OK && chip_id == BME280_CHIP_ID);
}

esp_err_t bme280_read_chip_id(uint32_t device_handle, uint8_t* chip_id) {
    if (!chip_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return read_register(device_handle, BME280_REG_CHIP_ID, chip_id);
}

esp_err_t bme280_reset(uint32_t device_handle) {
    return write_register(device_handle, BME280_REG_RESET, BME280_RESET_CMD);
}

esp_err_t bme280_configure(uint32_t device_handle, const bme280_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    bme280_device_t* device = &devices[slot];
    
    // Configure humidity oversampling (must be done first)
    uint8_t ctrl_hum = config->hum_oversampling & 0x07;
    esp_err_t ret = write_register(device_handle, BME280_REG_CTRL_HUM, ctrl_hum);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Configure measurement control (temperature and pressure oversampling + mode)
    uint8_t ctrl_meas = (config->temp_oversampling << 5) | (config->press_oversampling << 2) | BME280_MODE_SLEEP;
    ret = write_register(device_handle, BME280_REG_CTRL_MEAS, ctrl_meas);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Configure filter and standby time
    uint8_t config_reg = (config->standby_time << 5) | (config->filter_coefficient << 2);
    if (config->enable_spi3w) {
        config_reg |= 0x01;
    }
    ret = write_register(device_handle, BME280_REG_CONFIG, config_reg);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Update stored configuration
    device->config = *config;
    
    ESP_LOGI(TAG, "BME280 configured: T_os=%d, P_os=%d, H_os=%d, Filter=%d", 
             config->temp_oversampling, config->press_oversampling, 
             config->hum_oversampling, config->filter_coefficient);
    
    return ESP_OK;
}

esp_err_t bme280_trigger_measurement(uint32_t device_handle) {
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    bme280_device_t* device = &devices[slot];
    device->status = BME280_STATUS_READING;
    
    // Read current ctrl_meas register
    uint8_t ctrl_meas;
    esp_err_t ret = read_register(device_handle, BME280_REG_CTRL_MEAS, &ctrl_meas);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Set forced mode (keeps oversampling settings)
    ctrl_meas = (ctrl_meas & 0xFC) | BME280_MODE_FORCED;
    ret = write_register(device_handle, BME280_REG_CTRL_MEAS, ctrl_meas);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "BME280 measurement triggered");
    }
    
    return ret;
}

esp_err_t bme280_is_measurement_ready(uint32_t device_handle, bool* ready) {
    if (!ready) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t status;
    esp_err_t ret = read_register(device_handle, BME280_REG_STATUS, &status);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Check if measurement is in progress (bit 3) or image register update (bit 0)
    *ready = ((status & 0x09) == 0);
    
    return ESP_OK;
}

esp_err_t bme280_read_all(uint32_t device_handle, bme280_reading_t* reading) {
    if (!reading) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    bme280_device_t* device = &devices[slot];
    
    // Read all data registers in one burst (8 bytes: pressure + temperature + humidity)
    uint8_t data[8];
    esp_err_t ret = read_registers(device_handle, BME280_REG_PRESS_MSB, data, 8);
    if (ret != ESP_OK) {
        reading->valid = false;
        return ret;
    }
    
    // Extract raw ADC values
    int32_t adc_P = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);
    int32_t adc_T = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);
    int32_t adc_H = ((uint32_t)data[6] << 8) | (uint32_t)data[7];
    
    // Compensate temperature first (needed for pressure and humidity)
    int32_t temp_int = compensate_temperature(device, adc_T);
    reading->temperature = temp_int / 100.0f;
    
    // Compensate pressure
    uint32_t press_int = compensate_pressure(device, adc_P);
    reading->pressure = press_int / 25600.0f; // Convert to hPa
    
    // Compensate humidity
    uint32_t hum_int = compensate_humidity(device, adc_H);
    reading->humidity = hum_int / 1024.0f;
    
    reading->timestamp_ms = esp_log_timestamp();
    reading->valid = true;
    device->status = BME280_STATUS_READY;
    
    ESP_LOGD(TAG, "BME280 reading: T=%.2f°C, P=%.2f hPa, H=%.2f%%", 
             reading->temperature, reading->pressure, reading->humidity);
    
    return ESP_OK;
}

esp_err_t bme280_read_temperature(uint32_t device_handle, float* temperature) {
    bme280_reading_t reading;
    esp_err_t ret = bme280_read_all(device_handle, &reading);
    if (ret == ESP_OK && temperature) {
        *temperature = reading.temperature;
    }
    return ret;
}

esp_err_t bme280_read_pressure(uint32_t device_handle, float* pressure) {
    bme280_reading_t reading;
    esp_err_t ret = bme280_read_all(device_handle, &reading);
    if (ret == ESP_OK && pressure) {
        *pressure = reading.pressure;
    }
    return ret;
}

esp_err_t bme280_read_humidity(uint32_t device_handle, float* humidity) {
    bme280_reading_t reading;
    esp_err_t ret = bme280_read_all(device_handle, &reading);
    if (ret == ESP_OK && humidity) {
        *humidity = reading.humidity;
    }
    return ret;
}

esp_err_t bme280_set_sleep_mode(uint32_t device_handle, bool sleep) {
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    bme280_device_t* device = &devices[slot];
    
    // Read current ctrl_meas register
    uint8_t ctrl_meas;
    esp_err_t ret = read_register(device_handle, BME280_REG_CTRL_MEAS, &ctrl_meas);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Set mode bits
    if (sleep) {
        ctrl_meas = (ctrl_meas & 0xFC) | BME280_MODE_SLEEP;
        device->status = BME280_STATUS_SLEEPING;
    } else {
        ctrl_meas = (ctrl_meas & 0xFC) | BME280_MODE_NORMAL;
        device->status = BME280_STATUS_READY;
    }
    
    ret = write_register(device_handle, BME280_REG_CTRL_MEAS, ctrl_meas);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "BME280 %s", sleep ? "entered sleep mode" : "woke up");
    }
    
    return ret;
}

esp_err_t bme280_get_status(uint32_t device_handle, bme280_status_t* status) {
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int slot = find_device_slot(device_handle);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }
    
    *status = devices[slot].status;
    return ESP_OK;
}

float bme280_calculate_altitude(float pressure_hpa, float sea_level_hpa) {
    if (sea_level_hpa == 0) {
        sea_level_hpa = 1013.25f; // Standard sea level pressure
    }
    
    return 44330.0f * (1.0f - powf(pressure_hpa / sea_level_hpa, 0.1903f));
}

float bme280_calculate_sea_level_pressure(float pressure_hpa, float altitude_m) {
    return pressure_hpa / powf(1.0f - (altitude_m / 44330.0f), 5.255f);
}

bme280_config_t bme280_create_default_config(i2c_port_t i2c_port, uint8_t device_addr) {
    bme280_config_t config = {
        .i2c_config = {
            .device_addr = device_addr,
            .bus_port = i2c_port,
            .timeout_ms = 1000,
            .auto_retry = true,
            .max_retries = 3
        },
        .temp_oversampling = BME280_OVERSAMPLING_2X,
        .press_oversampling = BME280_OVERSAMPLING_16X,
        .hum_oversampling = BME280_OVERSAMPLING_1X,
        .filter_coefficient = BME280_FILTER_COEFF_16,
        .standby_time = BME280_STANDBY_62_5_MS,
        .enable_spi3w = false
    };
    
    return config;
}

// Private function implementations
static int find_device_slot(uint32_t device_handle) {
    for (int i = 0; i < MAX_BME280_DEVICES; i++) {
        if (device_slots_used[i] && devices[i].device_handle == device_handle) {
            return i;
        }
    }
    return -1;
}

static int allocate_device_slot(void) {
    for (int i = 0; i < MAX_BME280_DEVICES; i++) {
        if (!device_slots_used[i]) {
            device_slots_used[i] = true;
            return i;
        }
    }
    return -1;
}

static esp_err_t load_calibration_data(bme280_device_t* device) {
    uint8_t calib_data[26]; // T1-T3, P1-P9 (24 bytes) + H1 (1 byte) + reserved (1 byte)
    uint8_t calib_h_data[7]; // H2-H6 (7 bytes)
    
    // Read temperature and pressure calibration data
    esp_err_t ret = read_registers(device->device_handle, BME280_REG_CALIB_T1_LSB, calib_data, 26);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Read humidity calibration data
    ret = read_registers(device->device_handle, BME280_REG_CALIB_H2_LSB, calib_h_data, 7);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Parse calibration data
    bme280_calib_data_t* calib = &device->calib_data;
    
    calib->dig_T1 = (calib_data[1] << 8) | calib_data[0];
    calib->dig_T2 = (calib_data[3] << 8) | calib_data[2];
    calib->dig_T3 = (calib_data[5] << 8) | calib_data[4];
    
    calib->dig_P1 = (calib_data[7] << 8) | calib_data[6];
    calib->dig_P2 = (calib_data[9] << 8) | calib_data[8];
    calib->dig_P3 = (calib_data[11] << 8) | calib_data[10];
    calib->dig_P4 = (calib_data[13] << 8) | calib_data[12];
    calib->dig_P5 = (calib_data[15] << 8) | calib_data[14];
    calib->dig_P6 = (calib_data[17] << 8) | calib_data[16];
    calib->dig_P7 = (calib_data[19] << 8) | calib_data[18];
    calib->dig_P8 = (calib_data[21] << 8) | calib_data[20];
    calib->dig_P9 = (calib_data[23] << 8) | calib_data[22];
    
    calib->dig_H1 = calib_data[25];
    calib->dig_H2 = (calib_h_data[1] << 8) | calib_h_data[0];
    calib->dig_H3 = calib_h_data[2];
    calib->dig_H4 = (calib_h_data[3] << 4) | (calib_h_data[4] & 0x0F);
    calib->dig_H5 = (calib_h_data[5] << 4) | (calib_h_data[4] >> 4);
    calib->dig_H6 = calib_h_data[6];
    
    device->calibration_loaded = true;
    ESP_LOGI(TAG, "BME280 calibration data loaded successfully");
    
    return ESP_OK;
}

static esp_err_t write_register(uint32_t device_handle, uint8_t reg, uint8_t value) {
    return i2c_manager_write_reg(device_handle, reg, &value, 1);
}

static esp_err_t read_register(uint32_t device_handle, uint8_t reg, uint8_t* value) {
    if (!value) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_manager_read_reg(device_handle, reg, value, 1);
}

static esp_err_t read_registers(uint32_t device_handle, uint8_t reg, uint8_t* data, size_t length) {
    if (!data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_manager_read_reg(device_handle, reg, data, length);
}

// BME280 compensation algorithms from datasheet
static int32_t compensate_temperature(bme280_device_t* device, int32_t adc_T) {
    const bme280_calib_data_t* calib = &device->calib_data;
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calib->dig_T1 << 1))) * ((int32_t)calib->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib->dig_T1)) * ((adc_T >> 4) - ((int32_t)calib->dig_T1))) >> 12) * 
            ((int32_t)calib->dig_T3)) >> 14;
    device->t_fine = var1 + var2;
    T = (device->t_fine * 5 + 128) >> 8;
    
    return T;
}

static uint32_t compensate_pressure(bme280_device_t* device, int32_t adc_P) {
    const bme280_calib_data_t* calib = &device->calib_data;
    int64_t var1, var2, p;
    
    var1 = ((int64_t)device->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib->dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib->dig_P5) << 17);
    var2 = var2 + (((int64_t)calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib->dig_P3) >> 8) + ((var1 * (int64_t)calib->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib->dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0; // Avoid exception caused by division by zero
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib->dig_P7) << 4);
    
    return (uint32_t)p;
}

static uint32_t compensate_humidity(bme280_device_t* device, int32_t adc_H) {
    const bme280_calib_data_t* calib = &device->calib_data;
    int32_t v_x1_u32r;
    
    v_x1_u32r = (device->t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib->dig_H4) << 20) - (((int32_t)calib->dig_H5) * v_x1_u32r)) +
                   ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)calib->dig_H6)) >> 10) *
                   (((v_x1_u32r * ((int32_t)calib->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                   ((int32_t)calib->dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib->dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    
    return (uint32_t)(v_x1_u32r >> 12);
}