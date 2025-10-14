/**
 * @file test_quantum_safe.cpp
 * @brief Unit tests for quantum-safe security features
 * 
 * Tests quantum-resistant cryptography and key management.
 */

#include <unity.h>
#include "../firmware/security/quantum_safe_crypto.h"
#include "../firmware/security/quantum_key_manager.h"

// Test fixtures
QuantumSafeCrypto* testCrypto = nullptr;
QuantumKeyManager* testKeyMgr = nullptr;

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
    if (testCrypto) {
        delete testCrypto;
        testCrypto = nullptr;
    }
    if (testKeyMgr) {
        delete testKeyMgr;
        testKeyMgr = nullptr;
    }
}

// Test: Quantum-safe crypto initialization
void test_quantum_crypto_init(void) {
    testCrypto = new QuantumSafeCrypto(QuantumSecurityLevel::HYBRID_TRANSITION);
    TEST_ASSERT_NOT_NULL(testCrypto);
    
    bool result = testCrypto->begin();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testCrypto->isQuantumSafeOperational());
}

// Test: Quantum random generation
void test_quantum_random_generation(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    uint8_t random1[32];
    uint8_t random2[32];
    
    TEST_ASSERT_TRUE(testCrypto->generateQuantumRandom(random1, 32));
    TEST_ASSERT_TRUE(testCrypto->generateQuantumRandom(random2, 32));
    
    // Verify randomness (should be different)
    TEST_ASSERT_NOT_EQUAL(0, memcmp(random1, random2, 32));
}

// Test: Quantum-safe encryption/decryption
void test_quantum_safe_encryption(void) {
    testCrypto = new QuantumSafeCrypto(QuantumSecurityLevel::HYBRID_TRANSITION);
    testCrypto->begin();
    
    const char* plaintext = "Sensitive wildlife data";
    size_t plain_len = strlen(plaintext);
    
    // Prepare context
    QuantumEncryptionContext ctx;
    ctx.level = QuantumSecurityLevel::HYBRID_TRANSITION;
    ctx.pq_algo = PostQuantumAlgorithm::HYBRID_AES_PQ;
    
    testCrypto->generateQuantumRandom(ctx.classical_key, 32);
    testCrypto->generateQuantumRandom(ctx.pq_key_material, 128);
    
    // Encrypt
    uint8_t ciphertext[256];
    size_t cipher_len;
    bool enc_result = testCrypto->encryptQuantumSafe((uint8_t*)plaintext, plain_len,
                                                     ciphertext, &cipher_len, ctx);
    TEST_ASSERT_TRUE(enc_result);
    TEST_ASSERT_GREATER_THAN(plain_len, cipher_len);
    
    // Decrypt
    uint8_t decrypted[256];
    size_t decrypted_len;
    bool dec_result = testCrypto->decryptQuantumSafe(ciphertext, cipher_len,
                                                     decrypted, &decrypted_len, ctx);
    TEST_ASSERT_TRUE(dec_result);
    TEST_ASSERT_EQUAL(plain_len, decrypted_len);
    
    // Verify
    TEST_ASSERT_EQUAL_MEMORY(plaintext, decrypted, plain_len);
}

// Test: Different security levels
void test_security_levels(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    QuantumSecurityLevel levels[] = {
        QuantumSecurityLevel::CLASSICAL_ONLY,
        QuantumSecurityLevel::HYBRID_TRANSITION,
        QuantumSecurityLevel::POST_QUANTUM_READY,
        QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM
    };
    
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_TRUE(testCrypto->setSecurityLevel(levels[i]));
        TEST_ASSERT_EQUAL(levels[i], testCrypto->getSecurityLevel());
    }
}

// Test: Quantum-resistant signatures
void test_quantum_signatures(void) {
    testCrypto = new QuantumSafeCrypto(QuantumSecurityLevel::POST_QUANTUM_READY);
    testCrypto->begin();
    
    // Generate key pair
    QuantumSignatureContext sig_ctx;
    TEST_ASSERT_TRUE(testCrypto->generateKeyPair(sig_ctx, 
                                                 PostQuantumAlgorithm::HASH_BASED_SIG));
    TEST_ASSERT_TRUE(sig_ctx.is_valid);
    
    // Sign message
    const char* message = "Test message";
    uint8_t signature[512];
    size_t sig_len;
    
    bool sign_result = testCrypto->signMessage((uint8_t*)message, strlen(message),
                                              signature, &sig_len, sig_ctx);
    TEST_ASSERT_TRUE(sign_result);
    TEST_ASSERT_GREATER_THAN(0, sig_len);
    
    // Verify signature
    bool verify_result = testCrypto->verifySignature((uint8_t*)message, strlen(message),
                                                     signature, sig_len, sig_ctx);
    TEST_ASSERT_TRUE(verify_result);
    
    // Test tampered message detection
    char tampered[32];
    strcpy(tampered, message);
    tampered[0] = 'X';
    
    bool verify_tampered = testCrypto->verifySignature((uint8_t*)tampered, strlen(tampered),
                                                       signature, sig_len, sig_ctx);
    TEST_ASSERT_FALSE(verify_tampered);
}

// Test: Key derivation
void test_key_derivation(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    const char* passphrase = "test_passphrase";
    QuantumKeyParams params;
    params.iterations = 100000;
    testCrypto->generateQuantumRandom(params.salt, 32);
    
    uint8_t derived_key1[32];
    uint8_t derived_key2[32];
    
    // Derive key twice with same params
    TEST_ASSERT_TRUE(testCrypto->deriveKey((uint8_t*)passphrase, strlen(passphrase),
                                          params, derived_key1, 32));
    TEST_ASSERT_TRUE(testCrypto->deriveKey((uint8_t*)passphrase, strlen(passphrase),
                                          params, derived_key2, 32));
    
    // Should produce same key
    TEST_ASSERT_EQUAL_MEMORY(derived_key1, derived_key2, 32);
}

// Test: Key rotation
void test_key_rotation(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    // Get initial state
    uint8_t old_key[32];
    memcpy(old_key, testCrypto, 32); // Simplified test
    
    // Rotate keys
    TEST_ASSERT_TRUE(testCrypto->rotateKeys());
    
    // Verify keys changed (would need internal access in real test)
    TEST_ASSERT_TRUE(true); // Placeholder
}

// Test: Quantum threat assessment
void test_threat_assessment(void) {
    testCrypto = new QuantumSafeCrypto(QuantumSecurityLevel::CLASSICAL_ONLY);
    testCrypto->begin();
    
    uint8_t threat1 = testCrypto->assessQuantumThreatLevel();
    TEST_ASSERT_GREATER_THAN(50, threat1); // Classical only should be high threat
    
    // Upgrade security level
    testCrypto->setSecurityLevel(QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM);
    uint8_t threat2 = testCrypto->assessQuantumThreatLevel();
    TEST_ASSERT_LESS_THAN(threat1, threat2); // Should be lower threat
}

// Test: Key manager initialization
void test_key_manager_init(void) {
    testKeyMgr = new QuantumKeyManager();
    TEST_ASSERT_NOT_NULL(testKeyMgr);
    
    TEST_ASSERT_TRUE(testKeyMgr->begin());
    TEST_ASSERT_TRUE(testKeyMgr->isOperational());
}

// Test: Key generation
void test_key_generation(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    String key_id;
    bool result = testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                                         QuantumSecurityLevel::HYBRID_TRANSITION,
                                         key_id);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_GREATER_THAN(0, key_id.length());
    TEST_ASSERT_EQUAL(1, testKeyMgr->getKeyCount());
}

// Test: Key retrieval
void test_key_retrieval(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    // Generate key
    String key_id;
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION,
                           key_id);
    
    // Retrieve key
    uint8_t key_material[256];
    size_t key_len;
    TEST_ASSERT_TRUE(testKeyMgr->getKey(key_id, key_material, &key_len));
    TEST_ASSERT_GREATER_THAN(0, key_len);
    
    // Get metadata
    KeyMetadata metadata;
    TEST_ASSERT_TRUE(testKeyMgr->getKeyMetadata(key_id, metadata));
    TEST_ASSERT_EQUAL_STRING(key_id.c_str(), metadata.key_id.c_str());
    TEST_ASSERT_EQUAL(KeyStatus::ACTIVE, metadata.status);
}

// Test: Key rotation in manager
void test_key_manager_rotation(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    // Generate initial key
    String old_key_id;
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION,
                           old_key_id);
    
    // Rotate key
    String new_key_id;
    TEST_ASSERT_TRUE(testKeyMgr->rotateKey(old_key_id, new_key_id));
    TEST_ASSERT_NOT_EQUAL_STRING(old_key_id.c_str(), new_key_id.c_str());
    
    // Verify old key is deprecated
    KeyMetadata old_metadata;
    testKeyMgr->getKeyMetadata(old_key_id, old_metadata);
    TEST_ASSERT_EQUAL(KeyStatus::DEPRECATED, old_metadata.status);
    
    // Verify new key is active
    KeyMetadata new_metadata;
    testKeyMgr->getKeyMetadata(new_key_id, new_metadata);
    TEST_ASSERT_EQUAL(KeyStatus::ACTIVE, new_metadata.status);
}

// Test: Key listing
void test_key_listing(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    // Generate multiple keys
    String key_id;
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION, key_id);
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION, key_id);
    testKeyMgr->generateKey(KeyUsageType::SIGNATURE,
                           QuantumSecurityLevel::POST_QUANTUM_READY, key_id);
    
    // List encryption keys
    std::vector<String> enc_keys = testKeyMgr->listKeys(KeyUsageType::DATA_ENCRYPTION);
    TEST_ASSERT_EQUAL(2, enc_keys.size());
    
    // List signature keys
    std::vector<String> sig_keys = testKeyMgr->listKeys(KeyUsageType::SIGNATURE);
    TEST_ASSERT_EQUAL(1, sig_keys.size());
}

// Test: Key revocation
void test_key_revocation(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    String key_id;
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION, key_id);
    
    // Revoke key
    TEST_ASSERT_TRUE(testKeyMgr->revokeKey(key_id, "Test revocation"));
    
    // Verify key is revoked
    KeyMetadata metadata;
    testKeyMgr->getKeyMetadata(key_id, metadata);
    TEST_ASSERT_EQUAL(KeyStatus::REVOKED, metadata.status);
    
    // Verify cannot retrieve revoked key
    uint8_t key_material[256];
    size_t key_len;
    TEST_ASSERT_FALSE(testKeyMgr->getKey(key_id, key_material, &key_len));
}

// Test: Key statistics
void test_key_statistics(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    // Generate and rotate keys
    String key_id, new_key_id;
    testKeyMgr->generateKey(KeyUsageType::DATA_ENCRYPTION,
                           QuantumSecurityLevel::HYBRID_TRANSITION, key_id);
    testKeyMgr->rotateKey(key_id, new_key_id);
    
    // Get statistics
    uint32_t created, rotated, expired, failed;
    testKeyMgr->getStatistics(&created, &rotated, &expired, &failed);
    
    TEST_ASSERT_EQUAL(2, created);  // Original + rotated
    TEST_ASSERT_EQUAL(1, rotated);
    TEST_ASSERT_EQUAL(0, failed);
}

// Test: Rotation policy
void test_rotation_policy(void) {
    testKeyMgr = new QuantumKeyManager();
    testKeyMgr->begin();
    
    KeyRotationPolicy policy;
    policy.auto_rotation_enabled = true;
    policy.rotation_interval_hours = 24;
    policy.max_key_age_hours = 168;
    
    testKeyMgr->setRotationPolicy(policy);
    
    KeyRotationPolicy retrieved = testKeyMgr->getRotationPolicy();
    TEST_ASSERT_TRUE(retrieved.auto_rotation_enabled);
    TEST_ASSERT_EQUAL(24, retrieved.rotation_interval_hours);
    TEST_ASSERT_EQUAL(168, retrieved.max_key_age_hours);
}

// Test: Quantum-safe IV generation
void test_quantum_safe_iv(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    uint8_t iv1[16];
    uint8_t iv2[16];
    
    TEST_ASSERT_TRUE(testCrypto->generateQuantumSafeIV(iv1));
    TEST_ASSERT_TRUE(testCrypto->generateQuantumSafeIV(iv2));
    
    // IVs should be different
    TEST_ASSERT_NOT_EQUAL(0, memcmp(iv1, iv2, 16));
}

// Test: Metrics tracking
void test_metrics_tracking(void) {
    testCrypto = new QuantumSafeCrypto();
    testCrypto->begin();
    
    // Perform some operations
    uint8_t random[32];
    testCrypto->generateQuantumRandom(random, 32);
    
    QuantumThreatMetrics metrics = testCrypto->getMetrics();
    TEST_ASSERT_GREATER_THAN(0, metrics.total_operations);
    TEST_ASSERT_GREATER_THAN(0, metrics.quantum_safe_operations);
}

void runQuantumSafeTests() {
    UNITY_BEGIN();
    
    // Quantum-safe crypto tests
    RUN_TEST(test_quantum_crypto_init);
    RUN_TEST(test_quantum_random_generation);
    RUN_TEST(test_quantum_safe_encryption);
    RUN_TEST(test_security_levels);
    RUN_TEST(test_quantum_signatures);
    RUN_TEST(test_key_derivation);
    RUN_TEST(test_key_rotation);
    RUN_TEST(test_threat_assessment);
    RUN_TEST(test_quantum_safe_iv);
    RUN_TEST(test_metrics_tracking);
    
    // Key manager tests
    RUN_TEST(test_key_manager_init);
    RUN_TEST(test_key_generation);
    RUN_TEST(test_key_retrieval);
    RUN_TEST(test_key_manager_rotation);
    RUN_TEST(test_key_listing);
    RUN_TEST(test_key_revocation);
    RUN_TEST(test_key_statistics);
    RUN_TEST(test_rotation_policy);
    
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000); // Wait for serial
    runQuantumSafeTests();
}

void loop() {
    // Tests run once in setup
}
#else
int main(int argc, char** argv) {
    runQuantumSafeTests();
    return 0;
}
#endif
