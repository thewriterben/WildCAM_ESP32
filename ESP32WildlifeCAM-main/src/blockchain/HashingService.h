#ifndef BLOCKCHAIN_HASHING_SERVICE_H
#define BLOCKCHAIN_HASHING_SERVICE_H

#include <Arduino.h>
#include <mbedtls/sha256.h>

/**
 * @brief Optimized SHA-256 hashing service for blockchain operations
 * 
 * Leverages existing mbedTLS implementation from OTA manager with
 * optimizations for wildlife camera blockchain use cases.
 */
class HashingService {
public:
    HashingService();
    ~HashingService();

    /**
     * @brief Initialize the hashing service
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Calculate SHA-256 hash of string data
     * @param data Input data to hash
     * @return Hex-encoded hash string (64 characters)
     */
    String calculateHash(const String& data);

    /**
     * @brief Calculate SHA-256 hash of binary data
     * @param data Pointer to data buffer
     * @param length Length of data in bytes
     * @return Hex-encoded hash string (64 characters)
     */
    String calculateHash(const uint8_t* data, size_t length);

    /**
     * @brief Calculate hash of file contents
     * @param filename Path to file
     * @return Hex-encoded hash string, empty on error
     */
    String calculateFileHash(const String& filename);

    /**
     * @brief Calculate combined hash of multiple inputs (for merkle tree)
     * @param leftHash Left child hash
     * @param rightHash Right child hash
     * @return Combined hash
     */
    String calculateCombinedHash(const String& leftHash, const String& rightHash);

    /**
     * @brief Verify data against expected hash
     * @param data Data to verify
     * @param expectedHash Expected hash value
     * @return true if hash matches
     */
    bool verifyHash(const String& data, const String& expectedHash);

    /**
     * @brief Verify binary data against expected hash
     * @param data Pointer to data buffer
     * @param length Length of data in bytes
     * @param expectedHash Expected hash value
     * @return true if hash matches
     */
    bool verifyHash(const uint8_t* data, size_t length, const String& expectedHash);

    /**
     * @brief Calculate incremental hash for large data
     * @param data Data chunk to add to hash
     * @param isFirst True if this is the first chunk
     * @param isLast True if this is the last chunk
     * @return Hex-encoded hash string (only valid when isLast=true)
     */
    String calculateIncrementalHash(const String& data, bool isFirst, bool isLast);

    /**
     * @brief Get hash performance statistics
     * @param hashCount Number of hashes calculated
     * @param totalTime Total time spent hashing (ms)
     * @param avgTime Average time per hash (ms)
     */
    void getHashStats(uint32_t& hashCount, uint32_t& totalTime, float& avgTime) const;

    /**
     * @brief Reset performance statistics
     */
    void resetStats();

    /**
     * @brief Check if service is initialized
     * @return true if initialized and ready
     */
    bool isInitialized() const { return initialized_; }

private:
    bool initialized_;
    mbedtls_sha256_context incrementalContext_;
    bool incrementalActive_;

    // Performance tracking
    uint32_t hashCount_;
    uint32_t totalHashTime_;

    /**
     * @brief Convert hash bytes to hex string
     * @param hash 32-byte hash array
     * @return Hex-encoded string
     */
    String hashToHexString(const uint8_t* hash) const;

    /**
     * @brief Internal hash calculation
     * @param data Data to hash
     * @param length Data length
     * @param output Output buffer (32 bytes)
     * @return true if successful
     */
    bool calculateSHA256(const uint8_t* data, size_t length, uint8_t* output);
};

#endif // BLOCKCHAIN_HASHING_SERVICE_H