/**
 * @file quantum_safe_crypto.h
 * @brief Quantum-Safe Cryptography Framework for ESP32
 * @version 1.0.0
 * 
 * This module provides a quantum-resistant security framework for wildlife
 * monitoring data protection. It implements hybrid classical-quantum resistant
 * cryptography suitable for ESP32 constraints while preparing for future
 * quantum-safe hardware integration.
 * 
 * Key Features:
 * - Hybrid classical + post-quantum encryption
 * - Hash-based signature schemes
 * - Quantum-resistant key derivation
 * - Forward secrecy with quantum safety
 * - Cryptographic agility for algorithm updates
 */

#ifndef QUANTUM_SAFE_CRYPTO_H
#define QUANTUM_SAFE_CRYPTO_H

#include <Arduino.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

/**
 * @brief Quantum security levels
 */
enum class QuantumSecurityLevel {
    CLASSICAL_ONLY,          // Traditional cryptography (AES-256)
    HYBRID_TRANSITION,       // Hybrid classical + quantum-resistant
    POST_QUANTUM_READY,      // Full post-quantum algorithms
    QUANTUM_SAFE_MAXIMUM     // Maximum quantum resistance
};

/**
 * @brief Post-quantum algorithm types
 */
enum class PostQuantumAlgorithm {
    NONE,                    // No post-quantum algorithm
    HASH_BASED_SIG,          // Hash-based signatures (SPHINCS+-like)
    LATTICE_BASED,           // Lattice-based (Kyber-like - future)
    CODE_BASED,              // Code-based (McEliece-like - future)
    MULTIVARIATE,            // Multivariate polynomials (future)
    HYBRID_AES_PQ            // AES-256 + Post-quantum
};

/**
 * @brief Quantum key derivation parameters
 */
struct QuantumKeyParams {
    uint32_t iterations;      // PBKDF2 iterations (minimum 100000 for quantum resistance)
    uint8_t salt[32];         // 256-bit salt
    uint8_t info[64];         // Context information
    size_t info_len;          // Length of context info
    bool use_hardware_rng;    // Use hardware RNG for quantum randomness
};

/**
 * @brief Quantum-safe signature context
 */
struct QuantumSignatureContext {
    uint8_t public_key[64];   // Public key (hash-based)
    uint8_t private_key[128]; // Private key (hash-based, larger for quantum safety)
    uint32_t key_version;     // Key version for agility
    PostQuantumAlgorithm algorithm; // Algorithm used
    bool is_valid;            // Context validity
};

/**
 * @brief Quantum-safe encryption context
 */
struct QuantumEncryptionContext {
    uint8_t classical_key[32];    // AES-256 key
    uint8_t pq_key_material[128]; // Post-quantum key material
    uint8_t iv[16];               // Initialization vector
    QuantumSecurityLevel level;   // Security level
    PostQuantumAlgorithm pq_algo; // Post-quantum algorithm
    uint32_t nonce_counter;       // Nonce counter for forward secrecy
};

/**
 * @brief Quantum threat detection metrics
 */
struct QuantumThreatMetrics {
    unsigned long total_operations;
    unsigned long quantum_safe_operations;
    unsigned long hybrid_operations;
    unsigned long potential_quantum_threats;
    unsigned long algorithm_updates;
    uint32_t current_security_level;
};

/**
 * @brief Quantum-Safe Cryptography Manager
 * 
 * Provides quantum-resistant cryptographic operations optimized for ESP32.
 * Implements hybrid encryption schemes and prepares for future quantum
 * hardware integration.
 */
class QuantumSafeCrypto {
private:
    // Cryptographic contexts
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    
    // Configuration
    QuantumSecurityLevel security_level_;
    bool hardware_rng_available_;
    bool quantum_safe_mode_enabled_;
    
    // Key management
    uint8_t master_key_[64];           // 512-bit master key for quantum safety
    uint8_t current_session_key_[32];  // Current session key
    uint32_t key_generation_counter_;   // For key versioning
    
    // Hash-based signature state
    QuantumSignatureContext signature_ctx_;
    
    // Metrics
    QuantumThreatMetrics metrics_;
    unsigned long last_key_rotation_;
    
    // Internal methods
    bool deriveQuantumSafeKey(const uint8_t* input, size_t input_len,
                             const QuantumKeyParams& params,
                             uint8_t* output, size_t output_len);
    
    bool generateHashBasedSignature(const uint8_t* message, size_t msg_len,
                                   uint8_t* signature, size_t* sig_len);
    
    bool verifyHashBasedSignature(const uint8_t* message, size_t msg_len,
                                 const uint8_t* signature, size_t sig_len,
                                 const uint8_t* public_key);
    
    void updateQuantumThreatMetrics();
    
    bool performHybridEncryption(const uint8_t* plaintext, size_t plain_len,
                                uint8_t* ciphertext, size_t* cipher_len,
                                const QuantumEncryptionContext& ctx);
    
    bool performHybridDecryption(const uint8_t* ciphertext, size_t cipher_len,
                                uint8_t* plaintext, size_t* plain_len,
                                const QuantumEncryptionContext& ctx);

public:
    /**
     * @brief Constructor
     * @param level Initial quantum security level
     */
    QuantumSafeCrypto(QuantumSecurityLevel level = QuantumSecurityLevel::HYBRID_TRANSITION);
    
    /**
     * @brief Destructor - securely wipe sensitive data
     */
    ~QuantumSafeCrypto();
    
    /**
     * @brief Initialize quantum-safe cryptography system
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Set quantum security level
     * @param level Desired security level
     * @return true if level change successful
     */
    bool setSecurityLevel(QuantumSecurityLevel level);
    
    /**
     * @brief Get current quantum security level
     * @return Current security level
     */
    QuantumSecurityLevel getSecurityLevel() const { return security_level_; }
    
    /**
     * @brief Generate quantum-resistant random data
     * @param output Output buffer
     * @param length Number of bytes to generate
     * @return true if generation successful
     */
    bool generateQuantumRandom(uint8_t* output, size_t length);
    
    /**
     * @brief Derive quantum-safe key from passphrase
     * @param passphrase Input passphrase
     * @param pass_len Passphrase length
     * @param params Key derivation parameters
     * @param output_key Output key buffer
     * @param key_len Desired key length
     * @return true if derivation successful
     */
    bool deriveKey(const uint8_t* passphrase, size_t pass_len,
                   const QuantumKeyParams& params,
                   uint8_t* output_key, size_t key_len);
    
    /**
     * @brief Encrypt data with quantum-safe hybrid scheme
     * @param plaintext Input data
     * @param plain_len Input data length
     * @param ciphertext Output encrypted data
     * @param cipher_len Output length
     * @param context Encryption context
     * @return true if encryption successful
     */
    bool encryptQuantumSafe(const uint8_t* plaintext, size_t plain_len,
                           uint8_t* ciphertext, size_t* cipher_len,
                           QuantumEncryptionContext& context);
    
    /**
     * @brief Decrypt data encrypted with quantum-safe hybrid scheme
     * @param ciphertext Input encrypted data
     * @param cipher_len Input length
     * @param plaintext Output decrypted data
     * @param plain_len Output length
     * @param context Decryption context
     * @return true if decryption successful
     */
    bool decryptQuantumSafe(const uint8_t* ciphertext, size_t cipher_len,
                           uint8_t* plaintext, size_t* plain_len,
                           const QuantumEncryptionContext& context);
    
    /**
     * @brief Generate hash-based quantum-resistant signature
     * @param message Message to sign
     * @param msg_len Message length
     * @param signature Output signature
     * @param sig_len Signature length
     * @param context Signature context
     * @return true if signing successful
     */
    bool signMessage(const uint8_t* message, size_t msg_len,
                    uint8_t* signature, size_t* sig_len,
                    QuantumSignatureContext& context);
    
    /**
     * @brief Verify hash-based quantum-resistant signature
     * @param message Original message
     * @param msg_len Message length
     * @param signature Signature to verify
     * @param sig_len Signature length
     * @param context Signature context with public key
     * @return true if signature is valid
     */
    bool verifySignature(const uint8_t* message, size_t msg_len,
                        const uint8_t* signature, size_t sig_len,
                        const QuantumSignatureContext& context);
    
    /**
     * @brief Generate quantum-resistant key pair
     * @param context Output signature context
     * @param algorithm Post-quantum algorithm to use
     * @return true if key generation successful
     */
    bool generateKeyPair(QuantumSignatureContext& context,
                        PostQuantumAlgorithm algorithm = PostQuantumAlgorithm::HASH_BASED_SIG);
    
    /**
     * @brief Rotate encryption keys with quantum-safe properties
     * @return true if rotation successful
     */
    bool rotateKeys();
    
    /**
     * @brief Check if quantum-safe mode is operational
     * @return true if system is operational
     */
    bool isQuantumSafeOperational() const { return quantum_safe_mode_enabled_; }
    
    /**
     * @brief Get quantum threat metrics
     * @return Current threat metrics
     */
    QuantumThreatMetrics getMetrics() const { return metrics_; }
    
    /**
     * @brief Assess quantum threat level for current configuration
     * @return Threat level (0-100, higher = more vulnerable)
     */
    uint8_t assessQuantumThreatLevel() const;
    
    /**
     * @brief Generate quantum-safe initialization vector
     * @param iv Output IV buffer (16 bytes)
     * @return true if generation successful
     */
    bool generateQuantumSafeIV(uint8_t iv[16]);
    
    /**
     * @brief Perform quantum-resistant key exchange (preparation)
     * @param peer_public_key Peer's public key
     * @param peer_key_len Peer public key length
     * @param shared_secret Output shared secret
     * @param secret_len Output secret length
     * @return true if key exchange successful
     */
    bool performQuantumSafeKeyExchange(const uint8_t* peer_public_key, size_t peer_key_len,
                                      uint8_t* shared_secret, size_t* secret_len);
    
    /**
     * @brief Securely wipe memory
     * @param buffer Buffer to wipe
     * @param length Buffer length
     */
    void secureWipe(uint8_t* buffer, size_t length);
};

#endif // QUANTUM_SAFE_CRYPTO_H
