/**
 * @file quantum_key_manager.h
 * @brief Quantum-Safe Key Management System
 * @version 1.0.0
 * 
 * Manages quantum-resistant encryption keys with support for:
 * - Post-quantum key lifecycle management
 * - Hybrid classical-quantum key storage
 * - Automatic key rotation
 * - Key escrow and recovery
 * - Forward secrecy
 */

#ifndef QUANTUM_KEY_MANAGER_H
#define QUANTUM_KEY_MANAGER_H

#include <Arduino.h>
#include "quantum_safe_crypto.h"
#include <vector>
#include <map>

/**
 * @brief Key usage types
 */
enum class KeyUsageType {
    DATA_ENCRYPTION,        // Data encryption keys
    SIGNATURE,              // Digital signature keys
    KEY_EXCHANGE,           // Key exchange/agreement
    AUTHENTICATION,         // Authentication tokens
    INTEGRITY,              // Integrity verification
    BACKUP                  // Backup/recovery keys
};

/**
 * @brief Key status
 */
enum class KeyStatus {
    ACTIVE,                 // Currently active
    ROTATING,               // In rotation process
    DEPRECATED,             // Deprecated but still valid for decryption
    EXPIRED,                // Expired and invalid
    COMPROMISED,            // Potentially compromised
    REVOKED                 // Revoked and must not be used
};

/**
 * @brief Key metadata
 */
struct KeyMetadata {
    String key_id;                      // Unique key identifier
    uint32_t version;                   // Key version
    KeyUsageType usage;                 // Intended usage
    KeyStatus status;                   // Current status
    QuantumSecurityLevel security_level; // Security level
    uint32_t created_at;                // Creation timestamp
    uint32_t expires_at;                // Expiration timestamp
    uint32_t rotation_interval;         // Auto-rotation interval (ms)
    uint32_t usage_count;               // Number of times used
    uint32_t max_usage;                 // Maximum usage count
    bool allow_export;                  // Whether key can be exported
};

/**
 * @brief Key storage entry
 */
struct KeyEntry {
    KeyMetadata metadata;
    uint8_t key_material[256];          // Key material (encrypted at rest)
    size_t key_len;                     // Actual key length
    uint8_t checksum[32];               // Integrity checksum
    bool is_encrypted;                  // Whether stored encrypted
};

/**
 * @brief Key rotation policy
 */
struct KeyRotationPolicy {
    bool auto_rotation_enabled;         // Enable automatic rotation
    uint32_t rotation_interval_hours;   // Rotation interval in hours
    uint32_t max_key_age_hours;         // Maximum key age before forced rotation
    uint32_t overlap_period_hours;      // Overlap period for smooth transition
    bool require_manual_approval;       // Require manual approval for rotation
};

/**
 * @brief Key backup configuration
 */
struct KeyBackupConfig {
    bool backup_enabled;                // Enable key backup
    uint8_t backup_copies;              // Number of backup copies
    bool encrypt_backups;               // Encrypt backup data
    bool offsite_backup;                // Enable offsite backup
    uint32_t backup_interval_hours;     // Backup interval
};

/**
 * @brief Quantum Key Manager
 * 
 * Manages quantum-resistant keys with lifecycle management, rotation,
 * and recovery capabilities.
 */
class QuantumKeyManager {
private:
    // Key storage
    std::map<String, KeyEntry> key_store_;
    String active_encryption_key_id_;
    String active_signature_key_id_;
    
    // Master key for encrypting stored keys
    uint8_t master_encryption_key_[64];
    bool master_key_initialized_;
    
    // Policies
    KeyRotationPolicy rotation_policy_;
    KeyBackupConfig backup_config_;
    
    // Crypto instance
    QuantumSafeCrypto* crypto_;
    bool crypto_owned_;
    
    // Metrics
    uint32_t total_keys_created_;
    uint32_t total_keys_rotated_;
    uint32_t total_keys_expired_;
    uint32_t failed_operations_;
    
    // Internal methods
    bool encryptKeyMaterial(const uint8_t* key, size_t key_len,
                           uint8_t* encrypted, size_t* enc_len);
    
    bool decryptKeyMaterial(const uint8_t* encrypted, size_t enc_len,
                           uint8_t* key, size_t* key_len);
    
    bool computeKeyChecksum(const KeyEntry& entry, uint8_t checksum[32]);
    
    bool verifyKeyIntegrity(const KeyEntry& entry);
    
    String generateKeyId(KeyUsageType usage);
    
    bool shouldRotateKey(const KeyMetadata& metadata);
    
    void updateKeyUsageCount(const String& key_id);
    
    bool backupKey(const KeyEntry& entry);

public:
    /**
     * @brief Constructor
     * @param crypto Optional QuantumSafeCrypto instance
     */
    QuantumKeyManager(QuantumSafeCrypto* crypto = nullptr);
    
    /**
     * @brief Destructor
     */
    ~QuantumKeyManager();
    
    /**
     * @brief Initialize key manager
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Generate new quantum-safe key
     * @param usage Key usage type
     * @param security_level Desired security level
     * @param key_id Output key identifier
     * @return true if generation successful
     */
    bool generateKey(KeyUsageType usage, 
                    QuantumSecurityLevel security_level,
                    String& key_id);
    
    /**
     * @brief Get key material by ID
     * @param key_id Key identifier
     * @param key_material Output buffer
     * @param key_len Output length
     * @return true if key retrieved successfully
     */
    bool getKey(const String& key_id, uint8_t* key_material, size_t* key_len);
    
    /**
     * @brief Get key metadata
     * @param key_id Key identifier
     * @param metadata Output metadata
     * @return true if metadata retrieved
     */
    bool getKeyMetadata(const String& key_id, KeyMetadata& metadata);
    
    /**
     * @brief Rotate specific key
     * @param key_id Key to rotate
     * @param new_key_id Output new key identifier
     * @return true if rotation successful
     */
    bool rotateKey(const String& key_id, String& new_key_id);
    
    /**
     * @brief Rotate all keys of specific type
     * @param usage Key usage type to rotate
     * @return Number of keys rotated
     */
    uint32_t rotateAllKeys(KeyUsageType usage);
    
    /**
     * @brief Perform automatic key rotation check
     * @return Number of keys rotated
     */
    uint32_t performAutoRotation();
    
    /**
     * @brief Revoke key
     * @param key_id Key to revoke
     * @param reason Revocation reason
     * @return true if revocation successful
     */
    bool revokeKey(const String& key_id, const String& reason);
    
    /**
     * @brief Export key (if allowed)
     * @param key_id Key identifier
     * @param exported_key Output buffer
     * @param key_len Output length
     * @return true if export successful
     */
    bool exportKey(const String& key_id, uint8_t* exported_key, size_t* key_len);
    
    /**
     * @brief Import key
     * @param key_material Key material
     * @param key_len Key length
     * @param usage Key usage type
     * @param key_id Output key identifier
     * @return true if import successful
     */
    bool importKey(const uint8_t* key_material, size_t key_len,
                  KeyUsageType usage, String& key_id);
    
    /**
     * @brief Set rotation policy
     * @param policy Rotation policy
     */
    void setRotationPolicy(const KeyRotationPolicy& policy);
    
    /**
     * @brief Get rotation policy
     * @return Current rotation policy
     */
    KeyRotationPolicy getRotationPolicy() const { return rotation_policy_; }
    
    /**
     * @brief Set backup configuration
     * @param config Backup configuration
     */
    void setBackupConfig(const KeyBackupConfig& config);
    
    /**
     * @brief Get backup configuration
     * @return Current backup configuration
     */
    KeyBackupConfig getBackupConfig() const { return backup_config_; }
    
    /**
     * @brief Get active encryption key ID
     * @return Active encryption key ID
     */
    String getActiveEncryptionKeyId() const { return active_encryption_key_id_; }
    
    /**
     * @brief Get active signature key ID
     * @return Active signature key ID
     */
    String getActiveSignatureKeyId() const { return active_signature_key_id_; }
    
    /**
     * @brief List all keys
     * @param usage Optional filter by usage type
     * @return Vector of key IDs
     */
    std::vector<String> listKeys(KeyUsageType usage = KeyUsageType::DATA_ENCRYPTION) const;
    
    /**
     * @brief Get total number of keys
     * @return Number of keys stored
     */
    size_t getKeyCount() const { return key_store_.size(); }
    
    /**
     * @brief Check if key manager is operational
     * @return true if operational
     */
    bool isOperational() const { return master_key_initialized_; }
    
    /**
     * @brief Get statistics
     * @param created Output total keys created
     * @param rotated Output total keys rotated
     * @param expired Output total keys expired
     * @param failed Output failed operations
     */
    void getStatistics(uint32_t* created, uint32_t* rotated, 
                      uint32_t* expired, uint32_t* failed);
    
    /**
     * @brief Backup all keys
     * @return true if backup successful
     */
    bool backupAllKeys();
    
    /**
     * @brief Restore keys from backup
     * @return true if restore successful
     */
    bool restoreFromBackup();
    
    /**
     * @brief Clean expired keys
     * @return Number of keys cleaned
     */
    uint32_t cleanExpiredKeys();
    
    /**
     * @brief Verify integrity of all stored keys
     * @return true if all keys are valid
     */
    bool verifyAllKeys();
};

#endif // QUANTUM_KEY_MANAGER_H
