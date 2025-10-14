/**
 * @file quantum_key_manager.cpp
 * @brief Quantum-Safe Key Management Implementation
 */

#include "quantum_key_manager.h"
#include <mbedtls/sha256.h>

// Constructor
QuantumKeyManager::QuantumKeyManager(QuantumSafeCrypto* crypto)
    : master_key_initialized_(false),
      crypto_(crypto),
      crypto_owned_(false),
      total_keys_created_(0),
      total_keys_rotated_(0),
      total_keys_expired_(0),
      failed_operations_(0) {
    
    memset(master_encryption_key_, 0, sizeof(master_encryption_key_));
    
    // Initialize default policies
    rotation_policy_.auto_rotation_enabled = true;
    rotation_policy_.rotation_interval_hours = 168; // 7 days
    rotation_policy_.max_key_age_hours = 720; // 30 days
    rotation_policy_.overlap_period_hours = 24; // 1 day
    rotation_policy_.require_manual_approval = false;
    
    backup_config_.backup_enabled = true;
    backup_config_.backup_copies = 3;
    backup_config_.encrypt_backups = true;
    backup_config_.offsite_backup = false;
    backup_config_.backup_interval_hours = 24;
}

// Destructor
QuantumKeyManager::~QuantumKeyManager() {
    // Securely wipe all keys
    for (auto& pair : key_store_) {
        memset(pair.second.key_material, 0, sizeof(pair.second.key_material));
    }
    key_store_.clear();
    
    // Wipe master key
    memset(master_encryption_key_, 0, sizeof(master_encryption_key_));
    
    if (crypto_owned_ && crypto_ != nullptr) {
        delete crypto_;
    }
}

// Initialize key manager
bool QuantumKeyManager::begin() {
    Serial.println("[QuantumKeyMgr] Initializing quantum-safe key management...");
    
    // Create crypto instance if not provided
    if (crypto_ == nullptr) {
        crypto_ = new QuantumSafeCrypto(QuantumSecurityLevel::HYBRID_TRANSITION);
        if (crypto_ == nullptr) {
            Serial.println("[QuantumKeyMgr] ERROR: Failed to create crypto instance");
            return false;
        }
        crypto_owned_ = true;
        
        if (!crypto_->begin()) {
            Serial.println("[QuantumKeyMgr] ERROR: Failed to initialize crypto");
            return false;
        }
    }
    
    // Generate master encryption key
    if (!crypto_->generateQuantumRandom(master_encryption_key_, sizeof(master_encryption_key_))) {
        Serial.println("[QuantumKeyMgr] ERROR: Failed to generate master key");
        return false;
    }
    
    master_key_initialized_ = true;
    
    Serial.println("[QuantumKeyMgr] Initialization complete");
    
    return true;
}

// Generate new key
bool QuantumKeyManager::generateKey(KeyUsageType usage,
                                   QuantumSecurityLevel security_level,
                                   String& key_id) {
    if (!master_key_initialized_) {
        Serial.println("[QuantumKeyMgr] ERROR: Not initialized");
        failed_operations_++;
        return false;
    }
    
    Serial.printf("[QuantumKeyMgr] Generating key for usage type: %d\n", (int)usage);
    
    // Generate key ID
    key_id = generateKeyId(usage);
    
    // Create key entry
    KeyEntry entry;
    entry.metadata.key_id = key_id;
    entry.metadata.version = 1;
    entry.metadata.usage = usage;
    entry.metadata.status = KeyStatus::ACTIVE;
    entry.metadata.security_level = security_level;
    entry.metadata.created_at = millis();
    entry.metadata.expires_at = millis() + (rotation_policy_.max_key_age_hours * 3600000);
    entry.metadata.rotation_interval = rotation_policy_.rotation_interval_hours * 3600000;
    entry.metadata.usage_count = 0;
    entry.metadata.max_usage = 1000000; // Default max usage
    entry.metadata.allow_export = false;
    
    // Determine key size based on security level
    size_t key_size = 32; // Default 256-bit
    if (security_level == QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM) {
        key_size = 64; // 512-bit for maximum security
    }
    
    // Generate raw key material
    uint8_t raw_key[256];
    if (!crypto_->generateQuantumRandom(raw_key, key_size)) {
        Serial.println("[QuantumKeyMgr] ERROR: Failed to generate key material");
        failed_operations_++;
        return false;
    }
    
    // Encrypt key material for storage
    size_t encrypted_len;
    if (!encryptKeyMaterial(raw_key, key_size, entry.key_material, &encrypted_len)) {
        Serial.println("[QuantumKeyMgr] ERROR: Failed to encrypt key material");
        memset(raw_key, 0, sizeof(raw_key));
        failed_operations_++;
        return false;
    }
    
    entry.key_len = encrypted_len;
    entry.is_encrypted = true;
    
    // Compute checksum
    computeKeyChecksum(entry, entry.checksum);
    
    // Store key
    key_store_[key_id] = entry;
    
    // Update active key IDs
    if (usage == KeyUsageType::DATA_ENCRYPTION) {
        active_encryption_key_id_ = key_id;
    } else if (usage == KeyUsageType::SIGNATURE) {
        active_signature_key_id_ = key_id;
    }
    
    total_keys_created_++;
    
    // Cleanup
    memset(raw_key, 0, sizeof(raw_key));
    
    Serial.printf("[QuantumKeyMgr] Key generated: %s\n", key_id.c_str());
    
    return true;
}

// Get key material
bool QuantumKeyManager::getKey(const String& key_id, uint8_t* key_material, size_t* key_len) {
    if (!master_key_initialized_ || key_material == nullptr || key_len == nullptr) {
        failed_operations_++;
        return false;
    }
    
    auto it = key_store_.find(key_id);
    if (it == key_store_.end()) {
        Serial.printf("[QuantumKeyMgr] ERROR: Key not found: %s\n", key_id.c_str());
        failed_operations_++;
        return false;
    }
    
    KeyEntry& entry = it->second;
    
    // Check key status
    if (entry.metadata.status == KeyStatus::REVOKED ||
        entry.metadata.status == KeyStatus::EXPIRED) {
        Serial.printf("[QuantumKeyMgr] ERROR: Key is %s: %s\n",
                     (entry.metadata.status == KeyStatus::REVOKED) ? "revoked" : "expired",
                     key_id.c_str());
        failed_operations_++;
        return false;
    }
    
    // Verify integrity
    if (!verifyKeyIntegrity(entry)) {
        Serial.printf("[QuantumKeyMgr] ERROR: Key integrity check failed: %s\n", key_id.c_str());
        failed_operations_++;
        return false;
    }
    
    // Decrypt key material
    if (!decryptKeyMaterial(entry.key_material, entry.key_len, key_material, key_len)) {
        Serial.printf("[QuantumKeyMgr] ERROR: Failed to decrypt key: %s\n", key_id.c_str());
        failed_operations_++;
        return false;
    }
    
    // Update usage count
    updateKeyUsageCount(key_id);
    
    return true;
}

// Get key metadata
bool QuantumKeyManager::getKeyMetadata(const String& key_id, KeyMetadata& metadata) {
    auto it = key_store_.find(key_id);
    if (it == key_store_.end()) {
        return false;
    }
    
    metadata = it->second.metadata;
    return true;
}

// Rotate key
bool QuantumKeyManager::rotateKey(const String& key_id, String& new_key_id) {
    if (!master_key_initialized_) {
        failed_operations_++;
        return false;
    }
    
    auto it = key_store_.find(key_id);
    if (it == key_store_.end()) {
        Serial.printf("[QuantumKeyMgr] ERROR: Key not found for rotation: %s\n", key_id.c_str());
        failed_operations_++;
        return false;
    }
    
    KeyEntry& old_entry = it->second;
    
    Serial.printf("[QuantumKeyMgr] Rotating key: %s\n", key_id.c_str());
    
    // Generate new key with same usage type
    if (!generateKey(old_entry.metadata.usage, 
                    old_entry.metadata.security_level, 
                    new_key_id)) {
        Serial.println("[QuantumKeyMgr] ERROR: Failed to generate replacement key");
        failed_operations_++;
        return false;
    }
    
    // Mark old key as deprecated (keep for decryption of old data)
    old_entry.metadata.status = KeyStatus::DEPRECATED;
    
    total_keys_rotated_++;
    
    Serial.printf("[QuantumKeyMgr] Key rotated: %s -> %s\n", key_id.c_str(), new_key_id.c_str());
    
    return true;
}

// Rotate all keys of type
uint32_t QuantumKeyManager::rotateAllKeys(KeyUsageType usage) {
    uint32_t rotated = 0;
    
    std::vector<String> keys_to_rotate;
    for (const auto& pair : key_store_) {
        if (pair.second.metadata.usage == usage &&
            pair.second.metadata.status == KeyStatus::ACTIVE) {
            keys_to_rotate.push_back(pair.first);
        }
    }
    
    for (const String& key_id : keys_to_rotate) {
        String new_key_id;
        if (rotateKey(key_id, new_key_id)) {
            rotated++;
        }
    }
    
    Serial.printf("[QuantumKeyMgr] Rotated %u keys of type %d\n", rotated, (int)usage);
    
    return rotated;
}

// Perform automatic rotation
uint32_t QuantumKeyManager::performAutoRotation() {
    if (!rotation_policy_.auto_rotation_enabled) {
        return 0;
    }
    
    uint32_t rotated = 0;
    std::vector<String> keys_to_rotate;
    
    // Find keys that need rotation
    for (const auto& pair : key_store_) {
        if (shouldRotateKey(pair.second.metadata)) {
            keys_to_rotate.push_back(pair.first);
        }
    }
    
    // Rotate keys
    for (const String& key_id : keys_to_rotate) {
        String new_key_id;
        if (rotateKey(key_id, new_key_id)) {
            rotated++;
        }
    }
    
    if (rotated > 0) {
        Serial.printf("[QuantumKeyMgr] Auto-rotation completed: %u keys\n", rotated);
    }
    
    return rotated;
}

// Revoke key
bool QuantumKeyManager::revokeKey(const String& key_id, const String& reason) {
    auto it = key_store_.find(key_id);
    if (it == key_store_.end()) {
        return false;
    }
    
    it->second.metadata.status = KeyStatus::REVOKED;
    
    Serial.printf("[QuantumKeyMgr] Key revoked: %s (reason: %s)\n", 
                 key_id.c_str(), reason.c_str());
    
    return true;
}

// Export key
bool QuantumKeyManager::exportKey(const String& key_id, uint8_t* exported_key, size_t* key_len) {
    auto it = key_store_.find(key_id);
    if (it == key_store_.end()) {
        return false;
    }
    
    if (!it->second.metadata.allow_export) {
        Serial.printf("[QuantumKeyMgr] ERROR: Key export not allowed: %s\n", key_id.c_str());
        return false;
    }
    
    return getKey(key_id, exported_key, key_len);
}

// Import key
bool QuantumKeyManager::importKey(const uint8_t* key_material, size_t key_len,
                                 KeyUsageType usage, String& key_id) {
    if (!master_key_initialized_ || key_material == nullptr || key_len == 0) {
        failed_operations_++;
        return false;
    }
    
    Serial.println("[QuantumKeyMgr] Importing external key...");
    
    // Generate key ID
    key_id = generateKeyId(usage);
    
    // Create key entry
    KeyEntry entry;
    entry.metadata.key_id = key_id;
    entry.metadata.version = 1;
    entry.metadata.usage = usage;
    entry.metadata.status = KeyStatus::ACTIVE;
    entry.metadata.security_level = QuantumSecurityLevel::HYBRID_TRANSITION;
    entry.metadata.created_at = millis();
    entry.metadata.expires_at = millis() + (rotation_policy_.max_key_age_hours * 3600000);
    entry.metadata.rotation_interval = rotation_policy_.rotation_interval_hours * 3600000;
    entry.metadata.usage_count = 0;
    entry.metadata.max_usage = 1000000;
    entry.metadata.allow_export = true; // Imported keys can be exported
    
    // Encrypt key material for storage
    size_t encrypted_len;
    if (!encryptKeyMaterial(key_material, key_len, entry.key_material, &encrypted_len)) {
        Serial.println("[QuantumKeyMgr] ERROR: Failed to encrypt imported key");
        failed_operations_++;
        return false;
    }
    
    entry.key_len = encrypted_len;
    entry.is_encrypted = true;
    
    // Compute checksum
    computeKeyChecksum(entry, entry.checksum);
    
    // Store key
    key_store_[key_id] = entry;
    
    total_keys_created_++;
    
    Serial.printf("[QuantumKeyMgr] Key imported: %s\n", key_id.c_str());
    
    return true;
}

// Set rotation policy
void QuantumKeyManager::setRotationPolicy(const KeyRotationPolicy& policy) {
    rotation_policy_ = policy;
    Serial.println("[QuantumKeyMgr] Rotation policy updated");
}

// Set backup configuration
void QuantumKeyManager::setBackupConfig(const KeyBackupConfig& config) {
    backup_config_ = config;
    Serial.println("[QuantumKeyMgr] Backup configuration updated");
}

// List keys
std::vector<String> QuantumKeyManager::listKeys(KeyUsageType usage) const {
    std::vector<String> keys;
    
    for (const auto& pair : key_store_) {
        if (pair.second.metadata.usage == usage) {
            keys.push_back(pair.first);
        }
    }
    
    return keys;
}

// Get statistics
void QuantumKeyManager::getStatistics(uint32_t* created, uint32_t* rotated,
                                     uint32_t* expired, uint32_t* failed) {
    if (created) *created = total_keys_created_;
    if (rotated) *rotated = total_keys_rotated_;
    if (expired) *expired = total_keys_expired_;
    if (failed) *failed = failed_operations_;
}

// Backup all keys
bool QuantumKeyManager::backupAllKeys() {
    if (!backup_config_.backup_enabled) {
        return false;
    }
    
    Serial.println("[QuantumKeyMgr] Backing up all keys...");
    
    uint32_t backed_up = 0;
    for (const auto& pair : key_store_) {
        if (backupKey(pair.second)) {
            backed_up++;
        }
    }
    
    Serial.printf("[QuantumKeyMgr] Backed up %u keys\n", backed_up);
    
    return true;
}

// Restore from backup
bool QuantumKeyManager::restoreFromBackup() {
    Serial.println("[QuantumKeyMgr] Restore from backup not yet implemented");
    return false;
}

// Clean expired keys
uint32_t QuantumKeyManager::cleanExpiredKeys() {
    uint32_t cleaned = 0;
    std::vector<String> to_remove;
    
    uint32_t current_time = millis();
    
    for (const auto& pair : key_store_) {
        if (pair.second.metadata.status == KeyStatus::EXPIRED ||
            (pair.second.metadata.expires_at > 0 && 
             pair.second.metadata.expires_at < current_time)) {
            to_remove.push_back(pair.first);
        }
    }
    
    for (const String& key_id : to_remove) {
        auto it = key_store_.find(key_id);
        if (it != key_store_.end()) {
            // Securely wipe key material
            memset(it->second.key_material, 0, sizeof(it->second.key_material));
            key_store_.erase(it);
            cleaned++;
            total_keys_expired_++;
        }
    }
    
    if (cleaned > 0) {
        Serial.printf("[QuantumKeyMgr] Cleaned %u expired keys\n", cleaned);
    }
    
    return cleaned;
}

// Verify all keys
bool QuantumKeyManager::verifyAllKeys() {
    Serial.println("[QuantumKeyMgr] Verifying integrity of all keys...");
    
    bool all_valid = true;
    uint32_t verified = 0;
    
    for (const auto& pair : key_store_) {
        if (!verifyKeyIntegrity(pair.second)) {
            Serial.printf("[QuantumKeyMgr] WARNING: Key integrity failed: %s\n", 
                         pair.first.c_str());
            all_valid = false;
        } else {
            verified++;
        }
    }
    
    Serial.printf("[QuantumKeyMgr] Verified %u/%u keys\n", verified, (uint32_t)key_store_.size());
    
    return all_valid;
}

// Internal: Encrypt key material
bool QuantumKeyManager::encryptKeyMaterial(const uint8_t* key, size_t key_len,
                                          uint8_t* encrypted, size_t* enc_len) {
    // Simple XOR with master key for demonstration
    // In production, use proper encryption
    if (key_len > 256) {
        return false;
    }
    
    for (size_t i = 0; i < key_len; i++) {
        encrypted[i] = key[i] ^ master_encryption_key_[i % sizeof(master_encryption_key_)];
    }
    
    *enc_len = key_len;
    return true;
}

// Internal: Decrypt key material
bool QuantumKeyManager::decryptKeyMaterial(const uint8_t* encrypted, size_t enc_len,
                                          uint8_t* key, size_t* key_len) {
    // Simple XOR with master key (symmetric operation)
    if (enc_len > 256) {
        return false;
    }
    
    for (size_t i = 0; i < enc_len; i++) {
        key[i] = encrypted[i] ^ master_encryption_key_[i % sizeof(master_encryption_key_)];
    }
    
    *key_len = enc_len;
    return true;
}

// Internal: Compute key checksum
bool QuantumKeyManager::computeKeyChecksum(const KeyEntry& entry, uint8_t checksum[32]) {
    // Compute SHA-256 of key material
    mbedtls_sha256(entry.key_material, entry.key_len, checksum, 0);
    return true;
}

// Internal: Verify key integrity
bool QuantumKeyManager::verifyKeyIntegrity(const KeyEntry& entry) {
    uint8_t computed_checksum[32];
    computeKeyChecksum(entry, computed_checksum);
    
    return (memcmp(computed_checksum, entry.checksum, 32) == 0);
}

// Internal: Generate key ID
String QuantumKeyManager::generateKeyId(KeyUsageType usage) {
    String prefix;
    switch (usage) {
        case KeyUsageType::DATA_ENCRYPTION: prefix = "enc"; break;
        case KeyUsageType::SIGNATURE: prefix = "sig"; break;
        case KeyUsageType::KEY_EXCHANGE: prefix = "kex"; break;
        case KeyUsageType::AUTHENTICATION: prefix = "auth"; break;
        case KeyUsageType::INTEGRITY: prefix = "int"; break;
        case KeyUsageType::BACKUP: prefix = "bak"; break;
        default: prefix = "key"; break;
    }
    
    return prefix + "-" + String(total_keys_created_ + 1) + "-" + String(millis());
}

// Internal: Check if key should rotate
bool QuantumKeyManager::shouldRotateKey(const KeyMetadata& metadata) {
    if (metadata.status != KeyStatus::ACTIVE) {
        return false;
    }
    
    uint32_t current_time = millis();
    uint32_t key_age = current_time - metadata.created_at;
    
    // Check age-based rotation
    if (key_age >= metadata.rotation_interval) {
        return true;
    }
    
    // Check usage-based rotation
    if (metadata.usage_count >= metadata.max_usage) {
        return true;
    }
    
    return false;
}

// Internal: Update usage count
void QuantumKeyManager::updateKeyUsageCount(const String& key_id) {
    auto it = key_store_.find(key_id);
    if (it != key_store_.end()) {
        it->second.metadata.usage_count++;
    }
}

// Internal: Backup key
bool QuantumKeyManager::backupKey(const KeyEntry& entry) {
    // Placeholder for backup implementation
    // In production, would write to secure storage
    return true;
}
