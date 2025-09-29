#include "HashingService.h"
#include "../utils/logger.h"
#include <SD_MMC.h>

HashingService::HashingService() 
    : initialized_(false), incrementalActive_(false), hashCount_(0), totalHashTime_(0) {
    mbedtls_sha256_init(&incrementalContext_);
}

HashingService::~HashingService() {
    mbedtls_sha256_free(&incrementalContext_);
}

bool HashingService::initialize() {
    if (initialized_) {
        return true;
    }

    // Test basic hash functionality
    String testHash = calculateHash("test");
    if (testHash.length() == 64) {  // Valid SHA-256 hex string length
        initialized_ = true;
        Logger::log("HashingService: Initialized successfully");
        return true;
    }

    Logger::log("HashingService: Initialization failed");
    return false;
}

String HashingService::calculateHash(const String& data) {
    return calculateHash((const uint8_t*)data.c_str(), data.length());
}

String HashingService::calculateHash(const uint8_t* data, size_t length) {
    if (!initialized_ || !data || length == 0) {
        return "";
    }

    uint32_t startTime = millis();
    uint8_t hash[32];

    if (!calculateSHA256(data, length, hash)) {
        return "";
    }

    // Update performance stats
    hashCount_++;
    totalHashTime_ += (millis() - startTime);

    return hashToHexString(hash);
}

String HashingService::calculateFileHash(const String& filename) {
    if (!initialized_) {
        return "";
    }

    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        Logger::log("HashingService: Failed to open file: " + filename);
        return "";
    }

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);  // SHA256, not SHA224

    uint8_t buffer[1024];  // 1KB buffer for file reading
    size_t bytesRead;

    while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
        mbedtls_sha256_update(&ctx, buffer, bytesRead);
    }

    uint8_t hash[32];
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    file.close();

    hashCount_++;
    return hashToHexString(hash);
}

String HashingService::calculateCombinedHash(const String& leftHash, const String& rightHash) {
    if (!initialized_ || leftHash.length() != 64 || rightHash.length() != 64) {
        return "";
    }

    String combined = leftHash + rightHash;
    return calculateHash(combined);
}

bool HashingService::verifyHash(const String& data, const String& expectedHash) {
    if (!initialized_ || expectedHash.length() != 64) {
        return false;
    }

    String calculatedHash = calculateHash(data);
    return calculatedHash.equalsIgnoreCase(expectedHash);
}

bool HashingService::verifyHash(const uint8_t* data, size_t length, const String& expectedHash) {
    if (!initialized_ || expectedHash.length() != 64) {
        return false;
    }

    String calculatedHash = calculateHash(data, length);
    return calculatedHash.equalsIgnoreCase(expectedHash);
}

String HashingService::calculateIncrementalHash(const String& data, bool isFirst, bool isLast) {
    if (!initialized_) {
        return "";
    }

    if (isFirst) {
        if (incrementalActive_) {
            mbedtls_sha256_free(&incrementalContext_);
        }
        mbedtls_sha256_init(&incrementalContext_);
        mbedtls_sha256_starts(&incrementalContext_, 0);
        incrementalActive_ = true;
    }

    if (!incrementalActive_) {
        return "";
    }

    mbedtls_sha256_update(&incrementalContext_, (const uint8_t*)data.c_str(), data.length());

    if (isLast) {
        uint8_t hash[32];
        mbedtls_sha256_finish(&incrementalContext_, hash);
        mbedtls_sha256_free(&incrementalContext_);
        incrementalActive_ = false;
        hashCount_++;
        return hashToHexString(hash);
    }

    return "";  // Return empty string for intermediate chunks
}

void HashingService::getHashStats(uint32_t& hashCount, uint32_t& totalTime, float& avgTime) const {
    hashCount = hashCount_;
    totalTime = totalHashTime_;
    avgTime = hashCount_ > 0 ? (float)totalHashTime_ / hashCount_ : 0.0;
}

void HashingService::resetStats() {
    hashCount_ = 0;
    totalHashTime_ = 0;
}

String HashingService::hashToHexString(const uint8_t* hash) const {
    String result = "";
    for (int i = 0; i < 32; i++) {
        if (hash[i] < 16) {
            result += "0";
        }
        result += String(hash[i], HEX);
    }
    result.toUpperCase();
    return result;
}

bool HashingService::calculateSHA256(const uint8_t* data, size_t length, uint8_t* output) {
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);  // SHA256, not SHA224
    mbedtls_sha256_update(&ctx, data, length);
    mbedtls_sha256_finish(&ctx, output);
    mbedtls_sha256_free(&ctx);
    return true;
}