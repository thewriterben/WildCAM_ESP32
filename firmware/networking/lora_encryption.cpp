/**
 * @file lora_encryption.cpp
 * @brief AES-256 encryption implementation for LoRa
 */

#include "lora_encryption.h"
#include <esp_random.h>

// WARNING: This default key is for demonstration only
// In production, use secure key storage (e.g., ESP32 eFuse, secure element)
const uint8_t DEFAULT_LORA_KEY[32] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    0x76, 0x2e, 0x71, 0x60, 0xf3, 0x8b, 0x4d, 0xa5,
    0x6a, 0x78, 0x4d, 0x90, 0x41, 0xd3, 0xa4, 0xf3
};

LoRaEncryption::LoRaEncryption(const uint8_t* key) 
    : m_initialized(false), m_lastError("") {
    memcpy(m_key, key, 32);
    mbedtls_aes_init(&m_aes_encrypt);
    mbedtls_aes_init(&m_aes_decrypt);
}

bool LoRaEncryption::begin() {
    if (m_initialized) {
        return true;
    }
    
    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&m_aes_encrypt, m_key, 256);
    if (ret != 0) {
        setError("Failed to set encryption key");
        return false;
    }
    
    // Set decryption key
    ret = mbedtls_aes_setkey_dec(&m_aes_decrypt, m_key, 256);
    if (ret != 0) {
        setError("Failed to set decryption key");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void LoRaEncryption::generateIV(uint8_t* iv) {
    // Generate cryptographically secure random IV
    for (int i = 0; i < 16; i++) {
        iv[i] = (uint8_t)esp_random();
    }
}

size_t LoRaEncryption::addPadding(uint8_t* data, size_t length, size_t blockSize) {
    // PKCS7 padding
    size_t padding = blockSize - (length % blockSize);
    for (size_t i = 0; i < padding; i++) {
        data[length + i] = (uint8_t)padding;
    }
    return length + padding;
}

size_t LoRaEncryption::removePadding(uint8_t* data, size_t length) {
    if (length == 0) return 0;
    
    // Get padding value from last byte
    uint8_t padding = data[length - 1];
    
    // Validate padding
    if (padding > 16 || padding > length) {
        return length; // Invalid padding, return original length
    }
    
    // Verify all padding bytes are correct
    for (size_t i = length - padding; i < length; i++) {
        if (data[i] != padding) {
            return length; // Invalid padding
        }
    }
    
    return length - padding;
}

bool LoRaEncryption::encrypt(const String& plaintext, String& encrypted) {
    if (!m_initialized) {
        setError("Encryption system not initialized");
        return false;
    }
    
    if (plaintext.length() == 0) {
        setError("Empty plaintext");
        return false;
    }
    
    // Generate random IV
    uint8_t iv[16];
    generateIV(iv);
    
    // Prepare input buffer with padding
    size_t input_len = plaintext.length();
    size_t padded_len = ((input_len / 16) + 1) * 16; // Round up to block size
    uint8_t* input_buf = (uint8_t*)malloc(padded_len);
    if (!input_buf) {
        setError("Memory allocation failed");
        return false;
    }
    
    // Copy plaintext and add padding
    memcpy(input_buf, plaintext.c_str(), input_len);
    size_t final_len = addPadding(input_buf, input_len, 16);
    
    // Allocate output buffer
    uint8_t* output_buf = (uint8_t*)malloc(final_len);
    if (!output_buf) {
        free(input_buf);
        setError("Memory allocation failed");
        return false;
    }
    
    // Encrypt using AES-256-CBC
    uint8_t iv_copy[16];
    memcpy(iv_copy, iv, 16); // IV gets modified, so use copy
    
    int ret = mbedtls_aes_crypt_cbc(&m_aes_encrypt, MBEDTLS_AES_ENCRYPT,
                                    final_len, iv_copy, input_buf, output_buf);
    
    if (ret != 0) {
        free(input_buf);
        free(output_buf);
        setError("Encryption failed");
        return false;
    }
    
    // Combine IV + ciphertext
    size_t combined_len = 16 + final_len;
    uint8_t* combined = (uint8_t*)malloc(combined_len);
    if (!combined) {
        free(input_buf);
        free(output_buf);
        setError("Memory allocation failed");
        return false;
    }
    
    memcpy(combined, iv, 16);
    memcpy(combined + 16, output_buf, final_len);
    
    // Base64 encode for safe transmission
    size_t encoded_len;
    mbedtls_base64_encode(NULL, 0, &encoded_len, combined, combined_len);
    
    char* encoded = (char*)malloc(encoded_len + 1);
    if (!encoded) {
        free(input_buf);
        free(output_buf);
        free(combined);
        setError("Memory allocation failed");
        return false;
    }
    
    ret = mbedtls_base64_encode((unsigned char*)encoded, encoded_len + 1, &encoded_len,
                                combined, combined_len);
    
    free(input_buf);
    free(output_buf);
    free(combined);
    
    if (ret != 0) {
        free(encoded);
        setError("Base64 encoding failed");
        return false;
    }
    
    encrypted = String(encoded);
    free(encoded);
    
    return true;
}

bool LoRaEncryption::decrypt(const String& encrypted, String& plaintext) {
    if (!m_initialized) {
        setError("Encryption system not initialized");
        return false;
    }
    
    if (encrypted.length() == 0) {
        setError("Empty encrypted message");
        return false;
    }
    
    // Base64 decode
    size_t decoded_len;
    mbedtls_base64_decode(NULL, 0, &decoded_len, 
                         (const unsigned char*)encrypted.c_str(), encrypted.length());
    
    uint8_t* decoded = (uint8_t*)malloc(decoded_len);
    if (!decoded) {
        setError("Memory allocation failed");
        return false;
    }
    
    int ret = mbedtls_base64_decode(decoded, decoded_len, &decoded_len,
                                    (const unsigned char*)encrypted.c_str(), 
                                    encrypted.length());
    
    if (ret != 0) {
        free(decoded);
        setError("Base64 decoding failed");
        return false;
    }
    
    // Extract IV and ciphertext
    if (decoded_len < 16) {
        free(decoded);
        setError("Invalid encrypted message (too short)");
        return false;
    }
    
    uint8_t iv[16];
    memcpy(iv, decoded, 16);
    
    size_t ciphertext_len = decoded_len - 16;
    uint8_t* ciphertext = decoded + 16;
    
    // Allocate output buffer
    uint8_t* output_buf = (uint8_t*)malloc(ciphertext_len);
    if (!output_buf) {
        free(decoded);
        setError("Memory allocation failed");
        return false;
    }
    
    // Decrypt using AES-256-CBC
    ret = mbedtls_aes_crypt_cbc(&m_aes_decrypt, MBEDTLS_AES_DECRYPT,
                                ciphertext_len, iv, ciphertext, output_buf);
    
    if (ret != 0) {
        free(decoded);
        free(output_buf);
        setError("Decryption failed");
        return false;
    }
    
    // Remove padding
    size_t plaintext_len = removePadding(output_buf, ciphertext_len);
    
    // Convert to string
    char* result = (char*)malloc(plaintext_len + 1);
    if (!result) {
        free(decoded);
        free(output_buf);
        setError("Memory allocation failed");
        return false;
    }
    
    memcpy(result, output_buf, plaintext_len);
    result[plaintext_len] = '\0';
    
    plaintext = String(result);
    
    free(decoded);
    free(output_buf);
    free(result);
    
    return true;
}

void LoRaEncryption::setError(const char* error) {
    m_lastError = String(error);
}
