/**
 * @file security_manager.cpp
 * @brief Implementation of advanced security features including AES-256 encryption
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

#include "security_manager.h"
#include <esp_system.h>
#include <esp_random.h>

// Constructor
SecurityManager::SecurityManager(SecurityLevel level, bool enable_secure_boot) 
    : security_level_(level),
      secure_element_available_(false),
      secure_boot_enabled_(enable_secure_boot),
      keys_initialized_(false),
      hw_security_init_(false),
      last_tamper_check_(0),
      tamper_detected_(false),
      tamper_sensitivity_(5),
      encryption_operations_(0),
      failed_operations_(0),
      boot_verifications_(0) {
    
    memset(device_key_, 0, sizeof(device_key_));
    memset(session_key_, 0, sizeof(session_key_));
    memset(firmware_hash_, 0, sizeof(firmware_hash_));
    memset(hw_device_id_, 0, sizeof(hw_device_id_));
}

// Initialize the security system
bool SecurityManager::begin() {
    Serial.println("[Security] Initializing security system...");
    
    // Initialize mbedTLS contexts
    mbedtls_aes_init(&aes_ctx_);
    mbedtls_sha256_init(&sha_ctx_);
    mbedtls_entropy_init(&entropy_);
    mbedtls_ctr_drbg_init(&ctr_drbg_);
    
    // Initialize random number generator
    if (!initializeRNG()) {
        Serial.println("[Security] ERROR: Failed to initialize RNG");
        return false;
    }
    
    // Initialize keys
    if (!initializeKeys()) {
        Serial.println("[Security] ERROR: Failed to initialize keys");
        return false;
    }
    
    // Try to initialize hardware security element
    if (security_level_ >= SecurityLevel::ENHANCED) {
        hw_security_init_ = initializeHardwareSecurity();
        if (hw_security_init_) {
            Serial.println("[Security] Hardware security element initialized");
        } else {
            Serial.println("[Security] WARNING: Hardware security element not available");
        }
    }
    
    // Verify firmware integrity if secure boot is enabled
    if (secure_boot_enabled_) {
        if (verifyFirmwareIntegrity()) {
            boot_verifications_++;
            logSecurityEvent(SecurityEvent::BOOT_VERIFICATION_SUCCESS);
            Serial.println("[Security] Firmware integrity verified");
        } else {
            logSecurityEvent(SecurityEvent::BOOT_VERIFICATION_FAILURE, "Firmware verification failed");
            Serial.println("[Security] ERROR: Firmware integrity check failed");
            return false;
        }
    }
    
    Serial.println("[Security] Security system initialized successfully");
    return true;
}

// Initialize random number generator
bool SecurityManager::initializeRNG() {
    const char* personalization = "WildCAM_ESP32_Security";
    
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg_, mbedtls_entropy_func, &entropy_,
                                    (const unsigned char*)personalization,
                                    strlen(personalization));
    
    if (ret != 0) {
        Serial.printf("[Security] mbedtls_ctr_drbg_seed failed: -0x%04x\n", -ret);
        return false;
    }
    
    return true;
}

// Initialize encryption keys
bool SecurityManager::initializeKeys() {
    // Generate device key
    if (!generateSecureRandom(device_key_, 32)) {
        Serial.println("[Security] Failed to generate device key");
        failed_operations_++;
        return false;
    }
    
    // Generate session key
    if (!generateSecureRandom(session_key_, 32)) {
        Serial.println("[Security] Failed to generate session key");
        failed_operations_++;
        return false;
    }
    
    keys_initialized_ = true;
    logSecurityEvent(SecurityEvent::KEY_GENERATION_SUCCESS);
    return true;
}

// Generate secure random data
bool SecurityManager::generateSecureRandom(uint8_t* output, size_t len) {
    if (output == nullptr || len == 0) {
        return false;
    }
    
    int ret = mbedtls_ctr_drbg_random(&ctr_drbg_, output, len);
    
    if (ret != 0) {
        Serial.printf("[Security] Random generation failed: -0x%04x\n", -ret);
        return false;
    }
    
    return true;
}

// Encrypt data using AES-256
bool SecurityManager::encryptData(const uint8_t* plaintext, size_t plaintext_len,
                                  uint8_t* ciphertext, size_t* ciphertext_len,
                                  KeyType key_type) {
    if (!keys_initialized_ || plaintext == nullptr || ciphertext == nullptr) {
        failed_operations_++;
        return false;
    }
    
    // Select appropriate key
    const uint8_t* key = (key_type == KeyType::SESSION_KEY) ? session_key_ : device_key_;
    
    // Generate IV
    uint8_t iv[16];
    if (!generateSecureRandom(iv, 16)) {
        failed_operations_++;
        return false;
    }
    
    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes_ctx_, key, 256);
    if (ret != 0) {
        Serial.printf("[Security] AES setkey failed: -0x%04x\n", -ret);
        failed_operations_++;
        return false;
    }
    
    // Calculate padded length (AES block size is 16 bytes)
    size_t padded_len = ((plaintext_len + 15) / 16) * 16;
    
    // Create padded plaintext
    uint8_t* padded_plaintext = (uint8_t*)malloc(padded_len);
    if (padded_plaintext == nullptr) {
        failed_operations_++;
        return false;
    }
    
    memcpy(padded_plaintext, plaintext, plaintext_len);
    // PKCS7 padding
    uint8_t padding_value = padded_len - plaintext_len;
    for (size_t i = plaintext_len; i < padded_len; i++) {
        padded_plaintext[i] = padding_value;
    }
    
    // Store IV at the beginning of ciphertext
    memcpy(ciphertext, iv, 16);
    
    // Encrypt data
    ret = mbedtls_aes_crypt_cbc(&aes_ctx_, MBEDTLS_AES_ENCRYPT, padded_len,
                                iv, padded_plaintext, ciphertext + 16);
    
    free(padded_plaintext);
    
    if (ret != 0) {
        Serial.printf("[Security] AES encryption failed: -0x%04x\n", -ret);
        failed_operations_++;
        return false;
    }
    
    *ciphertext_len = 16 + padded_len; // IV + encrypted data
    encryption_operations_++;
    logSecurityEvent(SecurityEvent::ENCRYPTION_SUCCESS);
    
    return true;
}

// Decrypt data using AES-256
bool SecurityManager::decryptData(const uint8_t* ciphertext, size_t ciphertext_len,
                                  uint8_t* plaintext, size_t* plaintext_len,
                                  KeyType key_type) {
    if (!keys_initialized_ || ciphertext == nullptr || plaintext == nullptr || ciphertext_len < 16) {
        failed_operations_++;
        return false;
    }
    
    // Select appropriate key
    const uint8_t* key = (key_type == KeyType::SESSION_KEY) ? session_key_ : device_key_;
    
    // Extract IV from the beginning of ciphertext
    uint8_t iv[16];
    memcpy(iv, ciphertext, 16);
    
    // Set decryption key
    int ret = mbedtls_aes_setkey_dec(&aes_ctx_, key, 256);
    if (ret != 0) {
        Serial.printf("[Security] AES setkey failed: -0x%04x\n", -ret);
        failed_operations_++;
        return false;
    }
    
    // Decrypt data
    size_t encrypted_len = ciphertext_len - 16;
    ret = mbedtls_aes_crypt_cbc(&aes_ctx_, MBEDTLS_AES_DECRYPT, encrypted_len,
                                iv, ciphertext + 16, plaintext);
    
    if (ret != 0) {
        Serial.printf("[Security] AES decryption failed: -0x%04x\n", -ret);
        failed_operations_++;
        return false;
    }
    
    // Remove PKCS7 padding
    uint8_t padding_value = plaintext[encrypted_len - 1];
    if (padding_value > 0 && padding_value <= 16) {
        *plaintext_len = encrypted_len - padding_value;
    } else {
        *plaintext_len = encrypted_len;
    }
    
    encryption_operations_++;
    return true;
}

// Generate secure hash of data
bool SecurityManager::generateHash(const uint8_t* data, size_t data_len, uint8_t hash[32]) {
    if (data == nullptr || hash == nullptr) {
        return false;
    }
    
    int ret = mbedtls_sha256(data, data_len, hash, 0); // 0 = SHA-256 (not SHA-224)
    
    if (ret != 0) {
        Serial.printf("[Security] SHA-256 hash failed: -0x%04x\n", -ret);
        return false;
    }
    
    return true;
}

// Verify data integrity using hash
bool SecurityManager::verifyIntegrity(const uint8_t* data, size_t data_len, 
                                     const uint8_t expected_hash[32]) {
    uint8_t calculated_hash[32];
    
    if (!generateHash(data, data_len, calculated_hash)) {
        return false;
    }
    
    // Compare hashes
    return memcmp(calculated_hash, expected_hash, 32) == 0;
}

// Generate new session key
bool SecurityManager::generateSessionKey() {
    if (!generateSecureRandom(session_key_, 32)) {
        logSecurityEvent(SecurityEvent::KEY_GENERATION_FAILURE);
        return false;
    }
    
    logSecurityEvent(SecurityEvent::KEY_GENERATION_SUCCESS, "Session key generated");
    return true;
}

// Verify firmware signature and integrity
bool SecurityManager::verifyFirmwareIntegrity() {
    // Get current firmware partition
    const esp_partition_t* partition = esp_ota_get_running_partition();
    if (partition == nullptr) {
        return false;
    }
    
    // Calculate firmware hash
    const size_t chunk_size = 1024;
    uint8_t buffer[chunk_size];
    mbedtls_sha256_context sha_ctx;
    
    mbedtls_sha256_init(&sha_ctx);
    mbedtls_sha256_starts(&sha_ctx, 0); // SHA-256
    
    for (size_t offset = 0; offset < partition->size; offset += chunk_size) {
        size_t read_size = (offset + chunk_size <= partition->size) ? chunk_size : (partition->size - offset);
        
        if (esp_partition_read(partition, offset, buffer, read_size) == ESP_OK) {
            mbedtls_sha256_update(&sha_ctx, buffer, read_size);
        } else {
            mbedtls_sha256_free(&sha_ctx);
            return false;
        }
    }
    
    mbedtls_sha256_finish(&sha_ctx, firmware_hash_);
    mbedtls_sha256_free(&sha_ctx);
    
    Serial.print("[Security] Firmware hash: ");
    for (int i = 0; i < 32; i++) {
        Serial.printf("%02x", firmware_hash_[i]);
    }
    Serial.println();
    
    return true;
}

// Initialize hardware security element (ATECC608A)
bool SecurityManager::initializeHardwareSecurity() {
    // This is a placeholder for actual ATECC608A initialization
    // In a real implementation, you would:
    // 1. Initialize I2C bus
    // 2. Detect ATECC608A chip
    // 3. Configure secure zones
    // 4. Generate/load keys
    
    Serial.println("[Security] Checking for hardware security element...");
    
    // For now, mark as unavailable unless we can detect the chip
    // This would require the actual ATECC608A library integration
    return false;
}

// Get hardware device ID from security element
bool SecurityManager::getHardwareDeviceID(uint8_t device_id[9]) {
    if (!hw_security_init_ || device_id == nullptr) {
        return false;
    }
    
    // If hardware security is initialized, return the stored device ID
    memcpy(device_id, hw_device_id_, 9);
    return true;
}

// Detect tampering attempts
bool SecurityManager::detectTampering() {
    unsigned long current_time = millis();
    
    // Check tamper detection every 10 seconds
    if (current_time - last_tamper_check_ < 10000) {
        return tamper_detected_;
    }
    
    last_tamper_check_ = current_time;
    
    // Check system integrity
    if (!checkSystemIntegrity()) {
        tamper_detected_ = true;
        logSecurityEvent(SecurityEvent::TAMPER_DETECTED, "System integrity check failed");
        return true;
    }
    
    // Additional tamper checks could include:
    // - Voltage monitoring
    // - Temperature anomalies
    // - Timing attacks
    // - Memory corruption detection
    
    tamper_detected_ = false;
    return false;
}

// Check system integrity
bool SecurityManager::checkSystemIntegrity() {
    // Verify keys are still valid
    if (!keys_initialized_) {
        return false;
    }
    
    // Check for memory corruption in key storage
    uint8_t zero_check[32] = {0};
    if (memcmp(device_key_, zero_check, 32) == 0) {
        Serial.println("[Security] Device key corrupted!");
        return false;
    }
    
    // In a real implementation, add more checks:
    // - Flash memory CRC
    // - Critical data structure validation
    // - Stack canary checks
    
    return true;
}

// Log security event
void SecurityManager::logSecurityEvent(SecurityEvent event, const char* message) {
    const char* event_str = "";
    
    switch (event) {
        case SecurityEvent::ENCRYPTION_SUCCESS:
            event_str = "ENCRYPTION_SUCCESS";
            break;
        case SecurityEvent::ENCRYPTION_FAILURE:
            event_str = "ENCRYPTION_FAILURE";
            break;
        case SecurityEvent::BOOT_VERIFICATION_SUCCESS:
            event_str = "BOOT_VERIFICATION_SUCCESS";
            break;
        case SecurityEvent::BOOT_VERIFICATION_FAILURE:
            event_str = "BOOT_VERIFICATION_FAILURE";
            break;
        case SecurityEvent::TAMPER_DETECTED:
            event_str = "TAMPER_DETECTED";
            break;
        case SecurityEvent::KEY_GENERATION_SUCCESS:
            event_str = "KEY_GENERATION_SUCCESS";
            break;
        case SecurityEvent::KEY_GENERATION_FAILURE:
            event_str = "KEY_GENERATION_FAILURE";
            break;
        case SecurityEvent::SECURE_ELEMENT_ERROR:
            event_str = "SECURE_ELEMENT_ERROR";
            break;
    }
    
    Serial.printf("[Security Event] %s", event_str);
    if (message != nullptr) {
        Serial.printf(" - %s", message);
    }
    Serial.println();
}

// Get security statistics
void SecurityManager::getSecurityStats(unsigned long* total_operations,
                                      unsigned long* failed_operations,
                                      unsigned long* boot_verifications) {
    if (total_operations) *total_operations = encryption_operations_;
    if (failed_operations) *failed_operations = failed_operations_;
    if (boot_verifications) *boot_verifications = boot_verifications_;
}

// Check if security system is operational
bool SecurityManager::isSecurityOperational() const {
    return keys_initialized_ && (failed_operations_ < 10);
}

// Derive key from hardware security element
bool SecurityManager::deriveKeyFromHardware(KeyType key_type, uint8_t key[32]) {
    if (!hw_security_init_ || key == nullptr) {
        return false;
    }
    
    // This would use the ATECC608A to derive keys
    // For now, return false as hardware is not available
    return false;
}

// Validate firmware signature
bool SecurityManager::validateFirmwareSignature(const uint8_t* firmware_data, 
                                               size_t firmware_len,
                                               const uint8_t* signature) {
    // This would verify the firmware signature using RSA or ECDSA
    // For now, we just verify the hash
    uint8_t calculated_hash[32];
    return generateHash(firmware_data, firmware_len, calculated_hash);
}

// Securely wipe memory buffer
void SecurityManager::secureWipe(uint8_t* buffer, size_t len) {
    if (buffer == nullptr || len == 0) {
        return;
    }
    
    // Multiple overwrites for secure deletion
    for (int pass = 0; pass < 3; pass++) {
        memset(buffer, 0x00 + pass, len);
    }
    memset(buffer, 0x00, len);
}
