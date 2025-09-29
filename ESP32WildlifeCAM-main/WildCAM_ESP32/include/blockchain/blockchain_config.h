#ifndef BLOCKCHAIN_CONFIG_H
#define BLOCKCHAIN_CONFIG_H

/**
 * @brief Blockchain system configuration constants
 * 
 * Centralized configuration for blockchain operations optimized
 * for ESP32 Wildlife CAM memory and power constraints.
 */

// ===========================
// BLOCKCHAIN CORE CONFIGURATION
// ===========================

// Memory constraints
#define BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY     5      // Maximum blocks kept in RAM
#define BLOCKCHAIN_MAX_TRANSACTIONS_PER_BLOCK 10   // Maximum transactions per block
#define BLOCKCHAIN_MAX_BLOCK_SIZE          8192    // Maximum block size in bytes

// Timing configuration
#define BLOCKCHAIN_DEFAULT_BLOCK_INTERVAL   300    // Default block creation interval (seconds)
#define BLOCKCHAIN_MIN_BLOCK_INTERVAL       60     // Minimum block creation interval (seconds)
#define BLOCKCHAIN_MAX_BLOCK_INTERVAL       3600   // Maximum block creation interval (seconds)

// Hash configuration
#define BLOCKCHAIN_HASH_SIZE               32      // SHA-256 hash size in bytes
#define BLOCKCHAIN_HASH_HEX_SIZE           64      // Hex-encoded hash size
#define BLOCKCHAIN_DEFAULT_HASH_FREQUENCY   1      // Hash every capture by default

// Network configuration
#define BLOCKCHAIN_MAX_SYNC_NODES          10      // Maximum nodes to sync with
#define BLOCKCHAIN_SYNC_TIMEOUT           30000    // Network sync timeout (ms)
#define BLOCKCHAIN_RETRY_COUNT             3       // Number of retry attempts

// Storage configuration  
#define BLOCKCHAIN_STORAGE_DIR            "/blockchain"  // Storage directory on SD card
#define BLOCKCHAIN_BLOCK_FILE_PREFIX      "block_"       // Block file prefix
#define BLOCKCHAIN_CHAIN_FILE             "chain.json"   // Main chain file
#define BLOCKCHAIN_CONFIG_FILE            "config.json"  // Configuration file

// ===========================
// PERFORMANCE OPTIMIZATION
// ===========================

// Asynchronous operations
#define BLOCKCHAIN_ASYNC_OPERATIONS        true    // Enable async blockchain ops
#define BLOCKCHAIN_ASYNC_QUEUE_SIZE        20      // Async operation queue size
#define BLOCKCHAIN_ASYNC_TIMEOUT          5000     // Async operation timeout (ms)

// Selective hashing for battery optimization
#define BLOCKCHAIN_SELECTIVE_HASHING       true    // Enable selective hashing
#define BLOCKCHAIN_CRITICAL_DATA_ONLY      false   // Hash only critical data
#define BLOCKCHAIN_POWER_SAVE_MODE         true    // Enable power saving optimizations

// Memory optimization
#define BLOCKCHAIN_ENABLE_COMPRESSION      false   // Block compression (disabled for simplicity)
#define BLOCKCHAIN_LAZY_LOADING           true     // Load blocks on demand
#define BLOCKCHAIN_CACHE_SIZE             3        // Number of blocks to cache

// ===========================
// SECURITY CONFIGURATION
// ===========================

// Digital signatures
#define BLOCKCHAIN_ENABLE_SIGNATURES       false   // Digital signatures (disabled for now)
#define BLOCKCHAIN_SIGNATURE_ALGORITHM     "ECDSA" // Signature algorithm
#define BLOCKCHAIN_KEY_SIZE               256      // Key size in bits

// Integrity verification
#define BLOCKCHAIN_AUTO_VERIFY            true     // Auto-verify blockchain integrity
#define BLOCKCHAIN_VERIFY_INTERVAL        3600     // Verification interval (seconds)
#define BLOCKCHAIN_VERIFY_ON_STARTUP      true     // Verify on system startup

// Node authentication
#define BLOCKCHAIN_REQUIRE_NODE_AUTH       false   // Require node authentication
#define BLOCKCHAIN_NODE_ID_SIZE           16       // Node ID size in characters

// ===========================
// DATA CLASSIFICATION
// ===========================

// Transaction types priority
#define BLOCKCHAIN_IMAGE_PRIORITY          1       // High priority
#define BLOCKCHAIN_AI_PRIORITY             1       // High priority  
#define BLOCKCHAIN_SENSOR_PRIORITY         2       // Medium priority
#define BLOCKCHAIN_SYSTEM_PRIORITY         3       // Low priority
#define BLOCKCHAIN_POWER_PRIORITY          2       // Medium priority
#define BLOCKCHAIN_NETWORK_PRIORITY        3       // Low priority

// Data retention
#define BLOCKCHAIN_KEEP_ALL_BLOCKS         true    // Keep all blocks (research requirement)
#define BLOCKCHAIN_MAX_CHAIN_LENGTH        0       // Unlimited (0 = no limit)
#define BLOCKCHAIN_ARCHIVE_OLD_BLOCKS      false   // Archive vs delete old blocks

// ===========================
// INTEGRATION SETTINGS
// ===========================

// Storage manager integration
#define BLOCKCHAIN_USE_STORAGE_MANAGER     true    // Integrate with existing StorageManager
#define BLOCKCHAIN_STORAGE_WARNING_THRESHOLD 90    // Warning when storage > 90% full
#define BLOCKCHAIN_AUTO_CLEANUP           true     // Auto cleanup when storage full

// Networking integration
#define BLOCKCHAIN_USE_EXISTING_NETWORK    true    // Use existing network infrastructure
#define BLOCKCHAIN_MESH_SYNC              true     // Sync via LoRa mesh network
#define BLOCKCHAIN_WIFI_SYNC              true     // Sync via WiFi when available
#define BLOCKCHAIN_CELLULAR_SYNC          false    // Sync via cellular (disabled for power)

// AI processor integration
#define BLOCKCHAIN_AI_CONFIDENCE_THRESHOLD 0.7     // Minimum confidence to hash AI results
#define BLOCKCHAIN_AI_SPECIES_FILTERING    true    // Only hash specific species
#define BLOCKCHAIN_AI_BEHAVIOR_TRACKING    true    // Track behavior patterns

// ===========================
// DEBUGGING AND MONITORING
// ===========================

// Debug configuration
#define BLOCKCHAIN_DEBUG_ENABLED           true    // Enable debug logging
#define BLOCKCHAIN_VERBOSE_LOGGING         false   // Verbose debug output
#define BLOCKCHAIN_PERFORMANCE_MONITORING  true    // Monitor performance metrics

// Status reporting
#define BLOCKCHAIN_STATUS_UPDATE_INTERVAL  60      // Status update interval (seconds)
#define BLOCKCHAIN_HEALTH_CHECK_INTERVAL   300     // Health check interval (seconds)
#define BLOCKCHAIN_WEB_INTERFACE           true    // Enable web interface display

// Error handling
#define BLOCKCHAIN_MAX_CONSECUTIVE_ERRORS   5      // Max errors before disable
#define BLOCKCHAIN_ERROR_RECOVERY_TIMEOUT  300    // Error recovery timeout (seconds)
#define BLOCKCHAIN_FAILSAFE_MODE           true    // Continue camera ops on blockchain failure

// ===========================
// VALIDATION CONSTANTS
// ===========================

// Hash validation
#define BLOCKCHAIN_VALIDATE_ALL_HASHES     true    // Validate all hash calculations
#define BLOCKCHAIN_DOUBLE_CHECK_CRITICAL   true    // Double-check critical hashes
#define BLOCKCHAIN_HASH_PERFORMANCE_TARGET 100     // Target hashes per second

// Block validation
#define BLOCKCHAIN_VALIDATE_BLOCK_STRUCTURE true   // Validate block structure
#define BLOCKCHAIN_VALIDATE_MERKLE_TREES   true    // Validate merkle tree calculations
#define BLOCKCHAIN_VALIDATE_TIMESTAMPS     true    // Validate timestamp sequences

// Chain validation
#define BLOCKCHAIN_VALIDATE_CHAIN_INTEGRITY true   // Validate entire chain
#define BLOCKCHAIN_VALIDATE_BLOCK_LINKS    true    // Validate block linking
#define BLOCKCHAIN_DETECT_TAMPERING        true    // Detect tampering attempts

#endif // BLOCKCHAIN_CONFIG_H