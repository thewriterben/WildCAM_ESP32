/**
 * @file security_manager.h
 * @brief Advanced security features including AES-256 encryption and hardware security
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

/**
 * @brief Security level enumeration
 */
enum class SecurityLevel {
    BASIC,      // Basic encryption only
    ENHANCED,   // Hardware security element + encryption
    MAXIMUM     // Full security with secure boot validation
};

/**
 * @brief Encryption key types
 */
enum class KeyType {
    DEVICE_KEY,         // Device-specific encryption key
    SESSION_KEY,        // Temporary session key
    FIRMWARE_KEY,       // Firmware signing key
    DATA_INTEGRITY_KEY  // Data integrity verification key
};

/**
 * @brief Security event types for logging
 */
enum class SecurityEvent {
    ENCRYPTION_SUCCESS,
    ENCRYPTION_FAILURE,
    BOOT_VERIFICATION_SUCCESS,
    BOOT_VERIFICATION_FAILURE,
    TAMPER_DETECTED,
    KEY_GENERATION_SUCCESS,
    KEY_GENERATION_FAILURE,
    SECURE_ELEMENT_ERROR
};

/**
 * @brief Advanced security manager for wildlife camera
 */
class SecurityManager {
private:
    // Encryption contexts
    mbedtls_aes_context aes_ctx_;
    mbedtls_sha256_context sha_ctx_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    
    // Security configuration
    SecurityLevel security_level_;
    bool secure_element_available_;
    bool secure_boot_enabled_;
    
    // Key management
    uint8_t device_key_[32];        // 256-bit device key
    uint8_t session_key_[32];       // 256-bit session key
    uint8_t firmware_hash_[32];     // Firmware integrity hash
    bool keys_initialized_;
    
    // Hardware security element (ATECC608A)
    bool hw_security_init_;
    uint8_t hw_device_id_[9];       // Hardware device ID
    
    // Tamper detection
    unsigned long last_tamper_check_;
    bool tamper_detected_;
    int tamper_sensitivity_;
    
    // Security metrics
    unsigned long encryption_operations_;
    unsigned long failed_operations_;
    unsigned long boot_verifications_;
    
public:
    /**
     * @brief Initialize security manager
     * @param level Security level to implement
     * @param enable_secure_boot Enable secure boot verification
     */
    SecurityManager(SecurityLevel level = SecurityLevel::ENHANCED, bool enable_secure_boot = true);
    
    /**
     * @brief Initialize the security system
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Encrypt data using AES-256
     * @param plaintext Input data to encrypt
     * @param plaintext_len Length of input data
     * @param ciphertext Output encrypted data
     * @param ciphertext_len Length of encrypted data
     * @param key_type Type of key to use for encryption
     * @return true if encryption successful
     */
    bool encryptData(const uint8_t* plaintext, size_t plaintext_len,
                     uint8_t* ciphertext, size_t* ciphertext_len,
                     KeyType key_type = KeyType::DEVICE_KEY);
    
    /**
     * @brief Decrypt data using AES-256
     * @param ciphertext Input encrypted data
     * @param ciphertext_len Length of encrypted data
     * @param plaintext Output decrypted data
     * @param plaintext_len Length of decrypted data
     * @param key_type Type of key to use for decryption
     * @return true if decryption successful
     */
    bool decryptData(const uint8_t* ciphertext, size_t ciphertext_len,
                     uint8_t* plaintext, size_t* plaintext_len,
                     KeyType key_type = KeyType::DEVICE_KEY);
    
    /**
     * @brief Generate secure hash of data
     * @param data Input data
     * @param data_len Length of input data
     * @param hash Output hash (32 bytes)
     * @return true if hash generation successful
     */
    bool generateHash(const uint8_t* data, size_t data_len, uint8_t hash[32]);
    
    /**
     * @brief Verify data integrity using hash
     * @param data Input data
     * @param data_len Length of input data
     * @param expected_hash Expected hash value
     * @return true if data integrity verified
     */
    bool verifyIntegrity(const uint8_t* data, size_t data_len, const uint8_t expected_hash[32]);
    
    /**
     * @brief Generate new session key
     * @return true if key generation successful
     */
    bool generateSessionKey();
    
    /**
     * @brief Verify firmware signature and integrity
     * @return true if firmware is valid and signed
     */
    bool verifyFirmwareIntegrity();
    
    /**
     * @brief Initialize hardware security element (ATECC608A)
     * @return true if hardware security element available
     */
    bool initializeHardwareSecurity();
    
    /**
     * @brief Get hardware device ID from security element
     * @param device_id Output device ID (9 bytes)
     * @return true if device ID retrieved successfully
     */
    bool getHardwareDeviceID(uint8_t device_id[9]);
    
    /**
     * @brief Detect tampering attempts
     * @return true if tampering detected
     */
    bool detectTampering();
    
    /**
     * @brief Set tamper detection sensitivity
     * @param sensitivity Sensitivity level (1-10, higher = more sensitive)
     */
    void setTamperSensitivity(int sensitivity) { tamper_sensitivity_ = sensitivity; }
    
    /**
     * @brief Generate secure random number
     * @param output Output buffer for random data
     * @param len Length of random data to generate
     * @return true if random generation successful
     */
    bool generateSecureRandom(uint8_t* output, size_t len);
    
    /**
     * @brief Log security event
     * @param event Security event type
     * @param message Optional message
     */
    void logSecurityEvent(SecurityEvent event, const char* message = nullptr);
    
    /**
     * @brief Get security statistics
     * @param total_operations Output total encryption operations
     * @param failed_operations Output failed operations
     * @param boot_verifications Output boot verifications performed
     */
    void getSecurityStats(unsigned long* total_operations, 
                         unsigned long* failed_operations,
                         unsigned long* boot_verifications);
    
    /**
     * @brief Check if security system is operational
     * @return true if all security features are working
     */
    bool isSecurityOperational() const;
    
    /**
     * @brief Enable/disable secure boot verification
     * @param enabled Enable secure boot
     */
    void setSecureBootEnabled(bool enabled) { secure_boot_enabled_ = enabled; }
    
    /**
     * @brief Get current security level
     * @return Current security level
     */
    SecurityLevel getSecurityLevel() const { return security_level_; }
    
    /**
     * @brief Check if hardware security element is available
     * @return true if ATECC608A is available and initialized
     */
    bool isHardwareSecurityAvailable() const { return hw_security_init_; }

private:
    /**
     * @brief Initialize encryption keys
     * @return true if key initialization successful
     */
    bool initializeKeys();
    
    /**
     * @brief Derive key from hardware security element
     * @param key_type Type of key to derive
     * @param key Output key buffer (32 bytes)
     * @return true if key derivation successful
     */
    bool deriveKeyFromHardware(KeyType key_type, uint8_t key[32]);
    
    /**
     * @brief Validate firmware signature
     * @param firmware_data Firmware data to validate
     * @param firmware_len Length of firmware data
     * @param signature Firmware signature
     * @return true if signature is valid
     */
    bool validateFirmwareSignature(const uint8_t* firmware_data, size_t firmware_len, 
                                 const uint8_t* signature);
    
    /**
     * @brief Check system integrity (memory, configuration, etc.)
     * @return true if system integrity is maintained
     */
    bool checkSystemIntegrity();
    
    /**
     * @brief Initialize secure random number generator
     * @return true if RNG initialization successful
     */
    bool initializeRNG();
    
    /**
     * @brief Securely wipe memory buffer
     * @param buffer Buffer to wipe
     * @param len Length of buffer
     */
    void secureWipe(uint8_t* buffer, size_t len);
};

#endif // SECURITY_MANAGER_H