/**
 * @file test_lora_encryption.cpp
 * @brief Unit tests for LoRa AES-256 encryption
 */

#include <unity.h>
#include "../networking/lora_encryption.h"

LoRaEncryption* encryption = nullptr;

void setUp(void) {
    // Initialize encryption with default key
    encryption = new LoRaEncryption(DEFAULT_LORA_KEY);
    encryption->begin();
}

void tearDown(void) {
    if (encryption) {
        delete encryption;
        encryption = nullptr;
    }
}

/**
 * Test encryption initialization
 */
void test_encryption_initialization(void) {
    LoRaEncryption enc(DEFAULT_LORA_KEY);
    bool result = enc.begin();
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(enc.getLastError().isEmpty());
}

/**
 * Test basic encryption/decryption
 */
void test_encrypt_decrypt_basic(void) {
    String plaintext = "Wildlife detected: deer";
    String encrypted;
    
    bool encrypt_result = encryption->encrypt(plaintext, encrypted);
    TEST_ASSERT_TRUE(encrypt_result);
    TEST_ASSERT_FALSE(encrypted.isEmpty());
    TEST_ASSERT_NOT_EQUAL(plaintext, encrypted);
    
    String decrypted;
    bool decrypt_result = encryption->decrypt(encrypted, decrypted);
    TEST_ASSERT_TRUE(decrypt_result);
    TEST_ASSERT_EQUAL_STRING(plaintext.c_str(), decrypted.c_str());
}

/**
 * Test encryption with JSON data
 */
void test_encrypt_json_message(void) {
    String json_msg = "{\"id\":\"ESP32-001\",\"species\":\"bear\",\"conf\":0.95}";
    String encrypted;
    
    bool result = encryption->encrypt(json_msg, encrypted);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_GREATER_THAN(json_msg.length(), encrypted.length()); // Encrypted is longer (IV + padding + base64)
    
    String decrypted;
    result = encryption->decrypt(encrypted, decrypted);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(json_msg.c_str(), decrypted.c_str());
}

/**
 * Test encryption with empty string
 */
void test_encrypt_empty_string(void) {
    String plaintext = "";
    String encrypted;
    
    bool result = encryption->encrypt(plaintext, encrypted);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(encryption->getLastError().isEmpty());
}

/**
 * Test decryption with invalid data
 */
void test_decrypt_invalid_data(void) {
    String invalid = "not-valid-encrypted-data";
    String decrypted;
    
    bool result = encryption->decrypt(invalid, decrypted);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(encryption->getLastError().isEmpty());
}

/**
 * Test encryption produces different output each time (due to random IV)
 */
void test_encryption_randomness(void) {
    String plaintext = "Test message";
    String encrypted1, encrypted2;
    
    encryption->encrypt(plaintext, encrypted1);
    encryption->encrypt(plaintext, encrypted2);
    
    // Same plaintext should produce different ciphertext (different IV)
    TEST_ASSERT_NOT_EQUAL(encrypted1, encrypted2);
    
    // But both should decrypt to same plaintext
    String decrypted1, decrypted2;
    encryption->decrypt(encrypted1, decrypted1);
    encryption->decrypt(encrypted2, decrypted2);
    
    TEST_ASSERT_EQUAL_STRING(plaintext.c_str(), decrypted1.c_str());
    TEST_ASSERT_EQUAL_STRING(plaintext.c_str(), decrypted2.c_str());
}

/**
 * Test encryption with long message
 */
void test_encrypt_long_message(void) {
    String long_msg = "This is a much longer message that contains multiple blocks of data. ";
    long_msg += "It will test the padding and block cipher mode implementation. ";
    long_msg += "The message should be encrypted and decrypted correctly regardless of length.";
    
    String encrypted;
    bool result = encryption->encrypt(long_msg, encrypted);
    TEST_ASSERT_TRUE(result);
    
    String decrypted;
    result = encryption->decrypt(encrypted, decrypted);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(long_msg.c_str(), decrypted.c_str());
}

/**
 * Test encryption with special characters
 */
void test_encrypt_special_characters(void) {
    String special = "Test!@#$%^&*(){}[]|\\:;\"'<>,.?/~`±§";
    String encrypted;
    
    bool result = encryption->encrypt(special, encrypted);
    TEST_ASSERT_TRUE(result);
    
    String decrypted;
    result = encryption->decrypt(encrypted, decrypted);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(special.c_str(), decrypted.c_str());
}

/**
 * Test memory usage for encryption/decryption
 */
void test_encryption_memory_usage(void) {
    size_t heap_before = ESP.getFreeHeap();
    
    String plaintext = "Wildlife detection message";
    String encrypted;
    
    for (int i = 0; i < 10; i++) {
        encryption->encrypt(plaintext, encrypted);
        String decrypted;
        encryption->decrypt(encrypted, decrypted);
    }
    
    delay(100);
    size_t heap_after = ESP.getFreeHeap();
    
    // Should not leak memory
    int leak = heap_before - heap_after;
    TEST_ASSERT_LESS_THAN(512, abs(leak));
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_encryption_initialization);
    RUN_TEST(test_encrypt_decrypt_basic);
    RUN_TEST(test_encrypt_json_message);
    RUN_TEST(test_encrypt_empty_string);
    RUN_TEST(test_decrypt_invalid_data);
    RUN_TEST(test_encryption_randomness);
    RUN_TEST(test_encrypt_long_message);
    RUN_TEST(test_encrypt_special_characters);
    RUN_TEST(test_encryption_memory_usage);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
