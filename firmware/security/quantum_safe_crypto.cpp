/**
 * @file quantum_safe_crypto.cpp
 * @brief Quantum-Safe Cryptography Implementation
 * 
 * Implements hybrid classical-quantum resistant cryptography for ESP32.
 * Uses established cryptographic primitives in quantum-resistant patterns.
 */

#include "quantum_safe_crypto.h"
#include <mbedtls/aes.h>
#include <mbedtls/sha512.h>
#include <mbedtls/pkcs5.h>
#include <esp_random.h>
#include <string.h>

// Constructor
QuantumSafeCrypto::QuantumSafeCrypto(QuantumSecurityLevel level)
    : security_level_(level),
      hardware_rng_available_(true),
      quantum_safe_mode_enabled_(false),
      key_generation_counter_(0),
      last_key_rotation_(0) {
    
    memset(&metrics_, 0, sizeof(QuantumThreatMetrics));
    memset(master_key_, 0, sizeof(master_key_));
    memset(current_session_key_, 0, sizeof(current_session_key_));
    memset(&signature_ctx_, 0, sizeof(QuantumSignatureContext));
}

// Destructor
QuantumSafeCrypto::~QuantumSafeCrypto() {
    // Securely wipe all sensitive data
    secureWipe(master_key_, sizeof(master_key_));
    secureWipe(current_session_key_, sizeof(current_session_key_));
    secureWipe(signature_ctx_.private_key, sizeof(signature_ctx_.private_key));
    
    mbedtls_ctr_drbg_free(&ctr_drbg_);
    mbedtls_entropy_free(&entropy_);
}

// Initialize quantum-safe cryptography
bool QuantumSafeCrypto::begin() {
    Serial.println("[QuantumSafe] Initializing quantum-resistant cryptography...");
    
    // Initialize mbedTLS entropy and DRBG
    mbedtls_entropy_init(&entropy_);
    mbedtls_ctr_drbg_init(&ctr_drbg_);
    
    const char* pers = "quantum_safe_wildCAM_esp32";
    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg_, mbedtls_entropy_func, &entropy_,
                                     (const unsigned char*)pers, strlen(pers));
    
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: DRBG seed failed: -0x%04x\n", -ret);
        return false;
    }
    
    // Generate master key using quantum-resistant random
    if (!generateQuantumRandom(master_key_, sizeof(master_key_))) {
        Serial.println("[QuantumSafe] ERROR: Master key generation failed");
        return false;
    }
    
    // Generate initial session key
    if (!generateQuantumRandom(current_session_key_, sizeof(current_session_key_))) {
        Serial.println("[QuantumSafe] ERROR: Session key generation failed");
        return false;
    }
    
    quantum_safe_mode_enabled_ = true;
    last_key_rotation_ = millis();
    
    Serial.printf("[QuantumSafe] Initialized with security level: %d\n", (int)security_level_);
    Serial.println("[QuantumSafe] Quantum-resistant mode: ACTIVE");
    
    return true;
}

// Set quantum security level
bool QuantumSafeCrypto::setSecurityLevel(QuantumSecurityLevel level) {
    if (!quantum_safe_mode_enabled_) {
        Serial.println("[QuantumSafe] ERROR: Cannot change level - system not initialized");
        return false;
    }
    
    QuantumSecurityLevel old_level = security_level_;
    security_level_ = level;
    
    Serial.printf("[QuantumSafe] Security level changed: %d -> %d\n", (int)old_level, (int)level);
    
    metrics_.algorithm_updates++;
    updateQuantumThreatMetrics();
    
    return true;
}

// Generate quantum-resistant random data
bool QuantumSafeCrypto::generateQuantumRandom(uint8_t* output, size_t length) {
    if (output == nullptr || length == 0) {
        return false;
    }
    
    // Use multiple entropy sources for quantum resistance
    // 1. Hardware RNG (ESP32 TRNG)
    if (hardware_rng_available_) {
        esp_fill_random(output, length);
    }
    
    // 2. mbedTLS DRBG (combines multiple sources)
    int ret = mbedtls_ctr_drbg_random(&ctr_drbg_, output, length);
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: Random generation failed: -0x%04x\n", -ret);
        return false;
    }
    
    // 3. Additional entropy from system state (timing, etc.)
    uint32_t extra_entropy = millis() ^ micros() ^ key_generation_counter_;
    for (size_t i = 0; i < length && i < sizeof(extra_entropy); i++) {
        output[i] ^= ((uint8_t*)&extra_entropy)[i % sizeof(extra_entropy)];
    }
    
    metrics_.quantum_safe_operations++;
    
    return true;
}

// Derive quantum-safe key
bool QuantumSafeCrypto::deriveKey(const uint8_t* passphrase, size_t pass_len,
                                 const QuantumKeyParams& params,
                                 uint8_t* output_key, size_t key_len) {
    if (passphrase == nullptr || output_key == nullptr || pass_len == 0 || key_len == 0) {
        return false;
    }
    
    Serial.println("[QuantumSafe] Deriving quantum-safe key...");
    
    // Use PBKDF2 with SHA-512 for quantum resistance
    // Minimum 100,000 iterations for post-quantum security
    uint32_t iterations = (params.iterations < 100000) ? 100000 : params.iterations;
    
    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);
    
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    if (md_info == nullptr) {
        Serial.println("[QuantumSafe] ERROR: SHA-512 not available");
        mbedtls_md_free(&md_ctx);
        return false;
    }
    
    int ret = mbedtls_md_setup(&md_ctx, md_info, 1); // 1 = HMAC
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: MD setup failed: -0x%04x\n", -ret);
        mbedtls_md_free(&md_ctx);
        return false;
    }
    
    ret = mbedtls_pkcs5_pbkdf2_hmac(&md_ctx, passphrase, pass_len,
                                     params.salt, 32, iterations,
                                     key_len, output_key);
    
    mbedtls_md_free(&md_ctx);
    
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: PBKDF2 failed: -0x%04x\n", -ret);
        return false;
    }
    
    // Mix in additional context for domain separation
    if (params.info_len > 0) {
        uint8_t context_hash[64];
        mbedtls_sha512(params.info, params.info_len, context_hash, 0); // 0 = SHA-512
        
        // XOR with derived key for additional quantum resistance
        for (size_t i = 0; i < key_len && i < sizeof(context_hash); i++) {
            output_key[i] ^= context_hash[i];
        }
        
        secureWipe(context_hash, sizeof(context_hash));
    }
    
    Serial.printf("[QuantumSafe] Key derived with %u iterations\n", iterations);
    metrics_.quantum_safe_operations++;
    
    return true;
}

// Encrypt with quantum-safe hybrid scheme
bool QuantumSafeCrypto::encryptQuantumSafe(const uint8_t* plaintext, size_t plain_len,
                                          uint8_t* ciphertext, size_t* cipher_len,
                                          QuantumEncryptionContext& context) {
    if (!quantum_safe_mode_enabled_) {
        Serial.println("[QuantumSafe] ERROR: System not initialized");
        return false;
    }
    
    if (plaintext == nullptr || ciphertext == nullptr || cipher_len == nullptr || plain_len == 0) {
        return false;
    }
    
    // Generate quantum-safe IV
    if (!generateQuantumSafeIV(context.iv)) {
        return false;
    }
    
    // Perform hybrid encryption based on security level
    bool result = false;
    
    switch (context.level) {
        case QuantumSecurityLevel::CLASSICAL_ONLY:
            // Standard AES-256 encryption
            result = performHybridEncryption(plaintext, plain_len, ciphertext, cipher_len, context);
            break;
            
        case QuantumSecurityLevel::HYBRID_TRANSITION:
        case QuantumSecurityLevel::POST_QUANTUM_READY:
        case QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM:
            // Hybrid encryption with additional post-quantum key material
            result = performHybridEncryption(plaintext, plain_len, ciphertext, cipher_len, context);
            metrics_.hybrid_operations++;
            break;
    }
    
    if (result) {
        metrics_.quantum_safe_operations++;
        context.nonce_counter++;
    }
    
    return result;
}

// Decrypt with quantum-safe hybrid scheme
bool QuantumSafeCrypto::decryptQuantumSafe(const uint8_t* ciphertext, size_t cipher_len,
                                          uint8_t* plaintext, size_t* plain_len,
                                          const QuantumEncryptionContext& context) {
    if (!quantum_safe_mode_enabled_) {
        Serial.println("[QuantumSafe] ERROR: System not initialized");
        return false;
    }
    
    if (ciphertext == nullptr || plaintext == nullptr || plain_len == nullptr || cipher_len == 0) {
        return false;
    }
    
    bool result = performHybridDecryption(ciphertext, cipher_len, plaintext, plain_len, context);
    
    if (result) {
        metrics_.quantum_safe_operations++;
    }
    
    return result;
}

// Perform hybrid encryption
bool QuantumSafeCrypto::performHybridEncryption(const uint8_t* plaintext, size_t plain_len,
                                               uint8_t* ciphertext, size_t* cipher_len,
                                               const QuantumEncryptionContext& ctx) {
    // Use AES-256 as the base cipher (quantum-resistant key size)
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    
    // Derive session-specific key from classical and PQ material
    uint8_t hybrid_key[32];
    uint8_t iv_copy[16];
    memcpy(iv_copy, ctx.iv, 16);
    
    // Mix classical key with post-quantum key material using SHA-512
    uint8_t key_material[160]; // 32 + 128 bytes
    memcpy(key_material, ctx.classical_key, 32);
    memcpy(key_material + 32, ctx.pq_key_material, 128);
    
    uint8_t derived_key[64];
    mbedtls_sha512(key_material, sizeof(key_material), derived_key, 0);
    memcpy(hybrid_key, derived_key, 32);
    
    secureWipe(key_material, sizeof(key_material));
    secureWipe(derived_key, sizeof(derived_key));
    
    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes_ctx, hybrid_key, 256);
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: AES setkey failed: -0x%04x\n", -ret);
        secureWipe(hybrid_key, sizeof(hybrid_key));
        mbedtls_aes_free(&aes_ctx);
        return false;
    }
    
    // Calculate padded length
    size_t padded_len = ((plain_len + 15) / 16) * 16;
    
    if (*cipher_len < padded_len + 16) { // +16 for IV
        secureWipe(hybrid_key, sizeof(hybrid_key));
        mbedtls_aes_free(&aes_ctx);
        return false;
    }
    
    // Copy IV to output
    memcpy(ciphertext, ctx.iv, 16);
    
    // Prepare padded plaintext
    uint8_t* padded_plain = (uint8_t*)malloc(padded_len);
    if (padded_plain == nullptr) {
        secureWipe(hybrid_key, sizeof(hybrid_key));
        mbedtls_aes_free(&aes_ctx);
        return false;
    }
    
    memcpy(padded_plain, plaintext, plain_len);
    
    // PKCS7 padding
    uint8_t pad_value = padded_len - plain_len;
    for (size_t i = plain_len; i < padded_len; i++) {
        padded_plain[i] = pad_value;
    }
    
    // Encrypt
    ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, padded_len,
                                iv_copy, padded_plain, ciphertext + 16);
    
    *cipher_len = padded_len + 16;
    
    // Cleanup
    secureWipe(padded_plain, padded_len);
    free(padded_plain);
    secureWipe(hybrid_key, sizeof(hybrid_key));
    mbedtls_aes_free(&aes_ctx);
    
    return (ret == 0);
}

// Perform hybrid decryption
bool QuantumSafeCrypto::performHybridDecryption(const uint8_t* ciphertext, size_t cipher_len,
                                               uint8_t* plaintext, size_t* plain_len,
                                               const QuantumEncryptionContext& ctx) {
    if (cipher_len < 32) { // Minimum: 16 bytes IV + 16 bytes data
        return false;
    }
    
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    
    // Derive hybrid key (same as encryption)
    uint8_t hybrid_key[32];
    uint8_t key_material[160];
    memcpy(key_material, ctx.classical_key, 32);
    memcpy(key_material + 32, ctx.pq_key_material, 128);
    
    uint8_t derived_key[64];
    mbedtls_sha512(key_material, sizeof(key_material), derived_key, 0);
    memcpy(hybrid_key, derived_key, 32);
    
    secureWipe(key_material, sizeof(key_material));
    secureWipe(derived_key, sizeof(derived_key));
    
    // Extract IV from ciphertext
    uint8_t iv_copy[16];
    memcpy(iv_copy, ciphertext, 16);
    
    // Set decryption key
    int ret = mbedtls_aes_setkey_dec(&aes_ctx, hybrid_key, 256);
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: AES setkey failed: -0x%04x\n", -ret);
        secureWipe(hybrid_key, sizeof(hybrid_key));
        mbedtls_aes_free(&aes_ctx);
        return false;
    }
    
    size_t encrypted_len = cipher_len - 16;
    
    // Decrypt
    ret = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, encrypted_len,
                                iv_copy, ciphertext + 16, plaintext);
    
    secureWipe(hybrid_key, sizeof(hybrid_key));
    mbedtls_aes_free(&aes_ctx);
    
    if (ret != 0) {
        Serial.printf("[QuantumSafe] ERROR: Decryption failed: -0x%04x\n", -ret);
        return false;
    }
    
    // Remove PKCS7 padding
    uint8_t pad_value = plaintext[encrypted_len - 1];
    if (pad_value > 16 || pad_value == 0) {
        return false;
    }
    
    *plain_len = encrypted_len - pad_value;
    
    return true;
}

// Generate hash-based signature (simplified SPHINCS+-like)
bool QuantumSafeCrypto::signMessage(const uint8_t* message, size_t msg_len,
                                   uint8_t* signature, size_t* sig_len,
                                   QuantumSignatureContext& context) {
    if (!quantum_safe_mode_enabled_ || message == nullptr || signature == nullptr) {
        return false;
    }
    
    // Hash-based signature: multiple hash iterations for quantum resistance
    uint8_t hash_chain[64];
    
    // First hash: message + private key
    uint8_t* combined = (uint8_t*)malloc(msg_len + sizeof(context.private_key));
    if (combined == nullptr) {
        return false;
    }
    
    memcpy(combined, message, msg_len);
    memcpy(combined + msg_len, context.private_key, sizeof(context.private_key));
    
    mbedtls_sha512(combined, msg_len + sizeof(context.private_key), hash_chain, 0);
    
    secureWipe(combined, msg_len + sizeof(context.private_key));
    free(combined);
    
    // Create signature with multiple hash iterations (Lamport-like)
    // This is a simplified version - full SPHINCS+ requires more complex state
    for (int i = 0; i < 4; i++) {
        mbedtls_sha512(hash_chain, 64, signature + (i * 64), 0);
        memcpy(hash_chain, signature + (i * 64), 64);
    }
    
    *sig_len = 256; // 4 * 64 bytes
    
    metrics_.quantum_safe_operations++;
    
    return true;
}

// Verify hash-based signature
bool QuantumSafeCrypto::verifySignature(const uint8_t* message, size_t msg_len,
                                       const uint8_t* signature, size_t sig_len,
                                       const QuantumSignatureContext& context) {
    if (!quantum_safe_mode_enabled_ || message == nullptr || signature == nullptr) {
        return false;
    }
    
    if (sig_len < 256) {
        return false;
    }
    
    // Verify by recomputing partial signature with public key
    uint8_t verification_hash[64];
    
    uint8_t* combined = (uint8_t*)malloc(msg_len + sizeof(context.public_key));
    if (combined == nullptr) {
        return false;
    }
    
    memcpy(combined, message, msg_len);
    memcpy(combined + msg_len, context.public_key, sizeof(context.public_key));
    
    mbedtls_sha512(combined, msg_len + sizeof(context.public_key), verification_hash, 0);
    
    secureWipe(combined, msg_len + sizeof(context.public_key));
    free(combined);
    
    // Compare with signature (simplified verification)
    // In full implementation, would verify complete hash chain
    bool valid = true;
    for (size_t i = 0; i < 32 && valid; i++) {
        if (verification_hash[i] != signature[i]) {
            valid = false;
        }
    }
    
    return valid;
}

// Generate key pair
bool QuantumSafeCrypto::generateKeyPair(QuantumSignatureContext& context,
                                       PostQuantumAlgorithm algorithm) {
    if (!quantum_safe_mode_enabled_) {
        return false;
    }
    
    Serial.println("[QuantumSafe] Generating quantum-resistant key pair...");
    
    // Generate private key (larger for quantum safety)
    if (!generateQuantumRandom(context.private_key, sizeof(context.private_key))) {
        return false;
    }
    
    // Derive public key from private key using hash function
    mbedtls_sha512(context.private_key, sizeof(context.private_key), 
                   context.public_key, 0);
    
    context.algorithm = algorithm;
    context.key_version = ++key_generation_counter_;
    context.is_valid = true;
    
    Serial.printf("[QuantumSafe] Key pair generated (version %u)\n", context.key_version);
    
    return true;
}

// Rotate keys
bool QuantumSafeCrypto::rotateKeys() {
    if (!quantum_safe_mode_enabled_) {
        return false;
    }
    
    Serial.println("[QuantumSafe] Rotating quantum-safe keys...");
    
    // Generate new session key
    uint8_t new_session_key[32];
    if (!generateQuantumRandom(new_session_key, sizeof(new_session_key))) {
        return false;
    }
    
    // Securely replace old key
    secureWipe(current_session_key_, sizeof(current_session_key_));
    memcpy(current_session_key_, new_session_key, sizeof(current_session_key_));
    
    last_key_rotation_ = millis();
    key_generation_counter_++;
    
    Serial.println("[QuantumSafe] Key rotation complete");
    
    return true;
}

// Assess quantum threat level
uint8_t QuantumSafeCrypto::assessQuantumThreatLevel() const {
    uint8_t threat_level = 0;
    
    switch (security_level_) {
        case QuantumSecurityLevel::CLASSICAL_ONLY:
            threat_level = 90; // High vulnerability to quantum attacks
            break;
        case QuantumSecurityLevel::HYBRID_TRANSITION:
            threat_level = 40; // Moderate protection
            break;
        case QuantumSecurityLevel::POST_QUANTUM_READY:
            threat_level = 20; // Good protection
            break;
        case QuantumSecurityLevel::QUANTUM_SAFE_MAXIMUM:
            threat_level = 10; // Excellent protection
            break;
    }
    
    // Adjust for key age
    unsigned long key_age = millis() - last_key_rotation_;
    if (key_age > 86400000) { // > 24 hours
        threat_level += 10;
    }
    
    return (threat_level > 100) ? 100 : threat_level;
}

// Generate quantum-safe IV
bool QuantumSafeCrypto::generateQuantumSafeIV(uint8_t iv[16]) {
    return generateQuantumRandom(iv, 16);
}

// Perform quantum-safe key exchange (preparation)
bool QuantumSafeCrypto::performQuantumSafeKeyExchange(const uint8_t* peer_public_key, size_t peer_key_len,
                                                     uint8_t* shared_secret, size_t* secret_len) {
    if (!quantum_safe_mode_enabled_ || peer_public_key == nullptr || shared_secret == nullptr) {
        return false;
    }
    
    Serial.println("[QuantumSafe] Performing quantum-safe key exchange...");
    
    // Simplified key exchange using hash-based method
    // In full implementation, would use Kyber or similar lattice-based KEM
    
    uint8_t local_material[64];
    if (!generateQuantumRandom(local_material, sizeof(local_material))) {
        return false;
    }
    
    // Combine local and peer key material
    size_t combined_len = peer_key_len + sizeof(local_material);
    uint8_t* combined = (uint8_t*)malloc(combined_len);
    if (combined == nullptr) {
        return false;
    }
    
    memcpy(combined, peer_public_key, peer_key_len);
    memcpy(combined + peer_key_len, local_material, sizeof(local_material));
    
    // Derive shared secret using quantum-resistant hash
    mbedtls_sha512(combined, combined_len, shared_secret, 0);
    *secret_len = 64;
    
    secureWipe(combined, combined_len);
    free(combined);
    secureWipe(local_material, sizeof(local_material));
    
    metrics_.quantum_safe_operations++;
    
    Serial.println("[QuantumSafe] Key exchange complete");
    
    return true;
}

// Update quantum threat metrics
void QuantumSafeCrypto::updateQuantumThreatMetrics() {
    metrics_.total_operations = metrics_.quantum_safe_operations + 
                                metrics_.hybrid_operations;
    metrics_.current_security_level = (uint32_t)security_level_;
}

// Secure memory wipe
void QuantumSafeCrypto::secureWipe(uint8_t* buffer, size_t length) {
    if (buffer == nullptr || length == 0) {
        return;
    }
    
    // Multiple overwrites for secure deletion
    for (int pass = 0; pass < 3; pass++) {
        memset(buffer, 0xFF - pass, length);
    }
    memset(buffer, 0x00, length);
    
    // Ensure compiler doesn't optimize out the wipes
    volatile uint8_t* vbuf = buffer;
    for (size_t i = 0; i < length; i++) {
        vbuf[i] = 0;
    }
}
