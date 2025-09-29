/**
 * @file i2c_coordinator.h
 * @brief Multi-board coordination system using I2C slave mode
 */

#ifndef I2C_COORDINATOR_H
#define I2C_COORDINATOR_H

#include "../i2c_config.h"
#include "../../hal/camera_board.h"

#ifdef __cplusplus
extern "C" {
#endif

// Multi-board roles
typedef enum {
    MULTIBOARD_ROLE_COORDINATOR = 0,    // Master coordinator board
    MULTIBOARD_ROLE_NODE               // Slave node board
} multiboard_role_t;

// Message types for multi-board communication
typedef enum {
    MSG_TYPE_HEARTBEAT = 0x01,          // Periodic heartbeat
    MSG_TYPE_CONFIG_UPDATE = 0x02,      // Configuration update
    MSG_TYPE_WILDLIFE_DETECTED = 0x03,  // Wildlife detection alert
    MSG_TYPE_CAPTURE_REQUEST = 0x04,    // Request image capture
    MSG_TYPE_STATUS_REQUEST = 0x05,     // Request status update
    MSG_TYPE_SYNC_TIME = 0x06,          // Time synchronization
    MSG_TYPE_POWER_STATUS = 0x07,       // Power/battery status
    MSG_TYPE_SENSOR_DATA = 0x08,        // Environmental sensor data
    MSG_TYPE_ERROR_REPORT = 0x09,       // Error reporting
    MSG_TYPE_RESET_REQUEST = 0x0A       // Reset request
} multiboard_message_type_t;

// Message structure
typedef struct {
    uint8_t msg_type;                   // Message type
    uint8_t source_addr;                // Source node address
    uint8_t dest_addr;                  // Destination address (0xFF = broadcast)
    uint8_t sequence;                   // Message sequence number
    uint16_t data_length;               // Data payload length
    uint8_t data[240];                  // Data payload (max I2C transaction)
    uint32_t timestamp;                 // Message timestamp
    uint8_t checksum;                   // Simple checksum
} multiboard_message_t;

// Node status structure
typedef struct {
    uint8_t node_addr;                  // Node I2C address
    bool online;                        // Node online status
    uint32_t last_heartbeat;            // Last heartbeat timestamp
    uint32_t total_messages;            // Total messages received
    uint32_t last_sequence;             // Last sequence number
    float battery_voltage;              // Battery voltage
    float temperature;                  // Temperature sensor reading
    uint32_t wildlife_detections;       // Number of wildlife detections
    char node_name[32];                 // Node friendly name
} multiboard_node_status_t;

// Coordinator configuration
typedef struct {
    multiboard_role_t role;             // Coordinator or node role
    uint8_t coordinator_addr;           // Coordinator I2C address
    uint8_t node_addr;                  // This node's I2C address
    uint16_t heartbeat_interval_ms;     // Heartbeat interval
    uint16_t status_timeout_ms;         // Node timeout for offline detection
    uint8_t max_retries;                // Message retry count
    bool enable_auto_capture;           // Auto-capture on wildlife detection
    bool enable_time_sync;              // Enable time synchronization
} multiboard_config_t;

// Message callback types
typedef void (*multiboard_message_cb_t)(const multiboard_message_t* message);
typedef void (*multiboard_wildlife_cb_t)(uint8_t source_node, const uint8_t* detection_data, size_t data_len);
typedef void (*multiboard_status_cb_t)(uint8_t node_addr, bool online);

/**
 * @brief Initialize multi-board coordinator system
 * @param config Coordinator configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_coordinator_init(const multiboard_config_t* config);

/**
 * @brief Deinitialize multi-board coordinator
 * @return ESP_OK on success
 */
esp_err_t multiboard_coordinator_deinit(void);

/**
 * @brief Start coordinator services (heartbeat, status monitoring)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_coordinator_start(void);

/**
 * @brief Stop coordinator services
 * @return ESP_OK on success
 */
esp_err_t multiboard_coordinator_stop(void);

/**
 * @brief Send message to specific node or broadcast
 * @param dest_addr Destination address (0xFF for broadcast)
 * @param msg_type Message type
 * @param data Data payload
 * @param data_len Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_send_message(uint8_t dest_addr, multiboard_message_type_t msg_type,
                                  const uint8_t* data, size_t data_len);

/**
 * @brief Send configuration update to nodes
 * @param config_json JSON configuration string
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_send_config_update(const char* config_json);

/**
 * @brief Send wildlife detection alert
 * @param detection_data Wildlife detection data
 * @param data_len Data length
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_send_wildlife_alert(const uint8_t* detection_data, size_t data_len);

/**
 * @brief Request image capture from specific node
 * @param node_addr Node address
 * @param capture_params Capture parameters (can be NULL)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_request_capture(uint8_t node_addr, const char* capture_params);

/**
 * @brief Get status of all nodes
 * @param nodes Array to store node statuses
 * @param max_nodes Maximum number of nodes
 * @param num_nodes Pointer to store actual number of nodes
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_get_node_status(multiboard_node_status_t* nodes, size_t max_nodes, size_t* num_nodes);

/**
 * @brief Get status of specific node
 * @param node_addr Node address
 * @param status Pointer to store node status
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_get_node_info(uint8_t node_addr, multiboard_node_status_t* status);

/**
 * @brief Set message received callback
 * @param callback Callback function
 * @return ESP_OK on success
 */
esp_err_t multiboard_set_message_callback(multiboard_message_cb_t callback);

/**
 * @brief Set wildlife detection callback
 * @param callback Callback function
 * @return ESP_OK on success
 */
esp_err_t multiboard_set_wildlife_callback(multiboard_wildlife_cb_t callback);

/**
 * @brief Set node status change callback
 * @param callback Callback function
 * @return ESP_OK on success
 */
esp_err_t multiboard_set_status_callback(multiboard_status_cb_t callback);

/**
 * @brief Synchronize time across all nodes
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_sync_time(void);

/**
 * @brief Get coordinator role
 * @return Current role (coordinator or node)
 */
multiboard_role_t multiboard_get_role(void);

/**
 * @brief Get number of online nodes
 * @return Number of online nodes
 */
size_t multiboard_get_online_nodes_count(void);

/**
 * @brief Check if coordinator system is active
 * @return true if active, false otherwise
 */
bool multiboard_is_active(void);

/**
 * @brief Get coordinator statistics
 * @param total_messages Pointer to store total messages
 * @param successful_messages Pointer to store successful messages
 * @param failed_messages Pointer to store failed messages
 * @return ESP_OK on success
 */
esp_err_t multiboard_get_stats(uint32_t* total_messages, uint32_t* successful_messages, uint32_t* failed_messages);

/**
 * @brief Reset coordinator statistics
 */
void multiboard_reset_stats(void);

/**
 * @brief Create default coordinator configuration
 * @param role Coordinator or node role
 * @param node_addr This node's I2C address
 * @return Default configuration
 */
multiboard_config_t multiboard_create_default_config(multiboard_role_t role, uint8_t node_addr);

/**
 * @brief Process pending multi-board operations (call from main loop)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t multiboard_process(void);

#ifdef __cplusplus
}
#endif

#endif // I2C_COORDINATOR_H