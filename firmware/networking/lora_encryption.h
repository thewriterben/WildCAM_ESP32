/**
 * @file lora_encryption.h
 * @brief AES-256 encryption for LoRa communications
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * Provides secure encrypted messaging for wildlife data transmission over LoRa
 */

#ifndef LORA_ENCRYPTION_H
#define LORA_ENCRYPTION_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>

/**
 * @class LoRaEncryption
 * @brief AES-256-CBC encryption for LoRa messages
 */
class LoRaEncryption {
public:
    /**
     * @brief Constructor
     * @param key 32-byte encryption key (256-bit)
     */
    LoRaEncryption(const uint8_t* key);
    
    /**
     * @brief Initialize encryption system
     * @return true if successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Encrypt a message
     * @param plaintext Input message
     * @param encrypted Output encrypted message (base64 encoded)
     * @return true if successful, false otherwise
     */
    bool encrypt(const String& plaintext, String& encrypted);
    
    /**
     * @brief Decrypt a message
     * @param encrypted Input encrypted message (base64 encoded)
     * @param plaintext Output decrypted message
     * @return true if successful, false otherwise
     */
    bool decrypt(const String& encrypted, String& plaintext);
    
    /**
     * @brief Generate a random IV for encryption
     * @param iv Output 16-byte IV
     */
    void generateIV(uint8_t* iv);
    
    /**
     * @brief Get last error message
     * @return Error description
     */
    String getLastError() const { return m_lastError; }
    
private:
    mbedtls_aes_context m_aes_encrypt;
    mbedtls_aes_context m_aes_decrypt;
    uint8_t m_key[32]; // 256-bit key
    bool m_initialized;
    String m_lastError;
    
    /**
     * @brief Set error message
     */
    void setError(const char* error);
    
    /**
     * @brief Apply PKCS7 padding
     */
    size_t addPadding(uint8_t* data, size_t length, size_t blockSize);
    
    /**
     * @brief Remove PKCS7 padding
     */
    size_t removePadding(uint8_t* data, size_t length);
};

/**
 * @brief Default encryption key (MUST BE CHANGED IN PRODUCTION)
 * In production, load from secure storage or generate per-device keys
 */
extern const uint8_t DEFAULT_LORA_KEY[32];

#endif // LORA_ENCRYPTION_H
