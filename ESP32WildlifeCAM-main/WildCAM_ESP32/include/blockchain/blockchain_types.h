#ifndef BLOCKCHAIN_TYPES_H
#define BLOCKCHAIN_TYPES_H

#include <Arduino.h>

/**
 * @brief Common blockchain data types and enumerations
 * 
 * Shared types used across blockchain system components
 * for the ESP32 Wildlife CAM implementation.
 */

// ===========================
// BLOCKCHAIN ENUMERATIONS
// ===========================

/**
 * @brief Blockchain operation status codes
 */
enum BlockchainStatus {
    BLOCKCHAIN_SUCCESS = 0,              // Operation successful
    BLOCKCHAIN_ERROR_INIT_FAILED,        // Initialization failed
    BLOCKCHAIN_ERROR_INVALID_BLOCK,      // Invalid block structure
    BLOCKCHAIN_ERROR_INVALID_TRANSACTION, // Invalid transaction
    BLOCKCHAIN_ERROR_HASH_MISMATCH,      // Hash verification failed
    BLOCKCHAIN_ERROR_STORAGE_FULL,       // Storage space exhausted
    BLOCKCHAIN_ERROR_NETWORK_FAILURE,    // Network operation failed
    BLOCKCHAIN_ERROR_MEMORY_EXHAUSTED,   // Insufficient memory
    BLOCKCHAIN_ERROR_INVALID_CONFIG,     // Invalid configuration
    BLOCKCHAIN_ERROR_FILE_IO,            // File I/O error
    BLOCKCHAIN_ERROR_TIMEOUT,            // Operation timeout
    BLOCKCHAIN_ERROR_DISABLED           // Blockchain disabled
};

/**
 * @brief Transaction priority levels
 */
enum TransactionPriority {
    PRIORITY_CRITICAL = 1,               // Critical data (images, AI detections)
    PRIORITY_IMPORTANT = 2,              // Important data (sensors, power)
    PRIORITY_NORMAL = 3,                 // Normal data (system events)
    PRIORITY_LOW = 4                     // Low priority data (debug, logs)
};

/**
 * @brief Block validation levels
 */
enum ValidationLevel {
    VALIDATION_NONE = 0,                 // No validation
    VALIDATION_BASIC = 1,                // Basic structure validation
    VALIDATION_STANDARD = 2,             // Standard validation with hashes
    VALIDATION_STRICT = 3,               // Strict validation with signatures
    VALIDATION_PARANOID = 4              // Full cryptographic validation
};

/**
 * @brief Network synchronization modes
 */
enum SyncMode {
    SYNC_DISABLED = 0,                   // No synchronization
    SYNC_PASSIVE = 1,                    // Receive only
    SYNC_ACTIVE = 2,                     // Send and receive
    SYNC_AGGRESSIVE = 3                  // Continuous sync
};

/**
 * @brief Hash frequency modes
 */
enum HashFrequency {
    HASH_NEVER = 0,                      // Disabled
    HASH_EVERY_CAPTURE = 1,              // Hash every capture
    HASH_EVERY_SECOND = 2,               // Hash every second capture
    HASH_EVERY_FIFTH = 5,                // Hash every fifth capture
    HASH_EVERY_TENTH = 10,               // Hash every tenth capture
    HASH_TIME_BASED = 99                 // Time-based hashing
};

// ===========================
// BLOCKCHAIN STRUCTURES
// ===========================

/**
 * @brief Blockchain performance metrics
 */
struct BlockchainMetrics {
    uint32_t totalOperations;            // Total operations performed
    uint32_t successfulOperations;       // Successful operations
    uint32_t failedOperations;           // Failed operations
    float averageOperationTime;          // Average operation time (ms)
    float hashPerformance;               // Hashes per second
    uint32_t blocksCreated;              // Blocks created
    uint32_t transactionsProcessed;      // Transactions processed
    size_t memoryUsage;                  // Current memory usage
    size_t storageUsage;                 // Storage space used
    uint32_t networkSyncCount;           // Network sync operations
    uint32_t errorCount;                 // Total errors encountered
    String lastError;                    // Last error message
    uint32_t uptime;                     // System uptime (seconds)
    
    BlockchainMetrics() : totalOperations(0), successfulOperations(0), 
        failedOperations(0), averageOperationTime(0.0), hashPerformance(0.0),
        blocksCreated(0), transactionsProcessed(0), memoryUsage(0), 
        storageUsage(0), networkSyncCount(0), errorCount(0), lastError(""),
        uptime(0) {}
};

/**
 * @brief Node identification and capabilities
 */
struct NodeInfo {
    String nodeId;                       // Unique node identifier
    String nodeType;                     // Node type (e.g., "ESP32CAM")
    String firmwareVersion;              // Firmware version
    String location;                     // Physical location
    float latitude;                      // GPS latitude
    float longitude;                     // GPS longitude
    uint32_t capabilities;               // Capability flags
    uint32_t lastSeen;                   // Last seen timestamp
    bool isActive;                       // Node active status
    String publicKey;                    // Public key for verification
    
    NodeInfo() : nodeId(""), nodeType("ESP32CAM"), firmwareVersion(""),
        location(""), latitude(0.0), longitude(0.0), capabilities(0),
        lastSeen(0), isActive(false), publicKey("") {}
};

/**
 * @brief Blockchain synchronization state
 */
struct SyncState {
    bool isActive;                       // Sync operation active
    uint32_t lastSyncTime;              // Last successful sync
    uint32_t nextSyncTime;              // Next scheduled sync
    uint32_t syncErrors;                // Sync error count
    String syncPeer;                    // Current sync peer
    uint32_t blocksToSync;              // Blocks remaining to sync
    uint32_t blocksSynced;              // Blocks successfully synced
    float syncProgress;                 // Sync progress percentage
    String syncStatus;                  // Human-readable status
    
    SyncState() : isActive(false), lastSyncTime(0), nextSyncTime(0),
        syncErrors(0), syncPeer(""), blocksToSync(0), blocksSynced(0),
        syncProgress(0.0), syncStatus("Idle") {}
};

/**
 * @brief Wildlife data context for blockchain transactions
 */
struct WildlifeContext {
    String speciesDetected;              // Detected species name
    float confidence;                    // Detection confidence
    String behaviorType;                 // Observed behavior
    String environmentalConditions;      // Weather/environment
    float temperature;                   // Temperature (°C)
    float humidity;                      // Humidity (%)
    float windSpeed;                     // Wind speed (km/h)
    String timeOfDay;                   // Time classification
    String season;                      // Season information
    bool isRareSpecies;                 // Rare species flag
    bool isProtectedSpecies;            // Protected species flag
    
    WildlifeContext() : speciesDetected(""), confidence(0.0), behaviorType(""),
        environmentalConditions(""), temperature(0.0), humidity(0.0),
        windSpeed(0.0), timeOfDay(""), season(""), isRareSpecies(false),
        isProtectedSpecies(false) {}
};

// ===========================
// UTILITY MACROS
// ===========================

// Status checking macros
#define BLOCKCHAIN_SUCCESS_CHECK(op) ((op) == BLOCKCHAIN_SUCCESS)
#define BLOCKCHAIN_FAILED(op) ((op) != BLOCKCHAIN_SUCCESS)

// Memory alignment for ESP32
#define BLOCKCHAIN_ALIGN_SIZE(size) (((size) + 3) & ~3)

// Time conversion utilities
#define BLOCKCHAIN_SECONDS_TO_MS(s) ((s) * 1000)
#define BLOCKCHAIN_MS_TO_SECONDS(ms) ((ms) / 1000)

// Hash validation macros
#define BLOCKCHAIN_VALID_HASH(hash) ((hash).length() == 64)
#define BLOCKCHAIN_EMPTY_HASH(hash) ((hash).isEmpty() || (hash) == "")

// Transaction validation
#define BLOCKCHAIN_VALID_CONFIDENCE(conf) ((conf) >= 0.0 && (conf) <= 1.0)
#define BLOCKCHAIN_VALID_TIMESTAMP(ts) ((ts) > 0)

// Node ID validation
#define BLOCKCHAIN_VALID_NODE_ID(id) (!(id).isEmpty() && (id).length() <= 32)

// Memory usage helpers
#define BLOCKCHAIN_KB_TO_BYTES(kb) ((kb) * 1024)
#define BLOCKCHAIN_MB_TO_BYTES(mb) ((mb) * 1024 * 1024)
#define BLOCKCHAIN_BYTES_TO_KB(bytes) ((bytes) / 1024)

// ===========================
// FUNCTION POINTER TYPES
// ===========================

// Callback function types for integration
typedef void (*BlockchainEventCallback)(const String& event, uint32_t timestamp);
typedef void (*BlockchainErrorCallback)(BlockchainStatus error, const String& details);
typedef bool (*BlockchainValidationCallback)(const String& data, const String& hash);
typedef void (*BlockchainSyncCallback)(const SyncState& state);
typedef void (*BlockchainMetricsCallback)(const BlockchainMetrics& metrics);

// ===========================
// CONSTANTS
// ===========================

// String constants for transaction types
static const char* TRANSACTION_TYPE_STRINGS[] = {
    "IMAGE_CAPTURE",
    "AI_DETECTION", 
    "SENSOR_DATA",
    "SYSTEM_EVENT",
    "POWER_EVENT",
    "NETWORK_EVENT"
};

// Default values
static const char* DEFAULT_NODE_ID = "ESP32CAM_UNKNOWN";
static const char* DEFAULT_CHAIN_NAME = "WildlifeCAM_Chain";
static const char* DEFAULT_GENESIS_MESSAGE = "ESP32 Wildlife CAM Genesis Block";

// Version information
static const char* BLOCKCHAIN_VERSION = "1.0.0";
static const uint32_t BLOCKCHAIN_PROTOCOL_VERSION = 1;

#endif // BLOCKCHAIN_TYPES_H