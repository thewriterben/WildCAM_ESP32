/**
 * @file mesh_config.h
 * @brief Mesh Network Configuration for Wildlife Monitoring
 * 
 * Defines configuration parameters for the mesh networking system
 * including multi-hop communication, encryption, and optimization settings.
 */

#ifndef MESH_CONFIG_H
#define MESH_CONFIG_H

#include <stdint.h>

// ===========================
// MESH NETWORK PARAMETERS
// ===========================

#define MESH_MAX_HOPS               5       // Maximum hop count for mesh routing
#define MESH_NODE_TIMEOUT           300000  // Node timeout in milliseconds (5 minutes)
#define MESH_ROUTE_TIMEOUT          600000  // Route timeout in milliseconds (10 minutes)
#define MESH_BEACON_INTERVAL        30000   // Beacon interval in milliseconds (30 seconds)
#define MESH_MAX_NODES              32      // Maximum number of nodes in mesh
#define MESH_MAX_PACKET_SIZE        256     // Maximum packet size in bytes
#define MESH_RETRY_ATTEMPTS         3       // Number of retry attempts for failed transmissions

// ===========================
// ENCRYPTION CONFIGURATION
// ===========================

#define MESH_ENCRYPTION_ENABLED     true    // Enable AES-256 encryption
#define MESH_KEY_ROTATION_INTERVAL  86400000 // Key rotation interval (24 hours)
#define MESH_KEY_SIZE               32      // AES-256 key size in bytes

// ===========================
// BANDWIDTH OPTIMIZATION
// ===========================

#define MESH_MAX_BANDWIDTH          125000  // Maximum bandwidth in bytes/second (1 Mbps)
#define MESH_PRIORITY_LEVELS        4       // Number of priority levels for traffic
#define MESH_CONGESTION_THRESHOLD   0.8     // Congestion threshold (80% utilization)
#define MESH_LOAD_BALANCE_ENABLED   true    // Enable load balancing across paths

// ===========================
// PACKET TYPES
// ===========================

typedef enum {
    MESH_PACKET_DATA = 0x01,        // Regular data packet
    MESH_PACKET_BEACON = 0x02,      // Network discovery beacon
    MESH_PACKET_ROUTING = 0x03,     // Routing information
    MESH_PACKET_ACK = 0x04,         // Acknowledgment packet
    MESH_PACKET_IMAGE = 0x05,       // Wildlife image data
    MESH_PACKET_TELEMETRY = 0x06,   // Environmental telemetry
    MESH_PACKET_EMERGENCY = 0x07    // Emergency/priority packet
} MeshPacketType;

// ===========================
// PRIORITY LEVELS
// ===========================

typedef enum {
    MESH_PRIORITY_LOW = 0,          // Background data
    MESH_PRIORITY_NORMAL = 1,       // Regular telemetry
    MESH_PRIORITY_HIGH = 2,         // Wildlife detection alerts
    MESH_PRIORITY_EMERGENCY = 3     // System emergencies
} MeshPriority;

// ===========================
// MESH ROLES
// ===========================

typedef enum {
    MESH_ROLE_NODE = 0,             // Regular mesh node
    MESH_ROLE_COORDINATOR = 1,      // Network coordinator
    MESH_ROLE_GATEWAY = 2,          // Gateway to external networks
    MESH_ROLE_REPEATER = 3          // Dedicated repeater node
} MeshRole;

// ===========================
// CONFIGURATION STRUCTURE
// ===========================

typedef struct {
    uint32_t nodeId;                // Unique node identifier
    MeshRole role;                  // Node role in mesh network
    uint8_t channel;                // Radio channel (0-255)
    uint16_t networkId;             // Network identifier
    uint8_t encryptionKey[MESH_KEY_SIZE]; // Encryption key
    bool loadBalancingEnabled;      // Load balancing flag
    uint8_t maxHops;                // Maximum hops for this node
    uint32_t beaconInterval;        // Beacon transmission interval
    float congestionThreshold;      // Congestion detection threshold
} MeshConfig;

// ===========================
// WILDLIFE-SPECIFIC SETTINGS
// ===========================

#define MESH_WILDLIFE_IMAGE_CHUNK_SIZE  1024    // Image chunk size for transmission
#define MESH_WILDLIFE_DETECTION_PRIORITY MESH_PRIORITY_HIGH
#define MESH_WILDLIFE_TELEMETRY_INTERVAL 60000  // Telemetry interval (1 minute)
#define MESH_WILDLIFE_GPS_SYNC_INTERVAL  300000 // GPS sync interval (5 minutes)

// ===========================
// POWER OPTIMIZATION
// ===========================

#define MESH_SLEEP_MODE_ENABLED     true    // Enable sleep mode for power saving
#define MESH_WAKE_INTERVAL          1000    // Wake interval in milliseconds
#define MESH_LOW_POWER_THRESHOLD    10      // Battery percentage for low power mode

#endif // MESH_CONFIG_H